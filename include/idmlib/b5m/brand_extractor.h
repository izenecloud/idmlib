#ifndef IDMLIB_B5M_BRANDEXTRACTOR_H_
#define IDMLIB_B5M_BRANDEXTRACTOR_H_
#include "b5m_helper.h"
#include "b5m_types.h"
#include "scd_doc_processor.h"
#include "b5m_helper.h"
#include <sf1common/ScdParser.h>
#include <sf1common/ScdWriter.h>
#include <am/sequence_file/ssfr.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/algorithm/searching/boyer_moore.hpp>

//#define BE_DEBUG

NS_IDMLIB_B5M_BEGIN

using izenelib::util::UString;

class BrandExtractor {

    typedef uint32_t term_t;
    typedef std::vector<term_t> word_t;
    typedef boost::unordered_map<std::string, term_t> CategoryIndex;
    typedef boost::unordered_map<term_t, std::pair<double, std::string> > CategoryScore;
    typedef boost::unordered_set<word_t> Invalid;

    struct ScoreBuffer {
        typedef boost::unordered_map<term_t, std::size_t> TModeCount;
        typedef boost::unordered_map<term_t, std::string> TModeText;
        TModeCount tmc;
        TModeText tmt;
        void add(term_t tmode, std::size_t count=1)
        {
            TModeCount::iterator it = tmc.find(tmode);
            if(it==tmc.end())
            {
                tmc.insert(std::make_pair(tmode, count));
            }
            else
            {
                it->second+=count;
            }
        }
        void add_text(term_t tmode, const std::string& text)
        {
            tmt[tmode] = text;
        }
        std::size_t get(term_t tmode) const
        {
            TModeCount::const_iterator it = tmc.find(tmode);
            if(it==tmc.end()) return 0;
            else return it->second;
        }
        bool get_text(term_t tmode, std::string& text) const
        {
            TModeText::const_iterator it = tmt.find(tmode);
            if(it==tmt.end()) return false;
            else
            {
                text = it->second;
                return true;
            }
        }
    };

    struct BrandValue {

        typedef std::pair<term_t, term_t> key_t;
        struct Value {
            typedef boost::unordered_map<std::string, std::size_t> Count;
            Value():all_count(0)
            {
            }
            Count text_count;
            std::size_t all_count;
            void add(const std::string& text, std::size_t count=1)
            {
                Count::iterator it = text_count.find(text);
                if(it!=text_count.end()) it->second+=count;
                else text_count.insert(std::make_pair(text, count));
                all_count+=count;
                //if(all_count%100==0)
                //{
                //    std::cerr<<"[D]"<<text<<","<<all_count<<std::endl;
                //}
            }
        };
        typedef boost::unordered_map<key_t, Value> Map;

        void add(term_t cid, const std::string& text, term_t tmode)
        {
            key_t key(tmode, cid);
            Value& v = map[key];
            v.add(text);
        }

        void flush(CategoryScore& cscore) const
        {
            typedef boost::unordered_map<term_t, ScoreBuffer> BufferMap;//key is cid
            BufferMap buffer_map;
            for(Map::const_iterator it=map.begin();it!=map.end();++it)
            {
                term_t tmode = it->first.first;
                term_t cid = it->first.second;
                const Value& value = it->second;
                ScoreBuffer& sb = buffer_map[cid];
                sb.add(tmode, value.all_count);
                std::pair<std::string, std::size_t> max("", 0);
                for(Value::Count::const_iterator cit = value.text_count.begin();cit!=value.text_count.end();++cit)
                {
                    const std::string& text = cit->first;
                    std::size_t count = cit->second;
                    if(count>max.second)
                    {
                        max.first = text;
                        max.second = count;
                    }
                }
                sb.add_text(tmode, max.first);
            }
            for(BufferMap::const_iterator it = buffer_map.begin();it!=buffer_map.end();++it)
            {
                term_t cid = it->first;
                const ScoreBuffer& buffer = it->second;
                std::size_t spu_app = buffer.get(1);
                std::size_t offer_app = buffer.get(2);
                if(spu_app==0&&offer_app<30) continue;
                std::string spu_text;
                std::string offer_text;
                buffer.get_text(1, spu_text);
                buffer.get_text(2, offer_text);
                std::string text = offer_text;
                if(!spu_text.empty()) text = spu_text;
                if(text.empty()) continue;
                double score = 0.0;
                if(spu_app>=3)
                {
                    score = std::log((double)spu_app);
                }
                else
                {
                    score = (double)offer_app/10000;
                    if(score>1.0) score = 1.0;
                    if(spu_app>0) score *=1.2;
                }
                cscore[cid] = std::make_pair(score,text);
            }
        }

