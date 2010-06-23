/*
 * NameEntity.h
 *
 *  Created on: Mar 30, 2010
 *      Author: eric
 */

#ifndef DM_NAMEENTITY_H_
#define DM_NAMEENTITY_H_

#include <ml/ClassificationData.h>
#include <ml/Taxonomy.h>
#include <util/ustring/UString.h>
#include <vector>


namespace idmlib
{

class NameEntity : public ml::ClassificationData
{
typedef izenelib::util::UString string_type;
typedef ml::ClassificationData base_type;
public:
    NameEntity():base_type()
    {
    }
    NameEntity(const string_type& curStr):base_type(),cur(curStr)
    {
    }
    NameEntity(const string_type& curStr
    ,const std::vector<string_type>& preStr,const std::vector<string_type>& sucStr)
    :base_type(),cur(curStr), pre(preStr), suc(sucStr)
    {
    }
public:
    string_type cur;
    std::vector<string_type> pre;
    std::vector<string_type> suc;

};


}

#endif /* NAMEENTITY_H_ */
