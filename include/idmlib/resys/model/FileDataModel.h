/*
 * FileDataModel.h
 *
 *  Created on: 2011-3-25
 *      Author: yode
 */

#ifndef IDMLIB_RESYS_FILEDATAMODEL_H
#define IDMLIB_RESYS_FILEDATAMODEL_H

#include <idmlib/idm_types.h>
#include <idmlib/resys/model/ItemPreference.h>
#include <idmlib/resys/model/UserPreference.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>

NS_IDMLIB_RESYS_BEGIN

typedef std::vector<ItemPreference>  ItemPreferenceArray;
typedef std::vector<UserPreference>  UserPreferenceArray;
class FileDataModel
{
public:

    FileDataModel();
    virtual ~FileDataModel();
    void buildDataModel(std::string dataFile);
    std::set<uint32_t>& getItems();
    std::set<uint32_t>& getUsers();
    std::map<uint32_t, std::vector<ItemPreference> >& getUserModel();
    std::map<uint32_t, std::vector<UserPreference> >& getItemModel();
    uint32_t getItemNums();
    uint32_t getUserNums();
    void setItems(std::set<uint32_t>& items);
    void setUsers(std::set<uint32_t>&users);
    UserPreferenceArray& getUserPreferencesForItem(uint32_t itemId);
    ItemPreferenceArray& getItemPreferencesForUser(uint32_t userId);
    double getPreferenceValue(uint32_t userId, uint32_t itemId);
    void processFileLine(std::string& line);
private:

    std::map<uint32_t, std::vector<ItemPreference> > userModel_;
    std::map<uint32_t, std::vector<UserPreference> > itemModel_;

    std::set<uint32_t> items_;
    std::set<uint32_t> users_;
};

NS_IDMLIB_RESYS_END

#endif /* IDMLIB_RESYS_FILEDATAMODEL_H */
