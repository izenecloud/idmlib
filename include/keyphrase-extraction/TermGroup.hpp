///
/// @file TermGroup.hpp
/// @brief The term's group information used for context dependency check.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-02-25
/// @date Updated 2010-04-12 Migrate from sf1v5 to idmlib
///  --- Log


#ifndef IDMKPETERMGROUP_H_
#define IDMKPETERMGROUP_H_

NS_IDMLIB_KPE_BEGIN

template <class IDManager >
class TermGroup
{
    public:
        
        
        TermGroup(IDManager* idManager):idManager_(idManager)
        {
            uint32_t groupId = 0;
            std::vector<wiselib::UString> vec;
            vec.push_back(wiselib::UString("首", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("零", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("一", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("二", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("三", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("四", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("五", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("六", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("七", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("八", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("九", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("十", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("百", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("千", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("万", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("亿", wiselib::UString::UTF_8) );
            
            for(uint32_t i=0;i<vec.size();i++)
            {
                uint32_t id = 0;
                idManager_->getTermIdByTermString( vec[i], 'C', id);
                insert(id, groupId);
            }
            vec.resize(0);
            vec.push_back(wiselib::UString("one", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("two", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("three", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("four", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("five", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("six", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("seven", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("eight", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("nine", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("ten", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("eleven", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("twelve", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("thirteen", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("fourteen", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("fifteen", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("sixteen", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("seventeen", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("eighteen", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("nighteen", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("twenty", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("thirty", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("fourty", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("fifty", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("fixty", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("sixty", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("seventy", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("eighty", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("ninety", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("hundred", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("thousand", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("million", wiselib::UString::UTF_8) );
            vec.push_back(wiselib::UString("billion", wiselib::UString::UTF_8) );
            for(uint32_t i=0;i<vec.size();i++)
            {
                uint32_t id = 0;
                idManager_->getTermIdByTermString( vec[i], 'F', id);
                insert(id, groupId);
            }
            vec.resize(0);
        }
       
        void insert(termid_t termId, termid_t groupId)
        {
            groupMapping_.insert(termId, groupId);
            
        }
        
        void insert(const wiselib::UString& ustr, char tag, termid_t groupId)
        {
            uint32_t termId = 0;
            idManager_->getTermIdByTermString( ustr, tag, termId);
            insert(termId, groupId);
        }
        
        void filter(const std::vector<termid_t>& contextTermIdList, const std::vector<uint32_t>& contextCountList,
        std::vector<uint32_t>& resultCountList)
        {
            resultCountList.reserve(contextCountList.size());
            std::vector<uint32_t> inGroupCount(0, 0);
            for( uint32_t i=0;i<contextTermIdList.size();i++)
            {
                termid_t* groupId = groupMapping_.find(contextTermIdList[i]);
                if( groupId == NULL )
                {
                    resultCountList.push_back( contextCountList[i] );
                }
                else
                {
                    if( *groupId >= inGroupCount.size() )
                    {
                        inGroupCount.resize( (*groupId) + 1, 0);
                    }
                    inGroupCount[*groupId] += contextCountList[i];
                }
            }
            if( inGroupCount.size() > 0 )
            {
                resultCountList.insert( resultCountList.end(), inGroupCount.begin(), inGroupCount.end() );
            }
        }
        
        
        
        
                
    private:
        IDManager* idManager_;
        izenelib::am::rde_hash<termid_t, termid_t> groupMapping_;
};
    

    
NS_IDMLIB_KPE_END

#endif
