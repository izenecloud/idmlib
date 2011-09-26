///
/// @file kpe_task.h
/// @brief A single round of kpe task
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-08-17
/// @date Updated 2011-08-17
///

#ifndef IDMLIB_KPE_KPETASK_H_
#define IDMLIB_KPE_KPETASK_H_

#include "kpe_output.h"
#include "kpe_knowledge.h"
#include "kpe_algorithm_def.h"
#include "../idm_types.h"
#include <boost/type_traits.hpp>
#include "../util/FSUtil.hpp"
#include "../util/idm_term.h"
#include "../util/idm_id_manager.h"
#include <util/izene_log.h>
#include <am/sequence_file/ssfr.h>
NS_IDMLIB_KPE_BEGIN





class KpeTask
{



public:
    
    typedef izenelib::util::UString StringType;
    typedef boost::function<void (uint32_t, const StringType&, uint32_t) > KpTextCallback;
//     typedef boost::function<void (uint32_t, const std::vector<uint32_t>&) > DocKpCallback;
    typedef boost::function<void (uint32_t, const std::vector<DocKpItem>&) > DocKpCallback;
  
    KpeTask(const std::string& dir, idmlib::util::IDMAnalyzer* analyzer, idmlib::util::IDMAnalyzer* cma_analyzer, idmlib::util::IDMIdManager* id_manager, KpeKnowledge* knowledge);
    
    ~KpeTask();
    
    void SetCallback(const DocKpCallback& dk_callback);
    
    void SetNoFreqLimit();
    
    void AddManMade(const StringType& ustr);
    
    void SetTracing(const StringType& ustr);
    
    void SetMaxPhraseLen(uint8_t max_len);
    
    
    void Insert(uint32_t docid, const StringType& title, const StringType& content);
    
    bool Close();
    
    
    
    uint32_t GetDocCount() const;
    
private:
  
  
      
    bool Compute_();
    
    bool ProcessNgram_(const std::string& input_ngram_file);
    
    bool ProcessFoundKp_();
    
    bool ProcessFoundKp2_();
    
    void Init_();
    
    
    void Release_();
    
    
    
    bool AddTerms_(uint32_t docId, const std::vector<idmlib::util::IDMTerm>& termList, uint32_t& iBegin);
    
    
    void AppendNgram_(uint32_t docId, const std::vector<TermInNgram>& termList, bool bFirstTerm = true, bool bLastTerm = true);
    
    void WriteNgram_(const Ngram& ngram);
    
//     bool AppendHashItem_(hash_t hash_value);
    
//     void ReleaseCachedHashItem_();
    
    uint32_t GetTermCount_();
    
    inline hash_t Hash_(const std::vector<uint32_t>& termIdList)
    {
        return izenelib::util::HashFunction<std::vector<uint32_t> >::generateHash64BySer(termIdList);
    }
    
    static bool AstartWithB(const std::vector<uint32_t>& termIdList1, const std::vector<uint32_t>& termIdList2);
    
    bool MakeKPStr_(const std::vector<uint32_t>& termIdList, std::vector<StringType>& strList, StringType& result);
    
    bool MakeKPStr_(const std::vector<uint32_t>& termIdList, std::vector<StringType>& strList);
    
    bool MakeKPStr_(const std::vector<StringType>& strList, izenelib::util::UString& result);
    
    template <class T>
    static bool VecEqual_(const std::vector<T>& v1, const std::vector<T>& v2)
    {
      if(v1.size()!=v2.size()) return false;
      for(uint32_t i=0;i<v1.size();i++)
      {
        if(v1[i]!=v2[i]) return false;
      }
      return true;
    }
    
    template <class T>
    static bool VecStarts_(const std::vector<T>& v1, const std::vector<T>& v2)
    {
      if(v1.size()<v2.size()) return false;
      for(uint32_t i=0;i<v2.size();i++)
      {
          if(v1[i]!=v2[i]) return false;
      }
      return true;
    }
    
    uint8_t GetScore(uint32_t f, uint32_t termCount, double logL);

    void FindKP_(const std::vector<uint32_t>& terms, const std::vector<id2count_t>& docItem
              , uint8_t score ,const std::vector<id2count_t>& leftTermList
              ,const std::vector<id2count_t>& rightTermList);
    
    void OutputKP_(const KPItem& kpItem);
    
    
    
    //with complete termidlist version.
    void GetCandidate_(const std::vector<NgramInCollection>& data, izenelib::am::ssf::Writer<>* candidate_writer, izenelib::am::ssf::Writer<>* h2h_writer, izenelib::am::ssf::Writer<>* h2c_writer);
    
    
private:
  const std::string dir_;
  idmlib::util::IDMAnalyzer* analyzer_;
  idmlib::util::IDMAnalyzer* cma_analyzer_;
  idmlib::util::IDMIdManager* id_manager_;
  KpeKnowledge* knowledge_;
  KpTextCallback kp_callback_;
  DocKpCallback dk_callback_;
  uint8_t max_phrase_len_;
  izenelib::am::ssf::Writer<>* title_writer_;
  izenelib::am::ssf::Writer<>* ngram_writer_;
  izenelib::am::ssf::Writer<>* kp_writer_;
  
  
  uint32_t last_doc_id_;
  uint32_t doc_count_;
  uint32_t all_term_count_;
  
  uint32_t min_freq_threshold_;
  uint32_t min_df_threshold_;
  
  bool no_freq_limit_;
  izenelib::am::rde_hash<std::vector<uint32_t>, int> manmade_;
  std::vector<uint32_t> tracing_;
  uint32_t test_num_;
    
    
};

NS_IDMLIB_KPE_END

#endif 
