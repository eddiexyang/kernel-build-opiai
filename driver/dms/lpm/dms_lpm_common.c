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

#include "dms_lpm.h"
#include "dms_lpm_common.h"
#include "dms_ipc_interface.h"
#include "devdrv_manager_common.h"

int dms_lpm_id_check(u32 dev_id, u32 core_id)
{
    int ret;
    u32 dev_num;

    ret = devdrv_get_devnum(&dev_num);
    if (ret != 0) {
        dms_err("Get device num failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    if (dev_id >= dev_num) {
        dms_err("Device id not valied. (dev_id=%u; dev_num=%u)\n", dev_id, dev_num);
        return -ENODEV;
    }

    if (core_id >= LPM_INVALID_ID) {
        dms_err("Core id not valied. (dev_id=%u; core_id=%u)\n", dev_id, core_id);
        return -EINVAL;
    }

    return 0;
}

int dms_lpm_reg_op(unsigned char op_type, unsigned long base_phy_reg, unsigned long reg_offset,
    unsigned long map_size, unsigned int *val)
{
    void __iomem *regs = NULL;

    if ((map_size < sizeof(unsigned int)) || (reg_offset > (map_size - sizeof(unsigned int)))) {
        dms_err("Invalid offset. (offset=%lu; map_size=%lu)\n", reg_offset, map_size);
        return -EINVAL;
    }

    regs = ioremap(base_phy_reg, map_size);
    if (regs == NULL) {
        dms_err("Remap register space failed.\n");
        return DRV_ERROR_INVALID_HANDLE;
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

int dms_lpm_sharemem_op(unsigned char op_type, unsigned long base_phy_addr, unsigned long offset,
    unsigned long map_size, unsigned int *val)
{
    void __iomem *viraddr = NULL;

    if ((map_size < sizeof(unsigned int)) || (offset > (map_size - sizeof(unsigned int)))) {
        dms_err("Invalid offset. (offset = %lu; map_size = %lu)\n", offset, map_size);
        return -EINVAL;
    }

    viraddr = ioremap(base_phy_addr, map_size);
    if (viraddr == NULL) {
        dms_err("Remap memory address space failed.\n");
        return -EFAULT;
    }

    if (op_type == REG_OP_TYPE_WR) {
        writel(*val, (void __iomem *)(uintptr_t)((long)(uintptr_t)viraddr + offset));
    } else if (op_type == REG_OP_TYPE_RD) {
        *val = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)viraddr + offset));
    }

    (void)iounmap(viraddr);
    viraddr = NULL;
    return 0;
}

int dms_lpm_sharemem_op_u16(unsigned char op_type, unsigned long base_phy_addr, unsigned long offset,
    unsigned long map_size, unsigned short *val)
{
    void __iomem *viraddr = NULL;

    if ((map_size < sizeof(unsigned short)) || (offset > (map_size - sizeof(unsigned short)))) {
        dms_err("Invalid offset. (offset = %lu; map_size = %lu)\n", offset, map_size);
        return -EINVAL;
    }

    viraddr = ioremap(base_phy_addr, map_size);
    if (viraddr == NULL) {
        dms_err("Remap memory address space failed.\n");
        return -EFAULT;
    }

    if (op_type == REG_OP_TYPE_WR) {
        writew(*val, (void __iomem *)(uintptr_t)((long)(uintptr_t)viraddr + offset));
    } else if (op_type == REG_OP_TYPE_RD) {
        *val = readw((void __iomem *)(uintptr_t)((long)(uintptr_t)viraddr + offset));
    }

    (void)iounmap(viraddr);
    viraddr = NULL;
    return 0;
}

unsigned short dms_crc16(unsigned char *data, unsigned short len)
{
    unsigned short val = DMS_NULL_USHORT;
    const unsigned short poly = DMS_CRC_POLYNOMIAL;
    unsigned char ch;
    unsigned int i;

    while (len--) {
        ch = *(data++);
        val ^= (ch << DMS_BITS_PER_BYTE);
        for (i = 0; i < DMS_BITS_PER_BYTE; i++) {
            if (val & DMS_BIT15) {
                val = (val << 1) ^ poly;
            } else {
                val = val << 1;
            }
        }
    }
    return (val);
}
