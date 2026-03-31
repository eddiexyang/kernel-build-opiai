/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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


#include "devdrv_manager_msg.h"
#include "devdrv_manager_time.h"
#include "devdrv_manager.h"
#include "dms_ts_info.h"
#include "dms_ipc_interface.h"

int devdrv_manager_get_devinfo_from_host(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devmng_msg_h2d_info *h2d_info = NULL;
    struct devdrv_info *dev_info = NULL;
    u32 dev_id;
    int ret, i;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_H2D_MAGIC) ||
        (in_len != (sizeof(struct devdrv_manager_msg_head) + sizeof(struct devmng_msg_h2d_info)))) {
        devdrv_drv_err("Invalid message from host. (dev_id=%u; valid=%u; in_len=%u)\n",
                       devid, dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }
    h2d_info = (struct devmng_msg_h2d_info *)dev_manager_msg_info->payload;

    dev_id = dev_manager_msg_info->header.dev_id;
    dev_info = devdrv_manager_get_devdrv_info(devid);
    if ((dev_id != devdrv_manager_get_devid(devid)) || (dev_info == NULL)) {
        devdrv_drv_warn("invalid device id, dev_id = %u, "
                       "dev_manager_info->dev_id = %u, "
                       "dev_info is %s\n",
                       dev_id, devdrv_manager_get_devid(devid), (dev_info == NULL) ? "NULL" : "not NULL");
        return -EAGAIN;
    }

    for (i = 0; i < DEVDRV_MAX_COMPUTING_POWER_TYPE; i++) {
        h2d_info->computing_power[i] = dev_info->computing_power[i];
    }

    ret = devdrv_get_ffts_type(&h2d_info->ffts_type);
    if (ret != 0) {
        devdrv_drv_err("devdrv_get_ffts_type failed ret = %d.\n", ret);
        return ret;
    }

    h2d_info->cpu_system_count = devdrv_manager_get_cpu_tick();
    h2d_info->monotonic_raw_time_ns = ktime_get_raw_ns();

    dev_manager_msg_info->header.result = 0;
    dev_manager_msg_info->header.valid = DEVDRV_MANAGER_MSG_D2H_MAGIC;
    *ack_len = sizeof(struct devdrv_manager_msg_head) + sizeof(struct devmng_msg_h2d_info);

    return 0;
}

int devdrv_manager_h2d_sync_low_power(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_power_state *dev_manager_power_state = NULL;
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_info *dev_info = NULL;
    u32 dev_id;
    int ret;

    devdrv_drv_info("devid %u get into devdrv_manager_h2d_sync_low_power.\n", devid);

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((msg == NULL) || (ack_len == NULL)) {
        devdrv_drv_err("Invalid message from host. (dev_id=%u; msg or ack_len is null)\n", devid);
        return -EINVAL;
    }
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_H2D_MAGIC) ||
        (in_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("Invalid message from host. (dev_id=%u; valid=%u; in_len=%u)\n",
                       devid, dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }
    dev_manager_power_state = (struct devdrv_manager_power_state *)dev_manager_msg_info->payload;

    dev_id = dev_manager_msg_info->header.dev_id;
    dev_info = devdrv_manager_get_devdrv_info(devid);
    if ((dev_id != devdrv_manager_get_devid(devid)) || (dev_info == NULL)) {
        devdrv_drv_warn("invalid device id, dev_id = %u "
                       "dev_manager_info->dev_id = %u, "
                       "dev_info is %s\n",
                       dev_id, devdrv_manager_get_devid(devid), (dev_info == NULL) ? "NULL" : "not NULL");
        return -EAGAIN;
    }

    if (dev_manager_power_state->IsLowPowerState == 1) { /* low power status */
        ret = devdrv_manager_msg_suspend();
    } else {
        ret = devdrv_manager_msg_resume();
    }

    dev_manager_msg_info->header.result = ret;
    dev_manager_msg_info->header.valid = DEVDRV_MANAGER_MSG_D2H_MAGIC;
    *ack_len = sizeof(*dev_manager_msg_info);

    return 0;
}

typedef int (*soc_misc_calc_aicpu_utilization)(unsigned int dev_id, unsigned int *utilization);
int devdrv_manager_get_core_utilization_from_host(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_core_utilization *util_info = NULL;
    static soc_misc_calc_aicpu_utilization calc_aicpu_func = NULL;
    struct devdrv_info *dev_info = NULL;
    struct dms_ts_info_in ts_info = {0};
    unsigned int dev_id;
    unsigned int vfid;
    int ret;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_H2D_MAGIC) ||
        (in_len != (sizeof(struct devdrv_manager_msg_head) + sizeof(struct devdrv_core_utilization)))) {
        devdrv_drv_err("Invalid message from host. (dev_id=%u; valid=%u; in_len=%u)\n",
                       devid, dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }
    util_info = (struct devdrv_core_utilization *)dev_manager_msg_info->payload;

    dev_id = dev_manager_msg_info->header.dev_id;
    vfid = dev_manager_msg_info->header.vfid;
    dev_info = devdrv_manager_get_devdrv_info(devid);
    if (dev_id != devdrv_manager_get_devid(devid) || dev_info == NULL) {
        devdrv_drv_warn("Invalid device id, dev_id = %u, "
            "dev_manager_info->dev_id = %u, "
            "dev_info is %s\n",
            dev_id, devdrv_manager_get_devid(devid), (dev_info == NULL) ? "NULL" : "not NULL");
        return -EAGAIN;
    }

    if (util_info->core_type == DEV_DRV_TYPE_AICORE || util_info->core_type == DEV_DRV_TYPE_AIVECTOR) {
        ts_info.vfid = vfid;
        ts_info.dev_id = devid;
        ts_info.core_id = LPM_AICORE0_ID;
        ret = dms_calc_aicore_aivector_utilization(devid, util_info->core_type, &ts_info, &(util_info->utilization));
        if (ret != 0) {
            devdrv_drv_err("Get aicore or aivector utilization failed. (dev_id=%u; ret=%d)", dev_id, ret);
            return ret;
        }
    } else if (util_info->core_type == DEV_DRV_TYPE_AICPU) {
        if (calc_aicpu_func == NULL) {
            calc_aicpu_func =
                (soc_misc_calc_aicpu_utilization)(uintptr_t)__kallsyms_lookup_name("soc_misc_calc_aicpu_utilization");
            if (calc_aicpu_func == NULL) {
                devdrv_drv_err("Can't find soc_misc_calc_aicpu_utilization. (dev_id=%u)\n", dev_id);
                return -EFAULT;
            }
        }

        ret = calc_aicpu_func(devid, &(util_info->utilization));
        if (ret != 0) {
            devdrv_drv_err("Get aicpu utilization failed. (dev_id=%u, ret=%d)\n", dev_id, ret);
            return ret;
        }
    } else {
        devdrv_drv_err("Cmd is not support. (dev_id=%u)\n", dev_id);
        return -EOPNOTSUPP;
    }

    dev_manager_msg_info->header.result = 0;
    dev_manager_msg_info->header.valid = DEVDRV_MANAGER_MSG_D2H_MAGIC;
    *ack_len = sizeof(struct devdrv_manager_msg_head) + sizeof(struct devdrv_core_utilization);
    return 0;
}


