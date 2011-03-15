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
  #external
  ${Boost_LIBRARIES}

  )
SET(Boost_USE_STATIC_LIBS OFF)

ADD_EXECUTABLE(kp_similarity
kp_similarity.cc
)
TARGET_LINK_LIBRARIES(kp_similarity
  ${libs}
)

SET_TARGET_PROPERTIES(kp_similarity PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)
