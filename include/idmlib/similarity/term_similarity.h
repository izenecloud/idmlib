#ifndef IDMLIB_SIM_TERMSIMILARITY_H_
#define IDMLIB_SIM_TERMSIMILARITY_H_

#include <time.h> 
#include <stdio.h> 
#include <string>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/random_indexing.h>
#include "cosine_similarity.h"
#include "simple_similarity.h"
#include "term_similarity_table.h"
#include "sim_output_collector.h"
#include <idmlib/semantic_space/normalizer.h>
#include <idmlib/semantic_space/vector_traits.h>
#include <idmlib/semantic_space/apss.h>
#include <am/sequence_file/ssfr.h>
NS_IDMLIB_SIM_BEGIN

///@see RandomIndexing class
template <class OutputHandlerType, typename IdType = uint32_t, typename ContextIdType = uint32_t, typename FreqType = uint32_t>
class TermSimilarity : public idmlib::ssp::RandomIndexing<IdType, ContextIdType, FreqType>
{
public:

  ///@brief The random indexing base type
  typedef idmlib::ssp::RandomIndexing<IdType, ContextIdType, FreqType> RIType;
  typedef typename RIType::VT VectorType;
  typedef typename RIType::SparseType SparseType;
//   typedef RIType::RIGType RIGType;
  typedef idmlib::ssp::Apss<IdType> ApssType;
  typedef TermSimilarityTable<uint32_t> SimTableType;
  typedef izenelib::am::SparseVector<double, typename RIType::DimensionsType> NormalizedType;
  
  TermSimilarity(const std::string& dir, const std::string& rig_dir, OutputHandlerType* output_handler, double sim_min_score = 0.4)
  :RIType(dir+"/ri", rig_dir), dir_(dir), apss_(NULL)
  , output_handler_(output_handler), sim_min_score_(sim_min_score)
  {
    apss_ = new ApssType(sim_min_score_, boost::bind( &TermSimilarity::FindSim_, this, _1, _2, _3), RIType::GetDimensions() );
  }
  
  ~TermSimilarity()
  {
    delete apss_;
  }
  
  bool Open()
  {
    try
    {
      boost::filesystem::create_directories(dir_);
      if(!RIType::Open()) return false;
    }
    catch(std::exception& ex)
    {
      std::cerr<<ex.what()<<std::endl;
      return false;
    }
    return true;
  }
  
 
  bool Compute()
  {
    RIType::Clear();
    if(!RIType::Flush())
    {
      return false;
    }
    
    izenelib::am::ssf::Reader<>* vector_reader = RIType::GetVectorReader();
    if(!vector_reader->Open())
    {
      std::cerr<<"open vector reader error"<<std::endl;
      delete vector_reader;
      return false;
    }
//     IdType min = 1;
//     IdType max = RIType::VectorCount();
    
    
    std::cout<<"[TermSimilarity] total count"<<vector_reader->Count()<<std::endl;
//     typedef typename idmlib::ssp::VectorTraits<VectorType>::NormalizedType NormalizedType;
    
//     std::vector<NormalizedType> normal_matrix(max);
    std::string normal_matrix_file = dir_+"/normal_vec";
    boost::filesystem::remove_all(normal_matrix_file);
    izenelib::am::ssf::Writer<> normal_matrix_writer(normal_matrix_file);
    if(!normal_matrix_writer.Open())
    {
      delete vector_reader;
      return false;
    }
    IdType key;
    SparseType value;
    while(vector_reader->Next(key, value))
    {
      NormalizedType normal_vec;
      idmlib::ssp::Normalizer::Normalize(value, normal_vec);
      if(!normal_matrix_writer.Append(key, normal_vec))
      {
        delete vector_reader;
        return false;
      }
    }
    vector_reader->Close();
    delete vector_reader;
    normal_matrix_writer.Close();
    std::cout<<"[TermSimilarity] normalization finished."<<std::endl;

    
    IdType id = 0;
    NormalizedType normal_vec;

    apss_->Compute(normal_matrix_file, id, normal_vec);
    
    if(!output_handler_->Flush())
    {
      std::cerr<<"output handler flush failed"<<std::endl;
      return false;
    }
    return true;
  }
  
//   bool GetSimVector(IdType id, std::vector<IdType>& vec)
//   {
//     return table_->Get(id, vec);
//   }
  
 private:
  
  void FindSim_(IdType id1, IdType id2, double score)
  {
    output_handler_->AddSimPair(id1, id2, score);
  }
  
  
  
  
  std::string time_string_()
  {
    time_t t = time(NULL);
    char tmp[14]; 
    strftime( tmp, sizeof(tmp), "%H:%M:%S",localtime(&t) ); 
    std::string r(tmp);
    return r;
  }
 private: 
  
  std::string dir_;
  ApssType* apss_;
  OutputHandlerType* output_handler_;
  double sim_min_score_;//min score of similarity measure
};

   
NS_IDMLIB_SIM_END



#endif 
