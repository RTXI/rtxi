include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package rtxi)

install(
    TARGETS rtxi_exe
    RUNTIME COMPONENT rtxi_Runtime
)

install(
    TARGETS rtxi xfifo rtos dlplugin rtdsp rtgen
    EXPORT rtxiLibraryTargets
)

install(
    FILES 
        src/debug.hpp src/event.hpp src/io.hpp src/rt.hpp 
        src/daq.hpp src/module.hpp src/logger.hpp src/fifo.hpp
        src/rtos.hpp src/dlplugin.hpp  
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rtxi
    COMPONENT rtxiLibraryTargets
)

install(
    DIRECTORY libs/dsp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rtxi
    COMPONENT rtxiLibraryTargets
    FILES_MATCHING PATTERN "*.h"
)

install(
    DIRECTORY libs/gen
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rtxi
    COMPONENT rtxiLibraryTargets
    FILES_MATCHING PATTERN "*.h"
)

install(
    EXPORT rtxiLibraryTargets
    NAMESPACE rtxi::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rtxi
)

#export(EXPORT rtxiLibraryTargets
#    FILE "${CMAKE_CURRENT_BINARY_DIRECTORY}/cmake/rtxiLibraryTargets.cmake"
#    NAMESPACE rtxi::
#)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/rtxiConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rtxi
)

install(
    FILES 
        "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
        "${PROJECT_BINARY_DIR}/${package}Config.cmake"
    DESTINATION 
        ${CMAKE_INSTALL_LIBDIR}/cmake/rtxi
    COMPONENT 
        rtxi_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
