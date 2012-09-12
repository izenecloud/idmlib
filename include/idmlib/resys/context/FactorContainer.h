#ifndef IDMLIB_RESYS_FACTOR_CONTAINER_H
#define IDMLIB_RESYS_FACTOR_CONTAINER_H

#include <idmlib/idm_types.h>

#include <am/detail/cache_policy.h>
#include <3rdparty/am/luxio/array.h>

#include <glog/logging.h>

#include <boost/filesystem/path.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>

#include <vector>
#include <string>

namespace idmlib{ namespace recommender{ 

template<class VertexDataType>
class FactorContainerInMem
{
    std::string fullPath_;
    std::vector<VertexDataType > container_;
public:
    FactorContainerInMem(const std::string& fullPath)
        :fullPath_(fullPath)
    {
    }

    ~FactorContainerInMem()
    {
        Save();
    }

    void Init(unsigned size)
    {
        Load();
        if(size > container_.size())
        {
            container_.resize(size);
        }
    }

    bool Load()
    {
        febird::FileStream ifs;
        if (! ifs.xopen(fullPath_.c_str(), "r"))
            return true;

        try
        {
            febird::NativeDataInput<febird::InputBuffer> ar;
            ar.attach(&ifs);
            ar & container_;
        }
        catch(const std::exception& e)
        {
            LOG(ERROR) << "exception in febird::NativeDataInput: " << e.what()
                       << ", fileName: " << fullPath_;
            return false;
        }
        return true;
    }

    bool Save()
    {
        febird::FileStream ofs(fullPath_.c_str(), "w");
        if (! ofs)
        {
            LOG(ERROR) << "failed opening file " << fullPath_;
            return false;
        }

        try
        {
            febird::NativeDataOutput<febird::OutputBuffer> ar;
            ar.attach(&ofs);
            ar & container_;
        }
        catch(const std::exception& e)
        {
            LOG(ERROR) << "exception in febird::NativeDataOutput: " << e.what()
                   << ", fileName: " << fullPath_;
            return false;
        }
        return true;
    }

    VertexDataType& operator[] (unsigned idx) { return container_[idx];}

    unsigned Size() const { return container_.size(); }	
protected:
    DISALLOW_COPY_AND_ASSIGN(FactorContainerInMem);
};



/// We do not use a strict memory upper bound limitation
/// Internally, we use a ptr_vector to serve as the memory cache
/// Due to the pointer overheads of that container, in the worst 
/// cases, the overall memory consumption should be 
/// max_cache_count_ * VERTEX_SIZE + capacity * sizeof(void*)

template<
    class VertexDataType,
    class CacheEvictionPolicy = izenelib::am::detail::policy_lrlfu<unsigned> >
class FactorContainerDB
{
    enum { VERTEX_SIZE = sizeof(VertexDataType) };

    typedef typename boost::ptr_vector<boost::nullable<VertexDataType> > cache_container_type;
    typedef typename cache_container_type::auto_type vertex_ptr;

    std::string file_name_;

    std::string size_store_;

    cache_container_type mem_cache_;

    boost::scoped_ptr<Lux::IO::Array> db_;

    CacheEvictionPolicy policy_;

    unsigned curr_cache_count_;

    unsigned max_cache_count_;

    unsigned size_;
public:
    explicit FactorContainerDB(
            const std::string& fullPath,
            std::size_t memlimit = 500*1024*1024)
        :file_name_(fullPath)
        ,curr_cache_count_(0)
        ,max_cache_count_(memlimit/VERTEX_SIZE)
        ,size_(0)
    {
        boost::filesystem::path filePath(fullPath);
        filePath.remove_filename();
        filePath /= "factordb_maxid.xml";
        size_store_ = filePath.string();
        db_.reset(new Lux::IO::Array(Lux::IO::NONCLUSTER));
        db_->set_noncluster_params(Lux::IO::Linked);
        db_->set_lock_type(Lux::IO::LOCK_THREAD);
        OpenDB_();
        RestoreMaxIDDb_();
    }

