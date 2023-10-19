cmake_minimum_required(VERSION 3.24)

if(DEFINED evl_FOUND AND RTXI_RT_CORE STREQUAL "evl")
  message("EVL library found. Using EVL rtos interface")
  list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${evl_INCLUDE_DIRS})
  add_library(dlplugin SHARED "dlplugin.hpp" "dlplugin.cpp")
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
elseif(DEFINED Xenomai_FOUND AND RTXI_RT_CORE STREQUAL "xenomai")
  message("Xenomai library found. using Cobalt Alchemy interface")
  #list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${xenomai3_ALCHEMY_CFLAGS})
  add_library(dlplugin SHARED "dlplugin.hpp" "dlplugin.cpp")
  add_library(xfifo SHARED "fifo.hpp" "fifo_xenomai.cpp")
  target_include_directories(xfifo PRIVATE ${XENOMAI_INCLUDE_DIRS})
  target_link_libraries(xfifo 
    PUBLIC
      ${XENOMAI_LIBRARIES}
    PRIVATE 
      fmt::fmt
  )
  target_compile_definitions(xfifo PRIVATE ${XENOMAI_DEFINITIONS})
  add_library(rtos SHARED "rtos.hpp" "rtos_xenomai.cpp")
  target_include_directories(rtos PRIVATE ${XENOMAI_INCLUDE_DIRS})
  target_link_libraries(rtos PRIVATE 
    xfifo
  PUBLIC
    ${XENOMAI_LIBRARIES}
    fmt::fmt
  )
  target_compile_definitions(rtos PRIVATE ${XENOMAI_DEFINITIONS})
else()
  message("RT core Library not found. Reverting to posix Interface.")
  add_library(dlplugin SHARED "dlplugin.hpp" "dlplugin.cpp")
  add_library(xfifo SHARED "fifo.hpp" "fifo_posix.cpp")
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
    widgets.hpp widgets.cpp
    logger.hpp logger.cpp
)

#target_include_directories(rtxi 
#  PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
#  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
#)
#
#target_include_directories(dlplugin
#  PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
#  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
#)
#
#target_include_directories(rtos
#  PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
#  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
#)
#
#target_include_directories(xfifo
#  PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
#  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
#)

target_link_libraries(rtxi 
  PRIVATE 
    Threads::Threads
    fmt::fmt
    qwt::qwt
    xfifo
    dlplugin
    Boost::headers
    backtrace
    dl
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

add_library(workspace OBJECT
    workspace.hpp workspace.cpp
)

target_link_libraries(workspace PRIVATE 
    rtxi
    dlplugin
    rtmath_lib
    performance_measurement_lib
    userprefs_lib
    system_control_lib
    connector_lib
    oscilloscope_lib
    data_recorder_lib
    module_installer_lib
    HDF5::HDF5
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    qwt::qwt
    Boost::headers
    backtrace
    dl
)

# Create RTXI executable
add_executable(rtxi_exe
    main_window.hpp main_window.cpp
    tempmain.cpp
)

add_executable(rtxi::exe ALIAS rtxi_exe)
set_target_properties(
    rtxi_exe PROPERTIES
    OUTPUT_NAME rtxi
    EXPORT_NAME exe
)

# Link all of the static and dynamic libraries to rtxi
target_link_libraries(rtxi_exe PRIVATE 
    xfifo
    rtos 
    rtxi
    rtmath_lib
    workspace
    performance_measurement_lib
    userprefs_lib
    system_control_lib
    connector_lib
    oscilloscope_lib
    data_recorder_lib
    module_installer_lib
    Boost::headers
    qwt::qwt
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    fmt::fmt
    HDF5::HDF5
    git2
    markdown
    backtrace
    dl
)

if(nidaqmx)
message("National Instruments Library found")
add_library(rtxinidaqdriver MODULE
  nidaq_driver.cpp
)

target_include_directories(rtxinidaqdriver
 	PUBLIC	
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(rtxinidaqdriver PRIVATE nidaqmx rtos)
endif()

target_compile_features(rtxi PUBLIC cxx_std_17)
target_compile_features(rtos PUBLIC cxx_std_17)
target_compile_features(xfifo PUBLIC cxx_std_17)
target_compile_features(dlplugin PUBLIC cxx_std_17)
target_compile_features(rtxi_exe PRIVATE cxx_std_17)

target_compile_definitions(rtxi_exe PRIVATE BOOST_STACKTRACE_USE_ADDR2LINE)
target_compile_definitions(rtxi_exe PRIVATE BOOST_STACKTRACE_USE_BACKTRACE)
target_compile_definitions(rtxi PRIVATE BOOST_STACKTRACE_USE_ADDR2LINE)
target_compile_definitions(rtxi PRIVATE BOOST_STACKTRACE_USE_BACKTRACE)
