#ifndef ADDRLIB_ADDRESSEXTRACT_H
#define ADDRLIB_ADDRESSEXTRACT_H 
#include <util/ustring/UString.h>
#include <am/sequence_file/ssfr.h>
#include <idmlib/util/idm_analyzer.h>
#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <boost/algorithm/string.hpp>
//#define ADDR_DEBUG

NS_IDMLIB_B5M_BEGIN
class AddressExtract{
    enum SUFFIX_STATUS {NOT_SUFFIX, SUFFIX_NEXT, SUFFIX_BREAK, SUFFIX_DICT};
    enum ADDR_TYPE {NOT_ADDR = 1, PROVINCE, CITY, COUNTY, DISTRICT, ROAD, POI};
    enum FLAG_TYPE {LEFT_CONTEXT=101, RIGHT_CONTEXT, SUFFIX, ADDR};
    typedef uint32_t term_t;
    typedef std::vector<term_t> word_t;
    typedef std::pair<word_t, ADDR_TYPE> token_t;
    typedef std::map<word_t, ADDR_TYPE> Dict;
    typedef std::map<word_t, double> Prob;
    typedef std::map<word_t, std::size_t> Count;
    typedef std::map<std::string, std::string> SynDict;
    typedef std::map<term_t, std::string> TermText;
    typedef izenelib::util::UString UString;
    struct TrainItem
    {
        std::string text;
        ADDR_TYPE type;
        word_t word;
    };
    struct Node
    {
        std::size_t start;
        std::size_t end;
        term_t type;
        double prob;
        std::string text;
        bool operator<(const Node& n) const
        {
            if(type!=n.type) return type<n.type;
            return start>n.start;
        }
    };
    struct SuffixTerm
    {
        term_t term;
        std::string text;
        ADDR_TYPE type;
        bool is_suffix;
        double prob;
        std::size_t len;
        bool in_dict;

        SuffixTerm() : term(0), type(NOT_ADDR), is_suffix(false), prob(0.0), len(0), in_dict(false)
        {
        }
    };
    typedef std::vector<SuffixTerm> SuffixWord;
    typedef std::vector<Node> NodeList;
    typedef std::vector<NodeList> NodeMatrix;
    //enum {STATUS_NO, STATUS_NEXT, STATUS_YES};
public:
    AddressExtract()
    {
        ADDR_LIST.push_back(PROVINCE);
        ADDR_LIST.push_back(CITY);
        ADDR_LIST.push_back(COUNTY);
        ADDR_LIST.push_back(DISTRICT);
        ADDR_LIST.push_back(ROAD);
        ADDR_LIST.push_back(POI);
        idmlib::util::IDMAnalyzerConfig csconfig = idmlib::util::IDMAnalyzerConfig::GetCommonConfig("","", "");
        csconfig.symbol = true;
        analyzer_ = new idmlib::util::IDMAnalyzer(csconfig);
        AddPredefineSuffix("镇", POI);
        AddPredefineSuffix("村", POI);
        AddPredefineSuffix("号", POI);
        AddPredefineSuffix("室", POI);
        AddPredefineSuffix("大道", ROAD);
        user_syn_["一号"] = "1号";
        user_syn_["二号"] = "2号";
        user_syn_["三号"] = "3号";
        user_syn_["四号"] = "4号";
        user_syn_["五号"] = "5号";
        user_syn_["六号"] = "6号";
        user_syn_["七号"] = "7号";
        user_syn_["八号"] = "8号";
        user_syn_["九号"] = "9号";
        user_syn_["十号"] = "10号";
        num_road_regex_ = boost::regex("^.*?([\\d\\-]*?\\d+号)$");
        pure_road_regex_ = boost::regex("^[\\d\\-]*?\\d+号$");
    }
    void AddPredefineSuffix(const std::string& suffix, ADDR_TYPE type)
    {
        static const double predefine_prob = 0.1;
        word_t word(1, SUFFIX);
        word.push_back(type);
        word_t w;
        GetWord_(suffix, w, false);
        if(w.size()>2 || w.size()<1) return;
        word.push_back(w.back());
        if(w.size()>1) word.push_back(w.front());
        suffix_prob_.insert(std::make_pair(word, predefine_prob));
    }
    ~AddressExtract()
    {
        delete analyzer_;
    }

    void Process(const std::string& knowledge)
    {
        Train(knowledge);
        std::string train_file = knowledge+"/train";
        std::ifstream ifst(train_file.c_str());
        std::string line;
        while( getline(ifst, line) )
        {
            boost::algorithm::trim(line);
            if(line.empty()) continue;
            std::string r = Evaluate(line);
            std::cout<<line<<std::endl;
            std::cout<<"\t"<<r<<std::endl;
        }
        ifst.close();
    }

