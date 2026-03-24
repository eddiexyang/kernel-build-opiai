/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2022. All rights reserved.
 * Description: define hdmi init header file
 * Author: Hisilicon HDMI software group
 * Create: 2020/03/27
 */
#ifndef HDMI_MOD_INIT_H
#define HDMI_MOD_INIT_H

#include "ot_defines.h"
#include "ot_type.h"
#include "ot_osal.h"

#define HDMI_BOOTDOT_BLOCK_ID      (23U)
#define HDMI_BOOTDOT_MAGIC_NUM     (1U)

#define HDMI_INIT_FAIL             0xA83C1003U

#define INIT_START                 (0U)
#define INIT_END                   (1U)

int hdmi_drv_mod_init(void);
void hdmi_drv_mod_exit(void);

#endif /* HDMI_MOD_INIT_H */

