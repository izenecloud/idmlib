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

FILE(GLOB query-suggestion_SRC
  "query-suggestion/*.cpp"
  "query-suggestion/*.cc"
)

FILE(GLOB dc_SRC
  "dc/*.cpp"
  "dc/*.cc"
)


ADD_LIBRARY(idmlib 
${keyphrase_extraction_SRC}
${duplicate-detection_SRC}
${nec_SRC}
${query-suggestion_SRC}
${dc_SRC}
${util_SRC}

)

INSTALL(TARGETS idmlib
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )


