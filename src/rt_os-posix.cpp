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

#include <iostream>

#include "rt.hpp"

#include <errno.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "debug.hpp"

thread_local bool realtime_key = false;

int RT::OS::initiate()
{
  /*
   * I want users to be very much aware that they aren't running in realtime.
   */
  std::cout << "***WARNING*** You are using the POSIX compatibility layer, "
               "RTXI is NOT running in realtime!!!\n";
  int retval = mlockall(MCL_CURRENT | MCL_FUTURE);
  if (retval != 0) {
    std::cout << "RT::OS(POSIX)::initiate : failed to lock memory.\n";
  }
  realtime_key = true;
  return retval;
}

void RT::OS::shutdown()
{
  realtime_key = false;
  munlockall();
}

void rt_thread_wrapper(void* (*rt_loop)(void*), void* args){
  RT::OS::initiate();
  rt_loop(args);
  RT::OS::shutdown();
}

int RT::OS::createTask(RT::OS::Task *task,
                       void* (*entry)(void*),
                       void* arg,
                       int prio)
{
  // Should not be creating real-time tasks from another real-time task
  if (RT::OS::isRealtime()) {
    ERROR_MSG("RT::OS::createTask : Task cannot be created from rt context");
    return -1;
  }
  if (task->rt_thread->joinable()){
    ERROR_MSG("RT::OS::createTask : RT Task is already initialized");
    return -1;
  }
  auto thread_obj = std::make_shared<std::thread>(rt_thread_wrapper,
                                                  entry,
                                                  arg);
  task->rt_thread = std::move(thread_obj);
  return 0;
}


void RT::OS::deleteTask(RT::OS::Task *task)
{
  // Should not be deleting real-time tasks from another real-time task
  if (RT::OS::isRealtime()) {
    ERROR_MSG("RT::OS::createTask : Task cannot be deleted from rt context");
    return;
  }
  task->task_finished = true;
  if (task->rt_thread->joinable()){
    task->rt_thread->join();
  }
}

bool RT::OS::isRealtime()
{
  return realtime_key;
}

int64_t RT::OS::getTime()
{
  timespec tp = { };

  clock_gettime(CLOCK_MONOTONIC, &tp);

  return RT::OS::SECONDS_TO_NANOSECONDS * tp.tv_sec + tp.tv_nsec;
}

int RT::OS::setPeriod(RT::OS::Task *task, int64_t period)
{
  task->period = period;
  return 0;
}

void RT::OS::sleepTimestep(RT::OS::Task *task)
{
  int64_t sleep_time = task->next_t;
  task->next_t += task->period;

  const struct timespec ts = {
      sleep_time / 1000000000,
      sleep_time % 1000000000
  };

  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, nullptr);
}

timespec last_clock_read;
timespec last_proc_time;

double RT::OS::getCpuUsage()
{
  // Should not attempt this in the real-time thread
  if (RT::OS::isRealtime()) {
    return 0.0;
  }

  ;
  double cpu_percent = 0.0;
  int64_t cpu_time_elapsed = 0;
  int64_t proc_time_elapsed = 0;

  timespec clock_time = { };
  timespec proc_time = { };
  // rusage resource_usage;

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &proc_time);
  clock_gettime(CLOCK_REALTIME, &clock_time);
  // getrusage(RUSAGE_SELF, &resource_usage);

  cpu_time_elapsed = RT::OS::SECONDS_TO_NANOSECONDS * (clock_time.tv_sec - last_clock_read.tv_sec)
      + (clock_time.tv_nsec - last_clock_read.tv_nsec);
  if (cpu_time_elapsed <= 0) {
    return 0.0;
  }
  proc_time_elapsed = RT::OS::SECONDS_TO_NANOSECONDS * (proc_time.tv_sec - last_proc_time.tv_sec)
      + (proc_time.tv_nsec - last_proc_time.tv_nsec);
  cpu_percent = 100.0 * (static_cast<double>(proc_time_elapsed)) / static_cast<double>(cpu_time_elapsed);

  last_proc_time = proc_time;
  last_clock_read = clock_time;
  return cpu_percent;
}