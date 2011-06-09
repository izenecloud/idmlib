/**
 * @file SparseVectorSetFile.h
 * @author Zhongxia Li
 * @date Jun 3, 2011
 * @brief 
 */
#ifndef SPARSE_VECTOR_SETFILE_H_
#define SPARSE_VECTOR_SETFILE_H_

#include <idmlib/idm_types.h>

#include "SparseVector.h"

#include <fstream>
#include <iostream>

#include <boost/serialization/vector.hpp>
#include <boost/archive/archive_exception.hpp>

NS_IDMLIB_SSP_BEGIN

/**
 * Output file for sparse vector set
 */
template <typename IdT = uint32_t, typename VT = float>
class SparseVectorSetOFile
{
public:
	SparseVectorSetOFile(const string& filename, size_t cache_size = 0)
    : filename_(filename)
    , cache_size_(cache_size)
	, total_count_(0)
	{
    }

    ~SparseVectorSetOFile()
    {
    	close();
    }

public:
    bool open(ios::openmode mode = ios::out)
    {
    	pof_.reset(new std::ofstream(filename_.c_str(), mode));
    	if (!pof_->is_open())
    	{
    		cerr << "** failed to open (out): " << filename_ <<endl;
    		return false;
    	}

    	poa_.reset(new boost::archive::text_oarchive(*pof_));
    	return true;
    }

    size_t size()
    {
    	return total_count_;
    }

    void put(SparseVector<IdT, VT>& spvec)
    {
    	(*poa_) << spvec;
    	total_count_ ++;
    }

    void flush()
    {
    	pof_->flush();
    }

    /// todo
    void put_to_cache(SparseVector<IdT, VT>& spvec)
    {
        cache_.push_back(spvec);
    }

    /// todo
    void flush_cache()
    {
    	if (!pof_->is_open())
    		return;

        size_t count = 0;
        for (cache_iterator_t it = cache_.begin(); it != cache_.end(); it++)
        {
            (*poa_) << *it;
            count ++;
        }

        pof_->flush();

        cache_.clear(); //must clear cache
    }

    void close()
    {
    	if (!pof_->is_open())
    		return;

		if (cache_.size() > 0)
			flush_cache();

		pof_->close();

		total_count_ = 0;
    }

private:
    std::string filename_;
    boost::shared_ptr<std::ofstream> pof_;
    boost::shared_ptr<boost::archive::text_oarchive> poa_;

    typedef std::vector<SparseVector<IdT, VT> >  cache_t;
    typedef typename std::vector<SparseVector<IdT, VT> >::iterator cache_iterator_t;

    cache_t cache_;
    size_t cache_size_; // todo

    size_t total_count_;
};

/**
 * Input file for sparse vector set
 */
template <typename IdT = uint32_t, typename VT = float>
class SparseVectorSetIFile
{
public:
	SparseVectorSetIFile(const string& filename, size_t max_cache_size = 1000)
    : filename_(filename)
    , max_cache_size_(max_cache_size)
	, pos_(0)
	, pos_in_cache_(0)
	, reached_eof_(false)
    {

    }

public:
    bool open(ios::openmode mode = ios::in)
    {
    	pif_.reset(new std::ifstream(filename_.c_str(), mode));
    	if (!pif_->is_open())
    	{
    		cerr << "failed to open (in): " << filename_ <<endl;
    		return false;
    	}

    	pia_.reset(new boost::archive::text_iarchive(*pif_));
    	return true;
    }

    void init()
    {
    	pos_ = 0;
    	reached_eof_ = false;
    	refresh_cache_();
    }

    /**
     * @return true if have next, or false
     */
    bool next()
    {
    	if (pos_in_cache_ >= cache_.size())
    	{
    		if (reached_eof_)
    			return false;

    		refresh_cache_();
    		return next();
    	}

    	return true;
    }

    SparseVector<IdT, VT>& get()
    {
    	pos_ ++;
    	return cache_[(pos_in_cache_ ++)];
    }

    void close()
    {
    	if (!pif_->is_open())
    		return;

		pif_->close();

		cache_.clear();
    }

    void remove()
    {
    	boost::filesystem::remove(filename_);
    }

private:
    void refresh_cache_()
    {
        cout<<"[SparseVectorSetIFile] loading data ..."<<endl;
    	cache_.clear();
    	pos_in_cache_ = 0;

    	if (!pif_->is_open())
    	{
    		reached_eof_ = true;
    		return;
    	}

    	size_t read = 0;
        while (read < max_cache_size_)
        {
            SparseVector<IdT, VT> sv;
            try
            {
                (*pia_) >> sv;
            }
//            catch (boost::archive::archive_exception& aex)
//            {
//            	  reached_eof_ = true;
//                break;
//            }
            catch (std::exception& ex)
            {
            	reached_eof_ = true;
                return;
            }

            read ++;
            cache_.push_back(sv);
        };

        cout<<"[SparseVectorSetIFile] cached "<<read<<endl;
    }

public:
    typedef size_t pos_t;

private:
    std::string filename_;
    boost::shared_ptr<std::ifstream> pif_;
    boost::shared_ptr<boost::archive::text_iarchive> pia_;

    typedef std::vector<SparseVector<IdT, VT> >  cache_t;
    typedef typename std::vector<SparseVector<IdT, VT> >::iterator cache_iterator_t;

    cache_t cache_;
    size_t max_cache_size_;

    pos_t pos_;
    pos_t pos_in_cache_;
    bool reached_eof_;
};


typedef SparseVectorSetOFile<uint32_t, float>  SparseVectorSetOFileType;
typedef SparseVectorSetIFile<uint32_t, float>  SparseVectorSetIFileType;

NS_IDMLIB_SSP_END

#endif /* SPARSE_VECTOR_SETFILE_H_ */
