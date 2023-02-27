#ifndef MODULE_HPP
#define MODULE_HPP

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QValidator>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "event.hpp"
#include "io.hpp"
#include "main_window.hpp"
#include "rt.hpp"

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
  DefaultGUILineEdit(QWidget* parent);

  void blacken();
  QPalette palette;

public slots:
  void redden();
};  // class DefaultGUILineEdi

// Forward declare plugin class for the component and panel private pointers
class Plugin;

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

  void execute() override;

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
        MainWindow* mw,
        Event::Manager* event_manager);

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
  virtual void createGUI(const std::vector<Modules::Variable::Info>& vars,
                         MainWindow* mw);

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
  void setState(const QString& name, Modules::Variable::state_t ref);

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
  std::string getName() { return this->name; }

  /*!
   * retrieve the host plugin that controls this panel
   *
   * \return pointer to host plugin
   */
  Modules::Plugin* getHostPlugin() { return this->hostPlugin; }

  /*!
   * Retrieve the main window for the application
   *
   * \return pointer of type MainWindow that controls the RTXI application
   */
  MainWindow* getMainWindowPtr() { return this->main_window; }

  /*!
   * Obtain the event manager attached to this session of the RTXI application.
   * This is useful for lower level control of RTXI events.
   *
   * \return Pointer to the RTXI event manager
   */
  Event::Manager* getRTXIEventManager() { return this->event_manager; }

private:
  MainWindow* main_window = nullptr;
  QWidget* gridBox = nullptr;
  QGroupBox* buttonGroup = nullptr;
  std::string name;
  QMdiSubWindow* subWindow = nullptr;
  Modules::Plugin* hostPlugin = nullptr;
  QGridLayout* layout = nullptr;
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
  Plugin(Event::Manager* ev_manager,
         MainWindow* mw,
         const std::string& mod_name);
  Plugin(const Plugin& plugin) = delete;  // copy constructor
  Plugin& operator=(const Plugin& plugin) =
      delete;  // copy assignment noperator
  Plugin(Plugin&&) = delete;  // move constructor
  Plugin& operator=(Plugin&&) = delete;  // move assignment operator
  virtual ~Plugin();

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
  int64_t getComponentIntParameter(const size_t& parameter_id);

  /*!
   * Retrieves an unsigned integer parameter from the component object. Usually
   * called from a non-realtime context
   *
   * \param parameter_id Identification number of the parameter
   *
   * \return the unsigned integer value
   */
  uint64_t getComponentUIntParameter(const size_t& parameter_id);

  /*!
   * Retrieves a double parameter from the component object. Usually called
   * from a non-realtime context
   *
   * \param parameter_id Identification number of the parameter
   *
   * \return the double value
   */
  double getComponentDoubleParameter(const size_t& parameter_id);

  /*!
   * Assigns a new integer value to the parameter
   *
   * \param parameter_id Identification number of the parameter
   * \param value the new value
   *
   * \return 0 if successful -1 otherwise
   */
  int setComponentIntParameter(const size_t& parameter_id, int64_t value);

  /*!
   * Assigns a new double value to the parameter
   *
   * \param parameter_id Identification number of the parameter
   * \param value the new value
   *
   * \return 0 if successful -1 otherwise
   */
  int setComponentDoubleParameter(const size_t& parameter_id, double value);

  /*!
   * Assigns a new unsigned integer value to the parameter
   *
   * \param parameter_id Identification number of the parameter
   * \param value the new value
   *
   * \return 0 if successful -1 otherwise
   */
  int setComponentUintParameter(const size_t& parameter_id, uint64_t value);

  /*!
   * Assigns a new comment value to the parameter
   *
   * \param parameter_id Identification number of the parameter
   * \param value the new value
   *
   * \return 0 if successful -1 otherwise
   */
  int setComponentComment(const size_t& parameter_id, std::string value);

  /*!
   * Assigns a new state value to the component
   *
   * \param parameter_id Identification number of the parameter
   * \param value the new value
   *
   * \return 0 if successful -1 otherwise
   */
  int setComponentState(const size_t& parameter_id,
                        Modules::Variable::state_t value);

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
   * For dynamically loadable modules in RTXI, this function is called
   * to obtain a handle to the shared library.
   *
   * \return a void pointer representing the library handle
   */
  void* getHandle() { return this->handle; }

  /*!
   * Get the name of the library from which the object was loaded.
   *
   * \return The library file the object from which the object was created.
   */
  std::string getLibrary() const { return this->library; }

  std::unique_ptr<Modules::Plugin> load();

  /*!
   * A mechanism which an object can use to unload itself. Should only be
   *   called from within the GUI thread.
   */
  void unload();

  // These functions are here in order to have backwards compatibility
  // with previous versions of RTXI that used DefaultGuiModel
  // (before RTXI 3.0.0)

  void registerComponent();

protected:
  // owned pointers
  std::unique_ptr<Modules::Component> plugin_component;

  // not owned pointers (managed by external objects)
  Event::Manager* event_manager = nullptr;
  MainWindow* main_window = nullptr;  // Qt handles this lifetime
  Modules::Panel* widget_panel = nullptr;  // Qt handles this lifetime

private:
  std::string library;
  void* handle =
      nullptr;  // if it is a shared object then this will not be null
  std::string name;
};

/*!
 * This structure contains functions for creating instances. It is used when
 * loading and unloading RTXI modules dynamically
 */
struct FactoryMethods
{
  std::unique_ptr<Modules::Plugin> (*createPlugin)(Event::Manager*,
                                                   MainWindow*) = nullptr;
  std::unique_ptr<Modules::Component> (*createComponent)(Modules::Plugin*) =
      nullptr;
  Modules::Panel* (*createPanel)(MainWindow*, Event::Manager*) = nullptr;
};

/*!
 * This class is responsible for managing module loading and unloading
 */
class Manager : public Event::Handler
{
public:
  Manager(Event::Manager* event_manager, MainWindow* mw);
  ~Manager();

  /*!
   * loads plugin
   */
  int loadPlugin(const std::string& library);

  /*!
   * unloads plugin
   */
  void unloadPlugin(Modules::Plugin* plugin);

  /*!
   * Handles plugin loading/unloadin gevents from gui thread
   */
  void receiveEvent(Event::Object* event) override;

  /*!
   * Checks whether plugin is registered
   */
  bool isRegistered(const Modules::Plugin* plugin);

private:
  void registerModule(std::unique_ptr<Modules::Plugin> module);
  void unregisterModule(Modules::Plugin* plugin);

  void registerFactories(std::string module_name, Modules::FactoryMethods);
  void unregisterFactories(std::string module_name);

  std::unordered_map<std::string, std::vector<std::unique_ptr<Modules::Plugin>>>
      rtxi_modules_registry;
  std::unordered_map<std::string, Modules::FactoryMethods>
      rtxi_factories_registry;
  Event::Manager* event_manager;
  MainWindow* main_window;
};

}  // namespace Modules

#endif