    ~FactorContainerDB()
    {
        Dump_();
    }

    void Init(unsigned size)
    {
        if(size > (size_))
        {
            for(unsigned id = size_; id < size; ++id)
            {
                VertexDataType v;
                WriteToDB_(id, v);
            }
            size_ = size;
        }
        mem_cache_.resize(size, 0);
    }

    VertexDataType& operator[] (unsigned idx)
    {
        size_ = idx > size_ ? idx : size_;
        if(mem_cache_.is_null(idx))
        {
            VertexDataType* v = new VertexDataType;
            GetFromDB_(idx, *v);
            ++curr_cache_count_;
            mem_cache_.replace(idx, v);
            Evict_();
        }
        policy_.touch(idx);
        return mem_cache_[idx];
    }

    bool Save() { return Dump_(); }

    unsigned Size() const { return size_; }

protected:
    DISALLOW_COPY_AND_ASSIGN(FactorContainerDB);

    void Evict_()
    {
        unsigned key;
        while (IsCacheFull_() && policy_.evict(key))
        {
            if(!mem_cache_.is_null(key))
            {
                vertex_ptr v = mem_cache_.replace(key,0);
                WriteToDB_(key, *v);
                --curr_cache_count_;
            }
        }
    }

    bool IsCacheFull_() const
    {
        return curr_cache_count_ >= max_cache_count_;
    }

    bool Dump_()
    {
        for(unsigned i = 0; i < size_; ++i)
        {
            if(!mem_cache_.is_null(i))
            {
                VertexDataType v = mem_cache_[i];
                WriteToDB_(i,v);
            }
        }
        return SaveMaxID_();
    }

    bool SaveMaxID_() const
    {
        try
        {
            std::ofstream ofs(size_store_.c_str());
            if (ofs)
            {
                boost::archive::xml_oarchive oa(ofs);
                oa << boost::serialization::make_nvp(
                    "MaxID", size_
                );
            }

            return ofs;
        }
        catch (boost::archive::archive_exception& e)
        {
            return false;
        }
    }

    bool RestoreMaxIDDb_()
    {
        try
        {
            std::ifstream ifs(size_store_.c_str());
            if (ifs)
            {
                boost::archive::xml_iarchive ia(ifs);
                ia >> boost::serialization::make_nvp(
                    "MaxID", size_
                );
            }
            return ifs;
        }
        catch (boost::archive::archive_exception& e)
        {
            size_ = 0;
            return false;
        }
    }


    bool OpenDB_()
    {
        try
        {
            if ( !boost::filesystem::exists(file_name_) )
            {
                db_->open(file_name_.c_str(), Lux::IO::DB_CREAT);
            }
            else
            {
                db_->open(file_name_.c_str(), Lux::IO::DB_RDWR);
            }
        }
        catch (...)
        {
            return false;
        }
        return true;
    }

    bool WriteToDB_(const unsigned int idx, const VertexDataType& v)
    {
        izenelib::util::izene_serialization<VertexDataType> izs(v);
        char* src;
        size_t srcLen;
        izs.write_image(src, srcLen);
        if ( srcLen == 0 )
            return false;
        return db_->put(idx, (const unsigned char*)src, srcLen, Lux::IO::OVERWRITE);
    }

    bool GetFromDB_(const unsigned int idx, VertexDataType& v)
    {
        Lux::IO::data_t *val_p = NULL;
        if (!db_->get(idx, &val_p, Lux::IO::SYSTEM))
        {
            db_->clean_data(val_p);
            return false;
        }
        if ( val_p->size == 0 )
        {
            db_->clean_data(val_p);
            return false;
        }
        izenelib::util::izene_deserialization<VertexDataType> izd((char*)val_p->data, val_p->size);
        izd.read_image(v);
        db_->clean_data(val_p);
        return true;
    }


};

}}
#endif

