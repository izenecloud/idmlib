///
/// @file StopWordContainer.hpp
/// @brief Provide suffix search function for some stop words.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-02-24
/// @date Updated 2010-02-24
///  --- Log


#ifndef IDMUTIL_STOPWORDCONTAINER_H_
#define IDMUTIL_STOPWORDCONTAINER_H_

#include "MemoryTrie.hpp"


NS_IDMLIB_UTIL_BEGIN

class StopWordContainer
{
    public:
        typedef MemoryTrie<std::vector<uint32_t>, uint32_t, uint32_t, false, true> DBType;
        
        StopWordContainer():db_(), stopWordId_(1)
        {
            
        }
       
        void insert(const std::vector<uint32_t>& termIdList)
        {
            db_.insert(termIdList, stopWordId_);
            stopWordId_++;
        }
        
        void insert(uint32_t termId)
        {
            std::vector<uint32_t> termIdList(1, termId);
            insert(termIdList);
        }
        
        bool isStopWord(uint32_t termId)
        {
            uint32_t child = 0;
            std::pair<bool, bool> r = db_.isSuffix(termId, db_.getRootID(), child);
            return r.second;
        }
        
        bool endWithStopWord(const std::vector<uint32_t>& termIdList)
        {
            if( termIdList.size() == 0 ) return false;
            if( termIdList.size() == 1 ) return isStopWord(termIdList[0]);
            uint32_t parent = db_.getRootID();
            uint32_t child = 0;
            std::vector<uint32_t>::const_reverse_iterator it = termIdList.rbegin();
            std::pair<bool, bool> r;
            while( it != termIdList.rend() )
            {
                r = db_.isSuffix(*it, parent, child);
                if( !r.first )
                {
                    return false;
                }
                if( r.second )
                {
                    return true;
                }
                parent = child;
                ++it;
            }
            return false;
        }
        
        
                
    private:
        DBType db_;
        uint32_t stopWordId_;
};
    
NS_IDMLIB_UTIL_END

#endif
