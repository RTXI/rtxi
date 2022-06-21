cmake_minimum_required(VERSION 3.20)

if(${evl_FOUND})
    add_library(rtos_fifo_lib OBJECT "fifo.hpp" "fifo_evl.cpp")
    target_link_libraries(rtos_fifo_lib PUBLIC fmt::fmt)
    add_library(rtos_lib OBJECT "rtos.hpp" "rtos_evl.cpp")
    target_link_libraries(rtos_lib PUBLIC 
        ${evl_LINK_LIBRARIES}
        rtos_fifo_lib
        fmt::fmt
    )
    target_include_directories(rtos_lib SYSTEM PUBLIC ${evl_INCLUDE_DIRS})
else()
    message("EVL Library not found. Reverting to posix Interface.")
    add_library(rtos_fifo_lib OBJECT "fifo.hpp" "fifo.cpp")
    target_link_libraries(rtos_fifo_lib PUBLIC fmt::fmt)
    add_library(rtos_lib OBJECT "rtos.hpp" "rtos_posix.cpp")
    target_link_libraries(rtos_lib PUBLIC 
        rtos_fifo_lib
        fmt::fmt
    )
endif()


# Create static library for rtxi core components
add_library(rtxi_lib OBJECT
    debug.hpp debug.cpp
    event.hpp event.cpp
    io.hpp io.cpp
    rt.hpp rt.cpp
)

target_link_libraries(rtxi_lib PUBLIC 
    Threads::Threads
    fmt::fmt
)

# Create RTXI executable
add_executable(rtxi_exe
    "tempmain.cpp"
)
add_executable(rtxi::exe ALIAS rtxi_exe)
set_target_properties(
    rtxi_exe PROPERTIES
    OUTPUT_NAME rtxi
    EXPORT_NAME exe
)

# Link all of the static and dynamic librarise to rtxi
target_link_libraries(rtxi_exe 
    rtxi_lib 
    rtos_lib 
    Qt5::Widgets 
    Threads::Threads
)
