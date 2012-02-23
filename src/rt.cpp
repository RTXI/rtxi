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
#include <event.h>
#include <mutex.h>
#include <rt.h>

namespace {

    class SetThreadActive : public RT::Event {

    public:

        SetThreadActive(RT::Thread *,bool);
        ~SetThreadActive(void);

        int callback(void);

    private:

        RT::Thread *thread;
        bool active;

    }; // class SetThreadActive

    class SetDeviceActive : public RT::Event {

    public:

        SetDeviceActive(RT::Device *,bool);
        ~SetDeviceActive(void);

        int callback(void);

    private:

        RT::Device *device;
        bool active;

    }; // class SetDeviceActive

}; // namespace

SetThreadActive::SetThreadActive(RT::Thread *t,bool a)
    : thread(t), active(a) {}

SetThreadActive::~SetThreadActive(void) {}

int SetThreadActive::callback(void) {
    thread->setActive(active);
    return 0;
}

SetDeviceActive::SetDeviceActive(RT::Device *d,bool a)
    : device(d), active(a) {}

SetDeviceActive::~SetDeviceActive(void) {}

int SetDeviceActive::callback(void) {
    device->setActive(active);
    return 0;
}

RT::System::SetPeriodEvent::SetPeriodEvent(long long p)
    : period(p) {}

RT::System::SetPeriodEvent::~SetPeriodEvent(void) {}

int RT::System::SetPeriodEvent::callback(void) {
    int retval;
    RT::System *sys = RT::System::getInstance();

    if(!(retval = RT::OS::setPeriod(sys->task,period))) {
        sys->period = period;

        ::Event::Object event(::Event::RT_PERIOD_EVENT);
        event.setParam("period",&period);
        ::Event::Manager::getInstance()->postEventRT(&event);
    }

    return retval;
}

RT::Event::Event(void) {
    sem_init(&signal,0,0);
}

RT::Event::~Event(void) {
    sem_destroy(&signal);
}

void RT::Event::execute(void) {
    retval = callback();
    sem_post(&signal);
}

void RT::Event::wait(void) {
    sem_wait(&signal);
}

RT::Device::Device(void)
    : active(false) {
    RT::System::getInstance()->insertDevice(this);
}

RT::Device::~Device(void) {
    RT::System::getInstance()->removeDevice(this);
}

void RT::Device::setActive(bool state) {
    if(RT::OS::isRealtime())
        active = state;
    else {
        SetDeviceActive event(this,state);
        RT::System::getInstance()->postEvent(&event);
    }
}

RT::Thread::Thread(Priority p)
    : active(false), priority(p) {
    RT::System::getInstance()->insertThread(this);
}

RT::Thread::~Thread(void) {
    RT::System::getInstance()->removeThread(this);
}

void RT::Thread::setActive(bool state) {
    if(RT::OS::isRealtime())
        active = state;
    else {
        SetThreadActive event(this,state);
        RT::System::getInstance()->postEvent(&event);
    }
}

RT::System::System(void)
    : finished(false), eventFifo(100*sizeof(RT::Event *)) {
    period = 1000000; // 1 kHz

    if(RT::OS::initiate()) {
        ERROR_MSG("RT::System::System : failed to initialize the realtime system\n");
        return;
    }

    if(RT::OS::createTask(&task,&System::bounce,this)) {
        ERROR_MSG("RT::System::System : failed to create realtime thread\n");
        return;
    }
}

RT::System::~System(void) {
    finished = true;
    RT::OS::deleteTask(task);
    RT::OS::shutdown();
}

int RT::System::setPeriod(long long period) {
    ::Event::Object event_pre(::Event::RT_PREPERIOD_EVENT);
    event_pre.setParam("period",&period);
    ::Event::Manager::getInstance()->postEvent(&event_pre);

    SetPeriodEvent event(period);
    int retval = postEvent(&event);

    ::Event::Object event_post(::Event::RT_POSTPERIOD_EVENT);
    event_post.setParam("period",&period);
    ::Event::Manager::getInstance()->postEvent(&event_post);

    return retval;
}

void RT::System::foreachDevice(void (*callback)(RT::Device *,void *),void *param) {
    Mutex::Locker lock(&deviceMutex);
    for(List<Device>::iterator i = deviceList.begin();i != deviceList.end();++i)
        callback(&*i,param);
}

