#include <idmlib/b5m/tuan_processor.h>
#include <idmlib/b5m/b5m_types.h>
#include <idmlib/b5m/b5m_helper.h>
#include <idmlib/b5m/scd_doc_processor.h>
#include <idmlib/b5m/product_db.h>
#include <idmlib/b5m/tuan_clustering.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <glog/logging.h>
#include <sf1common/ScdParser.h>
#include <sf1common/Document.h>
#include <sf1common/split_ustr.h>
#include <idmlib/b5m/product_term_analyzer.h>


using namespace idmlib::b5m;
TuanProcessor::TuanProcessor(const B5mM& b5mm) : b5mm_(b5mm)
{
}

////////////////////////////////////////////////////////////////////

//add by wangbaobao@b5m.com ---------------begin--------------------
//simply calculate the utf8 encoding word length
size_t TuanProcessor::UTF8Length(char z)
{
	if ((z & 0xfc) == 0xfc) {
		return 6;
	} else if ((z & 0xf8) == 0xf8) {
		return 5;
	} else if ((z & 0xf0) == 0xf0) {
		return 4;
	} else if ((z & 0xe0) == 0xe0) {
		return 3;
	} else if ((z & 0xc0) == 0xc0) {
		return 2;
	} else if ((z & 0x80) == 0x00) {
		return 1;
	}
	assert(0);
}

//extract the n-gram words
//specify the max length of n-gram
void TuanProcessor::Ngram(const std::string	&value, 
						  size_t max_size, 
						  std::vector<std::string> &out_ngram)
{
	out_ngram.clear();
	if(value.empty() || max_size < 2){
		return;
	}
	size_t length = value.length();
	size_t index = 0;

	while(index < length)
	{
		size_t end_position = index;
		size_t count = 2;
		end_position += UTF8Length(value[end_position]);
		if(end_position >= length) {
			break;
		}
		do 
		{
			assert(end_position < length);
			end_position += UTF8Length(value[end_position]);
			out_ngram.push_back(value.substr(index, end_position - index));
			++count;
		} while(end_position < length && count <= max_size);

		//update start index
		assert(index < length);
		index += UTF8Length(value[index]);
	}
}

//calculate the similarity of two shop names
bool TuanProcessor::JudgeSimilar_v2(const std::string &name1,
								    const std::string &name2)
{
	if(name1.empty() || name2.empty()) return false;
	
	//using vector model calculate the cos(name1, name2)
	static const double      kThreshold = 0.85;
	std::vector<std::string> out_ngram1;
	std::vector<std::string> out_ngram2;
	WordFreqMap				 word_freq_map;

	//find the reasonable max_size
	Ngram(name1, 4, out_ngram1);
	Ngram(name2, 4, out_ngram2);

	for(size_t i = 0; i < out_ngram1.size(); i++)
	{
		word_freq_map[out_ngram1[i]].freq[0] += 1;
	}

	for(size_t i = 0; i < out_ngram2.size(); i++)
	{
		word_freq_map[out_ngram2[i]].freq[1] += 1;
	}
	
	WordFreqMap::const_iterator iter0;
	double v0 = 0.0;
	double v1 = 0.0;
	double v2 = 0.0;
	for(iter0 = word_freq_map.begin(); iter0 != word_freq_map.end(); ++iter0)
	{
		v0 += iter0->second.freq[0] * iter0->second.freq[1];	
		v1 += iter0->second.freq[0] * iter0->second.freq[0];
		v2 += iter0->second.freq[1] * iter0->second.freq[1];
	}
	return v0 / (sqrt(v1) * sqrt(v2)) >= kThreshold ? true : false;
}

