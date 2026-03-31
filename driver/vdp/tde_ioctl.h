/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: tde osr operation
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */

#ifndef TDE_IOCTL_H
#define TDE_IOCTL_H

#include "ot_type.h"
#include "tde_define.h"
#ifdef TDE_UT
#include "ioctl.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

td_s32 tde_init_module_k(td_void);
td_void tde_cleanup_module_k(td_void);
long tde_ioctl(unsigned int cmd, unsigned long arg, td_void *private_data);
int tde_open(td_void *private_data);
int tde_release(td_void *private_data);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* TDE_IOCTL_H */

