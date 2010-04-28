CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

##################################################
# !!! IMPORTANT
# Must at the beginning of the file and before PROJECT.
# 
# Set default install prefix for user, use following order to select
# the value:
#
# 1) Specified in cmake -DCMAKE_INSTALL_PREFIX=/your/prefix
# 2) Evironement variable MY_INSTALL_PREFIX
# 3) Try $HOME/myroot
#
# User also can change CMAKE_INSTALL_PREFIX in cache
#####
IF(NOT CMAKE_INSTALL_PREFIX)
  IF(IS_DIRECTORY "$ENV{MY_INSTALL_PREFIX}")
    SET(CMAKE_INSTALL_PREFIX "$ENV{MY_INSTALL_PREFIX}" CACHE STRING "Install path prefix, prepended onto install directories.")
  ELSEIF(IS_DIRECTORY "$ENV{HOME}/myroot")
    SET(CMAKE_INSTALL_PREFIX "$ENV{HOME}/myroot" CACHE STRING "Install path prefix, prepended onto install directories.")
  ENDIF(IS_DIRECTORY "$ENV{MY_INSTALL_PREFIX}")
ENDIF(NOT CMAKE_INSTALL_PREFIX)

##################################################
# Find extra cmake modules dirs
#
# You can clone all modules from izenesoft.com:/git/cmake.git.
# 
# Copy it to your install dir or change the variable
# EXTRA_CMAKE_MODULES_DIRS.
#
IF(IS_DIRECTORY "${CMAKE_INSTALL_PREFIX}/cmake")
  SET(EXTRA_CMAKE_MODULES_DIRS ${EXTRA_CMAKE_MODULES_DIRS} "${CMAKE_INSTALL_PREFIX}/cmake")
ENDIF(IS_DIRECTORY "${CMAKE_INSTALL_PREFIX}/cmake")
SET(ENV_EXTRA_CMAKE_MODULES_DIRS $ENV{EXTRA_CMAKE_MODULES_DIRS})
IF(ENV_EXTRA_CMAKE_MODULES_DIRS)
  SET(EXTRA_CMAKE_MODULES_DIRS ${EXTRA_CMAKE_MODULES_DIRS} ${ENV_EXTRA_CMAKE_MODULES_DIRS})
