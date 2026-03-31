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

#ifndef MEMORY_FAULT_INIT_CFG_H
#define MEMORY_FAULT_INIT_CFG_H

#include <linux/types.h>
#include <linux/securec.h>
#include <linux/securectype.h>
#include <linux/cdev.h>

#include "ascend_kernel_hal.h"
#include "dms_sensor.h"
#include "dms_sensor_type.h"
#include "drv_log.h"
#include "dms_define.h"
#include "fpdc.h"
#include "fpdc_ras_receiver.h"

#ifdef CFG_MEMORY_SAFETY_RAS
#include "safety_ras_node.h"
#endif

#define TYPE_RAS DMS_SEN_TYPE_RAS_SENSOR
#define TYPE_MEM DMS_SEN_TYPE_MEMORY

#ifndef DRV_HBM_UT
#define STATIC static
#else
#define STATIC
#endif

/* MEM: memory error type define by UEFI, [0x0,0xf] is standard */
#define STANDARD_ERR_TYPE_SINGLE_BIT_ECC		(0x2)
#define STANDARD_ERR_TYPE_SINGLE_BIT_ECC_THOLD (0x16)
#define STANDARD_ERR_TYPE_MULTI_BIT_ECC		 (0x3)
#define STANDARD_ERR_TYPE_MEMORY_SPARING		(0xC)
#define STANDARD_ERR_TYPE_SCRUB_CORRECT		 (0xD)
#define STANDARD_ERR_TYPE_SCRUB_UNCORRECT	   (0xE)
#ifdef CFG_HBM_FAULT_EVENT
#define STANDARD_ERR_TYPE_DSCRUB_UNCORRECT	  (0x17)
#else
#define STANDARD_ERR_TYPE_DSCRUB_UNCORRECT	  (0xF7)
#endif

/* MEM: error type greater than 0xf is custom */
#define HISI_ERR_TYPE_HA_CORRECT				(0x10)
#define HISI_ERR_TYPE_HA_UNCORRECT			  (0x11)
#define HISI_ERR_TYPE_VLS_CORRECT			   (0x12)
#define HISI_ERR_TYPE_VLS_UNCORRECT			 (0x13)
#define HISI_ERR_TYPE_RVLS_CORRECT			  (0x14)
#define HISI_ERR_TYPE_RVLS_UNCORRECT			(0x15)

/* module id */
#define RAS_SMMU_MODULE	 0
#define RAS_HHA_MODULE	  1
#define RAS_PA_MODULE	   2
#define RAS_HLLC_MODULE	 3
#define RAS_L2BUFF_MODULE   4
#define RAS_MEMORY_MODULE   5
#define RAS_INVALID_MODULE  0xff

/* SUB MODULE ID */
#define DDRC0_SUB_MODULE		0x00
#define DDRC1_SUB_MODULE		0x01

#define HBM0_HBMC00_SUB_MODULE  0x02
#define HBM0_HBMC01_SUB_MODULE  0x03
#define HBM0_HBMC02_SUB_MODULE  0x04
#define HBM0_HBMC03_SUB_MODULE  0x05
#define HBM0_HBMC04_SUB_MODULE  0x06
#define HBM0_HBMC05_SUB_MODULE  0x07
#define HBM0_HBMC06_SUB_MODULE  0x08
#define HBM0_HBMC07_SUB_MODULE  0x09
#define HBM0_HBMC08_SUB_MODULE  0x0A
#define HBM0_HBMC09_SUB_MODULE  0x0B
#define HBM0_HBMC10_SUB_MODULE  0x0C
#define HBM0_HBMC11_SUB_MODULE  0x0D
#define HBM0_HBMC12_SUB_MODULE  0x0E
#define HBM0_HBMC13_SUB_MODULE  0x0F
#define HBM0_HBMC14_SUB_MODULE  0x10
#define HBM0_HBMC15_SUB_MODULE  0x11