bool TuanProcessor::JudgeSimilar(const std::string &name1, 
								 const std::string &name2)
{
	if(name1.empty() || name2.empty()) return false;

	static const double kThreshold = 0.9;
	size_t matched_length = 0;
	std::vector<std::string> out_ngram1;
	std::vector<std::string> out_ngram2;
	NGramSet				 ngram_set;
	NGramSet::const_iterator iter0;

	//find the reasonable max_size
	Ngram(name1, 4, out_ngram1);
	Ngram(name2, 4, out_ngram2);
	
	for(size_t i = 0; i < out_ngram1.size(); i++)
	{
		ngram_set.insert(out_ngram1[i]);
	}

	for(size_t i = 0; i < out_ngram2.size(); i++)
	{
		iter0 = ngram_set.find(out_ngram2[i]);	
		if(iter0 != ngram_set.end())
		{
			matched_length += out_ngram2[i].length();
		}
	}

	double similarity = matched_length / std::min(name1.length(), name2.length());
	return similarity >= kThreshold ? true : false;
}

//pre-process training for address-normalization
int TuanProcessor::InitAddressModule(const std::string &knowledge_path)
{
	if(!address_extractor_.Train(knowledge_path)) {
		LOG(INFO) << ">>Train address normalization failed.";
		return 1;
	}
	is_init_ = true;
	return 0;
}

//split the shop collection into different samll parts using <city + merchant-area> as key
int TuanProcessor::PreprocessShopSystem(const std::string& scd_path)
{
	static const std::string kDocID    = "DOCID";
	static const std::string kAddress  = "Address";
	static const std::string kShopName = "ShopName";
	static const std::string kCity     = "City";
	static const std::string kArea     = "Shangquan";

    namespace bfs = boost::filesystem;
    std::vector<std::string> scd_list;
    B5MHelper::GetIUScdList(scd_path, scd_list);
    if(scd_list.empty()) {
		LOG(INFO) << ">>shop system scd path is empty.";
		return 1;
	}

	for(size_t i = 0; i < scd_list.size(); i++)
	{
		LOG(INFO) << ">>preprocess scd file:" << scd_list[i];
		ScdParser parser(izenelib::util::UString::UTF_8);
		parser.load(scd_list[i]);
		size_t doc_count = 0;
		for(ScdParser::iterator doc_iter = parser.begin(); 
				doc_iter!= parser.end(); ++doc_iter, ++doc_count)
		{
			if(doc_count % 10000 == 0){
				LOG(INFO)<<"preprocess shop collection doc count:" << doc_count;
			}
			SCDDoc& scddoc = *(*doc_iter);
			SCDDoc::iterator p = scddoc.begin();
			Document doc;
			for(; p != scddoc.end(); ++p)
			{
				const std::string& property_name = p->first;
				doc.property(property_name) = p->second;
			}
			ShopSystemAux shop_info;
			CityAreaPair  city_area_key;
            doc.getString(kDocID, shop_info.docid);
            doc.getString(kAddress, shop_info.address);
            doc.getString(kShopName, shop_info.shop_name);
            doc.getString(kCity, city_area_key.first);
            doc.getString(kArea, city_area_key.second);

			if(city_area_key.first.empty() || city_area_key.second.empty()){
				LOG(INFO) << ">>DOCID: " << shop_info.docid << " lack of city and area."; 
				continue;
			}
			//pre-calculate the address normalization result
			shop_info.address_evaluate = address_extractor_.Evaluate_(shop_info.address);
			//insert to shop system cache
			shop_cache_[city_area_key].push_back(shop_info);
		}
	}
	return 0;
}

