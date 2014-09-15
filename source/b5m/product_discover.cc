#include <idmlib/b5m/product_discover.h>
#include <idmlib/b5m/scd_doc_processor.h>

using namespace idmlib::b5m;
namespace bfs = boost::filesystem;

ProductDiscover::ProductDiscover(ProductMatcher* matcher, BrandExtractor* be)
: matcher_(matcher), be_(be), analyzer_(NULL), oid_(1), bid_(1), model_regex_("[a-zA-Z\\d\\-]{4,}"), capacity_regex_("(\\d+\\.)?\\d+([gG]|[mM][lL])(?=([^a-zA-Z]+|$))")
{
    error_model_regex_.push_back(boost::regex("^[a-z]+$"));
    error_model_regex_.push_back(boost::regex("\\d{2}\\-\\d{2}"));
    error_model_regex_.push_back(boost::regex("\\d{1,3}\\-\\d{1,2}0"));
    error_model_regex_.push_back(boost::regex("[a-z]*201\\d"));
    error_model_regex_.push_back(boost::regex("201\\d[a-z]*"));
    error_3cmodel_regex_.push_back(boost::regex("\\d+"));
    error_3cmodel_regex_.push_back(boost::regex("^.*cdma$"));
    error_3cmodel_regex_.push_back(boost::regex("^.*ghz$"));
    error_3cmodel_regex_.push_back(boost::regex("^.*\\d+mm$"));
    error_3cmodel_regex_.push_back(boost::regex("^.*wifi.*$"));
    error_3cmodel_regex_.push_back(boost::regex("^.*wi-fi.*$"));
    cregexps_.push_back(boost::regex("^手机数码>手机$"));
    cregexps_.push_back(boost::regex("^手机数码>摄像摄影.*$"));
    cregexps_.push_back(boost::regex("^家用电器>大家电.*$"));
    error_cregexps_.push_back(boost::regex("^.*配件$"));
    if(matcher_!=NULL)
    {
        matcher_->SetUsePriceSim(false);
    }
    idmlib::util::IDMAnalyzerConfig csconfig = idmlib::util::IDMAnalyzerConfig::GetCommonConfig("","", "");
    csconfig.symbol = true;
    analyzer_ = new idmlib::util::IDMAnalyzer(csconfig);
}

ProductDiscover::~ProductDiscover()
{
    if(analyzer_!=NULL) delete analyzer_;
}
uint32_t ProductDiscover::GetTerm_(const std::string& text) const
{
    term_t term = izenelib::util::HashFunction<std::string>::generateHash32(text);
    return term;
}

void ProductDiscover::GetWord_(const std::string& text, word_t& word)
{
    UString title(text, UString::UTF_8);
    std::vector<idmlib::util::IDMTerm> terms;
    analyzer_->GetTermList(title, terms);
    for(uint32_t i=0;i<terms.size();i++)
    {
        std::string str = terms[i].TextString();
        if(terms[i].tag==idmlib::util::IDMTermTag::SYMBOL)
        {
            continue;
        }
        boost::algorithm::to_lower(str);
        word.push_back(GetTerm_(str));
    }
}
bool ProductDiscover::Output_(const std::string& category, const std::string& brand, const std::string& model, double price, const std::string& otitle) const
{
    std::string url = "http://www.b5m.com/spus/"+category+"/"+brand+"/"+model;
    std::string docid = B5MHelper::GetPidByUrl(url);
    std::vector<std::string> cvec;
    boost::algorithm::split(cvec, category, boost::is_any_of(">"));
    const std::string& category_leaf = cvec.back();
    Document doc;
    doc.property("DOCID") = str_to_propstr(docid);
    doc.property("Price") = price;
    doc.property("Url") = str_to_propstr(url);
    std::string title = brand+" "+model+" "+category_leaf;
    doc.property("OTitle") = str_to_propstr(otitle);
    doc.property("Title") = str_to_propstr(title);
    doc.property("Category") = str_to_propstr(category);
    std::string attrib = "品牌:"+brand+",型号:"+model;
    doc.property("Attribute") = str_to_propstr(attrib);
    writer_->Append(doc);
    return true;
}
//bool ProductDiscover::Output_(const std::string& key, const CValue& value) const
//{
//    std::vector<std::string> vec;
//    boost::algorithm::split(vec, key, boost::is_any_of(","));
//    std::string category = vec[0];
//    std::string brand = vec[1];
//    std::string model = vec[2];
//    SMap::const_iterator mit = msmap_.find(model);
//    if(mit!=msmap_.end()) model = mit->second;
//    std::string url = "http://www.b5m.com/spus/"+category+"/"+brand+"/"+model;
//    std::string docid = B5MHelper::GetPidByUrl(url);
//    double sum = 0.0;
//    uint32_t count=0;
//    for(uint32_t i=0;i<value.size();i++)
//    {
//        const ScdDocument& doc = value[i];
//        std::string sprice;
//        doc.getString("Price", sprice);
//        ProductPrice price;
//        if(!price.Parse(sprice)) continue;
//        double dprice = 0.0;
//        price.GetMid(dprice);
//        if(dprice<=0.0) continue;
//        sum += dprice;
//        count++;
//    }
//    double price = sum/count;
//    std::vector<std::string> cvec;
//    boost::algorithm::split(cvec, category, boost::is_any_of(">"));
//    const std::string& category_leaf = cvec.back();
//
//    Document doc;
//    doc.property("DOCID") = str_to_propstr(docid);
//    doc.property("Price") = price;
//    doc.property("Url") = str_to_propstr(url);
//    std::string title = brand+" "+model+" "+category_leaf;
//    doc.property("OTitle") = value.front().property("Title");
//    doc.property("Title") = str_to_propstr(title);
//    doc.property("Category") = str_to_propstr(category);
//    std::string attrib = "品牌:"+brand+",型号:"+model;
//    doc.property("Attribute") = str_to_propstr(attrib);
//    writer_->Append(doc);
//    return true;
//}

