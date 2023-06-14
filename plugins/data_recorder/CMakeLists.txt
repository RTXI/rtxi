cmake_minimum_required(VERSION 3.24)

add_library(data_recorder_lib OBJECT
    data_recorder.h
    data_recorder.cpp
)

target_link_libraries(data_recorder_lib PUBLIC
    rtxi
    HDF5::HDF5
)
