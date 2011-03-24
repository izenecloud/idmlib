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
#include <idmlib/semantic_space/document_vector_space.h>
#include <idmlib/semantic_space/explicit_semantic_interpreter.h>
#include <idmlib/util/collection_file_util.h>
#include <idmlib/util/FSUtil.hpp>

using namespace idmlib::ssp;

NS_IDMLIB_SIM_BEGIN

class DocumentSimilarity
{
public:
	DocumentSimilarity(const std::string& colBasePath)
	: colFileUtil_(new idmlib::util::CollectionFileUtil(colBasePath))
	{
		// 1. build doc vec space
		//pDocVecSpace_  =

		// 2. build doc-concept inverted index
		// for docVec in collection
		//     interVec = interpret(docVec)
		//     addtoInvertedIndex(interVec) //

		// 3. compute doc similarity increamently
	}

	DocumentSimilarity(boost::shared_ptr<DocumentVectorSpace>& pDocVecSpace)
	: pDocVecSpace_(pDocVecSpace)
	{

	}

	// todo, remove
	DocumentSimilarity(
			const std::string& colPath,
			const boost::shared_ptr<SemanticInterpreter> pSSPInter)
	: colBasePath_(colPath)
	, pSSPInter_(pSSPInter)
	{
		idmlib::util::FSUtil::normalizeFilePath(colBasePath_);
		encoding_ = izenelib::util::UString::UTF_8;

		scdPath_ = colBasePath_ + "/scd/index";

		std::string idDir = colBasePath_ + "/collection-data/default-collection-dir/id/";
		pIdManager_.reset(new IDManager(idDir));
	}

	~DocumentSimilarity()
	{
	}

public:
	/**
	 * @brief Compute similarity for all pair of documents & build index
	 */
	bool ComputeAll();

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
	boost::shared_ptr<idmlib::util::CollectionFileUtil> colFileUtil_;
	std::string colBasePath_;
	std::string scdPath_;
	izenelib::util::UString::EncodingType encoding_;
	boost::shared_ptr<DocumentVectorSpace> pDocVecSpace_;

	///
	boost::shared_ptr<SemanticInterpreter> pSSPInter_;
	boost::shared_ptr<IDManager > pIdManager_;

	typedef std::map<termid_t, count_t> termid_df_map;
	termid_df_map termid2DF_;

	typedef std::vector< std::vector<weight_t> > docIVecsT;
	docIVecsT docIVecs_;

	docid_index_map docid2Index_;
	///
};

NS_IDMLIB_SIM_END

#endif /* DOCUMENT_SIMILARITY_H_ */
