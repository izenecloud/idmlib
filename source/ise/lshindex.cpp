#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <idmlib/ise/lshindex.hpp>

namespace idmlib{ namespace ise{
////////////////////////////////////////////////
bool LshBuckets::Find(const LshBucket& b, LshBucketIt& bit, uint32_t& id)
{
    bool found;
    LshBucketList* blist;

    //get the index
    id = b.id_ % buckets_.size();
    //get the iterator of the bucket head
    blist = &(buckets_[id]);
    // binary search using lower_bound
    bit = std::lower_bound(blist->begin(), blist->end(), b);

    if (bit!=blist->end() && *bit == b)
        found = true;
    else
        found = false;

    return found;
}

LshBucketIt LshBuckets::Insert(LshBucket& b, LshBucketIt bit, uint32_t id)
{
    bit = buckets_[id].insert(bit, b);
    return bit;
}

////////////////////////////////////////////////
void LshTable::Init(LshOptions& opt)
{
    funcs_.Init(opt);
}

BucketId_t LshTable::GetId(Domain data)
{
    BucketId_t id = 0;
    for (unsigned i=0, s=funcs_.Size(); i<s; ++i)
    {
        unsigned val = funcs_[i](data);

        if (s>1)
            id = (id << funcs_.GetW()) | val;
        else
            id = val;
    }

    return id;
}

void LshTable::InsertPoint(Domain data, uint32_t pid)
{
    BucketId_t bid = GetId(data);

    // make a bucket and search for it
    LshBucket b(bid);
    bool found;
    LshBucketIt bit;
    uint32_t id;
    found = buckets_.Find(b, bit, id);

    //if not found, isnert it
    if (!found)
        bit = buckets_.Insert(b, bit, id);

    //now we have an iterator for the bucket, so insert the point in there
    bit->Insert(pid);
}

void LshTable::SearchPoint(Domain data, LshPointList& pl)
{
    //get the bucket id for that point
    BucketId_t bid = GetId(data);

    //search for this bucket
    LshBucket b(bid);
    bool found;
    LshBucketIt bit;
    uint32_t id;
    found = buckets_.Find(b, bit, id);

    //check if found
    if (found && !bit->plist_.empty())
    {
        pl.insert(pl.end(), bit->plist_.begin(), bit->plist_.end());
    }
}

////////////////////////////////////////////////
void LshTables::InsertPoint(Domain data, uint32_t id)
{
    for(unsigned i = 0; i < tables_.size(); ++i)
        tables_[i].InsertPoint(data, id);
}

void LshTables::SearchPoint(Domain data, LshPointList& pls)
{
    for(unsigned i = 0; i < tables_.size(); ++i)
        tables_[i].SearchPoint(data, pls);
}

////////////////////////////////////////////////
void LshIndex::Init()
{
    //create the tables
    lshTables_.Resize(opt_.ntables);

    for (unsigned i=0; i<lshTables_.Size(); ++i)
        lshTables_[i].Init(opt_);

    //allocate buckets if fixed size
    if (opt_.tablesize > 0)
        for (unsigned i=0; i<lshTables_.Size(); ++i)
        {
            lshTables_[i].buckets_.Resize(opt_.tablesize);
        }
}

void LshIndex::InsertPoint(Domain data, uint32_t id)
{
    lshTables_.InsertPoint(data, id);
}

void LshIndex::SearchPoint(Domain data, LshPointList& pls)
{
    lshTables_.SearchPoint(data, pls);
}

void LshIndex::Load(const std::string& filename)
{
    std::ifstream f;
    f.open(filename.c_str(), std::ifstream::binary);
    f >> *this;
    f.close();
}

void LshIndex::Save(const std::string& filename)
{
    std::ofstream f;
    f.open(filename.c_str(), std::ofstream::binary);
    f << *this;
    f.close();
}

////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& s, LshOptions& o)
{
    s.write((char*) &o, sizeof(LshOptions));
    return s;
}

std::istream& operator>>(std::istream& s, LshOptions& o)
{
    s.read((char*) &o, sizeof(LshOptions));
    return s;
}

std::ostream& operator<<(std::ostream& s, LshBucket& b)
{
    //write the bucket id
    s << b.id_;
    //put the size of list of points
    unsigned z = b.Size();
    s.write((char*) &z, sizeof(z));
    //put the ponits
    for (unsigned i=0; i<z; ++i)
        s.write((char*) &(b.plist_[i]), sizeof(b.plist_[i]));
    return s;
}

std::istream& operator>>(std::istream& s, LshBucket& b)
{
    //read the bucket id
    s >> b.id_;
    //get the size of list of points
    unsigned z;
    s.read((char*) &z, sizeof(z));
    b.Resize(z);
    //get the ponits
    for (unsigned i=0; i<z; ++i)
        s.read((char*) &b.plist_[i], sizeof(b.plist_[i]));
    return s;
}

std::ostream& operator<<(std::ostream& s, LshBucketList& b)
{
    //write size
    unsigned z = b.size();
    s.write((char*) &z, sizeof(z));
    //write the list of buckets
    for (LshBucketIt bs=b.begin(), be=b.end(); bs!=be; bs++)
        s << *bs;
    return s;
}
std::istream& operator>>(std::istream& s, LshBucketList& b)
{
    //read size
    unsigned z;
    s.read((char*) &z, sizeof(z));
    //allocate
    b.resize(z);
    //read the list of buckets
    for (LshBucketIt bs=b.begin(), be=b.end(); bs!=be; bs++)
        s >> *bs;
    return s;
}

std::ostream& operator<<(std::ostream& s, LshBuckets& b)
{
    unsigned z = b.Size();
    s.write((char*) &z, sizeof(z));
    //loop on lists and write
    for (LshBucketListIt bs=b.buckets_.begin(), be=b.buckets_.end();
                bs!=be; bs++)
        s << *bs;
    return s;
}

std::istream& operator>>(std::istream& s, LshBuckets& b)
{
    //read number of lists
    unsigned z;
    s.read((char*) &z, sizeof(z));
    //resize
    b.buckets_.resize(z);
    for (LshBucketListIt bs=b.buckets_.begin(), be=b.buckets_.end();
            bs!=be; bs++)
        s >> *bs;
    return s;
}

std::ostream& operator<<(std::ostream& s, LshTable& t)
{
    s << t.buckets_;
    return s;
}

std::istream& operator>>(std::istream& s, LshTable& t)
{
    s >> t.buckets_;
    return s;
}

std::ostream& operator<<(std::ostream& s, LshTables& t)
{
    //write size
    unsigned z = t.Size();
    s.write((char*) &z, sizeof(z));
    //write the list of tables
    for (unsigned i=0; i<z; ++i)
        s << t[i];
    return s;
}

std::istream& operator>>(std::istream& s, LshTables& t)
{
    //read size
    unsigned z;
    s.read((char*) &z, sizeof(z));
    //allocate
    t.Resize(z);
    //read the list of buckets
    for (unsigned i=0; i<z; ++i)
        s >> t[i];
    return s;
}

std::ostream& operator<<(std::ostream& s, LshIndex& l)
{
    s << l.opt_ << l.lshTables_;
    return s;
}

std::istream& operator>>(std::istream& s, LshIndex& l)
{
    s >> l.opt_ >> l.lshTables_;
    return s;
}

}}
