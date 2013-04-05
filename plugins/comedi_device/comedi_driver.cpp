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

#include <comedi_device.h>
#include <comedi_driver.h>
#include <debug.h>
#include <sstream>

extern "C" Plugin::Object *createRTXIPlugin(void)
{
    return new ComediDriver();
}

ComediDriver::~ComediDriver(void) {
    for(std::list<ComediDevice *>::iterator i = deviceList.begin();i != deviceList.end();++i)
        delete *i;
}

DAQ::Device *ComediDriver::createDevice(const std::list<std::string> &args) {
    void *device;
    ComediLib::comedi_t *comedi_device;
    std::string name = args.front();
    if(!(device = comedi_open(name.c_str()))) {
        ERROR_MSG("ComediDriver::createDevice : unable to open %s.\n",name.c_str());
        return 0;
    }

    if(!(comedi_device = ComediLib::comedi_open(name.c_str()))) { // ComediLib version used for integration with comedi_calibrate
        ERROR_MSG("ComediDriver::createDevice : (ComediLib) unable to open %s.\n",name.c_str());
        return 0;
    }

    int subd;
    size_t count[5] = { 0, 0, 0, 0, 0,};
    if((subd = comedi_find_subdevice_by_type(device,COMEDI_SUBD_AI,0)) >= 0)
        count[0] = comedi_get_n_channels(device,subd);
    if((subd = comedi_find_subdevice_by_type(device,COMEDI_SUBD_AO,0)) >= 0)
        count[1] = comedi_get_n_channels(device,subd);

    count[2] = count[3] = count[4] = 0;

    if((subd = comedi_find_subdevice_by_type(device,COMEDI_SUBD_DIO,0)) >= 0)
        count[2] = comedi_get_n_channels(device,subd);

/*
    if((subd = comedi_find_subdevice_by_type(device,COMEDI_SUBD_DI,0)) >= 0)
        count[3] = comedi_get_n_channels(device,subd);
    if((subd = comedi_find_subdevice_by_type(device,COMEDI_SUBD_DO,0)) >= 0)
        count[4] = comedi_get_n_channels(device,subd);
*/
    if(!(count[0]+count[1]+count[2]+count[3]+count[4])) {
        ERROR_MSG("ComediDriver::createDevice : no Comedi device configured on %s.\n",name.c_str());
        comedi_close(device);
        return 0;
    }

    IO::channel_t channel[count[0]+count[1]+2*count[2]];
    for(size_t i=0;i<count[0];++i) {
        std::ostringstream name;
        name << "Analog Input " << i;
        channel[i].name = name.str();
        channel[i].description = "";
        channel[i].flags = IO::OUTPUT;
    }
    for(size_t i=count[0];i<count[0]+count[1];++i) {
        std::ostringstream name;
        name << "Analog Output " << i-count[0];
        channel[i].name = name.str();
        channel[i].description = "";
        channel[i].flags = IO::INPUT;
    }
    for(size_t i=count[0]+count[1];i<count[0]+count[1]+count[2];++i) {
        std::ostringstream name;
        name << "Digital IO " << i-count[0]-count[1];
        channel[i].name = name.str();
        channel[i].description = "";
        channel[i].flags = IO::OUTPUT;
    }
    for(size_t i=count[0]+count[1]+count[2];i<count[0]+count[1]+2*count[2];++i) {
        std::ostringstream name;
        name << "Digital Input/Output " << i-count[0]-count[1]-count[2];
        channel[i].name = name.str();
        channel[i].description = "";
        channel[i].flags = IO::INPUT;
    }
    for(size_t i=count[0]+count[1]+2*count[2];i<count[0]+count[1]+2*count[2]+count[3];++i) {
        std::ostringstream name;
        name << "Digital Input " << i-count[0]-count[1]-2*count[2];
        channel[i].name = name.str();
        channel[i].description = "";
        channel[i].flags = IO::OUTPUT;
    }
    for(size_t i=count[0]+count[1]+2*count[2]+count[3];i<count[0]+count[1]+2*count[2]+count[3]+count[4];++i) {
        std::ostringstream name;
        name << "Digital Output " << i-count[0]-count[1]-2*count[2]-count[3];
        channel[i].name = name.str();
        channel[i].description = "";
        channel[i].flags = IO::INPUT;
    }

    ComediDevice *dev = new ComediDevice(device,comedi_device,name,channel,count[0]+count[1]+2*count[2]);
    deviceList.push_back(dev);
    return dev;
}

void ComediDriver::doLoad(const Settings::Object::State &s) {
    for(size_t i = 0, end = s.loadInteger("Num Devices");i < end;++i) {
        std::list<std::string> args;
        args.push_back(s.loadString(QString::number(i)));
        DAQ::Device *device = createDevice(args);
        if(device)
            device->load(s.loadState(QString::number(i)));
    }
}

void ComediDriver::doSave(Settings::Object::State &s) const {
    s.saveInteger("Num Devices",deviceList.size());
    size_t n = 0;
    for(std::list<ComediDevice *>::const_iterator i = deviceList.begin(),end = deviceList.end();i != end; ++i) {
        std::ostringstream str;
        str << n++;
        s.saveString(str.str(),(*i)->getName());
        s.saveState(str.str(),(*i)->save());
    }
}
