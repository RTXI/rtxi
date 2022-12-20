cmake_minimum_required(VERSION 3.24)

add_library(performance_measurement_lib OBJECT
    performance_measurement.hpp
    performance_measurement.cpp
)

target_link_libraries(performance_measurement_lib PRIVATE 
    rtxi_lib
    Qt5::Core Qt5::Widgets Qt5::Gui
    fmt::fmt
)
