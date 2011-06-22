/*
 * IdMgrFactory.h
 *
 *  Created on: Mar 22, 2010
 *      Author: eric
 */

#ifndef IDMGRFACTORY_H_
#define IDMGRFACTORY_H_

#include <ir/id_manager/IDManager.h>
#include <util/ustring/UString.h>

using namespace izenelib::ir::idmanager;
using namespace izenelib::util;

namespace idmlib {

typedef IDManagerIClassifier IdManager;

/**
 * @brief this class is used to generate singleton IDManager
 */
class IdMgrFactory
{
public:
	/*!
	 * \brief get a singleton IDManager
	 */
	static IdManager* getIdManager();

	/**
	 * \brief get a singleton IDManager
	 */
	static IDManagerESA* getIdManagerESA();

};


}

#endif /* IDMGRFACTORY_H_ */
