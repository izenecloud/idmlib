/**
 * @file SparseVector.h
 * @author Zhongxia Li
 * @date Jun 3, 2011
 * @brief
 */

#ifndef SPARSE_VECTOR_H_
#define SPARSE_VECTOR_H_

#include <idmlib/idm_types.h>

#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>

NS_IDMLIB_SSP_BEGIN

template <typename IdT = uint32_t, typename VT = float>
struct SparseVectorItem
{
    IdT itemid;
    VT value;

    SparseVectorItem() : itemid(0), value(0) {}

    SparseVectorItem(IdT id, VT v)
    : itemid(id), value(v) {}

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
    typedef std::vector<SparseVectorItem<IdT, VT> > list_t;
    typedef typename std::vector<SparseVectorItem<IdT, VT> >::iterator list_iter_t;

    /// data
    IdT rowid;
    IdT len;
    list_t list;

    /// methods
    SparseVector() : rowid(0), len(0)  {}

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


typedef SparseVectorItem<uint32_t, float> SparseVectorItemType;
typedef SparseVector<uint32_t, float> SparseVectorType;

NS_IDMLIB_SSP_END

#endif /* SPARSE_VECTOR_H_ */
