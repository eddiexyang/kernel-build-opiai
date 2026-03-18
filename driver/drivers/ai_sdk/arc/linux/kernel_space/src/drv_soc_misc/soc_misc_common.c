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

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include "soc_misc_common.h"
#include "soc_misc_init.h"
#include "devdrv_manager_comm.h"

unsigned int soc_misc_dev_num(unsigned int dev_num)
{
    return (dev_num < SOC_MISC_DEVICE_NUM_MAX) ? (dev_num) : (SOC_MISC_DEVICE_NUM_MAX);
}

int soc_misc_check_dev_id(unsigned int dev_id)
{
#ifndef CFG_FEATURE_DEVICE_DEVID_CONVERT
    int ret;
    unsigned int dev_num = 0;

    ret = devdrv_get_devnum(&dev_num);
    if (ret != 0) {
        soc_misc_drv_err("Get dev_num failed. (ret=%d)\n", ret);
        return ret;
    }

    if (dev_id >= soc_misc_dev_num(dev_num)) {
        soc_misc_drv_err("Device id not vailid. (dev_num=%u; dev_id=%u)\n", dev_num, dev_id);
        return -EINVAL;
    }
#endif
    return 0;
}

#ifndef AOS_LLVM_BUILD
/*
 * register operation, read or write
 */
int soc_misc_reg_op(unsigned char op_type, unsigned long base_phy_reg, unsigned long reg_offset,
    unsigned long map_size, unsigned int *val)
{
    void __iomem *regs = NULL;

    regs = ioremap(base_phy_reg, map_size);
    if (regs == NULL) {
        soc_misc_drv_err("Remap register space failed.\n");
        return -ENOMEM;
    }

    if (op_type == REG_OP_TYPE_WR) {
        writel(*val, (void __iomem *)(uintptr_t)((long)(uintptr_t)regs + reg_offset));
    } else {
        *val = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)regs + reg_offset));
    }

    (void)iounmap(regs);
    regs = NULL;
    return 0;
}
#endif

