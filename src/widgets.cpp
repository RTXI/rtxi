
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

#include "widgets.hpp"

#include <dlfcn.h>
#include <qmdisubwindow.h>

#include "debug.hpp"
#include "rtxiConfig.h"

std::string Widgets::Variable::state2string(RT::State::state_t state)
{
  std::string result;
  switch (state) {
    case RT::State::INIT:
      result = std::string("INIT");
      break;
    case RT::State::MODIFY:
      result = std::string("MODIFIED PARAMETERS");
      break;
    case RT::State::PERIOD:
      result = std::string("PERIOD CHANGE");
      break;
    case RT::State::PAUSE:
      result = std::string("PLUGIN PAUSED");
      break;
    case RT::State::UNPAUSE:
      result = std::string("PLUGIN UNPAUSED");
      break;
    case RT::State::EXIT:
      result = std::string("EXIT");
      break;
    default:
      result = std::string("UNKNOWN STATE");
      break;
  }
  return result;
}

std::string Widgets::Variable::vartype2string(
    Widgets::Variable::variable_t type)
{
  std::string result;
  switch (type) {
    case Widgets::Variable::INT_PARAMETER:
      result = std::string("INTEGER");
      break;
    case Widgets::Variable::DOUBLE_PARAMETER:
      result = std::string("DOUBLE");
      break;
    case Widgets::Variable::UINT_PARAMETER:
      result = std::string("UNSIGNED INTEGER");
      break;
    case Widgets::Variable::STATE:
      result = std::string("STATE");
      break;
    default:
      result = std::string("UNKNOWN PARAMETER TYPE");
      break;
  }
  return result;
}

Widgets::DefaultGUILineEdit::DefaultGUILineEdit(QWidget* parent)
    : QLineEdit(parent)
{
  QObject::connect(
      this, SIGNAL(textChanged(const QString&)), this, SLOT(redden()));
}

void Widgets::DefaultGUILineEdit::blacken()
{
  palette.setBrush(this->foregroundRole(),
                   QApplication::palette().color(QPalette::WindowText));
  this->setPalette(palette);
  setModified(false);
}

void Widgets::DefaultGUILineEdit::redden()
{
  if (isModified()) {
    palette.setBrush(this->foregroundRole(), Qt::red);
    this->setPalette(palette);
  }
}

Widgets::Component::Component(
    Widgets::Plugin* hplugin,
    const std::string& mod_name,
    const std::vector<IO::channel_t>& channels,
    const std::vector<Widgets::Variable::Info>& variables)
    : RT::Thread(mod_name, channels)
    , hostPlugin(hplugin)
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

std::string Widgets::Component::getDescription(const size_t& var_id)
{
  return this->parameters[var_id].description;
}

std::string Widgets::Component::getValueString(const size_t& var_id)
{
  std::string value;
  switch (this->parameters[var_id].vartype) {
    case Widgets::Variable::UINT_PARAMETER:
      value =
          std::to_string(std::get<uint64_t>(this->parameters[var_id].value));
      break;
    case Widgets::Variable::INT_PARAMETER:
      value = std::to_string(std::get<int64_t>(this->parameters[var_id].value));
      break;
    case Widgets::Variable::DOUBLE_PARAMETER:
      value = std::to_string(std::get<double>(this->parameters[var_id].value));
      break;
    case Widgets::Variable::STATE:
      value = "";
      break;
    case Widgets::Variable::COMMENT:
      value = std::get<std::string>(this->parameters[var_id].value);
      break;
    case Widgets::Variable::UNKNOWN:
      value = "UNKNOWN";
      break;
    default:
      value = "ERROR";
      break;
  }
  return value;
}

Widgets::Panel::Panel(const std::string& mod_name,
                      QMainWindow* mw,
                      Event::Manager* ev_manager)
    : QWidget(mw)
    , main_window(mw)
    , m_name(mod_name)
    , event_manager(ev_manager)
{
  setWindowTitle(QString(mod_name.c_str()));

  auto* central_widget = dynamic_cast<QMdiArea*>(mw->centralWidget());
  this->m_subwindow = central_widget->addSubWindow(this);
  this->m_subwindow->setWindowIcon(
      QIcon("/usr/share/rtxi/RTXI-widget-icon.png"));
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint
                       | Qt::WindowMinimizeButtonHint);
  qRegisterMetaType<RT::State::state_t>();
  QObject::connect(this,
                   &Widgets::Panel::signal_state_change,
                   this,
                   &Widgets::Panel::update_state);
}

