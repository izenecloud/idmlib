///
/// @file   tdt_scorer.h
/// @brief  The scorer class for tdt temporal kpe
/// @author Jia Guo
/// @date   2011-04-18
/// @date   2011-04-18
///
#ifndef IDM_TDT_TDTSCORER_H_
#define IDM_TDT_TDTSCORER_H_
#include <string>

#include <ctime>
#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp> 
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <cache/IzeneCache.h>
#include <util/izene_serialization.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/function.hpp>
#include "../util/StopWordContainer.hpp"
#include "../util/Util.hpp"
#include "../util/idm_analyzer.h"

#include "../idm_types.h"
#include "tdt_scorer_types.h"
#include "../keyphrase-extraction/kpe_term_group.h"
NS_IDMLIB_TDT_BEGIN


class StatisticalScorer : public boost::noncopyable
{
public:    
        StatisticalScorer()
        {
            
        }
        inline static double mi(uint32_t f, uint32_t f1, uint32_t f2, uint32_t n)
        {
            return ( double ) f* n / f1/f2;
        }
        inline static double logL(uint32_t f, uint32_t f1, uint32_t f2, uint32_t n)
        {
            uint32_t k1 = f;
            uint32_t k2 = 0;
            if ( f2>f ) k2 = f2-f;
            uint32_t n1 = f1;
            uint32_t n2 = n-n1;
            return ll_(k1, n1, k2, n2);
        }
    private:
        
        static double ll_ ( uint32_t k1, uint32_t n1, uint32_t k2, uint32_t n2 )
        {
            return ll_ ( ( double ) k1/n1,k1,n1 ) +
            ll_ ( ( double ) k2/n2,k2,n2 )-ll_ ( ( double ) ( k1+k2 ) / ( n1+n2 ),k1,n1 )-
            ll_ ( ( double ) ( k1+k2 ) / ( n1+n2 ),k2,n2 );
        }
        static double ll_ ( double p,uint32_t k,uint32_t n )
        {
            double v1 = 0.0;
            double v2 = 0.0;
            if( p==0 || k==0) v1 = 0.0;
            else
            {
                v1 = std::log ( p ) *k;
            }
            if( p==1 || k==n) v2 = 0.0;
            else
            {
                v2 = std::log ( 1-p ) * ( n-k );
            }
            return v1+v2;
        }
        
};

class TDTScorer : public boost::noncopyable
{
  typedef std::pair<uint32_t, uint32_t> id2count_t;
  public:
    TDTScorer(idmlib::util::IDMAnalyzer* analyzer)
    :analyzer_(analyzer), swContainer_(NULL),
    invalidChnBigram_(new izenelib::am::rde_hash<uint64_t, bool>()), 
    nonAppearTerms_(new izenelib::am::rde_hash<uint32_t, bool>()), 
    midAppearTerms_(new izenelib::am::rde_hash<uint32_t, bool>())
    {

    }
    ~TDTScorer()
    {
        if( swContainer_ != NULL ) delete swContainer_;
        if( invalidChnBigram_ != NULL ) delete invalidChnBigram_;
        if( nonAppearTerms_ != NULL ) delete nonAppearTerms_;
        if( midAppearTerms_ != NULL ) delete midAppearTerms_;
    }
    bool Load(const std::string& resPath)
    {
        std::cout<<"[TDTScorer] loading TDT resources..."<<std::endl;
        namespace bfs = boost::filesystem;
        swContainer_ = new idmlib::util::StopWordContainer();
        {
            std::istream* ifs = idmlib::util::getResourceStream(resPath+"/stop_words");
            if( ifs== NULL)
            {
                std::cerr<<"[TDTScorer] load stop_words failed."<<std::endl;
                return false;
            }
            std::string word;
            while ( getline ( *ifs,word ) )
            {
                boost::to_lower(word);
                if ( word.length() >0 )
                {
        //                       std::cout<<"add stop "<<word<<std::endl;
                    std::vector<uint32_t> termIdList;
                    analyzer_->GetIdList(izenelib::util::UString(word,izenelib::util::UString::UTF_8), termIdList);

                    if( termIdList.size() > 0 )
                    {
                        swContainer_->insert(termIdList);
                    }
                    
                }
            }
            delete ifs;
        }
            
        
        singleEnglishChar_ = idmlib::util::IDMIdConverter::GetId( izenelib::util::UString("SINGLEENGLISHCHAR",izenelib::util::UString::UTF_8) );
        symbol_id_ = idmlib::util::IDMIdConverter::GetId( izenelib::util::UString("THISISSYMBOL",izenelib::util::UString::UTF_8) );
        
        std::cout<<"[TDTScorer] loading TDT resources finished."<<std::endl;
        return true;
    }

