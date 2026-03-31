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

#include <asm/io.h>
#include "apb_drv.h"
#include "agentdrv_unit.h"

devdrv_hw_info_t g_hw_info = {0};

/* get the same os's dev_id */
int devdrv_get_devid_by_bus(unsigned char bus_number, int *devid)
{
    int bus_offset;

    if ((bus_number < DEVDRV_BASE_BUS_ID) || (bus_number > DEVDRV_MAX_BUS_ID)) {
        devdrv_err("bus_number %d invalid\n", bus_number);
        return -EINVAL;
    }

    bus_offset = bus_number - DEVDRV_BASE_BUS_ID;
    *devid = bus_offset % AGENTDRV_MAX_DIE_NUM;

    return 0;
}

int agentdrv_get_connect_type_by_hw_info(void)
{
    return CONNECT_PROTOCOL_PCIE;
}

unsigned char agentdrv_get_mainboard_id_by_hw_info(void)
{
    return g_hw_info.mainboard_id;
}

/* diff os's chip offset addr size */
int agentdrv_get_chip_offset_by_devid(int devid, unsigned long long *chip_offset)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u64 chip_offset_size = 0;
    u32 chip_id = 0;
    u32 func_id = 0;
    u32 bus_offset;

    devdrv_dev2chipfunc((u32)devid, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agent_dev->pdev == NULL)) {
        devdrv_err("Get agent dev fail by chip id\n");
        return -EINVAL;
    }

    chip_offset_size = DEVDRV_DIFF_OS_CHIP_OFFSET_SIZE;
    bus_offset = (u32)(agent_dev->pdev->bus->number - DEVDRV_BASE_BUS_ID);
    *chip_offset = (bus_offset / AGENTDRV_MAX_DIE_NUM) * chip_offset_size;

    return 0;
}

int agentdrv_get_rc_ep_mode(u32 *mode)
{
    if (mode == NULL) {
        return -EINVAL;
    }

    *mode = DEVDRV_PCIE_EP_MODE;

    return 0;
}
EXPORT_SYMBOL(agentdrv_get_rc_ep_mode);

int devdrv_get_pf_type(void)
{
    return DEVDRV_DAVINCI_DEV_NUM_1PF1P;
}

STATIC int agentdrv_get_chip_id_and_chip_offset(u32 *chip_id, u64 *chip_offset)
{
    void __iomem *chip_info_addr = NULL;
    devdrv_hw_info_t *hw_data = NULL;

#ifndef DRV_UT
    chip_info_addr = ioremap_cache(DEVDRV_CHIP_ID_INFO_ADDR, sizeof(devdrv_hw_info_t));
    if (chip_info_addr == NULL) {
        devdrv_err("Ioremap chip id address error.\n");
        return -1;
    }
    hw_data = (devdrv_hw_info_t __iomem *)chip_info_addr;
    g_hw_info.chip_id = hw_data->chip_id;
    g_hw_info.multi_chip = hw_data->multi_chip;
    g_hw_info.multi_die = hw_data->multi_die;
    g_hw_info.mainboard_id = hw_data->mainboard_id;
    g_hw_info.hccs_connect_status = hw_data->hccs_connect_status;
    g_hw_info.board_id = hw_data->board_id;

    iounmap(chip_info_addr);
    chip_info_addr = NULL;

    if (g_hw_info.chip_id >= DEVDRV_MAX_CHIP_ID) {
        devdrv_err("Get chip id(%u) is invalid.\n", *chip_id);
        return -1;
    }

    *chip_id = g_hw_info.chip_id;
    *chip_offset = DEVDRV_DIFF_OS_CHIP_OFFSET_SIZE;

#else
    *chip_id = 0;
    *chip_offset = 0;
#endif

    devdrv_info("Get hw info.(chip_id=%u, hccs_connect_status=%u)\n", g_hw_info.chip_id, g_hw_info.hccs_connect_status);
    return 0;
}

u32 agentdrv_soc_get_pf_func_total(void)
{
    void __iomem *rd_base = NULL;
    u32 pf_support_enable;
    u32 pf_total_num = 0;
    u64 chip_offset;
    u32 chip_id;
    int ret;

    ret = agentdrv_get_chip_id_and_chip_offset(&chip_id, &chip_offset);
    if (ret != 0) {
        return 1;
    }

    rd_base = ioremap(chip_id * chip_offset + DEVDRV_RM_PF_SUPPOT_ENABLE, sizeof(u32));
    if (rd_base == NULL) {
        return DEVDRV_PF_ENABLE_SINGLE;
    }
    pf_support_enable = readl(rd_base);
    iounmap(rd_base);
    rd_base = NULL;

    if ((pf_support_enable & DEVDRV_PF_ENABLE_MASK) == DEVDRV_PF_ENABLE_MASK) {
        pf_total_num = DEVDRV_PF_ENABLE_DOUBLE;
    } else {
        pf_total_num = DEVDRV_PF_ENABLE_SINGLE;
    }

    devdrv_info("Get pf totl num in device. (val=%u; pf_num=%u; hccs_status=%u)\n",
        pf_support_enable, pf_total_num, g_hw_info.hccs_connect_status);

    return pf_total_num;
}

u32 agentdrv_soc_get_func_total(void)
{
    u32 total_num = 0;
    u32 pf_total_num = 0;
    u32 vf_total_num = 0;

    pf_total_num = agentdrv_soc_get_pf_func_total();
    if (pf_total_num > 1) {
        /* 1PCIe+2PF */
        vf_total_num = 0;
    } else {
        /* 1PCIe+1PF */
        vf_total_num = DEVDRV_SUPPORT_MAX_VF_NUM;
    }
    total_num = pf_total_num + vf_total_num;

    return total_num;
}

int devdrv_check_dlcmsm(const void __iomem *io_base)
{
    return 0;
}

void agentdrv_flush_cache(u64 base, u32 len)
{
#ifdef __aarch64__
    u64 addr_loop, addr_end;

    addr_loop = base & (~AGENTDRV_CACHELINE_MASK);
    addr_end = (base + len + AGENTDRV_CACHELINE_MASK) & (~AGENTDRV_CACHELINE_MASK);

    asm volatile("dsb st"
                 :
                 :
                 : "memory");
    for (; addr_loop < addr_end;) {
        asm volatile("DC CIVAC ,%x0" ::"r"(addr_loop));
        mb();
        addr_loop += AGENTDRV_CACHELINE_SIZE;
    }
    asm volatile("dsb st"
                 :
                 :
                 : "memory");
#endif
}
