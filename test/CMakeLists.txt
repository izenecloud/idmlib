FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS unit_test_framework serialization)
#LIST(FIND REAL_BUILD_COMPONENTS "febird" _febird)

IF(Boost_FOUND)

  GET_FILENAME_COMPONENT(FileTestHelper
    "${CMAKE_CURRENT_SOURCE_DIR}/TestHelper.h" ABSOLUTE)
  SET(FileTestHelper "${FileTestHelper}")

  GET_FILENAME_COMPONENT(TEST_TMP_DIR
    ${IDMLIB_SOURCE_DIR}/testbin/tmp
    ABSOLUTE
    )
  EXECUTE_PROCESS(COMMAND cmake -E make_directory "${TEST_TMP_DIR}")

  # escape path
  STRING(REPLACE "\\" "\\\\" TEST_TMP_DIR ${TEST_TMP_DIR})
  STRING(REPLACE "\n" "\\n" TEST_TMP_DIR ${TEST_TMP_DIR})
  STRING(REPLACE "\"" "\\\"" TEST_TMP_DIR ${TEST_TMP_DIR})

  CONFIGURE_FILE(TestDir.h.in ${CMAKE_CURRENT_SOURCE_DIR}/TestDir.h)

  ADD_SUBDIRECTORY(keyphrase-extraction)
  ADD_SUBDIRECTORY(nec)
#  ADD_SUBDIRECTORY(duplicate-detection)

ENDIF(Boost_FOUND)
