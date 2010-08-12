CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

##################################################
# Find extra cmake modules dirs
#
# You can clone all modules from https://ssl.izenesoft.cn/projects/cmake
#
# Clone it and set environment variable EXTRA_CMAKE_MODULES_DIRS point to it.
#
SET(ENV_EXTRA_CMAKE_MODULES_DIRS $ENV{EXTRA_CMAKE_MODULES_DIRS})
IF(ENV_EXTRA_CMAKE_MODULES_DIRS)
  SET(EXTRA_CMAKE_MODULES_DIRS ${EXTRA_CMAKE_MODULES_DIRS} ${ENV_EXTRA_CMAKE_MODULES_DIRS})
ENDIF(ENV_EXTRA_CMAKE_MODULES_DIRS)
IF(NOT EXTRA_CMAKE_MODULES_DIRS)
  MESSAGE(FATAL_ERROR
    "Cannot find EXTRA_CMAKE_MODULES_DIRS.
1. Please clone it from https://ssl.izenesoft.cn/cmake.
2. Then set EXTRA_CMAKE_MODULES_DIRS through environment variable

       export EXTRA_CMAKE_MODULES_DIRS=/path/to/cmake

   You can put it in your .bashrc so it is loaded automatically.

   or cmake options

       cmake  -DEXTRA_CMAKE_MODULES_DIRS=/path/to/cmake (other options)
")
ENDIF(NOT EXTRA_CMAKE_MODULES_DIRS)
SET(CMAKE_MODULE_PATH ${EXTRA_CMAKE_MODULES_DIRS})

# force out-of-source build
IF (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
  MESSAGE(FATAL_ERROR "In source build is not allowed")
ENDIF (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)

##################################################
# Project and some options
#####
PROJECT(IDMLIB)
OPTION(VERBOSE "print detailed configure messages" ON)
OPTION(ENABLE_TEST "build test applications?" ON)

##################################################
# BUILD TYPE & OPTIONS
#
# Put this section just under PROJECT
#####

# Use BUILD_FLAGS_HAS_BEEN_CUSTOMIZED as guard.
#
# The options are only customized in first run. User can edit cache to
# set their own options. If it's required to customized these in cmake
# command line options, please also add
# "-DBUILD_FLAGS_HAS_BEEN_CUSTOMIZED".
IF (CMAKE_COMPILER_IS_GNUCXX)

  INCLUDE(MacroSetBuildTypes)

  MACRO_SET_BUILD_TYPES(
    RelWithDebInfo
    "-O2 -g"
    "Flags used by the compiler during Release with Debug Info builds."

    Release
    "-O3 -DNDEBUG"
    "Flags used by the compiler during release builds"

    MinSizeRel
    "-Os -DNDEBUG"
    "Flags used by the compiler during release minsize builds."

    Debug
    "-g -O0 -fno-reorder-blocks -fno-schedule-insns -fno-inline"
    "Flags used by the compiler during debug builds."

    DebugFull
    "-g3 -fno-inline"
    "Debugfull build type"

    Profile
    "-g -O0 -fno-inline -ftest-coverage -fprofile-arcs"
    "Profile build type"
    )
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

INCLUDE(MacroSetDefaultBuildType)
MACRO_SET_DEFAULT_BUILD_TYPE(RelWithDebInfo)

INCLUDE(SetCompilerWarningAll)
# Write to cache
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING
  "Flags used by the compiler during all build types." FORCE)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING
  "Flags used by the compiler during all build types." FORCE)

# build shared libs by default
OPTION(BUILD_SHARED_LIBS "Build shared libraries?" ON)

##################################################
# Check Dependencies
####
INCLUDE(SetPkgConfigPath)
INCLUDE(ConfigureChecks.cmake)

##################################################
# Install Headers
#####

# trailing / is required, otherwise dectory name is appended to
# install directory.
INSTALL(
    DIRECTORY include/
    DESTINATION include
    COMPONENT "iDMlib_headers"
    )

##################################################
# Config File
#####

##################################################
# Sub Directories
#####

# macros
INCLUDE(MacroCheckShouldBuild)

# all headers
include_directories(
${IDMLIB_SOURCE_DIR}/include
${Boost_INCLUDE_DIRS}
${izenelib_INCLUDE_DIRS}
${ilplib_INCLUDE_DIRS}
${imllib_INCLUDE_DIRS}
${xml2_INCLUDE_DIRS}
${Glog_INCLUDE_DIRS}
)

