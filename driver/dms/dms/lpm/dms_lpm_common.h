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
#ifndef __DMS_LPM_COMMON_H__
#define __DMS_LPM_COMMON_H__

#define REG_OP_TYPE_WR 0
#define REG_OP_TYPE_RD 1

#define LPM_THERMAL_THRESHOLD_NUM 2
#define LPM_NTC_NUM 4

#define DMS_CRC_POLYNOMIAL  0x1021
#define DMS_NULL_USHORT     0xFFFF
#define DMS_BIT15           0x8000
#define DMS_BITS_PER_BYTE   8

enum dms_tsensor_id {
    DMS_CLUSTER_TEMP_ID = 0,
    DMS_PERI_TEMP_ID = 1,
    DMS_AICORE0_TEMP_ID,
    DMS_AICORE1_TEMP_ID,
    DMS_AICORE_LIMIT_ID,
    DMS_AICORE_TOTAL_PER_ID,
    DMS_AICORE_ELIM_PER_ID,
    DMS_AICORE_BASE_FREQ_ID,
    DMS_NPU_DDR_FREQ_ID,
    DMS_THERMAL_THRESHOLD_ID,
    DMS_NTC_TEMP_ID,
    DMS_SOC_TEMP_ID,
    DMS_FP_TEMP_ID,
    DMS_N_DIE_TEMP_ID,
    DMS_HBM_TEMP_ID,
    DMS_INVALID_TSENSOR_ID,
};

struct dms_ipc_lpm_msg {
    u8 cmd_type1;
    u8 cmd_type0;
    u8 target_id;
    u8 source_id;

    u8 cmd_para0;
    u8 cmd_para1;
    u8 cmd_para2;
    u8 cmd_para3;

    u8 cmd_data0;
    u8 cmd_data1;
    u8 cmd_data2;
    u8 cmd_data3;

    u32 data1;
    u32 data2;
    u32 data3;
    u32 data4;
    u32 data5;
};

int dms_lpm_id_check(unsigned int dev_id, unsigned int core_id);
int dms_lpm_reg_op(unsigned char op_type, unsigned long base_phy_reg, unsigned long reg_offset,
    unsigned long map_size, unsigned int *val);
int dms_lpm_sharemem_op(unsigned char op_type, unsigned long base_phy_addr, unsigned long offset,
    unsigned long map_size, unsigned int *val);
int dms_lpm_sharemem_op_u16(unsigned char op_type, unsigned long base_phy_addr, unsigned long offset,
    unsigned long map_size, unsigned short *val);
unsigned short dms_crc16(unsigned char *data, unsigned short len);
#endif