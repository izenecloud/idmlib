INCLUDE_DIRECTORIES(		 
  ${CMAKE_CURRENT_SOURCE_DIR}/common
  ${izenelib_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  )

# sequences is important for some linker
# if a dpendes b, a must precede b
SET(libs
  ${imllib_LIBRARIES}
  ${ilplib_LIBRARIES}
  ${wisekma_LIBRARIES}
  ${izenelib_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${wisekma_LIBRARIES}
  xml2
  )

FILE(GLOB kpe_SRC "*.cpp")

#ADD_EXECUTABLE(t_TG ${tg_SRC})
ADD_EXECUTABLE(t_KPE ${kpe_SRC})

  
TARGET_LINK_LIBRARIES(t_KPE
${IDMLIB_SOURCE_DIR}/lib/libidmlib.so
${libs}	
)
   
SET_TARGET_PROPERTIES(
  t_KPE
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${IDMLIB_SOURCE_DIR}/testbin/
)
