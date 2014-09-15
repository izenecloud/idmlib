#ifndef IDMLIB_B5M_B5MPPROCESSOR2_H_
#define IDMLIB_B5M_B5MPPROCESSOR2_H_

#include <string>
#include <vector>
#include "b5m_types.h"
#include "b5m_helper.h"
#include "product_db.h"
#include "offer_db.h"
#include "b5m_m.h"
//#include "history_db_helper.h"
#include <sf1common/ScdMerger.h>
#include <sf1common/PairwiseScdMerger.h>
#include <am/sequence_file/ssfr.h>
#include <util/DynamicBloomFilter.h>

NS_IDMLIB_B5M_BEGIN

///B5mpProcessor is responsibility to generate b5mp scds and also b5mo_mirror scd
class B5mpProcessor2{
public:
    B5mpProcessor2(const B5mM& b5mm): b5mm_(b5mm)
    {
    }

    void SetBufferSize(const std::string& bs)
    {
        buffer_size_ = bs;
    }
    void SetSorterBin(const std::string& bin)
    {
        sorter_bin_ = bin;
    }

    bool Generate(const std::string& m, const std::string& last_m)
    {
        LOG(INFO)<<"b5mp merger begin"<<std::endl;
        B5moSorter sorter(m);
        sorter.SetB5mM(b5mm_);
        if(!b5mm_.buffer_size.empty())
        {
            sorter.SetBufferSize(b5mm_.buffer_size);
        }
        if(!b5mm_.sorter_bin.empty())
        {
            sorter.SetSorterBin(b5mm_.sorter_bin);
        }

        bool succ = sorter.StageTwo(last_m);
        LOG(INFO)<<"b5mp merger finish with succ status : "<<succ<<std::endl;
        if(!succ) return false;
        if(b5mm_.gen_b5mq&&!b5mm_.rtype)
        {
            const std::string& b5mq_path = b5mm_.b5mq_path;
            B5MHelper::PrepareEmptyDir(b5mq_path);
            LOG(INFO)<<"start b5mq on "<<b5mq_path<<std::endl;
            const std::string& b5mo_path = b5mm_.b5mo_path;
            std::string cmd = "/opt/b5mq_preprocess/do.sh -i "+b5mo_path+" -o "+b5mq_path+" -n "+boost::lexical_cast<std::string>(b5mm_.thread_num)+" -t /opt/b5mq_preprocess/working_dir";
            if(b5mm_.mode>0)
            {
                cmd += " -r";
            }
            LOG(INFO)<<"[cmd]"<<cmd<<std::endl;
            int status = system(cmd.c_str());
            if(status!=0)
            {
                LOG(ERROR)<<"cmd error"<<std::endl;
                return false;
            }
            LOG(INFO)<<"b5mq end"<<std::endl;
        }
        return true;
    }

private:

private:
    B5mM b5mm_;
    std::string m_;
    std::string last_m_;
    std::string buffer_size_;
    std::string sorter_bin_;
};

NS_IDMLIB_B5M_END

#endif


