
#include <vector>
#include <any>

#include "module.hpp"

Modules::Component(std::string name, 
                  std::vector<IO::channel_t> channels,
                  size_t channel_size,
                  std::vector<Modules::Variable::Info> variables)
    : RT::Thread(name, channels, channel_size)
{
  for(auto var : variables){
    switch(var.vartype){
      case Modules::Variable::State:
        this->states.push_back(var);
        break;
      case Modules::Variable::Parameter:
        this->parameters.push_back(var);
        break;
      case Modules::Variable::Comment:
        this->comments.push_back(var);
        break;
      default:
        ERROR_MSG("Unknown or empty variable type provided for module {}. Ignoring", this->name);
    }
  }
}

size_t Modules::Component::getCount(Modules::Variable::variable_t vartype)
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
    default:
      result =  0;
  }
  return result;
}

std::string Modules::Component::getName(Modules::Variable::variable_t vartype, size_t index)
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
    default:
      result = "";
  }

  return result;
}

std::string Modules::Component::getDescription(IO::flags_t vartype, size_t n) const
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
    default:
      result = "";
  }
  return result;
}

std::any Modules::Component::getValue(Modules::Variable::variable_t vartype, size_t index)
{
  std::any retval;
  switch(vartype){
    case Modules::Variable::PARAMETERS:
      retval = this->parameters.at(index);
    case Modules::Variable::STATE:
      retval = this->states.at(index);
    case Modules::Variable::COMMENT:
      retval = this->comments.at(index);
    default:
      ERROR_MSG("Modules::Component::getValue : Unknown variable type provided!");
  }
  return retval;
}

std::string Modules::Component::getValueString(Modules::Variable::variable_t vartype, size_t index) const
{
  std::string result;
  switch(vartype){
    case Modules::Variable::STATE:
      result = fmt::format("{}", std::any_cast<int>(this->getvalue(vartype, n)));
      break;
    case Modules::Variable::PARAMETER:
      result = fmt::format("{}", std::any_cast<double>(this->getvalue(vartype, n)));
      break;
    case Modules::Variable::COMMENT:
      result = fmt::format("{}", std::any_cast<std::string>(this->getvalue(vartype, n)));
      break;
    default:
      result = ""
  }
  return result;
}

void Modules::Component::setParameter(size_t n, double value)
{
  if (n >= this->parameters.size()) { return; }
  this->parameters[n] = value;
}

void Modules::Component::setState(size_t n, int value)
{
  if (n >= this->states.size()) { return; }
  this->states[n] = value;
}

void Modules::Component::setComment(size_t n, std::string newComment)
{
  if (n >= comment.size()) { return; }
  this->comments[n] = newComment;
}

Modules::Plugin* Modules::Component::getHostPlugin();
{
  return this->hostPlugin;
}

int Modules::Manager::loadPlugin(std::string dynlib)
{
  return 0;
}

Modules::Panel(std::string name, QMainWindow main_window)
    : QWidget(main_window)
{
  setWindowTitle(QString::number(getID()) + " " + QString::fromStdString(name));

  QTimer* timer = new QTimer(this);
  timer->setTimerType(Qt::PreciseTimer);
  timer->start(1000);
  QObject::connect(timer, SIGNAL(timeout(void)), this, SLOT(refresh(void)));
}

