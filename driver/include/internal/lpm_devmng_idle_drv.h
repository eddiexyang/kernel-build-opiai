/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
*/

#ifndef LPM_DEVMNG_IDLE_DRV_H
#define LPM_DEVMNG_IDLE_DRV_H

#include <linux/types.h>
#include "lpm_devmng_common.h"

#ifdef LPM_IDLE

struct lpm_idle_dsmi_cfg_in {
	uint8_t idle_switch; // 0: close, 1:open
};

int32_t lpm_idle_drv_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_idle_drv_remove(uint64_t *param, uint32_t param_num);
int32_t lpm_idle_drv_suspend_prepare(void);
int32_t lpm_idle_drv_post_suspend(void);

// need check param
int32_t lpm_idle_drv_set_switch(uint32_t dev_id, struct lpm_idle_dsmi_cfg_in *in_param);

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
ssize_t lpm_idle_drv_debugfs_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos);
int32_t lpm_idle_drv_debugfs_show(struct seq_file *seq, void *v);
#endif

#endif
#endif