#include <idmlib/b5m/tour_processor.h>
#include <idmlib/b5m/b5m_types.h>
#include <idmlib/b5m/b5m_helper.h>
#include <idmlib/b5m/scd_doc_processor.h>
#include <idmlib/similarity/string_similarity.h>
#include <boost/algorithm/string.hpp>
#include <glog/logging.h>
#include <sf1common/ScdParser.h>
#include <sf1common/ScdDocument.h>
#include <assert.h>

//#define TOUR_DEBUG

using namespace idmlib::b5m;
using namespace idmlib::sim;

TourProcessor::TourProcessor(const B5mM& b5mm): b5mm_(b5mm)
{
    idmlib::util::IDMAnalyzerConfig csconfig = idmlib::util::IDMAnalyzerConfig::GetCommonConfig("","", "");
    csconfig.symbol = false;
    analyzer_ = new idmlib::util::IDMAnalyzer(csconfig);
}

TourProcessor::~TourProcessor()
{
    delete analyzer_;
}
std::string TourProcessor::GetText_(const word_t& word)
{
    std::string result = "";
    for(std::size_t i=0;i<word.size();i++)
    {
        result+=term_text_[word[i]];
    }
    return result;
}
uint32_t TourProcessor::GetTerm_(const std::string& str)
{
    term_t term = izenelib::util::HashFunction<std::string>::generateHash32(str);
#ifdef TOUR_DEBUG
    term_text_[term] = str;
#endif
    return term;
}
void TourProcessor::GetWord_(const std::string& text, word_t& word)
{
    UString title(text, UString::UTF_8);
    std::vector<idmlib::util::IDMTerm> terms;
    analyzer_->GetTermList(title, terms);
    for(uint32_t i=0;i<terms.size();i++)
    {
        std::string str = terms[i].TextString();
        boost::algorithm::to_lower(str);
        word.push_back(GetTerm_(str));
    }
}

bool TourProcessor::Generate(const std::string& mdb_instance)
{
    //load knowledge
    LOG(INFO)<<"Now loading tour knowledge"<<std::endl;
    std::string dic_file = b5mm_.knowledge+"/dict";
    std::ifstream ifs(dic_file.c_str());
    std::string line;
    while(getline(ifs, line))
    {
        std::vector<std::string> vec;
        boost::algorithm::split(vec, line, boost::is_any_of("@"));
        word_t word;
        if(vec.size()==2)
        {
            if(vec[1]!="poi") continue;
            const std::string& poi = vec[0];
            GetWord_(poi, word);
        }
        else if(vec.size()==1)
        {
            UString utext(vec[0], UString::UTF_8);
            if(utext.length()<3) continue;
            GetWord_(vec[0], word);
        }
        else continue;
        if(!word.empty())
        {
            location_dict_.insert(word);
        }
    }
    ifs.close();
    if(location_dict_.empty())
    {
        LOG(ERROR)<<"location dict empty"<<std::endl;
        return false;
    }
    LOG(INFO)<<"Location dict size "<<location_dict_.size()<<std::endl;
    const std::string& scd_path = b5mm_.scd_path;
    //scan all the *.scd
    m_ = mdb_instance;
    ScdDocProcessor::ProcessorType p = boost::bind(&TourProcessor::Insert_, this, _1);
    int thread_num = 1;
    ScdDocProcessor sd_processor(p, thread_num);
    sd_processor.AddInput(scd_path);
    sd_processor.Process();
    Finish_();
    return true;
}

