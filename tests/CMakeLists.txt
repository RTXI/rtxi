cmake_minimum_required(VERSION 3.20)

add_executable(rtxiTests
    "system_tests.cpp"
    "system_tests.h"
    "rtxiTests.cpp"
)

install(TARGETS rtxi DESTINATION bin)

target_link_libraries(rtxiTests 
    GTest::gtest GTest::gtest_main
    GTest::gmock GTest::gmock_main 
    Qt5::Core Qt5::Widgets Qt5::Gui
    Threads::Threads
)
