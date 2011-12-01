#include <idmlib/duplicate-detection/group_table.h>
#include <boost/serialization/deque.hpp>
#include <boost/filesystem.hpp>
// #define GT_DEBUG

using namespace idmlib::dd;
GroupTable::GroupTable(const std::string& file):file_(file)
{
}

GroupTable::~GroupTable()
{
}

bool GroupTable::Load()
{
    if(!boost::filesystem::exists(file_))
    {
        return true;
    }
    std::ifstream ifs(file_.c_str(), std::ios::binary);
    if ( ifs.fail()) return false;
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> group_info_ >> valid_groupid_;
    }
    ifs.close();
    if ( ifs.fail() )
    {
        return false;
    }
    
    InitMap_();

}

void GroupTable::InitMap_()
{
    docid_group_.clear();
    for (uint32_t group_id=0;group_id<group_info_.size();group_id++)
    {
        for (uint32_t i=0;i<group_info_[group_id].size();i++)
        {
            docid_group_.insert(group_info_[group_id][i], group_id);
        }
    }
}

bool GroupTable::Set(const std::vector<std::vector<DocIdType> >& data)
{
    group_info_ = data;
    InitMap_();
    return Flush();
}

bool GroupTable::Flush()
{
    std::ofstream ofs(file_.c_str());
    if ( ofs.fail()) return false;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << group_info_ << valid_groupid_ ;
    }
    ofs.close();
    if ( ofs.fail() )
    {
        return false;
    }
    return true;
}

void GroupTable::AddDoc(const DocIdType& docid1, const DocIdType& docid2)
{
    GroupIdType* value1 = docid_group_.find(docid1);
    GroupIdType* value2 = docid_group_.find(docid2);
    GroupIdType group_id = 0;
    if ( value1 == NULL )
    {
        if ( value2 == NULL )
        {
            if (valid_groupid_.empty())
            {
                group_id = group_info_.size();
                docid_group_.insert(docid1, group_id);
                docid_group_.insert(docid2, group_id);
                std::vector<DocIdType> this_group(2);
                this_group[0] = docid1;
                this_group[1] = docid2;
                group_info_.push_back(this_group);
#ifdef GT_DEBUG
                std::cout<<"[c1] "<<group_id<<","<<docid1<<","<<docid2<<std::endl;
#endif
            }
            else
            {
                group_id = valid_groupid_.front();
                valid_groupid_.pop_front();
                docid_group_.insert(docid1, group_id);
                docid_group_.insert(docid2, group_id);
                group_info_[group_id].resize(2);
                group_info_[group_id][0] = docid1;
                group_info_[group_id][1] = docid2;
#ifdef GT_DEBUG
                std::cout<<"[c1-1] "<<group_id<<","<<docid1<<","<<docid2<<std::endl;
#endif
            }
        }
        else
        {
            group_id = *value2;
            docid_group_.insert(docid1, group_id);
            group_info_[group_id].push_back(docid1);
#ifdef GT_DEBUG
                std::cout<<"[c2] "<<group_id<<","<<docid1<<","<<docid2<<std::endl;
#endif
        }
    }
    else
    {
        if ( value2 == NULL )
        {
            group_id = *value1;
            docid_group_.insert(docid2, group_id);
            group_info_[group_id].push_back(docid2);
#ifdef GT_DEBUG
                std::cout<<"[c3] "<<group_id<<","<<docid1<<","<<docid2<<std::endl;
#endif
        }
        else
        {
            //exist
            GroupIdType groupid1 = *value1;
            GroupIdType groupid2 = *value2;
            if (groupid1!=groupid2)
            {
                GroupIdType from = groupid2;
                GroupIdType to = groupid1;
                if (groupid1>groupid2)
                {
                    from = groupid1;
                    to = groupid2;
                }
                group_info_[to].insert(group_info_[to].end(), group_info_[from].begin(), group_info_[from].end() );
                for (uint32_t i=0;i<group_info_[from].size();i++)
                {
                    DocIdType& docid = group_info_[from][i];
                    docid_group_.del(docid);
                    docid_group_.insert(docid, to);
                }
                group_info_[from].resize(0);
                valid_groupid_.push_back(from);
#ifdef GT_DEBUG
                std::cout<<"[c4] "<<to<<","<<docid1<<","<<docid2<<std::endl;
#endif
            }
        }
    }
}

void GroupTable::RemoveDoc(const DocIdType& docid)
{
    //TODO
}

bool GroupTable::IsSameGroup(const DocIdType& docid1, const DocIdType& docid2)
{
    GroupIdType* value1 = docid_group_.find(docid1);
    if ( value1 == NULL ) return false;
    GroupIdType* value2 = docid_group_.find(docid2);
    if ( value2 == NULL ) return false;
    return ((*value1)==(*value2));
}

bool GroupTable::GetGroupId(const DocIdType& docid, GroupIdType& groupid)
{
    return docid_group_.get(docid, groupid);
}

bool GroupTable::GetDocIdList(const GroupIdType& groupid, std::vector<DocIdType>& docid_list)
{
    if(groupid>=group_info_.size()) return false;
    docid_list = group_info_[groupid];
    return true;
}

void GroupTable::Find(const DocIdType& docid, std::vector<DocIdType>& list)
{
    list.resize(0);
    GroupIdType* value = docid_group_.find(docid);
    if ( value == NULL ) return;
    list = group_info_[*value];
}

void GroupTable::PrintStat() const
{
    std::cout<<"[GroupTable] table size : "<<group_info_.size()<<std::endl;
    uint32_t total = 0;
    for (uint32_t group_id=0;group_id<group_info_.size();group_id++)
    {
        total += group_info_[group_id].size();
    }
    std::cout<<"[GroupTable] total item size : "<<total<<std::endl;
}

