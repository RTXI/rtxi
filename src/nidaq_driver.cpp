
extern "C"
{
#include <NIDAQmx.h>
}

#include <tuple>
#include <utility>

#include <fmt/core.h>

#include "daq.hpp"

const std::string_view DEFAULT_DRIVER_NAME = "National Instruments";

namespace
{

inline std::vector<std::string> split_string(const std::string& buffer,
                                             const std::string& delim)
{
  if (buffer.empty()) {
    return {};
  }
  size_t pos_start = 0;
  size_t pos_end = buffer.find(delim, pos_start);
  if (pos_end == std::string::npos) {
    return {buffer};
  }
  std::string token;
  std::vector<std::string> split_tokens;
  const size_t delim_len = delim.size();
  while (true) {
    token = buffer.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    split_tokens.push_back(token);
    pos_end = buffer.find(delim, pos_start);
    if (pos_end == std::string::npos) {
      break;
    }
  }
  token = buffer.substr(pos_start, buffer.size() - pos_start);
  split_tokens.push_back(token);
  return split_tokens;
}

inline int32_t index_to_reference(DAQ::index_t index)
{
  int32_t result = 0;
  switch (static_cast<DAQ::Reference::reference_t>(index)) {
    case DAQ::Reference::GROUND:
      result = DAQmx_Val_RSE;
      break;
    case DAQ::Reference::COMMON:
      result = DAQmx_Val_NRSE;
      break;
    case DAQ::Reference::DIFFERENTIAL:
      result = DAQmx_Val_Diff;
      break;
    default:
      result = DAQmx_Val_Cfg_Default;
      break;
  }
  return result;
}

inline DAQ::Reference::reference_t reference_to_index(int32_t ni_daq_reference)
{
  DAQ::Reference::reference_t result = DAQ::Reference::UNKNOWN;
  switch (ni_daq_reference) {
    case DAQmx_Val_RSE:
      result = DAQ::Reference::GROUND;
      break;
    case DAQmx_Val_NRSE:
      result = DAQ::Reference::COMMON;
      break;
    case DAQmx_Val_Diff:
      result = DAQ::Reference::DIFFERENTIAL;
      break;
    default:
      result = DAQ::Reference::OTHER;
      break;
  }
  return result;
}

inline std::vector<std::string> physical_channel_names(
    const std::string& device, DAQ::ChannelType::type_t chan_type)
{
  int32_t (*func)(const char*, char*, uint32_t) = nullptr;
  switch (chan_type) {
    case DAQ::ChannelType::AO:
      func = &DAQmxGetDevAOPhysicalChans;
      break;
    case DAQ::ChannelType::AI:
      func = &DAQmxGetDevAIPhysicalChans;
      break;
    case DAQ::ChannelType::DO:
      func = &DAQmxGetDevDOLines;
      break;
    case DAQ::ChannelType::DI:
      func = &DAQmxGetDevDILines;
      break;
    default:
      return {};
  }
  std::string channel_names;
  const int32_t buff_size = func(device.c_str(), nullptr, 0);
  if (buff_size <= 0) {
    return {};
  }
  channel_names.resize(static_cast<size_t>(buff_size));
  func(device.c_str(), channel_names.data(), static_cast<uint32_t>(buff_size));
  return split_string(channel_names, ", ");
}

inline void printError(int32_t status)
{
  if (status == 0) {
    return;
  }
  ERROR_MSG("NIDAQ ERROR : code {}", status);
  const int32_t error_size = DAQmxGetErrorString(status, nullptr, 0);
  if (error_size < 0) {
    ERROR_MSG("Unable to get code message");
    return;
  }
  std::string err_str(static_cast<size_t>(error_size), '\0');
  const int32_t errcode = DAQmxGetErrorString(
      status, err_str.data(), static_cast<uint32_t>(error_size));
  if (errcode < 0) {
    ERROR_MSG("Unable to parse message");
    return;
  }
  ERROR_MSG("Message : {}", err_str);
}

inline std::string physical_card_name(const std::string& device_name)
{
  const int32_t err = DAQmxGetDevProductType(device_name.c_str(), nullptr, 0);
  std::string result(static_cast<size_t>(err), '\0');
  DAQmxGetDevProductType(
      device_name.c_str(), result.data(), static_cast<uint32_t>(result.size()));
  return result;
}

struct physical_channel_t
{
  explicit physical_channel_t(std::string chan_name,
                              DAQ::ChannelType::type_t chan_type,
                              size_t chan_id)
      : name(std::move(chan_name))
      , type(chan_type)
      , id(chan_id)
  {
  }
  int32_t addToTask(TaskHandle task_handle) const;
  std::string name;
  DAQ::ChannelType::type_t type = DAQ::ChannelType::UNKNOWN;
  size_t id;
  int32_t reference = DAQmx_Val_RSE;
  double offset = 0.0;
  double gain = 1.0;
  size_t range_index = 0;
  size_t units_index = 0;
  bool active = false;
};

int32_t physical_channel_t::addToTask(TaskHandle task_handle) const
{
  int32_t err = 0;
  const std::string units = DAQ::get_default_units().at(units_index);
  auto [min, max] = DAQ::get_default_ranges().at(range_index);
  switch (type) {
    case DAQ::ChannelType::AI:
      if (units == "volts") {
        err = DAQmxCreateAIVoltageChan(task_handle,
                                       name.c_str(),
                                       nullptr,
                                       reference,
                                       min,
                                       max,
                                       DAQmx_Val_Volts,
                                       nullptr);
      } else if (units == "amps") {
        err = DAQmxCreateAICurrentChan(task_handle,
                                       name.c_str(),
                                       nullptr,
                                       reference,
                                       min,
                                       max,
                                       DAQmx_Val_Amps,
                                       DAQmx_Val_Default,
                                       0,
                                       nullptr);
      } else {
        ERROR_MSG("NIDAQ : Virtual Channel Creation : Unknown units {}", units);
      }
      printError(err);
      break;
    case DAQ::ChannelType::AO:
      if (units == "volts") {
        err = DAQmxCreateAOVoltageChan(task_handle,
                                       name.c_str(),
                                       nullptr,
                                       min,
                                       max,
                                       DAQmx_Val_Volts,
                                       nullptr);
      } else if (units == "amps") {
        err = DAQmxCreateAOCurrentChan(task_handle,
                                       name.c_str(),
                                       nullptr,
                                       min,
                                       max,
                                       DAQmx_Val_Amps,
                                       nullptr);
      } else {
        ERROR_MSG("NIDAQ : Virtual Channel Creation : Unknown units {}", units);
      }
      printError(err);
      break;
    case DAQ::ChannelType::DI:
      err = DAQmxCreateDIChan(
          task_handle, name.c_str(), nullptr, DAQmx_Val_ChanPerLine);
      printError(err);
      break;
    case DAQ::ChannelType::DO:
      err = DAQmxCreateDOChan(
          task_handle, name.c_str(), nullptr, DAQmx_Val_ChanPerLine);
      printError(err);
      break;
    default:
      ERROR_MSG("NIDAQ_DRIVER : Channel Type Unknown");
      break;
  }
  return err;
}

class Device final : public DAQ::Device
{
public:
  Device(const Device&) = default;
  Device(Device&&) = delete;
  Device& operator=(const Device&) = delete;
  Device& operator=(Device&&) = delete;
  Device(const std::string& dev_name,
         const std::vector<IO::channel_t>& channels,
         std::string internal_name);
  ~Device() final;

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
  std::array<TaskHandle, DAQ::ChannelType::UNKNOWN> task_list {};
  std::array<std::vector<physical_channel_t*>, DAQ::ChannelType::UNKNOWN>
      active_channels;
  std::string internal_dev_name;
  std::array<std::vector<physical_channel_t>, 4> physical_channels_registry;
  std::array<DAQ::analog_range_t, 7> default_ranges = DAQ::get_default_ranges();
  std::array<std::string, 2> default_units = DAQ::get_default_units();
  std::tuple<std::vector<double>,
             std::vector<double>,
             std::vector<uint8_t>,
             std::vector<uint8_t>>
      buffer_arrays;
};

class Driver : public DAQ::Driver
{
public:
  static DAQ::Driver* getInstance();
  void loadDevices() final;
  void unloadDevices() final;
  std::vector<DAQ::Device*> getDevices() final;

private:
  Driver();
  std::vector<Device> nidaq_devices;
};

Device::Device(const std::string& dev_name,
               const std::vector<IO::channel_t>& channels,
               std::string internal_name)
    : DAQ::Device(dev_name, channels)
    , internal_dev_name(std::move(internal_name))
{
  size_t inputs_count = 0;
  size_t outputs_count = 0;
  std::string task_name;
  for (size_t type = 0; type < DAQ::ChannelType::UNKNOWN; type++) {
    task_name = DAQ::ChannelType::type2string(
        static_cast<DAQ::ChannelType::type_t>(type));
    if (DAQmxCreateTask(task_name.c_str(), &task_list.at(type)) < 0) {
      ERROR_MSG("NIDAQ::Device : Unable to create {} task for device {}",
                task_name,
                dev_name);
    } else {
      switch (type) {
        case DAQ::ChannelType::AI:
        case DAQ::ChannelType::DI:
          DAQmxCfgInputBuffer(task_list.at(type), 1);
          DAQmxSetReadOverWrite(task_list.at(type),
                                DAQmx_Val_OverwriteUnreadSamps);
          break;
        case DAQ::ChannelType::AO:
        case DAQ::ChannelType::DO:
          DAQmxCfgOutputBuffer(task_list.at(type), 1);
          break;
        default:
          break;
      }
    }
  }
  std::vector<std::string> chan_names;
  for (size_t type = 0; type < physical_channels_registry.size(); type++) {
    chan_names = physical_channel_names(
        internal_dev_name, static_cast<DAQ::ChannelType::type_t>(type));
    for (const auto& chan_name : chan_names) {
      physical_channels_registry.at(type).emplace_back(
          chan_name,
          static_cast<DAQ::ChannelType::type_t>(type),
          type % 2 == 0 ? inputs_count : outputs_count);
      type % 2 == 0 ? inputs_count++ : outputs_count++;
    }
  }
  std::get<DAQ::ChannelType::AI>(buffer_arrays)
      .assign(getChannelCount(DAQ::ChannelType::AI), 0);
  std::get<DAQ::ChannelType::AO>(buffer_arrays)
      .assign(getChannelCount(DAQ::ChannelType::AO), 0);
  std::get<DAQ::ChannelType::DI>(buffer_arrays)
      .assign(getChannelCount(DAQ::ChannelType::DI), 0);
  std::get<DAQ::ChannelType::DO>(buffer_arrays)
      .assign(getChannelCount(DAQ::ChannelType::DO), 0);

  for (auto& task : task_list) {
    DAQmxSetSampTimingType(task, DAQmx_Val_OnDemand);
    DAQmxTaskControl(task, DAQmx_Val_Task_Commit);
    DAQmxStartTask(task);
  }
  this->setActive(/*act=*/true);
}

Device::~Device()
{
  for (const auto& task : task_list) {
    DAQmxStopTask(task);
    DAQmxClearTask(task);
  }
}

size_t Device::getChannelCount(DAQ::ChannelType::type_t type) const
{
  return physical_channels_registry.at(type).size();
}

bool Device::getChannelActive(DAQ::ChannelType::type_t type,
                              DAQ::index_t index) const
{
  return physical_channels_registry.at(type).at(index).active;
}

int Device::setChannelActive(DAQ::ChannelType::type_t type,
                             DAQ::index_t index,
                             bool state)
{
  // Unfortunately National Instruments DAQ cards under nidaqmx library best
  // work under tasks, which are more efficient at reading when started before
  // any action. The bad thing is that NI does not make it easy to just remove a
  // channel. Removing means clearing the task, then
  // starting the task again, and adding all of the other channels.
  int32_t err = 0;
  TaskHandle& task = task_list.at(type);
  physical_channel_t& chan = physical_channels_registry.at(type).at(index); 
  if (chan.active == state) {
    return 0;
  }
  DAQmxStopTask(task);
  if (state) {
    err = chan.addToTask(
        task);
    chan.active = err == 0;
  } else {
    chan.active = false;
    DAQmxClearTask(task);
    err = DAQmxCreateTask(DAQ::ChannelType::type2string(type).c_str(),
                          &task);
    switch (type) {
      case DAQ::ChannelType::AI:
      case DAQ::ChannelType::DI:
        DAQmxCfgInputBuffer(task, 1);
        DAQmxSetReadOverWrite(task,
                              DAQmx_Val_OverwriteUnreadSamps);
        break;
      case DAQ::ChannelType::AO:
      case DAQ::ChannelType::DO:
        DAQmxCfgOutputBuffer(task, 1);
        break;
      default:
        break;
    }
    if (err != 0) {
      for (auto& channel : physical_channels_registry.at(type)) {
        channel.active = false;
      }
      active_channels.at(type).clear();
      return err;
    }
    for (const auto& channel : physical_channels_registry.at(type)) {
      if (!channel.active) {
        continue;
      }
      err = channel.addToTask(task);
      if (err != 0) {
        break;
      }
    }
  }
  printError(err);
  // active channels is an optimization for faster reading/writing in realtime.
  active_channels.at(type).clear();
  for (auto& channel : physical_channels_registry.at(type)) {
    if (channel.active) {
      active_channels.at(type).push_back(&channel);
    }
  }
  if(DAQmxGetTaskChannels(task, nullptr, 0) > 0){
    printError(DAQmxSetSampTimingType(task, DAQmx_Val_OnDemand));
    printError(DAQmxTaskControl(task, DAQmx_Val_Task_Commit));
    printError(DAQmxStartTask(task));
  }
  return err;
}

size_t Device::getAnalogRangeCount(DAQ::index_t /*index*/) const
{
  return default_ranges.size();
}

size_t Device::getAnalogReferenceCount(DAQ::index_t /*index*/) const
{
  return DAQ::Reference::UNKNOWN;
}

size_t Device::getAnalogUnitsCount(DAQ::index_t /*index*/) const
{
  return default_units.size();
}

size_t Device::getAnalogDownsample(DAQ::ChannelType::type_t /*type*/,
                                   DAQ::index_t /*index*/) const
{
  return 0;
}

std::string Device::getAnalogRangeString(DAQ::ChannelType::type_t type,
                                         DAQ::index_t index,
                                         DAQ::index_t /*range*/) const
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return "";
  }
  const std::string formatting = "{:.1f}";
  auto [min, max] = default_ranges.at(
      physical_channels_registry.at(type).at(index).range_index);
  return fmt::format(formatting, min) + std::string(" to ")
      + fmt::format(formatting, max);
}