void Widgets::Panel::closeEvent(QCloseEvent* event)
{
  event->accept();
  this->exit();
}

void Widgets::Panel::createGUI(
    const std::vector<Widgets::Variable::Info>& vars,
    const std::vector<Widgets::Variable::Id>& skip_ids)
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
    param.label = new QLabel(QString(varinfo.name.c_str()), customParamArea);
    param.edit = new DefaultGUILineEdit(customParamArea);
    param.str_value = QString();
    param.type = varinfo.vartype;
    param.info = varinfo;
    switch (varinfo.vartype) {
      case Widgets::Variable::DOUBLE_PARAMETER:
        param.edit->setValidator(new QDoubleValidator(param.edit));
        param.edit->setText(QString::number(std::get<double>(varinfo.value)));
        break;
      case Widgets::Variable::UINT_PARAMETER:
        param.edit->setValidator(new QIntValidator(param.edit));
        param.edit->setText(QString::number(std::get<uint64_t>(varinfo.value)));
        break;
      case Widgets::Variable::INT_PARAMETER:
        param.edit->setValidator(new QIntValidator(param.edit));
        param.edit->setText(QString::number(std::get<int64_t>(varinfo.value)));
        break;
      case Widgets::Variable::STATE:
        param.edit->setReadOnly(true);
        palette.setBrush(param.edit->foregroundRole(), Qt::darkGray);
        param.edit->setPalette(palette);
        break;
      case Widgets::Variable::COMMENT:
        break;
      case Widgets::Variable::UNKNOWN:
        ERROR_MSG("Variable {} in Widget {} is of category UNKNOWN",
                  varinfo.name,
                  this->getName());
        break;
      default:
        ERROR_MSG("Variable {} in Widget {} has undefined or broken category",
                  varinfo.name,
                  this->getName());
    }
    param.label->setToolTip(QString(varinfo.description.c_str()));
    param.edit->setToolTip(QString(varinfo.description.c_str()));
    param.str_value = param.edit->text();
    parameter[varinfo.name] = param;
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

void Widgets::Panel::update_state(RT::State::state_t flag)
{
  Widgets::Plugin* hplugin = this->getHostPlugin();
  hplugin->setComponentState(flag);
}

void Widgets::Panel::resizeMe()
{
  m_subwindow->adjustSize();
}

void Widgets::Panel::exit()
{
  this->event_manager->unregisterHandler(this->hostPlugin);
  Event::Object event(Event::Type::PLUGIN_REMOVE_EVENT);
  event.setParam("pluginPointer",
                 std::any(static_cast<Widgets::Plugin*>(this->hostPlugin)));
  this->event_manager->postEvent(&event);
}

void Widgets::Panel::refresh()
{
  if (!this->hostPlugin->hasComponent()) {
    return;
  }
  Widgets::Variable::Id param_id = Widgets::Variable::INVALID_ID;
  double double_value = 0.0;
  int64_t int_value = 0;
  uint64_t uint_value = 0ULL;
  std::stringstream sstream;
  for (auto& i : this->parameter) {
    switch (i.second.type) {
      case Widgets::Variable::STATE:
        i.second.edit->setText(i.second.str_value);
        palette.setBrush(i.second.edit->foregroundRole(), Qt::darkGray);
        i.second.edit->setPalette(palette);
        break;
      case Widgets::Variable::UINT_PARAMETER:
        param_id = static_cast<Widgets::Variable::Id>(i.second.info.id);
        uint_value = this->hostPlugin->getComponentUIntParameter(param_id);
        sstream << uint_value;
        i.second.edit->setText(QString(sstream.str().c_str()));
        break;
      case Widgets::Variable::INT_PARAMETER:
        param_id = static_cast<Widgets::Variable::Id>(i.second.info.id);
        int_value = this->hostPlugin->getComponentIntParameter(param_id);
        sstream << int_value;
        i.second.edit->setText(QString(sstream.str().c_str()));
        break;
      case Widgets::Variable::DOUBLE_PARAMETER:
        param_id = static_cast<Widgets::Variable::Id>(i.second.info.id);
        double_value = this->hostPlugin->getComponentDoubleParameter(param_id);
        sstream << double_value;
        i.second.edit->setText(QString(sstream.str().c_str()));
        break;
      default:
        ERROR_MSG("Unable to determine refresh type for component {}",
                  this->getName());
    }
  }

  // Make sure we actually have a pauseButton object (default constructed with
  // createGUI)
  if (this->pauseButton != nullptr) {
    pauseButton->setChecked(!(this->hostPlugin->getActive()));
  }
}

