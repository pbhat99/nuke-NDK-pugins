cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(LDPK LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "This CMakeLists.txt is configured for Windows builds")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# Nuke NDK Configuration
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

set(LDPK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_definitions(
    -DWIN32
    -D_WINDOWS
    -DNDEBUG
    -DNOMINMAX
    -DFN_OS_WINDOWS
    -D_CRT_SECURE_NO_WARNINGS
    -D_SCL_SECURE_NO_WARNINGS
    -D_USE_MATH_DEFINES
    -D_USRDLL
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

set(BASE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/source/ldpk/nuke_ld_3de4_base.C")

# List of all 18 LDPK lens distortion models: TargetName -> Specific Source File
set(MODELS
    "LD_3DE_Classic_LD_Model:source/ldpk/nuke_ld_3de_classic_ld_model.C"
    "LD_3DE4_Anamorphic_Standard_Degree_4:source/ldpk/nuke_ld_3de4_anamorphic_standard_degree_4.C"
    "LD_3DE4_Radial_Fisheye_Degree_8:source/ldpk/nuke_ld_3de4_radial_fisheye_degree_8.C"
    "LD_3DE4_Radial_Standard_Degree_4:source/ldpk/nuke_ld_3de4_radial_standard_degree_4.C"
    "LD_3DE4_Anamorphic_Rescaled_Degree_4:source/ldpk/nuke_ld_3de4_anamorphic_rescaled_degree_4.C"
    "LD_3DE4_Radial_Fisheye_Equidistant_Degree_8:source/ldpk/nuke_ld_3de4_radial_fisheye_equidistant_degree_8.experimental.C"
    "LD_3DE4_Radial_Fisheye_Equisolid_Degree_8:source/ldpk/nuke_ld_3de4_radial_fisheye_equisolid_degree_8.experimental.C"
    "LD_3DE4_Radial_Fisheye_Orthographic_Degree_8:source/ldpk/nuke_ld_3de4_radial_fisheye_orthographic_degree_8.experimental.C"
    "LD_3DE4_Radial_Fisheye_Stereographic_Degree_8:source/ldpk/nuke_ld_3de4_radial_fisheye_stereographic_degree_8.experimental.C"
    "LD_3DE4_Radial_Homomorphic_Degree_2:source/ldpk/nuke_ld_3de4_radial_homomorphic_degree_2.experimental.C"
    "LD_3DE4_Anamorphic_Rescaled_Degree_6:source/ldpk/nuke_ld_3de4_anamorphic_rescaled_degree_6.C"
    "LD_3DE4_Anamorphic_Standard_Degree_6:source/ldpk/nuke_ld_3de4_anamorphic_standard_degree_6.C"
    "LD_3DE4_Brown_Conrady_Degree_6_2:source/ldpk/nuke_ld_3de4_brown_conrady_degree_6_2.C"
    "LD_3DE4_Anamorphic_Homomorphic_Degree_2:source/ldpk/nuke_ld_3de4_anamorphic_homomorphic_degree_2.experimental.C"
    "LD_3DE4_Radial_Stabilizer_Degree_6:source/ldpk/nuke_ld_3de4_radial_stabilizer_degree_6.C"
    "LD_3DE4_Anamorphic_Stabilizer_Degree_6:source/ldpk/nuke_ld_3de4_anamorphic_stabilizer_degree_6.C"
    "LD_3DE4_Anamorphic_Degree_6:source/ldpk/nuke_ld_3de4_anamorphic_degree_6.C"
    "LD_OpenCV_Radial_Degree_6_Decentered_Degree_2:source/ldpk/nuke_ld_opencv_radial_deg_6_decentered_deg_2.C"
)

set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/compiled/ldpk")

foreach(ITEM ${MODELS})
    string(REPLACE ":" ";" MODEL_PARTS ${ITEM})
    list(GET MODEL_PARTS 0 TARGET_NAME)
    list(GET MODEL_PARTS 1 MODEL_SRC)

    add_library(${TARGET_NAME} SHARED
        "${CMAKE_CURRENT_SOURCE_DIR}/${MODEL_SRC}"
        "${BASE_SOURCE}"
    )

    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME "${TARGET_NAME}"
        PREFIX ""
        SUFFIX ".dll"
    )

    target_include_directories(${TARGET_NAME} PRIVATE
        "${NUKE_INCLUDE_DIR}"
        "${LDPK_INCLUDE_DIR}"
    )

    target_link_libraries(${TARGET_NAME} PRIVATE
        "${NDKDIR}/DDImage.lib"
    )

    install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION "${OUTPUT_DIR}"
        LIBRARY DESTINATION "${OUTPUT_DIR}"
    )
endforeach()

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/init.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/menu.py"
    DESTINATION "${OUTPUT_DIR}"
)
