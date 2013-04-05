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
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <rt.h>
#include <rtai_lxrt.h>
#include <rtai_usi.h>
#include <semaphore.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>

/**************************************************************************************
 * Sometime between RTAI 3.1, which is used by Debian stable, and RTAI 3.3, which is  *
 *   likely used by those building RTAI from source, the interrupt clearing/restoring *
 *   functions had their names changed... so below is a bit of magic to work around   *
 *   that problem.                                                                    *
 **************************************************************************************/

#ifndef rtai_save_flags_and_cli
#define rtai_save_flags_and_cli(flags) \
    do { flags = hard_save_flags_and_cli(); } while(0)
#endif

#ifndef rtai_restore_flags
#define rtai_restore_flags(flags) \
    hard_restore_flags(flags)
#endif

typedef struct {
    long long period;
    long long next_t;
    long long wakeup;
    RT_TASK *task;
    pthread_t thread;
    unsigned long eflags;
} rtai3_task_t;

static bool init_rt = false;
static pthread_key_t is_rt_key;

static int rtai_lxrt_loaded(void);

int RT::OS::initiate(void) {
    if(rtai_lxrt_loaded()) {
        ERROR_MSG("RTOS:RTAI::initiate : rtai_lxrt module not loaded.\n");
        return -ENOENT;
    }

    rt_allow_nonroot_hrt();
    rt_set_oneshot_mode();
    start_rt_timer(0);

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
        stop_rt_timer();
        ERROR_MSG("RTOS:RTAI::initiate : failed to lock memory.\n");
        return -EPERM;
    }

    pthread_key_create(&is_rt_key,0);
    init_rt = true;

    return 0;
}

void RT::OS::shutdown(void) {
    pthread_key_delete(is_rt_key);
    stop_rt_timer();
}

struct rtai3_bounce_info_t {
    void *(*entry)(void *);
    rtai3_task_t *t;
    void *arg;
    int prio;
    sem_t sem;
};

static void *bounce(void *bounce_info) {
    rtai3_bounce_info_t *info = reinterpret_cast<rtai3_bounce_info_t *>(bounce_info);

    rtai3_task_t *t = info->t;
    void *(*entry)(void *) = info->entry;
    void *arg = info->arg;

    if(!(t->task = rt_task_init_schmod(reinterpret_cast<unsigned long>(t),1,2000,0,SCHED_FIFO,info->prio))) {
        ERROR_MSG("RT::OS::createTask : failed to create task\n");
        return reinterpret_cast<void *>(-EPERM);
    }

    t->period = -1;
    t->next_t = -1;
    t->wakeup = -1;
    t->thread = pthread_self();

    pthread_setspecific(is_rt_key,reinterpret_cast<const void *>(t));

    rt_make_hard_real_time();
    rtai_save_flags_and_cli(t->eflags);

    sem_post(&info->sem);

    void *retval = entry(arg);

    rtai_restore_flags(t->eflags);
    rt_task_delete(t->task);

    return retval;
}

int RT::OS::createTask(RT::OS::Task *task,void *(*entry)(void *),void *arg,int prio) {
    int retval = 0;
    rtai3_task_t *t = new rtai3_task_t;
    *task = t;

    rtai3_bounce_info_t info = {
        entry,
        t,
        arg,
        prio,
    };
    sem_init(&info.sem,0,0);

    retval = pthread_create(&t->thread,NULL,&::bounce,&info);
    if(!retval)
        sem_wait(&info.sem);
    else
        ERROR_MSG("RT::OS::createTask : pthread_create failed\n");

    sem_destroy(&info.sem);
    return retval;
}

void RT::OS::deleteTask(RT::OS::Task task) {
    rtai3_task_t *t = reinterpret_cast<rtai3_task_t *>(task);
    pthread_join(t->thread,0);
}

bool RT::OS::isRealtime(void) {
    if(init_rt && pthread_getspecific(is_rt_key))
        return true;
    return false;
}

long long RT::OS::getTime(void) {
    return rt_get_time_ns();
}

int RT::OS::setPeriod(RT::OS::Task task,long long period) {
    rtai3_task_t *t = reinterpret_cast<rtai3_task_t *>(task);

    /*
     * Set how early the realtime thread should wakeup.
     *   Upper limit constraints:
     *     It doesn't make sense for this value to exceed the period.
     *     The earlier the realtime thread wakes up the less time it spends in a blocking sleep,
     *       which means less time for the non-realtime thread to work. Thus the machine could
     *       appear crashed if the realtime thread wakes up too early.
     *   Lower Limit constraints:
     *     If the early_wakeup time is too small then you again begin to see the affects of scheduling jitter.
     *
     *   Ideally the early_wakeup time is set to the worst-case timestep - the desired period.
     *     If that number is sufficently small to not starve the non-realtime component of the
     *     system then you should see sub-microsecond jitter in the period.
     */
    if(period/10 > 100000ll)
        /*
         * Early wake isn't to exceed 100us
         *
         *   This is a bit arbitrary but if your worst-case timestep exceeds
         *     100us you are having some problems...
         */
        t->wakeup = nano2count(100000ll);
    else
        t->wakeup = nano2count(period/10);

    t->period = nano2count(period);
    t->next_t = rt_get_time()+t->period;

    return 0;
}

void RT::OS::sleepTimestep(RT::OS::Task task) {
    rtai3_task_t *t = reinterpret_cast<rtai3_task_t *>(task);

    register RTIME sleep_time;

    /*
     * Sleep blocked until early_wakeup nanoseconds before the
     *   next period starts. The actual time is subject to jitter.
     */
    rtai_restore_flags(t->eflags);
    rt_sleep_until(t->next_t-t->wakeup);

    /*
     * Busy sleep until it is time for the next period, this
     *   time isn't subject to scheduling jitter.
     */
    rtai_save_flags_and_cli(t->eflags);
    sleep_time = t->next_t-rt_get_time();
    if(sleep_time > 0)
        rt_busy_sleep(count2nano(sleep_time));

    t->next_t += t->period;
}

#define CHUNK_SIZE 512

int rtai_lxrt_loaded(void) {
    char buffer[CHUNK_SIZE+1], *tmp;
    int fd, size;

    if((fd = open("/proc/modules",O_RDONLY)) < 0) {
        ERROR_MSG("rtai_lxrt_loaded : failed to open /proc/modules for reading.\n");
        return -ENOENT;
    }
                                                                                                                               
    memset(buffer,0,CHUNK_SIZE+1);
    do {
        size = read(fd,buffer,CHUNK_SIZE);
        buffer[size] = 0;
        if(!strncmp(buffer,"rtai_lxrt ",10))
            return 0;
        tmp = strchr(buffer,'\n');
        lseek(fd,(off_t)tmp-size-(off_t)buffer+1,SEEK_CUR);
    } while(size > 0);

    return -ENOENT;
}
