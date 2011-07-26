#ifndef IDMLIB_QC_QCTYPES_H_
#define IDMLIB_QC_QCTYPES_H_

#include <string>
#include <vector>
#include <util/ustring/UString.h>

NS_IDMLIB_QC_BEGIN

typedef izenelib::util::UCS2Char Unigram;
typedef std::pair<Unigram, Unigram> Bigram;
typedef std::pair<Bigram, Unigram> Trigram;
typedef std::vector<Unigram> Ngram;

template <class T>
struct ScoreItem
{
    T value;
    double score;
    
    bool operator<( const ScoreItem<T>& other) const
    {
        return score> other.score;
    }
};

typedef ScoreItem<izenelib::util::UString> CandidateResult;


struct ViterbiItem
{
    izenelib::util::UCS2Char text;
    double score;
    uint32_t pre_index;
};

//for trigram viterbi
struct ViterbiItemT
{
    izenelib::util::UString text;
    double score;
    uint32_t pre_index;
    
    const izenelib::util::UCS2Char& GetLastChar() const
    {
        return text[text.length()-1];
    }
    
};

NS_IDMLIB_QC_END

#endif
