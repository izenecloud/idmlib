##################################################


##################################################
# Our Proprietary Libraries
#####
FIND_PACKAGE(izenelib REQUIRED
  COMPONENTS
  febird
  )


FIND_PACKAGE(ilplib REQUIRED COMPONENTS langid la)
FIND_PACKAGE(imllib REQUIRED)

SET(libxml2_FOUND FALSE)
IF(IS_DIRECTORY "$ENV{LIBXML2}")
	SET(libxml2_FOUND TRUE)
	SET(libxml2_INCLUDE_DIRS "$ENV{LIBXML2}")
ELSE(IS_DIRECTORY "$ENV{LIBXML2}")
	FIND_PACKAGE(xml2 REQUIRED)
ENDIF(IS_DIRECTORY "$ENV{LIBXML2}")

IF(NOT libxml2_FOUND)
	MESSAGE(FATAL_ERROR "cannot found libxml2, please set env variable LIBXML2 (e.g. -DLIBXML2=/usr/include/libxml2)")
ENDIF(NOT libxml2_FOUND)

##################################################
# Other Libraries
#####

FIND_PACKAGE(Threads REQUIRED)

SET(Boost_ADDITIONAL_VERSIONS 1.40 1.40.0 1.39 1.39.0 1.38 1.38.0 1.37 1.37.0 1.36 1.36.0)
FIND_PACKAGE(Boost 1.36 REQUIRED
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

