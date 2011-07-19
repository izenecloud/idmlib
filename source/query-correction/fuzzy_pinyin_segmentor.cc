#include <idmlib/query-correction/fuzzy_pinyin_segmentor.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
using namespace idmlib::qc;

FuzzyPinyinSegmentor::FuzzyPinyinSegmentor()
{
    InitRule_();
}


void FuzzyPinyinSegmentor::LoadPinyinFile(const std::string& file)
{
    std::ifstream ifs(file.c_str());
    std::string line;
    while( getline(ifs, line) )
    {
        boost::algorithm::trim(line);
        std::vector<std::string> vec;
        boost::algorithm::split( vec, line, boost::algorithm::is_any_of(" ") );;
        if(vec.size()!=2) continue;
        std::string pinyin = vec[1].substr(0, vec[1].length()-1);
        bool b = false;
        if(!filter_pinyin_.get(pinyin, b))
        {
            AddPinyin(pinyin);
        }
        
    }
    ifs.close();
}

void FuzzyPinyinSegmentor::AddPinyin(const std::string& pinyin)
{
    pinyin_dict_.Add(pinyin);
//     std::cout<<"load "<<pinyin<<std::endl;
}

void FuzzyPinyinSegmentor::InitRule_()
{
    filter_pinyin_.insert("n", 1);
    filter_pinyin_.insert("ng", 1);
    filter_pinyin_.insert("ang", 1);
    filter_pinyin_.insert("m", 1);
    filter_pinyin_.insert("o", 1);
    
    AddSuffixFuzzy_("an", "ang");
    AddSuffixFuzzy_("en", "eng");
    AddSuffixFuzzy_("in", "ing");
    AddSuffixFuzzy_("on", "ong");
    
    AddPrefixFuzzy_("c", "ch");
    AddPrefixFuzzy_("s", "sh");
    AddPrefixFuzzy_("z", "zh");
    
    AddPrefixFuzzy_("ch", "ch");
    AddPrefixFuzzy_("sh", "sh");
    AddPrefixFuzzy_("zh", "zh");
}

void FuzzyPinyinSegmentor::AddSuffixFuzzy_(const std::string& suffix, const std::string& replace)
{
    std::string n(suffix.rbegin(), suffix.rend());
    suffix_fuzzy_.Add(n, replace);
}

void FuzzyPinyinSegmentor::AddPrefixFuzzy_(const std::string& prefix, const std::string& replace)
{
    prefix_fuzzy_.Add(prefix, replace);
}

bool FuzzyPinyinSegmentor::FuzzyFilter_(const std::string& pinyin_term, std::string& fuzzy_term)
{
    {
        std::string::const_reverse_iterator rit = pinyin_term.rbegin();
        uint32_t id = suffix_fuzzy_.GetRootID();
        uint32_t pos = 0;
        while(rit != pinyin_term.rend())
        {
            pos++;
            uint32_t c_id = 0;
            std::pair<bool, bool> r = suffix_fuzzy_.Find(*rit, id, c_id);
            id = c_id;
            if(!r.first) break;
            if(r.second)
            {
                std::string f_term;
                suffix_fuzzy_.GetData(c_id, f_term);
                std::string a = pinyin_term.substr(0, pinyin_term.length()-pos);
                fuzzy_term = a+f_term;
                break;
            }
            ++rit;
        }
    }
    if(fuzzy_term.length()>0) //detected suffix fuzzy, return
    {
        return true;
    }
    {
        std::string::const_iterator it = pinyin_term.begin();
        uint32_t id = suffix_fuzzy_.GetRootID();
        uint32_t pos = 0;
        std::string f_term;
        while(it != pinyin_term.end())
        {
            uint32_t c_id = 0;
            std::pair<bool, bool> r = prefix_fuzzy_.Find(*it, id, c_id);
            id = c_id;
            if(!r.first) break;
            if(r.second)
            {
                prefix_fuzzy_.GetData(c_id, f_term);
            }
            pos++;
            ++it;
        }
        if(f_term.length()>0)
        {
            std::string a = pinyin_term.substr(0, pos);
            if( a!= f_term)
            {
                std::string b = pinyin_term.substr(pos);
                fuzzy_term = f_term + b;
                return true;
            }
            
        }
    }
    return false;
}

