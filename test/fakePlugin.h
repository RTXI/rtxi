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

#ifndef FAKE_PLUGIN_H
#define FAKE_PLUGIN_H

#include <plugin.h>
#include <gmock/gmock.h>
#include <iostream>

// This is a fake plugin created in order to test Plugin Manager
class fakePlugin : public ::Plugin::Object 
{
public:
    fakePlugin() { std::cout << "Fake Plugin Constructed\n"; }
    ~fakePlugin() { std::cout << "Fake Plugin Destroyed\n"; }
};

#endif
