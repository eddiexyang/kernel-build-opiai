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

#ifndef DRV_MEMORY_DDR_H
#define DRV_MEMORY_DDR_H
#include "drv_profile.h"
#include "memory_ddr_cfg.h"
#include "memory_fault_init_cfg.h"

#define FREQ_OFFSET 0
#define DRAM_SIZE_OFFSET (sizeof(unsigned int) * 1)
#define DRAM_TPYE_OFFSET (sizeof(unsigned int) * 2)
#define CHAN_MASK_OFFSET (sizeof(unsigned int) * 3)
#define RANK_NUM_OFFSET (sizeof(unsigned int) * 4)
#define TMON_ALARM_OFFSET (sizeof(unsigned int) * 5)
#define IECC_EN_OFFSET (sizeof(unsigned int) * 6)
#define ECC_SINGLE_BIT_OFFSET (sizeof(unsigned int) * 7)
#define ECC_DOBIT_OFFSET (sizeof(unsigned int) * 8)
#define MANU_INFO_OFFSET (sizeof(unsigned int) * 9)
#define TMON_RATE_OFFSET (sizeof(unsigned int) * 9 + DDR_MAX_CHAN_NUM)
typedef struct {
	/* ddr frequency */
	unsigned int freq;
	/* ddr total size */
	unsigned int dram_size;
	/* ddr type, 0:LPDDR4 1:LPDDR5 */
	unsigned int dram_type;
	/* channel bitmask, 0:not exist 1:exist */
	unsigned int chan_mask;
	/* rank number, 1 single 2: double */
	unsigned int rank_num;
	/* not use, Alarm temperature gear */
	unsigned int tmon_alarm;
	/* ecc enable, 1: enable, 0: disable */
	unsigned int iecc_en;
	/* ecc single bit err */
	unsigned int ce_count;
	/* ecc Dobit err */
	unsigned int ue_count;
	/* Vendors */
	unsigned char manu_info[DDR_MAX_CHAN_NUM];
	/* not use, Current Temperature gear */
	unsigned char tmon_rate[DDR_MAX_CHAN_NUM];
} MEMORY_INFO;

/* The same as DSMI sub commond for memory module */
#define DMS_SUBCMD_MEMORY_TYPE 0
#define DMS_SUBCMD_MEMORY_CAPACITY 1
#define DMS_SUBCMD_MEMORY_CHANNEL 2
#define DMS_SUBCMD_MEMORY_RANK_NUM 3
#define DMS_SUBCMD_MEMORY_ECC_ENABLE 4
#define DMS_SUBCMD_MEMORY_MANUFACTURES 5

#define DMS_FILTER_MEMORY "main_cmd=0x9"

typedef int32_t (*FP_MEMORY_DSMI_GET_DEVICE_INFO)(
	unsigned int dev_id, unsigned sub_cmd, char *in, unsigned int in_len, unsigned int *out_len);

struct memory_dsmi_get_device_info {
	unsigned int sub_cmd;
	FP_MEMORY_DSMI_GET_DEVICE_INFO fn_get;
};

#ifdef CFG_MEMORY_DDR_INFO_FROM_REG
struct ddrc_info_t {
	unsigned long reg_base_array[DDR_MAX_CHAN_NUM];
	unsigned int chan_num;
	unsigned int bitmap;
};

int get_ddr_info_from_dts(struct ddrc_info_t *info);
#endif
int memory_map_ddr_init(void);
void memory_map_ddr_exit(void);
int memory_ddr_prof_sample(struct prof_peri_para para);
int memory_ddr_prof_init_user_cfg(struct prof_peri_para para);
int memory_ddr_reg_op(unsigned char op_type, const void __iomem *vir_addr, unsigned long reg_offset, unsigned int *val);
int memory_ddr_stat_init(unsigned int device_id, int chip_type, unsigned int chan_num, unsigned int chan_bitmap);
int memory_get_ddr_bandwidth_utilization_rate(void *feature, char *in, unsigned int in_len, char *out,
	unsigned int out_len);
int get_ddr_ecc_statistics_from_reg(struct memory_info *user_arg);
int memory_get_ddr_bw_util_rate(u32 dev_id, u32 chan_num, u32 bitmap, u32 *rate);
#ifdef CFG_MEMORY_DDR_INFO_FROM_SHAREMEM
int memory_get_ddr_info_from_sharemem(unsigned long offset, void *buff, unsigned int size);
int get_ddr_ecc_statistics_from_sharemem(struct memory_info *memory_arg);
int memory_get_ddr_frequency(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len);
#endif

#if defined(CFG_MEMORY_DDR_INFO_FROM_LP) || defined(CFG_MEMORY_DDR_INFO_FROM_SHAREMEM)
int memory_dmsi_get_device_info(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len);
#endif

#endif
