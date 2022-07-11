
#include <string>
#include <vector>
#include <optional>
#include <QWidgets>

#include "event.hpp"
#include "io.hpp"
#include "rt.hpp"

namespace Modules{

namespace Variable{
enum variable_t : size_t{
  PARAMETER = 0,
  STATE,
  EVENT,
  COMMENT,
  UNKNOWN
}

/*!
 * Structure used to store information about module upon creation.
 * It is a structure describing module specific constants and 
 * variables.
 *
 * \param name The name of the channel
 * \param description short description of the channel
 * \param vartype type of variable that is stored
 * \param value the data stored for this variable
 * 
 * \sa IO::Block::Block()
 */
typedef struct
{
  std::string name;
  std::string description;
  Modules::Variable::variable_t vartype = Modules::Variable::UNKNOWN;
  std::variant<bool, int, float, Event::Object, std::string> value;
} Info;
}

class Component : public RT::Thread, public Event::Handler
{
public:
  Component(std::string name,
            std::vector<IO::Channel_t> channels,
            std::vector<Modules::Variable::Info> variables,
            Event::Manager event_manager);
  ~Component();

private:
  std::unordered_map<std::string, Modules::Component::Info> variables;
}


class Manager
{
  
}

}
