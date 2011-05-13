/**
 * @file document_vector_space.cc
 * @author Zhongxia Li
 * @date Mar 23, 2011
 * @brief 
 */

#include <idmlib/semantic_space/document_vector_space.h>

using namespace idmlib::ssp;

void DocumentVectorSpace::Print()
{
	std::cout << "[Document representation vectors]  " << docList_.size() << "  terms: " << termid2df_.size() << std::endl;
	docid_t docid;
	std::vector<docid_t>::iterator docIter;
	for (docIter = docList_.begin(); docIter != docList_.end(); docIter ++)
	{
		docid = *docIter;
		term_sp_vector representDocVec;
		pDocRepVectors_->GetVector(docid, representDocVec);

		stringstream ss;
		ss << "doc:" << docid;
		idmlib::ssp::PrintSparseVec(representDocVec, ss.str());
	}
}

/// Private methods ////////////////////////////////////////////////////////////

bool DocumentVectorSpace::doDocumentProcess(docid_t& docid, TermIdList& termIdList, IdmTermList& termList)
{
	std::set<termid_t> unique_term_set; // unique terms in the document
	std::set<termid_t> unique_term_iter;
	std::pair<set<termid_t>::iterator,bool> unique_term_ret;

	termid_t termid;
	term_doc_tf_map termid2doctf;
	for (TermIdList::iterator iter = termIdList.begin(); iter != termIdList.end(); iter ++)
	{
		// term index
		termid = iter->termid_;
		index_t index = termid;

		// DF, TF in document
		unique_term_ret = unique_term_set.insert(termid);
		if (unique_term_ret.second == true) {
		    if (!isPreLoadTermInfo_) {
                // update DF
                if (termid2df_.find(index) != termid2df_.end()) {
                    termid2df_[index] ++;
                }
                else {
                    termid2df_.insert(term_df_map::value_type(index, 1));
                }
		    }
			// update doc TF
			termid2doctf.insert(term_doc_tf_map::value_type(index, 1));
		}
		else {
			// update doc TF
			termid2doctf[index] ++;
		}
	}

	term_sp_vector representDocVec;
	index_t doc_index = docid;
	index_t term_index;
	weight_t tf;
	weight_t wegt; // df, idf, tf*idf
	count_t doc_length = termIdList.size();
	term_doc_tf_map::iterator dtfIter = termid2doctf.begin();
	for (; dtfIter != termid2doctf.end(); dtfIter++)
	{
		term_index = dtfIter->first;
		tf = dtfIter->second / doc_length; // normalized tf
		if (isPreLoadTermInfo_) {
			wegt = pTermInfoReader_->getDFByTermId(dtfIter->first); // df
			if (wegt < 1) { // non-zero
				continue;
			}
			wegt = std::log( pTermInfoReader_->getDocNum() / wegt); // idf
			wegt *= tf; // weight = tf * idf
			if (wegt < thresholdWegt_) {
				continue;
			}
		}
		representDocVec.value.push_back(std::make_pair(term_index, wegt));
		pDocRepVectors_->SetVector(doc_index, representDocVec);
	}

#ifdef SSP_BUIDER_TEST
	dtfIter = termid2doctf.begin();
	for (; dtfIter != termid2doctf.end(); dtfIter++) {
	    weight_t idf = std::log(pTermInfoReader_->getDocNum() / pTermInfoReader_->getDFByTermId(dtfIter->first));

	    cout << "("<< termList[dtfIter->first] << " " << dtfIter->first
	            << " tf:" << dtfIter->second
                << " df:" << pTermInfoReader_->getDFByTermId(dtfIter->first)
                << " wegt:" << dtfIter->second / doc_length * idf << ") " << endl;
	}
#endif
}

void DocumentVectorSpace::calcWeight()
{
	docid_t docNum = docList_.size();
	docid_t docid;
	std::vector<docid_t>::iterator docIter;
	for (docIter = docList_.begin(); docIter != docList_.end(); docIter ++)
	{
		docid = *docIter;
		term_sp_vector representDocVec;
		pDocRepVectors_->GetVector(docid, representDocVec);
		weight_t idf;
		weight_t weight;

		term_sp_vector::ValueType::iterator termIter = representDocVec.value.begin();
		for (; termIter != representDocVec.value.end(); termIter++)
		{
			idf = std::log(docNum / termid2df_[termIter->first]);
			termIter->second *= idf;
			/*weight = termIter->second * idf;
			if (weight < thresholdWegt_) {
			    termIter = representDocVec.value.erase(termIter);
			}
			else {
			    termIter->second = weight;
			    termIter ++;
			}*/
		}

		pDocRepVectors_->SetVector(docid, representDocVec);
	}
}

