
#include <chrono>
#include <iostream>

#include "event.hpp"
#include "logger.hpp"
#include "module.hpp"

void eventLogger::log(Event::Object* event)
{
  try {
    std::unique_lock<std::mutex> lk(this->log_mutex);
    const auto time_point = std::chrono::system_clock::now();
    const std::time_t now = std::chrono::system_clock::to_time_t(time_point);
    this->ss << "[ "; 
    this->ss << std::put_time(std::localtime(&now), "%F %T");
    this->ss << " ] (EVENT FIRED)\t";
    this->ss << " TYPE -- " << event->getName();
    switch (event->getType()){
      case Event::Type::RT_PERIOD_EVENT:
        this->ss << "\t VALUE -- ";
        this->ss << std::any_cast<int64_t>(event->getParam("period"));
      case Event::Type::RT_PREPERIOD_EVENT:
      case Event::Type::RT_POSTPERIOD_EVENT:
        break;
      case Event::Type::RT_THREAD_PAUSE_EVENT:
      case Event::Type::RT_THREAD_UNPAUSE_EVENT:
      case Event::Type::RT_THREAD_INSERT_EVENT:
      case Event::Type::RT_THREAD_REMOVE_EVENT:
        this->ss << "\t SOURCE -- ";
        this->ss << std::any_cast<RT::Thread*>(event->getParam("thread"))->getName();
        break;
      case Event::Type::RT_DEVICE_PAUSE_EVENT:
      case Event::Type::RT_DEVICE_UNPAUSE_EVENT:
      case Event::Type::RT_DEVICE_INSERT_EVENT:
      case Event::Type::RT_DEVICE_REMOVE_EVENT:
        this->ss << "\t SOURCE -- ";
        this->ss << std::any_cast<RT::Device*>(event->getParam("device"))->getName();
      case Event::Type::IO_LINK_INSERT_EVENT:
      case Event::Type::IO_LINK_REMOVE_EVENT:
        this->ss << "\t SOURCE -- ";
        this->ss << std::any_cast<IO::Block*>(event->getParam("block"))->getName();
        break;
      case Event::Type::PLUGIN_INSERT_EVENT:
        this->ss << "\t SOURCE -- ";
        this->ss << std::any_cast<std::string>(event->getParam("pluginName"));
        break;
      case Event::Type::PLUGIN_REMOVE_EVENT:
        this->ss << "\t SOURCE -- ";
        this->ss << std::any_cast<Modules::Plugin*>(event->getParam("pluginPointer"))->getName();
        break;
      default:
        break;
    }
    this->ss << "\n";
    std::cout << this->ss.str();
    this->ss.str("");
  } catch (std::bad_any_cast&){
    ERROR_MSG("Error while parsing event of type: {}", event->getName());
    ERROR_MSG("Flushing event stream");
    ERROR_MSG("{}", this->ss.str());
    this->ss.str("");
  }

}

void eventLogger::log(RT::Telemitry::Response response)
{
  try {
    std::unique_lock<std::mutex> lk(this->log_mutex);
    const auto time_point = std::chrono::system_clock::now();
    const std::time_t now = std::chrono::system_clock::to_time_t(time_point);
    this->ss << "[ "; 
    this->ss << std::put_time(std::localtime(&now), "%F %T");
    this->ss << " ] (TELEMITRY)\t";
    this->ss << " TYPE -- ";
    switch (response.type){
      case RT::Telemitry::RT_PERIOD_UPDATE:
        this->ss << "Period Updated";
        break;
      case RT::Telemitry::RT_THREAD_LIST_UPDATE:
        this->ss << "System Threadlist Updated";
        break;
      case RT::Telemitry::RT_DEVICE_LIST_UPDATE:
        this->ss << " System Devicelist Updated";
        break;
      case RT::Telemitry::RT_NOOP:
        this->ss << "NO-OP Acknowledged";
        break;
      case RT::Telemitry::RT_SHUTDOWN:
        this->ss << "Real-Time System Shutdown";
        break;
      case RT::Telemitry::RT_MODULE_PARAM_UPDATE:
        this->ss << "Module Parameter Updated";
        break;
      case RT::Telemitry::RT_ERROR:
        this->ss << "Real-Time System Error";
        break;
      case RT::Telemitry::NO_TELEMITRY:
        this->ss << "NO TELEMITRY";
        break;
      default:
        break;
    }  
    this->ss << "\n";
    std::cout << this->ss.str();
    this->ss.str("");
  } catch (...) {
    ERROR_MSG("Something went wrong while logging telemitry from RT system");
  }
}