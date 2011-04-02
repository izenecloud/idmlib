/**
 * @file idmlib/semantic_space/document_vector_space.h
 * @author Zhongxia Li
 * @date Mar 23, 2011
 * @brief representation of documents which are weighted(tf.idf) vectors, a document can be represented as a sequence of words(terms)
 */

#ifndef DOCUMENT_VECTOR_SPACE_H_
#define DOCUMENT_VECTOR_SPACE_H_

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>

NS_IDMLIB_SSP_BEGIN

class DocumentVectorSpace : public SemanticSpace
{
public:
	DocumentVectorSpace(
			const std::string& sspPath,
			SemanticSpace::eSSPInitType initType = SemanticSpace::CREATE)
	: SemanticSpace(sspPath, initType)
	{
		pDocRepVectors_.reset(new doc_term_matrix_file_oi(sspPath_)); // "sspPath/storage"
		pDocRepVectors_->Open();
	}

	/// @brief Incrementally process every document
	void ProcessDocument(docid_t& docid, std::vector<termid_t>& termids,
	        IdmTermList& termList = NULLTermList)
	{
		docList_.push_back(docid);
		doDocumentProcess(docid, termids);
	}

	/// @brief Post process after all documents are processed
	void ProcessSpace()
	{
		calcWeight();
		SaveSpace();
	}

	void SaveSpace()
	{
		SemanticSpace::SaveSpace();
		pDocRepVectors_->Flush();
	}

	boost::shared_ptr<doc_term_matrix_file_oi>& getDocVectors()
	{
		return pDocRepVectors_;
	}

	void getVectorByDocid(docid_t& docid, term_sp_vector& termsVec)
	{
		pDocRepVectors_->GetVector(docid, termsVec);
	}

public:
	void Print();

private:
	bool doDocumentProcess(docid_t& docid, std::vector<termid_t>& termids);

	void calcWeight();

private:
	static const weight_t thresholdWegt_ = 0.001f;

private:
	// representation vectors of documents
	boost::shared_ptr<doc_term_matrix_file_oi> pDocRepVectors_;

	typedef std::map<termid_t, weight_t> term_doc_tf_map;
};

NS_IDMLIB_SSP_END

#endif /* DOCUMENT_VECTOR_SPACE_H_ */
