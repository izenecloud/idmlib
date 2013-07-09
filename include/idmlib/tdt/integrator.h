#ifndef _IDMLIB_TDT_INTEGRATOR_H_
#define _IDMLIB_TDT_INTEGRATOR_H_

#include "../idm_types.h"

#include <algorithm>
#include <cmath>
#include "temporal_kpe.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <idmlib/util/time_util.h>

NS_IDMLIB_TDT_BEGIN

template <class DMType>
class Integrator
{
    public:
        typedef typename DMType::DocumentType DocumentType;
    public:

        Integrator(const std::string& dir, const std::string& rig_dir, const std::string& kpe_res_dir, idmlib::util::IDMAnalyzer* analyzer)
        : dir_(dir), rig_dir_(rig_dir), kpe_res_dir_(kpe_res_dir), analyzer_(analyzer), data_source_(NULL)
        {

        }

        ~Integrator()
        {

        }

        void SetDataSource(DMType* data_source)
        {
            data_source_ = data_source;
        }

        bool Process(Storage* storage)
        {
            if(data_source_==NULL)
            {
                std::cerr<<"TDT Integrator: no data source"<<std::endl;
                return true;
            }
            uint32_t start_id = 1;
            uint32_t end_id = data_source_->getMaxDocId();
            DateRange date_range;
            uint32_t max_docid = 0;
            bool b_check = CheckDataSource_(start_id, end_id, date_range, max_docid);
            if(!b_check)
            {
                return true;
            }
            TemporalKpe kpe(dir_, analyzer_, date_range, rig_dir_, max_docid);
            if(!kpe.load(kpe_res_dir_))
            {
                std::cerr<<"load kpe res failed"<<std::endl;
                return false;
            }
            kpe.SetStorage(storage);
            uint32_t process_count = 0;
            DocumentType doc;
            for(uint32_t docid = start_id; docid<=end_id;docid++)
            {
                process_count++;
                if( process_count %1000 == 0 )
                {
                    MEMLOG("[TDT] inserted %d. docid: %d", process_count, docid);
                }
                bool b = data_source_->getDocument(docid, doc);
                if(!b) continue;
                izenelib::util::UString title;
                izenelib::util::UString content;
                boost::gregorian::date date;
                if(!ParseDocument_(doc, title, content, date)) continue;
                kpe.Insert(date, docid, title, content);
            }
            kpe.close();
            storage->Flush();
            return true;
        }


    private:

        bool ParseDocument_(const DocumentType& doc, izenelib::util::UString& title, izenelib::util::UString& content, boost::gregorian::date& date)
        {
            typename DocumentType::property_const_iterator property_it = doc.propertyBegin();
            while(property_it != doc.propertyEnd() )
            {
                std::string property_name = property_it->first;
                boost::algorithm::to_lower(property_name);
                if(property_name == "date")
                {
                    const sf1r::PropertyValue::PropertyValueStrType& date_value = property_it->second.getPropertyStrValue();
//                     std::string date_str;
//                     date_value.convertString(date_str, izenelib::util::UString::UTF_8);
//                     std::cout<<"got date : "<<date_str<<std::endl;
                    date = ParseDate_(sf1r::propstr_to_ustr(date_value));
//                     std::cout<<"parse finished"<<std::endl;
//                     if(date.is_not_a_date())
//                     {
//                         std::cout<<"date invalid "<<date_value<<std::endl;
//                     }
//                     else
//                     {
//                         std::cout<<"date "<<boost::gregorian::to_iso_extended_string(date)<<std::endl;
//                     }
                }
                else if(property_name == "title")
                {
                    title = sf1r::propstr_to_ustr(property_it->second.getPropertyStrValue());
                }
                else if(property_name == "content")
                {
                    content = sf1r::propstr_to_ustr(property_it->second.getPropertyStrValue());
                }
                property_it++;
            }
            if(date.is_not_a_date() || title.length()==0)
            {
                return false;
            }
            return true;
        }

        bool CheckDataSource_(uint32_t start_id, uint32_t end_id, DateRange& date_range, uint32_t& max_docid)
        {
            uint32_t total = end_id-start_id+1;
            uint32_t valid = 0;
            uint32_t process_count = 0;
            DocumentType doc;
            for(uint32_t docid = start_id; docid<=end_id;docid++)
            {
                process_count++;
                if( process_count %10000 == 0 )
                {
                    MEMLOG("[TDT-check] inserted %d. docid: %d", process_count, docid);
                }
                bool b = data_source_->getDocument(docid, doc);
                if(!b) continue;
                izenelib::util::UString title;
                izenelib::util::UString content;
                boost::gregorian::date date;
                if(!ParseDocument_(doc, title, content, date)) continue;


                if(date_range.start.is_not_a_date())
                {
                    date_range.start = date;
                    date_range.end = date;
                }
                else
                {
                    if(date<date_range.start)
                    {
                        date_range.start = date;
                    }
                    else if(date>date_range.end)
                    {
                        date_range.end = date;
                    }
                }
                max_docid = docid;
                valid++;
            }
            double v_ratio = (double)valid/total;
            if(v_ratio<0.8)
            {
                std::cerr<<"To low valid documents for tdt "<<valid<<","<<total<<std::endl;
                return false;
            }
            return true;
        }

        boost::gregorian::date ParseDate_(const izenelib::util::UString& text)
        {
            boost::gregorian::date date;
            boost::gregorian::date cand;
            if(idmlib::util::TimeUtil::GetDateByUString(text, cand))
            {
                return cand;
            }
            return date;
        }

        boost::gregorian::date ParseDate_(int64_t time_stamp)
        {
            boost::gregorian::date date;
            boost::gregorian::date cand;
            if(idmlib::util::TimeUtil::GetDateByInt(time_stamp, cand))
            {
                return cand;
            }
            return date;
        }

    private:
        std::string dir_;
        std::string rig_dir_;
        std::string kpe_res_dir_;
        idmlib::util::IDMAnalyzer* analyzer_;
        DMType* data_source_;

};

NS_IDMLIB_TDT_END

#endif
