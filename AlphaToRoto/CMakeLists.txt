# SPDX-License-Identifier: GPL-3.0-only
#
# AlphaToRoto -- Trace alpha-channel mattes into Roto shapes in Nuke
# Copyright (C) 2026 Peter Mercell

cmake_minimum_required(VERSION 3.18)

cmake_policy(SET CMP0091 NEW)

project(AlphaToRoto CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_SKIP_BUILD_RPATH   TRUE)
set(CMAKE_SKIP_INSTALL_RPATH TRUE)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# ============================================================================
# Windows / MSVC configuration
# ============================================================================
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

    add_definitions(
        -DNOMINMAX
        -D_USE_MATH_DEFINES
        -DWIN32_LEAN_AND_MEAN
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_WARNINGS
    )
endif()

# ============================================================================
# Nuke location
# ============================================================================
if(NOT NUKE_INSTALL_PATH)
    if(DEFINED ENV{NUKE_INSTALL_PATH})
        set(NUKE_INSTALL_PATH "$ENV{NUKE_INSTALL_PATH}")
    elseif(WIN32)
        set(NUKE_INSTALL_PATH "C:/Program Files/Nuke17.1v2")
    else()
        set(NUKE_INSTALL_PATH "/opt/Nuke17.0v1")
    endif()
endif()

set(_nuke_include "${NUKE_INSTALL_PATH}/include")
set(_nuke_lib     "${NUKE_INSTALL_PATH}")

if(WIN32)
    set(_nuke_libname "DDImage.lib")
else()
    set(_nuke_libname "libDDImage.so")
endif()

if(NOT EXISTS "${_nuke_include}")
    message(FATAL_ERROR
        "Nuke include dir not found: ${_nuke_include}\n"
        "Please specify -DNUKE_INSTALL_PATH=<path to Nuke install>")
endif()
if(NOT EXISTS "${_nuke_lib}/${_nuke_libname}")
    message(FATAL_ERROR
        "DDImage library not found: ${_nuke_lib}/${_nuke_libname}")
endif()

# ============================================================================
# potrace (static, embedded)
# ============================================================================
set(POTRACE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/potrace"
    CACHE PATH "Path to potrace source tree (containing src/)")

set(_potrace_src "${POTRACE_ROOT}/src")

set(_potrace_files
    "${_potrace_src}/potracelib.c"
    "${_potrace_src}/curve.c"
    "${_potrace_src}/decompose.c"
    "${_potrace_src}/trace.c"
)

foreach(_f IN LISTS _potrace_files)
    if(NOT EXISTS "${_f}")
        message(FATAL_ERROR "potrace source file missing: ${_f}")
    endif()
endforeach()

add_library(potrace_static STATIC ${_potrace_files})
target_include_directories(potrace_static PUBLIC "${_potrace_src}")

target_compile_definitions(potrace_static PRIVATE
    HAVE_INTTYPES_H=1
    VERSION="1.16"
)

if(MSVC)
    target_compile_options(potrace_static PRIVATE
        /W3
        /wd4244
        /wd4267
        /wd4305
        /wd4101
        /wd4505
        /wd4146
    )
else()
    target_compile_options(potrace_static PRIVATE
        -Wall -Wno-unused-function -Wno-unused-variable)
endif()

# ============================================================================
# AlphaToRoto plugin
# ============================================================================
add_library(AlphaToRoto SHARED src/AlphaToRoto.cpp)

set_target_properties(AlphaToRoto PROPERTIES
    PREFIX ""
    SUFFIX ".dll"
)

target_include_directories(AlphaToRoto PRIVATE
    "${_nuke_include}"
    "${_potrace_src}"
    src
)

target_link_libraries(AlphaToRoto PRIVATE
    potrace_static
    "${_nuke_lib}/${_nuke_libname}"
)

if(MSVC)
    target_compile_options(AlphaToRoto PRIVATE
        /W3 /O2 /Ob2 /EHsc /FS /nologo /MP
        /wd4996
        /wd4244
        /wd4267
        /wd4305
        /wd4251
    )
else()
    target_compile_options(AlphaToRoto PRIVATE
        -fPIC
        -fvisibility=hidden)
endif()

# ============================================================================
# Install
# ============================================================================
set(PLUGIN_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/compiled/AlphaToRoto")

install(TARGETS AlphaToRoto
    RUNTIME DESTINATION "${PLUGIN_OUTPUT_DIR}"
    LIBRARY DESTINATION "${PLUGIN_OUTPUT_DIR}"
)
