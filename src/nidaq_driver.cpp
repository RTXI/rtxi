
extern "C"
{
#include <NIDAQmx.h>
}

#include <utility>

#include <fmt/core.h>

#include "daq.hpp"

const std::string_view DEFAULT_DRIVER_NAME = "National Instruments";

std::vector<std::string> split_string(const std::string& buffer,
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
  size_t delim_len = delim.size();
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

std::vector<std::string> physical_channel_names(
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
      func = &DAQmxGetDevDOPorts;
      break;
    case DAQ::ChannelType::DI:
      func = &DAQmxGetDevDIPorts;
      break;
    default:
      return {};
  }
  int32_t buff_size = 0;
  std::vector<IO::channel_t> channels;
  std::string channel_names;
  std::vector<std::string> split_channel_names;
  std::string temp_channel_name;
  std::string description;
  buff_size = func(device.c_str(), nullptr, 0);
  if (buff_size <= 0) {
    return {};
  }
  channel_names.resize(static_cast<size_t>(buff_size));
  func(device.c_str(), channel_names.data(), static_cast<uint32_t>(buff_size));
  return split_string(channel_names, ", ");
}

void printError(int32_t status)
{
  if (status == 0) {
    return;
  }
  ERROR_MSG("NIDAQ ERROR : code {}", status);
  int32_t error_size = DAQmxGetErrorString(status, nullptr, 0);
  if (error_size < 0) {
    ERROR_MSG("Unable to get code message");
    return;
  }
  std::string err_str(static_cast<size_t>(error_size), '\0');
  int32_t errcode = DAQmxGetErrorString(
      status, err_str.data(), static_cast<uint32_t>(error_size));
  if (errcode < 0) {
    ERROR_MSG("Unable to parse message");
    return;
  }
  ERROR_MSG("Message : {}", err_str);
}

std::string physical_card_name(const std::string& device_name)
{
  int32_t err = DAQmxGetDevProductType(device_name.c_str(), nullptr, 0);
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
  std::string units = DAQ::get_default_units().at(units_index);
  auto [min, max] = DAQ::get_default_ranges().at(range_index);
  // DAQmxStopTask(task_handle);
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
          task_handle, name.c_str(), nullptr, DAQmx_Val_ChanForAllLines);
      printError(err);
      break;
    case DAQ::ChannelType::DO:
      err = DAQmxCreateDOChan(
          task_handle, name.c_str(), nullptr, DAQmx_Val_ChanForAllLines);
      printError(err);
      break;
    default:
      ERROR_MSG("NIDAQ_DRIVER : Channel Type Unknown");
      break;
  }
  // DAQmxTaskControl(task_handle, DAQmx_Val_Task_Commit);
  // DAQmxStartTask(task_handle);
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
  std::array<std::vector<double>, DAQ::ChannelType::UNKNOWN> buffer_arrays;
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
  for (size_t i = 0; i < buffer_arrays.size(); i++) {
    buffer_arrays.at(i).assign(
        getChannelCount(static_cast<DAQ::ChannelType::type_t>(i)), 0);
  }

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
  // channel. Adding is easy, but removing means clearing the task, then
  // starting the task again, and adding all of the other channels.
  int32_t err = 0;
  if (physical_channels_registry.at(type).at(index).active == state) {
    return err;
  }
  DAQmxStopTask(task_list.at(type));
  if (state) {
    err = physical_channels_registry.at(type).at(index).addToTask(
        task_list.at(type));
    physical_channels_registry.at(type).at(index).active = err == 0;
  } else {
    physical_channels_registry.at(type).at(index).active = false;
    DAQmxClearTask(task_list.at(type));
    err = DAQmxCreateTask(DAQ::ChannelType::type2string(type).c_str(),
                          &task_list.at(type));
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
      err = channel.addToTask(task_list.at(type));
      if (err != 0) {
        break;
      }
    }
  }
  // active channels is an optimization for faster reading/writing in realtime.
  active_channels.at(type).clear();
  for (auto& channel : physical_channels_registry.at(type)) {
    if (channel.active) {
      active_channels.at(type).push_back(&channel);
    }
  }
  printError(DAQmxSetSampTimingType(task_list.at(type), DAQmx_Val_OnDemand));
  printError(DAQmxTaskControl(task_list.at(type), DAQmx_Val_Task_Commit));
  printError(DAQmxStartTask(task_list.at(type)));
  printError(err);
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
  std::string formatting = "{:.1f}";
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
  int32_t ref = physical_channels_registry.at(type).at(index).reference;
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
  return static_cast<size_t>(
      physical_channels_registry.at(type).at(index).reference);
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
  physical_channels_registry.at(type).at(index).range_index = range;
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
  physical_channels_registry.at(type).at(index).range_index = reference;
  return 0;
}

int Device::setAnalogUnits(DAQ::ChannelType::type_t type,
                           DAQ::index_t index,
                           DAQ::index_t units)
{
  if (type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO) {
    return -1;
  }
  physical_channels_registry.at(type).at(index).units_index = units;
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
  if (this->active_channels.at(DAQ::ChannelType::AI).empty()) {
    return;
  }
  DAQmxReadAnalogF64(task_list[DAQ::ChannelType::AI],
                     DAQmx_Val_Auto,
                     DAQmx_Val_WaitInfinitely,
                     DAQmx_Val_GroupByScanNumber,
                     buffer_arrays.at(DAQ::ChannelType::AI).data(),
                     buffer_arrays.at(DAQ::ChannelType::AI).size(),
                     &samples_read,
                     nullptr);
  size_t value_index = 0;
  for (auto* chan : this->active_channels.at(DAQ::ChannelType::AI)) {
    writeoutput(chan->id,
                buffer_arrays.at(DAQ::ChannelType::AI).at(value_index));
    ++value_index;
  }
}

void Device::write()
{
  if (this->active_channels.at(DAQ::ChannelType::AO).empty()) {
    return;
  }
  size_t samples_to_write = 0;
  int samples_written = 0;
  for (auto* chan : this->active_channels.at(DAQ::ChannelType::AO)) {
    buffer_arrays.at(DAQ::ChannelType::AO).at(samples_to_write) =
        readinput(chan->id);
    ++samples_to_write;
  }
  DAQmxWriteAnalogF64(task_list[DAQ::ChannelType::AO],
                      1,
                      0U,
                      DAQmx_Val_WaitInfinitely,
                      DAQmx_Val_GroupByScanNumber,
                      buffer_arrays.at(DAQ::ChannelType::AO).data(),
                      &samples_written,
                      nullptr);
}

Driver::Driver()
    : DAQ::Driver(std::string(DEFAULT_DRIVER_NAME))
{
  this->loadDevices();
}

void Driver::loadDevices()
{
  int32_t device_names_buffer_size = DAQmxGetSysDevNames(nullptr, 0);
  if (device_names_buffer_size < 0) {
    printError(device_names_buffer_size);
    return;
  }
  std::string string_buffer(static_cast<size_t>(device_names_buffer_size),
                            '\0');
  DAQmxGetSysDevNames(string_buffer.data(),
                      static_cast<uint32_t>(string_buffer.size()));
  std::vector<std::string> device_names = split_string(string_buffer, ", ");
  std::vector<IO::channel_t> channels;
  std::vector<std::string> split_channel_names;
  size_t pos = 0;
  std::string temp_channel_name;
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
        pos = chan_name.find_first_of("0123456789");
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

namespace
{
Driver* instance = nullptr;
}  // namespace

DAQ::Driver* Driver::getInstance()
{
  if (instance == nullptr) {
    instance = new Driver();
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