bool ProductDiscover::CValid_(const std::string& key, const CValue& value) const
{
    uint32_t count=0;
    for(uint32_t i=0;i<value.size();i++)
    {
        const ScdDocument& doc = value[i];
        std::string sprice;
        doc.getString("Price", sprice);
        ProductPrice price;
        if(!price.Parse(sprice)) continue;
        double dprice = 0.0;
        price.GetMid(dprice);
        if(dprice<=0.0) continue;
        count++;
    }
    if(count<3) return false;
    return true;
}
void ProductDiscover::ProcessBrand_(ScdDocument& doc)
{
    std::string source;
    doc.getString("Source", source);
    if(source=="淘宝网"||source=="天猫")
    {
    }
    else
    {
        return;
    }
    UString attrib;
    doc.getString("Attribute", attrib);
    std::vector<Attribute> attributes;
    ProductMatcher::ParseAttributes(attrib, attributes);
    std::vector<std::string> brands;
    for(uint32_t i=0;i<attributes.size();i++)
    {
        if(attributes[i].name=="品牌") 
        {
            std::string avalue = attributes[i].GetValue();
            brands = attributes[i].values;
            break;
        }
    }
    bool tmatch = false;
    if(!brands.empty())
    {
        std::string title;
        doc.getString("Title", title);
        for(std::size_t i=0;i<brands.size();i++)
        {
            if(title.find(brands[i])!=std::string::npos)
            {
                tmatch = true;
                break;
            }
        }
    }
    boost::unique_lock<boost::mutex> lock(mutex_);
    bp_[0]++;
    if(!brands.empty()) bp_[1]++;
    if(tmatch) bp_[2]++;
    if(bp_[0]%10000==0)
    {
        LOG(INFO)<<"bp "<<bp_[0]<<","<<bp_[1]<<","<<bp_[2]<<std::endl;
    }
}
bool ProductDiscover::BrandProcess(const std::string& scd_path, int thread_num)
{
    bp_.resize(3, 0);
    ScdDocProcessor processor(boost::bind(&ProductDiscover::ProcessBrand_, this, _1), thread_num);
    processor.AddInput(scd_path);
    processor.Process();
    return true;
}
bool ProductDiscover::NgramProcess_(NgramStat& ngram)
{
    if(ngram.word.size()<4) return false;
    if(ngram.freq<5) return false;
    if(ngram.roid<=otitle_list_.size())
    {
        ngram.otitle = otitle_list_[ngram.roid-1];
    }
    return true;
}
bool ProductDiscover::NgramFilter_(NgramStat& ngram)
{
    if(ngram.word.size()<5) return false;
    double max_prop = 0.0;
    bool has_capacity = false;
    double en_limit = 0.3*std::log((double)ngram.nci_array.size())/std::log(2.0);
    double en = 0.0;
    for(std::size_t n=0;n<ngram.nci_array.size();n++)
    {
        const CosmeticsNgram::NCI& nci = ngram.nci_array[n];
        //std::cerr<<"[D]"<<nci.cid<<","<<nci.bid<<","<<nci.freq<<std::endl;
        std::string brand = bid_text_[nci.bid];
        UString ubrand(brand, UString::UTF_8);
        if(ubrand.length()<2) return false;
        std::string capacity;
        std::vector<double> price_list;
        for(std::size_t i=0;i<nci.cpa.size();i++)
        {
            const std::pair<std::string, double>& cp = nci.cpa[i];
            //std::cerr<<"\t"<<cp.first<<","<<cp.second<<std::endl;
            if(!cp.first.empty()) has_capacity = true;
            if(cp.first!=capacity&&!price_list.empty())
            {
                double prop = price_list.back()/price_list.front();
                if(prop>max_prop) max_prop = prop;
                price_list.resize(0);
            }
            capacity = cp.first;
            price_list.push_back(cp.second);
        }
        if(!price_list.empty())
        {
            double prop = price_list.back()/price_list.front();
            if(prop>max_prop) max_prop = prop;
        }
        double p = (double)nci.freq/ngram.freq;
        en += p*std::log(p)/std::log(2.0);
    }
    en*=-1.0;
    if(en>en_limit) return false;
    if(!has_capacity) return false;
    if(max_prop>2.5) return false;
    return true;
}

