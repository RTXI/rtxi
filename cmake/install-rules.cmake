include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package rtxi)

install(
    TARGETS rtxi_exe
    RUNTIME COMPONENT rtxi_Runtime
)

install(
    TARGETS rtxi rtxififo rtxipal rtxiplugin rtxidsp rtxigen rtximath rtxiplot
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    EXPORT rtxiLibraryTargets
)

if(nidaqmx)
install(
    TARGETS rtxinidaqdriver 
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    EXPORT rtxiLibraryTargets
)
endif()

install(
    FILES 
        src/debug.hpp src/event.hpp src/io.hpp src/rt.hpp 
        src/daq.hpp src/widgets.hpp src/logger.hpp src/fifo.hpp
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
    DIRECTORY libs/math
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rtxi
    COMPONENT rtxiLibraryTargets
    FILES_MATCHING PATTERN "*.h"
)

install(
    DIRECTORY libs/plot
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rtxi
    COMPONENT rtxiLibraryTargets
    FILES_MATCHING PATTERN "*.h"
)

install(
    FILES
        res/icons/RTXI-icon.png
        res/icons/RTXI-icon.svg
        res/icons/RTXI-widget-icon.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/rtxi
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
    # ----- REQUIRED VARIABLES FOR DEBIAN PACKAGE BUILDING -----
    set(CPACK_VERBATIM_VARIABLES YES)
    set(CPACK_PACKAGE_DESCRIPTION 
        "The Real-Time eXperiment Interface (RTXI) is a \n"
        "collaborative open-source software development project aimed at producing a \n"
        "real-time Linux based software system for hard real-time data acquisition and \n"
        "control applications.")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Ivan F. Valerio <ivan@rtxi.org>")
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")
    set(CPACK_STRIP_FILES YES)
    
    # package name for deb. If set, then instead of some-application-0.9.2-Linux.deb
    # you'll get some-application_0.9.2_amd64.deb (note the underscores too)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

    # that is if you want every group to have its own package,
    # although the same will happen if this is not set (so it defaults to ONE_PER_GROUP)
    # and CPACK_DEB_COMPONENT_INSTALL is set to YES
    set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)#ONE_PER_GROUP)

    # without this you won't be able to pack only specified component
    set(CPACK_DEB_COMPONENT_INSTALL YES)
    set(CPACK_PACKAGE_RELOCATABLE FALSE)
    # create library dependency list. Useful for keeping back packages during updates
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS YES)
    set(
        CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    )
    # -----                     END                      -----

    include(CPack)
endif()
