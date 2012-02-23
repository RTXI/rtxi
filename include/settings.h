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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <debug.h>

#include <list>
#include <map>
#include <mutex.h>
#include <string>

class QDomDocument;
class QDomElement;

//! Objects for saving/loading setting information between sessions.
/*!
 *
 */
namespace Settings {

    /*!
     *
     */
    class Object {

        friend class Manager;

    public:

        /*!
         *
         */
        typedef unsigned long ID;
        /*!
         *
         */
        const static ID INVALID = 0;

        /*!
         *
         */
        class State {

            friend class Manager;
            friend class Object;

        public:

            State(void);
            State(ID n);
            ~State(void);

            /*!
             *
             */
            double loadDouble(const std::string &name) const;
            /*!
             *
             */
            int loadInteger(const std::string &name) const;
            /*!
             *
             */
            std::string loadString(const std::string &name) const;
            /*!
             *
             */
            void saveDouble(const std::string &name,double);
            /*!
             *
             */
            void saveInteger(const std::string &name,int);
            /*!
             *
             */
            void saveString(const std::string &name,const std::string &value);

            /*!
             *
             */
            State loadState(const std::string &name) const;
            /*!
             *
             */
            void saveState(const std::string &name,const State &value);

            /*!
             *
             */
            QDomElement xml(QDomDocument &) const;
            /*!
             *
             */
            void xml(const QDomElement &);

        private:

            ID id;
            std::map<std::string,std::string> paramMap;
            std::map<std::string,State> stateMap;

        };

        Object(void);
        virtual ~Object(void);

        /*!
         *
         */
        ID getID(void) const { return id; };

        /*!
         *
         */
        State save(void) const;
        /*!
         *
         */
        void load(const State &);
        /*!
         *
         */
        void deferred(const State &);

    protected:

        /*!
         *
         */
        virtual void doLoad(const State &) {};
        /*!
         *
         */
        virtual void doDeferred(const State &) {};
        /*!
         *
         */
        virtual void doSave(State &) const {};

    private:

        ID id;

    }; // class Object

    /*!
     *
     */
    class Manager {

        friend class Object;

    public:

        /*!
         *
         */
        static Manager *getInstance(void);

        /*!
         *
         */
        Object *getObject(Object::ID) const;
        /*!
         *
         */
        void foreachObject(void (*callback)(Object *,void *),void *param);

        /*!
         *
         */
        int load(const std::string &);
        /*!
         *
         */
        int save(const std::string &);

    private:

        Manager(void) : mutex(Mutex::RECURSIVE), currentID(Object::INVALID+1) {};
        ~Manager(void) {};
        Manager(const Manager &) {};
        Manager &operator=(const Manager &) { return *getInstance(); };

        static Manager *instance;

        void acquireID(Object *,Object::ID =Object::INVALID);
        void releaseID(Object *);

        void insertObject(Object *);
        void removeObject(Object *);

        mutable Mutex mutex;
        Object::ID currentID;
        std::list<Object *> objectList;
        std::map<Object::ID,Object *> objectMap;

    }; // class Manager

}; // namespace Settings

#endif // SETTINGS_H
