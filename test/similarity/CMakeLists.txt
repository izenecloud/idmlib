include_directories(
  ${IDMLIB_SOURCE_DIR}/include
  ${imllib_INCLUDE_DIRS}
)

ADD_EXECUTABLE(doc_sim doc_sim.cpp)

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
  
TARGET_LINK_LIBRARIES(doc_sim
  ${libs}
)

SET_TARGET_PROPERTIES(doc_sim PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)