    bool Train(const std::string& knowledge)
    {
        namespace bfs = boost::filesystem;
        std::string resource_path = knowledge+"/resource";
        if(Load_(resource_path))
        {
            LOG(INFO)<<"Load "<<resource_path<<" finish"<<std::endl;
            return true;
        }
        std::string dict = knowledge+"/dict";
        std::string syn_dict = knowledge+"/dict.syn";
        std::string train_file = knowledge+"/train";
        if(!bfs::exists(dict)) return false;
        if(!bfs::exists(syn_dict)) return false;
        if(!bfs::exists(train_file)) return false;
        std::string line;
        std::ifstream sifs(syn_dict.c_str());
        while( getline(sifs, line) )
        {
            boost::algorithm::trim(line);
            std::vector<std::string> vec;
            boost::algorithm::split(vec, line, boost::algorithm::is_any_of("@"));
            if(vec.size()!=2) continue;
            std::string low = vec[0];
            std::string high = vec[1];
            if(low==high) continue;
            if(low>high) std::swap(low, high);
            syn_dict_[low] = high;
            syn_dict_[high] = low;
        }
        sifs.close();
        std::vector<std::pair<ADDR_TYPE, std::string> > syn_fields;
        syn_fields.push_back(std::make_pair(ROAD, "路"));
        std::ifstream ifs(dict.c_str());
        std::vector<TrainItem> item_list;
        while( getline(ifs, line))
        {
            boost::algorithm::trim(line);
            std::vector<std::string> vec;
            boost::algorithm::split(vec, line, boost::algorithm::is_any_of("@"));
            if(vec.size()!=2) continue;
            const std::string& str = vec[0];
            word_t word;
            GetWord_(str, word, false);
            word_t rword(word.rbegin(), word.rend());
            ADDR_TYPE type = NOT_ADDR;
            if(vec[1]=="province")
            {
                type = PROVINCE;
            }
            else if(vec[1]=="city")
            {
                type = CITY;
            }
            else if(vec[1]=="county")
            {
                type = COUNTY;
            }
            else if(vec[1]=="district")
            {
                type = DISTRICT;
            }
            else if(vec[1]=="road")
            {
                type = ROAD;
            }
            else if(vec[1]=="poi")
            {
                type = POI;
            }
            if(type==POI&&word.size()<=2) continue;
            //if(type==ROAD&&word.size()<=2) continue;
            TrainItem item;
            item.text = str;
            item.type = type;
            item.word = word;
            item_list.push_back(item);
            dict_[word] = type;
            rdict_[rword] = type;
        }
        for(uint32_t i=0;i<item_list.size();i++)
        {
            const TrainItem& item = item_list[i];
            const std::string& str = item.text;
            ADDR_TYPE type = item.type;
            const word_t& word = item.word;
            word_t syn_word;
            SynDict::const_iterator sdit = syn_dict_.find(str);
            if(sdit!=syn_dict_.end())
            {
                const std::string& syn = sdit->second;
                GetWord_(syn, syn_word, false);
            }
            if(syn_word.empty())
            {
                std::string syn_str;
                for(std::size_t f=0;f<syn_fields.size();f++)
                {
                    const std::pair<ADDR_TYPE, std::string>& field = syn_fields[f];
                    if(type==field.first&&word.size()>=4&&boost::algorithm::ends_with(str, field.second))
                    {
                        std::size_t p = str.find(field.second);
                        syn_str = str.substr(0, p);
                        GetWord_(syn_str, syn_word, false);
                        break;
                    }
                }
                if(dict_.find(syn_word)!=dict_.end())
                {
                    syn_word.clear();
                    syn_str.clear();
                }
                if(!syn_str.empty())
                {
                    syn_dict_[str] = syn_str;
                    syn_dict_[syn_str] = str;
                }
            }
            if(!syn_word.empty())
            {
                word_t rsyn_word(syn_word.rbegin(), syn_word.rend());
                dict_[syn_word] = type;
                rdict_[rsyn_word] = type;
            }
        }
        std::cerr<<"dict size "<<dict_.size()<<std::endl;
        std::cerr<<"rdict size "<<rdict_.size()<<std::endl;
        ifs.close();
        std::ifstream ifst(train_file.c_str());
        while( getline(ifst, line) )
        {
            boost::algorithm::trim(line);
            if(line.empty()) continue;
            ProcessTrainAddr_(line);
        }
        ifst.close();
        for(Count::const_iterator it=count_.begin();it!=count_.end();++it)
        {
            const word_t& key = it->first;
            FLAG_TYPE type = (FLAG_TYPE)key[0];
            if(type==LEFT_CONTEXT || type==RIGHT_CONTEXT)
            {
                term_t addr = key[1];
                word_t addrv(1, addr);
                double p = (double)it->second/count_[addrv];
                prob_[key] = p;
            }
            else if(type==SUFFIX)
            {
                //term_t term = key.back();
                term_t addr = key[1];
                word_t addrv(1, addr);
                double p = (double)it->second/count_[addrv];
                prob_[key] = p;
            }
        }
        Save_(resource_path);
        return true;
    }

    //void Evaluate(const std::string& address)
    //{
    //    word_t word;
    //    GetWord_(address, word);
    //    word_t rword(word.rbegin(), word.rend());
    //    std::vector<token_t> tokens;
    //    GetTokens_(word, tokens);
    //    for(uint32_t i=0;i<tokens.size();i++)
    //    {
    //        const token_t& token = tokens[i];
    //        if(token.second!=NOT_ADDR) continue;
    //        const word_t& word = token.first;
    //        std::string wtext = GetText_(word);
    //        for(uint32_t w=1;w<word.size();w++)
    //        {
    //            term_t term = word[w];
    //            word_t left;
    //            if(i>0)
    //            {
    //                left.push_back(tokens[i-1].first.back());
    //            }
    //            left.push_back(word[w-1]);
    //            word_t right;
    //            if(w<word.size()-1) right.push_back(word[w+1]);
    //            if(w<word.size()-2) right.push_back(word[w+2]);
    //            word_t textw(word.begin(), word.begin()+w+1);
    //            std::string text = GetText_(textw);
    //            std::string left_text;
    //            if(!left.empty()) left_text = GetText_(left);
    //            std::string right_text;
    //            if(!right.empty()) right_text = GetText_(right);
    //            for(uint32_t l=0;l<ADDR_LIST.size();l++)
    //            {
    //                word_t key(3);
    //                key[0] = SUFFIX;
    //                key[1] = ADDR_LIST[l];
    //                key[2] = term;
    //                Prob::const_iterator it = prob_.find(key);
    //                double prob = 0.0;
    //                if(it!=prob_.end())
    //                {
    //                    prob = it->second;
    //                }
    //                if(prob<0.2) continue;
    //                double ru_prob = 0.0;
    //                double rb_prob = 0.0;
    //                if(right.size()>=1)
    //                {
    //                    key.resize(3);
    //                    key[0] = RIGHT_CONTEXT;
    //                    key[1] = ADDR_LIST[l];
    //                    key[2] = right[0];
    //                    it = prob_.find(key);
    //                    if(it!=prob_.end())
    //                    {
    //                        ru_prob = it->second;
    //                    }
    //                }
    //                if(right.size()>=2)
    //                {
    //                    key.resize(4);
    //                    key[0] = RIGHT_CONTEXT;
    //                    key[1] = ADDR_LIST[l];
    //                    key[2] = right[0];
    //                    key[3] = right[1];
    //                    it = prob_.find(key);
    //                    if(it!=prob_.end())
    //                    {
    //                        rb_prob = it->second;
    //                    }
    //                }
    //                double lu_prob = 0.0;
    //                double lb_prob = 0.0;
    //                if(left.size()>=1)
    //                {
    //                    key.resize(3);
    //                    key[0] = LEFT_CONTEXT;
    //                    key[1] = ADDR_LIST[l];
    //                    key[2] = left[0];
    //                    it = prob_.find(key);
    //                    if(it!=prob_.end())
    //                    {
    //                        lu_prob = it->second;
    //                    }
    //                }
    //                if(left.size()>=2)
    //                {
    //                    key.resize(4);
    //                    key[0] = LEFT_CONTEXT;
    //                    key[1] = ADDR_LIST[l];
    //                    key[2] = left[0];
    //                    key[3] = left[1];
    //                    it = prob_.find(key);
    //                    if(it!=prob_.end())
    //                    {
    //                        lb_prob = it->second;
    //                    }
    //                }
    //                std::cerr<<"find candidate "<<text<<","<<left_text<<","<<right_text<<" in "<<ADDR_LIST[l]<<" : "<<prob<<","<<lu_prob<<","<<lb_prob<<","<<ru_prob<<","<<rb_prob<<std::endl;
    //                std::cerr<<"\tall text "<<wtext<<std::endl;
    //                if(i>0)
    //                {
    //                    std::cerr<<"\tleft addr "<<GetText_(tokens[i-1].first)<<","<<tokens[i-1].second<<std::endl;
    //                }
    //            }
    //        }
    //    }
    //}


private:
    bool Load_(const std::string& resource)
    {
        std::string file = resource+"/dict";
        namespace bfs = boost::filesystem;
        if(!bfs::exists(file)) return false;
        izenelib::am::ssf::Util<>::Load(file, rdict_);
        file = resource+"/syn";
        if(!bfs::exists(file)) return false;
        izenelib::am::ssf::Util<>::Load(file, syn_dict_);
        file = resource+"/prob";
        if(!bfs::exists(file)) return false;
        izenelib::am::ssf::Util<>::Load(file, prob_);
        return true;
    }
    void Save_(const std::string& resource)
    {
        namespace bfs = boost::filesystem;
        bfs::remove_all(resource);
        bfs::create_directories(resource);
        std::string file = resource+"/dict";
        izenelib::am::ssf::Util<>::Save(file, rdict_);
        file = resource+"/syn";
        izenelib::am::ssf::Util<>::Save(file, syn_dict_);
        file = resource+"/prob";
        izenelib::am::ssf::Util<>::Save(file, prob_);
    }
    std::string Clean_(const std::string& addr)
    {
        std::vector<std::pair<std::string, std::string> > ranges;
        ranges.push_back(std::make_pair("(", ")"));
        ranges.push_back(std::make_pair("（", "）"));
        std::string result;
        for(std::size_t i=0;i<ranges.size();i++)
        {
            const std::string& begin = ranges[i].first;
            std::size_t p1 = addr.find(begin);
            if(p1==std::string::npos) continue;
            const std::string end = ranges[i].second;
            std::size_t p2 = addr.find(end);
            if(p2==std::string::npos) continue;
            if(p1>=p2) continue;
            result = addr.substr(0, p1);
            result += addr.substr(p2+1);
            break;
        }
        if(result.empty()) result = addr;
        return result;
    }

