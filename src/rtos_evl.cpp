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

#include <evl/evl.h>

#include <iostream>

#include "rtos.hpp"

#include <errno.h>
#include <string.h>
#include <unistd.h>



int RT::OS::initiate()
{
  int retval = evl_init();
  if (retval != 0) {
    ERROR_MSG("RT::OS(EVL)::initiate : evl_init() : {}", strerror(errno));
    return retval;
  }
  retval = evl_attach_self("RTXI-RT-Thread:%d", getpid());  // NOLINT
  if (retval != 0) {
    ERROR_MSG("RT::OS(EVL)::initiate : evl_attach_self() : {}",
              strerror(errno));
  }
  return retval;
}

void RT::OS::shutdown()
{
  int retval = evl_detach_self();
  if (retval != 0) {
    ERROR_MSG("Unable to detach thread from evl core!");
    ERROR_MSG("RT::OS(EVL)::shutdown : evl_detach_self() : {}",
              strerror(errno));
  }
}

void RT::OS::deleteTask(std::unique_ptr<RT::OS::Task> & task)
{
  // Should not be deleting real-time tasks from another real-time task
  if (RT::OS::isRealtime()) {
    ERROR_MSG("RT::OS::createTask : Task cannot be deleted from rt context");
    return;
  }
  //task->task_finished = true;
  if (task->rt_thread.joinable()) {
    task->rt_thread.join();
  }
}

bool RT::OS::isRealtime()
{
  return !evl_is_inband();
}

int64_t RT::OS::getTime()
{
  timespec tp = {};

  evl_read_clock(EVL_CLOCK_MONOTONIC, &tp);

  return RT::OS::SECONDS_TO_NANOSECONDS * tp.tv_sec + tp.tv_nsec;
}

int RT::OS::setPeriod(std::unique_ptr<RT::OS::Task> & task, int64_t period)
{
  task->period = period;
  return 0;
}

void RT::OS::sleepTimestep(const std::unique_ptr<RT::OS::Task> & task)
{
  if (task->next_t < RT::OS::DEFAULT_PERIOD)
  {
    task->next_t = RT::OS::getTime() + task->period;
  }
  int64_t wakeup_time = task->next_t;
  task->next_t += task->period;

  const struct timespec ts = {wakeup_time / RT::OS::SECONDS_TO_NANOSECONDS, 
                              wakeup_time % RT::OS::SECONDS_TO_NANOSECONDS};

  evl_sleep_until(EVL_CLOCK_MONOTONIC, &ts);
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
