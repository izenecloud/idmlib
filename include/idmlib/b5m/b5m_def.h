#ifndef IDMLIB_B5M_B5MDEF_H_
#define IDMLIB_B5M_B5MDEF_H_

#include <idmlib/idm_types.h>
#include <sf1common/PropertyValue.h>
#include <sf1common/ScdDocument.h>
#include <sf1common/Document.h>
#include <sf1common/JsonDocument.h>
#include <sf1common/ScdWriter.h>
#include <sf1common/ScdTypeWriter.h>
#include <sf1common/PairwiseScdMerger.h>


NS_IDMLIB_B5M_BEGIN
typedef uint32_t term_t;
typedef uint32_t cid_t;
typedef izenelib::ScdDocument ScdDocument;
typedef izenelib::Document Document;
typedef izenelib::JsonDocument JsonDocument;
typedef izenelib::ScdWriter ScdWriter;
typedef izenelib::ScdTypeWriter ScdTypeWriter;
typedef izenelib::PairwiseScdMerger PairwiseScdMerger;
typedef izenelib::PropertyValue PropertyValue;
NS_IDMLIB_B5M_END

#endif

