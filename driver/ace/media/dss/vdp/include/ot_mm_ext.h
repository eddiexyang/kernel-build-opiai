#ifndef OT_MM_EXT_H
#define OT_MM_EXT_H

#include "ot_type.h"
#include "../../../../dvpp/dvpp/HiDvpp/cbb/base/ext_inc/mm_ext.h"

static inline td_s32 cmpi_check_mmz_phy_addr(td_phys_addr_t phy_addr, td_u32 size)
{
    if ((phy_addr == 0) || (size == 0)) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

#endif
