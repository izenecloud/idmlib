#ifndef IDMLIB_RESYS_ITEM_COVISITATION_H
#define IDMLIB_RESYS_ITEM_COVISITATION_H

#include "CoVisitFreq.h"
#include "UpdateCoVisitFunc.h"
#include "GetTopCoVisitFunc.h"
#include "ItemRescorer.h"
#include "RecommendItem.h"
#include <idmlib/idm_types.h>

#include <am/matrix/matrix_db.h>

#include <string>
#include <vector>
#include <list>

using namespace std;

NS_IDMLIB_RESYS_BEGIN

template<typename CoVisitation>
class ItemCoVisitation
{
public:
    typedef izenelib::am::MatrixDB<ItemType, CoVisitation> MatrixType;
    typedef typename MatrixType::row_type RowType;

    ItemCoVisitation(
        const std::string& homePath,
        size_t cache_size = 1024*1024
    )
        : db_(cache_size, homePath)
    {
    }

    ~ItemCoVisitation()
    {
        flush();
    }

    /**
     * Update items visited by one user.
     * @param oldItems the items visited before
     * @param newItems the new visited items
     *
     * As the covisit matrix records the number of users who have visited both item i and item j,
     * the @c visit function has below pre-conditions:
     * @pre each items in @p oldItems should be unique
     * @pre each items in @p newItems should be unique
     * @pre there should be no items contained in both @p oldItems and @p newItems
     */
    void visit(
        const std::list<ItemType>& oldItems,
        const std::list<ItemType>& newItems
    )
    {
        // update old*new pairs
        std::list<ItemType> emptyList;
        updateCoVisit_(oldItems, newItems, emptyList);

        // update new*total pairs
        updateCoVisit_(newItems, oldItems, newItems);
    }

    void getCoVisitation(
        size_t topCount,
        ItemType item,
        RecommendItemVec& recItems,
        const ItemRescorer* rescorer = NULL
    )
    {
        GetTopCoVisitFunc<CoVisitation, RowType> func(item, rescorer, topCount);
        db_.read_row_with_func(item, func);
        func.getResult(recItems);
    }

    void flush()
    {
        db_.flush();
    }

    MatrixType& matrix()
    {
        return db_;
    }

private:
    void updateCoVisit_(
        const std::list<ItemType>& rows,
        const std::list<ItemType>& cols1,
        const std::list<ItemType>& cols2
    )
    {
        UpdateCoVisitFunc<RowType> func(cols1, cols2);
        for(std::list<ItemType>::const_iterator iter = rows.begin();
            iter != rows.end(); ++iter)
        {
            db_.update_row_with_func(*iter, func);
        }
    }

private:
    MatrixType db_;
};

NS_IDMLIB_RESYS_END

#endif
