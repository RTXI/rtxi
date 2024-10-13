#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <filesystem>
#include <mutex>
#include <sstream>

#include "rt.hpp"

namespace Event
{
class Object;
}  // namespace Event

/*!
 * Class responsible for logging all events and telemitry
 */
class eventLogger
{
public:
  /*!
   * Log the fired event in human readable format
   *
   * The log function will take the pointer to the event object
   * and send to standard output a string describing the event
   * in human readable format. It will prepend information like
   * the current local time and type of event fired. Unknown
   * event types will be marked is type UNKNOWN and printed. It
   * is thread safe.
   *
   * \param event A pointer to the fired event
   */
  void log(Event::Object* event);

  /*!
   * Log the fired telemitry in human readable format
   *
   * The log function will take the pointer to the telemitry object
   * and send to standard output a string describing the event
   * in human readable format. It will prepend information like
   * the current local time and type of telemitry fired. It is thread
   * safe.
   *
   * \param event A pointer to the fired telemitry
   */
  void log(RT::Telemitry::Response response);

private:
  std::filesystem::path logfile;
  std::stringstream ss;
  std::mutex log_mutex;
};

#endif