void Widgets::Panel::modify()
{
  Widgets::Variable::Id param_id = Widgets::Variable::INVALID_ID;
  double double_value = 0.0;
  int int_value = 0;
  uint64_t uint_value = 0ULL;
  std::stringstream sstream;
  this->update_state(RT::State::PAUSE);
  for (auto& var : this->parameter) {
    if (!var.second.edit->isModified()) {
      continue;
    }
    switch (var.second.type) {
      case Widgets::Variable::UINT_PARAMETER:
        param_id = static_cast<Widgets::Variable::Id>(var.second.info.id);
        uint_value = var.second.edit->text().toUInt();
        this->hostPlugin->setComponentParameter<uint64_t>(param_id, uint_value);
        sstream << uint_value;
        var.second.edit->setText(QString(sstream.str().c_str()));
        var.second.edit->blacken();
        break;
      case Widgets::Variable::INT_PARAMETER:
        param_id = static_cast<Widgets::Variable::Id>(var.second.info.id);
        int_value = var.second.edit->text().toInt();
        this->hostPlugin->setComponentParameter<int>(param_id, int_value);
        sstream << int_value;
        var.second.edit->setText(QString(sstream.str().c_str()));
        var.second.edit->blacken();
        break;
      case Widgets::Variable::DOUBLE_PARAMETER:
        param_id = static_cast<Widgets::Variable::Id>(var.second.info.id);
        double_value = var.second.edit->text().toDouble();
        this->hostPlugin->setComponentParameter(param_id, double_value);
        sstream << double_value;
        var.second.edit->setText(QString(sstream.str().c_str()));
        var.second.edit->blacken();
        break;
      default:
        ERROR_MSG("Unable to determine refresh type for component {}",
                  this->getName());
    }
  }
  this->update_state(RT::State::MODIFY);
  refresh();
}

// NOLINTNEXTLINE
void Widgets::Panel::setComment(const QString& var_name, const QString& comment)
{
  auto n = parameter.find(var_name.toStdString());
  if (n != parameter.end() && (n->second.type == Widgets::Variable::COMMENT)) {
    n->second.edit->setText(comment);
    const QByteArray textData = comment.toLatin1();
    const char* text = textData.constData();
    auto param_id = static_cast<Widgets::Variable::Id>(n->second.info.id);
    this->hostPlugin->setComponentParameter<std::string>(param_id, text);
  }
}

void Widgets::Panel::setParameter(const QString& var_name, double value)
{
  auto n = parameter.find(var_name.toStdString());
  if ((n != parameter.end())
      && (n->second.type == Widgets::Variable::DOUBLE_PARAMETER))
  {
    n->second.edit->setText(QString::number(value));
    n->second.str_value = n->second.edit->text();
    auto param_id = static_cast<Widgets::Variable::Id>(n->second.info.id);
    this->hostPlugin->setComponentParameter<double>(param_id, value);
  }
}

void Widgets::Panel::setParameter(const QString& var_name, int value)
{
  auto n = parameter.find(var_name.toStdString());
  if ((n != parameter.end())
      && (n->second.type == Widgets::Variable::INT_PARAMETER))
  {
    n->second.edit->setText(QString::number(value));
    n->second.str_value = n->second.edit->text();
    auto param_id = static_cast<Widgets::Variable::Id>(n->second.info.id);
    this->hostPlugin->setComponentParameter<int>(param_id, value);
  }
}

