#ifndef IDMLIB_B5M_B5MDEF_H_
#define IDMLIB_B5M_B5MDEF_H_

#include <idmlib/idm_types.h>
#include <sf1common/PropertyValue.h>
#include <sf1common/ScdDocument.h>
#include <sf1common/Document.h>
#include <sf1common/JsonDocument.h>
#include <sf1common/ScdWriter.h>
#include <sf1common/ScdParser.h>
#include <sf1common/ScdTypeWriter.h>
#include <sf1common/PairwiseScdMerger.h>


NS_IDMLIB_B5M_BEGIN
typedef uint32_t term_t;
typedef uint32_t cid_t;
//typedef izenelib::ScdDocument ScdDocument;
using izenelib::ScdDocument;
using izenelib::Document;
using izenelib::JsonDocument;
using izenelib::ScdParser;
using izenelib::ScdWriter;
using izenelib::SCDDoc;
using izenelib::ScdTypeWriter;
using izenelib::PairwiseScdMerger;
using izenelib::PropertyValue;
using izenelib::SCD_TYPE;
using izenelib::NOT_SCD;
using izenelib::UPDATE_SCD;
using izenelib::INSERT_SCD;
using izenelib::DELETE_SCD;
using izenelib::RTYPE_SCD;
NS_IDMLIB_B5M_END

#endif

