#include <idmlib/query-correction/cn_query_correction.h>
#include <util/functional.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <cmath>
using namespace idmlib::qc;

#define CN_QC_UNIGRAM

#define CN_QC_DEBUG

// #define CN_QC_DEBUG_DETAIL

// #define VITERBI_DEBUG

CnQueryCorrection::TransProbType CnQueryCorrection::global_trans_prob_;

CnQueryCorrection::CnQueryCorrection(const std::string& res_dir, const std::string& log_dir)
:res_dir_(res_dir), log_dir_(log_dir)
,threshold_(0.0005)
// :threshold_(0.0)
, mid_threshold_(0.0001)
, max_pinyin_term_(7)
{
}

bool CnQueryCorrection::Load()
{
    std::cout<<"[CnQueryCorrection] starting load resources."<<std::endl;
    std::string pinyin_file = res_dir_+"/pinyin.txt";
    if(!boost::filesystem::exists(pinyin_file)) return false;
    pinyin_.LoadPinyinFile(pinyin_file);
    std::cout<<"[CnQueryCorrection] loaded pinyin."<<std::endl;
    global_trans_prob_.clear();
    if (!LoadRawTextTransProb_(res_dir_+"/trans_prob.txt")) return false;
    std::cout<<"[CnQueryCorrection] loaded resources."<<std::endl;
    return true;
}

bool CnQueryCorrection::Update(const std::list<QueryLogType>& query_logs)
{
#ifdef CN_QC_UNIGRAM
    double u_weight = 0.000001;
#endif
    double b_weight = 0.004;
    double t_weight = 0.004;
    for (std::list<QueryLogType>::const_iterator qit = query_logs.begin();
            qit != query_logs.end(); ++qit)
    {
        izenelib::util::UString text(qit->get<2>(), izenelib::util::UString::UTF_8);
        const uint32_t &log_count = qit->get<0>();

        //TODO maybe need refine
        if(text.length()>=1)
        {
#ifdef CN_QC_UNIGRAM
            double score = u_weight*log_count;
            Unigram u(text[0]);
            boost::unordered_map<Unigram, double>::iterator it = collection_trans_prob_.u_trans_prob_.find(u);
            if( it==collection_trans_prob_.u_trans_prob_.end() )
            {
                collection_trans_prob_.u_trans_prob_.insert(std::make_pair(u, score));
            }
            else
            {
                it->second += score;
            }
#endif
        }
        if(text.length()>=2)
        {
            double score = b_weight*log_count;
            Bigram b(text[0], text[1]);
            boost::unordered_map<Bigram, double>::iterator it = collection_trans_prob_.b_trans_prob_.find(b);
            if( it==collection_trans_prob_.b_trans_prob_.end() )
            {
                collection_trans_prob_.b_trans_prob_.insert(std::make_pair(b, score));
            }
            else
            {
                it->second += score;
            }
        }
        if(text.length()>=3)
        {
            double score = t_weight*log_count;
            Bigram b(text[0], text[1]);
            Trigram t(b, text[2]);
            boost::unordered_map<Trigram, double>::iterator it = collection_trans_prob_.t_trans_prob_.find(t);
            if( it==collection_trans_prob_.t_trans_prob_.end() )
            {
                collection_trans_prob_.t_trans_prob_.insert(std::make_pair(t, score));
            }
            else
            {
                it->second += score;
            }
        }
    }

    return true;
}

bool CnQueryCorrection::LoadRawTextTransProb_(const std::string& file)
{
    std::ifstream ifs(file.c_str());
    std::string line;
    uint64_t count = 0;
    while( getline(ifs, line) )
    {
        std::vector<std::string> vec;
        boost::algorithm::split( vec, line, boost::algorithm::is_any_of("\t") );
        if(vec.size()!=2) continue;
        izenelib::util::UString text(vec[0], izenelib::util::UString::UTF_8);
        double score = boost::lexical_cast<double>(vec[1]);

        if(text.length()==1)
        {
#ifdef CN_QC_UNIGRAM
            Unigram u(text[0]);
            global_trans_prob_.u_trans_prob_.insert(std::make_pair(u, score));
#endif
        }
        else if(text.length()==2)
        {
            Bigram b(text[0], text[1]);
            global_trans_prob_.b_trans_prob_.insert(std::make_pair(b, score));
        }
        else if(text.length()==3)
        {
            Bigram b(text[0], text[1]);
            Trigram t(b, text[2]);
            global_trans_prob_.t_trans_prob_.insert(std::make_pair(t, score));
        }
        ++count;
    }
    ifs.close();
    std::cout<<"trans_prob count: "<<count<<std::endl;
    return true;
}

