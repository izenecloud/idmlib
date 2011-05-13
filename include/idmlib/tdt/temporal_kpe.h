///
/// @file temporal_kpe.h
/// @brief temporal version of kpe
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-04-14
/// @date Updated 2011-04-14
///

#ifndef _IDM_TDT_TEMPORALKPE_H_
#define _IDM_TDT_TEMPORALKPE_H_

#include "tdt_scorer.h"
#include "tdt_types.h"
#include "macd_histogram.h"
#include "../idm_types.h"
#include <boost/type_traits.hpp>
#include "../util/FSUtil.hpp"
#include "../util/idm_term.h"
#include "../util/idm_id_manager.h"
#include <util/izene_log.h>
#include <util/functional.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <am/sequence_file/ssfr.h>
#include <idmlib/similarity/term_similarity.h>

NS_IDMLIB_TDT_BEGIN



class TemporalKpe : public boost::noncopyable
{
typedef uint32_t id_type;
typedef char pos_type;
typedef izenelib::util::UString string_type;
typedef izenelib::util::UString UString;
;
typedef uint32_t DocIdType;
typedef MacdHistogram<4,8,5,3> MacdType;
typedef TDTScorer ScorerType;
typedef idmlib::sim::TermSimilarity<> TermSimilarityType;
public:
  
    
    
    TemporalKpe(const std::string& dir, idmlib::util::IDMAnalyzer* analyzer, const DateRange& date_range, TermSimilarityType* sim = NULL)
    : dir_(dir), id_dir_(dir+"/id"), tmp_dir_(dir+"/tmp"), backup_dir_(dir+"/backup")
    , output_dir_(dir+"/output")
    , analyzer_(analyzer)
    , date_range_(date_range)
    , sim_(sim)
    , id_manager_(NULL), outside_idmanager_(false)
    , max_phrase_len_(7)
    , pTermListWriter_(NULL), pHashWriter_(NULL)
    , cache_size_(0), cache_vec_(cache_size_),scorer_(NULL), outside_scorer_(false)
    , last_doc_id_(0), doc_count_(0), all_term_count_(0)
    {
      init_();
    }
    
    TemporalKpe(const std::string& dir, idmlib::util::IDMAnalyzer* analyzer, const DateRange& date_range, idmlib::util::IDMIdManager* id_manager )
    : dir_(dir), id_dir_(dir+"/id"), tmp_dir_(dir+"/tmp"), backup_dir_(dir+"/backup")
    , output_dir_(dir+"/output")
    , analyzer_(analyzer)
    , date_range_(date_range)
    , id_manager_(id_manager), outside_idmanager_(true)
    , max_phrase_len_(7)
    , pTermListWriter_(NULL), pHashWriter_(NULL)
    , cache_size_(0), cache_vec_(cache_size_),scorer_(NULL), outside_scorer_(false)
    {
      init_();
    }
        
    ~TemporalKpe()
    {
      release_();
    }
    
    void set_no_freq_limit()
    {
      no_freq_limit_ = true;
    }
    
    void add_manmade(const izenelib::util::UString& ustr)
    {
      std::vector<idmlib::util::IDMTerm> term_list;
      analyzer_->GetTgTermList(ustr, term_list );
      std::vector<uint32_t> termid_list(term_list.size());
      for(uint32_t i=0;i<term_list.size();i++)
      {
        termid_list[i] = term_list[i].id;
      }
      manmade_.insert(termid_list,0);
    }
    
    void set_tracing(const izenelib::util::UString& ustr)
    {
      std::vector<idmlib::util::IDMTerm> term_list;
      analyzer_->GetTermList(ustr, term_list );
      tracing_.resize(term_list.size());
      std::cout<<"[tracing] ";
      for(uint32_t i=0;i<term_list.size();i++)
      {
        tracing_[i] = term_list[i].id;
        std::cout<<tracing_[i]<<",";
      }
      std::cout<<std::endl;
    }
    
    void AddException(const izenelib::util::UString& ustr)
    {
        exception_.insert(ustr, 1);
    }
    
    void set_max_phrase_len(uint8_t max_len)
    {
        max_phrase_len_ = max_len;
    }
    
    void set_cache_size(uint32_t size)
    {
      cache_size_ = size;
      cache_vec_.resize(cache_size_);
      cache_vec_.resize(0);
    }
    
    bool load(const std::string& res_path)
    {
      if( scorer_ == NULL )
      {
        scorer_ = new ScorerType(analyzer_);
        bool b = scorer_->Load(res_path);
        if(!b)
        {
          std::cerr<<"[TDT] resource load failed"<<std::endl;
          delete scorer_;
          scorer_ = NULL;
          return false;
        }
        outside_scorer_ = false;
      }
      
      //start to load backup documents
      
      try
      {
          boost::filesystem::remove_all(tmp_backup_file_);
          if(boost::filesystem::exists(backup_file_))
          {
              boost::filesystem::rename(backup_file_, tmp_backup_file_);
              izenelib::am::ssf::Reader<> reader(tmp_backup_file_);
              if(reader.Open())
              {
                  BackupDocument doc;
                  while(reader.Next(doc))
                  {
                      Insert(doc.time, doc.docid, doc.title, doc.content);
                  }
              }
              reader.Close();
          }
      }
      catch(std::exception& ex)
      {
          std::cerr<<ex.what()<<std::endl;
      }
      return true;
    }
    
    void set_scorer(ScorerType* scorer)
    {
      scorer_ = scorer;
      outside_scorer_ = true;
    }
    
    
    void SetDocTime_(DocIdType docid, const TimeIdType& timeid)
    {
        for(DocIdType i=doc2timemap_.size(); i<docid-1; i++)
        {
            doc2timemap_.push_back(TimeIdType());
        }
        if(docid>doc2timemap_.size())
        {
            doc2timemap_.push_back(timeid);
        }
        else
        {
            doc2timemap_[docid-1] = timeid;
        }
    }
    
    bool GetDocTime_(DocIdType docid, TimeIdType& timeid)
    {
        if(docid>doc2timemap_.size()) return false;
        timeid = doc2timemap_[docid-1];
        if(timeid==TimeIdType()) return false;
        return true;
    }
    
//     void AddTimeCount_(TimeIdType timeid, uint64_t count)
//     {
//         for(uint32_t i=time2countmap_.size(); i<timeid-1; i++)
//         {
//             time2countmap_.push_back(0);
//         }
//         if(timeid>time2countmap_.size())
//         {
//             time2countmap_.push_back(count);
//         }
//         else
//         {
//             time2countmap_[timeid-1] += count;
//         }
//     }
//     
//     bool GetTimeCount_(TimeIdType timeid, uint64_t& count)
//     {
//         if(timeid>time2countmap_.size()) return false;
//         count = time2countmap_[timeid-1];
//         if(count==0) return false;
//         return true;
//     }
    
    inline void TryBackup_(const TimeIdType& time_id, DocIdType doc_id, const UString& title, const UString& content)
    {
        if(backup_writer_==NULL)
        {
            backup_writer_ = new izenelib::am::ssf::Writer<>(backup_file_);
            backup_writer_->Open();
        }
        if(backup_range_.Contains(time_id) )
        {
            BackupDocument doc(time_id, doc_id, title, content);
            backup_writer_->Append(doc);
        }
    }

