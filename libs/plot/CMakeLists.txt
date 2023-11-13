cmake_minimum_required(VERSION 3.24)

file(GLOB PLOT_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB PLOT_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(rtxiplot SHARED
  ${PLOT_SOURCE_FILES}  
  ${PLOT_HEADER_FILES}
)

target_link_libraries(rtxiplot PRIVATE
	Qt5::Core Qt5::Widgets Qt5::Network 
	Qt5::Gui Qt5::OpenGL Qt5::Svg Qt5::Xml 
	qwt::qwt GSL::gsl GSL::gslcblas 
)