bool ProductDiscover::ProcessCosmetics(const std::string& path, int thread_num)
{
    std::string work_dir = path+"/work_dir";
    if(!boost::filesystem::exists(work_dir))
    {
        boost::filesystem::create_directories(work_dir);
    }
    {
        std::string spu_scd = path+"/SPU.SCD";
        ScdDocProcessor processor(boost::bind(&ProductDiscover::ProcessCosmeticsSPU_, this, _1), 1);
        processor.AddInput(spu_scd);
        processor.Process();
        std::cout<<"SPU brand size "<<smap_.size()<<std::endl;
        //for(SMap::const_iterator it = smap_.begin();it!=smap_.end();++it)
        //{
        //    std::cerr<<it->first<<" -> "<<it->second<<std::endl;
        //}
        std::cout<<"SPU bid map size "<<bid_map_.size()<<std::endl;
        //std::string bf_file = work_dir+"/brand_freq";
        //if(!boost::filesystem::exists(bf_file))
        //{
        //    izenelib::am::ssf::Util<>::Save(bf_file, brand_freq_);
        //}
    }
    ngram_processor_ = new CosmeticsNgram(work_dir);
    std::string file2 = work_dir+"/file2";
    std::string mid_file = work_dir+"/mid";
    if(boost::filesystem::exists(mid_file))
    {
        Reader reader(mid_file);
        reader.Open();
        NgramStat ngram;
        ResultMap rmap;
        while(reader.Next(ngram))
        {
            if(!NgramFilter_(ngram)) continue;
            std::pair<std::size_t, std::size_t> select(0, 0);
            for(std::size_t n=0;n<ngram.nci_array.size();n++)
            {
                const CosmeticsNgram::NCI& nci = ngram.nci_array[n];
                if(nci.freq>select.first)
                {
                    select.first = nci.freq;
                    select.second = n;
                }
            }
            const CosmeticsNgram::NCI& nci = ngram.nci_array[select.second];
            std::string brand = bid_text_[nci.bid];
            std::string category = category_text_[nci.cid];
            std::string model = ngram.GetText();
            std::vector<std::string> brands;
            boost::algorithm::split(brands, brand, boost::algorithm::is_any_of("/"));
            for(std::size_t i=0;i<brands.size();i++)
            {
                if(boost::algorithm::starts_with(model, brands[i]))
                {
                    model = model.substr(brands[i].length());
                    break;
                }
            }
            Result r(category, brand, model, ngram.otitle);
            ResultAppend_(rmap, r);
            //std::cerr<<brand<<"-"<<ngram.GetText()<<"("<<category<<")"<<std::endl;
        }
        reader.Close();
        Results all_results;
        for(ResultMap::const_iterator it = rmap.begin();it!=rmap.end();it++)
        {
            const Results& results = it->second;
            all_results.insert(all_results.end(), results.begin(), results.end());

        }
        std::sort(all_results.begin(), all_results.end());
        std::string output_scd = path+"/OUTPUT";
        if(boost::filesystem::exists(output_scd))
        {
            boost::filesystem::remove_all(output_scd);
        }
        boost::filesystem::create_directories(output_scd);
        ScdWriter writer(output_scd, UPDATE_SCD);
        for(std::size_t i=0;i<all_results.size();i++)
        {
            const Result& r = all_results[i];
            Document doc;
            std::string url = "http://www.b5m.com/spus/"+r.category+"/"+r.brand+"/"+r.model;
            std::string docid = izenelib::Utilities::generateMD5(url);
            doc.property("DOCID") = str_to_propstr(docid);
            //doc.property("Url") = str_to_propstr(url);
            doc.property("Category") = str_to_propstr(r.category);
            std::string attribute = "品牌:"+r.brand+",型号:"+r.model;
            std::string title = r.brand+" "+r.model;
            doc.property("Attribute") = str_to_propstr(attribute);
            //doc.property("Title") = str_to_propstr(title);
            doc.property("Price") = str_to_propstr("");
            doc.property("OTitle") = str_to_propstr(r.otitle);
            writer.Append(doc);
            //std::cerr<<r.category<<","<<r.brand<<","<<r.model<<std::endl;
        }
        writer.Close();
    }
    else if(boost::filesystem::exists(file2))
    {
        ngram_processor_->Finish(boost::bind(&ProductDiscover::NgramProcess_, this, _1));
    }
    else
    {
        std::string offer_scd = path+"/OFFER.SCD";
        ScdDocProcessor processor(boost::bind(&ProductDiscover::ProcessCosmetics_, this, _1), thread_num);
        processor.AddInput(offer_scd);
        processor.Process();
        std::cout<<"candidate map size "<<cmap_.size()<<std::endl;
        ngram_processor_->Finish(boost::bind(&ProductDiscover::NgramProcess_, this, _1));
    }
    return true;
}
void ProductDiscover::ResultAppend_(ResultMap& map, const Result& result)
{
    ResultMap::iterator it = map.find(result.brand);
    if(it==map.end())
    {
        Results v(1, result);
        map.insert(std::make_pair(result.brand, v));
    }
    else
    {
        Results& eresults = it->second;
        bool processed = false;
        for(std::size_t i=0;i<eresults.size();i++)
        {
            Result& eresult = eresults[i];
            if(eresult.model==result.model)
            {
                processed = true;
                break;
            }
            if(eresult.model.find(result.model)!=std::string::npos)
            {
                processed = true;
                break;
            }
            if(result.model.find(eresult.model)!=std::string::npos)
            {
                eresult = result;
                processed = true;
                break;
            }
        }
        if(!processed)
        {
            eresults.push_back(result);
        }
    }
}
bool ProductDiscover::Process(const std::string& scd_path, int thread_num)
{
    std::string category_file = scd_path+"/category";
    LoadCategory_(category_file);
    {
        std::string spu_scd = scd_path+"/SPU.SCD";
        ScdDocProcessor processor(boost::bind(&ProductDiscover::ProcessSPU_, this, _1), 1);
        processor.AddInput(spu_scd);
        processor.Process();
        std::cout<<"SPU map size "<<map_.size()<<std::endl;
    }
    std::string output_path = scd_path+"/OUTPUT";
    if(!boost::filesystem::exists(output_path))
    {
        boost::filesystem::create_directories(output_path);
    }
    std::string ser_file = output_path+"/data";
    if(boost::filesystem::exists(ser_file))
    {
        izenelib::am::ssf::Util<>::Load(ser_file, bm_manager_);
        BrandModelList& bm_list = bm_manager_.bm_list;
        std::cerr<<"BM size "<<bm_list.size()<<std::endl;
        for(std::size_t i=0;i<bm_list.size();i++)
        {
            BrandModelInfo& bmi = bm_list[i];
            if(!bmi.valid) continue;
            const std::string& brand = bmi.brand;
            std::string model = bmi.model;
            if(ModelFilter3C_(model))
            {
                bmi.valid = false;
                continue;
            }
            std::size_t id = bmi.id;
            std::vector<Found> found_list;
            std::cerr<<"[BMI]"<<brand<<","<<model<<","<<id<<std::endl;
            for(CoocMap::const_iterator it = bmi.cooc.begin();it!=bmi.cooc.end();++it)
            {
                term_t cid = it->first.first;
                std::size_t tid = it->first.second;
                const CoocValue& v = it->second;
                std::string category = category_text_[cid];
                BrandModelInfo& tbmi = bm_list[tid];
                std::cerr<<"\t"<<category<<","<<tbmi.brand<<","<<tbmi.model<<","<<v.count<<","<<v.otitle<<std::endl;
                //if(v.count<10) continue;
            }
        }
        //return true;
    }
    else
    {
        std::string offer_scd = scd_path+"/OFFER.SCD";
        ScdDocProcessor processor(boost::bind(&ProductDiscover::Process_, this, _1), thread_num);
        processor.AddInput(offer_scd);
        processor.Process();
        std::cout<<"candidate map size "<<cmap_.size()<<std::endl;
        izenelib::am::ssf::Util<>::Save(ser_file, bm_manager_);
    }
    writer_.reset(new ScdWriter(output_path, UPDATE_SCD));
    BrandModelList& bm_list = bm_manager_.bm_list;
    for(std::size_t i=0;i<bm_list.size();i++)
    {
        BrandModelInfo& bmi = bm_list[i];
        std::string& model = bmi.model;
        SMap::const_iterator mit = msmap_.find(model);
        if(mit!=msmap_.end()) model = mit->second;
    }
    for(std::size_t i=0;i<bm_list.size();i++)
    {
        BrandModelInfo& bmi = bm_list[i];
        if(!bmi.valid) continue;
        const std::string& brand = bmi.brand;
        std::string model = bmi.model;
        std::size_t id = bmi.id;
        std::vector<Found> found_list;
        for(CoocMap::iterator it = bmi.cooc.begin();it!=bmi.cooc.end();++it)
        {
            term_t cid = it->first.first;
            std::size_t tid = it->first.second;
            CoocValue& v = it->second;
            if(!v.valid) continue;
            if(v.count<3) continue;
            if(id==tid)
            {
                std::vector<double>& price_list = v.price_list;
                std::sort(price_list.begin(), price_list.end());
                double mid_price = price_list[price_list.size()/2];
                std::pair<double, double> price_range(mid_price/2.0, mid_price*2.0);
                double price = 0.0;
                //check price_list and gen price
                double en_max = std::log((double)price_list.size())/std::log(2.0);
                double en_limit = en_max*0.3;
                std::cerr<<"[TP]"<<price_list.size()<<","<<en_max<<","<<en_limit<<std::endl;
                double en = 0.0;
                std::size_t in_range_count=0;
                for(std::size_t f=0;f<price_list.size();f++)
                {
                    std::cerr<<"[MP]"<<price_list[f]<<std::endl;
                    if(price_list[f]>=price_range.first&&price_list[f]<=price_range.second)
                    {
                        in_range_count++;
                    }
                    price += price_list[f];
                }
                for(std::size_t f=0;f<price_list.size();f++)
                {
                    double p = (double)price_list[f]/price;
                    en += p*std::log(p)/std::log(2.0);
                }
                en*=-1.0;
                price /= price_list.size();
                std::cerr<<"[AP]"<<price<<","<<en<<","<<mid_price<<","<<in_range_count<<std::endl;
                if(in_range_count<price_list.size()*0.7) continue;
                //if(en<en_limit) continue;


                //if valid;
                Found found;
                found.count = v.count;
                found.cid = cid;
                found.price = mid_price;
                found.otitle = v.otitle;
                found_list.push_back(found);
            }
        }
        if(found_list.empty()) continue;
        std::sort(found_list.begin(), found_list.end());
        std::size_t max_count = found_list.front().count;
        for(std::size_t f=0;f<found_list.size();f++)
        {
            std::size_t count = found_list[f].count;
            double ratio = (double)count/max_count;
            if(ratio<0.8) 
            {
                found_list.resize(f+1);
                break;
            }
        }
        for(std::size_t f=0;f<found_list.size();f++)
        {
            std::size_t count = found_list[f].count;
            term_t cid = found_list[f].cid;
            std::string omodel = model;
            for(CoocMap::iterator it2 = bmi.cooc.begin();it2!=bmi.cooc.end();++it2)
            {
                term_t tcid = it2->first.first;
                if(tcid!=cid) continue;
                std::size_t tid = it2->first.second;
                if(tid==id) continue;
                BrandModelInfo& tbmi = bm_list[tid];
                if(!tbmi.valid) continue;
                CoocValue& v = it2->second;
                if(!v.valid) continue;
                std::pair<std::size_t, std::size_t> count_pair(count, v.count);
                if(count_pair.second<count_pair.first) std::swap(count_pair.first, count_pair.second);
                double ratio = (double)count_pair.first/count_pair.second;
                if(ratio>=0.8)
                {
                    //can merge model text
                    omodel += "/"+tbmi.model;
                    std::cerr<<"[MERGE]"<<brand<<"\t"<<omodel<<"\t"<<count_pair.first<<","<<count_pair.second<<std::endl;
                    v.valid = false;
                    break;
                }
            }
            Output_(category_text_[cid], brand, omodel, found_list[f].price, found_list[f].otitle);
        }
    }
    //for(CMap::const_iterator it = cmap_.begin(); it!=cmap_.end(); it++)
    //{
    //    if(CValid_(it->first, it->second))
    //    {
    //        Output_(it->first, it->second);
    //    }
    //}
    writer_->Close();
    return true;
}
void ProductDiscover::ProcessCosmeticsSPU_(ScdDocument& doc)
{
    std::string category;
    doc.getString("Category", category);
    if(!boost::algorithm::starts_with(category, "美容美妆")) return;
    CategoryIndex::const_iterator cit = category_index_.find(category);
    if(cit==category_index_.end())
    {
        term_t cid = category_index_.size()+1;
        category_index_[category] = cid;
        category_text_[cid] = category;
    }
    UString attrib;
    doc.getString("Attribute", attrib);
    std::vector<Attribute> attributes;
    ProductMatcher::ParseAttributes(attrib, attributes);
    for(uint32_t i=0;i<attributes.size();i++)
    {
        if(attributes[i].name=="品牌") 
        {
            std::string avalue = attributes[i].GetValue();
            bool is_bad = false;
            for(uint32_t a=0;a<attributes[i].values.size();a++)
            {
                boost::algorithm::to_lower(attributes[i].values[a]);
                UString ua(attributes[i].values[a], UString::UTF_8);
                if(ua.length()<2)
                {
                    is_bad = true;
                }
            }
            if(is_bad) break;
            StringBidMap::const_iterator it = sbid_map_.find(avalue);
            term_t bid = 0;
            if(it==sbid_map_.end())
            {
                bid = bid_;
                bid_++;
            }
            else bid = it->second;
            for(uint32_t a=0;a<attributes[i].values.size();a++)
            {
                const std::string& text = attributes[i].values[a];
                smap_[text] = avalue;
                word_t word;
                GetWord_(text, word);
                bid_map_[word] = bid;
                bid_text_[bid] = avalue;
            }
            BrandFreq::iterator bit = brand_freq_.find(bid);
            if(bit==brand_freq_.end())
            {
                brand_freq_.insert(std::make_pair(bid, 1));
            }
            else
            {
                bit->second+=1;
            }
            break;
        }
    }
}

