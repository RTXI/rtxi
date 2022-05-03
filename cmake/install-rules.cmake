include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package rtxi)

install(
    TARGETS rtxi_exe
    RUNTIME COMPONENT rtxi_Runtime
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    rtxi_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(rtxi_INSTALL_CMAKEDIR)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${rtxi_INSTALL_CMAKEDIR}"
    COMPONENT rtxi_Development
)

# Export variables for the install script to use
install(CODE "
set(rtxi_NAME [[$<TARGET_FILE_NAME:rtxi_exe>]])
set(rtxi_INSTALL_CMAKEDIR [[${rtxi_INSTALL_CMAKEDIR}]])
set(CMAKE_INSTALL_BINDIR [[${CMAKE_INSTALL_BINDIR}]])
" COMPONENT rtxi_Development)

install(
    SCRIPT cmake/install-script.cmake
    COMPONENT rtxi_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
