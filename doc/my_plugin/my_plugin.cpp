/*
 * This is a template implementation file for a user module derived from
 * DefaultGUIModel using the default GUI.
 */

#include <my_plugin.h>
#include <qwhatsthis.h>

extern "C" Plugin::Object *
createRTXIPlugin(void)
{
  return new MyPlugin();
}

static DefaultGUIModel::variable_t vars[] =
  {
    { "GUI label", "Tooltip description", DefaultGUIModel::PARAMETER
        | DefaultGUIModel::DOUBLE, },
    { "A State", "Tooltip description", DefaultGUIModel::STATE, }, };

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

MyPlugin::MyPlugin(void) :
  DefaultGUIModel("MyPlugin", ::vars, ::num_vars)
{

  QWhatsThis::add(this, "<p><b>MyPlugin:</b><br>QWhatsThis description.</p>");
  update( INIT); // this is optional, you may place initialization code directly into the constructor
  createGUI(vars, num_vars); // this is required to create the GUI
  refresh(); // this is required to update the GUI with parameter and state values
}

MyPlugin::~MyPlugin(void)
{
}

void
MyPlugin::execute(void)
{
  return;
}

void
MyPlugin::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag)
    {
  case INIT:
    period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
    setParameter("GUI label", some_parameter);
    setState("A State", some_state);
    break;
  case MODIFY:
    some_parameter = getParameter("GUI label").toDouble();
    break;
  case UNPAUSE:
    break;
  case PAUSE:
    break;
  case PERIOD:
    period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
    break;
  default:
    break;

    }

}
