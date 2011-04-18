/*
 * Copyright (C) 2004 Boston University
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

#ifndef SYSTEM_CONTROL_H
#define SYSTEM_CONTROL_H

#include <plugin.h>
#include <qobject.h>
#include <system_control_panel.h>

class SystemControl : public QObject, public Plugin::Object
{

    Q_OBJECT

    friend class SystemControlPanel;

public:

    static SystemControl *getInstance(void);

public slots:

    void createControlPanel(void);

private:

    SystemControl(void);
    ~SystemControl(void);
    SystemControl(const SystemControl &) {};
    SystemControl &operator=(const SystemControl &) { return *getInstance(); };

    static SystemControl *instance;

    void removeControlPanel(SystemControlPanel *);

    int menuID;
    std::list<SystemControlPanel *> panelList;

};

#endif /* SYSTEM_CONTROL_H */
