#ifndef MODULE_HPP
#define MODULE_HPP

#include <optional>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <unordered_map>
#include <memory>

#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QValidator>

#include "event.hpp"
#include "io.hpp"
#include "rt.hpp"
#include "main_window.hpp"

namespace Modules
{

namespace Variable
{
enum variable_t : size_t
{
  INT_PARAMETER = 0,
  DOUBLE_PARAMETER,
  UINT_PARAMETER,
  STATE,
  COMMENT,
  UNKNOWN
};

/*!
  * Flag passed to DefaultGUIModel::update to signal the kind of update.
  *
  * \sa DefaultGUIModel::update()
  */
enum state_t : int64_t
{
  INIT, /*!< The parameters need to be initialized.         */
  MODIFY, /*!< The parameters have been modified by the user. */
  PERIOD, /*!< The system period has changed.                 */
  PAUSE, /*!< The Pause button has been activated            */
  UNPAUSE, /*!< When the pause button has been deactivated     */
  EXIT, /*!< When the module has been told to exit        */
};

std::string state2string(state_t state);

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
  std::variant<int, double, uint64_t, std::string, state_t> value;
};

}  // namespace Variable

class DefaultGUILineEdit : public QLineEdit
{

  Q_OBJECT

public:
  DefaultGUILineEdit(QWidget *);
  ~DefaultGUILineEdit();
  void blacken();
  QPalette palette;

public slots:
  void redden(void);
}; // class DefaultGUILineEdit

// Forward declare plugin class for the component and panel private pointers
class Plugin;

class Component
    : public RT::Thread
{
public:
  Component(Modules::Plugin* hostPlugin,
            const std::string& name,
            std::vector<IO::channel_t> channels,
            std::vector<Modules::Variable::Info> variables);
  virtual ~Component()=default;

  template<typename T>
  T getValue(const std::string& varname)
  {
    return std::get<T>(this->parameter[varname].value);
  }

  template<typename T>
  void setValue(const std::string& varname, T value)
  {
    this->parameter[varname].value = value;
  }

  std::string getDescription(const std::string& varname);
  std::string getValueString(const std::string& varname);

 
  // Here are a list of functions inherited from RT::Thread

  virtual void execute() override;

  //virtual void input(size_t channel, const std::vector<double>& data) override;
  //virtual const std::vector<double>& output(size_t channel) override;
  
private:
  std::unordered_map<std::string, Modules::Variable::Info> parameter;
  bool active;
  Modules::Plugin* hostPlugin;

};

class Panel : public QWidget
{
  Q_OBJECT
public:
  Panel(std::string name, QMainWindow* main_window);

  /*
   * Getter function go allow customization of
   * user interface
   */
  QGridLayout* getLayout() { return layout; };

  /*!
   * Callback function that is called when the system state changes.
   *
   * \param flag The kind of update to signal.
   */
  virtual void update(Modules::Variable::state_t flag);

  /*!
   * Function that builds the Qt GUI.
   *
   * \param var The structure defining the module's parameters, states, inputs,
   * and outputs. \param size The size of the structure vars.
   *
   * \sa DefaultGUIModel::update_flags_t
   */
  virtual void createGUI(std::vector<Modules::Variable::Info> vars, MainWindow* main_window);

  // Default buttons
  QPushButton* pauseButton;
  QPushButton* modifyButton;
  QPushButton* unloadButton;

  struct param_t
  {
    QLabel* label;
    QString str_value;
    DefaultGUILineEdit* edit;
    Modules::Variable::variable_t type = Modules::Variable::UNKNOWN;
  };
  std::unordered_map<QString, param_t> parameter;
  QPalette palette;

public slots:

  /*!
   * Function that resizes widgets to properly fit layouts after overloading
   */
  void resizeMe();

  /*!
   * Function that allows the object to safely delete and unload itself.
   */
  virtual void exit();

