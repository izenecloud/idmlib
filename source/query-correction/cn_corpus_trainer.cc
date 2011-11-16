#include <idmlib/query-correction/cn_corpus_trainer.h>
#include <iostream>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include <am/tokyo_cabinet/tc_hash.h>
using namespace idmlib::qc;


CnCorpusTrainer::CnCorpusTrainer(idmlib::util::IDMAnalyzer* analyzer)
:analyzer_(analyzer),unigram_count_(0)
{
}

// void CnCorpusTrainer::LoadPinyinFile(const std::string& file)
// {
// }

void CnCorpusTrainer::Train(const std::string& raw_text_input, const std::string& output_dir)
{
    std::ifstream ifs(raw_text_input.c_str());
    std::string line;
    uint64_t count = 0;
    while( getline( ifs, line) )
    {
        count++;
        if(count%1000==0)
        {
            std::cout<<"Processed "<<count<<" lines"<<std::endl;
        }
        boost::algorithm::trim(line);
        izenelib::util::UString ustr(line, izenelib::util::UString::UTF_8);
        std::vector<idmlib::util::IDMTerm> term_list;
        analyzer_->GetTermList(ustr, term_list);

        Bigram pre_bigram;
        bool pre_bigram_set = false;
        izenelib::util::UString pre_text;
        uint32_t pre_position = 0;
        char pre_tag;
        for(uint32_t i=0;i<term_list.size();i++)
        {
            bool pb_set = false;
            const idmlib::util::IDMTerm& term = term_list[i];
            if(term.tag=='C')
            {
                FindUnigram_( term.text[0] );
                if(i>0 && (term.position == pre_position+1) && pre_tag=='C' )
                {
                    Bigram b(pre_text[0], term.text[0]);
                    FindBigram_(b);
                    if(pre_bigram_set)
                    {
                        Trigram t(pre_bigram, term.text[0]);
                        FindTrigram_(t);
                    }
                    pre_bigram = b;
                    pre_bigram_set = true;
                    pb_set = true;
                }
            }
            pre_text = term.text;
            pre_position = term.position;
            pre_tag = term.tag;
            if(!pb_set) pre_bigram_set = false;
        }
    }
    ifs.close();
    //output
    boost::filesystem::create_directories(output_dir);
    std::string output_file = output_dir+"/trans_prob.txt";
    std::ofstream ofs(output_file.c_str());

    double bigram_threshold = 0.0015;
    double trigram_threshold = 0.00015;
    boost::unordered_map<Bigram, double> bigram_prob;
//     izenelib::am::tc_hash<Ngram, double> ngram_tc(output_dir+"/ngram_tc");
//     ngram_tc.open();

    //output unigram
    {
        boost::unordered_map<Unigram, uint64_t>::iterator it = unigram_.begin();
        while(it!=unigram_.end())
        {
            izenelib::util::UString text;
            text += it->first;
            double prob = (double) it->second/ unigram_count_;
            std::string str;
            text.convertString(str, izenelib::util::UString::UTF_8);
            ofs<<str<<"\t"<<prob<<std::endl;
            ++it;
        }
    }


    //output bigram
    {
        boost::unordered_map<Bigram, uint64_t>::iterator it = bigram_.begin();
        while(it!=bigram_.end())
        {
            izenelib::util::UString text;
            text += it->first.first;
            text += it->first.second;
            boost::unordered_map<Unigram, uint64_t>::iterator uit = unigram_.find(it->first.first);
            double trans_prob = (double) it->second / uit->second;
            std::string str;
            text.convertString(str, izenelib::util::UString::UTF_8);
            bigram_prob.insert( std::make_pair( it->first, trans_prob) );
            if( trans_prob >= bigram_threshold )
            {
                ofs<<str<<"\t"<<trans_prob<<std::endl;
            }

//             Ngram ngram(2);
//             ngram[0] = it->first.first;
//             ngram[1] = it->first.second;
//             ngram_tc.insert(ngram, trans_prob);
            ++it;
        }
    }

    //output trigram
    {
        boost::unordered_map<Trigram, uint64_t>::const_iterator it = trigram_.begin();
        while(it!=trigram_.end())
        {
            const Trigram& trigram = it->first;
            izenelib::util::UString text;
            text += trigram.first.first;
            text += trigram.first.second;
            text += trigram.second;
            boost::unordered_map<Bigram, uint64_t>::iterator bit = bigram_.find(trigram.first);
            double trans_prob = (double) it->second / bit->second;
            std::string str;
            text.convertString(str, izenelib::util::UString::UTF_8);

            boost::unordered_map<Bigram, double>::iterator bpit = bigram_prob.find(trigram.first);
            double bt_prob = bpit->second * trans_prob;
            if(bt_prob >= trigram_threshold)
            {
                ofs<<str<<"\t"<<trans_prob<<std::endl;
            }

//             Ngram ngram(3);
//             ngram[0] = trigram.first.first;
//             ngram[1] = trigram.first.second;
//             ngram[2] = trigram.second;
//             ngram_tc.insert(ngram, trans_prob);

            ++it;
        }
    }
//     ngram_tc.close();
    ofs.close();
}

void CnCorpusTrainer::FindUnigram_(const Unigram& u)
{
    boost::unordered_map<Unigram, uint64_t>::iterator it = unigram_.find(u);
    if(it==unigram_.end())
    {
        unigram_.insert(std::make_pair(u, 1) );
    }
    else
    {
        it->second += 1;
    }
    ++unigram_count_;
}

void CnCorpusTrainer::FindBigram_(const Bigram& b)
{
    boost::unordered_map<Bigram, uint64_t>::iterator it = bigram_.find(b);
    if(it==bigram_.end())
    {
        bigram_.insert(std::make_pair(b, 1) );
    }
    else
    {
        it->second += 1;
    }
}

void CnCorpusTrainer::FindTrigram_(const Trigram& t)
{
    boost::unordered_map<Trigram, uint64_t>::iterator it = trigram_.find(t);
    if(it==trigram_.end())
    {
        trigram_.insert(std::make_pair(t, 1) );
    }
    else
    {
        it->second += 1;
    }
}
