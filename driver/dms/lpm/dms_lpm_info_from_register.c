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

#include <linux/fs.h>
#include <linux/of_gpio.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include "dms_lpm.h"
#include "dms_lpm_common.h"
#include "dms_lpm_info.h"
#include "dms_ipc_interface.h"
#include "ascend_platform.h"

int lpm_get_power_from_register(u32 dev_id, u32 core_id, u32 *power)
{
    int ret;
    unsigned int power_info[POWER_LEVEL] = {POWER_LEVEL_0, POWER_LEVEL_1,
                                            POWER_LEVEL_2, POWER_LEVEL_3};
    unsigned int reg_val = 0;

    ret = dms_lpm_reg_op(REG_OP_TYPE_RD, MINI_BOARD_POWER_REG_BASE,
        MINI_BOARD_POWER_REG_OFFSET, MAP_PAGE_SIZE, &reg_val);
    if (ret != 0) {
        dms_err("dms_lpm_reg_op failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    reg_val = (reg_val & POWER_LEVEL_MASK);
    if (reg_val >= POWER_LEVEL) {
        dms_err("Power reg value error. (dev_id=%u; reg_val=%u)\n", dev_id, reg_val);
        return -EINVAL;
    }

    *power = power_info[reg_val];

    return ret;
}

int lpm_get_ccpu_freq(u32 dev_id, u32 *ccpu_freq)
{
    unsigned long base_addr;
    u32 pll_ctrl;
    u32 pll_ctrl_frac;
    u32 ref_div;
    u32 fb_div;
    u32 post_div1;
    u32 post_div2;
    u32 frac;
    u32 fout_vco;
    int ret;

    base_addr = dev_id * CHIP_OFFSET + PLL_CTRL_BASE;

    ret = dms_lpm_reg_op(REG_OP_TYPE_RD, base_addr, PLL0_FCTRL_OFFSET, MAP_PAGE_SIZE, &pll_ctrl);
    if (ret != 0) {
        dms_err("dms_reg_op failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = dms_lpm_reg_op(REG_OP_TYPE_RD, base_addr, PLL0_FCTRL_FRAC_OFFSET, MAP_PAGE_SIZE, &pll_ctrl_frac);
    if (ret != 0) {
        dms_err("dms_reg_op failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ref_div = (pll_ctrl & REF_DIV_MASK);
    fb_div = (pll_ctrl >> FB_DIV_OFFSET) & FB_DIV_MASK;
    post_div1 = (pll_ctrl >> POST_DIV1_OFFSET) & POST_DIV1_MASK;
    post_div2 = (pll_ctrl >> POST_DIV2_OFFSET) & POST_DIV2_MASK;
    frac = pll_ctrl_frac & FRAC_MSAK;

    if ((ref_div == 0) || (post_div1 == 0) || (post_div2 == 0)) {
        dms_err("Data error. (ref_div=%u; post_div1=%u; post_div2=%u)\n", ref_div, post_div1, post_div2);
        return -EINVAL;
    }

    fout_vco = ((CCPU_FREF / ref_div) * (fb_div + frac / TOTAL_QUAN));
    *ccpu_freq = (fout_vco / post_div1) / post_div2;
    return 0;
}

int lpm_get_sharemem_head_info(unsigned long base_phy_addr, unsigned long map_size, struct dms_sharemem_header *head)
{
    void __iomem *viraddr = NULL;
    unsigned long type_addr;
    unsigned int i;
    unsigned short crc16_value = 0;
    unsigned short crc16_check_len = 0;
    int ret = 0;

    if (head == NULL) {
        dms_err("Parameter error.\n");
        return -EINVAL;
    }

    viraddr = ioremap(base_phy_addr, map_size);
    if (viraddr == NULL) {
        dms_err("Remap memory address space failed.\n");
        return -EFAULT;
    }

    head->magic_num = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)viraddr));
    head->version = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)viraddr + sizeof(u32)));
    head->entry_num = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)viraddr + sizeof(u32) * 2));
    if (head->entry_num > DMS_MEM_MAX) {
        dms_err("Incorrect entry_num. (entry_num=%u)\n", head->entry_num);
        ret = -EINVAL;
        goto IOUNMAP;
    }
    crc16_check_len = sizeof(u32) + sizeof(u32) + sizeof(u32);

    for (i = 0; i < head->entry_num; i++) {
        type_addr = (unsigned long)viraddr + (sizeof(u32) + sizeof(u32)+ sizeof(u32)) +
            (sizeof(struct dms_info_entry) * i);

        head->entry[i].type = readl((void __iomem *)(uintptr_t)type_addr);
        head->entry[i].offset = readl((void __iomem *)(uintptr_t)(type_addr + sizeof(u32)));
        head->entry[i].len = readl((void __iomem *)(uintptr_t)(type_addr + (sizeof(u32) * 2)));
        crc16_check_len += sizeof(struct dms_info_entry);
    }
    head->crc16 = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)viraddr + crc16_check_len));

    crc16_value = dms_crc16((u8 *)head, crc16_check_len);
    if (crc16_value != head->crc16) {
        dms_err("The CRC16 check fails, and the memory is overwritten. (actual=%u; expect=%u)\n",
            crc16_value, head->crc16);
        ret = -EINVAL;
    }

