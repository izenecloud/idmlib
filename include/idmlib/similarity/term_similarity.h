#ifndef IDMLIB_SIM_TERMSIMILARITY_H_
#define IDMLIB_SIM_TERMSIMILARITY_H_

#include <time.h> 
#include <stdio.h> 
#include <string>
#include <iostream>
#include <boost/function.hpp>
#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/random_indexing.h>
#include "cosine_similarity.h"
#include "simple_similarity.h"
#include "term_similarity_table.h"

NS_IDMLIB_SIM_BEGIN

///@see RandomIndexing class
template <template <typename TVT, typename TI> class MatrixIo, class VectorType, typename IdType = uint32_t, typename ContextIdType = uint32_t, typename FreqType = uint32_t>
class TermSimilarity : public idmlib::ssp::RandomIndexing<MatrixIo, VectorType, IdType, ContextIdType, FreqType>
{
public:

  ///@brief The random indexing base type
  typedef idmlib::ssp::RandomIndexing<MatrixIo, VectorType, IdType, ContextIdType, FreqType> RIType;
  
  
  TermSimilarity(const std::string& dir)
  :RIType(dir+"/ri"), SIM_PER_TERM(5), SIM_MIN_SCORE(0.4), dir_(dir), table_(new TermSimilarityTable<uint32_t>(dir+"/table", SIM_PER_TERM))
  {
  }
  
  ~TermSimilarity()
  {
    delete table_;
  }
  
  bool Open()
  {
    try
    {
      boost::filesystem::create_directories(dir_);
      if(!RIType::Open()) return false;
      if(!table_->Open()) return false;
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
    if(!RIType::Flush())
    {
      return false;
    }
    IdType min = 1;
    IdType max = RIType::VectorCount();
    
    
    std::cout<<"[TermSimilarity] total "<<max<<std::endl;
    
    for(IdType i = min;i<=max;i++)
    {
      if(i%100==0)
      {
        std::cout<<"[TermSimilarity] computing similarity for "<<i<<" , "<<time_string_()<<std::endl;
      }
      VectorType& vec = RIType::GetVector(i);
//       if(!RIType::GetVector(i, vec))
//       {
//         continue;
//       }
      std::vector<std::pair<double, IdType> > for_sort;
      for(IdType j=min;j<=max;j++)
      {
        if(i==j) continue;
        VectorType& vec2 = RIType::GetVector(j);
//         if(!RIType::GetVector(j, vec2))
//         {
//           continue;
//         }
        double sim = 0.0;
        //compute cosine similarity between two.
        sim = idmlib::sim::CosineSimilarity::Sim(vec, vec2);
//         sim = idmlib::sim::SimpleSimilarity<VectorType>::Sim(vec, vec2);
        for_sort.push_back(std::make_pair(sim, j));
      }
      std::sort(for_sort.begin(), for_sort.end(), std::greater<std::pair<double, IdType> >());
      uint32_t count = for_sort.size()>SIM_PER_TERM?SIM_PER_TERM:for_sort.size();
      std::vector<IdType> sim_vec;
      for(uint32_t c=0;c<count;c++)
      {
        if(for_sort[c].first>=SIM_MIN_SCORE)
        {
          sim_vec.push_back(for_sort[c].second);
        }
      }
      //storage the similar items
      if(!table_->Update(i, sim_vec))
      {
        std::cerr<<"sim table update on "<<i<<" failed."<<std::endl;
        return false;
      }
      {
        //debug score
//         std::cout<<"[score] "<<i<<" : "<<for_sort[0].first<<","<<for_sort[count-1].first<<std::endl;
      }
    }
    return true;
  }
  
  bool GetSimVector(IdType id, std::vector<IdType>& vec)
  {
    return table_->Get(id, vec);
  }
  
 private:
  std::string time_string_()
  {
    time_t t = time(NULL);
    char tmp[14]; 
    strftime( tmp, sizeof(tmp), "%H:%M:%S",localtime(&t) ); 
    std::string r(tmp);
    return r;
  }
 private: 
  uint16_t SIM_PER_TERM;//max similar items count per id
  double SIM_MIN_SCORE;//min score of similarity measure
  std::string dir_;
  
  TermSimilarityTable<uint32_t>* table_;
};

   
NS_IDMLIB_SIM_END



#endif 
