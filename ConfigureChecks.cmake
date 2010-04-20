##################################################


##################################################
# Our Proprietary Libraries
#####
FIND_PACKAGE(izenelib REQUIRED COMPONENTS
  message_framework
  index_manager
  febird
  udt3
  izene_log
  bigint
  procmeminfo
  luxio
  jemalloc
  )


FIND_PACKAGE(wiselib REQUIRED)
FIND_PACKAGE(lalib REQUIRED)
FIND_PACKAGE(imllib REQUIRED)
FIND_PACKAGE(xml2 REQUIRED)

##################################################
# Other Libraries
#####

FIND_PACKAGE(Threads REQUIRED)

SET(Boost_ADDITIONAL_VERSIONS 1.40 1.40.0 1.39 1.39.0 1.38 1.38.0 1.37 1.37.0)
FIND_PACKAGE(Boost 1.38 REQUIRED
  COMPONENTS
  system
  program_options
  thread
  regex
  date_time
  serialization
  filesystem
  unit_test_framework
  iostreams
  )

FIND_PACKAGE(TokyoCabinet 1.4.29 REQUIRED)
FIND_PACKAGE(Glog REQUIRED)

