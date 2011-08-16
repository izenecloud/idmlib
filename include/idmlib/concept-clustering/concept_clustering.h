///
/// @file algorithm.hpp
/// @brief The first version of concept clustering algorithm
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-05-05
/// @date Updated 2010-05-05
///

#ifndef _IDMCCALGORITHM_HPP_
#define _IDMCCALGORITHM_HPP_


#include "../idm_types.h"
#include <boost/type_traits.hpp>
#include "cc_types.h"
#include <idmlib/util/StringDistance.hpp>
#include <idmlib/util/Util.hpp>
#include <idmlib/util/idm_analyzer.h>
NS_IDMLIB_CC_BEGIN

struct Parameters
{
    uint32_t topDocNum_;
    uint32_t maxLevel_;
    uint32_t perLevelNum_;
    uint32_t canConceptNum_;
    uint32_t minDocSupp_;
    double minContainessValue_;
    double maxContainessValue_;
    uint32_t specialDocCount4ContainessValue_;
    double minContainessValue2_;
    double maxContainessValue2_;
    double minContainess_;
};


template <typename DocIdType>
class ConceptClustering : public boost::noncopyable
{
typedef uint32_t id_type;
typedef izenelib::util::UString StringType;
typedef ConceptClusteringInput<DocIdType> InputType;
typedef ConceptNode<DocIdType> ConceptNodeType;
typedef std::pair<uint32_t, uint32_t> id_pair;
typedef std::vector<id_pair> input_type;
typedef std::vector<boost::shared_ptr<ClusterRep<DocIdType> > > OutputType;
typedef idmlib::util::StringDistance<ConceptItem, std::list<ConceptItem> > DistanceType;
public:

    ConceptClustering()
    {
    }
    
    ~ConceptClustering()
    {
    }


    void DoClustering(const InputType& raw, const Parameters& params, OutputType& output)
    {
        InputType input;
        PreprocessInput_(raw, input);
        izenelib::am::rde_hash<uint64_t, double> containess;
        ComputeContainess_(input, params, containess);
        izenelib::am::rde_hash<uint64_t, double> sibship;
        ComputeSibship_(input, sibship);
        DoHierarchical_(input, params,containess, sibship, output);
    }
    
    
    
private:
    
    void PreprocessInput_(const InputType& input, InputType& filtered)
    {
        filtered.query = input.query;
        filtered.query_term_list = input.query_term_list;
        filtered.query_termid_list = input.query_termid_list;
        filtered.doc_list = input.doc_list;
        const std::vector<ConceptItem>& concept_list = input.concept_list;
        for(uint32_t i=0;i<concept_list.size();i++)
        {
            bool bInsert = false;
            if(concept_list[i].doc_invert.count()<50)
            {
                if(!DistanceType::isDuplicate(concept_list[i], input.query_term_list, input.query))
                {
                    bInsert = true;
                }
            }
            else
            {
                if(!IsDuplicateCluster_(concept_list[i], filtered.concept_list)&&!DistanceType::isDuplicate(concept_list[i], input.query_term_list, input.query))
                {
                    bInsert = true;
                }
            }
            if(bInsert)
            {
                filtered.concept_list.push_back(concept_list[i]);
            }
        }
    }
    
    bool IsDuplicateCluster_(
    const ConceptItem& concept,
    const std::vector<ConceptItem>& valid_concepts
    )
    {
        for(uint32_t j=0;j<valid_concepts.size();j++)
        {
            boost::dynamic_bitset<uint32_t> common=concept.doc_invert& valid_concepts[j].doc_invert;
            if((float)common.count()/concept.doc_invert.count()>0.95
                    && concept.doc_invert.count()<3)
            {

                return true;
            }
        }
        return false;
    }
    
