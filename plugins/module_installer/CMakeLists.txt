cmake_minimum_required(VERSION 3.24)

add_library(module_installer_lib OBJECT
    rtxi_wizard.h
    rtxi_wizard.cpp
)

target_link_libraries(module_installer_lib PRIVATE 
    rtxi 
    Qt::Core
    Qt::Widgets
    Qt::Gui
    Qt::Network 
    git2 
    markdown 
)
