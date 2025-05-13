if(DEFINED evl_FOUND AND RTXI_RT_CORE STREQUAL "evl")
  message("EVL library found. Using EVL rtxipal interface")
  list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${evl_INCLUDE_DIRS})
  add_library(rtxiplugin SHARED "dlplugin.hpp" "dlplugin.cpp")
  target_link_libraries(rtxiplugin PRIVATE fmt::fmt)
  add_library(rtxififo SHARED "fifo.hpp" "fifo_evl.cpp")
  target_link_libraries(rtxififo 
    PRIVATE 
      Threads::Threads
      ${evl_LINK_LIBRARIES}
      fmt::fmt
  )
  target_include_directories(rtxififo SYSTEM PRIVATE ${evl_INCLUDE_DIRS})
  add_library(rtxipal SHARED "rtos.hpp" "rtos_evl.cpp")
  target_link_libraries(rtxipal PRIVATE 
    ${evl_LINK_LIBRARIES}
    rtxififo
    fmt::fmt
  )
  target_include_directories(rtxipal SYSTEM PRIVATE ${evl_INCLUDE_DIRS})
elseif(DEFINED XENOMAI_FOUND AND RTXI_RT_CORE STREQUAL "xenomai")
  message("Xenomai library found. using Cobalt Alchemy interface")
  #list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${XENOMAI_INCLUDE_DIRS})
  add_library(rtxiplugin SHARED "dlplugin.hpp" "dlplugin.cpp")
  target_link_libraries(rtxiplugin PRIVATE fmt::fmt)
  add_library(rtxififo SHARED "fifo.hpp" "fifo_xenomai.cpp")
  target_include_directories(rtxififo PRIVATE ${XENOMAI_INCLUDE_DIRS})
  target_link_libraries(rtxififo 
    PRIVATE 
      ${XENOMAI_LIBRARIES}
      fmt::fmt
  )
  target_compile_options(rtxififo PRIVATE ${XENOMAI_LDFLAGS})
  target_compile_definitions(rtxififo PRIVATE ${XENOMAI_DEFINITIONS})
  target_link_directories(rtxififo PRIVATE "${XENOMAI_ROOT_DIR}/lib")
  add_library(rtxipal SHARED "rtos.hpp" "rtos_xenomai.cpp")
  target_include_directories(rtxipal PUBLIC ${XENOMAI_INCLUDE_DIRS})
  target_link_libraries(rtxipal
    PRIVATE
      ${XENOMAI_LIBRARIES}
      rtxififo
      fmt::fmt
  )
  target_compile_options(rtxipal PRIVATE ${XENOMAI_LDFLAGS})
  target_compile_definitions(rtxipal PRIVATE ${XENOMAI_DEFINITIONS})
  target_link_directories(rtxipal PUBLIC "${XENOMAI_ROOT_DIR}/lib")
elseif(RTXI_RT_CORE STREQUAL "preempt_rt")
  message("Preempt-RT core Library found.")
  add_library(rtxiplugin SHARED "dlplugin.hpp" "dlplugin.cpp")
  target_link_libraries(rtxiplugin PRIVATE fmt::fmt)
  add_library(rtxififo SHARED "fifo.hpp" "fifo_preempt_rt.cpp")
  target_link_libraries(rtxififo PRIVATE fmt::fmt)
  add_library(rtxipal SHARED "rtos.hpp" "rtos_preempt_rt.cpp")
  target_link_libraries(rtxipal PRIVATE 
      rtxififo
      fmt::fmt
  )
else()
  message("RT core Library not found. Reverting to posix Interface.")
  add_library(rtxiplugin SHARED "dlplugin.hpp" "dlplugin.cpp")
  target_link_libraries(rtxiplugin PRIVATE fmt::fmt)
  add_library(rtxififo SHARED "fifo.hpp" "fifo_posix.cpp")
  target_link_libraries(rtxififo PRIVATE fmt::fmt)
  add_library(rtxipal SHARED "rtos.hpp" "rtos_posix.cpp")
  target_link_libraries(rtxipal PRIVATE 
      rtxififo
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

target_link_libraries(rtxi 
  PRIVATE 
    Threads::Threads
    fmt::fmt
    qwt::qwt
    rtxififo
    rtxiplugin 
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
    rtxiplugin
    rtximath
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
    fmt::fmt
    qwt::qwt
    dl
)

# Create RTXI executable
add_executable(rtxi_exe
    main_window.hpp main_window.cpp
    main.cpp
)

add_executable(rtxi::exe ALIAS rtxi_exe)
set_target_properties(
    rtxi_exe PROPERTIES
    OUTPUT_NAME rtxi
    EXPORT_NAME exe
)

# Link all of the static and dynamic libraries to rtxi
target_link_libraries(rtxi_exe PRIVATE 
    rtxififo
    rtxipal 
    rtxi
    rtximath
    workspace
    performance_measurement_lib
    userprefs_lib
    system_control_lib
    connector_lib
    oscilloscope_lib
    data_recorder_lib
    module_installer_lib
    qwt::qwt
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    fmt::fmt
    HDF5::HDF5
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

target_link_libraries(rtxinidaqdriver PRIVATE nidaqmx rtxipal)
endif()

if(16aio168_api)
message("General Standards Corporation 16aio168 Driver API found")
add_library(rtxi_gsc16aio168_driver MODULE
  gsc_aio168_driver.cpp
)

target_include_directories(rtxi_gsc16aio168_driver 
 	PUBLIC	
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(rtxi_gsc16aio168_driver PRIVATE 16aio168_api rtxipal)
endif()

target_compile_features(rtxi PUBLIC cxx_std_17)
target_compile_features(rtxipal PUBLIC cxx_std_17)
target_compile_features(rtxififo PUBLIC cxx_std_17)
target_compile_features(rtxiplugin PUBLIC cxx_std_17)
target_compile_features(rtxi_exe PRIVATE cxx_std_17)
