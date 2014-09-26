include_directories(
  ${IDMLIB_SOURCE_DIR}/include
  ${imllib_INCLUDE_DIRS}
)

# sequences is important for some linker
# if a dpendes b, a must precede b
SET(libs
  idmlib
  ${ilplib_LIBRARIES}
  ${izenelib_LIBRARIES}

  #external
  ${Boost_LIBRARIES}
  ${TokyoCabinet_LIBRARIES}
  ${Glog_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${izenejma_LIBRARIES}
  ${XML2_LIBRARIES}
  ${LibCURL_LIBRARIES}
  ${OpenSSL_LIBRARIES}
  ${ImageMagick_LIBRARIES}
  )

ADD_EXECUTABLE(esa_builder esa_builder.cpp)

TARGET_LINK_LIBRARIES(esa_builder
  ${libs}
)

SET_TARGET_PROPERTIES(esa_builder PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)
