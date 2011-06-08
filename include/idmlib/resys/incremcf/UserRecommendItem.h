#ifndef IDMLIB_RESYS_USER_RECOMMEND_ITEM_H
#define IDMLIB_RESYS_USER_RECOMMEND_ITEM_H

#include <idmlib/idm_types.h>

#include <am/beansdb/Hash.h>
#include <sdb/SequentialDB.h>
#include <cache/IzeneCache.h>
#include <util/Int2String.h>
#include <util/ThreadModel.h>
#include <util/timestamp.h>
#include <util/PriorityQueue.h>

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <list>
#include <algorithm>

#include <sys/time.h>

using namespace std;

NS_IDMLIB_RESYS_BEGIN

using izenelib::util::Int2String;

typedef uint32_t ItemType;
typedef uint32_t UserType;
typedef float MeasureType;

typedef std::vector<std::pair<ItemType,MeasureType> > RecommendItemType;

class UserRecommendItem
{
    // use unordered_sdb_tc instead of beansdb for thread safe
    //typedef izenelib::am::beansdb::Hash<Int2String, RecommendItemType > StorageType;
    typedef izenelib::sdb::unordered_sdb_tc<UserType, RecommendItemType, ReadWriteLock > StorageType;
public:
    UserRecommendItem(
          const std::string& homePath
          )
        : store_(homePath)
    {
        store_.open();
    }

    ~UserRecommendItem()
    {
        store_.flush();
        store_.close();
    }

    bool getRecommendItem(UserType userId, RecommendItemType& recommendItem)
    {
        return store_.get(userId, recommendItem);
    }

    void setRecommendItem(UserType userId, RecommendItemType& recommendItem)
    {
        store_.update(userId, recommendItem);
    }
private:
    StorageType store_;
};


NS_IDMLIB_RESYS_END

#endif