IOUNMAP:
    (void)iounmap(viraddr);
    viraddr = NULL;
    return ret;
}

int lpm_get_sharemem_entry_info(struct dms_info_entry *info)
{
    int ret;
    struct dms_sharemem_header head = {0};

    if (info == NULL) {
        dms_err("Parameter error.\n");
        return -EINVAL;
    }

    if (info->type >= DMS_MEM_MAX) {
        dms_err("Get share memory entry info type error.\n");
        return -EINVAL;
    }

    ret = lpm_get_sharemem_head_info(SHAREMEM_BASE_ADDR, sizeof(struct dms_sharemem_header), &head);
    if ((ret != 0) || (head.magic_num != SHAREMEM_MAGIC_NUM)) {
        dms_err("Get head information failed. (ret=%d; magic_num=%#x)\n", ret, head.magic_num);
        return -ENODATA;
    }

    if (info->type != head.entry[info->type].type) {
        dms_err("Get head information type not match. (info.type=%u; entry.type=%u)\n", info->type,
            head.entry[info->type].type);
        return -ENODATA;
    }

    info->offset = head.entry[info->type].offset;
    info->len = head.entry[info->type].len;

    if ((info->offset > SHAREMEM_MAX_SIZE) || (info->len > SHAREMEM_MAX_SIZE) ||
        ((info->offset + info->len) > SHAREMEM_MAX_SIZE)) {
        dms_err("Offset or length over max value. (offset=%u; len=%u)\n", info->offset, info->len);
        return -ENODATA;
    }

    return 0;
}

