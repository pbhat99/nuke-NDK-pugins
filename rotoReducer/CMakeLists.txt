cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(RotoReducer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

if (MSVC)
    add_compile_definitions(HAVE_SNPRINTF NOMINMAX _USE_MATH_DEFINES)
endif()

if(NOT DEFINED NDKDIR)
    set(NDKDIR "C:/Program Files/Nuke17.1v2" CACHE PATH "Nuke install directory")
endif()
if(NOT DEFINED Nuke_ROOT)
    set(Nuke_ROOT "${NDKDIR}" CACHE PATH "Nuke root directory")
endif()

find_package(Nuke REQUIRED)

# 1. Dedicated RotoReducer NDK Node
add_nuke_plugin(RotoReducer RotoReducer.cpp)
target_include_directories(RotoReducer PRIVATE "${NDKDIR}/include/include")
target_link_directories(RotoReducer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp")
target_link_libraries(RotoReducer PRIVATE python311)

# 2. Dedicated RotoExplode NDK Node
add_nuke_plugin(RotoExplode RotoExplode.cpp)
target_include_directories(RotoExplode PRIVATE "${NDKDIR}/include/include")
target_link_directories(RotoExplode PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp")
target_link_libraries(RotoExplode PRIVATE python311)

# 3. Python Native Accelerator Module (roto_native.pyd)
add_library(roto_native MODULE cpp/roto_native.cpp)
set_target_properties(roto_native PROPERTIES SUFFIX ".pyd" PREFIX "")
target_include_directories(roto_native PRIVATE "${NDKDIR}/include/include")
target_link_directories(roto_native PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp")
target_link_libraries(roto_native PRIVATE python311)

