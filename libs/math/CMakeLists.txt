cmake_minimum_required(VERSION 3.24)

add_library(rtximath SHARED 
    powfast.hpp powfast.cpp
    rtmath.h rtmath.cpp
    runningstat.h runningstat.cpp
)
