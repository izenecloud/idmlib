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

#include <idmlib/lshkit.h>

namespace idmlib{ namespace ise{

///-----------------------------------------------------------------------
// forward declarations
class LshBucket;
class LshBuckets;
class LshTable;
class LshTables;

typedef uint64_t BucketId_t;
typedef std::vector<BucketId_t> BucketIdList;

///-----------------------------------------------------------------------
///Options for the LSH
struct LshOptions
{
    unsigned dim;   
    /// Window size.
    float W;
    ///number of tables
    unsigned ntables;
    ///number of hash functions
    unsigned nfuncs;
    ///number of dimensions for the data
    unsigned ndims;
    /// max of each indiv. hash function
    int hwidth;
    /// set fixed size for tables so that we don't search for buckets
    /// zero if size not fixed
    unsigned tablesize;

    lshkit::DefaultRng rng;

    LshOptions()
    {
        dim = 128;
        W = 8;
        ntables = 5;
        nfuncs = 10;
        hwidth = 0;
        tablesize = 0;
    }

    friend std::ostream& operator<<(std::ostream& s, LshOptions& o);
    friend std::istream& operator>>(std::istream& s, LshOptions& o);
};

template <typename LSH, typename CHUNK = unsigned char>
class LSHFuncs
{
    BOOST_CONCEPT_ASSERT((lshkit::DeltaLshConcept<LSH>));

    unsigned dim_;
    unsigned w_;
    std::vector<LSH> lsh_;

public:
    /// LSH parameter
    typedef typename LSH::Parameter Parameter;
    /// Domain of LSH 
    typedef typename LSH::Domain Domain; 
    /// Number of bits in each CHUNK.
    static const unsigned CHUNK_BIT = sizeof(CHUNK) * 8; // #bits in CHUNK

    LSHFuncs() {}

    void Init(LshOptions& opt)
    {
        dim_ = opt.nfuncs;
        w_ = opt.W;
        lsh_.resize(dim_);
        Parameter lshParam;
        lshParam.dim = opt.dim;
        lshParam.W = opt.W;
        for (unsigned i = 0; i < lsh_.size(); ++i) lsh_[i].reset(lshParam, opt.rng);
    }

    /// operator [] to access the member functions
    LSH& operator[](unsigned i)
    {
        return lsh_[i];
    }

    unsigned GetW()
    {
        return w_;
    }

    unsigned Size()
    {
        return lsh_.size();
    }

    /// Serialize the sketcher.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & dim_;
        ar & lsh_;
    }

    void Load (std::istream &is) {
        serialize(is, 0);
    }

    void Save (std::ostream &os) {
        serialize(os, 0);
    }
};

///-----------------------------------------------------------------------
///point list and iterator
typedef std::vector<uint32_t> LshPointList;
typedef LshPointList::iterator LshPointIt;

///Lsh bucket
class LshBucket
{
public:
    /// Bucket id
    BucketId_t id_;

    LshBucket(BucketId_t id)
        :id_(id)
    {}

    friend std::ostream& operator<<(std::ostream& s, LshBucket& b);
    friend std::istream& operator>>(std::istream& s, LshBucket& b);

    bool operator==(const LshBucket& b)
    {
        return (id_ == b.id_);
    }

    bool operator<(const LshBucket& b)
    {
        return (id_ < b.id_);
    }

    LshPointList plist_;

    LshBucket() {}

    ~LshBucket() {}

    void Insert(uint32_t pid)
    {
        plist_.push_back(pid);
    }

    unsigned Size()
    {
        return plist_.size();
    }

    void Resize(unsigned i)
    {
        plist_.resize(i);
    }

    uint32_t operator[](unsigned i)
    {
        return plist_[i];
    }
};

typedef std::vector<LshBucket> LshBucketList;
typedef LshBucketList::iterator LshBucketIt;
typedef std::vector<LshBucketList> LshBucketListList;
typedef LshBucketListList::iterator LshBucketListIt;


///-----------------------------------------------------------------------
class LshBuckets
{
public:
    LshBucketListList buckets_;

