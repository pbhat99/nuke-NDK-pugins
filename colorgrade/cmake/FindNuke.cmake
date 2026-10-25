# - CMake find module for Nuke
#
# Output variables:
#  NUKE_FOUND
#  NUKE_EXECUTABLE
#  NUKE_INCLUDE_DIRS
#  NUKE_LIBRARY_DIRS
#  NUKE_LIBRARIES
#  NUKE_DDIMAGE_LIBRARY
#  NUKE_VERSION_MAJOR
#  NUKE_VERSION_MINOR
#  NUKE_VERSION_RELEASE

if(NOT Nuke_ROOT)
    if(EXISTS "C:/Program Files/Nuke17.1v2")
        set(Nuke_ROOT "C:/Program Files/Nuke17.1v2")
    endif()
endif()

if(Nuke_ROOT)
    set(_nuke_TEST_PATHS ${Nuke_ROOT})
else()
    set(_nuke_TEST_PATHS "C:/Program Files/Nuke17.1v2" "C:/Program Files/Nuke17.0v1" "C:/Program Files/Nuke16.0v1" "C:/Program Files/Nuke15.1v1")
endif()

find_library(NUKE_DDIMAGE_LIBRARY DDImage
    PATHS ${_nuke_TEST_PATHS}
    DOC "Nuke DDImage library path"
    NO_SYSTEM_ENVIRONMENT_PATH)

if(NUKE_DDIMAGE_LIBRARY)
    get_filename_component(NUKE_LIBRARY_DIRS ${NUKE_DDIMAGE_LIBRARY} PATH)
    find_path(NUKE_INCLUDE_DIRS DDImage/Op.h "${NUKE_LIBRARY_DIRS}/include")

    file(STRINGS "${NUKE_INCLUDE_DIRS}/DDImage/ddImageVersionNumbers.h" _nuke_DDIMAGE_VERSION_H)
    string(REGEX REPLACE ".*#define kDDImageVersionMajorNum ([0-9]+).*" "\\1"
        NUKE_VERSION_MAJOR ${_nuke_DDIMAGE_VERSION_H})
    string(REGEX REPLACE ".*#define kDDImageVersionMinorNum ([0-9]+).*" "\\1"
        NUKE_VERSION_MINOR ${_nuke_DDIMAGE_VERSION_H})
    string(REGEX REPLACE ".*#define kDDImageVersionReleaseNum ([0-9]+).*" "\\1"
        NUKE_VERSION_RELEASE ${_nuke_DDIMAGE_VERSION_H})

    find_program(NUKE_EXECUTABLE
        NAMES
            Nuke
            "Nuke${NUKE_VERSION_MAJOR}.${NUKE_VERSION_MINOR}"
            "Nuke${NUKE_VERSION_MAJOR}.${NUKE_VERSION_MINOR}v${NUKE_VERSION_RELEASE}"
        PATHS ${NUKE_LIBRARY_DIRS}
        NO_SYSTEM_ENVIRONMENT_PATH
        DOC "Nuke executable path")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Nuke DEFAULT_MSG
    NUKE_DDIMAGE_LIBRARY NUKE_INCLUDE_DIRS NUKE_LIBRARY_DIRS)
