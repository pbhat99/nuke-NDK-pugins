# ==============================================================================
# FindNuke.cmake -- Universal CMake Find Module for Nuke NDK
# ==============================================================================

# 1. Resolve Nuke Root directory
if(NOT Nuke_ROOT)
    if(DEFINED NDKDIR)
        set(Nuke_ROOT "${NDKDIR}")
    elseif(DEFINED ENV{NDKDIR})
        set(Nuke_ROOT "$ENV{NDKDIR}")
    elseif(DEFINED ENV{Nuke_ROOT})
        set(Nuke_ROOT "$ENV{Nuke_ROOT}")
    elseif(DEFINED NUKE_INSTALL_PATH)
        set(Nuke_ROOT "${NUKE_INSTALL_PATH}")
    elseif(DEFINED ENV{NUKE_INSTALL_PATH})
        set(Nuke_ROOT "$ENV{NUKE_INSTALL_PATH}")
    elseif(DEFINED NUKE_INSTALL_DIR)
        set(Nuke_ROOT "${NUKE_INSTALL_DIR}")
    elseif(DEFINED ENV{NUKE_INSTALL_DIR})
        set(Nuke_ROOT "$ENV{NUKE_INSTALL_DIR}")
    endif()
endif()

set(_nuke_SEARCH_PATHS)
if(Nuke_ROOT)
    list(APPEND _nuke_SEARCH_PATHS "${Nuke_ROOT}")
endif()

if(WIN32)
    list(APPEND _nuke_SEARCH_PATHS
        "C:/Program Files/Nuke17.1v2"
        "C:/Program Files/Nuke16.0v9"
        "C:/Program Files/Nuke17.0v1"
        "C:/Program Files/Nuke16.0v1"
    )
elseif(APPLE)
    list(APPEND _nuke_SEARCH_PATHS
        "/Applications/Nuke17.1v2/Nuke17.1v2.app/Contents/MacOS"
        "/Applications/Nuke16.0v9/Nuke16.0v9.app/Contents/MacOS"
    )
else() # Linux
    list(APPEND _nuke_SEARCH_PATHS
        "/usr/local/Nuke17.1v2"
        "/usr/local/Nuke16.0v9"
        "/opt/Nuke17.1v2"
        "/opt/Nuke16.0v9"
    )
endif()

# 2. Find DDImage library
find_library(NUKE_DDIMAGE_LIBRARY
    NAMES DDImage libDDImage
    PATHS ${_nuke_SEARCH_PATHS}
    PATH_SUFFIXES lib bin ""
    DOC "Nuke DDImage library path"
    NO_SYSTEM_ENVIRONMENT_PATH
)

if(NUKE_DDIMAGE_LIBRARY)
    get_filename_component(NUKE_LIBRARY_DIRS "${NUKE_DDIMAGE_LIBRARY}" PATH)

    # 3. Find include directory
    find_path(NUKE_INCLUDE_DIRS
        NAMES DDImage/Op.h
        PATHS ${NUKE_LIBRARY_DIRS} ${_nuke_SEARCH_PATHS}
        PATH_SUFFIXES include
        DOC "Nuke include directory"
        NO_SYSTEM_ENVIRONMENT_PATH
    )

    # 4. Extract version numbers from header if available
    if(EXISTS "${NUKE_INCLUDE_DIRS}/DDImage/ddImageVersionNumbers.h")
        file(STRINGS "${NUKE_INCLUDE_DIRS}/DDImage/ddImageVersionNumbers.h" _nuke_VER_H)
        string(REGEX REPLACE ".*#define kDDImageVersionMajorNum ([0-9]+).*" "\\1"
            NUKE_VERSION_MAJOR "${_nuke_VER_H}")
        string(REGEX REPLACE ".*#define kDDImageVersionMinorNum ([0-9]+).*" "\\1"
            NUKE_VERSION_MINOR "${_nuke_VER_H}")
        string(REGEX REPLACE ".*#define kDDImageVersionReleaseNum ([0-9]+).*" "\\1"
            NUKE_VERSION_RELEASE "${_nuke_VER_H}")
    else()
        set(NUKE_VERSION_MAJOR "17")
        set(NUKE_VERSION_MINOR "1")
        set(NUKE_VERSION_RELEASE "2")
    endif()

    set(NUKE_LIBRARIES "${NUKE_DDIMAGE_LIBRARY}")
endif()

# 5. Helper function for creating Nuke plugins
function(add_nuke_plugin PLUGIN_NAME)
    add_library(${PLUGIN_NAME} MODULE ${ARGN})
    if(NOT TARGET NukePlugins::${PLUGIN_NAME})
        add_library(NukePlugins::${PLUGIN_NAME} ALIAS ${PLUGIN_NAME})
    endif()
    if(NUKE_INCLUDE_DIRS)
        target_include_directories(${PLUGIN_NAME} PRIVATE "${NUKE_INCLUDE_DIRS}")
    endif()
    if(NUKE_DDIMAGE_LIBRARY)
        target_link_libraries(${PLUGIN_NAME} PRIVATE "${NUKE_DDIMAGE_LIBRARY}")
    endif()
    set_target_properties(${PLUGIN_NAME} PROPERTIES PREFIX "")
    if(WIN32)
        target_compile_definitions(${PLUGIN_NAME} PRIVATE NOMINMAX _USE_MATH_DEFINES)
        set_target_properties(${PLUGIN_NAME} PROPERTIES SUFFIX ".dll")
    elseif(APPLE)
        set_target_properties(${PLUGIN_NAME} PROPERTIES SUFFIX ".dylib")
    else()
        set_target_properties(${PLUGIN_NAME} PROPERTIES SUFFIX ".so")
    endif()
endfunction()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Nuke DEFAULT_MSG
    NUKE_DDIMAGE_LIBRARY
    NUKE_INCLUDE_DIRS
)