void TourProcessor::Insert_(ScdDocument& doc)
{
    std::string name;
    doc.getString("Name", name);
    if(name.empty()) return;
    std::string sdays;
    std::string price;
    BufferValueItem value;
    value.bcluster = true;
    value.is_free = CheckFree_(doc);
    doc.getString(SCD_FROM_CITY, value.from);
    std::string sto;
    doc.getString(SCD_TO_CITY, sto);
    boost::algorithm::split(value.to,sto,boost::is_any_of(","));
    std::vector<std::string> nto;
    for(std::size_t i=0;i<value.to.size();i++)
    {
        const std::string& t = value.to[i];
        if(t.empty()) continue;
        if(t==value.from) continue;
        nto.push_back(t);
    }
    std::swap(value.to, nto);
    std::sort(value.to.begin(), value.to.end());
    doc.getString(SCD_PRICE,price);
    doc.getString(SCD_TIME_PLAN, sdays);
    value.days = ParseDays_(sdays);
    value.doc = doc;
    value.price = 0.0;
    try
    {
        value.price = boost::lexical_cast<double>(price);
    }
    catch(std::exception& ex)
    {
        //std::string error_doc_id;
        //doc.getString(SCD_DOC_ID,error_doc_id);
        //LOG(INFO) << "doc id:" << error_doc_id;
        //LOG(INFO) << ex.what() << "\n";
    }
    if(value.days.second==0||value.price==0.0||
            value.from.empty()||value.to.empty())
    {
        value.bcluster = false;
    }
    std::stringstream ss;
    ss<<value.from;
    for(std::size_t i=0;i<value.to.size();i++)
    {
        ss<<","<<value.to[i];
    }
    BufferKey key = ss.str();

    //generate union key by to_city
    //UnionBufferKey union_key;
    //GenerateUnionKey(union_key,value.from,value.to);
    if(!value.is_free)
    {
        GenLocations_(value, key);
    }
    boost::unique_lock<boost::mutex> lock(mutex_);
    buffer_[key].push_back(value);
}

std::pair<uint32_t, uint32_t> TourProcessor::ParseDays_(const std::string& sdays) const
{
    //find "天" Ps:4-7天/8天
    std::pair<uint32_t, uint32_t> r(0,0);
    size_t pos = sdays.find("天");
    if(pos == std::string::npos)
    {
        pos = sdays.length();
    }

    std::string tmp_days = sdays.substr(0,pos);
    //find '-'
    pos = tmp_days.find('-');
    if(pos == std::string::npos)
    {
        r.first = atoi(tmp_days.c_str());
        r.second = r.first;
    }
    else
    {
        int min_days = atoi(tmp_days.substr(0,pos).c_str());
        int max_days = atoi(tmp_days.substr(pos+1).c_str());
        r.first = min_days;
        r.second = max_days;
    }
    return r;
}
bool TourProcessor::CheckFree_(const ScdDocument& doc) const
{
    bool r = false;
    std::string name;
    doc.getString("Name", name);
    if(name.find("自由行")!=std::string::npos)
    {
        r = true;
    }
    return r;
}
void TourProcessor::GenLocations_(BufferValueItem& value, const BufferKey& key)
{
    std::string name;
    value.doc.getString("Name", name);
    word_t word;
    GetWord_(name, word);
    if(word.empty()) 
    {
        return;
    }
    std::vector<std::string> ktokens;
    boost::algorithm::split(ktokens,key,boost::is_any_of(","));
    Dict key_set;
    for(std::size_t i=0;i<ktokens.size();i++)
    {
        //const std::pair<std::string, std::string>& k = union_key.union_key_[i];
        //std::pair<word_t, word_t> word;
        //GetWord_(k.first, word.first);
        //GetWord_(k.second, word.second);
        //key_set.insert(word.first);
        //key_set.insert(word.second);
        word_t word;
        GetWord_(ktokens[i], word);
        key_set.insert(word);
    }
    word_t loc_word;
    std::vector<word_t> results;
    for(std::size_t i=0;i<word.size();i++)
    {
        loc_word.push_back(word[i]);
        Dict::const_iterator it = location_dict_.lower_bound(loc_word);
        if(it==location_dict_.end()||!boost::algorithm::starts_with(*it, loc_word))
        {
            loc_word.resize(0);
        }
        else if(*it==loc_word)
        {
            if(!results.empty())
            {
                if(boost::algorithm::starts_with(loc_word, results.back()))
                {
                    results.back() = loc_word;
                    continue;
                }
            }
            results.push_back(loc_word);
        }
    }
#ifdef TOUR_DEBUG
    //std::cerr<<"[N]"<<name<<std::endl;
#endif
    for(std::size_t i=0;i<results.size();i++)
    {
        const word_t& r = results[i];
        if(key_set.find(r)!=key_set.end()) continue;
        value.locations.insert(r);
#ifdef TOUR_DEBUG
        //std::cerr<<"[L]"<<GetText_(r)<<std::endl;
#endif
    }
}

