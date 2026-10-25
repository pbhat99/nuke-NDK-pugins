cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(RotoSimplifier)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

if (MSVC)
    add_compile_definitions(HAVE_SNPRINTF NOMINMAX _USE_MATH_DEFINES)
endif()

find_package(Nuke REQUIRED)

add_nuke_plugin(RotoSimplifier RotoSimplifier.cpp)

# Include Nuke's Python headers and link against python311
target_include_directories(RotoSimplifier PRIVATE "C:/Program Files/Nuke17.1v2/include/include")
target_link_directories(RotoSimplifier PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp")
target_link_libraries(RotoSimplifier PRIVATE python311)

# Install directly as single DLL into compiled/RotoSimplifier
install(TARGETS RotoSimplifier
    DESTINATION "${CMAKE_SOURCE_DIR}/compiled/RotoSimplifier"
)
