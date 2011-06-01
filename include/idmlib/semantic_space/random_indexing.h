#ifndef IDMLIB_SSP_RANDOMINDEXING_H_
#define IDMLIB_SSP_RANDOMINDEXING_H_


#include <string>
#include <iostream>
#include <vector>
#include <idmlib/idm_types.h>
#include <am/sequence_file/ssfr.h>
#include "random_indexing_generator.h"
#include "vector_util.h"
#include "random_indexing_vector_traits.h"
NS_IDMLIB_SSP_BEGIN

///@brief Random indexing is used to generate a fix length(column) matrix. IdType is used for row id, if we used it for termsimilarity, then it is term id type.  ContextIdType is the type used to represent the context, if this is a term-document matrix, then ContextIdType is doc id type, otherwise, for a term - word context type, ContextIdType is word id type.
template <typename IdType = uint32_t, typename ContextIdType = uint32_t, typename FreqType = uint32_t>
class RandomIndexing
{
  
    
public:
  
  typedef uint16_t DimensionsType;
  typedef RandomIndexingVectorTraits<FreqType> RIVTraitsType;
  typedef typename RIVTraitsType::ValueType VectorValueType;
  ///@brief The random indexing vector generator
  typedef RandomIndexingGenerator<DimensionsType,ContextIdType>  RIGType;
  typedef std::vector<VectorValueType> VT;
  typedef izenelib::am::SparseVector<VectorValueType, DimensionsType> SparseType;
  ///@brief The container matrix type
  RandomIndexing(const std::string& dir, const std::string& rig_dir)
  :dir_(dir), storage_file_(dir_+"/storage"), run_file_(dir_+"/run"), writer_(NULL), rig_(new RIGType(rig_dir, 5000, 5))
  {
  }
  ~RandomIndexing()
  {
    if(rig_!=NULL) delete rig_;
    if(writer_!=NULL) delete writer_;
  }
  
  inline DimensionsType GetDimensions() const
  {
    return rig_->GetDimensions();
  }
  
  bool Open()
  {
    if(!rig_->Open())
    {
      return false;
    }
    try{
      boost::filesystem::create_directories(dir_);
      boost::filesystem::remove_all(run_file_);
    }
    catch(std::exception& ex)
    {
      std::cerr<<ex.what()<<std::endl;
      return false;
    }
    writer_ = new izenelib::am::ssf::Writer<>(run_file_);
    if(!writer_->Open())
    {
      std::cerr<<"RI open writer error"<<std::endl;
      return false;
    }
    izenelib::am::ssf::Reader<> reader(storage_file_);
    if(reader.Open())
    {
      IdType key;
      VT value;
      while(reader.Next(key, value))
      {
        if(!writer_->Append(key, value))
        {
          return false;
        }
      }
    }
    reader.Close();
    return true;
  }
  
  bool Flush()
  {
    if(writer_!=NULL)
    {
      writer_->Close();
      delete writer_;
      writer_ = NULL;
    }
    if(boost::filesystem::exists(run_file_))
    {
      izenelib::am::ssf::Sorter<uint32_t, uint32_t>::Sort(run_file_);
      izenelib::am::ssf::Reader<> reader(run_file_);
      if(reader.Open())
      {
        boost::filesystem::remove_all(storage_file_);
        izenelib::am::ssf::Writer<> writer(storage_file_);
        if(!writer.Open())
        {
          return false;
        }
        IdType key;
        SparseType value;
        IdType last_key = 0;
        SparseType last_value;
        bool first = true;
        while(reader.Next(key, value))
        {
          if(!first && key!=last_key)
          {
            if(!writer.Append(last_key, last_value)) return false;
            last_value.value.clear();
          }
          VectorUtil::VectorAdd(last_value, value);
          last_key = key;
          first = false;
        }
        if(!writer.Append(last_key, last_value)) return false;
        writer.Close();
      }
      reader.Close();
      boost::filesystem::remove_all(run_file_);
    }
    return true;
  }
  
  izenelib::am::ssf::Reader<>* GetVectorReader()
  {
    izenelib::am::ssf::Reader<>* reader = new izenelib::am::ssf::Reader<>(storage_file_);
    return reader;
  }
  
  void Clear()
  {
    rig_->Clear();
  }
  
  bool SetContextMax(ContextIdType end)
  {
    if(end<1) return false;
    std::cout<<"generating riv for max cid "<<end<<std::endl;
    rig_->ResetMax(end);
//     for(ContextIdType cid = 1; cid<=end; cid++)
//     {
//       typename RIGType::RIVType riv;
//       if(!rig_->Get(cid, riv))
//       {
//         std::cerr<<"can not get riv for "<<cid<<std::endl;
//         return false;
//       }
//     }
    std::cout<<"finished generating riv for max cid "<<end<<std::endl;
    return true;
  }
  
  ///@brief Append context to matrix
  bool Append(IdType id, const std::pair<ContextIdType, FreqType>& item)
  {
    std::vector<std::pair<ContextIdType, FreqType> > item_list(1, item);
    return Append(item_list);
  }
  
  bool Append(IdType id, const std::vector<std::pair<ContextIdType, FreqType> >& item_list)
  {
    VT vec(GetDimensions(), RIVTraitsType::GetZero());
    for(uint32_t i=0;i<item_list.size();i++)
    {
      const std::pair<ContextIdType, FreqType>& item = item_list[i];
      ContextIdType cid = item.first;
      FreqType f = item.second;
      const typename RIGType::RIVType& riv = rig_->Get(cid);
//       {
//         //debug info in riv
//         std::cout<<"[riv] "<<cid<<"\t";
//         for(uint32_t i=0;i<riv.positive_position.size();i++)
//         {
//           std::cout<<riv.positive_position[i]<<",";
//         }
//         std::cout<<"\t";
//         for(uint32_t i=0;i<riv.negative_position.size();i++)
//         {
//           std::cout<<riv.negative_position[i]<<",";
//         }
//         std::cout<<std::endl;
//       }
      
      riv.AddTo(vec, f);//add the random indexing vector to this row.
    }
    SparseType s_vec;
    VectorUtil::ConvertToSparse(vec, s_vec);
    return writer_->Append(id, s_vec);
  }
  

 
 private: 
  std::string dir_;
  std::string storage_file_;
  std::string run_file_;
  izenelib::am::ssf::Writer<>* writer_;
  RIGType* rig_;
};

   
NS_IDMLIB_SSP_END



#endif 
