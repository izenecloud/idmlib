///
/// @file ibm_model1.h
/// @brief ibm translation model 1
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-08-29
/// @date Updated 2011-08-29
///

#ifndef IDMLIB_TL_IBMMODEL1_H_
#define IDMLIB_TL_IBMMODEL1_H_

#include <vector>
#include "../idm_types.h"
#include <boost/unordered_map.hpp>

NS_IDMLIB_TL_BEGIN


/// use translation definition of Noise Channel Model like giza++.
/// for example, we want to translate french(unknow language) to english(known language)
/// then english is source(know language) and french is target(unknow language)
/// we want to calculate P(source | target).
class IbmModel1
{
    public:
        
        IbmModel1(uint32_t iteration);
        
        typedef uint32_t Term;
        typedef std::pair<Term, Term> TermPair;
        typedef std::vector<Term> Sentence;
        typedef std::pair<Sentence, Sentence> SentencePair;
        typedef boost::unordered_map<TermPair, double> Result;
        
        ///build p(source | target).
        /// sentence pair is  source, then target,( english, then french )
        boost::unordered_map<TermPair, double>* Train(uint32_t source_num, const std::vector<SentencePair>& corpus);
    
    private:
        
        uint32_t iteration_;
   
    
    
};

NS_IDMLIB_TL_END

#endif 
