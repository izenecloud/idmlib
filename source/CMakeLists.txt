FILE(GLOB util_SRC
 "util/*.cc"
 "util/*.cpp"
)

FILE(GLOB duplicate-detection_SRC
  "duplicate-detection/*.cpp"
  "duplicate-detection/*.cc"
  )

FILE(GLOB nec_SRC
  "nec/*.cpp"
  "nec/*.cc"
)

FILE(GLOB keyphrase_extraction_SRC
  "keyphrase-extraction/*.cpp"
  "keyphrase-extraction/*.cc"
)

FILE(GLOB query-correction_SRC
  "query-correction/*.cc"
)

FILE(GLOB concept-clustering_SRC
  "concept-clustering/*.cc"
)

FILE(GLOB query-suggestion_SRC
  "query-suggestion/*.cpp"
  "query-suggestion/*.cc"
)

FILE(GLOB dc_SRC
  "dc/*.cpp"
  "dc/*.cc"
)

FILE(GLOB itemset_SRC
  "itemset/*.cpp"
)

FILE(GLOB wiki_SRC
  "wiki/*.cc"
)

FILE(GLOB ssp_SRC
  "semantic_space/*.cc"
  "semantic_space/esa/*.cpp"
)

FILE(GLOB sim_SRC
  "similarity/*.cc"
  "similarity/all-pairs-similarity-search/*.cpp"
)

FILE(GLOB tl_SRC
  "translation/*.cc"
)

FILE(GLOB_RECURSE resys_SRC
  "resys/*.cpp"
)

FILE(GLOB_RECURSE ise_SRC
  "ise/*.cpp"
  "ise/*.c"
)

FILE(GLOB_RECURSE topicmodels_SRC
  "topic-models/*.cpp"
  "topic-models/*.c"
)

FILE(GLOB_RECURSE  lshkit_SRCS
  "lshkit/*.cpp"
  "lshkit/*.c"
)

ADD_LIBRARY(idmlib
  ${keyphrase_extraction_SRC}
  ${duplicate-detection_SRC}
  ${nec_SRC}
  ${query-correction_SRC}
  ${concept-clustering_SRC}
  ${query-suggestion_SRC}
  ${dc_SRC}
  ${itemset_SRC}
  ${util_SRC}
  ${wiki_SRC}
  ${ssp_SRC}
  ${sim_SRC}
  ${tl_SRC}
  ${resys_SRC}
  ${ise_SRC}
  ${topicmodels_SRC}
  ${lshkit_SRCS}
)

INSTALL(TARGETS idmlib
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