void ProductDiscover::ProcessSPU_(ScdDocument& doc)
{
    std::string category;
    doc.getString("Category", category);
    if(!ValidCategory_(category)) return;
    UString attrib;
    doc.getString("Attribute", attrib);
    std::vector<Attribute> attributes;
    ProductMatcher::ParseAttributes(attrib, attributes);
    std::vector<std::string> brands;
    std::vector<std::string> models;
    for(uint32_t i=0;i<attributes.size();i++)
    {
        if(attributes[i].name=="品牌") 
        {
            std::string avalue = attributes[i].GetValue();
            brands = attributes[i].values;
            for(uint32_t a=0;a<attributes[i].values.size();a++)
            {
                boost::algorithm::to_lower(attributes[i].values[a]);
            }
            for(uint32_t a=0;a<attributes[i].values.size();a++)
            {
                smap_[attributes[i].values[a]] = avalue;
            }
            brands.clear();
            brands.push_back(avalue);
        }
        else if(attributes[i].name=="型号") models = attributes[i].values;
    }
    if(brands.empty()||models.empty()) return;
    for(uint32_t i=0;i<brands.size();i++)
    {
        Key key(category, brands[i]);
        Map::iterator it = map_.find(key);
        if(it==map_.end())
        {
            map_.insert(std::make_pair(key, models));
        }
        else
        {
            it->second.insert(it->second.end(), models.begin(), models.end());
        }
    }
    
}
void ProductDiscover::LoadCategory_(const std::string& file)
{
    std::string line;
    std::ifstream ifs(file.c_str());
    term_t cid=1;
    while(getline(ifs, line))
    {
        boost::algorithm::trim(line);
        std::vector<std::string> vec;
        boost::algorithm::split(vec, line, boost::algorithm::is_any_of(","));
        if(vec.size()<1) continue;
        const std::string& scategory = vec[0];
        if(scategory.empty()) continue;
        category_index_[scategory] = cid;
        category_text_[cid] = scategory;
        cid++;
    }
    ifs.close();
}
bool ProductDiscover::GetCategoryId_(const std::string& category, term_t& cid)
{
    std::string cs = category;
    if(boost::algorithm::ends_with(cs, ">"))
    {
        cs = cs.substr(0, cs.length()-1);
    }
    CategoryIndex::const_iterator it = category_index_.find(cs);
    if(it==category_index_.end()) return false;
    cid = it->second;
    return true;
    
}
void ProductDiscover::ProcessCosmetics_(ScdDocument& doc)
{
    std::string category;
    doc.getString("Category", category);
    if(!boost::algorithm::starts_with(category, "美容美妆")) return;
    Product product;
    matcher_->Process(doc, product);
    //if(!product.spid.empty()&&!product.stitle.empty()) return;
    if(product.type==Product::SPU) 
    {
        //std::string title;
        //doc.getString("Title", title);
        //std::cerr<<"matched: "<<title<<std::endl;
        return;
    }
    //if(!product.why.empty()&&product.why!="text_error") return;
    std::vector<Ngram> ngrams;
    if(!Extract_(doc, ngrams)) return;
    ngram_processor_->Insert(ngrams);
}

