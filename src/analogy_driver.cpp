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


#include <rtdm/analogy.h>

#include <string>

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "debug.hpp"
#include "daq.hpp"

constexpr std::string_view DEFAULT_DRIVER_NAME = "Analogy";

class AnalogyDevice : public DAQ::Device
{
public:
  AnalogyDevice(a4l_desc_t*, std::string, IO::channel_t*, size_t);
  AnalogyDevice(const std::string& dev_name,
                const std::vector<IO::channel_t>& channels,
                std::string internal_name);
  ~AnalogyDevice(void);

  size_t getChannelCount(DAQ::ChannelType::type_t type) const final;
  bool getChannelActive(DAQ::ChannelType::type_t type,
                        DAQ::index_t index) const final;
  int setChannelActive(DAQ::ChannelType::type_t type,
                       DAQ::index_t index,
                       bool state) final;
  size_t getAnalogRangeCount(DAQ::index_t index) const final;
  size_t getAnalogReferenceCount(DAQ::index_t index) const final;
  size_t getAnalogUnitsCount(DAQ::index_t index) const final;
  size_t getAnalogDownsample(DAQ::ChannelType::type_t type,
                             DAQ::index_t index) const final;
  std::string getAnalogRangeString(DAQ::ChannelType::type_t type,
                                   DAQ::index_t index,
                                   DAQ::index_t range) const final;
  std::string getAnalogReferenceString(DAQ::ChannelType::type_t type,
                                       DAQ::index_t index,
                                       DAQ::index_t reference) const final;
  std::string getAnalogUnitsString(DAQ::ChannelType::type_t type,
                                   DAQ::index_t index,
                                   DAQ::index_t units) const final;
  double getAnalogGain(DAQ::ChannelType::type_t type,
                       DAQ::index_t index) const final;
  double getAnalogZeroOffset(DAQ::ChannelType::type_t type,
                             DAQ::index_t index) const final;
  DAQ::index_t getAnalogRange(DAQ::ChannelType::type_t type,
                              DAQ::index_t index) const final;
  DAQ::index_t getAnalogReference(DAQ::ChannelType::type_t type,
                                  DAQ::index_t index) const final;
  DAQ::index_t getAnalogUnits(DAQ::ChannelType::type_t type,
                              DAQ::index_t index) const final;
  DAQ::index_t getAnalogOffsetUnits(DAQ::ChannelType::type_t type,
                                    DAQ::index_t index) const final;
  int setAnalogGain(DAQ::ChannelType::type_t type,
                    DAQ::index_t index,
                    double gain) final;
  int setAnalogRange(DAQ::ChannelType::type_t type,
                     DAQ::index_t index,
                     DAQ::index_t range) final;
  int setAnalogZeroOffset(DAQ::ChannelType::type_t type,
                          DAQ::index_t index,
                          double offset) final;
  int setAnalogReference(DAQ::ChannelType::type_t type,
                         DAQ::index_t index,
                         DAQ::index_t reference) final;
  int setAnalogUnits(DAQ::ChannelType::type_t type,
                     DAQ::index_t index,
                     DAQ::index_t units) final;
  int setAnalogOffsetUnits(DAQ::ChannelType::type_t type,
                           DAQ::index_t index,
                           DAQ::index_t units) final;
  int setAnalogDownsample(DAQ::ChannelType::type_t type,
                          DAQ::index_t index,
                          size_t downsample) final;
  int setAnalogCounter(DAQ::ChannelType::type_t type, DAQ::index_t index) final;
  int setAnalogCalibrationValue(DAQ::ChannelType::type_t type,
                                DAQ::index_t index,
                                double value) final;
  double getAnalogCalibrationValue(DAQ::ChannelType::type_t type,
                                   DAQ::index_t index) const final;
  int setAnalogCalibrationActive(DAQ::ChannelType::type_t type,
                                 DAQ::index_t index,
                                 bool state) final;
  bool getAnalogCalibrationActive(DAQ::ChannelType::type_t type,
                                  DAQ::index_t index) const final;
  bool getAnalogCalibrationState(DAQ::ChannelType::type_t type,
                                 DAQ::index_t index) const final;
  int setDigitalDirection(DAQ::index_t index, DAQ::direction_t direction) final;

