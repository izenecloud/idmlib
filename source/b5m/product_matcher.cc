#include <idmlib/b5m/product_matcher.h>
#include <idmlib/b5m/scd_doc_processor.h>
#include <idmlib/b5m/category_psm.h>
#include <util/hashFunction.h>
#include <sf1common/ScdParser.h>
#include <sf1common/ScdWriter.h>
#include <sf1common/JsonDocument.h>
#include <sf1common/split_ustr.h>
#include <idmlib/b5m/product_price.h>
#include <boost/unordered_set.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/serialization/hash_map.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <algorithm>
#include <cmath>
#include <idmlib/util/svm.h>
#include <util/functional.h>
#include <util/ClockTimer.h>
#include <3rdparty/json/json.h>

using namespace idmlib::b5m;
using namespace idmlib::sim;
using namespace idmlib::kpe;
using namespace idmlib::util;
namespace bfs = boost::filesystem;


//#define B5M_DEBUG

const std::string ProductMatcher::AVERSION("20130901000000");

ProductMatcher::KeywordTag::KeywordTag():type_app(0), kweight(0.0), ngram(1)
{
}

void ProductMatcher::KeywordTag::Flush()
{
    std::sort(category_name_apps.begin(), category_name_apps.end());
    std::vector<CategoryNameApp> new_category_name_apps;
    new_category_name_apps.reserve(category_name_apps.size());
    for(uint32_t i=0;i<category_name_apps.size();i++)
    {
        const CategoryNameApp& app = category_name_apps[i];
        if(new_category_name_apps.empty())
        {
            new_category_name_apps.push_back(app);
            continue;
        }
        CategoryNameApp& last_app = new_category_name_apps.back();
        if(app.cid==last_app.cid)
        {
            if(app.is_complete&&!last_app.is_complete)
            {
                last_app = app;
            }
            else if(app.is_complete==last_app.is_complete&&app.depth<last_app.depth)
            {
                last_app = app;
            }
        }
        else
        {
            new_category_name_apps.push_back(app);
        }
    }
    std::swap(new_category_name_apps, category_name_apps);
    //SortAndUnique(category_name_apps);
    SortAndUnique(attribute_apps);
    //SortAndUnique(spu_title_apps);
}
void ProductMatcher::KeywordTag::Append(const KeywordTag& another, bool is_complete)
{
    if(is_complete)
    {
        for(uint32_t i=0;i<another.category_name_apps.size();i++)
        {
            CategoryNameApp aapp = another.category_name_apps[i];
            if(!is_complete) aapp.is_complete = false;
            category_name_apps.push_back(aapp);
        }
    }
    if(is_complete)
    {
        attribute_apps.insert(attribute_apps.end(), another.attribute_apps.begin(), another.attribute_apps.end());
    }
    //spu_title_apps.insert(spu_title_apps.end(), another.spu_title_apps.begin(), another.spu_title_apps.end());
}
void ProductMatcher::KeywordTag::PositionMerge(const Position& pos)
{
    bool combine = false;
    for(uint32_t i=0;i<positions.size();i++)
    {
        if(positions[i].Combine(pos))
        {
            combine = true;
            break;
        }
    }
    if(!combine)
    {
        positions.push_back(pos);
    }
}
bool ProductMatcher::KeywordTag::Combine(const KeywordTag& another)
{
    //check if positions were overlapped
    //for(uint32_t i=0;i<another.positions.size();i++)
    //{
        //const Position& ap = another.positions[i];
        ////std::cerr<<"ap "<<ap.first<<","<<ap.second<<std::endl;
        //for(uint32_t j=0;j<positions.size();j++)
        //{
            //const Position& p = positions[j];
            ////std::cerr<<"p "<<p.first<<","<<p.second<<std::endl;
            //bool _overlapped = true;
            //if( ap.first>=p.second || p.first>=ap.second) _overlapped = false;
            //if(_overlapped)
            //{
                //return false;
            //}
        //}
    //}
    category_name_apps.clear();
    std::vector<AttributeApp> new_attribute_apps;
    new_attribute_apps.reserve(std::max(attribute_apps.size(), another.attribute_apps.size()));

    uint32_t i=0;
    uint32_t j=0;
    while(i<attribute_apps.size()&&j<another.attribute_apps.size())
    {
        AttributeApp& app = attribute_apps[i];
        const AttributeApp& aapp = another.attribute_apps[j];
        if(app.spu_id==aapp.spu_id)
        {
            if(app.attribute_name!=aapp.attribute_name)
            {
                new_attribute_apps.push_back(app);
                new_attribute_apps.push_back(aapp);
                //app.attribute_name += "+"+aapp.attribute_name;
                //app.is_optional = app.is_optional | aapp.is_optional;
            }
            else
            {
                new_attribute_apps.push_back(app);
            }
            //else
            //{
                //app.spu_id = 0;
            //}
            ++i;
            ++j;
        }
        else if(app.spu_id<aapp.spu_id)
        {
            //app.spu_id = 0;
            ++i;
        }
        else
        {
            ++j;
        }
    }
    std::swap(new_attribute_apps, attribute_apps);
    //i = 0;
    //j = 0;
    //while(i<spu_title_apps.size()&&j<another.spu_title_apps.size())
    //{
        //SpuTitleApp& app = spu_title_apps[i];
        //const SpuTitleApp& aapp = another.spu_title_apps[j];
        //if(app.spu_id==aapp.spu_id)
        //{
            //app.pstart = std::min(app.pstart, aapp.pstart);
            //app.pend = std::max(app.pend, aapp.pend);
            //++i;
            //++j;
        //}
        //else if(app<aapp)
        //{
            //app.spu_id = 0;
            //++i;
        //}
        //else
        //{
            //++j;
        //}
    //}
    //while(i<spu_title_apps.size())
    //{
        //SpuTitleApp& app = spu_title_apps[i];
        //app.spu_id = 0;
        //++i;
    //}
    ++ngram;
    positions.insert(positions.end(), another.positions.begin(), another.positions.end());
    return true;
}

bool ProductMatcher::KeywordTag::IsAttribSynonym(const KeywordTag& another) const
{
    bool result = false;
    for(uint32_t i=0;i<attribute_apps.size();i++)
    {
        const AttributeApp& ai = attribute_apps[i];
        for(uint32_t j=0;j<another.attribute_apps.size();j++)
        {
            const AttributeApp& aj = another.attribute_apps[j];
            if(ai==aj)
            {
                result = true;
                break;
            }
        }
        if(result) break;
    }
    return result;
}
bool ProductMatcher::KeywordTag::IsBrand() const
{
    for(uint32_t i=0;i<attribute_apps.size();i++)
    {
        const AttributeApp& ai = attribute_apps[i];
        if(ai.attribute_name=="品牌") return true;
    }
    return false;
}
bool ProductMatcher::KeywordTag::IsModel() const
{
    for(uint32_t i=0;i<attribute_apps.size();i++)
    {
        const AttributeApp& ai = attribute_apps[i];
        if(ai.attribute_name=="型号") return true;
    }
    return false;
}
bool ProductMatcher::KeywordTag::IsCategoryKeyword() const
{
    return !category_name_apps.empty();
}

ProductMatcher::ProductMatcher()
:is_open_(false),
 use_price_sim_(true),
 analyzer_(NULL), char_analyzer_(NULL), chars_analyzer_(NULL),
 test_docid_("7bc999f5d10830d0c59487bd48a73cae"),
 left_bracket_("("), right_bracket_(")"), place_holder_("__PLACE_HOLDER__"), blank_(" "),
 left_bracket_term_(0), right_bracket_term_(0), place_holder_term_(0),
 type_regex_("[a-zA-Z\\d\\-]{4,}"), vol_regex_("^(8|16|32|64)gb?$"),
 book_category_("书籍/杂志/报纸"),
 spu_matcher_(NULL),
 psm_(NULL),
 brand_manager_(new BrandManager)
{
}

ProductMatcher::~ProductMatcher()
{
    if(analyzer_!=NULL)
    {
        delete analyzer_;
    }
    if(char_analyzer_!=NULL)
    {
        delete char_analyzer_;
    }
    if(chars_analyzer_!=NULL)
    {
        delete chars_analyzer_;
    }
    if(spu_matcher_!=NULL) delete spu_matcher_;
    if(psm_!=NULL) delete psm_;
    if(brand_manager_!=NULL) delete brand_manager_;
    //for(uint32_t i=0;i<psms_.size();i++)
    //{
        //delete psms_[i];
    //}
    //if(cr_result_!=NULL)
    //{
        //cr_result_->flush();
        //delete cr_result_;
    //}
}

bool ProductMatcher::IsOpen() const
{
    return is_open_;
}

