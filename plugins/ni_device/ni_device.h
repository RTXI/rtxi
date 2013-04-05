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

#ifndef NI_DEVICE_H
#define NI_DEVICE_H

#include <daq.h>
#include <plugin.h>
#include <rt.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include <ni_devices.h>
#include <osiBus.h>
#include <scale.h>
#include <tMSeries.h>

class NIDevice : public DAQ::Device
{

public:

    NIDevice(IO::channel_t *,size_t,const char *,ni_device_t *,u32 *,size_t);
    ~NIDevice(void);

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
  bool getAnalogCalibrationActive(DAQ::type_t,DAQ::index_t) const {return true;};
  bool getAnalogCalibrationState(DAQ::type_t,DAQ::index_t) const {return false;};
    int setAnalogGain(DAQ::type_t,DAQ::index_t,double);
    int setAnalogZeroOffset(DAQ::type_t,DAQ::index_t,double);
    int setAnalogRange(DAQ::type_t,DAQ::index_t,DAQ::index_t);
    int setAnalogReference(DAQ::type_t,DAQ::index_t,DAQ::index_t);
    int setAnalogUnits(DAQ::type_t,DAQ::index_t,DAQ::index_t);
    int setAnalogOffsetUnits(DAQ::type_t,DAQ::index_t,DAQ::index_t);
    int setAnalogCalibration(DAQ::type_t,DAQ::index_t) {};
    int setAnalogCalibrationActive(DAQ::type_t,DAQ::index_t,bool) {};

    DAQ::direction_t getDigitalDirection(DAQ::index_t) const;
    int setDigitalDirection(DAQ::index_t,DAQ::direction_t);

    void read(void);
    void write(void);

    std::string getDeviceName(void) { return deviceName; }

protected:

    virtual void doLoad(const Settings::Object::State &);
    virtual void doSave(Settings::Object::State &) const;

private:

    bool analog_exists(DAQ::type_t,DAQ::index_t) const;

    struct analog_channel_t {
        bool active;
        double gain;
        DAQ::index_t range;
        DAQ::index_t reference;
        DAQ::index_t units;
        double zerooffset;
        tScalingCoefficients scale;
        DAQ::index_t offsetunits;
    };

    struct  digital_channel_t {
        bool active;
        DAQ::direction_t direction;
    };

    struct channel_t {
        union {
            analog_channel_t analog;
            digital_channel_t digital;
        };
    };

    struct subdevice_t {
        size_t active;
        DAQ::index_t count;
        channel_t *chan;
    };

    class AIConfig : public RT::Event {

    public:

        AIConfig(tMSeries *,ni_device_t *,u8 *,subdevice_t *,DAQ::index_t,bool);
        ~AIConfig(void);

        int callback(void);

    private:

        tMSeries *board;
        ni_device_t *info;
        u8 *eepromMemory;
        subdevice_t *subd;
        size_t channel;
        bool active;

    }; // class AIConfig

    class AOConfig : public RT::Event {

    public:

        AOConfig(tMSeries *,ni_device_t *,u8 *,subdevice_t *,DAQ::index_t,bool);
        ~AOConfig(void);

        int callback(void);

    private:

        tMSeries *board;
        ni_device_t *info;
        u8 *eepromMemory;
        subdevice_t *subd;
        size_t channel;
        bool active;

    }; // class AOConfig

    class DIOConfig : public RT::Event {

    public:

        DIOConfig(tMSeries *,ni_device_t *,subdevice_t *,DAQ::index_t,bool,DAQ::direction_t);
        ~DIOConfig(void);

        int callback(void);

    private:

        tMSeries *board;
        ni_device_t *info;
        u8 *eepromMemory;
        subdevice_t *subd;
        size_t channel;
        bool active;
        DAQ::direction_t direction;

    }; // class DIOConfig

    std::string deviceName;
    ni_device_t *info;

    subdevice_t subdevice[3];

    iBus *bus;

    u8 eepromMemory[1024];
    tAddressSpace bar1;
    tMSeries *board;

};

#endif /* NI_DEVICE_H */
