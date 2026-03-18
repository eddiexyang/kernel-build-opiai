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
 * Description: DMS module user mode head file, include each node head file
 * Author: huawei
 * Create: 2021-09-27
 */

#ifndef __DMS_USER_INTERFACE_H
#define __DMS_USER_INTERFACE_H

#include "dms_soc_interface.h"
#include "dms_qos_interface.h"
#include "dms_memory_interface.h"
#include "dms_lpm_interface.h"
#include "dsmi_common_interface.h"

#define DMS_FAULT_INJECT_RESERVED_LEN     32
typedef struct dms_fault_inject_stru {
    int node_type;
    int node_id;
    int sub_node_type;
    int sub_node_id;
    int fault_type;
    int sub_fault_type;
    int times;              /* inject times */
    unsigned char reserved[DMS_FAULT_INJECT_RESERVED_LEN]; /* reserved data must be set to 0 */
} DMS_FAULT_INJECT_STRU;

struct dms_osc_freq {
    unsigned int dev_id;
    unsigned int sub_cmd;
    unsigned long long value;
};

int dms_power_set_all_device(void);

int DmsGetTsInfo(unsigned int dev_id, unsigned int vfid, unsigned int core_id, void *result, unsigned int result_size);
int dms_get_average_util_from_ts(unsigned int dev_id, unsigned int vfid, unsigned int sub_cmd, unsigned int *value);
int dms_get_single_util_from_ts(unsigned int dev_id, unsigned int vfid, unsigned int sub_cmd, void *out_buf,
    unsigned int *size);
int dms_get_ts_info(unsigned int dev_id, unsigned int vfid, unsigned int sub_cmd, void *out_buf, unsigned int *size);
int dms_set_ts_info(unsigned int dev_id, unsigned int sub_cmd, void *buf, unsigned int size);

drvError_t dms_get_device_topology(unsigned int dev_id1, unsigned int dev_id2, int *topology_type);
drvError_t dms_get_phy_devices_topology(unsigned int dev_id1, unsigned int dev_id2, int *topology_type);
int dms_get_dvpp_info(unsigned int dev_id, unsigned int vfid, unsigned int subcmd, void *buf, unsigned int *size);
drvError_t dms_set_sriov_switch(unsigned int dev_id, unsigned int sub_cmd, const void *buf, unsigned int buf_size);
drvError_t DmsCreateCapGroupInfo(int dev_id, int ts_id, struct capability_group_info *group_info);
drvError_t DmsDelCapGroupInfo(int dev_id, int ts_id, int group_id);
drvError_t DmsGetCapGroupInfo(int dev_id, int ts_id, int group_id, struct capability_group_info *group_info,
                              int group_count);
int DmsGetHccsInfo(unsigned int dev_id, unsigned int vfid, unsigned int sub_cmd, void *buf, unsigned int *size);
int DmsGetLastBootState(unsigned int dev_id, BOOT_TYPE boot_type, unsigned int *state);
int dms_fault_inject(unsigned int dev_id, unsigned int vfid, unsigned int main_cmd,
    unsigned int sub_cmd, DMS_FAULT_INJECT_STRU *para);
drvError_t DmsSetHostAicpuInfo(unsigned int dev_id, unsigned int sub_cmd, const void *buf, unsigned int size);
drvError_t DmsGetHostAicpuInfo(unsigned int dev_id, unsigned int main_cmd,
    unsigned int sub_cmd, void *buf, unsigned int *size);
int dms_get_aicpu_utilization(unsigned int dev_id, unsigned int *utilization);

drvError_t DmsDeviceInitStatus(unsigned int dev_id, unsigned int *status);
drvError_t DmsTsHeartbeatStatus(unsigned int dev_id, unsigned int vf_id, unsigned int ts_id, unsigned int *status);

#endif /* __DMS_USER_INTERFACE_H */
