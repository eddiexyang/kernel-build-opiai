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

#include <linux/securec.h>
#include <linux/delay.h>
#include <linux/mutex.h>

#include "drv_profile.h"
#include "devdrv_pm.h"
#include "ascend_platform.h"
#include "ascend_kernel_hal.h"
#include "devdrv_user_common.h"
#include "devdrv_manager_comm.h"
#include "devdrv_manager_hccs.h"

struct hccs_flow_info g_hcc_flow_info[MAX_DEVICE_COUNT] = {
    { HCCS_DEV0_PHY_BASE_ADDR, {NULL}, 0, {0}, 0, 0 },
    { HCCS_DEV1_PHY_BASE_ADDR, {NULL}, 0, {0}, 0, 0 },
    { HCCS_DEV2_PHY_BASE_ADDR, {NULL}, 0, {0}, 0, 0 },
    { HCCS_DEV3_PHY_BASE_ADDR, {NULL}, 0, {0}, 0, 0 }
};

unsigned int hccs_reg_read(unsigned long vir_addr)
{
    void __iomem *p_dst = NULL;
    unsigned int val = 0;

    p_dst = (void __iomem *)(uintptr_t)vir_addr;
    if (p_dst != NULL) {
        val = readl((const volatile void __iomem *)p_dst);
    }

    devdrv_drv_debug("rd,addr:0x%016lX:0x%08X\n", vir_addr, val);

    return val;
}

void hccs_reg_write(unsigned long vir_addr, unsigned int val)
{
    void __iomem *p_dst = NULL;

    p_dst = (void __iomem *)(uintptr_t)vir_addr;
    if (p_dst != NULL) {
        writel(val, (volatile void __iomem *)p_dst);
    }
}

STATIC void devdrv_hccs_prof_memory_unmap(unsigned int dev_id)
{
    unsigned int i;
    struct hccs_flow_info *device_hccs_info = NULL;

    device_hccs_info = &g_hcc_flow_info[dev_id];
    for (i = 0; i < HCCS_NUM; i++) {
        if (device_hccs_info->hccs_base_addr[i] != NULL) {
            iounmap(device_hccs_info->hccs_base_addr[i]);
            device_hccs_info->hccs_base_addr[i] = NULL;
        }
    }

    device_hccs_info->reg_map_flag = false;
}

STATIC int devdrv_hccs_prof_memory_map(unsigned int dev_id)
{
    int ret;
    unsigned int i;
    struct hccs_flow_info *device_hccs_info = NULL;
    devdrv_hardware_info_t hardware_info = {0};

    ret = hal_kernel_get_hardware_info(dev_id, &hardware_info);
    if (ret != 0) {
        devdrv_drv_err("Failed to invoke hal_kernel_get_hardware_info. (devid=%u)\n", dev_id);
        return ret;
    }

    device_hccs_info = &g_hcc_flow_info[dev_id];
    for (i = 0; i < HCCS_NUM; i++) {
        if (device_hccs_info->hccs_phy_addr == (unsigned long)(uintptr_t)NULL) {
            break;
        }

        device_hccs_info->hccs_base_addr[i] = ioremap(
            device_hccs_info->hccs_phy_addr + hardware_info.phy_addr_offset + i * HCCS_PHY_ADDR_INTERVAL, HCCS_REG_LEN);
        if (device_hccs_info->hccs_base_addr[i] == NULL) {
            devdrv_hccs_prof_memory_unmap(dev_id);
            devdrv_drv_err("Remap memory for hccs failed. (dev_id=%u; hccs_id=%u)\n", dev_id, i);
            return -ENOMEM;
        }
    }

    device_hccs_info->reg_map_flag = true;
    return 0;
}

