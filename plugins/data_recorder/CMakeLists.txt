add_library(data_recorder_lib OBJECT
    data_recorder.hpp
    data_recorder.cpp
)

target_link_libraries(data_recorder_lib PRIVATE 
    rtxi
    hdf5::hdf5_hl
    hdf5::hdf5
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    fmt::fmt
)