std::string Device::getAnalogReferenceString(DAQ::ChannelType::type_t type,
                                             DAQ::index_t index,
                                             DAQ::index_t /*reference*/) const
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return "";
  }
  const int32_t ref = physical_channels_registry.at(type).at(index).reference;
  std::string refstr;
  switch (ref) {
    case DAQmx_Val_RSE:
      refstr = "Ground";
      break;
    case DAQmx_Val_NRSE:
      refstr = "Common";
      break;
    case DAQmx_Val_Diff:
      refstr = "Differential";
      break;
    case DAQmx_Val_PseudoDiff:
      refstr = "Other";
      break;
    default:
      refstr = "Unknown";
      break;
  }
  return refstr;
}

std::string Device::getAnalogUnitsString(DAQ::ChannelType::type_t type,
                                         DAQ::index_t index,
                                         DAQ::index_t /*units*/) const
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return "";
  }
  return default_units.at(
      physical_channels_registry.at(type).at(index).units_index);
}

double Device::getAnalogGain(DAQ::ChannelType::type_t type,
                             DAQ::index_t index) const
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return 1.0;
  }
  return physical_channels_registry.at(type).at(index).gain;
}

double Device::getAnalogZeroOffset(DAQ::ChannelType::type_t type,
                                   DAQ::index_t index) const
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return 1.0;
  }
  return physical_channels_registry.at(type).at(index).offset;
}

