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

//! Event oriented classes
/*!
 * Objects contained within this namespace are responsible
 *   for dispatching signals.
 */
namespace Event {

    /*!
     * Name of the event that is posted when the period is changed.
     *
     * \sa RT::System::setPeriod()
     * \sa Event::RTHandler
     */
    extern const char *RT_PERIOD_EVENT;
    /*!
     * Name of the event that is posted before the period is changed.
     *
     * \sa RT::System::setPeriod()
     * \sa Event::Handler
     */
    extern const char *RT_PREPERIOD_EVENT;
    /*!
     * Name of the event that is posted after the period is changed.
     *
     * \sa RT::System::setPeriod()
     * \sa Event::Handler
     */
    extern const char *RT_POSTPERIOD_EVENT;
    /*!
     * Name of the event that is posted when a thread is inserted.
     *
     * \sa RT::Thread
     */
    extern const char *RT_THREAD_INSERT_EVENT;
    /*!
     * Name of the event that is posted when a thread is removed.
     *
     * \sa RT::Thread
     */
    extern const char *RT_THREAD_REMOVE_EVENT;
    /*!
     * Name of the event that is posted when a device is inserted.
     *
     * \sa RT::Device
     */
    extern const char *RT_DEVICE_INSERT_EVENT;
    /*!
     * Name of the event that is posted when a device is removed.
     *
     * \sa RT::Device
     */
    extern const char *RT_DEVICE_REMOVE_EVENT;
    /*!
     * Name of the event that is posted when an IO::Block is inserted.
     *
     * \sa IO::Block
     */
    extern const char *IO_BLOCK_INSERT_EVENT;
    /*!
     * Name of the event that is posted when an IO::Block is removed.
     *
     * \sa IO::Block
     */
    extern const char *IO_BLOCK_REMOVE_EVENT;
    /*!
     * Name of the event that is posted when a link is created between IO::Blocks.
     *
     * \sa IO::Block
     * \sa IO::Connector::connect()
     */
    extern const char *IO_LINK_INSERT_EVENT;
    /*!
     * Name of the event that is posted when a link is removed between IO::Blocks.
     *
     * \sa IO::Block
     * \sa IO::Connector::disconnect()
     */
    extern const char *IO_LINK_REMOVE_EVENT;
    /*!
     * Name of the event that is posted when a Workspace::Instance's parameter has changed.
     *
     * \sa Workspace::Instance
     */
    extern const char *WORKSPACE_PARAMETER_CHANGE_EVENT;
    /*!
     * Name of the event that is posted when a plugin is inserted.
     *
     * \sa Plugin::Manager::load()
     */
    extern const char *PLUGIN_INSERT_EVENT;
    /*!
     * Name of the event that is posted when a plugin is removed.
     *
     * \sa Plugin::Manager::unload()
     * \sa Plugin::Manager::unloadAll()
     */
    extern const char *PLUGIN_REMOVE_EVENT;
    /*!
     * Name of the event that is posted when a Settings::Object is created.
     *
     * \sa Settings::Object
     */
    extern const char *SETTINGS_OBJECT_INSERT_EVENT;
    /*!
     * Name of the event that is posted when a Settings::Object is removed.
     *
     * \sa Settings::Object
     */
    extern const char *SETTINGS_OBJECT_REMOVE_EVENT;
    /*!
     * An event that is posted to signal data recorders to start recording.
     */
    extern const char *START_RECORDING_EVENT;
    /*!
     * An event that is posted to signal data recorders to stop recording.
     */
    extern const char *STOP_RECORDING_EVENT;
    /*!
     * An event that signals some data to be recorded.
     */
    extern const char *ASYNC_DATA_EVENT;
    /*!
     * An event that signals that some threshold has been crossed.
     */
    extern const char *THRESHOLD_CROSSING_EVENT;

    /*!
     * Object that represents a single instance of an event.
     *   Stores all the parameters related to that event.
     */
    class Object {

    public:

        /*!
         * Constructor.
         *
         * \param name The name of the event type to be constructed.
         */
        Object(const char *name);
        ~Object(void);

        /*!
         * Function used to identify the type of event.
         *
         * \return The type of event represented by this object.
         */
        const char *getName(void) const { return name; };

        /*!
         * Function used to access the event's parameters.
         *
         * \param name The parameter name.
         *
         * \return The value of the parameter.
         *
         * \sa Event::Object::setParam()
         */
        void *getParam(const char *name) const;
        /*!
         * Function used to set the event's parameters.
         *
         * \param name The parameter's name.
         * \param value The parameter's value.
         */
        void setParam(const char *name,void *value);

        const static size_t MAX_PARAMS = 8;

    private:

        const char *name;
        size_t nparams;
        struct { const char *name; void *value; } params[MAX_PARAMS];

    }; // class Object

    class Handler;
    class RTHandler;

    /*!
     * Managaes the collection of all objects waiting to
     *   receive signals from events.
     */
    class Manager {

        friend class Handler;
        friend class RTHandler;

    public:

        static void initialize(void);

        /*!
         * Function for posting an event to be signaled. This function
         * should only be called from non-realtime, and blocks until
         * it has been dispatched to all handlers.
         *
         * \param event The event to be posted.
         *
         * \sa Event::Handler
         * \sa Event::Object
         */
        static void postEvent(const Object *event);

        /*!
         * Function for posting an event to be signaled. This function
         * should only be called from realtime, and blocks until
         * it has been dispatched to all handlers.
         *
         * \param event The event to be posted.
         *
         * \sa Event::RTHandler
         * \sa Event::Object
         */
        static void postEventRT(const Object *event);

    private:

        Manager(void);
        ~Manager(void);
        Manager(const Manager &) {};
        Manager &operator=(const Manager &) { return *instance; };

        static Manager *instance;

        static void registerHandler(Handler *);
        static void unregisterHandler(Handler *);

        static void registerRTHandler(RTHandler *);
        static void unregisterRTHandler(RTHandler *);

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
