/*
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

typedef struct {
    long long period;
    RT_TASK task;
} xenomai_task_t;

static bool init_rt = false;
static pthread_key_t is_rt_key;

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

    if(mlockall(MCL_CURRENT | MCL_FUTURE)) {
        ERROR_MSG("RTOS:RTAI::initiate : failed to lock memory.\n");
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

    // Invert priority, default prio=0 but max priority for xenomai task is 99
    if ((prio >=0) && (prio <=99))
        priority -= prio;

    if((retval = rt_task_create(&t->task,"RTXI RT Thread",0,priority,T_FPU|T_JOINABLE))) {
        ERROR_MSG("RT::OS::createTask : failed to create task\n");
        return retval;
    }

    t->period = -1;

    *task = t;
    pthread_setspecific(is_rt_key,reinterpret_cast<const void *>(t));

    if((retval = rt_task_start(&t->task,reinterpret_cast<void(*)(void *)>(entry),arg))) {
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
    if(init_rt && pthread_getspecific(is_rt_key))
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
