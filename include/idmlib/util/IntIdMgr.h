/*
 * IdMgrFactory.h
 *
 *  Created on: Mar 22, 2010
 *      Author: eric
 */

#ifndef INTIDMGR_H_
#define INTIDMGR_H_

#include <ir/id_manager/IDManager.h>
#include <wiselib/ustring/UString.h>
#include "IdMgrFactory.h"

using namespace izenelib::ir::idmanager;
using namespace wiselib;

namespace idmlib {

/**
 * @brief this class is used to generate singleton IDManager
 */
class IntIdMgr
{
public:
	/*!
	 * \brief get a singleton IDManager
	 */
	static void getTermIdListByTermStringList(std::vector<UString>& terms, std::vector<ml::AttrID>& ids)
	{
		std::vector<ml::AttrID> termIds;
		IdManager* idMgr = IdMgrFactory::getIdManager();
		idMgr->getTermIdListByTermStringList(terms, termIds);
		for (std::vector<ml::AttrID>::iterator it = termIds.begin();
				it != termIds.end(); ++it)
		{
			ids.push_back((*it)/2+1);
		}
	}

};
}

#endif /* INTIDMGR_H_ */
