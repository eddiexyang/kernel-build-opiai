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

#ifndef __SOC_MISC_CHIP_INFO_H
#define __SOC_MISC_CHIP_INFO_H

/* DIE ID */
#define DFT_SOC_DIEID 0
#define PMU_FIRST_DIEID 1
#define PMU_SECOND_PRE_DIEID 2
#define PMU_SECOND_BACK_DIEID 3
#define DFT_SOC_N_DIEID 4
#define PMU_DIEID_TYPE_MAX_ID 4
#define DEVDRV_SOC_N_DIE_ID 1
#define PMU_MAIN_DIEID 0
#define PMU_SECOND_DIEID 1
#define FIRST_PMU_DEVICE_ID 0x00
#define SECOND_PMU_PRE_DEVICE_ID 0x02
#define SECOND_PMU_BACK_DEVICE_ID 0x0B
#define PMU_DIEID_MAX_LEN 32
#define DMS_PMU_DIEID_DATA_SIZE 8

typedef struct dms_soc_dieid_stru {
    unsigned int dev_id;
    unsigned int soc_type;
    unsigned int pmu_dieid_type;
    unsigned int pmu_dev_id;
    unsigned char size;
    unsigned int data[DMS_PMU_DIEID_DATA_SIZE];
} dms_soc_die_id_t;

int soc_misc_get_die_id(dms_soc_die_id_t *die_id);
int soc_misc_get_chip_version(u32 dev_id, u8 *chip_version);

#endif /* __SOC_MISC_CHIP_INFO_H */
