cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(NukeOIDNDenoise)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  if(WIN32)
    # Was getting LNK4098: defaultlib 'MSCVRT' conflicts with use of other libs
    # It took many hours to figure out the -MT flag
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O2 -DNDEBUG -MT")
  else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -DNDEBUG -Wall")
  endif()
endif()

# Windows' math include does not define constants by default.
# Set this definition so it does.
# Also set NOMINMAX so the min and max functions are not overwritten with macros.
IF(MSVC)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
ENDIF()

# Set install RPATH
set(CMAKE_INSTALL_RPATH "$ORIGIN/lib")
# Build with with install RPATH
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
# Add RPATHS outside of the build tree to install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_MACOSX_RPATH TRUE)

# CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# /fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Plugin source code
add_subdirectory(src)

# Set default install prefix to compiled/<plugin_name> in current directory
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/compiled/oidnDenoise" CACHE PATH "Install path prefix" FORCE)
endif()

message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

install(FILES scripts/menu.py DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES scripts/init.py DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES images/icon.png DESTINATION ${CMAKE_INSTALL_PREFIX})

if(WIN32 AND EXISTS "${OIDN_ROOT}/bin")
  file(GLOB OIDN_RUNTIME_DLLS "${OIDN_ROOT}/bin/*.dll")
  install(FILES ${OIDN_RUNTIME_DLLS} DESTINATION ${CMAKE_INSTALL_PREFIX})
elseif(EXISTS "${OIDN_ROOT}/lib")
  file(GLOB OIDN_RUNTIME_LIBS "${OIDN_ROOT}/lib/*.so*" "${OIDN_ROOT}/lib/*.dylib*")
  if(OIDN_RUNTIME_LIBS)
    install(FILES ${OIDN_RUNTIME_LIBS} DESTINATION ${CMAKE_INSTALL_PREFIX})
  endif()
endif()
