/**
 * @file semantic_space_builder.h
 * @author Zhongxia Li
 * @date Mar 14, 2011
 * @brief 
 */
#ifndef SEMANTIC_SPACE_BUILDER_H_
#define SEMANTIC_SPACE_BUILDER_H_

#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>
#include <la/util/UStringUtil.h>
// izenelib
#include <util/scd_parser.h>
#include <ir/id_manager/IDManager.h>

using namespace boost::filesystem;
using namespace izenelib::ir::idmanager;

NS_IDMLIB_SSP_BEGIN

class SemanticSpaceBuilder
{
	typedef std::vector<std::pair<izenelib::util::UString, izenelib::util::UString> >::iterator doc_properties_iterator;

public:
	SemanticSpaceBuilder(
			const std::string& collectionPath,
			const std::string& outPath,
			boost::shared_ptr<SemanticSpace>& pSSpace,
			docid_t maxDoc = MAX_DOC_ID)
	: collectionPath_(collectionPath)
	, outPath_(outPath)
	, pSSpace_(pSSpace)
	, maxDoc_(maxDoc)
	{
		normalizeFilePath(collectionPath_);
		normalizeFilePath(outPath_);

		scdPath_ = collectionPath_ + "/scd/index";
		coldataPath_ = collectionPath_ + "/collection-data/default-collection-dir";

		encoding_ = izenelib::util::UString::UTF_8;

		pIdManager_ = createIdManager();
		if (!pIdManager_) {
			DLOG(WARNING) << "Failed to create IdManager!" << std::endl;
		}
	}

	virtual ~SemanticSpaceBuilder()
	{

	}

public:
	bool Build()
	{
		std::vector<std::string> scdFileList;
		if (!getScdFileList(scdPath_, scdFileList)) {
			return false;
		}

	    // doc_terms_map docTermsMap;
	    term_vector termVec;
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

		    // parse SCD
		    izenelib::util::ScdParser::iterator iter = scdParser.begin();
			for ( ; iter != scdParser.end(); iter ++)
			{
				if ( (doc_count++) >= maxDoc_ )
					break;

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
						//std::cout << endl << "docid: " << la::to_utf8(propertyValue) << " => " << docid << endl;
					}
					else if ( propertyName == izenelib::util::UString("title", encoding_) ) {
						//std::cout << la::to_utf8(proIter->second) << std::endl;
					}
					else if ( propertyName == izenelib::util::UString("content", encoding_)) {
						// process raw content
						termVec.clear();
						getDocTerms(propertyValue, termVec);
					}
				}

				// check docid
				if (docid == last_docid) {
					DLOG(WARNING) << "Duplicated docid: " << docid << "(Check SCD properties!)" << std::endl;
					return false;
				}

				// docTermsMap.clear();
				//docTermsMap[docid] = termVec;
				//docTermsMap.insert(make_pair(docid, termVec));
				pSSpace_->processDocument(docid, termVec);
			}

			if ( doc_count >= maxDoc_ )
				break;
		}

		std::cout << "process Space..." << std::endl;
		time_t time1, time2;
		time1 = time (NULL);
		pSSpace_->processSpace();
		time2 = time (NULL);
		std::cout << "time elapsed: " << (time2-time1) << std::endl;

		return true;
	}

	boost::shared_ptr<SemanticSpace>& getSemanticSpace()
	{
		return pSSpace_;
	}

protected:

	virtual bool getDocTerms(const izenelib::util::UString& ustrDoc, term_vector& termVec) = 0;

	boost::shared_ptr<IDManager> createIdManager()
	{
	    std::string dir = coldataPath_ + "/id/";
	    //boost::filesystem::create_directories(dir);

	    boost::shared_ptr<IDManager> pIdManager( new IDManager(dir) );

	    return pIdManager;
	}

protected:
	/**
	 * @brief normalize file path string
	 */
	void normalizeFilePath(std::string& path)
	{
		std::string normalPath;

		std::string::const_iterator iter;
		for (iter = path.begin(); iter != path.end(); iter ++)
		{
			if (*iter == '\\' || *iter == '/') {
				if ( (iter+1) != path.end()) {
					normalPath.push_back('/');
				}
			}
			else {
				normalPath.push_back(*iter);
			}
		}

		path.swap(normalPath);
	}

	bool getScdFileList(const std::string& scdDir, std::vector<std::string>& fileList)
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

protected:
	std::string collectionPath_;
	std::string scdPath_;
	std::string coldataPath_;

	std::string outPath_;
	boost::shared_ptr<SemanticSpace> pSSpace_;
	docid_t maxDoc_;

	izenelib::util::UString::EncodingType encoding_;

	boost::shared_ptr<IDManager > pIdManager_;
};

NS_IDMLIB_SSP_END

#endif /* SEMANTIC_SPACE_BUILDER_H_ */
