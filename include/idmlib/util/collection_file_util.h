/**
 * @file collection_data_base.h
 * @author Zhongxia Li
 * @date Mar 23, 2011
 * @brief 
 */
#ifndef COLLECTION_FILE_UTIL_H_
#define COLLECTION_FILE_UTIL_H_

#include <boost/filesystem.hpp>

#include <idmlib/idm_types.h>
#include <idmlib/util/FSUtil.hpp>

using namespace boost::filesystem;

NS_IDMLIB_UTIL_BEGIN

class CollectionFileUtil
{
public:
	CollectionFileUtil(const std::string& basePath)
	: basePath_(basePath)
	{
		idmlib::util::FSUtil::normalizeFilePath(basePath_);
		// check path
		if ( !exists(basePath_) ) {
			std::cout << "Error: collection data path dose not existed: " << basePath_ << std::endl;
			return;
		}
		if ( !is_directory(basePath_) ) {
			std::cout << "Error: collection data path should be a directory: " <<basePath_ << std::endl;
			return;
		}

		scdPath_ = basePath_ + "/scd/index";
		colDataPath_ = basePath_ + "/collection-data/default-collection-dir";
		queryDataPath_ = basePath_ + "/query-data";
	}

	std::string& getBaseDir()
	{
		return basePath_;
	}

	std::string& getScdDir()
	{
		return scdPath_;
	}

	std::string& getCollectionDataDir()
	{
		return colDataPath_;
	}

	std::string& getQueryDataDir()
	{
		return queryDataPath_;
	}

private:
	std::string basePath_;
	std::string scdPath_;
	std::string colDataPath_;
	std::string queryDataPath_;
};

NS_IDMLIB_UTIL_END

#endif /* COLLECTION_DATA_BASE_H_ */
