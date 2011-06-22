/*
 * IdMgrFactory.cc
 *
 *  Created on: Mar 22, 2010
 *      Author: eric
 */

#include <idmlib/util/IdMgrFactory.h>

namespace idmlib {

IdManager* IdMgrFactory::getIdManager()
{
	static IdManager idMgr;
	return &idMgr;
}

IDManagerESA* IdMgrFactory::getIdManagerESA()
{
	static IDManagerESA IdManagerESA;
	return &IdManagerESA;
}

}
