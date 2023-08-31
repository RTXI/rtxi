#include <NIDAQmx.h>

#include <utility>
#include <fmt/core.h>
#include "daq.hpp"

const std::string_view DEFAULT_DRIVER_NAME = "National Instruments";

inline std::array<std::pair<double, double>, 7> get_default_ranges()
{
  return 
  {
    std::pair(-10.0, 10.0),
    std::pair(-5.0, 5.0),
    std::pair(-1.0, 1.0),
    std::pair(-0.5, 0.5),
    std::pair(-0.2, 0.2),
    std::pair(-0.1, 0.1),
  };
}

inline std::array<std::string, 2> get_default_units()
{
  return 
  {
    "volts",
    "amps"
  }; 
}

inline std::vector<std::string> split_string(const std::string& buffer, const std::string& delim)
{
  if(buffer.empty()) { return {}; }
  size_t pos_start=0;
  size_t pos_end=buffer.find(delim, pos_start);
  if(pos_end == std::string::npos) { return {buffer}; }
  std::string token;
  std::vector<std::string> split_tokens;
  size_t delim_len = delim.size();
  while (true) {
    token = buffer.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    split_tokens.push_back(token);
    pos_end = buffer.find(delim, pos_start);
    if(pos_end == std::string::npos) { break; }
  } 
  token = buffer.substr(pos_start, buffer.size()-pos_start);
  split_tokens.push_back(token);
  return split_tokens;
}

inline std::vector<std::string> physical_channel_names(const std::string& device, DAQ::ChannelType::type_t chan_type)
{
  int32_t (*func)(const char*, char*, uint32_t)=nullptr;
  switch(chan_type){
    case DAQ::ChannelType::AO :
      func = &DAQmxGetDevAOPhysicalChans;
      break;
    case DAQ::ChannelType::AI :
      func = &DAQmxGetDevAIPhysicalChans;
      break;
    case DAQ::ChannelType::DO :
      func = &DAQmxGetDevDOPorts;
      break;
    case DAQ::ChannelType::DI :
      func = &DAQmxGetDevDIPorts;
      break;
    default:
      return {};
  }  
  int32_t buff_size=0;
  std::vector<IO::channel_t> channels;
  std::string channel_names;
  std::vector<std::string> split_channel_names;
  std::string temp_channel_name;
  std::string description;
  buff_size = func(device.c_str(), nullptr, 0);
  if(buff_size <= 0){ return {}; }
  channel_names.resize(static_cast<size_t>(buff_size));
  func(device.c_str(), 
       channel_names.data(), 
       static_cast<uint32_t>(buff_size));
  return split_string(channel_names, ", "); 
}

inline std::string physical_card_name(const std::string& device_name)
{
  int32_t err = DAQmxGetDevProductType(device_name.c_str(), nullptr, 0);
  std::string result(static_cast<size_t>(err), '\0');
  err = DAQmxGetDevProductType(device_name.c_str(), 
                               result.data(), 
                               static_cast<uint32_t>(result.size()));
  return result;
}

void printError(int32_t status)
{
  ERROR_MSG("NIDAQ ERROR : code {}", status);
  int32_t error_size = DAQmxGetErrorString(status, nullptr, 0);
  if(error_size < 0){
    ERROR_MSG("Unable to get code message");
    return;
  }
  std::string err_str(static_cast<size_t>(error_size), '\0');
  int32_t errcode = DAQmxGetErrorString(status, err_str.data(), static_cast<uint32_t>(error_size));
  if(errcode < 0){
    ERROR_MSG("Unable to parse message");
    return;
  }
  ERROR_MSG("Message : {}", err_str);
}

struct physical_channel_t 
{
  explicit physical_channel_t(std::string name, DAQ::ChannelType::type_t type) : name(std::move(name)), type(type) {}
  int32_t addToTask(TaskHandle task_handle) const;
  std::string name;
  DAQ::ChannelType::type_t type=DAQ::ChannelType::UNKNOWN;
  int32_t reference = DAQmx_Val_RSE;
  double offset=0.0;
  double gain=1.0;
  size_t range_index=0;
  size_t units_index=0;
  bool active=false;
};

