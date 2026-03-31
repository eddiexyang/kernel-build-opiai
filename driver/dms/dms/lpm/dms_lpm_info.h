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
#ifndef __DMS_LPM_INFO_H__
#define __DMS_LPM_INFO_H__

#include "drv_type.h"
#include "devdrv_manager.h"

/* sharemem head magic num */
#define SHAREMEM_MAGIC_NUM  0x4c50534d

/* temperature offset */
#define SOC_TEMP_OFFSET  0
#define HBM_TEMP_OFFSET  4

/*
* temperature offset used in 310B, e.g.
* IO_TEMP_OFFSET: 8 (Saved in 2 bytes * 4 offset)
* MATA_TEMP_OFFSET: 12 (Saved in 2 bytes * 6 offset)
*/
#define IO_TEMP_OFFSET 8
#define CLUSTER_TEMP_OFFSET 10
#define AIC_TEMP_OFFSET 14

/* power offset */
#define POWER_OFFSET 0

/* voltage offset */
#define VOLTAGE_OFFSET 0

/* frenquency offset */
#define AICPU_CRUFREQ_OFFSET 0
#define AICPU_MAX_FREQ_OFFSET 4
#define AICORE_CRUFREQ_OFFSET 8
#define AICORE_RATED_FREQ_OFFSET 12

/* ccpu freq */
#define CHIP_OFFSET 0x200000000000
#define CCPU_FREF 100
#define PLL_CTRL_BASE 0x892A0000
#define PLL0_FCTRL_OFFSET 0x10
#define PLL0_FCTRL_FRAC_OFFSET 0x14
#define REF_DIV_MASK 0x3F
#define FB_DIV_OFFSET (6)
#define FB_DIV_MASK (0xFFF)
#define POST_DIV1_OFFSET (18)
#define POST_DIV1_MASK (0x7)
#define POST_DIV2_OFFSET (21)
#define POST_DIV2_MASK (0x7)
#define FRAC_MSAK (0xffffff)
#define TOTAL_QUAN (0x1000000)

/* device power */
#define MINI_BOARD_POWER_REG_BASE 0x1100c3000
#define MINI_BOARD_POWER_REG_OFFSET 0x434
#define MAP_PAGE_SIZE 4096
#define POWER_LEVEL 4
#define POWER_LEVEL_0 20
#define POWER_LEVEL_1 40
#define POWER_LEVEL_2 80
#define POWER_LEVEL_3 128
#define POWER_LEVEL_MASK 0xff

#define OFFSET_EIGHT 8
#define THERMISTOR_NUM 4
#define CHANNAL_OFFSET 3

#define DMS_VALUE_SIZE 4

#define DMS_VRD_TEMP 0xFE
#define DMS_MOVE_OFFSET 32
#define DMS_PRECISION_CONVER 100
#define UINT16_MAX 0xffffU /* 65535U */

#define SHAREMEM_MAX_SIZE 0x1F000

enum dms_info_type {
    DMS_MEM_TEMP,
    DMS_MEM_FREQ,
    DMS_MEM_POWER,
    DMS_MEM_VOLT,
    DMS_MEM_MAX = 32,
};

struct dms_info_entry {
    u32 type;
    u32 offset;
    u32 len;
};

/*
 * head addr range: SHAREMEM_BASE_ADDR ~ (SHAREMEM_BASE_ADDR + sizeof(struct dms_sharemem_header))
 * temp_data addr range:  (SHAREMEM_BASE_ADDR + entry[DMS_MEM_TEMP].offset) ~
 *                        (SHAREMEM_BASE_ADDR + entry[DMS_MEM_TEMP].offset + entry[DMS_MEM_TEMP].len)
 * freq_data addr range:  (SHAREMEM_BASE_ADDR + entry[DMS_MEM_FREQ].offset) ~
 *                        (SHAREMEM_BASE_ADDR + entry[DMS_MEM_FREQ].offset + entry[DMS_MEM_FREQ].len)
 * power_data addr range: (SHAREMEM_BASE_ADDR + entry[DMS_MEM_POWER].offset) ~
 *                        (SHAREMEM_BASE_ADDR + entry[DMS_MEM_POWER].offset + entry[DMS_MEM_POWER].len)
 * volt_data addr range:  (SHAREMEM_BASE_ADDR + entry[DMS_MEM_VOLT].offset) ~
 *                        (SHAREMEM_BASE_ADDR + entry[DMS_MEM_VOLT].offset + entry[DMS_MEM_VOLT].len)
 */
