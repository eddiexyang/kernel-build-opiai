/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2020-4-1
 */

#ifndef PNGD_EXT_H__
#define PNGD_EXT_H__

#include "hi_type.h"
#include "hi_comm_video.h"
#include "mod_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // end of #ifdef __cplusplus

typedef void fn_pngd_add_chn(void);
typedef void fn_pngd_del_chn(void);

typedef struct {
    fn_pngd_add_chn *pfn_pngd_add_chn;
    fn_pngd_del_chn *pfn_pngd_del_chn;
} pngd_extern_func;

extern int32_t g_is_depend_sys;

int32_t pngd_module_init(void);
void pngd_module_exit(void);
hi_void pngd_module_ref_ctrl(module_ref_ctrl ref_ctrl);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // end of #ifdef __cplusplus

#endif // end of #ifndef  PNGD_EXT_H__