/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef PROF_PERIPHERAL_H
#define PROF_PERIPHERAL_H

#include "prof_def.h"

#define PROF_PRINT_MAX 5

/* these function names is provided by peripheral modules that will be sampled.
 * profiling driver do not depend on them by dynamicly load their symbols */
#define PROF_HBM_USERDATA_FUNC_NAME "memory_hbm_prof_init_user_cfg"
#define PROF_HBM_SAMPLE_FUNC_NAME   "memory_hbm_prof_sample"
#define PROF_NIC_SAMPLE_FUNC_NAME   "higmac_sample_fun"
#define PROF_DVPP_SAMPLE_FUNC_NAME  "dvpp_profile_func"
#define PROF_DDR_USERDATA_FUNC_NAME "memory_ddr_prof_init_user_cfg"
#define PROF_DDR_SAMPLE_FUNC_NAME   "memory_ddr_prof_sample"
#define PROF_PCIE_USERDATA_FUNC_NAME    "agentdrv_pcie_profiling_open"
#define PROF_PCIE_SAMPLE_FUNC_NAME  "agentdrv_pcie_profiling_sampling"
#define PROF_PCIE_STOP_FUNC_NAME    "agentdrv_pcie_profiling_close"
#define PROF_LLC_USERDATA_FUNC_NAME "devdrv_llc_prof_sample_userdata"
#define PROF_LLC_SAMPLE_FUNC_NAME   "devdrv_llc_prof_sample"
#define PROF_LLC_STOP_FUNC_NAME     "devdrv_llc_prof_sample_stop"
#define PROF_HCCS_SAMPLE_FUNC_NAME  "devdrv_hccs_prof_sample"
#define PROF_HCCS_STOP_FUNC_NAME        "devdrv_hccs_prof_stop"
#define PROF_ROCE_SAMPLE_FUNC_NAME  "hns_roce_prof_sample"
#define PROF_NPU_APP_USERDATA_FUNC_NAME  "prof_proc_mem_sample_init"
#define PROF_NPU_APP_SAMPLE_FUNC_NAME  "prof_sample_proc_mem_used_info"
#define PROF_NPU_USERDATA_FUNC_NAME  "prof_dev_mem_sample_init"
#define PROF_NPU_SAMPLE_FUNC_NAME  "prof_sample_dev_mem_used_info"
#define PROF_LP_SAMPLE_FUNC_NAME  "lpm_devmng_prof_sample_syscnt_handle"

#define PROF_DVPP_VENC_SAMPLE_FUNC_NAME "dvpp_venc_profile_func"
#define PROF_DVPP_JPEGE_SAMPLE_FUNC_NAME "dvpp_jpege_profile_func"
#define PROF_DVPP_VDEC_SAMPLE_FUNC_NAME "dvpp_vdec_profile_func"
#define PROF_DVPP_JPEGD_SAMPLE_FUNC_NAME "dvpp_jpegd_profile_func"
#define PROF_DVPP_VPC_SAMPLE_FUNC_NAME "dvpp_vpc_profile_func"

enum prof_sample_thread_status {
    PROF_SAMPLE_THREAD_DISABLE = 0,
    PROF_SAMPLE_THREAD_ENABLE,
    PROF_SAMPLE_THREAD_EXIT
};

#define PROF_PERI_SAMPLE_MODE_MAX   3
enum peri_sample_mode {
    PROF_PERI_SAMPLE,
    PROF_PERI_SAMPLE_USERDATA,
    PROF_PERI_SAMPLE_STOP
};

int prof_agent_start(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para);
int prof_peripheral_callback_register(u32 channel_id, struct prof_sub_channel_info *sub_channel_info);
char *prof_get_peri_cb_regs_handler(u32 chann_id, u32 mode);
int prof_agent_read(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para);
int prof_agent_stop(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para);
u32 prof_agent_get_data_len(struct prof_sub_channel_info *sub_channel_info);
void prof_dev_agent_all_stop(u32 device_id);
bool prof_agent_sample_valid(u32 channel_id);
void prof_peripheral_chan_free_mem(unsigned char *va, u32 size);
#endif

