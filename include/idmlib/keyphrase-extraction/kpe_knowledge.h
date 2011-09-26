///
/// @file   kpe_knowledge.h
/// @brief  The knowledge class for KPE
/// @author Jia Guo
/// @date   2011-08-22
///
#ifndef IDMLIB_KPE_KNOWLEDGE_H_
#define IDMLIB_KPE_KNOWLEDGE_H_
#include <string>

#include <ctime>
#include <boost/filesystem.hpp>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/function.hpp>
#include <boost/unordered_set.hpp>
#include "../util/StopWordContainer.hpp"
#include "../util/Util.hpp"
#include "../util/idm_analyzer.h"
#include "kpe_term_group.h"
#include "../idm_types.h"
#include "kpe_algorithm_def.h"
NS_IDMLIB_KPE_BEGIN


class KpeStatistic
{
public:    

        inline static double MI(uint32_t f, uint32_t f1, uint32_t f2, uint32_t n)
        {
            return ( double ) f* n / f1/f2;
        }
        inline static double LogL(uint32_t f, uint32_t f1, uint32_t f2, uint32_t n)
        {
            uint32_t k1 = f;
            uint32_t k2 = 0;
            if ( f2>f ) k2 = f2-f;
            uint32_t n1 = f1;
            uint32_t n2 = n-n1;
            return ll_(k1, n1, k2, n2);
        }
        
        static double ChiTest(uint32_t freq, const std::vector<id2count_t>& docitem_list, uint32_t total_doc_count)
        {
            double e = freq/(double)total_doc_count;
            double result = 0.0;
            for(uint32_t i=0;i<docitem_list.size();i++)
            {
                double o = (double)docitem_list[i].second;
                result += (o-e)*(o-e);
            }
            uint32_t left_doc_count = total_doc_count-docitem_list.size();
            result += e*e*left_doc_count;
            result /= e;
            return result;
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

class KpeKnowledge
{
  typedef izenelib::util::UString StringType;
  typedef std::pair<uint32_t, uint32_t> id2count_t;
  public:
    KpeKnowledge(idmlib::util::IDMAnalyzer* analyzer);
    ~KpeKnowledge();
    bool Load(const std::string& res_dir);
    
//     int PrefixTest(const std::vector<uint32_t>& termIdList);
    
    
    bool IsSplitTerm(const idmlib::util::IDMTerm& term);
    
    bool Test(const std::vector<TermInNgram>& term_list, uint32_t freq, const std::vector<id2count_t>& docitem_list, uint32_t total_doc_count
    , const std::vector<std::pair<TermInNgram, uint32_t> >& left_list, const std::vector<std::pair<TermInNgram, uint32_t> >& right_list);
    
    void Flush();
    
    void PostProcess_(const std::vector<std::pair<DocKpItem, double> >& input, std::vector<std::pair<DocKpItem, double> >& output);
  
  private:    
      
    bool TestContext_(const std::vector<std::pair<TermInNgram, uint32_t> >& left_list, const std::vector<std::pair<TermInNgram, uint32_t> >& right_list);
    
    double TestContextScore_(const std::vector<std::pair<TermInNgram, uint32_t> >& context_list);
    
    double TestContextScore2_(const std::vector<std::pair<TermInNgram, uint32_t> >& context_list);
    
    inline double Log2_(double x)
    {
      return Log_(2, x);
    }
    
    inline double Log_(uint32_t n, double x)
    {
      return std::log(x)/std::log(n);
    }
    
    
    
  private:
    idmlib::util::IDMAnalyzer* analyzer_;
    boost::unordered_set<std::string> a_set_;
//     uint32_t arabic_number_;
//     uint32_t singleEnglishChar_;
//     uint32_t symbol_id_;
//     
//     UBInfo ub_info_;

      
        
};

NS_IDMLIB_KPE_END
#endif
