/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: voie_drv
 * Author: Hisilicon multimedia software group
 * Create: 2012-12-22
 * History: 2019-08-01 update for CSEC
 */

#ifndef VOIE_DRV_H
#define VOIE_DRV_H

#include "ot_type.h"
#include "hi_comm_audio.h"

#ifdef CONFIG_OT_PLATFORM_V8
#include "ot_inner_common_aenc.h"
#else
#include "ot_comm_aenc_adapt.h"
#endif

#include "voie_hal.h"

typedef enum {
    VOIE_CODE_TYPE_NONE = 0,
    VOIE_CODE_G711A = 1,
    VOIE_CODE_G711U = 2,
    VOIE_CODE_ADPCM_DIV4 = 3,
    VOIE_CODE_G726_16KBPS = 4,
    VOIE_CODE_G726_24KBPS = 5,
    VOIE_CODE_G726_32KBPS = 6,
    VOIE_CODE_G726_40KBPS = 7,
    VOIE_CODE_MEIDA_G726_16KBPS = 0x24,
    VOIE_CODE_MEIDA_G726_24KBPS = 0x25,
    VOIE_CODE_MEIDA_G726_32KBPS = 0x26,
    VOIE_CODE_MEIDA_G726_40KBPS = 0x27,
    VOIE_CODE_ADPCM_ORG_DIV4 = 0x43,
    VOIE_CODE_TYPE_BUTT
} voie_code_type;

td_s32 _voie_get_type(const ot_aenc_chn_attr *chn_attr, voie_code_type *voie_type);

td_s32 voie_drv_init(td_void);

td_void voie_drv_exit(td_void);

td_void voie_drv_set_clk(td_bool enable);

td_void voie_drv_get_int_status(td_u32 *int_status);

td_void voie_drv_clear_int_status(td_void);

td_void voie_drv_start(td_u32 phys_addr);

td_void voie_drv_set_g726_state(td_u32 *g726_state);

td_void voie_drv_set_adpcm_state(td_u32 *adpcm_state);

#endif /* end of VOIE_DRV_H */
