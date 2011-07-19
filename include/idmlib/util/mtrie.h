///
/// @file mtrie
/// @brief a simpler version of memory trie
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-07-15
/// @date Updated 2011-07-15
///  --- Log


#ifndef IDMLIB_UTIL_MTRIE_H_
#define IDMLIB_UTIL_MTRIE_H_
#include <string>
#include <am/3rdparty/rde_hash.h>
#include <am/3rdparty/stl_map.h>
#include "../idm_types.h"
NS_IDMLIB_UTIL_BEGIN


template<typename StringType, typename NodeIDType = uint32_t, class DataType = uint32_t>
class MTrie
{
    public:
        typedef typename StringType::value_type CharType;
        
        MTrie():key_(1)
        {
            
        }
        
        void Add(const StringType& str)
        {
            NodeIDType id = GetRootID();
            Add_(str, id);
        }
        
        void Add(const StringType& str, const DataType& data)
        {
            NodeIDType id = GetRootID();
            Add_(str, id);
            data_.insert(id, data);
        }
        
        NodeIDType GetRootID() const
        {
            return 0;
        }

        
        std::pair<bool, bool> Find(const CharType& c, const NodeIDType& parentNID, NodeIDType& childNID)
        {
            std::pair<bool, bool> result(false, false);
            std::pair<NodeIDType, CharType> key_pair(parentNID, c);
            std::pair<NodeIDType, bool> value_pair(0, false);
            if(trie_.get(key_pair,value_pair))
            {
                result.first = true;
                result.second = value_pair.second;
                childNID = value_pair.first;
            }
            //debug info
//             std::cout<<"find "<<key_pair.first<<","<<key_pair.second<<" : "<<value_pair.first<<","<<value_pair.second<<" - "<<childNID<<std::endl;
            return result;
        }
        
        bool GetData(const NodeIDType& id, DataType& data)
        {
            return data_.get(id, data);
        }
        
       
        
        
    private:
        void Add_(const StringType& str, NodeIDType& id)
        {
            id = GetRootID();
            if(str.empty()) return;
            for(std::size_t i = 0; i<str.size(); i++)
            {
                bool last = (i==str.size()-1);
                std::pair<NodeIDType, CharType> key_pair( id, str[i]);
                std::pair<NodeIDType, bool> value_pair(0, false);
                if (trie_.get(key_pair, value_pair ) )
                {
                    id = value_pair.first;
                    if(last && !value_pair.second)
                    {
                        value_pair.second = true;
                        trie_.del(key_pair);
                        trie_.insert(key_pair, value_pair);
//                         std::cout<<"insert "<<key_pair.first<<","<<key_pair.second<<" : "<<value_pair.first<<","<<value_pair.second<<std::endl;
                    }
                }
                else
                {
                    id = key_;
                    key_++;
                    value_pair.first = id;
                    value_pair.second = last;
                    trie_.insert(key_pair, value_pair);
//                     std::cout<<"insert "<<key_pair.first<<","<<key_pair.second<<" : "<<value_pair.first<<","<<value_pair.second<<std::endl;
                    
                    
                    
                }
            }
        }
        
                
    private:
        izenelib::am::stl_map<std::pair<NodeIDType, CharType> , std::pair<NodeIDType, bool> > trie_;
        izenelib::am::stl_map<NodeIDType, DataType > data_;
        NodeIDType key_ ;
};
    
NS_IDMLIB_UTIL_END

#endif
