cmake_minimum_required(VERSION 3.20)

add_executable(rtxi_exe
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

add_executable(rtxi::exe ALIAS rtxi_exe)

set_target_properties(
    rtxi_exe PROPERTIES
    OUTPUT_NAME rtxi
    EXPORT_NAME exe
)

target_link_libraries(rtxi_exe Qt5::Widgets Threads::Threads)
