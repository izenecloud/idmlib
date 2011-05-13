/**
 * @file izene_index_helper.h
 * @author Zhongxia Li
 * @date May 13, 2011
 * @brief 
 */
#ifndef IZENE_INDEX_HELPER_H_
#define IZENE_INDEX_HELPER_H_

#include <idmlib/idm_types.h>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/utility/StringUtils.h>
#include <ir/index_manager/utility/IndexManagerConfig.h>

using namespace izenelib::ir::indexmanager;

NS_IDMLIB_SSP_BEGIN

class IzeneIndexHelper
{
public:
    static boost::shared_ptr<Indexer> createIndexer(std::string& indexDir, std::string collectionName = "ChnWiki")
    {
        boost::shared_ptr<Indexer> indexer(new Indexer());

        if (indexer)
        {
            IndexManagerConfig config;
            setIndexConfig(config, collectionName, indexDir);

            std::map<std::string, unsigned int> collectionIdMapping;
            collectionIdMapping[collectionName] = 1;

            indexer->setIndexManagerConfig(config, collectionIdMapping);
        }

        return indexer;
    }

private:
    static void setIndexConfig(IndexManagerConfig& config, std::string& collectionName, std::string& indexDir)
    {
        // set as default configuration
        boost::filesystem::create_directories(indexDir);
        config.indexStrategy_.indexLocation_ = indexDir;
        config.indexStrategy_.memory_ = 128000000;
        config.indexStrategy_.indexDocLength_ = true;
        config.indexStrategy_.skipInterval_ = 8;
        config.indexStrategy_.maxSkipLevel_ = 3;
        config.indexStrategy_.indexMode_ = "default";
        config.storeStrategy_.param_ = "file";
        config.mergeStrategy_.param_ = "dbt";
        config.indexStrategy_.indexLocations_.push_back("default-collection-dir");

        IndexerCollectionMeta indexCollectionMeta;
        indexCollectionMeta.setName(collectionName);
        indexCollectionMeta.addPropertyConfig( makeIndexPropertyConfig(1, "DOCID", false, false) );
        indexCollectionMeta.addPropertyConfig( makeIndexPropertyConfig(2, "DATE", true, false) );
        indexCollectionMeta.addPropertyConfig( makeIndexPropertyConfig(3, "Title", true, true) );
        indexCollectionMeta.addPropertyConfig( makeIndexPropertyConfig(4, "Content", true, true) );
        config.addCollectionMeta(indexCollectionMeta);
    }

    static IndexerPropertyConfig makeIndexPropertyConfig(unsigned int propertyid, std::string propertyname, bool index, bool analyzed)
    {
        IndexerPropertyConfig indexerPropertyConfig(propertyid, propertyname, index, analyzed);
        indexerPropertyConfig.setIsFilter(false);
        indexerPropertyConfig.setIsMultiValue(false);
        indexerPropertyConfig.setIsStoreDocLen(true);

        return indexerPropertyConfig;
    }
};

NS_IDMLIB_SSP_END

#endif /* IZENE_INDEX_HELPER_H_ */