    LshBuckets() {}

    ~LshBuckets() { }

    unsigned Size()
    {
        return buckets_.size();
    }

    ///resize list
    void Resize(unsigned s)
    {
        buckets_.resize(s);
    }

    LshBucketIt Begin(unsigned i=0)
    {
        return buckets_[i].begin();
    }

    LshBucketIt End(unsigned i=0)
    {
        return buckets_[i].end();
    }

    void Clear()
    {
        LshBucketListList().swap(buckets_);
    };

    /// find a specific bucket in the list
    ///
    /// b     - the input bucket
    /// bit   - return iterator to the found one, or position to insert in
    /// id    - the index of the bucket list (in case of fixed bucket lis)
    ///
    /// output returns whether found or not
    ///
    bool Find(const LshBucket& b, LshBucketIt& bit, uint32_t& id);

    /// insert a bucket into the list
    ///
    /// b     - the bucket
    /// bit   - the bucket iterator to insert at
    /// id    - the id of the bucket list to insert in, in case of fixed size
    ///
    /// output  - an iterator to the inserted one
    LshBucketIt Insert(LshBucket& b, LshBucketIt bit, uint32_t id=0);

    /// stream overloads
    friend std::ostream& operator<<(std::ostream& s, LshBuckets& b);
    friend std::istream& operator>>(std::istream& s, LshBuckets& b);
};

///-----------------------------------------------------------------------
typedef LSHFuncs<lshkit::DeltaLSB<lshkit::GaussianLsh>  > LshFuncsType;
typedef typename LshFuncsType::Domain Domain; 
class LshTable
{
public:
    LshFuncsType funcs_;

    LshBuckets buckets_;

    LshTable() {}

    ~LshTable() {}

    friend std::ostream& operator<<(std::ostream& s, LshTable& t);
    friend std::istream& operator>>(std::istream& s, LshTable& t);

    void Init(LshOptions& opt);

    BucketId_t GetId(Domain data);

    void InsertPoint(Domain data, uint32_t pid);

    void SearchPoint(Domain data, LshPointList& pl);
};

class LshTables
{
public:
    /// the list of tables
    std::vector<LshTable> tables_;

    LshTables() {}

    ~LshTables() {}

    friend std::ostream& operator<<(std::ostream& s, LshTables& t);
    friend std::istream& operator>>(std::istream& s, LshTables& t);

    void Clear()
    {
        std::vector<LshTable>().swap(tables_);
    }

    unsigned Size()
    {
        return tables_.size();
    }

    void Resize(unsigned i)
    {
        tables_.resize(i);
    }

    LshTable& operator[](unsigned i)
    {
        return tables_[i];
    }

    void InsertPoint(Domain data, uint32_t id);

    void SearchPoint(Domain data, LshPointList& pls);
};


///-----------------------------------------------------------------------
///LSHIndex structure
class LshIndex
{
public:
    LshOptions opt_;

    LshTables lshTables_;

    LshIndex(const LshOptions& opt)
        :opt_(opt)
    {
        Init();
    }

    ~LshIndex() {}

    /// stream overloads
    friend std::ostream& operator<<(std::ostream& s, LshIndex& l);
    friend std::istream& operator>>(std::istream& s, LshIndex& l);

    void Clear()
    {
        lshTables_.Clear();
    }
    /// initialize
    void Init();
    /// load and save
    void Load(const std::string& filename);
    void Save(const std::string& filename);
    /// insert points into the LshIndex
    ///
    /// data  - pointer to the points to insert
    /// id  - image identifier
    void InsertPoint(Domain data, uint32_t id);
    /// look for points in the LshIndex
    ///
    /// point  - pointer to the point to search for
    /// pls  - an array of point lists to hold data
    void SearchPoint(Domain data, LshPointList& pls);
};

}}
#endif /*IDMLIB_ISE_LSHINDEX_HPP*/
