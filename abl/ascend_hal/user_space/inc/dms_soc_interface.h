/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Description: DMS module soc node head file.
 * Author: huawei
 * Create: 2021-09-27
 */

#ifndef __DMS_SOC_INTERFACE_H
#define __DMS_SOC_INTERFACE_H

#include "drv_internal.h"

/* Die ID */
#define DMS_PMU_DIEID_DATA_SIZE 8
typedef struct dms_soc_dieid_stru {
    unsigned int dev_id;
    unsigned int soc_type;
    unsigned int pmu_dieid_type;
    unsigned int pmu_dev_id;
    unsigned char size;
    unsigned int data[DMS_PMU_DIEID_DATA_SIZE];
} dms_soc_die_id_t;

/* PCIE ID */
typedef struct dms_pcie_id_info {
    unsigned int venderid;    /* Vender id */
    unsigned int subvenderid; /* Sub vender id */
    unsigned int deviceid;    /* Device id */
    unsigned int subdeviceid; /* Sub device id */
    unsigned int bus;         /* Bus id */
    unsigned int device;      /* Device id */
    unsigned int fn;          /* Function id */
    unsigned int davinci_id;  /* device logical id */
} dms_pcie_id_info_t;

#define MAX_CHIP_NAME 32
typedef struct dms_chip_info {
    unsigned char type[MAX_CHIP_NAME];
    unsigned char name[MAX_CHIP_NAME];
    unsigned char version[MAX_CHIP_NAME];
} dms_chip_info_t;

typedef struct dms_query_chip_info {
    unsigned int dev_id;
    unsigned int reg_val;
    dms_chip_info_t info;
} dms_query_chip_info_t;

drvError_t DmsGetBoardId(unsigned int dev_id, unsigned int *board_id);
drvError_t DmsGetPcbId(unsigned int dev_id, unsigned int *pcb_id);
drvError_t DmsGetBomId(unsigned int dev_id, unsigned int *bom_id);
drvError_t DmsGetSlotId(unsigned int dev_id, unsigned int *slot_id);
drvError_t DmsGetPcieIdInfo(unsigned int dev_id, dms_pcie_id_info_t *pcie_idinfo);
drvError_t DmsGetCpuInfo(unsigned int dev_id, drvCpuInfo_t *cpu_info);
drvError_t DmsGetSocDieId(dms_soc_die_id_t *soc_die_id);
drvError_t DmsGetChipInfo(dms_query_chip_info_t *chip_info);
drvError_t DmsGetRebootReason(unsigned int devId, void *rebootReason, unsigned int len);
drvError_t DmsSetBistInfo(unsigned int dev_id, unsigned int cmd, unsigned char *in_buf, unsigned int buf_len);
drvError_t DmsGetChipVersion(unsigned int devid, unsigned char *chip_version);

#endif /* __DMS_SOC_INTERFACE_H */
