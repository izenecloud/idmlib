#ifndef IDMLIB_FUZZYPINYINSEGMENTOR_H_
#define IDMLIB_FUZZYPINYINSEGMENTOR_H_

#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <util/ustring/UString.h>
#include <am/3rdparty/rde_hash.h>

#include <idmlib/util/mtrie.h>
#include <idmlib/util/MemoryTrie.hpp>

NS_IDMLIB_QC_BEGIN

struct PinyinItem
{
    std::string pinyin;
    double score;
};

typedef std::vector<PinyinItem> FuzzyPinyinItem;
typedef std::vector<FuzzyPinyinItem> PinyinSegmentResult;

class FuzzyPinyinSegmentor
{
    typedef idmlib::util::MTrie<std::string, uint32_t> PinyinDictType;
    typedef idmlib::util::MTrie<std::string, uint32_t, std::string> FuzzyDictType;
    typedef boost::unordered_map<izenelib::util::UCS2Char, std::vector<std::string> > Cn2PinyinType;
    typedef boost::unordered_map<std::string, std::vector<izenelib::util::UCS2Char> > Pinyin2CnType;
    public:
        FuzzyPinyinSegmentor();

        void AddPinyinMap(const std::string& pinyin, const izenelib::util::UCS2Char& cn_char);
        void AddPinyin(const std::string& pinyin);
        void LoadPinyinFile(const std::string& file);

        void Segment(const std::string& pinyin_str, std::vector<PinyinSegmentResult>& result_list);
        void SegmentRaw(const std::string& pinyin_str, std::vector<std::string>& result_list);
        void FuzzySegmentRaw(const std::string& pinyin_str, std::vector<std::pair<double, std::string> >& result_list);
        void FuzzySegmentRaw(const std::string& pinyin_str, std::vector<std::string>& result_list);

        void GetPinyin(const izenelib::util::UString& cn_chars, std::vector<std::pair<double, std::string> >& result_list);
        void GetPinyin(const izenelib::util::UString& cn_chars, std::vector<std::string>& result_list);
        bool GetPinyinTerm(const izenelib::util::UCS2Char& cn_char, std::vector<std::string>& result_list);

        bool GetChar(const std::string& pinyin_term, std::vector<izenelib::util::UCS2Char>& result_list);

        uint32_t CountPinyinTerm(const std::string& pinyin);
        bool GetFirstPinyinTerm(const std::string& pinyin, std::string& first, std::string& remain);
        bool GetFirstPinyinTerm(std::string& pinyin, std::string& first);

    private:

        void SegmentRaw_(const std::string& pinyin_str, const std::string& mid_result, std::vector<std::string>& result_list);

        void InitRule_();

        void AddPinyin_(const std::string& pinyin);

        void AddSuffixFuzzy_(const std::string& suffix, const std::string& replace);

        void AddPrefixFuzzy_(const std::string& prefix, const std::string& replace);

        bool FuzzyFilter_(const std::string& pinyin_term, std::string& fuzzy_term);

        void FuzzySegmentRaw_(const std::string& pinyin_str, const std::string& mid_result, double score, std::vector<std::pair<double, std::string> >& result_list);

        void GetPinyin_(const izenelib::util::UString& cn_chars, const std::string& mid_result, double score, std::vector<std::pair<double, std::string> >& result_list);

    private:
        PinyinDictType pinyin_dict_;
        Cn2PinyinType cn2pinyin_;
        Pinyin2CnType pinyin2cn_;
//         izenelib::am::rde_hash<std::string, bool> pinyin_term_;

        boost::unordered_map<std::string, bool> filter_pinyin_;
        FuzzyDictType suffix_fuzzy_;
        FuzzyDictType prefix_fuzzy_;
        double fuzzy_weight_;
};

NS_IDMLIB_QC_END

#endif
