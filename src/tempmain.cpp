#include <iostream>

#include "rt.hpp"
#include "rtxiConfig.h"

int main(int argc, char* argv[])
{
  std::cout << "Welcome to RTXI Version ";
  std::cout << RTXI_VERSION_MAJOR << ".";
  std::cout << RTXI_VERSION_MINOR << ".";
  std::cout << RTXI_VERSION_PATCH << "\n";
  RT::System* system = RT::System::getInstance();
}
