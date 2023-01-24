cmake_minimum_required(VERSION 3.24)

add_library(rtmath_lib OBJECT
    powfast.hpp powfast.cpp
    rtmath.h rtmath.cpp
    runningstat.h runningstat.cpp
)

#target_link_libraries(rtmath_lib PRIVATE rtxi_lib)
