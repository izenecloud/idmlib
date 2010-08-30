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
    if( idm_term_tag == idmlib::util::IDMTermTag::KOR_COMP_NOUN )
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
        
  static bool IsKP(uint32_t id)
  {
    return ((id & (((uint32_t)(1))<<(sizeof(uint32_t)*8-1)))>0);
  }
};

NS_IDMLIB_UTIL_END

#endif
