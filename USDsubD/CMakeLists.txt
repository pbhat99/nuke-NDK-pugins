cmake_minimum_required(VERSION 3.16)
project(USDSubD LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    add_compile_definitions(HAVE_SNPRINTF NOMINMAX _USE_MATH_DEFINES)
endif()

# ==============================================================================
# Nuke NDK
# ==============================================================================
if(NOT NUKE_INSTALL_DIR)
    if(DEFINED ENV{NUKE_INSTALL_DIR})
        set(NUKE_INSTALL_DIR "$ENV{NUKE_INSTALL_DIR}")
    else()
        set(NUKE_INSTALL_DIR "C:/Program Files/Nuke17.1v2")
    endif()
endif()

set(NUKE_INCLUDE_DIR "${NUKE_INSTALL_DIR}/include" CACHE PATH "Nuke header directory")

if(NOT EXISTS "${NUKE_INCLUDE_DIR}/DDImage/Op.h")
    message(FATAL_ERROR
        "Cannot find DDImage/Op.h in ${NUKE_INCLUDE_DIR}.\n"
        "Check that NUKE_INSTALL_DIR points to a valid Nuke installation.")
endif()

# Collect Nuke library stubs
file(GLOB NUKE_LIBRARIES "${NUKE_INSTALL_DIR}/*.lib")

# ==============================================================================
# Build the shared plugin (.dll)
# ==============================================================================
add_library(${PROJECT_NAME} SHARED USDSubD.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".dll"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${NUKE_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${NUKE_LIBRARIES}
)

# Windows-specific flags
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        FN_SHARED_EXPORT=__declspec\(dllexport\)
        _WIN32_WINNT=0x0601
    )
endif()

# ==============================================================================
# Install
# ==============================================================================
# Place compiled plugin directly in compiled/USDsubD/
set(PLUGIN_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/compiled/USDsubD")

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "${PLUGIN_OUTPUT_DIR}"
    LIBRARY DESTINATION "${PLUGIN_OUTPUT_DIR}"
)
