
#include <QApplication>
#include <QCloseEvent>
#include <QDoubleValidator>
#include <QGroupBox>
#include <QIntValidator>
#include <QLabel>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <any>
#include <memory>

#include "widgets.hpp"

#include <dlfcn.h>

#include "debug.hpp"
#include "event.hpp"

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
  QObject::connect(this,
                   &DefaultGUILineEdit::textChanged,
                   this,
                   &DefaultGUILineEdit::redden);
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
      throw std::invalid_argument(
          "Invalid variable id used during plugin load");
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
    case Widgets::Variable::STATE:
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
        param.edit->setText(QString::number(std::get<uint64_t>(varinfo.value)));
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
  QObject::connect(
      pauseButton, &QPushButton::toggled, this, &Widgets::Panel::pause);
  buttonLayout->addWidget(pauseButton);

  modifyButton = new QPushButton("Modify", this);
  QObject::connect(
      modifyButton, &QPushButton::clicked, this, &Widgets::Panel::modify);
  buttonLayout->addWidget(modifyButton);

  unloadButton = new QPushButton("Unload", this);
  QObject::connect(unloadButton,
                   &QPushButton::clicked,
                   parentWidget(),
                   &Widgets::Panel::close);
  buttonLayout->addWidget(unloadButton);

  buttonGroup->setLayout(buttonLayout);

  main_layout->addWidget(customParamArea, 0);

  main_layout->addWidget(buttonGroup, 1);

  defaultPauseUpdateTimer = new QTimer(this);
  QObject::connect(defaultPauseUpdateTimer,
                   &QTimer::timeout,
                   this,
                   &Widgets::Panel::updatePauseButton);
  QObject::connect(defaultPauseUpdateTimer,
                   &QTimer::timeout,
                   this,
                   &Widgets::Panel::refreshUserStates);
  // the timer updates pause state every second
  defaultPauseUpdateTimer->start(1000);
  this->setLayout(main_layout);
}

void Widgets::Panel::updatePauseButton()
{
  if (hostPlugin == nullptr) {
    return;
  }
  const RT::State::state_t state = hostPlugin->getComponentState();
  if (state == RT::State::UNDEFINED) {
    defaultPauseUpdateTimer->stop();
    return;
  }
  const bool paused = state == RT::State::PAUSE;
  pauseButton->setDown(paused);
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
  this->hostPlugin = nullptr;
}

void Widgets::Panel::refresh()
{
  if (hostPlugin == nullptr || !this->hostPlugin->hasComponent()) {
    return;
  }
  Widgets::Variable::Id param_id = Widgets::Variable::INVALID_ID;
  double double_value = 0.0;
  int64_t int_value = 0;
  uint64_t uint_value = 0ULL;
  for (auto& i : this->parameter) {
    switch (i.second.type) {
      case Widgets::Variable::STATE:
        param_id = static_cast<Widgets::Variable::Id>(i.second.info.id);
        uint_value = this->hostPlugin->getComponentUIntParameter(param_id);
        i.second.edit->setText(QString::number(uint_value));
        palette.setBrush(i.second.edit->foregroundRole(), Qt::darkGray);
        i.second.edit->setPalette(palette);
        break;
      case Widgets::Variable::UINT_PARAMETER:
        param_id = static_cast<Widgets::Variable::Id>(i.second.info.id);
        uint_value = this->hostPlugin->getComponentUIntParameter(param_id);
        i.second.edit->setText(QString::number(uint_value));
        break;
      case Widgets::Variable::INT_PARAMETER:
        param_id = static_cast<Widgets::Variable::Id>(i.second.info.id);
        int_value = this->hostPlugin->getComponentIntParameter(param_id);
        i.second.edit->setText(QString::number(int_value));
        break;
      case Widgets::Variable::DOUBLE_PARAMETER:
        param_id = static_cast<Widgets::Variable::Id>(i.second.info.id);
        double_value = this->hostPlugin->getComponentDoubleParameter(param_id);
        i.second.edit->setText(QString::number(double_value));
        break;
      default:
        ERROR_MSG("Unable to determine refresh type for component {}",
                  this->getName());
    }
  }

  // Make sure we actually have a pauseButton object (default constructed with
  // createGUI)
  if (this->pauseButton != nullptr) {
    pauseButton->setChecked(hostPlugin->getComponentState() != RT::State::EXEC);
  }
}

void Widgets::Panel::refreshUserStates()
{
  if (hostPlugin == nullptr || !this->hostPlugin->hasComponent()) {
    return;
  }
  Widgets::Variable::Id param_id = Widgets::Variable::INVALID_ID;
  uint64_t uint_value = 0ULL;
  for (auto& i : this->parameter) {
    switch (i.second.type) {
      case Widgets::Variable::STATE:
        param_id = static_cast<Widgets::Variable::Id>(i.second.info.id);
        uint_value = this->hostPlugin->getComponentUIntParameter(param_id);
        i.second.edit->setText(QString::number(uint_value));
        palette.setBrush(i.second.edit->foregroundRole(), Qt::darkGray);
        i.second.edit->setPalette(palette);
        break;
      case Widgets::Variable::UINT_PARAMETER:
      case Widgets::Variable::INT_PARAMETER:
      case Widgets::Variable::DOUBLE_PARAMETER:
        break;
      default:
        ERROR_MSG("Unable to determine refresh type for component {}",
                  this->getName());
    }
  }
}

