#include <idmlib/semantic_space/explicit_semantic_space.h>
#include <set>

using namespace idmlib::ssp;

bool ExplicitSemanticSpace::getTermIds(std::set<termid_t>& termIds)
{
	return false;
}

bool ExplicitSemanticSpace::getTermVector(termid_t termId, std::vector<docid_t> termVec)
{
	return false;
}

void ExplicitSemanticSpace::Print()
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

	cout << "\n[term  DF IDF] : [ (doc TF.IDF) .. ] " << pTermConceptIndex_->VectorCount() << "  "<< termid2df_.size() <<"*" << docList_.size()<< endl;
	index_t termIndex;
	term_df_map::iterator df_iter = termid2df_.begin();
	docid_t docNum = docList_.size();
	for (; df_iter != termid2df_.end(); df_iter ++) {
		termIndex = getIndexFromTermId(df_iter->first );
		cout << "[" <<df_iter->first << "  " << df_iter->second << "  " << docNum / df_iter->second <<"] : ";

		doc_sp_vector docVec;
		pTermConceptIndex_->GetVector(termIndex, docVec);
		idmlib::ssp::PrintSparseVec(docVec, "concepts list");
	}

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

void ExplicitSemanticSpace::doProcessDocument(docid_t& docid, TermIdList& termIdList, IdmTermList& termList)
{
	std::set<termid_t> unique_term_set; // unique terms in the document
	std::set<termid_t> unique_term_iter;
	std::pair<set<termid_t>::iterator,bool> unique_term_ret;

	termid_t termid;
	termid2doctf_.clear(); // TF in document
	for (TermIdList::iterator iter = termIdList.begin(); iter != termIdList.end(); iter ++)
	{
		// term index
		termid = iter->termid_;
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
	}

	// update term-concept index with TF
	index_t term_index;
	weight_t tf;
	count_t doc_length = termIdList.size();
	index_t jndex = getIndexFromDocId(docid); // doc index
	for (term_doc_tf_map::iterator dtfIter = termid2doctf_.begin(); dtfIter != termid2doctf_.end(); dtfIter++)
	{
		term_index = getIndexFromTermId(dtfIter->first);
		tf = (weight_t) dtfIter->second / doc_length ; // normalize tf
		updateTermConceptIndex(term_index, jndex, tf);
	}

#ifdef SSP_BUIDER_TEST
    std::vector<pair<termid_t, weight_t> > termtfVec;
    for (term_doc_tf_map::iterator dtfIter = termid2doctf_.begin(); dtfIter != termid2doctf_.end(); dtfIter++)
    {
        weight_t tf = (weight_t)dtfIter->second / doc_length;
        weight_t idf = std::log( docList_test_.size() / termid2df_test_[dtfIter->first] );
        termtfVec.push_back(std::make_pair(dtfIter->first, tf*idf));
//        cout <<"(" << termList[dtfIter->first] << " "<< dtfIter->first
//                << " tf:" << tf
//                << " idf:" << idf
//                << " wegt:" << tf*idf << ") " << endl;
    }
    cout << "[sort]" << endl;
    std::sort(termtfVec.begin(), termtfVec.end(), sort_second());

    std::vector<pair<termid_t, weight_t> >::iterator ttvIter;
    for (ttvIter = termtfVec.begin(); ttvIter != termtfVec.end(); ttvIter++)
    {
        cout <<"(" << termList[ttvIter->first] << " "<< ttvIter->first
                << " tf:" << termid2doctf_[ttvIter->first]
                << " df:" << termid2df_test_[ttvIter->first]
                << " wegt:" << ttvIter->second << ") " << endl;
    }
    cout << "[ExplicitSemanticSpace::doProcessDocument] finished." << endl;
#endif
}

void ExplicitSemanticSpace::calcWeight()
{
#ifdef SSP_TIME_CHECKER
        idmlib::util::TimeChecker timer("ExplicitSemanticSpace::calcWeight");
#endif
	index_t termIndex;
	weight_t idf;
	weight_t weight;
	term_df_map::iterator df_iter = termid2df_.begin();
	docid_t docNum = docList_.size();
	for (; df_iter != termid2df_.end(); df_iter ++)
	{
		termIndex = getIndexFromTermId(df_iter->first);
		idf = std::log( docNum / df_iter->second );

#ifdef SSP_BUIDER_TEST
		cout << "\n term:" << termIndex << "[ idf = Ln " << docNum << " / " << df_iter->second << " = " << idf
		        << ", threshold = "  << thresholdWegt_ << " ]  ";
#endif
		doc_sp_vector docVec;
		pTermConceptIndex_->GetVector(termIndex, docVec);
		doc_sp_vector::ValueType::iterator vec_iter = docVec.value.begin();
		while (vec_iter != docVec.value.end()) {
		    // pruning **
		    weight = vec_iter->second * idf; // weight = tf*idf
#ifdef SSP_BUIDER_TEST
        cout << "( doc:" << vec_iter->first << "  " <<  vec_iter->second << "*idf =" << weight << ") ";
#endif
		    if (weight < thresholdWegt_) {
		        vec_iter = docVec.value.erase(vec_iter);
		    }
		    else {
		        vec_iter->second = weight;
		        vec_iter ++;
		    }
		}
		pTermConceptIndex_->SetVector(termIndex, docVec);
	}

#ifdef SSP_BUIDER_TEST
	cout << "\n[ExplicitSemanticSpace::calcWeight] finished." << endl;
#endif
}
