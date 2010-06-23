# sequences is important for some linker
# if a dpendes b, a must precede b


SET(libs
  pthread 
  /home/eric/git/izenelib/lib/libprocmeminfo.so 
   ${ilplib_LIBRARIES}
  /usr/local/lib/libboost_unit_test_framework-gcc41-mt-1_38.so 
  /usr/local/lib/libboost_thread-gcc41-mt-1_38.so 
  /usr/local/lib/libboost_filesystem-gcc41-mt-1_38.so 
  /usr/local/lib/libboost_date_time-gcc41-mt.so 
  /usr/local/lib/libboost_serialization-gcc41-mt.so
  )

ADD_EXECUTABLE(t_dupd t_dupd.cpp)


TARGET_LINK_LIBRARIES(t_dupd
${IDMLIB_SOURCE_DIR}/lib/libidmlib.so
${libs}
)
