INCLUDE_DIRECTORIES(
  ${izenelib_INCLUDE_DIRS}
  ${izenelib_INCLUDE_DIRS}/3rdparty/am
  ${idmlib_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  )

# sequences is important for some linker
# if a dpendes b, a must precede b
SET(libs
  idmlib
  ${ilplib_LIBRARIES}
  ${izenecma_LIBRARIES}
  ${izenejma_LIBRARIES}
  ${izenelib_LIBRARIES}
  ${TokyoCabinet_LIBRARIES}
  ${XML2_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  ${LibCURL_LIBRARIES}
  ${OpenSSL_LIBRARIES}
  ${ImageMagick_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  gomp
  )

SET(Boost_USE_STATIC_LIBS OFF)
FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS unit_test_framework)

IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
  )

  ADD_EXECUTABLE(t_ItemCoVisitation
    Runner.cpp
    t_ItemCoVisitation.cpp
    ItemCFTest.cpp
    )
  TARGET_LINK_LIBRARIES(t_ItemCoVisitation ${libs})
  SET_TARGET_PROPERTIES(t_ItemCoVisitation PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
    )
  ADD_TEST(resys "${PROJECT_SOURCE_DIR}/testbin/t_ItemCoVisitation")

  ADD_EXECUTABLE(t_IncrementalItemCF
    Runner.cpp
    t_IncrementalItemCF.cpp
    ItemCFTest.cpp
    )
  TARGET_LINK_LIBRARIES(t_IncrementalItemCF ${libs})
  SET_TARGET_PROPERTIES(t_IncrementalItemCF PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
    )
  ADD_TEST(resys "${PROJECT_SOURCE_DIR}/testbin/t_IncrementalItemCF")

ADD_DEFINITIONS("-fopenmp -Wunused-variable")

  ADD_EXECUTABLE(t_FactorizationMachine
    Runner.cpp
    t_FactorizationMachine.cpp
    )
  TARGET_LINK_LIBRARIES(t_FactorizationMachine ${libs})
  SET_TARGET_PROPERTIES(t_FactorizationMachine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
    )
  ADD_TEST(resys "${PROJECT_SOURCE_DIR}/testbin/t_FactorizationMachine")

ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
