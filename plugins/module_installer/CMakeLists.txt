cmake_minimum_required(VERSION 3.24)

add_library(module_installer_lib OBJECT
    rtxi_wizard.h
    rtxi_wizard.cpp
)

target_link_libraries(module_installer_lib PUBLIC
	rtxi Qt5::Network git2 markdown 
)