  /*!
   * Function that updates the GUI with new parameter values.
   *
   * \sa DefaultGUIModel::update_flags_t
   */
  virtual void refresh();

  /*!
   * Function that calls DefaultGUIModel::update with the MODIFY flag
   *
   * \sa DefaultGUIModel::update_flags_t
   */
  virtual void modify();

  /*!
   * Function that pauses/unpauses the model.
   */
  virtual void pause(bool);

protected:


  /*!
   * Get the value of the parameter in the GUI, and update the value
   *   within the Workspace.
   *
   * \param name The parameter's name.
   * \return The value of the parameter.
   */
  QString getParameter(const QString& name);
  
  /*!
   * Set the value of this parameter within the Workspace and GUI.
   *
   * \param name The name of the parameter.
   * \param ref A reference to the parameter.
   *
   */
  void setParameter(const QString& name, double value);
  void setParameter(const QString& name, uint64_t value);
  void setParameter(const QString& name, int value);

  /*!
   *
   */
  QString getComment(const QString& name);

  /*!
   *
   */
  void setComment(const QString& name, QString comment);

  /*!
   * Set the reference to this state within the Workspace
   *   via Workspace::setData().
   *
   * \param name The state's name.
   * \param ref A reference to the state.
   *
   * \sa Workspace::setData()
   */
  void setState(const QString& name, Modules::Variable::state_t ref);

  std::string getName(){ return this->myname; }
  //virtual void receiveEvent(const Event::Object* event) override;
  
private:

  bool periodEventPaused;

  QWidget* gridBox;
  QGroupBox* buttonGroup;
  std::string myname;
  QMdiSubWindow* subWindow;
  Modules::Plugin* hostPlugin;
  QGridLayout* layout;
};

class Plugin : public Event::Handler
{
public:
  Plugin(Event::Manager* ev_manager, QMainWindow* main_window, std::string name);
  Plugin(const Plugin& plugin) = default; // copy constructor
  Plugin& operator=(const Plugin& plugin) = default; // copy assignment noperator
  Plugin(Plugin &&) = default; // move constructor
  Plugin& operator=(Plugin &&) = default; // move assignment operator
  ~Plugin();

  int exit();
  void attachComponent(std::unique_ptr<Modules::Component> component);
  int getComponentIntParameter(const std::string& parameter_name);
  uint64_t getComponentUIntParameter(const std::string& parameter_name);
  double getComponentDoubleParameter(const std::string& parameter_name);

  int setComponentIntParameter(const std::string& parameter_name, int value);
  int setComponentDoubleParameter(const std::string& parameter_name, double value);
  int setComponentUintParameter(const std::string& parameter_name, uint64_t value);
  int setComponentComment(const std::string& parameter_name, std::string value);
  int setComponentState(const std::string& parameter_name, Modules::Variable::state_t value);

  std::string getName() { return this->name; }
  bool getActive();
  int setActive(bool state);
  void receiveEvent(Event::Object* event) override;

private:
  std::string name;
  // owned pointers
  std::unique_ptr<Modules::Component> plugin_component;

  // not owned pointers (managed by external objects)
  QMainWindow* main_window;
  Modules::Panel* widget_panel; // Qt handles this lifetime
  Event::Manager* event_manager;
};

class Manager : public Event::Handler
{
public:
  Manager(Event::Manager* event_manager, QMainWindow* main_window);

  int loadPlugin(const std::string& dynlib_name);
  int loadPlugin(Modules::Plugin* dynlib_pointer);
  int unloadPlugin(Modules::Plugin* dynlib_pointer);

  int registerModule(std::unique_ptr<Modules::Plugin> module);
  int unregisterModule(std::string module_name);

  void receiveEvent(Event::Object* event) override;

private:
  std::unordered_map<std::string, std::unique_ptr<Modules::Plugin>> rtxi_modules_registry;
  QMainWindow* main_window;
  Event::Manager* event_manager;
};

}  // namespace Modules

#endif