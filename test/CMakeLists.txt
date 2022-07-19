cmake_minimum_required(VERSION 3.14)
project(rtxiTests LANGUAGES CXX)

add_executable(rtxiTests
    rt_os_tests.hpp rt_os_tests.cpp
    fifo_tests.hpp fifo_tests.cpp
    event_tests.hpp event_tests.cpp
    io_tests.hpp io_tests.cpp
    system_tests.hpp system_tests.cpp
    rtxiTests.cpp
)

target_include_directories(rtxiTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(rtxiTests PRIVATE
    rtos_lib
    rtos_fifo_lib
    rtxi_lib 
    GTest::gtest GTest::gtest_main
    GTest::gmock GTest::gmock_main 
    Qt6::Core Qt6::Widgets Qt6::Gui
    Threads::Threads
)

add_test(NAME rtxiTests COMMAND rtxiTests)

add_folders(Test)