DAQ::index_t Device::getAnalogRange(DAQ::ChannelType::type_t type,
                                    DAQ::index_t index) const
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return 0;
  }
  return physical_channels_registry.at(type).at(index).range_index;
}

DAQ::index_t Device::getAnalogReference(DAQ::ChannelType::type_t type,
                                        DAQ::index_t index) const
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return 0;
  }
  return reference_to_index(physical_channels_registry.at(type).at(index).reference);
}

DAQ::index_t Device::getAnalogUnits(DAQ::ChannelType::type_t type,
                                    DAQ::index_t index) const
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return 0;
  }
  return physical_channels_registry.at(type).at(index).units_index;
}

DAQ::index_t Device::getAnalogOffsetUnits(DAQ::ChannelType::type_t type,
                                          DAQ::index_t index) const
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return 0;
  }
  return physical_channels_registry.at(type).at(index).units_index;
}

int Device::setAnalogGain(DAQ::ChannelType::type_t type,
                          DAQ::index_t index,
                          double gain)
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return -1;
  }
  // gain is handled by DAQ class and not nidaqmx
  physical_channels_registry.at(type).at(index).gain = gain;
  return 0;
}

int Device::setAnalogRange(DAQ::ChannelType::type_t type,
                           DAQ::index_t index,
                           DAQ::index_t range)
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return -1;
  }
  physical_channel_t& chan = physical_channels_registry.at(type).at(index);
  if(!chan.active) { 
    chan.range_index = range;
    return 0;
  }
  int32_t (*set_analog_max)(TaskHandle, const char*, float64) = nullptr;
  int32_t (*set_analog_min)(TaskHandle, const char*, float64) = nullptr;
  auto [min, max] = DAQ::get_default_ranges().at(range);
  TaskHandle task = task_list.at(type);
  const char* chan_name = chan.name.c_str();
  switch(type){
    case DAQ::ChannelType::AI:
      set_analog_max = DAQmxSetAIMax;
      set_analog_min = DAQmxSetAIMin;
      break;
    case DAQ::ChannelType::AO:
      set_analog_max = DAQmxSetAOMax;
      set_analog_min = DAQmxSetAOMin;
      break;
    default:
      return -1;
  }
  // We attempt to change the range 
  printError(DAQmxStopTask(task));
  const int32_t max_result = set_analog_max(task, chan_name, max); 
  printError(max_result);
  const int32_t min_result = set_analog_min(task, chan_name, min); 
  printError(min_result);
  printError(DAQmxStartTask(task));
  if(max_result != 0 || min_result != 0) { return -1; }
  chan.range_index = range;
  return 0;
}

