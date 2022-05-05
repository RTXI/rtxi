cmake_minimum_required(VERSION 3.14)
project(rtxiTests LANGUAGES CXX)

include(CTest)
add_executable(rtxiTests
    "system_tests.cpp"
    "system_tests.hpp"
    "rtxiTests.cpp"
)

target_include_directories(rtxiTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(rtxiTests rtxi_lib
    GTest::gtest GTest::gtest_main
    GTest::gmock GTest::gmock_main 
    Qt5::Core Qt5::Widgets Qt5::Gui
    Threads::Threads
)

add_test(NAME rtxiTests COMMAND rtxiTests)

add_folders(Test)

