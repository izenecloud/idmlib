///
/// @file ub_info.h
/// @brief The unigram and bigram statistic information extracted from some corpora, like Wikipedia.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-11-02
/// @date Updated 2010-11-02
///  --- Log


#ifndef IDM_KPEUBINFO_H_
#define IDM_KPEUBINFO_H_

#include <idmlib/util/idm_id_converter.h>
#include <idmlib/util/resource_util.h>
#include <iostream>
NS_IDMLIB_KPE_BEGIN

struct UnigramInfo
{
  UnigramInfo()
  : total_count(0), p_count(0), a_count(0), b_count(0), d_count(0), e_count(0), s_count(0)
  {
  }
  UnigramInfo(uint64_t _total_count, uint64_t _p_count, uint64_t _a_count, uint64_t _b_count, uint64_t _d_count, uint64_t _e_count, uint64_t _s_count)
  : total_count(_total_count), p_count(_p_count), a_count(_a_count), b_count(_b_count), d_count(_d_count), e_count(_e_count), s_count(_s_count)
  {
  }
  
  uint64_t total_count;
  //for example prefix, abcde, suffix
  uint64_t p_count;
  uint64_t a_count;
  uint64_t b_count;
  uint64_t d_count;
  uint64_t e_count;
  uint64_t s_count;
};

struct BigramInfo
{
  BigramInfo()
  : total_count(0), pa_count(0), ab_count(0), de_count(0), es_count(0)
  {
  }
  BigramInfo(uint64_t _total_count, uint64_t _pa_count, uint64_t _ab_count, uint64_t _de_count, uint64_t _es_count)
  : total_count(_total_count), pa_count(_pa_count), ab_count(_ab_count), de_count(_de_count), es_count(_es_count)
  {
  }
  uint64_t total_count;
  //for example prefix, abcde, suffix
  uint64_t pa_count;
  uint64_t ab_count;
  uint64_t de_count;
  uint64_t es_count;
};


class UBInfo
{
public:
    
    
  UBInfo()
  {
      
  }
  
  //only for Chinese
  bool load(const std::string& file)
  {
    std::istream* ifs = idmlib::util::getResourceStream(file);
    if( ifs== NULL)
    {
      std::cerr<<"ubinfo load failed."<<std::endl;
      return false;
    }
    std::string line;
    while ( getline ( *ifs,line ) )
    {
      boost::algorithm::trim( line );
      std::vector<std::string> items;
      boost::algorithm::split( items, line, boost::algorithm::is_any_of(",") );
      if( items[0] == "stat" )
      {
        unigram_count_ = boost::lexical_cast<uint64_t>(items[1]);
        bigram_count_ = boost::lexical_cast<uint64_t>(items[2]);
      }
      else if( items[0] == "bigram" )
      {
        izenelib::util::UString bigram_text(items[1], izenelib::util::UString::UTF_8);
        uint64_t bigram_id = idmlib::util::IDMIdConverter::GetCNBigramId(bigram_text);
        uint64_t total_count = boost::lexical_cast<uint64_t>(items[2]);
        uint64_t pa_count = boost::lexical_cast<uint64_t>(items[3]);
        uint64_t ab_count = boost::lexical_cast<uint64_t>(items[4]);
        uint64_t de_count = boost::lexical_cast<uint64_t>(items[5]);
        uint64_t es_count = boost::lexical_cast<uint64_t>(items[6]);
        BigramInfo bigram_info(total_count, pa_count, ab_count, de_count, es_count);
        bigram_info_.insert(bigram_id, bigram_info);
      }
      else if( items[0] == "unigram" )
      {
        izenelib::util::UString unigram_text(items[1], izenelib::util::UString::UTF_8);
        uint64_t unigram_id = idmlib::util::IDMIdConverter::GetId(unigram_text);
        uint64_t total_count = boost::lexical_cast<uint64_t>(items[2]);
        uint64_t p_count = boost::lexical_cast<uint64_t>(items[3]);
        uint64_t a_count = boost::lexical_cast<uint64_t>(items[4]);
        uint64_t b_count = boost::lexical_cast<uint64_t>(items[5]);
        uint64_t d_count = boost::lexical_cast<uint64_t>(items[6]);
        uint64_t e_count = boost::lexical_cast<uint64_t>(items[7]);
        uint64_t s_count = boost::lexical_cast<uint64_t>(items[8]);
        UnigramInfo unigram_info(total_count, p_count, a_count, b_count, d_count, e_count, s_count);
        unigram_info_.insert(unigram_id, unigram_info);
      }
      
    }
    delete ifs;
    return true;
  }
  
  UnigramInfo* GetUnigramInfo( uint32_t id)
  {
    return unigram_info_.find(id);
  }
  
  BigramInfo* GetBigramInfo( uint64_t id)
  {
    return bigram_info_.find(id);
  }
  
  BigramInfo* GetBigramInfo( uint32_t id1, uint32_t id2)
  {
    uint64_t id = idmlib::util::IDMIdConverter::make64UInt(id1, id2);
    return GetBigramInfo(id);
  }
    
    
            
private:
    uint64_t unigram_count_;
    uint64_t bigram_count_;
    izenelib::am::rde_hash<uint32_t, UnigramInfo > unigram_info_;
    izenelib::am::rde_hash<uint64_t, BigramInfo > bigram_info_;
};
    

    
NS_IDMLIB_KPE_END

#endif
