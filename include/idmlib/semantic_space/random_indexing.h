#ifndef IDMLIB_SSP_RANDOMINDEXING_H_
#define IDMLIB_SSP_RANDOMINDEXING_H_


#include <string>
#include <iostream>
#include <vector>
#include <idmlib/idm_types.h>

#include "random_indexing_generator.h"

NS_IDMLIB_SSP_BEGIN

///@brief Random indexing is used to generate a fix length(column) matrix. IdType is used for row id, if we used it for termsimilarity, then it is term id type.  ContextIdType is the type used to represent the context, if this is a term-document matrix, then ContextIdType is doc id type, otherwise, for a term - word context type, ContextIdType is word id type.
template <template <typename TVT, typename TI> class MatrixIo, class VT = std::vector<int32_t>, typename IdType = uint32_t, typename ContextIdType = uint32_t, typename FreqType = uint32_t>
class RandomIndexing : public MatrixIo<VT, IdType>
{
public:
  
  ///@brief The random indexing vector generator
  typedef RandomIndexingGenerator<uint16_t,ContextIdType>  RIGType;
  
  ///@brief The container matrix type
  typedef MatrixIo<VT, IdType> MatrixIoType;
  RandomIndexing(const std::string& dir)
  :MatrixIoType(dir+"/matrix"), dir_(dir), rig_(new RIGType(dir_+"/rig", 5000, 20))
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
  
  ///@brief Append context to matrix
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
//     {
//       //debug info in riv
//       std::cout<<"[riv] "<<cid<<"\t";
//       for(uint32_t i=0;i<riv.positive_position.size();i++)
//       {
//         std::cout<<riv.positive_position[i]<<",";
//       }
//       std::cout<<"\t";
//       for(uint32_t i=0;i<riv.negative_position.size();i++)
//       {
//         std::cout<<riv.negative_position[i]<<",";
//       }
//       std::cout<<std::endl;
//     }
    VT vec;
    if(!GetVector(id, vec) )
    {
      //id appears in first time.
      //do nothing.
    }
    riv.AddTo(vec, f, rig_->GetDimensions());//add the random indexing vector to this row.
    if(!SetVector(id, vec) )//update it
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
