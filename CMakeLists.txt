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


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find EVL Core library
include(FindPkgConfig)
pkg_check_modules(evl evl)

# Make sure we can find the rtxiConfig.h generated file in binary directory
include_directories(${PROJECT_BINARY_DIR})

#Find the packages
find_package(Qt5 COMPONENTS Core Widgets Gui Network REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED) 

#Handle QT and boost libraries shenanigans
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED OFF)  
set(Boost_USE_STATIC_RUNTIME OFF)

#find_library(BOOST_STACKTRACE_BACKTRACE_LIBRARY NAMES boost_stacktrace_backtrace)
#find_library(BOOST_STACKTRACE_BASIC_LIBRARY NAMES Boost_stacktrace_basic)

add_subdirectory(src)
#add_subdirectory(libs)
#add_subdirectory(plugins)
#add_subdirectory(res)
#add_subdirectory(scripts)

if(${evl_FOUND})
  set(RTOS_INCLUDES ${evl_INCLUDE_DIRS})
else()
  set(RTOS_INCLUDES "")
endif()

# Clang-tidy is by far the most annoying tool to please. Depending on your development
# environment you may get useless errors unless you add the line below.
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES 
  ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}
  RTOS_INCLUDES
)

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