int Device::setAnalogZeroOffset(DAQ::ChannelType::type_t type,
                                DAQ::index_t index,
                                double offset)
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return -1;
  }
  physical_channels_registry.at(type).at(index).offset = offset;
  return 0;
}

int Device::setAnalogReference(DAQ::ChannelType::type_t type,
                               DAQ::index_t index,
                               DAQ::index_t reference)
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return -1;
  }
  physical_channel_t& chan = physical_channels_registry.at(type).at(index);
  int32_t ref = index_to_reference(reference);
  if(!chan.active) { 
    chan.reference = ref;
    return 0;
  }
  int32_t (*set_analog_ref)(TaskHandle, const char*, int32_t) = nullptr;
  TaskHandle task = task_list.at(type);
  const char* chan_name = chan.name.c_str();
  switch(type){
    case DAQ::ChannelType::AI:
      set_analog_ref = DAQmxSetAITermCfg;
      break;
    case DAQ::ChannelType::AO:
      set_analog_ref = DAQmxSetAOTermCfg;
      break;
    default:
      return -1;
  }
  // We attempt to change the range 
  printError(DAQmxStopTask(task));
  const int32_t ref_result = set_analog_ref(task, chan_name, ref); 
  printError(ref_result);
  printError(DAQmxStartTask(task));
  if(ref_result != 0) { return -1; }
  chan.reference = ref;
  return 0;
}

