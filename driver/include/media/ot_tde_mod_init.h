/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: tde init operation
 */

#ifndef __SOURCE_MSP_DRV_TDE_INIT__
#define __SOURCE_MSP_DRV_TDE_INIT__

#include "ot_type.h"
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/poll.h>

#define TDE_BOOTDOT_BLOCK_ID      (23U)
#define TDE_BOOTDOT_MAGIC_NUM     (2U)

#define TDE_INIT_FAIL             0xA83C1004U
#define TDE_SUSPEND_FAIL          0xA83C2004U
#define TDE_RESUME_FAIL           0xA83C3004U

#define INIT_START                (0U)
#define INIT_END                  (1U)

#define SUSPEND_START             (0U)
#define SUSPEND_END               (1U)

#define RESUME_START                          (0U)
#define RESUME_END                            (1U)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

td_void set_tde_irq(unsigned int temp_tde_irq);
td_s32 tde_drv_mod_init(td_void);
td_void tde_drv_mod_exit(td_void);
td_void tde_hal_set_base_vir_addr(td_u32 *temp_base_vir_addr);
td_void tde_set_max_node_num(td_u32 node_num);
td_void tde_set_resize_filter(td_bool is_resize_filter);
td_void tde_set_tde_tmp_buffer(td_u32 tde_tmp_buf);
td_void tde_init_set_rgb_truncation_mode(td_u32 rgb_truncation_mode);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __SOURCE_MSP_DRV_TDE_INIT__ */

