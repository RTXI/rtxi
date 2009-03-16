/*
 * Copyright (C) 2005 Boston University
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <algorithm>
#include <compiler.h>
#include <debug.h>
#include <event.h>
#include <rt.h>
#include <workspace.h>

namespace {

    class ParameterChangeEvent : public RT::Event {

    public:

        ParameterChangeEvent(Settings::Object::ID,size_t,double,double *);
        ~ParameterChangeEvent(void);

        int callback(void);

    private:

        Settings::Object::ID object;
        size_t index;
        double value;
        double *data;

    }; // class ParameterChangeEvent

}; // namespace

ParameterChangeEvent::ParameterChangeEvent(Settings::Object::ID id,size_t i,double v,double *d)
    : object(id), index(i), value(v), data(d) {}

ParameterChangeEvent::~ParameterChangeEvent(void) {}

int ParameterChangeEvent::callback(void) {

    if(*data == value)
        return 0;

    *data = value;

    ::Event::Object event(::Event::WORKSPACE_PARAMETER_CHANGE_EVENT);
    event.setParam("object",(void *)object);
    event.setParam("index",(void *)index);
    event.setParam("value",(void *)data);
    ::Event::Manager::postEventRT(&event);

    return 0;
}

Workspace::Instance::Instance(std::string name,Workspace::variable_t *d,size_t n)
    : IO::Block(name,d,n) {
    size_t count[3] = { 0, 0, 0 };
    for(size_t i=0;i<n;++i)
        switch(d[i].flags & (PARAMETER|STATE|EVENT)) {
          case PARAMETER:
              count[0]++;
              break;
          case STATE:
              count[1]++;
              break;
          case EVENT:
              count[2]++;
              break;
        }

    parameter = std::vector<var_t>(count[0]);
    state = std::vector<var_t>(count[1]);
    event = std::vector<var_t>(count[2]);

    size_t i[3] = { 0, 0, 0 };
    for(size_t j=0;j<n;++j) {
        switch(d[j].flags & (PARAMETER|STATE|EVENT)) {
          case PARAMETER:
              parameter[i[0]].name = d[j].name;
              parameter[i[0]].description = d[j].description;
              parameter[i[0]].data = new double;
              i[0]++;
              break;
          case STATE:
              state[i[1]].name = d[j].name;
              state[i[1]].description = d[j].description;
              state[i[1]].data = 0;
              i[1]++;
              break;
	case EVENT:
	     event[i[2]].name = d[j].name;
	     event[i[2]].description = d[j].description;
	     event[i[2]].data = 0;
	     i[2]++;
	     break;
        }
    }

    Workspace::Manager::insertWorkspace(this);
}

Workspace::Instance::~Instance(void) {
    Workspace::Manager::removeWorkspace(this);

    for(std::vector<var_t>::iterator i = parameter.begin(), end = parameter.end();i != end;++i)
        delete i->data;
}

size_t Workspace::Instance::getCount(IO::flags_t type) const {
    if(type & (INPUT|OUTPUT))
        return IO::Block::getCount(type);
    if(type & PARAMETER)
        return parameter.size();
    if(type & STATE)
        return state.size();
    if(type & EVENT)
        return event.size();

    return 0;
}

std::string Workspace::Instance::getName(IO::flags_t type,size_t n) const {
    if(type & (INPUT|OUTPUT))
        return IO::Block::getName(type,n);
    if(type & PARAMETER && n < parameter.size())
        return parameter[n].name;
    if(type & STATE && n < state.size())
        return state[n].name;
    if(type & EVENT && n < event.size())
        return event[n].name;

    return "";
}

std::string Workspace::Instance::getDescription(IO::flags_t type,size_t n) const {
    if(type & PARAMETER && n < parameter.size())
        return parameter[n].description;
    if(type & STATE && n < state.size())
        return state[n].description;
    if(type & EVENT && n < event.size())
        return event[n].description;

    return "";
}

double Workspace::Instance::getValue(IO::flags_t type,size_t n) const {
    if(type & INPUT)
        return input(n);
    if(type & OUTPUT)
        return output(n);
    if(type & PARAMETER && n < parameter.size() && parameter[n].data)
        return *parameter[n].data;
    if(type & STATE && n < state.size() && state[n].data)
        return *state[n].data;
    if(type & EVENT && n < event.size() && event[n].data)
        return *event[n].data;
    return 0.0;
}

void Workspace::Instance::setValue(size_t n,double value) {
    if(n >= parameter.size() || !parameter[n].data)
        return;

    if(RT::OS::isRealtime() && *parameter[n].data != value) {
        *parameter[n].data = value;

        ::Event::Object event(::Event::WORKSPACE_PARAMETER_CHANGE_EVENT);
        event.setParam("object",(void *)getID());
        event.setParam("index",(void *)n);
        event.setParam("value",(void *)parameter[n].data);
        ::Event::Manager::postEventRT(&event);
    } else {
        ParameterChangeEvent event(getID(),n,value,parameter[n].data);
        RT::System::postEvent(&event);
    }
}

double *Workspace::Instance::getData(IO::flags_t type,size_t n) {
    if(type & PARAMETER && n < parameter.size())
        return parameter[n].data;
    if(type & STATE && n < state.size())
        return state[n].data;
    if(type & EVENT && n < event.size())
        return event[n].data;
    return 0;
}

void Workspace::Instance::setData(IO::flags_t type,size_t n,double *data) {
    if(type & PARAMETER && n < parameter.size())
        parameter[n].data = data;
    if(type & STATE && n < state.size())
        state[n].data = data;
    if(type & EVENT && n < event.size())
        event[n].data = data;
}

void Workspace::Manager::foreachWorkspace(void (*callback)(Workspace::Instance *,void *),void *param) {
    if(unlikely(!instance))
        initialize();

    Mutex::Locker lock(&instance->mutex);
    for(std::list<Instance *>::iterator i = instance->instanceList.begin();i != instance->instanceList.end();++i)
        callback(*i,param);
}

void Workspace::Manager::insertWorkspace(Workspace::Instance *workspace) {
    if(unlikely(!instance))
        initialize();

    if(!workspace) {
        ERROR_MSG("Workspace::Manager::insertWorkspace : invalid workspace\n");
        return;
    }

    Mutex::Locker lock(&instance->mutex);

    if(std::find(instance->instanceList.begin(),instance->instanceList.end(),workspace) != instance->instanceList.end()) {
        ERROR_MSG("Workspace::Manager::insertWorkspace : workspace already present\n");
        return;
    }

    instance->instanceList.push_back(workspace);
}

void Workspace::Manager::removeWorkspace(Workspace::Instance *workspace) {
    if(unlikely(!instance))
        initialize();

    if(!workspace) {
        ERROR_MSG("Workspace::Manager::removeWorkspace : invalid workspace\n");
        return;
    }

    Mutex::Locker lock(&instance->mutex);
    instance->instanceList.remove(workspace);
}

static Mutex mutex;
Workspace::Manager *Workspace::Manager::instance = 0;

void Workspace::Manager::initialize(void) {
    if(instance)
        return;

    /*************************************************************************
     * Seems like alot of hoops to jump through, but static allocation isn't *
     *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
     *************************************************************************/

    Mutex::Locker lock(&::mutex);
    if(!instance) {
        static Manager manager;
        instance = &manager;
    }
}
