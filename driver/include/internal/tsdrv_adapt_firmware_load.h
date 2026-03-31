/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
 * Create: 2019-10-15
 */
#ifndef TSDRV_ADAPT_FIRMWARE_LOAD_H
#define TSDRV_ADAPT_FIRMWARE_LOAD_H
#include "devdrv_common.h"

int aicpu_dereset(u32 type, struct devdrv_info *dev_info);
int aicpu_get_dts_config(struct devdrv_info *dev_info, struct aicpu_dts_config *conf);
void *devdrv_tscpu_alloc_memory(struct devdrv_info *dev_info, u32 tsid,
    struct device *dev, dma_addr_t *dma_addr);
int devdrv_fw_load_init(struct devdrv_info *dev_info, u32 tsid);
#ifdef CFG_SOC_PLATFORM_MDC_V51
const char *tsdrv_get_tsfw_path(enum tsdrv_hwts_spec ts_spec, char *tsfw_path, u32 size);
#endif

struct semaphore *devdrv_get_core_info_sema(u32 dev_id);
void devdrv_get_active_core(struct devdrv_info *dev_info);

/* define in device manager  */
extern void devdrv_mn_mutex_lock(int dev_id);
extern void devdrv_mn_mutex_unlock(int dev_id);

#endif /* __DEVDRV_ADAPT_FIRMWARE_LOAD_H */