    bool Insert(const TimeIdType& time_id, DocIdType doc_id, const UString& title, const UString& content)
    {
        if(!valid_range_.Contains(time_id)) return false;
        SetDocTime_(doc_id, time_id);
        TryBackup_(time_id, doc_id, title, content);
        {
            std::vector<idmlib::util::IDMTerm> term_list;
            analyzer_->GetTermList(title, term_list );
        
//             AddTimeCount_(time_id, term_list.size());
            if( !term_list.empty() )
            {
                uint32_t begin = 0;
                while( AppendTerms_(doc_id, term_list, begin) ) {}
            }
        }
        
//         {
//             std::vector<idmlib::util::IDMTerm> term_list;
//             analyzer_->GetTermList(content, term_list );
//         
// //             AddTimeCount_(time_id, term_list.size());
//             if( !term_list.empty() )
//             {
//                 uint32_t begin = 0;
//                 while( AppendTerms_(doc_id, term_list, begin) ) {}
//             }
//         }
        
        return true;
    }
    
    
    bool AppendTerms_(DocIdType docid, const std::vector<idmlib::util::IDMTerm>& termList, uint32_t& iBegin)
    {
//         std::cout<<docId<<"#### "<<termList.size()<<std::endl;
        if(iBegin >= termList.size()) return false;
        std::vector<std::pair<bool,uint32_t> > splitVec;
        uint32_t i=iBegin;
        uint32_t _begin = iBegin;
        uint32_t insertTermId;
        for ( ;i<termList.size();i++ )
        {
            bool bSplit = scorer_->IsSplitTerm(termList[i], insertTermId);
          
            splitVec.push_back(std::make_pair(bSplit, insertTermId));
            if( i == iBegin ) //first term
            {
                continue;
            }
            else
            {
                if(bSplit)
                {
                    break;
                }
                if( termList[i].position != termList[i-1].position+1 )
                {
                    splitVec.erase( splitVec.end()-1 );
                    break;
                }
            }
            
        }
        iBegin = i;
        if( splitVec.size() == 0 ) return false;
        bool bFirstTerm = true;
        bool bLastTerm = true;
        if( splitVec.size() == 1 ) 
        {
            if( splitVec[0].first == true )
            {
                return true;
            }
        }
        else
        {
            bFirstTerm = !splitVec.front().first;
            bLastTerm = !splitVec.back().first;
            
        }
        std::vector<uint32_t> terms( splitVec.size() );
        for(uint32_t p=_begin;p<_begin+splitVec.size();p++)
        {
            uint32_t _index = p-_begin;
            terms[_index] = splitVec[_index].second;
            id_manager_->Put(terms[_index], termList[p].text);
//             if( !splitVec[_index].first )
//             {
//                 
//             }
        }
        AppendNgram_(docid, terms, bFirstTerm, bLastTerm);
        return true;
    }
    
    
    void AppendNgram_(DocIdType docid, const std::vector<uint32_t>& termList, bool bFirstTerm = true, bool bLastTerm = true)
    {
//         std::cout<<docId<<"### "<<termList.size()<<" "<<(int)bFirstTerm<<" "<<(int)bLastTerm<<std::endl;
        if( termList.size() == 0 ) return;
        if( termList.size() == 1 ) 
        {
            if( bFirstTerm && bLastTerm )
            {
                std::vector<uint32_t> inputItem(termList);
                inputItem.push_back(0);
                inputItem.push_back(docid);
                if( scorer_->PrefixTest(termList) != KPStatus::RETURN)
                {
                  appendTermList_(inputItem);
                }
                
                appendHashItem_(hash_(termList));
                incTermCount_(1);
            }
            return;
        }
        if( termList.size() == 2 && !bFirstTerm && !bLastTerm ) 
        {
            return;
        }
        uint32_t start = 0;
        uint32_t end = termList.size();
        uint32_t increase = termList.size();
        if(!bFirstTerm)
        {
            increase--;
            start = 1;
        }
        if(!bLastTerm)
        {
            increase--;
            end -= 1;
        }
        
        for(uint32_t i=start; i<end; i++)
        {
            uint32_t len = std::min(end-i,(uint32_t)(max_phrase_len_+1));
            std::vector<uint32_t> frag( termList.begin()+i, termList.begin()+i+len );
            bool valid_frag =true;
            if( scorer_->PrefixTest(frag) != KPStatus::RETURN)
            {
              frag.push_back(0);
              frag.push_back(docid);
              if( i!= 0 )
              {
                  frag.push_back(termList[i-1]);
              }
              appendTermList_(frag);
//               if(docId==3)
//               {
//                 std::cout<<"{{3}}";
//                 for(uint32_t dd=0;dd<frag.size();dd++)
//                 {
//                   std::cout<<frag[dd]<<",";
//                 }
//                 std::cout<<std::endl;
//               }
            }
            else
            {
              valid_frag = false;
            }
            if(!tracing_.empty())
            {
              if(vec_starts_(frag, tracing_))
              {
                std::cout<<"[tracing] ["<<(int)valid_frag<<"] ";
                for(uint32_t dd=0;dd<frag.size();dd++)
                {
                  std::cout<<frag[dd]<<",";
                }
                std::cout<<std::endl;
              }
            }
            for(uint32_t j= i+1; j<= end; j++)
            {
                if( j-i >= max_phrase_len_ ) continue;
                std::vector<uint32_t> ifrag( termList.begin()+i, termList.begin()+j );
                appendHashItem_(hash_(ifrag));
            }
            
        }

        
        incTermCount_(increase);
    }
    
    
    void close()
    {
      
      compute_();
//       kp_construct_();
    }
    
    
    
    uint32_t getDocCount()
    {
        return doc_count_;
    }
    
private:
  
  
  
    void try_compute_()
    {
      if( try_compute_num_>0 && pHashWriter_->getItemCount() >= try_compute_num_ )
      {
        compute_();
        reinit_();
      }
    }
  
    void reinit_()
    {
      scorer_->Flush();
      last_doc_id_ = 0;
      doc_count_ = 0;
      all_term_count_ = 0;
      boost::filesystem::remove_all(tmp_dir_);
      boost::filesystem::create_directories(tmp_dir_);
      if( pTermListWriter_ == NULL )
      {
        pTermListWriter_ = new TermListWriter(tmp_dir_+"/pInputItemWriter");
        pTermListWriter_->open();
      }
      if( pHashWriter_ == NULL)
      {
        pHashWriter_ = new HashWriter(tmp_dir_+"/pHashItemWriter");
        pHashWriter_->open();
      }
    }
  
