cmake_minimum_required(VERSION 3.24)

add_library(userprefs_lib OBJECT
	userprefs.h
	userprefs.cpp
)

target_link_libraries(userprefs_lib PUBLIC
	rtxi_lib
)