bool CnQueryCorrection::GetResult(const izenelib::util::UString& input, std::vector<izenelib::util::UString>& output)
{
   int type = GetInputType_(input);
   std::vector<CandidateResult> candidate_output;
   if(!GetResultWithScore_(input, type, candidate_output)) return false;
   if(candidate_output.empty())
   {
       if(type<0)// cn char
       {
           return true;
       }
       else //maybe english
       {
           return false;
       }
   }
#ifdef CN_QC_DEBUG
   for(uint32_t i=0;i<candidate_output.size();i++)
   {
       std::string str;
       candidate_output[i].value.convertString(str, izenelib::util::UString::UTF_8);
       std::cout<<"[CR] "<<str<<"\t"<<candidate_output[i].score<<std::endl;
   }
#endif

   std::sort(candidate_output.begin(), candidate_output.end());
   //check if in the result list
   for(uint32_t i=0;i<candidate_output.size();i++)
   {
       if(candidate_output[i].value == input)
       {
           return true;
       }
   }
   output.push_back( candidate_output[0].value);
   double pre_score = candidate_output[0].score;
   double step = 0.7;
   for(uint32_t i=1;i<candidate_output.size();i++)
   {
       double inner_threshold = pre_score*step;
       if( candidate_output[i].score >= inner_threshold)
       {
           output.push_back( candidate_output[i].value);
           pre_score = candidate_output[i].score;
       }
       else
       {
           break;
       }
   }
   return true;
}

void CnQueryCorrection::GetPinyin(const izenelib::util::UString& cn_chars, std::vector<std::string>& result_list)
{
    return pinyin_.GetPinyin(cn_chars, result_list);
}

double CnQueryCorrection::GetScore_(const izenelib::util::UString& text, double ori_score, double pinyin_score)
{
    double score = std::pow( ori_score, 1.0/(text.length()) );
    score *= pinyin_score;
    return score;

}

bool CnQueryCorrection::IsCandidate_(const izenelib::util::UString& text, double ori_score, double pinyin_score, double& score)
{
    score = GetScore_(text, ori_score, pinyin_score);
    if(text.length()==1) return true;
    if(score < mid_threshold_) return false;
    else return true;
}

bool CnQueryCorrection::IsCandidateResult_(const izenelib::util::UString& text, double ori_score, double pinyin_score, double& score)
{
    score = GetScore_(text, ori_score, pinyin_score);
    if(score < threshold_) return false;
    else return true;
}

double CnQueryCorrection::TransProb_(const izenelib::util::UCS2Char& from, const izenelib::util::UCS2Char& to)
{
//     double r = 0.0;
//     boost::unordered_map<Bigram, double>::iterator it = trans_prob_.find(std::make_pair(from, to) );
//     if(it!=b_trans_prob_.end())
//     {
//         r = it->second;
//     }
//     return r;
    return 0.0;
}

double CnQueryCorrection::TransProbT_(const izenelib::util::UString& from, const izenelib::util::UCS2Char& to)
{
    double r = 0.0;
    if(from.length()==0)
    {
#ifdef CN_QC_UNIGRAM
        Unigram u(to);
        boost::unordered_map<Unigram, double>::iterator it = global_trans_prob_.u_trans_prob_.find(u);
        if(it!=global_trans_prob_.u_trans_prob_.end())
        {
            r = it->second;
        }
#else
        r = 1.0;
#endif
    }
    else if(from.length()==1)
    {
        Bigram b(from[0], to);
        boost::unordered_map<Bigram, double>::iterator it = global_trans_prob_.b_trans_prob_.find(b);
        if(it!=global_trans_prob_.b_trans_prob_.end())
        {
            r = it->second;
        }
    }
    else
    {
        Bigram b(from[from.length()-2], from[from.length()-1]);
        Trigram t(b, to);
        boost::unordered_map<Trigram, double>::iterator it = global_trans_prob_.t_trans_prob_.find(t);
        if(it!=global_trans_prob_.t_trans_prob_.end())
        {
            r = it->second;
        }
    }

#ifdef CN_QC_DEBUG_DETAIL
    std::string from_str;
    from.convertString(from_str, izenelib::util::UString::UTF_8);
    std::string to_str;
    izenelib::util::UString tmp;
    tmp+= to;
    tmp.convertString(to_str, izenelib::util::UString::UTF_8);
    std::cout<<"[TP] "<<from_str<<","<<to_str<<" : "<<r<<std::endl;
#endif

    return r;
}

