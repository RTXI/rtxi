cmake_minimum_required(VERSION 3.14)

add_executable(rtxi_exe
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
    "tempmain.cpp"
)

add_executable(rtxi::exe ALIAS rtxi_exe)

set_target_properties(
    rtxi_exe PROPERTIES
    OUTPUT_NAME rtxi
    EXPORT_NAME exe
)

target_link_libraries(rtxi_exe PUBLIC Qt5::Widgets Threads::Threads)