    void compute_()
    {
      releaseCachedHashItem_();
      uint64_t hash_total_count = pHashWriter_->getItemCount();
      if(  hash_total_count==0 )
      {
        return;
      }
      pTermListWriter_->close();
      pHashWriter_->close();
      std::string inputItemPath = pTermListWriter_->getPath();
      std::string hashItemPath = pHashWriter_->getPath();
      delete pTermListWriter_;
      pTermListWriter_ = NULL;
      delete pHashWriter_;
      pHashWriter_ = NULL;
      
      //set thresholds
      uint32_t docCount = getDocCount_();
      std::cout<<"[KPE] running for "<<docCount<<" docs, hash total count: "<<hash_total_count<<std::endl;
      min_freq_threshold_ = 1;
      min_df_threshold_ = 1;
      if(!no_freq_limit_)
      {
        if( docCount >=10 )
        {
            min_freq_threshold_ = (uint32_t)std::floor( std::log( (double)docCount )/2 );
            if( min_freq_threshold_ < 3 ) min_freq_threshold_ = 3;
            
            min_df_threshold_ = (uint32_t)std::floor( std::log( (double)docCount )/4 );
            if( min_df_threshold_ < 2 ) min_df_threshold_ = 2;
        }
      }
      uint64_t p = 0;
      HashSSFType::SorterType hsorter;
      hsorter.sort(hashItemPath);
      MEMLOG("[KPE1] finished");
      typedef izenelib::am::SSFType<hash_t, uint32_t, uint8_t> HashItemCountSSFType;
      HashItemCountSSFType::WriterType hcwriter(tmp_dir_+"/HCWRITER");
      hcwriter.open();
      {
          HashSSFType::ReaderType hreader(hashItemPath);
          hreader.open();
          hash_t hashId;
          hash_t saveId;
          uint32_t count = 0;
          uint32_t icount = 1;
          LOG_BEGIN("Hash", &hreader);
          while( true )
          {
            bool b = true;
            if( cache_size_>0)
            {
              b = hreader.next(hashId, icount);
            }
            else
            {
              b = hreader.nextKey(hashId);
            }
            if( !b ) break;
            if(p==0)
            {
                saveId = hashId;
            }
            else
            {
                if( saveId != hashId )
                {
                    hcwriter.append(saveId, count);
                    saveId = hashId;
                    count = 0;
                }
            }
            count += icount;
            p++;
            LOG_PRINT("Hash", 1000000);
          }
          LOG_END();
          hcwriter.append(saveId, count);
          hreader.close();
          idmlib::util::FSUtil::del(hashItemPath);
          
      }
      hcwriter.close();
      MEMLOG("[KPE2] finished");
//       if(!tracing_.empty())
//       {
//         typename TermListSSFType::ReaderType fireader(inputItemPath);
//         fireader.open();
//         std::vector<uint32_t> keyList;
//         while( fireader.nextKeyList(keyList) )
//         {
//           if(vec_starts_(keyList, tracing_))
//           {
//             std::cout<<"[tracing] [before-suffix] ";
//             for(uint32_t dd=0;dd<keyList.size();dd++)
//             {
//               std::cout<<keyList[dd]<<",";
//             }
//             std::cout<<std::endl;
//           }
//         }
//         fireader.close();
//       }
      TermListSSFType::SorterType fisorter;
      fisorter.sort(inputItemPath);
//       if(!tracing_.empty())
//       {
//         typename TermListSSFType::ReaderType fireader(inputItemPath);
//         fireader.open();
//         std::vector<uint32_t> keyList;
//         while( fireader.nextKeyList(keyList) )
//         {
//           if(vec_starts_(keyList, tracing_))
//           {
//             std::cout<<"[tracing] [after-suffix] ";
//             for(uint32_t dd=0;dd<keyList.size();dd++)
//             {
//               std::cout<<keyList[dd]<<",";
//             }
//             std::cout<<std::endl;
//           }
//         }
//         fireader.close();
//       }
      MEMLOG("[KPE3] finished");
      typedef izenelib::am::SSFType<uint32_t, uint32_t, uint32_t> SortedFragmentItemSSFType;
      SortedFragmentItemSSFType::WriterType swriter(tmp_dir_+"/SWRITER");
      swriter.open();
      {
          TermListSSFType::ReaderType fireader(inputItemPath);
          fireader.open();
          std::vector<uint32_t> keyList;

          LOG_BEGIN("Suffix", &fireader);
          SortedFragmentItem sItem;
          std::vector<uint32_t> lastTermIdList;
          while( fireader.nextKeyList(keyList) )
          {
              if(!tracing_.empty())
              {
                  if(vec_starts_(keyList, tracing_))
                  {
                      std::cout<<"[tracing] [suffix] ";
                      for(uint32_t dd=0;dd<keyList.size();dd++)
                      {
                      std::cout<<keyList[dd]<<",";
                      }
                      std::cout<<std::endl;
                  }
              }
              uint32_t zeroPos = 0;
              for(uint32_t i=0;i<keyList.size();i++)
              {
                  if( keyList[i] == 0 )
                  {
                      zeroPos = i;
                      break;
                  }
              }
              std::vector<uint32_t> termIdList(keyList.begin(), keyList.begin()+zeroPos);
              uint32_t docId = 0;
              uint32_t prefixTermId = 0;
              bool hasPrefix = false;
              if( zeroPos == keyList.size()-2 )
              {
                  docId = keyList[keyList.size()-1];
              }
              else if( zeroPos == keyList.size()-3 )
              {
                  docId = keyList[keyList.size()-2];
                  prefixTermId = keyList[keyList.size()-1];
                  hasPrefix = true;
              }
              else
              {
                  //impossible, just a reminder.
              }
              if( p == 0 )
              {
                  SortedFragmentItem tmp(0, termIdList);
                  std::swap(sItem, tmp);
              }
              else
              {
                  uint32_t inc;
                  sItem.getIncrement(termIdList, inc);
                  if( inc != termIdList.size() )//the same termidlist
                  {
                      std::vector<uint32_t> value;
                      sItem.getValue(value);
                      swriter.append( value );
                      
                      std::vector<uint32_t> incTermIdList(sItem.termIdList_.begin(),
                      sItem.termIdList_.begin()+inc);
                      incTermIdList.insert(incTermIdList.end(), 
                      termIdList.begin()+inc, termIdList.end());
                      SortedFragmentItem tmp(inc, incTermIdList);
                      std::swap(sItem, tmp);
                  }
              }
              sItem.addDocId(docId, 1);
              if(hasPrefix)
                  sItem.addPrefixTerm(prefixTermId);
              
              p++;
              LOG_PRINT("Suffix", 1000000);
          }
          LOG_END();
          std::vector<uint32_t> value;
          sItem.getValue(value);
          swriter.append(value);
          fireader.close();
          MEMLOG("Total output: %lu",swriter.getItemCount() );
          idmlib::util::FSUtil::del(inputItemPath);
      }
      swriter.close();
      MEMLOG("[KPE4] finished.");
      SortedFragmentItemSSFType::ReaderType reader(swriter.getPath());
      reader.open();
      std::vector<uint32_t> keyList;
      CandidateSSFType::WriterType hclWriter(tmp_dir_+"/HCLWRITER");
      hclWriter.open();
      Hash2HashSSFType::WriterType h2hWriter(tmp_dir_+"/H2HWRITER");
      h2hWriter.open();
      std::vector<uint32_t> termIdList(0);
      std::vector<uint32_t> docIdList(0);

      LOG_BEGIN("AAA", &reader);
      
      std::vector<Data > data;
      double aaa_time = 0.0;
      izenelib::util::ClockTimer aaa_clocker;
      
      std::vector<uint32_t> completeTermIdList;
      while( reader.nextKeyList(keyList) )
      {
          Data this_data;
          this_data.inc = SortedFragmentItem::parseInc(keyList);
          this_data.termid_list = SortedFragmentItem::parseTermIdList(keyList);
          this_data.docitem_list = SortedFragmentItem::parseDocItemList(keyList);
          this_data.freq = SortedFragmentItem::parseFreq(keyList);
          this_data.lc_list = SortedFragmentItem::parsePrefixTermList(keyList);
          
          completeTermIdList.resize(this_data.inc);
          completeTermIdList.insert( completeTermIdList.end(), this_data.termid_list.begin(), this_data.termid_list.end() );
          this_data.termid_list = completeTermIdList;
          if( this_data.inc==0 )
          {
              aaa_clocker.restart();
              getCandidateLabel2_(data, &hclWriter, &h2hWriter);
              aaa_time += aaa_clocker.elapsed();
              data.resize(0);
          }
          if(!tracing_.empty())
          {
            if(vec_starts_(this_data.termid_list, tracing_))
            {
              std::cout<<"[tracing] [data] ";
              for(uint32_t dd=0;dd<this_data.docitem_list.size();dd++)
              {
                std::cout<<"("<<this_data.docitem_list[dd].first<<"|"<<this_data.docitem_list[dd].second<<"),";
              }
              std::cout<<std::endl;
            }
          }
          data.push_back(this_data);
          p++;
          LOG_PRINT("AAA", 100000);
              
      }
      aaa_clocker.restart();
      getCandidateLabel2_(data, &hclWriter, &h2hWriter);
      aaa_time += aaa_clocker.elapsed();
      LOG_END();
      hclWriter.close();
      h2hWriter.close();
      MEMLOG("[KPE4] finished.");
      MEMLOG("[KPE4-AAA] %f seconds.", aaa_time);
      
      {
          CandidateSSFType::SorterType sorter;
          sorter.sort(hclWriter.getPath());
      }
      {
          Hash2HashSSFType::SorterType sorter;
          sorter.sort(h2hWriter.getPath());
      }
      HashCountMerger hcMerger;
      hcMerger.setObj(hcwriter.getPath(), h2hWriter.getPath() );
      hash_t key = 0;
      std::vector<uint32_t> valueList1;
      std::vector<Hash2HashItem> valueList2;
      Hash2CountSSFType::WriterType h2cWriter( tmp_dir_+"/H2CWRITER" );
      h2cWriter.open();
      while( hcMerger.next( key, valueList1, valueList2) )
      {
          if( valueList1.size() == 0 )
          {
              //impossible, just a reminder.
              continue;
          }
          if( valueList1.size() > 1 )
          {
              //impossible, just a reminder.
              continue;
          }
          for(uint32_t i=0;i<valueList2.size();i++)
          {
              h2cWriter.append( valueList2[i].first, Hash2CountItem(valueList1[0],valueList2[i].second) );
              
          }
      }
      h2cWriter.close();
      {
          Hash2CountSSFType::SorterType sorter;
          sorter.sort( h2cWriter.getPath() );
      }
//         sleep(1000);
      HashCountListSSFType::WriterType hcltWriter( tmp_dir_+"/HCLTWRITER" );
      hcltWriter.open();
      {
          Hash2CountSSFType::ReaderType h2cReader(h2cWriter.getPath());
          h2cReader.open();
          Hash2CountItem hash2CountValue;
          LOG_BEGIN("HashCount", &h2cReader);
          HashCountListItem output(0, 0, 0, 0);
          hash_t key;
          hash_t saveKey;
          while( h2cReader.next(key, hash2CountValue) )
          {
              if(p==0)
              {
                  saveKey = key;
              }
              else
              {
                  if(saveKey != key )
                  {
                      hcltWriter.append(saveKey, output);
                      output = boost::make_tuple(0,0,0,0);
                      saveKey = key;
                  }
              }
              if( hash2CountValue.second == 1 )
              {
                  boost::get<0>(output) = hash2CountValue.first;
              }
              else if( hash2CountValue.second == 2 )
              {
                  boost::get<1>(output) = hash2CountValue.first;
              }
              else if( hash2CountValue.second == 3 )
              {
                  boost::get<2>(output) = hash2CountValue.first;
              }
              else if( hash2CountValue.second == 4 )
              {
                  boost::get<3>(output) = hash2CountValue.first;
              }
              else
              {
                  //impossible, just a reminder.
              }
              p++;
              LOG_PRINT("HashCount", 100000);
          }
          hcltWriter.append(saveKey, output);
          LOG_END();
          h2cReader.close();
          idmlib::util::FSUtil::del(h2cReader.getPath());
      }
      hcltWriter.close();
//             typename HashCandidateLabelItemSSFType::ReaderType tmpReader(hclWriter.getPath());
//             tmpReader.open();
//             while( tmpReader.next() )
//             {
//                 hash_t key;
//                 bool b = tmpReader.getCurrentKey(key);
//                 std::cout<<"TMP :: KEY :: "<<b<<"|||"<<key<<std::endl;
//             }
//             tmpReader.close();

       
      {
          double min_logl = 10.0;
          double min_mi = 5.0;
          merger_t finalMerger;
          finalMerger.setObj( hclWriter.getPath(), hcltWriter.getPath() );
          hash_t key = 0;
          std::vector<CandidateItem> valueList1;
          std::vector<HashCountListItem> valueList2;
          uint32_t n = getTermCount_();
          while( finalMerger.next(key, valueList1, valueList2) )
          {
              if( valueList1.size()==0 )
              {
                  //impossible, just a reminder.
                  continue;
              }
              if( valueList1.size()>1 )
              {
                  //we may face to a hash conflict, ignore them
                  continue;
              }
              
              if( valueList2.size()!=1 )
              {
                  //impossible, just a reminder.
                  continue;
              }
              CandidateItem& hlItem = valueList1[0];
              HashCountListItem& hclItem = valueList2[0];
              std::vector<uint32_t> termIdList = hlItem.termid_list;
              std::vector<id2count_t> docItem = hlItem.docitem_list;
              uint32_t f = hlItem.freq;
              std::vector<id2count_t> leftTermList = hlItem.lc_list;
              std::vector<id2count_t> rightTermList = hlItem.rc_list;
              if( termIdList.size()==1 )
              {
                insertKP_( termIdList, docItem, getScore(f, termIdList.size(),0.0) , leftTermList,rightTermList);
              }
              else if(termIdList.size()==2 )
              {
                  uint32_t f1 = boost::get<0>(hclItem);
                  uint32_t f2 = boost::get<1>(hclItem);
                  if( f1==0 || f2==0 )
                  {
                      //impossible, just a reminder.
                      continue;
                  }
                  double logL = StatisticalScorer::logL(f,f1,f2,n);
                  double mi = StatisticalScorer::mi(f,f1,f2,n);
//                     std::cout<<"LogL: "<<logL<<" , MI: "<<mi<<std::endl;
                  if( logL>=min_logl && mi>=min_mi )
                  {
                      insertKP_( termIdList, docItem, getScore(f, termIdList.size(),logL) , leftTermList,rightTermList);
                  }
              }
              else if(termIdList.size()>2)
              {
                  uint32_t f1 = boost::get<0>(hclItem);
                  uint32_t f2 = boost::get<1>(hclItem);
                  uint32_t f3 = boost::get<2>(hclItem);
                  uint32_t f4 = boost::get<3>(hclItem);
                  if( f1==0 || f2==0 || f3==0 || f4==0 )
                  {
                      //impossible, just a reminder.
                      continue;
                  }
                  double logL = StatisticalScorer::logL(f,f1,f2,n);
                  if( logL>=min_logl && StatisticalScorer::logL(f,f1,f4,n)>=min_logl 
                      && StatisticalScorer::logL(f,f3,f2,n)>=min_logl )
                  {
                      insertKP_( termIdList, docItem, getScore(f, termIdList.size(),logL) , leftTermList,rightTermList);
                  }
              }
          }
      }
      //post-process
      {
          //topic similarity
          if(!sim_->Compute())
          {
              std::cerr<<"sim Compute error"<<std::endl;
          }
          else
          {
              
              std::cout<<"[sim] output : "<<std::endl;
              TermSimilarityType::SimTableType* table = sim_->GetSimTable();
              for(uint32_t i=0;i<valid_topics_.size();i++)
              {
                  uint32_t topic_id = i+1;
                  const izenelib::util::UString& text = valid_topics_[i];
                  std::string str;
                  text.convertString(str, izenelib::util::UString::UTF_8);
                  std::string topic_output_file = output_dir_+"/"+str+".json";
                  std::ofstream topic_ofs(topic_output_file.c_str());
                  topic_ofs<<"{"<<std::endl;
                  topic_ofs<<"\t\"topic\":"<<std::endl;
                  topic_ofs<<"\t{"<<std::endl;
                  topic_ofs<<"\t\t\"text\" : \""<<str<<"\","<<std::endl;
                  topic_ofs<<"\t\t\"ts\" : "<<std::endl;
                  topic_ofs<<"\t\t["<<std::endl;
                  const std::vector<uint32_t>& freq = freq_[i];
                  
                  for(uint32_t j=0;j<freq.size();j++)
                  {
                      boost::gregorian::date_duration d(j);
                      boost::gregorian::date date = valid_range_.start + d;
                      std::string date_str = boost::gregorian::to_iso_extended_string(date);
                      topic_ofs<<"\t\t\t{"<<std::endl;
                      topic_ofs<<"\t\t\t\t\"date\" : \""<<date_str<<"\","<<std::endl;
                      topic_ofs<<"\t\t\t\t\"freq\" : "<<freq[j]<<std::endl;
                      topic_ofs<<"\t\t\t}";
                      if(j!=freq.size()-1)
                      {
                          topic_ofs<<",";
                      }
                      topic_ofs<<std::endl;
                  }
                  topic_ofs<<"\t\t],"<<std::endl;
                  topic_ofs<<"\t\t\"related\" : "<<std::endl;
                  topic_ofs<<"\t\t["<<std::endl;
                  std::vector<uint32_t> sim_id_list;
                  if(table->Get(topic_id, sim_id_list))
                  {
                      if(sim_id_list.size()>0)
                      {
                        
                        std::cout<<str<<",";
                        for(uint32_t j=0;j<sim_id_list.size();j++)
                        {
                            const izenelib::util::UString& sim_text = valid_topics_[sim_id_list[j]-1];
                            sim_text.convertString(str, izenelib::util::UString::UTF_8);
                            std::cout<<str<<",";
                            topic_ofs<<"\t\t\t{"<<std::endl;
                            topic_ofs<<"\t\t\t\t\"text\" : \""<<str<<"\","<<std::endl;
                            topic_ofs<<"\t\t\t}";
                            if(j!=sim_id_list.size()-1)
                            {
                                topic_ofs<<",";
                            }
                            topic_ofs<<std::endl;
                        }
                        std::cout<<std::endl;
                      }
                  }
                  topic_ofs<<"\t\t]"<<std::endl;
                  topic_ofs<<"\t}"<<std::endl;
                  topic_ofs<<"}"<<std::endl;
                  topic_ofs.close();
              }
          }
          
          //output cloud file
          std::string cloud_file = output_dir_+"/cloud.json";
          std::ofstream cloud_ofs(cloud_file.c_str());
          cloud_ofs<<"{\n\t\"cloud\":\n\t[\n";
          boost::gregorian::date_duration d = valid_range_.end - valid_range_.start;
          uint32_t days = d.days();
          for(uint32_t day=0;day<days;day++)
          {
              boost::gregorian::date_duration dd(day);
              boost::gregorian::date date = valid_range_.start+dd;
              std::vector<std::pair<UString, double> >* pburst_item = burst_items_.find(date);
              if(pburst_item==NULL) continue;
              std::vector<std::pair<UString, double> >& burst_item = *pburst_item;
              typedef izenelib::util::second_greater<std::pair<UString, double> > greater_than;
              std::sort(burst_item.begin(), burst_item.end(), greater_than());
              std::string date_str = boost::gregorian::to_iso_extended_string(date);
              cloud_ofs<<"\t\t{\n\t\t\t\"date\":\""<<date_str<<"\",\n\t\t\t\"burst\":\n\t\t\t[\n";
              for(uint32_t i=0;i<burst_item.size();i++)
              {
                  std::string str;
                  burst_item[i].first.convertString(str, izenelib::util::UString::UTF_8);
                  cloud_ofs<<"\t\t\t\t{\n\t\t\t\t\t\"text\":\""<<str<<"\",\n\t\t\t\t\t\"weight\":"<<burst_item[i].second<<"\n\t\t\t\t}";
                  if(i!=burst_item.size()-1)
                  {
                      cloud_ofs<<",";
                  }
                  cloud_ofs<<std::endl;
              }
              cloud_ofs<<"\t\t\t]\n\t\t}";
              if(day!=days-1)
              {
                  cloud_ofs<<",";
              }
              cloud_ofs<<std::endl;
          }
          cloud_ofs<<"\t]\n}\n";
          cloud_ofs.close();
      }
    }
    
