
#include <QApplication>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QScrollArea>
#include <QTimer>
#include <algorithm>
#include <any>
#include <memory>
#include <sstream>

#include "module.hpp"

#include <dlfcn.h>
#include <qmdisubwindow.h>

#include "connector/connector.h"
#include "debug.hpp"
#include "dlplugin.hpp"
#include "oscilloscope/oscilloscope.h"
#include "performance_measurement/performance_measurement.hpp"
#include "rtxiConfig.h"
#include "system_control/system_control.h"
#include "userprefs/userprefs.h"

std::string Modules::Variable::state2string(Modules::Variable::state_t state)
{
  std::string result;
  switch (state) {
    case Modules::Variable::INIT:
      result = std::string("INIT");
      break;
    case Modules::Variable::MODIFY:
      result = std::string("MODIFIED PARAMETERS");
      break;
    case Modules::Variable::PERIOD:
      result = std::string("PERIOD CHANGE");
      break;
    case Modules::Variable::PAUSE:
      result = std::string("PLUGIN PAUSED");
      break;
    case Modules::Variable::UNPAUSE:
      result = std::string("PLUGIN UNPAUSED");
      break;
    case Modules::Variable::EXIT:
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

Modules::Component::Component(
    Modules::Plugin* hplugin,
    const std::string& mod_name,
    const std::vector<IO::channel_t>& channels,
    const std::vector<Modules::Variable::Info>& variables)
    : RT::Thread(mod_name, channels)
    , hostPlugin(hplugin)
    , active(false)
{
  for (const auto& var : variables) {
    if (var.id != parameters.size()) {
      ERROR_MSG("Error parsing variables in module \"{}\" while loading",
                this->getName());
      ERROR_MSG("Variable {} has id {} but was inserted in position {}",
                var.name,
                var.id,
                parameters.size());
      return;
    }
    this->parameters.push_back(var);
  }
}

std::string Modules::Component::getDescription(const size_t& var_id)
{
  return this->parameters[var_id].description;
}

std::string Modules::Component::getValueString(const size_t& var_id)
{
  std::string value;
  switch (this->parameters[var_id].vartype) {
    case Modules::Variable::UINT_PARAMETER:
      value =
          std::to_string(std::get<uint64_t>(this->parameters[var_id].value));
      break;
    case Modules::Variable::INT_PARAMETER:
      value = std::to_string(std::get<int64_t>(this->parameters[var_id].value));
      break;
    case Modules::Variable::DOUBLE_PARAMETER:
      value = std::to_string(std::get<double>(this->parameters[var_id].value));
      break;
    case Modules::Variable::STATE:
      value = "";
      break;
    case Modules::Variable::COMMENT:
      value = std::get<std::string>(this->parameters[var_id].value);
      break;
    case Modules::Variable::UNKNOWN:
      value = "UNKNOWN";
      break;
    default:
      value = "ERROR";
      break;
  }
  return value;
}

Modules::Panel::Panel(const std::string& mod_name,
                      QMainWindow* mw,
                      Event::Manager* ev_manager)
    : QWidget(mw)
    , main_window(mw)
    , m_name(mod_name)
    , event_manager(ev_manager)
{
  setWindowTitle(QString::fromStdString(mod_name));
  
  auto* central_widget = dynamic_cast<QMdiArea*>(mw->centralWidget());
  this->m_subwindow = central_widget->addSubWindow(this);
  this->m_subwindow->setWindowIcon(
      QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint
                             | Qt::WindowMinimizeButtonHint);
  auto* timer = new QTimer(this);
  timer->setTimerType(Qt::PreciseTimer);
  timer->start(1000);
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));
}

void Modules::Panel::closeEvent(QCloseEvent* event)
{
  this->exit();
  event->accept();
}

