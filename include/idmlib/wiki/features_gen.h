#ifndef IDMLIB_WIKI_FEATURESGEN_H_
#define IDMLIB_WIKI_FEATURESGEN_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <am/3rdparty/rde_hash.h>
#include <util/ustring/UString.h>
NS_IDMLIB_WIKI_BEGIN
    
class FeaturesGen
{
public:
  FeaturesGen(const std::string& dir);
  bool gen_redirect();
  bool gen_ner_types();
  bool output(const std::string& output_file);
  
  bool gen_all(const std::string& output_file);
    
private:
  
  void t2s_(izenelib::util::UString& ustr);
  
  bool gen_types_(const std::string& file, int type);
  
  std::string dir_;
  izenelib::am::rde_hash<izenelib::util::UCS2Char, izenelib::util::UCS2Char> t2s_map_;
  izenelib::am::rde_hash<std::string, std::string> redirect_;
  izenelib::am::rde_hash<std::string, std::vector<std::string> > redirect_image_;
  izenelib::am::rde_hash<std::string, int> types_;//peop as 1, loc as 2, org as 3
};

   
NS_IDMLIB_WIKI_END



#endif 
