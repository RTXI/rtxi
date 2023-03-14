/*
 	 The Real-Time eXperiment Interface (RTXI)
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Will Cornell Medical College

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

#ifndef FAKE_PLUGIN_H
#define FAKE_PLUGIN_H

#include <iostream>

#include "module.hpp"

#include "io_tests.hpp"
#include "module_tests.hpp"


const std::vector<Modules::Variable::Info> defaultFakeModuleVariables = 
    generateDefaultComponentVariables();
const std::vector<IO::channel_t> defaultFakeChannelList = 
    generateDefaultChannelList();

// This is a fake plugin created in order to test Plugin Manager
class fakePlugin : public Modules::Plugin 
{
public:
    fakePlugin(Event::Manager* ev_manager, MainWindow* mw) : 
            Modules::Plugin(ev_manager, mw, "fakeModule") 
        { std::cout << "Fake Plugin Constructed\n"; }
    ~fakePlugin() { std::cout << "Fake Plugin Destroyed\n"; }
};

class fakeComponent : public Modules::Component
{
public:
    fakeComponent(Modules::Plugin* hplugin) :
        Modules::Component(hplugin, 
                           "fakeModule", 
                           defaultFakeChannelList, 
                           defaultFakeModuleVariables)
        { std::cout << "Fake Component Created\n"; } 
    ~fakeComponent() { std::cout << "Fake Component Destroyed\n"; }
};

#endif
