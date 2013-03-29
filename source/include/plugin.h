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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <list>
#include <mutex.h>
#include <settings.h>
#include <string>
#include <qobject.h>

#include <sys/types.h>

//! Classes associated with the loading/unloading of binaries at run-time.
/*!
 * Collection of classes to control the loading and unloading of code at run-time.
 */
namespace Plugin {

    class Object;

    /*!
     * Provides mechanisms for the loading and unloading of a Plugin::Object
     */
    class Manager : public QObject {

        Q_OBJECT

        friend class Object;

    public:

        /*!
         * Manager is a Singleton, which means that there can only be one instance.
         *   This function returns a pointer to that single instance.
         *
         * \return The instance of Manager.
         */
        static Manager *getInstance(void);

        /*!
         * Function for loading a Plugin::Object from a shared library file.
         *
         * \param library The file name of a shared library.
         * \return A pointer to the newly created Plugin::Object.
         *
         * \sa Plugin::Object
         */
        Object *load(const std::string &library);
        /*!
         * Function for unloading a single Plugin::Object in the system.
         *
         * \param object The plugin object to be unloaded.
         */
        void unload(Object *object);
        /*!
         * Function for unloading all Plugin::Object's in the system.
         */
        void unloadAll(void);

        /*!
         * Loop through each Plugin and execute a callback.
         * The callback takes two parameters, a Plugin pointer and param,
         *   the second parameter to foreachPlugin.
         *
         * \param callback The callback function.
         * \param param A parameter to the callback function.
         *
         * \sa Plugin::Object
         */
        void foreachPlugin(void (*callback)(Plugin::Object *,void *),void *param);

    public slots:

        /*!
         *
         */
        void customEvent(QCustomEvent *);

    private:

        /*****************************************************************
         * The constructor, destructor, and assignment operator are made *
         *   private to control instantiation of the class.              *
         *****************************************************************/

        Manager(void) : mutex(Mutex::RECURSIVE) {};
        ~Manager(void) {};
        Manager(const Manager &) {};
        Manager &operator=(const Manager &) { return *getInstance(); };

        static Manager *instance;

        void insertPlugin(Object *);
        void removePlugin(Object *);

        static const QEvent::Type CloseEvent = QEvent::User;

        Mutex mutex;
        std::list<Object *> pluginList;

    }; // class Manager

    /*!
     * Provides interface for objects that are loaded from external binaries.
     */
    class Object : public virtual Settings::Object {

        friend class Manager;

    public:

        Object(void);
        virtual ~Object(void);

        /*!
         * Get the name of the library from which the object was loaded.
         *
         * \return The library file the object from which the object was created.
         */
        std::string getLibrary(void) const;

        /*!
         * A mechanism which an object can use to unload itself. Should only be
         *   called from within the GUI thread.
         */
        void unload(void);

    private:

        static const u_int32_t MAGIC_NUMBER = 0xCA24CB3F;

        u_int32_t magic_number;
        std::string library;
        void *handle;

    }; // class Object

}; // namespace Plugin

#endif // PLUGIN_H
