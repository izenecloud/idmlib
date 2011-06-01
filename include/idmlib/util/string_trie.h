///
/// @file string_trie.h
/// @brief Provide containess search for string
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-05-24
/// @date Updated 2011-05-24


#ifndef IDMLIB_UTIL_STRINGTRIE_H_
#define IDMLIB_UTIL_STRINGTRIE_H_
#include <string>
#include <util/ustring/UString.h>
#include <am/3rdparty/rde_hash.h>
#include "../idm_types.h"
NS_IDMLIB_UTIL_BEGIN


template<typename NodeIdType = uint64_t>
class StringTrie
{
    public:
        typedef izenelib::util::UString StringType;
        typedef typename StringType::value_type CharType;
        typedef std::pair<NodeIdType, StringType> KeyType;
        typedef izenelib::am::rde_hash<KeyType, NodeIdType> ContainerType;
        
        
        
        StringTrie():available_id_(1)
        {
            
        }
                
        void Insert(const StringType& text)
        {
            for(uint32_t i=0;i<text.length();i++)
            {
                NodeIdType id = 0;
                for(uint32_t j=i;j<text.length();j++)
                {
                    KeyType key(id, GetUString_(text,j));
                    NodeIdType* p_next_id = container_.find(key);
                    if(p_next_id==NULL)
                    {
                        NodeIdType next_id = available_id_;
                        available_id_++;
                        container_.insert(key, next_id);
                        id = next_id;
                    }
                    else
                    {
                        id = *p_next_id;
                    }
                }
            }
        }
        
        bool Contains(const StringType& text)
        {
            NodeIdType id = 0;
            for(uint32_t i=0;i<text.length();i++)
            {
                KeyType key(id, GetUString_(text,i));
                NodeIdType* p_next_id = container_.find(key);
                if(p_next_id==NULL)
                {
                    return false;
                }
                else
                {
                    id = *p_next_id;
                }
            }
            return true;
        }

    private:
        static StringType GetUString_(const StringType& text, uint32_t pos)
        {
            return text.substr(pos, 1);
        }
       
                
    private:
        ContainerType container_;
        NodeIdType available_id_;
};
    
NS_IDMLIB_UTIL_END

#endif
