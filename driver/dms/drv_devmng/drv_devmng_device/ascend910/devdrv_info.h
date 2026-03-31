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

#ifndef __DEVDRV_INFO_H
#define __DEVDRV_INFO_H

#define LOCK_INTERRUPT_RETRY_TIME 3

#define DMANAGE_ENTIRE_SOC 0xFF
#define THERMISTOR_NUM 4
#define CHANNAL_OFFSET 3
#define FLASH_MAX_ID 1000
#define THERMAL_RESULT_SIZE 2
#define DDR_FREQ_RESULT_SIZE 1
#define SOC_TMP_RESULT_SIZE 1

#define FATAL_ERROR_MASK 0x4
#define IMPORTANT_ERROR_MASK 0x3
#define NORMAL_ERROR_MASK 0x2
#define TOTAL_ERROR_MASK 0x7
#define NONE_ERROR 0x0
#define ERROR_BIT_POSITION 25
#define ERROR_MAX_VALUE 0x4
#define TEMP_FROM_IMU_NUM 3

#define DEVDRV_KO_INSERT_DONE 1
#define DEVDRV_KO_INSERT_WAIT 0

#define CHIP_INFO_MAP_SIZE 4096

/* device power */
#define SLEEP_POWER_DELAY_MS 2U

/* reset i2c controller */
#define SM_BUS_CTL_BASE_ADDR 0x130070000
#define RESET_I2C_OFFSET 0xD00
#define RESET_REG_VALUE_OFFSET 0x5D00
#define UNRESET_I2C_OFFSET 0xD04
#define I2C_RESET_MAP_SIZE (4096 * 6)

/* gpio retry read */
#define MAX_RETRY 0xffff
#define DEVDRV_GPIO_NAME "gpio-read"

/* xloader boot info */
#define SIZE_OF_64K 0x10000
#define SYSCTRL_REG_BASE 0x1100C0000
#define SC_SOFT_POR_RSV3 0xFF2C
#define SC_BAK_DATA14 0x3448
#define ERRNO_NONSUPPORT_ITEM 0x4

#define DEV_UPDATE_XLOADER_AREA0 0x0
#define DEV_UPDATE_XLOADER_AREA1 0x1
#define DEV_UPDATE_XLOADER_BOTH 0x2

#define DEV_GET_CURR_BOOT_AREA 0
#define DEV_CLEAR_BOOT_COUNT 1
/* soc die id */
#define SOC_DIEID_MAP_SIZE (4096UL * 2)

#define SYS_CTRL_SC_DIE_ID0_OFFSET 0x200
#define SYS_CTRL_SC_DIE_ID_REG_OFFSET 0x4
/* register operation */
#define DEVDRV_REG_WR 0
#define DEVDRV_REG_RD 1

/* FFTS register base addr */
#define FFTS_REG_BASE 0x6A0000000UL

/* FFTS offset addr */
#define FFTS_OFFSET 0x0024U

#define IPC_DATA_MSG_LEN 1

#define CHECK_PROCESS_DMP "dmp_daemon"

#define KEY_CHIP_TYPE_INDEX   8

#define CHIP_TYPE_NOT_ASCEND 0xFF

typedef int (*dms_get_info_ops)(unsigned int dev_id, void *arg);
typedef int (*dms_get_vinfo_ops)(unsigned int dev_id, unsigned int vfid, void *arg);
#define DMS_OPS_DVPP_GET_STATUS "sys_get_dvpp_status"
#define DMS_OPS_DVPP_GET_UTIL_RATIO "sys_get_dvpp_utilization_ratio"

typedef enum {
    BOOT_FROM_ONCHIPROM,
    BOOT_FROM_XLOADER0,
    BOOT_FROM_XLOADER1,
    BOOT_FROM_BOTTOM
} BOOT_MODE;

struct dmanage_flash_info {
    unsigned long flash_id;         /* combined device & manufacturer code */
    unsigned short device_id;       /* device id */
    unsigned short vendor;          /* the primary vendor id */
    unsigned int state;             /* flash health */
    unsigned long size;             /* total size in bytes */
    unsigned int sector_count;      /* number of erase units */
    unsigned short manufacturer_id; /* manufacturer id */
};

