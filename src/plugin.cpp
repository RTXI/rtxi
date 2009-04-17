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

#include <compiler.h>
#include <debug.h>
#include <dlfcn.h>
#include <event.h>
#include <plugin.h>
#include <qapplication.h>

Plugin::Object::Object(void)
    : magic_number(Plugin::Object::MAGIC_NUMBER), handle(0) {
    Plugin::Manager::insertPlugin(this);
}

Plugin::Object::~Object(void) {
    Plugin::Manager::removePlugin(this);
}

std::string Plugin::Object::getLibrary(void) const {
    return library;
}

void Plugin::Object::unload(void) {
    Plugin::Manager::unload(this);
}

Plugin::Object *Plugin::Manager::load(const std::string &library) {
    if(unlikely(!instance))
        initialize();

    Mutex::Locker lock(&instance->mutex);

    void *handle = dlopen(library.c_str(),RTLD_GLOBAL|RTLD_NOW);
    if(!handle) {
        std::string plugin_dir = std::string(EXEC_PREFIX) + std::string("/lib/rtxi/");
        handle = dlopen((plugin_dir+library).c_str(),RTLD_GLOBAL|RTLD_NOW);
    }
    if(!handle) {
        ERROR_MSG("Plugin::load : failed to load %s: %s\n",library.c_str(),dlerror());
        return 0;
    }

    /*********************************************************************************
     * Apparently ISO C++ forbids against casting object pointer -> function pointer *
     *   But what the hell do they know? It is probably safe here...                 *
     *********************************************************************************/

    Object *(*create)(void) = (Object *(*)(void))(dlsym(handle,"createRTXIPlugin"));
    if(!create) {
        ERROR_MSG("Plugin::load : failed to load %s : %s\n",library.c_str(),dlerror());
        dlclose(handle);
        return 0;
    }

    Object *plugin = create();
    if(!plugin) {
        ERROR_MSG("Plugin::load : failed to load %s : failed to create instance\n",library.c_str());
        dlclose(handle);
        return 0;
    }
    if(plugin->magic_number != Plugin::Object::MAGIC_NUMBER) {
        ERROR_MSG("Plugin::load : the pointer returned from %s::createRTXIPlugin() isn't a valid Plugin::Object *.\n",library.c_str());
        dlclose(handle);
        return 0;
    }

    plugin->handle = handle;
    plugin->library = library;

    Event::Object event(Event::PLUGIN_INSERT_EVENT);
    event.setParam("plugin",plugin);
    Event::Manager::postEvent(&event);

    return plugin;
}

void Plugin::Manager::unload(Plugin::Object *plugin) {
    if(unlikely(!instance))
        initialize();

    if(!plugin) {
        ERROR_MSG("Plugin::Manager::unload : invalid plugin\n");
        return;
    }

    QCustomEvent *event = new QCustomEvent(CloseEvent,reinterpret_cast<void *>(plugin));
    QApplication::postEvent(instance,event);
}

void Plugin::Manager::unloadAll(void) {
    if(unlikely(!instance))
        initialize();

    void *handle;
    for(std::list<Object *>::iterator i = instance->pluginList.begin();i != instance->pluginList.end();i = instance->pluginList.begin()) {
        Event::Object event(Event::PLUGIN_REMOVE_EVENT);
        event.setParam("plugin",*i);
        Event::Manager::postEvent(&event);

        handle = (*i)->handle;
        delete *i;
        dlclose(handle);
    }
}

void Plugin::Manager::foreachPlugin(void (*callback)(Plugin::Object *,void *),void *param) {
    if(unlikely(!instance))
        initialize();

    Mutex::Locker lock(&instance->mutex);
    for(std::list<Plugin::Object *>::iterator i = instance->pluginList.begin();i != instance->pluginList.end();++i)
        callback(*i,param);
}

void Plugin::Manager::insertPlugin(Plugin::Object *plugin) {
    if(unlikely(!instance))
        initialize();

    if(!plugin) {
        ERROR_MSG("Plugin::Manager::insertPlugin : invalid plugin\n");
        return;
    }

    Mutex::Locker lock(&instance->mutex);
    instance->pluginList.push_back(plugin);
}

void Plugin::Manager::removePlugin(Plugin::Object *plugin) {
    if(unlikely(!instance))
        initialize();

    if(!plugin) {
        ERROR_MSG("Plugin::Manager::removePlugin : invalid plugin\n");
        return;
    }

    Mutex::Locker lock(&instance->mutex);
    instance->pluginList.remove(plugin);
}

void Plugin::Manager::customEvent(QCustomEvent *e) {
    if(e->type() == CloseEvent) {
        Mutex::Locker lock(&instance->mutex);

        Object *plugin = static_cast<Plugin::Object *>(e->data());

        Event::Object event(Event::PLUGIN_REMOVE_EVENT);
        event.setParam("plugin",plugin);
        Event::Manager::postEvent(&event);

        void *handle = plugin->handle;
        delete plugin;
        if(handle) dlclose(handle);
    }
}

static Mutex mutex;
Plugin::Manager *Plugin::Manager::instance = 0;

void Plugin::Manager::initialize(void) {
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
