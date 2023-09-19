cmake_minimum_required(VERSION 3.24)

add_library(data_recorder_lib OBJECT
    data_recorder.h
    data_recorder.cpp
)

target_link_libraries(data_recorder_lib PRIVATE 
    rtxi
    hdf5::hdf5_hl
    hdf5::hdf5
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
)
