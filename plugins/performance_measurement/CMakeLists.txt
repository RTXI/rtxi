cmake_minimum_required(VERSION 3.24)

add_library(performance_measurement_lib OBJECT
    performance_measurement.hpp
    performance_measurement.cpp
)

target_link_libraries(performance_measurement_lib PUBLIC
    rtxi
    rtmath_lib
)
