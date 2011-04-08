/*
 * FileDataModel.cpp
 *
 *  Created on: 2011-3-25
 *      Author: yode
 */

#include <idmlib/resys/model/FileDataModel.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

using namespace std;
NS_IDMLIB_RESYS_BEGIN

FileDataModel::FileDataModel()
{
    // TODO Auto-generated constructor stub
}

void FileDataModel::buildDataModel(std::string dataFile)
{

    std::ifstream ifs(dataFile.c_str());
    while (ifs.good())
    {
        string line;
        getline(ifs,line);
#ifdef DEBUG
        cout<<line<<endl;
#endif
        processFileLine(line);
    }
}
std::set<uint32_t>& FileDataModel::getItems()
{
    return this->items_;
}
std::set<uint32_t>& FileDataModel::getUsers()
{
    return this->users_;
}
void FileDataModel::setItems(std::set<uint32_t>& items)
{
    this->items_ =items;
}
void FileDataModel::setUsers(std::set<uint32_t>&users)
{
    this->users_ = users;
}
std::map<uint32_t, std::vector<ItemPreference> > & FileDataModel::getUserModel()
{
    return this->userModel_;
}
uint32_t FileDataModel::getItemNums()
{
    return this->items_.size();
}
uint32_t FileDataModel::getUserNums()
{
    return this->users_.size();
}


double FileDataModel::getPreferenceValue(uint32_t userId, uint32_t itemId)
{
    if (userModel_.find(userId)!=userModel_.end())
    {

        ItemPreferenceArray itemPrefs = userModel_.find(userId)->second;
        for (uint32_t i=0; i<itemPrefs.size(); i++)
        {
            if (itemPrefs[i].getItemId() ==itemId)
                return itemPrefs[i].getValue();
        }


    }
    return 0;
}
/**
* Process one line data of movielens format data:  userid::itemid::score::timestamp
*
* @param line
*         one record of movielens data
*/
void FileDataModel::processFileLine(string& line)
{

    try
    {
        vector<string> tokens;
        // boost::algorithm::split( tokens, line, boost::algorithm::is_any_of(":") );
        boost::algorithm::split_regex( tokens, line, boost::regex("::") );
        uint32_t userID=boost::lexical_cast<uint32_t>(tokens[0]);
        uint32_t itemID=boost::lexical_cast<uint32_t>(tokens[1]);
        uint32_t preferenceValue=boost::lexical_cast<uint32_t>(tokens[2]);

        ItemPreference itemPref(itemID,preferenceValue);
        UserPreference userPref(userID,preferenceValue);
        uint32_t timestamp;
        string timestampStr = tokens.size()>3? tokens[3]:"";
        if (!timestampStr.empty())
        {
            timestamp= boost::lexical_cast<uint32_t>(tokens[3]);
            itemPref.setTimeStamp(timestamp);
            userPref.setTimeStamp(timestamp);
        }

        if ( itemModel_.find(itemID)!= itemModel_.end())
        {
            std::vector<UserPreference> userPrefs = itemModel_.find(itemID)->second;
            userPrefs.push_back(userPref);
            itemModel_[itemID] = userPrefs;

#ifdef DEBUG
            cout<<"exist itemID is "<<itemID<<",userID is "<<userID<<",preferValue is "<<preferenceValue<<endl;
#endif
        }
        else
        {
            std::vector<UserPreference> userPrefs ;
            userPrefs.push_back(userPref);
            itemModel_[itemID] = userPrefs;

#ifdef DEBUG
            cout<<"new itemID is "<<itemID<<",userID is "<<userID<<",preferValue is "<<preferenceValue<<endl;
#endif
        }

        if ( userModel_.find(userID)!= userModel_.end())
        {
            std::vector<ItemPreference> itemPrefs = userModel_.find(userID)->second;
            itemPrefs.push_back(itemPref);
            userModel_[userID] = itemPrefs;
        }
        else
        {
            std::vector<ItemPreference> itemPrefs ;
            itemPrefs.push_back(itemPref);
            userModel_.insert(make_pair( userID,itemPrefs));
        }

        users_.insert(userID);
        items_.insert(itemID);
#ifdef DEBUG
        cout<<"userID is "<<userID<<",itemID is "<<itemID<<",preferValue is "<<preferenceValue<<endl;
#endif
    }
    catch (boost::bad_lexical_cast &)
    {

    }


}
UserPreferenceArray& FileDataModel:: getUserPreferencesForItem(uint32_t itemId)
{

    try
    {
        if (itemModel_.find(itemId)!=itemModel_.end())
        {
#ifdef DEBUG
            cout<<"itemId is "<<itemId<<", userPrefs size is "<<itemModel_.find(itemId)->second.size()<<endl;
#endif
            return itemModel_.find(itemId)->second;
        }
    }
    catch (...)
    {

    }
    throw std::runtime_error( "getUserPreferencesForItem is NULLl" );


}
std::map<uint32_t, std::vector<UserPreference> >& FileDataModel::getItemModel()
{
    return this->itemModel_;
}

ItemPreferenceArray& FileDataModel:: getItemPreferencesForUser(uint32_t userId)
{
    try
    {
        if (userModel_.find(userId)!=userModel_.end())
        {
            return userModel_.find(userId)->second;
        }
    }

    catch (...)
    {

    }
    throw std::runtime_error( "getItemPreferencesForUser is NULLl" );
}
FileDataModel::~FileDataModel()
{
    // TODO Auto-generated destructor stub
}

NS_IDMLIB_RESYS_END
