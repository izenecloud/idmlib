#ifndef IDMLIB_SIM_TERMSIMILARITYTABLE_H_
#define IDMLIB_SIM_TERMSIMILARITYTABLE_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <am/matrix/matrix_mem_io.h>

NS_IDMLIB_SIM_BEGIN

template <typename IdType>
class TermSimilarityTable
{
public:
  typedef std::vector<IdType> ValueType;
  typedef izenelib::am::MatrixMemIo<ValueType> Container;
  
  TermSimilarityTable(const std::string& path, uint8_t max = 5)
  :storage_(new Container(path)), max_(max)
  {
  }
  
  ~TermSimilarityTable()
  {
    delete storage_;
  }
  
  bool Open()
  {
    return storage_->Open();
  }
  
  bool Flush()
  {
    return storage_->Flush();
  }
  
  bool Update(IdType id, const ValueType& value)
  {
    if(value.size()>max_) return false;
    if(value.size()<max_)
    {
      ValueType new_value(value.begin(), value.end());
      new_value.resize(max_, 0);
      return storage_->SetVector(id, new_value);
    }
    else
    {
      return storage_->SetVector(id, value);
    }
  }
  
  bool Get(IdType id, ValueType& value)
  {
    if(!storage_->GetVector(id, value)) return false;
    if(value.size()!=max_) return false;
    typename ValueType::size_type non_zero = value.size();
    for(typename ValueType::size_type i=value.size()-1; i>=0 ;i--)
    {
      if(value[i]!=0) break;
      non_zero = i;
    }
    value.resize(non_zero);
    return true;
  }
  

 private: 
  Container* storage_;
  uint8_t max_;
};

   
NS_IDMLIB_SIM_END



#endif 
