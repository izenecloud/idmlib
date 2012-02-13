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
  :RIType(dir+"/ri", rig_dir), dir_(dir), apss_(NULL), doc_apss_(NULL)
  , output_handler_(output_handler), sim_min_score_(sim_min_score)
  {
    apss_ = new ApssType(sim_min_score_, boost::bind( &TermSimilarity::FindSim_, this, _1, _2, _3), RIType::GetDimensions() );
    doc_apss_ = new ApssType(0.8, boost::bind( &TermSimilarity::FindDocSim_, this, _1, _2, _3), RIType::GetDimensions() );
  }

  ~TermSimilarity()
  {
    delete apss_;
    delete doc_apss_;
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

  void SetDocSim(bool sim)
  {
      RIType::SetContextSim(sim);
  }

  bool DocSim() const
  {
      return RIType::ContextSim();
  }

  bool Compute()
  {
    RIType::Clear();
    if(!RIType::Flush())
    {
      return false;
    }
    std::string normal_matrix_file = dir_+"/normal_vec";
    if(true)
    {
        izenelib::am::ssf::Reader<>* vector_reader = RIType::GetVectorReader();
        if(!vector_reader->Open())
        {
            std::cerr<<"open vector reader error"<<std::endl;
            delete vector_reader;
            return false;
        }

        std::cout<<"[TermSimilarity] total count"<<vector_reader->Count()<<std::endl;
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
    }
    if(DocSim())
    {
        izenelib::am::ssf::Reader<> vector_reader(normal_matrix_file);
        if(!vector_reader.Open())
        {
            std::cerr<<"doc sim open vector reader error"<<std::endl;
            return false;
        }
        std::cout<<"[TermSimilarity] total count"<<vector_reader.Count()<<std::endl;
        //load term vector to memory
        std::vector<NormalizedType> term_vector_list(vector_reader.Count());
        std::size_t i = 0;
        IdType term_id = 0;
        while(vector_reader.Next(term_id, term_vector_list[i]))
        {
            ++i;
        }
        std::cout<<"term_vector_list load finished, size: "<<term_vector_list.size()<<std::endl;
        vector_reader.Close();

        izenelib::am::ssf::Reader<>* doc_vector_reader = RIType::GetContextVectorReader();
        if(!doc_vector_reader->Open())
        {
            std::cerr<<"open doc vector reader error"<<std::endl;
            delete doc_vector_reader;
            return false;
        }

        std::cout<<"[TermSimilarity] doc total count"<<doc_vector_reader->Count()<<std::endl;
        typedef izenelib::am::ssf::Merger<uint32_t, uint32_t, SparseType, std::vector<std::pair<ContextIdType, FreqType> > > MergerType;
//         MergerType merger(vector_reader, doc_vector_reader);
        NormalizedType mean_vector;
        {
            IdType key = 0;
//             std::vector<SparseType> value_list1;
//             std::vector<std::vector<std::pair<ContextIdType, FreqType> > > value_list2;
            std::vector<std::pair<ContextIdType, FreqType> > value;
            izenelib::am::ssf::Writer<> doc_raw_writer(dir_+"/doc_raw_writer");
            doc_raw_writer.Open();
            NormalizedType sum_vector;

            FreqType all_term_count = 0;
            while(doc_vector_reader->Next(key, value))
            {
                if(key%2000==0)
                {
                    std::cout<<"Processing term id : "<<key<<std::endl;
                }

                const NormalizedType& term_vector = term_vector_list[key-1];
                FreqType count =0;
                for(uint32_t i=0;i<value.size();i++)
                {
                    ContextIdType doc_id = value[i].first;
                    FreqType freq = value[i].second;

                    doc_raw_writer.Append(doc_id, std::make_pair(key, freq));
//                     doc_raw_writer.Append(app[i].first, std::make_pair(term_vector, app[i].second));
                    count += freq;
                }
                all_term_count += count;
                idmlib::ssp::VectorUtil::VectorAdd(sum_vector, term_vector, count);
            }
            doc_raw_writer.Close();
            mean_vector.value.resize(sum_vector.value.size());
            for(uint32_t i=0;i<sum_vector.value.size();i++)
            {
                mean_vector.value[i].first = sum_vector.value[i].first;
                mean_vector.value[i].second = sum_vector.value[i].second/all_term_count;
            }
            izenelib::am::ssf::Sorter<uint32_t, ContextIdType>::Sort(dir_+"/doc_raw_writer");
        }
        doc_vector_reader->Close();
        delete doc_vector_reader;

        std::string doc_normal_matrix_file = dir_+"/doc_normal_vec";
        boost::filesystem::remove_all(doc_normal_matrix_file);
        izenelib::am::ssf::Writer<> doc_normal_matrix_writer(doc_normal_matrix_file);
        if(!doc_normal_matrix_writer.Open())
        {
            return false;
        }

//         for(uint32_t i=0;i<doc_raw_vec.size();i++)
//         {
//             const SparseType& sum = doc_raw_vec[i].first;
//             FreqType count = doc_raw_vec[i].second;
//             NormalizedType n;
//             n.value.resize(sum.value.size());
//             for(uint32_t i=0;i<sum.value.size();i++)
//             {
//                 n.value[i].first = sum.value[i].first;
//                 n.value[i].second = (double)sum.value[i].second/count;
//             }
//             idmlib::ssp::VectorUtil::VectorAdd(n, mean_vector, -1);
//             NormalizedType normal_vec;
//             idmlib::ssp::Normalizer::Normalize(n, normal_vec);
//             ContextIdType doc_id = i+1;
//             if(!doc_normal_matrix_writer.Append(doc_id, normal_vec))
//             {
//                 delete vector_reader;
//                 delete doc_vector_reader;
//                 return false;
//             }
//         }

        izenelib::am::ssf::Joiner<uint32_t, ContextIdType, std::pair<IdType, FreqType> > joiner(dir_+"/doc_raw_writer");
        joiner.Open();
        ContextIdType key = 0;
        std::vector<std::pair<IdType, FreqType> > value_list;
        while(joiner.Next(key, value_list))
        {
            if(key%10==0)
            {
                std::cout<<"Processing docid : "<<key<<std::endl;
            }
            NormalizedType sum;
            FreqType count = 0;
            for(uint32_t i=0;i<value_list.size();i++)
            {
                IdType term_id = value_list[i].first;
                FreqType freq = value_list[i].second;
                const NormalizedType& s = term_vector_list[term_id-1];
                idmlib::ssp::VectorUtil::Push(sum, s, freq);
//                 idmlib::ssp::VectorUtil::VectorAdd(sum, s, freq);
                count += freq;
            }
            idmlib::ssp::VectorUtil::Flush(sum);
//             std::cout<<"docid "<<key<<" size : "<<value_list.size()<<","<<sum.value.size()<<std::endl;
            NormalizedType n;
            n.value.resize(sum.value.size());
            for(uint32_t i=0;i<sum.value.size();i++)
            {
                n.value[i].first = sum.value[i].first;
                n.value[i].second = sum.value[i].second/count;
            }
            idmlib::ssp::VectorUtil::VectorAdd(n, mean_vector, -1);
            NormalizedType normal_vec;
            idmlib::ssp::Normalizer::Normalize(n, normal_vec);
            if(!doc_normal_matrix_writer.Append(key, normal_vec))
            {
                return false;
            }
        }

        doc_normal_matrix_writer.Close();
        std::cout<<"[TermSimilarity] doc normalization finished."<<std::endl;

        IdType id = 0;
        NormalizedType normal_vec;

        doc_apss_->Compute(doc_normal_matrix_file, id, normal_vec);
//         doc_apss_->ComputeMR(doc_normal_matrix_file, id, normal_vec, dir_+"/tmp_mr");
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

  void FindDocSim_(IdType id1, IdType id2, double score)
  {
    std::cout<<"[DocSim] "<<id1<<","<<id2<<":"<<score<<std::endl;
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
  ApssType* doc_apss_;
  OutputHandlerType* output_handler_;
  double sim_min_score_;//min score of similarity measure

};

NS_IDMLIB_SIM_END


#endif