//main entrance of shop matching algorithms
//we should call this function after running common clustering
void TuanProcessor::MatchShop()
{
	static const std::string kMerchantArea    = "MerchantArea";
	static const std::string kMerchantAddress = "MainAddr";
	static const std::string kCity			  = "City";
	static const std::string kShopName		  = "MerchantName";
	static const std::string kDOCID		      = "DOCID";
	static const std::string kShopIds         = "ShopIds";

    for(CacheType::iterator iter0 = cache_.begin(); iter0 != cache_.end(); ++iter0)
    {
		//notice : new tuan scd data store the mapping area--->address,
		//if address has no mapping area, it store like this ,,
		//it means that address and area split result count is euqal
		std::string address;
		std::string area;
		std::string city;
		std::string docid;
		std::string merchant_name;
		std::string shopids;
		Document &doc = iter0->second.doc;
        doc.getString(kMerchantAddress, address);
        doc.getString(kMerchantArea, area);
        doc.getString(kCity, city);
        doc.getString(kDOCID, docid);
		doc.getString(kShopName, merchant_name);

        std::vector<std::string> area_array;
        std::vector<std::string> address_array;
		boost::algorithm::split(area_array, area, boost::is_any_of(",;"));
		boost::algorithm::split(address_array, address, boost::is_any_of(",;"));
		if(area_array.size() != address_array.size()) {
			LOG(WARNING) << ">>scd format error: {area_array.size() != address_array.size()} docid : " << docid;
			continue;
		}
		
		assert(address_array.size() == area_array.size());
		for(size_t i = 0; i < address_array.size(); i++)
		{
			bool is_matched = false;
			AddressExtract::EvaluateResult address_evaluate = address_extractor_.Evaluate_(address_array[i]);
			CityAreaPair city_area_key = std::make_pair(city, area_array[i]);
			ShopSystemMap::iterator iter1 = shop_cache_.find(city_area_key);
			if(iter1 != shop_cache_.end())
			{
				std::vector<ShopSystemAux> &shop_collection = iter1->second;
				for(size_t j = 0; j < shop_collection.size(); j++)
				{
					double similarity = address_extractor_.Similarity_(
						shop_collection[j].address_evaluate.second, address_evaluate.second);

					if(similarity >= AddressExtract::kChineseThreshold)
					{
						LOG(INFO) << ">>find similar address: " << 
							address_array[i] << "{" << docid << "}" << "||" << 
								shop_collection[j].address << "{" << shop_collection[j].docid << "}\n";
						//match the shop address success
						//next step match the shop name
						//using vector model caculate the similarity
						if(true == JudgeSimilar_v2(merchant_name, shop_collection[j].shop_name))
						{
							//add "shopids" property associate with shop and tuan collection
							//format like this docid1,docid4,,,,docid100, if the no-matching we
							//should using ",," , it means that if we using boost::split algorithms
							//split the "shopids" the array size must be equal to size of address array. 
							LOG(INFO) << ">>find similar merchant name: " <<
								merchant_name << "{" << docid << "}" << "||" << 
									shop_collection[j].shop_name << "{" << shop_collection[j].docid << "}\n";

							BuildShopids(shopids, shop_collection[j].docid);
							//store the matched tuan product docid in shop collection
							//shop_collection[j].tuan_docid = docid;
							
							//hash shop docid and matched tuan docid
							shop_tuan_map_[shop_collection[j].docid] = docid; 
							is_matched = true;
							break;							
						}
					} 
				}
			}
			
			//build shopids property for each shop matching
			if (!is_matched) {
				BuildShopids(shopids, "");
			}
		}
		//notice erase last ","
		//add new property "shopids"
		shopids.erase(shopids.end() - 1);
        doc.property(kShopIds) = shopids;

#ifdef _DEBUG
        std::vector<std::string> shopids_array;
		boost::algorithm::split(shopids_array, shopids, boost::is_any_of(","));
		assert(shopids_array.size() == address_array.size());
#endif
    }
}