        std::string token;
        Map map;


    };

    typedef boost::unordered_map<word_t, BrandValue> BV;
    typedef std::map<word_t, CategoryScore> Data;

    struct MatchCandidate {
        MatchCandidate()
        {
        }
        MatchCandidate(const word_t& k, term_t p, const CategoryScore* c)
        :key(k), pos(p), cscore(c)
        {
        }
        word_t key;
        term_t pos;
        const CategoryScore* cscore;
    };

public:

    BrandExtractor()
    {
        idmlib::util::IDMAnalyzerConfig csconfig = idmlib::util::IDMAnalyzerConfig::GetCommonConfig("","", "");
        csconfig.symbol = true;
        analyzer_ = new idmlib::util::IDMAnalyzer(csconfig);
    }

    ~BrandExtractor()
    {
        delete analyzer_;
    }

    bool Load(const std::string& path)
    {
        std::string category_file = path+"/category";
        LoadCategory_(category_file);
        std::string invalid_file = path+"/invalid";
        if(boost::filesystem::exists(invalid_file))
        {
            std::ifstream ifs(invalid_file.c_str());
            std::string line;
            while(getline(ifs, line))
            {
                word_t word;
                GetWord_(line, word);
                invalid_.insert(word);
            }
            ifs.close();
        }
        std::string resource_dir = path+"/resource";
        std::string rfile = resource_dir+"/result";
        if(boost::filesystem::exists(rfile))
        {
            izenelib::am::ssf::Util<>::Load(rfile, data_);
            LOG(INFO)<<"data size "<<data_.size()<<std::endl;
            return true;
        }
        return false;
    }

    bool Train(const std::string& path, int thread_num=1)
    {
        if(Load(path)) return true;
        {
        std::string spu_scd = path+"/SPU.SCD";
        ScdDocProcessor processor(boost::bind(&BrandExtractor::ProcessSPU_, this, _1), 1);
        processor.AddInput(spu_scd);
        processor.Process();
        }
        {
        std::string offer_scd = path+"/OFFER.SCD";
        ScdDocProcessor processor(boost::bind(&BrandExtractor::ProcessOffer_, this, _1), thread_num);
        processor.AddInput(offer_scd);
        processor.Process();
        }
        for(BV::const_iterator it=brand_value_.begin();it!=brand_value_.end();++it)
        {
            const BrandValue& value = it->second;
            CategoryScore cscore;
            value.flush(cscore);
            data_[it->first] = cscore;
        }
        std::string resource_dir = path+"/resource";
        std::string rfile = resource_dir+"/result";
        if(!boost::filesystem::exists(resource_dir))
        {
            boost::filesystem::create_directories(resource_dir);
            izenelib::am::ssf::Util<>::Save(rfile, data_);
        }
        Show();
        return true;
    }

