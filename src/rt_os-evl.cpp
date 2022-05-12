/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Weill Cornell Medical College

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

#include <evl/thread.h>
#include <evl/sched.h>
#include <string.h>
#include <errno.h>

#include <iostream>

#include "rt.hpp"
#include "debug.hpp"

int RT::OS::initiate()
{
  int retval = 0;
  if (retval = evl_init()) {
    ERROR_MSG("RT::OS(EVL)::initiate : evl_init() : %s", strerror(errno));
  }
  return retval;
}

void RT::OS::shutdown()
{
}

void rt_thread_wrapper(void* (*rt_loop)(void*), void* args){
  evl_attach_self("RTXI-RT-Thread");
  rt_loop(args);
  evl_detach_self();
}

int RT::OS::createTask(RT::OS::Task &task,
                       void* (*entry)(void*),
                       void* arg,
                       int prio)
{
  auto thread_obj = std::make_unique<std::thread>(rt_thread_wrapper, arg);
  task.task_id = thread_obj.get_id();
  task.rt_thread = std::move(thread_obj);

}

void RT::OS::deleteTask(RT::OS::Task task)
{
  posix_task_t* t = reinterpret_cast<posix_task_t*>(task);
  if (t == NULL)
    return;

  pthread_join(t->thread, 0);
  delete t;
}

bool RT::OS::isRealtime()
{
  return !evl_is_inband();
}

long long RT::OS::getTime()
{
  timespec tp;

  evl_read_clock(EVL_CLOCK_MONOTONIC, &tp);

  return 1000000000ll * tp.tv_sec + tp.tv_nsec;
}

int RT::OS::setPeriod(RT::OS::Task task, long long period)
{
  posix_task_t* t = reinterpret_cast<posix_task_t*>(task);

  t->period = period;
  t->next_t = getTime() + period;

  return 0;
}

void RT::OS::sleepTimestep(RT::OS::Task task)
{
  posix_task_t* t = reinterpret_cast<posix_task_t*>(task);
  if (t == NULL)
    return;

  long long sleep_time = t->next_t;
  t->next_t += t->period;

  const struct timespec ts = {
      sleep_time / 1000000000ll,
      sleep_time % 1000000000ll,
  };

  evl_sleep_until(EVL_CLOCK_MONOTONIC, &ts);
}

timespec last_clock_read;
timespec last_proc_time;

double RT::OS::getCpuUsage()
{
  // Should not attempt this in the real-time thread
  if (RT::OS::isRealtime()) {
    ERROR_MSG(
        "RT::OS::getCpuUsage : This function should only be run in user space. "
        "Aborting.");
    return 0.0;
  }

  double cpu_percent;
  long cpu_time_elapsed;
  long proc_time_elapsed;

  timespec clock_time;
  timespec proc_time;
  // rusage resource_usage;

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &proc_time);
  clock_gettime(CLOCK_REALTIME, &clock_time);
  // getrusage(RUSAGE_SELF, &resource_usage);

  cpu_time_elapsed = 1e9 * (clock_time.tv_sec - last_clock_read.tv_sec)
      + (clock_time.tv_nsec - last_clock_read.tv_nsec);
  if (cpu_time_elapsed <= 0)
    return 0.0;
  proc_time_elapsed = 1e9 * (proc_time.tv_sec - last_proc_time.tv_sec)
      + (proc_time.tv_nsec - last_proc_time.tv_nsec);
  cpu_percent = 100.0 * (proc_time_elapsed) / cpu_time_elapsed;

  last_proc_time = proc_time;
  last_clock_read = clock_time;
  return cpu_percent;
}