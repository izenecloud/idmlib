#ifndef IDMLIB_NEC_NEC_H_
#define IDMLIB_NEC_NEC_H_


#include <string>
#include <iostream>
#include <sstream>
#include <idmlib/idm_types.h>
#include <idmlib/util/svm.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <util/ustring/UString.h>
#include <am/3rdparty/rde_hash.h>
#include "nec_item.h"
NS_IDMLIB_NEC_BEGIN
    
class NEC
{
public:
  NEC();
  ~NEC();
  
  bool Load(const std::string& dir);
  
  int Predict(const NECItem& item);
  
private:
  izenelib::am::rde_hash<std::string, int> feature_index_;
  int len_index_;
  struct svm_model* model_;
  
  izenelib::am::rde_hash<izenelib::util::UString, int> predefined_types_;
};

   
NS_IDMLIB_NEC_END



#endif 
