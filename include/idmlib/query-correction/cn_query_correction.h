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
        CnQueryCorrection(const std::string& res_dir, const std::string& log_dir);
        
        bool Load();
        
        bool Update(const std::list<std::pair<izenelib::util::UString, uint32_t> >& query_logs);
        
        bool GetResult(const izenelib::util::UString& input, std::vector<izenelib::util::UString>& output);
        
        
        
    private:
        
        bool LoadRawTextTransProb_(const std::string& file);
        
        bool LoadRawText_(const std::string& file);
                
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
        std::string res_dir_;
        std::string log_dir_;
        FuzzyPinyinSegmentor pinyin_;
        boost::unordered_map<Unigram, double> u_trans_prob_;
        boost::unordered_map<Bigram, double> b_trans_prob_;
        boost::unordered_map<Trigram, double> t_trans_prob_;
        
        
        double threshold_;
        double mid_threshold_;
        uint16_t max_pinyin_term_;
    
};

NS_IDMLIB_QC_END

#endif
