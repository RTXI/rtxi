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

#ifndef NI_DRIVER_H
#define NI_DRIVER_H

#include <daq.h>
#include <plugin.h>

class NIDriver : public Plugin::Object, public DAQ::Driver
{

public:

    NIDriver(void) : DAQ::Driver("NI") {};
    virtual ~NIDriver(void);

    virtual DAQ::Device *createDevice(const std::list<std::string> &);

protected:

    virtual void doLoad(const State &);
    virtual void doSave(State &) const;

private:

    std::list<NIDevice *> devices;

};

#endif // NI_DRIVER_H
