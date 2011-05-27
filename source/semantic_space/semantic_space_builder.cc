#include <idmlib/semantic_space/semantic_space_builder.h>

#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/IndexReader.h>

using namespace idmlib::ssp;
using namespace izenelib::util;
using namespace izenelib::ir::indexmanager;

bool SemanticSpaceBuilder::Build()
{
#ifdef SSP_TIME_CHECKER
    idmlib::util::TimeChecker timer("Build wiki index");
#endif

	std::vector<std::string> scdFileList;
	if (!getScdFileList(scdPath_, scdFileList)) {
		return false;
	}

	docid_t docid = 0;
	docid_t last_docid = 0;
	docid_t doc_count = 0;

	// process all SCD files
	for (size_t i = 0; i < scdFileList.size(); i++)
	{
		//std::cout << scdFileList[i] << std::endl;
		std::string scdFile = scdFileList[i];
		izenelib::util::ScdParser scdParser(encoding_);
		if(!scdParser.load(scdFile) )
		{
		  DLOG(WARNING) << "Load scd file failed: " << scdFile << std::endl;
		  return false;
		}

		/// test
		std::vector<izenelib::util::UString> list;
		scdParser.getDocIdList(list);
		size_t totalDocNum = list.size();

		// parse SCD
		izenelib::util::ScdParser::iterator iter = scdParser.begin();
		for ( ; iter != scdParser.end(); iter ++)
		{
			izenelib::util::SCDDocPtr pDoc = *iter;

			doc_properties_iterator proIter;
			for (proIter = pDoc->begin(); proIter != pDoc->end(); proIter ++)
			{
				izenelib::util::UString propertyName = proIter->first;
				const izenelib::util::UString & propertyValue = proIter->second;
				propertyName.toLowerString();
				if ( propertyName == izenelib::util::UString("docid", encoding_) ) {
					// collection should be indexed (using index data)
					last_docid = docid;
					bool ret = pIdManager_->getDocIdByDocName(propertyValue, docid, false);
					if (!ret) {
						DLOG(WARNING) << "Document (" << propertyValue << ") does not existed: !" << std::endl;
						return false;
					}
//					std::cout << "docid: " << la::to_utf8(propertyValue) << " => " << docid << endl;
				}
				else if ( propertyName == izenelib::util::UString("title", encoding_) ) {
					//std::cout << la::to_utf8(proIter->second) << std::endl;
				}
				else if ( propertyName == izenelib::util::UString("content", encoding_)) {
					//TimeChecker timer("sspbuilder LA");
					termIdList_.clear();
					//idmTermList.clear();
					getDocTermIdList(propertyValue, termIdList_/*, idmTermList*/);
				}
                else if ( propertyName == izenelib::util::UString("unit", encoding_)) { // cnki test
                	termIdList_.clear();
                    getDocTermIdList(propertyValue, termIdList_/*, idmTermList*/);
                }
			}

			// check docid
			if (docid == last_docid) {
				DLOG(WARNING) << "Duplicated docid: " << docid << "(Check SCD properties!)" << std::endl;
				return false;
			}

			pSSpace_->ProcessDocument(docid, termIdList_/*, idmTermList*/);

			doc_count++;

            if ((doc_count) % 2000 == 0) {
                DLOG(INFO) << " total scd file(s) " << scdFileList.size() << " - processing " << i+1
                        << " [" << doc_count << ", total " << totalDocNum << ", max " << maxDoc_<< "] - "
                        << doc_count*100.0f / totalDocNum << "%" << endl;
            }

            if ((doc_count) >= maxDoc_)
                break;
		}

		if ( doc_count >= maxDoc_ )
			break;
	}
	pSSpace_->SaveSpace(); //
	std::cout << "Post-processing Space ... (" << doc_count << " documents processed(inserted) )"  << std::endl;

#ifdef SSP_TIME_CHECKER
	idmlib::util::TimeChecker timer("Process Space");
#endif
	pSSpace_->ProcessSpace();
	return true;
}

bool SemanticSpaceBuilder::BuildWikiSource()
{
    std::vector<std::string> scdFileList;
    if (!getScdFileList(scdPath_, scdFileList)) {
        return false;
    }

    // parsing all SCD files
    for (size_t i = 0; i < scdFileList.size(); i++)
    {
        std::string scdFile = scdFileList[i];
        izenelib::util::ScdParser scdParser(encoding_);
        if(!scdParser.load(scdFile) )
        {
          DLOG(WARNING) << "Load scd file failed: " << scdFile << std::endl;
          return false;
        }

        /// test
        std::vector<izenelib::util::UString> list;
        scdParser.getDocIdList(list);
        size_t totalDocNum = list.size();
        size_t curDocNum = 0;
        size_t indexDocNum = 0;

        // parse SCD
        izenelib::util::ScdParser::iterator iter = scdParser.begin();
        for ( ; iter != scdParser.end(); iter ++)
        {
            SCDDocPtr pDoc = *iter;
            IndexerDocument indexDocument;

            bool ret = prepareDocument_(pDoc, indexDocument);

            if ( !ret)
                continue;

            pIndexer_->insertDocument(indexDocument);

            curDocNum ++;
            indexDocNum = pIndexer_->getIndexReader()->numDocs();

            if (curDocNum % 1000 == 0) {
                DLOG(INFO) << "["<<scdFile<<"] processing: " << curDocNum<<" = "<<indexDocNum<<" / total: "<<totalDocNum
                           << " - "<< curDocNum*100.0f / totalDocNum << "%" << endl;
            }
        }
    }

    return true;
}