    void init_()
    {
        try_compute_num_ = 400000000;
        test_num_ = 0;
        no_freq_limit_ = false;
        last_doc_id_ = 0;
        doc_count_ = 0;
        all_term_count_ = 0;
        backup_range_.end = date_range_.end;
        boost::gregorian::date_duration d(MacdType::G_Value);
        backup_range_.start = backup_range_.end-d;
        
        valid_range_.end = date_range_.end;
        valid_range_.start = date_range_.start-d;
        
        duration_ = (valid_range_.end - valid_range_.start).days()+1;
        backup_writer_ = NULL;
        boost::filesystem::create_directories(dir_);
        boost::filesystem::remove_all(tmp_dir_);
        boost::filesystem::create_directories(tmp_dir_);
        boost::filesystem::create_directories(id_dir_);
        boost::filesystem::create_directories(backup_dir_);
        boost::filesystem::create_directories(output_dir_);
        std::string run_plot_file = output_dir_+"/run.plot";
        plot_writer_.open(run_plot_file.c_str());
        
        
        plot_writer_<<"set xdata time"<<std::endl;
        plot_writer_<<"set timefmt \"%Y-%m-%d\""<<std::endl;
        plot_writer_<<"set format x \"%b %d\""<<std::endl;
        plot_writer_<<"set terminal png"<<std::endl;
        plot_writer_<<"set term png font \"/usr/share/fonts/truetype/simsun.ttf,12\""<<std::endl;
        plot_writer_<<"set style fill solid 1.0"<<std::endl;
        plot_writer_<<"set boxwidth 0.2 absolute"<<std::endl;
        backup_file_ = backup_dir_ + "/backup_doc";
        tmp_backup_file_ = backup_dir_ + "/tmp_backup_doc";
        if( pTermListWriter_ == NULL )
        {
            pTermListWriter_ = new TermListWriter(tmp_dir_+"/pInputItemWriter");
            pTermListWriter_->open();
        }
        if( pHashWriter_ == NULL)
        {
            pHashWriter_ = new HashWriter(tmp_dir_+"/pHashItemWriter");
            pHashWriter_->open();
        }

        //init idmanager
        if( id_manager_ == NULL )
        {
            id_manager_ = new idmlib::util::IDMIdManager(id_dir_);
        }
        
        if(!sim_->Open()) 
        {
            std::cerr<<"sim open error"<<std::endl;
            return ;
        }
        if(!sim_->SetContextMax(1000000))
        {
            std::cerr<<"sim SetContextMax error"<<std::endl;
            return;
        }
//       kp_construct_();
    }
    
    
    void release_()
    {
        plot_writer_.close();
        if( pTermListWriter_!= NULL)
        {
            delete pTermListWriter_;
            pTermListWriter_ = NULL;
        }
        if( pHashWriter_!= NULL)
        {
            delete pHashWriter_;
            pHashWriter_ = NULL;
        }
        if( backup_writer_!= NULL)
        {
          delete backup_writer_;
          backup_writer_ = NULL;
        }
        if( id_manager_ != NULL && !outside_idmanager_)
        {
          id_manager_->Flush();
          delete id_manager_;
          id_manager_ = NULL;
        }
        if( !outside_scorer_ && scorer_ != NULL)
        {
            delete scorer_;
            scorer_ = NULL;
        }
        boost::filesystem::remove_all(tmp_dir_);
        
    }
    
    
    
    
    