int lpm_get_temperature_from_sharemem(u32 dev_id, u32 core_id, u32 *temperature)
{
    int ret;
    unsigned short val_u16;
    struct dms_info_entry info = {0};

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (ret=%d)\n", ret);
        return ret;
    }

    info.type = DMS_MEM_TEMP;
    ret = lpm_get_sharemem_entry_info(&info);
    if (ret != 0) {
        dms_err("Get temperature entry information failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    switch (core_id) {
        case LPM_CLUSTER_ID:
            ret = dms_lpm_sharemem_op_u16(REG_OP_TYPE_RD, SHAREMEM_BASE_ADDR + info.offset, CLUSTER_TEMP_OFFSET,
                info.len, &val_u16);
            *temperature = val_u16;
            *temperature &= 0xFFFF;
            break;
        case LPM_PERI_ID:
            ret = dms_lpm_sharemem_op_u16(REG_OP_TYPE_RD, SHAREMEM_BASE_ADDR + info.offset, IO_TEMP_OFFSET,
                info.len, &val_u16);
            *temperature = val_u16;
            *temperature &= 0xFFFF;
            break;
        case LPM_AICORE0_ID:
            ret = dms_lpm_sharemem_op_u16(REG_OP_TYPE_RD, SHAREMEM_BASE_ADDR + info.offset, AIC_TEMP_OFFSET,
                info.len, &val_u16);
            *temperature = val_u16;
            *temperature &= 0xFFFF;
            break;
        case LPM_SOC_ID:
            ret = dms_lpm_sharemem_op(REG_OP_TYPE_RD, SHAREMEM_BASE_ADDR + info.offset, SOC_TEMP_OFFSET,
                info.len, temperature);
            *temperature &= 0xFFFF;
            break;
        case LPM_HBM_ID:
            ret = dms_lpm_sharemem_op(REG_OP_TYPE_RD, SHAREMEM_BASE_ADDR + info.offset, HBM_TEMP_OFFSET,
                info.len, temperature);
            *temperature &= 0xFFFF;
            break;
        default:
            dms_err("Invalid core id. (dev_id=%u; core_id=%u)\n", dev_id, core_id);
            return -EOPNOTSUPP;
    }

    if (ret != 0) {
        dms_err("Get temperature failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return 0;
}

int lpm_get_power_from_sharemem(u32 dev_id, u32 core_id, u32 *power)
{
    int ret;
    struct dms_info_entry info = {0};

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (ret=%d)\n", ret);
        return ret;
    }

    info.type = DMS_MEM_POWER;
    ret = lpm_get_sharemem_entry_info(&info);
    if (ret != 0) {
        dms_err("Get power entry information failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = dms_lpm_sharemem_op(REG_OP_TYPE_RD, SHAREMEM_BASE_ADDR + info.offset, POWER_OFFSET, info.len, power);
    if (ret != 0) {
        dms_err("Get power failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return 0;
}

int lpm_get_voltage_from_sharemem(u32 dev_id, u32 core_id, u32 *voltage)
{
    int ret;
    struct dms_info_entry info = {0};

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (ret=%d)\n", ret);
        return ret;
    }

    info.type = DMS_MEM_VOLT;
    ret = lpm_get_sharemem_entry_info(&info);
    if (ret != 0) {
        dms_err("Get voltage entry information failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = dms_lpm_sharemem_op(REG_OP_TYPE_RD, SHAREMEM_BASE_ADDR + info.offset, VOLTAGE_OFFSET, info.len, voltage);
    if (ret != 0) {
        dms_err("Get voltage failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return 0;
}

int lpm_get_tsensor_from_sharemem(u32 dev_id, u32 core_id, u32 *value)
{
    int ret;
    int type;

    switch (core_id) {
        case DMS_CLUSTER_TEMP_ID:
            type = LPM_CLUSTER_ID;
            break;
        case DMS_PERI_TEMP_ID:
            type = LPM_PERI_ID;
            break;
        case DMS_AICORE0_TEMP_ID:
            type = LPM_AICORE0_ID;
            break;
        case DMS_AICORE_BASE_FREQ_ID:
            type = LPM_AICORE1_ID;
            ret = lpm_get_frequency_from_sharemem(dev_id, type, value);
            if (ret != 0) {
                dms_err("Get soc sensor info failed. (dev_id=%u; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
                return ret;
            }
            return 0;
        case DMS_SOC_TEMP_ID:
            type = LPM_SOC_ID;
            break;
#ifdef CFG_SUPPORT_NDIE
        case DMS_N_DIE_TEMP_ID:
            type = LPM_SOC_ID;
            break;
#endif
        case DMS_HBM_TEMP_ID:
            type = LPM_HBM_ID;
            break;
        default:
            dms_err("Invaild core id. (dev_id=%u; core_id=%u)\n", dev_id, core_id);
            return DRV_ERROR_NOT_SUPPORT;
    }

    ret = lpm_get_temperature_from_sharemem(dev_id, type, value);
    if (ret != 0) {
        dms_err("Get soc sensor info failed. (dev_id=%u; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
        return ret;
    }

    return 0;
}

int lpm_get_frequency_from_sharemem(u32 dev_id, u32 core_id, u32 *frequency)
{
    int ret;
    struct dms_info_entry info = {0};

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (ret=%d)\n", ret);
        return ret;
    }

    info.type = DMS_MEM_FREQ;
    ret = lpm_get_sharemem_entry_info(&info);
    if (ret != 0) {
        dms_err("Get frequency entry information failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    switch (core_id) {
        /* aicpu freq */
        case LPM_CLUSTER_ID:
            ret = dms_lpm_sharemem_op(REG_OP_TYPE_RD, SHAREMEM_BASE_ADDR + info.offset, AICPU_CRUFREQ_OFFSET,
                info.len, frequency);
            break;
        /* aicore crufreq */
        case LPM_AICORE0_ID:
            ret = dms_lpm_sharemem_op(REG_OP_TYPE_RD, SHAREMEM_BASE_ADDR + info.offset, AICORE_CRUFREQ_OFFSET,
                info.len, frequency);
            break;
        /* aicore ratedfreq */
        case LPM_AICORE1_ID:
            ret = dms_lpm_sharemem_op(REG_OP_TYPE_RD, SHAREMEM_BASE_ADDR + info.offset, AICORE_RATED_FREQ_OFFSET,
            info.len, frequency);
            break;
        default:
            dms_err("Invalid core id. (dev_id=%u; core_id=%u)\n", dev_id, core_id);
            return -EOPNOTSUPP;
    }

    if (ret != 0) {
        dms_err("Get frequency failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return 0;
}

int lpm_get_max_frequency_from_sharemem(u32 dev_id, u32 core_id, u32 *frequency)
{
    int ret;
    struct dms_info_entry info = {0};

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (ret=%d)\n", ret);
        return ret;
    }

    info.type = DMS_MEM_FREQ;
    ret = lpm_get_sharemem_entry_info(&info);
    if (ret != 0) {
        dms_err("Get frequency entry information failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = dms_lpm_sharemem_op(REG_OP_TYPE_RD, SHAREMEM_BASE_ADDR + info.offset, AICPU_MAX_FREQ_OFFSET,
        info.len, frequency);
    if (ret != 0) {
        dms_err("Get frequency offset failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return 0;
}
