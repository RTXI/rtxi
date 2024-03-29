project(rtxiTests LANGUAGES CXX)

add_library(testing_lib OBJECT
    rt_os_tests.hpp rt_os_tests.cpp
    fifo_tests.hpp fifo_tests.cpp
    event_tests.hpp event_tests.cpp
    io_tests.hpp io_tests.cpp
    system_tests.hpp system_tests.cpp
    module_tests.hpp module_tests.cpp
    plugin_tests.hpp plugin_tests.cpp
)

target_link_libraries(testing_lib PRIVATE 
    rtxipal
    rtxififo 
    rtxi
    workspace
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    GTest::gtest GTest::gtest_main
    GTest::gmock GTest::gmock_main 
    dl
    fmt::fmt
)

add_executable(rtxiTests
        rtxiTests.cpp
)

target_include_directories(rtxiTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(rtxiTests PRIVATE
    testing_lib
    rtxipal
    rtxififo
    rtxi
    rtximath
    workspace
    performance_measurement_lib
    userprefs_lib
    system_control_lib
    connector_lib
    oscilloscope_lib
    data_recorder_lib
    module_installer_lib
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    GTest::gtest GTest::gtest_main
    GTest::gmock GTest::gmock_main 
    fmt::fmt
)

add_library(fakePlugin MODULE fakePlugin.h fakePlugin.cpp)

target_include_directories(fakePlugin PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(fakePlugin PRIVATE 
    testing_lib
    rtxi
    rtxipal
    rtxiplugin
    workspace
    rtximath
    performance_measurement_lib
    userprefs_lib
    system_control_lib
    connector_lib
    oscilloscope_lib
    data_recorder_lib
    module_installer_lib
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    fmt::fmt
    GTest::gtest GTest::gtest_main
    GTest::gmock GTest::gmock_main 
)

add_test(NAME rtxiTests COMMAND rtxiTests)

add_folders(Test)

