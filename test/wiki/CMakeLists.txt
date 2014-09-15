include_directories(
${IDMLIB_SOURCE_DIR}/include
${izenelib_INCLUDE_DIRS}
${Boost_INCLUDE_DIRS}
${ilplib_INCLUDE_DIRS}
${imllib_INCLUDE_DIRS}
${XML2_INCLUDE_DIRS}
${Glog_INCLUDE_DIRS}
)

# sequences is important for some linker
# if a dpendes b, a must precede b
SET(libs
  idmlib
  ${ilplib_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${izenejma_LIBRARIES}
  ${izenelib_LIBRARIES}

  #external
  ${Boost_LIBRARIES}
  ${TokyoCabinet_LIBRARIES}
  ${Glog_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${XML2_LIBRARIES}
  ${LibCURL_LIBRARIES}
  ${ImageMagick_LIBRARIES}
  )
SET(Boost_USE_STATIC_LIBS OFF)

ADD_EXECUTABLE(entity_extractor
entity_extractor_main.cc
)
TARGET_LINK_LIBRARIES(entity_extractor
  ${libs}
)

SET_TARGET_PROPERTIES(entity_extractor PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)

ADD_EXECUTABLE(category_extractor
category_extractor_main.cc
)
TARGET_LINK_LIBRARIES(category_extractor
  ${libs}
)

SET_TARGET_PROPERTIES(category_extractor PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)

ADD_EXECUTABLE(ner_extractor
ner_extractor_main.cc
)
TARGET_LINK_LIBRARIES(ner_extractor
  ${libs}
)

SET_TARGET_PROPERTIES(ner_extractor PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)

ADD_EXECUTABLE(feature_gen
feature_gen_main.cc
)
TARGET_LINK_LIBRARIES(feature_gen
  ${libs}
)

SET_TARGET_PROPERTIES(feature_gen PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)

ADD_EXECUTABLE(feature_libsvm_converter
feature_libsvm_converter.cc
)
TARGET_LINK_LIBRARIES(feature_libsvm_converter
  ${libs}
)

SET_TARGET_PROPERTIES(feature_libsvm_converter PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)

ADD_EXECUTABLE(entity_check
entity_check.cc
)
TARGET_LINK_LIBRARIES(entity_check
  ${libs}
)

SET_TARGET_PROPERTIES(entity_check PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)

ADD_EXECUTABLE(ne_filter
ne_filter.cc
)
TARGET_LINK_LIBRARIES(ne_filter
  ${libs}
)

SET_TARGET_PROPERTIES(ne_filter PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/bin
)
