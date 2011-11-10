#ifndef IDMLIB_DD_GROUP_TABLE_H
#define IDMLIB_DD_GROUP_TABLE_H
#include <idmlib/idm_types.h>
#include "dd_types.h"
#include <am/sequence_file/SimpleSequenceFile.hpp>
#include <am/3rdparty/rde_hash.h>

NS_IDMLIB_DD_BEGIN

class GroupTable
{
    
public:
    GroupTable(const std::string& file);

    ~GroupTable();

    bool Load();

    bool Flush();

    void AddDoc(const DocIdType& docid1, const DocIdType& docid2);
    
    void RemoveDoc(const DocIdType& docid);

    bool IsSameGroup(const DocIdType& docid1, const DocIdType& docid2);
    
    bool GetGroupId(const DocIdType& docid, GroupIdType& groupid);

    /**
       @return a reference of vector of docids that are in the same group.
     */
    void Find(const DocIdType& docid, std::vector<DocIdType>& list);

    void PrintStat() const;
    
    const std::vector<std::vector<DocIdType> >& GetGroupInfo() const
    {
        return group_info_;
    }

private:
    std::string file_;
    izenelib::am::rde_hash<DocIdType, GroupIdType> docid_group_;
    std::vector<std::vector<DocIdType> > group_info_;
    std::deque<GroupIdType> valid_groupid_;
};

NS_IDMLIB_DD_END

#endif
