#ifndef IDMLIB_RESYS_USER_REC_STORAGE_H
#define IDMLIB_RESYS_USER_REC_STORAGE_H

#include <idmlib/idm_types.h>
#include <idmlib/resys/RecommendItem.h>

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

#include <glog/logging.h>

using namespace std;

NS_IDMLIB_RESYS_BEGIN

typedef uint32_t UserType;

class UserRecStorage
{
    // use unordered_sdb_tc instead of beansdb for thread safe
    //typedef izenelib::am::beansdb::Hash<Int2String, RecommendItemVec > StorageType;
    typedef izenelib::sdb::unordered_sdb_tc<UserType, RecommendItemVec, ReadWriteLock > StorageType;
public:
    UserRecStorage(
          const std::string& homePath
          )
        : store_(homePath)
    {
        bool result = store_.open();
        if (!result)
        {
            LOG(ERROR) << "failed open db: " << homePath;
        }
    }

    ~UserRecStorage()
    {
        flush();
        store_.close();
    }

    void flush()
    {
        store_.flush();
    }

    bool get(UserType userId, RecommendItemVec& recommendItem)
    {
        return store_.get(userId, recommendItem);
    }

    void update(UserType userId, const RecommendItemVec& recommendItem)
    {
        store_.update(userId, recommendItem);
    }
private:
    StorageType store_;
};


NS_IDMLIB_RESYS_END

#endif

