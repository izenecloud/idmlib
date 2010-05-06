///
/// @file cc_types.h
/// @brief Some type definitions for concept-clustering.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-05-05
/// @date Updated 2010-05-05
///

#ifndef IDMCCTYPES_H_
#define IDMCCTYPES_H_

#include "../idm_types.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <ext/hash_map>
#include <cmath>
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <wiselib/ustring/UString.h>

#include <am/cccr_hash/cccr_hash.h>


#include <boost/dynamic_bitset.hpp>
#include <functional>

#include <boost/foreach.hpp>
#include <iostream>

#include <util/izene_serialization.h>

NS_IDMLIB_CC_BEGIN
    

typedef uint32_t conceptid_t;


class ConceptInfo
{
    public:
        ConceptInfo():min_query_distance_(10)
        {}
        ConceptInfo(uint32_t docCount):docInvert_(docCount), score_(1.0), min_query_distance_(10)
        {}
        
        inline void set(uint32_t i)
        {
            docInvert_.set(i);
        }
        
        inline wiselib::UString getTerm(uint32_t i) const
        {
            return termList_[i];
        }
        
        inline wiselib::UString getName() const
        {
            return name_;
        }
        
        inline std::vector<wiselib::UString> getTermList() const
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
        std::vector<wiselib::UString> termList_;
        std::vector<uint32_t> termIdList_;
        wiselib::UString name_;
        boost::dynamic_bitset<> docInvert_;
        double score_;
        uint32_t min_query_distance_;
        
        
};


class ClusterRep
{
    public:
        ClusterRep(){}
        
        ClusterRep(const ClusterRep& rhs):termIdList_(rhs.termIdList_), name_(rhs.name_), docInvert_(rhs.docInvert_), docContain_(rhs.docContain_), children_(rhs.children_)
        {
        }
        
        ClusterRep& operator=(const ClusterRep& rhs)
        {
            termIdList_ = rhs.termIdList_;
            name_ = rhs.name_;
            docInvert_ = rhs.docInvert_;
            docContain_ = rhs.docContain_;
            children_ = rhs.children_;
            return *this;
        }
        
        /// @brief To set whose doc id list that this taxonomy node has.
        /// 
        /// @param doc_ids The doc id list.
        /// @param doc_invert The bitset which indicates whether this node has corresponding doc id to doc_ids.
        void set_doc_contain(const std::vector<uint32_t>& doc_ids, const boost::dynamic_bitset<>& doc_invert)
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
//             name_ = wiselib::UString(" ",wiselib::UString::UTF_8);
//             return;
            name_.clear();
            wiselib::UString str;
            idManager->getTermStringByTermId(termIdList_[0],str);
            name_ += str;
            for(std::size_t i=1;i<termIdList_.size();i++)
            {
                idManager->getTermStringByTermId(termIdList_[i],str);
                name_ += wiselib::UString(" ",wiselib::UString::UTF_8);
                name_ += str;
            }
            
        }
        
    public:
        std::vector<uint32_t> termIdList_;
        wiselib::UString name_;
        boost::dynamic_bitset<> docInvert_;
        std::vector<uint32_t> docContain_;
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
        const boost::dynamic_bitset<>& doc_supp,
        const std::vector<uint32_t>& termIdList)
        : index_(index), isTop_(false), docSupp_(doc_supp), termIdList_(termIdList)
        {
        }
        
    public:
        uint32_t index_;
        bool isTop_;
        boost::dynamic_bitset<> docSupp_;
        std::vector<uint32_t> termIdList_;
        uint32_t conceptId_;
};

/// @brief Represent the node used by taxonomy building.
class ConceptNode
{
    public:
        ConceptNodeValue value_;
        uint32_t level_;
        ConceptNode* parent_;
        std::vector<ConceptNode* > children_;
        boost::shared_ptr<ClusterRep> clusterRep_;
        
        ConceptNode(const ConceptNodeValue& value, ConceptNode* parent = NULL ):value_(value),parent_(parent),children_(0)
        {
            if(parent==NULL) level_ = 0;
            else level_ = parent->level_ +1;
            
        }
        
        ~ConceptNode(){}
        
        void addChild(ConceptNode* child)
        {
            children_.push_back(child);
        }
        
        void bind(boost::shared_ptr<ClusterRep> clusterRep)
        {
            clusterRep_ = clusterRep;
        }
};
   
NS_IDMLIB_CC_END



#endif 
