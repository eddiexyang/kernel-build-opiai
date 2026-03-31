/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Header of adec module driver
 * Author: Hisilicon multimedia software group
 * Create: 2019-09-01
 */

#ifndef ADEC_H
#define ADEC_H

#include "mkp_adec.h"

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

typedef struct {
    td_u32 buf_size;
    td_bool created;
    ot_adec_chn_attr chn_attr;
    adec_dbg_info dbg_info;
} adec_chn_ctx;

adec_chn_ctx *adec_get_chn_ctx(td_s32 chn_id);

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef ADEC_H */