bool CnQueryCorrection::GetResultWithScore_(const izenelib::util::UString& input, int type, std::vector<CandidateResult>& output)
{
    if(type==0)
    {
#ifdef CN_QC_DEBUG
        std::cout<<"type equals 0"<<std::endl;
#endif
        return false;
    }
    std::vector<std::pair<double, std::string> > pinyin_list;
    if(type<0) // cn chars
    {
        pinyin_.GetPinyin(input, pinyin_list);
#ifdef CN_QC_DEBUG
        std::cout<<"Chinese chars"<<std::endl;
        for(uint32_t i=0;i<pinyin_list.size();i++)
        {
            std::cout<<"[pinyin] "<<pinyin_list[i].second<<","<<pinyin_list[i].first<<std::endl;
        }
#endif
        if(pinyin_list.empty()) return true;
    }
    else // input pinyin
    {
        std::string pinyin_str;
        input.convertString(pinyin_str, izenelib::util::UString::UTF_8);
        pinyin_.FuzzySegmentRaw(pinyin_str, pinyin_list);
#ifdef CN_QC_DEBUG
        std::cout<<"Input Pinyin"<<std::endl;
        for(uint32_t i=0;i<pinyin_list.size();i++)
        {
            std::cout<<"[pinyin] "<<pinyin_list[i].second<<","<<pinyin_list[i].first<<std::endl;
        }
#endif
        if(pinyin_list.empty()) return false;
    }
    for(uint32_t i=0;i<pinyin_list.size();i++)
    {
        GetResultByPinyinT_( pinyin_list[i].second, pinyin_list[i].first, output);
    }
    return true;
}

void CnQueryCorrection::GetResultByPinyin_(const std::string& pinyin, double pinyin_score, std::vector<CandidateResult>& output)
{
    uint32_t pinyin_term_count = pinyin_.CountPinyinTerm(pinyin);
    if(pinyin_term_count<=1 ) return;//ignore single pinyin term
    std::string p_str(pinyin);
    //start viterbi
    std::vector<std::vector<ViterbiItem> > matrix;
    std::string first_pinyin;
    while( pinyin_.GetFirstPinyinTerm(p_str, first_pinyin) )
    {
        std::vector<izenelib::util::UCS2Char> char_list;
        if(!pinyin_.GetChar( first_pinyin, char_list) ) return;
        if( matrix.empty() )
        {
            std::vector<ViterbiItem> column(char_list.size());
            for(uint32_t i=0;i<column.size();i++)
            {
                column[i].text = char_list[i];
                column[i].score = 1.0;
                column[i].pre_index = 0;
            }
            matrix.push_back(column);
        }
        else
        {
            std::vector<ViterbiItem> column(char_list.size());
            const std::vector<ViterbiItem>& last_col = matrix.back();
            for(uint32_t i=0;i<column.size();i++)
            {
                double max_score = 0.0;
                uint32_t max_pre_index = 0;
                for(uint32_t j=0;j<last_col.size();j++)
                {
                    double score = TransProb_(last_col[j].text, char_list[i]) * last_col[j].score;
                    if(score>max_score)
                    {
                        max_score = score;
                        max_pre_index = j;
                    }
                }
                column[i].text = char_list[i];
                column[i].score = max_score;
                column[i].pre_index = max_pre_index;
            }
            matrix.push_back(column);
        }
    }

    std::vector<std::vector<ViterbiItem> >::reverse_iterator rit = matrix.rbegin();
    std::vector<ViterbiItem>& last_col = *rit;
    ++rit;
    for(uint32_t i=0;i<last_col.size();i++)
    {
        std::vector<std::vector<ViterbiItem> >::reverse_iterator cp_rit = rit;
        std::vector<izenelib::util::UCS2Char> reverse_char_list;
        double score = last_col[i].score;
        double normalized_score = std::pow( score, 1.0/(pinyin_term_count-1) ) * pinyin_score;
        if(normalized_score<threshold_) continue;
        uint32_t index = last_col[i].pre_index;
        reverse_char_list.push_back(last_col[i].text);
        while( cp_rit!= matrix.rend() )
        {
            std::vector<ViterbiItem>& col = *cp_rit;
            reverse_char_list.push_back( col[index].text );
            index = col[index].pre_index;
            ++cp_rit;
        }
        //make the string
        izenelib::util::UString result_str;
        std::vector<izenelib::util::UCS2Char>::reverse_iterator rit2 = reverse_char_list.rbegin();
        while( rit2!= reverse_char_list.rend())
        {
            result_str+= (*rit2);
            ++rit2;
        }
        CandidateResult result;
        result.value = result_str;
        result.score = normalized_score;
        output.push_back(result);
    }

}

