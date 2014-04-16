INCLUDE_DIRECTORIES(
  ${izenelib_INCLUDE_DIRS}
  ${ilplib_INCLUDE_DIRS}
  ${idmlib_INCLUDE_DIRS}
  ${wisekma_INCLUDE_DIRS}
  ${izenecma_INCLUDE_DIRS}
  ${izenejma_INCLUDE_DIRS}
  ${addrlib_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${TokyoCabinet_INCLUDE_DIRS}
  ${Glog_INCLUDE_DIRS}
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
  ${izenelib_LIBRARIES}

  #external
  ${Boost_LIBRARIES}
  ${TokyoCabinet_LIBRARIES}
  ${Glog_LIBRARIES}
  ${XML2_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  ${MYSQL_LIBRARIES}
  ${LibCURL_LIBRARIES}
  ${ImageMagick_LIBRARIES}
  )
SET(Boost_USE_STATIC_LIBS OFF)
FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS unit_test_framework)

IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
  )


ADD_EXECUTABLE(t_commentdb
Runner.cpp
t_commentdb.cc
)
TARGET_LINK_LIBRARIES(t_commentdb  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  ${libs})
SET_TARGET_PROPERTIES(t_commentdb PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
)



ADD_EXECUTABLE(t_offerdb
Runner.cpp
t_offerdb.cc
)
TARGET_LINK_LIBRARIES(t_offerdb  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  ${libs})
SET_TARGET_PROPERTIES(t_offerdb PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
)

ADD_EXECUTABLE(t_offer_db_recorder
Runner.cpp
t_offer_db_recorder.cc
)
TARGET_LINK_LIBRARIES(t_offer_db_recorder  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  ${libs})
SET_TARGET_PROPERTIES(t_offer_db_recorder PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
)

ADD_EXECUTABLE(t_b5mo_processor
Runner.cpp
t_b5mo_processor.cc
)
TARGET_LINK_LIBRARIES(t_b5mo_processor  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  ${libs})
SET_TARGET_PROPERTIES(t_b5mo_processor PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
)

ADD_EXECUTABLE(t_product_db
Runner.cpp
t_product_db.cc
)
TARGET_LINK_LIBRARIES(t_product_db  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  ${libs})
SET_TARGET_PROPERTIES(t_product_db PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/testbin
)


#ADD_EXECUTABLE(product-matcher-test
#b5m_product_matcher.cc
#)
#TARGET_LINK_LIBRARIES(product-matcher-test ${libs})
#SET_TARGET_PROPERTIES(product-matcher-test PROPERTIES
#RUNTIME_OUTPUT_DIRECTORY ${SF1RENGINE_ROOT}/testbin
#)
#ADD_EXECUTABLE(b5m_psm
#b5m_psm.cc
#)
#TARGET_LINK_LIBRARIES(b5m_psm ${libs})
#SET_TARGET_PROPERTIES(b5m_psm PROPERTIES
#RUNTIME_OUTPUT_DIRECTORY ${SF1RENGINE_ROOT}/b5m
#)
#ADD_EXECUTABLE(b5m_search_keyword
#b5m_search_keyword.cc
#)
#TARGET_LINK_LIBRARIES(b5m_search_keyword  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  ${libs})
#SET_TARGET_PROPERTIES(b5m_search_keyword PROPERTIES
#RUNTIME_OUTPUT_DIRECTORY ${SF1RENGINE_ROOT}/b5m
#)
#
#ADD_EXECUTABLE(b5m_extract_keyword
#b5m_extract_keyword.cc
#)
#TARGET_LINK_LIBRARIES(b5m_extract_keyword  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  ${libs})
#SET_TARGET_PROPERTIES(b5m_extract_keyword PROPERTIES
#RUNTIME_OUTPUT_DIRECTORY ${SF1RENGINE_ROOT}/b5m
#)
#
#
#ADD_EXECUTABLE(b5m_category_mapper
#b5m_category_mapper.cc
#)
#TARGET_LINK_LIBRARIES(b5m_category_mapper  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  ${libs})
#SET_TARGET_PROPERTIES(b5m_category_mapper PROPERTIES
#RUNTIME_OUTPUT_DIRECTORY ${SF1RENGINE_ROOT}/b5m
#)
#ADD_EXECUTABLE(categorize_test
#categorize_test.cc
#)
#TARGET_LINK_LIBRARIES(categorize_test  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  ${libs})
#SET_TARGET_PROPERTIES(categorize_test PROPERTIES
#RUNTIME_OUTPUT_DIRECTORY ${SF1RENGINE_ROOT}/testbin/b5m
#)
#ADD_EXECUTABLE(spu_match_test
#spu_match_test.cc
#)
#TARGET_LINK_LIBRARIES(spu_match_test  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  ${libs})
#SET_TARGET_PROPERTIES(spu_match_test PROPERTIES
#RUNTIME_OUTPUT_DIRECTORY ${SF1RENGINE_ROOT}/testbin/b5m
#)
#ADD_EXECUTABLE(keywords_test
#keywords_test.cc
#)
#TARGET_LINK_LIBRARIES(keywords_test  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  ${libs})
#SET_TARGET_PROPERTIES(keywords_test PROPERTIES
#RUNTIME_OUTPUT_DIRECTORY ${SF1RENGINE_ROOT}/testbin/b5m
#)
#ADD_EXECUTABLE(keyword_found_test
#keyword_found_test.cc
#)
#TARGET_LINK_LIBRARIES(keyword_found_test  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  ${libs})
#SET_TARGET_PROPERTIES(keyword_found_test PROPERTIES
#RUNTIME_OUTPUT_DIRECTORY ${SF1RENGINE_ROOT}/testbin/b5m
#)


ADD_TEST(t_commentdb "${PROJECT_SOURCE_DIR}/testbin/t_commentdb")
ADD_TEST(t_offerdb "${PROJECT_SOURCE_DIR}/testbin/t_offerdb")
ADD_TEST(t_offer_db_recorder "${PROJECT_SOURCE_DIR}/testbin/t_offer_db_recorder")
ADD_TEST(t_product_db "${PROJECT_SOURCE_DIR}/testbin/t_product_db")
#ADD_TEST(commentdb "${SF1RENGINE_ROOT}/testbin/t_commentdb")
#ADD_TEST(commentdb "${SF1RENGINE_ROOT}/testbin/t_commentdb")

ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
