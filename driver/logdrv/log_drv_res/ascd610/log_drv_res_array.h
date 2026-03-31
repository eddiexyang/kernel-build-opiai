/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */
#ifndef LOG_DRV_RES_ARRAY_H
#define LOG_DRV_RES_ARRAY_H

#include "log_drv_res.h"
#include "log_drv_dev.h"
#include "tsdrv_common.h"

static struct log_channel_desc log_channel_sets_ascend610[] = {
    {   .channel_type = LOG_CHANNEL_TYPE_TS,
        .channel_conn = LOG_CHANNEL_CONN_SQCQ,
        .channel_ids = LOG_CHANNEL_TYPE_TS0,
        .channel_ids_num = TS_MDC_CHANNEL_NUM,
        .log_level_support = 1,
        .buf_phy_addr = TSCPU0_RESERVED_MEM_ADDR_RUN,
        .buf_size = LOG_TS_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN,
        .reserve = LOG_DC_TS_ID
    },
    {   .channel_type = LOG_CHANNEL_TYPE_TS,
        .channel_conn = LOG_CHANNEL_CONN_SQCQ,
        .channel_ids = (int32_t)LOG_CHANNEL_TYPE_TS1,
        .channel_ids_num = 0,
        .log_level_support = 1,
        .buf_phy_addr = TSCPU1_RESERVED_MEM_ADDR_RUN,
        .buf_size = LOG_TS_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN,
        .reserve = LOG_MDC_TS_ID
    },
    {   .channel_type = LOG_CHANNEL_TYPE_BIOS,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_BIOS,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = BIOS_RESERVED_MEM_ADDR,
        .buf_size = LOG_BIOS_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_LPM3,
        .channel_conn = LOG_CHANNEL_CONN_IPC,
        .channel_ids = LOG_CHANNEL_TYPE_LPM3,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = LP_RESERVED_MEM_ADDR,
        .buf_size = LOG_LP_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_ISP,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_ISP0,
        .channel_ids_num = ISP_CHANNEL_NUM,
        .log_level_support = 1,
        .buf_phy_addr = ISP0_RESERVED_MEM_ADDR,
        .buf_size = LOG_ISP_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_ISP,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_ISP1,
        .channel_ids_num = 0,
        .log_level_support = 1,
        .buf_phy_addr = ISP1_RESERVED_MEM_ADDR,
        .buf_size = LOG_ISP_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_ISP,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_ISP2,
        .channel_ids_num = 0,
        .log_level_support = 1,
        .buf_phy_addr = ISP2_RESERVED_MEM_ADDR,
        .buf_size = LOG_ISP_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_ISP,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_ISP3,
        .channel_ids_num = 0,
        .log_level_support = 1,
        .buf_phy_addr = ISP3_RESERVED_MEM_ADDR,
        .buf_size = LOG_ISP_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_SIS,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_SIS,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = SIS_RESERVED_MEM_ADDR,
        .buf_size = LOG_SIS_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_HSM,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_HSM,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = HSM_RESERVED_MEM_ADDR,
        .buf_size = LOG_HSM_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_BIOS_ATF,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_BIOS_ATF,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = BIOS_ATF_RESERVED_MEM_ADDR,
        .buf_size = LOG_BIOS_ATF_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
};

static struct log_channel_desc log_channel_sets_bs9sx1a[] = {
    {   .channel_type = LOG_CHANNEL_TYPE_TS,
        .channel_conn = LOG_CHANNEL_CONN_SQCQ,
        .channel_ids = LOG_CHANNEL_TYPE_TS0,
        .channel_ids_num = TS_MDC_CHANNEL_NUM,
        .log_level_support = 1,
        .buf_phy_addr = TSCPU0_RESERVED_MEM_ADDR_RUN,
        .buf_size = LOG_TS_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN,
        .reserve = LOG_DC_TS_ID
    },
    {   .channel_type = LOG_CHANNEL_TYPE_TS,
        .channel_conn = LOG_CHANNEL_CONN_SQCQ,
        .channel_ids = (int32_t)LOG_CHANNEL_TYPE_TS1,
        .channel_ids_num = 0,
        .log_level_support = 1,
        .buf_phy_addr = TSCPU1_RESERVED_MEM_ADDR_RUN,
        .buf_size = LOG_TS_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN,
        .reserve = LOG_MDC_TS_ID
    },
    {   .channel_type = LOG_CHANNEL_TYPE_BIOS,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_BIOS,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = BIOS_RESERVED_MEM_ADDR,
        .buf_size = LOG_BIOS_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_LPM3,
        .channel_conn = LOG_CHANNEL_CONN_IPC,
        .channel_ids = LOG_CHANNEL_TYPE_LPM3,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = LP_RESERVED_MEM_ADDR,
        .buf_size = LOG_LP_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_ISP,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_ISP0,
        .channel_ids_num = ISP_CHANNEL_NUM,
        .log_level_support = 1,
        .buf_phy_addr = ISP0_BS9SX1A_RESERVED_MEM_ADDR,
        .buf_size = LOG_ISP_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_ISP,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_ISP1,
        .channel_ids_num = 0,
        .log_level_support = 1,
        .buf_phy_addr = ISP1_BS9SX1A_RESERVED_MEM_ADDR,
        .buf_size = LOG_ISP_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_SIS,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_SIS,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = SIS_RESERVED_MEM_ADDR,
        .buf_size = LOG_SIS_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_HSM,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_HSM,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = HSM_RESERVED_MEM_ADDR,
        .buf_size = LOG_HSM_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_BIOS_ATF,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_BIOS_ATF,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = BIOS_ATF_RESERVED_MEM_ADDR,
        .buf_size = LOG_BIOS_ATF_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
    {   .channel_type = LOG_CHANNEL_TYPE_SIS_BIST,
        .channel_conn = LOG_CHANNEL_CONN_SHAREMEM,
        .channel_ids = LOG_CHANNEL_TYPE_SIS_BIST,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = SIS_BIST_RESERVED_MEM_ADDR,
        .buf_size = LOG_SIS_BIST_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_INFO,
        .log_type = LOG_TYPE_RUN
    },
};

static int g_log_chip_type = -1;
static int log_get_chip_type(void)
{
    if (g_log_chip_type != -1) {
        return g_log_chip_type;
    }
    g_log_chip_type = tsdrv_get_chip_type();
    if (g_log_chip_type == -1) {
        g_log_chip_type = CHIP_TYPE_MDC_BS9SX1A;
    }
    return g_log_chip_type;
}

static inline struct log_channel_desc *log_get_channel_desc_array(void)
{
    if (log_get_chip_type() == CHIP_TYPE_MDC_ASCEND610) {
        return log_channel_sets_ascend610;
    } else {
        return log_channel_sets_bs9sx1a;
    }
}

static inline s32 log_get_channel_num_array(void)
{
    if (log_get_chip_type() == CHIP_TYPE_MDC_ASCEND610) {
        return sizeof(log_channel_sets_ascend610) / sizeof(struct log_channel_desc);
    } else {
        return sizeof(log_channel_sets_bs9sx1a) / sizeof(struct log_channel_desc);
    }
}

#endif

