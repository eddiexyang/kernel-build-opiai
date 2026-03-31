/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: handle manage
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */
#ifndef TDE_HANDLE_H
#define TDE_HANDLE_H

/*********************************add include here**********************************************/

#include "tde_osilist.h"
#include "tde_define.h"
#include "osal_list.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define TDE_MAX_HANDLE_VALUE 0x7fffffff

typedef struct {
    struct osal_list_head list_head;
    osal_spinlock_t lock;
    td_s32 handle;
    td_void *res;
} tde_handle_mgr;

td_s32 initial_handle(td_void);
td_void get_handle(tde_handle_mgr *res, td_s32 *handle);
td_bool tde_query_handle(td_s32 handle, tde_handle_mgr **res);
td_bool release_handle(td_s32 handle);
td_void destroy_handle(td_void);

tde_handle_mgr *tde_get_handle_list(td_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* TDE_HANDLE_H */