struct dms_sharemem_header {
    u32 magic_num;
    u32 version;
    u32 entry_num;
    struct dms_info_entry entry[DMS_MEM_MAX];
    u32 crc16;
};

struct dms_share_mem {
    char cluster_temp;
    char peri_temp;
    char aicore0_temp;
    char aicore1_temp;
    u8 aicore_limit;
    u8 aicore_total_period;
    u8 aicore_elim_period;
    u8 aicore_base_freq_l;
    u8 aicore_base_freq_h; /* default little-edian */
    char soc_max_temp;
    u8 ddr_capacity_l;
    u8 ddr_capacity_h;
    u8 ddr_freq_l;
    u8 ddr_freq_h;
    char limited_temp;
    char reset_temp;
};

int lpm_get_temperature(u32 dev_id, u32 core_id, u32 *temperature);
int lpm_get_power(u32 dev_id, u32 core_id, u32 *power);
int lpm_get_voltage(u32 dev_id, u32 core_id, u32 *voltage);
int lpm_get_frequency(u32 dev_id, u32 core_id, u32 *frequency);
int lpm_get_tsensor(u32 dev_id, u32 core_id, u32 *value);
int lpm_get_max_frequency(u32 dev_id, u32 core_id, u32 *frequency);

int lpm_get_temperature_from_ipc(u32 dev_id, u32 core_id, u32 *temperature);
int lpm_get_temperature_from_devinfo(u32 dev_id, u32 core_id, u32 *temperature);
int lpm_get_temperature_from_sharemem(u32 dev_id, u32 core_id, u32 *temperature);

int lpm_get_power_from_ipc(u32 dev_id, u32 core_id, u32 *power);
int lpm_get_power_from_register(u32 dev_id, u32 core_id, u32 *power);
int lpm_get_power_from_sharemem(u32 dev_id, u32 core_id, u32 *power);

int lpm_get_voltage_from_ipc(u32 dev_id, u32 core_id, u32 *voltage);
int lpm_get_voltage_from_pmu(u32 dev_id, u32 core_id, u32 *voltage);
int lpm_get_voltage_from_sharemem(u32 dev_id, u32 core_id, u32 *voltage);

int lpm_get_ccpu_freq(u32 dev_id, u32 *ccpu_freq);
int lpm_get_aicore_rated_frequency(u32 dev_id, u32 *freq);
int lpm_get_frequency_from_ipc(u32 dev_id, u32 core_id, u32 *frequency);
int lpm_get_frequency_info(u32 dev_id, u32 core_id, u32 *frequency);
int lpm_get_frequency_from_devinfo(u32 dev_id, u32 core_id, u32 *frequency);
int lpm_get_frequency_from_sharemem(u32 dev_id, u32 core_id, u32 *frequency);
int lpm_get_max_frequency_from_sharemem(u32 dev_id, u32 core_id, u32 *frequency);

int lpm_get_temp_from_sharemem(u32 core_id, u32 *temp);
int lpm_get_ddr_freq_from_ipc(u32 dev_id, u32 *freq);
int lpm_get_thermal_threshold_from_ipc(u32 dev_id, u32 *temp);
int lpm_get_soc_temp_from_ipc(u32 dev_id, u32 *temp);
int lpm_get_temp_ntc(u32 *temp);
int lpm_get_tsensor_from_ipc(u32 dev_id, u32 core_id, u32 *value);
int lpm_get_tsensor_info(u32 dev_id, u32 core_id, u32 *value);
int lpm_get_tsensor_from_sharemem(u32 dev_id, u32 core_id, u32 *value);

