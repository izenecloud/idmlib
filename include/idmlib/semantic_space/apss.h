#ifndef IDMLIB_SSP_APSS_H_
#define IDMLIB_SSP_APSS_H_


#include <string>
#include <iostream>
#include <vector>
#include <boost/function.hpp>
#include <idmlib/idm_types.h>
#include <idmlib/similarity/cosine_similarity.h>
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
    
    template <typename IdType, typename KK>
    void ComputeWithStart(const std::string& file, IdType start_id, izenelib::am::SparseVector<double, KK>& vec)
    {
      typedef izenelib::am::SparseVector<double, IdType, std::vector> ColType;
      std::vector<ColType> inverted_index(dimensions_);
      std::vector<IdType> id_list;
      izenelib::am::ssf::Reader2<> normal_matrix_reader(file);
      if(!normal_matrix_reader.Open())
      {
        return;
      }
      std::cout<<"x count : "<<normal_matrix_reader.Count()<<std::endl;
      IdType x = 0;
      IdType id;
//       izenelib::am::SparseVector<double, KK> vec;
      while(normal_matrix_reader.Next(id, vec))
      {
        if(x%100==0)
        {
          std::cout<<"x:"<<x<<std::endl;
        }

        std::vector<double> score;
        if(id>=start_id)
        {
            score.resize(x, 0.0);
        }
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
          if(id>=start_id)
          {
            while(it!=it_end)
            {
                score[(*it).first] += value * (*it).second;
                ++it;
            }
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
        if(id>=start_id)
        {
            for(uint32_t s=0;s<score.size();s++)
            {
                if(score[s]>=t_)
                {
                    callback_(id_list[s], id, score[s]);
                }
            }
        }
        id_list.push_back(id);
        ++x;
      }
      normal_matrix_reader.Close();
    }
    
    template <typename IdType, typename KK>
    void ComputeMR(const std::string& file, IdType id, izenelib::am::SparseVector<double, KK>& vec, const std::string& work_dir)
    {
      boost::filesystem::create_directories(work_dir);
      
      izenelib::am::ssf::Writer<> mapper1(work_dir+"/map1");
      mapper1.Open();
      
      izenelib::am::ssf::Reader<> normal_matrix_reader(file);
      if(!normal_matrix_reader.Open())
      {
        return;
      }
      std::cout<<"x count : "<<normal_matrix_reader.Count()<<std::endl;
      std::size_t x = 0;
//       IdType id;
//       izenelib::am::SparseVector<double, KK> vec;
      while(normal_matrix_reader.Next(id, vec))
      {
        if(x%100==0)
        {
          std::cout<<"m1:"<<x<<std::endl;
        }
        for(uint32_t i=0;i<vec.value.size();i++)
        {
            mapper1.Append(vec.value[i].first, std::make_pair(id, (float)vec.value[i].second));
        }
        ++x;
      }
      normal_matrix_reader.Close();
      mapper1.Close();
      izenelib::am::ssf::Sorter<uint32_t, KK>::Sort(mapper1.GetPath());
      std::cout<<"mapper 1 finished"<<std::endl;
      
      //mapper2
      izenelib::am::ssf::Writer<> mapper2(work_dir+"/map2");
      mapper2.Open();
      
      {
        izenelib::am::ssf::Joiner<uint32_t, KK, std::pair<IdType, float> > joiner1(mapper1.GetPath());
        joiner1.Open();
        std::vector<std::pair<IdType, float> > value_list;
        
        KK col_num = 0;
        x = 0;
        while(joiner1.Next(col_num, value_list))
        {
            if(x%1==0)
            {
                std::cout<<"j1: "<<x<<std::endl;
            }
            std::cout<<"j1 size:"<<value_list.size()<<std::endl;
            for(uint32_t i=0;i<value_list.size();i++)
            {
                for(uint32_t j=i+1;j<value_list.size();j++)
                {
                    float v = value_list[i].second*value_list[j].second;
                    std::pair<IdType, IdType> key(value_list[i].first, value_list[j].first);
                    if(key.first>key.second)
                    {
                        IdType temp = key.first;
                        key.first = key.second;
                        key.second = temp;
                    }
                    mapper2.Append(key, v);
                }
            }
            
            ++x;
        }
      }
      mapper2.Close();
      izenelib::am::ssf::Sorter<uint32_t, std::pair<IdType, IdType> >::Sort(mapper2.GetPath());
      std::cout<<"mapper 2 finished"<<std::endl;
      {
        izenelib::am::ssf::Joiner<uint32_t, std::pair<IdType, IdType>, float > joiner2(mapper2.GetPath());
        joiner2.Open();
        std::vector<float> value_list;
        
        std::pair<IdType, IdType> key_pair;
        x = 0;
        while(joiner2.Next(key_pair, value_list))
        {
            if(x%100000==0)
            {
                std::cout<<"j2: "<<x<<std::endl;
            }
            float sum = 0;
//             std::cout<<"j2:"<<key_pair.first<<","<<key_pair.second<<std::endl;
            for(uint32_t i=0;i<value_list.size();i++)
            {
                sum+=value_list[i];
            }
            if(sum>=t_)
            {
              callback_(key_pair.first, key_pair.second, sum);
            }
            ++x;
        }
      }
      boost::filesystem::remove_all(work_dir);
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
