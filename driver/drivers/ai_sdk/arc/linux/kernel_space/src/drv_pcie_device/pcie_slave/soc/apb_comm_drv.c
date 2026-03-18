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

#include <linux/delay.h>
#include <linux/errno.h>

#include "apb_drv.h"
#include "devdrv_atu.h"
#include "devdrv_interface.h"

void devdrv_apb_reg_wr(void __iomem *io_base, u32 offset, u32 val)
{
    writel(val, (io_base + offset));
}

void devdrv_apb_reg_rd(const void __iomem *io_base, u32 offset, u32 *val)
{
    *val = readl(io_base + offset);
}

void devdrv_rx_atu_init(const void __iomem *io_base, u32 pf_num, u32 bar_num, struct devdrv_iob_atu atu[], int num)
{
    int i, atu_id = 0;
    u32 var;
    u32 offset;

    for (i = 0; i < DEVDRV_RX_ATU_NUM; i++) {
        if (atu_id >= num)
            break;

        offset = i * DEVDRV_RX_ATU_REG_SIZE;
        devdrv_apb_reg_rd(io_base, offset + DEVDRV_RX_ATU_CONTROL0_REG, &var);

        /* disable */
        if ((var & RX_ATU_ENABLEE_MASK) == 0)
            continue;

        /* not pf mode */
        if ((var & RX_ATU_FUNC_MODE_MASK) != (RX_ATU_FUNC_MODE_PF << RX_ATU_FUNC_MODE_OFFSET))
            continue;

        devdrv_apb_reg_rd(io_base, offset + DEVDRV_RX_ATU_CONTROL1_REG, &var);

        /* diff pf num */
        if ((var & RX_ATU_PF_NUM_MASK) != (pf_num << RX_ATU_PF_NUM_OFFSET))
            continue;

        /* diff bar num */
        if ((var & RX_ATU_BAR_NUM_MASK) != (bar_num << RX_ATU_BAR_NUM_OFFSET))
            continue;

        /* size */
        devdrv_apb_reg_rd(io_base, offset + DEVDRV_RX_ATU_CONTROL2_REG, &var);
        atu[atu_id].size = (u64)var << 32;
        devdrv_apb_reg_rd(io_base, offset + DEVDRV_RX_ATU_REGION_SIZE_REG, &var);
        atu[atu_id].size |= var;

        /* base addr */
        devdrv_apb_reg_rd(io_base, offset + DEVDRV_RX_ATU_BASE_H_REG, &var);
        atu[atu_id].base_addr = (u64)var << 32;
        devdrv_apb_reg_rd(io_base, offset + DEVDRV_RX_ATU_BASE_L_REG, &var);
        atu[atu_id].base_addr |= var;

        /* target addr */
        devdrv_apb_reg_rd(io_base, offset + DEVDRV_RX_ATU_TAR_H_REG, &var);
        atu[atu_id].target_addr = (u64)var << 32;
        devdrv_apb_reg_rd(io_base, offset + DEVDRV_RX_ATU_TAR_L_REG, &var);
        atu[atu_id].target_addr |= var;

        atu[atu_id].valid = ATU_VALID;
        atu[atu_id].atu_id = atu_id;

        devdrv_info("Get atu information. (rx_atu_index=%d; atu_id=%d; size=0x%llx)\n", i, atu_id, atu[atu_id].size);

        atu_id++;
    }

    devdrv_info("Get parameter information. (pf_num=%d; bar_num=%d; total_rx_atu_num=%d)\n", pf_num, bar_num, atu_id);
}

