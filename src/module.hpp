#ifndef MODULE_HPP
#define MODULE_HPP

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMdiSubWindow>
#include <QPushButton>
#include <QValidator>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "daq.hpp"
#include "dlplugin.hpp"
#include "event.hpp"
#include "io.hpp"

/*!
 * Contains all the classes and structures relevant to Modules
 */
namespace Modules
{

/*!
 * variable and state structures about constants and parameters
 */
namespace Variable
{

typedef size_t Id;
constexpr Id INVALID_ID = static_cast<Id>(std::numeric_limits<size_t>::max());

/*!
 * Code description of the variable type used for the parameter
 */
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
  EXEC, /*!< The module is in execution mode                */
  MODIFY, /*!< The parameters have been modified by the user. */
  PERIOD, /*!< The system period has changed.                 */
  PAUSE, /*!< The Pause button has been activated            */
  UNPAUSE, /*!< When the pause button has been deactivated     */
  EXIT, /*!< When the module has been told to exit        */
};

/*!
 * Converts state code to human readable string
 *
 * \param code the state code to convert
 *
 * \returns A string describing the code
 */
std::string state2string(state_t state);

/*!
 * Structure used to store information about module upon creation.
 * It is a structure describing module specific constants and
 * variables.
 *
 * \param id The identification number for the parameter
 * \param name The name of the channel
 * \param description short description of the channel
 * \param vartype type of variable that is stored
 *
 * \sa IO::Block::Block()
 */
struct Info
{
  size_t id = 0;
  std::string name;
  std::string description;
  Modules::Variable::variable_t vartype;
  std::variant<int64_t, double, uint64_t, std::string, state_t> value;
};

}  // namespace Variable

class DefaultGUILineEdit : public QLineEdit
{
  Q_OBJECT

public:
  explicit DefaultGUILineEdit(QWidget* parent);

  void blacken();
  QPalette palette;

public slots:
  void redden();
};  // class DefaultGUILineEdi

// Forward declare plugin class for the component and panel private pointers
class Plugin;
class Panel;

/*!
 * This structure contains functions for creating instances. It is used when
 * loading and unloading RTXI modules dynamically
 */
struct FactoryMethods
{
  std::unique_ptr<Modules::Plugin> (*createPlugin)(Event::Manager*) = nullptr;
  std::unique_ptr<Modules::Component> (*createComponent)(Modules::Plugin*) =
      nullptr;
  Modules::Panel* (*createPanel)(QMainWindow*, Event::Manager*) = nullptr;
};

/*!
 * This is where the magic happens. This class contains the low level logic to
 * interface with the real-time loop, as well as low level facilities for
 * parameter and input/output update. Inherit this class to run a periodic
 * real-time function
 */
class Component : public RT::Thread
{
public:
  Component(Modules::Plugin* hplugin,
            const std::string& mod_name,
            const std::vector<IO::channel_t>& channels,
            const std::vector<Modules::Variable::Info>& variables);

  /*!
   * Retrieves value from the component. Typically this function is only called
   * by the real time loop and should not be called from the gui thread.
   *
   * \param var_id The parameter identification number
   *
   * \return the value stored
   */
  template<typename T>
  T getValue(const size_t& var_id)
  {
    return std::get<T>(this->parameters.at(var_id).value);
  }

  /*!
   * Changes a parameter value
   *
   * \param var_id The parameter identification number
   * \param value the value to store
   */
  template<typename T>
  void setValue(const size_t& var_id, T value)
  {
    this->parameters.at(var_id).value = value;
  }

  /*!
   * This function returns the developer description of the parameter
   *
   * \param var_id the variable id
   *
   * \returns the variable description
   */
  std::string getDescription(const size_t& var_id);

  /*!
   * This function automatically converts the value stored as a human
   * readable string
   *
   * \param var_id the variable id
   *
   * \returns a string representation of the value stored
   */
  std::string getValueString(const size_t& var_id);

private:
  std::vector<Modules::Variable::Info> parameters;
  Modules::Plugin* hostPlugin;
  bool active;
};

class Panel : public QWidget
{
  Q_OBJECT
public:
  Panel(const std::string& mod_name,
        QMainWindow* mw,
        Event::Manager* ev_manager);

  /*
   * Getter function go allow customization of
   * user interface
   */
  QGridLayout* getLayout() { return m_layout; };

