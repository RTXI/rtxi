/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Will Cornell Medical College

         This program is free software: you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the Free Software Foundation, either version 3 of the License, or
         (at your option) any later version.

         This program is distributed in the hope that it will be useful,
         but WITHOUT ANY WARRANTY; without even the implied warranty of
         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
         GNU General Public License for more details.

         You should have received a copy of the GNU General Public License
         along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "rtos.hpp"

#include <debug.hpp>
#include <errno.h>
#include <evl/evl.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>

// NOLINTNEXTLINE
thread_local bool realtime_key = false;
// NOLINTNEXTLINE
thread_local int64_t* RT_PERIOD = nullptr;

int RT::OS::initiate(RT::OS::Task* task)
{
  std::string strbuf(256, '\0');
  int retval = evl_init();
  if (retval != 0) {
    strerror_r(errno, strbuf.data(), strbuf.size());
    ERROR_MSG("RT::OS(EVL)::initiate : evl_init() : {}", strbuf);
    return retval;
  }

  // set high affinity to obtain real-time guarantees
  struct sched_param param
  {
  };
  param.sched_priority = 8;
  retval = pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
  if (retval != 0) {
    ERROR_MSG("RT::OS(EVL)::initiate : Unable to set scheduling parameters");
    return retval;
  }

  int thread_fd = evl_attach_self("RTXI-RT-Thread:%d", getpid());  // NOLINT
  if (thread_fd < 0) {
    strerror_r(errno, strbuf.data(), strbuf.size());
    ERROR_MSG("RT::OS(EVL)::initiate : evl_attach_self() : {}", strbuf);
  }
  realtime_key = true;
  task->period = RT::OS::DEFAULT_PERIOD;
  RT_PERIOD = &(task->period);
  task->thread_id = std::any(thread_fd);
  return retval;
}

void RT::OS::shutdown(RT::OS::Task* task)
{
  int retval = evl_detach_self();
  std::string strbuf(256, '\0');
  if (retval != 0) {
    strerror_r(errno, strbuf.data(), strbuf.size());
    ERROR_MSG("Unable to detach thread from evl core!");
    ERROR_MSG("RT::OS(EVL)::shutdown : evl_detach_self() : {}", strbuf);
  }
  realtime_key = false;
  task->task_finished = true;
  RT_PERIOD = nullptr;
}

int RT::OS::createTask(Task* task, void (*func)(void*), void* arg)
{
  int result = 0;
  // Should not be creating real-time tasks from another real-time task
  if (RT::OS::isRealtime()) {
    ERROR_MSG("RT::OS::createTask : Task cannot be created from rt context");
    return -1;
  }
  auto wrapper = [](RT::OS::Task* tsk, void (*fn)(void*), void* args)
  {
    std::string strbuf(256, '\0');
    auto resval = RT::OS::initiate(tsk);
    if (resval != 0) {
      strerror_r(errno, strbuf.data(), strbuf.size());
      ERROR_MSG("RT::OS::createTask : RT::OS::initiate() : {}", strbuf);
      // In the event that we fail to initiate real-time environment let's just
      // quit
      return;
    }
    fn(args);
    RT::OS::shutdown(tsk);
  };
  std::thread thread_obj(wrapper, task, func, arg);
  if (thread_obj.joinable()) {
    task->rt_thread = std::move(thread_obj);
  } else {
    result = -1;
  }
  return result;
}

void RT::OS::deleteTask(RT::OS::Task* task)
{
  // Should not be deleting real-time tasks from another real-time task
  if (RT::OS::isRealtime()) {
    ERROR_MSG("RT::OS::createTask : Task cannot be deleted from rt context");
    return;
  }
  task->task_finished = true;
  if (task->rt_thread.joinable()) {
    task->rt_thread.join();
  }
}

bool RT::OS::isRealtime()
{
  return realtime_key;
}

int64_t RT::OS::getTime()
{
  timespec tp = {};

  evl_read_clock(EVL_CLOCK_MONOTONIC, &tp);

  return RT::OS::SECONDS_TO_NANOSECONDS * tp.tv_sec + tp.tv_nsec;
}

int RT::OS::setPeriod(RT::OS::Task* task, int64_t period)
{
  task->period = period;
  return 0;
}

int64_t RT::OS::getPeriod()
{
  // This function should only ever be accessed withint a real-tim context
  if (RT_PERIOD == nullptr || !RT::OS::isRealtime()) {
    return -1;
  };
  return *(RT_PERIOD);
}

void RT::OS::sleepTimestep(RT::OS::Task* task)
{
  const int64_t current_time = RT::OS::getTime();
  if (task->next_t < current_time) {
    task->next_t = current_time + task->period;
    return;
  }
  int64_t wakeup_time = task->next_t;
  task->next_t += task->period;

  const struct timespec ts = {wakeup_time / RT::OS::SECONDS_TO_NANOSECONDS,
                              wakeup_time % RT::OS::SECONDS_TO_NANOSECONDS};

  evl_sleep_until(EVL_CLOCK_MONOTONIC, &ts);
}

void RT::OS::renameOSThread(std::thread& thread, const std::string& name)
{
  if (RT::OS::isRealtime()) {
    return;
  }

  if (pthread_setname_np(thread.native_handle(), name.c_str()) != 0) {
    ERROR_MSG("RT::OS::renameOSThread : unable to set name to thread");
  }
}

// NOLINTNEXTLINE
timespec last_clock_read;
// NOLINTNEXTLINE
timespec last_proc_time;

double RT::OS::getCpuUsage()
{
  // Should not attempt this in the real-time thread
  if (RT::OS::isRealtime()) {
    return 0.0;
  }

  double cpu_percent = 0.0;
  int64_t cpu_time_elapsed = 0;
  int64_t proc_time_elapsed = 0;

  timespec clock_time = {};
  timespec proc_time = {};
  // rusage resource_usage;

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &proc_time);
  clock_gettime(CLOCK_REALTIME, &clock_time);
  // getrusage(RUSAGE_SELF, &resource_usage);

  cpu_time_elapsed = RT::OS::SECONDS_TO_NANOSECONDS
          * (clock_time.tv_sec - last_clock_read.tv_sec)
      + (clock_time.tv_nsec - last_clock_read.tv_nsec);
  if (cpu_time_elapsed <= 0) {
    return 0.0;
  }
  proc_time_elapsed = RT::OS::SECONDS_TO_NANOSECONDS
          * (proc_time.tv_sec - last_proc_time.tv_sec)
      + (proc_time.tv_nsec - last_proc_time.tv_nsec);
  cpu_percent = 100.0 * (static_cast<double>(proc_time_elapsed))
      / static_cast<double>(cpu_time_elapsed);

  last_proc_time = proc_time;
  last_clock_read = clock_time;
  return cpu_percent;
}
