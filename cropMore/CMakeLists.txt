cmake_minimum_required(VERSION 3.15)

if(WIN32)
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(cropMore LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED NUKE_VERSION)
    set(NUKE_VERSION "17.1v2")
endif()

if(NOT DEFINED NDKDIR)
    if(WIN32)
        set(NDKDIR "C:/Program Files/Nuke${NUKE_VERSION}")
    elseif(APPLE)
        set(NDKDIR "/Applications/Nuke${NUKE_VERSION}/Nuke${NUKE_VERSION}.app/Contents/MacOS")
    else()
        set(NDKDIR "/opt/Nuke${NUKE_VERSION}")
    endif()
endif()

message(STATUS "Configuring CropMore for Nuke ${NUKE_VERSION}")
message(STATUS "Nuke directory: ${NDKDIR}")

add_definitions(-DNDEBUG -DNOMINMAX)

if(WIN32)
    add_definitions(
        -DWIN32
        -D_WINDOWS
        -D_CRT_SECURE_NO_WARNINGS
        -D_SCL_SECURE_NO_WARNINGS
        -D_USE_MATH_DEFINES
        -DWIN32_LEAN_AND_MEAN
    )
    if(MSVC)
        add_compile_options(
            /W3 /O2 /Ob2 /EHsc /FS /nologo
            /wd4514 /wd4710 /wd4711 /wd4668
        )
    endif()
    set(NUKE_LIB "${NDKDIR}/DDImage.lib")
    set(PLATFORM_LIBS opengl32)
    set(PLUGIN_EXT ".dll")
elseif(APPLE)
    add_compile_options(-fPIC -Wall -O3)
    set(CMAKE_SKIP_BUILD_RPATH TRUE)
    set(CMAKE_SKIP_INSTALL_RPATH TRUE)
    set(NUKE_LIB "${NDKDIR}/libDDImage.dylib")
    find_library(OPENGL_LIB OpenGL)
    set(PLATFORM_LIBS ${OPENGL_LIB})
    set(PLUGIN_EXT ".dylib")
else() # Linux
    add_compile_options(-fPIC -Wall -O3)
    set(CMAKE_SKIP_BUILD_RPATH TRUE)
    set(CMAKE_SKIP_INSTALL_RPATH TRUE)
    set(NUKE_LIB "${NDKDIR}/libDDImage.so")
    set(PLATFORM_LIBS GL)
    set(PLUGIN_EXT ".so")
endif()

add_library(CropMore SHARED src/CropMore.cpp)

if(WIN32)
    set_property(TARGET CropMore PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

target_include_directories(CropMore PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${NDKDIR}/include"
)
target_compile_definitions(CropMore PRIVATE
    _USE_MATH_DEFINES
    NOMINMAX
)
target_link_libraries(CropMore PRIVATE
    "${NUKE_LIB}"
    ${PLATFORM_LIBS}
)
set_target_properties(CropMore PROPERTIES
    OUTPUT_NAME "CropMore"
    SUFFIX "${PLUGIN_EXT}"
    PREFIX ""
)
