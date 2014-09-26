include_directories(
${IDMLIB_SOURCE_DIR}/include
${izenelib_INCLUDE_DIRS}
${Boost_INCLUDE_DIRS}
${ilplib_INCLUDE_DIRS}
${imllib_INCLUDE_DIRS}
${XML2_INCLUDE_DIRS}
${Glog_INCLUDE_DIRS}
)

# sequences is important for some linker
# if a dpendes b, a must precede b
SET(libs
  idmlib
  ${ilplib_LIBRARIES}
  ${imllib_LIBRARIES}
  ${izenelib_LIBRARIES}

  #external
  ${Boost_LIBRARIES}
  ${TokyoCabinet_LIBRARIES}
  ${Glog_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${wisekma_LIBRARIES}
  ${izenejma_LIBRARIES}
  ${XML2_LIBRARIES}
  ${LibCURL_LIBRARIES}
  ${OpenSSL_LIBRARIES}
  ${ImageMagick_LIBRARIES}
  bz2
  )
SET(Boost_USE_STATIC_LIBS OFF)
FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS unit_test_framework)



ADD_EXECUTABLE(maxent_title_util
	maxent_title_util.cpp
	${PROJECT_SOURCE_DIR}/source/maxent/modelfile.cpp
#	~/codebase/ilplib/source/knlp/normalize.cpp
)
TARGET_LINK_LIBRARIES(maxent_title_util
  ${libs}
  ${Boost_SYSTEM_LIBRARY}
#  /usr/lib/gcc/x86_64-linux-gnu/4.4.7/libgfortran.a
)

SET_TARGET_PROPERTIES(maxent_title_util PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/testbin
)
