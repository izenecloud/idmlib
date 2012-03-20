#ifndef IDMLIB_ISEINDEX_HPP
#define IDMLIB_ISEINDEX_HPP

#include "lshindex.hpp"
#include "extractor.hpp"

#include <am/leveldb/Table.h>

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
};

class IseIndex
{
    typedef lshkit::Tail<lshkit::RepeatHash<lshkit::GaussianLsh> > LshFuncType;
    typedef LSHIndex<LshFuncType, unsigned> LshIndexType;
    //typedef MultiProbeLSHIndex<unsigned> LshIndexType;
    typedef izenelib::am::leveldb::Table<unsigned, std::string > ImageMetaStorageType;

    std::string lshHome_;

    std::string sketchHome_;

    std::string imgMetaHome_;

    LshIndexType lshIndex_;

    std::vector<std::pair<Sketch,unsigned> > sketches_;

    Extractor extractor_;

    ImageMetaStorageType imgMetaStorage_;

public:
    IseIndex(const std::string& homePath);

    ~IseIndex();

    void Reset(const IseOptions& options);

    void Save();

    bool Insert(const std::string& imgPath);

    void Search(const std::string& queryImgpath, std::vector<std::string>& results);

private:
    void ResetLSH_(const IseOptions& options);

    void SaveLSH_();

    void LoadSketch_();

    void SaveSketch_();

    void DoLSHSearch_(std::vector<Sift::Feature>& sift, std::vector<unsigned>& results);

    void DoSketchSearch_(std::vector<Sift::Feature>& sift, std::vector<unsigned>& results);

    void DoPostFiltering_(std::vector<unsigned>& in, std::vector<unsigned>& out);

};

}}

#endif