int32_t physical_channel_t::addToTask(TaskHandle task_handle) const
{
  int32_t err=-1; 
  std::string units = get_default_units().at(units_index);
  auto [min, max] = get_default_ranges().at(range_index);
  switch(type){
    case DAQ::ChannelType::AI :
      if(units == "volts"){
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
    case DAQ::ChannelType::AO :
      if(units == "volts"){
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
    case DAQ::ChannelType::DI :
      err = DAQmxCreateDIChan(task_handle,
                              name.c_str(),
                              nullptr,
                              DAQmx_Val_ChanForAllLines);   
      printError(err);
      break;
    case DAQ::ChannelType::DO :
      err = DAQmxCreateDOChan(task_handle,
                              name.c_str(),
                              nullptr,
                              DAQmx_Val_ChanForAllLines);   
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
  Device(const std::string& dev_name,
         const std::vector<IO::channel_t>& channels,
         std::string  internal_name);
  ~Device() final;

  size_t getChannelCount(DAQ::ChannelType::type_t type) const final;
  bool getChannelActive(DAQ::ChannelType::type_t type, DAQ::index_t index) const final;
  int setChannelActive(DAQ::ChannelType::type_t type, DAQ::index_t index, bool state) final;
  size_t getAnalogRangeCount(DAQ::index_t index) const final;
  size_t getAnalogReferenceCount(DAQ::index_t index) const final;
  size_t getAnalogUnitsCount(DAQ::index_t index) const final;
  size_t getAnalogDownsample(DAQ::ChannelType::type_t type, DAQ::index_t index) const final;
  std::string getAnalogRangeString(DAQ::ChannelType::type_t type,
                                           DAQ::index_t index,
                                           DAQ::index_t range) const final;
  std::string getAnalogReferenceString(DAQ::ChannelType::type_t type,
                                               DAQ::index_t index,
                                               DAQ::index_t reference) const final;
  std::string getAnalogUnitsString(DAQ::ChannelType::type_t type,
                                           DAQ::index_t index,
                                           DAQ::index_t units) const final;
  double getAnalogGain(DAQ::ChannelType::type_t type, DAQ::index_t index) const final;
  double getAnalogZeroOffset(DAQ::ChannelType::type_t type, DAQ::index_t index) const final;
  DAQ::index_t getAnalogRange(DAQ::ChannelType::type_t type, DAQ::index_t index) const final;
  DAQ::index_t getAnalogReference(DAQ::ChannelType::type_t type, DAQ::index_t index) const final;
  DAQ::index_t getAnalogUnits(DAQ::ChannelType::type_t type, DAQ::index_t index) const final;
  DAQ::index_t getAnalogOffsetUnits(DAQ::ChannelType::type_t type, DAQ::index_t index) const final;
  int setAnalogGain(DAQ::ChannelType::type_t type, DAQ::index_t index, double gain) final;
  int setAnalogRange(DAQ::ChannelType::type_t type, DAQ::index_t index, DAQ::index_t range) final;
  int setAnalogZeroOffset(DAQ::ChannelType::type_t type,
                                  DAQ::index_t index,
                                  double offset) final;
  int setAnalogReference(DAQ::ChannelType::type_t type,
                                 DAQ::index_t index,
                                 DAQ::index_t reference) final;
  int setAnalogUnits(DAQ::ChannelType::type_t type, DAQ::index_t index, DAQ::index_t units) final;
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
  bool getAnalogCalibrationActive(DAQ::ChannelType::type_t type, DAQ::index_t index) const final;
  bool getAnalogCalibrationState(DAQ::ChannelType::type_t type, DAQ::index_t index) const final;
  int setDigitalDirection(DAQ::index_t index, DAQ::direction_t direction) final;

  void read() final;
  void write() final;
private:
  TaskHandle nidaq_task_handle{};
  std::string daq_card_name;
  std::array<std::vector<physical_channel_t>, 4> physical_channels_registry;
  std::array<std::pair<double, double>, 7> default_ranges = get_default_ranges();
  std::array<std::string, 2> default_units = get_default_units();
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
               std::string  internal_name) 
  : DAQ::Device(dev_name, channels), daq_card_name(std::move(internal_name))
{
  if(DAQmxCreateTask("read", &this->nidaq_task_handle) < 0) {
    ERROR_MSG("NIDAQ::Device : Unable to create read task for device {}", dev_name);
    return;
  }
  std::vector<std::string> chan_names;
  for(size_t type=0; type < physical_channels_registry.size(); type++){
    chan_names = physical_channel_names(dev_name, static_cast<DAQ::ChannelType::type_t>(type));
    for(auto chan_name : chan_names){
      physical_channels_registry.at(type).emplace_back(chan_name, static_cast<DAQ::ChannelType::type_t>(type));
    }
  }
}

Device::~Device()
{
  DAQmxClearTask(this->nidaq_task_handle);
}

size_t Device::getChannelCount(DAQ::ChannelType::type_t type) const  
{
  return physical_channels_registry.at(type).size();   
}

bool Device::getChannelActive(DAQ::ChannelType::type_t type, DAQ::index_t index) const
{
  return physical_channels_registry.at(type).at(index).active;
}

int Device::setChannelActive(DAQ::ChannelType::type_t type, DAQ::index_t index, bool state) 
{
  int32_t err = 0;
  if(state) {
    err = physical_channels_registry.at(type).at(index).addToTask(nidaq_task_handle);
    physical_channels_registry.at(type).at(index).active = err == 0;
  } else {
    physical_channels_registry.at(type).at(index).active = state;
  }
  return err;
}

size_t Device::getAnalogRangeCount(DAQ::index_t  /*index*/) const  
{
  return default_ranges.size();
}

size_t Device::getAnalogReferenceCount(DAQ::index_t  /*index*/) const  
{
  return DAQ::Reference::UNKNOWN;
}

size_t Device::getAnalogUnitsCount(DAQ::index_t  /*index*/) const  
{
  return default_units.size();
}

size_t Device::getAnalogDownsample(DAQ::ChannelType::type_t  /*type*/, DAQ::index_t  /*index*/) const  
{
  return 0; 
}

std::string Device::getAnalogRangeString(DAQ::ChannelType::type_t type,
                                         DAQ::index_t index,
                                         DAQ::index_t  /*range*/) const  
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return ""; }
  std::string formatting = "{:.1f}";
  auto [min, max] = default_ranges.at(physical_channels_registry.at(type).at(index).range_index);
  return fmt::format(formatting, min) + std::string(" to ") + fmt::format(formatting, max);
}

std::string Device::getAnalogReferenceString(DAQ::ChannelType::type_t type,
                                             DAQ::index_t index,
                                             DAQ::index_t  /*reference*/) const  
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return ""; }
  int32_t ref = physical_channels_registry.at(type).at(index).reference;
  std::string refstr;
  switch(ref){
    case DAQmx_Val_RSE :
      refstr = "Ground";
      break;
    case DAQmx_Val_NRSE :
      refstr = "Common";
      break;
    case DAQmx_Val_Diff :
      refstr = "Differential";
      break;
    case DAQmx_Val_PseudoDiff :
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
                                         DAQ::index_t  /*units*/) const  
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return ""; }
  return default_units.at(physical_channels_registry.at(type).at(index).units_index);
}

