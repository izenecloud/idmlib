include_directories(
${IDMLIB_SOURCE_DIR}/include
${izenelib_INCLUDE_DIRS}
${Boost_INCLUDE_DIRS}
${ilplib_INCLUDE_DIRS}
${imllib_INCLUDE_DIRS}
${xml2_INCLUDE_DIRS}
${Glog_INCLUDE_DIRS}
)

# sequences is important for some linker
# if a dpendes b, a must precede b
SET(libs
  idmlib
  ${ilplib_LIBRARIES}
  ${imllib_LIBRARIES}
  ${wisekma_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${iise_LIBRARIES}
  ${izenelib_LIBRARIES}

  #external
  ${Boost_LIBRARIES}
  ${TokyoCabinet_LIBRARIES}
  ${Glog_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${wisekma_LIBRARIES}
  ${xml2_LIBRARIES}
  )
SET(Boost_USE_STATIC_LIBS OFF)
FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS unit_test_framework)

IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
  )
  ADD_EXECUTABLE(t_analyzer
    Runner.cpp
    t_mining_analyzer.cc
    )
  TARGET_LINK_LIBRARIES(t_analyzer ${libs})
  SET_TARGET_PROPERTIES(t_analyzer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/testbin
    )
  ADD_TEST(ANALYZER "${IDMLIB_SOURCE_DIR}/testbin/t_analyzer")
ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)

IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
  )
  ADD_EXECUTABLE(t_container_switch
    Runner.cpp
    t_container_switch.cc
    )
  TARGET_LINK_LIBRARIES(t_container_switch ${libs})
  SET_TARGET_PROPERTIES(t_container_switch PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/testbin
    )
  ADD_TEST(ANALYZER "${IDMLIB_SOURCE_DIR}/testbin/t_container_switch")
ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)

ADD_EXECUTABLE(resource_generate
resource_generate.cc
)
TARGET_LINK_LIBRARIES(resource_generate
  ${libs}
)

SET_TARGET_PROPERTIES(resource_generate PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)

ADD_EXECUTABLE(text_converter
text_converter.cc
)
TARGET_LINK_LIBRARIES(text_converter
  ${libs}
)

SET_TARGET_PROPERTIES(text_converter PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)
