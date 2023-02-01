/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Weill Cornell Medical College

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

#include "fakePlugin.h"

extern "C" {
std::unique_ptr<Modules::Plugin> createRTXIPlugin(Event::Manager* ev_manager, 
                                                            MainWindow* mw)
{
  return std::make_unique<fakePlugin>(ev_manager, mw);
}

std::unique_ptr<Modules::Component> createRTXIComponent(Modules::Plugin* host_plugin)
{
  return std::make_unique<fakeComponent>(host_plugin);
}

Modules::Panel* createRTXIPanel(MainWindow* mw, Event::Manager* ev_manager)
{
  return nullptr;
}

}
