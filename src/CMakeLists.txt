cmake_minimum_required(VERSION 3.14)

add_library(rtxi_lib
    "debug.hpp"
    "debug.cpp"
    "fifo.hpp"
    "fifo.cpp"
    "rt.hpp" 
    "rt.cpp"
    "rt_os-evl.cpp"
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

target_link_libraries(rtxi_lib PRIVATE ${evl_LINK_LIBRARIES})
target_include_directories(rtxi_lib PRIVATE ${evl_INCLUDE_DIRS})
target_link_libraries(
    rtxi_exe rtxi_lib 
    Qt5::Widgets Threads::Threads
)
