#ifndef IDMLIB_FUZZYPINYINSEGMENTOR_H_
#define IDMLIB_FUZZYPINYINSEGMENTOR_H_

#include <string>
#include <vector>
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

// struct FuzzyPinyinItem
// {
//     std::vector<PinyinItem> pinyin_item_list;
// };
// 
// struct PinyinSegmentResult
// {
//     std::vector<FuzzyPinyinItem> result_item;
// };

typedef std::vector<PinyinItem> FuzzyPinyinItem;
typedef std::vector<FuzzyPinyinItem> PinyinSegmentResult;

class FuzzyPinyinSegmentor
{
    typedef idmlib::util::MTrie<std::string, uint32_t> PinyinDictType;
    typedef idmlib::util::MTrie<std::string, uint32_t, std::string> FuzzyDictType;
    public:
        FuzzyPinyinSegmentor();
        void AddPinyin(const std::string& pinyin);
        void LoadPinyinFile(const std::string& file);
        
        
        void Segment(const std::string& pinyin_str, std::vector<PinyinSegmentResult>& result_list);
        void SegmentRaw(const std::string& pinyin_str, std::vector<std::string>& result_list);
        void FuzzySegmentRaw(const std::string& pinyin_str, std::vector<std::string>& result_list);
        
        bool GetFirstPinyinTerm(const std::string& pinyin, std::string& first, std::string& remain);
        bool GetFirstPinyinTerm(std::string& pinyin, std::string& first);
    private:
        
        void SegmentRaw_(const std::string& pinyin_str, const std::string& mid_result, std::vector<std::string>& result_list);
        
        void InitRule_();
        
        void AddPinyin_(const std::string& pinyin);
        
        void AddSuffixFuzzy_(const std::string& suffix, const std::string& replace);
        
        void AddPrefixFuzzy_(const std::string& prefix, const std::string& replace);
        
        bool FuzzyFilter_(const std::string& pinyin_term, std::string& fuzzy_term);
        
        
        void FuzzySegmentRaw_(const std::string& pinyin_str, const std::string& mid_result, std::vector<std::string>& result_list);
        
        
        
    private:
        PinyinDictType pinyin_dict_;
//         izenelib::am::rde_hash<std::string, bool> pinyin_term_;

        izenelib::am::stl_map<std::string, bool> filter_pinyin_;
        FuzzyDictType suffix_fuzzy_;
        FuzzyDictType prefix_fuzzy_;
    
};

NS_IDMLIB_QC_END

#endif
