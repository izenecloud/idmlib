#include <idmlib/query-correction/cn_query_correction.h>
#include <util/functional.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <cmath>
using namespace idmlib::qc;

#define CN_QC_UNIGRAM

//#define CN_QC_DEBUG

//#define CN_QC_DEBUG_DETAIL

//#define VITERBI_DEBUG

namespace {

#ifdef CN_QC_UNIGRAM
    const double u_weight = 0.000001;
#endif
    const double b_weight = 0.004;
    const double t_weight = 0.004;

}

std::string CnQueryCorrection::res_dir_;
CnQueryCorrection::TransProbType CnQueryCorrection::global_trans_prob_;
FuzzyPinyinSegmentor CnQueryCorrection::pinyin_;

CnQueryCorrection::CnQueryCorrection(const std::string& collection_dir)
    : collection_dir_(collection_dir)
    , threshold_(0.0005)
    , mid_threshold_(0.0001)
    , max_pinyin_term_(7)
{
}

bool CnQueryCorrection::ForceReload()
{
    global_trans_prob_.clear();
    return Load();
}

bool CnQueryCorrection::Load()
{
    boost::mutex::scoped_lock scopedLock(mutex_);

    std::cout << "[CnQueryCorrection] start loading Chinese resources." << std::endl;
    if (global_trans_prob_.empty())
    {
        std::cout << "[CnQueryCorrection] loading pinyin." << std::endl;
        std::string pinyin_file = res_dir_ + "/pinyin.txt";
        if (!boost::filesystem::exists(pinyin_file))
        {
            std::cout << "[CnQueryCorrection] failed loading pinyin." << std::endl;
            return false;
        }
        pinyin_.LoadPinyinFile(pinyin_file);
        std::cout << "[CnQueryCorrection] loaded pinyin." << std::endl;

        std::cout << "[CnQueryCorrection] loading global_trans_prob." << std::endl;
        std::string trans_prob_file = res_dir_ + "/trans_prob.txt";
        if (!boost::filesystem::exists(trans_prob_file))
        {
            std::cout << "[CnQueryCorrection] failed loading global_trans_prob." << std::endl;
            return false;
        }
        LoadRawTextTransProb_(global_trans_prob_, trans_prob_file);
        std::cout << "[CnQueryCorrection] loaded global_trans_prob." << std::endl;
    }
    if (!collection_dir_.empty())
    {
        std::string trans_prob_file = collection_dir_ + "/trans_prob.txt";
        if (boost::filesystem::exists(trans_prob_file))
        {
            std::cout << "[CnQueryCorrection] loading collection_trans_prob." << std::endl;
            LoadRawTextTransProb_(collection_trans_prob_, trans_prob_file);
            std::cout << "[CnQueryCorrection] loaded collection_trans_prob." << std::endl;
        }
        else
            boost::filesystem::create_directories(collection_dir_);
    }
    std::cout << "[CnQueryCorrection] loaded Chinese resources." << std::endl;

    return true;
}

bool CnQueryCorrection::Update(const std::list<QueryLogType>& queryList, const std::list<PropertyLabelType>& labelList, bool forceMode)
{
    if (forceMode && !ForceReload())
        return false;

    std::cout << "[CnQueryCorrection] start loading query logs." << std::endl;
    collection_trans_prob_.clear();

    for (std::list<QueryLogType>::const_iterator it = queryList.begin();
            it != queryList.end(); ++it)
    {
        const izenelib::util::UString &text = it->get<2>();
        if (GetInputType_(text) != -1)
            continue;

        const uint32_t &df = it->get<0>();
        UpdateItem_(df, text);
    }

    for (std::list<PropertyLabelType>::const_iterator it = labelList.begin();
            it != labelList.end(); ++it)
    {
        const izenelib::util::UString &text = it->second;
        if (GetInputType_(text) != -1)
            continue;

        const uint32_t &df = it->first;
        UpdateItem_(df, text);
    }
    std::cout << "[CnQueryCorrection] loaded query logs." << std::endl;

    std::cout << "[CnQueryCorrection] start flushing collection_trans_prob." << std::endl;
    std::string trans_prob_file = collection_dir_ + "/trans_prob.txt";
    FlushRawTextTransProb_(trans_prob_file, collection_trans_prob_);
    std::cout << "[CnQueryCorrection] flushed collection_trans_prob." << std::endl;

    return true;
}

