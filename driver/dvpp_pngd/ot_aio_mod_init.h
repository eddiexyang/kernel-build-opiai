/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description:define aio init header file
 * Author: Hisilicon multimedia software group
 * Create: 2019/12/25
 */

#ifndef OT_AIO_MOD_INIT_H
#define OT_AIO_MOD_INIT_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/of_platform.h>
#include "ot_type.h"
#include "hi_osal.h"

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

/* variable init */
td_void aiao_set_aio_base(td_void *aio_base);
td_void aiao_set_acodec_base(td_void *acodec_base);
td_void aiao_set_reg_acodec_base(td_void *reg_acodec_base);
td_void aiao_set_aiao_irq(td_u32 aiao_irq);

/* export symbol */
td_void *aiao_get_acodec_base(td_void);

int aiao_module_init(void);
void aiao_module_exit(void);

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef OT_AIO_MOD_INIT_H */

