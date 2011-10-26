#include <idmlib/query-correction/fuzzy_pinyin_segmentor.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace idmlib::qc;

FuzzyPinyinSegmentor::FuzzyPinyinSegmentor()
{
    InitRule_();
}

void FuzzyPinyinSegmentor::LoadPinyinFile(const std::string& file)
{
    std::ifstream ifs(file.c_str());
    std::string line;
    while (getline(ifs, line))
    {
        boost::algorithm::trim(line);
        std::vector<std::string> vec;
        boost::algorithm::split(vec, line, boost::algorithm::is_any_of(" "));
        if (vec.size()!=2) continue;
        izenelib::util::UString ustr(line, izenelib::util::UString::UTF_8);
        izenelib::util::UCS2Char& cn_char = ustr[0];
        std::string pinyin = vec[1].substr(0, vec[1].length() - 1);
        if (filter_pinyin_.find(pinyin) == filter_pinyin_.end())
        {
            AddPinyin(pinyin);
        }
        AddPinyinMap(pinyin, cn_char);
    }
    ifs.close();

    pinyin_dict_.Add("chon");
    pinyin_dict_.Add("con");
    pinyin_dict_.Add("don");
    pinyin_dict_.Add("gon");
    pinyin_dict_.Add("hon");
    pinyin_dict_.Add("jion");
    pinyin_dict_.Add("kon");
    pinyin_dict_.Add("lon");
    pinyin_dict_.Add("non");
    pinyin_dict_.Add("qion");
    pinyin_dict_.Add("ron");
    pinyin_dict_.Add("son");
    pinyin_dict_.Add("ton");
    pinyin_dict_.Add("xion");
    pinyin_dict_.Add("yon");
    pinyin_dict_.Add("zhon");
    pinyin_dict_.Add("zon");
}

void FuzzyPinyinSegmentor::AddPinyin(const std::string& pinyin)
{
    pinyin_dict_.Add(pinyin);
}

void FuzzyPinyinSegmentor::AddPinyinMap(const std::string& pinyin, const izenelib::util::UCS2Char& cn_char)
{
    {
        Cn2PinyinType::iterator it = cn2pinyin_.find(cn_char);
        if (it == cn2pinyin_.end())
        {
            std::vector<std::string> pinyin_list(1, pinyin);
            cn2pinyin_.insert(std::make_pair(cn_char, pinyin_list));
        }
        else
        {
            std::vector<std::string>& pinyin_list = it->second;
            std::vector<std::string>::iterator pit = std::find(pinyin_list.begin(), pinyin_list.end(), pinyin);
            if (pit == pinyin_list.end())
            {
                pinyin_list.push_back(pinyin);
            }
        }
    }

    {
        Pinyin2CnType::iterator it = pinyin2cn_.find(pinyin);
        if (it == pinyin2cn_.end())
        {
            std::vector<izenelib::util::UCS2Char> char_list(1, cn_char);
            pinyin2cn_.insert(std::make_pair(pinyin, char_list));
        }
        else
        {
            std::vector<izenelib::util::UCS2Char>& char_list = it->second;
            std::vector<izenelib::util::UCS2Char>::iterator pit = std::find(char_list.begin(), char_list.end(), cn_char);
            if (pit == char_list.end())
            {
                char_list.push_back(cn_char);
            }
        }
    }
}

