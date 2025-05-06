#ifndef WIDGET_HPP
#define WIDGET_HPP

#include <QLineEdit>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "debug.hpp"
#include "event.hpp"
#include "io.hpp"
#include "rt.hpp"

class QGridLayout;
class QGroupBox;
class QLabel;
class QMainWindow;
class QMdiSubWindow;
class QPushButton;
class QValidator;

// These metatype declarations are needed by qt to store
// the types in QVariant, which is very convenient and reduces
// unnecessary repetiotion in panel construction
Q_DECLARE_METATYPE(IO::Block*)
Q_DECLARE_METATYPE(IO::flags_t)
Q_DECLARE_METATYPE(IO::endpoint)
Q_DECLARE_METATYPE(RT::block_connection_t)
Q_DECLARE_METATYPE(RT::State::state_t)
Q_DECLARE_METATYPE(std::string)

class QSettings;
/*!
 * Contains all the classes and structures relevant to Widgets
 */
namespace Widgets
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
 * Converts state code to human readable string
 *
 * \param code the state code to convert
 *
 * \returns A string describing the code
 */
std::string state2string(RT::State::state_t state);

/*!
 * Converts variable type to human readable string
 *
 * \param code the variable type code to convert
 *
 * \returns A string describing the code
 */

std::string vartype2string(variable_t type);

/*!
 * Structure used to store information about module upon creation.
 * It is a structure describing module specific constants and
 * variables.
 *
 * \param id The identification number for the parameter
 * \param name The name of the channel
 * \param description short description of the channel
 * \param vartype type of variable that is stored
 * \param value The value of the parameter stored in a variant
 *
 * \sa IO::Block::Block()
 */
struct Info
{
  size_t id = 0;
  std::string name;
  std::string description;
  Widgets::Variable::variable_t vartype = Widgets::Variable::UNKNOWN;
  std::variant<int64_t, double, uint64_t, std::string> value;
};

template<typename... Types, typename T>
void var_assert(Types... args, T last_var)
{
  if constexpr (sizeof...(args) > 0) {
    var_assert(args...);
  }
  static_assert(last_var.id == sizeof...(args),
                "RTXI Plugin ID order does not match. Make sure ENUM IDs "
                "matches Info ID");
}

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
 * This structure contains functions for creating Widget instances.
 *
 * This struct is used when loading and unloading RTXI modules. It
 * allows RTXI to use the builder pattern for loading widgets by calling these
 * functions to generate the widget specific classes, then using relevant
 * assembly methods to put them together and display them.
 *
 */
struct FactoryMethods
{
  /*!
   * \fn std::unique_ptr<Widgets::Plugin> createPlugin(Event::Manager* event)
   * \brief Function that returns a smart pointer to plugin object
   *
   * \param event_manager Raw pointer to Event::Manager object
   * \return Smart pointer to plugin object
   *
   * \sa Widgets::Plugin
   */
  std::unique_ptr<Widgets::Plugin> (*createPlugin)(Event::Manager*) = nullptr;

  /*!
   * \fn std::unique_ptr<Widgets::Component> createComponent(Widgets::Plugin*
   * plugin) \brief Function that returns a smart pointer to component object
   *
   * \param event_manager Raw pointer to Widgets::Plugin object
   * \return Smart pointer to component object
   *
   * \sa Widgets::Component
   */
  std::unique_ptr<Widgets::Component> (*createComponent)(Widgets::Plugin*) =
      nullptr;

  /*!
   * \fn Widgets::Panel* createPanel(QMainWindow* window, Event::Manager*
   * event_manager) \brief Function that creates Widgets::Panel object and
   * returns the pointer
   *
   * \param window QMainWindow pointer to attach the panel to
   * \param event_manager Raw pointer to Event::Manager object
   * \return Widgets::Plugin pointer to created panel
   *
   * \sa Widgets::Panel
   */
  Widgets::Panel* (*createPanel)(QMainWindow*, Event::Manager*) = nullptr;
};

/*!
 * Class responsible for running real-time code.
 *
 * This is where the magic happens. This class contains the low level logic to
 * interface with the real-time loop, as well as low level facilities for
 * parameter and input/output update. Inherit this class to run a periodic
 * real-time function. It inherits the RT::Thread interface and therefore is
 * considered dependent on inputs from other blocks. It will autoamtically be
 * scheduled to run after dependency Components from other plugins.
 */
class Component : public RT::Thread
{
public:
  Component(Widgets::Plugin* hplugin,
            const std::string& mod_name,
            const std::vector<IO::channel_t>& channels,
            const std::vector<Widgets::Variable::Info>& variables);

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

  /*!
   * Obtain Widget realtime state information
   *
   * \return RT::State::state_t representing Widgets::Component state
   * \sa RT::State::state_t
   */
  RT::State::state_t getState() const { return this->component_state; }

  /*!
   * set realtime state
   *
   * \param state an RT::State::state_t enum representing new state
   */
  void setState(RT::State::state_t state) { this->component_state = state; }

