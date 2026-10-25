cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(NukeShenanigans)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

if (MSVC)
    add_compile_definitions(HAVE_SNPRINTF NOMINMAX _USE_MATH_DEFINES)
endif()

find_package(Nuke REQUIRED)

function(add_shenanigan_plugin PLUGIN_NAME PLUGIN_SRC)
    add_nuke_plugin(${PLUGIN_NAME} ${PLUGIN_SRC} ${ARGN})
    target_compile_definitions(${PLUGIN_NAME} PRIVATE FN_EXAMPLE_PLUGIN)

    # Install plugin directly into compiled/NukeShenanigans/
    install(TARGETS ${PLUGIN_NAME}
        DESTINATION "${CMAKE_SOURCE_DIR}/compiled/NukeShenanigans"
    )
endfunction()

# 4 requested plugins
add_shenanigan_plugin(RemoveUnlimited RemoveUnlimited.cpp)
add_shenanigan_plugin(DeepKeymix DeepKeymix.cpp)
add_shenanigan_plugin(DeepVolumise DeepVolumise.cpp)
add_shenanigan_plugin(ContactSheetAll ContactSheetAll.cpp)

# Additional available plugins in this repo (without external deps)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/DeepCount.cpp")
    add_shenanigan_plugin(DeepCount DeepCount.cpp)
endif()
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/DeepHistogram.cpp")
    add_shenanigan_plugin(DeepHistogram DeepHistogram.cpp)
endif()
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/DeepSampleMerge.cpp")
    add_shenanigan_plugin(DeepSampleMerge DeepSampleMerge.cpp)
endif()
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/MirrorExtend.cpp")
    add_shenanigan_plugin(MirrorExtend MirrorExtend.cpp)
endif()
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/DummyCamera.cpp")
    add_shenanigan_plugin(DummyCamera DummyCamera.cpp)
endif()
