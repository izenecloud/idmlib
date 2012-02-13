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
#include "link_analysis.h"
#include "link_analysis_similb.h"
#include "storage.h"
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
#include <idmlib/util/string_trie.h>

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
typedef idmlib::sim::TermSimilarityTable<uint32_t> SimTableType;
typedef idmlib::sim::SimOutputCollector<SimTableType> SimCollectorType;
typedef idmlib::sim::TermSimilarity<SimCollectorType> TermSimilarityType;

// typedef LinkAnalysis LinkAnalysisType;
typedef LinkAnalysisSimilB LinkAnalysisType;

public:



    TemporalKpe(const std::string& dir, idmlib::util::IDMAnalyzer* analyzer, const DateRange& date_range, const std::string& rig_dir, uint32_t max_docid)
    : dir_(dir), rig_dir_(rig_dir), id_dir_(dir+"/id"), tmp_dir_(dir+"/tmp"), link_dir_(dir+"/link")
    , backup_dir_(dir+"/backup")
    , output_dir_(dir+"/output")
    , analyzer_(analyzer)
    , date_range_(date_range)
    , id_manager_(NULL), outside_idmanager_(false)
    , max_phrase_len_(7)
    , ngram_writer_(NULL), hashcount_writer_(NULL)
    , cache_size_(0), cache_vec_(cache_size_),scorer_(NULL), outside_scorer_(false)
    , last_doc_id_(0), doc_count_(0), all_term_count_(0)
    , storage_(NULL)
    {
      init_(max_docid);
    }

    TemporalKpe(const std::string& dir, idmlib::util::IDMAnalyzer* analyzer, const DateRange& date_range, idmlib::util::IDMIdManager* id_manager, const std::string& rig_dir , uint32_t max_docid)
    : dir_(dir), rig_dir_(rig_dir), id_dir_(dir+"/id"), tmp_dir_(dir+"/tmp"), link_dir_(dir+"/link")
    , backup_dir_(dir+"/backup")
    , output_dir_(dir+"/output")
    , analyzer_(analyzer)
    , date_range_(date_range)
    , id_manager_(id_manager), outside_idmanager_(true)
    , max_phrase_len_(7)
    , ngram_writer_(NULL), hashcount_writer_(NULL)
    , cache_size_(0), cache_vec_(cache_size_),scorer_(NULL), outside_scorer_(false)
    , storage_(NULL)
    {
      init_(max_docid);
    }

    ~TemporalKpe()
    {
      release_();
    }

    void SetStorage(Storage* storage)
    {
        storage_ = storage;
    }

    void EnableBackup()
    {
        backup_ = true;
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
      if(backup_)
      {
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
        if(backup_)
        {
            TryBackup_(time_id, doc_id, title, content);
        }
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
        std::vector<std::pair<bool,TermInNgram> > splitVec;
        uint32_t i=iBegin;
        uint32_t _begin = iBegin;
        TermInNgram new_term;
        for ( ;i<termList.size();i++ )
        {
            bool bSplit = scorer_->IsSplitTerm(termList[i], new_term);

            splitVec.push_back(std::make_pair(bSplit, new_term));
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
        std::vector<TermInNgram> terms( splitVec.size() );
        for(uint32_t p=_begin;p<_begin+splitVec.size();p++)
        {
            uint32_t _index = p-_begin;
            terms[_index] = splitVec[_index].second;
            id_manager_->Put(terms[_index].id, termList[p].text);
//             if( !splitVec[_index].first )
//             {
//
//             }
        }
        AppendNgram_(docid, terms, bFirstTerm, bLastTerm);
        return true;
    }


    void AppendNgram_(DocIdType docid, const std::vector<TermInNgram>& termList, bool bFirstTerm = true, bool bLastTerm = true)
    {
//         std::cout<<docId<<"### "<<termList.size()<<" "<<(int)bFirstTerm<<" "<<(int)bLastTerm<<std::endl;
        if( termList.size() == 0 ) return;
        std::vector<uint32_t> termid_list(termList.size());
        for(uint32_t i=0;i<termList.size();i++)
        {
            termid_list[i] = termList[i].id;
        }
        if( termList.size() == 1 )
        {
            if( bFirstTerm && bLastTerm )
            {
                Ngram ngram(termList, docid);
                if( scorer_->PrefixTest(termid_list) != KPStatus::RETURN)
                {
                  WriteNgram_(ngram);
                }

                WriteHashItem_(hash_(termid_list));
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
            std::vector<TermInNgram> new_term_list(termList.begin()+i, termList.begin()+i+len );
            std::vector<uint32_t> new_id_list(termid_list.begin()+i, termid_list.begin()+i+len );
            bool valid_frag =true;
            if( scorer_->PrefixTest(new_id_list) != KPStatus::RETURN)
            {
              Ngram ngram(new_term_list, docid);
              if( i!= 0 )
              {
                  ngram.left_term = termList[i-1];

              }
              WriteNgram_(ngram);
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
              if(vec_starts_(new_id_list, tracing_))
              {
                std::cout<<"[tracing] ["<<(int)valid_frag<<"] ";
                for(uint32_t dd=0;dd<new_id_list.size();dd++)
                {
                  std::cout<<new_id_list[dd]<<",";
                }
                std::cout<<std::endl;
              }
            }
            for(uint32_t j= i+1; j<= end; j++)
            {
                if( j-i >= max_phrase_len_ ) continue;
                std::vector<uint32_t> ifrag( termid_list.begin()+i, termid_list.begin()+j );
                WriteHashItem_(hash_(ifrag));
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
      if( try_compute_num_>0 && hashcount_writer_->Count() >= try_compute_num_ )
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
      if( ngram_writer_ == NULL )
      {
        ngram_writer_ = new izenelib::am::ssf::Writer<>(tmp_dir_+"/ngram_writer_file");
        ngram_writer_->Open();
      }
      if( hashcount_writer_ == NULL)
      {
        hashcount_writer_ = new izenelib::am::ssf::Writer<>(tmp_dir_+"/hashcount_writer_file");
        hashcount_writer_->Open();
      }
    }



    void init_(uint32_t max_docid)
    {
        backup_ = false;
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
        topic_id_ = 0;
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
        if( ngram_writer_ == NULL )
        {
            ngram_writer_ = new izenelib::am::ssf::Writer<>(tmp_dir_+"/ngram_writer_file");
            ngram_writer_->Open();
        }
        if( hashcount_writer_ == NULL)
        {
            hashcount_writer_ = new izenelib::am::ssf::Writer<>(tmp_dir_+"/hashcount_writer_file");
            hashcount_writer_->Open();
        }

        //init idmanager
        if( id_manager_ == NULL )
        {
            id_manager_ = new idmlib::util::IDMIdManager(id_dir_);
        }

        sim_collector_ = new SimCollectorType(tmp_dir_+"/sim_collector", 10);
        if(!sim_collector_->Open())
        {
            std::cerr<<"sim collector open error"<<std::endl;
            return ;
        }

        sim_ = new TermSimilarityType(tmp_dir_+"/sim", rig_dir_, sim_collector_, 0.3);
        if(!sim_->Open())
        {
            std::cerr<<"sim open error"<<std::endl;
            return ;
        }
        if(!sim_->SetContextMax(max_docid))
        {
            std::cerr<<"sim SetContextMax error"<<std::endl;
            return;
        }

        link_ = new LinkAnalysisType(tmp_dir_+"/link", rig_dir_);
        if(!link_->Open(date_range_))
        {
            std::cerr<<"link open error"<<std::endl;
            return ;
        }
    }


    void release_()
    {
        plot_writer_.close();
        if( ngram_writer_!= NULL)
        {
            delete ngram_writer_;
            ngram_writer_ = NULL;
        }
        if( hashcount_writer_!= NULL)
        {
            delete hashcount_writer_;
            hashcount_writer_ = NULL;
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
        delete sim_collector_;
        delete sim_;
        delete link_;
        boost::filesystem::remove_all(tmp_dir_);

    }





    inline void WriteNgram_(const Ngram& ngram)
    {
      std::vector<uint32_t> list;
      ngram.ToUint32List(list);
      ngram_writer_->Append(list);
    }

    void WriteHashItem_(hash_t hash_value)
    {
      if( cache_size_ > 0 )
      {
        if( cache_vec_.size() >= cache_size_ )
        {
//           std::cout<<"[FULL]"<<std::endl;
          //output
          for(uint32_t i=0;i<cache_vec_.size();i++)
          {
            hashcount_writer_->Append(cache_vec_[i].first, cache_vec_[i].second);
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
        hashcount_writer_->Append(hash_value);
      }

    }

    void releaseCachedHashItem_()
    {
      if( cache_size_ > 0 )
      {
        for(uint32_t i=0;i<cache_vec_.size();i++)
        {
          hashcount_writer_->Append(cache_vec_[i].first, cache_vec_[i].second);
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
                    result.push_back(' ');
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
                    burst_item.start_time = valid_range_.start+DD(i);
                }
                burst_item.period += 1;
                burst_item.weight.push_back(macd_result[i]);
            }
            else
            {
                if(!burst_item.empty())
                {
                    tmp_track_result.burst.push_back(burst_item);
                    burst_item.reset();
                }
            }
            if( macd_result[i] > max_macd ) max_macd = macd_result[i];
        }
        //do filter on tmp_track_result
        for(uint32_t i=0;i<tmp_track_result.burst.size();i++)
        {
            if(tmp_track_result.burst[i].period==0) continue;
            if(tmp_track_result.burst[i].period==1 /*&& tmp_track_result.items[i].weight[0]<2.0*/) continue;
            track_result.burst.push_back(tmp_track_result.burst[i]);
        }
        if(track_result.burst.empty()) return false;
        if(special_weight<special_weight_threshold) return false;
        track_result.text = text;
        track_result.start_time = date_range_.start;
        track_result.ts.assign(time_series.begin()+MacdType::G_Value, time_series.end());
//         std::string str;
//         text.convertString(str, izenelib::util::UString::UTF_8);
//         std::cout<<"["<<str<<"]"<<std::endl;

//         std::cout<<"[max-macd] : "<<max_macd<<std::endl;
//         std::cout<<"[special_weight] : "<<special_weight<<std::endl;
//
//         std::cout<<"[macd] ";
//         for(uint32_t i=0;i<macd_result.size();i++)
//         {
//             std::cout<<"("<<i+1<<","<<time_series[i]<<","<<macd_result[i]<<"),";
//         }
//         std::cout<<std::endl;
//         std::cout<<"[track-result] : "<<std::endl;
//         for(uint32_t i=0;i<track_result.items.size();i++)
//         {
//             std::cout<<"("<<track_result.items[i].start_time_diff<<","<<track_result.items[i].period<<") - ";
//             for(uint32_t j=0;j<track_result.items[i].weight.size();j++)
//             {
//                 std::cout<<"{"<<track_result.items[i].weight[j]<<"},";
//             }
//             std::cout<<std::endl;
//         }

        //output to gnuplot
//         std::cout<<"[gnuplot] : "<<std::endl;
//         for(uint32_t i=0;i<macd_result.size();i++)
//         {
//             boost::gregorian::date_duration d(i);
//             boost::gregorian::date date = valid_range_.start + d;
//             std::string date_str = boost::gregorian::to_iso_extended_string(date);
//             std::cout<<date_str<<"\t"<<time_series[i]<<"\t"<<macd_result[i]<<std::endl;
//         }
//




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
//         ++topic_id_;
        valid_topics_.push_back(track_result);
//         topic_result_writer_->Append(topic_id_, track_result);
        uint32_t topic_id = valid_topics_.size();
        if(!sim_->Append(topic_id, docItem))
        {
            std::cerr<<"sim append error for topic id "<<topic_id_<<std::endl;
        }

    }

    void InsertTopic_(const TrackResult& tr)
    {

        link_->Add(tr);
    }

    void OutputTopicToFile_(const TrackResult& topic, const std::vector<UString>& similar)
    {
        std::string str;
        topic.text.convertString(str, izenelib::util::UString::UTF_8);
        std::cout<<"[topic] "<<str<<std::endl;
        std::string topic_output_file = output_dir_+"/"+str+".json";
        std::ofstream topic_ofs(topic_output_file.c_str());
        topic_ofs<<"{"<<std::endl;
        topic_ofs<<"\t\"topic\":"<<std::endl;
        topic_ofs<<"\t{"<<std::endl;
        topic_ofs<<"\t\t\"text\" : \""<<str<<"\","<<std::endl;
        topic_ofs<<"\t\t\"ts\" : "<<std::endl;
        topic_ofs<<"\t\t["<<std::endl;
        const std::vector<uint32_t>& freq = topic.ts;
        for(uint32_t j=0;j<freq.size();j++)
        {
            boost::gregorian::date_duration d(j);
            boost::gregorian::date date = date_range_.start + d;
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
        for(uint32_t i=0;i<similar.size();i++)
        {
            similar[i].convertString(str, izenelib::util::UString::UTF_8);
            topic_ofs<<"\t\t\t\""<<str<<"\"";
            if(i!=similar.size()-1)
            {
                topic_ofs<<",";
            }
            topic_ofs<<std::endl;
        }
        topic_ofs<<"\t\t]"<<std::endl;
        topic_ofs<<"\t}"<<std::endl;
        topic_ofs<<"}"<<std::endl;
        topic_ofs.close();
    }

    void OutputTopicToPlot_(const TrackResult& topic, const std::vector<UString>& similar)
    {
        double threshold = 1.0;
        std::string str;
        topic.text.convertString(str, izenelib::util::UString::UTF_8);
        std::string output_file = output_dir_+"/"+str+".dat";
//         std::string output_v_file = output_dir_+"/"+str+"-v.dat";
//         std::string output_a_file = output_dir_+"/"+str+"-a.dat";
        std::vector<double> v_value;
        std::vector<double> a_value;
        macd_.ComputeMacd(topic.ts, v_value);
        macd_.Compute(topic.ts, a_value);
        std::ofstream ofs(output_file.c_str());

        for(uint32_t i=0;i<topic.ts.size();i++)
        {
            boost::gregorian::date_duration d(i);
            boost::gregorian::date date = date_range_.start + d;
            std::string date_str = boost::gregorian::to_iso_extended_string(date);
            ofs<<date_str<<"\t"<<topic.ts[i]<<"\t"<<v_value[i]<<"\t"<<a_value[i]<<"\t"<<threshold<<std::endl;
        }
        ofs.close();

        plot_writer_<<"set output \""<<str<<".png\""<<std::endl;
        plot_writer_<<"set title \""<<str<<"\""<<std::endl;
        plot_writer_<<"plot [\""<<boost::gregorian::to_iso_extended_string(valid_range_.start)
        <<"\":\""<<boost::gregorian::to_iso_extended_string(valid_range_.end)
        <<"\"] '"<<str<<".dat' using 1:2 with linespoints linetype 3 title 'frequency', '"<<str<<".dat' using 1:3 with linespoints linetype 5 title 'velocity', '"<<str<<".dat' using 1:4 with linespoints linetype 1 title 'acceleration', '"<<str<<".dat' using 1:5 with linespoints linetype 4 pointtype -1 title 'threshold'"<<std::endl<<std::endl;

//         std::ofstream ofs_burst(output_burst_file.c_str());
//         for(uint32_t i=0;i<track_result.items.size();i++)
//         {
//             uint32_t start_diff = track_result.items[i].start_time_diff;
//             for(uint32_t j=0;j<track_result.items[i].period;j++)
//             {
//                 uint32_t diff = start_diff+j;
//                 boost::gregorian::date_duration d(diff);
//                 boost::gregorian::date date = valid_range_.start + d;
//                 //str burst on date
//                 std::string date_str = boost::gregorian::to_iso_extended_string(date);
//                 ofs_burst<<date_str<<"\t"<<time_series[diff]<<"\t"<<macd_result[diff]<<std::endl;
//                 {
//                     std::vector<std::pair<UString, double> >* vec= NULL;
//                     vec = burst_items_.find(date);
//                     std::pair<UString, double> p(text, macd_result[diff]);
//                     if(vec==NULL)
//                     {
//                         std::vector<std::pair<UString, double> > item(1, p);
//                         burst_items_.insert(date, item);
//                     }
//                     else
//                     {
//                         vec->push_back(p);
//                     }
//                 }
//             }
//         }
//         ofs_burst.close();
    }

    void OutputTopic_(const TrackResult& topic, const std::vector<UString>& similar)
    {
        if(storage_!=NULL)
        {
            storage_->Add(topic, similar);
        }
        else
        {
            OutputTopicToFile_(topic, similar);
            OutputTopicToPlot_(topic, similar);
        }
    }


    void TrackCombine_(const std::vector<TrackResult>& source_list, TrackResult& tt)
    {
        std::vector<std::pair<UString, uint32_t> > text_list(source_list.size());
        for(uint32_t i=0;i<source_list.size();i++)
        {
            text_list[i].first = source_list[i].text;
            text_list[i].second = source_list[i].text.length();
        }
        typedef izenelib::util::second_greater<std::pair<UString, uint32_t> > greater_than;
        std::sort(text_list.begin(), text_list.end(), greater_than());
        idmlib::util::StringTrie<> trie;
        for(uint32_t i=0;i<text_list.size();i++)
        {
            std::string str;
            text_list[i].first.convertString(str, izenelib::util::UString::UTF_8);
            if(!trie.Contains(text_list[i].first))
            {
                if(tt.text.length()==0)
                {
                    tt.text = text_list[i].first;
                }
                else
                {
                    tt.text += izenelib::util::UString(",", izenelib::util::UString::UTF_8);
                    tt.text += text_list[i].first;
                }
                trie.Insert(text_list[i].first);
            }
        }

        tt.ts.resize(source_list[0].ts.size());
        for(uint32_t i=0;i<tt.ts.size();i++)
        {
            uint32_t max = source_list[0].ts[i];
            for(uint32_t j=1;j<source_list.size();j++)
            {
                uint32_t v = source_list[j].ts[i];
                if(v>max) max = v;
            }
            tt.ts[i] = max;
        }
        std::vector<TimeIdType> burst_date_list;
        for(uint32_t i=0;i<source_list.size();i++)
        {
            for(uint32_t j=0;j<source_list[i].burst.size();j++)
            {
                const BurstItem& bi = source_list[i].burst[j];
                for(uint32_t k=0;k<bi.period;k++)
                {
                    TimeIdType date = bi.start_time + DD(k);
                    burst_date_list.push_back(date);
                }
            }
        }
        std::sort(burst_date_list.begin(), burst_date_list.end());
        std::vector<TimeIdType>::iterator uit = std::unique(burst_date_list.begin(), burst_date_list.end());
        burst_date_list.erase(uit, burst_date_list.end());

        BurstItem item;
        for(uint32_t i=0;i<burst_date_list.size();i++)
        {
            if(item.empty())
            {
                item.start_time = burst_date_list[i];
            }
            else
            {
                DD dd = burst_date_list[i] - item.start_time;
                uint32_t d = dd.days();
                if(d>1)
                {
                    tt.burst.push_back(item);
                    item.reset();
                    item.start_time = burst_date_list[i];
                }
            }
            item.period++;
        }
        tt.burst.push_back(item);
    }


    //with complete termidlist version.
    void getCandidateLabel2_(const std::vector<NgramInCollection>& data, izenelib::am::ssf::Writer<>* htlWriter, izenelib::am::ssf::Writer<>* h2hWriter)
    {
        if( data.size()==0 ) return;
//         std::cout<<"getCandidateLabel2_ "<<data.size()<<std::endl;
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
        std::stack<std::vector<std::pair<TermInNgram, uint32_t> > > prefix_term_stack;
        std::stack<std::pair<TermInNgram, uint32_t> > suffix_term_stack;
        if(!tracing_.empty())
        {
          test_num_++;
        }
        for( uint32_t m=0;m<data.size();m++)
        {
          uint32_t depth = data[post_order[m]].inc;
          std::vector<TermInNgram> termList = data[post_order[m]].term_list;
          std::vector<id2count_t> docItemList = data[post_order[m]].docitem_list;
          uint32_t freq = data[post_order[m]].freq;
          std::vector<std::pair<TermInNgram, uint32_t> > prefixTermList = data[post_order[m]].lc_list;
          std::vector<std::pair<TermInNgram, uint32_t> > suffixTermList;
//           if(!tracing_.empty())
//           {
//             if(vec_starts_(termIdList, tracing_))
//             {
//               std::cout<<"[tracing] {data} "<<test_num_<<","<<m<<","<<post_order[m]<<","<<depth;
//               for(uint32_t dd=0;dd<docItemList.size();dd++)
//               {
//                 std::cout<<"("<<docItemList[dd].first<<"|"<<docItemList[dd].second<<"),";
//               }
//               std::cout<<std::endl;
//             }
//           }
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
          TermInNgram suffix_term = termList[depth];
          suffix_term_stack.push(std::make_pair(suffix_term, freq) );

          if( termList.size() > max_phrase_len_ ) continue;
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

          std::vector<uint32_t> termIdList(termList.size());
          for(uint32_t i=0;i<termList.size();i++)
          {
              termIdList[i] = termList[i].id;
          }

          idmlib::util::accumulateList(docItemList);
          idmlib::util::accumulateList(prefixTermList);
          idmlib::util::accumulateList(suffixTermList);
          std::vector<std::pair<uint32_t, uint32_t> > left_termid_list(prefixTermList.size());
          for(uint32_t i=0;i<prefixTermList.size();i++)
          {
              left_termid_list[i].first = prefixTermList[i].first.id;
              left_termid_list[i].second = prefixTermList[i].second;
          }

          std::vector<std::pair<uint32_t, uint32_t> > right_termid_list(suffixTermList.size());
          for(uint32_t i=0;i<suffixTermList.size();i++)
          {
              right_termid_list[i].first = suffixTermList[i].first.id;
              right_termid_list[i].second = suffixTermList[i].second;
          }
          SI lri(termList, freq);
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
            insertKP_( termIdList, docItemList, freq, left_termid_list, right_termid_list );
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
              insertKP_( termIdList, docItemList, freq, left_termid_list, right_termid_list );
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



          CandidateItem htList(termIdList, docItemList, freq, left_termid_list,right_termid_list );
          hash_t hashId = hash_(termIdList);
          htlWriter->Append(hashId , htList );
          if( termIdList.size() >= 2 )
          {
              {
                  std::vector<uint32_t> vec(termIdList.begin(), termIdList.end()-1);
                  Hash2HashItem item(hashId, 1);
                  h2hWriter->Append( hash_(vec), item);
              }
              {
                  std::vector<uint32_t> vec(termIdList.begin()+1, termIdList.end());
                  Hash2HashItem item(hashId, 2);
                  h2hWriter->Append( hash_(vec), item);
              }
          }
          if( termIdList.size() >= 3 )
          {
              {
                  std::vector<uint32_t> vec(termIdList.begin(), termIdList.begin()+1);
                  Hash2HashItem item(hashId, 3);
                  h2hWriter->Append( hash_(vec), item);
              }
              {
                  std::vector<uint32_t> vec(termIdList.end()-1, termIdList.end());
                  Hash2HashItem item(hashId, 4);
                  h2hWriter->Append( hash_(vec), item);
              }
          }

        }



    }

    void compute_()
    {
      releaseCachedHashItem_();
      uint64_t hash_total_count = hashcount_writer_->Count();
      if(  hash_total_count==0 )
      {
        return;
      }
      ngram_writer_->Close();
      hashcount_writer_->Close();
      std::string inputItemPath = ngram_writer_->GetPath();
      std::string hashItemPath = hashcount_writer_->GetPath();
      delete ngram_writer_;
      ngram_writer_ = NULL;
      delete hashcount_writer_;
      hashcount_writer_ = NULL;

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
      izenelib::am::ssf::Sorter<uint32_t, uint64_t>::Sort(hashItemPath);

      MEMLOG("[KPE1] finished");
      typedef izenelib::am::SSFType<hash_t, uint32_t, uint8_t> HashItemCountSSFType;

      izenelib::am::ssf::Writer<> hcwriter(tmp_dir_+"/HCWRITER");
      hcwriter.Open();
      {
          izenelib::am::ssf::Reader<> hreader(hashItemPath);
          hreader.Open();
          hash_t hash_id;
          hash_t save_id;
          uint32_t count = 0;
          uint32_t icount = 1;
          LOG_BEGIN2("Hash", &hreader);
          while( true )
          {
            if(!hreader.Next(hash_id, icount)) break;
            if(p==0)
            {
                save_id = hash_id;
            }
            else
            {
                if( save_id != hash_id )
                {
                    hcwriter.Append(save_id, count);
                    save_id = hash_id;
                    count = 0;
                }
            }
            count += icount;
            p++;
            LOG_PRINT2("Hash", 1000000);
          }
          LOG_END2();
          hcwriter.Append(save_id, count);
          hreader.Close();
          idmlib::util::FSUtil::del(hashItemPath);

      }
      hcwriter.Close();
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
      izenelib::am::ssf::Sorter<uint32_t, uint32_t, true>::Sort(inputItemPath);
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
      izenelib::am::ssf::Writer<> hclWriter(tmp_dir_+"/HCLWRITER");
      izenelib::am::ssf::Writer<> h2hWriter(tmp_dir_+"/H2HWRITER");
      {

          hclWriter.Open();
          h2hWriter.Open();
          izenelib::am::ssf::Reader<> fireader(inputItemPath);
          fireader.Open();
          std::vector<uint32_t> key_list;

          LOG_BEGIN2("Suffix", &fireader);

          NgramInCollection last_ngram;
          std::vector<NgramInCollection> for_process;
          while( fireader.Next(key_list) )
          {
              if(!tracing_.empty())
              {
                  if(vec_starts_(key_list, tracing_))
                  {
                      std::cout<<"[tracing] [suffix] ";
                      for(uint32_t dd=0;dd<key_list.size();dd++)
                      {
                        std::cout<<key_list[dd]<<",";
                      }
                      std::cout<<std::endl;
                  }
              }
              Ngram ngram = Ngram::ParseUint32List(key_list);
              if(last_ngram.IsEmpty())
              {
                  last_ngram += ngram;
              }
              else
              {
                  uint32_t inc = last_ngram.GetInc(ngram);
                  if(inc==ngram.term_list.size()) //the same
                  {
                      last_ngram += ngram;
                  }
                  else
                  {
                      last_ngram.Flush();
                      for_process.push_back(last_ngram);
                      if(inc==0)
                      {
                          //TODO do for_process
                          getCandidateLabel2_(for_process, &hclWriter, &h2hWriter);
                          for_process.resize(0);
                      }
                      last_ngram.Clear();
                      last_ngram.inc = inc;
                      last_ngram += ngram;

                  }
              }


              p++;
              LOG_PRINT2("Suffix", 1000000);
          }
          LOG_END2();
          //TODO do for_process
          getCandidateLabel2_(for_process, &hclWriter, &h2hWriter);
          fireader.Close();
          idmlib::util::FSUtil::del(inputItemPath);
          std::cout<<"[AAA stat] "<<hclWriter.Count()<<","<<h2hWriter.Count()<<std::endl;
          hclWriter.Close();
          h2hWriter.Close();
      }


      MEMLOG("[KPE4] finished.");


      {
          izenelib::am::ssf::Sorter<uint32_t, uint64_t>::Sort(hclWriter.GetPath());
          izenelib::am::ssf::Sorter<uint32_t, uint64_t>::Sort(h2hWriter.GetPath());
      }
      izenelib::am::ssf::Writer<> h2cWriter( tmp_dir_+"/H2CWRITER" );
      h2cWriter.Open();
      {
        izenelib::am::ssf::Reader<> reader1(hcwriter.GetPath());
        reader1.Open();
        izenelib::am::ssf::Reader<> reader2(h2hWriter.GetPath());
        reader2.Open();

        izenelib::am::ssf::Merger<uint32_t, hash_t, uint32_t, Hash2HashItem> merger(&reader1, &reader2);


        hash_t key = 0;
        std::vector<uint32_t> valueList1;
        std::vector<Hash2HashItem> valueList2;

        while( merger.Next( key, valueList1, valueList2) )
        {
//             std::cout<<"[merger] "<<key<<","<<valueList1.size()<<","<<valueList2.size()<<std::endl;
            if( valueList1.size() == 0 )
            {
                //impossible, just a reminder.
                std::cerr<<"[warning] valuelist1 size 0"<<std::endl;
                continue;
            }
            if( valueList1.size() > 1 )
            {
                //impossible, just a reminder.
                std::cerr<<"[warning] valuelist1 size "<<valueList1.size()<<std::endl;
                continue;
            }
            for(uint32_t i=0;i<valueList2.size();i++)
            {
                h2cWriter.Append( valueList2[i].first, Hash2CountItem(valueList1[0],valueList2[i].second) );

            }
        }
        reader1.Close();
        reader2.Close();
        h2cWriter.Close();
      }
      {
          izenelib::am::ssf::Sorter<uint32_t, uint64_t>::Sort(h2cWriter.GetPath());
      }
//         sleep(1000);
      izenelib::am::ssf::Writer<> hcltWriter( tmp_dir_+"/HCLTWRITER" );
      hcltWriter.Open();
      {
          izenelib::am::ssf::Reader<> h2cReader(h2cWriter.GetPath());
          h2cReader.Open();
          Hash2CountItem hash2CountValue;
          LOG_BEGIN2("HashCount", &h2cReader);
          HashCountListItem output(0, 0, 0, 0);
          hash_t key;
          hash_t saveKey;
          while( h2cReader.Next(key, hash2CountValue) )
          {
              if(p==0)
              {
                  saveKey = key;
              }
              else
              {
                  if(saveKey != key )
                  {
                      hcltWriter.Append(saveKey, output);
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
              LOG_PRINT2("HashCount", 100000);
          }
          hcltWriter.Append(saveKey, output);
          LOG_END2();
          h2cReader.Close();
          idmlib::util::FSUtil::del(h2cReader.GetPath());
      }
      hcltWriter.Close();


      {
          double min_logl = 10.0;
          double min_mi = 5.0;
          izenelib::am::ssf::Reader<> reader1(hclWriter.GetPath());
          reader1.Open();
          izenelib::am::ssf::Reader<> reader2(hcltWriter.GetPath());
          reader2.Open();

          izenelib::am::ssf::Merger<uint32_t, hash_t, CandidateItem, HashCountListItem > merger(&reader1, &reader2);

          hash_t key = 0;
          std::vector<CandidateItem> valueList1;
          std::vector<HashCountListItem> valueList2;
          uint32_t n = getTermCount_();
          while( merger.Next(key, valueList1, valueList2) )
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
          reader1.Close();
          reader2.Close();
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
              SimTableType* table = sim_collector_->GetContainer();
              izenelib::am::rde_hash<uint32_t, bool> apps;

              for(uint32_t i=0;i<valid_topics_.size();i++)
              {
                  uint32_t topic_id = i+1;
                  if(apps.find(topic_id)!=NULL) continue;
                  std::vector<uint32_t> sim_id_list;
                  table->Get(topic_id, sim_id_list);
                  if(sim_id_list.size()==0)
                  {
                      InsertTopic_(valid_topics_[i]);
                  }
                  else
                  {
                      std::vector<TrackResult> sim_list(sim_id_list.size()+1);
                      sim_list[0] = valid_topics_[i];
                      for(uint32_t j=1;j<sim_list.size();j++)
                      {
                          sim_list[j] = valid_topics_[sim_id_list[j-1]-1];
                      }
                      TrackResult tt;
                      TrackCombine_(sim_list, tt);
                      InsertTopic_(tt);


                  }
                  for(uint32_t j=0;j<sim_id_list.size();j++)
                  {
                      apps.insert(sim_id_list[j], 1);
                  }

              }

              LinkAnalysis::CallbackType func = boost::bind( &TemporalKpe::OutputTopic_, this, _1, _2);
              link_->Compute(func);
          }

          //output cloud file
//           std::string cloud_file = output_dir_+"/cloud.json";
//           std::ofstream cloud_ofs(cloud_file.c_str());
//           cloud_ofs<<"{\n\t\"cloud\":\n\t[\n";
//           boost::gregorian::date_duration d = valid_range_.end - valid_range_.start;
//           uint32_t days = d.days();
//           for(uint32_t day=0;day<days;day++)
//           {
//               boost::gregorian::date_duration dd(day);
//               boost::gregorian::date date = valid_range_.start+dd;
//               std::vector<std::pair<UString, double> >* pburst_item = burst_items_.find(date);
//               if(pburst_item==NULL) continue;
//               std::vector<std::pair<UString, double> >& burst_item = *pburst_item;
//               typedef izenelib::util::second_greater<std::pair<UString, double> > greater_than;
//               std::sort(burst_item.begin(), burst_item.end(), greater_than());
//               std::string date_str = boost::gregorian::to_iso_extended_string(date);
//               cloud_ofs<<"\t\t{\n\t\t\t\"date\":\""<<date_str<<"\",\n\t\t\t\"burst\":\n\t\t\t[\n";
//               for(uint32_t i=0;i<burst_item.size();i++)
//               {
//                   std::string str;
//                   burst_item[i].first.convertString(str, izenelib::util::UString::UTF_8);
//                   cloud_ofs<<"\t\t\t\t{\n\t\t\t\t\t\"text\":\""<<str<<"\",\n\t\t\t\t\t\"weight\":"<<burst_item[i].second<<"\n\t\t\t\t}";
//                   if(i!=burst_item.size()-1)
//                   {
//                       cloud_ofs<<",";
//                   }
//                   cloud_ofs<<std::endl;
//               }
//               cloud_ofs<<"\t\t\t]\n\t\t}";
//               if(day!=days-1)
//               {
//                   cloud_ofs<<",";
//               }
//               cloud_ofs<<std::endl;
//           }
//           cloud_ofs<<"\t]\n}\n";
//           cloud_ofs.close();
      }
    }


private:
  const std::string dir_;
  const std::string rig_dir_;
  const std::string id_dir_;
  const std::string tmp_dir_;
  const std::string link_dir_;
  const std::string backup_dir_;
  const std::string output_dir_;
  bool backup_;
  std::string backup_file_;
  std::string tmp_backup_file_;

  std::ofstream plot_writer_;

  uint32_t topic_id_;

  std::vector<TrackResult> valid_topics_;

  idmlib::util::IDMAnalyzer* analyzer_;
  DateRange date_range_;
  SimCollectorType* sim_collector_;
  TermSimilarityType* sim_;
  LinkAnalysisType* link_;
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

  izenelib::am::ssf::Writer<>* ngram_writer_;
  izenelib::am::ssf::Writer<>* hashcount_writer_;

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
  Storage* storage_;

};

NS_IDMLIB_TDT_END

#endif