    inline void appendTermList_(const std::vector<uint32_t>& inputItem)
    {
      pTermListWriter_->append(inputItem);
    }
    
    void appendHashItem_(hash_t hash_value)
    {
      if( cache_size_ > 0 )
      {
        if( cache_vec_.size() >= cache_size_ )
        {
//           std::cout<<"[FULL]"<<std::endl;
          //output
          for(uint32_t i=0;i<cache_vec_.size();i++)
          {
            pHashWriter_->append(cache_vec_[i].first, cache_vec_[i].second);
          }
          //clean
          cache_vec_.resize(0);
          cache_map_.clear();
        }
        uint32_t* index=  cache_map_.find(hash_value);
        if( index == NULL)
        {
          cache_vec_.push_back( std::make_pair(hash_value, 1) );
          cache_map_.insert( hash_value, (uint32_t)(cache_vec_.size())-1 );
        }
        else
        {
          cache_vec_[*index].second += 1;
        }
      }
      else
      {
        pHashWriter_->append(hash_value);
      }
      
    }
    
    void releaseCachedHashItem_()
    {
      if( cache_size_ > 0 )
      {
        for(uint32_t i=0;i<cache_vec_.size();i++)
        {
          pHashWriter_->append(cache_vec_[i].first, cache_vec_[i].second);
        }
        //clean
        cache_vec_.resize(0);
        cache_map_.clear();
      }
    }
    
