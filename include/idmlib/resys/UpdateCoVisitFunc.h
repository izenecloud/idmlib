#ifndef IDMLIB_RESYS_UPDATE_COVISIT_FUNC_H
#define IDMLIB_RESYS_UPDATE_COVISIT_FUNC_H

#include "CoVisitFreq.h"

#include <list>

NS_IDMLIB_RESYS_BEGIN

template <typename RowType>
class UpdateCoVisitFunc
{
public:
    typedef std::list<ItemType> ItemList;

    UpdateCoVisitFunc(const ItemList& list1, const ItemList& list2)
    : list1_(list1)
    , list2_(list2)
    {}

    void operator() (RowType& row) const
    {
        updateList_(row, list1_);
        updateList_(row, list2_);
    }

private:
    void updateList_(RowType& row, const ItemList& list) const
    {
        for(ItemList::const_iterator it = list.begin();
            it != list.end(); ++it)
        {
            row[*it].update();
        }
    }

private:
    const ItemList& list1_;
    const ItemList& list2_;
};

NS_IDMLIB_RESYS_END

#endif
