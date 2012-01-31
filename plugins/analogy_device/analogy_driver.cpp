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
#include <analogy_device.h>
#include <analogy_driver.h>
#include <debug.h>
#include <sstream>

extern "C" Plugin::Object *createRTXIPlugin(void)
{
    return new AnalogyDriver();
}

AnalogyDriver::~AnalogyDriver(void) {
    for(std::list<AnalogyDevice *>::iterator i = deviceList.begin();i != deviceList.end();++i)
        delete *i;
}

DAQ::Device *AnalogyDriver::createDevice(const std::list<std::string> &args) {
    int err = 0;
    a4l_desc_t dsc;
    a4l_sbinfo_t *sbinfo;
    std::string name = args.front();

    err = a4l_open(&dsc, name.c_str());
    if(err < 0) {
        ERROR_MSG("AnalogyDriver::createDevice : unable to open %s (err=%d).\n", name.c_str(), err);
        return 0;
    }

	// Allocate a buffer so as to get more info (subd, chan, rng) 
	dsc.sbdata = malloc(dsc.sbsize);
	if (dsc.sbdata == NULL) {
		err = -ENOMEM;
		ERROR_MSG("AnalogyDriver: info buffer allocation failed\n");
        return 0;
	}

	// Get this data 
	err = a4l_fill_desc(&dsc);
	if (err < 0) {
		ERROR_MSG("AnalogyDriver: a4l_fill_desc failed (err=%d)\n",	err);
        return 0;
	}

    // We need to find each subdevice index manually since idx_*_subd often fails
    // Go over all subdevices and save the indexes of the first AI, AO and DIO
    int idx_ai  = -1; 
    int idx_ao  = -1; 
    int idx_dio = -1; 
    for (int i=0; i < dsc.nb_subd; i++) {
        err = a4l_get_subdinfo(&dsc, i, &sbinfo); 
        if(err != 0) {
            ERROR_MSG("AnalogyDriver: a4l_get_subd_info failed, wrong subdevice index %i (err=%d)\n",
                      err, i);
            return 0;
        }
        // Assign index; save just the first device if many
        if (((sbinfo->flags & A4L_SUBD_TYPES )== A4L_SUBD_AI) && (idx_ai < 0))
            idx_ai  = i; 
        else if (((sbinfo->flags & A4L_SUBD_TYPES) == A4L_SUBD_AO) && (idx_ao < 0))
            idx_ao  = i; 
        else if (((sbinfo->flags & A4L_SUBD_TYPES) == A4L_SUBD_DIO) && (idx_dio < 0))
            idx_dio  = i; 
    }

    size_t count[5] = { 0, 0, 0, 0, 0,};
    err = a4l_get_subdinfo(&dsc, idx_ai, &sbinfo); 
    if(err == 0)
        count[0] = sbinfo->nb_chan;
    err = a4l_get_subdinfo(&dsc, idx_ao, &sbinfo); 
    if(err == 0)
        count[1] = sbinfo->nb_chan;

    count[2] = count[3] = count[4] = 0;

    err = a4l_get_subdinfo(&dsc, idx_dio, &sbinfo); 
    if(err != 0)
        count[2] = sbinfo->nb_chan;

    if(!(count[0]+count[1]+count[2]+count[3]+count[4])) {
        ERROR_MSG("AnalogyDriver::createDevice : no Analogy device configured on %s.\n",name.c_str());
        a4l_close(&dsc);
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

    AnalogyDevice *dev = new AnalogyDevice(&dsc,name,channel,count[0]+count[1]+2*count[2]);
    deviceList.push_back(dev);
    return dev;
 
    return 0;
}

void AnalogyDriver::doLoad(const Settings::Object::State &s) {
    for(size_t i = 0, end = s.loadInteger("Num Devices");i < end;++i) {
        std::list<std::string> args;
        args.push_back(s.loadString(QString::number(i)));
        DAQ::Device *device = createDevice(args);
        if(device)
            device->load(s.loadState(QString::number(i)));
    }
}

void AnalogyDriver::doSave(Settings::Object::State &s) const {
    s.saveInteger("Num Devices",deviceList.size());
    size_t n = 0;
    for(std::list<AnalogyDevice *>::const_iterator i = deviceList.begin(),end = deviceList.end();i != end; ++i) {
        std::ostringstream str;
        str << n++;
        s.saveString(str.str(),(*i)->getName());
        s.saveState(str.str(),(*i)->save());
    }
}