    bool LostTooMuch_(std::size_t cover, const SuffixWord& word) const
    {
        double ratio = (double)cover/word.size();
        if(word.size()<=10)
        {
            if(ratio<0.6) return true;
        }
        else if(word.size()<=20)
        {
            if(ratio<0.3) return true;
        }
        else
        {
            if(ratio<0.3) return true;
        }
        //std::size_t addr_len = word.size();
        //std::size_t return_len = 0;
        //for(std::size_t i=0;i<nl.size();i++)
        //{
        //    return_len += (nl[i].end-nl[i].start);
        //}
        //double ratio = (double)return_len/addr_len;
        //if(addr_len<=10)
        //{
        //    if(ratio<0.6) return true;
        //}
        return false;
    }

    std::size_t PostProcessResult_(NodeList& nl, const SuffixWord& word)
    {
        for(uint32_t i=0;i<nl.size();i++)
        {
            Node& node = nl[i];
            node.text = GetText_(word, node.start, node.end);
            SynDict::const_iterator it = syn_dict_.find(node.text);
            if(it!=syn_dict_.end())
            {
                if(it->second>node.text)
                {
                    node.text = it->second;
                }
            }
            it = user_syn_.find(node.text);
            if(it!=user_syn_.end())
            {
                node.text = it->second;
            }
            node.text = boost::regex_replace(node.text, num_road_regex_, "\\1");
        }
        std::sort(nl.begin(), nl.end());
        std::vector<std::size_t> app(word.size(), 0);
        bool changed = false;
        for(uint32_t i=0;i<nl.size();i++)
        {
            Node& ni = nl[i];
            for(std::size_t f=ni.start;f<ni.end;f++)
            {
                app[f]++;
            }
            if(ni.text.empty()) continue;
            for(uint32_t j=i+1;j<nl.size();j++)
            {
                Node& nj = nl[j];
                if(nj.text.empty()) continue;
                if(ni.text==nj.text)
                {
                    changed = true;
                    if(nj.prob>ni.prob) ni.text.clear();
                    else nj.text.clear();
                }
            }
        }
        std::size_t result=0;
        for(std::size_t a=0;a<app.size();a++)
        {
            if(app[a]>0) result++;
        }
        if(changed)
        {
            NodeList newlist;
            for(uint32_t i=0;i<nl.size();i++)
            {
                if(!nl[i].text.empty()) newlist.push_back(nl[i]);
            }
            std::swap(nl, newlist);
        }
        return result;
    }
    void GetSuffixWord_(const std::string& text, SuffixWord& word)
    {
        typedef std::vector<idmlib::util::IDMTerm> TermList;
        TermList term_list;
        UString utext(text, UString::UTF_8);
        analyzer_->GetTermList(utext, term_list);
        for(TermList::const_reverse_iterator it = term_list.rbegin(); it!=term_list.rend(); ++it)
        {
            SuffixTerm st;
            st.text = it->TextString();
            if(it->tag==idmlib::util::IDMTermTag::SYMBOL)
            {
                if(st.text!="-") continue;
            }
            st.term = izenelib::util::HashFunction<std::string>::generateHash32(st.text);
            word.push_back(st);
#ifdef ADDR_DEBUG
            term_text_[st.term] = st.text;
#endif
        }
        for(uint32_t i=0;i<word.size();i++)
        {
            SuffixTerm& st = word[i];
            std::size_t select_j = 0;
            double max_weight = 0.0;
            ADDR_TYPE max_type=NOT_ADDR;
            SUFFIX_STATUS max_status = NOT_SUFFIX;
            std::size_t last_dict_j = 0;
            for(std::size_t j=i+2;j<=word.size();j++)
            {
                word_t frag(j-i);
                for(std::size_t f=i;f<j;f++)
                {
                    frag[f-i] = word[f].term;
                }
                word_t left;
                if(j<word.size()) left.push_back(word[j].term);
                if(j<word.size()-1) 
                {
                    left.push_back(word[j+1].term);
                    //std::swap(left[0], left[1]);
                }
                double weight = 0.0;
                ADDR_TYPE type = NOT_ADDR;
                SUFFIX_STATUS status = NOT_SUFFIX;
                if(last_dict_j>0 && j-last_dict_j>2) status = NOT_SUFFIX;
                else
                {
                    status = GetSuffixStatus_(frag, left, type, weight);
                }
                //this is a hook
                std::string frag_str = GetText_(word, i, j);
                if(type==POI&&!left.empty())
                {
                    std::string left_str = GetText_(word, j, j+1);
                    bool valid = true;
                    if(!left_str.empty())
                    {
                        if(boost::algorithm::ends_with(left_str, "-")) valid = false;
                        else
                        {
                            char lc = left_str[left_str.length()-1];
                            if(lc>='0'&&lc<='9') valid = false;
                        }
                    }
                    if(valid)
                    {
                        if(boost::regex_match(frag_str, pure_road_regex_))
                        {
                            status = SUFFIX_BREAK;
                            weight = 0.2;
                        }
                    }
                }

                int force_select = 0;
                if(last_dict_j>0)
                {
                    if(status==SUFFIX_NEXT) force_select = -1;
                    else if(status==SUFFIX_DICT)
                    {
                        force_select = 1;
                    }
                }
#ifdef ADDR_DEBUG
                //std::string frag_str = GetText_(word, i, j);
                std::cerr<<"[SUFFIX_STATUS]"<<frag_str<<","<<status<<","<<type<<","<<weight<<std::endl;
#endif
                if(status==SUFFIX_DICT||status==SUFFIX_NEXT||status==SUFFIX_BREAK)
                {
                    if(force_select>=0 && (weight>max_weight || force_select>0))
                    {
                        max_weight = weight;
                        max_type = type;
                        select_j = j;
                        max_status = status;
                    }
                }
                if(status==NOT_SUFFIX || status==SUFFIX_BREAK) break;
                if(status==SUFFIX_DICT) last_dict_j = j;
            }
            if(select_j==0) //not found
            {
            }
            else
            {
                st.type = max_type;
                st.is_suffix = true;
                st.prob = max_weight;
                st.len = select_j-i;
                if(max_status==SUFFIX_DICT)
                {
                    st.in_dict = true;
                }
#ifdef ADDR_DEBUG
                std::cerr<<"[FIND]"<<GetText_(word, i, select_j)<<std::endl;
#endif
            }
        }
    }
    void Recursive_(const SuffixWord& word, std::size_t start, std::size_t maxi_limit, NodeList& current, NodeMatrix& global)
    {
#ifdef ADDR_DEBUG
        std::string sw = GetText_(word, start, word.size());
        std::cerr<<"[D1]"<<sw<<","<<start<<","<<maxi_limit<<std::endl;
#endif
        std::size_t i=start;
        while(i<word.size())
        {
            const SuffixTerm& st = word[i];
#ifdef ADDR_DEBUG
            std::cerr<<"[SUFFIXTERM]"<<i<<","<<st.is_suffix<<","<<st.len<<","<<st.type<<","<<st.prob<<std::endl;
#endif
            if(st.is_suffix&&st.len>0)
            {
#ifdef ADDR_DEBUG
                std::cerr<<"[FIND]"<<GetText_(word, i, i+st.len)<<std::endl;
#endif
                NodeList bc(current);
                Node node;
                node.start = i;
                node.end = i+st.len;
                node.type = st.type;
                node.prob = st.prob;
                current.push_back(node);
                Recursive_(word, i+st.len, 0, current, global);
                if(st.len>2)
                {
                    Recursive_(word, i+1, i+st.len, bc, global);
                }
                break;
            }
            else
            {
                //bool flag = false;
                //if(!current.empty())
                //{
                //    Node& n = current.back();
                //    if(n.type==NOT_ADDR&&n.end>=i) 
                //    {
                //        n.end = i+1;
                //        flag = true;
                //    }
                //}
                //if(!flag)
                //{
                //    Node node;
                //    node.start = i;
                //    node.end = i+1;
                //    node.type = NOT_ADDR;
                //    current.push_back(node);
                //}
                ++i;
                if(maxi_limit>0&&i>=maxi_limit) break;
                if(i>=word.size()) break;
            }
        }
        if(i>=word.size())//finish search
        {
            global.push_back(current);
        }
    }
//    void Recursive_(const word_t& word, std::size_t start, std::size_t maxi_limit, NodeList& current, NodeMatrix& global)
//    {
//        word_t cword(word.begin()+start, word.end());
//        word_t fword(cword.rbegin(), cword.rend());
//        std::string sw = GetText_(fword);
//#ifdef ADDR_DEBUG
//        std::cerr<<"[D1]"<<sw<<","<<start<<","<<maxi_limit<<std::endl;
//#endif
//        std::size_t i=start;
//        while(true)
//        {
//            //term_t term = word[i];
//            term_t type=NOT_ADDR;
//            std::size_t select_j = 0;
//            double max_weight = 0.0;
//            term_t max_type=NOT_ADDR;
//            for(std::size_t j=i+2;j<=word.size();j++)
//            {
//                word_t frag(word.begin()+i, word.begin()+j);
//                word_t left;
//                if(j<word.size()) left.push_back(word[j]);
//                if(j<word.size()-1) 
//                {
//                    left.push_back(word[j+1]);
//                    std::swap(left[0], left[1]);
//                }
//                double weight = 0.0;
//                int status = IsCandidate_(frag, left, type, weight);
//#ifdef ADDR_DEBUG
//                word_t ffrag(frag.rbegin(), frag.rend());
//                std::cerr<<"[F]"<<GetText_(ffrag)<<","<<status<<std::endl;
//#endif
//                if(status==STATUS_YES||status==STATUS_NEXT)
//                {
//                    if(weight>max_weight)
//                    {
//                        max_weight = weight;
//                        max_type = type;
//                        select_j = j;
//                    }
//                }
//                if(status==STATUS_YES || status==STATUS_NO) break;
//            }
//#ifdef ADDR_DEBUG
//            std::cerr<<"[SJ]"<<i<<","<<select_j<<std::endl;
//#endif
//            if(select_j>0)
//            {
//#ifdef ADDR_DEBUG
//                word_t find_word(word.begin()+i, word.begin()+select_j);
//                word_t ffind_word(find_word.rbegin(), find_word.rend());
//                std::cerr<<"[FIND]"<<GetText_(ffind_word)<<std::endl;
//#endif
//                NodeList bc(current);
//                Node node;
//                node.start = i;
//                node.end = select_j;
//                node.type = type;
//                current.push_back(node);
//                Recursive_(word, select_j, 0, current, global);
//                if(select_j-i>2)
//                {
//                    Recursive_(word, i+1, select_j, bc, global);
//                }
//                break;
//            }
//            else
//            {
//                ++i;
//                if(maxi_limit>0&&i>=maxi_limit) break;
//                if(i>=word.size()) break;
//            }
//        }
//        if(i>=word.size())//finish search
//        {
//            global.push_back(current);
//        }
//    }
    bool IsValidSpecifySuffix_(term_t term, term_t type, double& prob)
    {
        word_t key(3);
        key[0] = SUFFIX;
        key[1] = type;
        key[2] = term;
        Prob::const_iterator it = prob_.find(key);
        if(it!=prob_.end())
        {
            prob = it->second;
        }
        if(prob<0.2) return false;
        return true;
    }
    bool IsValidSuffix_(term_t term, term_t& type)
    {
        std::pair<double, term_t> max_type(0.0, NOT_ADDR);
        for(uint32_t l=0;l<ADDR_LIST.size();l++)
        {
            term_t type = ADDR_LIST[l];
            double prob = 0.0;
            if(!IsValidSpecifySuffix_(term, type, prob)) continue;
            else
            {
                if(prob>max_type.first)
                {
                    max_type.first = prob;
                    max_type.second = type;
                }
            }
        }
        if(max_type.first>0.0)
        {
            type = max_type.second;
            return true;
        }
        return false;
    }
    double GetSuffixProb_(const word_t& key) const
    {
        Prob::const_iterator it = suffix_prob_.find(key);
        if(it!=suffix_prob_.end()) return it->second;
        it = prob_.find(key);
        if(it!=prob_.end()) return it->second;
        return 0.0;
    }
    ADDR_TYPE GetSuffixType_(const word_t& word, std::size_t d, double& prob)
    {
        if(word.size()<=d) return NOT_ADDR;
        std::pair<double, ADDR_TYPE> select(0.0, NOT_ADDR);
        for(std::size_t i=0;i<ADDR_LIST.size();i++)
        {
            ADDR_TYPE type = ADDR_LIST[i];
            word_t key(1, SUFFIX);
            key.push_back(type);
            key.push_back(word[d]);
            double uprob = GetSuffixProb_(key);
            double bprob = 0.0;
#ifdef ADDR_DEBUG
            //std::cerr<<"[SP-U]"<<(int)type<<","<<GetText_(word[d])<<","<<uprob<<std::endl;
#endif
            if(word.size()>d+1) 
            {
                key.push_back(word[d+1]);
                bprob = GetSuffixProb_(key);
                bprob *= 2;
#ifdef ADDR_DEBUG
                //std::cerr<<"[SP-B]"<<(int)type<<","<<GetText_(word[d+1])<<","<<bprob<<std::endl;
#endif
            }
            double cprob = bprob>uprob ? bprob : uprob;
            if(cprob>select.first)
            {
                select.first = cprob;
                select.second = type;
            }
        }
        if(select.first>=0.1)
        {
            prob = select.first;
            return select.second;
        }
        return NOT_ADDR;
    }

