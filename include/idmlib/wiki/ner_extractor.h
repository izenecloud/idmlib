#ifndef IDMLIB_WIKI_NEREXTRACTOR_H_
#define IDMLIB_WIKI_NEREXTRACTOR_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
NS_IDMLIB_WIKI_BEGIN
    
class NerExtractor
{
public:
  
  static bool extract_from_catelink(const std::string& input_file, const std::string& category_regex, const std::string& page_regex);
  
  static bool extract_from_xml(const std::string& input_xml, const std::string& title_regex);
    
};

   
NS_IDMLIB_WIKI_END



#endif 
