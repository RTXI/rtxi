add_library(module_installer_lib OBJECT
    rtxi_wizard.hpp
    rtxi_wizard.cpp
)

target_link_libraries(module_installer_lib PRIVATE 
    rtxi 
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Network 
    fmt::fmt
)