bool ProductDiscover::Extract_(const ScdDocument& doc, std::vector<Ngram>& ngrams)
{
    std::string category;
    doc.getString("Category", category);
    Ngram ngram;
    if(!GetCategoryId_(category, ngram.cid)) return false;
    UString title;
    doc.getString("Title", title);
    std::vector<idmlib::util::IDMTerm> terms;
    analyzer_->GetTermList(title, terms);
    word_t brand_word;
    //term_t bid = 0;
    //std::size_t bend=0;
    std::vector<std::pair<term_t, std::size_t> > bid_list;
    std::vector<word_t> bid_word_list;
    for(uint32_t i=0;i<terms.size();i++)
    {
        std::string str = terms[i].TextString();
        if(terms[i].tag==idmlib::util::IDMTermTag::SYMBOL)
        {
            continue;
        }
        boost::algorithm::to_lower(str);
        term_t id = GetTerm_(str);
        brand_word.push_back(id);
        BidMap::const_iterator it = bid_map_.lower_bound(brand_word);
        if(it==bid_map_.end()||!boost::algorithm::starts_with(it->first, brand_word))
        {
            brand_word.resize(0);
        }
        else if(it->first==brand_word)
        {
            std::size_t bend = i;
            term_t bid = it->second; 
            bid_list.push_back(std::make_pair(bid, bend));
            bid_word_list.push_back(brand_word);
        }
    }
    if(bid_list.empty()) return false;
    if(bid_list.size()>1)
    {
        for(std::size_t i=0;i<bid_word_list.size();i++)
        {
            term_t ibid = bid_list[i].first;
            if(ibid==0) continue;
            for(std::size_t j=i+1;j<bid_word_list.size();j++)
            {
                term_t jbid = bid_list[j].first;
                if(jbid==0) continue;
                if(ibid==jbid)
                {
                    bid_list[j].first = 0;
                }
                else if(boost::algorithm::starts_with(bid_word_list[i], bid_word_list[j]))
                {
                    bid_list[j].first = 0;
                }
                else if(boost::algorithm::starts_with(bid_word_list[j], bid_word_list[i]))
                {
                    bid_list[i].first = 0;
                }
            }
        }
    }
    std::pair<std::size_t, std::size_t> max(0,0);
    for(std::size_t i=0;i<bid_list.size();i++)
    {
        if(bid_list[i].first==0) continue;
        std::size_t freq = brand_freq_[bid_list[i].first];
        if(freq>max.first)
        {
            max.first = freq;
            max.second = i;
        }
    }
    ngram.bid = bid_list[max.second].first;
    std::size_t bend = bid_list[max.second].second;
    ngram.price = ProductPrice::ParseDocPrice(doc);
    if(ngram.price<=0.0) return false;
    std::string stitle;
    title.convertString(stitle, UString::UTF_8);
    //std::cerr<<"[B]"<<stitle<<",{"<<bid_text_[ngram.bid]<<"}"<<std::endl;
    std::string::const_iterator start = stitle.begin();
    std::string::const_iterator end = stitle.end();
    boost::smatch what;
    if( boost::regex_search( start, end, what, capacity_regex_))
    {
        std::string tmatch(what[0].first, what[0].second);
        ngram.capacity = tmatch;
    }
    word_t word;
    std::vector<std::string> text;
    for(uint32_t i=bend+1;i<terms.size();i++)
    {
        std::string str = terms[i].TextString();
        boost::algorithm::to_lower(str);
        if(terms[i].text.isChineseChar(0))
        {
            word.push_back(GetTerm_(str));
            text.push_back(str);
        }
        else
        {
            if(word.size()>=4)
            {
                ngrams.push_back(ngram);
                Ngram& n = ngrams.back();
                n.word = word;
                n.text = text;
            }
            word.clear();
            text.clear();
        }
    }
    if(word.size()>=4)
    {
        ngrams.push_back(ngram);
        Ngram& n = ngrams.back();
        n.word = word;
        n.text = text;
    }
    if(ngrams.empty()) return false;
    std::size_t oid = 0;
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        otitle_list_.push_back(stitle);
        oid = otitle_list_.size();
    }
    for(std::size_t i=0;i<ngrams.size();i++)
    {
        ngrams[i].oid = oid;
    }
    return true;

}