//void TourProcessor::GenerateUnionKey(UnionBufferKey& union_key,
//                                     const std::string& from_city,
//                                     const std::string& to_city)const
//{
//    std::vector<std::string> union_to_city;
//    boost::split(union_to_city,to_city,boost::is_any_of(","));
//
//    if(union_to_city.size() == 0)
//    {
//        union_key.push_back(std::make_pair(from_city,""));
//        return;
//    }
//
//    for(size_t i = 0;i<union_to_city.size();i++)
//    {
//        //ps:from city:上海  to city：上海，马累，马尔代夫
//        //so we should remove key pair(上海，上海)
//        if(union_to_city[i].empty()) continue;
//        if(union_to_city.size() >= 2)
//        {
//            if(from_city == union_to_city[i]) continue;
//        }
//        union_key.push_back(std::make_pair(from_city,union_to_city[i]));
//    }
//
//    if(union_key.size()==0)
//    {
//        union_key.push_back(std::make_pair(from_city,from_city));
//    }
//
//    //sort key vector
//    std::sort(union_key.union_key_.begin(),union_key.union_key_.end());
//}

void TourProcessor::Finish_()
{
    //static size_t group_index = 0;
    std::string odir = b5mm_.b5mo_path;
    std::string pdir = b5mm_.b5mp_path;
    B5MHelper::PrepareEmptyDir(odir);
    B5MHelper::PrepareEmptyDir(pdir);
    ScdWriter owriter(odir, UPDATE_SCD);
    ScdWriter pwriter(pdir, UPDATE_SCD);
    typedef std::pair<double, std::pair<std::size_t, std::size_t> > SimItem;
    typedef boost::unordered_map<std::pair<std::size_t, std::size_t>, double> SimIndex;
    std::size_t p=0;
    LOG(INFO)<<"Total buffer size "<<buffer_.size()<<std::endl;
    StringSimilarity ss;
    for(Buffer::iterator it = buffer_.begin();it!=buffer_.end();++it)
    {
        BufferValue& value = it->second;
        p++;
        if(p%1000==0)
        {
            LOG(INFO)<<"Buffer index "<<p<<", size "<<value.size()<<std::endl;
        }
        std::sort(value.begin(), value.end());
        std::vector<StringSimilarity::Object> sim_objects(value.size());
        for(uint32_t i = 0;i < value.size(); i++)
        {
            std::string name;
            value[i].doc.getString("Name", name);
            ss.Convert(name, sim_objects[i]);
        }
        std::vector<SimItem> sim_list;
        SimIndex sim_index;
        for(uint32_t i = 0;i < value.size(); i++)
        {
            const BufferValueItem& vi = value[i];
            if(!vi.bcluster) continue;
            for(uint32_t j = i+1;j < value.size(); j++)
            {
                const BufferValueItem& vj = value[j];
                if(!vj.bcluster) continue;
                if(vi.days!=vj.days) continue;
                if(vi.is_free!=vj.is_free) continue;
                double str_sim = ss.Sim(sim_objects[i], sim_objects[j]);
                double sim = 0.0;
                if(vi.locations.empty()&&vj.locations.empty())
                {
                    sim = str_sim;
                }
                else if(vi.locations.empty()||vj.locations.empty())
                {
                    sim = 0.0;
                }
                else
                {
                    std::size_t same_count=0;
                    for(boost::unordered_set<word_t>::const_iterator iti = vi.locations.begin();iti!=vi.locations.end();++iti)
                    {
                        if(vj.locations.find(*iti)!=vj.locations.end())
                        {
                            same_count++;
                        }
                    }
                    if(same_count==0) sim = 0.0;
                    else
                    {
                        //sim = 0.3+0.1*same_count;
                        sim = str_sim+0.1*same_count;
                        if(sim>1.0) sim = 1.0;
                    }
                }
#ifdef TOUR_DEBUG
                //std::string namei;
                //vi.doc.getString("Name", namei);
                //std::string namej;
                //vj.doc.getString("Name", namej);
                //std::cerr<<"[SIM]"<<namei<<"{"<<vi.locations.size()<<"},"<<namej<<"{"<<vj.locations.size()<<"},"<<sim<<std::endl;
#endif
                if(sim>0.0)
                {
                    SimItem si(sim, std::make_pair(i, j));
                    sim_list.push_back(si);
                    sim_index[std::make_pair(i, j)] = sim;
                }
            }
        }
        //LOG(INFO)<<"similarity calculation finished"<<std::endl;
        std::sort(sim_list.begin(), sim_list.end(), std::greater<SimItem>());
        typedef boost::unordered_map<std::size_t, std::size_t> GroupIndex;
        GroupIndex group_index;
        typedef std::vector<std::size_t> IGroup;
        std::vector<IGroup> igroups;
        //typedef std::map<std::pair<std::size_t, std::size_t>, double> GroupSimIndex;
        //GroupSimIndex cross_group_min;//group index to sim
        static const double kSimMin = 0.33;
        for(std::size_t i=0;i<sim_list.size();i++)
        {
            const SimItem& si = sim_list[i];
            double sim = si.first;
            if(sim<kSimMin) continue;
            const std::pair<std::size_t, std::size_t>& p = si.second;
            GroupIndex::const_iterator iti = group_index.find(p.first);
            GroupIndex::const_iterator itj = group_index.find(p.second);
            if(iti==group_index.end()&&itj==group_index.end())
            {
                std::size_t index = igroups.size();
                IGroup g;
                g.push_back(p.first);
                g.push_back(p.second);
                igroups.push_back(g);
                group_index[p.first] = index;
                group_index[p.second] = index;
            }
            else if(iti==group_index.end()||itj==group_index.end())
            {
                std::size_t gindex=0;
                std::size_t vindex=0;
                if(iti==group_index.end())
                {
                    gindex = itj->second;
                    vindex = p.first;
                }
                else
                {
                    gindex = iti->second;
                    vindex = p.second;
                }
                IGroup& igroup = igroups[gindex];
                double sim_min = 0.0;
                for(std::size_t j=0;j<igroup.size();j++)
                {
                    std::pair<std::size_t, std::size_t> key(vindex, igroup[j]);
                    if(key.second<key.first) std::swap(key.first, key.second);
                    double xysim = 0.0;
                    SimIndex::const_iterator sit = sim_index.find(key);
                    if(sit!=sim_index.end())
                    {
                        xysim = sit->second;
                    }
                    if(j==0) sim_min = xysim;
                    else if(xysim<sim_min)
                    {
                        sim_min = xysim;
                    }
                }
                if(sim_min<kSimMin)
                {
                    std::size_t index = igroups.size();
                    IGroup g;
                    g.push_back(vindex);
                    igroups.push_back(g);
                    group_index[vindex] = index;
                    //std::pair<std::size_t, std::size_t> gkey(gindex, index);
                    //GroupSimIndex::iterator sit = cross_group_min.find(gkey);
                    //if(sit==cross_group_min.end())
                    //{
                    //    cross_group_min.insert(std::make_pair(gkey, sim_min));
                    //}
                    //else
                    //{
                    //    if(sim_min<sit->second) sit->second = sim_min;
                    //}
                }
                else
                {
                    igroup.push_back(vindex);
                    group_index[vindex] = gindex;
                }
            }
            else
            {
                //if(iti->second!=itj->second)
                //{
                //    std::pair<std::size_t, std::size_t> key(iti->second, itj->second);
                //    if(key.first>key.second) std::swap(key.first, key.second);
                //    GroupSimIndex::iterator sit = cross_group_min.find(key);
                //    if(sit==cross_group_min.end())
                //    {
                //        cross_group_min.insert(std::make_pair(key, sim));
                //    }
                //    else
                //    {
                //        if(sim<sit->second) sit->second = sim;
                //    }
                //}
            }
        }
        //for(std::size_t i=0;i<igroups.size();i++)
        //{
        //    std::cerr<<"group id "<<i<<", size "<<igroups[i].size()<<std::endl;
        //    for(std::size_t j=0;j<igroups[i].size();j++)
        //    {
        //        const BufferValueItem& item = value[igroups[i][j]];
        //        std::string name;
        //        item.doc.getString("Name", name);
        //        std::cerr<<"\t"<<name<<","<<item.locations.size()<<std::endl;
        //    }
        //}
        //std::cerr<<"start to regrouping"<<std::endl;
        //for(GroupSimIndex::const_iterator sit = cross_group_min.begin();sit!=cross_group_min.end();++sit)
        //{
        //    const std::pair<std::size_t, std::size_t>& key = sit->first;
        //    double sim = sit->second;
        //    if(sim<kSimMin) continue;
        //    IGroup& gi = igroups[key.first];
        //    IGroup& gj = igroups[key.second];
        //    if(gi.empty()||gj.empty()) continue;
        //    std::cerr<<"merge "<<key.second<<" to "<<key.first<<", sim "<<sim<<std::endl;
        //    for(std::size_t vj=0;vj<gj.size();vj++)
        //    {
        //        std::size_t jid = gj[vj];
        //        group_index[jid] = key.first;
        //    }
        //    gi.insert(gi.end(), gj.begin(), gj.end());
        //    gj.clear();
        //}
        //LOG(INFO)<<"First stage grouping finished"<<std::endl;
        for(std::size_t i=0;i<igroups.size();i++)
        {
            std::size_t index = i;
            IGroup& gi = igroups[i];
            if(gi.empty()) continue;
            for(std::size_t j=i+1;j<igroups.size();j++)
            {
                IGroup& gj = igroups[j];
                if(gj.empty()) continue;
                bool can_merge = true;
                for(std::size_t vi=0;vi<gi.size();vi++)
                {
                    for(std::size_t vj=0;vj<gj.size();vj++)
                    {
                        std::pair<std::size_t, std::size_t> key(gi[vi], gj[vj]);
                        if(key.second<key.first) std::swap(key.first, key.second);
                        double sim = 0.0;
                        SimIndex::const_iterator sit = sim_index.find(key);
                        if(sit!=sim_index.end()) sim = sit->second;
                        if(sim<kSimMin)
                        {
                            can_merge = false;
                            break;
                        }
                    }
                    if(!can_merge) break;
                }
                if(can_merge)
                {
                    //move all items in gj to gi
                    for(std::size_t vj=0;vj<gj.size();vj++)
                    {
                        std::size_t jid = gj[vj];
                        group_index[jid] = index;
                    }
                    gi.insert(gi.end(), gj.begin(), gj.end());
                    gj.clear();
                }
            }
        }
        //LOG(INFO)<<"re-grouping finished"<<std::endl;
        for(uint32_t i = 0;i < value.size(); i++)
        {
            GroupIndex::const_iterator it = group_index.find(i);
            if(it==group_index.end())
            {
                std::size_t index = igroups.size();
                IGroup g(1, i);
                igroups.push_back(g);
                group_index[i] = index;
            }
        }
        std::vector<Group> groups;
        for(std::size_t i=0;i<igroups.size();i++)
        {
            const IGroup& igroup = igroups[i];
            if(igroup.empty()) continue;
            Group g;
            for(std::size_t j=0;j<igroup.size();j++)
            {
                g.push_back(value[igroup[j]]);
            }
            groups.push_back(g);
        }
        //LOG(INFO)<<"Groups generated"<<std::endl;
        //for(uint32_t i = 0;i < value.size(); i++)
        //{
        //    const BufferValueItem& vi = value[i];
        //    Group* find_group = NULL;
        //    for(uint32_t j = 0; j < groups.size(); j++)
        //    {
        //        Group& g = groups[j];
        //        if(!g.front().bcluster)
        //        {
        //            continue;
        //        }

        //        //TimePlan must be equal in aggregation result
        //        if(g.front().days != vi.days)
        //        {
        //            continue;
        //        }
        //        bool all_has_same_location = true;
        //        for(std::size_t k=0;k<g.size();k++)
        //        {
        //            bool has_same_location = false;
        //            const BufferValueItem& vk = g[k];
        //            if(vi.locations.empty()&&vk.locations.empty())
        //            {
        //                has_same_location = true;
        //            }
        //            else
        //            {
        //                for(boost::unordered_set<word_t>::const_iterator iti = vi.locations.begin();iti!=vi.locations.end();++iti)
        //                {
        //                    if(vk.locations.find(*iti)!=vk.locations.end())
        //                    {
        //                        has_same_location = true;
        //                        break;
        //                    }
        //                }
        //            }
        //            if(!has_same_location)
        //            {
        //                all_has_same_location = false;
        //                break;
        //            }
        //        }
        //        if(!all_has_same_location) continue;
        //        find_group = &g;
        //    }

        //    if(find_group==NULL)
        //    {
        //        Group g;
        //        g.push_back(vi);
        //        groups.push_back(g);
		//		++group_index;
        //    }
        //    else
        //    {
        //        find_group->push_back(vi);
        //    }
        //}
        for(uint32_t i = 0; i < groups.size(); i++)
        {
            Group& g = groups[i];
#ifdef TOUR_DEBUG
            LogGroup(g);
#endif
            std::string pid;
            g.front().doc.getString(SCD_DOC_ID, pid);
            for(uint32_t j=0;j<g.size();j++)
            {
                BufferValueItem& vi = g[j];
                vi.doc.property(SCD_UUID) = str_to_propstr(pid);
                owriter.Append(vi.doc);
            }
            Document pdoc;
            GenP_(g, pdoc);
            pwriter.Append(pdoc);
        }
    }
    //LOG(INFO) << "Group total: " << group_index << "\n\n";
    owriter.Close();
    pwriter.Close();
}

