cmake_minimum_required(VERSION 3.24)

add_library(oscilloscope_lib OBJECT
    scope.hpp
    scope.cpp
    oscilloscope.hpp
    oscilloscope.cpp
)

target_link_libraries(oscilloscope_lib PRIVATE 
    rtos
    rtxi
    qwt::qwt
    Qt5::Core
    Qt5::Widgets
)
