#include <iostream>

#include <QApplication>

//#include "rtos.hpp"
#include "rtxiConfig.h"
#include "main_window.hpp"

int main(int argc, char *argv[])
{
  std::cout << "Welcome to RTXI Version ";
  std::cout << RTXI_VERSION_MAJOR << ".";
  std::cout << RTXI_VERSION_MINOR << ".";
  std::cout << RTXI_VERSION_PATCH << "\n";
  QApplication::setDesktopSettingsAware(false);
  QApplication* app = new QApplication(argc, argv);
  app->connect(app, SIGNAL(lastWindowClosed()), app, SLOT(quit()));
  auto rtxi_window = std::make_unique<MainWindow>();
  rtxi_window->loadWindow();
  int retval = app->exec();
  return 0;
}
