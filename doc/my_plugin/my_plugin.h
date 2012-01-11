/*
 * This is a template header file for a user modules derived from
 * DefaultGUIModel using the default GUI.
 */

#include <default_gui_model.h>

class MyPlugin : public DefaultGUIModel
{

public:

  MyPlugin(void);
  virtual
  ~MyPlugin(void);

  void
  execute(void);

protected:

  virtual void
  update(DefaultGUIModel::update_flags_t);

private:

  double some_parameter;
  double some_state;
  double period;

private slots:

};
