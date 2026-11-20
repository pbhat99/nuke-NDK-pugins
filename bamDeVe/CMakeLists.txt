cmake_minimum_required(VERSION 3.15)

cmake_policy(SET CMP0091 NEW)

project(bamDeVe LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "bamDeVe build is configured for Windows.")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Force static runtime library (/MT) to eliminate MSVCP/VCRUNTIME runtime dependencies
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED NUKE_VERSION)
    set(NUKE_VERSION "17.1v2")
endif()

if(NOT DEFINED NDKDIR)
    set(NDKDIR "C:/Program Files/Nuke${NUKE_VERSION}")
endif()

message(STATUS "Configuring bamDeVe for Nuke ${NUKE_VERSION}")
message(STATUS "Nuke directory: ${NDKDIR}")

if(NOT EXISTS "${NDKDIR}")
    message(FATAL_ERROR "Nuke directory not found: ${NDKDIR}")
endif()
if(NOT EXISTS "${NDKDIR}/DDImage.lib")
    message(FATAL_ERROR "DDImage.lib not found in: ${NDKDIR}")
endif()

add_definitions(
    -DWIN32
    -D_WINDOWS
    -DNDEBUG
    -DNOMINMAX
    -D_CRT_SECURE_NO_WARNINGS
    -D_SCL_SECURE_NO_WARNINGS
    -D_USE_MATH_DEFINES
    -DWIN32_LEAN_AND_MEAN
)

if(MSVC)
    add_compile_options(
        /W3
        /O2
        /Ob2
        /EHsc
        /FS
        /nologo
        /wd4514
        /wd4710
        /wd4711
        /wd4668
    )
endif()

# Primary bamDeVe NDK C++ Plugin
add_library(bamDeVe SHARED src/bamDeVe.cpp)
set_property(TARGET bamDeVe PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_include_directories(bamDeVe PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${NDKDIR}/include"
)
target_compile_definitions(bamDeVe PRIVATE
    _USE_MATH_DEFINES
    NOMINMAX
)
target_link_libraries(bamDeVe PRIVATE
    "${NDKDIR}/DDImage.lib"
    opengl32
)
set_target_properties(bamDeVe PROPERTIES
    OUTPUT_NAME "bamDeVe"
    SUFFIX ".dll"
    PREFIX ""
)
