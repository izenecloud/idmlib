SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${IDMLIB_SOURCE_DIR}/testbin")
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

CONFIGURE_FILE(
  TestResources.h.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/TestResources.h
  ESCAPE_QUOTES @ONLY
  )

ADD_SUBDIRECTORY(tdt)
ADD_SUBDIRECTORY(keyphrase-extraction)
ADD_SUBDIRECTORY(query-correction)
ADD_SUBDIRECTORY(duplicate-detection)
ADD_SUBDIRECTORY(wiki)
ADD_SUBDIRECTORY(semantic-space)
ADD_SUBDIRECTORY(similarity)
ADD_SUBDIRECTORY(itemset)
ADD_SUBDIRECTORY(resys)
ADD_SUBDIRECTORY(topic-models)
ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(ctr)
ADD_SUBDIRECTORY(maxent_title)