void Modules::Panel::createGUI(const std::vector<Modules::Variable::Info>& vars,
                               QMainWindow* mw)
{
  // Make Mdi
  this->m_subwindow->setAttribute(Qt::WA_DeleteOnClose);
  // m_subwindow->setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
  this->m_subwindow->setWindowFlags(Qt::CustomizeWindowHint
                                    | Qt::WindowCloseButtonHint
                                    | Qt::WindowMinimizeButtonHint);
  this->m_subwindow->setOption(QMdiSubWindow::RubberBandResize, true);
  this->m_subwindow->setOption(QMdiSubWindow::RubberBandMove, true);

  // Create main layout
  this->m_layout = new QGridLayout;

  // Create child widget and gridLayout
  auto* gridArea = new QScrollArea;
  this->gridBox = new QWidget;
  gridArea->setWidget(this->gridBox);
  gridArea->ensureWidgetVisible(this->gridBox, 0, 0);
  gridArea->setWidgetResizable(true);
  auto* gridLayout = new QGridLayout;

  for (const auto& varinfo : vars) {
    param_t param;

    param.label = new QLabel(QString::fromStdString(varinfo.name), gridBox);
    gridLayout->addWidget(
        param.label, static_cast<int>(this->parameter.size()), 0);
    gridLayout->addWidget(
        param.edit, static_cast<int>(this->parameter.size()), 1);

    param.label->setToolTip(QString::fromStdString(varinfo.description));
    param.edit->setToolTip(QString::fromStdString(varinfo.description));

    param.str_value = QString();
    param.label = new QLabel;
    param.type = varinfo.vartype;
    param.info = varinfo;
    switch (varinfo.vartype) {
      case Modules::Variable::DOUBLE_PARAMETER:
        param.edit->setValidator(new QDoubleValidator(param.edit));
        break;
      case Modules::Variable::UINT_PARAMETER:
      case Modules::Variable::INT_PARAMETER:
        param.edit->setValidator(new QIntValidator(param.edit));
        break;
      case Modules::Variable::STATE:
        param.edit->setReadOnly(true);
        palette.setBrush(param.edit->foregroundRole(), Qt::darkGray);
        param.edit->setPalette(palette);
        break;
      case Modules::Variable::COMMENT:
        break;
      case Modules::Variable::UNKNOWN:
        ERROR_MSG("Variable {} in Module {} is of category UNKNOWN",
                  varinfo.name,
                  this->getName());
        break;
      default:
        ERROR_MSG("Variable {} in Module {} has undefined or broken category",
                  varinfo.name,
                  this->getName());
    }
    parameter[QString::fromStdString(varinfo.name)] = param;
  }

  // Create child widget
  buttonGroup = new QGroupBox;
  auto* buttonLayout = new QHBoxLayout;

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
  m_layout->addWidget(gridArea, 1, 0);

  // Attempt to put these at the bottom at all times
  m_layout->addWidget(buttonGroup, 10, 0);

  // Set layout to Mdi and show
  setLayout(m_layout);
  m_subwindow->setWidget(this);
  m_subwindow->show();
}

void Modules::Panel::update(Modules::Variable::state_t /*unused*/)
{
  // TODO: Update function needs to have default functionality for derived
  // classes
}

void Modules::Panel::resizeMe()
{
  m_subwindow->adjustSize();
}

void Modules::Panel::exit()
{
  this->event_manager->unregisterHandler(this->hostPlugin);
  Event::Object event(Event::Type::PLUGIN_REMOVE_EVENT);
  event.setParam("pluginPointer",
                 std::any(static_cast<Modules::Plugin*>(this->hostPlugin)));
  this->event_manager->postEvent(&event);
  // this->m_subwindow->close();
}