    int PrefixTest(const std::vector<uint32_t>& termid_list)
    {
        uint32_t termCount = termid_list.size();
        if( termCount == 0 ) return KPStatus::NON_KP;
        uint32_t first_termid = termid_list[0];
        if( IsNonAppearTerm(first_termid) )
        {
            return KPStatus::RETURN;
        }
        if(termCount == 1)
        {
            //check if single korean noun
            if( idmlib::util::IDMIdConverter::IsKP(first_termid) )
            {
                return KPStatus::KP;
            }
            else
            {
                return KPStatus::NON_KP;
            }
        }

        //is it should be startWithStopWord?
        if ( swContainer_->endWithStopWord( termid_list ) )
        {
            return KPStatus::RETURN;
        }
        if( IsMidAppearTerm(termid_list.front()) || IsMidAppearTerm(termid_list.back())  )
        {
            return KPStatus::NON_KP;
        }
        
        return KPStatus::CANDIDATE;
    }


    void InsertNonAppearTerm( uint32_t termId )
    {
        if( nonAppearTerms_ == NULL )
        {
            nonAppearTerms_ = new izenelib::am::rde_hash<uint32_t, bool>();
        }
        nonAppearTerms_->insert(termId, 0);
    }

    bool IsNonAppearTerm( uint32_t termId)
    {
        if( termId == singleEnglishChar_ ) return true;
        if( nonAppearTerms_ == NULL ) return false;
        return (nonAppearTerms_->find(termId)!=NULL);
    }

    void InsertMidAppearTerm( uint32_t termId )
    {
        if( midAppearTerms_ == NULL )
        {
            midAppearTerms_ = new izenelib::am::rde_hash<uint32_t, bool>();
        }
        midAppearTerms_->insert(termId, 0);
    }

    bool IsMidAppearTerm( uint32_t termId)
    {
        if( midAppearTerms_ == NULL ) return false;
        return (midAppearTerms_->find(termId)!=NULL);
    }

    bool IsSplitTerm(const idmlib::util::IDMTerm& term, uint32_t& insertTermId)
    {
        //TODO need to refactor
        uint32_t indicateId = term.id;
        insertTermId = term.id;
        bool result = false;
        char tag = term.tag;
    //             std::cout<<"aaa"<<std::endl;
        if(tag == idmlib::util::IDMTermTag::SYMBOL)
        {
            return true;
        }
        else if( tag == idmlib::util::IDMTermTag::ENG)
        {
            
            izenelib::util::UString lower = term.text;
            lower.toLowerString();
            indicateId = idmlib::util::IDMIdConverter::GetId(lower, tag);
            
            if( IsNonAppearTerm(indicateId) )
            {
                insertTermId = indicateId;
                return true;
            }
            if( lower.length() == 1 )
            {
                InsertNonAppearTerm(indicateId);
                insertTermId = indicateId;
                return true;
            }
            
        }
        else if( tag == idmlib::util::IDMTermTag::CHN)
        {
            if( IsNonAppearTerm(indicateId) )
            {
                return true;
            }
        }
    //             std::cout<<"bbb"<<std::endl;
        if( swContainer_->isStopWord( indicateId ) )
        {
            if( tag == idmlib::util::IDMTermTag::ENG)
            {
                insertTermId = indicateId;
            }
            return true;
        }
    //             std::cout<<"ccc"<<std::endl;
        if ( tag == idmlib::util::IDMTermTag::KOR && term.text.isDigitChar(0) )
        {
            InsertNonAppearTerm(term.id);
            result = true;
        }
        else if( (tag == idmlib::util::IDMTermTag::KOR||tag == idmlib::util::IDMTermTag::KOR_NOUN) && term.text.length()==1 )
        {
            InsertNonAppearTerm(term.id);
            result = true;
        }
        else if( tag == idmlib::util::IDMTermTag::KOR )
        {
            InsertMidAppearTerm(term.id);
        }
    //             std::cout<<"ddd"<<std::endl;
        
        return result;
    }
    
    double Bce_(const std::vector<double>& prob_list, const std::vector<double>& weight_list)
    {
        double result = 0.0;
        for (std::size_t i=0; i<prob_list.size();++i)
        {
        //     std::cout<<"{bce_} "<<prob_list[i]<<","<<weight_list[i]<<std::endl;
            if( prob_list[i] > 0.0)
            {
                double inc = 0.0;
                if(i==prob_list.size()-1)//this is the space
                {
                    inc = weight_list[i] * prob_list[i]*(-1)*(-1);
                }
                else
                {
                    inc = weight_list[i] * prob_list[i]*log2_(prob_list[i])*(-1);
                }
                
                result += inc;
            }
        }
        //normalize
        double normal_factor = 1.0/log_(10, prob_list.size()+1 );
        result *= normal_factor;
        return result;
    }
    
