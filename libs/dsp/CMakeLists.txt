cmake_minimum_required(VERSION 3.24)

file(GLOB DSP_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB DSP_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(rtxidsp SHARED
  ${DSP_SOURCE_FILES}  
  ${DSP_HEADER_FILES}
)

target_include_directories(rtxidsp
 	PUBLIC	
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(rtxidsp PRIVATE
	GSL::gsl GSL::gslcblas
)
