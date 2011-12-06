INCLUDE_DIRECTORIES(         

  ${izenelib_INCLUDE_DIRS}
  ${idmlib_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  )

# sequences is important for some linker
# if a dpendes b, a must precede b
SET(libs
  idmlib
  ${ilplib_LIBRARIES}
  ${imllib_LIBRARIES}
  ${wisekma_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${izenejma_LIBRARIES}
  ${iise_LIBRARIES}
  ${izenelib_LIBRARIES}
  ${TokyoCabinet_LIBRARIES}
  ${xml2_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  )

SET(Boost_USE_STATIC_LIBS OFF)
FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS unit_test_framework)

# IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
#   INCLUDE_DIRECTORIES(
#     ${Boost_INCLUDE_DIRS}
#   )
#   ADD_EXECUTABLE(t_dupd
#     Runner.cpp
#     t_dupd.cpp
#     )
#   TARGET_LINK_LIBRARIES(t_dupd ${libs})
#   SET_TARGET_PROPERTIES(t_dupd PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
#     )
#   ADD_TEST(QS "${PROJECT_SOURCE_DIR}/testbin/t_dupd")
# ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)


ADD_EXECUTABLE(grouptable
group_table.cpp
)
TARGET_LINK_LIBRARIES(grouptable ${libs})
SET_TARGET_PROPERTIES(grouptable PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
)