void FuzzyPinyinSegmentor::InitRule_()
{
    fuzzy_weight_ = 0.5;
    filter_pinyin_.insert(std::make_pair("n", 1));
    filter_pinyin_.insert(std::make_pair("ng", 1));
    filter_pinyin_.insert(std::make_pair("m", 1));
    filter_pinyin_.insert(std::make_pair("o", 1));

    AddSuffixFuzzy_("an", "ang");
    AddSuffixFuzzy_("en", "eng");
    AddSuffixFuzzy_("in", "ing");
    AddSuffixFuzzy_("ue", "ve");

    AddSuffixFuzzy_("ang", "an");
    AddSuffixFuzzy_("eng", "en");
    AddSuffixFuzzy_("ing", "in");
    AddSuffixFuzzy_("ve", "ue");

    AddPrefixFuzzy_("c", "ch");
    AddPrefixFuzzy_("s", "sh");
    AddPrefixFuzzy_("z", "zh");

    AddPrefixFuzzy_("ch", "c");
    AddPrefixFuzzy_("sh", "s");
    AddPrefixFuzzy_("zh", "z");

    pinyin_correction_dict_.push_back(std::make_pair("gn", "ng"));
    pinyin_correction_dict_.push_back(std::make_pair("mg", "ng"));
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

bool FuzzyPinyinSegmentor::FuzzyFilter_(const std::string& pinyin_term, std::vector<std::pair<double, std::string> >& fuzzy_term_list)
{
    {
        std::string::const_reverse_iterator rit = pinyin_term.rbegin();
        uint32_t id = suffix_fuzzy_.GetRootID();
        uint32_t pos = 0;
        while (rit != pinyin_term.rend())
        {
            pos++;
            uint32_t c_id = 0;
            std::pair<bool, bool> r = suffix_fuzzy_.Find(*rit, id, c_id);
            id = c_id;
            if (!r.first) break;
            if (r.second)
            {
                std::string f_term;
                suffix_fuzzy_.GetData(c_id, f_term);
                std::string a = pinyin_term.substr(0, pinyin_term.length() - pos);
                fuzzy_term_list.push_back(std::make_pair(fuzzy_weight_, a + f_term));
                break;
            }
            ++rit;
        }
    }
    {
        std::string::const_iterator it = pinyin_term.begin();
        uint32_t id = suffix_fuzzy_.GetRootID();
        uint32_t pos = 0;
        std::string f_term;
        while (it != pinyin_term.end())
        {
            uint32_t c_id = 0;
            std::pair<bool, bool> r = prefix_fuzzy_.Find(*it, id, c_id);
            id = c_id;
            if (!r.first) break;
            if (r.second)
            {
                prefix_fuzzy_.GetData(c_id, f_term);
            }
            pos++;
            ++it;
        }
        if (!f_term.empty())
        {
            std::string b = pinyin_term.substr(pos);
            fuzzy_term_list.push_back(std::make_pair(fuzzy_weight_, f_term + b));
            if (fuzzy_term_list.size() == 2)
            {
                b = fuzzy_term_list[0].second.substr(pos);
                fuzzy_term_list.push_back(std::make_pair(fuzzy_weight_ * fuzzy_weight_, f_term + b));
            }
        }
    }

    return !fuzzy_term_list.empty();
}

void FuzzyPinyinSegmentor::Segment(const std::string& pinyin_str, std::vector<PinyinSegmentResult>& result_list)
{
    if (pinyin_str.empty()) return;
    uint32_t id = pinyin_dict_.GetRootID();
    for (uint32_t i = 0; i < pinyin_str.length(); i++)
    {
        uint32_t c_id = 0;
        std::pair<bool, bool> r = pinyin_dict_.Find(pinyin_str[i], id, c_id);
        //make maximum match.
        id = c_id;
        if (!r.first) break;
    }
}

void FuzzyPinyinSegmentor::SegmentRaw(const std::string& pinyin_str, std::vector<std::string>& result_list)
{
    SegmentRaw_(pinyin_str, "", result_list);
}

void FuzzyPinyinSegmentor::SegmentRaw_(const std::string& pinyin_str, const std::string& mid_result, std::vector<std::string>& result_list)
{
    if (pinyin_str.empty()) return;
    uint32_t id = pinyin_dict_.GetRootID();

    for (uint32_t i = 0; i < pinyin_str.length(); i++)
    {
        uint32_t c_id = 0;
        char c = pinyin_str[i];

        std::pair<bool, bool> r = pinyin_dict_.Find(c, id, c_id);
        id = c_id;

        //make maximum match.
        if (!r.first) break;
        else
        {
            if (r.second)
            {
                std::string a = pinyin_str.substr(0, i + 1);
                std::string b = pinyin_str.substr(i + 1);
                std::string n_mid_result = mid_result;
                if (!n_mid_result.empty()) n_mid_result.push_back(',');
                n_mid_result += a;
                if (!b.empty())
                {
                    SegmentRaw_(b, n_mid_result, result_list);
                }
                else
                {
                    n_mid_result.push_back(',');
                    result_list.push_back(n_mid_result);
                }
            }
        }
    }
}

void FuzzyPinyinSegmentor::FuzzySegmentRaw(const std::string& pinyin_str, std::vector<std::string>& result_list)
{
    std::vector<std::pair<double, std::string> > score_result_list;
    FuzzySegmentRaw(pinyin_str, score_result_list);
    result_list.resize(score_result_list.size());
    for (uint32_t i = 0; i < score_result_list.size(); i++)
    {
        result_list[i] = score_result_list[i].second;
    }
}

void FuzzyPinyinSegmentor::FuzzySegmentRaw(const std::string& pinyin_str, std::vector<std::pair<double, std::string> >& result_list)
{
    if (pinyin_str.empty())
        return;

    std::vector<std::string> r_result_list;
    CorrectPinyin_((std::string &) pinyin_str);
    SegmentRaw(pinyin_str, r_result_list);
    for (uint32_t i = 0; i < r_result_list.size(); i++)
    {
        //TODO
        FuzzySegmentRaw_(r_result_list[i], "", 1.0, result_list);
    }
}

void FuzzyPinyinSegmentor::CorrectPinyin_(std::string& pinyin_str)
{
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = pinyin_correction_dict_.begin();
            it != pinyin_correction_dict_.end(); ++it)
        boost::replace_all(pinyin_str, it->first, it->second);
}