    SUFFIX_STATUS GetSuffixStatus_(const word_t& tword, const word_t& left, ADDR_TYPE& type, double& weight)
    {
        double suffix_prob = 0.0;
        ADDR_TYPE suffix_type = GetSuffixType_(tword, 0, suffix_prob);
        Dict::const_iterator it = rdict_.lower_bound(tword);
        if(it!=rdict_.end())
        {
            if(boost::algorithm::starts_with(it->first, tword))
            {
                if(it->first==tword)
                {
                    type = it->second;
                    weight = 1.0;
                    return SUFFIX_DICT;
                }
                else 
                {
                    if(suffix_type==POI && suffix_prob>0.0)
                    {
                    }
                    else
                    {
                        type = it->second;
                        weight = 0.0;
#ifdef ADDR_DEBUG
                        std::cerr<<"[SUFFIX-NEXT]"<<GetText_(tword)<<","<<type<<","<<weight<<","<<GetText_(it->first)<<std::endl;
#endif
                        return SUFFIX_NEXT;
                    }
                }
            }
        }
        if(suffix_type==NOT_ADDR||suffix_type==PROVINCE||suffix_type==CITY)
        {
            return NOT_SUFFIX;
        }
        type = suffix_type;
        if(left.empty())
        {
            //if(type==PROVINCE || type==CITY || type==COUNTY || type==DISTRICT) return STATUS_YES;
            weight = 0.8;
            return SUFFIX_BREAK;
        }
        double left_suffix_prob = 0.0;
        ADDR_TYPE left_suffix_type = GetSuffixType_(left, 0, left_suffix_prob);
        if(left_suffix_type!=NOT_ADDR) 
        {
            weight = 0.6;
            return SUFFIX_BREAK;
        }
#ifdef ADDR_DEBUG
        std::cerr<<"[LEFT_S_P]"<<GetText_(left)<<","<<left_suffix_type<<":"<<left_suffix_prob<<std::endl;
#endif
        //term_t first = word.front();
        //term_t first_type = NOT_ADDR;
        //if(!IsValidSuffix_(first, first_type))
        //{
        //    return STATUS_NO;
        //}
        double lu_prob = 0.0;
        double lb_prob = 0.0;
        word_t key;
        if(left.size()>=1)
        {
            key.resize(3);
            key[0] = LEFT_CONTEXT;
            key[1] = type;
            key[2] = left[0];
            Prob::const_iterator it = prob_.find(key);
            if(it!=prob_.end())
            {
                lu_prob = it->second;
            }
#ifdef ADDR_DEBUG
            std::cerr<<"[LUP]"<<GetText_(left[0])<<","<<type<<":"<<lu_prob<<std::endl;
#endif
        }
        if(left.size()>=2)
        {
            key.resize(4);
            key[0] = LEFT_CONTEXT;
            key[1] = type;
            key[2] = left[0];
            key[3] = left[1];
            Prob::const_iterator it = prob_.find(key);
            if(it!=prob_.end())
            {
                lb_prob = it->second;
            }
#ifdef ADDR_DEBUG
            std::cerr<<"[LUB]"<<GetText_(left[0])<<GetText_(left[1])<<","<<type<<":"<<lu_prob<<std::endl;
#endif
        }
        double lc_prob = lb_prob>0.0 ? lb_prob : lu_prob;//left context prob
        //double last_suffix_prob = 0.0;
        //bool last_maybe_suffix = false;
        //term_t last = word.back();
        //if(IsValidSpecifySuffix_(last, type, last_suffix_prob))
        //{
        //    last_maybe_suffix = true;
        //}
#ifdef ADDR_DEBUG
        //std::cerr<<"[LPROB]"<<lc_prob<<std::endl;
#endif
        double left_prob = left_suffix_prob;
        if(lc_prob>0.0) left_prob*=2;
        //if(word.size()>=4 && l_prob<0.01 && last_maybe_suffix)
        //{
        //    return STATUS_NO;
        //}
        weight = left_prob*tword.size();

        return SUFFIX_NEXT;
    }
//    int IsCandidate_(const word_t& word, const word_t& left, term_t& type, double& weight)
//    {
//        Dict::const_iterator it = rdict_.lower_bound(word);
//        if(it!=rdict_.end())
//        {
//            if(boost::algorithm::starts_with(it->first, word))
//            {
//                if(it->first==word)
//                {
//                    type = it->second;
//                    weight = 1.0;
//                    return STATUS_YES;
//                }
//                else 
//                {
//                    type = it->second;
//                    weight = 0.0;
//                    return STATUS_NEXT;
//                }
//            }
//        }
//        //static const std::size_t SUFFIX_SEARCH_SPACE = 2;
//        double suffix_prob = 0.0;
//        term_t suffix_type = GetSuffixType_(word, 0, suffix_prob);
//        if(suffix_type==NOT_ADDR||suffix_type==PROVINCE||suffix_type==CITY)
//        {
//#ifdef ADDR_DEBUG
//            std::cerr<<"[ST]"<<suffix_type<<std::endl;
//#endif
//            return STATUS_NO;
//        }
//        type = suffix_type;
//        if(left.empty())
//        {
//            //if(type==PROVINCE || type==CITY || type==COUNTY || type==DISTRICT) return STATUS_YES;
//            weight = 0.8;
//            return STATUS_YES;
//        }
//        word_t rleft(left.rbegin(), left.rend());
//        double left_suffix_prob = 0.0;
//        term_t left_suffix_type = GetSuffixType_(rleft, 0, left_suffix_prob);
//        if(left_suffix_type!=NOT_ADDR) 
//        {
//            weight = 0.6;
//            return STATUS_YES;
//        }
//        //term_t first = word.front();
//        //term_t first_type = NOT_ADDR;
//        //if(!IsValidSuffix_(first, first_type))
//        //{
//        //    return STATUS_NO;
//        //}
//        double lu_prob = 0.0;
//        double lb_prob = 0.0;
//        word_t key;
//        if(left.size()>=1)
//        {
//            key.resize(3);
//            key[0] = LEFT_CONTEXT;
//            key[1] = type;
//            key[2] = left[0];
//            Prob::const_iterator it = prob_.find(key);
//            if(it!=prob_.end())
//            {
//                lu_prob = it->second;
//            }
//        }
//        if(left.size()>=2)
//        {
//            key.resize(4);
//            key[0] = LEFT_CONTEXT;
//            key[1] = type;
//            key[2] = left[0];
//            key[3] = left[1];
//            Prob::const_iterator it = prob_.find(key);
//            if(it!=prob_.end())
//            {
//                lb_prob = it->second;
//            }
//        }
//        double lc_prob = lb_prob>0.0 ? lb_prob : lu_prob;//left context prob
//        //double last_suffix_prob = 0.0;
//        //bool last_maybe_suffix = false;
//        //term_t last = word.back();
//        //if(IsValidSpecifySuffix_(last, type, last_suffix_prob))
//        //{
//        //    last_maybe_suffix = true;
//        //}
//#ifdef ADDR_DEBUG
//        std::cerr<<"[LPROB]"<<lc_prob<<std::endl;
//#endif
//        double left_prob = left_suffix_prob;
//        if(lc_prob>0.0) left_prob*=2;
//        //if(word.size()>=4 && l_prob<0.01 && last_maybe_suffix)
//        //{
//        //    return STATUS_NO;
//        //}
//        weight = left_prob*word.size();
//        return STATUS_NEXT;
//    }
//

