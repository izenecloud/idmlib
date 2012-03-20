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

DRUM_END_NAMESPACE

namespace idmlib{ namespace ise{

class ProbSimMatch
{
public:
    struct Parameter
    {
        typedef unsigned key_t;

        static const unsigned p = 20;
        static const unsigned h = 3;
        static const unsigned k = 17;
    };

    typedef Parameter::key_t key_t;

    typedef izenelib::drum::Drum<
        Sketch,
        std::set<key_t>,
        char,
        izenelib::am::leveldb::TwoPartComparator,
        izenelib::am::leveldb::Table
    > SketchToImgIdType;

    ProbSimMatch(const std::string& path);

    ~ProbSimMatch();

    void Init();

    void Insert(key_t key, const std::vector<Sketch>& sketches);

    void Delete(key_t key);

    void BFSearch(const std::vector<Sketch>& sketches, std::vector<key_t>& results) const;

    void PSMSearch(
            const std::vector<Sift::Feature>& sifts,
            const std::vector<Sketch>& sketches,
            std::vector<key_t>& results) const;

    void Finish();

private:
    void LoadSketches_(std::istream& iar);

    void SaveSketches_(std::ostream& oar);

    unsigned CalcHammingDist_(const Sketch& s1, const Sketch& s2) const;

    void GenTableIds_(const std::vector<float>& components, std::vector<unsigned>& table_ids) const;

private:
    std::string sketch_path_;
    std::string drum_path_;

    std::vector<std::vector<Sketch> > sketches_;
    boost::shared_ptr<SketchToImgIdType> sketch_to_imgid_;
};

}}

MAKE_MEMCPY_TYPE(idmlib::ise::Sketch)
MAKE_FEBIRD_SERIALIZATION(std::set<idmlib::ise::ProbSimMatch::key_t>)

#endif
