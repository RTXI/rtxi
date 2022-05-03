cmake_minimum_required(VERSION 3.20)

add_executable(rtxi 
    "debug.h"
    "debug.cpp"
    "mutex.h"
    "mutex.cpp"
    "fifo.h"
    "fifo.cpp"
    "rt.h" 
    "rt.cpp"
    "rt_os-posix.cpp"
    "event.h" 
    "event.cpp"
    "tempmain.cpp"
)

target_link_libraries(rtxi Qt5::Widgets Threads::Threads)