struct dmanage_temp_share_mem {
    s8 cluster_temp;
    s8 peri_temp;
    s8 aicore0_temp;
    s8 aicore1_temp;
    u8 aicore_limit;
    u8 aicore_total_period;
    u8 aicore_elim_period;
    u8 aicore_base_freq_l;
    u8 aicore_base_freq_h;  // 基础频率占两个字节 默认小端
    s8 soc_max_temp;
    u8 ddr_capacity_l;
    u8 ddr_capacity_h;
    u8 ddr_freq_l;
    u8 ddr_freq_h;
    s8 limited_temp;
    s8 reset_temp;
};

enum lpm3_core_id {
    LPM3_CLUSTER_ID = 0,
    LPM3_PERI_ID = 1,
    LPM3_TS_ID = 2,
    LPM3_AICORE0_ID = 3,
    LPM3_AICORE1_ID = 4,
    LPM3_INVALID_ID,
};

#define DEVDRV_TS_CONFLICT_PROFILING 0xEF
enum dmanager_core_id {
    CLUSTER_ID = 0,  // because each ccpu's frequency is the same, so name cluster
    PERI_ID = 1,
    TS_ID = 2,
    DDR_ID = 3,
    AICORE0_ID = 4,  // cloud only use this
    AICORE1_ID = 5,
    INVALID_ID,
};

enum dmanage_sensor_id {
    CLUSTER_SENSOR = 0,
    PERI_SENSOR = 1,
    AICORE0_SENSOR = 2,
    AICORE1_SENSOR = 3,
    INVALID_SENSOR,
};

enum dmanager_tsensor_id {
    CLUSTER_TEMP_ID = 0,
    PERI_TEMP_ID = 1,
    AICORE0_TEMP_ID,
    AICORE1_TEMP_ID,
    AICORE_LIMIT_ID,       // AICORE限核状态 0 不限核 1 限核
    AICORE_TOTAL_PER_ID,   // AICORE 脉冲总周期
    AICORE_ELIM_PER_ID,    // aicore 可消除周期
    AICORE_BASE_FREQ_ID,   // aicore 基准频率 MHZ  返回 u16
    NPU_DDR_FREQ_ID,       // DDR 频率	单位 MHZ 返回 u16
    THERMAL_THRESHOLD_ID,  // 温饱限频温度 系统复位温度 返回 u8 * temp temp[0] temp[1]
    NTC_TEMP_ID,           // 四个热敏电阻温度   返回是数组 int * ret ret[0] ret[1] ret[2] ret[3]
    SOC_TEMP_ID,           // SOC最高温
    FP_TEMP_ID,            // optical module's temp
    N_DIE_TEMP_ID,
    HBM_TEMP_ID,  // hbm's max temp
    INVALID_TSENSOR_ID,
};

enum dmanage_freq_id {
    CCPU_FREQ = 0,
    DDR_FREQ = 1,
    AICORE0_FREQ = 2,
    AICORE1_FREQ = 3,
    HBM_FREQ = 4,
    VECTOR_FREQ = 5,
    DDR_INIT = 6,
    DDR_STATISTIC = 7,
    INVALID_FREQ,
};

void set_last_ko_insert_flag(void);
void clear_last_ko_insert_flag(void);
int is_last_ko_insert(void);
int devdrv_get_max_health_code(unsigned int dev_id, unsigned int *max_health_value,
    struct devdrv_error_code_para *error_code_para);
int devdrv_manager_get_emmc_voltage(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_enable_efuse_ldo2(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_disable_efuse_ldo2(struct file *filep, unsigned int cmd, unsigned long arg);


extern struct semaphore compute_power_sema[MAX_CHIP_NUM];
extern u32 devdrv_get_freq(void);
int devdrv_manager_check_call_process(void);
int devdrv_reg_op(unsigned char op_type, unsigned long base_phy_reg, unsigned long reg_offset,
    unsigned long map_size, unsigned int *val);

void devdrv_fresh_error_code_to_shm(struct work_struct *work);
int devdrv_fresh_event_code_to_shm(u32 devid, u32 *health_code, u32 health_len,
    struct shm_event_code *event_code, u32 event_len);
int devdrv_fresh_random_to_shm(u32 devid, char *random_number, u32 random_len);

#endif /* __DEVDRV_INFO_H */