void TourProcessor::LogGroup(const Group&group)
{
    //static int group_index = 1;
    //std::cout << "1--->group " << group_index++ <<
    //             " count:" << group.size() << std::endl;

    if(group.size()<2) return;
    std::cerr<<"[G]"<<group.size()<<std::endl;
    for(Group::const_iterator doc_iter= group.begin();
            doc_iter != group.end(); doc_iter++)
    {
        const ScdDocument &doc = doc_iter->doc;
        //bool has_location = doc_iter->locations.empty()? false : true;
        std::string name;
        doc.getString("Name", name);
        std::string from;
        doc.getString("FromCity", from);
        std::string url;
        doc.getString("Url", url);
        std::cerr<<"\t"<<name<<","<<from<<","<<doc_iter->locations.size()<<","<<doc_iter->is_free<<"{"<<url<<"}"<<std::endl;
        //std::string doc_id;
        //std::string from;
        //std::string to;
        //std::string time_plan;
        //std::string price;
        //doc.getString(SCD_DOC_ID,doc_id);
        //doc.getString(SCD_FROM_CITY,from);
        //doc.getString(SCD_TO_CITY,to);
        //doc.getString(SCD_TIME_PLAN,time_plan);
        //doc.getString(SCD_PRICE,price);
        //std::cout << "\t" << doc_id << ":" <<
        //            from << "," << to << " Time Plan:" <<
        //            time_plan << " Price:" << price << std::endl;
    }
    //    std::cout << "\n";
}

