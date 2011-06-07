/**
 * @file DataSetIterator.h
 * @author Zhongxia Li
 * @date Jun 7, 2011
 * @brief 
 */
#ifndef DATA_SET_ITERATOR_H_
#define DATA_SET_ITERATOR_H_

#include <idmlib/idm_types.h>

#include <idmlib/semantic_space/esa/SparseVectorSetFile.h>

using namespace idmlib::ssp;

NS_IDMLIB_SIM_BEGIN

class DataSetIterator
{
public:
    DataSetIterator(const std::string& filename)
    : fileName_(filename)
    {
    }

    virtual ~DataSetIterator()
    {
    }

public:
    virtual bool next() = 0;

    virtual SparseVectorType& get() = 0;

    bool seek(uint32_t vecid) { return false; }

protected:
    std::string fileName_;
};

class SparseVectorSetIterator : public DataSetIterator
{
public:
    SparseVectorSetIterator(const std::string& filename="./inter_vec.dat")
    : DataSetIterator(filename)
    , data_(filename)
    {
        data_.open();
    }

    ~SparseVectorSetIterator()
    {
        data_.close();
    }

public:
    /// start postion
    bool seek(uint32_t vecid)
    {
        //data_.close();
        return false;
    }

    bool next()
    {
        return data_.next();
    }

    SparseVectorType& get()
    {
        return data_.get();
    }

private:
    SparseVectorSetIFileType data_;
};

NS_IDMLIB_SIM_END

#endif /* DATA_SET_ITERATOR_H_ */