void Widgets::Panel::setParameter(const QString& var_name, uint64_t value)
{
  auto n = parameter.find(var_name.toStdString());
  if ((n != parameter.end())
      && (n->second.type == Widgets::Variable::UINT_PARAMETER))
  {
    n->second.edit->setText(QString::number(value));
    n->second.str_value = n->second.edit->text();
    auto param_id = static_cast<Widgets::Variable::Id>(n->second.info.id);
    this->hostPlugin->setComponentParameter<uint64_t>(param_id, value);
  }
}

void Widgets::Panel::pause(bool p)
{
  if (pauseButton->isChecked() != p) {
    pauseButton->setDown(p);
  }

  if (p) {
    this->update_state(RT::State::PAUSE);
  } else {
    this->update_state(RT::State::UNPAUSE);
  }
}

Widgets::Plugin::Plugin(Event::Manager* ev_manager, std::string mod_name)
    : event_manager(ev_manager)
    , name(std::move(mod_name))
{
}

Widgets::Plugin::~Plugin()
{
  if (this->plugin_component != nullptr) {
    Event::Object unplug_block_event(Event::Type::RT_THREAD_REMOVE_EVENT);
    unplug_block_event.setParam(
        "thread",
        std::any(static_cast<RT::Thread*>(this->plugin_component.get())));
    this->event_manager->postEvent(&unplug_block_event);
  }
}

void Widgets::Plugin::registerComponent()
{
  if (this->plugin_component == nullptr) {
    return;
  }
  Event::Object event = Event::Object(Event::Type::RT_THREAD_INSERT_EVENT);
  event.setParam("thread",
                 static_cast<RT::Thread*>(this->plugin_component.get()));
  this->event_manager->postEvent(&event);
}

void Widgets::Plugin::setComponentState(RT::State::state_t state)
{
  if (!this->plugin_component) {
    return;
  }
  Event::Object event(Event::Type::RT_WIDGET_STATE_CHANGE_EVENT);
  event.setParam(
      "component",
      std::any(static_cast<Widgets::Component*>(this->plugin_component.get())));
  event.setParam("state", std::any(state));
  this->event_manager->postEvent(&event);
}

std::vector<Widgets::Variable::Info>
Widgets::Plugin::getComponentParametersInfo()
{
  return this->plugin_component->getParametersInfo();
}

void Widgets::Plugin::attachComponent(
    std::unique_ptr<Widgets::Component> component)
{
  // If there is a component already attached or invalid
  // componnet pointer provided then cancel attach
  if (this->plugin_component != nullptr || component == nullptr) {
    return;
  }
  this->plugin_component = std::move(component);
  // Let's set the component as active before registering it to rt thread
  // this avoids unnecessary use of event firing which is much slower
  this->plugin_component->setActive(/*act=*/true);
  this->registerComponent();
}

void Widgets::Plugin::attachPanel(Widgets::Panel* panel)
{
  this->widget_panel = panel;
  panel->setHostPlugin(this);
}

int64_t Widgets::Plugin::getComponentIntParameter(
    const Widgets::Variable::Id& parameter_id)
{
  return this->plugin_component->getValue<int64_t>(parameter_id);
}

uint64_t Widgets::Plugin::getComponentUIntParameter(
    const Widgets::Variable::Id& parameter_id)
{
  return this->plugin_component->getValue<uint64_t>(parameter_id);
}

double Widgets::Plugin::getComponentDoubleParameter(
    const Widgets::Variable::Id& parameter_id)
{
  return this->plugin_component->getValue<double>(parameter_id);
}

void Widgets::Plugin::receiveEvent(Event::Object* event)
{
  // We provide base functionality for handling period changes
  switch (event->getType()) {
    case Event::Type::RT_PERIOD_EVENT:
      if (this->widget_panel != nullptr) {
        this->widget_panel->signal_state_change(RT::State::PERIOD);
      }
      break;
    default:
      break;
  }
}

bool Widgets::Plugin::getActive()
{
  bool active = false;
  // some plugins may not have a valid component pointer
  if (this->plugin_component != nullptr) {
    active = this->plugin_component->getActive();
  }
  return active;
}

int Widgets::Plugin::setActive(bool state)
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

Widgets::Component* Widgets::Plugin::getComponent()
{
  return this->plugin_component.get();
}

Event::Manager* Widgets::Plugin::getEventManager()
{
  return this->event_manager;
}

Widgets::Panel* Widgets::Plugin::getPanel()
{
  return this->widget_panel;
}
