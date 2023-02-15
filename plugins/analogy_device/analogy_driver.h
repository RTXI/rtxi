/*
 * Copyright (C) 2012 University of Bristol, UK
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

#ifndef ANALOGY_DRIVER_H
#define ANALOGY_DRIVER_H

#include <daq.h>
#include <plugin.h>

class AnalogyDriver
    : public Plugin::Object
    , public DAQ::Driver
{
public:
  AnalogyDriver(void)
      : DAQ::Driver("analogy") {};
  virtual ~AnalogyDriver(void);

  virtual DAQ::Device* createDevice(const std::list<std::string>&);

protected:
  virtual void doLoad(const State&);
  virtual void doSave(State&) const;

private:
  std::list<AnalogyDevice*> devices;
};

#endif
