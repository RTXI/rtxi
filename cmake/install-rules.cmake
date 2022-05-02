include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package testcmake)

install(
    TARGETS testcmake_exe
    RUNTIME COMPONENT testcmake_Runtime
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    testcmake_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(testcmake_INSTALL_CMAKEDIR)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${testcmake_INSTALL_CMAKEDIR}"
    COMPONENT testcmake_Development
)

# Export variables for the install script to use
install(CODE "
set(testcmake_NAME [[$<TARGET_FILE_NAME:testcmake_exe>]])
set(testcmake_INSTALL_CMAKEDIR [[${testcmake_INSTALL_CMAKEDIR}]])
set(CMAKE_INSTALL_BINDIR [[${CMAKE_INSTALL_BINDIR}]])
" COMPONENT testcmake_Development)

install(
    SCRIPT cmake/install-script.cmake
    COMPONENT testcmake_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
