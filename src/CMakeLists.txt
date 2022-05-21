cmake_minimum_required(VERSION 3.20)

if(${evl_FOUND})
  add_library(rt_os_lib OBJECT "rtos.hpp" "rtos_evl.cpp")
  target_link_libraries(rt_os_lib PUBLIC ${evl_LINK_LIBRARIES} fmt::fmt)
  target_include_directories(rt_os_lib SYSTEM PUBLIC ${evl_INCLUDE_DIRS})
  #target_compile_options(rt_os_lib PUBLIC ${evl_CFLAGS})
else()
  message("EVL Library not found. Reverting to POSIX Interface.")
  add_library(rt_os_lib OBJECT "rtos.hpp" "rtos_posix.cpp")
  target_link_libraries(rt_os_lib PUBLIC fmt::fmt)
endif()


# Create static library for rtxi core components
add_library(rtxi_lib OBJECT
    "debug.hpp"
    "debug.cpp"
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
    rt_os_lib 
    Qt5::Widgets 
    Threads::Threads
)
