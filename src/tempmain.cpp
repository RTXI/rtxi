#include <iostream>
#include <signal.h>

#include <QApplication>
#include <boost/stacktrace.hpp>

#include "rtxiConfig.h"
#include "debug.hpp"
#include "rt.hpp"
#include "module.hpp"
#include "main_window.hpp"

static void signal_handler(int signum)
{
  static int count = 0;

  ERROR_MSG("signal_handler : signal type {} received\n", signum);
  std::cerr << boost::stacktrace::stacktrace();
}

int main(int argc, char *argv[])
{
  signal(SIGINT, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGSEGV, signal_handler);

  std::cout << "Welcome to RTXI Version ";
  std::cout << RTXI_VERSION_MAJOR << ".";
  std::cout << RTXI_VERSION_MINOR << ".";
  std::cout << RTXI_VERSION_PATCH << "\n";

  // Initializing core classes
  auto event_manager = std::make_unique<Event::Manager>();
  auto rt_connector = std::make_unique<RT::Connector>();
  auto rt_system = std::make_unique<RT::System>(event_manager.get(), rt_connector.get());

  // Initializing GUI
  QApplication::setDesktopSettingsAware(false);
  QApplication* app = new QApplication(argc, argv);
  app->connect(app, SIGNAL(lastWindowClosed()), app, SLOT(quit()));

  auto rtxi_window = std::make_unique<MainWindow>(event_manager.get());
  auto mod_manager = std::make_unique<Modules::Manager>(event_manager.get(), rtxi_window.get());
  rtxi_window->loadWindow();
  int retval = app->exec();
  delete app;
  return retval;
}
