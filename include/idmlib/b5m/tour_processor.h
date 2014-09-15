#ifndef IDMLIB_B5M_TOURPROCESSOR_H
#define IDMLIB_B5M_TOURPROCESSOR_H 
#include <util/ustring/UString.h>
#include "product_price.h"
#include "b5m_helper.h"
#include "b5m_types.h"
#include "b5m_m.h"
#include <boost/unordered_map.hpp>
#include <sf1common/ScdWriter.h>
#include <sf1common/ScdDocument.h>

NS_IDMLIB_B5M_BEGIN

const static std::string SCD_SOURCE		= "Source";
const static std::string SCD_PRICE		= "SalesPrice";
const static std::string SCD_FROM_CITY	= "FromCity";
const static std::string SCD_TO_CITY	= "ToCity";
const static std::string SCD_DOC_ID		= "DOCID";
const static std::string SCD_TIME_PLAN	= "TimePlan";
const static std::string SCD_UUID		= "uuid";
const static std::string SCD_START_DATE	= "StartDate";

class TourProcessor{
	typedef uint32_t term_t;
	typedef std::vector<term_t> word_t;
	
	struct BufferValueItem
	{
		std::string from;
        std::vector<std::string> to;
		std::pair<uint32_t, uint32_t> days;
		double price;
        boost::unordered_set<word_t> locations;
		ScdDocument doc;
		bool is_free;//is free walk
		bool bcluster;
		bool operator<(const BufferValueItem& another) const
		{
			return price < another.price ? true:false;
		}
	};

	typedef izenelib::util::UString UString;
	typedef std::string BufferKey;
	//typedef std::pair<std::string, std::string> BufferKey;	
	typedef std::vector<BufferValueItem> BufferValue;
	typedef boost::unordered_set<std::string> Set;
	typedef std::set<word_t> Dict;
	typedef BufferValue Group;
	typedef boost::unordered_map<term_t, std::string> TermText;

	//struct UnionBufferKey
	//{
	//	std::vector<BufferKey> union_key_;
	//	
	//	bool operator<(const UnionBufferKey&key)const 
	//	{
	//		boost::unordered_set<BufferKey> key_set;
	//		for(std::vector<BufferKey>::const_iterator iter = union_key_.begin();
	//					iter != union_key_.end(); ++iter)
	//		{
	//			key_set.insert(*iter);
	//		}

	//		for(std::vector<BufferKey>::const_iterator iter = key.union_key_.begin();
	//					iter != key.union_key_.end(); ++iter)
	//		{
	//			if(key_set.find(*iter) != key_set.end())
	//			{
	//				//notice:equal key return false
	//				return false;
	//			}
	//		}
	//		return union_key_[0] < key.union_key_[0] ? true:false;
	//	}

	//	void push_back(const BufferKey&key)
	//	{
	//		union_key_.push_back(key);
	//	}

	//	size_t size()const
	//	{
	//		return union_key_.size();
	//	}
	//};
	typedef std::map<BufferKey, BufferValue> Buffer;

public:
	TourProcessor(const B5mM&b5mm);
	~TourProcessor();
	bool Generate(const std::string& mdb_instance);

private:
    void GetWord_(const std::string& text, word_t& word);
    term_t GetTerm_(const std::string& text);
    std::string GetText_(const word_t& word);
	void Insert_(ScdDocument& doc);
	std::pair<uint32_t, uint32_t> ParseDays_(const std::string& sdays) const;
	void GenLocations_(BufferValueItem& value, const BufferKey& key);
	bool CheckFree_(const ScdDocument& doc) const;
	void Finish_();
	
	void FindGroups_(BufferValue& value);
	void GenP_(Group& g, Document& doc) const;

	//void GenerateUnionKey(UnionBufferKey&union_key, const std::string&from_city, const std::string& to_city)const;
	void LogGroup(const Group&group);

private:
	B5mM  b5mm_;
	std::string m_;
    idmlib::util::IDMAnalyzer* analyzer_;
	Buffer buffer_;
	Dict location_dict_;
	TermText term_text_;
    //std::size_t debug_;
	boost::mutex mutex_;
};

NS_IDMLIB_B5M_END

#endif
