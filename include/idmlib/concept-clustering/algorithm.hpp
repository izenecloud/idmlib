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

template <class ConceptManagerType>
class Algorithm : public boost::noncopyable
{
typedef uint32_t id_type;
typedef izenelib::util::UString string_type;
typedef std::pair<uint32_t, uint32_t> id_pair;
typedef std::vector<id_pair> input_type;
typedef std::vector<boost::shared_ptr<ClusterRep> > OutputType;
typedef idmlib::util::StringDistance<ConceptInfo, std::list<ConceptInfo> > distance_type;
public:

    Algorithm(ConceptManagerType* conceptManager, idmlib::util::IDMAnalyzer* analyzer)
    : conceptManager_(conceptManager), analyzer_(analyzer), isQuiet_(false)
    {

    }
    
    ~Algorithm()
    {
    }

    void setQuiet(bool isQuiet)
    {
        isQuiet_ = isQuiet;
    }


    void doClustering(input_type& inputPairList, const std::vector<uint32_t>& docIdList,
                      uint32_t totalDocCount, const Parameters& params, OutputType& output)
    {
        string_type emptyQuery;
        doClustering(inputPairList, emptyQuery,docIdList,totalDocCount, params, output);
    }
    
    void doClustering(input_type& inputPairList, const string_type& query, 
                      const std::vector<uint32_t>& docIdList,
                      uint32_t totalDocCount, const Parameters& params, OutputType& output)
    {
        std::vector<uint32_t> queryTermIdList;
        std::vector<string_type> queryTermStrList;
        analyzer_->GetTermList(query, queryTermStrList, queryTermIdList);
        std::vector<ConceptInfo> conceptList;
        uint32_t docCount = docIdList.size();
        getConceptList_(inputPairList, docCount, totalDocCount, query, queryTermStrList, params, conceptList);
        izenelib::am::rde_hash<uint64_t, double> containess;
        computeContainess_(params, conceptList, docIdList.size(),  containess);
        izenelib::am::rde_hash<uint64_t, double> sibship;
        computeSibship_(conceptList, sibship);
        doHierarchical_(params, queryTermIdList, conceptList,containess, sibship, docIdList, output);
    }
    
private:
    
