add_library(performance_measurement_lib OBJECT
    performance_measurement.hpp
    performance_measurement.cpp
)

target_link_libraries(performance_measurement_lib PRIVATE 
    rtxi
    rtximath
    Qt5::Core
    Qt5::Widgets
    fmt::fmt
)