void Widgets::Panel::modify()
{
  Widgets::Variable::Id param_id = Widgets::Variable::INVALID_ID;
  double double_value = 0.0;
  int int_value = 0;
  uint64_t uint_value = 0ULL;
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
        var.second.edit->setText(QString::number(uint_value));
        var.second.edit->blacken();
        break;
      case Widgets::Variable::INT_PARAMETER:
        param_id = static_cast<Widgets::Variable::Id>(var.second.info.id);
        int_value = var.second.edit->text().toInt();
        this->hostPlugin->setComponentParameter<int>(param_id, int_value);
        var.second.edit->setText(QString::number(int_value));
        var.second.edit->blacken();
        break;
      case Widgets::Variable::DOUBLE_PARAMETER:
        param_id = static_cast<Widgets::Variable::Id>(var.second.info.id);
        double_value = var.second.edit->text().toDouble();
        this->hostPlugin->setComponentParameter(param_id, double_value);
        var.second.edit->setText(QString::number(double_value));
        var.second.edit->blacken();
        break;
      case Widgets::Variable::STATE:
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

QString Widgets::Panel::getComment(const QString& name)
{
  QString result;
  auto n = parameter.find(name.toStdString());
  if (n != parameter.end() && (n->second.type == Widgets::Variable::COMMENT)) {
    result = n->second.edit->text();
  }
  return result;
}

void Widgets::Panel::setParameter(const QString& var_name, double value)
{
  auto n = parameter.find(var_name.toStdString());
  if ((n != parameter.end())
      && (n->second.type == Widgets::Variable::DOUBLE_PARAMETER))
  {
    n->second.edit->setText(QString::number(value));
    auto param_id = static_cast<Widgets::Variable::Id>(n->second.info.id);
    this->hostPlugin->setComponentParameter<double>(param_id, value);
  }
}

void Widgets::Panel::setParameter(const QString& var_name, int64_t value)
{
  auto n = parameter.find(var_name.toStdString());
  if ((n != parameter.end())
      && (n->second.type == Widgets::Variable::INT_PARAMETER))
  {
    n->second.edit->setText(QString::number(value));
    auto param_id = static_cast<Widgets::Variable::Id>(n->second.info.id);
    this->hostPlugin->setComponentParameter<int64_t>(param_id, value);
  }
}

void Widgets::Panel::setParameter(const QString& var_name, uint64_t value)
{
  auto n = parameter.find(var_name.toStdString());
  if ((n != parameter.end())
      && (n->second.type == Widgets::Variable::UINT_PARAMETER))
  {
    n->second.edit->setText(QString::number(value));
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

RT::State::state_t Widgets::Plugin::getComponentState()
{
  if (this->plugin_component == nullptr) {
    return RT::State::UNDEFINED;
  }
  return this->plugin_component->getState();
}

std::vector<Widgets::Variable::Info>
Widgets::Plugin::getComponentParametersInfo() const
{
  return plugin_component == nullptr
      ? std::vector<Widgets::Variable::Info>({})
      : this->plugin_component->getParametersInfo();
}

size_t Widgets::Plugin::getID() const
{
  if (this->hasComponent()) {
    return plugin_component->getID();
  }
  return 0;
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

void Widgets::Plugin::loadParameterSettings(QSettings& userprefs)
{
  QString id_str;
  QVariant value;
  for (const auto& param_info : this->getComponentParametersInfo()) {
    id_str = QString::number(param_info.id);
    value = userprefs.value(id_str);
    if (!value.isValid()) {
      ERROR_MSG(
          "Widgets::Plugin::loadParameterSettings : The loaded setting for "
          "parameter {} is not valid! skipping...",
          param_info.name);
      continue;
    }
    // NOTE: Here we will assume that the type of the stored parameter matches
    // the one stored inside the component. This may not be true in the future
    // if the plugin developer decides to change the types of parameters. In
    // this case, the QVariant will return 0 instead.
    switch (param_info.vartype) {
      case Widgets::Variable::INT_PARAMETER:
        this->setComponentParameter(
            param_info.id,
            static_cast<int64_t>(userprefs.value(id_str).toInt()));
        break;
      case Variable::DOUBLE_PARAMETER:
        this->setComponentParameter(
            param_info.id,
            static_cast<double>(userprefs.value(id_str).toDouble()));
        break;
      case Variable::UINT_PARAMETER:
      case Variable::STATE:
        this->setComponentParameter(
            param_info.id,
            static_cast<uint64_t>(userprefs.value(id_str).toUInt()));
        break;
      case Variable::COMMENT:
      case Variable::UNKNOWN:
        break;
    }
  }
  // We should reflect the changes in the panel
  this->getPanel()->refresh();
}

void Widgets::Plugin::loadCustomParameterSettings(QSettings& /*userprefs*/) {}

void Widgets::Plugin::saveParameterSettings(QSettings& userprefs) const
{
  for (const auto& param_info : this->getComponentParametersInfo()) {
    userprefs.setValue(QString::number(static_cast<size_t>(param_info.id)),
                       QVariant::fromStdVariant(param_info.value));
  }
}

void Widgets::Plugin::saveCustomParameterSettings(
    QSettings& /*userprefs*/) const
{
}
