///
/// @file idm_id_manager.h
/// @brief indicate the interfaces of input class for idmlib
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-08-27
/// @date Updated 2010-08-27
///

#ifndef IDM_IDMIDMANAGER_H_
#define IDM_IDMIDMANAGER_H_

#include <string>
#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <util/ustring/UString.h>
#include "../idm_types.h"

NS_IDMLIB_UTIL_BEGIN

class IDMIdManager
{
  typedef izenelib::ir::idmanager::HDBIDStorage< izenelib::util::UString, uint32_t> StorageType ;
  public:
    IDMIdManager(const std::string& dir)
    {
      boost::filesystem::create_directories(dir);
      storage_ = new StorageType(dir+"/idmid");
    }
    
    ~IDMIdManager()
    {
      delete storage_;
    }
    
    bool GetStringById(uint32_t id, izenelib::util::UString& ustr)
    {
        boost::mutex::scoped_lock lock(mutex_);
        return storage_->get(id, ustr);
    }

    void Put(uint32_t termId, const izenelib::util::UString& ustr)
    {
        boost::mutex::scoped_lock lock(mutex_);
        storage_->put(termId, ustr);
    }
    
    void Flush()
    {
      storage_->flush();
    }
    
  private:
    StorageType* storage_;
    boost::mutex mutex_;
};


NS_IDMLIB_UTIL_END

#endif 
