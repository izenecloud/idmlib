///
/// @file kpe_evaluate.h
/// @brief for kpe evaluation
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-09-02
/// @date Updated 2011-09-02
///

#ifndef IDMLIB_KPE_EVALUATE_H_
#define IDMLIB_KPE_EVALUATE_H_

#include <vector>
#include "../idm_types.h"
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

NS_IDMLIB_KPE_BEGIN


struct DocEvaluateInfo
{
    boost::unordered_set<std::string> must;
    boost::unordered_set<std::string> all;
    uint32_t must_count;
};

class KpeEvaluate
{
    public:
        
        KpeEvaluate();
        
        void AddTaggedResult(const std::string& docid, const std::vector<std::string>& must_have, uint32_t must_count, const std::vector<std::string>& may_have);
        
        void FindDocResult(const std::string& docid, const std::vector<std::string>& doc_result);
        
        std::string Finish(uint32_t max_per_doc);
    
    private:
        
        std::string Stem_(const std::string& text);
        
        boost::unordered_map<std::string, DocEvaluateInfo> doc_info_;
        boost::unordered_map<std::string, std::vector<std::string> > doc_result_;
        
            
    
};

NS_IDMLIB_KPE_END

#endif 
