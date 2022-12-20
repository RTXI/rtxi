
#include <dlfcn.h>
#include <any>
#include <algorithm>
#include <sstream>

#include <QApplication>
#include <QScrollArea>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QTimer>

#include "debug.hpp"
#include "module.hpp"

std::string Modules::Variable::state2string(Modules::Variable::state_t state)
{
  std::string result;
  switch(state){
    case Modules::Variable::INIT :
      result = std::string("INIT");
      break;
    case Modules::Variable::MODIFY :
      result = std::string("MODIFIED PARAMETERS");
      break;
    case Modules::Variable::PERIOD :
      result = std::string("PERIOD CHANGE");
      break;
    case Modules::Variable::PAUSE :
      result = std::string("PLUGIN PAUSED");
      break;
    case Modules::Variable::UNPAUSE :
      result = std::string("PLUGIN UNPAUSED");
      break;
    case Modules::Variable::EXIT :
      result = std::string("EXIT");
      break;
    default:
      result = std::string("UNKNOWN STATE");
      break;
  }
  return result;
}

Modules::DefaultGUILineEdit::DefaultGUILineEdit(QWidget* parent)
    : QLineEdit(parent)
{
  QObject::connect(
      this, SIGNAL(textChanged(const QString&)), this, SLOT(redden()));
}

Modules::DefaultGUILineEdit::~DefaultGUILineEdit() {}

void Modules::DefaultGUILineEdit::blacken()
{
  palette.setBrush(this->foregroundRole(),
                   QApplication::palette().color(QPalette::WindowText));
  this->setPalette(palette);
  setModified(false);
}

void Modules::DefaultGUILineEdit::redden()
{
  if (isModified()) {
    palette.setBrush(this->foregroundRole(), Qt::red);
    this->setPalette(palette);
  }
}

Modules::Component::Component(Modules::Plugin* hostPlugin,
                              const std::string& name, 
                              std::vector<IO::channel_t> channels,
                              std::vector<Modules::Variable::Info> variables)
    : RT::Thread(name, channels), hostPlugin(hostPlugin)
{
  for(auto var : variables){
    this->parameter[var.name] = var;
  }
}

std::string Modules::Component::getDescription(const std::string& varname)
{
  return this->parameter[varname].description;
}

std::string Modules::Component::getValueString(const std::string& varname)
{
  std::string value;
  switch(this->parameter[varname].vartype){
    case Modules::Variable::UINT_PARAMETER :
      value = std::to_string(std::get<uint64_t>(this->parameter[varname].value));
      break;
    case Modules::Variable::INT_PARAMETER :
      value = std::to_string(std::get<int>(this->parameter[varname].value));
      break;
    case Modules::Variable::DOUBLE_PARAMETER :
      value = std::to_string(std::get<double>(this->parameter[varname].value));
      break;
    case Modules::Variable::STATE :
      value = "";
      break;
    case Modules::Variable::COMMENT :
      value = std::get<std::string>(this->parameter[varname].value);
      break;
    case Modules::Variable::UNKNOWN :
      value = "UNKNOWN";
      break;
    default:
      value = "ERROR";
      break;
  }
  return value;
}

void Modules::Component::execute()
{
  // This is defined by the user
}

Modules::Panel::Panel(std::string name, QMainWindow* main_window)
    : QWidget(main_window), maind_window(main_window)
{
  setWindowTitle(QString::fromStdString(name));

  QTimer* timer = new QTimer(this);
  timer->setTimerType(Qt::PreciseTimer);
  timer->start(1000);
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));
}