    void ComputeContainess_(
    const InputType& input, const Parameters& params,
    izenelib::am::rde_hash<uint64_t,double>& containess
    )
    {
        double minContainessValue = params.minContainessValue_;
        double maxContainessValue = params.maxContainessValue_;
        if( input.doc_list.size() < params.specialDocCount4ContainessValue_ )
        {
            minContainessValue = params.minContainessValue2_;
            maxContainessValue = params.maxContainessValue2_;
        }
        const std::vector<ConceptItem>& concepts = input.concept_list;
        for ( uint32_t i=0;i<concepts.size();i++ )
        {

            for ( unsigned int j=i+1;j<concepts.size();j++ )
            {
                boost::dynamic_bitset<uint32_t> common = concepts[i].doc_invert & concepts[j].doc_invert;
                double containessValue1 = ( double ) common.count() /concepts[j].doc_invert.count();
                double containessValue2 = ( double ) common.count() /concepts[i].doc_invert.count();
                if ( containessValue1 >= minContainessValue && containessValue2 <= maxContainessValue )
                {
                    uint64_t pairKey=idmlib::util::make64UInt(i,j);
                    containess.insert ( pairKey,containessValue1 );
                }
                
                if ( containessValue2 >= minContainessValue && containessValue1 <= maxContainessValue)
                {
                    uint64_t pairKey=idmlib::util::make64UInt(j,i);
                    containess.insert ( pairKey,containessValue2 );
                }
            }
        }
    }
    
    void ComputeSibship_(
    const InputType& input,
    izenelib::am::rde_hash<uint64_t,double>& sibship)
    {
        const std::vector<ConceptItem>& concepts = input.concept_list;
        int minSibScore=1;
        for ( unsigned int i=0;i<concepts.size();i++ )
        {
            for ( unsigned int j=i+1;j<concepts.size();j++ )
            {
                double sibScore = 0.0;
                if(concepts[i].termid_list.size()==concepts[j].termid_list.size())
                {
                    GetLexicalSibScore_(concepts[i], concepts[j], sibScore);

                }
                if(sibScore>=minSibScore)
                {
                    uint64_t pairKey=idmlib::util::make64UInt(i,j);
                    sibship.insert (pairKey,sibScore);
                    pairKey=idmlib::util::make64UInt(j,i);
                    sibship.insert (pairKey,sibScore);
                }
            }//end for
        }//end for
    }
    
    void GetLexicalSibScore_(const ConceptItem& concept1, const ConceptItem& concept2, double& lexicalScore)
    {
        for(uint32_t ki=0;ki<concept1.termid_list.size();ki++)
        {
            for(uint32_t kj=0;kj<concept2.termid_list.size();kj++)
            {
    //            uint16_t weight;
                if(ki==kj&&concept1.termid_list[ki]==concept2.termid_list[kj])
                {
                    lexicalScore+=1;
                }
            }
        }
    }
    
    void DoHierarchical_(
    const InputType& input,
    const Parameters& params,
    izenelib::am::rde_hash<uint64_t,double>& containess,
    izenelib::am::rde_hash<uint64_t,double>& sibship,
    OutputType& output)
    {
        std::vector<ConceptNodeType* > allConceptNode;
        ConceptNodeValue topNodeValue(input.doc_list.size(), input.query_termid_list);
    //     topNodeValue.conceptId_ = 0;// defined as 0 firstly
        ConceptNodeType* top = new ConceptNodeType ( topNodeValue ) ;
        allConceptNode.push_back ( top );
        const std::vector<ConceptItem>& concepts = input.concept_list;
        std::vector<std::vector<double> > dValue ( 1,std::vector<double> ( concepts.size(),0.0 ) );
        //keep dValue.size()==allTmpNode.size()
        for ( std::size_t i=0;i<concepts.size();i++ )
        {
            dValue[0][i] = ComputeScore_ ( *top,concepts[i],i,containess );
        }
        boost::dynamic_bitset<uint32_t> column_selected ( concepts.size() );
        boost::dynamic_bitset<uint32_t> row_selected ( 1 );
        std::list<ConceptItem > acquiredConcepts;
        while ( true )
        {
            std::pair<uint32_t, uint32_t> max_pos;
            double max_value = 0.0;
            bool foundChild = FindChild_(params, concepts,  row_selected, column_selected, dValue,
                    max_value, max_pos);
            
            if ( foundChild )
            {
                if(!InsertNode_(params, concepts, input.doc_list, containess,max_pos.second
                    , max_pos.first, allConceptNode
                    , row_selected, column_selected, acquiredConcepts, dValue, output))
                {
                    continue;
                }
    //             if(!row_selected[max_pos.first])
    //             {
    //                 std::vector<unsigned int> sib_label_ids;
    //                 findSibling(labels, sibship, column_selected,allLabelNode,
    //                         max_pos.second,max_pos.first, sib_label_ids);
    //                 for(unsigned int i=0;i<sib_label_ids.size();i++)
    //                 {
    //                     if(!insertNode(labels,sortedDocIdList, containess,sib_label_ids[i], max_pos.first, allLabelNode,
    //                             row_selected, column_selected, acquiredLabels, dValue, taxonomyRep))
    //                         continue;
    //                     if(row_selected[max_pos.first])
    //                         break;
    //                 }
    //             }
            }
            else
            {
                break;
            }
        }
        BOOST_FOREACH(ConceptNodeType* lNode, allConceptNode)
        {
            delete lNode;
        }
    }
    
