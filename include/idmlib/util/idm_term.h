///
/// @file idm_term.h
/// @brief indicate the interfaces of input class for keyphrase extraction algorithm.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-04-08
/// @date Updated 2010-04-08
///

#ifndef IDM_KPETERM_H_
#define IDM_KPETERM_H_

#include <string>
#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <util/ustring/UString.h>
#include "../idm_types.h"


NS_IDMLIB_UTIL_BEGIN


struct IDMTerm
{
  IDMTerm():text(), id(0), position(0), tag(0)
  {
  }
  
  IDMTerm(const std::string& ptext, uint32_t pposition, char ptag)
  : text(ptext, izenelib::util::UString::UTF_8), id(0), position(pposition), tag(ptag)
  {
  }
  
  IDMTerm(const izenelib::util::UString& ptext, uint32_t pposition, char ptag)
  : text(ptext), id(0), position(pposition), tag(ptag)
  {
  }
  
  bool EqualsWithoutId(const IDMTerm& term) const
  {
      if(term.text!=text) return false;
      if(term.position!=position) return false;
      if(term.tag!=tag) return false;
      return true;
  }
  
  izenelib::util::UString text;
  uint32_t id;
  uint32_t position;
  char tag;
  
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & text & id & position & tag;
    }
  
  std::string TextString() const
  {
      std::string str;
      text.convertString(str, izenelib::util::UString::UTF_8);
      return str;
  }
};


NS_IDMLIB_UTIL_END

#endif 
