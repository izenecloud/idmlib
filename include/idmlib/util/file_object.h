#ifndef IDMLIB_UTIL_FILEOBJECT_H_
#define IDMLIB_UTIL_FILEOBJECT_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
NS_IDMLIB_UTIL_BEGIN
    
template <typename T>
class FileObject
{
public:
  FileObject(const std::string& file):file_(file)
  {}
  ~FileObject()
  {}
  
  void SetValue(const T& value)
  {
    value_ = value;
  }
  
  T GetValue()
  {
    return value_;
  }
  
  
  bool Save()
  {
    std::ofstream ofs(file_.c_str());
    ofs<<value_;
    
    ofs.close();
    if( ofs.fail() ) return false;
    return true;
  }
  
  bool Load()
  {
    std::ifstream ifs(file_.c_str());
    ifs>>value_;
    
    ifs.close();
    if( ifs.fail() ) return false;
    return true;
  }
  
private:
  std::string file_;
  T value_;
        
};

   
NS_IDMLIB_UTIL_END



#endif /* SF1V5_FILEOBJECT_H_ */