file(
    RELATIVE_PATH relative_path
    "/${rtxi_INSTALL_CMAKEDIR}"
    "/${CMAKE_INSTALL_BINDIR}/${rtxi_NAME}"
)

get_filename_component(prefix "${CMAKE_INSTALL_PREFIX}" ABSOLUTE)
set(config_dir "${prefix}/${rtxi_INSTALL_CMAKEDIR}")
set(config_file "${config_dir}/rtxiConfig.cmake")

message(STATUS "Installing: ${config_file}")
file(WRITE "${config_file}" "\
get_filename_component(
    _rtxi_executable
    \"\${CMAKE_CURRENT_LIST_DIR}/${relative_path}\"
    ABSOLUTE
)
set(
    RTXI_EXECUTABLE \"\${_rtxi_executable}\"
    CACHE FILEPATH \"Path to the rtxi executable\"
)
")
list(APPEND CMAKE_INSTALL_MANIFEST_FILES "${config_file}")