int Device::setAnalogUnits(DAQ::ChannelType::type_t type,
                           DAQ::index_t index,
                           DAQ::index_t units)
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return -1;
  }
  // RTXI only supports voltage and current for now
  if (units > 1) { return -1; }
  physical_channel_t& chan = physical_channels_registry.at(type).at(index);
  if(!chan.active){
    chan.units_index = units;
    return 0;
  }
  // To set the units, we have to change shutdown and recreate the channel
  setChannelActive(type, index, /*state=*/false);
  physical_channels_registry.at(type).at(index).units_index = units;
  setChannelActive(type, index, /*state=*/true);
  return 0;
}

int Device::setAnalogOffsetUnits(DAQ::ChannelType::type_t /*type*/,
                                 DAQ::index_t /*index*/,
                                 DAQ::index_t /*units*/)
{
  return 0;
}

int Device::setAnalogDownsample(DAQ::ChannelType::type_t /*type*/,
                                DAQ::index_t /*index*/,
                                size_t /*downsample*/)
{
  return 0;
}

int Device::setAnalogCounter(DAQ::ChannelType::type_t /*type*/,
                             DAQ::index_t /*index*/)
{
  return 0;
}
int Device::setAnalogCalibrationValue(DAQ::ChannelType::type_t /*type*/,
                                      DAQ::index_t /*index*/,
                                      double /*value*/)
{
  return 0;
}
double Device::getAnalogCalibrationValue(DAQ::ChannelType::type_t /*type*/,
                                         DAQ::index_t /*index*/) const
{
  return 0.0;
}
int Device::setAnalogCalibrationActive(DAQ::ChannelType::type_t /*type*/,
                                       DAQ::index_t /*index*/,
                                       bool /*state*/)
{
  return 0;
}
bool Device::getAnalogCalibrationActive(DAQ::ChannelType::type_t /*type*/,
                                        DAQ::index_t /*index*/) const
{
  return false;
}
bool Device::getAnalogCalibrationState(DAQ::ChannelType::type_t /*type*/,
                                       DAQ::index_t /*index*/) const
{
  return false;
}

