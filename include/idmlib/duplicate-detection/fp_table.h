#ifndef IDMLIB_DD_FPTABLE_H_
#define IDMLIB_DD_FPTABLE_H_

#include "fp_item.h"


NS_IDMLIB_DD_BEGIN

class FpTable
{
public:
    FpTable()
    {
    }

    explicit FpTable(const FpType& bit_mask)
        : bit_mask_(bit_mask)
    {
    }

    void resetBitMask(const FpType& bit_mask)
    {
        bit_mask_ = bit_mask;
    }

    const FpType& getBitMask() const
    {
        return bit_mask_;
    }

    template <typename DocIdType, class AttachType>
    bool operator() (const FpItem<DocIdType, AttachType>& left, const FpItem<DocIdType, AttachType>& right) const
    {
        for (int i = FpType::FP_SIZE - 1; i >= 0; i--)
        {
            if ((left.fp.desc[i] & bit_mask_.desc[i]) < (right.fp.desc[i] & bit_mask_.desc[i]))
                return true;
            if ((left.fp.desc[i] & bit_mask_.desc[i]) > (right.fp.desc[i] & bit_mask_.desc[i]))
                return false;
        }
        return false;
    }

    void GetMaskedBits(const FpType& raw_bits, FpType& masked_bits) const
    {
        for (uint32_t i = 0; i < FpType::FP_SIZE; i++)
        {
            masked_bits.desc[i] = raw_bits.desc[i] & bit_mask_.desc[i];
        }
    }

private:
    friend class FpTables;

    FpType bit_mask_;
};

NS_IDMLIB_DD_END

#endif
