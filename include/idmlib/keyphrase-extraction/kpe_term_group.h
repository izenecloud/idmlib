///
/// @file kpe_term_group.h
/// @brief The term's group information used for context dependency check.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-02-25
/// @date Updated 2010-04-12 Migrate from sf1v5 to idmlib
///  --- Log


#ifndef IDM_KPETERMGROUP_H_
#define IDM_KPETERMGROUP_H_

#include <idmlib/util/idm_id_converter.h>

NS_IDMLIB_KPE_BEGIN

class KPETermGroup
{
    public:
        
        
        KPETermGroup()
        {
            uint32_t groupId = 0;
            std::vector<izenelib::util::UString> vec;
            vec.push_back(izenelib::util::UString("首", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("零", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("一", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("二", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("三", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("四", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("五", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("六", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("七", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("八", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("九", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("十", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("百", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("千", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("万", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("亿", izenelib::util::UString::UTF_8) );
            
            for(uint32_t i=0;i<vec.size();i++)
            {
              uint32_t id = idmlib::util::IDMIdConverter::GetId( vec[i], idmlib::util::IDMTermTag::CHN );
              insert(id, groupId);
            }
            vec.resize(0);
            vec.push_back(izenelib::util::UString("one", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("two", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("three", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("four", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("five", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("six", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("seven", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("eight", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("nine", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("ten", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("eleven", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("twelve", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("thirteen", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("fourteen", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("fifteen", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("sixteen", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("seventeen", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("eighteen", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("nighteen", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("twenty", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("thirty", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("fourty", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("fifty", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("fixty", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("sixty", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("seventy", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("eighty", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("ninety", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("hundred", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("thousand", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("million", izenelib::util::UString::UTF_8) );
            vec.push_back(izenelib::util::UString("billion", izenelib::util::UString::UTF_8) );
            for(uint32_t i=0;i<vec.size();i++)
            {
              uint32_t id = idmlib::util::IDMIdConverter::GetId( vec[i], idmlib::util::IDMTermTag::ENG );
              insert(id, groupId);
            }
            vec.resize(0);
        }
       
        void insert(uint32_t termId, uint32_t groupId)
        {
            groupMapping_.insert(termId, groupId);
            
        }
        
//         void insert(const izenelib::util::UString& ustr, char tag, uint32_t groupId)
//         {
//             uint32_t termId = 0;
//             idManager_->getTermIdByTermString( ustr, tag, termId);
//             insert(termId, groupId);
//         }
        
        void filter(const std::vector<uint32_t>& contextTermIdList, const std::vector<uint32_t>& contextCountList,
        std::vector<uint32_t>& resultCountList)
        {
            resultCountList.reserve(contextCountList.size());
            std::vector<uint32_t> inGroupCount(0, 0);
            for( uint32_t i=0;i<contextTermIdList.size();i++)
            {
                uint32_t* groupId = groupMapping_.find(contextTermIdList[i]);
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
        izenelib::am::rde_hash<uint32_t, uint32_t> groupMapping_;
};
    

    
NS_IDMLIB_KPE_END

#endif
