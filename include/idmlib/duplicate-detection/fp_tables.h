#ifndef IDMLIB_DD_FPTABLES_H_
#define IDMLIB_DD_FPTABLES_H_

#include <idmlib/idm_types.h>
#include <string>
#include <vector>
#include <ir/dup_det/index_fp.hpp>
#include <ir/dup_det/integer_dyn_array.hpp>
#include "charikar_algo.h"
#include "fp_table.h"
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>


NS_IDMLIB_DD_BEGIN

class FpTables
{
public:
    void GenTables(uint32_t f, uint8_t k, uint8_t partition, std::vector<FpTable>& table_list) const;

private:
    bool permutate_(std::vector<uint8_t>& vec, uint32_t index, uint32_t size) const;
};

NS_IDMLIB_DD_END

#endif
