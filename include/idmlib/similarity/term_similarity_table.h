#ifndef IDMLIB_SIM_TERMSIMILARITYTABLE_H_
#define IDMLIB_SIM_TERMSIMILARITYTABLE_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <util/file_object.h>

NS_IDMLIB_SIM_BEGIN

template <typename IdType>
class TermSimilarityTable
{
public:
  typedef std::vector<IdType> ValueType;
  typedef izenelib::util::FileObject<std::vector<ValueType> > Container;
  
  TermSimilarityTable(const std::string& path)
  :storage_(new Container(path))
  {
  }
  
  ~TermSimilarityTable()
  {
    delete storage_;
  }
  
  bool Open()
  {
    return storage_->Load();
  }
  
  bool Flush()
  {
    return storage_->Save();
  }
  
  void ResizeIf(IdType max)
  {
    if(storage_->value.size()<max)
    {
      storage_->value.resize(max);
    }
  }
  
  bool Update(IdType id, const ValueType& value)
  {
    if(id==0) return false;
    if(id>storage_->value.size())
    {
      storage_->value.resize(id);
    }
    storage_->value[id-1] = value;
    return true;
  }
  
  bool Get(IdType id, ValueType& value)
  {
    if(id==0) return false;
    if(id<=storage_->value.size())
    {
      value = storage_->value[id-1];
      return true;
    }
    return true;
  }
  

 private: 
  Container* storage_;
};

   
NS_IDMLIB_SIM_END



#endif 
