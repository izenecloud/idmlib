INCLUDE_DIRECTORIES(
  ${izenelib_INCLUDE_DIRS}
  ${idmlib_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  )

# sequences is important for some linker
# if a dpendes b, a must precede b
SET(libs
  idmlib
  ${ilplib_LIBRARIES}
  ${imllib_LIBRARIES}
  ${wisekma_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${izenejma_LIBRARIES}
  ${izenelib_LIBRARIES}
  ${TokyoCabinet_LIBRARIES}
  ${xml2_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  ${LibCURL_LIBRARIES}
  ${ImageMagick_LIBRARIES}
  )

SET(Boost_USE_STATIC_LIBS OFF)
FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS unit_test_framework)

IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
  )

ADD_EXECUTABLE(ise_builder ise_builder.cpp)

TARGET_LINK_LIBRARIES(ise_builder
  ${libs}
)

SET_TARGET_PROPERTIES(ise_builder PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)

ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
