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

#ifndef RTXI_TESTS_H
#define RTXI_TESTS_H

#include <rt.h>
#include <event.h>

// Define all fixtures for testing purposes
struct SystemFixture {
    SystemFixture() {dummySystem = RT::System::getInstance();}
    ~SystemFixture() { }
    RT::System* dummySystem;
    RT::Thread* dummyThread;
    RT::Event* dummyEvent;
};

struct eventManagerFixture {
    eventManagerFixture() {dummyEventManager = Event::Manager::getInstance();}
    ~eventManagerFixture() { }
    Event::Manager* dummyEventManager;
};

struct eventHandlerFixture {
    eventHandlerFixture() { }
    ~eventHandlerFixture() { }
    Event::Handler* dummyEventHandler;
};

struct eventHandlerRTFixture {
    eventHandlerRTFixture() { }
    ~eventHandlerRTFixture() { }
    Event::RTHandler* dummyEventHandlerRT;
};

struct deviceFixture {
    deviceFixture() {dummyDevice = new testTypeDevice();}
    ~deviceFixture() { }
    class testTypeDevice : public RT::Device
    {
        int readvar = 0;
        int writevar = 0;
        void read() { readvar++; }
        void write() { writevar++; }
    }
    testTypeDevice* dummyDevice;
}











#endif
