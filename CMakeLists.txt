
# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.20)

project ("rtxi" 
    VERSION 3.0.0 
    DESCRIPTION "The Real-Time eXperiment Interface"
    HOMEPAGE_URL "http://rtxi.org/"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# Let's find some packages and libraries
find_package(Qt5 COMPONENTS Widgets REQUIRED)
#add_library()

# Include sub-projects.
add_subdirectory("src")
#add_subdirectory("libs")
#add_subdirectory("plugins")
#add_subdirectory("res")
#add_subdirectory("tests")

# Installation targets
install(TARGETS rtxi DESTINATION bin)

configure_file(rtxiConfig.h.in 
    rtxiConfig.h)

target_include_directories(${PROJECT_NAME} 
    PUBLIC ${PROJECT_BINARY_DIR}
)


target_link_libraries(${PROJECT_NAME} 
    Qt5::Widgets
)

# ---- Install Rules start here ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

if(NOT testcmake_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of testcmake"
  )
endif()

include(cmake/dev-mode.cmake)
