cmake_minimum_required(VERSION 3.20)

include(cmake/prelude.cmake)

project(
    "rtxi"
    VERSION 3.0.0
    DESCRIPTION "The Real-Time eXperiment Interface"
    HOMEPAGE_URL "http://rtxi.org/"
    LANGUAGES CXX
)

configure_file(rtxiConfig.h.in rtxiConfig.h)
include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# We need to let third-party tools know how to compile our project
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_policy(SET CMP0135 NEW) 

# Find EVL Core library
include(FindPkgConfig)
pkg_check_modules(evl evl)

# Make sure we can find the rtxiConfig.h generated file in binary directory
include_directories(${PROJECT_BINARY_DIR})

#Find the packages
find_package(fmt REQUIRED)
find_package(HDF5 REQUIRED)
find_package(libgit2 REQUIRED)
find_package(qwt REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

#Handle QT and boost libraries shenanigans
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)

# Clang-tidy is by far the most annoying tool to please. Depending on your development
# environment you may get useless errors unless you add the line below.
list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

add_subdirectory(src)
#add_subdirectory(libs)
#add_subdirectory(plugins)
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
