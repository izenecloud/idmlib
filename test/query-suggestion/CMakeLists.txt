# sequences is important for some linker
# if a dpendes b, a must precede b
SET(libs
  ${imllib_LIBRARIES}
  ${wiselib_LIBRARIES}
  ${izenelib_LIBRARIES}
  ${Boost_LIBRARIES}
  xml2
  ${Glog_LIBRARIES}
  )

MESSAGE(STATUS "xml2: ${xml2_LIBRARIES}")

ADD_EXECUTABLE(t_qs t_qs.cpp)

TARGET_LINK_LIBRARIES(t_qs
${IDMLIB_SOURCE_DIR}/lib/libidmlib.so
${libs}
)
  
