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

#ifndef ANALOGY_DEVICE_H
#define ANALOGY_DEVICE_H

#include <string>

#include <daq.h>
#include <errno.h>
#include <plugin.h>
#include <rtdm/analogy.h>
#include <sys/stat.h>
#include <sys/types.h>

class AnalogyDevice : public DAQ::Device
{
public:
  AnalogyDevice(a4l_desc_t*, std::string, IO::channel_t*, size_t);
  ~AnalogyDevice(void);

  size_t getChannelCount(DAQ::type_t) const;
  bool getChannelActive(DAQ::type_t, DAQ::index_t) const;
  int setChannelActive(DAQ::type_t, DAQ::index_t, bool);

  size_t getAnalogRangeCount(DAQ::type_t, DAQ::index_t) const;
  size_t getAnalogReferenceCount(DAQ::type_t, DAQ::index_t) const;
  size_t getAnalogUnitsCount(DAQ::type_t, DAQ::index_t) const;
  size_t getAnalogDownsample(DAQ::type_t, DAQ::index_t) const;
  std::string getAnalogRangeString(DAQ::type_t,
                                   DAQ::index_t,
                                   DAQ::index_t) const;
  std::string getAnalogReferenceString(DAQ::type_t,
                                       DAQ::index_t,
                                       DAQ::index_t) const;
  std::string getAnalogUnitsString(DAQ::type_t,
                                   DAQ::index_t,
                                   DAQ::index_t) const;
  double getAnalogGain(DAQ::type_t, DAQ::index_t) const;
  double getAnalogZeroOffset(DAQ::type_t, DAQ::index_t) const;
  DAQ::index_t getAnalogRange(DAQ::type_t, DAQ::index_t) const;
  DAQ::index_t getAnalogReference(DAQ::type_t, DAQ::index_t) const;
  DAQ::index_t getAnalogUnits(DAQ::type_t, DAQ::index_t) const;
  DAQ::index_t getAnalogOffsetUnits(DAQ::type_t, DAQ::index_t) const;
  int setAnalogGain(DAQ::type_t, DAQ::index_t, double);
  int setAnalogZeroOffset(DAQ::type_t, DAQ::index_t, double);
  int setAnalogRange(DAQ::type_t, DAQ::index_t, DAQ::index_t);
  int setAnalogReference(DAQ::type_t, DAQ::index_t, DAQ::index_t);
  int setAnalogUnits(DAQ::type_t, DAQ::index_t, DAQ::index_t);
  int setAnalogOffsetUnits(DAQ::type_t, DAQ::index_t, DAQ::index_t);
  int setAnalogDownsample(DAQ::type_t, DAQ::index_t, size_t);
  int setAnalogCounter(DAQ::type_t, DAQ::index_t);
  void setAnalogConversion(DAQ::type_t, DAQ::index_t) {};  // Placeholder
  int setAnalogCalibrationValue(DAQ::type_t, DAQ::index_t, double);
  double getAnalogCalibrationValue(DAQ::type_t, DAQ::index_t) const;
  // TODO: Finish defining the functions below for AnalogyDevice class
  int setAnalogCalibrationActive(DAQ::type_t, DAQ::index_t, bool) { return 0; };
  bool getAnalogCalibrationActive(DAQ::type_t, DAQ::index_t) const
  {
    return false;
  };
  bool getAnalogCalibrationState(DAQ::type_t, DAQ::index_t) const
  {
    return false;
  };

  DAQ::direction_t getDigitalDirection(DAQ::index_t) const;
  int setDigitalDirection(DAQ::index_t, DAQ::direction_t);

  void read(void);
  void write(void);

protected:
  virtual void doLoad(const Settings::Object::State&);
  virtual void doSave(Settings::Object::State&) const;

private:
  bool analog_exists(DAQ::type_t, DAQ::index_t) const;

  struct analog_channel_t
  {
    double gain;
    DAQ::index_t range;
    DAQ::index_t reference;
    DAQ::index_t units;
    double conv;
    double offset;
    double zerooffset;
    lsampl_t maxdata;
    DAQ::index_t offsetunits;
    size_t downsample;
    size_t counter;
    bool calibrationActive;
    double calOffset;
  };

  struct digital_channel_t
  {
    DAQ::direction_t direction;
    int previous_value;
  };

  struct channel_t
  {
    bool active;
    union
    {
      analog_channel_t analog;
      digital_channel_t digital;
    };
  };

  struct subdevice_t
  {
    int id;
    DAQ::index_t active;
    DAQ::index_t count;
    channel_t* chan;
  };

  std::string deviceName;
  subdevice_t subdevice[3];
  a4l_desc_t dsc;
};

#endif /* ANALOGY_DEVICE_H */