double Device::getAnalogGain(DAQ::ChannelType::type_t type, DAQ::index_t index) const 
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return 1.0; }
  return physical_channels_registry.at(type).at(index).gain;
}

double Device::getAnalogZeroOffset(DAQ::ChannelType::type_t type, DAQ::index_t index) const  
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return 1.0; }
  return physical_channels_registry.at(type).at(index).offset;
}

DAQ::index_t Device::getAnalogRange(DAQ::ChannelType::type_t type, DAQ::index_t index) const
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return 0; }
  return physical_channels_registry.at(type).at(index).range_index;
}

DAQ::index_t Device::getAnalogReference(DAQ::ChannelType::type_t type, DAQ::index_t index) const  
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return 0; }
  return static_cast<size_t>(physical_channels_registry.at(type).at(index).reference);
}

DAQ::index_t Device::getAnalogUnits(DAQ::ChannelType::type_t type, DAQ::index_t index) const
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return 0; }
  return physical_channels_registry.at(type).at(index).units_index;
}

DAQ::index_t Device::getAnalogOffsetUnits(DAQ::ChannelType::type_t type, DAQ::index_t index) const
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return 0; }
  return physical_channels_registry.at(type).at(index).units_index;
}

int Device::setAnalogGain(DAQ::ChannelType::type_t type, DAQ::index_t index, double gain) 
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return -1; }
  physical_channels_registry.at(type).at(index).gain = gain;
  return 0;
}

int Device::setAnalogRange(DAQ::ChannelType::type_t type, DAQ::index_t index, DAQ::index_t range) 
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return -1; }
  physical_channels_registry.at(type).at(index).range_index = range;
  return 0;
}

int Device::setAnalogZeroOffset(DAQ::ChannelType::type_t type,
                                DAQ::index_t index,
                                double offset)
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return -1; }
  physical_channels_registry.at(type).at(index).offset = offset; 
  return 0;
}