void CnQueryCorrection::GetResultByPinyinT_(const std::string& pinyin, double pinyin_score, std::vector<CandidateResult>& output)
{
//     std::cout<<"GetResultByPinyinT_ "<<pinyin<<std::endl;
    uint32_t pinyin_term_count = pinyin_.CountPinyinTerm(pinyin);
//     std::cout<<"GetResultByPinyinT_ "<<pinyin_term_count<<std::endl;
    if(pinyin_term_count<=1 || pinyin_term_count>max_pinyin_term_ ) return;//ignore single pinyin term
    std::pair<double, izenelib::util::UString> start_mid_result(1.0, izenelib::util::UString("", izenelib::util::UString::UTF_8));
    GetResultByPinyinTRecur_(pinyin, pinyin_score, start_mid_result, output);

}

void CnQueryCorrection::GetResultByPinyinTRecur_(const std::string& pinyin, double pinyin_score, const std::pair<double, izenelib::util::UString>& mid_result, std::vector<CandidateResult>& output)
{
    std::string first_pinyin;
    std::string remain;
    if(pinyin_.GetFirstPinyinTerm(pinyin, first_pinyin, remain))
    {
        std::vector<izenelib::util::UCS2Char> char_list;
        if(!pinyin_.GetChar( first_pinyin, char_list) ) return;
        const izenelib::util::UString& mid_text = mid_result.second;
        std::vector<std::pair<double, izenelib::util::UString> > new_mid_list(char_list.size(), std::make_pair(1.0, mid_text));
        for(uint32_t i=0;i<char_list.size();i++)
        {
            std::pair<double, izenelib::util::UString>& new_mid_result = new_mid_list[i];
            new_mid_result.first = TransProbT_(mid_text, char_list[i]) * mid_result.first;
//             if(mid_text.length()==0)
//             {
//                 new_mid_result.first = 1.0;
//
//             }
//             else
//             {
//
//             }
            new_mid_result.second += char_list[i];
        }
        //TODO cut some results here to speed up
        typedef izenelib::util::first_greater<std::pair<double, izenelib::util::UString> > greater_than;
        std::sort( new_mid_list.begin(), new_mid_list.end(), greater_than());
        double max_score = 0.0;
        for(uint32_t i=0;i<new_mid_list.size();i++)
        {
            std::pair<double, izenelib::util::UString>& new_mid_result = new_mid_list[i];
            double score = 1.0;
            uint32_t text_length = new_mid_result.second.length();
            if(!IsCandidate_( new_mid_result.second, new_mid_result.first, pinyin_score, score))
            {
#ifdef CN_QC_DEBUG_DETAIL
                std::string str;
                new_mid_result.second.convertString(str, izenelib::util::UString::UTF_8);
                std::cout<<"[BR] "<<str<<" , "<<score<<std::endl;
#endif
                break;
            }
            if(i==0)
            {
                max_score = score;
            }
            else
            {
                double rate = score/max_score;
                if(rate<0.2 && text_length>=2) break;
            }
#ifdef CN_QC_DEBUG_DETAIL
            std::string str;
            new_mid_result.second.convertString(str, izenelib::util::UString::UTF_8);
            std::cout<<"[NBR] "<<str<<" , "<<score<<std::endl;
#endif
            GetResultByPinyinTRecur_(remain, pinyin_score, new_mid_result, output);
        }

    }
    else
    {
        double score = 1.0;
        if(IsCandidateResult_( mid_result.second, mid_result.first, pinyin_score, score))
        {
            CandidateResult r;
            r.value = mid_result.second;
            r.score = score;
            output.push_back(r);
        }
    }
}

int CnQueryCorrection::GetInputType_(const izenelib::util::UString& input)
{
    int type = 0;//1:pinyin, -1: cn char
    for(uint32_t i=0;i<input.length();i++)
    {
        if(input.isAlphaChar(i))
        {
            if(type<0)
            {
                type = 0;
                break;
            }
            else
            {
                type = 1;
            }
        }
        else if(input.isChineseChar(i))
        {
            if(type>0)
            {
                type = 0;
                break;
            }
            else
            {
                type = -1;
            }
        }
        else
        {
            type = 0;
            break;
        }
//         std::cout<<"[type] "<<i<<" "<<type<<std::endl;
    }
    return type;
}
