
// General Standards Corporation does not define a standard location to install
// the library, so make sure the headers are saved in /usr/include/GSC and this
// should work
#include <algorithm>
#include <filesystem>
#include <fstream>

#include <GSC/16aio168_main.h>
#include <fmt/core.h>

#include "daq.hpp"

constexpr std::string_view DEFAULT_DRIVER_NAME = "General Standards";

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

inline void printError(int errcode)
{
  constexpr int ARRAY_SIZE = 1024;
  std::array<char, ARRAY_SIZE> BUFFER {};
  strerror_r(errcode, BUFFER.data(), ARRAY_SIZE);
  ERROR_MSG("16aio168 driver error with code {} : {}", errcode, BUFFER);
}

// Define the struct that represents a physical channel, which is different than
// the block channel used by RTXI
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
  std::string name;
  DAQ::ChannelType::type_t type = DAQ::ChannelType::UNKNOWN;
  size_t id;
  DAQ::Reference::reference_t reference = DAQ::Reference::GROUND;
  double offset = 0.0;
  double gain = 1.0;
  size_t range_index = 0;
  size_t units_index = 0;
  bool active = false;
};

inline int32_t index_to_reference(DAQ::index_t index)
{
  int32_t result = 0;
  switch (static_cast<DAQ::Reference::reference_t>(index)) {
    case DAQ::Reference::GROUND:
    case DAQ::Reference::COMMON:
      result = AIO168_AI_MODE_SINGLE;
      break;
    case DAQ::Reference::DIFFERENTIAL:
      result = AIO168_AI_MODE_DIFF;
      break;
    default:
      result = AIO168_AI_MODE_SINGLE;
      break;
  }
  return result;
}

inline DAQ::Reference::reference_t reference_to_index(int32_t aio168_reference)
{
  DAQ::Reference::reference_t result = DAQ::Reference::UNKNOWN;
  switch (aio168_reference) {
    case AIO168_AI_MODE_SINGLE:
      result = DAQ::Reference::GROUND;
      break;
    case AIO168_AI_MODE_DIFF:
      result = DAQ::Reference::DIFFERENTIAL;
      break;
    default:
      result = DAQ::Reference::OTHER;
      break;
  }
  return result;
}

inline DAQ::index_t range_to_index(int32_t aio168_range_id)
{
  DAQ::analog_range_t range;
  const auto default_ranges = DAQ::get_default_ranges();
  switch (aio168_range_id) {
    case AIO168_RANGE_2_5V:
      range = DAQ::analog_range_t(-2.5, 2.5);
      break;
    case AIO168_RANGE_5V:
      range = DAQ::analog_range_t(-5.0, 5.0);
      break;
    case AIO168_RANGE_10V:
    default:
      range = DAQ::analog_range_t(-10.0, 10.0);
      break;
  }
  const auto* iter =
      std::find(default_ranges.begin(), default_ranges.end(), range);
  return iter == default_ranges.end() ? 0 : iter - default_ranges.begin();
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
         int device_file_descriptor);
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
  int fd;
  std::array<std::vector<physical_channel_t>, 4> physical_channels_registry;
  // Used a tuple here because we want buffers to be in one place, and since not all
  // IO is flaot, that meant we could not use a plain vector of vectors. Maybe we can
  // just split them out to their own variables of same type for speed.
  std::tuple<std::vector<double>,
             std::vector<double>,
             std::vector<uint8_t>,
             std::vector<uint8_t>>
      buffer_arrays;
  // Just trying to keep track of active channels for efficiency
  std::array<std::vector<int>, DAQ::ChannelType::UNKNOWN>
      active_channels;

  int CURRENT_SCAN_SIZE=1;
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
  std::string version;
  bool support_32_bit;
  size_t total_board_count;
  std::vector<std::string> installed_boards;
  std::vector<Device> m_devices;
};

Device::Device(const std::string& dev_name,
               const std::vector<IO::channel_t>& channels,
               int device_file_descriptor)
    : DAQ::Device(dev_name, channels)
    , fd(device_file_descriptor)
{
}

Device::~Device()
{
  aio168_close(this->fd);
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
  physical_channels_registry.at(type).at(index).active = state;
  active_channels.at(type).push_back(static_cast<int>(index));
  if(type != DAQ::ChannelType::AI) { return 0; }
  const int32_t max_input_port = *std::max(active_channels.at(type).begin(),
                                           active_channels.at(type).end());
  int32_t scan_size = -1;
  if(max_input_port <= 2){
    scan_size = AIO168_AI_SCAN_SIZE_0_1;
    CURRENT_SCAN_SIZE = 2;
  } else if(max_input_port <= 4) {
    scan_size = AIO168_AI_SCAN_SIZE_0_3;
    CURRENT_SCAN_SIZE = 4;
  } else if(max_input_port <= 8) {
    scan_size = AIO168_AI_SCAN_SIZE_0_7;
    CURRENT_SCAN_SIZE = 8;
  } else {
    scan_size = AIO168_AI_SCAN_SIZE_0_15;
    CURRENT_SCAN_SIZE = 16;
  }
  int result = aio168_ioctl(fd, AIO168_IOCTL_AI_SCAN_SIZE, &scan_size);
  if(result < 0){
    printError(result);
  }
  return result;
}