    int ContextVerify_(const std::vector<id2count_t>& term_list, uint32_t id1, uint32_t id2, uint32_t f)
    {
        std::vector<uint32_t> freq_list;
        idmlib::kpe::KPETermGroup::GetInstance().filter(term_list, freq_list);
        std::vector<double> pors_score_list(freq_list.size()+1);
        std::vector<double> b_score_list(freq_list.size()+1);
        std::vector<double> prob_list(freq_list.size()+1);
        //diff with normal kpe here
        std::vector<double> result_weight_list(freq_list.size()+1, 1.0);
        result_weight_list.back() = 5.0;
        uint32_t sum = 0;
        for( uint32_t i=0;i<freq_list.size();i++)
        {
            prob_list[i] = (double) freq_list[i] / f;
            sum += freq_list[i];
        }
        prob_list.back() = ((double)f - sum)/f;
        
        double bce_score = Bce_(prob_list, result_weight_list);
        //judge by bce_score
//         std::cout<<"[BCE]"<<bce_score<<std::endl;
        if( bce_score>2.0 ) return KPStatus::CANDIDATE;
        else return KPStatus::NON_KP;
    }

    //only care about SI more than 1 length
    std::pair<bool, double> Test(const SI& item,const SCI& left_citem, const SCI& right_citem)
    {
        std::pair<bool, double> result(false, 0.0);
        if(item.termid_list.size()<2) return result;
        uint32_t a = item.termid_list[0];
        uint32_t b = item.termid_list[1];
        uint32_t d = item.termid_list[item.termid_list.size()-2];
        uint32_t e = item.termid_list[item.termid_list.size()-1];
        int pab_status = ContextVerify_(left_citem.term_list, a, b, item.freq);
        if( pab_status == KPStatus::KP )
        {
            result.first = true;
            result.second = 1.0;
            return result;
        }
        else if( pab_status != KPStatus::CANDIDATE )
        {
            return result;//not KPE
        }
        
        int sed_status = ContextVerify_(right_citem.term_list, e, d, item.freq);
        if( sed_status == KPStatus::KP )
        {
            result.first = true;
            result.second = 1.0;
            return result;
        }
        else if( sed_status != KPStatus::CANDIDATE )
        {
            return result;//not KPE
        }
        //passed all filter, it is treated as KP.
        result.first = true;
        result.second = 1.0;
        return result;
    }

    void Flush()
    {
        if( nonAppearTerms_ != NULL )
        {
            delete nonAppearTerms_;
            nonAppearTerms_ = NULL;
        }
        if( midAppearTerms_ != NULL )
        {
            delete midAppearTerms_;
            midAppearTerms_ = NULL;
        }
    }
  
  private:    
    
    
    
    int ub_context_verify_(const std::vector<id2count_t>& term_list, uint32_t id1, uint32_t id2, uint32_t f, boost::function<double (uint32_t) > u_scorer, boost::function<double (uint32_t, uint32_t) > b_scorer );
    
    int ub_pab_verify_(const std::vector<id2count_t>& p_term_list, uint32_t a, uint32_t b, uint32_t f);
    
    int ub_sed_verify_(const std::vector<id2count_t>& s_term_list, uint32_t e, uint32_t d, uint32_t f);
    
    void ub_context_weight_(const std::vector<double>& pors_score_list, const std::vector<double>& paorse_score_list, std::vector<double>& result_weight_list);
    
    double bce_(const std::vector<double>& prob_list, const std::vector<double>& weight_list);
    
    double ub_space_score_();
    
    double ub_noappearance_score_();
    
    double normalize_(uint32_t context_count);
    
    inline double log2_(double x)
    {
      return log_(2, x);
    }
    
    inline double log_(uint32_t n, double x)
    {
      return std::log(x)/std::log(n);
    }
    
    
  private:
    idmlib::util::IDMAnalyzer* analyzer_;
    idmlib::util::StopWordContainer* swContainer_;
    izenelib::am::rde_hash<uint64_t, bool>* invalidChnBigram_;
    izenelib::am::rde_hash<uint32_t, bool>* nonAppearTerms_;//never occur in label
    izenelib::am::rde_hash<uint32_t, bool>* midAppearTerms_;//can occur in the middle of labels
    
    uint32_t singleEnglishChar_;
    uint32_t symbol_id_;
    
      
        
};

NS_IDMLIB_TDT_END
#endif
