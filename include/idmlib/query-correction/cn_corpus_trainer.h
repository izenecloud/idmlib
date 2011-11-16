#ifndef IDMLIB_QC_CNCORPUSTRAINER_H_
#define IDMLIB_QC_CNCORPUSTRAINER_H_

#include <string>
#include <vector>
#include <util/ustring/UString.h>
#include <am/3rdparty/rde_hash.h>
#include <idmlib/util/idm_analyzer.h>
#include <idmlib/util/mtrie.h>
#include <idmlib/util/MemoryTrie.hpp>
#include <boost/unordered_map.hpp>
#include "qc_types.h"

NS_IDMLIB_QC_BEGIN

class CnCorpusTrainer
{

    typedef idmlib::util::MTrie<std::string, uint32_t> PinyinDictType;
    typedef idmlib::util::MTrie<std::string, uint32_t, std::string> FuzzyDictType;
public:
    CnCorpusTrainer(idmlib::util::IDMAnalyzer* analyzer);
//         void LoadPinyinFile(const std::string& file);
    void Train(const std::string& raw_text_input, const std::string& output_dir);
    void TrainT(const std::string& raw_text_input, const std::string& output_dir);

private:

    void FindUnigram_(const Unigram& u);
    void FindBigram_(const Bigram& b);
    void FindTrigram_(const Trigram& t);

private:
    idmlib::util::IDMAnalyzer* analyzer_;
    boost::unordered_map<Unigram, uint64_t> unigram_;
    boost::unordered_map<Bigram, uint64_t> bigram_;
    boost::unordered_map<Trigram, uint64_t> trigram_;
    uint64_t unigram_count_;
};

NS_IDMLIB_QC_END

#endif
