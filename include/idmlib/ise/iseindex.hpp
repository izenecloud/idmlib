#ifndef IDMLIB_ISEINDEX_HPP
#define IDMLIB_ISEINDEX_HPP

#include "lshindex.hpp"

namespace idmlib{ namespace ise{

struct IseOptions
{
    ///Lsh table size
    unsigned range;
    unsigned repeat;
    ///Lsh window size
    unsigned w;
    ///Lsh dimensional
    unsigned dim;
    ///Lsh tables 
    unsigned ntables;
};

class IseIndex
{
    typedef lshkit::Tail<lshkit::RepeatHash<lshkit::GaussianLsh> > LshFuncType;
    typedef LSHIndex<LshFuncType, unsigned> LshIndexType;

    LshIndexType lshIndex_;

public:
    IseIndex(const IseOptions& options);

    ~IseIndex();


};

}}

#endif

