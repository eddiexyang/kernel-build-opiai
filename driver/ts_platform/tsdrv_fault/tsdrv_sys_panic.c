/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-08-27
*/
#include <linux/notifier.h>
#include <asm/io.h>
#include "tsdrv_fault_init.h"
#include "devdrv_manager_comm.h"
#include "devdrv_common.h"
#include "tsdrv_common_fault_platform.h"
#include "tsdrv_heartbeat.h"
#include "tsdrv_log.h"
#include "tsdrv_sys_panic.h"

#ifdef CFG_FEATURE_SYS_PANIC_DFX
#define TS_SYSSTAT_REG_NUM 23
#define TS_SYSSTAT_REG_OFFSET 4
#define TS_SYSSTAT0_OFFSET 0x8
#define TS_SOFT_RESET_STAT_OFFSET 0xD0

int tsdrv_ts_panic_notify(struct notifier_block *nb, unsigned long event, void *args)
{
#ifdef CFG_FEATURE_TS_HB_DFX
    struct tsdrv_heart_beat_info *heart_beat_info = NULL;
#endif
    u64 ts_regs_virt_addr;
    u32 devid, dev_num, i;
    u32 reg_val = 0;
    int ret;

    ret = devdrv_get_devnum(&dev_num);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Get device number failed. (ret=%d)\n", ret);
        return ret;
    }

    TSDRV_PRINT_EVENT("--------- panic log print start ---------\n");
    for (devid = 0; devid < dev_num; devid++) {
        struct devdrv_info *info = NULL;
        TSDRV_PRINT_EVENT("device id : %u\n", devid);
        info = devdrv_manager_get_devdrv_info(devid);
        if (info == NULL) {
            TSDRV_PRINT_ERR("Failed to get devdrv_info. (devid=%u)\n", devid);
            return -EINVAL;
        }
        ts_regs_virt_addr = (u64)(uintptr_t)info->pdata->ts_pdata[0].ts_sysctl_vaddr;

        for (i = 0; i < TS_SYSSTAT_REG_NUM; i++) {
            reg_val = readl((void __iomem *)(uintptr_t)(ts_regs_virt_addr + TS_SYSSTAT0_OFFSET +
                i * TS_SYSSTAT_REG_OFFSET));

            TSDRV_PRINT_EVENT("TS SYSSTAT [%u] register value is [0x%x].\n", i, reg_val);
            reg_val = 0;
        }

        reg_val = readl((void __iomem *)(uintptr_t)(ts_regs_virt_addr + TS_SOFT_RESET_STAT_OFFSET));
        TSDRV_PRINT_EVENT("TS_SOFT_RESET_STAT_OFFSET register value is [0x%x].\n", reg_val);

#ifdef CFG_FEATURE_TS_HB_DFX
        heart_beat_info = tsdrv_hb_get_heart_beat_info(devid, 0);
        tsdrv_hb_tsfw_dfx_reg_show(0, heart_beat_info, TS_STATUS_ERR_SHOW);
#endif
    }
    TSDRV_PRINT_EVENT("--------- panic log print end ---------\n");
    return 0;
}

#else
int tsdrv_ts_panic_notify(struct notifier_block *nb, unsigned long event, void *args)
{
    return 0;
}
#endif

