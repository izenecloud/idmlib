#ifndef IDMLIB_ProbSimMatch_HPP
#define IDMLIB_ProbSimMatch_HPP

#include "sift.hpp"

#include <am/leveldb/Table.h>
#include <3rdparty/am/drum/drum.hpp>


DRUM_BEGIN_NAMESPACE

template <>
struct BucketIdentifier<idmlib::ise::Sketch>
{
    static std::size_t Calculate(idmlib::ise::Sketch const& key, std::size_t const& num_bucket_bits)
    {
        return static_cast<std::size_t>(key.desc[0] >> (64 - num_bucket_bits));
    }
};

template <>
struct BucketIdentifier<idmlib::ise::SimHash>
{
    static std::size_t Calculate(idmlib::ise::SimHash const& key, std::size_t const& num_bucket_bits)
    {
        return static_cast<std::size_t>(key.desc[0] >> (64 - num_bucket_bits));
    }
};

DRUM_END_NAMESPACE

namespace idmlib{ namespace ise{

class BruteForce
{
public:
    typedef izenelib::drum::Drum<
        Sketch,
        std::set<unsigned>,
        char,
        izenelib::am::leveldb::TwoPartComparator,
        izenelib::am::leveldb::Table
    > SketchToImgIdType;

    BruteForce(const std::string& path);

    ~BruteForce();

    void Init();

    void Insert(unsigned key, const std::vector<Sketch>& sketches);

    void Delete(unsigned key);

    void Search(const std::vector<Sketch>& sketches, std::vector<unsigned>& results) const;

    void Finish();

private:
    void LoadSketches_(std::istream& iar);

    void SaveSketches_(std::ostream& oar);

private:
    std::string sketch_path_;
    std::string drum_path_;

    std::vector<Sketch> sketches_;
    boost::shared_ptr<SketchToImgIdType> sketch_to_imgid_;
};

class ProbSimMatch
{
public:
    struct Parameter
    {
        static const unsigned p = 20;
        static const unsigned h = 3;
        static const unsigned k = 17;
    };

    typedef izenelib::drum::Drum<
        SimHash,
        std::set<unsigned>,
        char,
        izenelib::am::leveldb::TwoPartComparator,
        izenelib::am::leveldb::Table
    > SimHashToImgIdType;

    ProbSimMatch(const std::string& path);

    ~ProbSimMatch();

    void Init();

    void Insert(unsigned key, const std::vector<Sift::Feature>& sifts);

    void Delete(unsigned key);

    void Search(const std::vector<Sift::Feature>& sifts, std::vector<unsigned>& results) const;

    void Finish();

private:
    void LoadSimHashes_(std::istream& iar);

    void SaveSimHashes_(std::ostream& oar);

    void GenSimHash_(const std::vector<float>& components, SimHash& simhash) const;

    void GenTableIds_(const std::vector<float>& components, SimHash& simhash, std::vector<unsigned>& table_ids) const;

private:
    std::string simhash_path_;
    std::string drum_path_;

    std::vector<std::vector<SimHash> > simhashes_;
    boost::shared_ptr<SimHashToImgIdType> simhash_to_imgid_;
};

}}

MAKE_MEMCPY_TYPE(idmlib::ise::Sketch)
MAKE_MEMCPY_TYPE(idmlib::ise::SimHash)
MAKE_FEBIRD_SERIALIZATION(std::set<unsigned>)

#endif
