/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
#include "devdrv_atu_interface.h"
#include "devdrv_atu.h"
#include "devdrv_util.h"

int devdrv_atu_base_to_target(const struct devdrv_iob_atu atu[], int num, u64 base_addr, u64 *target_addr)
{
    int i;

    for (i = 0; i < num; i++) {
        if (atu[i].valid == ATU_INVALID)
            continue;

        if ((atu[i].base_addr <= base_addr) && ((atu[i].base_addr + atu[i].size) > base_addr)) {
            *target_addr = base_addr - atu[i].base_addr + atu[i].target_addr;
            return 0;
        }
    }
    return -EINVAL;
}

int devdrv_atu_target_to_base(const struct devdrv_iob_atu atu[], int num, u64 target_addr, u64 *base_addr)
{
    int i;

    for (i = 0; i < num; i++) {
        if (atu[i].valid == ATU_INVALID)
            continue;

        if ((atu[i].target_addr <= target_addr) && ((atu[i].target_addr + atu[i].size) > target_addr)) {
            *base_addr = target_addr - atu[i].target_addr + atu[i].base_addr;
            return 0;
        }
    }

    return -EINVAL;
}

int devdrv_devmem_addr_d2h(u32 devid, phys_addr_t device_phy_addr, phys_addr_t *host_bar_addr)
{
    struct devdrv_iob_atu *atu = NULL;
    u64 host_phy_base = 0;

    if (host_bar_addr == NULL) {
        devdrv_err("Device host_bar_addr is null. (dev_id=%u)\n", devid);
        return -EINVAL;
    }

    if (devdrv_get_atu_info(devid, ATU_TYPE_RX_MEM, &atu, &host_phy_base)) {
        devdrv_err("Device find atu failed. (dev_id=%u)\n", devid);
        return -EINVAL;
    }

    if (devdrv_atu_target_to_base(atu, DEVDRV_MAX_RX_ATU_NUM, (u64)device_phy_addr, (u64 *)host_bar_addr)) {
        devdrv_warn("Device phy_addr not found. (dev_id=%u)\n", devid);
        return -EFAULT;
    }

    *host_bar_addr += host_phy_base;

    return 0;
}
EXPORT_SYMBOL(devdrv_devmem_addr_d2h);

int devdrv_devmem_addr_h2d(u32 devid, phys_addr_t host_bar_addr, phys_addr_t *device_phy_addr)
{
    struct devdrv_iob_atu *atu = NULL;
    u64 host_phy_base = 0;

    if (device_phy_addr == NULL) {
        devdrv_err("Device phy_addr is null. (dev_id=%u)\n", devid);
        return -EINVAL;
    }

    if (devdrv_get_atu_info(devid, ATU_TYPE_RX_MEM, &atu, &host_phy_base)) {
        devdrv_err("Device find atu failed. (dev_id=%u)\n", devid);
        return -EINVAL;
    }

    if (host_bar_addr < host_phy_base) {
        devdrv_err("Device host_bar_addr is small than host_phy_base. (dev_id=%u)\n", devid);
        return -EINVAL;
    }

    if (devdrv_atu_base_to_target(atu, DEVDRV_MAX_RX_ATU_NUM, (u64)(host_bar_addr - host_phy_base),
        (u64 *)device_phy_addr)) {
        devdrv_err("Device host_bar_addr not found. (dev_id=%u)\n", devid);
        return -EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_devmem_addr_h2d);