    void getConceptList_(input_type& inputPairList, uint32_t docCount, uint32_t totalDocCount, const string_type& queryStr, const std::vector<string_type>& queryTermList, const Parameters& params, std::vector<ConceptInfo >& conceptResultList)
    {
        bool showtime = !isQuiet_;
        izenelib::util::ClockTimer clocker;

        std::sort(inputPairList.begin(), inputPairList.end() );

        if( showtime )
        {
            std::cout<<"[Sort on id pair]: "<<clocker.elapsed()<<" seconds."<<std::endl;
            clocker.restart();
        }
        uint32_t distinctCount = 0;
        conceptid_t lastConceptId = 0;

        for ( std::size_t i=0;i<inputPairList.size();i++ )
        {
            if(inputPairList[i].first!=lastConceptId)
            {
                ++distinctCount;
                lastConceptId = inputPairList[i].first;
            }

        }
        std::vector<ConceptInfo > conceptList(distinctCount, ConceptInfo(docCount) );
        lastConceptId = 0;
        uint32_t p=0;
        for ( std::size_t i=0;i<inputPairList.size();i++ )
        {
            if( inputPairList[i].first != lastConceptId )
            {
                conceptList[p].conceptId_ = inputPairList[i].first;
                ++p;
                
            }

            conceptList[p-1].set(inputPairList[i].second);
            lastConceptId = inputPairList[i].first;
        }
        if( showtime )
        {
            std::cout<<"[TG phase 3-1-3]: "<<clocker.elapsed()<<" seconds."<<std::endl;
            clocker.restart();
        }
        if( showtime )
        {
            std::cout<<"[TG phase 3-1]: "<<clocker.elapsed()<<" seconds."<<std::endl;
            clocker.restart();
            std::cout<<"TG: original "<<conceptList.size() <<" concepts."<<std::endl;
        }
        

        std::vector<std::pair<double, uint32_t> > score_conceptIndex(conceptList.size());
        for ( std::size_t i=0;i<conceptList.size();i++ )
        {
            score_conceptIndex[i].first = 0.0;
            score_conceptIndex[i].second = i;
            uint32_t partialDF = conceptList[i].getDocSupp();
            if ( partialDF < params.minDocSupp_ )
            {
                continue;
            }
            //compute score
            
            uint32_t globalDF =1;
            float inc = totalDocCount;
            inc = inc/10000;
            if(inc<2) inc = 2;
            conceptManager_->getConceptDFByConceptId(conceptList[i].conceptId_, globalDF);
            conceptList[i].score_ =log(partialDF+1)/log((sqrt(globalDF+1)+1)/log(totalDocCount+2)+2);


            double score2rank = log(partialDF+1)/log((globalDF+inc)/log(sqrt(totalDocCount+1)+1)+1);

            //Because the top K value is hard coded in SIA now,
            // so just also hard coded in MIA.
            //This should be made configurable further.
            float thresh=(float)docCount/3.5;
            if(docCount>10&&partialDF>thresh)
            {
                conceptList[i].score_ =0;
                score2rank=0;
            }

            score_conceptIndex[i].first = score2rank;
            score_conceptIndex[i].second = i;
        }
        if( showtime )
        {
            std::cout<<"[TG phase 3-2]: "<<clocker.elapsed()<<" seconds."<<std::endl;
            clocker.restart();
        }
        std::sort(score_conceptIndex.begin(),score_conceptIndex.end(),std::greater<std::pair<double, unsigned int> >());


        unsigned int concept_count = score_conceptIndex.size()>params.canConceptNum_?params.canConceptNum_:score_conceptIndex.size();
        
        for(uint32_t i=0;i<concept_count;i++)
        {
            uint32_t index = score_conceptIndex[i].second;
            
            conceptManager_->getConceptStringByConceptId( conceptList[index].conceptId_, conceptList[index].name_ );

            if( conceptList[index].name_.length()>0 && conceptList[index].name_.isKoreanChar(0) && conceptList[index].name_.length()<4)
            {
                conceptList[index].score_ *= 0.33;
            }
            analyzer_->GetTermList(conceptList[index].name_, conceptList[index].termList_, conceptList[index].termIdList_);
            
            bool bInsert = false;
            if(docCount<50)
            {
                if(!distance_type::isDuplicate(conceptList[index], queryTermList, queryStr))
                {
                    bInsert = true;
                    
                }
            }
            else
            {
                if(!isDuplicateCluster_(conceptList, index, conceptResultList)&&!distance_type::isDuplicate(conceptList[index], queryTermList, queryStr))
                bInsert = true;
            }
            if(bInsert)
            {
                conceptList[index].index_ = conceptResultList.size();
                conceptResultList.push_back(conceptList[index]);
                
        }

        }
        if( showtime )
        {
            std::cout<<"[TG phase 3-3]: "<<clocker.elapsed()<<" seconds."<<std::endl;
            clocker.restart();
        }
    }
    
    bool isDuplicateCluster_(
        const std::vector<ConceptInfo >& concepts,
        uint32_t pos,
        const std::vector<ConceptInfo>& valid_concepts
        )
    {
        for(unsigned int j=0;j<valid_concepts.size();j++)
        {
            boost::dynamic_bitset<> common=concepts[pos].docInvert_& valid_concepts[j].docInvert_;
            if((float)common.count()/concepts[pos].docInvert_.count()>0.95
                    && concepts[pos].docInvert_.count()<3)
            {

                return true;
            }
        }
        return false;
    }
    