void RT::System::foreachThread(void (*callback)(RT::Thread *,void *),void *param) {
    Mutex::Locker lock(&threadMutex);
    for(List<Thread>::iterator i = threadList.begin();i != threadList.end();++i)
        callback(&*i,param);
}

int RT::System::postEvent(RT::Event *event,bool blocking) {
    eventFifo.write(&event,sizeof(RT::Event *));

    if(blocking) {
        event->wait();
        return event->retval;
    }
    return 0;
}

void RT::System::insertDevice(RT::Device *device) {
    if(!device) {
        ERROR_MSG("RT::System::insertDevice : invalid device\n");
        return;
    }

    Mutex::Locker lock(&deviceMutex);

    ::Event::Object event(::Event::RT_DEVICE_INSERT_EVENT);
    event.setParam("device",device);
    ::Event::Manager::getInstance()->postEvent(&event);

    deviceList.insert(deviceList.end(),*device);
}

void RT::System::removeDevice(RT::Device *device) {
    if(!device) {
        ERROR_MSG("RT::System::removeDevice : invalid device\n");
        return;
    }

    Mutex::Locker lock(&deviceMutex);

    ::Event::Object event(::Event::RT_DEVICE_REMOVE_EVENT);
    event.setParam("device",device);
    ::Event::Manager::getInstance()->postEvent(&event);

    deviceList.remove(*device);
}

void RT::System::insertThread(RT::Thread *thread) {
    if(!thread) {
        ERROR_MSG("RT::System::insertThread : invalid thread\n");
        return;
    }

    Mutex::Locker lock(&threadMutex);

    /*******************************************************************************
     * Traverse the list of threads and find the first thread with lower priority. *
     *******************************************************************************/

    List<Thread>::iterator i = threadList.begin();
    for(;i != threadList.end() && i->getPriority() >= thread->getPriority();++i);

    ::Event::Object event(::Event::RT_THREAD_INSERT_EVENT);
    event.setParam("thread",thread);
    ::Event::Manager::getInstance()->postEvent(&event);

    threadList.insert(i,*thread);
}

void RT::System::removeThread(RT::Thread *thread) {
    if(!thread) {
        ERROR_MSG("RT::System::removeThread : invalid thread\n");
        return;
    }

    Mutex::Locker lock(&threadMutex);

    ::Event::Object event(::Event::RT_THREAD_REMOVE_EVENT);
    event.setParam("thread",thread);
    ::Event::Manager::getInstance()->postEvent(&event);

    threadList.remove(*thread);
}

void *RT::System::bounce(void *param) {
    RT::System *that = reinterpret_cast<RT::System *>(param);
    if(that) that->execute();
    return 0;
}

void RT::System::execute(void) {

    Event *event = 0;
    List<Device>::iterator iDevice;
    List<Thread>::iterator iThread;
    List<Device>::iterator deviceListBegin = deviceList.begin();
    List<Device>::iterator deviceListEnd   = deviceList.end();
    List<Thread>::iterator threadListBegin = threadList.begin();
    List<Thread>::iterator threadListEnd   = threadList.end();

    if(RT::OS::setPeriod(task,period)) {
        ERROR_MSG("RT::System::execute : failed to set the initial period of the realtime thread\n");
        return;
    }

    while(!finished) {
        RT::OS::sleepTimestep(task);

        for(iDevice = deviceListBegin;iDevice != deviceListEnd;++iDevice)
            if(iDevice->getActive()) iDevice->read();

        for(iThread = threadListBegin;iThread != threadListEnd;++iThread)
            if(iThread->getActive()) iThread->execute();

        for(iDevice = deviceListBegin;iDevice != deviceListEnd;++iDevice)
            if(iDevice->getActive()) iDevice->write();

        if(eventFifo.read(&event,sizeof(RT::Event *),false)) {
            do {
                event->execute();
            } while(eventFifo.read(&event,sizeof(RT::Event *),false));

            event = 0;
            deviceListBegin = deviceList.begin();
            threadListBegin = threadList.begin();
        }
    }
}

static Mutex mutex;
RT::System *RT::System::instance = 0;

RT::System *RT::System::getInstance(void) {
    if(instance)
        return instance;

    /*************************************************************************
     * Seems like alot of hoops to jump through, but static allocation isn't *
     *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
     *************************************************************************/

    Mutex::Locker lock(&::mutex);
    if(!instance) {
        static System system;
        instance = &system;
    }

    return instance;
}