void ProductDiscover::Process_(ScdDocument& doc)
{
    std::string category;
    doc.getString("Category", category);
    if(boost::algorithm::ends_with(category, ">"))
    {
        category = category.substr(0, category.length()-1);
        doc.property("Category") = str_to_propstr(category);
    }
    if(!ValidCategory_(category)) return;
    //std::string spu_title;
    //doc.getString(B5MHelper::GetSPTPropertyName(), spu_title);
    //if(!spu_title.empty()) return;
    Product product;
    matcher_->Process(doc, product);
    std::string title;
    doc.getString("Title", title);
    //std::cerr<<"Processing title "<<title<<","<<category<<std::endl;
    //std::cerr<<"Matched product "<<product.stitle<<","<<(int)product.type<<std::endl;
    if(product.type==Product::SPU&&!product.stitle.empty()) return;
    //if(!product.why.empty()&&product.why!="text_error") return;
    std::string brand;
    be_->Evaluate(doc, brand);
    if(brand.empty()) return;
    term_t cid=0;
    if(!GetCategoryId_(category, cid)) return;
    double price = ProductPrice::ParseDocPrice(doc);
    if(price<=0.0) return;
    //std::cout<<title<<"\t"<<category<<"\t"<<sprice<<std::endl;
    std::vector<std::string> models;
    ExtractModels_(title, models);
    if(models.empty()) return;
    boost::unique_lock<boost::mutex> lock(mutex_);
    bm_manager_.add(cid, brand, models, title, price);
    //for(uint32_t i=0;i<brands.size();i++)
    //{
    //    std::cout<<"\t[BRAND]"<<brands[i]<<std::endl;
    //}
    //for(uint32_t i=0;i<models.size();i++)
    //{
    //    std::cout<<"\t[MODEL]"<<models[i]<<std::endl;
    //}
}

