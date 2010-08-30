///
/// @file idm_term_tag.h
/// @brief indicate the interfaces of input class for idmlib
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-08-27
/// @date Updated 2010-08-27
///

#ifndef IDM_IDMTERMTAG_H_
#define IDM_IDMTERMTAG_H_

#include <string>
#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <util/ustring/UString.h>
#include "../idm_types.h"

NS_IDMLIB_UTIL_BEGIN

class IDMTermTag
{
  public:
    static const char CHN = 'C';
    static const char ENG = 'F';
    static const char KOR = '?';
    static const char KOR_NOUN = 'N';
    static const char KOR_LOAN = 'Z';
    static const char KOR_COMP_NOUN = 'P';
    static const char NUM = 'S';
    static const char OTHER = '@';
    
    static char GetTermTag(const std::string& la_tag)
    {
      return la_tag[0];
    }
};


NS_IDMLIB_UTIL_END

#endif 
