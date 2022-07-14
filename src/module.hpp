
#include <optional>
#include <string>
#include <vector>

#include <QWidgets>

#include "event.hpp"
#include "io.hpp"
#include "rt.hpp"

namespace Modules
{

namespace Variable
{
enum variable_t : size_t
{
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
 *
 * \sa IO::Block::Block()
 */
struct Info
{
  std::string name;
  std::string description;
  Modules::Variable::variable_t vartype;
};

struct State_var : Info
{
  int value;
};

struct Event_var : Info
{
  Event::Object value;
};

struct Comment_var : Info
{
  std::string value;
};

struct Parameter_var : Info
{
  double value;
};

}  // namespace Variable

class Settings
{
public:
  Settings();
  ~Settings();

protected:


private:
  std::vector<Modules::Variable::Parameter_var> parameters;
  std::vector<Modules::Variable::State_var> states;
  std::vector<Modules::Variable::Comment_var> comments;
};

class Component
    : public RT::Thread
    , public Event::Handler
{
public:
  Component(std::string name,
            std::vector<IO::Channel_t> channels,
            std::vector<Modules::Variable::Info> variables,
            Event::Manager event_manager);
  ~Component();

  virtual Modules::Settings getSettings();
  virtual void loadSettings(Module::Settings settings);
private:
  std::vector<Modules::Variable::Parameter_var> parameters;
  std::vector<Modules::Variable::State_var> states;
  std::vector<Modules::Variable::Comment_var> comments;
  std::vector<Modules::Variable::Event_var> events;
};

class UI : public QWidgets
{
public:
  UI();
  ~UI();
};

class Object
{
public:
  Object();
  ~Object();

private:
  std::unique_ptr<Modules::Component> rtxi_component;
  std::unique_ptr<Modules::UI> rtxi_interface;
}

class Manager
{
public:
  Manager();
  ~Manager();

private:
  std::vector<std::unique_ptr<Modules::Object>> rtxi_modules;
};

}  // namespace Modules
