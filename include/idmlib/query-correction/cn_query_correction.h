#ifndef IDMLIB_QC_CNQUERYCORRECTION_H_
#define IDMLIB_QC_CNQUERYCORRECTION_H_

#include <string>
#include <vector>
#include <util/ustring/UString.h>
#include <am/3rdparty/rde_hash.h>
#include <am/tokyo_cabinet/tc_hash.h>

#include <idmlib/util/mtrie.h>
#include <idmlib/util/MemoryTrie.hpp>
#include <boost/unordered_map.hpp>

#include "qc_types.h"
#include "fuzzy_pinyin_segmentor.h"

NS_IDMLIB_QC_BEGIN

class CnQueryCorrection
{
    
    typedef idmlib::util::MTrie<Ngram , uint32_t, double> TrieType;
    
    public:
        CnQueryCorrection();
        
        bool Load(const std::string& dir);
        
        bool GetResult(const izenelib::util::UString& input, std::vector<izenelib::util::UString>& output);
        
        
        
    private:
        
        bool LoadRawTextTransProb_(const std::string& file);
        bool LoadTcTransProb_(const std::string& file);
        
        double TransProb_(const izenelib::util::UCS2Char& from, const izenelib::util::UCS2Char& to);
        
        //trigram version only
        double TransProbT_(const izenelib::util::UString& from, const izenelib::util::UCS2Char& to);
        
        inline double EmitProb_(const izenelib::util::UCS2Char& from, const std::string& to)
        {
            return 1.0;
        }
        
        int GetInputType_(const izenelib::util::UString& input);
        
        bool GetResultWithScore_(const izenelib::util::UString& input, int type, std::vector<CandidateResult>& output);
        void GetResultByPinyin_(const std::string& pinyin, double pinyin_score, std::vector<CandidateResult>& output);
        
        //trigram LM
        void GetResultByPinyinT_(const std::string& pinyin, double pinyin_score, std::vector<CandidateResult>& output);
        
        void GetResultByPinyinTRecur_(const std::string& pinyin, double base_score, const std::pair<double, izenelib::util::UString>& mid_result, std::vector<CandidateResult>& output);
        
        double GetScore_(const izenelib::util::UString& text, double ori_score, double pinyin_score);
        
        bool IsCandidate_(const izenelib::util::UString& text, double ori_score, double pinyin_score, double& score);
        
        bool IsCandidateResult_(const izenelib::util::UString& text, double ori_score, double pinyin_score, double& score);
    private:

        FuzzyPinyinSegmentor pinyin_;
        boost::unordered_map<Unigram, double> u_trans_prob_;
        boost::unordered_map<Bigram, double> b_trans_prob_;
        boost::unordered_map<Trigram, double> t_trans_prob_;
        
        izenelib::am::tc_hash<Ngram, double>* tc_trans_prob_;
        
        
//         TrieType trans_prob_;
        
        
        double threshold_;
        double mid_threshold_;
        uint16_t max_pinyin_term_;
    
};

NS_IDMLIB_QC_END

#endif