bool ProductDiscover::ValidCategory_(const std::string& c) const
{
    std::string category = c;
    if(boost::algorithm::ends_with(category, ">"))
    {
        category = category.substr(0, category.length()-1);
    }
    for(uint32_t i=0;i<error_cregexps_.size();i++)
    {
        if(boost::regex_match(category, error_cregexps_[i])) return false;
    }
    for(uint32_t i=0;i<cregexps_.size();i++)
    {
        if(boost::regex_match(category, cregexps_[i])) return true;
    }
    return false;
}
void ProductDiscover::ExtractModels_(const std::string& otitle, std::vector<std::string>& models)
{
    boost::sregex_token_iterator iter(otitle.begin(), otitle.end(), model_regex_, 0);
    boost::sregex_token_iterator end;
    for( ; iter!=end; ++iter)
    {
        std::string ocandidate = *iter;
        std::string candidate = boost::algorithm::to_lower_copy(ocandidate);
        if(candidate[0]=='-' || candidate[candidate.length()-1]=='-') continue;
        //if(kset.find(candidate)!=kset.end()) continue;
        bool error = false;
        for(uint32_t e=0;e<error_model_regex_.size();e++)
        {
            if(boost::regex_match(candidate, error_model_regex_[e]))
            {
                error = true;
                break;
            }
        }
        if(error) continue;
        if(ModelFilter3C_(candidate)) continue;
        models.push_back(candidate);
        boost::unique_lock<boost::mutex> lock(mutex_);
        msmap_[candidate] = ocandidate;
    }
}
bool ProductDiscover::ModelFilter3C_(const std::string& model) const
{
    bool error = false;
    for(uint32_t e=0;e<error_3cmodel_regex_.size();e++)
    {
        if(boost::regex_match(model, error_3cmodel_regex_[e]))
        {
            error = true;
            break;
        }
    }
    if(error) return true;
    return false;
    UString um(model, UString::UTF_8);
    bool all_digit = true;
    for(std::size_t i=0;i<um.length();i++)
    {
        if(!um.isDigitChar(i))
        {
            all_digit = false;
            break;
        }
    }
    if(all_digit) return true;
    if(boost::algorithm::ends_with(model, "ghz")) return true;
    return false;
}

