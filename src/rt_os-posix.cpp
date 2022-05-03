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

#include <errno.h>
#include <list>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/resource.h>

#include <fstream>
#include <string>

#include "debug.h"
#include "rt.h"

typedef struct
{
    long long period;
    long long next_t;
    pthread_t thread;
} posix_task_t;

static bool init_rt = false;
static pthread_key_t is_rt_key;

int RT::OS::initiate(void)
{
    /*
     * I want users to be very much aware that they aren't running in realtime.
     */
    std::cout << "***WARNING*** You are using the POSIX compatibility layer, RTXI is NOT running in realtime!!!\n";

    if (mlockall(MCL_CURRENT | MCL_FUTURE))
        {
            std::cout << "RT::OS(POSIX)::initiate : failed to lock memory.\n";

            /*
             * I don't think it is necessary to return an error in this case.
             *  Because unless you are root it will always error.
             */
            //return -EPERM;
        }

    pthread_key_create(&is_rt_key,0);
    init_rt = true;

    return 0;
}

void RT::OS::shutdown(void)
{
    pthread_key_delete(is_rt_key);
}

struct posix_bounce_info_t
{
    void *(*entry)(void *);
    posix_task_t *t;
    void *arg;
    sem_t sem;
};

static void *bounce(void *bounce_info)
{
    posix_bounce_info_t *info = reinterpret_cast<posix_bounce_info_t *>(bounce_info);

    posix_task_t *t = info->t;
    void *(*entry)(void *) = info->entry;
    void *arg = info->arg;

    t->period = -1;
    t->next_t = -1;
    t->thread = pthread_self();

    pthread_setspecific(is_rt_key,reinterpret_cast<const void *>(t));

    sem_post(&info->sem);
    return entry(arg);
}

int RT::OS::createTask(RT::OS::Task *task,void *(*entry)(void *),void *arg,int)
{
    int retval = 0;
    posix_task_t *t = new posix_task_t;
    *task = t;

    posix_bounce_info_t info =
    {
        entry,
        t,
        arg,
    };
    sem_init(&info.sem,0,0);

    retval = pthread_create(&t->thread,NULL,&::bounce,&info);
    if (!retval)
        sem_wait(&info.sem);
    else
        ERROR_MSG("RT::OS::createTask : pthread_create failed\n");

    sem_destroy(&info.sem);
    return retval;
}

void RT::OS::deleteTask(RT::OS::Task task)
{
    posix_task_t *t = reinterpret_cast<posix_task_t *>(task);
    if (t == NULL)
        return;

    pthread_join(t->thread,0);
    delete t;
}

bool RT::OS::isRealtime(void)
{
    if (init_rt && pthread_getspecific(is_rt_key))
        return true;
    return false;
}

long long RT::OS::getTime(void)
{
    struct timeval tv;

    gettimeofday(&tv,NULL);

    return 1000000000ll*tv.tv_sec+1000ll*tv.tv_usec;
}

int RT::OS::setPeriod(RT::OS::Task task,long long period)
{
    posix_task_t *t = reinterpret_cast<posix_task_t *>(task);

    t->period = period;
    t->next_t = getTime()+period;

    return 0;
}

void RT::OS::sleepTimestep(RT::OS::Task task)
{
    posix_task_t *t = reinterpret_cast<posix_task_t *>(task);
    if (t == NULL)
        return;

    long long sleep_time = t->next_t-getTime();
    t->next_t += t->period;

    struct timespec ts =
    {
        sleep_time / 1000000000ll,
        sleep_time % 1000000000ll,
    };

    while (nanosleep(&ts,&ts) < 0 && errno == EINTR);
}

timespec last_clock_read;
timespec last_proc_time;

double RT::OS::getCpuUsage()
{
    // Should not attempt this in the real-time thread
    if(RT::OS::isRealtime()){
        ERROR_MSG("RT::OS::getCpuUsage : This function should only be run in user space. Aborting.");
        return 0.0;
    }

    double cpu_percent;
    long cpu_time_elapsed;
    long proc_time_elapsed;

    timespec clock_time;
    timespec proc_time;
    //rusage resource_usage;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &proc_time);
    clock_gettime(CLOCK_REALTIME, &clock_time);
    //getrusage(RUSAGE_SELF, &resource_usage); 

    cpu_time_elapsed = 1e9*(clock_time.tv_sec - last_clock_read.tv_sec) + 
                           (clock_time.tv_nsec - last_clock_read.tv_nsec);
    if (cpu_time_elapsed <= 0) return 0.0;
    proc_time_elapsed = 1e9*(proc_time.tv_sec - last_proc_time.tv_sec) + 
                            (proc_time.tv_nsec - last_proc_time.tv_nsec);
    cpu_percent = 100.0*(proc_time_elapsed) / cpu_time_elapsed;

    last_proc_time = proc_time; 
    last_clock_read = clock_time;
    return cpu_percent;
}

