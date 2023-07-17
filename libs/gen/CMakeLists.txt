cmake_minimum_required(VERSION 3.24)

file(GLOB GEN_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB GEN_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(rtgen SHARED
  ${GEN_SOURCE_FILES}  
  ${GEN_HEADER_FILES}
)

target_include_directories(rtgen
 	PUBLIC	
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(rtgen PRIVATE
	GSL::gsl GSL::gslcblas
)