void Modules::Panel::createGUI(std::vector<Modules::Variable::Info> vars, MainWindow* main_window)
{
  // Make Mdi
  this->subWindow = new QMdiSubWindow;
  this->subWindow->setAttribute(Qt::WA_DeleteOnClose);
  //subWindow->setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
  this->subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint
                            | Qt::WindowMinimizeButtonHint);
  this->subWindow->setOption(QMdiSubWindow::RubberBandResize, true);
  this->subWindow->setOption(QMdiSubWindow::RubberBandMove, true);
  main_window->createMdi(subWindow);

  // Create main layout
  this->layout = new QGridLayout;

  // Create child widget and gridLayout
  QScrollArea* gridArea = new QScrollArea;
  this->gridBox = new QWidget;
  gridArea->setWidget(this->gridBox);
  gridArea->ensureWidgetVisible(this->gridBox, 0, 0);
  gridArea->setWidgetResizable(true);
  QGridLayout* gridLayout = new QGridLayout;

  for (auto varinfo : vars) {
    param_t param;

    param.label = new QLabel(QString::fromStdString(varinfo.name), gridBox);
    gridLayout->addWidget(param.label, this->parameter.size(), 0);
    gridLayout->addWidget(param.edit, this->parameter.size(), 1);

    param.label->setToolTip(QString::fromStdString(varinfo.description));
    param.edit->setToolTip(QString::fromStdString(varinfo.description));

    param.str_value = QString();
    param.label = new QLabel;
    param.type = varinfo.vartype;
    switch(varinfo.vartype) {
      case Modules::Variable::DOUBLE_PARAMETER :
        param.edit->setValidator(new QDoubleValidator(param.edit));
        break;
      case Modules::Variable::UINT_PARAMETER :
        param.edit->setValidator(new QIntValidator(param.edit));
        //validator->setBottom(0);
        break;
      case Modules::Variable::INT_PARAMETER :
        param.edit->setValidator(new QIntValidator(param.edit));
        break;
      case Modules::Variable::STATE :
        param.edit->setReadOnly(true);
        palette.setBrush(param.edit->foregroundRole(), Qt::darkGray);
        param.edit->setPalette(palette);
        break;
      case Modules::Variable::COMMENT :
        break;
      case Modules::Variable::UNKNOWN :
        ERROR_MSG("Variable {} in Module {} is of category UNKNOWN", varinfo.name, this->getName());
        break;
      default :
        ERROR_MSG("Variable {} in Module {} has undefined or broken category", varinfo.name, this->getName());
    }
    parameter[QString::fromStdString(varinfo.name)] = param;
  }

  // Create child widget
  buttonGroup = new QGroupBox;
  QHBoxLayout* buttonLayout = new QHBoxLayout;

  // Create elements
  pauseButton = new QPushButton("Pause", this);
  pauseButton->setCheckable(true);
  QObject::connect(pauseButton, SIGNAL(toggled(bool)), this, SLOT(pause(bool)));
  buttonLayout->addWidget(pauseButton);

  modifyButton = new QPushButton("Modify", this);
  QObject::connect(
      modifyButton, SIGNAL(clicked(void)), this, SLOT(modify(void)));
  buttonLayout->addWidget(modifyButton);

  unloadButton = new QPushButton("Unload", this);
  QObject::connect(unloadButton, SIGNAL(clicked(void)), this, SLOT(exit(void)));
  buttonLayout->addWidget(unloadButton);

  // Add layout to box
  gridBox->setLayout(gridLayout);
  buttonGroup->setLayout(buttonLayout);

  // Keep one row of space above for users to place in grid
  layout->addWidget(gridArea, 1, 0);

  // Attempt to put these at the bottom at all times
  layout->addWidget(buttonGroup, 10, 0);

  // Set layout to Mdi and show
  setLayout(layout);
  subWindow->setWidget(this);
  subWindow->show();
}

void Modules::Panel::update(Modules::Variable::state_t state) {}

void Modules::Panel::resizeMe()
{
  subWindow->adjustSize();
}

void Modules::Panel::exit()
{
  this->hostPlugin->exit();
  this->subWindow->close();
}