bool ProductMatcher::Open(const std::string& kpath)
{
    if(!is_open_)
    {
        path_ = kpath;
        try
        {
            Init_();
            std::string path = path_+"/products";
            izenelib::am::ssf::Util<>::Load(path, products_);
            SetProductsId_();
            LOG(INFO)<<"products size "<<products_.size()<<std::endl;
            path = path_+"/category_list";
            izenelib::am::ssf::Util<>::Load(path, category_list_);
            LOG(INFO)<<"category list size "<<category_list_.size()<<std::endl;
            //path = path_+"/keywords";
            //izenelib::am::ssf::Util<>::Load(path, keywords_thirdparty_);
            path = path_+"/category_index";
            izenelib::am::ssf::Util<>::Load(path, category_index_);
            path = path_+"/cid_to_pids";
            izenelib::am::ssf::Util<>::Load(path, cid_to_pids_);
            path = path_+"/product_index";
            izenelib::am::ssf::Util<>::Load(path, product_index_);
            path = path_+"/keyword_trie";
            izenelib::am::ssf::Util<>::Load(path, trie_);
            LOG(INFO)<<"trie size "<<trie_.size()<<std::endl;
            //path = path_+"/fuzzy_trie";
            //izenelib::am::ssf::Util<>::Load(path, ftrie_);
            //LOG(INFO)<<"fuzzy size "<<ftrie_.size()<<std::endl;
            //path = path_+"/term_index";
            //std::map<cid_t, TermIndex> tmap;
            //izenelib::am::ssf::Util<>::Load(path, tmap);
            //term_index_map_.insert(tmap.begin(), tmap.end());
            //LOG(INFO)<<"term_index map size "<<term_index_map_.size()<<std::endl;
            //path = path_+"/back2front";
            //std::map<std::string, std::string> b2f_map;
            //izenelib::am::ssf::Util<>::Load(path, b2f_map);
            //back2front_.insert(b2f_map.begin(), b2f_map.end());
            //LOG(INFO)<<"back2front size "<<back2front_.size()<<std::endl;
            path = path_+"/book_category";
            if(boost::filesystem::exists(path))
            {
                izenelib::am::ssf::Util<>::Load(path, book_category_);
            }
            //path = path_+"/feature_vector";
            //izenelib::am::ssf::Util<>::LoadOne(path, feature_vectors_);

            //for(uint32_t i=0;i<feature_vectors_.size();i++)
            //{
            //    std::string category = category_list_[i].name;
            //    cid_t cid = GetLevelCid_(category, 1);
            //    first_level_category_[cid] = 1;
            //}
            std::vector<std::pair<size_t, string> > synonym_pairs;
            path = path_+"/synonym_map";
            izenelib::am::ssf::Util<>::Load(path, synonym_pairs);
            for (size_t i = 0; i < synonym_pairs.size(); ++i)
                synonym_map_.insert(std::make_pair(synonym_pairs[i].second, synonym_pairs[i].first));

            std::vector<string> tmp_sets;
            path = path_+"/synonym_dict";
            izenelib::am::ssf::Util<>::Load(path, tmp_sets);
            for (size_t i = 0; i < tmp_sets.size(); ++i)
            {
                std::vector<string> tmp_set;
                boost::algorithm::split(tmp_set, tmp_sets[i], boost::algorithm::is_any_of("/"));
                synonym_dict_.push_back(tmp_set);
            }

            LOG(INFO)<<"synonym map size "<<synonym_pairs.size();
            LOG(INFO)<<"synonym dict size "<<synonym_dict_.size();
            //path = path_+"/spu_price";
            //if(boost::filesystem::exists(path))
            //{
            //    LOG(INFO)<<"loading spu price"<<std::endl;
            //    std::ifstream ifs(path.c_str());
            //    std::string line;
            //    boost::unordered_map<std::string, ProductPrice> spu_price;
            //    while(getline(ifs, line))
            //    {
            //        boost::algorithm::trim(line);
            //        std::vector<std::string> vec;
            //        boost::algorithm::split(vec, line, boost::algorithm::is_any_of(","));
            //        if(vec.size()<3) continue;
            //        std::string spid = vec[0];
            //        ProductPrice price;
            //        price.value.first = boost::lexical_cast<double>(vec[1]);
            //        price.value.second = boost::lexical_cast<double>(vec[2]);
            //        spu_price[spid] = price;
            //    }
            //    ifs.close();
            //    for(uint32_t i=1;i<products_.size();i++)
            //    {
            //        Product& p = products_[i];
            //        boost::unordered_map<std::string, ProductPrice>::const_iterator sit = spu_price.find(p.spid);
            //        if(sit!=spu_price.end())
            //        {
            //            p.price = sit->second;
            //            //LOG(INFO)<<"set spu price for "<<p.stitle<<" : "<<p.price.Min()<<","<<p.price.Max()<<std::endl;
            //        }
            //    }
            //    offer_prices_finish_ = true;
            //}
            for(std::size_t i=0;i<products_.size();i++)
            {
                Product& p = products_[i];
                if(boost::algorithm::starts_with(p.scategory, "美容美妆"))
                {
                    //LOG(INFO)<<"cosmetics "<<p.stitle<<std::endl;
                    p.sub_prop = "(\\d+\\.)?\\d+([gG]|[mM][lL])(?=([^a-zA-Z]+|$))";
                    //as it may have several different capacities;
                    p.price.value.first = 0.1;
                    p.price.value.second= 99999.0;
                }
                //LOG(INFO)<<"spu price "<<p.stitle<<","<<p.price.Min()<<","<<p.price.Max()<<std::endl;
                if(!p.sub_prop.empty())
                {
                    p.sub_prop_regex = boost::regex(p.sub_prop);
                }
                if(!p.stitle.empty()) p.type = Product::SPU;
            }
            path = path_+"/brand_manager";
            if(boost::filesystem::exists(path))
            {
                if(brand_manager_!=NULL) brand_manager_->Load(path);
            }
            path = path_+"/invalid_isbn";
            {
                std::ifstream ifs(path.c_str());
                std::string line;
                while(getline(ifs, line))
                {
                    boost::algorithm::trim(line);
                    invalid_isbn_.insert(line);
                }
                ifs.close();
            }
            LOG(INFO)<<"invalid isbn size "<<invalid_isbn_.size()<<std::endl;
            spu_matcher_ = new SpuMatcher;
            spu_matcher_->Load(path_);
            psm_ = new CategoryPsm(path_);
        }
        catch(std::exception& ex)
        {
            LOG(ERROR)<<"product matcher open failed : "<<ex.what()<<std::endl;
            return false;
        }
        is_open_ = true;
    }
    return true;
}
bool ProductMatcher::OpenPsm(const std::string& path)
{
    if(psm_!=NULL)
    {   
        psm_->Open(path);
        return true;
    }
    else
    {
        return false;
    }
}

void ProductMatcher::UpdateSynonym(const std::string& dict_path)
{
            std::map<string, size_t> synonym_map;
            std::vector<std::pair<size_t, string> > synonym_pairs;
            std::string path = path_+"/synonym_map";
            izenelib::am::ssf::Util<>::Load(path, synonym_pairs);
            for (size_t j = 0; j < synonym_pairs.size(); ++j)
                synonym_map.insert(std::make_pair(synonym_pairs[j].second, synonym_pairs[j].first));
                
            std::vector<std::vector<string> > synonym_dict;
            std::vector<string> tmp_sets;
            path = path_+"/synonym_dict";
            izenelib::am::ssf::Util<>::Load(path, tmp_sets);
            for (size_t j = 0; j < tmp_sets.size(); ++j)
            {
                std::vector<string> tmp_set;
                boost::algorithm::split(tmp_set, tmp_sets[j], boost::algorithm::is_any_of("/"));
                synonym_dict.push_back(tmp_set);
            }        
            
            std::ifstream ifs(dict_path.c_str());
            std::string line;
            while(getline(ifs, line))
            {
//                boost::algorithm::trim(line);
                boost::algorithm::to_lower(line);
                std::vector<std::string> vec;
                boost::algorithm::split(vec, line, boost::algorithm::is_any_of(","));
                size_t find = 0;
                size_t id = 0;
                for (size_t j = 0; j < vec.size(); ++j)
                    if (synonym_map.find(vec[j]) != synonym_map.end())
                    {
                        id = synonym_map[vec[j]];
                        find = 1;
                    }
                if (1 == find)
                {
                    for (size_t j = 0; j < vec.size(); ++j)
                        if (synonym_map.find(vec[j]) == synonym_map.end())
                        {
                            synonym_dict[id].push_back(vec[j]);
                            synonym_map.insert(std::make_pair(vec[j], id));
                        }
                }
                else
                {
                    size_t size = synonym_dict.size();
                    synonym_dict.push_back(vec);
                    for (size_t j = 0; j < vec.size(); ++j)
                    {
                        synonym_map.insert(std::make_pair(vec[j], size));
                    }
                }
            }
                        
            {
                izenelib::util::ScopedWriteLock<izenelib::util::ReadWriteLock> lock(lock_);
                synonym_map_ = synonym_map;
                synonym_dict_ = synonym_dict;
                std::cout<<"after update, synonym dict size = "<<synonym_dict_.size()<<'\n';
            }
}

bool ProductMatcher::GetSynonymSet(const UString& pattern, std::vector<UString>& synonym_set, int& setid)
{
    izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(lock_);
    
    if (synonym_map_.empty() || synonym_dict_.empty())
    {
        LOG(INFO)<<"synonym dict is empty!";
        return false;
    }
    string st;
    pattern.convertString(st, UString::UTF_8);
    if (synonym_map_.find(st) == synonym_map_.end())
    {
        return false;
    }
    else
    {
        setid = synonym_map_[st];
        for (size_t i = 0; i < synonym_dict_[setid].size(); ++i)
        {
            UString ust;
            ust.assign(synonym_dict_[setid][i], UString::UTF_8);
            synonym_set.push_back(ust);
        }
    }
    return true;
}

bool ProductMatcher::GetSynonymId(const UString& pattern, int& setid)
{
    izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(lock_);
    
    if (synonym_map_.empty() || synonym_dict_.empty())
    {
        LOG(INFO)<<"synonym dict is empty!";
        return false;
    }
    string st;
    pattern.convertString(st, UString::UTF_8);
    if (synonym_map_.find(st) == synonym_map_.end())
    {
        return false;
    }
    else
    {
        setid = synonym_map_[st];
    }
    return true;
}

std::string ProductMatcher::GetVersion(const std::string& path)
{
    std::string version_file = path+"/VERSION";
    std::string version;
    std::ifstream ifs(version_file.c_str());
    getline(ifs, version);
    boost::algorithm::trim(version);
    return version;
}
std::string ProductMatcher::GetAVersion(const std::string& path)
{
    std::string version_file = path+"/AVERSION";
    std::string version;
    std::ifstream ifs(version_file.c_str());
    getline(ifs, version);
    boost::algorithm::trim(version);
    return version;
}
std::string ProductMatcher::GetRVersion(const std::string& path)
{
    std::string version_file = path+"/RVERSION";
    std::string version;
    std::ifstream ifs(version_file.c_str());
    getline(ifs, version);
    boost::algorithm::trim(version);
    return version;
}

//bool ProductMatcher::GetProduct(const std::string& pid, Product& product)
//{
//    ProductIndex::const_iterator it = product_index_.find(pid);
//    if(it==product_index_.end()) return false;
//    uint32_t index = it->second;
//    if(index>=products_.size()) return false;
//    product = products_[index];
//    return true;
//}

void ProductMatcher::SetIndexDone_(const std::string& path, bool b)
{
    static const std::string file(path+"/index.done");
    if(b)
    {
        std::ofstream ofs(file.c_str());
        ofs<<"a"<<std::endl;
        ofs.close();
    }
    else
    {
        boost::filesystem::remove_all(file);
    }
}
bool ProductMatcher::IsIndexDone() const
{
    static const std::string file(path_+"/products");
    bool b = boost::filesystem::exists(file);
    LOG(INFO)<<"file "<<file<<" exists? : "<<(int)b<<std::endl;
    return b;
}

