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
  ${izenecma_LIBRARIES}
  ${izenejma_LIBRARIES}
  ${izenelib_LIBRARIES}
  ${TokyoCabinet_LIBRARIES}
  ${XML2_LIBRARIES}
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
  ADD_EXECUTABLE(t_cn_correction
    Runner.cpp
    t_cn_correction.cc
    )
  TARGET_LINK_LIBRARIES(t_cn_correction ${libs})
  SET_TARGET_PROPERTIES(t_cn_correction PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
    )
  ADD_TEST(CN_CORRECTION "${PROJECT_SOURCE_DIR}/testbin/t_cn_correction")
ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)


IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
  )
  ADD_EXECUTABLE(t_py_seg
    Runner.cpp
    t_py_seg.cc
    )
  TARGET_LINK_LIBRARIES(t_py_seg ${libs})
  SET_TARGET_PROPERTIES(t_py_seg PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
    )
  ADD_TEST(PY_SEG "${PROJECT_SOURCE_DIR}/testbin/t_py_seg")
ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)


INCLUDE_DIRECTORIES(
${Boost_INCLUDE_DIRS}
)
ADD_EXECUTABLE(cn_trainer
cn_trainer.cc
)
TARGET_LINK_LIBRARIES(cn_trainer ${libs})
SET_TARGET_PROPERTIES(cn_trainer PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)