void TourProcessor::GenP_(Group& g, Document& doc) const
{
    assert(g.size() >= 1);
    doc = g[0].doc;
    doc.eraseProperty("uuid");
    std::string price;
    std::string p_docid;
    Set source_set;
    double min_price = g[0].price;
    double max_price = g[0].price;
    //get first doc uuid as pid
    g[0].doc.getString(SCD_UUID,p_docid);
    //set docid default use first docid
    doc.property(SCD_DOC_ID) = str_to_propstr(p_docid);
    for(std::size_t i=0;i<g.size();i++)
    {
        ScdDocument& doc_ref = g[i].doc;
        //get source list
        std::string source;
        doc_ref.getString(SCD_SOURCE,source);
        if(!source.empty()) source_set.insert(source);
        //get price range
        min_price = std::min(min_price,g[i].price);
        max_price = std::max(max_price,g[i].price);
        //maybe some other rules TODO........
    }

    //generate p <source>
    std::string source;
    for(Set::const_iterator it = source_set.begin();it!=source_set.end();++it)
    {
        if(!source.empty()) source+=",";
        source += *it;
    }
    doc.property(SCD_SOURCE)=str_to_propstr(source);

    //generate p <price>
    std::stringstream ss;
    if(min_price < max_price)
    {
        ss << min_price << '-'<< max_price;
    }
    else
    {
        assert(min_price == max_price);
        ss << min_price;
    }
    ss >> price;
    doc.property(SCD_PRICE) = str_to_propstr(price);
    doc.property("itemcount") = (int64_t)(g.size());
    //may have other attribute to set property todo......
}