bool ProductMatcher::IsIndexDone_(const std::string& path)
{
    static const std::string file(path+"/index.done");
    return boost::filesystem::exists(file);
}
void ProductMatcher::Init_()
{
    boost::filesystem::create_directories(path_);
    //if(analyzer_==NULL)
    //{
        //idmlib::util::IDMAnalyzerConfig aconfig = idmlib::util::IDMAnalyzerConfig::GetCommonConfig("",cma_path_, "");
        //aconfig.symbol = true;
        //analyzer_ = new idmlib::util::IDMAnalyzer(aconfig);
    //}
    //if(char_analyzer_==NULL)
    //{
        //idmlib::util::IDMAnalyzerConfig cconfig = idmlib::util::IDMAnalyzerConfig::GetCommonConfig("","", "");
        //cconfig.symbol = true;
        //char_analyzer_ = new idmlib::util::IDMAnalyzer(cconfig);
    //}
    if(chars_analyzer_==NULL)
    {
        idmlib::util::IDMAnalyzerConfig csconfig = idmlib::util::IDMAnalyzerConfig::GetCommonConfig("","", "");
        csconfig.symbol = true;
        chars_analyzer_ = new idmlib::util::IDMAnalyzer(csconfig);
    }
    left_bracket_term_ = GetTerm_(left_bracket_);
    right_bracket_term_ = GetTerm_(right_bracket_);
    place_holder_term_ = GetTerm_(place_holder_);
    blank_term_ = GetTerm_(blank_);
    //symbol_terms_.insert(blank_term_);
    //symbol_terms_.insert(left_bracket_term_);
    //symbol_terms_.insert(right_bracket_term_);

    std::vector<std::string> connect_symbol_strs;
    connect_symbol_strs.push_back("-");
    connect_symbol_strs.push_back("/");
    //connect_symbol_strs.push_back("|");
    connect_symbol_strs.push_back(blank_);

    for(uint32_t i=0;i<connect_symbol_strs.size();i++)
    {
        connect_symbols_.insert(GetTerm_(connect_symbol_strs[i]));
    }
    std::vector<std::string> text_symbol_strs;
    text_symbol_strs.push_back(".");
    text_symbol_strs.push_back("+");
    for(uint32_t i=0;i<text_symbol_strs.size();i++)
    {
        text_symbols_.insert(GetTerm_(text_symbol_strs[i]));
    }
    products_.clear();
    keywords_thirdparty_.clear();
    not_keywords_.clear();
    category_list_.clear();
    cid_to_pids_.clear();
    category_index_.clear();
    product_index_.clear();
    keyword_set_.clear();
    trie_.clear();
    //ftrie_.clear();
    if(spu_matcher_!=NULL)
    {
        delete spu_matcher_;
        spu_matcher_ = NULL;
    }
    if(psm_!=NULL)
    {
        delete psm_;
        psm_ = NULL;
    }

}
bool ProductMatcher::Index(const std::string& kpath, const std::string& scd_path, int omode, int thread_num)
{
    path_ = kpath;
    if(!boost::filesystem::exists(kpath))
    {
        boost::filesystem::create_directories(kpath);
    }
    std::string rversion = GetVersion(scd_path);
    LOG(INFO)<<"rversion "<<rversion<<std::endl;
    LOG(INFO)<<"aversion "<<AVERSION<<std::endl;
    std::string erversion = GetRVersion(kpath);
    std::string eaversion = GetAVersion(kpath);
    LOG(INFO)<<"erversion "<<erversion<<std::endl;
    LOG(INFO)<<"eaversion "<<eaversion<<std::endl;
    int mode = omode;
    if(AVERSION>eaversion)
    {
        if(mode<2) mode = 2;
    }
    if(mode<2) //not re-training
    {
        if(IsIndexDone())
        {
            std::cout<<"product trained at "<<path_<<std::endl;
            Init_();
            return true;
        }
        if(!Open(path_))
        {
            mode = 2;
            Init_();
        }
    }
    LOG(INFO)<<"mode "<<mode<<std::endl;
    if(mode==3)
    {
        B5MHelper::PrepareEmptyDir(kpath);
    }
    else if(mode==2)
    {
        SetIndexDone_(kpath, false);
        //std::string bdb_path = kpath+"/bdb";
        //B5MHelper::PrepareEmptyDir(bdb_path);
    }
    SetIndexDone_(kpath, false);
    Init_();
    std::vector<std::string> copy_list;
    copy_list.push_back("product_name.dict");
    copy_list.push_back("garbage.pat");
    copy_list.push_back("att.syn");
    for(uint32_t i=0;i<copy_list.size();i++)
    {
        std::string f = scd_path+"/"+copy_list[i];
        std::string t = path_+"/"+copy_list[i];
        if(boost::filesystem::exists(t))
        {
            boost::filesystem::remove_all(t);
        }
        boost::filesystem::copy_file(f, t);
    }
    std::string keywords_file = scd_path+"/keywords.txt";
    if(boost::filesystem::exists(keywords_file))
    {
        std::ifstream ifs(keywords_file.c_str());
        std::string line;
        while( getline(ifs, line))
        {
            boost::algorithm::trim(line);
            keywords_thirdparty_.push_back(line);
        }
        ifs.close();
    }
    std::string not_keywords_file = scd_path+"/not_keywords.txt";
    if(boost::filesystem::exists(not_keywords_file))
    {
        std::ifstream ifs(not_keywords_file.c_str());
        std::string line;
        while( getline(ifs, line))
        {
            boost::algorithm::trim(line);
            TermList tl;
            GetTerms_(line, tl);
            not_keywords_.insert(tl);
        }
        ifs.close();
    }
    std::string bookcategory_file= scd_path+"/book_category";
    if(boost::filesystem::exists(bookcategory_file))
    {
        std::ifstream ifs(bookcategory_file.c_str());
        std::string line;
        if( getline(ifs, line))
        {
            boost::algorithm::trim(line);
            if(!line.empty())
            {
                book_category_ = line;
                LOG(INFO)<<"book category set to "<<book_category_<<std::endl;
            }
        }
        ifs.close();
    }
    std::string spu_scd = scd_path+"/SPU.SCD";
    if(!boost::filesystem::exists(spu_scd))
    {
        std::cerr<<"SPU.SCD not exists"<<std::endl;
        return false;
    }
    products_.resize(1);
    category_list_.resize(1);
    {
        std::string category_path = scd_path+"/category";
        std::string line;
        std::ifstream ifs(category_path.c_str());
        while(getline(ifs, line))
        {
            boost::algorithm::trim(line);
            std::vector<std::string> vec;
            boost::algorithm::split(vec, line, boost::algorithm::is_any_of(","));
            if(vec.size()<1) continue;
            uint32_t cid = category_list_.size();
            const std::string& scategory = vec[0];
            if(scategory.empty()) continue;
            //ignore book category
            if(boost::algorithm::starts_with(scategory, book_category_))
            {
                continue;
            }
            //const std::string& tag = vec[1];
            Category c;
            c.name = scategory;
            c.cid = cid;
            c.parent_cid = 0;
            c.is_parent = false;
            c.has_spu = false;
            std::set<std::string> akeywords;
            std::set<std::string> rkeywords;
            for(uint32_t i=1;i<vec.size();i++)
            {
                std::string keyword = vec[i];
                bool remove = false;
                if(keyword.empty()) continue;
                if(keyword[0]=='+')
                {
                    keyword = keyword.substr(1);
                }
                else if(keyword[0]=='-')
                {
                    keyword = keyword.substr(1);
                    remove = true;
                }
                if(keyword.empty()) continue;
                if(!remove)
                {
                    akeywords.insert(keyword);
                }
                else
                {
                    rkeywords.insert(keyword);
                }
            }
            std::vector<std::string> cs_list;
            boost::algorithm::split( cs_list, c.name, boost::algorithm::is_any_of(">") );
            c.depth=cs_list.size();
            if(cs_list.empty()) continue;
            std::vector<std::string> keywords_vec;
            boost::algorithm::split( keywords_vec, cs_list.back(), boost::algorithm::is_any_of("/") );
            for(uint32_t i=0;i<keywords_vec.size();i++)
            {
                akeywords.insert(keywords_vec[i]);
            }
            for(std::set<std::string>::const_iterator it = rkeywords.begin();it!=rkeywords.end();it++)
            {
                akeywords.erase(*it);
            }
            for(std::set<std::string>::const_iterator it = akeywords.begin();it!=akeywords.end();it++)
            {
                UString uc(*it, UString::UTF_8);
                if(uc.length()<=1) continue;
                c.keywords.push_back(*it);
            }
            if(c.depth>1)
            {
                std::string parent_name;
                for(uint32_t i=0;i<c.depth-1;i++)
                {
                    if(!parent_name.empty())
                    {
                        parent_name+=">";
                    }
                    parent_name+=cs_list[i];
                }
                c.parent_cid = category_index_[parent_name];
                category_list_[c.parent_cid].is_parent = true;
                //std::cerr<<"cid "<<c.cid<<std::endl;
                //std::cerr<<"parent cid "<<c.parent_cid<<std::endl;
            }
            category_list_.push_back(c);
#ifdef B5M_DEBUG
            std::cerr<<"add category "<<c.name<<std::endl;
#endif
            category_index_[c.name] = cid;
        }
        ifs.close();
    }
    //std::string bdb_path = path_+"/bdb";
    //BrandDb bdb(bdb_path);
    //bdb.open();
    ScdParser parser(izenelib::util::UString::UTF_8);
    parser.load(spu_scd);
    uint32_t n=0;
    size_t synonym_dict_size = 0;
    std::vector<std::pair<size_t, string> > synonym_pairs;
    std::vector<string> term_set;

    for( ScdParser::iterator doc_iter = parser.begin();
      doc_iter!= parser.end(); ++doc_iter, ++n)
    {
        //if(n>=15000) break;
        if(n%100000==0)
        {
            LOG(INFO)<<"Find Product Documents "<<n<<std::endl;
        }
        Document doc;
        Document::doc_prop_value_strtype pid;
        Document::doc_prop_value_strtype title;
        Document::doc_prop_value_strtype category;
        Document::doc_prop_value_strtype attrib_ustr;
        SCDDoc& scddoc = *(*doc_iter);
        SCDDoc::iterator p = scddoc.begin();
        for(; p!=scddoc.end(); ++p)
        {
            const std::string& property_name = p->first;
            doc.property(property_name) = p->second;
            if(property_name=="DOCID")
            {
                pid = p->second;
            }
            else if(property_name=="Title")
            {
                title = p->second;
            }
            else if(property_name=="Category")
            {
                category = p->second;
            }
            else if(property_name=="Attribute")
            {
                attrib_ustr = p->second;
            }
        }
        if(category.length()==0 || attrib_ustr.length()==0 || title.length()==0)
        {
            continue;
        }
        //convert category
        uint32_t cid = 0;
        std::string scategory = propstr_to_str(category);
        CategoryIndex::iterator cit = category_index_.find(scategory);
        if(cit==category_index_.end())
        {
            continue;
        }
        else
        {
            cid = cit->second;
            Category& category = category_list_[cid];
            //if(category.is_parent) continue;
            category.has_spu = true;
        }
        ProductPrice price;
        Document::doc_prop_value_strtype uprice;
        if(doc.getProperty("Price", uprice))
        {
            price.Parse(propstr_to_ustr(uprice));
        }
        std::string stitle = propstr_to_str(title);
        std::string spid = propstr_to_str(pid);
        //uint128_t ipid = B5MHelper::StringToUint128(spid);
        std::string sattribute = propstr_to_str(attrib_ustr);
        Product product;
        product.spid = spid;
        product.stitle = stitle;
        product.scategory = scategory;
        product.cid = cid;
        product.price = price;
        doc.getString("Picture", product.spic);
        if(product.spic.empty())
        {
            doc.getString("OriginalPicture", product.spic);
        }
        doc.getString("Url", product.surl);
        doc.getString("MarketTime", product.smarket_time);
        ParseAttributes(propstr_to_ustr(attrib_ustr), product.attributes);

        for (size_t i = 0; i < product.attributes.size(); ++i)
            if (product.attributes[i].name == "品牌")
            {
                if (product.attributes[i].values.empty() || product.attributes[i].values.size() < 2) continue;

                size_t count = 0;
                size_t tmp_id = 0;
                string lower_string;
                for (size_t j = 0; j < product.attributes[i].values.size(); ++j)
                {
                    lower_string = product.attributes[i].values[j];
                    boost::to_lower(lower_string);
                    if (synonym_map_.find(lower_string)!=synonym_map_.end())//term is not in synonym map
                    {
                        ++count;
                        tmp_id = synonym_map_[lower_string];
                    }
                }
                if (!count)//new synonym set
                {
                    string st;
                    for (size_t j = 0; j < product.attributes[i].values.size(); ++j)
                    {
                        lower_string = product.attributes[i].values[j];
                        boost::to_lower(lower_string);
                        synonym_map_.insert(std::make_pair(lower_string, synonym_dict_size));
                        synonym_pairs.push_back(std::make_pair(synonym_dict_size, lower_string));
                        st += (lower_string + '/');
                    }
                    term_set.push_back(st);
                    ++synonym_dict_size;
                }
                else if (count < product.attributes[i].values.size())//add term in synonym set
                {
                    for (size_t j = 0; j < product.attributes[i].values.size(); ++j)
                    {
                        lower_string = product.attributes[i].values[j];
                        boost::to_lower(lower_string);
                        if (synonym_map_.find(lower_string) == synonym_map_.end())
                        {
                            synonym_map_.insert(std::make_pair(lower_string, tmp_id));
                            synonym_pairs.push_back(std::make_pair(tmp_id, lower_string));
                            term_set[tmp_id] += lower_string + '/';
                        }
                    }
                }
            }
        if(product.attributes.size()<2) continue;
        Document::doc_prop_value_strtype dattribute_ustr;
        doc.getProperty("DAttribute", dattribute_ustr);
        Document::doc_prop_value_strtype display_attribute_str;
        doc.getProperty("DisplayAttribute", display_attribute_str);
        product.display_attributes = propstr_to_ustr(display_attribute_str);
        Document::doc_prop_value_strtype filter_attribute_str;
        doc.getProperty("FilterAttribute", filter_attribute_str);
        product.filter_attributes = propstr_to_ustr(filter_attribute_str);
        //if(!filter_attribute_ustr.empty())
        //{
            //ParseAttributes(filter_attribute_ustr, product.dattributes);
        //}
        //else if(!display_attribute_ustr.empty())
        //{
            //ParseAttributes(display_attribute_ustr, product.dattributes);
        //}
        if(!dattribute_ustr.empty())
        {
            ParseAttributes(propstr_to_ustr(dattribute_ustr), product.dattributes);
            MergeAttributes(product.dattributes, product.attributes);
        }
        product.tweight = 0.0;
        product.aweight = 0.0;
        //std::cerr<<"[SPU][Title]"<<stitle<<std::endl;
        boost::unordered_set<std::string> attribute_value_app;
        bool invalid_attribute = false;
        for(uint32_t i=0;i<product.attributes.size();i++)
        {
            const Attribute& attrib = product.attributes[i];
            for(uint32_t a=0;a<attrib.values.size();a++)
            {
                std::string v = attrib.values[a];
                boost::algorithm::to_lower(v);
                if(attribute_value_app.find(v)!=attribute_value_app.end())
                {
                    invalid_attribute = true;
                    break;
                }
                attribute_value_app.insert(v);
            }
            if(invalid_attribute) break;
        }
        if(invalid_attribute)
        {
//#ifdef B5M_DEBUG
//#endif
            //LOG(INFO)<<"invalid SPU attribute "<<spid<<","<<sattribute<<std::endl;
            continue;
        }
        for(uint32_t i=0;i<product.attributes.size();i++)
        {
            if(product.attributes[i].is_optional)
            {
                //product.aweight+=0.5;
            }
            else if(product.attributes[i].name=="型号")
            {
                product.aweight+=1.5;
            }
            else
            {
                product.aweight+=1.0;
            }

        }
        //Document::doc_prop_value_strtype brand;
        std::string sbrand;
        for(uint32_t i=0;i<product.attributes.size();i++)
        {
            if(product.attributes[i].name=="品牌")
            {
                sbrand = product.attributes[i].GetValue();
                //brand = str_to_propstr(sbrand, UString::UTF_8);
                break;
            }
        }
        doc.getString("SubPropsRule", product.sub_prop);
        //if(!brand.empty())
        //{
            //BrandDb::BidType bid = bdb.set(ipid, brand);
            //bdb.set_source(brand, bid);
        //}
        product.sbrand = sbrand;
        uint32_t spu_id = products_.size();
        products_.push_back(product);
        product_index_[spid] = spu_id;
    }
    //bdb.flush();
    //add virtual spus
    for(uint32_t i=1;i<category_list_.size();i++)
    {
        const Category& c = category_list_[i];
        if(!c.has_spu)
        {
            Product p;
            p.scategory = c.name;
            p.cid = i;
            products_.push_back(p);
        }
    }
    cid_to_pids_.resize(category_list_.size());
    for(uint32_t spu_id=1;spu_id<products_.size();spu_id++)
    {
        Product& p = products_[spu_id];
        string_similarity_.Convert(p.stitle, p.title_obj);
        cid_to_pids_[p.cid].push_back(spu_id);
    }
    if(!products_.empty())
    {
        TrieType suffix_trie;
        ConstructSuffixTrie_(suffix_trie);
        ConstructKeywords_();
        LOG(INFO)<<"find "<<keyword_set_.size()<<" keywords"<<std::endl;
        ConstructKeywordTrie_(suffix_trie);
    }
    SetProductsId_();
    //SetProductsPrice_(false);
    is_open_ = true;
    std::string brand_error_file = scd_path+"/brand_error";
    if(boost::filesystem::exists(brand_error_file))
    {
        if(brand_manager_!=NULL)
        {
            brand_manager_->LoadBrandErrorFile(brand_error_file);
        }
    }
    //std::string offer_scd = scd_path+"/OFFER.SCD";
    //if(boost::filesystem::exists(offer_scd))
    //{
    //    IndexOffer_(offer_scd, thread_num);
    //}
    std::string path = path_+"/products";
    izenelib::am::ssf::Util<>::Save(path, products_);
    path = path_+"/category_list";
    LOG(INFO)<<"category list size "<<category_list_.size()<<std::endl;
    izenelib::am::ssf::Util<>::Save(path, category_list_);
    //path = path_+"/keywords";
    //izenelib::am::ssf::Util<>::Save(path, keywords_thirdparty_);
    path = path_+"/category_index";
    izenelib::am::ssf::Util<>::Save(path, category_index_);
    path = path_+"/cid_to_pids";
    izenelib::am::ssf::Util<>::Save(path, cid_to_pids_);
    path = path_+"/product_index";
    izenelib::am::ssf::Util<>::Save(path, product_index_);
    path = path_+"/keyword_trie";
    izenelib::am::ssf::Util<>::Save(path, trie_);
    //path = path_+"/fuzzy_trie";
    //izenelib::am::ssf::Util<>::Save(path, ftrie_);
    //path = path_+"/term_index";
    //std::map<cid_t, TermIndex> tmap(term_index_map_.begin(), term_index_map_.end());
    //izenelib::am::ssf::Util<>::Save(path, tmap);
    path = path_+"/book_category";
    izenelib::am::ssf::Util<>::Save(path, book_category_);
    path = path_+"/brand_manager";
    if(brand_manager_!=NULL) brand_manager_->Save(path);
    //path = path_+"/nf";
    //std::map<uint64_t, uint32_t> nf_map(nf_.begin(), nf_.end());
    //izenelib::am::ssf::Util<>::Save(path, nf_map);
    path = path_+"/synonym_map";
    izenelib::am::ssf::Util<>::Save(path, synonym_pairs);
    for (size_t i = 0; i < term_set.size(); ++i)
        if (term_set[i][term_set[i].length() - 1] == '/') term_set[i].erase(term_set[i].length() - 1, 1);
    path = path_+"/synonym_dict";
    izenelib::am::ssf::Util<>::Save(path, term_set);
    {
        std::string aversion_file = path_+"/AVERSION";
        std::ofstream ofs(aversion_file.c_str());
        ofs<<AVERSION<<std::endl;
        ofs.close();
    }
    {
        std::string rversion_file = path_+"/RVERSION";
        std::ofstream ofs(rversion_file.c_str());
        ofs<<rversion<<std::endl;
        ofs.close();
    }
    SetIndexDone_(path_, true);

    return true;
}


