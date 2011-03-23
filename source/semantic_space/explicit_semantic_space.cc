#include <idmlib/semantic_space/explicit_semantic_space.h>
#include <set>

using namespace idmlib::ssp;

/*
ExplicitSemanticSpace::ExplicitSemanticSpace()
{

}

ExplicitSemanticSpace::~ExplicitSemanticSpace()
{

}
*/

void ExplicitSemanticSpace::processDocument(docid_t& docid, term_vector& terms)
{
	buildTermIndex(docid, terms);
}

void ExplicitSemanticSpace::processSpace()
{
	calcWeight();
	print();
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
}

/// Private Methods ////////////////////////////////////////////////////////////

void ExplicitSemanticSpace::buildTermIndex(docid_t& docid, term_vector& terms)
{
	std::set<termid_t> tmp; // terms without repeat
	std::set<termid_t>::iterator tmp_it;
	std::pair<set<termid_t>::iterator,bool> tmp_ret;

	// column (doc) index
	docid_index_map::iterator iter_;
	index_t jndex = getOrAddIndex(docid2Index_, iter_, docid, docCount_);

	termid_t termid;
	for (term_vector::iterator iter = terms.begin(); iter != terms.end(); iter ++)
	{
		termid = iter->get()->termid;
		// row (term) index
		index_t index = getOrAddTermIndex(termid);

		// df
		tmp_ret = tmp.insert(termid);
		if (tmp_ret.second == true) {
			if ( termidx2DF_.find(index) != termidx2DF_.end() )
				termidx2DF_[index] += 1;
			else
				termidx2DF_[index] = 1;
		}

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
		rpsDoc->tf ++; // count
	}

	// normalize tf (without repeat for each term)
	count_t totalTerm = terms.size();
	//cout << "total term: " << totalTerm << endl;
	for (std::set<termid_t>::iterator iter = tmp.begin(); iter != tmp.end(); iter ++)
	{
		termid = *iter;
		index_t index = getOrAddTermIndex(termid);

		boost::shared_ptr<sDocUnit>& rpsDoc = termdocM_[index][jndex];
		rpsDoc->tf = rpsDoc->tf / (weight_t)totalTerm;
	}
}


void ExplicitSemanticSpace::buildDocIndex(docid_t& docid, term_vector& terms)
{
	// not supported
}
