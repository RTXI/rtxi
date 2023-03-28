#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <sstream>
#include <filesystem>
#include <mutex>

#include "event.hpp"
#include "rt.hpp"

class eventLogger
{
public:
  void log(Event::Object* event);
  void log(RT::Telemitry::Response response);
private:
  std::filesystem::path logfile;
  std::stringstream ss;
  std::mutex log_mutex;
};



#endif
