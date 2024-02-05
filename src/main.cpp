#include <QApplication>
#include <iostream>

#include <signal.h>
#include <string.h>

#include "debug.hpp"
#include "main_window.hpp"
#include "rt.hpp"
#include "rtxiConfig.h"
#include "workspace.hpp"

namespace
{
void signal_handler(int signum)
{
  // NOLINTNEXTLINE
  ERROR_MSG("signal_handler : signal type {} received\n", ::strsignal(signum));
  PRINT_BACKTRACE();
  exit(-1);  // NOLINT
}
}  // namespace

int main(int argc, char* argv[])
{
  if (signal(SIGINT, signal_handler) == SIG_ERR) {
    ERROR_MSG("MAIN: Unable to set SIGINT signal handler");
    return -1;
  }
  if (signal(SIGABRT, signal_handler) == SIG_ERR) {
    ERROR_MSG("MAIN: Unable to set SIGABRT signal handler");
    return -1;
  }
  if (signal(SIGSEGV, signal_handler) == SIG_ERR) {
    ERROR_MSG("MAIN: Unable to set SIGSEGV signal handler");
    return -1;
  }

  std::cout << "Welcome to RTXI Version ";
  std::cout << RTXI_VERSION_MAJOR << ".";
  std::cout << RTXI_VERSION_MINOR << ".";
  std::cout << RTXI_VERSION_PATCH << "\n";

  // Initializing core classes
  auto event_manager = std::make_unique<Event::Manager>();
  auto rt_connector = std::make_unique<RT::Connector>();
  auto rt_system =
      std::make_unique<RT::System>(event_manager.get(), rt_connector.get());
  rt_system->createTelemitryProcessor();
  // Initializing GUI
  // QApplication::setDesktopSettingsAware(false);
  auto* app = new QApplication(argc, argv);
  QApplication::connect(app, SIGNAL(lastWindowClosed()), app, SLOT(quit()));

  auto* rtxi_window = new MainWindow(event_manager.get());
  auto mod_manager = std::make_unique<Workspace::Manager>(event_manager.get());
  rtxi_window->loadWindow();
  const int retval = QApplication::exec();
  delete rtxi_window;
  delete app;
  return retval;
}
