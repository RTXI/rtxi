cmake_minimum_required(VERSION 3.14)

add_library(rtxi_lib
    "debug.hpp"
    "mutex.hpp"
    "fifo.hpp"
    "rt.hpp" 
    "event.hpp"
    "debug.cpp"
    "mutex.cpp"
    "fifo.cpp"
    "rt.cpp"
    "rt_os-posix.cpp"
    "event.cpp"
)

add_executable(rtxi_exe
    "tempmain.cpp"
)

add_executable(rtxi::exe ALIAS rtxi_exe)

set_target_properties(
    rtxi_exe PROPERTIES
    OUTPUT_NAME rtxi
    EXPORT_NAME exe
)

target_link_libraries(rtxi_exe rtxi_lib Qt5::Widgets Threads::Threads)