  /*!
   * Get a list of all parameters for this widget
   *
   * \return A vector of Widgets::Variable::Info structs representing widget
   * parameters \sa Widgets::Variable::Info
   */
  std::vector<Widgets::Variable::Info> getParametersInfo()
  {
    return this->parameters;
  }

  /*!
   * Get the plugin object that owns this component
   *
   * \param Pointer to Widgets::Plugin object that owns this component.
   */
  Widgets::Plugin* getHostPlugin() { return this->hostPlugin; }

private:
  std::vector<Widgets::Variable::Info> parameters;
  Widgets::Plugin* hostPlugin;
  RT::State::state_t component_state = RT::State::INIT;
};

/*!
 * RTXI Widgets UI class
 */
class Panel : public QWidget
{
  Q_OBJECT
public:
  Panel(const std::string& mod_name,
        QMainWindow* mw,
        Event::Manager* ev_manager);

  QMdiSubWindow* getMdiWindow() { return this->m_subwindow; }

  /*!
   * Function that builds the Qt GUI for the Widget's panel.
   *
   * This is a convenience function that will automatically generate a simple
   * panel with text fields corresponding to the parameters. It is useful for
   * quick and dirty gui construction.
   *
   * \param vars The structure defining the module's parameters, states, inputs,
   *    and outputs.
   * \param skip_ids A vector of IDs that this function should not consider
   *    building interface for. If not provided the function does not skip any
   *    IDs.
   *
   */
  void createGUI(const std::vector<Widgets::Variable::Info>& vars,
                 const std::vector<Widgets::Variable::Id>& skip_ids = {});

  /*!
   * Assigns a plugin to this panel. Typically used during construction of the
   * module and should not be used beyond initialization.
   *
   * \param hplugin A pointer to the host plugin this panel belongs to.
   */
  void setHostPlugin(Widgets::Plugin* hplugin) { this->hostPlugin = hplugin; }

signals:

  /*!
   * Signal used for changing the state of the internal component.
   */
  void signal_state_change(RT::State::state_t state);

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
   */
  virtual void refresh();

  /*!
   * Function that updates GUI user States only.
   */
  virtual void refreshUserStates();

  /*!
   * Function that calls update_state with the MODIFY flag
   */
  virtual void modify();

  /*!
   * Function that pauses/unpauses the model.
   *
   * \param paused True if pausing widget, false otherwise
   */
  virtual void pause(bool p);

  /*!
   * Callback function that is called when the system state changes.
   *
   * \param flag The kind of update to signal.
   */
  virtual void update_state(RT::State::state_t flag);

protected:
  /*!
   * Set the value of double parameter within the Workspace and GUI.
   *
   * \param name The name of the parameter.
   * \param ref A reference to the parameter.
   *
   */
  void setParameter(const QString& var_name, double value);

  /*!
   * Set the value of unsigned int parameter within the Workspace and GUI.
   *
   * \param name The name of the parameter.
   * \param ref A reference to the parameter.
   *
   */
  void setParameter(const QString& var_name, uint64_t value);

  /*!
   * Set the value of int parameter within the Workspace and GUI.
   *
   * \param name The name of the parameter.
   * \param ref A reference to the parameter.
   *
   */
  void setParameter(const QString& var_name, int64_t value);

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
   * retrieve the host plugin that owns this panel
   *
   * \return pointer to host plugin
   */
  Widgets::Plugin* getHostPlugin() { return this->hostPlugin; }

  /*!
   * Retrieve the main window for the application
   *
   * \return pointer of type QMainWindow that controls the RTXI application
   */
  QMainWindow* getQMainWindowPtr() { return this->main_window; }

  /*!
   * Obtain the event manager attached to this session of the RTXI application.
   *
   * \return Pointer to the RTXI event manager
   */
  Event::Manager* getRTXIEventManager() { return this->event_manager; }

private slots:
  void updatePauseButton();

private:
  QMainWindow* main_window = nullptr;
  std::string m_name;
  QMdiSubWindow* m_subwindow = nullptr;
  Widgets::Plugin* hostPlugin = nullptr;
  Event::Manager* event_manager = nullptr;

  // Default buttons
  QPushButton* pauseButton = nullptr;
  QPushButton* modifyButton = nullptr;
  QPushButton* unloadButton = nullptr;
  QTimer* defaultPauseUpdateTimer = nullptr;

  struct param_t
  {
    QLabel* label = nullptr;
    DefaultGUILineEdit* edit = nullptr;
    Widgets::Variable::variable_t type = Widgets::Variable::UNKNOWN;
    Widgets::Variable::Info info;
  };
  std::unordered_map<std::string, param_t> parameter;
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
 * Panel classes, forms the Widget.
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

  /*!
   * get the component ID
   *
   * \return ID of the widget assigned by the realtime system
   */
  size_t getID() const;

  /*!
   * Attaches a component to this plugin
   *
   * \param component a unique pointer to the component object
   */
  void attachComponent(std::unique_ptr<Widgets::Component> component);

  /*!
   * Attaches a panel to this plugin
   *
   * \param panel a pointer to the panel object
   */
  void attachPanel(Widgets::Panel* panel);

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

  /*!
   * Sets the component parameter
   *
   * This function sends an event that will be handled by the realtime system.
   * The event comprises with the parameter to change and the new value.
   *
   * \param parameter_id The id of the widget's parameter to change
   * \param value The new value to change the parameter to. accepted value
   *              types are: int, double, uint64_t, and std::string. Anything
   *              else is considered an error.
   * \return an error code 0 for success, and -1 for failure (no attached
   *         component)
   */
  template<typename T>
  int setComponentParameter(const Variable::Id& parameter_id, T value)
  {
    if (this->plugin_component == nullptr) {
      return -1;
    }
    const int result = 0;
    Widgets::Variable::variable_t param_type = Widgets::Variable::UNKNOWN;
    if (typeid(T) == typeid(int64_t)) {
      param_type = Widgets::Variable::INT_PARAMETER;
    } else if (typeid(T) == typeid(double)) {
      param_type = Widgets::Variable::DOUBLE_PARAMETER;
    } else if (typeid(T) == typeid(uint64_t)) {
      param_type = Widgets::Variable::UINT_PARAMETER;
    } else if (typeid(T) == typeid(std::string)) {
      param_type = Widgets::Variable::COMMENT;
    } else {
      ERROR_MSG(
          "Widgets::Plugin::setComponentParameter : Parameter type not "
          "supported");
      return -1;
    }
    Event::Object event(Event::Type::RT_WIDGET_PARAMETER_CHANGE_EVENT);
    event.setParam("paramID", std::any(parameter_id));
    event.setParam("paramType", std::any(param_type));
    event.setParam("paramValue", std::any(value));
    event.setParam("paramWidget", std::any(this->plugin_component.get()));
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

  /*!
   * Sets the library location from which this widget was created.
   *
   * \param lib The library location
   */
  void setLibrary(const std::string& lib) { this->library = lib; }

  /*!
   * Register the internal component to RT::System registry
   */
  void registerComponent();

  /*!
   * Set the internal component state
   *
   * \param state an RT::State::state_t value to set the component to
   */
  void setComponentState(RT::State::state_t state);

  /*!
   * Obtain the state of the components attached to the plugin
   *
   * \return An RT::State::state_t value representing the current state of the
   * component
   */
  RT::State::state_t getComponentState();

  /*!
   * Get the list of all widget parameters information
   *
   * \return A vector of Widgets::Variable::Info representing parameter
   * information
   */
  std::vector<Widgets::Variable::Info> getComponentParametersInfo() const;

  /*!
   * In some cases we need to know whether a component has been attached
   * to the plugin. This helps with that.
   *
   * \return True if there is an attached component, False otherwise.
   */
  bool hasComponent() const { return plugin_component != nullptr; }

  /*!
   * Function called my the main window when loading settings to automatically
   * retrieve parameter values in widget.
   *
   * \param userprefs A reference to QSettings object where the settings will be
   *                  dumped
   */
  void loadParameterSettings(QSettings& userprefs);

  /*!
   * Pass in a QSetting object for the plugin to load custom values loaded by
   * the custom parameters.
   *
   * \param userprefs a standard QSettings object that the plugin can use to
   *                  dump all of the settings as key/value pairs where the key
   *                  is the name of the parameter and the value is the actual
   *                  parameter value to store in the QSetting
   */
  virtual void loadCustomParameterSettings(QSettings& userprefs);

  /*!
   * Function called my the main window when saving settings to automatically
   * store parameter from widget.
   *
   * \param userprefs A reference to QSettings object where the settings will be
   *                  dumped
   */
  void saveParameterSettings(QSettings& userprefs) const;

  /*!
   * Pass in a QSetting object for the plugin to save custom parameters
   *
   *
   * \param userprefs a standard QSettings object that the plugin can use to
   *                  dump all of the settings as key/value pairs where the key
   *                  is the name of the parameter and the value is the actual
   *                  parameter value to store in the QSetting
   */
  virtual void saveCustomParameterSettings(QSettings& userprefs) const;

  /*!
   * get a pointer to the internal block of the plugin
   *
   * \return IO::Block pointer to the internal structure
   */
  IO::Block* getBlock() { return plugin_component.get(); }
  const IO::Block* getBlock() const { return plugin_component.get(); }

protected:
  Widgets::Component* getComponent();
  Event::Manager* getEventManager();
  QMainWindow* getQMainWindow();
  Widgets::Panel* getPanel();

private:
  // owned pointers
  std::unique_ptr<Widgets::Component> plugin_component;

  // not owned pointers (managed by external objects)
  Event::Manager* event_manager = nullptr;
  QMainWindow* main_window = nullptr;  // Qt handles this lifetime
  Widgets::Panel* widget_panel = nullptr;  // Qt handles this lifetime

  std::string library;
  std::string name;
};

}  // namespace Widgets

#endif
