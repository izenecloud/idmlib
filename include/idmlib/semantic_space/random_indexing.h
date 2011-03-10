#ifndef IDMLIB_SSP_RANDOMINDEXING_H_
#define IDMLIB_SSP_RANDOMINDEXING_H_


#include <string>
#include <iostream>
#include <vector>
#include <idmlib/idm_types.h>

#include "random_indexing_generator.h"

NS_IDMLIB_SSP_BEGIN


template <template <typename TVT, typename TI> class MatrixIo, class VT = std::vector<int32_t>, typename IdType = uint32_t, typename ContextIdType = uint32_t, typename FreqType = uint32_t>
class RandomIndexing : public MatrixIo<VT, IdType>
{
public:
  typedef RandomIndexingGenerator<uint16_t,ContextIdType>  RIGType;
  typedef MatrixIo<VT, IdType> MatrixIoType;
  RandomIndexing(const std::string& dir)
  :MatrixIoType(dir+"/matrix"), dir_(dir), rig_(new RIGType(dir_+"/rig", 5000, 50))
  {
  }
  ~RandomIndexing()
  {
    if(rig_!=NULL) delete rig_;
  }
  
  bool Open()
  {
    if(!MatrixIoType::Open())
    {
      return false;
    }
    if(!rig_->Open())
    {
      return false;
    }
    return true;
  }
  
  bool Flush()
  {
    if(!MatrixIoType::Flush())
    {
      return false;
    }
    if(!rig_->Flush())
    {
      return false;
    }
    return true;
  }
  
  bool Append(IdType id, const std::pair<ContextIdType, FreqType>& item)
  {
    ContextIdType cid = item.first;
    FreqType f = item.second;
    typename RIGType::RIVType riv;
    if(!rig_->Get(cid, riv))
    {
      std::cerr<<"can not get riv for "<<cid<<std::endl;
      return false;
    }
    VT vec;
    if(!GetVector(id, vec) )
    {
      vec.resize(rig_->GetDimensions(), 0);
    }
    riv.AddTo(vec, f);
    if(!SetVector(id, vec) )
    {
      std::cerr<<"can not set vector for "<<id<<std::endl;
      return false;
    }
    return true;
  }
  
  bool Append(IdType id, const std::vector<std::pair<ContextIdType, FreqType> >& item_list)
  {
    for(uint32_t i=0;i<item_list.size();i++)
    {
      if(!Append(id, item_list[i])) return false;
    }
    return true;
  }
  

 
 private: 
  std::string dir_;
  RIGType* rig_;
};

   
NS_IDMLIB_SSP_END



#endif 
