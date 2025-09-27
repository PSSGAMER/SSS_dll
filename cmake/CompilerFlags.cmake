# This module sets up compiler and linker flags for the SuperSexySteam target.

# --- Optional Toolchain Accelerators (ccache, mold) ---
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    message(STATUS "ccache found, enabling for faster compilation.")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_FOUND})
endif()

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
target_compile_options(SuperSexySteam PRIVATE
    $<$<CONFIG:Release>:$<IF:$<CXX_COMPILER_ID:MSVC>,/O2;/GL,>>
    $<$<CONFIG:Release>:$<IF:$<CXX_COMPILER_ID:GCC>>,-O3;-flto=auto,>>
)

target_link_options(SuperSexySteam PRIVATE
    $<$<CONFIG:Release>:$<IF:$<CXX_COMPILER_ID:MSVC>,/LTCG,>>
)

# --- Add -march=native if the option is enabled ---
if(ENABLE_NATIVE_OPTIMIZATIONS AND GCC)
    target_compile_options(SuperSexySteam PRIVATE -march=native)
    message(STATUS "Native CPU-specific optimizations enabled.")
endif()