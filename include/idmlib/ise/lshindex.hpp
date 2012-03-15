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
    BOOST_CONCEPT_ASSERT((lshkit::LshConcept<LSH>));
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
      */
    template <typename Engine>
    void Init (const Parameter &param, Engine &engine, unsigned L)
    {
        BOOST_VERIFY(lshs_.size() == 0);
        BOOST_VERIFY(tables_.size() == 0);
        lshs_.resize(L);
        tables_.resize(L);

        for (unsigned i = 0; i < L; ++i) {
            lshs_[i].reset(param, engine);
            if (lshs_[i].getRange() == 0) {
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
        for (unsigned i = 0; i < L; ++i) {
            lshs_[i].serialize(ar, 0);
            unsigned l;
            ar & l;
            LSHTable &table = tables_[i];
            table.resize(l);
            for (;;) {
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
        for (unsigned i = 0; i < L; ++i) {
            lshs_[i].serialize(ar, 0);
            LSHTable &table = tables_[i];
            unsigned l = table.size();
            ar & l;
            unsigned idx, ll;
            for (unsigned j = 0; j < l; ++j) {
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
        for (unsigned i = 0; i < lshs_.size(); ++i) {
            unsigned index = lshs_[i](obj);
            Bucket& bucket = tables_[i][index];
            if(bucket.empty()||bucket.back() < point)
                bucket.push_back(point);
        }
    }

    void Search (Domain obj, std::vector<unsigned>& result) const
    {
        for (unsigned i = 0; i < lshs_.size(); ++i) {
            unsigned index = lshs_[i](obj);
            const Bucket& bucket = tables_[i][index];
            result.resize(result.size() + bucket.size());
            std::copy(bucket.begin(), bucket.end(), std::back_inserter(result));
        }
        std::sort(result.begin(), result.end());		
        result.resize(std::unique(result.begin(), result.end()) - result.begin());		
    }
};


}}
#endif /*IDMLIB_ISE_LSHINDEX_HPP*/
