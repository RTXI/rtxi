cmake_minimum_required(VERSION 3.24)

# We only build driver if the relevant library is found
if(NOT nidaqmx-NOTFOUND)
message("National Instruments Library found")
add_library(nidaq_driver SHARED 
  nidaq_driver.hpp
  nidaq_driver.cpp
)

target_include_directories(nidaq_driver
 	PUBLIC	
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(nidaq_driver PRIVATE nidaqmx rtxi)

endif()