/// Private ////////////////////////////////////////////////////////////////////

bool SemanticSpaceBuilder::prepareDocument_(SCDDocPtr& pDoc, IndexerDocument& indexDocument)
{
    docid_t docId = 0;

    doc_properties_iterator proIter;
    for (proIter = pDoc->begin(); proIter != pDoc->end(); proIter ++)
    {
        izenelib::util::UString propertyName = proIter->first;
        const izenelib::util::UString & propertyValue = proIter->second;
        propertyName.toLowerString();

        // see IzeneIndexHelper::setIndexConfig()
        IndexerPropertyConfig indexerPropertyConfig;
        indexerPropertyConfig.setIsMultiValue(false);

        if (propertyName == izenelib::util::UString("docid", encoding_))
        {
            indexerPropertyConfig.setPropertyId(idmlib::ssp::IzeneIndexHelper::getPropertyIdByName("DOCID"));
            indexerPropertyConfig.setName("DOCID");
            indexerPropertyConfig.setIsIndex(false);
            indexerPropertyConfig.setIsFilter(false);
            indexerPropertyConfig.setIsAnalyzed(false);
            indexerPropertyConfig.setIsStoreDocLen(false);

            bool ret = pIdManager_->getDocIdByDocName(propertyValue, docId, false);
            if (ret)  /*exist*/;
            indexDocument.setId(0);
            indexDocument.setDocId(docId, idmlib::ssp::IzeneIndexHelper::COLLECTION_ID);
        }
        else if (propertyName == izenelib::util::UString("date", encoding_) )
        {
            indexerPropertyConfig.setPropertyId(idmlib::ssp::IzeneIndexHelper::getPropertyIdByName("DATE"));
            indexerPropertyConfig.setName("DATE");
            indexerPropertyConfig.setIsIndex(true);
            indexerPropertyConfig.setIsFilter(true);
            indexerPropertyConfig.setIsAnalyzed(false);
            indexerPropertyConfig.setIsStoreDocLen(false);
            struct tm atm;
            int64_t time = mktime(&atm);
            indexDocument.insertProperty(indexerPropertyConfig, time);
        }
        else
        {
            if ( propertyName == izenelib::util::UString("title", encoding_) ) {
                indexerPropertyConfig.setPropertyId(idmlib::ssp::IzeneIndexHelper::getPropertyIdByName("Title"));
                indexerPropertyConfig.setName("Title");
            }
            else if ( propertyName == izenelib::util::UString("content", encoding_)) {
                indexerPropertyConfig.setPropertyId(idmlib::ssp::IzeneIndexHelper::getPropertyIdByName("Content"));
                indexerPropertyConfig.setName("Content");
            }
            else {
                continue;
            }

            // "Title" or "Content"
            indexerPropertyConfig.setIsIndex(true);
            indexerPropertyConfig.setIsAnalyzed(true);
            indexerPropertyConfig.setIsFilter(false);
            indexerPropertyConfig.setIsStoreDocLen(true);

            laInput_->resize(0);
            laInput_->setDocId(docId); // <DOCID> property have to come first in SCD
            getDocTermIdList(propertyValue, *laInput_);
            indexDocument.insertProperty(indexerPropertyConfig, laInput_);
        }
    }

    return true;
}

bool SemanticSpaceBuilder::getScdFileList(const std::string& scdDir, std::vector<std::string>& fileList)
{
	if ( exists(scdDir) )
	{
		if ( !is_directory(scdDir) ) {
			std::cout << "It's not a directory: " << scdDir << std::endl;
			return false;
		}

		directory_iterator iterEnd;
		for (directory_iterator iter(scdDir); iter != iterEnd; iter ++)
		{
			std::string file_name = iter->path().filename();
			//std::cout << file_name << endl;

			if (izenelib::util::ScdParser::checkSCDFormat(file_name) )
			{
				izenelib::util::SCD_TYPE scd_type = izenelib::util::ScdParser::checkSCDType(file_name);
				if( scd_type == izenelib::util::INSERT_SCD ||scd_type == izenelib::util::UPDATE_SCD )
				{
				    cout << "scd file: "<<iter->path().string() << endl;
					fileList.push_back( iter->path().string() );
				}
			}
		}

		if (fileList.size() > 0) {
			return true;
		}
		else {
			std::cout << "There is no scd file in: " << scdDir << std::endl;
			return false;
		}
	}
	else
	{
		std::cout << "File path dose not existed: " << scdDir << std::endl;
		return false;
	}
}
