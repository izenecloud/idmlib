#ifndef IDMLIB_B5M_SPUTERMINDEX_H_
#define IDMLIB_B5M_SPUTERMINDEX_H_
#include "b5m_helper.h"
#include "b5m_types.h"
#include "scd_doc_processor.h"
#include "b5m_helper.h"
#include <sf1common/ScdParser.h>
#include <sf1common/ScdWriter.h>
#include <am/sequence_file/ssfr.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/algorithm/searching/boyer_moore.hpp>

//#define BE_DEBUG

NS_IDMLIB_B5M_BEGIN

using izenelib::util::UString;

class SpuTermIndex {
public:
    struct ForwardItem {
        word_t word;
        std::string text;
    };
    typedef std::vector<ForwardItem> Forward;
    struct PostingItem {
        uint32_t kid;//keyword id
        uint32_t pos;//position in keyword
    };
    typedef std::pair<uint32_t, uint32_t> InvertKey;//termid, cid
    typedef std::vector<PostingItem> InvertValue;
    typedef std::map<InvertKey, InvertValue> Invert;
    SpuMatcher()
    {
    }

    ~SpuMatcher()
    {
    }

    bool Load(const std::string& knowledge)
    {
    }

private:
    void LoadCategory_(const std::string& file)
    {
    }

private:
    Forward forward_;
    Invert invert_;
};

NS_IDMLIB_B5M_END


