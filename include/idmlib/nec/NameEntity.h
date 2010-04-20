/*
 * NameEntity.h
 *
 *  Created on: Mar 30, 2010
 *      Author: eric
 */

#ifndef NAMEENTITY_H_
#define NAMEENTITY_H_

#include <ml/ClassificationData.h>
#include <ml/Taxonomy.h>
#include <wiselib/ustring/UString.h>

using namespace wiselib;

namespace idmlib
{

class NameEntity : public ml::ClassificationData
{

public:
	UString cur;
	UString pre;
	UString suc;

};


}

#endif /* NAMEENTITY_H_ */
