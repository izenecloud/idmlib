#include <idmlib/keyphrase-extraction/kpe_evaluate.h>
#include <boost/algorithm/string.hpp>
#include <sstream>
using namespace idmlib::kpe;

KpeEvaluate::KpeEvaluate()
{
}

std::string KpeEvaluate::Stem_(const std::string& text)
{
    std::string str = boost::algorithm::to_lower_copy(text);
    boost::algorithm::replace_all(str, " ", "");
    return str;
}
        
void KpeEvaluate::AddTaggedResult(const std::string& docid, const std::vector<std::string>& must_have, uint32_t must_count, const std::vector<std::string>& may_have)
{
    DocEvaluateInfo doc;
    for(uint32_t i=0;i<must_have.size();i++)
    {
        std::string str = Stem_(must_have[i]);
        doc.must.insert(str);
        doc.all.insert(str);
    }
    for(uint32_t i=0;i<may_have.size();i++)
    {
        std::string str = Stem_(may_have[i]);
        doc.all.insert(str);
    }
    doc.must_count = must_count;
    doc_info_.insert(std::make_pair(docid, doc));
    
}

void KpeEvaluate::FindDocResult(const std::string& docid, const std::vector<std::string>& doc_result)
{
    if( doc_info_.find(docid) != doc_info_.end())
    {
        std::vector<std::string> s_doc_result(doc_result.size());
        for(uint32_t i=0;i<doc_result.size();i++)
        {
            s_doc_result[i] = Stem_(doc_result[i]);
        }
        doc_result_.insert(std::make_pair(docid, s_doc_result));
    }
}

std::string KpeEvaluate::Finish(uint32_t max_per_doc)
{
    uint32_t correct4precision = 0;
    uint32_t extract4precision = 0;
    uint32_t correct4recall = 0;
    uint32_t standard4recall = 0;
    boost::unordered_map<std::string, DocEvaluateInfo>::iterator info_it = doc_info_.begin();
    while( info_it!=doc_info_.end())
    {
        DocEvaluateInfo& info = info_it->second;
        const std::string& docid = info_it->first;
        ++info_it;
        boost::unordered_map<std::string, std::vector<std::string> >::iterator result_it = doc_result_.find(docid);
        if(result_it==doc_result_.end())
        {
            continue;
        }
        std::vector<std::string>& ori_result = result_it->second;
        uint32_t count = std::min( (uint32_t)ori_result.size(), max_per_doc);
        std::vector<std::string> result(ori_result.begin(), ori_result.begin()+count);
        standard4recall += info.must_count;
        extract4precision += result.size();
        for(uint32_t i=0;i<result.size();i++)
        {
            if(info.all.find(result[i])!=info.all.end())
            {
                ++correct4precision;
            }
            if(info.must.find(result[i])!=info.must.end())
            {
                ++correct4recall;
            }
        }
    }
    
    double precision = (double)correct4precision/extract4precision;
    double recall = (double)correct4recall/standard4recall;
    double f = 2*precision*recall/(precision+recall);
    std::stringstream ss;
    ss<<"precision:"<<precision<<", recall:"<<recall<<", f:"<<f;
    return ss.str();
}