//add by wangbaobao@b5m.com --------------end--------------------------------
//////////////////////////////////////////////////////////////////////////////
bool TuanProcessor::Generate(const std::string& mdb_instance)
{
    SetCmaPath(b5mm_.cma_path);
    const std::string& scd_path = b5mm_.scd_path;
    namespace bfs = boost::filesystem;
    std::vector<std::string> scd_list;
    B5MHelper::GetIUScdList(scd_path, scd_list);
    if(scd_list.empty()) return false;
    typedef boost::unordered_map<std::string, std::string> MatchResult;
    MatchResult match_result;
    bool use_clustering = true;
    std::string work_dir = mdb_instance+"/work_dir";

    {
        ProductTermAnalyzer analyzer(cma_path_);
        B5MHelper::PrepareEmptyDir(work_dir);

        std::string dd_container = work_dir +"/dd_container";
        std::string group_table_file = work_dir +"/group_table";
        GroupTableType group_table(group_table_file);
        group_table.Load();

        DDType dd(dd_container, &group_table);
        if(use_clustering)
        {
            dd.SetFixK(12);
        }
        if(!dd.Open())
        {
            std::cout<<"DD open failed"<<std::endl;
            return false;
        }

        for(uint32_t i=0;i<scd_list.size();i++)
        {
            std::string scd_file = scd_list[i];
            LOG(INFO)<<"DD Processing "<<scd_file<<std::endl;
            ScdParser parser(izenelib::util::UString::UTF_8);
            parser.load(scd_file);
            //int scd_type = ScdParser::checkSCDType(scd_file);
            uint32_t n=0;
            for( ScdParser::iterator doc_iter = parser.begin();
              doc_iter!= parser.end(); ++doc_iter, ++n)
            {
                if(n%10000==0)
                {
                    LOG(INFO)<<"Find Documents A "<<n<<std::endl;
                }
                SCDDoc& scddoc = *(*doc_iter);
                SCDDoc::iterator p = scddoc.begin();
                Document doc;
                for(; p!=scddoc.end(); ++p)
                {
                    const std::string& property_name = p->first;
                    doc.property(property_name) = p->second;
                }
                std::string soid;
                std::string category;
                std::string city;
                std::string title;
                std::string sprice;
                std::string address;
                std::string area;
                doc.getString("DOCID", soid);
                doc.getString("Category", category);
                doc.getString("City", city);
                doc.getString("Title", title);
                doc.getString("MerchantAddr", address);
                doc.getString("MerchantArea", area);
                if(category.empty()||city.empty()||title.empty()||area.empty())
                {
                    continue;
                }
                doc.getString("Price", sprice);
                ProductPrice pprice;
                pprice.Parse(sprice);
                const DocIdType& id = soid;

                TuanProcessorAttach attach;
                if(pprice.Positive()) attach.price = pprice;
                UString sid_str = UString(category, UString::UTF_8);
                sid_str.append(UString("|", UString::UTF_8));
                sid_str.append(UString(city, UString::UTF_8));
                std::vector<std::string> area_array;
                boost::algorithm::split(area_array, area, boost::is_any_of(",;"));
                if(area_array.size()!=1) continue;
                sid_str.append(UString("|", UString::UTF_8));
                sid_str.append(UString(area_array.front(), UString::UTF_8));
                attach.sid = izenelib::util::HashFunction<izenelib::util::UString>::generateHash32(sid_str);
                //std::sort(area_array.begin(), area_array.end());
                std::string text = title;
                //if(!address.empty()) text+="\t"+address;
                

                std::vector<std::pair<std::string, double> > doc_vector;
                analyzer.Analyze(UString(text, UString::UTF_8), doc_vector);

                if( doc_vector.empty() )
                {
                    continue;
                }
                dd.InsertDoc(id, doc_vector, attach);
            }
        }
        dd.RunDdAnalysis();
        const std::vector<std::vector<std::string> >& group_info = group_table.GetGroupInfo();
        for(uint32_t gid=0;gid<group_info.size();gid++)
        {
            const std::vector<std::string>& in_group = group_info[gid];
            if(in_group.empty()) continue;
            std::string pid = in_group[0];
            for(uint32_t i=0;i<in_group.size();i++)
            {
                match_result[in_group[i]] = pid;
            }
        }
    }

    LOG(INFO)<<"match result size "<<match_result.size()<<std::endl;
    typedef boost::unordered_map<std::string, std::vector<Document> > SimhashGroups;
    SimhashGroups simhash_groups;

    for(uint32_t i=0;i<scd_list.size();i++)
    {
        std::string scd_file = scd_list[i];
        LOG(INFO)<<"Processing "<<scd_file<<std::endl;
        ScdParser parser(izenelib::util::UString::UTF_8);
        parser.load(scd_file);
        //int scd_type = ScdParser::checkSCDType(scd_file);
        uint32_t n=0;
        for( ScdParser::iterator doc_iter = parser.begin();
          doc_iter!= parser.end(); ++doc_iter, ++n)
        {
            if(n%10000==0)
            {
                LOG(INFO)<<"Find Documents B "<<n<<std::endl;
            }
            Document doc;
            SCDDoc& scddoc = *(*doc_iter);
            SCDDoc::iterator p = scddoc.begin();
            for(; p!=scddoc.end(); ++p)
            {
                const std::string& property_name = p->first;
                doc.property(property_name) = p->second;
            }
            Document::doc_prop_value_strtype oid;
            std::string soid;
            doc.getProperty("DOCID", oid);
            soid = propstr_to_str(oid);
            std::string spid;
            MatchResult::const_iterator it = match_result.find(soid);
            if(it==match_result.end())
            {
                spid = soid;
            }
            else
            {
                spid = it->second;
                if(use_clustering)
                {
                    std::vector<Document>& docs = simhash_groups[spid];
                    //if(docs.size()>=1000)
                    //{
                    //    std::cerr<<"[XX]"<<spid<<","<<docs.size()<<std::endl;
                    //}
                    if(docs.size()<2000)
                    {
                        docs.push_back(doc);
                    }
                }
            }
        }
    }
    LOG(INFO)<<"state B finished"<<std::endl;
    if(use_clustering)
    {
        idmlib::util::IDMAnalyzerConfig aconfig = idmlib::util::IDMAnalyzerConfig::GetCommonConfig("","","");
        idmlib::util::IDMAnalyzer* analyzer = new idmlib::util::IDMAnalyzer(aconfig);
        std::size_t max_cluster_size = 0;
        LOG(INFO)<<"group size "<<simhash_groups.size()<<std::endl;
        std::size_t p=0;
        for(SimhashGroups::const_iterator it=simhash_groups.begin();it!=simhash_groups.end();++it)
        {
            if(p%1000==0)
            {
                LOG(INFO)<<"processing group "<<p<<std::endl;
            }
            ++p;
            const std::vector<Document>& docs = it->second;
            //if(docs.size()<50) continue;
            //std::cerr<<"clustering start, size "<<docs.size()<<std::endl;
            TuanClustering clustering(analyzer);
            for(std::size_t i=0;i<docs.size();i++)
            {
                clustering.InsertDoc(docs[i]);
            }
            clustering.Build(match_result);
            if(clustering.MaxClusterSize()>max_cluster_size)
            {
                max_cluster_size = clustering.MaxClusterSize();
            }
        }
        delete analyzer;
        LOG(INFO)<<"Max Cluster Size: "<<max_cluster_size<<std::endl;
    }

	//cache the b5mo/b5mp SCD file locally 
    const std::string& b5mo_hdfs_path = b5mm_.b5mo_path;
    B5MHelper::PrepareEmptyDir(b5mo_hdfs_path);
	std::string b5mo_path = work_dir + "/b5mo";
    B5MHelper::PrepareEmptyDir(b5mo_path);

    ScdWriter writer(b5mo_path, UPDATE_SCD);
	for(uint32_t i=0;i<scd_list.size();i++)
    {
        std::string scd_file = scd_list[i];
        LOG(INFO)<<"Processing "<<scd_file<<std::endl;
        ScdParser parser(izenelib::util::UString::UTF_8);
        parser.load(scd_file);
        //int scd_type = ScdParser::checkSCDType(scd_file);
        uint32_t n=0;
        for( ScdParser::iterator doc_iter = parser.begin();
          doc_iter!= parser.end(); ++doc_iter, ++n)
        {
            if(n%10000==0)
            {
                LOG(INFO)<<"Find Documents C "<<n<<std::endl;
            }
            Document doc;
            SCDDoc& scddoc = *(*doc_iter);
            SCDDoc::iterator p = scddoc.begin();
            for(; p!=scddoc.end(); ++p)
            {
                const std::string& property_name = p->first;
                doc.property(property_name) = p->second;
            }
            Document::doc_prop_value_strtype oid;
            std::string soid;
            doc.getProperty("DOCID", oid);
            soid = propstr_to_str(oid);
            std::string spid;
            MatchResult::const_iterator it = match_result.find(soid);
            if(it==match_result.end())
            {
                spid = soid;
            }
            else
            {
                spid = it->second;
            }
            doc.property("uuid") = str_to_propstr(spid, UString::UTF_8);
            writer.Append(doc);
        }
    }
    writer.Close();

    PairwiseScdMerger merger(b5mo_path);
    std::size_t odoc_count = ScdParser::getScdDocCount(b5mo_path);
    LOG(INFO)<<"tuan o doc count "<<odoc_count<<std::endl;
    uint32_t m = odoc_count/2400000+1;
    merger.SetM(m);
    merger.SetMProperty("uuid");
    merger.SetOutputer(boost::bind( &TuanProcessor::B5moOutput_, this, _1, _2));
    merger.SetMEnd(boost::bind( &TuanProcessor::POutputAll_, this));

    const std::string& b5mp_hdfs_path = b5mm_.b5mp_path;
    B5MHelper::PrepareEmptyDir(b5mp_hdfs_path);
	std::string b5mp_path = work_dir + "/b5mp";
    B5MHelper::PrepareEmptyDir(b5mp_path);
    pwriter_.reset(new ScdWriter(b5mp_path, UPDATE_SCD));
    merger.Run();
    pwriter_->Close();

	//copy b5mo/b5mp to hdfs and remove local file
	//remove unused file
	LOG(INFO) << "copy and remove file.";
	B5MHelper::CopyFile(b5mo_path, b5mo_hdfs_path);
	B5MHelper::CopyFile(b5mp_path, b5mp_hdfs_path);
	B5MHelper::RemoveDir(b5mo_path);
	B5MHelper::RemoveDir(b5mp_path);
    return true;
}

