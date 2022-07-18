
#include <optional>
#include <string>
#include <vector>

#include <QtWidgets>

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

class Component
    : public RT::Thread
    , public Event::Handler
{
public:
  Component(std::string name,
            std::vector<IO::Channel_t> channels,
            std::vector<Modules::Variable::Info> variables) ;
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
  Q_OBJECT
public:
  UI();
  ~UI();
  /*
    * Getter function go allow customization of
    * user interface
    */
  QGridLayout * getLayout(void)
  {
      return layout;
  };

  /*!
    * Flag passed to DefaultGUIModel::update to signal the kind of update.
    *
    * \sa DefaultGUIModel::update()
    */
  enum update_flags_t
  {
      INIT,     /*!< The parameters need to be initialized.         */
      MODIFY,   /*!< The parameters have been modified by the user. */
      PERIOD,   /*!< The system period has changed.                 */
      PAUSE,    /*!< The Pause button has been activated            */
      UNPAUSE,  /*!< When the pause button has been deactivated     */
      EXIT,     /*!< When the module has been told to exit        */
  };

  /*!
    * Callback function that is called when the system state changes.
    *
    * \param flag The kind of update to signal.
    *
    * \sa DefaultGUIModel::update_flags_t
    */
  virtual void update(update_flags_t flag);

  /*!
    * Function that builds the Qt GUI.
    *
    * \param var The structure defining the module's parameters, states, inputs, and outputs.
    * \param size The size of the structure vars.
    *
    * \sa DefaultGUIModel::update_flags_t
    */
  void createGUI(DefaultGUIModel::variable_t *var, int size);

  // Default buttons
  QPushButton *pauseButton;
  QPushButton *modifyButton;
  QPushButton *unloadButton;

  struct param_t
  {
      QLabel *label;
      DefaultGUILineEdit *edit;
      IO::flags_t type;
      size_t index;
      QString *str_value;
  };
  std::map<QString,param_t> parameter;
  QPalette palette;

public slots:

  /*!
    * Function that resizes widgets to properly fit layouts after overloading
    */
  void resizeMe();
  /*!
    * Function that allows the object to safely delete and unload itself.
    */
  virtual void exit(void);
  /*!
    * Function that updates the GUI with new parameter values.
    *
    * \sa DefaultGUIModel::update_flags_t
    */
  virtual void refresh(void);
  /*!
    * Function that calls DefaultGUIModel::update with the MODIFY flag
    *
    * \sa DefaultGUIModel::update_flags_t
    */
  virtual void modify(void);
  /*!
    * Function that pauses/unpauses the model.
    */
  virtual void pause(bool);

protected:

  QGridLayout *layout;

  /*!
    * Get the value of the parameter in the GUI, and update the value
    *   within the Workspace.
    *
    * \param name The parameter's name.
    * \return The value of the parameter.
    */
  QString getParameter(const QString &name);
  /*!
    * Set the value of this parameter within the Workspace and GUI.
    *
    * \param name The name of the parameter.
    * \param ref A reference to the parameter.
    *
    * \sa Workspace::setData()
    */
  void setParameter(const QString &name,double value);
  /*!
    * Set the value of this parameter within the Workspace and GUI.
    *
    * \param name The parameter's name.
    * \param value The parameter's new value.
    */
  void setParameter(const QString &name,const QString value);
  /*!
    *
    */
  QString getComment(const QString &name);
  /*!
    *
    */
  void setComment(const QString &name,const QString comment);

  /*!
    * Set the reference to this state within the Workspace
    *   via Workspace::setData().
    *
    * \param name The state's name.
    * \param ref A reference to the state.
    *
    * \sa Workspace::setData()
    */
  void setState(const QString &name,double &ref);
  /*!
    * Set the reference to this event within the Workspace
    *   via Workspace::setData().
    *
    * \param name The event name.
    * \param ref A reference to the event.
    *
    * \sa Workspace::setData()
    */
  void setEvent(const QString &name,double &ref);

private:

  void doDeferred(const Settings::Object::State &);
  void doLoad(const Settings::Object::State &);
  void doSave(Settings::Object::State &) const;

  void receiveEvent(const Event::Object *);

  bool periodEventPaused;

  QWidget *gridBox;
  QGroupBox *buttonGroup;
  std::string myname;
  QMdiSubWindow *subWind
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
