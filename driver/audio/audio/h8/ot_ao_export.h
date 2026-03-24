/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: ao export for user
 * Author: Hisilicon multimedia software group
 * Create: 2019/03/05
 */

#ifndef OT_AO_EXPORT_H
#define OT_AO_EXPORT_H

#include "hi_comm_audio.h"

typedef struct {
    void (*ao_notify) (int ao_dev);
} ot_ao_export_callback;

typedef td_s32 ot_ao_register_export_callback(ot_ao_export_callback *export_callback);

typedef struct {
    ot_ao_register_export_callback *register_export_callback;
} ot_ao_export_symbol;

ot_ao_export_symbol *ot_ao_get_export_symbol(td_void);

#endif  /* OT_AO_EXPORT_H */

