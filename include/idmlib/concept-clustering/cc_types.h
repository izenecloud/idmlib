///
/// @file cc_types.h
/// @brief Some type definitions for concept-clustering.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-05-05
/// @date Updated 2010-05-05
///

#ifndef IDMLIB_CCTYPES_H_
#define IDMLIB_CCTYPES_H_

#include "../idm_types.h"
#include <boost/shared_ptr.hpp>
#include <boost/dynamic_bitset.hpp>
#include <string>
#include <vector>
#include <3rdparty/msgpack/msgpack.hpp>
#include <util/ustring/UString.h>

NS_IDMLIB_CC_BEGIN
    

typedef uint32_t conceptid_t;
// typedef uint64_t ccdocid_t;

class ConceptInfo
{
    public:
        ConceptInfo():min_query_distance_(10)
        {
        }
        
        ConceptInfo(uint32_t docCount):docInvert_(docCount), score_(1.0), min_query_distance_(10)
        {
        }
        
        inline void set(uint32_t i)
        {
            docInvert_.set(i);
        }
        
        inline izenelib::util::UString getTerm(uint32_t i) const
        {
            return termList_[i];
        }
        
        inline izenelib::util::UString getName() const
        {
            return name_;
        }
        
        inline std::vector<izenelib::util::UString> getTermList() const
        {
            return termList_;
        }
        
        inline uint32_t getDocSupp() const
        {
            return docInvert_.count();
        }
        
        inline uint32_t size() const
        {
            return termList_.size();
        }
        
        uint32_t index_;
        uint32_t conceptId_;
        std::vector<izenelib::util::UString> termList_;
        std::vector<uint32_t> termIdList_;
        izenelib::util::UString name_;
        boost::dynamic_bitset<uint32_t> docInvert_;
        double score_;
        uint32_t min_query_distance_;
        
        
};

template <typename DocIdType>
class ClusterRep
{
    public:
        ClusterRep(){}
        
        
        /// @brief To set whose doc id list that this taxonomy node has.
        /// 
        /// @param doc_ids The doc id list.
        /// @param doc_invert The bitset which indicates whether this node has corresponding doc id to doc_ids.
        void set_doc_contain(const std::vector<DocIdType>& doc_ids, const boost::dynamic_bitset<uint32_t>& doc_invert)
        {
            docContain_.clear();
            for(std::size_t i=0;i<doc_ids.size();i++)
            {
                if(doc_invert[i])
                {
                    docContain_.push_back(doc_ids[i]);
                }
            }
            docInvert_ = doc_invert;
        }
        
        /// @brief Use id manager to convert id to ustring.
        /// 
        /// @param idManager The id manager.
        template <class T>
        void setIDManager(boost::shared_ptr<T> idManager)
        {
//             name_ = izenelib::util::UString(" ",izenelib::util::UString::UTF_8);
//             return;
            name_.clear();
            izenelib::util::UString str;
            idManager->getTermStringByTermId(termIdList_[0],str);
            name_ += str;
            for(std::size_t i=1;i<termIdList_.size();i++)
            {
                idManager->getTermStringByTermId(termIdList_[i],str);
                name_ += izenelib::util::UString(" ",izenelib::util::UString::UTF_8);
                name_ += str;
            }
            
        }
        
    public:
        std::vector<uint32_t> termIdList_;
        uint32_t conceptId_;
        izenelib::util::UString name_;
        boost::dynamic_bitset<uint32_t> docInvert_;
        std::vector<DocIdType> docContain_;
        std::vector<boost::shared_ptr<ClusterRep> > children_;
};


/// @brief The node value used by TgLabelNode.
class ConceptNodeValue
{
    public:
        ConceptNodeValue()
        {
            
        }
        
        /// @brief Top node.
        ConceptNodeValue(
        uint32_t doc_count,
        const std::vector<uint32_t>& termIdList)
        : index_(0), isTop_(true), docSupp_(doc_count), termIdList_(termIdList)
        {
            docSupp_.flip();
        }
        
        /// @brief Not top node.
        ConceptNodeValue(
        uint32_t index,
        const boost::dynamic_bitset<uint32_t>& doc_supp,
        const std::vector<uint32_t>& termIdList)
        : index_(index), isTop_(false), docSupp_(doc_supp), termIdList_(termIdList)
        {
        }
        
    public:
        uint32_t index_;
        bool isTop_;
        boost::dynamic_bitset<uint32_t> docSupp_;
        std::vector<uint32_t> termIdList_;
        uint32_t conceptId_;
};

/// @brief Represent the node used by taxonomy building.
template < typename DocIdType>
class ConceptNode
{
    public:
        ConceptNodeValue value_;
        uint32_t level_;
        ConceptNode* parent_;
        std::vector<ConceptNode<DocIdType>* > children_;
        boost::shared_ptr<ClusterRep<DocIdType> > clusterRep_;
        
