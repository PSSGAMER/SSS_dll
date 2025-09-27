# Finds the local yaml-cpp library and creates an IMPORTED target Libs::yaml-cpp

find_library(YAMLCPP_LIBRARY_PATH
    NAMES yaml-cpp
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    DOC "Path to the yaml-cpp library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(yaml-cpp
    REQUIRED_VARS YAMLCPP_LIBRARY_PATH
)

if(yaml-cpp_FOUND AND NOT TARGET Libs::yaml-cpp)
    add_library(Libs::yaml-cpp STATIC IMPORTED)
    set_target_properties(Libs::yaml-cpp PROPERTIES
        IMPORTED_LOCATION "${YAMLCPP_LIBRARY_PATH}"
    )
    message(STATUS "Found local yaml-cpp: ${YAMLCPP_LIBRARY_PATH}")
endif()