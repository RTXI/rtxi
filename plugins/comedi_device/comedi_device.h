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

#ifndef COMEDI_DEVICE_H
#define COMEDI_DEVICE_H

#include <daq.h>
#include <plugin.h>
#include <rtai_lxrt.h>
#include <rtai_comedi.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

/*
  ComediLib is used to gather calibration from comedi_calibrate and
  comedi_soft_calibrate. A namespace is used to prevent collisions
  between rtai_comedi.h and comedilib.h
*/
namespace ComediLib {
#include <comedilib.h>
}

class ComediDevice : public DAQ::Device
{

public:

    ComediDevice(void *,ComediLib::comedi_t *,std::string,IO::channel_t *,size_t);
    ~ComediDevice(void);

    size_t getChannelCount(DAQ::type_t) const;
    bool getChannelActive(DAQ::type_t,DAQ::index_t) const;
    int setChannelActive(DAQ::type_t,DAQ::index_t,bool);

    size_t getAnalogRangeCount(DAQ::type_t,DAQ::index_t) const;
    size_t getAnalogReferenceCount(DAQ::type_t,DAQ::index_t) const;
    size_t getAnalogUnitsCount(DAQ::type_t,DAQ::index_t) const;
    std::string getAnalogRangeString(DAQ::type_t,DAQ::index_t,DAQ::index_t) const;
    std::string getAnalogReferenceString(DAQ::type_t,DAQ::index_t,DAQ::index_t) const;
    std::string getAnalogUnitsString(DAQ::type_t,DAQ::index_t,DAQ::index_t) const;
    double getAnalogGain(DAQ::type_t,DAQ::index_t) const;
    double getAnalogZeroOffset(DAQ::type_t,DAQ::index_t) const;
    DAQ::index_t getAnalogRange(DAQ::type_t,DAQ::index_t) const;
    DAQ::index_t getAnalogReference(DAQ::type_t,DAQ::index_t) const;
    DAQ::index_t getAnalogUnits(DAQ::type_t,DAQ::index_t) const;
    DAQ::index_t getAnalogOffsetUnits(DAQ::type_t,DAQ::index_t) const;
    bool getAnalogCalibrationActive(DAQ::type_t,DAQ::index_t) const;
    bool getAnalogCalibrationState(DAQ::type_t,DAQ::index_t) const;
    int setAnalogGain(DAQ::type_t,DAQ::index_t,double);
    int setAnalogZeroOffset(DAQ::type_t,DAQ::index_t,double);
    int setAnalogRange(DAQ::type_t,DAQ::index_t,DAQ::index_t);
    int setAnalogReference(DAQ::type_t,DAQ::index_t,DAQ::index_t);
    int setAnalogUnits(DAQ::type_t,DAQ::index_t,DAQ::index_t);
    int setAnalogOffsetUnits(DAQ::type_t,DAQ::index_t,DAQ::index_t);
    int setAnalogCalibration(DAQ::type_t,DAQ::index_t);
    int setAnalogCalibrationActive(DAQ::type_t,DAQ::index_t,bool);

    DAQ::direction_t getDigitalDirection(DAQ::index_t) const;
    int setDigitalDirection(DAQ::index_t,DAQ::direction_t);

    void read(void);
    void write(void);

protected:

    virtual void doLoad(const Settings::Object::State &);
    virtual void doSave(Settings::Object::State &) const;

private:

    bool analog_exists(DAQ::type_t,DAQ::index_t) const;

    struct analog_channel_t {
        double gain;
        DAQ::index_t range;
        DAQ::index_t reference;
        DAQ::index_t units;
        double conv;
        double offset;
        double zerooffset;
        lsampl_t maxdata;
        DAQ::index_t offsetunits;
        bool calibrated;
        bool calibrationActive;
        double coefficients[4]; // If comedi calibrated, will have max 4 coefficients
        unsigned order;
        double expansionOrigin;
    };

    struct  digital_channel_t {
        DAQ::direction_t direction;
        int previous_value;
    };

    struct channel_t {
        bool active;
        union {
            analog_channel_t analog;
            digital_channel_t digital;
        };
    };

    struct subdevice_t {
        int id;
        DAQ::index_t active;
        DAQ::index_t count;
        channel_t *chan;
    };

    std::string deviceName;
    subdevice_t subdevice[3];
    void *device;
    ComediLib::comedi_t *comedi_device;
    ComediLib::comedi_calibration_t *calibration;

};

#endif /* COMEDI_DEVICE_H */
