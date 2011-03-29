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
#include <idmlib/util/idm_analyzer.h>
//#include <idmlib/keyphrase-extraction/kpe_algorithm.h>
//#include <idmlib/keyphrase-extraction/kpe_output.h>
#include <la/LA.h>
#include <la/util/UStringUtil.h>
#include <ir/id_manager/IDManager.h>
#include <ir/index_manager/index/LAInput.h>
// prevent conflicting with ScdParser when referred by sf1-revolution
#ifndef __SCD__PARSER__H__
#define __SCD__PARSER__H__
#include <util/scd_parser.h>
#endif

using namespace boost::filesystem;
using namespace izenelib::ir::idmanager;

NS_IDMLIB_SSP_BEGIN


class SemanticSpaceBuilder
{
	typedef std::vector<std::pair<izenelib::util::UString, izenelib::util::UString> >::iterator doc_properties_iterator;

public:
	SemanticSpaceBuilder(
			boost::shared_ptr<SemanticSpace>& pSSpace,
			const std::string& laResPath,
			const std::string& colBasePath,
			docid_t maxDoc = MAX_DOC_ID,
			izenelib::util::UString::EncodingType encoding = izenelib::util::UString::UTF_8)
	: pSSpace_(pSSpace)
	, colBasePath_(colBasePath)
	, maxDoc_(maxDoc)
	, encoding_(encoding)
	{
		normalizeFilePath(colBasePath_);
		scdPath_ = colBasePath_ + "/scd/index";
		colDataPath_ = colBasePath_ + "/collection-data/default-collection-dir";

		createIdManager(pIdManager_);
		if (!pIdManager_) {
			DLOG(ERROR) << "Failed to create IdManager!" << std::endl;
		}

		pIdmAnalyzer_.reset(
				new idmlib::util::IDMAnalyzer(
						laResPath,
						la::ChineseAnalyzer::minimum_match_no_overlap)
		);
	}

	virtual ~SemanticSpaceBuilder()
	{
	}

public:
	bool Build();

	boost::shared_ptr<SemanticSpace>& getSemanticSpace()
	{
		return pSSpace_;
	}

	//bool getDocTerms(const izenelib::util::UString& ustrDoc, term_vector& termVec);

	bool getDocTermids(const izenelib::util::UString& ustrDoc, std::vector<termid_t>& termids)
	{
		//termIdList_.clear();
		//pLA_->process(pIdManager_.get(), ustrDoc, termIdList_);

		termList_.clear();
		//pLA_->process(ustrDoc, termList_);
		pIdmAnalyzer_->GetTermList(ustrDoc, termList_, false);

		termid_t termid;
		for ( la::TermList::iterator iter = termList_.begin(); iter != termList_.end(); iter++ )
		{
			//if ( filter(iter->text_) )
			//	continue;
			pIdManager_->getTermIdByTermString(iter->text_, termid);
			termids.push_back(termid);

//			cout << iter->textString()  << "(" << termid << ") "; //
		}

//		cout << " ---- term count: " << termids.size() << endl;

		return true;
	}

private:
	bool createIdManager(boost::shared_ptr<IDManager>& pIdManager)
	{
	    std::string dir = colDataPath_ + "/id/";
	    if (!exists(dir)) {
	    	return false;
	    }

	    pIdManager.reset( new IDManager(dir) );
	}

private:
	/**
	 * @brief get scd files in current directory
	 */
	bool getScdFileList(const std::string& scdDir, std::vector<std::string>& fileList);

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



protected:
	// collection to be built
	std::string colBasePath_;
	std::string scdPath_;
	std::string colDataPath_;
	docid_t maxDoc_;
	izenelib::util::UString::EncodingType encoding_;
	// semantic space
	boost::shared_ptr<SemanticSpace> pSSpace_;
	// gather info
	TermIdList termIdList_;
	la::TermList termList_;
	// LA
	boost::shared_ptr<idmlib::util::IDMAnalyzer> pIdmAnalyzer_;
	// id
	boost::shared_ptr<IDManager> pIdManager_;
};

//class KpeSemanticSpaceBuilder : public SemanticSpaceBuilder
//{
//	typedef idmlib::kpe::KPEOutput<true, true, true> OutputType ;
//	typedef OutputType::function_type function_type ;
//public:
//	typedef idmlib::kpe::KPEAlgorithm<OutputType> kpe_type;
//
//public:
//	KpeSemanticSpaceBuilder(
//			const std::string& collectionPath,
//			boost::shared_ptr<SemanticSpace>& pSSpace )
//	: SemanticSpaceBuilder(pSSpace, "", collectionPath)
//	{
//	}
//
//protected:
//	bool getDocTerms(const izenelib::util::UString& ustrDoc, term_vector& termVec);
//
//private:
//	bool initKpe();
//
//private:
//	// key-phrase extraction
//	kpe_type* kpe_;
//    function_type callback_func_;
//    kpe_type::ScorerType* scorer_;
//
//};

NS_IDMLIB_SSP_END

#endif /* SEMANTIC_SPACE_BUILDER_H_ */
