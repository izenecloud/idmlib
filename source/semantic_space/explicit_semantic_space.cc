#include <idmlib/semantic_space/explicit_semantic_space.h>

using namespace idmlib::ssp;

/*
ExplicitSemanticSpace::ExplicitSemanticSpace()
{

}

ExplicitSemanticSpace::~ExplicitSemanticSpace()
{

}
*/

void ExplicitSemanticSpace::processDocument()
{

}

void ExplicitSemanticSpace::processSpace()
{
	// build matrix (inverted index)
}

bool ExplicitSemanticSpace::getTermIds(std::set<termid_t>& termIds)
{
	return false;
}

bool ExplicitSemanticSpace::getTermVector(termid_t termId, std::vector<docid_t> termVec)
{
	return false;
}
