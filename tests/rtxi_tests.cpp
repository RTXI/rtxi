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


#define BOOST_TEST_MODULE rtxi_test 
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"
#include "rtxi_tests.h"

// Event::Manager testing suite
BOOST_FIXTURE_TEST_SUITE(eventManager, eventManagerFixture);

BOOST_AUTO_TEST_CASE(instance)
{
    BOOST_CHECK_EQUAL(dummyEventManager, Event::Manager::getInstance());
    BOOST_CHECK_EQUAL(dummyEventManager, dummyEventManager->getInstance());
}

BOOST_AUTO_TEST_CASE(postEvent)
{
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(postEventRT)
{
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END();

// Event::Handler testing suite
BOOST_FIXTURE_TEST_SUITE(eventHandler, eventHandlerFixture);

BOOST_AUTO_TEST_CASE(receiveEvent)
{
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END();

// Event::HandlerRT testing suite
BOOST_FIXTURE_TEST_SUITE(eventHandlerRT, eventHandlerRTFixture);

BOOST_AUTO_TEST_CASE(receiveEventRT)
{
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END();

// RT::System testing suite
BOOST_FIXTURE_TEST_SUITE(System, SystemFixture);

BOOST_AUTO_TEST_CASE(instance) 
{
    BOOST_CHECK_EQUAL(dummySystem, RT::System::getInstance());
    BOOST_CHECK_EQUAL(dummySystem, dummySystem->getInstance());
}

BOOST_AUTO_TEST_CASE(period)
{
    auto period = dummySystem->getPeriod();
    BOOST_CHECK_EQUAL(period, dummySystem->getPeriod());
    dummySystem->setPeriod(1000000ll);
    BOOST_CHECK_EQUAL(1000000ll, dummySystem->getPeriod());
}

BOOST_AUTO_TEST_CASE(forEachThread)
{
    BOOST_CHECK(true);          
}

BOOST_AUTO_TEST_CASE(forEachDevice)
{
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(postEvent)
{
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()


