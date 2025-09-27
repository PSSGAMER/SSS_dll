# =============================================================================
# Project Definition
# =============================================================================
cmake_minimum_required(VERSION 3.16)
project(SuperSexySteam LANGUAGES CXX)

# Set the C++ standard in a cross-platform way.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enforce a 32-bit build, as required by the project.
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "This project must be compiled as a 32-bit application. "
                        "Please configure with -A Win32 (for Visual Studio) or the "
                        "appropriate 32-bit toolchain flags for your platform.")
endif()

# =============================================================================
# Project Options
# =============================================================================
option(SLS_ENABLE_NATIVE_OPTIMIZATIONS "Enable -march=native for host-specific optimizations (GCC/Clang only)" OFF)

# =============================================================================
# Find Source Files & Define Library
# =============================================================================
# Find all .cpp files in the src/ directory.
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create the shared library (SuperSexySteam.dll on Windows, SuperSexySteam.so on Linux)
add_library(SuperSexySteam SHARED ${SOURCES})

# Set the output directory for the final library to be "bin/".
set_target_properties(SuperSexySteam PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# =============================================================================
# Debugging
# =============================================================================
# Verbose outputs for debugging
# set_target_properties(SuperSexySteam PROPERTIES LINK_FLAGS "/VERBOSE")

# =============================================================================
# Compiler Flags & Options
# =============================================================================
# Add the 'include' directory for headers.
target_include_directories(SuperSexySteam SYSTEM PUBLIC include)

# Statically add yaml-cpp and libmem
target_compile_definitions(SuperSexySteam PRIVATE YAML_CPP_STATIC_DEFINE LM_EXPORT)

# --- Optional Toolchain Accelerators (ccache, mold) ---

# Automatically use ccache for compilation if found.
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    message(STATUS "ccache found, enabling for faster compilation.")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_FOUND})
endif()

# Automatically use mold linker if found (GCC).
if(GCC)
    find_program(MOLD_FOUND mold)
    if(MOLD_FOUND)
        message(STATUS "mold linker found, using for faster linking.")
        target_link_options(SuperSexySteam PRIVATE "-fuse-ld=mold")
    endif()
endif()


# --- General Compiler Flags ---
if(MSVC)
    target_compile_options(SuperSexySteam PRIVATE /W4)
    target_compile_definitions(SuperSexySteam PRIVATE _CRT_SECURE_NO_WARNINGS)
elseif(GCC)
    target_compile_options(SuperSexySteam PRIVATE -Wall -Wextra -Wpedantic -Wno-error=format-security)
endif()

# --- Optimization Flags for Release Builds ---
if(MSVC)
    # Apply MSVC-specific optimization flags ONLY for the Release configuration.
    target_compile_options(SuperSexySteam PRIVATE
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Release>:/GL>
    )
elseif(GCC)
    # Apply GCC/Clang-specific optimization flags ONLY for the Release configuration.
    target_compile_options(SuperSexySteam PRIVATE
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-flto=auto>
    )
endif()

# Note: For MSVC, /GL (Whole Program Optimization) is the equivalent of -flto.
# It requires the linker flag /LTCG, which we add below.
if(MSVC)
    target_link_options(SuperSexySteam PRIVATE $<$<CONFIG:Release>:/LTCG>)
endif()

# --- Add -march=native if the option is enabled ---
if(SLS_ENABLE_NATIVE_OPTIMIZATIONS AND GCC)
    target_compile_options(SuperSexySteam PRIVATE -march=native)
    message(STATUS "Native CPU-specific optimizations enabled.")
endif()


# =============================================================================
# Dependencies
# =============================================================================
# --- System Libraries ---
if(MSVC)
    # On Windows, we use the built-in Cryptography API (CNG).
    target_link_libraries(SuperSexySteam PRIVATE Bcrypt)
    target_compile_definitions(SuperSexySteam PRIVATE USE_WINDOWS_CRYPTO)
