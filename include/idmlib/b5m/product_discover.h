#ifndef IDMLIB_B5M_PRODUCTDISCOVER_H_
#define IDMLIB_B5M_PRODUCTDISCOVER_H_
#include "b5m_helper.h"
#include "b5m_types.h"
#include "product_matcher.h"
#include "cosmetics_ngram.h"
#include "brand_extractor.h"
#include <sf1common/ScdWriter.h>
#include <boost/shared_ptr.hpp>

NS_IDMLIB_B5M_BEGIN

using izenelib::util::UString;

class ProductDiscover {
public:
    typedef uint32_t term_t;
    typedef std::vector<term_t> word_t;
    typedef CosmeticsNgram::Ngram Ngram;
    typedef CosmeticsNgram::NgramStat NgramStat;
    typedef std::pair<std::string, std::string> Key;
    typedef std::vector<std::string> Value;
    typedef boost::unordered_map<Key, Value> Map;
    typedef std::vector<ScdDocument> CValue;
    typedef boost::unordered_map<std::string, CValue> CMap;
    typedef boost::unordered_map<std::string, std::string> SMap;
    typedef std::map<word_t, term_t> BidMap;
    typedef boost::unordered_map<std::size_t, std::string> IdStringMap;
    typedef boost::unordered_map<term_t, std::string> BidTextMap;
    typedef boost::unordered_map<std::string, std::size_t> StringIdMap;
    typedef boost::unordered_map<std::string, term_t> StringBidMap;
    typedef boost::unordered_map<term_t, std::size_t> BrandFreq;
    typedef StringIdMap CategoryIndex;
    typedef izenelib::am::ssf::Reader<std::size_t> Reader;
    struct Result {
        Result() {}
        Result(const std::string& p1, const std::string& p2, const std::string& p3, const std::string& p4)
        : category(p1), brand(p2), model(p3), otitle(p4)
        {
        }
        std::string category;
        std::string brand;
        std::string model;
        std::string otitle;
        bool operator<(const Result& y) const
        {
            if(brand!=y.brand) return brand<y.brand;
            return model<y.model;
        }
    };

    typedef std::pair<term_t, std::size_t> BrandModelCooc;//bm index and category
    struct CoocValue {
        CoocValue():count(0), valid(true)
        {
        }
        CoocValue(std::size_t c, const std::string& o):count(c), otitle(o), valid(true)
        {
        }
        std::size_t count;
        std::string otitle;
        std::vector<double> price_list;
        bool valid;
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & count & otitle & price_list;
        }
    };
    typedef boost::unordered_map<BrandModelCooc, CoocValue> CoocMap;

    struct BrandModelInfo {
        BrandModelInfo() : valid(true)
        {
        }

        BrandModelInfo(const std::string& b, const std::string& m, std::size_t i)
        : brand(b), model(m), id(i), valid(true)
        {
        }
        std::string brand;
        std::string model;
        std::size_t id;
        CoocMap cooc;
        bool valid;
        void add_cooc(term_t cid, std::size_t tid, const std::string& otitle, double price, std::size_t count=1)
        {
            BrandModelCooc c(cid, tid);
            CoocMap::iterator it = cooc.find(c);
            if(it==cooc.end())
            {
                CoocValue v(count, otitle);
                if(tid==id) v.price_list.push_back(price);
                cooc.insert(std::make_pair(c, v));
            }
            else
            {
                it->second.count+=count;
                if(tid==id) it->second.price_list.push_back(price);
            }
        }
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & brand & model & id & cooc & valid;
        }
    };
    typedef std::vector<BrandModelInfo> BrandModelList;

    struct BrandModelManager {

        typedef boost::unordered_map<std::string, std::size_t> Index;
        typedef boost::unordered_map<term_t, BrandModelInfo> BrandModelMap;

        std::size_t add_and_index(const std::string& brand, const std::string& model)
        {
            std::string key = brand+","+model;
            Index::const_iterator it = index.find(key);
            if(it==index.end())
            {
                std::size_t ind = bm_list.size();
                BrandModelInfo bmi(brand, model, ind);
                bm_list.push_back(bmi);
                index.insert(std::make_pair(key, ind));
                return ind;
            }
            else
            {
                return it->second;
            }
        }

        void add(term_t cid, const std::string& brand, const std::vector<std::string>& models, const std::string& otitle, double price)
        {
            std::vector<std::size_t> indexes(models.size());
            for(std::size_t i=0;i<models.size();i++)
            {
                std::size_t index = add_and_index(brand, models[i]);
                indexes[i] = index;
            }
            for(std::size_t i=0;i<indexes.size();i++)
            {
                std::size_t index = indexes[i];
                BrandModelInfo& bmi = bm_list[index];
                for(std::size_t j=0;j<indexes.size();j++)
                {
                    bmi.add_cooc(cid, indexes[j], otitle, price, 1);
                }
            }
        }
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & index & bm_list;
        }

        Index index;
        BrandModelList bm_list;
    };

    struct Found {
        std::size_t count;
        term_t cid;
        double price;
        std::string otitle;
        bool operator<(const Found& y) const
        {
            return count>y.count;
        }
    };
    typedef std::vector<Result> Results;
    typedef boost::unordered_map<std::string, Results> ResultMap;
    ProductDiscover(ProductMatcher* matcher=NULL, BrandExtractor* be=NULL);
    ~ProductDiscover();
    bool Process(const std::string& scd_path, int thread_num=1);
    bool ProcessCosmetics(const std::string& path, int thread_num=1);
    bool BrandProcess(const std::string& scd_path, int thread_num=1);
