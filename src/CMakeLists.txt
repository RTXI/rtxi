cmake_minimum_required(VERSION 3.24)

if(${evl_FOUND})
  list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${evl_INCLUDE_DIRS})
  add_library(rtos_fifo_lib OBJECT "fifo.hpp" "fifo_evl.cpp")
  target_link_libraries(rtos_fifo_lib 
    PRIVATE 
      Threads::Threads
      ${evl_LINK_LIBRARIES}
      fmt::fmt
  )
  target_include_directories(rtos_fifo_lib SYSTEM PRIVATE ${evl_INCLUDE_DIRS})
  add_library(rtos_lib OBJECT "rtos.hpp" "rtos_evl.cpp")
  target_link_libraries(rtos_lib PRIVATE 
    ${evl_LINK_LIBRARIES}
    rtos_fifo_lib
    fmt::fmt
  )
  target_include_directories(rtos_lib SYSTEM PRIVATE ${evl_INCLUDE_DIRS})
else()
  message("EVL Library not found. Reverting to posix Interface.")
  add_library(rtos_fifo_lib OBJECT "fifo.hpp" "fifo.cpp")
  target_link_libraries(rtos_fifo_lib PRIVATE fmt::fmt)
  add_library(rtos_lib OBJECT "rtos.hpp" "rtos_posix.cpp")
  target_link_libraries(rtos_lib PRIVATE 
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
    daq.hpp daq.cpp
    main_window.hpp main_window.cpp
    module.hpp module.cpp
)

target_link_libraries(rtxi_lib PUBLIC 
    Threads::Threads
    fmt::fmt
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

# Create RTXI executable
add_executable(rtxi_exe
    tempmain.cpp
)
add_executable(rtxi::exe ALIAS rtxi_exe)
set_target_properties(
    rtxi_exe PROPERTIES
    OUTPUT_NAME rtxi
    EXPORT_NAME exe
)

# Link all of the static and dynamic libraries to rtxi
target_link_libraries(rtxi_exe PUBLIC 
    rtxi_lib
    rtos_fifo_lib
    rtos_lib 
    rtmath_lib
    performance_measurement_lib
    userprefs_lib
    system_control_lib
    Boost::headers
    backtrace
    dl
)

target_compile_definitions(rtxi_exe PRIVATE BOOST_STACKTRACE_USE_ADDR2LINE)
target_compile_definitions(rtxi_exe PRIVATE BOOST_STACKTRACE_USE_BACKTRACE)