elseif(GCC)
    # On other platforms (e.g., Linux), we rely on OpenSSL.
    find_package(OpenSSL REQUIRED)
    target_link_libraries(SuperSexySteam PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

# --- Pre-compiled Local Libraries ---

# --- libmem ---
# Create an IMPORTED target for our static libmem.lib
add_library(Libs::mem STATIC IMPORTED)

# Find the library file in our 'lib' directory and set its location.
find_library(LIBMEM_LIBRARY_PATH NAMES libmem HINTS "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set_target_properties(Libs::mem PROPERTIES IMPORTED_LOCATION "${LIBMEM_LIBRARY_PATH}")

# Any target that links against Libs::mem will now also link against these.
set_target_properties(Libs::mem PROPERTIES
    INTERFACE_LINK_LIBRARIES "user32;psapi;ntdll;shell32"
)
message(STATUS "Found local libmem: ${LIBMEM_LIBRARY_PATH}")

# --- yaml-cpp ---
# Create an IMPORTED target for our static yaml-cpp.lib
add_library(Libs::yaml-cpp STATIC IMPORTED)

# Find the library file in our 'lib' directory and set its location.
find_library(YAMLCPP_LIBRARY_PATH NAMES yaml-cpp HINTS "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set_target_properties(Libs::yaml-cpp PROPERTIES IMPORTED_LOCATION "${YAMLCPP_LIBRARY_PATH}")
message(STATUS "Found local yaml-cpp: ${YAMLCPP_LIBRARY_PATH}")


# --- Final Linking ---
# Link all dependency targets to our main library.
target_link_libraries(SuperSexySteam PRIVATE
    Libs::mem
    Libs::yaml-cpp
)

# =============================================================================
# Installation Target
# =============================================================================
# This replaces the logic of your `setup.sh` script in a cross-platform way.
include(GNUInstallDirs)
install(TARGETS SuperSexySteam
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# =============================================================================
# Custom Target for Zipping
# =============================================================================
find_program(SEVEN_ZIP_EXECUTABLE NAMES 7z 7za)

if(SEVEN_ZIP_EXECUTABLE)
    # Define a helper script to generate archive names with a build-time timestamp.
    # This script will be created in the build directory at configure time.
    set(HELPER_SCRIPT "${CMAKE_BINARY_DIR}/create_zips.cmake")
    file(WRITE ${HELPER_SCRIPT} "
        # This script is called at BUILD time by the 'zips' target.
        string(TIMESTAMP ARCHIVE_DATE \"%Y%m%d%H%M%S\" UTC)

        set(MAIN_ARCHIVE \"${CMAKE_BINARY_DIR}/zips/SuperSexySteam \${ARCHIVE_DATE}.7z\")
        set(CONFIG_ARCHIVE \"${CMAKE_BINARY_DIR}/zips/SuperSexySteam - SLSConfig \${ARCHIVE_DATE}.7z\")

        message(STATUS \"Creating main archive: \${MAIN_ARCHIVE}\")
        execute_process(
            COMMAND \"${SEVEN_ZIP_EXECUTABLE}\" a -mx9 -m9=lzma2
                    \"\${MAIN_ARCHIVE}\"
                    \"$<TARGET_FILE:SuperSexySteam>\"
                    \"${CMAKE_CURRENT_SOURCE_DIR}/setup.sh\"
            RESULT_VARIABLE ret
        )
        if(NOT ret EQUAL 0)
            message(FATAL_ERROR \"7z failed to create main archive.\")
        endif()

        if(EXISTS \"${CMAKE_CURRENT_SOURCE_DIR}/config.yaml.default\")
            message(STATUS \"Creating config archive: \${CONFIG_ARCHIVE}\")
            execute_process(
                COMMAND \"${SEVEN_ZIP_EXECUTABLE}\" a -mx9 -m9=lzma2
                        \"\${CONFIG_ARCHIVE}\"
                        \"${CMAKE_CURRENT_SOURCE_DIR}/config.yaml.default\"
                RESULT_VARIABLE ret
            )
            if(NOT ret EQUAL 0)
                message(FATAL_ERROR \"7z failed to create config archive.\")
            endif()
        endif()
    ")

    add_custom_target(zips
        # First, ensure the output directory exists.
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/zips"
        # Then, run our helper script using cmake -P to get a build-time timestamp.
        COMMAND ${CMAKE_COMMAND} -P "${HELPER_SCRIPT}"
        COMMENT "Creating release archives with current timestamp..."
        DEPENDS SuperSexySteam
        VERBATIM
    )
else()
    message(WARNING "7z executable not found. The 'zips' target will not be available.")
endif()


# =============================================================================
# Final User Messages
# =============================================================================
message(STATUS "Install target created. Run 'cmake --install .' after building.")
message(STATUS "Example: cmake .. -G \"Visual Studio 17 2022\" -A Win32")