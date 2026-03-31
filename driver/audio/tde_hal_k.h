/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: tde hal operation
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */

#ifndef TDE_HAL_K_H
#define TDE_HAL_K_H

#include "ot_type.h"
#include "tde_hal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef TDE_COREDUMP_DEBUG
volatile td_u32 *tde_hal_get_base_vir_addr(td_void);
#endif
tde_color_fmt tde_get_common_drv_color_fmt(td_u32 count);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* TDE_HAL_K_H */
