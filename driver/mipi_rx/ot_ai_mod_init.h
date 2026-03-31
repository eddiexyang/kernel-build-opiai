/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description:define ai init header file
 * Author: Hisilicon multimedia software group
 * Create: 2019/12/25
 */

#ifndef OT_AI_MOD_INIT_H
#define OT_AI_MOD_INIT_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include "ot_type.h"

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

int ai_module_init(void);
void ai_module_exit(void);

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef OT_AI_MOD_INIT_H */

