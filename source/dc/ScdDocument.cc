/*
 * Document.cc
 *
 *  Created on: Mar 19, 2010
 *      Author: eric
 */
#include <idmlib/dc/ScdDocument.h>
#include <idmlib/dc/LaMgrFactory.h>
#include <idmlib/util/IntIdMgr.h>

namespace idmlib {


ScdDocument::ScdDocument()
{
	docId = "";
	content = "";
	lang = LANG_EN;
	id = 0;
}

ScdDocument::~ScdDocument()
{

}

void ScdDocument::setContent(const string& cont, bool process)
{
	if (process)
	{

		std::vector<UString> terms;
		std::vector<ml::AttrID> termIds;
		la::LA* laMgr = LaMgrFactory::getLaMgr();
		la::TermList termList;
		la::TermList::const_iterator it;
		laMgr->process(UString(cont, UString::UTF_8), termList); // bigram
		for(it=termList.begin(); it!=termList.end(); ++it )
		{
			terms.push_back((*it).text_);
		}
//		IdManager* idMgr = IdMgrFactory::getIdManager();
//		idMgr->getTermIdListByTermStringList(terms, termIds);
		IntIdMgr::getTermIdListByTermStringList(terms, termIds);


		for (std::vector<ml::AttrID>::const_iterator it = termIds.begin();
				it != termIds.end(); ++it)
		{
			tfMap[*it]++;
		}
	}
	else {
		content = cont;
	}
}

}
