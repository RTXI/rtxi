cmake_minimum_required(VERSION 3.24)

add_library(oscilloscope_lib OBJECT
    scope.h
    scope.cpp
    oscilloscope.h
    oscilloscope.cpp
)

target_link_libraries(oscilloscope_lib PUBLIC
    rtos
    rtxi
    qwt::qwt
)