void devdrv_add_tx_atu(void __iomem *io_base, u32 atu_id, u32 pf_num, const struct devdrv_iob_atu *atu)
{
    u32 offset;
    u32 val;

    if (atu_id >= DEVDRV_TX_ATU_NUM) {
        devdrv_info("Input parameter is invalid. (atu_id=%u)\n", atu_id);
        return;
    }

    offset = atu_id * DEVDRV_TX_ATU_REG_SIZE;

    val = (pf_num << TX_ATU_PF_NUM_OFFSET) & TX_ATU_PF_NUM_MASK;
    val |= (TX_ATU_EP_NUM_CORE1_PORT0 << TX_ATU_EP_NUM_OFFSET) & TX_ATU_EP_NUM_MASK;
    val |= (TX_ATU_TYPE_TRANS_MODE_MEM << TX_ATU_TYPE_TRANS_MODE_OFFSET) & TX_ATU_TYPE_TRANS_MODE_MASK;
    val |= (TX_ATU_WORK_MODE_EP << TX_ATU_WORK_MODE_OFFSET) & TX_ATU_WORK_MODE_MASK;
    val |= (0x1 << TX_ATU_EN_OFFSET) & TX_ATU_EN_MASK;
    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_CONTROL0_REG, val);

    /* size */
    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_CONTROL2_REG, (u32)(atu->size >> 32));
    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_REGION_SIZE_REG, (u32)(atu->size));

    /* base */
    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_BASE_H_REG, (u32)(atu->base_addr >> 32));
    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_BASE_L_REG, (u32)(atu->base_addr));

    /* target */
    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_TAR_H_REG, (u32)(atu->target_addr >> 32));
    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_TAR_L_REG, (u32)(atu->target_addr));
}

void devdrv_del_tx_atu(void __iomem *io_base, u32 atu_id, u32 pf_num, struct devdrv_iob_atu *atu)
{
    u32 offset;
    u32 val;
    u32 set_pf_num;

    if (atu_id >= DEVDRV_TX_ATU_NUM) {
        devdrv_info("Input parameter is invalid. (atu_id=%d)\n", atu_id);
        return;
    }

    offset = atu_id * DEVDRV_TX_ATU_REG_SIZE;

    devdrv_apb_reg_rd(io_base, offset + DEVDRV_TX_ATU_CONTROL0_REG, &val);

    /* atu in invalid */
    if ((val & TX_ATU_EN_MASK) == 0) {
        devdrv_info("Parameter atu don't need delete. (atu_id=%d)\n", atu_id);
        return;
    }

    /* pf not match */
    set_pf_num = (val & TX_ATU_PF_NUM_MASK) >> TX_ATU_PF_NUM_OFFSET;
    if (set_pf_num != pf_num) {
        devdrv_info("Parameter set_pf_num and pf_num not match. (atu_id=%d; set_pf_num=%d; pf_num=%d;)\n",
                    atu_id, set_pf_num, pf_num);
        return;
    }

    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_CONTROL0_REG, 0);

    atu->base_addr = 0;
    atu->size = 0;
    atu->target_addr = 0;
    /* size */
    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_CONTROL2_REG, 0);
    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_REGION_SIZE_REG, 0);

    /* base */
    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_BASE_H_REG, 0);
    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_BASE_L_REG, 0);

    /* target */
    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_TAR_H_REG, 0);
    devdrv_apb_reg_wr(io_base, offset + DEVDRV_TX_ATU_TAR_L_REG, 0);
}

