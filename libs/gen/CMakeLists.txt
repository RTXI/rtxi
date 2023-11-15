file(GLOB GEN_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB GEN_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(rtxigen SHARED
  ${GEN_SOURCE_FILES}  
  ${GEN_HEADER_FILES}
)

target_include_directories(rtxigen
 	PUBLIC	
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(rtxigen PRIVATE
	GSL::gsl GSL::gslcblas
)
