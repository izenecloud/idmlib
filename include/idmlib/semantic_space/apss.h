#ifndef IDMLIB_SSP_APSS_H_
#define IDMLIB_SSP_APSS_H_


#include <string>
#include <iostream>
#include <vector>
#include <boost/function.hpp>
#include <idmlib/idm_types.h>
#include <am/matrix/sparse_vector.h>
#include <am/sequence_file/ssfr.h>
#include <list>
NS_IDMLIB_SSP_BEGIN

template <typename K, typename D=uint16_t>
class Apss
{
  public:
    typedef boost::function<void (K, K, double) > CallbackType;
    Apss(double t, CallbackType callback, D dimensions)
    :t_(t), callback_(callback), dimensions_(dimensions)
    {
    }
    
    template <typename KK>
    void Compute(const std::vector<izenelib::am::SparseVector<double, KK> >& matrix)
    {
      std::vector<izenelib::am::SparseVector<double, K> > inverted_index(dimensions_);
      for(uint32_t x=0;x<matrix.size();x++)
      {
        if(x%100==0)
        {
          std::cout<<"x:"<<x<<std::endl;
        }
        std::vector<double> score(x, 0.0);
        const izenelib::am::SparseVector<double, KK>& vec = matrix[x];
        //compute score
        for(uint32_t m=0;m<vec.value.size();m++)
        {
          const KK& col = vec.value[m].first;
          const double& value = vec.value[m].second;
          izenelib::am::SparseVector<double, K>& index_col = inverted_index[col];
          for(K i=0;i<index_col.value.size();i++)
          {
//             std::cout<<"score index:"<<index_col.value[i].first<<std::endl;
            score[index_col.value[i].first] += value*index_col.value[i].second;
          }
        }
        //index it at inverted_index
        for(uint32_t m=0;m<vec.value.size();m++)
        {
          const KK& col = vec.value[m].first;
          const double& value = vec.value[m].second;
          inverted_index[col].value.push_back(std::make_pair(x, value));
        }
        for(uint32_t s=0;s<score.size();s++)
        {
          if(score[s]>=t_)
          {
            callback_(s, x, score[s]);
          }
        }
      }
    }
    
    template <typename IdType, typename KK>
    void Compute(const std::string& file, IdType id, izenelib::am::SparseVector<double, KK>& vec)
    {
      typedef izenelib::am::SparseVector<double, IdType, std::vector> ColType;
      std::vector<ColType> inverted_index(dimensions_);
      std::vector<IdType> id_list;
      izenelib::am::ssf::Reader<> normal_matrix_reader(file);
      if(!normal_matrix_reader.Open())
      {
        return;
      }
      std::cout<<"x count : "<<normal_matrix_reader.Count()<<std::endl;
      IdType x = 0;
//       IdType id;
//       izenelib::am::SparseVector<double, KK> vec;
      while(normal_matrix_reader.Next(id, vec))
      {
        if(x%100==0)
        {
          std::cout<<"x:"<<x<<std::endl;
        }

        std::vector<double> score(x, 0.0);
        //compute score
        for(uint32_t m=0;m<vec.value.size();m++)
        {
          const KK& col = vec.value[m].first;
          const double& value = vec.value[m].second;
          ColType& index_col = inverted_index[col];
          typename ColType::ValueType::iterator it = index_col.value.begin();
          typename ColType::ValueType::iterator it_end = index_col.value.end();
//           std::list<std::pair<K, double> >::iterator it = index_col.value.begin();
//           std::list<std::pair<K, double> >::iterator it_end = index_col.value.end();

          while(it!=it_end)
          {
            score[(*it).first] += value * (*it).second;
            ++it;
          }
          
          //index it at inverted_index
          index_col.value.push_back(std::make_pair(x, value));
        }
        //index it at inverted_index
//         for(uint32_t m=0;m<vec.value.size();m++)
//         {
//           const KK& col = vec.value[m].first;
//           const double& value = vec.value[m].second;
//           inverted_index[col].value.push_back(std::make_pair(x, value));
//         }
        for(uint32_t s=0;s<score.size();s++)
        {
          if(score[s]>=t_)
          {
            callback_(id_list[s], id, score[s]);
          }
        }
        id_list.push_back(id);
        ++x;
      }
      normal_matrix_reader.Close();
    }
    
    template <typename KK>
    void ComputeNaive(const std::vector<izenelib::am::SparseVector<double, KK> >& matrix)
    {
      for(K i = 0;i<matrix.size();i++)
      {
        if(i%100==0)
        {
          std::cout<<"ComputeNaive "<<i<<" , "<<time_string_()<<std::endl;
        }

  //       VectorType& vec = RIType::GetVector(i);
  //       VectorTraits<VectorType>::NormalizedType normal_vec;
  //       Normalizer::Normalize(vec, normal_vec);
  //       if(!RIType::GetVector(i, vec))
  //       {
  //         continue;
  //       }
        for(K j=i+1;j<matrix.size();j++)
        {
          double sim = 0.0;
          //compute cosine similarity between two.
  //         sim = idmlib::sim::CosineSimilarity::Sim(vec, vec2);
          sim = idmlib::sim::CosineSimilarity::SimNormal(matrix[i], matrix[j]);
          if(sim>=t_)
          {
            callback_(i, j, sim);
          }
        }
        
      }
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
    double t_;
    CallbackType callback_;
    D dimensions_;
};

   
NS_IDMLIB_SSP_END



#endif 