size_t Device::getAnalogRangeCount(DAQ::index_t /*index*/) const 
{
  // this device only supports 2.5, 5, and 10 ranges
  return 3;
}

size_t Device::getAnalogReferenceCount(DAQ::index_t /*index*/) const 
{
  // this device only supports single ended and differential
  return 2;
}

size_t Device::getAnalogUnitsCount(DAQ::index_t /*index*/) const 
{
  // this device only supports voltage
  return 1;
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
  auto [min, max] = DAQ::get_default_ranges().at(
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
    case AIO168_AI_MODE_SINGLE:
      refstr = "Ground";
      break;
    case AIO168_AI_MODE_DIFF:
      refstr = "Differential";
      break;
    default:
      refstr = "Other";
      break;
  }
  return refstr;

}

std::string Device::getAnalogUnitsString(DAQ::ChannelType::type_t type,
                                         DAQ::index_t index,
                                         DAQ::index_t /*units*/) const
{
  return "volts";
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
  return reference_to_index(
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
  if (!chan.active) {
    chan.range_index = range;
    return 0;
  }
  switch (range) {
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
  if (max_result != 0 || min_result != 0) {
    return -1;
  }
  chan.range_index = range;
  return 0;
}

int Device::setAnalogZeroOffset(DAQ::ChannelType::type_t type,
                                DAQ::index_t index,
                                double offset)
{
}

int Device::setAnalogReference(DAQ::ChannelType::type_t type,
                               DAQ::index_t index,
                               DAQ::index_t reference)
{
}

int Device::setAnalogUnits(DAQ::ChannelType::type_t type,
                           DAQ::index_t index,
                           DAQ::index_t units)
{
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

void Device::read() {}

void Device::write() {}

Driver::Driver()
    : DAQ::Driver(std::string(DEFAULT_DRIVER_NAME))
{
  int result = aio168_init();
  if (result < 0) {
    printError(result);
    throw std::runtime_error(
        "16aio168 ERROR: Could not initialize 16aio168 drivers");
  }
  const std::string device_info_file = "/proc/16aio168";
  if (!std::filesystem::exists(device_info_file)) {
    printError(result);
    throw std::runtime_error(fmt::format(
        "DAQ::16aio168 : {} does not exist! Cannot discover driver information",
        device_info_file));
  }
  // Parse Driver Information
  std::ifstream file_stream(device_info_file);
  std::string current_line;
  std::vector<std::string> tokens;
  std::getline(file_stream, current_line);
  std::vector<std::string> parsed_line = split_string(current_line, ": ");
  this->version = parsed_line[1];
  std::getline(file_stream, current_line);
  parsed_line = split_string(current_line, ": ");
  this->support_32_bit = parsed_line[1] == "yes";
  std::getline(file_stream, current_line);
  parsed_line = split_string(current_line, ": ");
  this->total_board_count = static_cast<size_t>(std::stoi(parsed_line[1]));
  std::getline(file_stream, current_line);
  parsed_line = split_string(current_line, ": ");
  this->installed_boards = split_string(parsed_line[1], ",");
  this->loadDevices();
}

void Driver::loadDevices()
{
  // We query information from drivers and store into device during
  // initialization
  int result = 0;
  int fd = 0;
  int channel_count = 0;
  std::vector<IO::channel_t> channels;
  for (size_t device_id = 0; device_id < total_board_count; device_id++) {
    result = aio168_open(static_cast<int>(device_id), 0, &fd);
    if (result < 0) {
      printError(result);
      ERROR_MSG("AIO168 DRIVER : Unable to open device {}. Skipping...",
                installed_boards[device_id]);
      continue;
    }
    result = aio168_ioctl(fd, AIO168_QUERY_CHAN_AI_MAX, &channel_count);
    for (int channel_id = 0; channel_id < channel_count; channel_id++) {
      channels.emplace_back(fmt::format("AI {}", channel_id),
                            fmt::format("Analog Input {}", channel_id),
                            IO::OUTPUT);
    }
    result = aio168_ioctl(fd, AIO168_QUERY_CHAN_AO_MAX, &channel_count);
    for (int channel_id = 0; channel_id < channel_count; channel_id++) {
      channels.emplace_back(fmt::format("AO {}", channel_id),
                            fmt::format("Analog Output {}", channel_id),
                            IO::INPUT);
    }
    m_devices.emplace_back(
        fmt::format("{}-{}", installed_boards[device_id], device_id),
        channels,
        fd);
    channels.clear();
  }
}

void Driver::unloadDevices() {}

std::vector<DAQ::Device*> Driver::getDevices()
{
  std::vector<DAQ::Device*> devices;
  devices.reserve(this->m_devices.size());
  for (auto& device : m_devices) {
    devices.push_back(&device);
  }
  return devices;
}

DAQ::Driver* Driver::getInstance()
{
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