void FuzzyPinyinSegmentor::Segment(const std::string& pinyin_str, std::vector<PinyinSegmentResult>& result_list)
{
    if(pinyin_str.length()==0) return;
    uint32_t id = pinyin_dict_.GetRootID();
    for(uint32_t i=0;i<pinyin_str.length();i++)
    {
        uint32_t c_id = 0;
        std::pair<bool, bool> r = pinyin_dict_.Find(pinyin_str[i], id, c_id);
        //make maximum match.
        id = c_id;
        if(!r.first) break;
    }
}

void FuzzyPinyinSegmentor::SegmentRaw(const std::string& pinyin_str, std::vector<std::string>& result_list)
{
//     uint32_t c_id = 0;
//     pinyin_dict_.Find('n', 0, c_id);
//     c_id = 0;
//     pinyin_dict_.Find('a', 1, c_id);
//     c_id = 0;
//     pinyin_dict_.Find('n', 0, c_id);
//     c_id = 0;
//     pinyin_dict_.Find('n', 0, c_id);
//     c_id = 0;
//     pinyin_dict_.Find(pinyin_str[0], 0, c_id);
//     std::vector<std::string> pre_result_list(1, "");
    SegmentRaw_(pinyin_str, "", result_list);
}

void FuzzyPinyinSegmentor::SegmentRaw_(const std::string& pinyin_str, const std::string& mid_result, std::vector<std::string>& result_list)
{
    if(pinyin_str.length()==0) return;
//     std::cout<<"pinyin_str "<<pinyin_str<<std::endl;
    uint32_t id = pinyin_dict_.GetRootID();
    
    for(uint32_t i=0;i<pinyin_str.length();i++)
    {
        uint32_t c_id = 0;
        char c = pinyin_str[i];
        
//         pinyin_dict_.Find('n', 0, c_id);
        
        std::pair<bool, bool> r = pinyin_dict_.Find(c, id, c_id);
//         std::cout<<pinyin_str[i]<<","<<(int)r.first<<","<<(int)r.second<<","<<id<<","<<c_id<<std::endl;
        id = c_id;
        
        //make maximum match.
        if(!r.first) break;
        else
        {
            if(r.second)
            {
                std::string a = pinyin_str.substr(0, i+1);
                std::string b = pinyin_str.substr(i+1);
//                     std::cout<<"[]"<<a<<","<<b<<std::endl;
                std::string n_mid_result = mid_result;
                if( n_mid_result.length()>0) n_mid_result+=",";
                n_mid_result += a;
                if(b.length()>0)
                {
                    SegmentRaw_(b, n_mid_result, result_list);
                }
                else
                {
                    result_list.push_back( n_mid_result);
                }
            }
        }
    }
}

void FuzzyPinyinSegmentor::FuzzySegmentRaw(const std::string& pinyin_str, std::vector<std::string>& result_list)
{
    std::vector<std::string> r_result_list;
    SegmentRaw(pinyin_str, r_result_list);
    for(uint32_t i=0;i<r_result_list.size();i++)
    {
        FuzzySegmentRaw_(r_result_list[i], "", result_list);
    }
}

bool FuzzyPinyinSegmentor::GetFirstPinyinTerm(const std::string& pinyin, std::string& first, std::string& remain)
{
    if(pinyin.empty()) return false;
    std::size_t pos = pinyin.find(',');
    if (pos!=std::string::npos)
    {
        first = pinyin.substr(0, pos);
        remain = pinyin.substr(pos+1);
    }
    else
    {
        first = pinyin;
        remain = "";
    }
    return true;
}

bool FuzzyPinyinSegmentor::GetFirstPinyinTerm(std::string& pinyin, std::string& first)
{
    if(pinyin.empty()) return false;
    std::size_t pos = pinyin.find(',');
    if (pos!=std::string::npos)
    {
        first = pinyin.substr(0, pos);
        pinyin = pinyin.substr(pos+1);
    }
    else
    {
        first = pinyin;
        pinyin = "";
    }
    return true;
}
        
void FuzzyPinyinSegmentor::FuzzySegmentRaw_(const std::string& pinyin_str, const std::string& mid_result, std::vector<std::string>& result_list)
{
    std::string first;
    std::string remain;
    if( GetFirstPinyinTerm(pinyin_str, first, remain) )
    {
        std::string new_mid(mid_result);
        if(!new_mid.empty()) new_mid += ",";
        
        FuzzySegmentRaw_(remain, new_mid+first, result_list);
        std::string f_term;
        if( FuzzyFilter_(first, f_term) )
        {
            FuzzySegmentRaw_(remain, new_mid+f_term, result_list);
        }
    }
    else
    {
        result_list.push_back(mid_result);
    }
}