int Device::setDigitalDirection(DAQ::index_t /*index*/,
                                DAQ::direction_t /*direction*/)
{
  return 0;
}

void Device::read()
{
  int samples_read = 0;
  size_t value_index = 0;
  if (!this->active_channels.at(DAQ::ChannelType::AI).empty()) {
    DAQmxReadAnalogF64(
        task_list[DAQ::ChannelType::AI],
        DAQmx_Val_Auto,
        DAQmx_Val_WaitInfinitely,
        DAQmx_Val_GroupByScanNumber,
        std::get<DAQ::ChannelType::AI>(buffer_arrays).data(),
        static_cast<uint32_t>(
            std::get<DAQ::ChannelType::AI>(buffer_arrays).size()),
        &samples_read,
        nullptr);
    for (const auto& chan : this->active_channels.at(DAQ::ChannelType::AI)) {
      writeoutput(
          chan->id,
          std::get<DAQ::ChannelType::AI>(buffer_arrays)[value_index]*chan->gain + chan->offset);
      ++value_index;
    }
  }
  samples_read = 0;
  value_index = 0;
  int32_t num_bytes_per_sample = 0;
  if (!this->active_channels.at(DAQ::ChannelType::DI).empty()) {
    DAQmxReadDigitalLines(
        task_list[DAQ::ChannelType::DI],
        DAQmx_Val_Auto,
        DAQmx_Val_WaitInfinitely,
        DAQmx_Val_GroupByScanNumber,
        std::get<DAQ::ChannelType::DI>(buffer_arrays).data(),
        static_cast<uint32_t>(
            std::get<DAQ::ChannelType::DI>(buffer_arrays).size()),
        &samples_read,
        &num_bytes_per_sample,
        nullptr);
    for (const auto& chan : this->active_channels.at(DAQ::ChannelType::DI)) {
      writeoutput(
          chan->id,
          std::get<DAQ::ChannelType::DI>(buffer_arrays).at(value_index));
      ++value_index;
    }
  }
}

