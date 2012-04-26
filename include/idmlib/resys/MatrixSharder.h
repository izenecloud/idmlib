/**
 * @file MatrixSharder.h
 * @brief an interface class, used to distribute a matrix on multiple nodes.
 * @author Jun Jiang
 * @date 2012-04-12
 */

#ifndef IDMLIB_RESYS_MATRIX_SHARDER_H
#define IDMLIB_RESYS_MATRIX_SHARDER_H

#include <idmlib/idm_types.h>

NS_IDMLIB_RESYS_BEGIN

class MatrixSharder
{
public:
    virtual ~MatrixSharder() {}

    /**
     * test whether current node should store the row of @p rowId.
     * @param rowId the row id to test
     * @return true for store, false for not store.
     */
    virtual bool testRow(uint32_t rowId) = 0;
};

NS_IDMLIB_RESYS_END

#endif // IDMLIB_RESYS_MATRIX_SHARDER_H