    void ProcessTrainAddr_(const std::string& address)
    {
        word_t word;
        GetWord_(address, word, false);
        std::vector<token_t> tokens;
        //std::cerr<<"After gettoken "<<address<<","<<word.size()<<std::endl;
        GetTokens_(word, tokens);
        for(uint32_t i=0;i<tokens.size();i++)
        {
            const token_t& current = tokens[i];
            if(current.second==NOT_ADDR) continue;
            const word_t& current_word = current.first;
            word_t key(1, SUFFIX);
            key.push_back(current.second);
            key.push_back(current_word.back());
            AddCount_(key, 1);
            if(current_word.size()>1)
            {
                key.push_back(current_word[current_word.size()-2]);
                AddCount_(key, 1);
            }
            key.clear();
            key.push_back(ADDR);
            AddCount_(key, 1);
            key.clear();
            key.push_back(current.second);
            AddCount_(key, 1);
            if(i>0)
            {
                const token_t& left = tokens[i-1];
                const word_t& left_word = left.first;
                word_t key(1, LEFT_CONTEXT);
                key.push_back(left.second);
                key.push_back(left_word.back());
                AddCount_(key, 1);
                if(left_word.size()>=2)
                {
                    word_t key(1, LEFT_CONTEXT);
                    key.push_back(left.second);
                    key.push_back(*(left_word.end()-1));
                    key.push_back(*(left_word.end()-2));
                    AddCount_(key, 1);
                }
            }
            if(i<tokens.size()-1)
            {
                const token_t& right = tokens[i+1];
                const word_t& right_word = right.first;
                word_t key(1, RIGHT_CONTEXT);
                key.push_back(right.second);
                key.push_back(right_word.back());
                AddCount_(key, 1);
                if(right_word.size()>=2)
                {
                    word_t key(1, RIGHT_CONTEXT);
                    key.push_back(right.second);
                    key.push_back(*(right_word.begin()+1));
                    key.push_back(*(right_word.begin()));
                    AddCount_(key, 1);
                }
            }

        }
    }
    void GetWord_(const std::string& text, word_t& word, bool get_sym = true)
    {
        std::vector<idmlib::util::IDMTerm> term_list;
        UString utext(text, UString::UTF_8);
        analyzer_->GetTermList(utext, term_list);
        for(uint32_t i=0;i<term_list.size();i++)
        {
            std::string str = term_list[i].TextString();
            if(term_list[i].tag==idmlib::util::IDMTermTag::SYMBOL)
            {
                if(get_sym)
                {
                    if(str!="-") continue;
                }
                else
                {
                    continue;
                }
            }
            word.push_back(GetTerm_(str));
        }
    }