void Modules::Panel::refresh()
{
  Modules::Variable::Id param_id = Modules::Variable::INVALID_ID;
  double double_value = 0.0;
  int64_t int_value = 0;
  uint64_t uint_value = 0ULL;
  std::stringstream sstream;
  for (auto i : this->parameter) {
    switch (i.second.type) {
      case Modules::Variable::STATE:
        i.second.edit->setText(i.second.str_value);
        palette.setBrush(i.second.edit->foregroundRole(), Qt::darkGray);
        i.second.edit->setPalette(palette);
        break;
      case Modules::Variable::UINT_PARAMETER:
        param_id = static_cast<Modules::Variable::Id>(i.second.info.id);
        uint_value = this->hostPlugin->getComponentUIntParameter(param_id);
        sstream << uint_value;
        i.second.edit->setText(QString::fromStdString(sstream.str()));
        break;
      case Modules::Variable::INT_PARAMETER:
        param_id = static_cast<Modules::Variable::Id>(i.second.info.id);
        int_value = this->hostPlugin->getComponentIntParameter(param_id);
        sstream << int_value;
        i.second.edit->setText(QString::fromStdString(sstream.str()));
        break;
      case Modules::Variable::DOUBLE_PARAMETER:
        param_id = static_cast<Modules::Variable::Id>(i.second.info.id);
        double_value = this->hostPlugin->getComponentDoubleParameter(param_id);
        sstream << double_value;
        i.second.edit->setText(QString::fromStdString(sstream.str()));
        break;
      default:
        ERROR_MSG("Unable to determine refresh type for component {}",
                  this->getName());
    }
  }

  // Make sure we actually have a pauseButton object (default constructed)
  if (this->pauseButton != nullptr) {
    pauseButton->setChecked(!(this->hostPlugin->getActive()));
  }
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

// NOLINTNEXTLINE
void Modules::Panel::setComment(const QString& var_name, const QString& comment)
{
  auto n = parameter.find(var_name);
  if (n != parameter.end() && (n->second.type == Modules::Variable::COMMENT)) {
    n->second.edit->setText(comment);
    const QByteArray textData = comment.toLatin1();
    const char* text = textData.constData();
    auto param_id = static_cast<Modules::Variable::Id>(n->second.info.id);
    this->hostPlugin->setComponentParameter<std::string>(param_id, text);
  }
}

void Modules::Panel::setParameter(const QString& var_name, double value)
{
  auto n = parameter.find(var_name);
  if ((n != parameter.end())
      && (n->second.type == Modules::Variable::DOUBLE_PARAMETER))
  {
    n->second.edit->setText(QString::number(value));
    n->second.str_value = n->second.edit->text();
    auto param_id = static_cast<Modules::Variable::Id>(n->second.info.id);
    this->hostPlugin->setComponentParameter<double>(param_id, value);
    // setValue(n->second.index, n->second.edit->text().toDouble());
  }
}

void Modules::Panel::setParameter(const QString& var_name, int value)
{
  auto n = parameter.find(var_name);
  if ((n != parameter.end())
      && (n->second.type == Modules::Variable::INT_PARAMETER))
  {
    n->second.edit->setText(QString::number(value));
    n->second.str_value = n->second.edit->text();
    auto param_id = static_cast<Modules::Variable::Id>(n->second.info.id);
    this->hostPlugin->setComponentParameter<int>(param_id, value);
    // setValue(n->second.index, n->second.edit->text().toDouble());
  }
}

void Modules::Panel::setParameter(const QString& var_name, uint64_t value)
{
  auto n = parameter.find(var_name);
  if ((n != parameter.end())
      && (n->second.type == Modules::Variable::UINT_PARAMETER))
  {
    n->second.edit->setText(QString::number(value));
    n->second.str_value = n->second.edit->text();
    auto param_id = static_cast<Modules::Variable::Id>(n->second.info.id);
    this->hostPlugin->setComponentParameter<uint64_t>(param_id, value);
    // setValue(n->second.index, n->second.edit->text().toDouble());
  }
}

void Modules::Panel::setState(const QString& var_name,
                              Modules::Variable::state_t ref)
{
  auto n = parameter.find(var_name);
  if ((n != parameter.end()) && (n->second.type == Modules::Variable::STATE)) {
    // setData(Workspace::STATE, n->second.index, &ref);

    n->second.edit->setText(QString::number(ref));
    auto param_id = static_cast<Modules::Variable::Id>(n->second.info.id);
    this->hostPlugin->setComponentParameter<Modules::Variable::state_t>(
        param_id, ref);
  }
}

void Modules::Panel::pause(bool p)
{
  if (pauseButton->isChecked() != p) {
    pauseButton->setDown(p);
  }
  const int result = this->hostPlugin->setActive(!p);
  if (result != 0) {
    ERROR_MSG("Unable to pause/Unpause Plugin {} ", this->getName());
    return;
  }
  if (p) {
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

Modules::Plugin::Plugin(Event::Manager* ev_manager,
                        std::string mod_name)
    : event_manager(ev_manager)
    , name(std::move(mod_name))
{
}

Modules::Plugin::~Plugin()
{
  if (this->plugin_component != nullptr) {
    Event::Object unplug_block_event(Event::Type::RT_THREAD_REMOVE_EVENT);
    unplug_block_event.setParam(
        "thread",
        std::any(static_cast<RT::Thread*>(this->plugin_component.get())));
    this->event_manager->postEvent(&unplug_block_event);
  }
}

void Modules::Plugin::registerComponent()
{
  if (this->plugin_component == nullptr) {
    return;
  }
  Event::Object event = Event::Object(Event::Type::RT_THREAD_INSERT_EVENT);
  event.setParam("thread",
                 static_cast<RT::Thread*>(this->plugin_component.get()));
  this->event_manager->postEvent(&event);
}

void Modules::Plugin::attachComponent(
    std::unique_ptr<Modules::Component> component)
{
  this->plugin_component = std::move(component);
  this->registerComponent();
}

void Modules::Plugin::attachPanel(Modules::Panel* panel)
{
  this->widget_panel = panel;
  panel->setHostPlugin(this);
}

int64_t Modules::Plugin::getComponentIntParameter(
    const Modules::Variable::Id& parameter_id)
{
  return this->plugin_component->getValue<int64_t>(parameter_id);
}

uint64_t Modules::Plugin::getComponentUIntParameter(
    const Modules::Variable::Id& parameter_id)
{
  return this->plugin_component->getValue<uint64_t>(parameter_id);
}

double Modules::Plugin::getComponentDoubleParameter(
    const Modules::Variable::Id& parameter_id)
{
  return this->plugin_component->getValue<double>(parameter_id);
}

void Modules::Plugin::receiveEvent(Event::Object* /*event*/)
{
  // Set by users who want to handle events
}

bool Modules::Plugin::getActive()
{
  bool active = false;
  // some plugins may not have a valid component pointer
  if (this->plugin_component != nullptr) {
    active = this->plugin_component->getActive();
  }
  return active;
}

int Modules::Plugin::setActive(bool state)
{
  const int result = 0;
  Event::Type event_type = Event::Type::NOOP;
  if (state) {
    event_type = Event::Type::RT_THREAD_UNPAUSE_EVENT;
  } else {
    event_type = Event::Type::RT_THREAD_PAUSE_EVENT;
  }
  Event::Object event(event_type);
  event.setParam(
      "thread",
      std::any(static_cast<RT::Thread*>(this->plugin_component.get())));
  this->event_manager->postEvent(&event);
  return result;
}

Modules::Component* Modules::Plugin::getComponent()
{
  return this->plugin_component.get();
}

DAQ::Device* Modules::Plugin::getDevice()
{
  return this->plugin_device.get();
}

Event::Manager* Modules::Plugin::getEventManager()
{
  return this->event_manager;
}

Modules::Panel* Modules::Plugin::getPanel()
{
  return this->widget_panel;
}

Modules::Manager::Manager(Event::Manager* ev_manager)
    : event_manager(ev_manager)
{
  this->event_manager->registerHandler(this);
  this->m_plugin_loader = std::make_unique<DLL::Loader>();
}

Modules::Manager::~Manager()
{
  for (const auto& plugin_list : this->rtxi_modules_registry) {
    for (const auto& plugin : plugin_list.second) {
      this->event_manager->unregisterHandler(plugin.get());
    }
  }
  this->event_manager->unregisterHandler(this);
}

bool Modules::Manager::isRegistered(const Modules::Plugin* plugin)
{
  const std::string plugin_name = plugin->getName();
  auto start_iter = this->rtxi_modules_registry[plugin_name].begin();
  auto end_iter = this->rtxi_modules_registry[plugin_name].end();
  return std::any_of(start_iter,
                     end_iter,
                     [plugin](const std::unique_ptr<Modules::Plugin>& module)
                     { return plugin == module.get(); });
}

Modules::Plugin* Modules::Manager::loadCorePlugin(const std::string& library)
{
  Modules::Plugin* plugin_ptr = nullptr;
  Modules::FactoryMethods fact_methods;
  if (library == std::string(PerformanceMeasurement::MODULE_NAME)) {
    fact_methods = PerformanceMeasurement::getFactories();
  } else if (library == std::string(UserPrefs::MODULE_NAME)) {
    fact_methods = UserPrefs::getFactories();
  } else if (library == std::string(SystemControl::MODULE_NAME)) {
    fact_methods = SystemControl::getFactories();
  } else if (library == std::string(Connector::MODULE_NAME)) {
    fact_methods = Connector::getFactories();
  } else if (library == std::string(Oscilloscope::MODULE_NAME)) {
    fact_methods = Oscilloscope::getFactories();
  } else {
    return nullptr;
  }

  std::unique_ptr<Modules::Plugin> plugin;
  this->registerFactories(library, fact_methods);
  plugin = this->rtxi_factories_registry[library].createPlugin(event_manager);
  plugin_ptr = this->registerModule(std::move(plugin));
  return plugin_ptr;
}

// TODO: extract plugin dynamic loading to another class
Modules::Plugin* Modules::Manager::loadPlugin(const std::string& library)
{
  std::string library_loc = library;
  Modules::Plugin* plugin_ptr = nullptr;
  // if module factory is already registered then all we have to do is run it
  if (this->rtxi_factories_registry.find(library_loc)
      != this->rtxi_factories_registry.end())
  {
    std::unique_ptr<Modules::Plugin> plugin =
        this->rtxi_factories_registry[library_loc].createPlugin(
            this->event_manager);
    plugin_ptr = this->registerModule(std::move(plugin));
    return plugin_ptr;
  }

  // If it is just a core plugin then handle that elsewhere and return
  plugin_ptr = this->loadCorePlugin(library_loc);
  if (plugin_ptr != nullptr) {
    return plugin_ptr;
  }

  int result = this->m_plugin_loader->load(library_loc.c_str());
  if (result != 0) {
    // We try to load it from another location besides locally
    ERROR_MSG("Modules::Plugin::loadPlugin : could not load module locally");
    library_loc = RTXI_DEFAULT_PLUGIN_DIR + library;
    result = this->m_plugin_loader->load(library_loc.c_str());
  }
  if (result != 0) {
    ERROR_MSG("Plugin::load : failed to load {}", library_loc.c_str());
    return nullptr;
  }

  auto gen_fact_methods =
      this->m_plugin_loader->dlsym<Modules::FactoryMethods* (*)()>(
          library_loc.c_str(), "getFactories");

  if (gen_fact_methods == nullptr) {
    ERROR_MSG("Plugin::load : failed to retreive getFactories symbol");
    // If we got here it means we loaded the lirbary but not the symbol.
    // Let's just unload the library and exit before we regret it.
    this->m_plugin_loader->unload(library_loc.c_str());
    return nullptr;
  }

  Modules::FactoryMethods* fact_methods = gen_fact_methods();
  this->rtxi_factories_registry[library] = *fact_methods;
  std::unique_ptr<Modules::Plugin> plugin =
      fact_methods->createPlugin(this->event_manager);
  if (plugin == nullptr) {
    ERROR_MSG("Plugin::load : failed to create plugin from library {} ",
              library);
    this->m_plugin_loader->unload(library_loc.c_str());
    return nullptr;
  }

  // if (plugin->magic_number != Plugin::Object::MAGIC_NUMBER) {
  //   ERROR_MSG(
  //       "Plugin::load : the pointer returned from {}::createRTXIPlugin()
  //       isn't " "a valid Plugin::Object *.\n",
  //       library.toStdString().c_str());
  //   dlclose(handle);
  //   return 0;
  // }
  plugin_ptr = this->registerModule(std::move(plugin));
  return plugin_ptr;
}

void Modules::Manager::unloadPlugin(Modules::Plugin* plugin)
{
  const std::string library = plugin->getName();
  this->unregisterModule(plugin);
  if (this->rtxi_modules_registry[library].empty()) {
    this->unregisterFactories(library);
  }
}

Modules::Plugin* Modules::Manager::registerModule(
    std::unique_ptr<Modules::Plugin> module)
{
  std::unique_lock<std::mutex> lk(this->m_modules_mut);
  const std::string mod_name = module->getName();
  this->rtxi_modules_registry[mod_name].push_back(std::move(module));
  return this->rtxi_modules_registry[mod_name].back().get();
}

void Modules::Manager::unregisterModule(Modules::Plugin* plugin)
{
  if (plugin == nullptr) {
    return;
  }
  std::unique_lock<std::mutex> lk(this->m_modules_mut);
  const std::string plugin_name = plugin->getName();
  auto start_iter = this->rtxi_modules_registry[plugin_name].begin();
  auto end_iter = this->rtxi_modules_registry[plugin_name].end();
  auto loc =
      std::find_if(start_iter,
                   end_iter,
                   [plugin](const std::unique_ptr<Modules::Plugin>& module)
                   { return plugin == module.get(); });
  if (loc == end_iter) {
    return;
  }
  this->m_plugin_loader->unload(plugin->getLibrary().c_str());
  this->rtxi_modules_registry[plugin_name].erase(loc);
}

void Modules::Manager::registerFactories(const std::string& module_name,
                                         Modules::FactoryMethods fact)
{
  this->rtxi_factories_registry[module_name] = fact;
}

void Modules::Manager::unregisterFactories(const std::string& module_name)
{
  if (this->rtxi_factories_registry.find(module_name)
      != this->rtxi_factories_registry.end())
  {
    this->rtxi_factories_registry.erase(module_name);
  }
}

void Modules::Manager::receiveEvent(Event::Object* event)
{
  std::string plugin_name;
  Modules::Plugin* plugin_ptr = nullptr;
  switch (event->getType()) {
    case Event::Type::PLUGIN_REMOVE_EVENT:
      plugin_ptr =
          std::any_cast<Modules::Plugin*>(event->getParam("pluginPointer"));
      this->unloadPlugin(plugin_ptr);
      break;
    case Event::Type::PLUGIN_INSERT_EVENT:
      plugin_name = std::any_cast<std::string>(event->getParam("pluginName"));
      plugin_ptr = this->loadPlugin(plugin_name);
      if (plugin_ptr != nullptr) {
        event->setParam("status", std::any(std::string("success")));
        event->setParam(
            "createRTXIPanel",
            std::any(this->rtxi_factories_registry[plugin_name].createPanel));
        event->setParam("pluginPointer", std::any(plugin_ptr));
      } else {
        event->setParam("status", std::any(std::string("failure")));
      }
      break;
    default:
      return;
  }
}