int lpm_get_temperature_stub(u32 dev_id, u32 core_id, u32 *temperature);
int lpm_get_power_stub(u32 dev_id, u32 core_id, u32 *power);
int lpm_get_voltage_stub(u32 dev_id, u32 core_id, u32 *voltage);
int lpm_get_frequency_stub(u32 dev_id, u32 core_id, u32 *frequency);
int lpm_get_tsensor_stub(u32 dev_id, u32 core_id, u32 *value);
int lpm_get_max_frequency_stub(u32 dev_id, u32 core_id, u32 *frequency);

int lpm_get_lp_status_from_ipc(u32 dev_id, u32 core_id, u32 *lp_status);
int lpm_get_vol_cur_from_ipc(u32 dev_id, u32 core_id, u32 *vol_cur);
int lpm_get_acg_from_ipc(u32 dev_id, u32 core_id, u32 *acg_data);
int lpm_get_ddr_temp_from_ipc(u32 dev_id, u32 core_id, u32 *ddr_temp);
int lpm_get_temp_thold_from_ipc(u32 dev_id, u32 core_id, u32 *temp_thold);

#ifdef CFG_LPM_INFO_FROM_IPC
#define lpm_get_temperature lpm_get_temperature_from_ipc
#define lpm_get_power lpm_get_power_from_ipc
#define lpm_get_voltage lpm_get_voltage_from_ipc
#define lpm_get_tsensor lpm_get_tsensor_from_ipc
#define lpm_get_status lpm_get_lp_status_from_ipc
#define lpm_get_vol_cur lpm_get_vol_cur_from_ipc
#define lpm_get_acg lpm_get_acg_from_ipc
#define lpm_get_ddr_temp lpm_get_ddr_temp_from_ipc
#define lpm_get_temp_thold lpm_get_temp_thold_from_ipc
#ifdef CFG_REATURE_GET_INFO_FORM_IPC
int lpm_set_temp_thold_from_ipc(u32 dev_id, u32 core_id, u32 *temp_state);
#define lpm_set_temp_thold lpm_set_temp_thold_from_ipc
#endif
#endif
#ifdef CFG_LPM_INFO_FROM_SHAREMEM
#define lpm_get_temperature lpm_get_temperature_from_sharemem
#define lpm_get_power lpm_get_power_from_sharemem
#define lpm_get_voltage lpm_get_voltage_from_sharemem
#define lpm_get_tsensor lpm_get_tsensor_from_sharemem
#define lpm_get_max_frequency lpm_get_max_frequency_from_sharemem
#endif
#ifdef CFG_LPM_INFO_MAX_FREQ_STUB
#define lpm_get_max_frequency lpm_get_max_frequency_stub
#endif
#ifdef CFG_LPM_INFO_FROM_DEVINFO
#define lpm_get_temperature lpm_get_temperature_from_devinfo
#endif
#ifdef CFG_LPM_INFO_FROM_REGISTER
#define lpm_get_power lpm_get_power_from_register
#endif
#ifdef CFG_LPM_INFO_FROM_PMU
#define lpm_get_voltage lpm_get_voltage_from_pmu
#endif
#ifdef CFG_GET_TSENSOR_INFO
#define lpm_get_tsensor lpm_get_tsensor_info
#endif

#ifdef CFG_FREQ_FROM_DEVINFO
#define lpm_get_frequency lpm_get_frequency_from_devinfo
#endif
#ifdef CFG_FREQ_FROM_IPC
#define lpm_get_frequency lpm_get_frequency_from_ipc
#endif
#ifdef CFG_GET_FREQ_INFO
#define lpm_get_frequency lpm_get_frequency_info
#endif

#ifdef CFG_LPM_INFO_STUB
#define lpm_get_temperature lpm_get_temperature_stub
#define lpm_get_power lpm_get_power_stub
#define lpm_get_voltage lpm_get_voltage_stub
#define lpm_get_frequency lpm_get_frequency_stub
#define lpm_get_tsensor lpm_get_tsensor_stub
#endif

#endif
