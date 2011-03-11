
#include <idmlib/idm_types.h>
#include <idmlib/similarity/document_similarity.h>

using namespace idmlib::ssp;
using namespace idmlib::sim;

bool DocumentSimilarity::buildSimIndex()
{
	return false;
}

bool DocumentSimilarity::Load()
{
	return false;
}

bool GetSimDocIdList (
		uint32_t docId,
		uint32_t maxNum,
		std::vector<std::pair<uint32_t, float> >& result )
{
	return false;
}

