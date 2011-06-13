///
/// @file link_analysis_similb.h
/// @brief Link(similarity) analysis among topics using SimilB approach
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-06-08
/// @date Updated 2011-06-08
///

#ifndef _IDMLIB_TDT_LINKANALYSISSIMILB_H_
#define _IDMLIB_TDT_LINKANALYSISSIMILB_H_

#include "../idm_types.h"

#include <algorithm>
#include <cmath>
#include <am/external_sort/izene_sort.hpp>
#include <am/sequence_file/SimpleSequenceFile.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/function.hpp>
#include "ema.h"
#include "similb.h"
#include "tdt_types.h"
NS_IDMLIB_TDT_BEGIN

class LinkAnalysisSimilB
{
    public:

typedef boost::function<void (const TrackResult&, const std::vector<izenelib::util::UString>&) > CallbackType;
    public:
        
        LinkAnalysisSimilB(const std::string& dir, const std::string& rig_dir)
        : dir_(dir), rig_dir_(rig_dir)
        {
            
        }
        
        ~LinkAnalysisSimilB()
        {

        }
        
        bool Open(const DateRange& date_range)
        {
            
            return true;
        }
        
        bool Add(const TrackResult& tr)
        {
            topic_list_.push_back(tr);
            return true;
        }
        
        bool Compute(CallbackType callback)
        {
            double threshold = 0.5;

            std::vector<izenelib::util::UString> similar;
            for(uint32_t i=0;i<topic_list_.size();i++)
            {
                const TrackResult& tr = topic_list_[i];
                similar.resize(0);
                for(uint32_t j=0;j<topic_list_.size();j++)
                {
                    if(i==j) continue;
                    double sim = SimilB::Sim(tr.ts, topic_list_[j].ts);
//                     std::cout<<"[SIMILB] "<<i<<","<<j<<" : "<<sim<<std::endl;
                    if(sim<threshold) continue;
                    similar.push_back( topic_list_[j].text);
                }
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
            std::cout<<"[Find-Sim] "<<str<<":";
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
        std::vector<TrackResult> topic_list_;
};

NS_IDMLIB_TDT_END

#endif 
