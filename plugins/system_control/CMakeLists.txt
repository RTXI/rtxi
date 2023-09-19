cmake_minimum_required(VERSION 3.24)

add_library(system_control_lib OBJECT
    system_control.h
    system_control.cpp
)

target_link_libraries(system_control_lib PRIVATE 
    rtxi
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
)
