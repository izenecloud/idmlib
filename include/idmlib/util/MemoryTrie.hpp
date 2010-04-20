///
/// @file MemoryTrie.hpp
/// @brief Provide prefix or suffix search function for some stop words.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-02-24
/// @date Updated 2010-02-24
///  --- Log


#ifndef MEMORYTRIE_H_
#define MEMORYTRIE_H_
#include <string>
#include <am/sdb_btree/sdb_bptree.h>
#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <am/db_trie/common_trie.h>
#include "../idm_types.h"
NS_IDMLIB_UTIL_BEGIN

template <typename ItemType,
          typename NodeIDType,
          typename LockType>
class MemoryTrieEdgeTable : public izenelib::am::CommonEdgeTable<ItemType, NodeIDType, LockType>
{
    typedef izenelib::am::CommonEdgeTable<ItemType, NodeIDType, LockType> CommonType;
public:
    typedef std::pair<NodeIDType, ItemType> EdgeTableKeyType;
    typedef NodeIDType EdgeTableValueType;
    typedef izenelib::am::rde_hash<EdgeTableKeyType, EdgeTableValueType> DBType;
    MemoryTrieEdgeTable(const std::string& name)
    : CommonType(name), storage_()
    {
    }
    virtual ~MemoryTrieEdgeTable() { flush(); }
    void open()
    {

    }
    void close()
    {

    }
    void flush()
    {

    }
    void optimize()
    {

    }
    void insertValue(const EdgeTableKeyType& key, const EdgeTableValueType& value)
    {
        storage_.insert(key, value);
    }

    bool getValue(const EdgeTableKeyType& key, EdgeTableValueType& value)
    {
        return storage_.get(key, value);
    }
    
    bool getValueBetween( std::vector<DataType<EdgeTableKeyType,EdgeTableValueType> > & results,
        const EdgeTableKeyType& start, const EdgeTableKeyType& end)
    {
        return false;
    }
    
private:
    DBType storage_;
};


template <typename NodeIDType,
          typename UserDataType,
          typename LockType>
class MemoryTrieDataTable : public izenelib::am::CommonDataTable<NodeIDType, UserDataType, LockType>
{
    typedef izenelib::am::CommonDataTable<NodeIDType, UserDataType, LockType> CommonType;
    

public:

    typedef typename CommonType::DataTableKeyType DataTableKeyType;
    typedef typename CommonType::DataTableValueType DataTableValueType;
    typedef izenelib::am::rde_hash<DataTableKeyType, DataTableValueType> DBType;

    MemoryTrieDataTable(const std::string& name)
    : CommonType(name), db_() { }

    virtual ~MemoryTrieDataTable() { flush(); }

    /**
     * @brief Open all db files, maps them to memory. Call it
     *        after config() and before any insert() or find() operations.
     */
    void open()
    {
    }

    void flush()
    {
    }

    void close()
    {
    }

    void optimize()
    {
    }

    unsigned int numItems()
    {
        return db_.num_items();
    }

    void insertValue(const DataTableKeyType& key, const DataTableValueType & value)
    {
        db_.insert(key, value);
    }

    void update(const DataTableKeyType& key, const DataTableValueType & value)
    {
        db_.update(key, value);
    }

    bool getValue(const DataTableKeyType& key, DataTableValueType & value)
    {
        return db_.get(key, value);
    }

    DBType& getDB()
    {
        return db_;
    }

private:

    DBType db_;
};

/**
 * @brief Definitions for distributed tries on each partition.
 */
template <typename StringType,
          typename UserDataType,
          typename NodeIDType = uint64_t,
          typename LockType = izenelib::util::NullLock>
