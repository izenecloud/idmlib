#ifndef IDMLIB_DD_FPTABLE_H_
#define IDMLIB_DD_FPTABLE_H_

#include <idmlib/idm_types.h>
#include "dd_types.h"
#include "fp_item.h"

#include <boost/assert.hpp>
#include <string>
#include <vector>


NS_IDMLIB_DD_BEGIN

class FpTable
{
public:
    FpTable()
    {
    }

    explicit FpTable(const std::vector<uint64_t>& bit_mask)
        : bit_mask_(bit_mask)
    {
    }

    void resetBitMask(const std::vector<uint64_t>& bit_mask)
    {
        bit_mask_ = bit_mask;
    }

    const std::vector<uint64_t>& getBitMask() const
    {
        return bit_mask_;
    }

    template <typename DocIdType, class AttachType>
    bool operator() (const FpItem<DocIdType, AttachType>& left, const FpItem<DocIdType, AttachType>& right) const
    {
        if (right.fp.empty()) return false;
        if (left.fp.empty()) return true;

        BOOST_ASSERT(bit_mask_.size() == left.size());
        BOOST_ASSERT(bit_mask_.size() == right.size());
        for (int i = bit_mask_.size() - 1; i >= 0; i--)
        {
            if ((left.fp[i] & bit_mask_[i]) < (right.fp[i] & bit_mask_[i]))
                return true;
            if ((left.fp[i] & bit_mask_[i]) > (right.fp[i] & bit_mask_[i]))
                return false;
        }
        return false;
    }

    void GetMaskedBits(const std::vector<uint64_t>& raw_bits, std::vector<uint64_t>& masked_bits) const
    {
        if (raw_bits.empty()) return;

        BOOST_ASSERT(bit_mask_.size() == raw_bits.size());
        masked_bits.resize(raw_bits.size());
        for (uint32_t i = 0; i < raw_bits.size(); i++)
        {
            masked_bits[i] = raw_bits[i] & bit_mask_[i];
        }
    }

private:
    friend class FpTables;

    std::vector<uint64_t> bit_mask_;
};

NS_IDMLIB_DD_END

#endif