void Modules::Panel::refresh()
{
  double double_value;
  int int_value;
  uint64_t uint_value;
  std::stringstream sstream;
  for (auto i : this->parameter)
  {
    double_value = 0;
    int_value = 0;
    uint_value = 0;
    sstream.str("");

    switch(i.second.type) {
      case Modules::Variable::STATE :
        i.second.edit->setText(i.second.str_value);
        palette.setBrush(i.second.edit->foregroundRole(), Qt::darkGray);
        i.second.edit->setPalette(palette);
        break;
      case Modules::Variable::UINT_PARAMETER :
        uint_value = this->hostPlugin->getComponentUIntParameter(i.first.toStdString());
        sstream << uint_value;
        i.second.edit->setText(QString::fromStdString(sstream.str()));
        break;
      case Modules::Variable::INT_PARAMETER :
        int_value = this->hostPlugin->getComponentIntParameter(i.first.toStdString());
        sstream << int_value;
        i.second.edit->setText(QString::fromStdString(sstream.str()));
        break;
      case Modules::Variable::DOUBLE_PARAMETER :
        double_value = this->hostPlugin->getComponentDoubleParameter(i.first.toStdString());
        sstream << double_value;
        i.second.edit->setText(QString::fromStdString(sstream.str()));
        break;
      default :
        ERROR_MSG("Unable to determine refresh type for component {}", this->getName());
    }
  }
  pauseButton->setChecked(!(this->hostPlugin->getActive()));
}

void Modules::Panel::modify()
{
  // bool active = getActive();
  // setActive(false);

  // for (auto i = parameter.begin(); i != parameter.end(); ++i) {
  //   if (i->second.type == Modules::Variable::COMMENT) {
  //     QByteArray textData = i->second.edit->text().toLatin1();
  //     const char* text = textData.constData();
  //     this->hostPlugin->setComment(i->second.index, text);
  //   }
  // }

  // this->update(Modules::Variable::MODIFY);
  // this->setActive(active);

  // for (auto i = parameter.begin(); i != parameter.end(); ++i){
  //   i->second.edit->blacken();
  // }
}

void Modules::Panel::setComment(const QString& name, QString comment)
{
  auto n = parameter.find(name);
  if (n != parameter.end() && (n->second.type == Modules::Variable::COMMENT)) {
    n->second.edit->setText(comment);
    QByteArray textData = comment.toLatin1();
    const char* text = textData.constData();
    this->hostPlugin->setComponentComment(n->first.toStdString(), text);
  }
}

void Modules::Panel::setParameter(const QString& name, double value)
{
  auto n = parameter.find(name);
  if ((n != parameter.end()) && (n->second.type == Modules::Variable::DOUBLE_PARAMETER)) {
    n->second.edit->setText(QString::number(value));
    n->second.str_value = n->second.edit->text();
    this->hostPlugin->setComponentDoubleParameter(n->first.toStdString(), value);
    //setValue(n->second.index, n->second.edit->text().toDouble());
  }
}

void Modules::Panel::setParameter(const QString& name, int value)
{
  auto n = parameter.find(name);
  if ((n != parameter.end()) && (n->second.type == Modules::Variable::INT_PARAMETER)) {
    n->second.edit->setText(QString::number(value));
    n->second.str_value = n->second.edit->text();
    this->hostPlugin->setComponentIntParameter(n->first.toStdString(), value);
    //setValue(n->second.index, n->second.edit->text().toDouble());
  }
}

void Modules::Panel::setParameter(const QString& name, uint64_t value)
{
  auto n = parameter.find(name);
  if ((n != parameter.end()) && (n->second.type == Modules::Variable::UINT_PARAMETER)) {
    n->second.edit->setText(QString::number(value));
    n->second.str_value = n->second.edit->text();
    this->hostPlugin->setComponentUintParameter(n->first.toStdString(), value);
    //setValue(n->second.index, n->second.edit->text().toDouble());
  }
}

void Modules::Panel::setState(const QString& name, Modules::Variable::state_t ref)
{
  auto n = parameter.find(name);
  if ((n != parameter.end()) && (n->second.type == Modules::Variable::STATE)) {
    //setData(Workspace::STATE, n->second.index, &ref);
    
    n->second.edit->setText(QString::number(ref));
    this->hostPlugin->setComponentState(n->first.toStdString(), ref);
  }
}

