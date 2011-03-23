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
#include <idmlib/semantic_space/normalizer.h>
#include <idmlib/semantic_space/vector_traits.h>
#include <idmlib/semantic_space/apss.h>
#include <am/sequence_file/ssfr.h>
NS_IDMLIB_SIM_BEGIN

///@see RandomIndexing class
template <typename IdType = uint32_t, typename ContextIdType = uint32_t, typename FreqType = uint32_t>
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
  
  TermSimilarity(const std::string& dir, const std::string& rig_dir, SimTableType* table, uint8_t sim_per_term = 5, double sim_min_score = 0.4)
  :RIType(dir+"/ri", rig_dir), dir_(dir), apss_(NULL), table_(table), sim_per_term_(sim_per_term), sim_min_score_(sim_min_score)
  , sim_list_writer_(NULL)
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
      if(!normal_matrix_writer.Append(normal_vec))
      {
        delete vector_reader;
        return false;
      }
    }
    vector_reader->Close();
    delete vector_reader;
    normal_matrix_writer.Close();
    std::cout<<"[TermSimilarity] normalization finished."<<std::endl;
    if(sim_list_writer_!=NULL)
    {
      delete sim_list_writer_;
    }
    std::string sim_list_file = dir_+"/sim_list_file";
    boost::filesystem::remove_all(sim_list_file);
    sim_list_writer_ = new izenelib::am::ssf::Writer<uint8_t>(sim_list_file);
    if(!sim_list_writer_->Open())
    {
      delete sim_list_writer_;
      return false;
    }
    max_id_ = 0;
    NormalizedType normal_vec;
    apss_->Compute(normal_matrix_file, normal_vec);
    sim_list_writer_->Close();
    delete sim_list_writer_;
    sim_list_writer_ = NULL;
    table_->ResizeIf(max_id_);
    izenelib::am::ssf::Sorter<uint8_t, IdType, false>::Sort(sim_list_file);
    izenelib::am::ssf::Reader<uint8_t> sim_list_reader(sim_list_file);
    if(!sim_list_reader.Open()) return false;
    IdType base = 0;
    std::pair<IdType, double> sim;
    IdType last_base = 0;
    std::vector<std::pair<double, IdType> > sim_list;
    while(sim_list_reader.Next(base, sim))
    {
      if(base!=last_base && !sim_list.empty())
      {
        std::sort(sim_list.begin(), sim_list.end(), std::greater<std::pair<double, IdType> >());
        if(!UpdateSim_(last_base, sim_list))
        {
          return false;
        }
        sim_list.resize(0);
      }
      sim_list.push_back(std::make_pair(sim.second, sim.first));
      last_base = base;
    }
    if(!sim_list.empty())
    {
      std::sort(sim_list.begin(), sim_list.end(), std::greater<std::pair<double, IdType> >());
      if(!UpdateSim_(last_base, sim_list))
      {
        return false;
      }
      sim_list.resize(0);
    }
    sim_list_reader.Close();
    boost::filesystem::remove_all(sim_list_file);
    if(!table_->Flush())
    {
      std::cerr<<"sim table flush failed"<<std::endl;
      return false;
    }
    return true;
  }
  
  bool GetSimVector(IdType id, std::vector<IdType>& vec)
  {
    return table_->Get(id, vec);
  }
  
 private:
  
  void FindSim_(IdType id1, IdType id2, double score)
  {
    IdType lid = id1+1;
    IdType rid = id2+1;
    AddSim_(lid, rid, score);
    AddSim_(rid, lid, score);
//     std::cout<<id1<<","<<id2<<"\t"<<score<<std::endl;
  }
  
  void AddSim_(IdType base, IdType sim, double score)
  {
    if(!sim_list_writer_->Append(base, std::make_pair(sim, score) ))
    {
      std::cerr<<"sim list append "<<base<<","<<sim<<","<<score<<" failed"<<std::endl;
    }
    if(base>max_id_) max_id_ = base;
  }
  
  bool UpdateSim_(IdType base, const std::vector<std::pair<double, IdType> >& sim_list)
  {
    uint32_t count = sim_list.size()>sim_per_term_?sim_per_term_:sim_list.size();
    std::vector<IdType> sim_vec;
    for(uint32_t c=0;c<count;c++)
    {
      
      sim_vec.push_back(sim_list[c].second);
      
    }
    //storage the similar items
    if(!table_->Update(base, sim_vec))
    {
      std::cerr<<"sim table update on "<<base<<" failed."<<std::endl;
      return false;
    }
    {
      //debug score
//         std::cout<<"[score] "<<i<<" : "<<for_sort[0].first<<","<<for_sort[count-1].first<<std::endl;
    }
    return true;
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
  SimTableType* table_;
  uint8_t sim_per_term_;//max similar items count per id
  double sim_min_score_;//min score of similarity measure
  izenelib::am::ssf::Writer<uint8_t>* sim_list_writer_;
  IdType max_id_;
//   std::vector<std::vector<std::pair<double, IdType> > > tmp_sim_list_;
};

   
NS_IDMLIB_SIM_END



#endif 
