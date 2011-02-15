#ifndef IDMLIB_WIKI_ENTITYEXTRACTOR_H_
#define IDMLIB_WIKI_ENTITYEXTRACTOR_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
NS_IDMLIB_WIKI_BEGIN
    
class EntityExtractor
{
public:
  
  static bool extract(const std::string& input_xml, const std::string& output_file);
    
};

   
NS_IDMLIB_WIKI_END



#endif 
