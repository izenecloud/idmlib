INCLUDE_DIRECTORIES(

  ${izenelib_INCLUDE_DIRS}
  ${idmlib_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  )

# sequences is important for some linker
# if a dpendes b, a must precede b
SET(libs
  idmlib
  ${izenelib_LIBRARIES}
  ${Boost_LIBRARIES}
  ${ilplib_LIBRARIES}
  ${imllib_LIBRARIES}

  #external
  ${TokyoCabinet_LIBRARIES}
  ${Glog_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${wisekma_LIBRARIES}
  ${izenejma_LIBRARIES}
  ${xml2_LIBRARIES}
  ${LibCURL_LIBRARIES}
  ${ImageMagick_LIBRARIES}
  bz2
  )

SET(Boost_USE_STATIC_LIBS OFF)
FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS unit_test_framework)

IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
  )
  ADD_EXECUTABLE(t_macd
    Runner.cpp
    t_macd.cc
    )
  TARGET_LINK_LIBRARIES(t_macd ${Boost_LIBRARIES})
  SET_TARGET_PROPERTIES(t_macd PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
    )
  ADD_TEST(MACD "${PROJECT_SOURCE_DIR}/testbin/t_macd")
ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)


IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
  )
  ADD_EXECUTABLE(t_similb
    Runner.cpp
    t_similb.cc
    )
  TARGET_LINK_LIBRARIES(t_similb ${Boost_LIBRARIES})
  SET_TARGET_PROPERTIES(t_similb PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
    )
  ADD_TEST(SIMILB "${PROJECT_SOURCE_DIR}/testbin/t_similb")
ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)

IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
  )
  ADD_EXECUTABLE(t_tdt_storage
    Runner.cpp
    t_storage.cc
    )
  TARGET_LINK_LIBRARIES(t_tdt_storage ${libs})
  SET_TARGET_PROPERTIES(t_tdt_storage PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
    )
  ADD_TEST(TDT_STORAGE "${PROJECT_SOURCE_DIR}/testbin/t_tdt_storage")
ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)

ADD_EXECUTABLE(tdt_scd_tool
tdt_scd_tool.cc
)
TARGET_LINK_LIBRARIES(tdt_scd_tool
  ${libs}
)

SET_TARGET_PROPERTIES(tdt_scd_tool PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)