class MemoryTrieEdgeTrie
{
public:
    typedef typename StringType::value_type CharType;
    typedef NodeIDType IDType;
    typedef MemoryTrieEdgeTable<typename StringType::value_type, NodeIDType, LockType> EdgeTableType;
    typedef typename EdgeTableType::EdgeTableKeyType EdgeTableKeyType;
    typedef typename EdgeTableType::EdgeTableValueType EdgeTableValueType;
    typedef DataType<EdgeTableKeyType, EdgeTableValueType> EdgeTableRecordType;
    typedef MemoryTrieDataTable      <NodeIDType, UserDataType, LockType>
            DataTableType;
    typedef CommonTrie_<CharType, UserDataType, NodeIDType,
        EdgeTableType, DataTableType> CommonTrieType;
    
//     typedef typename EdgeTableType::EdgeTableKeyType EdgeTableKeyType;
    MemoryTrieEdgeTrie(const std::string name)
    :   trie_(name) {}


    virtual ~MemoryTrieEdgeTrie() {}
    
    void open(){ trie_.open(); }
    void flush(){ trie_.flush(); }
    void close(){ trie_.close(); }
    
    
    NodeIDType getRootID() const
    {
        return izenelib::am::NodeIDTraits<NodeIDType>::RootValue;
    }
    
    void insert(const StringType& key, UserDataType value)
    {
        trie_.insert(key, value);
    }
    
    
    bool getEdge(const CharType& ch, const NodeIDType& parentNID, NodeIDType& childNID)
    {
        return trie_.getEdge(ch, parentNID, childNID);
    }
    
    bool getData( const NodeIDType& nid, UserDataType& userData)
    {
        return trie_.getData(nid, userData);
    }
    
private:
    
        
private:

    CommonTrieType trie_;
};

template<typename StringType, typename UserDataType, 
typename NodeIDType = uint64_t, bool FORWARD=true, bool REVERSE=true>
class MemoryTrie
{
    public:
        typedef typename StringType::value_type CharType;
        typedef MemoryTrieEdgeTrie<StringType, UserDataType, NodeIDType> TrieType;
        
        MemoryTrie():forward_(""), reverse_("")
        {
            
        }
        void open()
        {
            if( FORWARD )
                forward_.open();
            if( REVERSE )
                reverse_.open();
        }
        void flush()
        {
            if( FORWARD )
                forward_.flush();
            if( REVERSE )
                reverse_.flush();
        }

        void close()
        {
            if( FORWARD )
                forward_.close();
            if( REVERSE )
                reverse_.close();
        }
        
        void insert(const StringType& label, const UserDataType& labelId)
        {
            if( FORWARD )
            {
                forward_.insert(label, labelId);
            }
            if( REVERSE )
            {
                std::vector<termid_t> reverseVec(label);
                std::reverse(reverseVec.begin(), reverseVec.end());
                reverse_.insert(reverseVec, labelId);
            }
        }

        NodeIDType getRootID()
        {
            return forward_.getRootID();
        }
        
        std::pair<bool, bool> isPrefix(const CharType& termId, const NodeIDType& parentNID, NodeIDType& childNID)
        {
            if( FORWARD )
            {
                return isPrefix_(forward_, termId, parentNID, childNID);
            }
            else
            {
                return std::make_pair(false, false);
            }
        }
        
        std::pair<bool, bool> isSuffix(const CharType& termId, const NodeIDType& parentNID, NodeIDType& childNID)
        {
            if( REVERSE )
            {
                return isPrefix_(reverse_, termId, parentNID, childNID);
            }
            else
            {
                return std::make_pair(false, false);
            }
        }
        
        
        
    private:
        
        std::pair<bool, bool> isPrefix_(TrieType& trie, const CharType& termId, const NodeIDType& parentNID, NodeIDType& childNID)
        {
            std::pair<bool, bool> result(false, false);
            bool b = trie.getEdge( termId, parentNID, childNID);
            if(!b) return result;
            result.first = true;
            UserDataType dataId;
            b = trie.getData( childNID, dataId);
            if(!b) return result;
            result.second = true;
            return result;
        }
        
                
    private:
        TrieType forward_;
        TrieType reverse_;
};
    
NS_IDMLIB_UTIL_END

#endif