void Modules::Panel::createGUI(Modules::Panel::variable_t* var, int size)
{
  // Make Mdi
  subWindow = new QMdiSubWindow;
  subWindow->setAttribute(Qt::WA_DeleteOnClose);
  subWindow->setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
  subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint
                            | Qt::WindowMinimizeButtonHint);
  subWindow->setOption(QMdiSubWindow::RubberBandResize, true);
  subWindow->setOption(QMdiSubWindow::RubberBandMove, true);
  MainWindow::getInstance()->createMdi(subWindow);

  // Create main layout
  layout = new QGridLayout;

  // Create child widget and gridLayout
  QScrollArea* gridArea = new QScrollArea;
  gridBox = new QWidget;
  gridArea->setWidget(gridBox);
  gridArea->ensureWidgetVisible(gridBox, 0, 0);
  gridArea->setWidgetResizable(true);
  QGridLayout* gridLayout = new QGridLayout;

  size_t nstate = 0, nparam = 0, nevent = 0, ncomment = 0;
  for (int i = 0; i < size; i++) {
    if (var[i].flags & (PARAMETER | STATE | EVENT | COMMENT)) {
      param_t param;

      param.label = new QLabel(QString::fromStdString(var[i].name), gridBox);
      gridLayout->addWidget(param.label, parameter.size(), 0);
      param.edit = new DefaultGUILineEdit(gridBox);
      gridLayout->addWidget(param.edit, parameter.size(), 1);

      param.label->setToolTip(QString::fromStdString(var[i].description));
      param.edit->setToolTip(QString::fromStdString(var[i].description));

      if (var[i].flags & PARAMETER) {
        if (var[i].flags & DOUBLE) {
          param.edit->setValidator(new QDoubleValidator(param.edit));
          param.type = PARAMETER | DOUBLE;
        } else if (var[i].flags & UINTEGER) {
          QIntValidator* validator = new QIntValidator(param.edit);
          param.edit->setValidator(validator);
          validator->setBottom(0);
          param.type = PARAMETER | UINTEGER;
        } else if (var[i].flags & INTEGER) {
          param.edit->setValidator(new QIntValidator(param.edit));
          param.type = PARAMETER | INTEGER;
        } else
          param.type = PARAMETER;
        param.index = nparam++;
        param.str_value = new QString;
      } else if (var[i].flags & STATE) {
        param.edit->setReadOnly(true);
        palette.setBrush(param.edit->foregroundRole(), Qt::darkGray);
        param.edit->setPalette(palette);
        param.type = STATE;
        param.index = nstate++;
      } else if (var[i].flags & EVENT) {
        param.edit->setReadOnly(true);
        param.type = EVENT;
        param.index = nevent++;
      } else if (var[i].flags & COMMENT) {
        param.type = COMMENT;
        param.index = ncomment++;
      }
      parameter[QString::fromStdString(var[i].name)] = param;
    }
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

void Modules::Panel::update(Modules::Panel::State state) {}

void Modules::Panel::resizeMe()
{
  subWindow->adjustSize();
}

void Modules::Panel::exit(void)
{
  // Ensure that the realtime thread isn't in the middle of executing
  // Modules::Panel::execute()
  setActive(false);
  SyncEvent event;
  RT::System::getInstance()->postEvent(&event);

  update(EXIT);
  Plugin::Manager::getInstance()->unload(this);
  subWindow->close();
}

void Modules::Panel::refresh(void)
{
  for (std::map<QString, param_t>::iterator i = parameter.begin();
       i != parameter.end();
       ++i)
  {
    if (i->second.type & (STATE | EVENT)) {
      i->second.edit->setText(
          QString::number(getValue(i->second.type, i->second.index)));
      palette.setBrush(i->second.edit->foregroundRole(), Qt::darkGray);
      i->second.edit->setPalette(palette);
    } else if ((i->second.type & PARAMETER) && !i->second.edit->isModified()
               && i->second.edit->text() != *i->second.str_value)
    {
      i->second.edit->setText(*i->second.str_value);
    } else if ((i->second.type & COMMENT) && !i->second.edit->isModified()
               && i->second.edit->text()
                   != QString::fromStdString(
                       getValueString(COMMENT, i->second.index)))
    {
      i->second.edit->setText(
          QString::fromStdString(getValueString(COMMENT, i->second.index)));
    }
  }
  pauseButton->setChecked(!getActive());
}

void Modules::Panel::modify(void)
{
  bool active = getActive();
  setActive(false);
  // Ensure that the realtime thread isn't in the middle of executing
  // Modules::Panel::execute()
  SyncEvent event;
  RT::System::getInstance()->postEvent(&event);

  for (std::map<QString, param_t>::iterator i = parameter.begin();
       i != parameter.end();
       ++i)
    if (i->second.type & COMMENT) {
      QByteArray textData = i->second.edit->text().toLatin1();
      const char* text = textData.constData();
      Workspace::Instance::setComment(i->second.index, text);
    }

  update(MODIFY);
  setActive(active);

  for (std::map<QString, param_t>::iterator i = parameter.begin();
       i != parameter.end();
       ++i)
    i->second.edit->blacken();
}

QString Modules::Panel::getComment(const QString& name)
{
  std::map<QString, param_t>::iterator n = parameter.find(name);
  if (n != parameter.end() && (n->second.type & COMMENT))
    return QString::fromStdString(getValueString(COMMENT, n->second.index));
  return "";
}

void Modules::Panel::setComment(const QString& name, QString comment)
{
  std::map<QString, param_t>::iterator n = parameter.find(name);
  if (n != parameter.end() && (n->second.type & COMMENT)) {
    n->second.edit->setText(comment);
    QByteArray textData = comment.toLatin1();
    const char* text = textData.constData();
    Workspace::Instance::setComment(n->second.index, text);
  }
}

QString Modules::Panel::getParameter(const QString& name)
{
  std::map<QString, param_t>::iterator n = parameter.find(name);
  if ((n != parameter.end()) && (n->second.type & PARAMETER)) {
    *n->second.str_value = n->second.edit->text();
    setValue(n->second.index, n->second.edit->text().toDouble());
    return n->second.edit->text();
  }
  return "";
}

void Modules::Panel::setParameter(const QString& name, double value)
{
  std::map<QString, param_t>::iterator n = parameter.find(name);
  if ((n != parameter.end()) && (n->second.type & PARAMETER)) {
    n->second.edit->setText(QString::number(value));
    *n->second.str_value = n->second.edit->text();
    setValue(n->second.index, n->second.edit->text().toDouble());
  }
}

void Modules::Panel::setParameter(const QString& name, const QString value)
{
  std::map<QString, param_t>::iterator n = parameter.find(name);
  if ((n != parameter.end()) && (n->second.type & PARAMETER)) {
    n->second.edit->setText(value);
    *n->second.str_value = n->second.edit->text();
    setValue(n->second.index, n->second.edit->text().toDouble());
  }
}

void Modules::Panel::setState(const QString& name, double& ref)
{
  std::map<QString, param_t>::iterator n = parameter.find(name);
  if ((n != parameter.end()) && (n->second.type & STATE)) {
    setData(Workspace::STATE, n->second.index, &ref);
    n->second.edit->setText(QString::number(ref));
  }
}

void Modules::Panel::setEvent(const QString& name, double& ref)
{
  std::map<QString, param_t>::iterator n = parameter.find(name);
  if ((n != parameter.end()) && (n->second.type & EVENT)) {
    setData(Workspace::EVENT, n->second.index, &ref);
    n->second.edit->setText(QString::number(ref));
  }
}

void Modules::Panel::pause(bool p)
{
  if (pauseButton->isChecked() != p)
    pauseButton->setDown(p);

  setActive(!p);
  if (p)
    update(PAUSE);
  else
    update(UNPAUSE);
}

void Modules::Panel::doDeferred(const Settings::Object::State&)
{
  setWindowTitle(QString::number(getID()) + " "
                 + QString::fromStdString(myname));
}

void Modules::Panel::doLoad(const Settings::Object::State& s)
{
  for (std::map<QString, param_t>::iterator i = parameter.begin();
       i != parameter.end();
       ++i)
    i->second.edit->setText(
        QString::fromStdString(s.loadString((i->first).toStdString())));
  if (s.loadInteger("Maximized"))
    showMaximized();
  else if (s.loadInteger("Minimized"))
    showMinimized();
  // this only exists in RTXI versions >1.3
  if (s.loadInteger("W") != 0) {
    resize(s.loadInteger("W"), s.loadInteger("H"));
    parentWidget()->move(s.loadInteger("X"), s.loadInteger("Y"));
  }

  pauseButton->setChecked(s.loadInteger("paused"));
  modify();
}

void Modules::Panel::doSave(Settings::Object::State& s) const
{
  s.saveInteger("paused", pauseButton->isChecked());
  if (isMaximized())
    s.saveInteger("Maximized", 1);
  else if (isMinimized())
    s.saveInteger("Minimized", 1);

  QPoint pos = parentWidget()->pos();
  s.saveInteger("X", pos.x());
  s.saveInteger("Y", pos.y());
  s.saveInteger("W", width());
  s.saveInteger("H", height());

  for (std::map<QString, param_t>::const_iterator i = parameter.begin();
       i != parameter.end();
       ++i)
    s.saveString((i->first).toStdString(),
                 (i->second.edit->text()).toStdString());
}

void Modules::Panel::receiveEvent(const Event::Object* event)
{
  if (event->getName() == Event::RT_PREPERIOD_EVENT) {
    periodEventPaused = getActive();
    setActive(false);
  } else if (event->getName() == Event::RT_POSTPERIOD_EVENT) {
#ifdef DEBUG
    if (getActive())
      ERROR_MSG(
          "Modules::Panel::receiveEvent : model unpaused during a period "
          "update\n");
#endif
    update(PERIOD);
    setActive(periodEventPaused);
  }
}
