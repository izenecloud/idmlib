/**
 * @file document_similarity.h
 * @author Zhongxia Li
 * @date Mar 11, 2011
 * @brief Document Similarity Compute and Index
 */
#ifndef DOCUMENT_SIMILARITY_H_
#define DOCUMENT_SIMILARITY_H_

#include <iostream>
#include <string>

#include <boost/shared_ptr.hpp>

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/semantic_interpreter.h>
#include <idmlib/util/FSUtil.hpp>
//#include <util/ustring/UString.h>

using namespace idmlib::ssp;

NS_IDMLIB_SIM_BEGIN

class DocumentSimilarity
{
public:
	DocumentSimilarity(
			const std::string& colPath,
			const boost::shared_ptr<SemanticInterpreter> pSSPInter)
	: colPath_(colPath)
	, pSSPInter_(pSSPInter)
	{
		idmlib::util::FSUtil::normalizeFilePath(colPath_);
		encoding_ = izenelib::util::UString::UTF_8;

		scdPath_ = colPath_ + "/scd/index";

		std::string idDir = colPath_ + "/collection-data/default-collection-dir/id/";
		pIdManager_.reset(new IDManager(idDir));
	}

	~DocumentSimilarity()
	{
	}

public:
	bool Compute();

	/**
	 * @breif Build Interpretation Vectors for all documents in the collection
	 */
	bool buildInterpretationVectors();

	/**
	 * @breif Compute similarities between all pairs of documents.
	 */
	bool computeSimilarities();

	bool GetSimDocIdList(
			uint32_t docId,
			uint32_t maxNum,
			std::vector<std::pair<uint32_t, float> >& result);

private:
	bool getScdFileListInDir(const std::string& scdDir, std::vector<std::string>& fileList);

private:
	std::string colPath_;
	std::string scdPath_;
	izenelib::util::UString::EncodingType encoding_;
	boost::shared_ptr<SemanticInterpreter> pSSPInter_;
	boost::shared_ptr<IDManager > pIdManager_;

	typedef std::map<termid_t, count_t> termid_df_map;
	termid_df_map termid2DF_;

	typedef std::vector< std::vector<weight_t> > docIVecsT;
	docIVecsT docIVecs_;

	docid_index_map docid2Index_;
};

NS_IDMLIB_SIM_END

#endif /* DOCUMENT_SIMILARITY_H_ */
