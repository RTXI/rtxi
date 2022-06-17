#include <iostream>

#include "rtos.hpp"
#include "rtxiConfig.h"

void test_func(int retval){
  std::cout << "test function reached.\n";
  retval = 0;
}

int main()
{
  std::cout << "Welcome to RTXI Version ";
  std::cout << RTXI_VERSION_MAJOR << ".";
  std::cout << RTXI_VERSION_MINOR << ".";
  std::cout << RTXI_VERSION_PATCH << "\n";
  auto task = std::make_unique<RT::OS::Task>();
  int retval = 0;
  int function_retval = 0;
  retval = RT::OS::createTask<int>(task.get(), test_func, function_retval);
  RT::OS::deleteTask(task.get());
  return retval;
}
