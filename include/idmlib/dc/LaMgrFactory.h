/*
 * Segmenter.h
 *
 *  Created on: Mar 22, 2010
 *      Author: eric
 */

#ifndef LAMGRFACTORY_H_
#define LAMGRFACTORY_H_

#include <LA.h>

namespace idmlib {

class LaMgrFactory
{

public:
	static la::LA* getLaMgr();
};

}

#endif /* LAMGRFACTORY_H_ */
