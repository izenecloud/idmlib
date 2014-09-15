/**
* Build LSH index for SIFT image features.
* We don't store the raw SIFT points within the LSH tables,
* instead, we only store the image identifiers to save as much memory as possible
*/
#ifndef IDMLIB_ISE_LSHINDEX_HPP
#define IDMLIB_ISE_LSHINDEX_HPP

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <idmlib/lshkit.h>

namespace idmlib{ namespace ise{

using namespace lshkit;
template <typename LSH, typename KEY>
class LSHIndex
{
    //BOOST_CONCEPT_ASSERT((lshkit::LshConcept<LSH>));
public:
    typedef typename LSH::Parameter Parameter;
    typedef typename LSH::Domain Domain;
    typedef KEY Key;

protected:
    typedef std::vector<Key> Bucket;
    typedef std::vector<Bucket> LSHTable;
    std::vector<LSH> lshs_;
    std::vector<LSHTable > tables_;

public:
    LSHIndex() {}

    /**
      * @param param parameter of LSH function.
      * @param engine random number generator.
      * @param L number of hash table maintained.
      */
    template <typename Engine>
    void Init (const Parameter &param, Engine &engine, unsigned L)
    {
        BOOST_VERIFY(lshs_.size() == 0);
        BOOST_VERIFY(tables_.size() == 0);
        lshs_.resize(L);
        tables_.resize(L);
        for (unsigned i = 0; i < L; ++i)
        {
            lshs_[i].reset(param, engine);
            if (lshs_[i].getRange() == 0)
            {
                throw std::logic_error("LSH with unlimited range should not be used to construct an LSH index.  Use lshkit::Tail<> to wrapp the LSH.");
            }
            tables_[i].resize(lshs_[i].getRange());
        }
    }

    void Load (std::istream &ar)
    {
        unsigned L;
        ar & L;
        lshs_.resize(L);
        tables_.resize(L);
        for (unsigned i = 0; i < L; ++i)
        {
            lshs_[i].serialize(ar, 0);
            unsigned l;
            ar & l;
            LSHTable &table = tables_[i];
            table.resize(l);
            for (;;)
            {
                unsigned idx, ll;
                ar & idx;
                ar & ll;
                if (ll == 0) break;
                table[idx].resize(ll);
                ar.read((char *)&table[idx][0], ll * sizeof(Key));
            }
        }
    }

    void Save (std::ostream &ar)
    {
        unsigned L;
        L = lshs_.size();
        ar & L;
        for (unsigned i = 0; i < L; ++i)
        {
            lshs_[i].serialize(ar, 0);
            LSHTable &table = tables_[i];
            unsigned l = table.size();
            ar & l;
            unsigned idx, ll;
            for (unsigned j = 0; j < l; ++j)
            {
                if (table[j].empty()) continue;
                idx = j;
                ll = table[j].size();
                ar & idx;
                ar & ll;
                ar.write((char *)&table[j][0], ll * sizeof(Key));
            }
            idx = ll = 0;
            ar & idx;
            ar & ll;
        }
    }

    void Insert (Domain obj, Key point)
    {
        for (unsigned i = 0; i < lshs_.size(); ++i)
        {
            unsigned index = lshs_[i](obj);
            Bucket& bucket = tables_[i][index];
            if (bucket.empty() || bucket.back() < point)
                bucket.push_back(point);
        }
    }

    void Search (Domain obj, std::vector<unsigned>& result) const
    {
        for (unsigned i = 0; i < lshs_.size(); ++i)
        {
            unsigned index = lshs_[i](obj);
            const Bucket& bucket = tables_[i][index];
            result.insert(result.end(), bucket.begin(), bucket.end());
        }
    }
};

/// Multi-Probe LSH index.
template <typename KEY>
class MultiProbeLSHIndex: public LSHIndex<lshkit::MultiProbeLsh, KEY>
{
public:
    typedef LSHIndex<lshkit::MultiProbeLsh, KEY> Super;
    /**
     * Super::Parameter is the same as MultiProbeLsh::Parameter
     */
    typedef typename Super::Parameter Parameter;

private:

    Parameter param_;
    lshkit::MultiProbeLshRecallTable recall_;

public:
    typedef typename Super::Domain Domain;
    typedef KEY Key;

    /// Constructor.
    MultiProbeLSHIndex()
    {
    }

    /// Initialize MPLSH.
    /**
      * @param param parameters.
      * @param engine random number generator (if you are not sure about what to
      * use, then pass DefaultRng.
      * @param accessor object accessor (same as in LshIndex).
      * @param L number of hash tables maintained.
      */
    template <typename Engine>
    void Init (const Parameter &param, Engine &engine, unsigned L)
    {
        Super::Init(param, engine, L);
        param_ = param;
        // we are going to normalize the distance by window size, so here we pass W = 1.0.
        // We tune adaptive probing for KNN distance range [0.0001W, 20W].
        recall_.reset(lshkit::MultiProbeLshModel(Super::lshs_.size(), 1.0, param_.repeat, lshkit::Probe::MAX_T), 200, 0.0001, 20.0);
    }

    /// Load the index from stream.
    void Load (std::istream &ar)
    {
        Super::Load(ar);
        param_.serialize(ar, 0);
        recall_.load(ar);
    }

    /// Save to the index to stream.
    void Save (std::ostream &ar)
    {
        Super::Save(ar);
        param_.serialize(ar, 0);
        recall_.save(ar);
    }

    /// Query for K-NNs.
    /**
      * @param obj the query object.
      * @param scanner
      */
    void Search(Domain obj, std::vector<unsigned>& result)
    {
        std::vector<unsigned> seq;
        for (unsigned i = 0; i < Super::lshs_.size(); ++i)
        {
            Super::lshs_[i].genProbeSequence(obj, seq, lshkit::Probe::MAX_T);
            for (unsigned j = 0; j < seq.size(); ++j)
            {
                typename Super::Bucket &bucket = Super::tables_[i][seq[j]];
                result.insert(result.end(), bucket.begin(), bucket.end());
            }
        }
    }
};

}}
#endif /*IDMLIB_ISE_LSHINDEX_HPP*/