        ConceptNode(const ConceptNodeValue& value, ConceptNode<DocIdType>* parent = NULL ):value_(value),parent_(parent),children_(0)
        {
            if(parent==NULL) level_ = 0;
            else level_ = parent->level_ +1;
            
        }
        
        ~ConceptNode(){}
        
        void addChild(ConceptNode<DocIdType>* child)
        {
            children_.push_back(child);
        }
        
        void bind(boost::shared_ptr<ClusterRep<DocIdType> > clusterRep)
        {
            clusterRep_ = clusterRep;
        }
};


class ConceptItem
{
    public:
        ConceptItem():min_query_distance(1)
        {
        }
        
        ConceptItem(uint32_t doc_count):doc_invert(doc_count), score(1.0), min_query_distance(1)
        {
        }
        
//         inline void set(uint32_t i)
//         {
//             docInvert_.set(i);
//         }
//         
        inline izenelib::util::UString getTerm(uint32_t i) const
        {
            return term_list[i];
        }

        inline izenelib::util::UString getName() const
        {
            return text;
        }
       
        inline std::vector<izenelib::util::UString> getTermList() const
        {
            return term_list;
        }
//         
//         inline uint32_t getDocSupp() const
//         {
//             return docInvert_.count();
//         }
//         
        inline uint32_t size() const
        {
            return term_list.size();
        }
        
//         uint32_t concept_id;
        izenelib::util::UString text;
        std::vector<izenelib::util::UString> term_list;
        std::vector<uint32_t> termid_list;
        
        boost::dynamic_bitset<uint32_t> doc_invert;
        double score;
        uint32_t min_query_distance;
        uint8_t type;
        
        template <typename Packer>
        void msgpack_pack(Packer& pk) const
        {
            pk.pack_array(7);
            
            pk.pack(text);
            pk.pack(term_list);
            pk.pack(termid_list);
            std::vector<uint32_t> bitset_c(doc_invert.num_blocks(), 0);
            boost::to_block_range(doc_invert, bitset_c.begin());
            std::pair<std::vector<uint32_t>, uint32_t> bitset_v(bitset_c, doc_invert.size());
            pk.pack(bitset_v);
            pk.pack(score);
            pk.pack(min_query_distance);
            pk.pack(type);
        }
        void msgpack_unpack(msgpack::object o)
        {
            if(o.type != msgpack::type::ARRAY) { throw msgpack::type_error(); }
            const size_t size = o.via.array.size;
            
            if(size <= 0) { return; } o.via.array.ptr[0].convert(&text);
            if(size <= 1) { return; } o.via.array.ptr[1].convert(&term_list);
            if(size <= 2) { return; } o.via.array.ptr[2].convert(&termid_list);
            std::pair<std::vector<uint32_t>, uint32_t> bitset_v;
            if(size <= 3) { return; } o.via.array.ptr[3].convert(&bitset_v);
            doc_invert.append( bitset_v.first.begin(), bitset_v.first.end());
            doc_invert.resize( bitset_v.second);
            if(size <= 4) { return; } o.via.array.ptr[4].convert(&score);
            if(size <= 5) { return; } o.via.array.ptr[5].convert(&min_query_distance);
            if(size <= 6) { return; } o.via.array.ptr[6].convert(&type);
        }
        void msgpack_object(msgpack::object* o, msgpack::zone* z) const
        {
            o->type = msgpack::type::ARRAY;
            o->via.array.ptr = (msgpack::object*)z->malloc(sizeof(msgpack::object)*7);
            o->via.array.size = 7;
            
            o->via.array.ptr[0] = msgpack::object(text, z);
            o->via.array.ptr[1] = msgpack::object(term_list, z);
            o->via.array.ptr[2] = msgpack::object(termid_list, z);
            std::vector<uint32_t> bitset_c;
            o->via.array.ptr[3] = msgpack::object(bitset_c, z);
            o->via.array.ptr[4] = msgpack::object(score, z);
            o->via.array.ptr[5] = msgpack::object(min_query_distance, z);
            o->via.array.ptr[6] = msgpack::object(type, z);
        }
        
//         uint32_t index_;
//         uint32_t conceptId_;
        
        
};

template <typename DocIdType>
struct ConceptClusteringInput
{
    std::vector<ConceptItem> concept_list;
    izenelib::util::UString query;
    std::vector<izenelib::util::UString> query_term_list;
    std::vector<uint32_t> query_termid_list;
    std::vector<DocIdType> doc_list;
    
    MSGPACK_DEFINE(concept_list, query, query_term_list, query_termid_list, doc_list);

};

typedef ConceptClusteringInput<uint32_t> CCInput32;
typedef ConceptClusteringInput<uint64_t> CCInput64;

   
NS_IDMLIB_CC_END



#endif 