void ProductDiscover::GetBrandAndModel_(const ScdDocument& doc, std::vector<std::string>& brands, std::vector<std::string>& models)
{
    std::string otitle;
    doc.getString("Title", otitle);
    std::string title = boost::algorithm::to_lower_copy(otitle);
    boost::algorithm::trim(title);
    UString text(title, UString::UTF_8);
    
    std::string category;
    doc.getString("Category", category);
    std::vector<ProductMatcher::KeywordTag> keywords;
    matcher_->GetKeywords(text, keywords);
    boost::unordered_set<std::string> kset;
    for(uint32_t i=0;i<keywords.size();i++)
    {
        const ProductMatcher::KeywordTag& tag = keywords[i];
        std::string str;
        tag.text.convertString(str, UString::UTF_8);
        kset.insert(str);
        for(uint32_t j=0;j<tag.attribute_apps.size();j++)
        {
            if(tag.attribute_apps[j].attribute_name=="品牌")
            {
                std::string brand = str;
                SMap::const_iterator it = smap_.find(brand);
                if(it!=smap_.end())
                {
                    brand = it->second;
                }
                Key key(category, brand);
                if(map_.find(key)!=map_.end())
                {
                    bool exists = false;
                    for(uint32_t b=0;b<brands.size();b++)
                    {
                        if(brands[b]==brand)
                        {
                            exists = true;
                            break;
                        }
                    }
                    if(!exists) brands.push_back(brand);
                }
            }
        }
    }
    boost::sregex_token_iterator iter(otitle.begin(), otitle.end(), model_regex_, 0);
    boost::sregex_token_iterator end;
    for( ; iter!=end; ++iter)
    {
        std::string ocandidate = *iter;
        std::string candidate = boost::algorithm::to_lower_copy(ocandidate);
        if(candidate[0]=='-' || candidate[candidate.length()-1]=='-') continue;
        if(kset.find(candidate)!=kset.end()) continue;
        bool error = false;
        for(uint32_t e=0;e<error_model_regex_.size();e++)
        {
            if(boost::regex_match(candidate, error_model_regex_[e]))
            {
                error = true;
                break;
            }
        }
        if(error) continue;
        models.push_back(candidate);
        boost::unique_lock<boost::mutex> lock(mutex_);
        msmap_[candidate] = ocandidate;
    }
}