    void computeContainess_(
        const Parameters& params,
        const std::vector<ConceptInfo >& concepts,
        uint32_t docCount,
        izenelib::am::rde_hash<uint64_t,double>& containess
        )
    {
        double minContainessValue = params.minContainessValue_;
        double maxContainessValue = params.maxContainessValue_;
        if( docCount < params.specialDocCount4ContainessValue_ )
        {
            minContainessValue = params.minContainessValue2_;
            maxContainessValue = params.maxContainessValue2_;
        }

        for ( unsigned int i=0;i<concepts.size();i++ )
        {

            for ( unsigned int j=i+1;j<concepts.size();j++ )
            {
                boost::dynamic_bitset<> common = concepts[i].docInvert_ & concepts[j].docInvert_;
                double containessValue1 = ( double ) common.count() /concepts[j].docInvert_.count();
                double containessValue2 = ( double ) common.count() /concepts[i].docInvert_.count();
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
    
    void computeSibship_(
        const std::vector<ConceptInfo >& concepts,
        izenelib::am::rde_hash<uint64_t,double>& sibship)
    {
        int minSibScore=1;
        for ( unsigned int i=0;i<concepts.size();i++ )
        {
            for ( unsigned int j=i+1;j<concepts.size();j++ )
            {
                double sibScore = 0.0;
                if(concepts[i].termIdList_.size()==concepts[j].termIdList_.size())
                {
                    getLexicalSibScore_(concepts[i], concepts[j], sibScore);

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
    
    void getLexicalSibScore_(const ConceptInfo& concept1, const ConceptInfo& concept2, double& lexicalScore)
    {
        for(uint32_t ki=0;ki<concept1.termIdList_.size();ki++)
            for(uint32_t kj=0;kj<concept2.termIdList_.size();kj++)
            {
    //            uint16_t weight;
                if(ki==kj&&concept1.termIdList_[ki]==concept2.termIdList_[kj])
                {
                    lexicalScore+=1;
                }
            }
    }
    
    void getSupportSibScore_(const ConceptInfo& concept1, const ConceptInfo& concept2, double& supportScore)
    {
        boost::dynamic_bitset<> common = concept1.docInvert_ & concept2.docInvert_;
        if((float)common.count()/concept1.docInvert_.count()>0.9&&concept2.docInvert_.count()>=3)
            supportScore=log(sqrt(concept1.docInvert_.count()+1));
        else
            supportScore=0;
    }
    
    
    void doHierarchical_(
        const Parameters& params,
        const std::vector<uint32_t>& queryTermIdList,
        const std::vector<ConceptInfo >& concepts,
        izenelib::am::rde_hash<uint64_t,double>& containess,
        izenelib::am::rde_hash<uint64_t,double>& sibship,
        const std::vector<uint32_t>& docIdList,
        OutputType& output)
    {

        std::vector<ConceptNode* > allConceptNode;
        ConceptNodeValue topNodeValue(docIdList.size(), queryTermIdList);
        topNodeValue.conceptId_ = 0;// defined as 0 firstly
        ConceptNode* top = new ConceptNode ( topNodeValue ) ;
        allConceptNode.push_back ( top );

        std::vector<std::vector<double> > dValue ( 1,std::vector<double> ( concepts.size(),0.0 ) );
        //keep dValue.size()==allTmpNode.size()
        for ( std::size_t i=0;i<concepts.size();i++ )
        {
            dValue[0][i] = computeScore_ ( *top,concepts[i],containess );
        }
        boost::dynamic_bitset<> column_selected ( concepts.size() );
        boost::dynamic_bitset<> row_selected ( 1 );
        std::list<ConceptInfo > acquiredConcepts;
        while ( true )
        {
            std::pair<uint32_t, uint32_t> max_pos;
            double max_value = 0.0;
            bool foundChild = false;
            findChild_(params, concepts,  row_selected, column_selected, dValue,
                    max_value, max_pos, foundChild);
            if ( foundChild )
            {
                if(!insertNode_(params, concepts, docIdList, containess,max_pos.second
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
        BOOST_FOREACH(ConceptNode* lNode, allConceptNode)
        {
            delete lNode;
        }

    }
    
    double computeScore_(
        const ConceptNode& node,
        const ConceptInfo& conceptInfo,
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
            pairKey=idmlib::util::make64UInt(node.value_.index_,conceptInfo.index_);
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
            uint32_t i_incease = ( node.value_.docSupp_ & conceptInfo.docInvert_ ).count() ;

            if ( i_incease>0 )
            {
                increase = (double)(i_incease ) / node.value_.docSupp_.count();

                value = std::sqrt( dContain * increase);
                float match = 0;
                if(conceptInfo.termIdList_.size()>=node.value_.termIdList_.size())
                {
                    BOOST_FOREACH(uint32_t term_id, node.value_.termIdList_)
                    {
                        BOOST_FOREACH(uint32_t _term_id, conceptInfo.termIdList_)
                        {
                            if(term_id == _term_id) match+=0.6;
                        }

                    }
                }

                if(match>0)
                {
                    float fMatch = match;
                    if( conceptInfo.name_.length()>0 )
                    {
                        if( !conceptInfo.name_.isChineseChar(0) )
                        {
                            fMatch *= 5;
                        }
                    }
                    value=value*log(fMatch+2);

                }

                if(node.level_==1)
                {
                    if(conceptInfo.termIdList_.size()>2)
                    {
                        value *= 0.98;
                    }
                }

            }

        }
        return value;
    }

    void findChild_(
            const Parameters& params,
            const std::vector<ConceptInfo >& concepts,
            const boost::dynamic_bitset<>& row_selected,
            const boost::dynamic_bitset<>& column_selected,
            const std::vector<std::vector<double> >& dValue,
            double& max_value,
            std::pair<uint32_t, uint32_t>& max_pos,
            bool& foundChild)
    {
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
                    _score *= (concepts[j].score_+0.1);
    //                _score *= sqrt(labels[j].label_score_+0.1);
                    if ( _score> max_value )
                    {
                        max_value = _score;
                        max_pos = std::make_pair ( i,j );
                        foundChild = true;
                    }
                }
            }
        }

    }

    bool insertNode_(
            const Parameters& params,
            const std::vector<ConceptInfo >& concepts,
            const std::vector<uint32_t>& docIdList,
            izenelib::am::rde_hash<uint64_t,double>& containess,
            uint32_t index,
            uint32_t parent_index,
            std::vector<ConceptNode* >& allConceptNode,
            boost::dynamic_bitset<>& row_selected,
            boost::dynamic_bitset<>& column_selected,
            std::list<ConceptInfo >& acquiredConcepts,
            std::vector<std::vector<double> >& dValue,
            OutputType& output
            )
    {
        column_selected.set ( index );
        if( distance_type::isDuplicate(concepts[index], acquiredConcepts) )
            return false;
    //     std::string _str;
    //     labels[label_id].name_.convertString(_str, izenelib::util::UString::UTF_8);
    //     std::cout<<"inserting "<<label_id<<" "<<_str<<std::endl;
        acquiredConcepts.push_back(concepts[index]);
        ConceptNode* parent = allConceptNode[parent_index];
        boost::shared_ptr<ClusterRep> cluster ( new ClusterRep() );
    //             cluster->termIdList_ = labels[label_id].termIdList_;
        cluster->name_ = concepts[index].name_;
        cluster->conceptId_ = concepts[index].conceptId_;
        boost::dynamic_bitset<> tmpDocInvert = concepts[index].docInvert_;
        if ( parent->level_==0 ) //top level
        {
            cluster->set_doc_contain ( docIdList,tmpDocInvert );
            output.push_back ( cluster );
        }
        else
        {
            boost::shared_ptr<ClusterRep>& bindCluster = parent->clusterRep_;
            tmpDocInvert &= bindCluster->docInvert_;
            cluster->set_doc_contain ( docIdList,tmpDocInvert );
            bindCluster->children_.push_back ( cluster );
        }
    //             cluster->setIDManager(idManager_);
        ConceptNodeValue nodeValue(index, tmpDocInvert, concepts[index].termIdList_);
        nodeValue.conceptId_ = concepts[index].conceptId_;
        ConceptNode* node = new ConceptNode ( nodeValue,parent ) ;
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
        boost::dynamic_bitset<> seedInvert ( concepts[index].docInvert_.size() );
        getSeedInvert_(seedInvert);
        seedInvert &= concepts[index].docInvert_;
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
                    dValue[parent_index][i] = computeScore_ ( *parent,concepts[i],containess );
            }
        }
        //add new value
        dValue.push_back ( std::vector<double> ( concepts.size(),0.0 ) );
        if ( !row_selected[dValue.size()-1] )
        {
            for ( std::size_t i=0;i<concepts.size();i++ )
            {
                if ( column_selected[i] ) continue;
                double _score = computeScore_ ( *node,concepts[i],containess );
                dValue[dValue.size()-1][i] = _score;
            }
        }
        return true;
    }

//     void findSibling_(
//             const std::vector<TgLabelInfo >& labels,
//             izenelib::am::rde_hash<uint32_t,double>& sibship,
//             const boost::dynamic_bitset<>& column_selected,
//             const std::vector<TgLabelNode* >& allLabelNode,
//             unsigned int label_id,
//             unsigned int parent_id,
//             std::vector<unsigned int>& sib_label_ids)
//     {
//         uint32_t extension_num=0;
//         std::vector<std::pair<double, labelid_t> > score_labelId;
//         for ( std::size_t j=0;j<labels.size();j++ )
//         {
//             if ( column_selected[j] ) continue;
//             uint32_t pairKey=int_hash::mix_ints(label_id,j,0);
//             double weight;
//             if(sibship.get(pairKey, weight))
//             {
//                 if(LabelDistance::getDistance(labels[label_id].name_, labels[j].name_)<2)
//                     continue;
//                 TgLabelNode* parent = allLabelNode[parent_id];
//                 extension_num=parent->level_+1;
//                 boost::dynamic_bitset<> common = parent->value_.doc_supp_ & labels[j].doc_invert_;
//                 if (common.count()>2)
//                 {
//                     score_labelId.push_back(std::make_pair(weight*sqrt((float)common.count()/parent->value_.doc_supp_.count()),j));
//                 }
//             }
//         }
//         if(score_labelId.size()>0)
//         {
//             std::sort(score_labelId.begin(),score_labelId.end(),std::greater<std::pair<double, unsigned int> >());
//             for(unsigned int i=0;i<score_labelId.size();i++)
//             {
//     //             std::cout<<"insert sibling for ";
//     //             labels[label_id].name_.displayStringValue(izenelib::util::UString::UTF_8);
//     //             std::cout<<" : ";
//     //             labels[score_labelId[i].second].name_.displayStringValue(izenelib::util::UString::UTF_8);
//     //             std::cout<<" "<<score_labelId[i].first;
//     //             std::cout<<std::endl;
// 
//                 sib_label_ids.push_back(score_labelId[i].second);
//                 if(sib_label_ids.size()==extension_num)
//                     break;
//             }
//         }
// 
//     }

    void getSeedInvert_ (boost::dynamic_bitset<>&  seedInvert)
    {
        for(uint32_t i=0;i<seedInvert.size();i++)
        {

            if(i%2==0)
                seedInvert.set(i);

        }

    }
    
private:
    ConceptManagerType* conceptManager_;
    idmlib::util::IDMAnalyzer* analyzer_;
    bool isQuiet_;            
    
    
};

NS_IDMLIB_CC_END

#endif 