    term_t GetTerm_(const std::string& str)
    {
        term_t term = izenelib::util::HashFunction<std::string>::generateHash32(str);
#ifdef ADDR_DEBUG
        term_text_[term] = str;
#endif
        return term;
    }
#ifdef ADDR_DEBUG
    std::string GetText_(term_t term)
    {
        return term_text_[term];
    }
    std::string GetText_(const word_t& word)
    {
        std::string r;
        for(uint32_t i=0;i<word.size();i++)
        {
            std::string s = term_text_[word[i]];
            if(s.empty()) s = " ";
            r+=s;
        }
        return r;
    }
#endif
    std::string GetText_(const SuffixWord& word, std::size_t start, std::size_t end)
    {
        std::string str;
        for(SuffixWord::const_iterator it = word.begin()+start; it!=word.begin()+end; ++it)
        {
            str=it->text+str;
        }
        return str;
    }
    std::string GetText_(const NodeList& nl, bool debug = false)
    {
        std::string result;
        for(std::size_t i=0;i<nl.size();i++)
        {
            const Node& node = nl[i];
            if(node.type==NOT_ADDR) 
            {
                if(!debug) continue;
                if(!result.empty()) result+="|";
                result+="{";
                result+=node.text;
                result+="}";
                continue;
            }
            if(!result.empty()) result+="|";
            result+=node.text;
        }
        return result;
    }

    void GetTokens_(const word_t& word, std::vector<token_t>& tokens)
    {
        uint32_t start=0;
        std::string noaddr_token;
        word_t notaddr;
        while(true)
        {
            if(start>=word.size()) break;
            uint32_t len=1;
            word_t find_word;
            ADDR_TYPE find_type=NOT_ADDR;
            while(true)
            {
                uint32_t end=start+len;
                if(end>word.size()) break;
                word_t frag(word.begin()+start, word.begin()+end);
                Dict::const_iterator it = dict_.lower_bound(frag);
                if(it==dict_.end()) break;
                if(!boost::algorithm::starts_with(it->first, frag)) break;
                if(it->first==frag)
                {
                    find_word = frag;
                    find_type = it->second;
                }
                ++len;
            }
            if(!find_word.empty())
            {
                start += find_word.size();
                if(!notaddr.empty())
                {
                    tokens.push_back(std::make_pair(notaddr, NOT_ADDR));
                    notaddr.clear();
                }
                tokens.push_back(std::make_pair(find_word, find_type));
            }
            else
            {
                notaddr.push_back(word[start]);
                ++start;
                //if(start<word.size())
                //{
                //    std::string str = word.substr(start, 1);
                //    noaddr_token += str;
                //}
            }
        }
        if(!notaddr.empty())
        {
            tokens.push_back(std::make_pair(notaddr, NOT_ADDR));
        }
    }

