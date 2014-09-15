#ifndef IDMLIB_B5M_TUANPROCESSOR_H
#define IDMLIB_B5M_TUANPROCESSOR_H 
#include <util/ustring/UString.h>
#include <idmlib/duplicate-detection/dup_detector.h>
#include "product_price.h"
#include "b5m_helper.h"
#include "b5m_types.h"
#include "b5m_m.h"
#include <boost/unordered_map.hpp>
#include <sf1common/ScdWriter.h>
#include <sf1common/PairwiseScdMerger.h>
#include "address_extract.h"

NS_IDMLIB_B5M_BEGIN

class TuanProcessor{
    typedef izenelib::util::UString UString;

    class TuanProcessorAttach
    {
    public:
        uint32_t sid; //sid should be same
        ProductPrice price;
        std::vector<std::string> area_array;//sorted, should be at least one same

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            ar & sid & price & area_array;
        }

        bool dd(const TuanProcessorAttach& other) const
        {
            if(sid!=other.sid) return false;
            return true;
            double mid1;
            double mid2;
            if(!price.GetMid(mid1)) return false;
            if(!other.price.GetMid(mid2)) return false;
            double max = std::max(mid1, mid2);
            double min = std::min(mid1, mid2);
            if(min<=0.0) return false;
            double ratio = max/min;
            if(ratio>1.01) return false;
            if(area_array.empty()&&other.area_array.empty()) return true;
            bool found = false;
            uint32_t i=0,j=0;
            while(i<area_array.size()&&j<other.area_array.size())
            {
                if(area_array[i]==other.area_array[j])
                {
                    found = true;
                    break;
                }
                else if(area_array[i]<other.area_array[j])
                {
                    ++i;
                }
                else
                {
                    ++j;
                }
            }
            if(!found) return false;
            return true;
        }
    };
    typedef std::string DocIdType;

    struct ValueType
    {
        std::string soid;
        UString title;

        bool operator<(const ValueType& other) const
        {
            return title.length()<other.title.length();
        }
        
    };


    typedef idmlib::dd::DupDetector<DocIdType, uint32_t, TuanProcessorAttach> DDType;
    typedef DDType::GroupTableType GroupTableType;
    typedef PairwiseScdMerger::ValueType SValueType;
    typedef boost::unordered_map<uint128_t, SValueType> CacheType;

public:
    TuanProcessor(const B5mM& b5mm);
    bool Generate(const std::string& mdb_instance);

    void SetCmaPath(const std::string& path)
    { cma_path_ = path; }

private:

    void POutputAll_();
    void B5moOutput_(SValueType& value, int status);
    uint128_t GetPid_(const Document& doc);
    uint128_t GetOid_(const Document& doc);
    void ProductMerge_(SValueType& value, const SValueType& another_value);


//add by wangbaobao@b5m.com begin---------------------------------------------------------
	
	//pre-process shop-system scd files
	//only store the useful information like merchant area merchant address shop name and docid.
	struct ShopSystemAux
	{
		std::string					   docid;
		std::string					   address;
		std::string					   shop_name;
		AddressExtract::EvaluateResult address_evaluate;
	};
	
	struct Frequence
	{
		Frequence()
		{
			freq[0] = 0;
			freq[1] = 0;
		}
		size_t freq[2];
	};

	typedef std::pair<std::string, std::string>            CityAreaPair;
	typedef boost::unordered_map<CityAreaPair, std::vector<ShopSystemAux> > ShopSystemMap;
	typedef boost::unordered_set<std::string>			   NGramSet;
	typedef boost::unordered_map<std::string, std::string> ShopTuanMap;
	typedef boost::unordered_map<std::string, Frequence>   WordFreqMap;

private:
	size_t UTF8Length(char z);

	inline void BuildShopids(std::string &shopids, const std::string &id)
	{
		shopids += id;
		shopids += ",";
	}

	int InitAddressModule(const std::string &knowledge_path);

	int  PreprocessShopSystem(const std::string& scd_path);

	void MatchShop();

	void Ngram(const std::string &value, size_t max_size, std::vector<std::string> &out_ngram);
	
	bool JudgeSimilar(const std::string &name1, const std::string &name2);

	bool JudgeSimilar_v2(const std::string &name1, const std::string &name2);
	
private:
	bool		   is_init_;
	//train knowledge path
	//std::string	   knowledge_path_;
	//shop docid mapping tuan docid
	ShopTuanMap   shop_tuan_map_;
	//shop system cache
	ShopSystemMap shop_cache_;
	//address normalization module
	AddressExtract address_extractor_;
	
//add by wangbaobao@b5m.com end-------------------------------------------------------------------------
private:
    B5mM b5mm_;
    std::string cma_path_;
    CacheType cache_;
    boost::shared_ptr<ScdWriter> pwriter_;
};

NS_IDMLIB_B5M_END

#endif

