cmake_minimum_required(VERSION 3.24)
project(rtxiTests LANGUAGES CXX)

add_executable(rtxiTests
    rt_os_tests.hpp rt_os_tests.cpp
    fifo_tests.hpp fifo_tests.cpp
    event_tests.hpp event_tests.cpp
    io_tests.hpp io_tests.cpp
    system_tests.hpp system_tests.cpp
    module_tests.hpp module_tests.cpp
    rtxiTests.cpp
)

target_include_directories(rtxiTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(rtxiTests PRIVATE
    rtos_lib
    rtos_fifo_lib
    rtxi_lib 
    rtmath_lib
    performance_measurement_lib
    userprefs_lib
    GTest::gtest GTest::gtest_main
    GTest::gmock GTest::gmock_main 
    Boost::headers
    dl
    l
    backtrace
)

target_compile_definitions(rtxiTests PRIVATE BOOST_STACKTRACE_USE_ADDR2LINE)
target_compile_definitions(rtxiTests PRIVATE BOOST_STACKTRACE_USE_BACKTRACE)
add_test(NAME rtxiTests COMMAND rtxiTests)

add_folders(Test)