    void setDocCount_(uint32_t count)
    {
        doc_count_ = count;
    }
    
    uint32_t getDocCount_()
    {
        return doc_count_;
    }
    
    
    
    void incTermCount_(uint32_t inc = 1)
    {
       all_term_count_ += inc;
    }
    
    uint32_t getTermCount_()
    {
        return all_term_count_;
    }
    
    inline hash_t hash_(const std::vector<uint32_t>& termIdList)
    {
        return izenelib::util::HashFunction<std::vector<uint32_t> >::generateHash64BySer(termIdList);
    }
    
    inline const bool AstartWithB(const std::vector<uint32_t>& termIdList1, const std::vector<uint32_t>& termIdList2)
    {
        if(termIdList1.size()<termIdList2.size()) return false;
        for(uint32_t i=0;i<termIdList2.size();i++)
        {
            if( termIdList1[i] != termIdList2[i] ) return false;
        }
        return true;
    }
    
    bool makeKPStr_(const std::vector<uint32_t>& termIdList,
        std::vector<izenelib::util::UString>& strList,
        izenelib::util::UString& result)
    {
        bool b = makeKPStr_(termIdList, strList);
        if(!b) return false;
        b = makeKPStr_(strList, result);
        if(!b) return false;
        return true;
    }
    
    bool makeKPStr_(const std::vector<uint32_t>& termIdList, std::vector<izenelib::util::UString>& strList)
    {
        if ( termIdList.empty() ) return false;//if empty
        strList.resize(termIdList.size());
        izenelib::util::UString ustr;
        bool bb = true;
        bb = id_manager_->GetStringById(termIdList[0], strList[0]);
        if(!bb)
        {
            std::cout<<"!!!!can not find term id "<<termIdList[0]<<std::endl;
            return false;
        }
        for(std::size_t i=1;i<termIdList.size();i++)
        {
            bb = id_manager_->GetStringById(termIdList[i], strList[i]);
            if(!bb)
            {
                return false;
            }
        }
        return true;
    }
        
    bool makeKPStr_(const std::vector<izenelib::util::UString>& strList, izenelib::util::UString& result)
    {
        result.clear();
        if(strList.size()>0)
        {
            if( strList[0].length() == 0 ) return false;
            result.append(strList[0]);
            for(std::size_t i=1;i<strList.size();i++)
            {
                if( strList[i].length() == 0 ) return false;
                if( result.isChineseChar( result.length()-1 ) 
                    && strList[i].isChineseChar(0) )
                {
                }
                else
                {
                    result.append(izenelib::util::UString(" ", izenelib::util::UString::UTF_8));
                }
                result.append(strList[i]);
            }
        }
        return true;
    }
    
    template <class T>
    bool vec_equal_(const std::vector<T>& v1, const std::vector<T>& v2)
    {
      if(v1.size()!=v2.size()) return false;
      for(uint32_t i=0;i<v1.size();i++)
      {
        if(v1[i]!=v2[i]) return false;
      }
      return true;
    }
    
    template <class T>
    bool vec_starts_(const std::vector<T>& v1, const std::vector<T>& v2)
    {
      if(v1.size()<v2.size()) return false;
      for(uint32_t i=0;i<v2.size();i++)
      {
        if(v1[i]!=v2[i]) return false;
      }
      return true;
    }
    
    uint8_t getScore(uint32_t f, uint32_t termCount, double logL)
    {
        double score=sqrt(f> 200 ? 100 : f/2)*(termCount > 4 ? 4: termCount);
        if(logL>=10.0)
        {
            score *= sqrt(logL> 100 ? 100 : logL);
        }
        else
        {
            score *= 5;
        }
        return (uint8_t)(ceil(score)+1);
    }
    
//     void NormalizeCount_(const std::vector<id2count_t>& time_item, std::vector<std::pair<TimeIdType, double> >& n_time_item)
//     {
//         for(uint32_t i=0;i<time_item.size();i++)
//         {
//             uint64_t count = 0;
//             if(GetTimeCount_(time_item[i].first, count))
//             {
//                 double d = (double)time_item[i].second/count;
//                 n_time_item.push_back(std::make_pair(time_item[i].first, d));
//             }
//         }
//     }
    
    template <typename T>
    void ToTimeSeries_(const std::vector<std::pair<TimeIdType, T> >& time_item, std::vector<T>& time_series)
    {
//         TimeIdType last_time = timeitem[0].first;
//         time_series.push_back(timeitem[0].second);
        time_series.resize(duration_, 0);
//         std::cout<<"duration : "<<duration_<<std::endl;
        for(uint32_t i=0;i<time_item.size();i++)
        {
            TimeIdType time = time_item[i].first;
            boost::gregorian::date_duration d = time-valid_range_.start;
            int64_t diff = d.days();
            if(diff<0) continue;
            time_series[diff] = time_item[i].second;
//             std::cout<<"diff : "<<diff<<","<<time_item[i].second<<std::endl;
        }
    }
    
