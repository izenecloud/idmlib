/**
 * @file SparseVectorSetFile.h
 * @author Zhongxia Li
 * @date Jun 3, 2011
 * @brief 
 */
#ifndef SPARSE_VECTOR_SETFILE_H_
#define SPARSE_VECTOR_SETFILE_H_

#include <idmlib/idm_types.h>

#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/archive_exception.hpp>

NS_IDMLIB_SSP_BEGIN

template <typename IdT = uint32_t, typename VT = float>
struct SparseVectorItem
{
    IdT itemid;
    VT value;

    SparseVectorItem() {}

    SparseVectorItem(IdT conceptId, VT v)
    : itemid(itemid), value(v) {}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & itemid;
        ar & value;
    }
};

template <typename IdT = uint32_t, typename VT = float>
struct SparseVector
{
    IdT rowid;
    IdT len;
    std::vector<SparseVectorItem<IdT, VT> > list;

    SparseVector() {}

    SparseVector(IdT rowid, size_t size=0)
    : rowid(rowid), len(0)
    {
        //list.resize(size); //xxx
    }

    void insertItem(IdT itemid, VT v)
    {
        len ++;
        list.push_back(SparseVectorItem<IdT, VT>(itemid, v));
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & rowid;
        ar & len;
        ar & list;
    }

    void print()
    {
        cout << "vector [" << rowid << " " << len << "] => ";
        typename std::vector<SparseVectorItem<IdT, VT> >::iterator iter;
        for (iter = list.begin(); iter != list.end(); iter ++)
        {
            cout << "("<<iter->itemid<<","<<iter->value<< ") ";
        }
        cout << endl;
    }
};

template <typename IdT = uint32_t, typename VT = float>
class SparseVectorSetFile
{
public:
    SparseVectorSetFile(const string& filename="./spvec.dat", size_t cache_size = 0)
    : filename_(filename)
    , cache_size_(cache_size)
    , of_(filename.c_str()/*, ios::app*/)
    , oa_(of_)
    {
        //boost::filesystem::remove(filename); //?
    }

    ~SparseVectorSetFile()
    {
        if (cache_.size() > 0)
            flush();
        of_.close();
    }

public:


public:
    void add(SparseVector<IdT, VT>& spvec)
    {
        cache_.push_back(spvec);
    }

    void add_and_flush(SparseVector<IdT, VT>& spvec)
    {

    }

    void flush()
    {
        size_t count = 0;
        for (cache_iterator_t it = cache_.begin(); it != cache_.end(); it++)
        {
            oa_ << *it;
            count ++;
        }

        cout << "flushed: " << count << endl;

        cache_.clear(); //must clear cache
    }

    void close()
    {
        of_.close();
    }

    void load()
    {
        std::ifstream fin(filename_.c_str());
     return;
        boost::archive::text_iarchive ia(fin);

        return;

        // read list count
//        size_t count;
//        ia >> count;

//        for (size_t i = 0; i < count; i++)
//        {
//            SparseVector<IdT, VT> sv;
//            ia >> sv;
//            sv.print();
//        }

        size_t record_count = 0;
        while (1) {
            SparseVector<IdT, VT> sv;
            try
            {
                ia >> sv;
            }
            catch (boost::archive::archive_exception& aex)
            {
                // there is no more elegant way to judge eof
                break;
            }
            catch (std::exception& ex)
            {
                break;
            }

            record_count ++;
        }

        fin.close();
    }

private:
    std::string filename_;

    typedef std::vector<SparseVector<IdT, VT> >  cache_t;
    typedef typename std::vector<SparseVector<IdT, VT> >::iterator cache_iterator_t;
    cache_t cache_;
    size_t cache_size_; // todo

    std::ofstream of_;
    boost::archive::text_oarchive oa_;
};

NS_IDMLIB_SSP_END

#endif /* SPARSE_VECTOR_SETFILE_H_ */
