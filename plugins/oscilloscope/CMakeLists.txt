add_library(oscilloscope_lib OBJECT
    scope.hpp
    scope.cpp
    oscilloscope.hpp
    oscilloscope.cpp
)

target_link_libraries(oscilloscope_lib PRIVATE 
    rtxipal
    rtxi
    qwt::qwt
    Qt5::Core
    Qt5::Widgets
    fmt::fmt
)
