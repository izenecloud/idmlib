#ifndef IDMLIB_NEC_ACCURACIER_H_
#define IDMLIB_NEC_ACCURACIER_H_


#include <string>
#include <iostream>
#include <sstream>
#include <idmlib/idm_types.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <util/ustring/UString.h>
#include <am/3rdparty/rde_hash.h>
namespace idmlib{
    
class NecAccuracier
{
public:
  void append(const std::string& item, const std::string& tag)
  {
    if( boost::ends_with(tag, "*") )
    {
      std::string ctag = tag.substr(0, tag.length()-1);
      int type = boost::lexical_cast<int>(ctag);
      type_.insert(item, type);
      o_.insert(item, 1);
    }
    else
    {
      int type = boost::lexical_cast<int>(tag);
      type_.insert(item, type);
    }
  }
  
  //-1=ignore, 1=correct, 0=incorrect
  int is_correct(const std::string& item, int type)
  {
    if(type>0)
    {
      int* tagged = type_.find(item);
      if(tagged==NULL)
      {
        return 0;
      }
      else if( (*tagged) == type )
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
    else
    {
      int* tagged = type_.find(item);
      if(tagged==NULL)
      {
        return -1;
      }
      else
      {
        if(o_.find(item)!=NULL)
        {
          return -1;
        }
        else
        {
          return 0;
        }
      }
    }
  }
  
private:
  
  izenelib::am::rde_hash<std::string, int> type_;
  izenelib::am::rde_hash<std::string, bool> o_;
  
  
};

   
}



#endif 