void Modules::Panel::pause(bool p)
{
  if (pauseButton->isChecked() != p){
    pauseButton->setDown(p);
  }
  int result = this->hostPlugin->setActive(!p);
  if(result != 0) { 
    ERROR_MSG("Unable to pause/Unpause Plugin {} ", this->getName());
    return; 
  }
  if (p){
    this->update(Modules::Variable::PAUSE);
  } else {
    this->update(Modules::Variable::UNPAUSE);
  }
}

// void Modules::Panel::doDeferred(const Settings::Object::State&)
// {
//   setWindowTitle(QString::number(getID()) + " "
//                  + QString::fromStdString(myname));
// }

// void Modules::Panel::doLoad(const Settings::Object::State& s)
// {
//   for (std::map<QString, param_t>::iterator i = parameter.begin();
//        i != parameter.end();
//        ++i)
//     i->second.edit->setText(
//         QString::fromStdString(s.loadString((i->first).toStdString())));
//   if (s.loadInteger("Maximized"))
//     showMaximized();
//   else if (s.loadInteger("Minimized"))
//     showMinimized();
//   // this only exists in RTXI versions >1.3
//   if (s.loadInteger("W") != 0) {
//     resize(s.loadInteger("W"), s.loadInteger("H"));
//     parentWidget()->move(s.loadInteger("X"), s.loadInteger("Y"));
//   }

//   pauseButton->setChecked(s.loadInteger("paused"));
//   modify();
// }

// void Modules::Panel::doSave(Settings::Object::State& s) const
// {
//   s.saveInteger("paused", pauseButton->isChecked());
//   if (isMaximized())
//     s.saveInteger("Maximized", 1);
//   else if (isMinimized())
//     s.saveInteger("Minimized", 1);

//   QPoint pos = parentWidget()->pos();
//   s.saveInteger("X", pos.x());
//   s.saveInteger("Y", pos.y());
//   s.saveInteger("W", width());
//   s.saveInteger("H", height());

//   for (std::map<QString, param_t>::const_iterator i = parameter.begin();
//        i != parameter.end();
//        ++i)
//     s.saveString((i->first).toStdString(),
//                  (i->second.edit->text()).toStdString());
// }

Modules::Plugin::Plugin(Event::Manager* ev_manager, QMainWindow* main_window, std::string name) :
  event_manager(ev_manager), main_window(main_window), name(name)
{
  this->event_manager->registerHandler(this);
  Event::Object event(Event::Type::PLUGIN_INSERT_EVENT);
  event.setParam("plugin", std::any(this));
  this->event_manager->postEvent(&event);
  event.wait();
}

Modules::Plugin::~Plugin()
{
  if (this->plugin_component != nullptr) {
    Event::Object unplug_block_event(Event::Type::RT_THREAD_REMOVE_EVENT);
    unplug_block_event.setParam("thread", std::any(static_cast<RT::Thread*>(this->plugin_component.get())));
    this->event_manager->postEvent(&unplug_block_event);
    unplug_block_event.wait();
    if(!unplug_block_event.isdone()){
      ERROR_MSG("Component in {} was not removed by Real-Time system", this->name);
    }
  }
  Event::Object remove_plugin_event(Event::Type::PLUGIN_REMOVE_EVENT);
  remove_plugin_event.setParam("plugin", this);
  this->event_manager->postEvent(&remove_plugin_event);
  remove_plugin_event.wait();
  if (!remove_plugin_event.isdone()){
    ERROR_MSG("Plugin {} was not removed correctly from the Plugin registry", this->name);
  }

  this->event_manager->unregisterHandler(this);
}

