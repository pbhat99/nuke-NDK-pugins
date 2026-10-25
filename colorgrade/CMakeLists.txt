cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(ColorGrade)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type." FORCE)
endif()

cmake_policy(SET CMP0074 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(Nuke REQUIRED)

message(STATUS "Configuring ColorGrade for Nuke ${NUKE_VERSION_MAJOR}.${NUKE_VERSION_MINOR}v${NUKE_VERSION_RELEASE}")

if (NUKE_VERSION_MAJOR VERSION_GREATER_EQUAL 15)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${NUKE_INCLUDE_DIRS}
)

add_library(ColorGrade MODULE src/ColorGrade.cpp)
target_link_libraries(ColorGrade PRIVATE ${NUKE_DDIMAGE_LIBRARY})
set_target_properties(ColorGrade PROPERTIES PREFIX "")

if (WIN32)
    target_compile_definitions(ColorGrade PRIVATE NOMINMAX _USE_MATH_DEFINES)
endif()

configure_file(python/init.py init.py COPYONLY)
configure_file(python/menu.py menu.py COPYONLY)

set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/compiled/ColorGrade")

install(TARGETS ColorGrade DESTINATION "${INSTALL_DIR}")
install(FILES 
    "${CMAKE_CURRENT_BINARY_DIR}/init.py"
    "${CMAKE_CURRENT_BINARY_DIR}/menu.py"
    DESTINATION "${INSTALL_DIR}")
