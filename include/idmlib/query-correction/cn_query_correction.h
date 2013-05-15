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
#include <boost/tuple/tuple.hpp>
#include "qc_types.h"
#include "fuzzy_pinyin_segmentor.h"

NS_IDMLIB_QC_BEGIN


class CnQueryCorrection
{

    typedef idmlib::util::MTrie<Ngram , uint32_t, double> TrieType;
    typedef boost::tuple<uint32_t, uint32_t, izenelib::util::UString> QueryLogType;
    typedef std::pair<uint32_t, izenelib::util::UString> PropertyLabelType;
    struct TransProbType
    {
        boost::unordered_map<Unigram, double> u_trans_prob_;
        boost::unordered_map<Bigram, double> b_trans_prob_;
        boost::unordered_map<Trigram, double> t_trans_prob_;
        void clear()
        {
            u_trans_prob_.clear();
            b_trans_prob_.clear();
            t_trans_prob_.clear();
        }
        bool empty() const
        {
            return u_trans_prob_.empty() && b_trans_prob_.empty() && t_trans_prob_.empty();
        }
    };

public:
    explicit CnQueryCorrection(const std::string& collection_dir = "");

    bool Load(bool fromDb=false,const std::list<QueryLogType>& queryList=std::list<QueryLogType>() );

    bool ForceReload();

    bool Update(const std::list<QueryLogType>& queryList, const std::list<PropertyLabelType>& labelList, bool forceMode = false);

    void LoadRawTextFromVector_(TransProbType& trans_prob,const std::list<QueryLogType>& queryList);

    bool GetResult(const izenelib::util::UString& input, std::vector<izenelib::util::UString>& output);

    void GetPinyin(const izenelib::util::UString& cn_chars, std::vector<std::string>& result_list);
//WANG QIAN
    void GetPinyin2(const izenelib::util::UString& cn_chars2, std::vector<std::string>& result_list);
    void GetRelativeList(const izenelib::util::UString& hanzi,std::vector<std::pair<izenelib::util::UString,uint32_t> >& ResultList);
//...
private:

    void LoadRawTextTransProb_(TransProbType& trans_prob, const std::string& file);

    void FlushRawTextTransProb_(const std::string& file, const TransProbType& trans_prob);

    double TransProb_(const izenelib::util::UCS2Char& from, const izenelib::util::UCS2Char& to);

    void UpdateItem_(const uint32_t df, const izenelib::util::UString& text);

    //trigram version only
    double TransProbT_(const izenelib::util::UString& from, const izenelib::util::UCS2Char& to);

    int GetInputType_(const izenelib::util::UString& input);

    bool GetResultWithScore_(const izenelib::util::UString& input, int type, std::vector<CandidateResult>& output);

    void GetResultByPinyin_(const std::string& pinyin, double pinyin_score, std::vector<CandidateResult>& output);

    //trigram LM
    void GetResultByPinyinT_(const std::string& pinyin, double pinyin_score, std::vector<CandidateResult>& output);

    void GetResultByPinyinTRecur_(const std::string& pinyin, double base_score, const std::pair<double, izenelib::util::UString>& mid_result, std::vector<CandidateResult>& output);

    double GetScore_(const izenelib::util::UString& text, double ori_score, double pinyin_score);

    bool IsCandidate_(const izenelib::util::UString& text, double ori_score, double pinyin_score, double& score);

    bool IsCandidateResult_(const izenelib::util::UString& text, double ori_score, double pinyin_score, double& score);

public:
    static std::string res_dir_;

private:
    static FuzzyPinyinSegmentor pinyin_;

    static TransProbType global_trans_prob_;
    TransProbType collection_trans_prob_;

    std::string collection_dir_;
    double threshold_;
    double mid_threshold_;
    uint16_t max_pinyin_term_;

    boost::mutex mutex_;

};

NS_IDMLIB_QC_END

#endif