void TuanProcessor::POutputAll_()
{
    for(CacheType::iterator it = cache_.begin();it!=cache_.end();it++)
    {
        SValueType& svalue = it->second;
        svalue.doc.eraseProperty("OID");
        pwriter_->Append(svalue.doc);
    }
    cache_.clear();
}

void TuanProcessor::B5moOutput_(SValueType& value, int status)
{
    uint128_t pid = GetPid_(value.doc);
    if(pid==0)
    {
        return;
    }
    SValueType& svalue = cache_[pid];
    ProductMerge_(svalue, value);
}

uint128_t TuanProcessor::GetPid_(const Document& doc)
{
    std::string spid;
    doc.getString("uuid", spid);
    if(spid.empty()) return 0;
    return B5MHelper::StringToUint128(spid);
}

uint128_t TuanProcessor::GetOid_(const Document& doc)
{
    std::string soid;
    doc.getString("DOCID", soid);
    if(soid.empty()) return 0;
    return B5MHelper::StringToUint128(soid);
}

void TuanProcessor::ProductMerge_(SValueType& value, const SValueType& another_value)
{
    //value is pdoc or empty, another_value is odoc
    ProductProperty pp;
    if(!value.empty())
    {
        pp.Parse(value.doc);
    }
    ProductProperty another;
    another.Parse(another_value.doc);
    pp += another;
    if(value.empty() || another.oid==another.productid)
    {
        value.doc.copyPropertiesFromDocument(another_value.doc, true);
    }
    else
    {
        const PropertyValue& docid_value = value.doc.property("DOCID");
        //override if empty property
        for (Document::property_const_iterator it = another_value.doc.propertyBegin(); it != another_value.doc.propertyEnd(); ++it)
        {
            if (!value.doc.hasProperty(it->first))
            {
                value.doc.property(it->first) = it->second;
            }
            else
            {
                PropertyValue& pvalue = value.doc.property(it->first);
                if(pvalue.which()==docid_value.which()) //is UString
                {
                    const PropertyValue::PropertyValueStrType& uvalue = pvalue.getPropertyStrValue();
                    if(uvalue.empty())
                    {
                        pvalue = it->second;
                    }
                }
            }
        }
    }
    value.type = UPDATE_SCD;
    pp.Set(value.doc);
}
