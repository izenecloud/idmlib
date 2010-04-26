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

FILE(GLOB dc_SRC
  "dc/*.cpp"
  "dc/*.cc"
)


ADD_LIBRARY(idmlib 
${duplicate-detection_SRC}
${nec_SRC}
${dc_SRC}
${util_SRC}
)