#define HBM1_HBMC00_SUB_MODULE  0x12
#define HBM1_HBMC01_SUB_MODULE  0x13
#define HBM1_HBMC02_SUB_MODULE  0x14
#define HBM1_HBMC03_SUB_MODULE  0x15
#define HBM1_HBMC04_SUB_MODULE  0x16
#define HBM1_HBMC05_SUB_MODULE  0x17
#define HBM1_HBMC06_SUB_MODULE  0x18
#define HBM1_HBMC07_SUB_MODULE  0x19
#define HBM1_HBMC08_SUB_MODULE  0x1A
#define HBM1_HBMC09_SUB_MODULE  0x1B
#define HBM1_HBMC10_SUB_MODULE  0x1C
#define HBM1_HBMC11_SUB_MODULE  0x1D
#define HBM1_HBMC12_SUB_MODULE  0x1E
#define HBM1_HBMC13_SUB_MODULE  0x1F
#define HBM1_HBMC14_SUB_MODULE  0x20
#define HBM1_HBMC15_SUB_MODULE  0x21

#define HBM2_HBMC00_SUB_MODULE  0x22
#define HBM2_HBMC01_SUB_MODULE  0x23
#define HBM2_HBMC02_SUB_MODULE  0x24
#define HBM2_HBMC03_SUB_MODULE  0x25
#define HBM2_HBMC04_SUB_MODULE  0x26
#define HBM2_HBMC05_SUB_MODULE  0x27
#define HBM2_HBMC06_SUB_MODULE  0x28
#define HBM2_HBMC07_SUB_MODULE  0x29
#define HBM2_HBMC08_SUB_MODULE  0x2A
#define HBM2_HBMC09_SUB_MODULE  0x2B
#define HBM2_HBMC10_SUB_MODULE  0x2C
#define HBM2_HBMC11_SUB_MODULE  0x2D
#define HBM2_HBMC12_SUB_MODULE  0x2E
#define HBM2_HBMC13_SUB_MODULE  0x2F
#define HBM2_HBMC14_SUB_MODULE  0x30
#define HBM2_HBMC15_SUB_MODULE  0x31

#define HBM3_HBMC00_SUB_MODULE  0x32
#define HBM3_HBMC01_SUB_MODULE  0x33
#define HBM3_HBMC02_SUB_MODULE  0x34
#define HBM3_HBMC03_SUB_MODULE  0x35
#define HBM3_HBMC04_SUB_MODULE  0x36
#define HBM3_HBMC05_SUB_MODULE  0x37
#define HBM3_HBMC06_SUB_MODULE  0x38
#define HBM3_HBMC07_SUB_MODULE  0x39
#define HBM3_HBMC08_SUB_MODULE  0x3A
#define HBM3_HBMC09_SUB_MODULE  0x3B
#define HBM3_HBMC10_SUB_MODULE  0x3C
#define HBM3_HBMC11_SUB_MODULE  0x3D
#define HBM3_HBMC12_SUB_MODULE  0x3E
#define HBM3_HBMC13_SUB_MODULE  0x3F
#define HBM3_HBMC14_SUB_MODULE  0x40
#define HBM3_HBMC15_SUB_MODULE  0x41

#define HBM0_CHN0 0x00U
#define HBM0_CHN1 0x01U
#define HBM0_CHN2 0x02U
#define HBM0_CHN3 0x03U
#define HBM0_CHN4 0x04U
#define HBM0_CHN5 0x05U
#define HBM0_CHN6 0x06U
#define HBM0_CHN7 0x07U

#define HBM1_CHN0 0x10U
#define HBM1_CHN1 0x11U
#define HBM1_CHN2 0x12U
#define HBM1_CHN3 0x13U
#define HBM1_CHN4 0x14U
#define HBM1_CHN5 0x15U
#define HBM1_CHN6 0x16U
#define HBM1_CHN7 0x17U

#define HBM2_CHN0 0x20U
#define HBM2_CHN1 0x21U
#define HBM2_CHN2 0x22U
#define HBM2_CHN3 0x23U
#define HBM2_CHN4 0x24U
#define HBM2_CHN5 0x25U
#define HBM2_CHN6 0x26U
#define HBM2_CHN7 0x27U

#define HBM3_CHN0 0x30U
#define HBM3_CHN1 0x31U
#define HBM3_CHN2 0x32U
#define HBM3_CHN3 0x33U
#define HBM3_CHN4 0x34U
#define HBM3_CHN5 0x35U
#define HBM3_CHN6 0x36U
#define HBM3_CHN7 0x37U

