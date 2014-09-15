include_directories(
  ${IDMLIB_SOURCE_DIR}/include
  ${imllib_INCLUDE_DIRS}
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
  ${izenejma_LIBRARIES}
  ${XML2_LIBRARIES}
  ${LibCURL_LIBRARIES}
  ${ImageMagick_LIBRARIES}
  )

ADD_EXECUTABLE(doc_similarity doc_similarity.cpp)

TARGET_LINK_LIBRARIES(doc_similarity
  ${libs}
)

SET_TARGET_PROPERTIES(doc_similarity PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)
ADD_EXECUTABLE(kp_similarity
kp_similarity.cc
)
TARGET_LINK_LIBRARIES(kp_similarity
  ${libs}
)

SET_TARGET_PROPERTIES(kp_similarity PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)
