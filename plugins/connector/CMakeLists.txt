cmake_minimum_required(VERSION 3.24)

add_library(connector_lib OBJECT
    connector.h
    connector.cpp
)

target_link_libraries(connector_lib PRIVATE 
    rtxi
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
)
