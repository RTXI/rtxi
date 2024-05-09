
// General Standards Corporation does not define a standard location to install
// the library, so make sure the headers are saved in /usr/include/GSC and this
// should work
#include <algorithm>
#include <filesystem>
#include <fstream>

#include <GSC/16aio168.h>
#include <GSC/16aio168_main.h>
#include <fmt/core.h>

#include "daq.hpp"

constexpr std::string_view DEFAULT_DRIVER_NAME = "General Standards";

namespace
{

constexpr int MAX_DIGITAL_LANES =
    4;  // 16aio168 devices only have 4 digital lanes
constexpr int BYTE_RESOLUTION = 65535;  // 2 ^ 16 bit symbols

inline constexpr double binary_to_voltage(DAQ::analog_range_t volt_range,
                                          int32_t value)
{
  const double width = (volt_range.second - volt_range.first) / BYTE_RESOLUTION;
  const double offset = (volt_range.second - volt_range.first) / 2.0;
  return static_cast<double>(value & BYTE_RESOLUTION) * width - offset;
}

inline constexpr int32_t voltage_to_binary(DAQ::analog_range_t volt_range,
                                           double value)
{
  const double width = (volt_range.second - volt_range.first) / BYTE_RESOLUTION;
  const double offset = (volt_range.second - volt_range.first) / 2.0;
  return static_cast<int32_t>((value + offset) / width);
}

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
  std::string BUFFER(ARRAY_SIZE, '\0');
  strerror_r(errcode, BUFFER.data(), ARRAY_SIZE);
  ERROR_MSG("AIO168 driver error with code {} : {}", errcode, BUFFER);
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

inline int32_t daqref_to_gslref(DAQ::Reference::reference_t index)
{
  int32_t result = 0;
  switch (index) {
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

inline DAQ::Reference::reference_t gslref_to_daqref(int32_t aio168_reference)
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

inline int32_t index_to_range(DAQ::index_t index)
{
  int32_t range = 0;
  switch (index) {
    case 2:
      range = AIO168_RANGE_2_5V;
      break;
    case 1:
      range = AIO168_RANGE_5V;
      break;
    case 0:
    default:
      range = AIO168_RANGE_10V;
      break;
  }
  return range;
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
  std::vector<int32_t> ai_channels_buffer;
  std::vector<int32_t> ao_channels_buffer;
  std::vector<int8_t> di_channels_buffer;
  std::vector<int8_t> do_channels_buffer;
  std::array<DAQ::analog_range_t, 7> default_ranges;

  // Just trying to keep track of active channels for efficiency
  std::array<std::vector<int>, DAQ::ChannelType::UNKNOWN> active_channels;

  size_t CURRENT_SCAN_SIZE = 0;
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
  int32_t max_ai_count = AIO168_QUERY_CHAN_AI_MAX;
  int32_t max_ao_count = AIO168_QUERY_CHAN_AO_MAX;
  int result = aio168_ioctl(fd, AIO168_IOCTL_QUERY, &max_ai_count);
  if (result < 0) {
    printError(result);
    throw std::runtime_error(
        "AIO168 DRIVER : Unable to obtain max AI channel count");
  }
  result = aio168_ioctl(fd, AIO168_IOCTL_QUERY, &max_ao_count);
  if (result < 0) {
    printError(result);
    throw std::runtime_error(
        "AIO168 DRIVER : Unable to obtain max AO channel count");
  }
  std::string chan_name;
  for (int ai_count = 0; ai_count < max_ai_count; ai_count++) {
    physical_channels_registry.at(DAQ::ChannelType::AI)
        .emplace_back(
            fmt::format("AI{}", ai_count), DAQ::ChannelType::AI, ai_count);
  }
  for (int ao_count = 0; ao_count < max_ao_count; ao_count++) {
    physical_channels_registry.at(DAQ::ChannelType::AO)
        .emplace_back(
            fmt::format("AO{}", ao_count), DAQ::ChannelType::AO, ao_count);
  }
  for (int digital_lane = 0; digital_lane < MAX_DIGITAL_LANES; digital_lane++) {
    physical_channels_registry.at(DAQ::ChannelType::DI)
        .emplace_back(fmt::format("DI{}", digital_lane),
                      DAQ::ChannelType::DI,
                      digital_lane);
    physical_channels_registry.at(DAQ::ChannelType::DO)
        .emplace_back(fmt::format("DO{}", digital_lane),
                      DAQ::ChannelType::DO,
                      digital_lane);
  }

  // We need to tell the board that channel scanning is software timed.
  // Check 16aio168 manual for explenation of this setting
  int32_t scan_setting = AIO168_AI_SCAN_CLK_SRC_BCR;
  result = aio168_ioctl(fd, AIO168_IOCTL_AI_SCAN_CLK_SRC, &scan_setting);
  if (result < 0) {
    ERROR_MSG(
        "16QIO168 DRIVER : Unable to set the channel scan to software trigger "
        "for device");
    printError(result);
  }

  //int32_t sync_setting = AIO168_AO_BURST_CLK_SRC_BCR;
  //result = aio168_ioctl(fd, AIO168_IOCTL_AO_BURST_CLK_SRC, &sync_setting);
  //if (result < 0) {
  //  ERROR_MSG(
  //      "16QIO168 DRIVER : Unable to set the channel sync to software trigger "
  //      "for device");
  //  printError(result);
  //}
  //int32_t timing = AIO168_AO_TIMING_SIMUL;
  //result = aio168_ioctl(fd, AIO168_IOCTL_AO_TIMING, &timing);
  //if (result < 0) {
  //  ERROR_MSG(
  //      "16QIO168 DRIVER : Unable to set the channel sync to simultanous output "
  //      "for device");
  //  printError(result);
  //}

  ai_channels_buffer.assign(getChannelCount(DAQ::ChannelType::AI), 0);
  ao_channels_buffer.assign(getChannelCount(DAQ::ChannelType::AO), 0);
  di_channels_buffer.assign(getChannelCount(DAQ::ChannelType::DI), 0);
  do_channels_buffer.assign(getChannelCount(DAQ::ChannelType::DO), 0);
  default_ranges = DAQ::get_default_ranges();

  // Calibrate the device
  // result = aio168_ioctl(fd, AIO168_IOCTL_AUTOCAL, nullptr);

  // Don't let the device sleep when reading and just return immediately
  int32_t timeout = 0;
  result = aio168_ioctl(fd, AIO168_IOCTL_RX_IO_TIMEOUT, &timeout);
  //timeout = 0;
  //result = aio168_ioctl(fd, AIO168_IOCTL_TX_IO_TIMEOUT, &timeout);
  this->setActive(/*act=*/true);
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
  auto iter = std::find(active_channels.at(type).begin(),
                        active_channels.at(type).end(),
                        static_cast<int>(index));
  physical_channels_registry.at(type).at(index).active = state;
  if(state){
    if(iter == active_channels.at(type).end()) { active_channels.at(type).push_back(static_cast<int>(index)); }
  } else {
    if(iter != active_channels.at(type).end()) { active_channels.at(type).erase(iter); }
  }

  if (type != DAQ::ChannelType::AI) {
    return 0;
  }
  if(active_channels.at(type).empty()) { 
    CURRENT_SCAN_SIZE = 0;
    return 0;
  }
  const int32_t max_input_port = *std::max(active_channels.at(type).begin(),
                                           active_channels.at(type).end());
  int32_t scan_size = -1;
  if (max_input_port <= 2) {
    scan_size = AIO168_AI_SCAN_SIZE_0_1;
    CURRENT_SCAN_SIZE = 2;
  } else if (max_input_port <= 4) {
    scan_size = AIO168_AI_SCAN_SIZE_0_3;
    CURRENT_SCAN_SIZE = 4;
  } else if (max_input_port <= 8) {
    scan_size = AIO168_AI_SCAN_SIZE_0_7;
    CURRENT_SCAN_SIZE = 8;
  } else {
    scan_size = AIO168_AI_SCAN_SIZE_0_15;
    CURRENT_SCAN_SIZE = 16;
  }
  int result = aio168_ioctl(fd, AIO168_IOCTL_AI_SCAN_SIZE, &scan_size);
  if (result < 0) {
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
  return gslref_to_daqref(
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
  int gsc_range_value = index_to_range(range);
  int result = aio168_ioctl(fd, AIO168_IOCTL_RANGE, &gsc_range_value);
  if (result < 0) {
    ERROR_MSG("AIO168 ERROR : Unable to set range for device {}", getName());
    printError(result);
    int32_t retrieve = -1;
    result = aio168_ioctl(fd, AIO168_IOCTL_RANGE, &retrieve);
    chan.range_index = range_to_index(retrieve);
  }
  chan.range_index = range;
  return 0;
}

int Device::setAnalogZeroOffset(DAQ::ChannelType::type_t type,
                                DAQ::index_t index,
                                double offset)
{
  physical_channel_t& chan = physical_channels_registry.at(type).at(index);
  chan.offset = offset;
  return 0;
}

int Device::setAnalogReference(DAQ::ChannelType::type_t type,
                               DAQ::index_t index,
                               DAQ::index_t reference)
{
  physical_channel_t& chan = physical_channels_registry.at(type).at(index);
  chan.reference = static_cast<DAQ::Reference::reference_t>(reference);
  return 0;
}

int Device::setAnalogUnits(DAQ::ChannelType::type_t /*type*/,
                           DAQ::index_t /*index*/,
                           DAQ::index_t /*units*/)
{
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
  // initiate scan
  aio168_ioctl(fd, AIO168_IOCTL_AI_SYNC, nullptr);
  DAQ::analog_range_t range {};
  double gain = 1.0;
  double offset = 0.0;
  aio168_read(
      fd, ai_channels_buffer.data(), CURRENT_SCAN_SIZE * sizeof(int32_t));
  // we have to convert some stuff to float first before we continue
  for (size_t chan_id = 0; chan_id < CURRENT_SCAN_SIZE; chan_id++) {
    auto& chan_info = physical_channels_registry[DAQ::ChannelType::AI][chan_id];
    if(!chan_info.active) { continue; }
    range = default_ranges[chan_info.range_index];
    gain = chan_info.gain;
    offset = chan_info.offset;
    writeoutput(
        chan_id,
        binary_to_voltage(range, ai_channels_buffer[chan_id]) * gain + offset);
  }
}

void Device::write()
{
  DAQ::analog_range_t range {};
  double gain = 1.0;
  double offset = 0.0;
  double value = 0.0;
  for (size_t chan_id = 0; chan_id < ao_channels_buffer.size(); chan_id++) {
    // we have to saturate value before pushing to output
    auto& chan_info = physical_channels_registry[DAQ::ChannelType::AO][chan_id];
    range = default_ranges[chan_info.range_index];
    gain = chan_info.gain;
    offset = chan_info.offset;
    value = std::min(std::max(readinput(chan_id) * gain + offset, range.first), range.second);
    ao_channels_buffer[chan_id] = voltage_to_binary(range, value) | chan_id << 16;
  }
  ao_channels_buffer.back() |= 1 << 19;
  aio168_write(
      fd, ao_channels_buffer.data(), ao_channels_buffer.size() * sizeof(int32_t));
  // initate output sync
  // aio168_ioctl(fd, AIO168_IOCTL_AO_SYNC, nullptr);
}

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
        "DAQ::16aio168 : {} does not exist! Cannot discover driver information \n{}",
        device_info_file,
	"Make sure that the 16aio168 module is loaded before continuing. You can do this by runnign ./script in the driver directory."));
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
    channel_count = AIO168_QUERY_CHAN_AI_MAX;
    result = aio168_ioctl(fd, AIO168_IOCTL_QUERY, &channel_count);
    for (int channel_id = 0; channel_id < channel_count; channel_id++) {
      channels.push_back({fmt::format("AI {}", channel_id),
                          fmt::format("Analog Input {}", channel_id),
                          IO::OUTPUT});
    }
    channel_count = AIO168_QUERY_CHAN_AO_MAX;
    result = aio168_ioctl(fd, AIO168_IOCTL_QUERY, &channel_count);
    for (int channel_id = 0; channel_id < channel_count; channel_id++) {
      channels.push_back({fmt::format("AO {}", channel_id),
                          fmt::format("Analog Output {}", channel_id),
                          IO::INPUT});
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
