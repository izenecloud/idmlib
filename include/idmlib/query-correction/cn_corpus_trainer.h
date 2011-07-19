#ifndef IDMLIB_QC_CNCORPUSTRAINER_H_
#define IDMLIB_QC_CNCORPUSTRAINER_H_

#include <string>
#include <vector>
#include <util/ustring/UString.h>
#include <am/3rdparty/rde_hash.h>

#include <idmlib/util/mtrie.h>
#include <idmlib/util/MemoryTrie.hpp>
#include <boost/unordered_map.hpp>

NS_IDMLIB_QC_BEGIN

class CnCorpusTrainer
{
    typedef idmlib::util::MTrie<std::string, uint32_t> PinyinDictType;
    typedef idmlib::util::MTrie<std::string, uint32_t, std::string> FuzzyDictType;
    public:
        CnCorpusTrainer();
        void LoadPinyinFile(const std::string& file);
        void Train(const std::string& raw_text_input, const std::string& output_dir);
        
        
        
    private:
        
        
        
        
        
    private:
        boost::unordered_map<std::string, uint64_t> pinyin_bigram_;

    
};

NS_IDMLIB_QC_END

#endif