    void Show()
    {
        for(BV::const_iterator it=brand_value_.begin();it!=brand_value_.end();++it)
        {
            const BrandValue& value = it->second;
            std::cout<<value.token<<std::endl;
            BrandValue::Map map = value.map;
            for(BrandValue::Map::const_iterator it2 = map.begin();it2!=map.end();++it2)
            {
                const std::pair<term_t, term_t>& key = it2->first;
                term_t tmode = key.first;
                term_t cid = key.second;
                const std::string& category = category_list_[cid];
                const BrandValue::Value& v = it2->second;
                std::cout<<tmode<<","<<category<<","<<v.all_count<<std::endl;
                for(BrandValue::Value::Count::const_iterator it3 = v.text_count.begin();it3!=v.text_count.end();++it3)
                {
                    const std::string& text = it3->first;
                    std::size_t count = it3->second;
                    std::cout<<"\t"<<text<<","<<count<<std::endl;
                }
            }
        }
    }

    void Evaluate(const Document& doc, std::string& brand)
    {
        term_t cid=0;
        std::string category;
        doc.getString("Category", category);
        if(!GetCategoryId_(category, cid)) return;
        std::string title;
        doc.getString("Title", title);
#ifdef BE_DEBUG
        std::cerr<<"[T]"<<title<<std::endl;
#endif
        word_t word;
        GetWord_(title, word);
        word_t key;
        std::vector<MatchCandidate> matches;
        for(uint32_t i=0;i<word.size();i++)
        {
            key.push_back(word[i]);
            Data::const_iterator it = data_.lower_bound(key);
            if(it==data_.end()||!boost::algorithm::starts_with(it->first, key))
            {
                key.resize(0);
            }
            else if(it->first==key)
            {
                term_t pos = i+1-key.size();
                double pos_ratio = (double)pos/word.size();
                if(pos_ratio>=0.4) break;
                MatchCandidate mc(key, pos, &(it->second));
                //if(!matches.empty())
                //{
                //    MatchCandidate& last = matches.back();
                //    if(boost::algorithm::starts_with(key, last.key))
                //    {
                //        last = mc;
                //        continue;
                //    }
                //}
                matches.push_back(mc);
            }
        }
        std::pair<double, std::string> result(0.0, "");
        for(std::size_t i=0;i<matches.size();i++)
        {
            const MatchCandidate& mc = matches[i];
            if(invalid_.find(mc.key)!=invalid_.end()) continue;
            const CategoryScore& cscore = *(mc.cscore);
            CategoryScore::const_iterator it = cscore.find(cid);
            if(it==cscore.end()) continue;
            double score = it->second.first;
            const std::string& text = it->second.second;
            score *= (word.size()-mc.pos);//position weight
            if(score>result.first)
            {
                result.first = score;
                result.second = text;
            }
        }
        if(!result.second.empty()) brand = result.second;
    }

    void Test(const std::string& scd_path, int thread_num = 1)
    {
        ScdDocProcessor::ProcessorType p = boost::bind(&BrandExtractor::Test_, this, _1);
        ScdDocProcessor sd_processor(p, 1);
        sd_processor.AddInput(scd_path);
        sd_processor.Process();
    }


private:
    term_t GetTerm_(const std::string& str)
    {
        term_t term = izenelib::util::HashFunction<std::string>::generateHash32(str);
        return term;
    }
    void GetWord_(const std::string& text, word_t& word)
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
    void LoadCategory_(const std::string& file)
    {
        std::string line;
        std::ifstream ifs(file.c_str());
        category_list_.resize(1);
        while(getline(ifs, line))
        {
            boost::algorithm::trim(line);
            std::vector<std::string> vec;
            boost::algorithm::split(vec, line, boost::algorithm::is_any_of(","));
            if(vec.size()<1) continue;
            const std::string& scategory = vec[0];
            CategoryIndex::const_iterator it = category_index_.find(scategory);
            if(it==category_index_.end())
            {
                term_t cid = category_index_.size()+1;
                category_index_.insert(std::make_pair(scategory, cid));
                category_list_.push_back(scategory);
            }
        }
        ifs.close();
    }
    bool GetCategoryId_(const std::string& category, term_t& cid) const
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