ENDIF(ENV_EXTRA_CMAKE_MODULES_DIRS)
IF(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake")
  SET(EXTRA_CMAKE_MODULES_DIRS ${EXTRA_CMAKE_MODULES_DIRS} "${CMAKE_SOURCE_DIR}/cmake")
ENDIF(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake")
IF(NOT EXTRA_CMAKE_MODULES_DIRS)
  MESSAGE(FATAL_ERROR
    "Cannot find EXTRA_CMAKE_MODULES_DIRS.
Please clone it from https://git.izenesoft.cn/cmake/cmake. Then set EXTRA_CMAKE_MODULES_DIRS (-DEXTRA_CMAKE_MODULES_DIRS=/path/to/cmake) to path of the cloned repository. You also can copy the cloned cmakedirectory to ${CMAKE_INSTALL_PREFIX} or ${CMAKE_SOURCE_DIR}")
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
IF(ENABLE_TEST)
  ENABLE_TESTING()
ENDIF(ENABLE_TEST)

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

# build shared or static library
# default is shared
OPTION(BUILD_SHARED_LIBS "Build shared libraries?" ON)
IF(NOT CMAKE_BUILD_LIBRARY_TYPE)
  SET(CMAKE_BUILD_LIBRARY_TYPE $ENV{CMAKE_BUILD_LIBRARY_TYPE})
ENDIF(NOT CMAKE_BUILD_LIBRARY_TYPE)
STRING(TOLOWER "${CMAKE_BUILD_LIBRARY_TYPE}" _lower_library_type)
IF(_lower_library_type STREQUAL "static")
  SET(BUILD_SHARED_LIBS OFF)
ELSEIF(_lower_library_type STREQUAL "shared")
  SET(BUILD_SHARED_LIBS ON)
ENDIF(_lower_library_type STREQUAL "static")

# use static in profile mode and add linking flag -lgcov
STRING(TOLOWER "${CMAKE_BUILD_TYPE}" _lower_build_type)
IF(_lower_build_type STREQUAL "profile")
  SET(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries?" FORCE)

  IF(NOT CMAKE_EXE_LINKER_FLAGS MATCHES "-lgcov$")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
  ENDIF(NOT CMAKE_EXE_LINKER_FLAGS MATCHES "-lgcov$")

  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

  # remove default link flags, some of them may add -rdynamic
  SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
  SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
  SET(CMAKE_MODULE_LIBRARY_LINK_C_FLAGS)
  SET(CMAKE_MODULE_LIBRARY_LINK_CXX_FLAGS)
  SET(CMAKE_EXE_LIBRARY_LINK_C_FLAGS)
  SET(CMAKE_EXE_LIBRARY_LINK_CXX_FLAGS)

  SET(Boost_USE_STATIC_LIBS ON)

  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTEST_COVERAGE")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTEST_COVERAGE")
ENDIF(_lower_build_type STREQUAL "profile")

IF(_lower_build_type STREQUAL "profile")
  # No rpath in profile mode
  SET(CMAKE_SKIP_BUILD_RPATH  TRUE)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ELSE(_lower_build_type STREQUAL "profile")
  # Use rpath in non-profile mode
  SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDIF(_lower_build_type STREQUAL "profile")

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
INCLUDE_DIRECTORIES("
${IDMLIB_SOURCE_DIR}/include"
${izenelib_INCLUDE_DIRS}
${wiselib_INCLUDE_DIRS}
${Boost_INCLUDE_DIRS}
${lalib_INCLUDE_DIRS}
${imllib_INCLUDE_DIRS}
${xml2_INCLUDE_DIRS}
)

SET(libs
  ${wiselib_LIBRARIES}
  ${izenelib_LIBRARIES}
  ${lalib_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  )
  
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

  IF(lalib_FOUND)
    MESSAGE(STATUS "la library:")
    MESSAGE(STATUS "  header: ${lalib_INCLUDE_DIRS}")
    MESSAGE(STATUS "  lib: ${lalib_LIBRARIES}")
  ELSE(lalib_FOUND)
    MESSAGE(STATUS "la library: not found")
  ENDIF(lalib_FOUND)
  
  IF(imllib_FOUND)
    MESSAGE(STATUS "imllib:")
    MESSAGE(STATUS "  header: ${imllib_INCLUDE_DIRS}")
    MESSAGE(STATUS "  lib: ${imllib_LIBRARIES}")
  ELSE(imllib_FOUND)
    MESSAGE(STATUS "imllib: not found")
  ENDIF(imllib_FOUND)


  IF(BZIP2_FOUND)
    MESSAGE(STATUS "BZIP2:")
    MESSAGE(STATUS "  header: ${BZIP2_INCLUDE_DIR}")
    MESSAGE(STATUS "  libs: ${BZIP2_LIBRARIES}")
  ELSE(BZIP2_FOUND)
    MESSAGE(STATUS "BZIP2: not found")
  ENDIF(BZIP2_FOUND)
  
  IF(xml2_FOUND)
    MESSAGE(STATUS "xml2:")
    MESSAGE(STATUS "  header: ${xml2_INCLUDE_DIR}")
    MESSAGE(STATUS "  libs: ${xml2_LIBRARIES}")
  ELSE(xml2_FOUND)
    MESSAGE(STATUS "xml2: not found")
  ENDIF(xml2_FOUND)

  IF(ZLIB_FOUND)
    MESSAGE(STATUS "ZLIB:")
    MESSAGE(STATUS "  header: ${ZLIB_INCLUDE_DIR}")
    MESSAGE(STATUS "  libs: ${ZLIB_LIBRARIES}")
  ELSE(ZLIB_FOUND)
    MESSAGE(STATUS "ZLIB: not found")
  ENDIF(ZLIB_FOUND)

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
SET(CPACK_GENERATOR "TBZ2")
SET(CPACK_SOURCE_GENERATOR "TBZ2")
SET(CPACK_SOURCE_IGNORE_FILES
  "/\\\\.git/;\\\\.gitignore$;\\\\.swp$;\\\\.#;/#;~$"
  )

INCLUDE(CPack)