    double ComputeScore_(
    const ConceptNodeType& node,
    const ConceptItem& concept,
    uint32_t concept_index,
    izenelib::am::rde_hash<uint64_t,double>& containess )
    {
        double value = 0.0;
        double dContain = 0.0;
        if ( node.level_==0 ) //top node
        {
            dContain = 1.0;
        }
        else
        {
            double* ptrd = 0;
            uint64_t pairKey;
            pairKey=idmlib::util::make64UInt(node.value_.index_, concept_index);
            ptrd = containess.find ( pairKey );
            if ( ptrd == 0 ) //not a father-child relationship
            {

            }
            else
            {
                dContain = *ptrd;
            }
        }

        double increase = 0.0;
        if ( dContain>0.0)
        {
            uint32_t i_incease = ( node.value_.docSupp_ & concept.doc_invert ).count() ;

            if ( i_incease>0 )
            {
                increase = (double)(i_incease ) / node.value_.docSupp_.count();

                value = std::sqrt( dContain * increase);
                float match = 0;
                if(concept.termid_list.size()>=node.value_.termIdList_.size())
                {
                    BOOST_FOREACH(uint32_t term_id, node.value_.termIdList_)
                    {
                        BOOST_FOREACH(uint32_t _term_id, concept.termid_list)
                        {
                            if(term_id == _term_id) match+=0.6;
                        }

                    }
                }

                if(match>0)
                {
                    float fMatch = match;
                    if( concept.text.length()>0 )
                    {
                        if( !concept.text.isChineseChar(0) )
                        {
                            fMatch *= 5;
                        }
                    }
                    value=value*log(fMatch+2);

                }

                if(node.level_==1)
                {
                    if(concept.termid_list.size()>2)
                    {
                        value *= 0.98;
                    }
                }

            }

        }
        return value;
    }
    
    bool FindChild_(
    const Parameters& params,
    const std::vector<ConceptItem >& concepts,
    const boost::dynamic_bitset<uint32_t>& row_selected,
    const boost::dynamic_bitset<uint32_t>& column_selected,
    const std::vector<std::vector<double> >& dValue,
    double& max_value,
    std::pair<uint32_t, uint32_t>& max_pos)
    {
         bool r = false;
        double minContainess = params.minContainess_;
        for ( std::size_t i=0;i<row_selected.size();i++ )
        {
            if ( row_selected[i] ) continue;
            for ( std::size_t j=0;j<concepts.size();j++ )
            {
                if ( column_selected[j] ) continue;
                double _score = dValue[i][j];
                
                if( _score >= minContainess)
                {
    //                std::cout<<"@@@@@@@@@@@@@@@@@@@@@@@: "<<labels[j].label_score_<<std::endl;
                    _score *= (concepts[j].score+0.1);
    //                _score *= sqrt(labels[j].label_score_+0.1);
                    if ( _score> max_value )
                    {
                        max_value = _score;
                        max_pos = std::make_pair ( i,j );
                        r = true;
                    }
                }
            }
        }
        return r;
    }
    