void Modules::Plugin::attachComponent(std::unique_ptr<Modules::Component> component)
{
  this->plugin_component = std::move(component);
  Event::Object event = Event::Object(Event::Type::RT_THREAD_INSERT_EVENT);
  event.setParam("thread", static_cast<RT::Thread*>(this->plugin_component.get()));
  this->event_manager->postEvent(&event);
  event.wait();
  if(!event.isdone()){
    ERROR_MSG("Real-Time system was unable to register plugin component {}", this->name);
  }
}

int Modules::Plugin::exit()
{
  int result = 0;
  Event::Object event(Event::Type::PLUGIN_REMOVE_EVENT);
  event.setParam("plugin", std::any(this));
  this->event_manager->postEvent(&event);
  event.wait();
  if(!event.isdone()){
    ERROR_MSG("Plugin {} was not removed by the modules manager", this->plugin_component->getName());
    result = -1;
  }
  return result;
}

int Modules::Plugin::getComponentIntParameter(const std::string& parameter_name)
{
  return this->plugin_component->getValue<int>(parameter_name);
}

uint64_t Modules::Plugin::getComponentUIntParameter(const std::string& parameter_name)
{
  return this->plugin_component->getValue<uint64_t>(parameter_name);
}

double Modules::Plugin::getComponentDoubleParameter(const std::string& parameter_name)
{
  return this->plugin_component->getValue<double>(parameter_name);
}

int Modules::Plugin::setComponentIntParameter(const std::string& parameter_name, int value)
{
  int result = 0;
  Event::Object event(Event::Type::MODULE_PARAMETER_CHANGE_EVENT);
  event.setParam("paramName", std::any(parameter_name));
  event.setParam("paramType", std::any(Modules::Variable::INT_PARAMETER));
  event.setParam("paramValue", std::any(value));
  event.setParam("paramModule", std::any(this->plugin_component.get()));
  this->event_manager->postEvent(&event);
  event.wait();
  if(!(event.isdone()))
  {
    result = 1;
  }
  return result;
}

int Modules::Plugin::setComponentDoubleParameter(const std::string& parameter_name, double value)
{
  int result = 0;
  Event::Object event(Event::Type::MODULE_PARAMETER_CHANGE_EVENT);
  event.setParam("paramName", std::any(parameter_name));
  event.setParam("paramType", std::any(Modules::Variable::DOUBLE_PARAMETER));
  event.setParam("paramValue", std::any(value));
  event.setParam("paramModule", std::any(this->plugin_component.get()));
  this->event_manager->postEvent(&event);
  event.wait();
  if(!(event.isdone()))
  {
    result = 1;
  }
  return result;
}

int Modules::Plugin::setComponentUintParameter(const std::string& parameter_name, uint64_t value)
{
  int result = 0;
  Event::Object event(Event::Type::MODULE_PARAMETER_CHANGE_EVENT);
  event.setParam("paramName", std::any(parameter_name));
  event.setParam("paramType", std::any(Modules::Variable::UINT_PARAMETER));
  event.setParam("paramValue", std::any(value));
  event.setParam("paramModule", std::any(this->plugin_component.get()));
  this->event_manager->postEvent(&event);
  event.wait();
  if(!(event.isdone()))
  {
    result = 1;
  }
  return result;
}

int Modules::Plugin::setComponentComment(const std::string& parameter_name, std::string value)
{
  int result = 0;
  Event::Object event(Event::Type::MODULE_PARAMETER_CHANGE_EVENT);
  event.setParam("paramName", std::any(parameter_name));
  event.setParam("paramType", std::any(Modules::Variable::INT_PARAMETER));
  event.setParam("paramValue", std::any(value));
  event.setParam("paramModule", std::any(this->plugin_component.get()));
  this->event_manager->postEvent(&event);
  event.wait();
  if(!(event.isdone()))
  {
    result = 1;
  }
  return result;
}

