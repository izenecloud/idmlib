#include <idmlib/semantic_space/explicit_semantic_space.h>
#include <set>

using namespace idmlib::ssp;

void ExplicitSemanticSpace::ProcessDocument(docid_t& docid, std::vector<termid_t>& termids)
{
	doProcessDocument(docid, termids);
}

void ExplicitSemanticSpace::ProcessSpace()
{
	// Post process after all documents are processed
	calcWeight();
	print();

	pTermConceptIndex_->Flush();
}

bool ExplicitSemanticSpace::getTermIds(std::set<termid_t>& termIds)
{
	return false;
}

bool ExplicitSemanticSpace::getTermVector(termid_t termId, std::vector<docid_t> termVec)
{
	return false;
}

void ExplicitSemanticSpace::print()
{
#ifdef RESET_MATRIX_INDEX
	termid_index_map::iterator iter;
	cout << "[termid, index, df] : " << termid2Index_.size() << " == " << termCount_ << endl;
	for (iter = termid2Index_.begin(); iter != termid2Index_.end(); iter ++)
	{
		cout << iter->first << ", " << iter->second << ", " << termidx2DF_[iter->second] << endl;
	}

	docid_index_map::iterator iter2;
	cout << "[docid, index] : " << docid2Index_.size() << " == " << docCount_ << endl;
	for (iter2 = docid2Index_.begin(); iter2 != docid2Index_.end(); iter2++)
	{
		cout << iter2->first << ", " << iter2->second << endl;
	}
#endif

#ifdef SPARSE_MATRIX

#else
	term_doc_matrix::iterator miter;
	boost::shared_ptr<sDocUnit> pDoc;
	cout << "[term-index, (doc-index, weight)] " << termCount_ << "*" << docCount_ << endl;
	//for (miter = termdocM_.begin(); miter != termdocM_.end(); miter++) {
	for (termid_t t = 0; t < termdocM_.size(); t ++) {
		cout << t << " ";
		for (docid_t dt = 0; dt < termdocM_[t].size(); dt ++) {
			pDoc = termdocM_[t][dt];
			if (pDoc) {
				cout << " (" << pDoc->docid << ", " << pDoc->tf << ") ";
			}
		}
		cout << endl;
	}
#endif
}

/// Private Methods ////////////////////////////////////////////////////////////

void ExplicitSemanticSpace::doProcessDocument(docid_t& docid, std::vector<termid_t>& termids)
{
	std::set<termid_t> unique_term_set; // unique terms in the document
	std::set<termid_t> unique_term_iter;
	std::pair<set<termid_t>::iterator,bool> unique_term_ret;

	// doc index
	index_t jndex = getIndexFromDocId(docid);

	termid_t termid;
	termid2doctf_.clear();
	for (std::vector<termid_t>::iterator iter = termids.begin(); iter != termids.end(); iter ++)
	{
		// term index
		termid = *iter;
		index_t index = getIndexFromTermId(termid);

		// DF, TF in document
		unique_term_ret = unique_term_set.insert(termid);
		if (unique_term_ret.second == true) {
			// update DF
			if (termid2df_.find(index) != termid2df_.end()) {
				termid2df_[index] ++;
			}
			else {
				termid2df_.insert(term_df_map::value_type(index, 1));
			}
			// update doc TF
			termid2doctf_.insert(term_doc_tf_map::value_type(index, 1));
		}
		else {
			// update doc TF
			termid2doctf_[index] ++;
		}

		/*
		// tf
		if (index == termdocM_.size()) {
			doc_vector docVec;
			termdocM_.push_back(docVec);
		}
		else if (index > termdocM_.size()) {
			return; // error
		}
		doc_vector & docVec = termdocM_[index]; // row vector

		if (jndex >= docVec.size()) {
			docVec.resize(jndex+1); // performance..
		}
		//cout << "jndex: " << jndex << " " << docVec.size() << endl;
		boost::shared_ptr<sDocUnit>& rpsDoc = docVec[jndex];
		if (!rpsDoc) {
			rpsDoc.reset(new sDocUnit());
			rpsDoc->docid = docid;
		}
		rpsDoc->tf ++; // count*/
	}

	// update term-concept index with TF
	index_t term_index;
	weight_t tf;
	count_t doc_length = termids.size();
	term_doc_tf_map::iterator dtfIter = termid2doctf_.begin();
	for (; dtfIter != termid2doctf_.end(); dtfIter++)
	{
		term_index = getIndexFromTermId(dtfIter->first);
		tf = (weight_t) dtfIter->second / doc_length ; // normalize tf
//		cout << term_index << "  " << jndex << "  " << dtfIter->second << "  " << tf;
		updateTermConceptIndex(term_index, jndex, tf);
	}

/*
	// normalize tf (without repeat for each term)
	count_t totalTerm = terms.size();
	//cout << "total term: " << totalTerm << endl;
	for (std::set<termid_t>::iterator iter = tmp.begin(); iter != tmp.end(); iter ++)
	{
		termid = *iter;
		index_t index = getOrAddTermIndex(termid);

		boost::shared_ptr<sDocUnit>& rpsDoc = termdocM_[index][jndex];
		rpsDoc->tf = rpsDoc->tf / (weight_t)totalTerm;
	}*/
}