void CnQueryCorrection::UpdateItem_(const uint32_t df, const izenelib::util::UString& text)
{
    double score;
    size_t len = text.length();

    for (size_t i = 2; i < len; i++)
    {
#ifdef CN_QC_UNIGRAM
        Unigram u(text[i - 2]);
        score = u_weight * df;
        collection_trans_prob_.u_trans_prob_[u] += score;
#endif

        Bigram b(text[i - 2], text[i - 1]);
        score = b_weight * df;
        collection_trans_prob_.b_trans_prob_[b] += score;

        Trigram t(b, text[i]);
        score = t_weight * df;
        collection_trans_prob_.t_trans_prob_[t] += score;
    }

    if (len >= 2)
    {
#ifdef CN_QC_UNIGRAM
        Unigram u(text[len - 2]);
        score = u_weight * df;
        collection_trans_prob_.u_trans_prob_[u] += score;
#endif

        Bigram b(text[len - 2], text[len - 1]);
        score = b_weight * df;
        collection_trans_prob_.b_trans_prob_[b] += score;
    }

#ifdef CN_QC_UNIGRAM
    if (len >= 1)
    {
        Unigram u(text[len - 1]);
        score = u_weight * df;
        collection_trans_prob_.u_trans_prob_[u] += score;
    }
#endif
}

void CnQueryCorrection::LoadRawTextTransProb_(TransProbType& trans_prob, const std::string& file)
{
    std::ifstream ifs(file.c_str());
    std::string line;
    uint64_t count = 0;
    while (getline(ifs, line))
    {
        std::vector<std::string> vec;
        boost::algorithm::split(vec, line, boost::algorithm::is_any_of("\t"));
        if (vec.size() != 2) continue;
        izenelib::util::UString text(vec[0], izenelib::util::UString::UTF_8);
        double score = boost::lexical_cast<double>(vec[1]);

        switch (text.length())
        {
#ifdef CN_QC_UNIGRAM
        case 1:
        {
            Unigram u(text[0]);
            trans_prob.u_trans_prob_.insert(std::make_pair(u, score));
            break;
        }
#endif
        case 2:
        {
            Bigram b(text[0], text[1]);
            trans_prob.b_trans_prob_.insert(std::make_pair(b, score));
            break;
        }
        case 3:
        {
            Bigram b(text[0], text[1]);
            Trigram t(b, text[2]);
            trans_prob.t_trans_prob_.insert(std::make_pair(t, score));
            break;
        }
        default:
            break;
        }
        ++count;
    }
    ifs.close();
    std::cout << "trans_prob count: " << count << std::endl;
}

void CnQueryCorrection::FlushRawTextTransProb_(const std::string& file, const TransProbType& trans_prob)
{
    uint64_t count = 0;

    std::ofstream ofs(file.c_str());

    // flush unigram
#ifdef CN_QC_UNIGRAM
    for (boost::unordered_map<Unigram, double>::const_iterator it = trans_prob.u_trans_prob_.begin();
            it != trans_prob.u_trans_prob_.end(); ++it)
    {
        izenelib::util::UString text;
        text += it->first;
        std::string str;
        text.convertString(str, izenelib::util::UString::UTF_8);
        ofs << str << "\t" << it->second << std::endl;
        ++count;
    }
#endif

    // flush bigram
    for (boost::unordered_map<Bigram, double>::const_iterator it = trans_prob.b_trans_prob_.begin();
            it != trans_prob.b_trans_prob_.end(); ++it)
    {
        izenelib::util::UString text;
        text += it->first.first;
        text += it->first.second;
        std::string str;
        text.convertString(str, izenelib::util::UString::UTF_8);
        ofs << str << "\t" << it->second << std::endl;
        ++count;
    }

    // flush trigram
    for (boost::unordered_map<Trigram, double>::const_iterator it = trans_prob.t_trans_prob_.begin();
            it != trans_prob.t_trans_prob_.end(); ++it)
    {
        izenelib::util::UString text;
        text += it->first.first.first;
        text += it->first.first.second;
        text += it->first.second;
        std::string str;
        text.convertString(str, izenelib::util::UString::UTF_8);
        ofs << str << "\t" << it->second << std::endl;
        ++count;
    }

    ofs.close();
    std::cout << "trans_prob count: " << count << std::endl;
}