void ProductMatcher::GetPsmKeywords_(const KeywordVector& keywords, const std::string& scategory, std::vector<std::string>& brands, std::vector<std::string>& ckeywords) const
{
    for(uint32_t i=0;i<keywords.size();i++)
    {
        const KeywordTag& k = keywords[i];
        std::string str;
        k.text.convertString(str, UString::UTF_8);
        if(k.kweight>=0.8&&k.IsCategoryKeyword())
        {
            ckeywords.push_back(str);
        }
        else
        {
            if(IsBrand_(scategory, k))
            {
                brands.push_back(str);
            }
        }
    }
    std::sort(ckeywords.begin(), ckeywords.end());
}


bool ProductMatcher::DoMatch(const std::string& scd_path, const std::string& output_file)
{
    izenelib::util::ClockTimer clocker;
    std::vector<std::string> scd_list;
    B5MHelper::GetIUScdList(scd_path, scd_list);
    if(scd_list.empty()) return false;
    std::string match_file = output_file;
    if(match_file.empty())
    {
        match_file = path_+"/match";
    }
    std::ofstream ofs(match_file.c_str());
    std::size_t doc_count=0;
    std::size_t spu_matched_count=0;
    for(uint32_t i=0;i<scd_list.size();i++)
    {
        std::string scd_file = scd_list[i];
        LOG(INFO)<<"Processing "<<scd_file<<std::endl;
        ScdParser parser(izenelib::util::UString::UTF_8);
        parser.load(scd_file);
        uint32_t n=0;
        for( ScdParser::iterator doc_iter = parser.begin();
          doc_iter!= parser.end(); ++doc_iter, ++n)
        {
            if(n%10000==0)
            {
                LOG(INFO)<<"Find Offer Documents "<<n<<std::endl;
            }
            SCDDoc& scddoc = *(*doc_iter);
            SCDDoc::iterator p = scddoc.begin();
            Document doc;
            for(; p!=scddoc.end(); ++p)
            {
                const std::string& property_name = p->first;
                doc.property(property_name) = p->second;
            }
            Product result_product;
            Process(doc, result_product);
            doc_count++;
            std::string spid = result_product.spid;
            std::string sptitle = result_product.stitle;
            std::string scategory = result_product.scategory;
            std::string soid;
            std::string stitle;
            doc.getString("DOCID", soid);
            doc.getString("Title", stitle);
            if(spid.length()>0)
            {
                spu_matched_count++;
                //ofs<<soid<<","<<spid<<","<<stitle<<","<<sptitle<<","<<scategory<<std::endl;
            }
            //else
            //{
                //ofs<<soid<<","<<stitle<<","<<scategory<<std::endl;
            //}
            ofs<<soid<<","<<spid<<","<<stitle<<","<<sptitle<<","<<scategory<<std::endl;
        }
    }
    ofs.close();
    LOG(INFO)<<"clocker used "<<clocker.elapsed()<<std::endl;
    LOG(INFO)<<"stat: doc_count:"<<doc_count<<", spu_matched_count:"<<spu_matched_count<<std::endl;
    return true;
}
bool ProductMatcher::GetIsbnAttribute(const Document& doc, std::string& isbn_value)
{
    const static std::string isbn_name = "isbn";
    Document::doc_prop_value_strtype attrib_ustr;
    doc.getProperty("Attribute", attrib_ustr);
    std::vector<Attribute> attributes;
    ParseAttributes(propstr_to_ustr(attrib_ustr), attributes);
    for(uint32_t i=0;i<attributes.size();i++)
    {
        const Attribute& a = attributes[i];
        std::string aname = a.name;
        boost::algorithm::trim(aname);
        boost::to_lower(aname);
        if(aname==isbn_name)
        {
            if(!a.values.empty())
            {
                isbn_value = a.values[0];
                boost::algorithm::replace_all(isbn_value, "-", "");
            }
            break;
        }
    }
    if(!isbn_value.empty()) return true;
    return false;

}