void Device::write()
{
  size_t samples_to_write = 0;
  int samples_written = 0;
  if (!this->active_channels.at(DAQ::ChannelType::AO).empty()) {
    for (const auto& chan : this->active_channels.at(DAQ::ChannelType::AO)) {
      std::get<DAQ::ChannelType::AO>(buffer_arrays).at(samples_to_write) =
          readinput(chan->id)*chan->gain + chan->offset;
      ++samples_to_write;
    }
    DAQmxWriteAnalogF64(task_list[DAQ::ChannelType::AO],
                        1,
                        0U,
                        DAQmx_Val_WaitInfinitely,
                        DAQmx_Val_GroupByScanNumber,
                        std::get<DAQ::ChannelType::AO>(buffer_arrays).data(),
                        &samples_written,
                        nullptr);
  }
  samples_to_write = 0;
  if (!this->active_channels.at(DAQ::ChannelType::DO).empty()) {
    bool digital_value_changed = false;
    uint8_t old_value = 0;
    uint8_t new_value = 0;
    for (const auto& chan : this->active_channels.at(DAQ::ChannelType::DO)) {
      old_value =
          std::get<DAQ::ChannelType::DO>(buffer_arrays).at(samples_to_write);
      new_value = static_cast<uint8_t>(readinput(chan->id));
      if (old_value != new_value) {
        digital_value_changed = true;
      }
      std::get<DAQ::ChannelType::DO>(buffer_arrays).at(samples_to_write) =
          new_value;
      ++samples_to_write;
    }
    if (digital_value_changed) {
      DAQmxWriteDigitalLines(
          task_list[DAQ::ChannelType::DO],
          1,
          0U,
          DAQmx_Val_WaitInfinitely,
          DAQmx_Val_GroupByScanNumber,
          std::get<DAQ::ChannelType::DO>(buffer_arrays).data(),
          &samples_written,
          nullptr);
    }
  }
}

Driver::Driver()
    : DAQ::Driver(std::string(DEFAULT_DRIVER_NAME))
{
  this->loadDevices();
}

void Driver::loadDevices()
{
  const int32_t device_names_buffer_size = DAQmxGetSysDevNames(nullptr, 0);
  if (device_names_buffer_size < 0) {
    printError(device_names_buffer_size);
    return;
  }
  const std::string alpha = "abcdefghijklmnopqrstuvwxyz";
  std::string string_buffer(static_cast<size_t>(device_names_buffer_size),
                            '\0');
  DAQmxGetSysDevNames(string_buffer.data(),
                      static_cast<uint32_t>(string_buffer.size()));
  const std::vector<std::string> device_names =
      split_string(string_buffer, ", ");
  std::vector<IO::channel_t> channels;
  std::vector<std::string> split_channel_names;
  size_t pos = 0;
  std::string physical_daq_name;
  std::string description;
  int channel_id = 0;
  for (const auto& internal_dev_name : device_names) {
    for (size_t query_indx = 0; query_indx < 4; query_indx++) {
      split_channel_names = physical_channel_names(
          internal_dev_name, static_cast<DAQ::ChannelType::type_t>(query_indx));
      physical_daq_name = physical_card_name(internal_dev_name);
      for (const auto& chan_name : split_channel_names) {
        description = std::string(query_indx < 2 ? "Analog" : "Digital");
        description += " ";
        description += std::string(query_indx % 2 == 0 ? "Output" : "Input");
        description += " ";
        pos = chan_name.find_last_of(alpha) + 1;
        channel_id = std::stoi(chan_name.substr(pos, chan_name.size() - pos));
        description += std::to_string(channel_id);
        channels.push_back({chan_name,
                            description,
                            query_indx % 2 == 0 ? IO::OUTPUT : IO::INPUT});
      }
    }
    this->nidaq_devices.emplace_back(
        physical_daq_name, channels, internal_dev_name);
  }
}

void Driver::unloadDevices() {}

std::vector<DAQ::Device*> Driver::getDevices()
{
  std::vector<DAQ::Device*> devices;
  devices.reserve(this->nidaq_devices.size());
  for (auto& device : nidaq_devices) {
    devices.push_back(&device);
  }
  return devices;
}

DAQ::Driver* Driver::getInstance()
{
  // if (instance == nullptr) {
  //   instance = new Driver();
  // }
  static Driver instance;
  return &instance;
}

}  // namespace

extern "C"
{
DAQ::Driver* getRTXIDAQDriver()
{
  return Driver::getInstance();
}

void deleteRTXIDAQDriver() {}
}