SET(libs
  ${izenelib_LIBRARIES}
  ${ilplib_LIBRARIES}
  ${xml2_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  )

MESSAGE(STATUS "include path: ${include_directories}")

# User can decide which to build using:
# USER_BUILD_LIST, USER_IGNORE_LIST
SET_PROPERTY(GLOBAL PROPERTY BUILD_COMPONENTS)
SET_PROPERTY(GLOBAL PROPERTY NON_BUILD_COMPONENTS)
MARK_AS_ADVANCED(BUILD_COMPONENTS NON_BUILD_COMPONENTS)

SET(LIBRARY_OUTPUT_PATH "${IDMLIB_SOURCE_DIR}/lib")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/testbin)

ADD_SUBDIRECTORY(source)

GET_PROPERTY(REAL_BUILD_COMPONENTS GLOBAL PROPERTY BUILD_COMPONENTS)
GET_PROPERTY(REAL_NON_BUILD_COMPONENTS GLOBAL PROPERTY NON_BUILD_COMPONENTS)

# Test cases and applications
IF(ENABLE_TEST)
  ADD_SUBDIRECTORY(test)
ENDIF(ENABLE_TEST)

##################################################
# Messages
#####
IF(VERBOSE)

  MESSAGE(STATUS "========================================")
  MESSAGE(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
  MESSAGE(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
  STRING(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
  MESSAGE(STATUS "  corresponding flags: ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}}")
  MESSAGE(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
  MESSAGE(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
  MESSAGE(STATUS "----------------------------------------")

  IF(Boost_FOUND)
    MESSAGE(STATUS "BOOST:")
    MESSAGE(STATUS "  header: ${Boost_INCLUDE_DIR}")
    MESSAGE(STATUS "  version: ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
  ELSE(Boost_FOUND)
    MESSAGE(STATUS "BOOST: not found")
  ENDIF(Boost_FOUND)

  IF(ilplib_FOUND)
    MESSAGE(STATUS "ilplib library:")
    MESSAGE(STATUS "  header: ${ilplib_INCLUDE_DIRS}")
    MESSAGE(STATUS "  lib: ${ilplib_LIBRARIES}")
  ELSE(ilplib_FOUND)
    MESSAGE(STATUS "ilplib library: not found")
  ENDIF(ilplib_FOUND)

  IF(imllib_FOUND)
    MESSAGE(STATUS "imllib:")
    MESSAGE(STATUS "  header: ${imllib_INCLUDE_DIRS}")
    MESSAGE(STATUS "  lib: ${imllib_LIBRARIES}")
  ELSE(imllib_FOUND)
    MESSAGE(STATUS "imllib: not found")
  ENDIF(imllib_FOUND)

  IF(xml2_FOUND)
    MESSAGE(STATUS "LIBXML2:")
    MESSAGE(STATUS " header: ${xml2_INCLUDE_DIRS}")
    MESSAGE(STATUS " lib: ${xml2_LIBRARIES}")
  ELSE(xml2_FOUND)
    MESSAGE(STATUS "LIBXML2: not found")
  ENDIF(xml2_FOUND)

  IF(Threads_FOUND)
    MESSAGE(STATUS "Threads Library: ${CMAKE_THREAD_LIBS_INIT}")
  ELSE(Threads_FOUND)
    MESSAGE(STATUS "Threads Library: none")
  ENDIF(Threads_FOUND)

  IF(Glog_FOUND)
    MESSAGE(STATUS "Google Glog:")
    MESSAGE(STATUS "  header: ${Glog_INCLUDE_DIRS}")
    MESSAGE(STATUS "  libs: ${Glog_LIBRARIES}")
  ELSE(Glog_FOUND)
    MESSAGE(STATUS "Google Glog: not found")
  ENDIF(Glog_FOUND)
  

  MESSAGE(STATUS "----------------------------------------")
  MESSAGE(STATUS "COMPONENTS BUILD:")
  FOREACH(c ${REAL_BUILD_COMPONENTS})
    MESSAGE(STATUS "  ${c}")
  ENDFOREACH(c ${REAL_BUILD_COMPONENTS})
  IF(REAL_NON_BUILD_COMPONENTS)
    MESSAGE(STATUS "COMPONENTS NOT BUILD:")
    FOREACH(c ${REAL_NON_BUILD_COMPONENTS})
      MESSAGE(STATUS "  ${c}")
    ENDFOREACH(c ${REAL_NON_BUILD_COMPONENTS})
  ENDIF(REAL_NON_BUILD_COMPONENTS)
  MESSAGE(STATUS "========================================")

ENDIF(VERBOSE)

##################################################
# CPack
#####
INCLUDE(UseCPack)
CONFIGURE_FILE(
  "idmlib-version.h.cmake"
  "${CMAKE_SOURCE_DIR}/include/idmlib-version.h"
  @ONLY
  )
SET(CPACK_DEBIAN_PACKAGE_DEPENDS
  "libboost-dev, libc6-dev, libxml2-dev, izenelib, ilplib, imllib, icma")