int devdrv_get_tx_atu(const void __iomem *io_base, u32 atu_id, u32 pf_num, struct devdrv_iob_atu *atu)
{
    u32 offset;
    u32 val;
    u32 set_pf_num;

    if (atu_id >= DEVDRV_TX_ATU_NUM) {
        devdrv_info("Input parameter is invalid. (atu_id=%u)\n", atu_id);
        return -EINVAL;
    }

    offset = atu_id * DEVDRV_TX_ATU_REG_SIZE;

    devdrv_apb_reg_rd(io_base, offset + DEVDRV_TX_ATU_CONTROL0_REG, &val);

    /* atu in invalid */
    if ((val & TX_ATU_EN_MASK) == 0) {
        return -EINVAL;
    }

    /* pf not match */
    set_pf_num = (val & TX_ATU_PF_NUM_MASK) >> TX_ATU_PF_NUM_OFFSET;
    if (set_pf_num != pf_num) {
        devdrv_info("Parameter set_pf_num and pf_num not match. (atu_id=%u; set_pf_num=%u; pf_num=%u;)\n",
                    atu_id, set_pf_num, pf_num);
        return -EINVAL;
    }

    /* size */
    devdrv_apb_reg_rd(io_base, offset + DEVDRV_TX_ATU_CONTROL2_REG, &val);
    atu->size = (u64)val << DEVDRV_TX_ATU_REG_SHIFT_32;
    devdrv_apb_reg_rd(io_base, offset + DEVDRV_TX_ATU_REGION_SIZE_REG, &val);
    atu->size |= val;

    /* base */
    devdrv_apb_reg_rd(io_base, offset + DEVDRV_TX_ATU_BASE_H_REG, &val);
    atu->base_addr = (u64)val << DEVDRV_TX_ATU_REG_SHIFT_32;
    devdrv_apb_reg_rd(io_base, offset + DEVDRV_TX_ATU_BASE_L_REG, &val);
    atu->base_addr |= val;

    /* target */
    devdrv_apb_reg_rd(io_base, offset + DEVDRV_TX_ATU_TAR_H_REG, &val);
    atu->target_addr = (u64)val << DEVDRV_TX_ATU_REG_SHIFT_32;
    devdrv_apb_reg_rd(io_base, offset + DEVDRV_TX_ATU_TAR_L_REG, &val);
    atu->target_addr |= val;

    return 0;
}

/*
 * p_type stores the platform type: 0-FPGA 1-EMU 2-ESL 3-ASIC
 * version stores the soc version: 0x300 matchs b300,
 * 0x201 matchs b201 , as so on.
 */
int devdrv_get_platform_type(unsigned int *p_type, unsigned int *version)
{
    void __iomem *sysctl_base = NULL;
    unsigned int reg_val;
    unsigned int type_val;
    int ret = 0;

    if ((p_type == NULL) || (version == NULL)) {
        devdrv_err("Input parameter is invalid.\n");
        return -1;
    }

    sysctl_base = ioremap(DEVDRV_SYSCTL_BASE_ADDR, DEVDRV_SYSCTL_SIZE);
    if (sysctl_base == NULL) {
        devdrv_err("Calling ioremap failed.\n");
        return -1;
    }

    reg_val = readl_relaxed((void *)((u64)(uintptr_t)sysctl_base + DEVDRV_SYSCTL_PLTFORM_REG));
    type_val = reg_val >> DEVDRV_PLTFORM_TYPE_SHIFT_16;

    *version = reg_val & DEVDRV_PLTFORM_TYPE_MASK_16_BIT;
    *p_type = DEVDRV_PLTFORM_TYPE_MASK_8_BIT;

    if (((type_val & DEVDRV_PLATFORM_TYPE_MASK) == 0) && (*version == 0)) {
        *p_type = DEVDRV_PLATFORM_TYPE_ASIC;
    } else if ((type_val & DEVDRV_PLATFORM_TYPE_MASK) == DEVDRV_PLATFORM_TYPE_FPGA) {
        *p_type = DEVDRV_PLATFORM_TYPE_FPGA;
    } else if ((type_val & DEVDRV_PLATFORM_TYPE_MASK) == DEVDRV_PLATFORM_TYPE_ESL) {
        *p_type = DEVDRV_PLATFORM_TYPE_ESL;
    } else if ((type_val & DEVDRV_PLATFORM_TYPE_MASK) == DEVDRV_PLATFORM_TYPE_EMU) {
        *p_type = DEVDRV_PLATFORM_TYPE_EMU;
    } else {
        devdrv_err("Platform type error, check soc config.\n");
        ret = -1;
    }

    *version = reg_val & DEVDRV_PLTFORM_TYPE_MASK_16_BIT;
    iounmap(sysctl_base);
    sysctl_base = NULL;
    return ret;
}
EXPORT_SYMBOL(devdrv_get_platform_type);

