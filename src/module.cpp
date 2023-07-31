
#include <QApplication>
#include <QCloseEvent>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QMdiArea>
#include <QScrollArea>
#include <QTimer>
#include <algorithm>
#include <any>
#include <memory>
#include <sstream>

#include "module.hpp"

#include <dlfcn.h>
#include <qmdisubwindow.h>

#include "debug.hpp"
#include "rtxiConfig.h"

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

std::string Modules::Variable::vartype2string(
    Modules::Variable::variable_t type)
{
  std::string result;
  switch (type) {
    case Modules::Variable::INT_PARAMETER:
      result = std::string("INTEGER");
      break;
    case Modules::Variable::DOUBLE_PARAMETER:
      result = std::string("DOUBLE");
      break;
    case Modules::Variable::UINT_PARAMETER:
      result = std::string("UNSIGNED INTEGER");
      break;
    case Modules::Variable::STATE:
      result = std::string("STATE");
      break;
    default:
      result = std::string("UNKNOWN PARAMETER TYPE");
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
  event->accept();
  this->exit();
}

void Modules::Panel::createGUI(
    const std::vector<Modules::Variable::Info>& vars,
    const std::vector<Modules::Variable::Id>& skip_ids)
{
  // Create main layout
  auto* main_layout = new QVBoxLayout;

  // Create child widget and it's layout
  auto* customParamArea = new QGroupBox;
  auto* customParamLayout = new QGridLayout;
  int param_count = 0;
  for (const auto& varinfo : vars) {
    // Skip any unwanted ids
    if (std::count(skip_ids.begin(), skip_ids.end(), varinfo.id) != 0) {
      continue;
    }
    param_t param;
    param.label =
        new QLabel(QString::fromStdString(varinfo.name), customParamArea);
    param.edit = new DefaultGUILineEdit(customParamArea);
    param.str_value = QString();
    param.type = varinfo.vartype;
    param.info = varinfo;
    switch (varinfo.vartype) {
      case Modules::Variable::DOUBLE_PARAMETER:
        param.edit->setValidator(new QDoubleValidator(param.edit));
        param.edit->setText(QString::number(std::get<double>(varinfo.value)));
        break;
      case Modules::Variable::UINT_PARAMETER:
        param.edit->setValidator(new QIntValidator(param.edit));
        param.edit->setText(QString::number(std::get<uint64_t>(varinfo.value)));
        break;
      case Modules::Variable::INT_PARAMETER:
        param.edit->setValidator(new QIntValidator(param.edit));
        param.edit->setText(QString::number(std::get<int64_t>(varinfo.value)));
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
    param.label->setToolTip(QString::fromStdString(varinfo.description));
    param.edit->setToolTip(QString::fromStdString(varinfo.description));
    param.str_value = param.edit->text();
    parameter[QString::fromStdString(varinfo.name)] = param;
    customParamLayout->addWidget(param.label, param_count, 0);
    customParamLayout->addWidget(param.edit, param_count, 1);
    param_count++;
  }

  customParamArea->setLayout(customParamLayout);
  // Create child widget
  auto* buttonGroup = new QGroupBox;
  auto* buttonLayout = new QHBoxLayout;

  // Create elements
  pauseButton = new QPushButton("Pause", this);
  pauseButton->setCheckable(true);
  QObject::connect(pauseButton, SIGNAL(toggled(bool)), this, SLOT(pause(bool)));
  buttonLayout->addWidget(pauseButton);

  modifyButton = new QPushButton("Modify", this);
  QObject::connect(modifyButton, SIGNAL(clicked()), this, SLOT(modify()));
  buttonLayout->addWidget(modifyButton);

  unloadButton = new QPushButton("Unload", this);
  QObject::connect(
      unloadButton, SIGNAL(clicked()), parentWidget(), SLOT(close()));
  buttonLayout->addWidget(unloadButton);

  buttonGroup->setLayout(buttonLayout);

  main_layout->addWidget(customParamArea, 0);

  main_layout->addWidget(buttonGroup, 1);

  this->setLayout(main_layout);
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
  for (auto& i : this->parameter) {
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
  // const int result = this->hostPlugin->setActive(!p);
  // if (result != 0) {
  //   ERROR_MSG("Unable to pause/Unpause Plugin {} ", this->getName());
  //   return;
  // }
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

Modules::Plugin::Plugin(Event::Manager* ev_manager, std::string mod_name)
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
  // If there is a component already attached then cancel attach
  if (this->plugin_component != nullptr) {
    return;
  }
  this->plugin_component = std::move(component);
  // Let's set the component as active before registering it to rt thread
  // this avoids unecessary use of event firing which is much slower
  this->plugin_component->setActive(/*act=*/true);
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
