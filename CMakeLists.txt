cmake_minimum_required(VERSION 3.20)

project(
    "rtxi"
    VERSION 3.0.0
    DESCRIPTION "The Real-Time eXperiment Interface"
    HOMEPAGE_URL "http://rtxi.org/"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Make sure to complain for the tiniest things
add_compile_options(-Wall -Wextra -pedantic)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets Gui Network REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(src)
#add_subdirectory(libs)
#add_subdirectory(plugins)
#add_subdirectory(res)
#add_subdirectory(scripts)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_BINARY_DIR}
)

include_directories(src)

