
#include "nidaq_driver.hpp"



NIDAQ::Driver::Driver() : DAQ::Driver(std::string(NIDAQ::DEFAULT_DRIVER_NAME))
{}

void NIDAQ::Driver::loadDevices()
{

}

void NIDAQ::Driver::unloadDevices()
{

}

std::vector<DAQ::Device*> NIDAQ::Driver::getDevices()
{
  return {};
}

static std::mutex driver_mut;
static std::unique_ptr<NIDAQ::Driver> instance;

DAQ::Driver* NIDAQ::Driver::getInstance()
{
  if(instance == nullptr){
    std::unique_lock<std::mutex> lock(driver_mut);
    instance = std::unique_ptr<NIDAQ::Driver>(new NIDAQ::Driver());
  }
  std::cout << "I ran" << std::endl;
  return instance.get();
}

extern "C" {
DAQ::Driver* getDriver(){
  return NIDAQ::Driver::getInstance();
}
}