STATIC void devdrv_hccs_prof_get_sample_data(unsigned int dev_id)
{
    unsigned int i;
    struct hccs_flow_info *device_hccs_info = NULL;

    device_hccs_info = &g_hcc_flow_info[dev_id];
    for (i = 0; i < HCCS_NUM; i++) {
        if (HLLC_HYDRA_RX_CH0_FLIT_CNT_OFFSET != HCCS_OFFSET_INVALID) {
            device_hccs_info->send_amount += HCCS_REG_RD(device_hccs_info->hccs_base_addr[i],
                HLLC_HYDRA_RX_CH0_FLIT_CNT_OFFSET);
        }

        if (HLLC_HYDRA_RX_CH1_FLIT_CNT_OFFSET != HCCS_OFFSET_INVALID) {
            device_hccs_info->send_amount += HCCS_REG_RD(device_hccs_info->hccs_base_addr[i],
                HLLC_HYDRA_RX_CH1_FLIT_CNT_OFFSET);
        }

        if (HLLC_HYDRA_RX_CH2_FLIT_CNT_OFFSET != HCCS_OFFSET_INVALID) {
            device_hccs_info->send_amount += HCCS_REG_RD(device_hccs_info->hccs_base_addr[i],
                HLLC_HYDRA_RX_CH2_FLIT_CNT_OFFSET);
        }

        if (PHY_TX_CH0_FLIT_CNT_OFFSET != HCCS_OFFSET_INVALID) {
            device_hccs_info->receive_amount += HCCS_REG_RD(device_hccs_info->hccs_base_addr[i],
                PHY_TX_CH0_FLIT_CNT_OFFSET);
        }

        if (PHY_TX_CH1_FLIT_CNT_OFFSET != HCCS_OFFSET_INVALID) {
            device_hccs_info->receive_amount += HCCS_REG_RD(device_hccs_info->hccs_base_addr[i],
                PHY_TX_CH1_FLIT_CNT_OFFSET);
        }

        if (PHY_TX_CH2_FLIT_CNT_OFFSET != HCCS_OFFSET_INVALID) {
            device_hccs_info->receive_amount += HCCS_REG_RD(device_hccs_info->hccs_base_addr[i],
                PHY_TX_CH2_FLIT_CNT_OFFSET);
        }

        devdrv_drv_debug("Hccs sample data. (dev_id=%u; hccs_id=%u; send_amount=0x%llx; receive_amount=0x%llx)\n",
            dev_id, i, device_hccs_info->send_amount, device_hccs_info->receive_amount);
    }

    device_hccs_info->send_amount = device_hccs_info->send_amount * HCCS_PACKET_LEN;
    device_hccs_info->receive_amount = device_hccs_info->receive_amount * HCCS_PACKET_LEN;

    devdrv_drv_debug("Total Sample data. (dev_id=%u; send_amount=0x%llx; receive_amount=0x%llx)\n",
            dev_id, device_hccs_info->send_amount, device_hccs_info->receive_amount);
    ktime_get_raw_ts64(&device_hccs_info->current_time);
}

int devdrv_hccs_prof_sample(struct prof_peri_para para)
{
    int ret;
    int wr_position = 0;
    unsigned int buffer_len_tmp = para.buff_len;
    unsigned int wr_position_add = 0;
    unsigned int device_id = para.device_id;
    int flag = para.sample_flag;
    void *buffer_addr = para.buff;

    struct hccs_flow_info *device_hccs_info = NULL;
    CHECK_RET((buffer_addr != NULL), (-EINVAL));
    CHECK_DO_SOMETHING((device_id < MAX_DEVICE_COUNT),
                        devdrv_drv_err("Device id is invalid. (dev_id=%u)\n", device_id);
                        return -EINVAL);

    device_hccs_info = &g_hcc_flow_info[device_id];
    device_hccs_info->send_amount = 0;
    device_hccs_info->receive_amount = 0;

    if (device_hccs_info->reg_map_flag == false) {
        ret = devdrv_hccs_prof_memory_map(device_id);
        if (ret != 0) {
            devdrv_drv_err("Map memory for register failed. (dev_id=%u; ret=%d)\n", device_id, ret);
            return ret;
        }
    }

    devdrv_hccs_prof_get_sample_data(device_id);

    /* send header and data */
    if ((((unsigned int)flag) & SAMPLE_MASK) == SAMPLE_WITH_HEADER) {
        wr_position = snprintf_s(buffer_addr, buffer_len_tmp, buffer_len_tmp - 1,
                                 "time_stamp(s):time_stamp(ns) send_amount receive_amount\n");
        if (wr_position < 0) {
            devdrv_drv_err("Hccs flow write buff fail. (dev_id=%u; ret=%d; buffer_len_tmp=%u)\n",
                device_id, wr_position, buffer_len_tmp);
            return wr_position;
        }
        buffer_len_tmp -= wr_position;
        wr_position_add += wr_position;
    }

    wr_position = snprintf_s(buffer_addr + wr_position_add, buffer_len_tmp, buffer_len_tmp - 1, "%ld:%09ld %llu %llu\n",
                             (unsigned long)device_hccs_info->current_time.tv_sec,
                             device_hccs_info->current_time.tv_nsec, device_hccs_info->send_amount,
                             device_hccs_info->receive_amount);
    if (wr_position < 0) {
        devdrv_drv_err("Hccs flow write buff fail. (dev_id=%u; ret=%d; buffer_len_tmp=%u)\n",
            device_id, wr_position, buffer_len_tmp);
        return wr_position;
    }

    buffer_len_tmp -= wr_position;
    wr_position_add += wr_position;

    return wr_position_add;
}

EXPORT_SYMBOL(devdrv_hccs_prof_sample);

int devdrv_hccs_prof_stop(struct prof_peri_para para)
{
    unsigned int device_id = para.device_id;
    struct hccs_flow_info *device_hccs_info = NULL;

    CHECK_DO_SOMETHING((device_id < MAX_DEVICE_COUNT),
                        devdrv_drv_err("Device id is invalid. (dev_id=%u)\n", device_id);
                        return -EINVAL);

    device_hccs_info = &g_hcc_flow_info[device_id];
    devdrv_hccs_prof_memory_unmap(device_id);
    return 0;
}
EXPORT_SYMBOL(devdrv_hccs_prof_stop);
