# =============================================================================
# Project Definition
# =============================================================================
cmake_minimum_required(VERSION 3.16)
project(SuperSexySteam LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add our custom cmake modules directory to the search path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# =============================================================================
# Project Configuration
# =============================================================================
# Enforce a 32-bit build.
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "This project must be compiled as a 32-bit application. "
                        "Please configure with -A Win32 (for Visual Studio) or the "
                        "appropriate 32-bit toolchain flags for your platform.")
endif()

# --- OUTPUT DIRECTORY ---
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}") 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}") 

# With these settings:
# - SuperSexySteam.dll will be in `${CMAKE_BINARY_DIR}/Release` (or `Debug`)
# - SuperSexySteam.lib (import lib) will be in `${CMAKE_BINARY_DIR}/Release` (or `Debug`)
# - SuperSexySteam.exp will be in `${CMAKE_BINARY_DIR}/Release` (or `Debug`)

# =============================================================================
# Project Options (Toggleable)
# =============================================================================
option(ENABLE_NATIVE_OPTIMIZATIONS "Enable -march=native for host-specific optimizations (GCC/Clang only)" OFF)

# =============================================================================
# Define Library Target
# =============================================================================
# Find all .cpp files in the src/ directory.
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_library(SuperSexySteam SHARED ${SOURCES})

# =============================================================================
# Target Properties & Dependencies
# =============================================================================
# --- Headers ---
target_include_directories(SuperSexySteam SYSTEM PUBLIC include)

# --- Preprocessor Definitions ---
target_compile_definitions(SuperSexySteam PRIVATE
    YAML_CPP_STATIC_DEFINE
    LM_EXPORT
)


# --- Find & Link Dependencies ---
# System crypto libraries
if(MSVC)
    target_link_libraries(SuperSexySteam PRIVATE Bcrypt)
    target_compile_definitions(SuperSexySteam PRIVATE USE_WINDOWS_CRYPTO)
elseif(GCC) 
    find_package(OpenSSL REQUIRED)
    target_link_libraries(SuperSexySteam PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

# Local pre-compiled libraries
find_package(libmem REQUIRED) 
find_package(yaml-cpp REQUIRED)

target_link_libraries(SuperSexySteam PRIVATE
    Libs::mem
    Libs::yaml-cpp
)

# =============================================================================
# Installation & Packaging
# =============================================================================
include(GNUInstallDirs)
install(TARGETS SuperSexySteam
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Add the custom 'zips' target for creating release archives.
include(CreateZipPackage)

# =============================================================================
# Final User Messages
# =============================================================================
message(STATUS "Install target created. Run 'cmake --install .' after building.")
message(STATUS "Example: cmake .. -G \"Visual Studio 17 2022\" -A Win32")