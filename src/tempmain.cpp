#include <QApplication>
#include <iostream>

#include <boost/stacktrace.hpp>
#include <signal.h>
#include <string.h>

#include "debug.hpp"
#include "main_window.hpp"
#include "module.hpp"
#include "rt.hpp"
#include "rtxiConfig.h"

static void signal_handler(int signum)
{
  static int count = 0;

  ERROR_MSG("signal_handler : signal type {} received\n", ::strsignal(signum));
  std::cerr << boost::stacktrace::stacktrace();
  exit(-1);
}

int main(int argc, char* argv[])
{
  signal(SIGINT, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGSEGV, signal_handler);

  std::cout << "Welcome to RTXI Version ";
  std::cout << RTXI_VERSION_MAJOR << ".";
  std::cout << RTXI_VERSION_MINOR << ".";
  std::cout << RTXI_VERSION_PATCH << "\n";

  std::cout << "Defalt config location: " << RTXI_DEFAULT_SETTINGS_DIR << "\n";

  // Initializing core classes
  auto event_manager = std::make_unique<Event::Manager>();
  auto rt_connector = std::make_unique<RT::Connector>();
  auto rt_system =
      std::make_unique<RT::System>(event_manager.get(), rt_connector.get());

  // Initializing GUI
  QApplication::setDesktopSettingsAware(false);
  auto* app = new QApplication(argc, argv);
  app->connect(app, SIGNAL(lastWindowClosed()), app, SLOT(quit()));

  MainWindow* rtxi_window = new MainWindow(event_manager.get());
  auto mod_manager =
      std::make_unique<Modules::Manager>(event_manager.get(), rtxi_window);
  rtxi_window->loadWindow();
  int retval = app->exec();
  delete rtxi_window;
  delete app;
  return retval;
}