    void AddCount_(const word_t& word, std::size_t count)
    {
        Count::iterator it = count_.find(word);
        if(it==count_.end())
        {
            count_.insert(std::make_pair(word, count));
        }
        else
        {
            it->second += count;
        }
    }


/////////////////////////////////////////////////////////////////////////////////////////////
//Addby wangbaobao@b5m.com begin------------------------------
//define weight for each address type

private:
	//type common define
	template<class _Ty>
	class Weight
	{
		public:
			Weight()
			{
				weight[0] = static_cast<_Ty>(0);
				weight[1] = static_cast<_Ty>(0);
			}
			_Ty weight[2];
	};

	//threshold define
	static const double kChineseThreshold = 0.90;
	static const double kEnglishThreshold = 0.75;
	static const double kAddressWeight[];
	
	typedef std::pair<size_t,double> TypeAndWeight;
	typedef std::pair<std::string,TypeAndWeight> TextAndType;
	typedef std::vector<TextAndType> TextAndTypeCollection;
	typedef std::vector<std::string> TextEngCollection;
	typedef std::vector<TextAndTypeCollection> TextAndTypeCollectionUnion;
	typedef std::pair<std::string, TextAndTypeCollection> EvaluateResult;
	typedef boost::unordered_map<TextAndType, Weight<double> > TextAndTypeMap;

public:
	typedef std::pair<std::string,std::string> TwoAddress;
	typedef std::pair<size_t,size_t>	       AddressIndexPair;
	typedef std::vector<std::string>		   AddressCollection;
	typedef std::vector<AddressIndexPair>	   AddressIndexPairList;
	
	enum LanguageType
	{
		kCH,
		kEng,
		kMix
	};

	struct SimilarityResult
	{
		double				   similarity;
		LanguageType		   language_type;
		TextAndTypeCollection  collection1;
		TextAndTypeCollection  collection2;
	};
	typedef boost::unordered_map<TwoAddress, SimilarityResult> SimilarityMap;

private:
	//@brief:build <text,type> collection
	void GetTextAndTypeCollection(const NodeList&node_list, 
								  TextAndTypeCollection&collection)
	{
		for(size_t i = 0; i < node_list.size(); i++)
		{
			const Node& node = node_list[i];
			collection.push_back(std::make_pair(node.text,
				TypeAndWeight(node.type,kAddressWeight[node.type])));
		}
	}

	//@brief
	EvaluateResult Evaluate_(const std::string& addr)
	{        
		std::string result;
		TextAndTypeCollection collection;
		
		if(addr.length()>300) return std::make_pair(result,collection);
        std::string address = Clean_(addr);
        //word_t word;
        //GetWord_(address, word);
        //word_t rword(word.rbegin(), word.rend());
        SuffixWord word;
        GetSuffixWord_(address, word);
#ifdef ADDR_DEBUG
        std::cerr<<"[A]"<<address<<",["<<word.size()<<"]"<<std::endl;
#endif
        NodeList current;
        NodeMatrix global;
        Recursive_(word, 0, 0, current, global);
        std::vector<std::size_t> covers(global.size());
        for(uint32_t i=0;i<global.size();i++)
        {
			NodeList& nl = global[i];
			covers[i] = PostProcessResult_(nl, word);
        }
        std::pair<double, std::size_t> select(0.0, 0);
        for(uint32_t i=0;i<global.size();i++)
        {
            double score = 0.0;
            const NodeList& nl = global[i];
            for(uint32_t j=0;j<nl.size();j++)
            {
                const Node& node = nl[j];
                score+=node.prob;
            }
#ifdef ADDR_DEBUG
            std::string str = GetText_(nl, true);
            std::cerr<<"[RESULT-SCORE]"<<str<<","<<score<<std::endl;
#endif
            if(score>select.first)
            {
                select.first = score;
                select.second = i;
            }
        }

        if(select.first>0.0)
        {
            const NodeList& nresult = global[select.second];
            std::size_t cover = covers[select.second];
            if(!LostTooMuch_(cover, word))
            {
                result = GetText_(nresult);
				//build <Text,Type> collection
				GetTextAndTypeCollection(nresult,collection);
#ifdef ADDR_DEBUG
                std::cerr<<"[FINALD]";
                for(uint32_t i=0;i<nresult.size();i++)
                {
                    std::cerr<<"("<<nresult[i].text<<","<<nresult[i].type<<")";
                }
                std::cerr<<std::endl;
#endif
            }
        }
#ifdef ADDR_DEBUG
        std::cerr<<"[FINAL]"<<result<<std::endl;
#endif
        return std::make_pair(result, collection);
	}

	//@brief: adjust weight for road and poi			
	//      road: if contain more than one road type, we just 
	//		keep one which can be matched , ingnore others.
	//      poi:  strategy is similar to road type, but if different poi type more than 1
	//		we should not ignore no-matched poi.
	void Adjust_(TextAndTypeMap &text_type_table)
	{
		size_t type = 0;
		size_t different_poi = 0;
		size_t different_road = 0;
		size_t number_l = strlen("号");
		bool   has_poi_matched = false;
		bool   has_road_matched = false;
		TextAndTypeMap::iterator iter;

		for(iter = text_type_table.begin(); 
				iter != text_type_table.end(); ++iter)
		{
			type = iter->first.second.first;
			if(type == POI) 
			{
				if((iter->second.weight[0] == iter->second.weight[1]) && 
						(iter->first.first.find("号") == iter->first.first.length()-number_l))
				{
					assert(iter->second.weight[0] > 0.0);
					has_poi_matched = true;
				}
				else
				{
					++different_poi;
				}
			}
			else if(type == ROAD) 
			{
				if(iter->second.weight[0] == 
						iter->second.weight[1])
				{
					assert(iter->second.weight[0] > 0.0);
					has_road_matched = true;
				}
				else
				{
					++different_road;
				}
			}
		}
		
		for(iter = text_type_table.begin(); 
			iter != text_type_table.end(); ++iter)
		{
			Weight<double>& w = iter->second;
			type = iter->first.second.first;
			if(type == ROAD)
			{
				//adjust road weight, increase similarity if has_road_matched = true
				// or has only one different road
				if (has_road_matched &&
					w.weight[0] != w.weight[1])
				{
					w.weight[0] = 0.0;
					w.weight[1] = 0.0;
				}
			}
			else if(type == POI)
			{
				//we adjust poi weight according to conditions of 
				//"has_road_matched && has_poi_matched"
				//but like "xxx号" represent accurate information,so we should not
				//reduce the weight of xxx号
				if(has_road_matched &&
				   has_poi_matched && 
				   w.weight[0] != w.weight[1] &&
				   iter->first.first.find("号") != iter->first.first.length()-number_l)
				{
					w.weight[0] = 0.0;
					w.weight[1] = 0.0;
				}
			}
			else
			{
				assert(type < ROAD);
			}
		}		
	}

