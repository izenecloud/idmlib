# sequences is important for some linker
# if a dpendes b, a must precede b
SET(libs
  ${ilplib_LIBRARIES}
  ${lalib_LIBRARIES}
  ${imllib_LIBRARIES}
  ${izenelib_LIBRARIES}
  ${wisekma_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  xml2
  )

ADD_EXECUTABLE(t_nec_train t_nec_train.cpp)
ADD_EXECUTABLE(t_nec_test t_nec_test.cpp)

TARGET_LINK_LIBRARIES(t_nec_train
${IDMLIB_SOURCE_DIR}/lib/libidmlib.so
${libs}
)
  
TARGET_LINK_LIBRARIES(t_nec_test
${IDMLIB_SOURCE_DIR}/lib/libidmlib.so
${libs}
)
