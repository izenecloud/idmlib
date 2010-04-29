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
${duplicate-detection_SRC}
${nec_SRC}
${query-suggestion_SRC}
${dc_SRC}
${util_SRC}
${keyphrase_extraction_SRC}
)

TARGET_LINK_LIBRARIES(idmlib
	${libs}
)
