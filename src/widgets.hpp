#ifndef WIDGET_HPP
#define WIDGET_HPP

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

#include "dlplugin.hpp"
#include "event.hpp"
#include "io.hpp"
#include "rt.hpp"

// These metatype declarations are needed by qt to store
// the types in QVariant, which is very convenient and reduces
// unecessary repetiotion in panel construction
Q_DECLARE_METATYPE(IO::Block*)
Q_DECLARE_METATYPE(IO::flags_t)
Q_DECLARE_METATYPE(IO::endpoint)
Q_DECLARE_METATYPE(RT::block_connection_t)
Q_DECLARE_METATYPE(RT::State::state_t)

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
 *
 * \sa IO::Block::Block()
 */
struct Info
{
  size_t id = 0;
  std::string name;
  std::string description;
  Widgets::Variable::variable_t vartype;
  std::variant<int64_t, double, uint64_t, std::string, RT::State::state_t>
      value;
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
  std::unique_ptr<Widgets::Plugin> (*createPlugin)(Event::Manager*) = nullptr;
  std::unique_ptr<Widgets::Component> (*createComponent)(Widgets::Plugin*) =
      nullptr;
  Widgets::Panel* (*createPanel)(QMainWindow*, Event::Manager*) = nullptr;
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

  RT::State::state_t getState() const { return this->component_state; }
  void setState(RT::State::state_t state) { this->component_state = state; }
  std::vector<Widgets::Variable::Info> getParametersInfo()
  {
    return this->parameters;
  }

private:
  std::vector<Widgets::Variable::Info> parameters;
  Widgets::Plugin* hostPlugin;
  bool active;
  RT::State::state_t component_state;
};

class Panel : public QWidget
{
  Q_OBJECT
public:
  Panel(const std::string& mod_name,
        QMainWindow* mw,
        Event::Manager* ev_manager);

  QMdiSubWindow* getMdiWindow() { return this->m_subwindow; }

  /*!
   * Function that builds the Qt GUI.
   *
   * \param vars The structure defining the module's parameters, states, inputs,
   *    and outputs.
   * \param skip_ids A vector of IDs that this function should not consider
   *    building interface for.
   *
   */
  virtual void createGUI(const std::vector<Widgets::Variable::Info>& vars,
                         const std::vector<Widgets::Variable::Id>& skip_ids);

  /*!
   * Assigns a plugin to this panel. Typically used during construction of the
   * module and should not be used beyond initialization.
   *
   * \param hplugin A pointer to the host plugin this panel belongs to.
   */
  void setHostPlugin(Widgets::Plugin* hplugin) { this->hostPlugin = hplugin; }

signals:
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

  /*!
   * Callback function that is called when the system state changes.
   *
   * \param flag The kind of update to signal.
   */
  virtual void update_state(RT::State::state_t flag);

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
  Widgets::Plugin* getHostPlugin() { return this->hostPlugin; }

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

  struct param_t
  {
    QLabel* label;
    QString str_value;
    DefaultGUILineEdit* edit;
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

  size_t getID();
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

  template<typename T>
  int setComponentParameter(const Variable::Id& parameter_id, T value)
  {
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

  // These functions are here in order to have backwards compatibility
  // with previous versions of RTXI that used DefaultGuiModel
  // (before RTXI 3.0.0)

  void registerComponent();
  void setComponentState(RT::State::state_t state);
  virtual std::vector<Widgets::Variable::Info> getComponentParametersInfo();

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