uint32_t FuzzyPinyinSegmentor::CountPinyinTerm(const std::string& pinyin)
{
    if (pinyin.empty()) return 0;
    uint32_t count = 1;
    std::size_t start = 0;
    while (true)
    {
        std::size_t pos = pinyin.find(',', start);
        if (pos!= std::string::npos)
        {
            start = pos + 1;
            ++count;
        }
        else
        {
            break;
        }
    }
    return count;
}

bool FuzzyPinyinSegmentor::GetPinyinTerm(const izenelib::util::UCS2Char& cn_char, std::vector<std::string>& result_list)
{
    Cn2PinyinType::iterator it = cn2pinyin_.find(cn_char);
    if (it != cn2pinyin_.end())
    {
        result_list = it->second;
        return true;
    }
    return false;
}

bool FuzzyPinyinSegmentor::GetChar(const std::string& pinyin_term, std::vector<izenelib::util::UCS2Char>& result_list)
{
    Pinyin2CnType::iterator it = pinyin2cn_.find(pinyin_term);
    if (it!=pinyin2cn_.end())
    {
        result_list = it->second;
        return true;
    }
    return false;
}

void FuzzyPinyinSegmentor::GetPinyin(const izenelib::util::UString& cn_chars, std::vector<std::pair<double, std::string> >& result_list)
{
    if (cn_chars.empty())
        return;

    GetPinyin_(cn_chars, "", 1.0, result_list);
}

void FuzzyPinyinSegmentor::GetPinyin(const izenelib::util::UString& cn_chars, std::vector<std::string>& result_list)
{
    std::vector<std::pair<double, std::string> > score_result_list;
    GetPinyin(cn_chars, score_result_list);
    result_list.resize(score_result_list.size());
    for (uint32_t i = 0; i < score_result_list.size(); i++)
    {
        result_list[i] = score_result_list[i].second;
    }
}

void FuzzyPinyinSegmentor::GetPinyin_(const izenelib::util::UString& cn_chars, const std::string& mid_result, double score, std::vector<std::pair<double, std::string> >& result_list)
{
    if (result_list.size() >= 1024)
        return;

    std::vector<std::string> pinyin_term_list;
    if (!cn_chars.empty() && GetPinyinTerm(cn_chars[0], pinyin_term_list))
    {
        izenelib::util::UString remain = cn_chars.substr(1);
        std::string new_mid(mid_result);
        if (!new_mid.empty())
            new_mid.push_back(',');

        for (uint32_t i = 0; i < pinyin_term_list.size(); i++)
        {
            std::string mid = new_mid + pinyin_term_list[i];
            GetPinyin_(remain, mid, score, result_list);
            std::vector<std::pair<double, string> > f_term_list;
            if (FuzzyFilter_(pinyin_term_list[i], f_term_list))
            {
                for (uint32_t j = 0; j < f_term_list.size(); j++)
                    GetPinyin_(remain, new_mid + f_term_list[j].second, score * f_term_list[j].first, result_list);
            }
        }
    }
    else
    {
        result_list.push_back(std::make_pair(score, mid_result));
    }
}

bool FuzzyPinyinSegmentor::GetFirstPinyinTerm(const std::string& pinyin, std::string& first, std::string& remain)
{
    if (pinyin.empty()) return false;
    std::size_t pos = pinyin.find(',');
    if (pos!=std::string::npos)
    {
        first = pinyin.substr(0, pos);
        remain = pinyin.substr(pos + 1);
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
    if (pinyin.empty()) return false;
    std::size_t pos = pinyin.find(',');
    if (pos!=std::string::npos)
    {
        first = pinyin.substr(0, pos);
        pinyin = pinyin.substr(pos + 1);
    }
    else
    {
        first = pinyin;
        pinyin = "";
    }
    return true;
}

void FuzzyPinyinSegmentor::FuzzySegmentRaw_(const std::string& pinyin_str, const std::string& mid_result, double score, std::vector<std::pair<double, std::string> >& result_list)
{
    if (result_list.size() >= 1024)
        return;

    std::string first;
    std::string remain;
    boost::replace_all((string &) pinyin_str, "on,", "ong,");
    if (GetFirstPinyinTerm(pinyin_str, first, remain))
    {
        std::string new_mid(mid_result);
        if (!new_mid.empty())
            new_mid.push_back(',');

        FuzzySegmentRaw_(remain, new_mid + first, score, result_list);
        std::vector<std::pair<double, std::string> > f_term_list;
        if (FuzzyFilter_(first, f_term_list))
        {
            for (size_t i = 0; i < f_term_list.size(); i++)
                FuzzySegmentRaw_(remain, new_mid + f_term_list[i].second, score * f_term_list[i].first, result_list);
        }
    }
    else
    {
        result_list.push_back(std::make_pair(score, mid_result));
    }
}