    void AppendBrand_(const std::string& brand, term_t cid, const std::string& title)
    {
        term_t tmode = 1;//spu
        if(!title.empty()) tmode=2;//offer
        const std::string& text = brand;
        std::string ntext = boost::algorithm::to_lower_copy(text);
        std::vector<std::string> tokens;
        boost::algorithm::split(tokens, ntext, boost::algorithm::is_any_of("/"));
        if(tokens.size()>2) return;

        std::vector<word_t> word_list(tokens.size());
        for(std::size_t i=0;i<tokens.size();i++)
        {
            GetWord_(tokens[i], word_list[i]);
        }
        if(!title.empty())
        {
            word_t tword;
            GetWord_(title, tword);
            bool find = false;
            for(std::size_t i=0;i<word_list.size();i++)
            {
                word_t::const_iterator it = boost::algorithm::boyer_moore_search(tword, word_list[i]);
                if(it!=tword.end())
                {
                    find=true;
                    break;
                }
            }
            if(!find) return;
        }

        boost::unique_lock<boost::mutex> lock(mutex_);
        ntext_map_[ntext] = text;
        for(std::size_t i=0;i<tokens.size();i++)
        {
            BV::iterator it = brand_value_.find(word_list[i]);
            if(it==brand_value_.end())
            {
                BrandValue v;
                v.token = tokens[i];
                v.add(cid, ntext, tmode);
                brand_value_.insert(std::make_pair(word_list[i], v));
            }
            else
            {
                it->second.add(cid, ntext, tmode);
            }
        }
    }

    void ProcessSPU_(ScdDocument& doc)
    {
        std::string category;
        doc.getString("Category", category);
        term_t cid=0;
        if(!GetCategoryId_(category, cid)) return;

        UString attrib;
        doc.getString("Attribute", attrib);
        std::vector<Attribute> attributes;
        B5MHelper::ParseAttributes(attrib, attributes);
        for(uint32_t i=0;i<attributes.size();i++)
        {
            if(attributes[i].name=="品牌") 
            {
                std::string avalue = attributes[i].GetValue();
                AppendBrand_(avalue, cid, "");
                break;
            }
        }
        
    }
    void ProcessOffer_(ScdDocument& doc)
    {
        //std::string source;
        //doc.getString("Source", source);
        //if(source!="淘宝网"&&source!="天猫") return;
        std::string category;
        doc.getString("Category", category);
        term_t cid=0;
        if(!GetCategoryId_(category, cid)) return;
        std::string brand;

        UString attrib;
        doc.getString("Attribute", attrib);
        std::vector<Attribute> attributes;
        B5MHelper::ParseAttributes(attrib, attributes);
        for(uint32_t i=0;i<attributes.size();i++)
        {
            if(attributes[i].name=="品牌") 
            {
                brand = attributes[i].GetValue();
                break;
            }
        }
        if(brand.empty())
        {
            for(uint32_t i=0;i<attributes.size();i++)
            {
                if(boost::algorithm::ends_with(attributes[i].name,"品牌")) 
                {
                    brand = attributes[i].GetValue();
                    break;
                }
            }
        }
        if(brand.empty()) return;
        std::string title;
        doc.getString("Title", title);
        if(title.empty()) return;
        AppendBrand_(brand, cid, title); 
        
    }

    void Test_(ScdDocument& doc)
    {
        std::string brand;
        Evaluate(doc, brand);
        if(!brand.empty())
        {
            std::string title;
            doc.getString("Title", title);
            std::string category;
            doc.getString("Category", category);
            std::cout<<"[R]"<<title<<","<<category<<"\t:\t"<<brand<<std::endl;
        }
    }

private:
    idmlib::util::IDMAnalyzer* analyzer_;
    BV brand_value_;
    boost::unordered_map<std::string, std::string> ntext_map_;
    CategoryIndex category_index_;
    std::vector<std::string> category_list_;
    Data data_;
    Invalid invalid_;

    boost::mutex mutex_;

};

NS_IDMLIB_B5M_END

#endif