	//@brief:calculate chinese merchant address similarity 
	//		 using vector weight cos(collection1,collection2)
	double Similarity_(TextAndTypeCollection& collection1, 
					   TextAndTypeCollection& collection2)
	{
		if(collection1.empty() || collection2.empty())
		{
			return 0.0;
		}
		double similarity = 0.0;
		double temp1 = 0.0;
		double temp2 = 0.0;
		double temp3 = 0.0;
		TextAndTypeMap text_type_table;
		TextAndTypeMap::const_iterator iter;
		
		for(size_t i = 0; i < collection1.size(); i++)
		{
			//using address type weight
			text_type_table[collection1[i]].weight[0] = collection1[i].second.second;
		}

		for(size_t i = 0; i < collection2.size(); i++)
		{
			text_type_table[collection2[i]].weight[1] = collection2[i].second.second;
		}

		//adjust poi weight if possible
		Adjust_(text_type_table);
		
		//calculate vector similarity
		for(iter = text_type_table.begin(); iter != text_type_table.end(); ++iter)
		{
			temp1 += (iter->second.weight[0] * iter->second.weight[1]);
			temp2 += (iter->second.weight[0] * iter->second.weight[0]);
			temp3 += (iter->second.weight[1] * iter->second.weight[1]);
		}

		assert(temp2 > 0.0 && temp3 > 0.0);
		similarity = temp1 / (sqrt(temp2 * temp3));
		return similarity;
	}

	//@brief:process those records not successed evaluated
	//currently simplely consider them conform rules of english address
	void BuildNotEvaluateRecord(const std::string& address, 
								TextAndTypeCollection& collection)
	{	
		double weight = 1.0;
		TextEngCollection eng_collection;
		boost::split(eng_collection,address,boost::is_any_of(",|"));
		collection.clear();
		for(size_t i = 0; i < eng_collection.size(); ++i)
		{
			if(eng_collection[i].empty())
			{
				continue;
			}
			boost::to_lower(eng_collection[i]);
			collection.push_back(std::make_pair(
				eng_collection[i],TypeAndWeight(0,weight)));
			weight /= 2.0;
		}
	}
		
public:
	//@brief:normalize merchant address
	//@return xxx|xxx|xxx|xxx
    std::string Evaluate(const std::string& address)
    {
		EvaluateResult evaluate_result = Evaluate_(address);
		return evaluate_result.first;
    }	

	//@brief:calculate similarity of <addr_i,addr_j> (1<=i<=n,1<=j<=n)
	//@return similar address index-pair vector
	void FindSimilar(const AddressCollection& address_collection, 
				     AddressIndexPairList&result)
	{
		double					   similarity = 0.0;
		TextAndTypeCollectionUnion pre_calculate_result;
		//reserve storage
		pre_calculate_result.reserve(address_collection.size());
		result.clear();
		//pre-calculate each address evalute result
		for(size_t i = 0; i < address_collection.size(); i++)
		{
			EvaluateResult evaluate_result = Evaluate_(address_collection[i]); 
			if(!evaluate_result.first.empty())
			{
				pre_calculate_result.push_back(evaluate_result.second);
			}
			else
			{
				TextAndTypeCollection collection;
				BuildNotEvaluateRecord(address_collection[i], collection);
				pre_calculate_result.push_back(collection);
			}
		}
		assert(pre_calculate_result.size() == address_collection.size());

		for(size_t i = 0; i < address_collection.size(); i++)
		{	
			TextAndTypeCollection &collection1 = pre_calculate_result[i];
			
			for(size_t j = i + 1; j < address_collection.size(); j++)
			{
				TextAndTypeCollection &collection2 = pre_calculate_result[j];
				if((!collection1.empty()) && (!collection2.empty()))
				{
					similarity = Similarity_(collection1,collection2);
					assert(collection1.size() > 0 && collection2.size() > 0);
					if(collection1[0].second.first != 0 && 
							collection2[0].second.first != 0)
					{
						//process two successed evaluated records
						if(similarity >= kChineseThreshold)
						{
							result.push_back(std::make_pair(i,j));
						}
					}
					else
					{
						//maybe english address here
						//mix language address similarity = 0.0
						if(similarity >= kEnglishThreshold)
						{
							result.push_back(std::make_pair(i,j));
						}
					}
				}
			}
		}
	}

	//@brief:calculate similarity of <addr_i,addr_j> (1<=i<=n,1<=j<=n)
	//@return hash map like : <addr1,addr2> -----><similarity,threshold>
	//@notice:<addr1,addr2>represent two address string  pair in address vector
	//		  sim<addri,addrj> must be equal to sim<addrj,addri>, but result of hash
	//		  map only store sim<addri,addrj>
	SimilarityMap Similarity(const AddressCollection& address_collection)
	{
		double					   similarity = 0.0;
		SimilarityMap			   similarity_map;
		SimilarityResult		   result;
		TextAndTypeCollectionUnion pre_calculate_result;
		//reserve storage
		pre_calculate_result.reserve(address_collection.size());
		//pre-calculate each address evalute result
		for(size_t i = 0; i < address_collection.size(); i++)
		{
			EvaluateResult evaluate_result = Evaluate_(address_collection[i]); 
			if(!evaluate_result.first.empty())
			{
				pre_calculate_result.push_back(evaluate_result.second);
			}
			else
			{
				TextAndTypeCollection collection;
				BuildNotEvaluateRecord(address_collection[i], collection);
				pre_calculate_result.push_back(collection);
			}
		}
		
		assert(pre_calculate_result.size() == address_collection.size());
		
		for(size_t i = 0; i < address_collection.size(); i++)
		{
			TextAndTypeCollection &collection1 = pre_calculate_result[i];
			for(size_t j = i + 1; j < address_collection.size(); j++)
			{
				TextAndTypeCollection &collection2 = pre_calculate_result[j];

				if((!collection1.empty()) && (!collection2.empty()))
				{
					similarity = Similarity_(collection1,collection2);
					if(collection1[0].second.first != 0 && 
							collection2[0].second.first != 0)
					{
						result.language_type = kCH;
					}
					else if(collection1[0].second.first == 0 &&
								collection2[0].second.first == 0)
					{
						result.language_type = kEng;
					}
					else
					{
						result.language_type = kMix;
					}
					result.similarity = similarity;
					result.collection1 = collection1;
					result.collection2 = collection2;
					assert(i < j);
					similarity_map.insert(std::make_pair(TwoAddress(
							address_collection[i],address_collection[j]), result));
				}
			}
		}
		return similarity_map;
	}
//Add by wangbaobao@b5m.com end------------------------------
/////////////////////////////////////////////////////////////////////////////////////////////////
private:
    idmlib::util::IDMAnalyzer* analyzer_;
    Dict dict_;
    Dict rdict_;
    SynDict syn_dict_;
    SynDict user_syn_;
    boost::regex num_road_regex_;
    boost::regex pure_road_regex_;
    Prob prob_;
    Count count_;
    Prob suffix_prob_;
    std::vector<ADDR_TYPE> ADDR_LIST;
    TermText term_text_;
};

const double AddressExtract::kAddressWeight[] = 
{
		0.0,/*NOT USED*/
		0.0,/*NOT_ADDR*/
		0.05,/*PROVINCE*/
		0.05,/*CITY*/
		0.06,/*COUNTY*/
		0.06,/*DISTRICT*/
		0.2,/*ROAD*/
		0.3 /*POI*/
};

NS_IDMLIB_B5M_END

#endif
