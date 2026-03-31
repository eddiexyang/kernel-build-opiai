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

#ifndef PROF_TS_H
#define PROF_TS_H
#include <linux/types.h>

#include "prof_def.h"

#define PROF_CQ_BUF_LEN  32
#define TS2DRV_TIMEOUT (5 * (HZ)) /* 5 second */

#ifdef CFG_SOC_PLATFORM_MINI
enum cq_info_type {
    callback_cq = 0,
    report_cq,
    callback_hs_cq = callback_cq,
    report_hs_cq = report_cq
};
#else
enum cq_info_type {
    callback_cq = 0,
    report_cq,
    callback_hs_cq, // support high speed data channel, such as HWTS
    report_hs_cq
};
#endif

enum prof_ts_cmd_type {
    TS_START = 1,
    TS_STOP = 2,
    TS_FULL = 3,
    TS_SYNC_READ_PTR = 4,
    TS_SYNC_WRITE_PTR = 5
};

enum mailbox_cmd_type {
    CALC_CQSQ_CREATE = 0x1,
    CALC_CQSQ_RELEASE,
    LOG_CQSQ_CREATE,
    LOG_CQSQ_RELEASE,
    DEBUG_CQSQ_CREATE,
    DEBUG_CQSQ_RELEASE,
    PROFILE_CQSQ_CREATE,
    PROFILE_CQSQ_RELEASE
};

typedef struct prof_cqsq_info {
    u32 sq_0_index;
    u32 cq_0_index;
    u32 cq_1_index;
    u32 cq_2_index;
    unsigned long long sq_0_addr;
    unsigned long long cq_0_addr;
    unsigned long long cq_1_addr;
    unsigned long long cq_2_addr;
    struct mutex sq_mutex;       /* for single thread */
} prof_cqsq_info_t;

/* need ts to fill */
typedef struct prof_cq_scheduler {
    u32 cmd_verify;
    u32 channel_id;
    u32 sub_channel_id;
    u32 channel_cmd; /* type: start, stop, full */
    int ret_val;              /* the result; */
    u32 device_id;   /* device_id is offered by ts */
    u32 vfid;
} prof_cq_scheduler_t;

/* sizeof(prof_sq_scheduler_t): 128 byte */
#define PROF_TS_CPU_DATA 72
typedef struct prof_sq_scheduler {
    u32 cmd_verify;
    u32 channel_id;
    u32 channel_cmd;
    u32 buf_len;
    unsigned long buf_phy_addr;
    u32 buf_num;                     /* add for ascd610 | ascd310p */
    u32 vfid;
    unsigned long com_buf_phy_addr;  /* add for ascd910 */
    u32 com_buf_len;                 /* add for ascd910 */
    u32 sub_channel_id;              /* add for ascd910B */
    u32 data_size;
    u32 collect_target_pid;
    unsigned char ts_cpu_data[PROF_TS_CPU_DATA];
} prof_sq_scheduler_t;

typedef struct prof_tscpu_buf_info {
    unsigned char *virt_addr;
    unsigned long phy_addr;
    u32 len;
} prof_tscpu_buf_info_t;

int prof_tscpu_start(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para);
int prof_tscpu_read(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para);
int prof_tscpu_stop(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para);
void prof_show_dfx_info(struct prof_sub_channel_info *sub_channel_info);
void prof_init_dfx_info(struct prof_sub_channel_info *sub_channel_info);
int prof_cqsq_init(u32 device_id, u32 tsid);
void prof_cqsq_uninit(u32 device_id, u32 tsid);
int prof_tscpu_init_channel_info(struct prof_sub_channel_info *sub_channel_info);
u32 prof_get_buff_data_len(u32 device_id, u32 channel_id, u32 vfid, u32 sub_channel_id);
void prof_init_tsid(struct prof_sub_channel_info *sub_channel_info);
int prof_channel_need_sync_rw_ptr(struct prof_sub_channel_info *sub_channel_info);
void prof_tscpu_free_channel_memory(struct prof_sub_channel_info *sub_channel_info);
int prof_tscpu_alloc_channel_memory(struct prof_sub_channel_info *sub_channel_info);
void prof_sq_report_profile(u32 device_id_in, u32 tsid,
    const unsigned char *cq_buf, unsigned char *sq_buf);
void prof_cq_callback_profile(u32 device_id_in, u32 tsid,
    const unsigned char *cq_buf, unsigned char *sq_buf);
void prof_cq_callback_aicpu(u32 device_id, u32 tsid,
    const unsigned char *cq_buf, unsigned char *sq_buf);
int prof_check_channel_with_vfid(u32 device_id, u32 channel_id, u32 vfid);
int prof_data_len_flush(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *prof_para);
prof_tscpu_buf_info_t *prof_tscpu_get_common_buf_info(u32 dev_id, u32 channel_id);
int prof_tscpu_platform_read_sub_ch(struct prof_ioctl_para *para,
    struct prof_sub_channel_info *sub_channel_info);
u32 prof_tscpu_get_one_zone_data_len(unsigned char *base, u32 channel_group_data_len);
u32 prof_tscpu_get_data_len(struct prof_sub_channel_info *sub_channel_info);
int prof_tscpu_ringbuffer(unsigned char *base, struct prof_ioctl_para *para,
    struct prof_dev_dfx_info *prof_dfx, u32 sub_channel_id);
int prof_alloc_all_tscpu_common_bufs(u32 device_id);
void prof_free_all_tscpu_common_bufs(u32 device_id);
u32 prof_sq_init_get_buf_num(u32 channel_id, u32 vfid);
u32 prof_sq_init_get_buf_len(struct prof_sub_channel_info *sub_channel_info);
void prof_platform_ts_buff_free_flag(enum prof_ts_buff_free_flag *flag);
void prof_platform_ts_buff_not_free_op(struct prof_sub_channel_info *sub_channel_info);
u64 prof_sq_init_get_buf_pa(struct prof_sub_channel_info *sub_channel_info);
void prof_dev_tscpu_all_stop(u32 device_id);
#endif
