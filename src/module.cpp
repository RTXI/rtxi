
#include <vector>

#include "module.hpp"

Module::Component(std::string name, 
                  std::vector<IO::Channel_t> channels,
                  size_t channel_size,
                  std::vector<Modules::Variable::Info> variables)
    : IO::Thread(name, channels, channel_size)
{
  for(auto var : variables){
    if(std::holds_alternative<int>(var)){
      this->states.push_back(var);
    } else if (std::holds_alternative<double>(var)){
      this->parameters.push_back(var);
    } else if (std::holds_alternative<std::string>(var)){
      this->comments.push_back(var);
    } else if (std::holds_alternative<Event::Object>(var)){
      this->events.push_back(var);
    } else {
      ERROR_MSG("Unknown or empty module variable type provided for module {}. Ignoring", this->name);
    }
  }
}

size_t Module::Component::getCount(Modules::Variable::variable_t vartype)
{
  size_t result;
  switch(vartype){
    case Modules::Variable::PARAMETER:
      result = this->parameter.size();
      break;
    case Modules::Variable::STATE:
      result = this->states.size();
      break;
    case Modules::Variable::COMMENT:
      result = this->comments.size();
      break;
    case Modules::Variable::EVENT:
      result = this->events.size();
      break;
    default:
      result =  0;
  }
  return result;
}

std::string Module::Component::getName(Modules::Variable::variable_t vartype, size_t index)
{
  std::string result;
  switch(vartype){
    case Modules::Variable::PARAMETER:
      result = this->parameter.at(index).name;
      break;
    case Modules::Variable::STATE:
      result = this->states.at(index).name;
      break;
    case Modules::Variable::COMMENT:
      result = this->comments.at(index).name;
      break;
    case Modules::Variable::EVENT:
      result = this->events.at(index).name;
      break;
    default:
      result = "";
  }

  return result;
}

std::string Module::Component::getDescription(IO::flags_t vartype, size_t n) const
{
  std::string result;
  switch(vartype){
    case Modules::Variable::PARAMETER:
      result = this->parameter.at(index).description;
      break;
    case Modules::Variable::STATE:
      result = this->states.at(index).description;
      break;
    case Modules::Variable::COMMENT:
      result = this->comments.at(index).description;
      break;
    case Modules::Variable::EVENT:
      result = this->events.at(index).description;
      break;
    default:
      result = "";
  }
  return result;
}

int Module::Component::getValue(Modules::Variable::STATE vartype, size_t index)
{
  return std::get<int>(this->states.at(index));
}

double Module::Component::getValue(Modules::Variable::PARAMETER vartype, size_t index)
{
  return std::get<double>(this->parameters.at(index));
}

std::string Module::Component::getValue(Modules::Variable::COMMENT vartype, size_t index)
{
  return std::get<std::string>(this->comments.at(index));
}

Event::Object Module::Component::getValue(Modules::Variable::EVENT vartype, size_t index)
{
  return std::get<Event::Object>(this->events.at(index));
}

std::string Module::Component::getValueString(Modules::Variable::variable_t vartype, size_t index) const
{
  std::string result;
  switch(vartype){
    case Module::Variable::STATE:
    case Module::Variable::DOUBLE:
    case Module::Variable::COMMENT:
      result = fmt::format("{}", this->getValue)
    default:
      result = ""
  }
  return result;
}

void Module::Component::setValue(size_t n, double value)
{
  if (n >= parameter.size() || !parameter[n].data)
    return;

  if (RT::OS::isRealtime() && *parameter[n].data != value) {
    *parameter[n].data = value;

    ::Event::Object event(::Event::WORKSPACE_PARAMETER_CHANGE_EVENT);
    event.setParam("object", (void*)getID());
    event.setParam("index", (void*)n);
    event.setParam("value", (void*)parameter[n].data);
    ::Event::Manager::getInstance()->postEventRT(&event);
  } else {
    ParameterChangeEvent event(getID(), n, value, parameter[n].data);
    RT::System::getInstance()->postEvent(&event);
  }
}

void Module::Component::setComment(size_t n, std::string newComment)
{
  if (n >= comment.size())
    return;

  comment[n].comment = newComment;
}

double* Module::Component::getData(IO::flags_t type, size_t n)
{
  if (type & PARAMETER && n < parameter.size())
    return parameter[n].data;
  if (type & STATE && n < state.size())
    return state[n].data;
  if (type & EVENT && n < event.size())
    return event[n].data;
  return 0;
}

void Module::Component::setData(IO::flags_t type, size_t n, double* data)
{
  if (type & PARAMETER && n < parameter.size())
    parameter[n].data = data;
  if (type & STATE && n < state.size())
    state[n].data = data;
  if (type & EVENT && n < event.size())
    event[n].data = data;
}

void Workspace::Manager::foreachWorkspace(void (*callback)(Module::Component*,
                                                           void*),
                                          void* param)
{
  Mutex::Locker lock(&mutex);
  for (std::list<Instance*>::iterator i = instanceList.begin();
       i != instanceList.end();
       ++i)
    callback(*i, param);
}

void Workspace::Manager::insertWorkspace(Module::Component* workspace)
{
  if (!workspace) {
    ERROR_MSG("Workspace::Manager::insertWorkspace : invalid workspace\n");
    return;
  }

  Mutex::Locker lock(&mutex);

  if (std::find(instanceList.begin(), instanceList.end(), workspace)
      != instanceList.end())
  {
    ERROR_MSG(
        "Workspace::Manager::insertWorkspace : workspace already present\n");
    return;
  }

  instanceList.push_back(workspace);
}

void Workspace::Manager::removeWorkspace(Module::Component* workspace)
{
  if (!workspace) {
    ERROR_MSG("Workspace::Manager::removeWorkspace : invalid workspace\n");
    return;
  }

  Mutex::Locker lock(&mutex);
  instanceList.remove(workspace);
}
