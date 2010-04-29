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


namespace idmlib
{

class NameEntity : public ml::ClassificationData
{
typedef wiselib::UString string_type;
typedef ml::ClassificationData base_type;
public:
    NameEntity():base_type()
    {
    }
    NameEntity(const string_type& curStr):base_type(),cur(curStr)
    {
    }
    NameEntity(const string_type& curStr
    ,const string_type& preStr,const string_type& sucStr)
    :base_type(),cur(curStr), pre(preStr), suc(sucStr)
    {
    }
public:
    string_type cur;
    string_type pre;
    string_type suc;

};


}

#endif /* NAMEENTITY_H_ */
