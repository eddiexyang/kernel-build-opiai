/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */

#ifndef __TSMNG_GROUP_H__
#define __TSMNG_GROUP_H__

#define MAX_IPC_MSG_TYPE 10

#define DMS_MODULE_TSMNG    "drv_tsmng"

#if defined(CFG_FEATURE_LP_ENABLE)
typedef int (*check_rsp_msg)(struct ts_ipcdrv_message *req_msg, struct ts_ipcdrv_message *rsp_msg, int retry_time);

typedef void (*ipc_rsp_msg_handle)(int dev_id, struct ts_ipcdrv_message *rsp_msg,
                                   void *send_channel_info);
// msg and channel is bind, sender and receive channel must agree and same
struct ipc_msg_channel_info {
    unsigned int device_id;
    unsigned int msg_type;
    unsigned int send_channel_id;
    unsigned int receive_channel_id;
    struct mutex msg_mutex;
    struct semaphore msg_sema;
    unsigned int msg_serial_num;
    unsigned char rsp_msg[ICM_MSG_MAX_LENGTH];
    ipc_rsp_msg_handle ipc_rsp_handle_fun;
    check_rsp_msg ipc_rsp_check_handle_fun;
};

void devdrv_tsid_decode(unsigned int m_id, unsigned int *tsid);
void devdrv_handle_ipc_msg(int device_id, void *data);
void devdrv_ipc_ts_msg_channel_init(u32 dev_id);
void devdrv_ipc_ts_msg_channel_destroy(u32 dev_id);
#endif

#ifdef AOS_LLVM_BUILD
#define TSMNG_AI_CORE_NUM 10  // 当前AI_CORE_NUM = 10针对MDC场景写死，后续收编linux侧需要针对DC场景区分定义
#define TSMNG_AI_VECTOR_NUM 8
#define TSMNG_AI_PROFILING_VALUE  0xEF
#define TSMNG_AI_PROFILING_CONFLIC_RET 2
#define TSMNG_AI_TIMEOUT_RET 7
#define START_MSG 0
#define END_MSG 1

typedef enum {
    DSMI_TS_SUB_AICORE_UTILIZATION_RATE = 0,
    DSMI_TS_SUB_VECTORCORE_UTILIZATION_RATE = 1,
    DSMI_TS_SUB_CMD_MAX_VALUE
} DSMI_SUB_TS_INFO;
typedef struct _core_utilization_rate {
    unsigned int dev_id;
    unsigned int cmd_type;
    unsigned char *core_utilization_rate;
    unsigned int core_num;
    unsigned int vfid;
} core_utilization_rate_t;

void tsmng_feature_init(void);
void tsmng_feature_exit(void);
#endif

#endif  /* __TSMNG_GROUP_H__ */