int Modules::Plugin::setComponentState(const std::string& parameter_name, Modules::Variable::state_t state)
{
  int result = 0;
  Event::Object event(Event::Type::MODULE_PARAMETER_CHANGE_EVENT);
  event.setParam("paramName", std::any(parameter_name));
  event.setParam("paramType", std::any(Modules::Variable::STATE));
  event.setParam("paramValue", std::any(state));
  event.setParam("paramModule", std::any(this->plugin_component.get()));
  this->event_manager->postEvent(&event);
  event.wait();
  if(!(event.isdone()))
  {
    result = 1;
  }
  return result;
}

void Modules::Plugin::receiveEvent(Event::Object* event)
{
  // Set by users who want to handle events
}

bool Modules::Plugin::getActive()
{
  return this->plugin_component->getActive();
}

int Modules::Plugin::setActive(bool state)
{
  int result = 0;
  Event::Type event_type;
  if(state){
    event_type = Event::Type::RT_BLOCK_UNPAUSE_EVENT;
  } else {
    event_type = Event::Type::RT_BLOCK_PAUSE_EVENT;
  }
  Event::Object event(event_type);
  event.setParam("block", std::any(static_cast<IO::Block*>(this->plugin_component.get())));
  this->event_manager->postEvent(&event);
  event.wait();
  if (!event.isdone()){
    result = -1;
  }
  return result;
}

Modules::Manager::Manager(Event::Manager* event_manager, QMainWindow* main_window) :
  event_manager(event_manager), main_window(main_window)
{
  this->rtxi_modules_registry = std::unordered_map<std::string, std::unique_ptr<Modules::Plugin>>();
}

int Modules::Manager::loadPlugin(const std::string& library)
{
  void* handle = dlopen(library.c_str(), RTLD_GLOBAL | RTLD_NOW);
  if (!handle) {
    // ERROR_MSG("Plugin::load : failed to load library: {}", dlerror());
    std::string plugin_dir = std::string("/lib/rtxi/");
    handle = dlopen((plugin_dir + library).c_str(),
                    RTLD_GLOBAL | RTLD_NOW);
  }
  if (!handle) {
    ERROR_MSG("Plugin::load : failed to load {}: {}\n",
              library.c_str(),
              dlerror());
    return -1;
  }

  /*********************************************************************************
   * Apparently ISO C++ forbids against casting object pointer -> function
   *pointer * But what the hell do they know? It is probably safe here... *
   *********************************************************************************/

  std::unique_ptr<Modules::Plugin> (*create)(void) =
      (std::unique_ptr<Modules::Plugin> (*)(void))(dlsym(handle, "createRTXIPlugin"));
  if (!create) {
    ERROR_MSG("Plugin::load : failed to load {} : {}\n",
              library,
              dlerror());
    dlclose(handle);
    return -1;
  }

  auto plugin = create();
  if (plugin == nullptr) {
    ERROR_MSG("Plugin::load : failed to load {} : failed to create instance\n",
              library);
    dlclose(handle);
    return -1;
  }
  // if (plugin->magic_number != Plugin::Object::MAGIC_NUMBER) {
  //   ERROR_MSG(
  //       "Plugin::load : the pointer returned from {}::createRTXIPlugin() isn't "
  //       "a valid Plugin::Object *.\n",
  //       library.toStdString().c_str());
  //   dlclose(handle);
  //   return 0;
  // }

  this->registerModule(std::move(plugin));
  return 0;
}

void Modules::Manager::unloadPlugin(const std::string& library)
{
  if(this->rtxi_modules_registry.find(library) != this->rtxi_modules_registry.end()){
    void* handle = this->rtxi_modules_registry[library]->getHandle();
    if(handle != nullptr) { dlclose(handle); }
  }
}

int Modules::Manager::registerModule(std::unique_ptr<Modules::Plugin> module)
{
  this->rtxi_modules_registry[module->getName()] = std::move(module);
}

int Modules::Manager::unregisterModule(std::string module_name)
{
  if(this->rtxi_modules_registry.find(module_name) != this->rtxi_modules_registry.end()){
    this->rtxi_modules_registry.erase(module_name);
  }
}

void Modules::Manager::receiveEvent(Event::Object* event)
{

}
