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

#ifndef EVENT_H
#define EVENT_H

#include <fifo.h>
#include <list>
#include <map>
#include <pthread.h>
#include <rt.h>
#include <string>

//! Event Oriented Classes
/*
 * Objects contained within this namespace are responsible
 *   for dispatching signals.
 */
namespace Event {

    class Object {

    public:

        Object(const char *);
        ~Object(void);

        const char *getName(void) const { return name; };

        void *getParam(const char *) const;
        void setParam(const char *,void *);

        const static size_t MAX_PARAMS = 8;

    private:

        const char *name;
        size_t nparams;
        struct { const char *name; void *value; } params[MAX_PARAMS];

    }; // class Object

    class Handler;
    class RTHandler;

    /*
     * Managaes the collection of all objects waiting to
     *   receive signals from events.
     */
    class Manager {

        friend class Handler;
        friend class RTHandler;

    public:

        /*!
         * Manager is a Singleton, which means that there can only be
         * one instance. This function returns a pointer to that
         * single instance.
         *
         * \return The instance of Manager.
         */
        static Manager *getInstance(void);

        /*!
         * Function for posting an event to be signaled. This function
         * should only be called from non-realtime, and blocks until
         * it is been dispatched to all handlers.
         *
         * \param event The event to be posted.
         *
         * \sa Event::Handler
         * \sa Event::Object
         */
        void postEvent(const Object *event);

        /*!
         * Function for posting an event to be signaled. This function
         * should only be called from realtime, and blocks until
         * it is been dispatched to all handlers.
         *
         * \param event The event to be posted.
         *
         * \sa Event::RTHandler
         * \sa Event::Object
         */
        void postEventRT(const Object *event);

    private:

        Manager(void);
        ~Manager(void);
        Manager(const Manager &) {};
        Manager &operator=(const Manager &) { return *getInstance(); };

        static Manager *instance;

        void registerHandler(Handler *);
        void unregisterHandler(Handler *);

        void registerRTHandler(RTHandler *);
        void unregisterRTHandler(RTHandler *);

        Mutex mutex;
        std::list<Handler *> handlerList;
        RT::List<RTHandler> rthandlerList;

    }; // class Manager

    /*!
     * Object that is signaled when an event is posted.
     *
     * \sa Event::Manager::postEvent()
     */
    class Handler {

    public:

        Handler(void);
        virtual ~Handler(void);

        /*!
         * Function that is called in non-realtime everytime an non-realtime
         *  event is posted.
         *
         * \param event The the event being posted.
         *
         * \sa Event::Object
         * \sa Event::Manager::postEvent()
         */
        virtual void receiveEvent(const Object *event);

    }; // class Handler

    /*!
     * Object that is signaled when a realtime event is posted.
     *
     * \sa Event::Manager::postEventRT()
     */
    class RTHandler : public RT::List<RTHandler>::Node {

    public:

        RTHandler(void);
        virtual ~RTHandler(void);

        /*!
         * Function that is called in realtime everytime a realtime
         *  event is posted.
         *
         * \param name The the event being posted.
         *
         * \sa Event::Object
         * \sa Event::Manager::postEventRT()
         */
        virtual void receiveEventRT(const Object *event);

    }; // class RTHandler

}; // namespace Event

#endif // EVENT_H
