#ifndef IDMLIB_RESYS_COVISIT_FREQ_H
#define IDMLIB_RESYS_COVISIT_FREQ_H

#include <idmlib/idm_types.h>
#include <am/matrix/matrix_db.h>

NS_IDMLIB_RESYS_BEGIN

typedef uint32_t ItemType;

struct CoVisitFreq : public ::izenelib::util::pod_tag
{
    CoVisitFreq():freq(0){}

    uint32_t freq;

    void update()
    {
        freq += 1;
    }
};

NS_IDMLIB_RESYS_END

#endif
