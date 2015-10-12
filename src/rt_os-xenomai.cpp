/*
	 The Real-Time eXperiment Interface (RTXI)
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

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

#include <debug.h>
#include <pthread.h>
#include <rt.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <native/task.h>
#include <native/timer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <execinfo.h>
#include <unistd.h>

#define DEBUG_RT

typedef struct {
	long long period;
	RT_TASK task;
} xenomai_task_t;

static bool init_rt = false;
static pthread_key_t is_rt_key;

#ifdef DEBUG_RT
static const char *sigdebug_reasons[] = {
	[SIGDEBUG_UNDEFINED] = "latency: received SIGXCPU for unknown reason",
	[SIGDEBUG_MIGRATE_SIGNAL] = "received signal",
	[SIGDEBUG_MIGRATE_SYSCALL] = "invoked syscall",
	[SIGDEBUG_MIGRATE_FAULT] = "triggered fault",
	[SIGDEBUG_MIGRATE_PRIOINV] = "affected by priority inversion",
	[SIGDEBUG_NOMLOCK] = "Xenomai: process memory not locked "
		"(missing mlockall?)",
	[SIGDEBUG_WATCHDOG] = "Xenomai: watchdog triggered "
		"(period too short?)",
};

void sigdebug_handler(int sig, siginfo_t *si, void *context)
{
	const char fmt[] = "Mode switch (reason: %s), aborting. Backtrace:\n";
	unsigned int reason = si->si_value.sival_int;
	static char buffer[256];
	static void *bt[200];
	unsigned int n;

	if (reason > SIGDEBUG_WATCHDOG)
		reason = SIGDEBUG_UNDEFINED;

	switch(reason) {
		case SIGDEBUG_UNDEFINED:
			n = snprintf(buffer, sizeof(buffer),
					"%s\n", sigdebug_reasons[reason]);
			write(STDERR_FILENO, buffer, n);
		case SIGDEBUG_MIGRATE_SIGNAL:
			n = snprintf(buffer, sizeof(buffer),
					"%s\n", sigdebug_reasons[reason]);
			write(STDERR_FILENO, buffer, n);
		case SIGDEBUG_WATCHDOG:
			/* These errors are lethal, something went really wrong. */
			n = snprintf(buffer, sizeof(buffer),
					"%s\n", sigdebug_reasons[reason]);
			write(STDERR_FILENO, buffer, n);
			exit(EXIT_FAILURE);
	}

	/* Retrieve the current backtrace, and decode it to stdout. */
	n = snprintf(buffer, sizeof(buffer), fmt, sigdebug_reasons[reason]);
	n = write(STDERR_FILENO, buffer, n);
	n = backtrace(bt, sizeof(bt)/sizeof(bt[0]));
	backtrace_symbols_fd(bt, n, STDERR_FILENO);

	signal(sig, SIG_DFL);
	kill(getpid(), sig);
}
#endif

int RT::OS::initiate(void) {
	rt_timer_set_mode(TM_ONESHOT);

	/**************************************************************************************
	 * On some systems like Fedora Core 4 the memory footprint for rtxi exceeds the mlock *
	 *   limit allowed by the linux kernel. As root this limit can be removed with a call *
	 *   to setrlimit. However, as a non-root user the call to setrlimit will fail. So if *
	 *   you aren't root setrlimit will fail, but that is okay if your system is one that *
	 *   doesn't need the extra space.                                                    *
	 **************************************************************************************/

	struct rlimit rlim = { RLIM_INFINITY, RLIM_INFINITY };
	setrlimit(RLIMIT_MEMLOCK,&rlim);

	if (mlockall(MCL_CURRENT | MCL_FUTURE)) {
		ERROR_MSG("RTOS:Xenomai::initiate : failed to lock memory.\n");
		return -EPERM;
	}

	pthread_key_create(&is_rt_key,0);
	init_rt = true;

	return 0;
}

void RT::OS::shutdown(void) {
	pthread_key_delete(is_rt_key);
}

int RT::OS::createTask(RT::OS::Task *task,void *(*entry)(void *),void *arg,int prio) {
	int retval = 0;
	xenomai_task_t *t = new xenomai_task_t;
	int priority = 99;

#ifdef DEBUG_RT
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = sigdebug_handler;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGDEBUG, &sa, NULL);

	// Tell Xenomai to report mode issues
	rt_task_set_mode(0, T_WARNSW, NULL);
#endif

	// Invert priority, default prio=0 but max priority for xenomai task is 99
	if ((prio >=0) && (prio <=99))
		priority -= prio;

	if ((retval = rt_task_create(&t->task,"RTXI RT Thread",0,priority,T_FPU|T_JOINABLE))) {
		ERROR_MSG("RT::OS::createTask : failed to create task\n");
		return retval;
	}

	t->period = -1;

	*task = t;
	pthread_setspecific(is_rt_key,reinterpret_cast<const void *>(t));

	if ((retval = rt_task_start(&t->task,reinterpret_cast<void(*)(void *)>(entry),arg))) {
		ERROR_MSG("RT::OS::createTask : failed to start task\n");
		return retval;
	}

	return 0;
}

void RT::OS::deleteTask(RT::OS::Task task) {
	xenomai_task_t *t = reinterpret_cast<xenomai_task_t *>(task);
	rt_task_delete(&t->task);
}

bool RT::OS::isRealtime(void) {
	if (init_rt && pthread_getspecific(is_rt_key))
		return true;
	return false;
}

long long RT::OS::getTime(void) {
	return rt_timer_tsc2ns(rt_timer_tsc());
}

int RT::OS::setPeriod(RT::OS::Task task,long long period) {
	xenomai_task_t *t = reinterpret_cast<xenomai_task_t *>(task);
	t->period = period;
	return rt_task_set_periodic(&t->task,TM_NOW,period);
}

void RT::OS::sleepTimestep(RT::OS::Task task) {
	rt_task_wait_period(0);
}
