# sequences is important for some linker
# if a dpendes b, a must precede b
SET(libs
  ${imllib_LIBRARIES}
  ${ilplib_LIBRARIES}
  ${izenelib_LIBRARIES}
  ${Boost_LIBRARIES}
  ${XML2_LIBRARIES}
  ${Glog_LIBRARIES}
  ${LibCURL_LIBRARIES}
  ${ImageMagick_LIBRARIES}
  )

ADD_EXECUTABLE(t_dc t_dc.cpp)

TARGET_LINK_LIBRARIES(t_nec_train
${IDMLIB_SOURCE_DIR}/lib/libidmlib.so
${libs}
)

TARGET_LINK_LIBRARIES(t_nec_test
${IDMLIB_SOURCE_DIR}/lib/libidmlib.so
${libs}
)
