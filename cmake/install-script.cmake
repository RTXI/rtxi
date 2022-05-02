file(
    RELATIVE_PATH relative_path
    "/${testcmake_INSTALL_CMAKEDIR}"
    "/${CMAKE_INSTALL_BINDIR}/${testcmake_NAME}"
)

get_filename_component(prefix "${CMAKE_INSTALL_PREFIX}" ABSOLUTE)
set(config_dir "${prefix}/${testcmake_INSTALL_CMAKEDIR}")
set(config_file "${config_dir}/testcmakeConfig.cmake")

message(STATUS "Installing: ${config_file}")
file(WRITE "${config_file}" "\
get_filename_component(
    _testcmake_executable
    \"\${CMAKE_CURRENT_LIST_DIR}/${relative_path}\"
    ABSOLUTE
)
set(
    TESTCMAKE_EXECUTABLE \"\${_testcmake_executable}\"
    CACHE FILEPATH \"Path to the testcmake executable\"
)
")
list(APPEND CMAKE_INSTALL_MANIFEST_FILES "${config_file}")
