/*
                        The Real-Time eXperiment Interface (RTXI)
                        Copyright (C) 2011 Georgia Institute of Technology,
   University of Utah, Will Cornell Medical College

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

#include <debug.h>
#include <execinfo.h>
#include <getopt.h>
#include <pthread.h>
#include <rt.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <studio.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>

#if CONFIG_XENO_VERSION_MAJOR >= 3
#  include <alchemy/task.h>
#  include <alchemy/timer.h>
#else
#  include <native/task.h>
#  include <native/timer.h>
#endif

typedef struct
{
  long long period;
  long long next_t;
  long long wakeup_t;
  RT_TASK task;
} xenomai_task_t;

static bool init_rt = false;
static pthread_key_t is_rt_key;
static char* RT_TASK_NAME = "RTXI RT Thread";

static const char* sigdebug_reasons[] = {
    [SIGDEBUG_UNDEFINED] = "latency: received SIGXCPU for unknown reason",
    [SIGDEBUG_MIGRATE_SIGNAL] = "received signal",
    [SIGDEBUG_MIGRATE_SYSCALL] = "invoked syscall",
    [SIGDEBUG_MIGRATE_FAULT] = "triggered fault",
    [SIGDEBUG_MIGRATE_PRIOINV] = "affected by priority inversion",
    [SIGDEBUG_NOMLOCK] =
        "Xenomai: process memory not locked "
        "(missing mlockall?)",
    [SIGDEBUG_WATCHDOG] =
        "Xenomai: watchdog triggered "
        "(period too short?)",
};

void sigdebug_handler(int sig, siginfo_t* si, void*)
{
  const char fmt[] = "Mode switch (reason: %s), aborting. Backtrace:\n";
  unsigned int reason = si->si_value.sival_int;
  static char buffer[256];
  static void* bt[200];
  unsigned int n;

  if (reason > SIGDEBUG_WATCHDOG)
    reason = SIGDEBUG_UNDEFINED;

  switch (reason) {
    case SIGDEBUG_UNDEFINED:
      n = snprintf(buffer, sizeof(buffer), "%s\n", sigdebug_reasons[reason]);
      write(STDERR_FILENO, buffer, n);
      break;
    case SIGDEBUG_MIGRATE_SIGNAL:
      n = snprintf(buffer, sizeof(buffer), "%s\n", sigdebug_reasons[reason]);
      write(STDERR_FILENO, buffer, n);
      break;
    case SIGDEBUG_WATCHDOG:
      /* These errors are lethal, something went really wrong. */
      n = snprintf(buffer, sizeof(buffer), "%s\n", sigdebug_reasons[reason]);
      write(STDERR_FILENO, buffer, n);
      exit(EXIT_FAILURE);
  }

  /* Retrieve the current backtrace, and decode it to stdout. */
  n = snprintf(buffer, sizeof(buffer), fmt, sigdebug_reasons[reason]);
  n = write(STDERR_FILENO, buffer, n);
  n = backtrace(bt, sizeof(bt) / sizeof(bt[0]));
  backtrace_symbols_fd(bt, n, STDERR_FILENO);

  signal(sig, SIG_DFL);
  kill(getpid(), sig);
}

int RT::OS::initiate(void)
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

  pthread_key_create(&is_rt_key, 0);
  init_rt = true;

  return 0;
}

void RT::OS::shutdown(void)
{
  pthread_key_delete(is_rt_key);
}

int RT::OS::createTask(RT::OS::Task* task,
                       void* (*entry)(void*),
                       void* arg,
                       int prio)
{
  int retval = 0;
  xenomai_task_t* t = new xenomai_task_t;
  int priority = 99;

  // Assign signal handler
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = sigdebug_handler;
  sa.sa_flags = SA_SIGINFO;
  sigaction(SIGDEBUG, &sa, NULL);

  // Tell Xenomai to report mode issues
  rt_task_set_mode(0, T_WARNSW, NULL);

  // Invert priority, default prio=0 but max priority for xenomai task is 99
  if ((prio >= 0) && (prio <= 99))
    priority -= prio;

  if ((retval = rt_task_create(&t->task, RT_TASK_NAME, 0, priority, 0))) {
    ERROR_MSG("RT::OS::createTask : failed to create task\n");
    return retval;
  }

  t->period = -1;

  *task = t;
  pthread_setspecific(is_rt_key, reinterpret_cast<const void*>(t));

  if ((retval = rt_task_start(
           &t->task, reinterpret_cast<void (*)(void*)>(entry), arg)))
  {
    ERROR_MSG("RT::OS::createTask : failed to start task\n");
    return retval;
  }

  return 0;
}

void RT::OS::deleteTask(RT::OS::Task task)
{
  xenomai_task_t* t = reinterpret_cast<xenomai_task_t*>(task);
  rt_task_delete(&t->task);
}

bool RT::OS::isRealtime(void)
{
  if (init_rt && rt_task_self() != NULL)
    return true;
  return false;
}

long long RT::OS::getTime(void)
{
#if CONFIG_XENO_VERSION_MAJOR >= 3
  return rt_timer_read();
#else
  return rt_timer_tsc2ns(rt_timer_tsc());
#endif
}

int RT::OS::setPeriod(RT::OS::Task task, long long period)
{
  // Retrieve task struct
  xenomai_task_t* t = reinterpret_cast<xenomai_task_t*>(task);

  // Set wake up limits
  if (period / 10 > 50000ll)
    t->wakeup_t = rt_timer_ns2ticks(50000ll);
  else
    t->wakeup_t = rt_timer_ns2ticks(period / 10);

  // Setup timing bounds for oneshot operation
  t->period = rt_timer_ns2ticks(period);
  t->next_t = rt_timer_read() + t->period;

  return 0;
}

void RT::OS::sleepTimestep(RT::OS::Task task)
{
  xenomai_task_t* t = reinterpret_cast<xenomai_task_t*>(task);

  // Prevent significant early wake up from happening and drying the Linux
  // system
  rt_task_sleep_until(t->next_t - t->wakeup_t);

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

  // calculate cpu usage by real-time therad
  rt_time_elapsed = task_info.stat.xtime - last_rt_clock;
  cpu_rt_percent = 100.0 * rt_time_elapsed / cpu_time_elapsed;

  // keep track of last clock reads
  last_proc_time = proc_time;
  last_clock_read = clock_time;
  last_rt_clock = task_info.stat.xtime;

  return cpu_rt_percent + cpu_user_percent;
}
