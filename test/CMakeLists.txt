cmake_minimum_required(VERSION 3.20)
project(rtxiTests LANGUAGES CXX)

add_executable(rtxiTests
    "system_tests.cpp"
    "system_tests.h"
    "rtxiTests.cpp"
)

target_link_libraries(rtxiTests PRIVATE
    GTest::gtest GTest::gtest_main
    GTest::gmock GTest::gmock_main 
    Qt5::Core Qt5::Widgets Qt5::Gui
    Threads::Threads
)

add_test(NAME rtxiTests COMMAND rtxiTests)

add_folders(Test)
