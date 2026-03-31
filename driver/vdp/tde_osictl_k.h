/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: tde osictl operation
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */

#ifndef TDE_OSICTL_K_H
#define TDE_OSICTL_K_H
#include "ot_drv_tde.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define TDE_MAX_LINE_NUM 4

typedef struct {
    drv_tde_color_fmt in_fmt;
    drv_tde_rect *in_rect;
    drv_tde_rect *out_rect;
} tde_rect_opt;

typedef struct {
    drv_tde_alpha_blending alpha_blending_cmd;
    drv_tde_rop_mode rop_code_color;
    drv_tde_rop_mode rop_code_alpha;
    td_bool single_sr2_rop;
} tde_rop_opt;

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* TDE_OSICTL_K_H */