    bool BurstDetect_(const UString& text, const std::vector<uint32_t>& termIdList, const std::vector<id2count_t>& docItem, TrackResult& track_result)
    {
        if(!tracing_.empty())
        {
            if(vec_equal_(termIdList, tracing_))
            {
                uint64_t total = 0;
                for(uint32_t dd=0;dd<docItem.size();dd++)
                {
                    std::cout<<"[tracing] doc item: "<<docItem[dd].first<<","<<docItem[dd].second<<"]"<<std::endl;
                    total += docItem[dd].second;
                }
                std::cout<<"[tracing] total:"<<total<<std::endl;
            }
        }
        std::vector<std::pair<TimeIdType, uint32_t> > time_item;
        for(uint32_t i=0;i<docItem.size();i++)
        {
            TimeIdType time;
            if(GetDocTime_(docItem[i].first, time))
            {
                time_item.push_back(std::make_pair(time, docItem[i].second));
            }
        }
        idmlib::util::accumulateList(time_item);
//         std::cout<<"bbb"<<std::endl;
//         std::vector<std::pair<TimeIdType, double> > n_time_item;
//         NormalizeCount_(time_item, n_time_item);
//         std::cout<<"[timeitem] ";
//         for(uint32_t i=0;i<timeitem.size();i++)
//         {
//             std::cout<<"("<<timeitem[i].first<<","<<timeitem[i].second<<"), ";
//             total += timeitem[i].second;
//         }
//         std::cout<<std::endl;
//         std::cout<<"[timeitem-total] "<<total<<std::endl;
        std::vector<uint32_t> time_series;
        ToTimeSeries_(time_item, time_series);
        uint32_t max_ts = 0;
        uint64_t total = 0;
        for(uint32_t i=0;i<time_series.size();i++)
        {
            if(time_series[i]>max_ts) max_ts = time_series[i];
            total += time_series[i];
        }
        double avg_ts = (double)total/time_series.size();
        double special_weight = (double)max_ts/avg_ts;
//         std::cout<<"[time_series] ";
//         for(uint32_t i=0;i<time_series.size();i++)
//         {
//             std::cout<<time_series[i]<<",";
//         }
//         std::cout<<std::endl;
        std::vector<double> macd_result;
        macd_.Compute(time_series, macd_result);
        //get the burst time scope;
        double threshold = 1.0;
        double special_weight_threshold = 13.0;
        double max_macd = 0.0;
        TrackResult tmp_track_result;
        BurstItem burst_item;
        burst_item.reset();
        for(uint32_t i=0;i<macd_result.size();i++)
        {
            if(macd_result[i]>=threshold)
            {
                if(burst_item.empty())
                {
                    burst_item.start_time_diff = i;
                }
                burst_item.period += 1;
                burst_item.weight.push_back(macd_result[i]);
            }
            else
            {
                if(!burst_item.empty())
                {
                    tmp_track_result.items.push_back(burst_item);
                    burst_item.reset();
                }
            }
            if( macd_result[i] > max_macd ) max_macd = macd_result[i];
        }
        //do filter on tmp_track_result
        for(uint32_t i=0;i<tmp_track_result.items.size();i++)
        {
            if(tmp_track_result.items[i].period==0) continue;
            if(tmp_track_result.items[i].period==1 /*&& tmp_track_result.items[i].weight[0]<2.0*/) continue;
            track_result.items.push_back(tmp_track_result.items[i]);
        }
        if(track_result.items.empty()) return false;
        if(special_weight<special_weight_threshold) return false;
        std::string str;
        text.convertString(str, izenelib::util::UString::UTF_8);
        std::cout<<"["<<str<<"]"<<std::endl;
        
//         std::cout<<"[max-macd] : "<<max_macd<<std::endl;
//         std::cout<<"[special_weight] : "<<special_weight<<std::endl;
//         
//         std::cout<<"[macd] ";
//         for(uint32_t i=0;i<macd_result.size();i++)
//         {
//             std::cout<<"("<<i+1<<","<<time_series[i]<<","<<macd_result[i]<<"),";
//         }
//         std::cout<<std::endl;
        std::cout<<"[track-result] : "<<std::endl;
        for(uint32_t i=0;i<track_result.items.size();i++)
        {
            std::cout<<"("<<track_result.items[i].start_time_diff<<","<<track_result.items[i].period<<") - ";
            for(uint32_t j=0;j<track_result.items[i].weight.size();j++)
            {
                std::cout<<"{"<<track_result.items[i].weight[j]<<"},";
            }
            std::cout<<std::endl;
        }
        
        //output to gnuplot
        std::cout<<"[gnuplot] : "<<std::endl;
        for(uint32_t i=0;i<macd_result.size();i++)
        {
            boost::gregorian::date_duration d(i);
            boost::gregorian::date date = valid_range_.start + d;
            std::string date_str = boost::gregorian::to_iso_extended_string(date);
            std::cout<<date_str<<"\t"<<time_series[i]<<"\t"<<macd_result[i]<<std::endl;
        }
        
        //output to file
        if(false)
        {
            std::string output_file = output_dir_+"/"+str+".dat";
            std::string output_burst_file = output_dir_+"/"+str+"-b.dat";
            std::ofstream ofs(output_file.c_str());
            
            for(uint32_t i=0;i<macd_result.size();i++)
            {
                boost::gregorian::date_duration d(i);
                boost::gregorian::date date = valid_range_.start + d;
                std::string date_str = boost::gregorian::to_iso_extended_string(date);
                ofs<<date_str<<"\t"<<time_series[i]<<"\t"<<macd_result[i]<<std::endl;
            }
            ofs.close();
            std::ofstream ofs_burst(output_burst_file.c_str());
            for(uint32_t i=0;i<track_result.items.size();i++)
            {
                uint32_t start_diff = track_result.items[i].start_time_diff;
                for(uint32_t j=0;j<track_result.items[i].period;j++)
                {
                    uint32_t diff = start_diff+j;
                    boost::gregorian::date_duration d(diff);
                    boost::gregorian::date date = valid_range_.start + d;
                    //str burst on date
                    std::string date_str = boost::gregorian::to_iso_extended_string(date);
                    ofs_burst<<date_str<<"\t"<<time_series[diff]<<"\t"<<macd_result[diff]<<std::endl;
                    {
                        std::vector<std::pair<UString, double> >* vec= NULL;
                        vec = burst_items_.find(date);
                        std::pair<UString, double> p(text, macd_result[diff]);
                        if(vec==NULL)
                        {
                            std::vector<std::pair<UString, double> > item(1, p);
                            burst_items_.insert(date, item);
                        }
                        else
                        {
                            vec->push_back(p);
                        }
                    }
                }
            }
            ofs_burst.close();
        }
        valid_topics_.push_back(text);
        freq_.push_back(time_series);
        uint32_t topic_id = valid_topics_.size();
        if(!sim_->Append(topic_id, docItem))
        {
            std::cerr<<"sim append error"<<std::endl;
        }
        
        plot_writer_<<"set output \""<<str<<".png\""<<std::endl;
        plot_writer_<<"set title \""<<str<<"\""<<std::endl;
        plot_writer_<<"plot [\""<<boost::gregorian::to_iso_extended_string(valid_range_.start)
        <<"\":\""<<boost::gregorian::to_iso_extended_string(valid_range_.end)
        <<"\"] '"<<str<<".dat' using 1:2 with boxes linetype 5 title '频率', '"<<str<<"-b.dat' using 1:2 with boxes linetype 1 title '高潮'"<<std::endl<<std::endl;
        
        return true;
    }

    void insertKP_(const std::vector<uint32_t>& terms, const std::vector<id2count_t>& docItem
              , uint8_t score ,const std::vector<id2count_t>& leftTermList
              ,const std::vector<id2count_t>& rightTermList)
    {
        izenelib::util::UString kpStr;
        std::vector<izenelib::util::UString> strList;
        if(!makeKPStr_(terms, strList, kpStr)) return;
        if(exception_.find(kpStr)!=NULL) return;
        TrackResult track_result;
        if(!BurstDetect_(kpStr, terms, docItem, track_result)) return;
        
//         outputKP_(OutputItem( kpStr, track) );
        
    }
    
    void outputKP_(const OutputItem& kpItem)
    {
      std::string str;
      kpItem.text.convertString(str, izenelib::util::UString::UTF_8);
      std::cout<<"["<<str<<"] ";
      for(uint32_t i=0;i<kpItem.timeitem_list.size();i++)
      {
          std::cout<<"("<<kpItem.timeitem_list[i].first<<","<<kpItem.timeitem_list[i].second<<"),";
      }
      std::cout<<std::endl;
    }
    
    
    
