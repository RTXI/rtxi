/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Will Cornell Medical College

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

#include <QApplication>

#include <debug.h>
#include <dlfcn.h>
#include <event.h>
#include <plugin.h>

Plugin::Object::Object(void)
    : magic_number(Plugin::Object::MAGIC_NUMBER)
    , handle(0)
{
  Plugin::Manager::getInstance()->insertPlugin(this);
}

Plugin::Object::~Object(void)
{
  Plugin::Manager::getInstance()->removePlugin(this);
}

std::string Plugin::Object::getLibrary(void) const
{
  return library;
}

void Plugin::Object::unload(void)
{
  Plugin::Manager::getInstance()->unload(this);
}

Plugin::Object* Plugin::Manager::load(const QString& library)
{
  Mutex::Locker lock(&mutex);

  void* handle = dlopen(library.toStdString().c_str(), RTLD_GLOBAL | RTLD_NOW);
  if (!handle) {
    // ERROR_MSG("Plugin::load : failed to load library: %s", dlerror());
    std::string plugin_dir =
        std::string(EXEC_PREFIX) + std::string("/lib/rtxi/");
    handle = dlopen((plugin_dir + library.toStdString()).c_str(),
                    RTLD_GLOBAL | RTLD_NOW);
  }
  if (!handle) {
    ERROR_MSG("Plugin::load : failed to load %s: %s\n",
              library.toStdString().c_str(),
              dlerror());
    return 0;
  }

  /*********************************************************************************
   * Apparently ISO C++ forbids against casting object pointer -> function
   *pointer * But what the hell do they know? It is probably safe here... *
   *********************************************************************************/

  Object* (*create)(void) =
      (Object * (*)(void))(dlsym(handle, "createRTXIPlugin"));
  if (!create) {
    ERROR_MSG("Plugin::load : failed to load %s : %s\n",
              library.toStdString().c_str(),
              dlerror());
    dlclose(handle);
    return 0;
  }

  Object* plugin = create();
  if (!plugin) {
    ERROR_MSG("Plugin::load : failed to load %s : failed to create instance\n",
              library.toStdString().c_str());
    dlclose(handle);
    return 0;
  }
  if (plugin->magic_number != Plugin::Object::MAGIC_NUMBER) {
    ERROR_MSG(
        "Plugin::load : the pointer returned from %s::createRTXIPlugin() isn't "
        "a valid Plugin::Object *.\n",
        library.toStdString().c_str());
    dlclose(handle);
    return 0;
  }

  plugin->handle = handle;
  plugin->library = library.toStdString();

  Event::Object event(Event::PLUGIN_INSERT_EVENT);
  event.setParam("plugin", plugin);
  Event::Manager::getInstance()->postEvent(&event);

  return plugin;
}

void Plugin::Manager::unload(Plugin::Object* plugin)
{
  if (!plugin) {
    ERROR_MSG("Plugin::Manager::unload : invalid plugin\n");
    return;
  }

  /* Unfortunate but we have to recast to properly
   * insert this event into the queue for QT and RTXI
   * to process.
   */
  RTXIEvent* event = new RTXIEvent();
  event->data = reinterpret_cast<void*>(plugin);
  QApplication::postEvent(this, reinterpret_cast<QEvent*>(event));
}

void Plugin::Manager::unloadAll(void)
{
  void* handle;
  for (std::list<Object*>::iterator i = pluginList.begin();
       i != pluginList.end();
       i = pluginList.begin())
  {
    Event::Object event(Event::PLUGIN_REMOVE_EVENT);
    event.setParam("plugin", *i);
    Event::Manager::getInstance()->postEvent(&event);

    handle = (*i)->handle;
    delete *i;
    dlclose(handle);
  }
}

void Plugin::Manager::foreachPlugin(void (*callback)(Plugin::Object*, void*),
                                    void* param)
{
  Mutex::Locker lock(&mutex);
  for (std::list<Plugin::Object*>::iterator i = pluginList.begin();
       i != pluginList.end();
       ++i)
    callback(*i, param);
}

void Plugin::Manager::insertPlugin(Plugin::Object* plugin)
{
  if (!plugin) {
    ERROR_MSG("Plugin::Manager::insertPlugin : invalid plugin\n");
    return;
  }

  Mutex::Locker lock(&mutex);
  pluginList.push_back(plugin);
}

void Plugin::Manager::removePlugin(Plugin::Object* plugin)
{
  if (!plugin) {
    ERROR_MSG("Plugin::Manager::removePlugin : invalid plugin\n");
    return;
  }

  Mutex::Locker lock(&mutex);
  // TODO: uhhh removing a plugin by calling plugin destructor, which calls this
  // function... not good.
  pluginList.remove(plugin);
}

void Plugin::Manager::customEvent(QEvent* e)
{
  if (e->type() == CloseEvent) {
    RTXIEvent* e2 = reinterpret_cast<RTXIEvent*>(e);

    Mutex::Locker lock(&mutex);

    Object* plugin = reinterpret_cast<Plugin::Object*>(e2->data);

    Event::Object event(Event::PLUGIN_REMOVE_EVENT);
    event.setParam("plugin", plugin);
    Event::Manager::getInstance()->postEvent(&event);

    void* handle = plugin->handle;
    delete plugin;
    if (handle)
      dlclose(handle);
  }
}

static Mutex mutex;
Plugin::Manager* Plugin::Manager::instance = 0;

Plugin::Manager* Plugin::Manager::getInstance(void)
{
  if (instance)
    return instance;

  /*************************************************************************
   * Seems like alot of hoops to jump through, but static allocation isn't *
   *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
   *************************************************************************/

  Mutex::Locker lock(&::mutex);
  if (!instance) {
    static Manager manager;
    instance = &manager;
  }

  return instance;
}
