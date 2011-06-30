///
/// @file link_analysis.h
/// @brief Link(similarity) analysis among topics
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-05-23
/// @date Updated 2011-05-23
///

#ifndef _IDMLIB_TDT_LINKANALYSIS_H_
#define _IDMLIB_TDT_LINKANALYSIS_H_

#include "../idm_types.h"

#include <algorithm>
#include <cmath>
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/function.hpp>
#include <idmlib/similarity/term_similarity.h>
#include <idmlib/similarity/term_similarity_table.h>
#include <idmlib/similarity/sim_output_collector.h>
#include "ema.h"
NS_IDMLIB_TDT_BEGIN

class LinkAnalysis
{
    public:
typedef idmlib::sim::TermSimilarityTable<uint32_t> SimTableType;
typedef idmlib::sim::SimOutputCollector<SimTableType> SimCollectorType;
typedef idmlib::sim::TermSimilarity<SimCollectorType, uint32_t, uint32_t, double> TermSimilarityType;
typedef boost::function<void (const TrackResult&, const std::vector<izenelib::util::UString>&) > CallbackType;
    public:
        
        LinkAnalysis(const std::string& dir, const std::string& rig_dir)
        : dir_(dir), rig_dir_(rig_dir)
        {
            
        }
        
        ~LinkAnalysis()
        {
            delete sim_collector_;
            delete sim_;
        }
        
        bool Open(const DateRange& date_range)
        {
            int context_count = date_range.RangeCount();
            if(context_count<0) return false;
            boost::filesystem::create_directories(dir_);
            sim_collector_ = new SimCollectorType(dir_+"/sim_collector", 10);
            if(!sim_collector_->Open()) 
            {
                std::cerr<<"sim collector open error"<<std::endl;
                return false;
            }
            sim_ = new TermSimilarityType(dir_+"/sim", rig_dir_, sim_collector_, 0.8);
            if(!sim_->Open()) 
            {
                std::cerr<<"sim open error"<<std::endl;
                return false;
            }
            if(!sim_->SetContextMax((uint32_t)context_count+1))
            {
                std::cerr<<"sim SetContextMax error"<<std::endl;
                return false;
            }
            return true;
        }
        
        bool Add(const TrackResult& tr)
        {
            topic_list_.push_back(tr);
            uint32_t tid = topic_list_.size();
            std::vector<double> ema_result;
            ema_.Compute(tr.ts, ema_result);
            std::vector<std::pair<uint32_t, double> > item_list(ema_result.size());
            for(uint32_t i=0;i<item_list.size();i++)
            {
                item_list[i].first = i+1;
                item_list[i].second = ema_result[i];
            }
            return sim_->Append(tid, item_list);
        }
        
        bool Compute(CallbackType callback)
        {
            if(!sim_->Compute()) return false;
            SimTableType* table = sim_collector_->GetContainer();

            for(uint32_t i=0;i<topic_list_.size();i++)
            {
                uint32_t tid = i+1;
                std::vector<uint32_t> sim_id_list;
                table->Get(tid, sim_id_list);
                std::vector<izenelib::util::UString> similar;
                for(uint32_t j=0;j<sim_id_list.size();j++)
                {
                    uint32_t stid = sim_id_list[j];
                    similar.push_back(topic_list_[stid-1].text);
                }
                const TrackResult& tr = topic_list_[i];
                callback(tr, similar);
                OutputToConsole_(tr, similar);
            }
            return true;
        }
        
    private:
        
        void OutputToConsole_(const TrackResult& tr, const std::vector<izenelib::util::UString>& similar)
        {
            std::string str;
            tr.text.convertString(str, izenelib::util::UString::UTF_8);
            std::cout<<str<<":";
            for(uint32_t i=0;i<similar.size();i++)
            {
                similar[i].convertString(str, izenelib::util::UString::UTF_8);
                std::cout<<str<<"|";
            }
            std::cout<<std::endl;
        }
        
    private:
        std::string dir_;
        std::string rig_dir_;
        SimCollectorType* sim_collector_;
        TermSimilarityType* sim_;
        Ema<5, uint32_t> ema_;
        std::vector<TrackResult> topic_list_;
};

NS_IDMLIB_TDT_END

#endif 
