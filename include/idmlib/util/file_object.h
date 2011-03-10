#ifndef IDMLIB_UTIL_FILEOBJECT_H_
#define IDMLIB_UTIL_FILEOBJECT_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <util/file_object.h>

NS_IDMLIB_UTIL_BEGIN
    
template <typename T>
class FileObject : public izenelib::util::FileObject<T>
{
  public:
  FileObject(const std::string& file)
  :izenelib::util::FileObject<T>(file)
  {
  }
};

   
NS_IDMLIB_UTIL_END



#endif /* SF1V5_FILEOBJECT_H_ */