    //with complete termidlist version.
    void getCandidateLabel2_(const std::vector<Data>& data, CandidateSSFType::WriterType* htlWriter, Hash2HashSSFType::WriterType* h2hWriter)
    {
        if( data.size()==0 ) return;
        
        //sorting with postorder
        std::vector<uint32_t> post_order(data.size());
        {
          std::stack<uint32_t> index_stack;
          std::stack<uint32_t> depth_stack;
          uint32_t i=0;
          for( uint32_t m=0;m<data.size();m++)
          {
            uint32_t depth = data[m].inc;
            if(m>0)
            {
              while( !depth_stack.empty() && depth <= depth_stack.top() )
              {
                post_order[i] = index_stack.top();
                ++i;
                
                index_stack.pop();
                depth_stack.pop();
              }
            }
            index_stack.push(m);
            depth_stack.push(depth);
          }
          while( !index_stack.empty() )
          {
            post_order[i] = index_stack.top();
            ++i;
            
            index_stack.pop();
            depth_stack.pop();
          }
        }
        
        std::stack<uint32_t> depth_stack;
        std::stack<uint32_t> freq_stack;
        std::stack<std::vector<id2count_t> > doc_item_stack;
        std::stack<std::vector<id2count_t> > prefix_term_stack;
        std::stack<id2count_t > suffix_term_stack;
        if(!tracing_.empty())
        {
          test_num_++;
        }
        for( uint32_t m=0;m<data.size();m++)
        {
          uint32_t depth = data[post_order[m]].inc;
          std::vector<uint32_t> termIdList = data[post_order[m]].termid_list;
          std::vector<id2count_t> docItemList = data[post_order[m]].docitem_list;
          uint32_t freq = data[post_order[m]].freq;
          std::vector<id2count_t > prefixTermList = data[post_order[m]].lc_list;
          std::vector<id2count_t > suffixTermList;
          if(!tracing_.empty())
          {
            if(vec_starts_(termIdList, tracing_))
            {
              std::cout<<"[tracing] {data} "<<test_num_<<","<<m<<","<<post_order[m]<<","<<depth;
              for(uint32_t dd=0;dd<docItemList.size();dd++)
              {
                std::cout<<"("<<docItemList[dd].first<<"|"<<docItemList[dd].second<<"),";
              }
              std::cout<<std::endl;
            }
          }
          while( !depth_stack.empty() && depth < depth_stack.top() )
          {
            freq+=freq_stack.top();
            docItemList.insert( docItemList.end(), doc_item_stack.top().begin(), doc_item_stack.top().end() );
            prefixTermList.insert( prefixTermList.end(), prefix_term_stack.top().begin(), prefix_term_stack.top().end() );
            //suffix howto?
            //suffixTermList, suffix_term_stack
//             suffixTermList.insert( suffixTermList.end(), suffix_term_stack.top().begin(), suffix_term_stack.top().end() );
            suffixTermList.push_back(suffix_term_stack.top() );
            //pop stack
            depth_stack.pop();
            freq_stack.pop();
            doc_item_stack.pop();
            prefix_term_stack.pop();
            suffix_term_stack.pop();
          }
          depth_stack.push( depth);
          freq_stack.push(freq);
          doc_item_stack.push( docItemList);
          prefix_term_stack.push(prefixTermList);
          //get the suffix term
          uint32_t suffix_termid = termIdList[depth];
          suffix_term_stack.push(std::make_pair(suffix_termid, freq) );
          
          if( termIdList.size() > max_phrase_len_ ) continue;
//           std::vector<uint32_t> docIdList;
//           std::vector<uint32_t> tfInDocList;
//           idmlib::util::getIdAndCountList(docItemList, docIdList, tfInDocList);
//                               
//           std::vector<uint32_t> leftTermIdList;
//           std::vector<uint32_t> leftTermCountList;
//           idmlib::util::getIdAndCountList(prefixTermList, leftTermIdList, leftTermCountList);
//           
//           std::vector<uint32_t> rightTermIdList;
//           std::vector<uint32_t> rightTermCountList;
//           idmlib::util::getIdAndCountList(suffixTermList, rightTermIdList, rightTermCountList);
          
          idmlib::util::accumulateList(docItemList);
          idmlib::util::accumulateList(prefixTermList);
          idmlib::util::accumulateList(suffixTermList);
          SI lri(termIdList, freq);
          SCI leftLC( prefixTermList);
          SCI rightLC( suffixTermList);
          if(!tracing_.empty())
          {
            if(vec_equal_(termIdList, tracing_))
            {
              std::cout<<"[tracing] freq: "<<freq<<std::endl;
              for(uint32_t dd=0;dd<docItemList.size();dd++)
              {
                std::cout<<"[tracing] doc item0: "<<docItemList[dd].first<<","<<docItemList[dd].second<<"]"<<std::endl;
              }
            }
          }
          if(manmade_.find(termIdList)!=NULL)
          {
            insertKP_( termIdList, docItemList, freq, prefixTermList, suffixTermList );
            continue;
          }
          int status = KPStatus::CANDIDATE;
          if( freq<min_freq_threshold_ ) status = KPStatus::NON_KP;
          else if( docItemList.size() < min_df_threshold_ ) status = KPStatus::NON_KP;
          else
          {
              status = scorer_->PrefixTest(termIdList);
          }
          
          if( status == KPStatus::NON_KP || status == KPStatus::RETURN )
          {
              continue;
          }
          if( status == KPStatus::KP )
          {
              insertKP_( termIdList, docItemList, freq, prefixTermList, suffixTermList );
              continue;
          }
//           std::pair<bool, double> lcdResult = scorer_->test(lri, leftLC);
//           if( !lcdResult.first ) continue;
//           std::pair<bool, double> rcdResult = scorer_->test(lri, rightLC);
//           if( !rcdResult.first ) continue;
          
          //debug output
//           std::cout<<lri.ToString(id_manager_)<<std::endl;
//           std::cout<<leftLC.ToString(id_manager_)<<std::endl;
//           std::cout<<rightLC.ToString(id_manager_)<<std::endl;
          std::pair<bool, double> scorer_result = scorer_->Test(lri, leftLC, rightLC);
          if( !scorer_result.first ) continue;
          CandidateItem htList(termIdList, docItemList, freq, prefixTermList,suffixTermList );
          hash_t hashId = hash_(termIdList);
          htlWriter->append(hashId , htList );
          if( termIdList.size() >= 2 )
          {
              {
                  std::vector<uint32_t> vec(termIdList.begin(), termIdList.end()-1);
                  Hash2HashItem item(hashId, 1);
                  h2hWriter->append( hash_(vec), item);
              }
              {
                  std::vector<uint32_t> vec(termIdList.begin()+1, termIdList.end());
                  Hash2HashItem item(hashId, 2);
                  h2hWriter->append( hash_(vec), item);
              }
          }
          if( termIdList.size() >= 3 )
          {
              {
                  std::vector<uint32_t> vec(termIdList.begin(), termIdList.begin()+1);
                  Hash2HashItem item(hashId, 3);
                  h2hWriter->append( hash_(vec), item);
              }
              {
                  std::vector<uint32_t> vec(termIdList.end()-1, termIdList.end());
                  Hash2HashItem item(hashId, 4);
                  h2hWriter->append( hash_(vec), item);
              }
          }
            
        }
            
            
        
    }
    
    
private:
  const std::string dir_;
  const std::string id_dir_;
  const std::string tmp_dir_;
  const std::string backup_dir_;
  const std::string output_dir_;
  std::string backup_file_;
  std::string tmp_backup_file_;
  
  std::ofstream plot_writer_;
  
  izenelib::am::rde_hash<TimeIdType, std::vector<std::pair<UString, double> > > burst_items_;
  std::vector<izenelib::util::UString> valid_topics_;
  std::vector<std::vector<uint32_t> > freq_;
  
  idmlib::util::IDMAnalyzer* analyzer_;
  DateRange date_range_;
  TermSimilarityType* sim_;
  idmlib::util::IDMIdManager* id_manager_;
  bool outside_idmanager_;
  uint8_t max_phrase_len_;
  
  uint32_t duration_;
  DateRange backup_range_;
  DateRange valid_range_;
  izenelib::am::ssf::Writer<>* backup_writer_;
  std::vector<TimeIdType> doc2timemap_;
  std::vector<uint64_t> time2countmap_;
  MacdType macd_;
  TermListWriter* pTermListWriter_;
  HashWriter* pHashWriter_;
  KPSSFType::WriterType* kpWriter_;
  uint32_t cache_size_;
  izenelib::am::rde_hash<hash_t, uint32_t> cache_map_;
  std::vector< std::pair<hash_t, uint32_t> > cache_vec_;
  
  ScorerType* scorer_;
  bool outside_scorer_;
  
  uint32_t last_doc_id_;
  uint32_t doc_count_;
  uint32_t all_term_count_;
  
  uint32_t min_freq_threshold_;
  uint32_t min_df_threshold_;
  uint32_t try_compute_num_;
  bool no_freq_limit_;
  izenelib::am::rde_hash<std::vector<uint32_t>, int> manmade_;
  izenelib::am::rde_hash<izenelib::util::UString, bool> exception_;
  std::vector<uint32_t> tracing_;
  uint32_t test_num_;
  
    
};

NS_IDMLIB_TDT_END

#endif 
