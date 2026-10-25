cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

cmake_policy(SET CMP0091 NEW)

project(OptiXDenoiser LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "This CMakeLists.txt is for Windows builds")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Force static runtime library for portability (no separate VC runtime needed)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

add_definitions(
    -DWIN32
    -D_WINDOWS
    -DNDEBUG
    -DNOMINMAX
    -D_CRT_SECURE_NO_WARNINGS
    -D_SCL_SECURE_NO_WARNINGS
    -D_USE_MATH_DEFINES
    -DWIN32_LEAN_AND_MEAN
    -DCM_GETIDLIST_FILTER_CLASS=0x00000200
    -DCM_GETIDLIST_FILTER_PRESENT=0x00000100
)

if(MSVC)
    add_compile_options(
        /W3
        /O2
        /Ob2
        /EHsc
        /FS
        /nologo
        /MP
        /wd4996
        /wd4244
        /wd4267
        /wd4305
        /wd4251
        /wd4018
    )
endif()

# ============================================================================
# Nuke NDK Configuration
# ============================================================================
if(NOT DEFINED NDKDIR)
    set(NDKDIR "C:/Program Files/Nuke17.1v2")
endif()

set(NUKE_INCLUDE_DIR "${NDKDIR}/include")
if(NOT EXISTS "${NUKE_INCLUDE_DIR}")
    message(FATAL_ERROR "Nuke include directory not found: ${NUKE_INCLUDE_DIR}")
endif()
if(NOT EXISTS "${NDKDIR}/DDImage.lib")
    message(FATAL_ERROR "DDImage.lib not found in: ${NDKDIR}")
endif()

# ============================================================================
# OptiX and CUDA Dependency Paths (Workspace-local)
# ============================================================================
set(OPTIX_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/optix-dev/include")
if(NOT EXISTS "${OPTIX_INCLUDE_DIR}/optix.h")
    message(FATAL_ERROR "OptiX headers not found at: ${OPTIX_INCLUDE_DIR}")
endif()

set(CUDA_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/cuda/nvidia/cuda_runtime")
set(CUDA_INCLUDE_DIR "${CUDA_ROOT_DIR}/include")
set(CUDA_LIB_DIR "${CUDA_ROOT_DIR}/lib/x64")

if(NOT EXISTS "${CUDA_INCLUDE_DIR}/cuda.h")
    message(FATAL_ERROR "CUDA headers not found at: ${CUDA_INCLUDE_DIR}")
endif()

# ============================================================================
# Build Plugin
# ============================================================================
set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OptiXDenoiser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OptiXDenoiser.h"
)

add_library(OptiXDenoiser SHARED ${SOURCES})

set_target_properties(OptiXDenoiser PROPERTIES
    OUTPUT_NAME "OptiXDenoiser"
    PREFIX ""
    SUFFIX ".dll"
)

target_include_directories(OptiXDenoiser PRIVATE
    "${NUKE_INCLUDE_DIR}"
    "${OPTIX_INCLUDE_DIR}"
    "${CUDA_INCLUDE_DIR}"
)

target_link_libraries(OptiXDenoiser PRIVATE
    "${NDKDIR}/DDImage.lib"
    "${CUDA_LIB_DIR}/cudart_static.lib"
    "${CUDA_LIB_DIR}/cuda.lib"
    ws2_32.lib
)

# ============================================================================
# Installation into compiled/OptixDenoiser
# ============================================================================
set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/compiled/OptixDenoiser")

install(TARGETS OptiXDenoiser
    RUNTIME DESTINATION "${OUTPUT_DIR}"
    LIBRARY DESTINATION "${OUTPUT_DIR}"
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/init.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/menu.py"
    DESTINATION "${OUTPUT_DIR}"
)