  void read() final;
  void write() final;

private:
  bool analog_exists(ChannelType::type_t, DAQ::index_t) const;

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


class AnalogyDriver
    : public DAQ::Driver
{
public:
  AnalogyDriver(void)
      : DAQ::Driver(std::string(DEFAULT_DRIVER_NAME)) {}
  virtual ~AnalogyDriver(void);

  static DAQ::Driver* getInstance();
  void loadDevices() final;
  void unloadDevices() final;
  std::vector<DAQ::Device*> getDevices() final;

private:
  std::vector<AnalogyDevice*> devices;
};

namespace
{
AnalogyDriver* instance = nullptr;
}  // namespace

DAQ::Driver* AnalogyDriver::getInstance()
{
  if (instance == nullptr) {
    instance = new AnalogyDriver();
  }
  return instance;
}

extern "C"
{
DAQ::Driver* getRTXIDAQDriver()
{
  return Driver::getInstance();
}

void deleteRTXIDAQDriver()
{
  delete instance;
}

}

AnalogyDriver::~AnalogyDriver(void)
{
  for (std::list<AnalogyDevice*>::iterator i = devices.begin();
       i != devices.end();
       ++i)
    delete *i;
}

DAQ::Device* AnalogyDriver::createDevice(const std::list<std::string>& args)
{
  int err = 0;
  a4l_desc_t dsc;
  a4l_sbinfo_t* sbinfo;
  std::string name = args.front();

  err = a4l_open(&dsc, name.c_str());
  if (err < 0) {
    ERROR_MSG("AnalogyDriver::createDevice : unable to open %s (err=%d).\n",
              name.c_str(),
              err);
    return 0;
  }

  // Allocate a buffer to get more info (subd, chan, rng)
  dsc.sbdata = malloc(dsc.sbsize);
  if (dsc.sbdata == NULL) {
    err = -ENOMEM;
    ERROR_MSG("AnalogyDriver: info buffer allocation failed\n");
    return 0;
  }

  // Get this data
  err = a4l_fill_desc(&dsc);
  if (err < 0) {
    ERROR_MSG(
        "AnalogyDriver: a4l_fill_desc failed (err=%d)\nPlease run "
        "rtxi_load_analogy script in scripts directory.\n",
        err);
    return 0;
  }

  // We need to find each subdevice index manually since idx_*_subd often fails
  // Go over all subdevices and save the indexes of the first AI, AO and DIO
  int idx_ai = -1;
  int idx_ao = -1;
  int idx_dio = -1;
  for (int i = 0; i < dsc.nb_subd; i++) {
    err = a4l_get_subdinfo(&dsc, i, &sbinfo);
    if (err != 0) {
      ERROR_MSG(
          "AnalogyDriver: a4l_get_subd_info failed, wrong subdevice index %i "
          "(err=%d)\n",
          err,
          i);
      return 0;
    }
    // Assign index; save just the first device if many
    if (((sbinfo->flags & A4L_SUBD_TYPES) == A4L_SUBD_AI) && (idx_ai < 0))
      idx_ai = i;
    else if (((sbinfo->flags & A4L_SUBD_TYPES) == A4L_SUBD_AO) && (idx_ao < 0))
      idx_ao = i;
    else if (((sbinfo->flags & A4L_SUBD_TYPES) == A4L_SUBD_DIO)
             && (idx_dio < 0))
      idx_dio = i;
  }

  size_t count[5] = {
      0,
      0,
      0,
      0,
      0,
  };
  err = a4l_get_subdinfo(&dsc, idx_ai, &sbinfo);
  if (err == 0)
    count[0] = sbinfo->nb_chan;
  err = a4l_get_subdinfo(&dsc, idx_ao, &sbinfo);
  if (err == 0)
    count[1] = sbinfo->nb_chan;
  err = a4l_get_subdinfo(&dsc, idx_dio, &sbinfo);
  if (err == 0)
    count[2] = sbinfo->nb_chan;

  if (!(count[0] + count[1] + count[2] + count[3] + count[4])) {
    ERROR_MSG(
        "AnalogyDriver::createDevice : no Analogy device configured on %s.\n",
        name.c_str());
    a4l_close(&dsc);
    return 0;
  }

  IO::channel_t channel[count[0] + count[1] + 2 * count[2]];
  for (size_t i = 0; i < count[0]; ++i) {
    std::ostringstream name;
    name << "Analog Input " << i;
    channel[i].name = name.str();
    channel[i].description = "";
    channel[i].flags = IO::OUTPUT;
  }
  for (size_t i = count[0]; i < count[0] + count[1]; ++i) {
    std::ostringstream name;
    name << "Analog Output " << i - count[0];
    channel[i].name = name.str();
    channel[i].description = "";
    channel[i].flags = IO::INPUT;
  }
  for (size_t i = count[0] + count[1]; i < count[0] + count[1] + count[2]; ++i)
  {
    std::ostringstream name;
    name << "Digital I/O " << i - count[0] - count[1];
    channel[i].name = name.str();
    channel[i].description = "";
    channel[i].flags = IO::OUTPUT;
  }
  for (size_t i = count[0] + count[1] + count[2];
       i < count[0] + count[1] + 2 * count[2];
       ++i)
  {
    std::ostringstream name;
    name << "Digital I/O " << i - count[0] - count[1] - count[2];
    channel[i].name = name.str();
    channel[i].description = "";
    channel[i].flags = IO::INPUT;
  }
  for (size_t i = count[0] + count[1] + 2 * count[2];
       i < count[0] + count[1] + 2 * count[2] + count[3];
       ++i)
  {
    std::ostringstream name;
    name << "Digital Input " << i - count[0] - count[1] - 2 * count[2];
    channel[i].name = name.str();
    channel[i].description = "";
    channel[i].flags = IO::OUTPUT;
  }
  for (size_t i = count[0] + count[1] + 2 * count[2] + count[3];
       i < count[0] + count[1] + 2 * count[2] + count[3] + count[4];
       ++i)
  {
    std::ostringstream name;
    name << "Digital Output "
         << i - count[0] - count[1] - 2 * count[2] - count[3];
    channel[i].name = name.str();
    channel[i].description = "";
    channel[i].flags = IO::INPUT;
  }

  AnalogyDevice* dev = new AnalogyDevice(
      &dsc, name, channel, count[0] + count[1] + 2 * count[2]);
  devices.push_back(dev);
  return dev;

  return 0;
}

void AnalogyDriver::doLoad(const Settings::Object::State& s)
{
  for (size_t i = 0, end = s.loadInteger("Num Devices"); i < end; ++i) {
    std::list<std::string> args;
    args.push_back(s.loadString(QString::number(i).toStdString()));
    DAQ::Device* device = createDevice(args);
    if (device)
      device->load(s.loadState(QString::number(i).toStdString()));
  }
}

void AnalogyDriver::doSave(Settings::Object::State& s) const
{
  s.saveInteger("Num Devices", devices.size());
  size_t n = 0;
  for (std::list<AnalogyDevice*>::const_iterator i = devices.begin(),
                                                 end = devices.end();
       i != end;
       ++i)
  {
    std::ostringstream str;
    str << n++;
    s.saveString(str.str(), (*i)->getName());
    s.saveState(str.str(), (*i)->save());
  }
}
