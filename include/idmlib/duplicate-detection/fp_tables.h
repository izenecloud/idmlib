#ifndef IDMLIB_DD_FPTABLES_H_
#define IDMLIB_DD_FPTABLES_H_

#include "fp_table.h"

#include <idmlib/idm_types.h>
#include <vector>
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
