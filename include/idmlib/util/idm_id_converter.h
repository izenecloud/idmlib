///
/// @file idm_analyzer.h
/// @brief To provide the LA functions for mining tasks.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-08-26
/// @date 
///

#ifndef IDM_UTIL_IDCONVERTER_H_
#define IDM_UTIL_IDCONVERTER_H_


#include <la/LA.h>
#include <idmlib/idm_types.h>
#include "idm_term_tag.h"
#include "idm_term.h"
NS_IDMLIB_UTIL_BEGIN



class IDMIdConverter
{
 public:
  static uint32_t GetId(const izenelib::util::UString& term_string, const std::string& la_tag)
  {
    char tag = IDMTermTag::GetTermTag(la_tag);
    return GetId(term_string, tag);
  }
  
  static uint32_t GetId(const izenelib::util::UString& term_string, char idm_term_tag = idmlib::util::IDMTermTag::CHN)
  {
    static uint32_t max = ((uint32_t)(-1))>>1;
    bool canBeSingleLabel = false;
    if( idm_term_tag == idmlib::util::IDMTermTag::COMP_NOUN )
    {
        canBeSingleLabel = true;
    }
    uint32_t flag = canBeSingleLabel? (((uint32_t)(1))<<(sizeof(uint32_t)*8-1)): 0;
    uint32_t termId = (izenelib::util::HashFunction<izenelib::util::UString>::generateHash32(term_string)
    & max)
    |flag;
    if( termId == 0 ) ++termId;
    return termId;
  }
  
  static void GenId(idmlib::util::IDMTerm& term)
  {
      term.id = GetId(term.text, term.tag);
  }
  
  static uint32_t GetSpaceId()
  {
    return 0;
  }
        
  static bool IsCompoundNoun(uint32_t id)
  {
    return ((id & (((uint32_t)(1))<<(sizeof(uint32_t)*8-1)))>0);
  }
  
  static uint64_t GetCNBigramId(const izenelib::util::UString& bigram_string)
  {
    if( bigram_string.length()!=2 ) return 0;
    uint32_t id1 = GetId( bigram_string.substr(0,1) );
    uint32_t id2 = GetId( bigram_string.substr(1,1) );
    return make64UInt(id1, id2);
    
  }
  
  inline static uint64_t make64UInt(uint32_t int1, uint32_t int2)
  {
      uint64_t r = int1;
      r = r<<32;
      r += int2;
      return r;
  }
};

NS_IDMLIB_UTIL_END

#endif
