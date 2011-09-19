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
  ${izenelib_LIBRARIES}

  #external
  ${Boost_LIBRARIES}
  ${TokyoCabinet_LIBRARIES}
  ${Glog_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${wisekma_LIBRARIES}
  ${izenejma_LIBRARIES}
  ${xml2_LIBRARIES}
  )
SET(Boost_USE_STATIC_LIBS OFF)
FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS unit_test_framework)

IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
  )
  ADD_EXECUTABLE(t_kpe
    Runner.cpp
    t_KPE.cpp
    )
  TARGET_LINK_LIBRARIES(t_kpe ${libs})
  SET_TARGET_PROPERTIES(t_kpe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/testbin
    )
  ADD_TEST(KPE "${IDMLIB_SOURCE_DIR}/testbin/t_kpe")
ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)

ADD_EXECUTABLE(kpe_tool
kpe_tool.cc
)
TARGET_LINK_LIBRARIES(kpe_tool
  ${libs}
)

SET_TARGET_PROPERTIES(kpe_tool PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)

ADD_EXECUTABLE(kpe_scd_tool
kpe_scd_tool.cc
)
TARGET_LINK_LIBRARIES(kpe_scd_tool
  ${libs}
)

SET_TARGET_PROPERTIES(kpe_scd_tool PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)

ADD_EXECUTABLE(kpe_task_tool
kpe_task_tool.cc
)
TARGET_LINK_LIBRARIES(kpe_task_tool
  ${libs}
)

SET_TARGET_PROPERTIES(kpe_task_tool PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)

ADD_EXECUTABLE(kpe_evaluate_tool
kpe_evaluate_tool.cc
)
TARGET_LINK_LIBRARIES(kpe_evaluate_tool
  ${libs}
)

SET_TARGET_PROPERTIES(kpe_evaluate_tool PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)

ADD_EXECUTABLE(kp_weight
kp_weight.cc
)
TARGET_LINK_LIBRARIES(kp_weight
  ${libs}
)

SET_TARGET_PROPERTIES(kp_weight PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)


ADD_EXECUTABLE(bce_trainer_cn
bce_trainer_cn.cc
)
TARGET_LINK_LIBRARIES(bce_trainer_cn
  ${libs}
)

SET_TARGET_PROPERTIES(bce_trainer_cn PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)
    
    

