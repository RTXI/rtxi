
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
  int total_board_count;
  std::vector<std::string> installed_boards;
};

Device::Device(const std::string& dev_name,
               const std::vector<IO::channel_t>& channels,
               int device_file_descriptor)
    : DAQ::Device(dev_name, channels)
{
  if(status < 0) { throw std::system_error("16AIO168 : could not open device"); }
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
}

size_t Device::getAnalogRangeCount(DAQ::index_t /*index*/) const
{
}

size_t Device::getAnalogReferenceCount(DAQ::index_t /*index*/) const
{
}

size_t Device::getAnalogUnitsCount(DAQ::index_t /*index*/) const
{
}

size_t Device::getAnalogDownsample(DAQ::ChannelType::type_t /*type*/,
                                   DAQ::index_t /*index*/) const
{
}

std::string Device::getAnalogRangeString(DAQ::ChannelType::type_t type,
                                         DAQ::index_t index,
                                         DAQ::index_t /*range*/) const
{

}

std::string Device::getAnalogReferenceString(DAQ::ChannelType::type_t type,
                                             DAQ::index_t index,
                                             DAQ::index_t /*reference*/) const
{
}

std::string Device::getAnalogUnitsString(DAQ::ChannelType::type_t type,
                                         DAQ::index_t index,
                                         DAQ::index_t /*units*/) const
{

}

double Device::getAnalogGain(DAQ::ChannelType::type_t type,
                             DAQ::index_t index) const
{
}

double Device::getAnalogZeroOffset(DAQ::ChannelType::type_t type,
                                   DAQ::index_t index) const
{
}

DAQ::index_t Device::getAnalogRange(DAQ::ChannelType::type_t type,
                                    DAQ::index_t index) const
{
}

DAQ::index_t Device::getAnalogReference(DAQ::ChannelType::type_t type,
                                        DAQ::index_t index) const
{
}

DAQ::index_t Device::getAnalogUnits(DAQ::ChannelType::type_t type,
                                    DAQ::index_t index) const
{
}

DAQ::index_t Device::getAnalogOffsetUnits(DAQ::ChannelType::type_t type,
                                          DAQ::index_t index) const
{
}

int Device::setAnalogGain(DAQ::ChannelType::type_t type,
                          DAQ::index_t index,
                          double gain)
{
}

int Device::setAnalogRange(DAQ::ChannelType::type_t type,
                           DAQ::index_t index,
                           DAQ::index_t range)
{
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

void Device::read()
{

}

void Device::write()
{

}

Driver::Driver()
    : DAQ::Driver(std::string(DEFAULT_DRIVER_NAME))
{
  int result = aio168_init();
  if(result < 0) { throw std::system_error(); }
  this->loadDevices();
}

void Driver::loadDevices()
{
  constexpr std::string_view device_info_file = "/proc/16aio168";
  if(!std::filesystem::exists(device_info_file)){ 
    ERROR_MSG("DAQ::16aio168 : {} does not exist! Cannot discover available devices", device_info_file);
    return; 
  }
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
  this->total_board_count = std::stoi(parsed_line[1]);
  std::getline(file_stream, current_line);
  parsed_line = split_string(current_line, ": ");
  this->installed_boards = split_string(parsed_line, ",");

  // We query information from drivers and store into device during initialization
  int result = 0;
  int fd = 0;
  int channel_count = 0;
  std::vector<IO::channel_t> channels;
  for(int device_id = 0; device_id<total_board_count; device_id++){
    result = aio168_open(device, 0, &fd);
    if(result < 0) { 
      ERROR_MSG("AIO168 DRIVER : Unable to open device {}. Skipping...", 
                installed_boards[device_id]);
      continue;
    }
    result = aio168_ioctl(fd, AIO168_QUERY_CHAN_AI_MAX, &channel_count);
    for(int channel_id = 0; channel_id < channel_count; channel_id++){
      channels.emplace_back({fmt::format("AI {}", channel_id),
                             fmt::format("Analog Input {}", channel_id),
                             IO::OUTPUT});
    }
    result = aio168_ioctl(fd, AIO168_QUERY_CHAN_AO_MAX, &channel_count);
    for(int channel_id = 0; channel_id < channel_count; channel_id++){
      channels.emplace_back({fmt::format("AO {}", channel_id),
                             fmt::format("Analog Output {}", channel_id),
                             IO::INPUT});
    }
  }
}

void Driver::unloadDevices() {}

std::vector<DAQ::Device*> Driver::getDevices()
{
  std::ofstream file_stream("/proc/16aio168");
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