  QMdiSubWindow* getMdiWindow() { return this->m_subwindow; }
  /*!
   * Callback function that is called when the system state changes.
   *
   * \param flag The kind of update to signal.
   */
  virtual void update(Modules::Variable::state_t flag);

  /*!
   * Function that builds the Qt GUI.
   *
   * \param vars The structure defining the module's parameters, states, inputs,
   *    and outputs. 
   * \param skip_ids A vector of IDs that this function should not consider 
   *    building interface for.
   *
   */
  virtual void createGUI(const std::vector<Modules::Variable::Info>& vars,
                         const std::vector<Modules::Variable::Id>& skip_ids);

  /*!
   * Assigns a plugin to this panel. Typically used during construction of the
   * module and should not be used beyond initialization.
   *
   * \param hplugin A pointer to the host plugin this panel belongs to.
   */
  void setHostPlugin(Modules::Plugin* hplugin) { this->hostPlugin = hplugin; }

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
  QString getParameter(const QString& var_name);

  /*!
   * Set the value of this parameter within the Workspace and GUI.
   *
   * \param name The name of the parameter.
   * \param ref A reference to the parameter.
   *
   */
  void setParameter(const QString& var_name, double value);
  void setParameter(const QString& var_name, uint64_t value);
  void setParameter(const QString& var_name, int value);

  /*!
   * retrieves the comment storedabout the component parameter
   *
   * \param name The name of the parameter
   *
   * \return The parameter description
   */
  QString getComment(const QString& name);

  /*!
   * Assigns a comment to the parameter
   *
   * \param var_name the name of the parameter
   * \param comment The comment assigned to this parameter
   */
  void setComment(const QString& var_name, const QString& comment);

  /*!
   * Set the reference to this state within the Workspace
   *   via Workspace::setData().
   *
   * \param name The state's name.
   * \param ref A reference to the state.
   *
   * \sa Workspace::setData()
   */
  void setState(const QString& var_name, Modules::Variable::state_t ref);

  /*!
   * This function overrides the base class from Qt. It handles the
   * closing of the widget and properly initiates unloading of plugin
   *
   * \param event The close event triggered
   */
  void closeEvent(QCloseEvent* event) override;

  /*!
   * Obtains the name that identifies this module
   *
   * \return The name of the module
   */
  std::string getName() { return this->m_name; }

  /*!
   * retrieve the host plugin that controls this panel
   *
   * \return pointer to host plugin
   */
  Modules::Plugin* getHostPlugin() { return this->hostPlugin; }

  /*!
   * Retrieve the main window for the application
   *
   * \return pointer of type QMainWindow that controls the RTXI application
   */
  QMainWindow* getQMainWindowPtr() { return this->main_window; }

  /*!
   * Obtain the event manager attached to this session of the RTXI application.
   * This is useful for lower level control of RTXI events.
   *
   * \return Pointer to the RTXI event manager
   */
  Event::Manager* getRTXIEventManager() { return this->event_manager; }

protected:
  QWidget* gridBox = nullptr;
  QGridLayout* m_layout = nullptr;
  QGroupBox* buttonGroup = nullptr;

private:
  QMainWindow* main_window = nullptr;
  std::string m_name;
  QMdiSubWindow* m_subwindow = nullptr;
  Modules::Plugin* hostPlugin = nullptr;
  Event::Manager* event_manager = nullptr;

  // Default buttons
  QPushButton* pauseButton = nullptr;
  QPushButton* modifyButton = nullptr;
  QPushButton* unloadButton = nullptr;

  struct param_t
  {
    QLabel* label;
    QString str_value;
    DefaultGUILineEdit* edit;
    Modules::Variable::variable_t type = Modules::Variable::UNKNOWN;
    Modules::Variable::Info info;
  };
  std::unordered_map<QString, param_t> parameter;
  QPalette palette;
};

/*!
 * This class handles the isntantiation and deletion of Component and Plugin
 * classes.
 *
 * The responsibility is mainly the proper creation and deletion of the
 * Component and Plugin classes, as well as the communication between the two.
 * This class acts as a mediator between the two objects. Finally, it handles
 * events pertaining to the module. This class, together with the Component and
 * Panel classes, forms the Module.
 */