bool ProductMatcher::ProcessBook(const Document& doc, Product& result_product)
{
    bool need_return = false;
    std::string scategory;
    doc.getString("Category", scategory);
    std::string isbn_value;
    if(boost::algorithm::starts_with(scategory, book_category_))
    {
        GetIsbnAttribute(doc, isbn_value);
        need_return = true;
    }
    else if(scategory.empty())
    {
        GetIsbnAttribute(doc, isbn_value);
        if(!isbn_value.empty())
        {
            need_return = true;
        }
    }
    if(!isbn_value.empty())
    {
        if(scategory.empty())
        {
            result_product.scategory = book_category_;
        }
        if(invalid_isbn_.find(isbn_value)==invalid_isbn_.end())
        {
            result_product.spid = B5MHelper::GetPidByIsbn(isbn_value);
        }
    }
    return need_return;
}
void ProductMatcher::PendingFinish(const boost::function<void (ScdDocument&)>& func, int thread_num)
{
    if(psm_!=NULL)
    {
        psm_->BmnFinish(func, thread_num);
    }
}

bool ProductMatcher::ProcessBookStatic(const Document& doc, Product& result_product)
{
    std::string isbn_value;
    GetIsbnAttribute(doc, isbn_value);
    if(!isbn_value.empty())
    {
        result_product.spid = B5MHelper::GetPidByIsbn(isbn_value);
        return true;
    }
    return false;
}

void ProductMatcher::ProcessSubProp_(const Document& doc, Product& product)
{
    const std::string& category = product.scategory;
    if(!boost::algorithm::starts_with(category, "美容美妆")) return;
    std::string stitle;
    doc.getString("Title", stitle);
    static const boost::regex sub_prop_regex("(\\d+\\.)?\\d+([gG]|[mM][lL])(?=([^a-zA-Z]+|$))");
    std::string rsub_prop;
    {
        std::string::const_iterator start = stitle.begin();
        std::string::const_iterator end = stitle.end();
        boost::smatch what;
        while( boost::regex_search( start, end, what, sub_prop_regex))
        {
            std::string tmatch(what[0].first, what[0].second);
            start = what[0].second;
            rsub_prop = tmatch;
            break;
        }
    }
    if(rsub_prop.empty())
    {
        std::vector<std::string> rsub_list;
        UString attrib_ustr;
        doc.getString("Attribute", attrib_ustr);
        std::vector<Attribute> attributes;
        ParseAttributes(attrib_ustr, attributes);
        for(uint32_t i=0;i<attributes.size();i++)
        {
            const Attribute& a = attributes[i];
            std::string aname = a.name;
            boost::algorithm::trim(aname);
            if(boost::algorithm::ends_with(aname,"容量"))
            {
                for(std::size_t j=0;j<a.values.size();j++)
                {
                    const std::string& v = a.values[j];
                    std::string::const_iterator start = v.begin();
                    std::string::const_iterator end = v.end();
                    boost::smatch what;
                    while( boost::regex_search( start, end, what, sub_prop_regex))
                    {
                        std::string tmatch(what[0].first, what[0].second);
                        start = what[0].second;
                        rsub_list.push_back(tmatch);
                    }
                }
                break;
            }
        }
        if(rsub_list.size()==1)
        {
            rsub_prop = rsub_list[0];
        }
    }
    boost::algorithm::to_lower(rsub_prop);
    if(boost::algorithm::ends_with(rsub_prop,"g"))
    {
        rsub_prop = rsub_prop.substr(0, rsub_prop.length()-1)+"ml";
    }
    product.sub_prop = rsub_prop;
}

bool ProductMatcher::Process(const Document& doc, Product& result_product)
{
    if(!IsOpen()) return false;
    if(ProcessBook(doc, result_product))
    {
        if(!result_product.spid.empty())
        {
            result_product.type = Product::BOOK;
        }
        return true;
    }
    if(spu_matcher_==NULL) return true;
    SpuMatcher::SpuProduct smproduct;
    spu_matcher_->Evaluate(doc, smproduct);
    if(!smproduct.docid.empty())
    {
        result_product.spid = smproduct.docid;
        result_product.stitle = smproduct.title;
        result_product.scategory = smproduct.category;
        result_product.dattributes = smproduct.dattributes;
        result_product.type = Product::SPU;
        ProcessSubProp_(doc, result_product);
        return true;
    }
    Document::doc_prop_value_strtype ptitle;
    Document::doc_prop_value_strtype pcategory;
    doc.getProperty("Category", pcategory);
    doc.getProperty("Title", ptitle);
    UString title = propstr_to_ustr(ptitle);
    UString category = propstr_to_ustr(pcategory);

    if(title.length()==0)
    {
        return false;
    }
    std::string scategory;
    category.convertString(scategory, UString::UTF_8);
    cid_t cid = GetCid_(category);
    std::vector<Term> term_list;
    Analyze_(title, term_list);
    KeywordVector keyword_vector;
    GetKeywords(term_list, keyword_vector, cid);
    std::string spid;
    std::string stitle;
    std::vector<std::string> brands;
    std::vector<std::string> keywords;
    GetPsmKeywords_(keyword_vector, scategory, brands, keywords);
    CategoryPsm::FLAG cf = psm_->Search(doc, brands, keywords, result_product);
    if(cf!=CategoryPsm::NO)
    {
        //p.why = why;
        return true;
    }
    return true;
}

bool ProductMatcher::GetKeyword(const UString& text, KeywordTag& tag)
{
    std::vector<Term> term_list;
    Analyze_(text, term_list);
    std::vector<term_t> ids(term_list.size());
    for(uint32_t i=0;i<ids.size();i++)
    {
        ids[i] = term_list[i].id;
    }
    TrieType::const_iterator it = trie_.find(ids);
    if(it!=trie_.end())
    {
        tag = it->second;
        return true;
    }
    return false;
}

