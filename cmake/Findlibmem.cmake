# Finds the local libmem library and creates an IMPORTED target Libs::mem

find_library(LIBMEM_LIBRARY_PATH
    NAMES libmem
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    DOC "Path to the libmem library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libmem
    REQUIRED_VARS LIBMEM_LIBRARY_PATH
)

if(libmem_FOUND AND NOT TARGET Libs::mem)
    add_library(Libs::mem STATIC IMPORTED)
    set_target_properties(Libs::mem PROPERTIES
        IMPORTED_LOCATION "${LIBMEM_LIBRARY_PATH}"
        INTERFACE_LINK_LIBRARIES "user32;psapi;ntdll;shell32"
    )
    message(STATUS "Found local libmem: ${LIBMEM_LIBRARY_PATH}")
endif()