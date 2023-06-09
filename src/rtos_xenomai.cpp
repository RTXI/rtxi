/*
                        The Real-Time eXperiment Interface (RTXI)
                        Copyright (C) 2011 Georgia Institute of Technology,
   University of Utah, Weill Cornell Medical College

                        This program is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

                        This program is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
                        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
   the GNU General Public License for more details.

                        You should have received a copy of the GNU General
   Public License along with this program.  If not, see
   <http://www.gnu.org/licenses/>.

*/

#include <iostream>

#include "rtos.hpp"

#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "fifo.hpp"

#if CONFIG_XENO_VERSION_MAJOR >= 3
#  include <alchemy/task.h>
#  include <alchemy/timer.h>
#else
#  include <native/task.h>
#  include <native/timer.h>
#endif

thread_local bool realtime_key = false;
thread_local int64_t* RT_PERIOD = nullptr;

int RT::OS::initiate(RT::OS::Task* task)
{
  // Kernel limitations on memory lock are no longer present, however
  // still a useful (for performance) method for preventing paging
  // of active RTXI memory
  struct rlimit rlim = {RLIM_INFINITY, RLIM_INFINITY};
  setrlimit(RLIMIT_MEMLOCK, &rlim);

  if (mlockall(MCL_CURRENT | MCL_FUTURE)) {
    ERROR_MSG("RTOS:Xenomai::initiate : failed to lock memory.\n");
    return -EPERM;
  }

  realtime_key = true;
  task->period = RT::OS::DEFAULT_PERIOD;
  RT_PERIOD = &(task->period);
  return 0;
}

void RT::OS::shutdown(RT::OS::Task* task)
{
  realtime_key = false;
  task->task_finished = true;
  RT_PERIOD = nullptr;
}

int RT::OS::createTask(RT::OS::Task* task, void* (*func)(void*), void* arg)
{
  int retval = 0;
  RT_TASK xenomai_task;

  // Tell Xenomai to report mode issues
  rt_task_set_mode(0, T_WARNSW, NULL);

  if ((retval = rt_task_create(&xenomai_task, "Real-Time Task", 0, 50, 0))) {
    ERROR_MSG("RT::OS::createTask : failed to create task\n");
    return retval;
  }

  auto wrapper = [](RT::OS::Task* tsk, void (*fn)(void*), void* args)
  {
    auto resval = RT::OS::initiate(tsk);
    if (resval != 0) {
      ERROR_MSG("RT::OS::createTask : RT::OS::initiate() : {}",
                strerror(errno));
      // In the event that we fail to initiate real-time environment let's just
      // quit
      return;
    }
    fn(args);
    RT::OS::shutdown(tsk);
  };

  if ((retval = rt_task_start(
           &xenomai_task, reinterpret_cast<void (*)(void*)>(wrapper), arg)))
  {
    ERROR_MSG("RT::OS::createTask : failed to start task\n");
    return retval;
  }

  task->thread_id = std::any(xenomai_task);
  return 0;
}

void RT::OS::deleteTask(RT::OS::Task task)
{
  auto xenomai_task = std::any_cast<RT_TASK>(task);
  rt_task_delete(task);
}

bool RT::OS::isRealtime(void)
{
  return (realtime_key && rt_task_self() != nullptr);
}

int64_t RT::OS::getTime(void)
{
#if CONFIG_XENO_VERSION_MAJOR >= 3
  return rt_timer_read();
#else
  return rt_timer_tsc2ns(rt_timer_tsc());
#endif
}

int RT::OS::setPeriod(RT::OS::Task* task, int64_t period)
{
  // Set wake up limits
  if (period / 10 > 50000ll)
    task->next_t = rt_timer_ns2ticks(50000ll);
  else
    task->next_t = rt_timer_ns2ticks(period / 10);

  // Setup timing bounds for oneshot operation
  task->period = rt_timer_ns2ticks(period);
  task->next_t = rt_timer_read() + t->period;

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
  auto xenomai_task = std::any_cast<RT_TASK>(task->thread_d);
  rt_task_sleep_until(task->next_t);

  // Busy sleep until ready for the next cycle
  rt_timer_spin(rt_timer_ticks2ns(t->next_t - rt_timer_read()));

  // Update next interrupt time
  t->next_t += t->period;
}

timespec last_clock_read;
timespec last_proc_time;
ticks_t last_rt_clock;

double RT::OS::getCpuUsage()
{
  // Should not attempt this in the real-time thread
  if (RT::OS::isRealtime()) {
    ERROR_MSG(
        "RT::OS::getCpuUsage : This function should only be run in user space. "
        "Aborting.");
    return 0.0;
  }

  timespec clock_time;
  timespec proc_time;
  double cpu_rt_percent;
  double cpu_user_percent;
  long rt_time_elapsed;
  long proc_time_elapsed;
  long cpu_time_elapsed;
  RT_TASK_INFO task_info;

  // First get task information
  xenomai_task_t* task =
      reinterpret_cast<xenomai_task_t*>(RT::System::getInstance()->getTask());
  rt_task_inquire(&(task->task), &task_info);

  // get ticks from normal system
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &proc_time);
  clock_gettime(CLOCK_REALTIME, &clock_time);

  // calculate cpu usage in user space
  cpu_time_elapsed = 1e9 * (clock_time.tv_sec - last_clock_read.tv_sec)
      + (clock_time.tv_nsec - last_clock_read.tv_nsec);
  if (cpu_time_elapsed <= 0)
    return 0.0;
  proc_time_elapsed = 1e9 * (proc_time.tv_sec - last_proc_time.tv_sec)
      + (proc_time.tv_nsec - last_proc_time.tv_nsec);
  cpu_user_percent = 100.0 * (proc_time_elapsed) / cpu_time_elapsed;

  // calcualte cpu usage by real-time therad
  rt_time_elapsed = task_info.stat.xtime - last_rt_clock;
  cpu_rt_percent = 100.0 * rt_time_elapsed / cpu_time_elapsed;

  // keep track of last clock reads
  last_proc_time = proc_time;
  last_clock_read = clock_time;
  last_rt_clock = task_info.stat.xtime;

  return cpu_rt_percent + cpu_user_percent;
}
