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

#include <ni_device.h>
#include <ni_devices.h>
#include <ni_driver.h>
#include <debug.h>

#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <unistd.h>

extern "C" Plugin::Object *createRTXIPlugin(void)
{
    return new NIDriver();
}

NIDriver::~NIDriver(void) {
    for(std::list<NIDevice *>::iterator i = devices.begin();i != devices.end();++i)
        delete *i;
}

DAQ::Device *NIDriver::createDevice(const std::list<std::string> &args) {
    int bus = -1;
    float dev;

    sscanf(args.front().c_str(),"PCI::%d::%f",&bus,&dev);

    if(bus < 0) {
        u32 bus_id = (u32)-1;
        u32 pci_id;
        char buffer[512];
        std::ifstream file("/proc/bus/pci/devices");

        while(!file.eof()) {
            file.getline(buffer,sizeof(buffer));
            sscanf(buffer,"%x %x %*s",&bus_id,&pci_id);

            if((pci_id >> 16) == 0x1093)
                break;
        }

        if(bus_id == (u32)-1) {
            ERROR_MSG("NIDriver::createDevice : scan for NI devices failed, no devices detected.\n");
            return 0;
        }

        bus = (bus_id & 0x0000FF00) >> 8;
        dev = (bus_id & 0x000000F8) >> 3;
    }

    //printf("%s -> BUS : %d DEVICE : %f\n",args.front().c_str(),bus,dev);
    //printf("filename : /proc/bus/pci/%02d/%04.1f\n",bus,dev);

    ni_device_t *device_info = 0;
    struct {
        u16 vendor_id;
        u16 device_id;
        u32 junk[3];
        u32 bar[2];
    } pci_info;

    char filename[256];
    sprintf(filename,"/proc/bus/pci/%02d/%04.1f",bus,dev);

    int fd;
    if((fd = open(filename,O_RDONLY)) < 0) {
        ERROR_MSG("NIDriver::createDevice : failed to open %s for reading to acquire device information.\n");
        return 0;
    }
    read(fd,&pci_info,sizeof(pci_info));

    for(size_t i=0;i<ni_devices_size;++i)
        if(ni_devices[i].deviceID == pci_info.device_id) {
            device_info = ni_devices+i;
            break;
        }

    if(!device_info) {
        ERROR_MSG("NIDriver::createDevice : device (%X) isn't currently supported by this driver.\n",pci_info.device_id);
        return 0;
    }

    int offset;
    size_t dio_count = device_info->dio_count > 32 ? 32 : device_info->dio_count;
    size_t num_channels = device_info->ai_count+device_info->ao_count+2*dio_count;
    IO::channel_t channels[device_info->ai_count+device_info->ao_count+2*dio_count];

    offset = 0;
    for(int i=offset;i<device_info->ai_count+offset;++i) {
        std::ostringstream name;
        name << "Analog Input " << i-offset;;
        channels[i].name = name.str();
        channels[i].description = "";
        channels[i].flags = IO::OUTPUT;
    }
    offset += device_info->ai_count;
    for(int i=offset;i<device_info->ao_count+offset;++i) {
        std::ostringstream name;
        name << "Analog Output " << i-offset;
        channels[i].name = name.str();
        channels[i].description = "";
        channels[i].flags = IO::INPUT;
    }
    offset += device_info->ao_count;
    for(int i=offset;i<dio_count+offset;++i) {
        std::ostringstream name;
        name << "Digital Input/Output " << i-offset;
        channels[i].name = name.str();
        channels[i].description = "";
        channels[i].flags = IO::OUTPUT;
    }
    offset += dio_count;
    for(int i=offset;i<dio_count+offset;++i) {
        std::ostringstream name;
        name << "Digital Input/Output " << i-offset;
        channels[i].name = name.str();
        channels[i].description = "";
        channels[i].flags = IO::INPUT;
    }

    NIDevice *device = new NIDevice(channels,num_channels,args.front().c_str(),device_info,pci_info.bar,sizeof(pci_info.bar)/sizeof(pci_info.bar[0]));
    devices.push_back(device);
    return device;
}

void NIDriver::doLoad(const Settings::Object::State &s) {
    for(size_t i = 0, end = s.loadInteger("Num Devices");i < end;++i) {
        std::list<std::string> args;
        args.push_back(s.loadString(QString::number(i).toStdString()));
        DAQ::Device *device = createDevice(args);
        if(device)
            device->load(s.loadState(QString::number(i).toStdString()));
    }
}

void NIDriver::doSave(Settings::Object::State &s) const {
    s.saveInteger("Num Devices",devices.size());
    size_t n = 0;
    for(std::list<NIDevice *>::const_iterator i = devices.begin(),end = devices.end();i != end; ++i) {
        std::ostringstream str;
        str << n++;
        s.saveString(str.str(),(*i)->getDeviceName());
        s.saveState(str.str(),(*i)->save());
    }
}