bool CnQueryCorrection::GetResult(const izenelib::util::UString& input, std::vector<izenelib::util::UString>& output)
{
    int type = GetInputType_(input);
    std::vector<CandidateResult> candidate_output;
    if (!GetResultWithScore_(input, type, candidate_output)) return false;
    if (candidate_output.empty())
    {
        if (type < 0) // cn char
        {
            return true;
        }
        else //maybe english
        {
            return false;
        }
    }
#ifdef CN_QC_DEBUG
    for (uint32_t i = 0; i < candidate_output.size(); i++)
    {
        std::string str;
        candidate_output[i].value.convertString(str, izenelib::util::UString::UTF_8);
        std::cout << "[CR] " << str << "\t" << candidate_output[i].score << std::endl;
    }
#endif

    std::sort(candidate_output.begin(), candidate_output.end());
    //check if in the result list
    for (uint32_t i = 0; i != candidate_output.size(); i++)
    {
        if (candidate_output[i].value == input)
        {
            if (i != 0 && candidate_output[i].score < candidate_output[0].score * 0.5)
            {
                candidate_output.resize(i);
                break;
            }
            else
            {
                return false;
            }
        }
    }
    output.push_back(candidate_output[0].value);
    double pre_score = candidate_output[0].score;
    double step = 0.7;
    for (uint32_t i = 1; i < candidate_output.size(); i++)
    {
        double inner_threshold = pre_score * step;
        if (candidate_output[i].score >= inner_threshold)
        {
            output.push_back(candidate_output[i].value);
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
    double score = std::pow(ori_score, 1.0 / text.length());
    score *= pinyin_score;
    return score;

}

bool CnQueryCorrection::IsCandidate_(const izenelib::util::UString& text, double ori_score, double pinyin_score, double& score)
{
    score = GetScore_(text, ori_score, pinyin_score);
    if (text.length() == 1)
        return true;

    if (score < mid_threshold_)
        return false;

    return true;
}

bool CnQueryCorrection::IsCandidateResult_(const izenelib::util::UString& text, double ori_score, double pinyin_score, double& score)
{
    score = GetScore_(text, ori_score, pinyin_score);
    if (score < threshold_) return false;
    else return true;
}

double CnQueryCorrection::TransProbT_(const izenelib::util::UString& from, const izenelib::util::UCS2Char& to)
{
    double r = 0.0;
    if (from.empty())
    {
#ifdef CN_QC_UNIGRAM
        Unigram u(to);
        boost::unordered_map<Unigram, double>::iterator it = global_trans_prob_.u_trans_prob_.find(u);
        if (it != global_trans_prob_.u_trans_prob_.end())
        {
            r = it->second;
        }
        it = collection_trans_prob_.u_trans_prob_.find(u);
        if (it != collection_trans_prob_.u_trans_prob_.end())
        {
            r += it->second;
        }
#else
        r = 1.0;
#endif
    }
    else if (from.length() == 1)
    {
        Bigram b(from[0], to);
        boost::unordered_map<Bigram, double>::iterator it = global_trans_prob_.b_trans_prob_.find(b);
        if (it != global_trans_prob_.b_trans_prob_.end())
        {
            r = it->second;
        }
        it = collection_trans_prob_.b_trans_prob_.find(b);
        if (it != collection_trans_prob_.b_trans_prob_.end())
        {
            r += it->second;
        }
    }
    else
    {
        Bigram b(from[from.length() - 2], from[from.length() - 1]);
        Trigram t(b, to);
        boost::unordered_map<Trigram, double>::iterator it = global_trans_prob_.t_trans_prob_.find(t);
        if (it != global_trans_prob_.t_trans_prob_.end())
        {
            r = it->second;
        }
        it = collection_trans_prob_.t_trans_prob_.find(t);
        if (it != collection_trans_prob_.t_trans_prob_.end())
        {
            r += it->second;
        }
    }

#ifdef CN_QC_DEBUG_DETAIL
    std::string from_str;
    from.convertString(from_str, izenelib::util::UString::UTF_8);
    std::string to_str;
    izenelib::util::UString tmp;
    tmp += to;
    tmp.convertString(to_str, izenelib::util::UString::UTF_8);
    std::cout << "[TP] " << from_str << "," << to_str << " : " << r << std::endl;
#endif

    return r;
}

bool CnQueryCorrection::GetResultWithScore_(const izenelib::util::UString& input, int type, std::vector<CandidateResult>& output)
{
    if (type == 0)
    {
#ifdef CN_QC_DEBUG
        std::cout << "type equals 0" << std::endl;
#endif
        return false;
    }
    std::vector<std::pair<double, std::string> > pinyin_list;
    if (type < 0) // cn chars
    {
        pinyin_.GetPinyin(input, pinyin_list);
#ifdef CN_QC_DEBUG
        std::cout << "Input Hanzi" << std::endl;
        for (uint32_t i = 0; i < pinyin_list.size(); i++)
        {
            std::cout << "[pinyin] " << pinyin_list[i].second << "," << pinyin_list[i].first << std::endl;
        }
#endif
        if (pinyin_list.empty()) return true;
    }
    else // input pinyin
    {
        std::string pinyin_str;
        input.convertString(pinyin_str, izenelib::util::UString::UTF_8);
        pinyin_.FuzzySegmentRaw(pinyin_str, pinyin_list);
#ifdef CN_QC_DEBUG
        std::cout << "Input Pinyin" << std::endl;
        for (uint32_t i = 0; i < pinyin_list.size(); i++)
        {
            std::cout << "[pinyin] " << pinyin_list[i].second << "," << pinyin_list[i].first << std::endl;
        }
#endif
        if (pinyin_list.empty()) return false;
    }
    for (uint32_t i = 0; i < pinyin_list.size(); i++)
    {
        GetResultByPinyinT_(pinyin_list[i].second, pinyin_list[i].first, output);
    }
    return true;
}

void CnQueryCorrection::GetResultByPinyinT_(const std::string& pinyin, double pinyin_score, std::vector<CandidateResult>& output)
{
    uint32_t pinyin_term_count = pinyin_.CountPinyinTerm(pinyin);
    if (pinyin_term_count <= 1 || pinyin_term_count > max_pinyin_term_) return;//ignore single pinyin term
    std::pair<double, izenelib::util::UString> start_mid_result(1.0, izenelib::util::UString("", izenelib::util::UString::UTF_8));
    GetResultByPinyinTRecur_(pinyin, pinyin_score, start_mid_result, output);
}

void CnQueryCorrection::GetResultByPinyinTRecur_(const std::string& pinyin, double pinyin_score, const std::pair<double, izenelib::util::UString>& mid_result, std::vector<CandidateResult>& output)
{
    std::string first_pinyin;
    std::string remain;
    if (pinyin_.GetFirstPinyinTerm(pinyin, first_pinyin, remain))
    {
        std::vector<izenelib::util::UCS2Char> char_list;
        if (!pinyin_.GetChar(first_pinyin, char_list))
            return;

        const izenelib::util::UString& mid_text = mid_result.second;
        std::vector<std::pair<double, izenelib::util::UString> > new_mid_list(char_list.size(), std::make_pair(1.0, mid_text));
        for (uint32_t i = 0; i < char_list.size(); i++)
        {
            new_mid_list[i].first = TransProbT_(mid_text, char_list[i]) * mid_result.first;
            new_mid_list[i].second += char_list[i];
        }

        typedef izenelib::util::first_greater<std::pair<double, izenelib::util::UString> > greater_than;
        std::sort(new_mid_list.begin(), new_mid_list.end(), greater_than());
        double max_score = 0.0;
        for (uint32_t i = 0; i < new_mid_list.size(); i++)
        {
            std::pair<double, izenelib::util::UString>& new_mid_result = new_mid_list[i];
            double score = 1.0;
            uint32_t text_length = new_mid_result.second.length();
            if (!IsCandidate_(new_mid_result.second, new_mid_result.first, pinyin_score, score))
            {
#ifdef CN_QC_DEBUG_DETAIL
                std::string str;
                new_mid_result.second.convertString(str, izenelib::util::UString::UTF_8);
                std::cout << "[BR] " << str << " , " << score << std::endl;
#endif
                break;
            }
            if (i == 0)
            {
                max_score = score;
            }
            else
            {
                double rate = score / max_score;
                if (rate < 0.2 && text_length >= 2)
                    break;
            }
#ifdef CN_QC_DEBUG_DETAIL
            std::string str;
            new_mid_result.second.convertString(str, izenelib::util::UString::UTF_8);
            std::cout << "[NBR] " << str << " , " << score << std::endl;
#endif
            GetResultByPinyinTRecur_(remain, pinyin_score, new_mid_result, output);
        }
    }
    else
    {
        double score = 1.0;
        if (IsCandidateResult_(mid_result.second, mid_result.first, pinyin_score, score))
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
    for (uint32_t i = 0; i < input.length(); i++)
    {
        if (input.isAlphaChar(i))
        {
            if (type < 0)
            {
                type = 0;
                break;
            }
            else
            {
                type = 1;
            }
        }
        else if (input.isChineseChar(i))
        {
            if (type > 0)
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
    }
    return type;
}