private:
    void GetWord_(const std::string& text, word_t& word);
    void LoadCategory_(const std::string& file);
    bool GetCategoryId_(const std::string& category, term_t& cid);
    bool ValidCategory_(const std::string& category) const;
    void Process_(ScdDocument& doc);
    bool Extract_(const ScdDocument& doc, std::vector<Ngram>& ngrams);
    void ProcessCosmetics_(ScdDocument& doc);
    void ProcessSPU_(ScdDocument& doc);
    void ProcessCosmeticsSPU_(ScdDocument& doc);
    bool NgramProcess_(NgramStat& ngram);
    bool NgramFilter_(NgramStat& ngram);
    void ResultAppend_(ResultMap& result, const Result& r);
    void ProcessBrand_(ScdDocument& doc);
    void ExtractModels_(const std::string& otitle, std::vector<std::string>& models);
    bool ModelFilter3C_(const std::string& model) const;
    void GetBrandAndModel_(const ScdDocument& doc, std::vector<std::string>& brands, std::vector<std::string>& models);
    void Insert_(const ScdDocument& doc, const std::vector<std::string>& brands, const std::vector<std::string>& models);
    bool CValid_(const std::string& key, const CValue& value) const;
    bool Output_(const std::string& category, const std::string& brand, const std::string& model, double price, const std::string& otitle) const;
    term_t GetTerm_(const std::string& text) const;

private:
    ProductMatcher* matcher_;
    BrandExtractor* be_;
    idmlib::util::IDMAnalyzer* analyzer_;
    Map map_;
    CMap cmap_;
    SMap smap_;
    SMap msmap_;
    boost::atomic<std::size_t> oid_;
    std::vector<std::string> otitle_list_;
    CategoryIndex category_index_;
    IdStringMap category_text_;
    term_t bid_;
    StringBidMap sbid_map_;
    BidMap bid_map_;
    BidTextMap bid_text_;
    BrandFreq brand_freq_;
    BrandModelManager bm_manager_;
    CosmeticsNgram* ngram_processor_;
    boost::shared_ptr<ScdWriter> writer_;
    std::vector<boost::regex> cregexps_;
    std::vector<boost::regex> error_cregexps_;
    boost::regex model_regex_;
    boost::regex capacity_regex_;
    std::vector<boost::regex> error_model_regex_;
    std::vector<boost::regex> error_3cmodel_regex_;
    boost::mutex mutex_;
    std::vector<std::size_t> bp_;
};
NS_IDMLIB_B5M_END

#endif

