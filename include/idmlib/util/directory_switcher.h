#ifndef IDMLIB_UTIL_DIRECTORYSWITCHER_H_
#define IDMLIB_UTIL_DIRECTORYSWITCHER_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
NS_IDMLIB_UTIL_BEGIN
    
class DirectorySwitcher
{
public:
  DirectorySwitcher(const std::string& parent);
  ~DirectorySwitcher();
  
  bool Open();
  
  bool GetCurrent(std::string& dir);
  
  bool GetNext(std::string& dir);
  
  bool GetNextWithDelete(std::string& dir);
  
  bool SetNextValid();
    
private:
  std::string parent_;
  std::string dirs_[2];
  uint8_t choice_;
        
};

   
NS_IDMLIB_UTIL_END



#endif /* SF1V5_FILEOBJECT_H_ */
