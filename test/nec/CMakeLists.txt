INCLUDE_DIRECTORIES(

  ${izenelib_INCLUDE_DIRS}
  ${idmlib_INCLUDE_DIRS}
  ${ilplib_INCLUDE_DIRS}
  ${imllib_INCLUDE_DIRS}
  ${wisekma_INCLUDE_DIRS}
  ${izenecma_INCLUDE_DIRS}
  ${izenejma_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${TokyoCabinet_INCLUDE_DIRS}
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
  ${izenejma_LIBRARIES}
  ${iise_LIBRARIES}
  ${izenelib_LIBRARIES}

  #external
  ${Boost_LIBRARIES}
  ${TokyoCabinet_LIBRARIES}
  ${Glog_LIBRARIES}
  ${XML2_LIBRARIES}
  ${LibCURL_LIBRARIES}
  ${ImageMagick_LIBRARIES}
  )

IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
  )
  ADD_EXECUTABLE(t_nec
    Runner.cpp
    t_NEC.cpp
    )
  TARGET_LINK_LIBRARIES(t_nec ${libs})
  SET_TARGET_PROPERTIES(t_nec PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
    )
  ADD_TEST(NEC "${PROJECT_SOURCE_DIR}/testbin/t_nec")
ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)

ADD_EXECUTABLE(nec_accuracy
nec_accuracy.cc
)
TARGET_LINK_LIBRARIES(nec_accuracy
  ${libs}
)

SET_TARGET_PROPERTIES(nec_accuracy PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)

ADD_EXECUTABLE(libsvm_nec_accuracy
libsvm_nec_accuracy.cc
)
TARGET_LINK_LIBRARIES(libsvm_nec_accuracy
  ${libs}
)

SET_TARGET_PROPERTIES(libsvm_nec_accuracy PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)
