# This module creates a custom target 'zips' for packaging release archives.

find_program(SEVEN_ZIP_EXECUTABLE NAMES 7z 7za)

if(SEVEN_ZIP_EXECUTABLE)
    # Use configure_file to generate the zipping script from a template.
    # Place the helper script inside CMakeFiles to reduce top-level clutter.
    # CMAKE_FILES_DIRECTORY is an internal CMake variable that points to `${CMAKE_BINARY_DIR}/CMakeFiles`
    set(HELPER_SCRIPT "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/create_zips.cmake")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CreateZipPackage.cmake.in"
        "${HELPER_SCRIPT}"
        @ONLY
    )

    add_custom_target(zips
        # First, ensure the output directory exists.
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/zips"
        # Then, run our generated script to get a build-time timestamp.
        COMMAND ${CMAKE_COMMAND} -P "${HELPER_SCRIPT}"
        COMMENT "Creating release archives with current timestamp..."
        DEPENDS SuperSexySteam
        VERBATIM
    )
else()
    message(WARNING "7z executable not found. The 'zips' target will not be available.")
endif()