    bool InsertNode_(
    const Parameters& params,
    const std::vector<ConceptItem >& concepts,
    const std::vector<DocIdType>& docIdList,
    izenelib::am::rde_hash<uint64_t,double>& containess,
    uint32_t index,
    uint32_t parent_index,
    std::vector<ConceptNodeType* >& allConceptNode,
    boost::dynamic_bitset<uint32_t>& row_selected,
    boost::dynamic_bitset<uint32_t>& column_selected,
    std::list<ConceptItem >& acquiredConcepts,
    std::vector<std::vector<double> >& dValue,
    OutputType& output
    )
    {
        column_selected.set ( index );
        if( DistanceType::isDuplicate(concepts[index], acquiredConcepts) )
            return false;
    //     std::string _str;
    //     labels[label_id].name_.convertString(_str, izenelib::util::UString::UTF_8);
    //     std::cout<<"inserting "<<label_id<<" "<<_str<<std::endl;
        acquiredConcepts.push_back(concepts[index]);
        ConceptNodeType* parent = allConceptNode[parent_index];
        boost::shared_ptr<ClusterRep<DocIdType> > cluster ( new ClusterRep<DocIdType>() );
    //             cluster->termIdList_ = labels[label_id].termIdList_;
        cluster->name_ = concepts[index].text;
    //     cluster->conceptId_ = concepts[index].conceptId_;
        boost::dynamic_bitset<uint32_t> tmpDocInvert = concepts[index].doc_invert;
        if ( parent->level_==0 ) //top level
        {
            cluster->set_doc_contain ( docIdList,tmpDocInvert );
            output.push_back ( cluster );
        }
        else
        {
            boost::shared_ptr<ClusterRep<DocIdType> >& bindCluster = parent->clusterRep_;
            tmpDocInvert &= bindCluster->docInvert_;
            cluster->set_doc_contain ( docIdList,tmpDocInvert );
            bindCluster->children_.push_back ( cluster );
        }

        ConceptNodeValue nodeValue(index, tmpDocInvert, concepts[index].termid_list);
    //     nodeValue.conceptId_ = concepts[index].conceptId_;
        ConceptNodeType* node = new ConceptNodeType ( nodeValue,parent ) ;
        parent->addChild ( node );
        if ( parent->children_.size() >=params.perLevelNum_)
        {
            row_selected.set ( parent_index );
        }
        if ( node->level_>=params.maxLevel_ )
        {
            row_selected.push_back ( true );
        }
        else
        {
            row_selected.push_back ( false );
        }
        node->bind ( cluster );
        boost::dynamic_bitset<uint32_t> seedInvert ( concepts[index].doc_invert.size() );
        GetSeedInvert_(seedInvert);
        seedInvert &= concepts[index].doc_invert;
        parent->value_.docSupp_ -= seedInvert;
    //     parent->value_.doc_supp_ -= labels[label_id].doc_invert_;

        allConceptNode.push_back ( node );
        //update parent-cluster value
        if ( !row_selected[parent_index] )
        {
            for ( std::size_t i=0;i<concepts.size();i++ )
            {
                if ( column_selected[i] ) continue;
                if ( dValue[parent_index][i]==0.0 ) continue;//never be available
                    dValue[parent_index][i] = ComputeScore_ ( *parent,concepts[i], i,containess );
            }
        }
        //add new value
        dValue.push_back ( std::vector<double> ( concepts.size(),0.0 ) );
        if ( !row_selected[dValue.size()-1] )
        {
            for ( std::size_t i=0;i<concepts.size();i++ )
            {
                if ( column_selected[i] ) continue;
                double _score = ComputeScore_ ( *node,concepts[i],i,containess );
                dValue[dValue.size()-1][i] = _score;
            }
        }
        return true;
    }
        
    void GetSeedInvert_ (boost::dynamic_bitset<uint32_t>&  seedInvert)
    {
        for(uint32_t i=0;i<seedInvert.size();i++)
        {

            if(i%2==0)
                seedInvert.set(i);

        }
    }
    
    
    
    
private:

    
    
    
};

NS_IDMLIB_CC_END

#endif 
