#! /bin/bash
set -eu

#
# The Real-Time eXperiment Interface (RTXI) 
# 
# Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill
# Cornell Medical College
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <http://www.gnu.org/licenses/>.
#
# Created by Ivan F. Valerio <valerioif@gmail.com> 2023.11.36
#

STARTING_DIR=`pwd`

echo "Plugin name: "
read PLUGIN_NAME
PLUGIN_NAME_CLEANED=${PLUGIN_NAME//-/_}
PLUGIN_NAME_CLEANED=${PLUGIN_NAME_CLEANED// /_}
PLUGIN_NAME_CLEANED=${PLUGIN_NAME_CLEANED//[^a-zA-Z0-9_]/}

echo "Description: "
read PLUGIN_DESCRIPTION

OUTPUT_DIR=${STARTING_DIR}/${PLUGIN_NAME}
if [ ! -d "${OUTPUT_DIR}" ]; then
    mkdir ${OUTPUT_DIR}
fi
cd ${OUTPUT_DIR}

cat > widget.hpp << EOF

#include <string>
#include <rtxi/widgets.hpp>


// This is an generated header file. You may change the namespace, but 
// make sure to do the same in implementation (.cpp) file
namespace ${PLUGIN_NAME_CLEANED}
{

constexpr std::string_view MODULE_NAME = "${PLUGIN_NAME}";

enum PARAMETER : Widgets::Variable::Id
{
    // set parameter ids here
    FIRST_PARAMETER=0,
    SECOND_PARAMETER,
    THIRD_PARAMETER
};

inline  std::vector<Widgets::Variable::Info> get_default_vars() 
{
  return {
      {PARAMETER::FIRST_PARAMETER,
       "First Parameter Name",
       "First Parameter Description",     
       Widgets::Variable::INT_PARAMETER,
       int64_t{0}},
      {PARAMETER::SECOND_PARAMETER,
       "Second Parameter Name",
       "Second Parameter Description",
       Widgets::Variable::DOUBLE_PARAMETER,
       1.0},
      {PARAMETER::THIRD_PARAMETER,
       "Third Parameter Name",
       "Third Parameter Description",
       Widgets::Variable::UINT_PARAMETER,
       uint64_t{1}}
  };
}

inline std::vector<IO::channel_t> get_default_channels()
{
  return {
      {"First Channel Output Name", "First Channel Output Description", IO::OUTPUT},
      {"First Channel Input Name", "First Channel Input Description", IO::INPUT}
  };
}

class Panel : public Widgets::Panel
{
  Q_OBJECT
public:
  Panel(QMainWindow* main_window, Event::Manager* ev_manager);

  // Any functions and data related to the GUI are to be placed here
};

class Component : public Widgets::Component
{
public:
  explicit Component(Widgets::Plugin* hplugin);
  void execute() override;

  // Additional functionality needed for RealTime computation is to be placed here
};

class Plugin : public Widgets::Plugin
{
public:
  explicit Plugin(Event::Manager* ev_manager);
};

}  // namespace ${PLUGIN_NAME_CLEANED}
EOF

cat > widget.cpp << EOF
#include "widget.hpp"

${PLUGIN_NAME_CLEANED}::Plugin::Plugin(Event::Manager* ev_manager)
    : Widgets::Plugin(ev_manager, std::string(${PLUGIN_NAME_CLEANED}::MODULE_NAME))
{
}

${PLUGIN_NAME_CLEANED}::Panel::Panel(QMainWindow* main_window, Event::Manager* ev_manager)
    : Widgets::Panel(std::string(${PLUGIN_NAME_CLEANED}::MODULE_NAME), main_window, ev_manager)
{
  setWhatsThis("Template Plugin");
  createGUI(${PLUGIN_NAME_CLEANED}::get_default_vars(), {});  // this is required to create the GUI
}

${PLUGIN_NAME_CLEANED}::Component::Component(Widgets::Plugin* hplugin)
    : Widgets::Component(hplugin,
                         std::string(${PLUGIN_NAME_CLEANED}::MODULE_NAME),
                         ${PLUGIN_NAME_CLEANED}::get_default_channels(),
                         ${PLUGIN_NAME_CLEANED}::get_default_vars())
{
}

void ${PLUGIN_NAME_CLEANED}::Component::execute()
{
  // This is the real-time function that will be called
  switch (this->getState()) {
    case RT::State::EXEC:
      break;
    case RT::State::INIT:
      break;
    case RT::State::MODIFY:
      break;
    case RT::State::PERIOD:
      break;
    case RT::State::PAUSE:
      break;
    case RT::State::UNPAUSE:
      break;
    default:
      break;
  }
}

///////// DO NOT MODIFY BELOW //////////
// The exception is if your plugin is not going to need real-time functionality. 
// For this case just replace the craeteRTXIComponent return type to nullptr. RTXI
// will automatically handle that case and won't attach a component to the real
// time thread for your plugin.

std::unique_ptr<Widgets::Plugin> createRTXIPlugin(Event::Manager* ev_manager)
{
  return std::make_unique<${PLUGIN_NAME_CLEANED}::Plugin>(ev_manager);
}

Widgets::Panel* createRTXIPanel(QMainWindow* main_window,
                                Event::Manager* ev_manager)
{
  return new ${PLUGIN_NAME_CLEANED}::Panel(main_window, ev_manager);
}

std::unique_ptr<Widgets::Component> createRTXIComponent(
    Widgets::Plugin* host_plugin)
{
  return std::make_unique<${PLUGIN_NAME_CLEANED}::Component>(host_plugin);
}

Widgets::FactoryMethods fact;

extern "C"
{
Widgets::FactoryMethods* getFactories()
{
  fact.createPanel = &createRTXIPanel;
  fact.createComponent = &createRTXIComponent;
  fact.createPlugin = &createRTXIPlugin;
  return &fact;
}
};

//////////// END //////////////////////
EOF

cat > CMakeLists.txt << EOF
cmake_minimum_required(VERSION 3.14)

project(
    ${PLUGIN_NAME}
    VERSION 0.1.0
    DESCRIPTION "${PLUGIN_DESCRIPTION}"
    HOMEPAGE_URL "https://rtxi.org/"
    LANGUAGES CXX
)

# These lines help with third-party tooling integration
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(RTXI_CMAKE_SCRIPTS "" CACHE PATH "In development mode, RTXI uses this to expose conan dependencies")
set(CMAKE_PREFIX_PATH "\${RTXI_CMAKE_SCRIPTS}")
set(RTXI_PACKAGE_PATH "/usr/local/" CACHE PATH "Path hint to RTXI package information")
set(CMAKE_INSTALL_RPATH "\${RTXI_PACKAGE_PATH}/lib")
list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES \${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

# ---- find libraries ----
find_package(rtxi REQUIRED HINTS \${RTXI_PACKAGE_PATH})
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets HINTS \${RTXI_CMAKE_SCRIPTS})
find_package(fmt REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


#################################################################################################
### You can modify within this region for finding additional libraries and linking them to your #
### custom plugin. Make sure to install them prior to configuration or else build will fail     #
### with linking and include errors!                                                            # 
#################################################################################################
add_library(
    ${PLUGIN_NAME} MODULE
    widget.cpp
    widget.hpp
)

# Consult library website for how to link them to your plugin using cmake
target_link_libraries(${PLUGIN_NAME} PUBLIC 
    rtxi::rtxi rtxi::rtxidsp rtxi::rtxigen rtxi::rtxififo Qt5::Core Qt5::Gui Qt5::Widgets 
    dl fmt::fmt
)

################################################################################################ 

# We need to tell cmake to use the c++ version used to compile the dependent library or else...
get_target_property(REQUIRED_COMPILE_FEATURE rtxi::rtxi INTERFACE_COMPILE_FEATURES)
target_compile_features(${PLUGIN_NAME} PRIVATE \${REQUIRED_COMPILE_FEATURE})

install(
    TARGETS ${PLUGIN_NAME}
    DESTINATION \${RTXI_PACKAGE_PATH}/bin/rtxi_modules
)

EOF

cd ${STARTING_DIR}
