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
  ${wisekma_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${izenelib_LIBRARIES}

  #external
  ${Boost_LIBRARIES}
  ${TokyoCabinet_LIBRARIES}
  ${Glog_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${wisekma_LIBRARIES}
  ${xml2_LIBRARIES}
  )

ADD_EXECUTABLE(resys t_rec.cpp)
  
TARGET_LINK_LIBRARIES(resys
  ${libs}
)