class Plugin : public Event::Handler
{
public:
  Plugin(Event::Manager* ev_manager, std::string mod_name);
  Plugin(const Plugin& plugin) = delete;  // copy constructor
  Plugin& operator=(const Plugin& plugin) =
      delete;  // copy assignment noperator
  Plugin(Plugin&&) = delete;  // move constructor
  Plugin& operator=(Plugin&&) = delete;  // move assignment operator
  ~Plugin() override;

  size_t getID();
  /*!
   * Attaches a component to this plugin
   *
   * \param component a unique pointer to the component object
   */
  void attachComponent(std::unique_ptr<Modules::Component> component);

  /*!
   * Attaches a panel to this plugin
   *
   * \param panel a pointer to the panel object
   */
  void attachPanel(Modules::Panel* panel);

  /*!
   * Retrieves an integer parameter from the component object. Usually called
   * from a non-realtime context
   *
   * \param parameter_id Identification number of the parameter
   *
   * \return the integer value
   */
  int64_t getComponentIntParameter(const Variable::Id& parameter_id);

  /*!
   * Retrieves an unsigned integer parameter from the component object. Usually
   * called from a non-realtime context
   *
   * \param parameter_id Identification number of the parameter
   *
   * \return the unsigned integer value
   */
  uint64_t getComponentUIntParameter(const Variable::Id& parameter_id);

  /*!
   * Retrieves a double parameter from the component object. Usually called
   * from a non-realtime context
   *
   * \param parameter_id Identification number of the parameter
   *
   * \return the double value
   */
  double getComponentDoubleParameter(const Variable::Id& parameter_id);

  template<typename T>
  int setComponentParameter(const Variable::Id& parameter_id, T value)
  {
    const int result = 0;
    Modules::Variable::variable_t param_type = Modules::Variable::UNKNOWN;
    if (typeid(T) == typeid(int)) {
      param_type = Modules::Variable::INT_PARAMETER;
    } else if (typeid(T) == typeid(double)) {
      param_type = Modules::Variable::DOUBLE_PARAMETER;
    } else if (typeid(T) == typeid(uint64_t)) {
      param_type = Modules::Variable::UINT_PARAMETER;
    } else if (typeid(T) == typeid(std::string)) {
      param_type = Modules::Variable::COMMENT;
    } else if (typeid(T) == typeid(Modules::Variable::state_t)) {
      param_type = Modules::Variable::STATE;
    } else {
      ERROR_MSG(
          "Modules::Plugin::setComponentParameter : Parameter type not "
          "supported");
      return -1;
    }
    Event::Object event(Event::Type::RT_MODULE_PARAMETER_CHANGE_EVENT);
    event.setParam("paramID", std::any(parameter_id));
    event.setParam("paramType", std::any(param_type));
    event.setParam("paramValue", std::any(value));
    event.setParam("paramModule", std::any(this->plugin_component.get()));
    this->event_manager->postEvent(&event);
    return result;
  }

  /*!
   * Retrieves the name of the plugin
   *
   * \returns a string with the module name
   */
  std::string getName() const { return this->name; }

  /*!
   * Checks whether the component is active
   *
   * \return true if active, false otherwise
   */
  bool getActive();

  /*!
   * sets the activity state of the component
   *
   * \param state Boolean representing activity state
   *
   * \return 0 if successful, -1 otherwise
   */
  int setActive(bool state);

  /*!
   * Function called when a new event is fired
   *
   * \param event pointer to Qt event object
   */
  void receiveEvent(Event::Object* event) override;

  /*!
   * Get the name of the library from which the object was loaded.
   *
   * \return The library file the object from which the object was created.
   */
  std::string getLibrary() const { return this->library; }

  // These functions are here in order to have backwards compatibility
  // with previous versions of RTXI that used DefaultGuiModel
  // (before RTXI 3.0.0)

  void registerComponent();

protected:
  Modules::Component* getComponent();
  DAQ::Device* getDevice();
  Event::Manager* getEventManager();
  QMainWindow* getQMainWindow();
  Modules::Panel* getPanel();

private:
  // owned pointers
  std::unique_ptr<Modules::Component> plugin_component;
  std::unique_ptr<DAQ::Device> plugin_device;

  // not owned pointers (managed by external objects)
  Event::Manager* event_manager = nullptr;
  QMainWindow* main_window = nullptr;  // Qt handles this lifetime
  Modules::Panel* widget_panel = nullptr;  // Qt handles this lifetime

  std::string library;
  std::string name;
};

}  // namespace Modules

#endif