void ProductMatcher::GetKeywords(const ATermList& term_list, KeywordVector& keyword_vector, cid_t cid)
{
    //std::string stitle;
    //text.convertString(stitle, UString::UTF_8);
    uint32_t begin = 0;
    uint8_t bracket_depth = 0;
    uint32_t last_complete_keyword_fol = 0;
    keyword_vector.reserve(10);
    //typedef boost::unordered_map<TermList, uint32_t> KeywordIndex;
    //typedef KeywordVector::value_type ItemType;
    //typedef std::pair<double, std::string> ItemScore;
    //KeywordIndex keyword_index;
    //KeywordAppMap app_map;
    while(begin<term_list.size())
    {
        //if(bracket_depth>0)
        //{
            //begin++;
            //continue;
        //}
        //bool found_keyword = false;
        //typedef std::vector<ItemType> Buffer;
        //Buffer buffer;
        //buffer.reserve(5);
        //std::pair<TrieType::key_type, TrieType::mapped_type> value;
        KeywordTag found_keyword;
        uint32_t keyword_fol_position = 0;
        std::vector<term_t> candidate_keyword;
        std::vector<Term> candidate_terms;
        candidate_keyword.reserve(10);
        candidate_terms.reserve(10);
        uint32_t next_pos = begin;
        Term last_term;
        //std::cerr<<"[BEGIN]"<<begin<<std::endl;
        while(true)
        {
            if(next_pos>=term_list.size()) break;
            const Term& term = term_list[next_pos++];
            //std::string sterm = GetText_(term);
            //std::cerr<<"[A]"<<sterm<<std::endl;
            if(IsSymbol_(term))
            {
                if(IsConnectSymbol_(term.id))
                {
                    if(candidate_keyword.empty())
                    {
                        //std::cerr<<"[B0]"<<sterm<<","<<term<<std::endl;
                        break;
                    }
                    else
                    {
                        last_term = term;
                        //std::cerr<<"[B]"<<sterm<<","<<term<<std::endl;
                        continue;
                    }
                }
                else
                {
                    if(term.id==left_bracket_term_||term.id==right_bracket_term_)
                    {
                        if(next_pos-begin==1)
                        {
                            if(term.id==left_bracket_term_)
                            {
                                bracket_depth++;
                            }
                            else if(term.id==right_bracket_term_)
                            {
                                if(bracket_depth>0)
                                {
                                    bracket_depth--;
                                }
                            }
                        }
                    }
                    //std::cerr<<"[C]"<<sterm<<","<<term<<std::endl;
                    break;
                }
            }
            else
            {
                if(last_term.id==blank_term_&&!candidate_keyword.empty())
                {
                    const Term& last_candidate_term = candidate_terms.back();
                    //UString last_candidate = UString(GetText_(last_candidate_term), UString::UTF_8);
                    //UString this_text = UString(GetText_(term), UString::UTF_8);
                    if(IsBlankSplit_(last_candidate_term.text, term.text))
                    {
                        break;
                    }
                    //std::cerr<<"[D]"<<sterm<<","<<term<<std::endl;
                }
            }
            //std::cerr<<"[E]"<<sterm<<","<<term<<std::endl;
            candidate_keyword.push_back(term.id);
            candidate_terms.push_back(term);
            //std::string skeyword = GetText_(candidate_keyword);
            //std::cerr<<"[SK]"<<skeyword<<std::endl;
            TrieType::const_iterator it = trie_.lower_bound(candidate_keyword);
            if(it!=trie_.end())
            {
                if(candidate_keyword==it->first)
                {
                    //ItemType item;
                    //item.first = it->first;
                    //item.second = it->second;
                    KeywordTag tag = it->second;
                    tag.ngram = 1;
                    tag.kweight = 1.0;
                    Position pos(begin, next_pos);
                    tag.positions.push_back(pos);
                    if(bracket_depth>0) tag.kweight=0.1;
                    bool need_append = true;
                    for(uint32_t i=0;i<keyword_vector.size();i++)
                    {
                        KeywordTag& ek = keyword_vector[i];
                        if(tag.id==ek.id) //duplidate
                        {
                            if(tag.kweight>ek.kweight)
                            {
                                ek.kweight = tag.kweight;
                            }
                            ek.PositionMerge(pos);
                            need_append = false;
                            break;
                        }
                    }
                    if(need_append)
                    {
                        Term pre_term;
                        Term fol_term;
                        if(begin>0)
                        {
                            pre_term = term_list[begin-1];
                        }
                        if(next_pos<term_list.size())
                        {
                            fol_term = term_list[next_pos];
                        }
                        if(pre_term.TextString()=="." || fol_term.TextString()==".")
                        {
                            //std::cerr<<"[F]"<<pre_str<<","<<fol_str<<std::endl;
                            need_append = false;
                        }
                    }
                    if(need_append&&found_keyword.term_list.size()>0)
                    {
                        if((double)found_keyword.attribute_apps.size()/tag.attribute_apps.size()>=1.0)
                        {
                            found_keyword.kweight = 0.0;
                            keyword_vector.push_back(found_keyword);
                        }
                    }
                    //for(uint32_t i=0;i<keyword_vector.size();i++)
                    //{
                        //int select = SelectKeyword_(tag, keyword_vector[i]);
                        //if(select==1)
                        //{
                            //keyword_vector[i] = tag;
                            //need_append = false;
                            //break;
                        //}
                        //else if(select==2)
                        //{
                            //need_append = false;
                            //break;
                        //}
                        //else if(select==3)
                        //{
                            ////keyword_vector.push_back(tag);
                        //}
                    //}
                    if(need_append)
                    {
                        //keyword_vector.push_back(tag);
                        //std::cerr<<"[K]"<<sterm<<","<<term<<std::endl;
                        found_keyword = tag;
                        keyword_fol_position = next_pos;
                    }
                }
                else if(StartsWith_(it->first, candidate_keyword))
                {
                    //continue trying
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
            last_term = term;
        }
        if(found_keyword.term_list.size()>0)
        {
            if(begin>=last_complete_keyword_fol)
            {
                last_complete_keyword_fol = keyword_fol_position;
            }
            else
            {
                found_keyword.kweight = 0.0;//not complete keyword
            }
            keyword_vector.push_back(found_keyword);
            //begin = keyword_fol_position;
            ++begin;
        }
        else
        {
            //begin = next_pos;
            ++begin;
        }
        //begin = next_pos;
    }




}
void ProductMatcher::GetKeywords(const UString& text, KeywordVector& keywords)
{
    ATermList term_list;
    Analyze_(text, term_list);
    GetKeywords(term_list, keywords);
}

void ProductMatcher::ExtractKeywords(const UString& text, KeywordVector& keywords)
{
    if(!IsOpen()) return;
    ATermList term_list;
    Analyze_(text, term_list);
    GetKeywords(term_list, keywords);
    return;
}

void ProductMatcher::ExtractKeywordsFromPage(const UString& text, std::list<std::pair<UString, std::pair<uint32_t, uint32_t> > >& res_ca, std::list<std::pair<UString, std::pair<uint32_t, uint32_t> > >& res_brand, std::list<std::pair<UString, std::pair<uint32_t, uint32_t> > >& res_model)
{
}
void ProductMatcher::ExtractKeywordsFromPage(const UString& text, std::list<std::pair<UString, std::pair<uint32_t, uint32_t> > >&res)
{
}
void ProductMatcher::GetSearchKeywords(const UString& text, std::list<std::pair<UString, double> >& hits, std::list<std::pair<UString, double> >& left_hits, std::list<UString>& left)
{
}

cid_t ProductMatcher::GetCid_(const UString& category) const
{
    cid_t cid = 0;
    std::string scategory;
    category.convertString(scategory, UString::UTF_8);
    if(!scategory.empty())
    {
        CategoryIndex::const_iterator it = category_index_.find(scategory);
        if(it!=category_index_.end())
        {
            cid = it->second;
        }
    }
    return cid;
}

uint32_t ProductMatcher::GetCidBySpuId_(uint32_t spu_id)
{
    //LOG(INFO)<<"spu_id:"<<spu_id<<std::endl;
    const Product& product = products_[spu_id];
    const std::string& scategory = product.scategory;
    return category_index_[scategory];
}


double ProductMatcher::PriceSim_(double offerp, double spup) const
{
    if(!use_price_sim_) return 0.25;
    if(spup==0.0) return 0.25;
    if(offerp==0.0) return 0.0;
    if(offerp>spup) return spup/offerp;
    else return offerp/spup;
}

bool ProductMatcher::IsValuePriceSim_(double op, double p) const
{
    if(p<=0.0) return true;
    if(op<=0.0) return false;
    static const double thlow = 0.25;
    double thhigh = 4.0;
    if(p<=100.0) thhigh = 4.0;
    else if(p<=5000.0) thhigh = 3.0;
    else thhigh = 2.0;
    double v = op/p;
    if(v>=thlow&&v<=thhigh) return true;
    else return false;
}
ProductPrice ProductMatcher::GetProductPriceRange_(const ProductPrice& p) const
{
    ProductPrice result = p;
    static const double thlow = 0.25;
    double dp = 0.0;
    p.GetMid(dp);
    double thhigh = 4.0;
    if(dp<=100.0) thhigh = 4.0;
    else if(dp<=5000.0) thhigh = 3.0;
    else thhigh = 2.0;
    if(!p.Positive())
    {
        result.value.first = 0.0;
        result.value.second = std::numeric_limits<double>::max();
    }
    else
    {
        result.value.first *= thlow;
        result.value.second *= thhigh;
    }
    return result;
}

ProductPrice ProductMatcher::GetProductPriceRange_(const ProductPrice& p, const std::vector<ProductPrice>& offer_prices) const
{
    ProductPrice result = p;
    if(offer_prices.size()>=20)
    {
        double avg = 0.0;
        for(uint32_t j=0;j<offer_prices.size();j++)
        {
            avg += offer_prices[j].Min();
        }
        if(avg>0.0)
        {
            avg /= offer_prices.size();
        }
        if(avg>=1000.0)
        {
            avg *= 0.7;
            uint32_t invalid_count = 0;
            for(uint32_t j=0;j<offer_prices.size();j++)
            {
                if(offer_prices[j].Min()<avg) ++invalid_count;
            }
            double invalid_ratio = (double)invalid_count/offer_prices.size();
            if(invalid_ratio<=0.2)
            {
                double t = avg*1.1;
                std::cerr<<"change lower bound from "<<result.value.first<<" to "<<t<<std::endl;
                result.value.first = t;
            }
        }
    }
    return result;
}

bool ProductMatcher::IsPriceSim_(const ProductPrice& op, const ProductPrice& p) const
{
    if(!use_price_sim_) return true;
    if(!p.Positive()) return true;
    if(!op.Positive()) return false;
    //if(IsValuePriceSim_(op.Min(), p.Min())) return true;
    //if(IsValuePriceSim_(op.Max(), p.Max())) return true;
    if(op.Min()>=p.Min()&&op.Max()<=p.Max()) return true;
    return false;
}

double ProductMatcher::PriceDiff_(double op, double p) const
{
    if(op<=0.0||p<=0.0) return 999999.00;
    return std::abs(op-p);
}
double ProductMatcher::PriceDiff_(const ProductPrice& op, const ProductPrice& p) const
{
    if(!p.Positive()||!op.Positive()) return 999999.00;
    return std::min( std::abs(p.Min()-op.Min()), std::abs(p.Max()-op.Max()));
}

void ProductMatcher::Analyze_(const izenelib::util::UString& btext, std::vector<Term>& result)
{
    izenelib::util::UString text(btext);
    text.toLowerString();
    std::string stext;
    text.convertString(stext, UString::UTF_8);
    std::vector<idmlib::util::IDMTerm> term_list;
    chars_analyzer_->GetTermList(text, term_list);
    result.reserve(term_list.size());
    std::size_t spos = 0;
    bool last_match = true;
    for(std::size_t i=0;i<term_list.size();i++)
    {
        std::string str;
        term_list[i].text.convertString(str, izenelib::util::UString::UTF_8);
        term_t id = GetTerm_(str);
        //std::cerr<<"[A]"<<str<<","<<term_list[i].tag<<","<<term_list[i].position<<","<<spos<<","<<last_match<<std::endl;
        std::size_t pos = stext.find(str, spos);
        bool has_blank = false;
        if(pos!=std::string::npos)
        {
            if(pos>spos && last_match)
            {
                has_blank = true;
            }
            spos = pos+str.length();
            last_match = true;
        }
        else
        {
            last_match = false;
#ifdef B5M_DEBUG
            //std::cerr<<"[TE]"<<str<<std::endl;
            //std::cerr<<"[TITLE]"<<stext<<std::endl;
#endif
        }
        if(has_blank)
        {
            std::string append = blank_;
            term_t app_id = GetTerm_(append);
            Term term;
            term.id = app_id;
            term.text = UString(append, UString::UTF_8);
            term.tag = idmlib::util::IDMTermTag::SYMBOL;
            if(stext.at(pos-1) != ' ')term.position = -1;
            result.push_back(term);
        }

        char tag = term_list[i].tag;
        if(tag == idmlib::util::IDMTermTag::SYMBOL)
        {
            if(str=="("||str=="（")
            {
                str = left_bracket_;
            }
            else if(str==")"||str=="）")
            {
                str = right_bracket_;
            }
            //else
            //{
                //if(!IsTextSymbol_(id))
                //{
                    //symbol_terms_.insert(id);
                //}
            //}
        }
        id = GetTerm_(str);
        term_list[i].id = id;
        result.push_back(term_list[i]);
    }
}

void ProductMatcher::AnalyzeNoSymbol_(const izenelib::util::UString& btext, std::vector<Term>& result)
{
    izenelib::util::UString text(btext);
    text.toLowerString();
    //std::string stext;
    //text.convertString(stext, UString::UTF_8);
    //std::cerr<<"[ANSS]"<<stext<<std::endl;
    std::vector<idmlib::util::IDMTerm> term_list;
    chars_analyzer_->GetTermList(text, term_list);
    result.reserve(term_list.size());
    for(std::size_t i=0;i<term_list.size();i++)
    {
        std::string str;
        term_list[i].text.convertString(str, izenelib::util::UString::UTF_8);
        term_list[i].id = GetTerm_(str);
        //std::cerr<<"[ANS]"<<str<<","<<term_list[i].tag<<std::endl;
        char tag = term_list[i].tag;
        //std::cerr<<"ARB,"<<str<<std::endl;
        if(tag == idmlib::util::IDMTermTag::SYMBOL)
        {
            if(!IsTextSymbol_(term_list[i].id))
            {
                continue;
            }
        }
        result.push_back(term_list[i]);

    }
}


void ProductMatcher::ParseAttributes(const UString& ustr, std::vector<Attribute>& attributes)
{
    B5MHelper::ParseAttributes(ustr, attributes);
}

void ProductMatcher::MergeAttributes(std::vector<Attribute>& eattributes, const std::vector<Attribute>& attributes)
{
    if(attributes.empty()) return;
    boost::unordered_set<std::string> to_append_name;
    for(uint32_t i=0;i<attributes.size();i++)
    {
        to_append_name.insert(attributes[i].name);
    }
    for(uint32_t i=0;i<eattributes.size();i++)
    {
        to_append_name.erase(eattributes[i].name);
    }
    for(uint32_t i=0;i<attributes.size();i++)
    {
        const Attribute& a = attributes[i];
        if(to_append_name.find(a.name)!=to_append_name.end())
        {
            eattributes.push_back(a);
        }
    }
}

UString ProductMatcher::AttributesText(const std::vector<Attribute>& attributes)
{
    std::string str;
    for(uint32_t i=0;i<attributes.size();i++)
    {
        const Attribute& a = attributes[i];
        if(!str.empty()) str+=",";
        str+=a.GetText();
    }
    return UString(str, UString::UTF_8);
}

term_t ProductMatcher::GetTerm_(const UString& text)
{
    term_t term = izenelib::util::HashFunction<izenelib::util::UString>::generateHash32(text);
    //term_t term = izenelib::util::HashFunction<izenelib::util::UString>::generateHash64(text);
#ifdef B5M_DEBUG
    id_manager_[term] = text;
#endif
    return term;
}
term_t ProductMatcher::GetTerm_(const std::string& text)
{
    UString utext(text, UString::UTF_8);
    return GetTerm_(utext);
}

std::string ProductMatcher::GetText_(const TermList& tl, const std::string& s) const
{
    std::string result;
#ifdef B5M_DEBUG
    for(uint32_t i=0;i<tl.size();i++)
    {
        std::string str;
        term_t term = tl[i];
        IdManager::const_iterator it = id_manager_.find(term);
        if(it!=id_manager_.end())
        {
            const UString& ustr = it->second;
            ustr.convertString(str, UString::UTF_8);
        }
        else
        {
            str = "__UNKNOW__";
        }
        if(i>0) result+=s;
        result+=str;
    }
#endif
    return result;
}
std::string ProductMatcher::GetText_(const term_t& term) const
{
#ifdef B5M_DEBUG
    IdManager::const_iterator it = id_manager_.find(term);
    if(it!=id_manager_.end())
    {
        const UString& ustr = it->second;
        std::string str;
        ustr.convertString(str, UString::UTF_8);
        return str;
    }
#endif
    return "";
}

void ProductMatcher::GetTerms_(const UString& text, std::vector<term_t>& term_list)
{
    //TODO: this will return one token 'd90' for "D90" input in sf1r enviorment, why?
    std::vector<Term> t_list;
    AnalyzeNoSymbol_(text, t_list);
    term_list.resize(t_list.size());
    for(uint32_t i=0;i<t_list.size();i++)
    {
        term_list[i] = t_list[i].id;
    }
}
//void ProductMatcher::GetCRTerms_(const UString& text, std::vector<term_t>& term_list)
//{
    //std::vector<UString> text_list;
    //AnalyzeCR_(text, text_list);
    //term_list.resize(text_list.size());
    //for(uint32_t i=0;i<term_list.size();i++)
    //{
        //term_list[i] = GetTerm_(text_list[i]);
    //}
//}
void ProductMatcher::GetTerms_(const std::string& text, std::vector<term_t>& term_list)
{
    UString utext(text, UString::UTF_8);
    GetTerms_(utext, term_list);
}
void ProductMatcher::GetTermsString_(const UString& text, std::string& str)
{
    std::vector<Term> t_list;
    AnalyzeNoSymbol_(text, t_list);
    for(uint32_t i=0;i<t_list.size();i++)
    {
        std::string s;
        t_list[i].text.convertString(s, UString::UTF_8);
        str+=s;
    }
}

void ProductMatcher::GenSuffixes_(const std::vector<term_t>& term_list, Suffixes& suffixes)
{
    suffixes.resize(term_list.size());
    for(uint32_t i=0;i<term_list.size();i++)
    {
        suffixes[i].assign(term_list.begin()+i, term_list.end());
    }
}
void ProductMatcher::GenSuffixes_(const std::string& text, Suffixes& suffixes)
{
    std::vector<term_t> terms;
    GetTerms_(text, terms);
    GenSuffixes_(terms, suffixes);
}
void ProductMatcher::ConstructSuffixTrie_(TrieType& trie)
{
    for(uint32_t i=1;i<category_list_.size();i++)
    {
        if(i%100==0)
        {
            LOG(INFO)<<"scanning category "<<i<<std::endl;
        }
        Category category = category_list_[i];
        if(category.name.empty()) continue;
        uint32_t cid = category.cid;
        const std::vector<std::string>& keywords = category.keywords;
        for(uint32_t k=0;k<keywords.size();k++)
        {
            const std::string& w = keywords[k];
            //if(app.find(w)!=app.end()) continue;
            std::vector<term_t> term_list;
            GetTerms_(w, term_list);
            CategoryNameApp cn_app;
            cn_app.cid = cid;
            cn_app.depth = 1;
            cn_app.is_complete = true;
            trie[term_list].category_name_apps.push_back(cn_app);
            //trie[term_list].text = UString(w, UString::UTF_8);
            //Suffixes suffixes;
            //GenSuffixes_(term_list, suffixes);
            //for(uint32_t s=0;s<suffixes.size();s++)
            //{
                //CategoryNameApp cn_app;
                //cn_app.cid = cid;
                //cn_app.depth = depth;
                //cn_app.is_complete = false;
                //if(s==0) cn_app.is_complete = true;
                //trie[suffixes[s]].category_name_apps.push_back(cn_app);
            //}
            //app.insert(w);
        }
        //uint32_t depth = 1;
        //uint32_t ccid = cid;
        //Category ccategory = category;
        //std::set<std::string> app;
        //while(ccid>0)
        //{
            //const std::vector<std::string>& keywords = ccategory.keywords;
            //for(uint32_t k=0;k<keywords.size();k++)
            //{
                //const std::string& w = keywords[k];
                //if(app.find(w)!=app.end()) continue;
                //std::vector<term_t> term_list;
                //GetTerms_(w, term_list);
                //Suffixes suffixes;
                //GenSuffixes_(term_list, suffixes);
                //for(uint32_t s=0;s<suffixes.size();s++)
                //{
                    //CategoryNameApp cn_app;
                    //cn_app.cid = cid;
                    //cn_app.depth = depth;
                    //cn_app.is_complete = false;
                    //if(s==0) cn_app.is_complete = true;
                    //trie[suffixes[s]].category_name_apps.push_back(cn_app);
                //}
                //app.insert(w);
            //}
            //ccid = ccategory.parent_cid;
            //ccategory = category_list_[ccid];
            //depth++;
        //}
    }
    for(uint32_t i=1;i<products_.size();i++)
    {
        if(i%100000==0)
        {
            LOG(INFO)<<"scanning product "<<i<<std::endl;
        }
        uint32_t pid = i;
        const Product& product = products_[i];
        //std::vector<term_t> title_terms;
        //GetTerms_(product.stitle, title_terms);
        //Suffixes title_suffixes;
        //GenSuffixes_(title_terms, title_suffixes);
        //for(uint32_t s=0;s<title_suffixes.size();s++)
        //{
            //SpuTitleApp st_app;
            //st_app.spu_id = pid;
            //st_app.pstart = s;
            //st_app.pend = title_terms.size();
            //trie[title_suffixes[s]].spu_title_apps.push_back(st_app);
        //}
        const std::vector<Attribute>& attributes = product.attributes;
        for(uint32_t a=0;a<attributes.size();a++)
        {
            const Attribute& attribute = attributes[a];
            for(uint32_t v=0;v<attribute.values.size();v++)
            {
                AttributeApp a_app;
                a_app.spu_id = pid;
                a_app.attribute_name = attribute.name;
                a_app.is_optional = attribute.is_optional;
                std::vector<term_t> terms;
                GetTerms_(attribute.values[v], terms);
                trie[terms].attribute_apps.push_back(a_app);
                //trie[terms].text = UString(attribute.values[v], UString::UTF_8);
                //if(!attribute.is_optional&&NeedFuzzy_(attribute.values[v]))
                //{
//#ifdef B5M_DEBUG
                    //std::cerr<<"need fuzzy "<<attribute.values[v]<<std::endl;
//#endif
                    //Suffixes suffixes;
                    //GenSuffixes_(attribute.values[v], suffixes);
                    //for(uint32_t s=0;s<suffixes.size();s++)
                    //{
                        //FuzzyApp app;
                        //app.spu_id = pid;
                        //app.attribute_name = attribute.name;
                        //app.term_list = terms;
                        //app.pos.begin = s;
                        //ftrie_[suffixes[s]].fuzzy_apps.push_back(app);
                    //}
                //}

            }
        }
    }
}

void ProductMatcher::AddKeyword_(const UString& otext)
{
    std::string str;
    otext.convertString(str, UString::UTF_8);
    boost::algorithm::trim(str);
    boost::algorithm::to_lower(str);
    //std::cerr<<"[AK]"<<str<<std::endl;
    UString text(str, UString::UTF_8);
    if(text.length()<2) return;
    int value_type = 0;//0=various, 1=all digits, 2=all alphabets
    for(uint32_t u=0;u<text.length();u++)
    {
        int ctype = 0;
        if(text.isDigitChar(u))
        {
            ctype = 1;
        }
        else if(text.isAlphaChar(u))
        {
            ctype = 2;
        }
        if(u==0)
        {
            value_type = ctype;
        }
        else {
            if(value_type==1&&ctype==1)
            {
            }
            else if(value_type==2&&ctype==2)
            {
            }
            else
            {
                value_type = 0;
            }
        }
        if(value_type==0) break;
    }
    if(value_type==2 && text.length()<2) return;
    if(value_type==1 && text.length()<3) return;
    std::vector<Term> term_list;
    //std::vector<term_t> term_list;
    AnalyzeNoSymbol_(text, term_list);
    if(term_list.empty()) return;
    std::vector<term_t> id_list(term_list.size());
    for(uint32_t i=0;i<term_list.size();i++)
    {
        id_list[i] = term_list[i].id;
    }
    if(not_keywords_.find(id_list)!=not_keywords_.end()) return;
    if(term_list.size()==1)
    {
        if(term_list[0].text.length()<2) return;
    }
    keyword_set_.insert(id_list);
    keyword_text_[id_list] = text;
    //std::cout<<"[AKT]"<<GetText_(term_list);
    //for(uint32_t i=0;i<term_list.size();i++)
    //{
        //std::cout<<","<<term_list[i];
    //}
    //std::cout<<std::endl;
}

void ProductMatcher::ConstructKeywords_()
{
    for(uint32_t i=1;i<category_list_.size();i++)
    {
        if(i%100==0)
        {
            LOG(INFO)<<"keywords scanning category "<<i<<std::endl;
        }
        const Category& category = category_list_[i];
        //if(category.is_parent) continue;
        if(category.name.empty()) continue;
        std::vector<std::string> cs_list;
        boost::algorithm::split( cs_list, category.name, boost::algorithm::is_any_of(">") );

        for(uint32_t c=0;c<cs_list.size();c++)
        {
            std::string cs = cs_list[c];
            std::vector<std::string> words;
            boost::algorithm::split( words, cs, boost::algorithm::is_any_of("/") );
            for(uint32_t i=0;i<words.size();i++)
            {
                std::string w = words[i];
                UString text(w, UString::UTF_8);
                AddKeyword_(text);
            }
        }
        for(uint32_t k=0;k<category.keywords.size();k++)
        {
            AddKeyword_(UString(category.keywords[k], UString::UTF_8));
        }
    }
    LOG(INFO)<<"keywords count "<<keyword_set_.size()<<std::endl;
    for(uint32_t i=1;i<products_.size();i++)
    {
        if(i%100000==0)
        {
            LOG(INFO)<<"keywords scanning product "<<i<<std::endl;
        }
        const Product& product = products_[i];
        //std::cerr<<product.spid<<std::endl;
        const std::vector<Attribute>& attributes = product.attributes;
        for(uint32_t a=0;a<attributes.size();a++)
        {
            const Attribute& attribute = attributes[a];
            for(uint32_t v=0;v<attribute.values.size();v++)
            {
                const std::string& svalue = attribute.values[v];
                UString uvalue(svalue, UString::UTF_8);
                AddKeyword_(uvalue);
            }
        }
    }
    LOG(INFO)<<"keywords count "<<keyword_set_.size()<<std::endl;
    for(uint32_t i=0;i<keywords_thirdparty_.size();i++)
    {
        UString uvalue(keywords_thirdparty_[i], UString::UTF_8);
        AddKeyword_(uvalue);
    }
    LOG(INFO)<<"keywords count "<<keyword_set_.size()<<std::endl;
}

void ProductMatcher::ConstructKeywordTrie_(const TrieType& suffix_trie)
{
    std::vector<TermList> keywords;
    for(KeywordSet::const_iterator it = keyword_set_.begin(); it!=keyword_set_.end();it++)
    {
        keywords.push_back(*it);
    }
    std::sort(keywords.begin(), keywords.end());
    //typedef TrieType::const_node_iterator node_iterator;
    //std::stack<node_iterator> stack;
    //stack.push(suffix_trie.node_begin());
    all_keywords_.resize(keywords.size()+1);
    for(uint32_t k=0;k<keywords.size();k++)
    {
        if(k%100000==0)
        {
            LOG(INFO)<<"build keyword "<<k<<std::endl;
        }
        uint32_t keyword_id = k+1;
        const TermList& keyword = keywords[k];
        //std::string keyword_str = GetText_(keyword);
        //LOG(INFO)<<"keyword "<<keyword_str<<std::endl;
        KeywordTag tag;
        tag.id = keyword_id;
        tag.term_list = keyword;
        //find keyword in suffix_trie
        //for(TrieType::const_iterator it = suffix_trie.lower_bound(keyword);it!=suffix_trie.end();it++)
        //{
            //const TermList& key = it->first;
            ////std::string key_str = GetText_(key);
            //const KeywordTag& value = it->second;
            //if(StartsWith_(key, keyword))
            //{
                //bool is_complete = false;
                //if(key==keyword) is_complete = true;
                ////LOG(INFO)<<"key found "<<key_str<<std::endl;
                //tag.Append(value, is_complete);
                ////tag+=value;
            //}
            //else
            //{
                ////LOG(INFO)<<"key break "<<key_str<<std::endl;
                //break;
            //}
        //}
        TrieType::const_iterator it = suffix_trie.find(keyword);
        if(it!=suffix_trie.end())
        {
            const KeywordTag& value = it->second;
            tag.Append(value, true);//is complete
        }
        tag.Flush();
        //for(uint32_t i=0;i<tag.category_name_apps.size();i++)
        //{
            //const CategoryNameApp& app = tag.category_name_apps[i];
            //if(app.is_complete)
            //{
                //tag.type_app["c|"+boost::lexical_cast<std::string>(app.cid)] = 1;
                ////break;
            //}
        //}
        //if(tag.type_app.empty())
        //{
            //for(uint32_t i=0;i<tag.attribute_apps.size();i++)
            //{
                //const AttributeApp& app = tag.attribute_apps[i];
                //if(app.spu_id==0) continue;
                ////if(app.is_optional) continue;
                //const Product& p = products_[app.spu_id];
                //uint32_t cid = p.cid;
                //std::string key = boost::lexical_cast<std::string>(cid)+"|"+app.attribute_name;
                //tag.type_app[key]+=1;
                ////double share_point = 0.0;
                ////if(app.is_optional) share_point = 0.5;
                ////else if(app.attribute_name=="型号") share_point = 1.5;
                ////else share_point = 1.0;
                ////sts["a|"+app.attribute_name]+=share_point;
            //}
        //}
        tag.text = keyword_text_[keyword];
        //std::string stext;
        //tag.text.convertString(stext, UString::UTF_8);
        //std::cerr<<"tag:"<<stext<<std::endl;
        trie_[keyword] = tag;
        all_keywords_[keyword_id] = tag;
    }
    //post-process
#ifdef B5M_DEBUG
    for(TrieType::iterator it = trie_.begin();it!=trie_.end();it++)
    {
        KeywordTag& tag = it->second;
        UString utext = keyword_text_[tag.term_list];
        std::string text;
        utext.convertString(text, UString::UTF_8);
        if(!tag.category_name_apps.empty())
        {
            std::cerr<<"XXKTC,"<<text<<std::endl;
        }
        bool has_brand = false;
        bool has_type = false;
        bool has_other = false;
        for(uint32_t i=0;i<tag.attribute_apps.size();i++)
        {
            const AttributeApp& app = tag.attribute_apps[i];
            if(app.spu_id==0) continue;
            if(app.attribute_name=="品牌")
            {
                has_brand = true;
            }
            else if(app.attribute_name=="型号")
            {
                has_type = true;
            }
            else
            {
                has_other = true;
            }
        }
        if(has_brand)
        {
            std::cerr<<"XXKTB,"<<text<<std::endl;
        }
        if(has_type)
        {
            std::cerr<<"XXKTT,"<<text<<std::endl;
        }
        if(has_other)
        {
            std::cerr<<"XXKTO,"<<text<<std::endl;
        }
    }
#endif
}
bool ProductMatcher::IsBlankSplit_(const UString& t1, const UString& t2) const
{
    if(t1.length()==0||t2.length()==0) return true;
    if(t1.isDigitChar(t1.length()-1) && t2.isDigitChar(0)) return true;
    return false;
}
bool ProductMatcher::IsBrand_(const std::string& scategory, const KeywordTag& k) const
{
    if(brand_manager_==NULL) return false;
    if(!k.category_name_apps.empty()) return false;
    bool candidate = false;
    if(k.attribute_apps.empty()) 
    {
        candidate = true;
    }
    for(uint32_t i=0;i<k.attribute_apps.size();i++)
    {
        if(k.attribute_apps[i].attribute_name=="品牌")
        {
            candidate = true;
            break;
        }
    }
    if(brand_manager_==NULL) return candidate;
    else
    {
        std::string text;
        k.text.convertString(text, UString::UTF_8);
        return brand_manager_->IsBrand(scategory, text);
    }
}

