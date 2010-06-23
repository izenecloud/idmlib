/*
 * Segmenter.h
 *
 *  Created on: Mar 22, 2010
 *      Author: eric
 */

#ifndef DM_LAMGRFACTORY_H_
#define DM_LAMGRFACTORY_H_

#include <la/LA.h>

namespace idmlib {

class LaMgrFactory
{

public:
	static la::LA* getLaMgr();
};

}

#endif /* LAMGRFACTORY_H_ */
