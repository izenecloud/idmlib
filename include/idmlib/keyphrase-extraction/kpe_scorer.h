///
/// @file   kpe_scorer.h
/// @brief  The scorer class for KPE
/// @author Jia Guo
/// @date   2010-04-09
/// @date   2010-04-09
///
#ifndef IDM_KPESCORER_H_
#define IDM_KPESCORER_H_
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
#include "kpe_term_group.h"
#include "../idm_types.h"
#include "ub_info.h"
#include "kpe_scorer_types.h"
NS_IDMLIB_KPE_BEGIN


class StatisticalScorer : public boost::noncopyable
{
public:    
        //if term_group == NULL, means no group info
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

class KPEScorer : public boost::noncopyable
{
  public:
    KPEScorer(idmlib::util::IDMAnalyzer* analyzer);
    ~KPEScorer();
    bool load(const std::string& resPath);
    
    int prefixTest(const std::vector<uint32_t>& termIdList);
    
    
    
    
    
    
    void insertNonAppearTerm( uint32_t termId );
    
    bool isNonAppearTerm( uint32_t termId);
    
    
    void insertMidAppearTerm( uint32_t termId );
    
    bool isMidAppearTerm( uint32_t termId);
    
    bool isSplitTerm(const izenelib::util::UString& ustr, char tag, uint32_t termId, uint32_t& insertTermId);
    
    //only care about SI more than 1 length
    std::pair<bool, double> test(const SI& item,const SCI& left_citem, const SCI& right_citem);
    
    void flush();
  
  private:    
    
    double ub_score_(uint32_t u, uint32_t d);
    
    double ub_p_score_(uint32_t p);
      
    double ub_a_score_(uint32_t a);
    
    double ub_b_score_(uint32_t b);
    
    double ub_d_score_(uint32_t d);
    
    double ub_e_score_(uint32_t e);
    
    double ub_s_score_(uint32_t s);
    
    uint64_t bigram_id_(uint32_t id1, uint32_t id2);
    
    double ub_pa_score_(uint64_t pa);
    
    double ub_pa_score_(uint32_t p, uint32_t a);
    
    double ub_ab_score_(uint64_t ab);
    
    double ub_de_score_(uint64_t de);
    
    double ub_es_score_(uint64_t es);
    
    double ub_se_score_(uint32_t s, uint32_t e);
    
    int ub_prefix_ab_verify_(uint32_t a, uint32_t b);
    
    int ub_ab_verify_(uint32_t a, uint32_t b);
    
    int ub_de_verify_(uint32_t d, uint32_t e);
    
    int ub_aborde_score_verify_(double aborde, double aore, double bord);
    
    int ub_abcde_verify_(uint32_t a, uint32_t b, uint32_t d, uint32_t e);
    
    int ub_context_verify_(const std::vector<uint32_t>& termid_list, const std::vector<uint32_t>& count_list, uint32_t id1, uint32_t id2, uint32_t f, boost::function<double (uint32_t) > u_scorer, boost::function<double (uint32_t, uint32_t) > b_scorer );
    
    int ub_pab_verify_(const std::vector<uint32_t>& p_termid_list, const std::vector<uint32_t>& p_count_list, uint32_t a, uint32_t b, uint32_t f);
    
    int ub_sed_verify_(const std::vector<uint32_t>& s_termid_list, const std::vector<uint32_t>& s_count_list, uint32_t e, uint32_t d, uint32_t f);
    
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
    
    uint32_t arabicNumber_;
    uint32_t singleEnglishChar_;
    
    UBInfo ub_info_;

      
        
};

NS_IDMLIB_KPE_END
#endif