int Device::setAnalogReference(DAQ::ChannelType::type_t type,
                               DAQ::index_t index,
                               DAQ::index_t reference) 
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return -1; }
  physical_channels_registry.at(type).at(index).range_index = reference; 
  return 0;
}

int Device::setAnalogUnits(DAQ::ChannelType::type_t type, DAQ::index_t index, DAQ::index_t units)
{
  if(type == DAQ::ChannelType::DI || type == DAQ::ChannelType::DO){ return -1; }
  physical_channels_registry.at(type).at(index).units_index = units; 
  return 0;
}

int Device::setAnalogOffsetUnits(DAQ::ChannelType::type_t  /*type*/,
                                 DAQ::index_t  /*index*/,
                                 DAQ::index_t  /*units*/) 
{ return 0;}

int Device::setAnalogDownsample(DAQ::ChannelType::type_t  /*type*/,
                                DAQ::index_t  /*index*/,
                                size_t  /*downsample*/) 
{
  return 0;
}

int Device::setAnalogCounter(DAQ::ChannelType::type_t  /*type*/, DAQ::index_t index) { return 0; }
int Device::setAnalogCalibrationValue(DAQ::ChannelType::type_t  /*type*/,
                                      DAQ::index_t  /*index*/,
                                      double  /*value*/) { return 0; }
double Device::getAnalogCalibrationValue(DAQ::ChannelType::type_t  /*type*/,
                                         DAQ::index_t  /*index*/) const  { return 0.0; }
int Device::setAnalogCalibrationActive(DAQ::ChannelType::type_t  /*type*/,
                                       DAQ::index_t  /*index*/,
                                       bool  /*state*/) { return 0; }
bool Device::getAnalogCalibrationActive(DAQ::ChannelType::type_t  /*type*/, DAQ::index_t  /*index*/) const  { return false; }
bool Device::getAnalogCalibrationState(DAQ::ChannelType::type_t  /*type*/, DAQ::index_t  /*index*/) const  { return false; }

int Device::setDigitalDirection(DAQ::index_t index, DAQ::direction_t direction) {}

void Device::read(){}
void Device::write(){}

Driver::Driver() : DAQ::Driver(std::string(DEFAULT_DRIVER_NAME))
{
  this->loadDevices();
}

void Driver::loadDevices()
{
  int32_t device_names_buffer_size = DAQmxGetSysDevNames(nullptr, 0);
  std::string string_buffer(static_cast<size_t>(device_names_buffer_size), '\0');
  DAQmxGetSysDevNames(string_buffer.data(), static_cast<uint32_t>(string_buffer.size()));
  std::vector<std::string> device_names = split_string(string_buffer, ", ");
  std::vector<IO::channel_t> channels;
  std::vector<std::string> split_channel_names;
  size_t pos=0;
  std::string temp_channel_name;
  std::string temp_daq_name;
  std::string description;
  int channel_id = 0;
  for(const auto& internal_dev_name : device_names){
    for(size_t query_indx=0; query_indx<4; query_indx++){
      split_channel_names = physical_channel_names(internal_dev_name, 
                                                   static_cast<DAQ::ChannelType::type_t>(query_indx)); 
      temp_daq_name = physical_card_name(internal_dev_name);
      for(const auto& chan_name : split_channel_names){
        description = std::string(query_indx<2 ? "Analog" : "Digital");
        description += " ";
        description += std::string(query_indx%2==0 ? "Output" : "Input");
        description += " ";
        pos = chan_name.find_first_of("0123456789"); 
        channel_id = std::stoi(chan_name.substr(pos, chan_name.size()-pos));
        description += std::to_string(channel_id);
        channels.push_back({chan_name, 
                            description, 
                            query_indx%2==0 ? IO::INPUT : IO::OUTPUT, 
                            1});
      }
    }
    this->nidaq_devices.emplace_back(temp_daq_name, channels, internal_dev_name);
  }
}

void Driver::unloadDevices()
{

}

std::vector<DAQ::Device*> Driver::getDevices()
{
  std::vector<DAQ::Device*> devices;
  devices.reserve(this->nidaq_devices.size());
  for(auto& device : nidaq_devices){
    devices.push_back(&device);
  }
  return devices;
}

static std::mutex driver_mut;
static std::unique_ptr<Driver> instance;

DAQ::Driver* Driver::getInstance()
{
  if(instance == nullptr){
    std::unique_lock<std::mutex> lock(driver_mut);
    instance = std::unique_ptr<Driver>(new Driver());
  }
  return instance.get();
}

extern "C" {
DAQ::Driver* getDriver(){
  return Driver::getInstance();
}
}
