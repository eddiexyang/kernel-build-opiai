/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: tde proc
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */

#ifndef TDE_PROC_H
#define TDE_PROC_H

#include "tde_hal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef CONFIG_OT_PROC_SHOW_SUPPORT

#define TDE_MAX_PROC_NUM 8
td_void tde_proc_record_node(const tde_hw_node *hw_node);
int tde_read_proc(const osal_proc_entry_t *p);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* TDE_PROC_H */
