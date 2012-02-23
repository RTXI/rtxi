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

#include <debug.h>
#include <main_window.h>
#include <mutex.h>
#include <system_control.h>
#include <system_control_panel.h>

extern "C" Plugin::Object *createRTXIPlugin(void *) {
    return SystemControl::getInstance();
}

SystemControl::SystemControl(void) {
    menuID = MainWindow::getInstance()->createSystemMenuItem("Control Panel",this,SLOT(createControlPanel(void)));
}

SystemControl::~SystemControl(void) {
    MainWindow::getInstance()->removeSystemMenuItem(menuID);
    while(panelList.size())
        delete panelList.front();
    instance = 0;
}

void SystemControl::createControlPanel(void) {
    SystemControlPanel *panel = new SystemControlPanel(MainWindow::getInstance()->centralWidget());
    panelList.push_back(panel);
}

void SystemControl::removeControlPanel(SystemControlPanel *panel) {
    panelList.remove(panel);
}

static Mutex mutex;
SystemControl *SystemControl::instance = 0;

SystemControl *SystemControl::getInstance(void) {
    if(instance)
        return instance;

    /*************************************************************************
     * Seems like alot of hoops to jump through, but allocation isn't        *
     *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
     *************************************************************************/

    Mutex::Locker lock(&::mutex);
    if(!instance)
        instance = new SystemControl();

    return instance;
}
