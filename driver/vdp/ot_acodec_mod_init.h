/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description:define acodec init header file
 * Author: Hisilicon multimedia software group
 * Create: 2019/12/25
 */

#ifndef OT_ACODEC_MOD_INIT_H
#define OT_ACODEC_MOD_INIT_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include "ot_type.h"

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

int acodec_init(void);
void acodec_exit(void);

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef OT_ACODEC_MOD_INIT_H */