#define MAX_CPER_SUB_SYS_NUM	   4
#define MAX_CPER_DEVICE_OR_CORE_ID 16
#define MAX_DIE 2
#define MAX_GROUP_NUM 2U
#define MAX_MATA_NUM_PER_GROUP 4U
#define mask(n)     ((0x1UL << (n)) - 1UL)

#pragma pack(1)
struct hbm_addr_trans_info {
	uint32_t magic;
	uint8_t dmc_intlv_en;
	uint8_t dmc_intlv_xor;
	uint8_t dmc_intlv_bit;
	uint8_t group_intlv;
	uint8_t group_tgt;
	uint8_t group_intlv_xor;
	uint8_t group_intlv_bit;
	uint8_t mata_intlv_xor;
	uint8_t ch_intlv;
	uint8_t pg_bitmap;
	uint8_t sid_start_bit;
	uint8_t bank_xor;
	uint8_t bnk_start_bit;
	uint8_t col_start_bit_l;
	uint8_t col_start_bit_h;
	uint8_t col_width_l;
	uint8_t col_width_h;
	uint8_t row_start_bit_l;
	uint8_t row_start_bit_h;
	uint8_t row_width_l;
	uint8_t row_width_h;
	uint8_t mata_id_remap[MAX_DIE][MAX_GROUP_NUM][MAX_MATA_NUM_PER_GROUP];
};
#pragma pack()

// device_or_core_id -- stack number, sub_sys_num -- pc number
static inline uint32_t get_common_mem_submodule(uint32_t sub_sys_num, uint32_t device_or_core_id)
{
	return sub_sys_num * (MAX_CPER_DEVICE_OR_CORE_ID) + device_or_core_id + (DDRC1_SUB_MODULE + 1);
}

#define MEMORY_OFFSET_8BIT 8
#define MEMORY_OFFSET_16BIT 16
#define MEMORY_OFFSET_24BIT 24
#define MEMORY_OFFSET_32BIT 32
#define MEMORY_MASK_32BIT 0xFFFFFFFF
#define MEMORY_MASK_16BIT 0xFFFF

int memory_fault_event_scan(unsigned long long function, struct dms_sensor_event_data *data);
void memory_fault_event_handler(const struct notify_data *pdata);
struct drv_memory_devices *memory_get_ctrl(void);
unsigned int devm_get_chip_type(void);

int dms_register_dev_node(struct dms_node *node);
int dms_unregister_dev_node(struct dms_node *node);
int dms_register_notifier(struct notifier_block* nb);
int dms_unregister_notifier(struct notifier_block* nb);

int hbm_get_dev_info_list(struct dms_node *device, struct dms_dev_data_attr *info_list);
int hbm_get_dev_state(struct dms_node *device, unsigned int *state);
int hbm_get_dev_capacity(struct dms_node *device, unsigned long long *capacity);
int hbm_set_dev_power_state(struct dms_node *device, DSMI_POWER_STATE power_state);
int hbm_ops_init(struct dms_node *device);
void hbm_ops_exit(struct dms_node *device);

int ddr_get_dev_info_list(struct dms_node *device, struct dms_dev_data_attr *info_list);
int ddr_get_dev_state(struct dms_node *device, unsigned int *state);
int ddr_get_dev_capacity(struct dms_node *device, unsigned long long *capacity);
int ddr_set_dev_power_state(struct dms_node *device, DSMI_POWER_STATE power_state);
int ddr_ops_init(struct dms_node *device);
void ddr_ops_exit(struct dms_node *device);

int dha_get_dev_info_list(struct dms_node *device, struct dms_dev_data_attr *info_list);
int dha_get_dev_state(struct dms_node *device, unsigned int *state);
int dha_get_dev_capacity(struct dms_node *device, unsigned long long *capacity);
int dha_set_dev_power_state(struct dms_node *device, DSMI_POWER_STATE power_state);
int dha_ops_init(struct dms_node *device);
void dha_ops_exit(struct dms_node *device);

int hha_get_dev_info_list(struct dms_node *device, struct dms_dev_data_attr *info_list);
int hha_get_dev_state(struct dms_node *device, unsigned int *state);
int hha_get_dev_capacity(struct dms_node *device, unsigned long long *capacity);
int hha_set_dev_power_state(struct dms_node *device, DSMI_POWER_STATE power_state);
int hha_ops_init(struct dms_node *device);
void hha_ops_exit(struct dms_node *device);
#endif

