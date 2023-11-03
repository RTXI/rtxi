cmake_minimum_required(VERSION 3.24)

include(cmake/prelude.cmake)

project(
    "rtxi"
    VERSION 3.0.0
    DESCRIPTION "Real-Time eXperiment Interface"
    HOMEPAGE_URL "http://rtxi.org/"
    LANGUAGES CXX
)

# --- Some installation shenanigans
# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)
 
# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
 
# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/rtxi")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# ----

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake)
set(RTXI_RT_CORE "posix" CACHE STRING 
  "The real-time core to use. Accepted values are posix, evl, and xenomai")
set(RTXI_USER_HOMEDIR $ENV{HOME})
set(RTXI_SETTINGS_DIR ${RTXI_USER_HOMEDIR}/rtxi/)
set(XENOMAI_ROOT_DIR "/usr/xenomai")
configure_file(rtxiConfig.h.in rtxiConfig.h)
include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)
include(GNUInstallDirs)

# We need to let third-party tools know how to compile our project
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_policy(SET CMP0135 NEW) 

# Find EVL Core library
include(FindPkgConfig)
pkg_check_modules(evl evl)

# some libraries do not have cmake config files, so we use pkg-config for those
pkg_check_modules(git2 REQUIRED libgit2)
pkg_check_modules(markdown REQUIRED libmarkdown)

# Make sure we can find the rtxiConfig.h generated file in binary directory
include_directories(${PROJECT_BINARY_DIR})

# Find xenomai 3 library
find_package(Xenomai 3)

#Find the packages
find_package(fmt REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS C HL)
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network OpenGL Svg Xml)
find_package(qwt REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED)
find_package(GSL REQUIRED)
#Some libraries are harder to find as package, so this solves that
find_library(nidaqmx NAMES libnidaqmx.so)

#Handle QT and boost libraries shenanigans
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(
  ${CMAKE_SOURCE_DIR}/src 
  ${CMAKE_SOURCE_DIR}/libs
  ${CMAKE_SOURCE_DIR}/plugins
)

# Clang-tidy is by far the most annoying tool to please. Depending on your development
# environment you may get useless errors unless you add the line below.
list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

add_subdirectory(libs)
add_subdirectory(src)
add_subdirectory(plugins)
#add_subdirectory(res)
#add_subdirectory(scripts)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT rtxi_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of rtxi"
  )
endif()

include(cmake/dev-mode.cmake)
