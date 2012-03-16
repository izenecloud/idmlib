#ifndef IDMLIB_ISEINDEX_HPP
#define IDMLIB_ISEINDEX_HPP

#include "lshindex.hpp"
#include "extractor.hpp"

namespace idmlib{ namespace ise{

struct IseOptions
{
    ///Lsh table size
    unsigned range;
    ///Lsh repeat size
    unsigned repeat;
    ///Lsh window size
    unsigned w;
    ///Lsh dimensional
    unsigned dim;
    ///Lsh tables 
    unsigned ntables;
    ///Lsh index path
    std::string home;
};

class IseIndex
{
    typedef lshkit::Tail<lshkit::RepeatHash<lshkit::GaussianLsh> > LshFuncType;
    typedef LSHIndex<LshFuncType, unsigned> LshIndexType;

    LshIndexType lshIndex_;

    Extractor extractor_;

    std::string home_;
public:
    IseIndex(const IseOptions& options);

    ~IseIndex();

    void Insert(const std::string& imgPath);

};

}}

#endif

