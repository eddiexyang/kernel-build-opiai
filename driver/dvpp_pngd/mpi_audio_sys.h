#ifndef OT_MPI_SYS_H
#define OT_MPI_SYS_H

#include <sys/mman.h>
#include "vb_ext.h"
#include "hi_osal_user.h"
#include "mpi_sys.h"
#include "hi_comm_sys_adapt.h"
#include "mpi_bind.h"

#ifdef __cplusplus
extern "C" {
#endif

#define sys_check_null_ptr_return(ptr)      \
    do {                                    \
        if ((ptr) == HI_NULL) {                \
            sys_err_trace("Null point \n"); \
            return HI_ERR_SYS_NULL_PTR;     \
        }                                   \
    } while (0)

#define OT_PAGE_SIZE       (1UL << 12)
#define OT_PAGE_MASK       0xfffffffffffff000ULL

static inline td_s32 ot_mpi_sys_get_bind_by_src(const ot_mpp_chn *src_chn, ot_mpp_bind_dst *bind_dst)
{
    return mpi_sys_get_bind_by_src(src_chn, bind_dst);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
