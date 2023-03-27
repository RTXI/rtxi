cmake_minimum_required(VERSION 3.24)

if(DEFINED evl_FOUND AND RTXI_RT_CORE STREQUAL "evl")
  list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${evl_INCLUDE_DIRS})
  add_library(xfifo SHARED "fifo.hpp" "fifo_evl.cpp")
  target_link_libraries(xfifo 
    PRIVATE 
      Threads::Threads
      ${evl_LINK_LIBRARIES}
      fmt::fmt
  )
  target_include_directories(xfifo SYSTEM PRIVATE ${evl_INCLUDE_DIRS})
  add_library(rtos SHARED "rtos.hpp" "rtos_evl.cpp")
  target_link_libraries(rtos PRIVATE 
    ${evl_LINK_LIBRARIES}
    xfifo
    fmt::fmt
  )
  target_include_directories(rtos SYSTEM PRIVATE ${evl_INCLUDE_DIRS})
elseif(xenomai3_FOUND AND RTXI_RT_CORE STREQUAL "xenomai")
  list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${xenomai_ALCHEMY_CFLAGS})
  add_library(xfifo SHARED "fifo.hpp" "fifo_xenomai.cpp")
  target_link_libraries(xfifo 
    PRIVATE 
      Threads::Threads
      ${xenomai_ALCHEMY_LDFLAGS}
      fmt::fmt
  )
  target_include_directories(xfifo SYSTEM PRIVATE ${xenomai_ALCHEMY_CFLAGS})
  add_library(rtos SHARED "rtos.hpp" "rtos_evl.cpp")
  target_link_libraries(rtos PRIVATE 
    ${xenomai_ALCHEMY_LDFLAGS}
    xfifo
    fmt::fmt
  )
  target_include_directories(rtos SYSTEM PRIVATE ${xenomai_ALCHEMY_CFLAGS})

else()
  message("EVL Library not found. Reverting to posix Interface.")
  add_library(xfifo SHARED "fifo.hpp" "fifo.cpp")
  target_link_libraries(xfifo PRIVATE fmt::fmt)
  add_library(rtos SHARED "rtos.hpp" "rtos_posix.cpp")
  target_link_libraries(rtos PRIVATE 
      xfifo
      fmt::fmt
  )
endif()


# Create static library for rtxi core components
add_library(rtxi SHARED 
    debug.hpp debug.cpp
    event.hpp event.cpp
    io.hpp io.cpp
    rt.hpp rt.cpp
    daq.hpp daq.cpp
    main_window.hpp main_window.cpp
    module.hpp module.cpp
)

target_link_libraries(rtxi PUBLIC 
    Threads::Threads
    fmt::fmt
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    xfifo
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
    rtxi
    xfifo
    rtos 
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
