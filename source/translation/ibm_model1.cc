#include <idmlib/translation/ibm_model1.h>
#include <boost/unordered_map.hpp>
#include <iostream>
using namespace idmlib::tl;

IbmModel1::IbmModel1(uint32_t iteration)
:iteration_(iteration)
{
}

boost::unordered_map<std::pair<uint32_t, uint32_t>, double>* IbmModel1::Train(uint32_t source_num, const std::vector<SentencePair>& corpus)
{
    std::cout<<"IBM MODEL 1 starting.."<<std::endl;
    ///result gives P(source | target)
    boost::unordered_map<TermPair, double>* result = new boost::unordered_map<TermPair, double>();
    uint32_t iteration = 0;
    while(true)
    {
        ++iteration;
        if(iteration>iteration_) break;
        std::cout<<"IBM MODEL 1 iteration "<<iteration<<std::endl;
        boost::unordered_map<TermPair, double> count;
        boost::unordered_map<Term, double> total;
        for(std::size_t i=0;i<corpus.size();i++)
        {
            boost::unordered_map<Term, double> total_s;
            const Sentence& source = corpus[i].first;
            const Sentence& target = corpus[i].second;
            for(std::size_t i=0;i<source.size();i++)
            {
                Term source_term = source[i];
                boost::unordered_map<Term, double>::iterator total_s_it = total_s.insert(std::make_pair(source_term, 0.0) ).first;
                for(std::size_t j=0;j<target.size();j++)
                {
                    TermPair term_pair(source_term, target[j]);
                    
                    boost::unordered_map<TermPair, double>::iterator it = result->find(term_pair);
                    if( it == result->end())
                    {
                        it = result->insert(std::make_pair(term_pair, 1.0/source_num)).first;
                    }
                    total_s_it->second += it->second;
                }
            }
            
            for(std::size_t i=0;i<source.size();i++)
            {
                Term source_term = source[i];
                for(std::size_t j=0;j<target.size();j++)
                {
                    Term target_term = target[j];
                    TermPair term_pair(source_term, target_term);
                    
                    boost::unordered_map<TermPair, double>::iterator count_it = count.find(term_pair);
                    if(count_it == count.end())
                    {
                        count_it = count.insert(std::make_pair( term_pair, 0.0) ).first;
                    }
                    
                    boost::unordered_map<Term, double>::iterator total_it = total.find(target_term);
                    if(total_it == total.end())
                    {
                        total_it = total.insert(std::make_pair( target_term, 0.0) ).first;
                    }
                    
                    boost::unordered_map<TermPair, double>::iterator it = result->find(term_pair);
                    double t = it->second;

                    boost::unordered_map<Term, double>::iterator total_s_it = total_s.find(source_term);
                    double add = t/ (total_s_it->second);
                    count_it->second += add;
                    total_it->second += add;
                }
            }
        }
        
        boost::unordered_map<TermPair, double>::iterator it = result->begin();
        while(it!=result->end())
        {
            TermPair term_pair = it->first;
            boost::unordered_map<TermPair, double>::iterator count_it = count.find(term_pair);
            boost::unordered_map<Term, double>::iterator total_it = total.find(term_pair.second);
            it->second = count_it->second / total_it->second;
            ++it;
        }
    }
    std::cout<<"IBM MODEL 1 finished."<<std::endl;
    return result;
}
