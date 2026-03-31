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
#include <linux/securec.h>

#include "agentdrv_profiling.h"
#include "agentdrv_unit.h"
#include "devdrv_util.h"

#include "drv_profile.h"

STATIC int agentdrv_is_devid_valid(int dev_id)
{
    enum {
        INVALID = 0,
        VALID
    };

    if ((dev_id < 0) || (dev_id >= MAX_AGENTDEV_CNT)) {
        devdrv_err("Device ID is invalid. (dev_id=%d)\n", dev_id);
        return INVALID;
    }
    return VALID;
}

STATIC struct agentdrv_devctrl *agentdrv_get_valid_device_by_id(int dev_id)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;

    if (agentdrv_is_devid_valid(dev_id) == 0) {
        devdrv_err("Device ID is invalid. (dev_id=%d)\n", dev_id);
        return NULL;
    }

    devdrv_dev2chipfunc((u32)dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev(chip_id);
    if ((agent_dev != NULL) && (agentdrv_is_dev_valid(agent_dev, func_id) == false)) {
        devdrv_err("Device is invalid. (dev_id=%d)\n", dev_id);
        return NULL;
    }

    return agent_dev;
}

int agentdrv_pcie_profiling_open(struct prof_peri_para para)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    void __iomem *io_base = NULL;
    unsigned int dev_id = para.device_id;

    agent_dev = agentdrv_get_valid_device_by_id(dev_id);
    if (agent_dev == NULL) {
        devdrv_err("Get device failed. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    io_base = agent_dev->apb_base;

    return agentdrv_profling_open_dfx(agent_dev->agent_id, io_base);
}
EXPORT_SYMBOL(agentdrv_pcie_profiling_open);

int agentdrv_pcie_profiling_sampling(struct prof_peri_para para)
{
    struct agentdrv_profiling_buf info;
    struct agentdrv_devctrl *agent_dev = NULL;
    void __iomem *io_base = NULL;
    int write_len = sizeof(struct agentdrv_profiling_buf);
    unsigned int dev_id = para.device_id;
    void *buf = para.buff;
    int len = para.buff_len;

    agent_dev = agentdrv_get_valid_device_by_id(dev_id);
    if (agent_dev == NULL) {
        devdrv_err("Get device failed. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    if (buf == NULL) {
        devdrv_err("Device buf is null. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    /* len: 10K == (10 * 1024) */
    if ((((unsigned int)len) < sizeof(struct agentdrv_profiling_buf)) || (len < 0)) {
        devdrv_err("Intput buff_len is invalid. (dev_id=%u; buf_len=%d; defined_len=%ld)\n",
                   dev_id, len, sizeof(struct agentdrv_profiling_buf));
        return -EINVAL;
    }

    io_base = agent_dev->apb_base;

    if (agentdrv_profiling_get_info(agent_dev->agent_id, io_base, &info) != 0) {
        devdrv_err("Getting profiling info failed. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    if (memcpy_s(buf, (size_t)len, &info, sizeof(struct agentdrv_profiling_buf)) != 0) {
        devdrv_err("Information memcpy_s failed. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    return write_len;
}
EXPORT_SYMBOL(agentdrv_pcie_profiling_sampling);

int agentdrv_pcie_profiling_close(struct prof_peri_para para)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    void __iomem *io_base = NULL;
    unsigned int dev_id = para.device_id;

    agent_dev = agentdrv_get_valid_device_by_id(dev_id);
    if (agent_dev == NULL) {
        devdrv_err("Get device failed. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    io_base = agent_dev->apb_base;

    return agentdrv_profiling_close_dfx(agent_dev->agent_id, io_base);
}
EXPORT_SYMBOL(agentdrv_pcie_profiling_close);
