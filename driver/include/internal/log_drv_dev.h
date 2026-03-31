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
#ifndef LOG_DRV_DEV
#define LOG_DRV_DEV

#ifdef CONFIG_GENERIC_BUG
#undef CONFIG_GENERIC_BUG
#endif
#ifdef CONFIG_BUG
#undef CONFIG_BUG
#endif
#ifdef CONFIG_DEBUG_BUGVERBOSE
#undef CONFIG_DEBUG_BUGVERBOSE
#endif

#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/timer.h>
#include <linux/securec.h>
#include <linux/wait.h>
#include "drv_log.h"

#define MODULE_LOG             "drv_log"
#ifndef LOG_UNIT_TEST
#define slog_drv_err(fmt, ...)   \
    drv_err(MODULE_LOG, "<%s:%d,%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define slog_drv_info(fmt, ...)  \
    drv_info(MODULE_LOG, "<%s:%d,%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define slog_drv_debug(fmt, ...)
#define slog_drv_warn(fmt, ...)  \
    drv_warn(MODULE_LOG, "<%s:%d,%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define slog_drv_event(fmt, ...)  \
    drv_event(MODULE_LOG, "<%s:%d,%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#else
#define slog_drv_err(fmt, ...)   printf(fmt, ##__VA_ARGS__)
#define slog_drv_info(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#define slog_drv_debug(fmt, ...)
#define slog_drv_warn(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#define slog_drv_event(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#endif

#define LOG_DEBUG_CHANINFO_SHOW(c)  \
    slog_drv_debug("CHAN INFO. (channel_id=%d; channel_conn=%d; " \
        "channel_state=%x; buf_size=%u; phy_addr=%pK; " \
        " vir_addr=%pK; vir_addr_kmalloc=%pK; log_level=%d; " \
        "log_type=%d; ret_val=%d; print_num=%d; log_desc=0x%llx)\n", \
        (c)->channel_id, (c)->channel_conn, (u32)(c)->channel_state, \
        (c)->buf_size, (void *)(uintptr_t)(c)->phy_addr, (void *)(uintptr_t)(c)->vir_addr, \
        (void *)(uintptr_t)(c)->vir_addr_kmalloc, \
        (c)->log_level, (c)->log_type, (c)->ret_val, (c)->print_num, (u64)(c)->log_desc)

#define LOG_DEBUG_BUFFHEAD_SHOW(b)  \
    slog_drv_debug("BUFF HEAD. (buf_read=%u; buf_len=0x%x; " \
        "buf_write=%u; log_level=%u; rev[0]=0x%x)\n", \
        (b)->buf_read, (b)->buf_len, (b)->buf_write, (b)->log_level, (b)->rev[0])

#define LOG_DEBUG_READ_START_SHOW(c, b)  \
    slog_drv_debug("START READ. (channel_id=%d; phy_addr=%pK; data_phy_addr=%pK; buf_read=%u; buf_write=%u; " \
        "buf_len=0x%x; log_level=%u)\n", \
        (c)->channel_id, (void *)(uintptr_t)(c)->phy_addr, (void *)(uintptr_t)(c)->phy_addr + 128, (b)->buf_read, \
        (b)->buf_write, (b)->buf_len, (b)->log_level)

#define LOG_DEBUG_READ_STOP_SHOW(c, b)  \
    slog_drv_debug("STOP READ. (channel_id=%d; buf_read=%u; buf_write=%u)\n", \
        (c)->channel_id, (b)->buf_read, (b)->buf_write)

#define LOG_DEBUG_CQ_REPORT_SHOW(cq_report)  \
    slog_drv_debug("CQ REPORT. (device_id=%d; channel_id=%d; tsid=%u; cmd_verify=%u; channel_cmd=%u; ret_val=%u)\n", \
       (cq_report)->device_id, (cq_report)->channel_id, (cq_report)->tsid, (cq_report)->cmd_verify, \
       (cq_report)->channel_cmd, (cq_report)->ret_val)

#define LOG_CQSQ_INFO_SHOW(tsid, cqsq_info)  \
    slog_drv_debug("LOG_CQSQ_INFO. (tsid=%d; sq_0_index=%u; cq_0_index=%u; cq_1_index=%u)\n", \
        (tsid), (cqsq_info)->sq_0_index, (cqsq_info)->cq_0_index, (cqsq_info)->cq_1_index)
#ifdef LOG_UNIT_TEST
#define STATIC
#ifndef likely
#define likely(x) x
#endif
#ifndef unlikely
#define unlikely(x) x
#endif
#else
#define STATIC static
#endif

#ifndef UNUSED
#define UNUSED(x)   do {(void)(x);} while (0)
#endif

#define LOG_DEVICE_ID_MAX 64
#define LOG_CHANNEL_NUM_MAX 64

#define LOG_WAIT_TS_NUM      50
#define LOG_WAIT_HDC_NUM     300
#define LOG_SQ_BUF_LEN       128
#define LOG_CQ_BUF_LEN       32
#define LOG_COLLECT_BUF_SIZE (4 * 1024 * 1024)
#define LOG_COLLECT_BUF_THRESHOLD_SIZE (((u32)LOG_COLLECT_BUF_SIZE) >> 1)
#define LOG_SEGMENT_SIZE (512 * 1024)
#define LOG_SEGMENT_SIZE_MAX (1024 * 1024)
/* the same as HDCDRV_EPOLL_* */
/* the same as HDC_EPOLL_OP_* */
#define LOG_HDC_INVALID_SESSION (-1)

#define CHAR_DRIVER_NAME     "log_drv"

#define LOG_CHANNEL_DISABLE 0
#define LOG_CHANNEL_ENABLE  0xABABAB

#define TS2DRV_TIMEOUT (5 * HZ) /* 5 second */

#define ISP_START_FLAG 0xABABABABU

#define DEV_UNUSED    0
#define DEV_USED      1
#define LOG_PRINT_MAX 5

#define LOG_MSG_VERSION 0xff01
#define LOG_SUSPEND     0xabcd

#define LOG_COMPRESS_HW_ALG_NAME "zlib-deflate"

#define LOG_HDCDRV_SERVICE_TYPE_LOG 5
#define LOG_HDCDRV_SEND_TIMEOUT     3000  // ms

#define LOG_DELAY_TIME 1000  // ms

#define LOG_SCHEDULE_INTERVAL        200  // ms
#define LOG_CALC_GENERATE_RATE_UNIT  HZ
#define LOG_CALC_GENERATE_RATE_COUNT 5

#define LOG_MAX_PAGE_NUM_OF_KZALLOC 1024
#define DC_TSNUM  1
#ifdef CFG_SOC_MDC_V51_LITE
#define MDC_TSNUM 1
#else
#define MDC_TSNUM 2
#endif
// gtest regard 'class' as a tag word in c++.
#ifdef LOG_UNIT_TEST
#define CLASS_UT class_ut
#else
#define CLASS_UT class
#endif

#define LOG_DC_TS_ID 0
#define LOG_MDC_TS_ID 1

/* #pragma pack(1) */
#define LOG_RET_OK            0
#define LOG_RET_ERROR       (-1)
#define LOG_RET_TIMEOUT     (-2)
#define LOG_RET_EOF         (-3)
#define LOG_RET_THRESHOLD   (-4)
#define LOG_RET_NOT_SUPPORT (-5)
#define LOG_RET_NOT_READY   (-6)

enum log_user_msg_type {
    LOG_USER_MSG_TEST = 0,
    LOG_USER_MSG_WRITE_SLOG = 1,
    LOG_USER_MSG_SET_LEVEL = 2,
    LOG_USER_MSG_GET_CHANNEL_TYPE = 7,
    LOG_USER_MSG_GET_DEVICE_ID = 8,
    LOG_USER_MSG_GET_DATA = 9,
    LOG_USER_MSG_MAX
};

enum log_channel_msg_type {
    LOG_CHANNEL_MSG_TEST = 0,
    LOG_CHANNEL_MSG_CREATE = 1,
    LOG_CHANNEL_MSG_DELETE = 2,
    LOG_CHANNEL_MSG_SET = 3,
    // reserved LOG_CHANNEL_MSG_SHOW = 4,
    LOG_CHANNEL_MSG_READ = 5,
    // reserved LOG_CHANNEL_MSG_POLL = 6,
    LOG_CHANNEL_MSG_SYNC_READ_PTR = 7,
    LOG_CHANNEL_MSG_SYNC_WRITE_PTR = 8,
    LOG_CHANNEL_MSG_MAX
};

enum log_channel_type {
    LOG_CHANNEL_TYPE_TS = 0,
    LOG_CHANNEL_TYPE_TS_DUMP = 1,

    LOG_CHANNEL_TYPE_BIOS = 29,
    LOG_CHANNEL_TYPE_LPM3 = 30,
    LOG_CHANNEL_TYPE_IMP = 31,
    LOG_CHANNEL_TYPE_IMU = 32,

    LOG_CHANNEL_TYPE_ISP = 33,

    LOG_CHANNEL_TYPE_SIS = 37,
    LOG_CHANNEL_TYPE_HSM = 38,
    LOG_CHANNEL_TYPE_SIS_BIST = 39, // safe island log for bist
    LOG_CHANNEL_TYPE_BIOS_ATF = 40, // bios atf log

    /*
     * (LOG_CHANNEL_NUM_MAX - 10)~(LOG_CHANNEL_NUM_MAX) reserve for start channel
     * toolchain will never perception start channels
     */
    LOG_CHANNEL_TYPE_TS0_START = LOG_CHANNEL_NUM_MAX - 7,
    LOG_CHANNEL_TYPE_TS1_START = LOG_CHANNEL_NUM_MAX - 6,
    LOG_CHANNEL_TYPE_IMU_START = LOG_CHANNEL_NUM_MAX - 5,
    LOG_CHANNEL_TYPE_UEFI_START = LOG_CHANNEL_NUM_MAX - 4,
    LOG_CHANNEL_TYPE_MAX
};

#define LOG_CHANNEL_TYPE_TS0 LOG_CHANNEL_TYPE_TS
#define LOG_CHANNEL_TYPE_TS1 (enum log_channel_type)(LOG_CHANNEL_TYPE_TS + 2)
#define LOG_CHANNEL_TYPE_TS0_DUMP LOG_CHANNEL_TYPE_TS_DUMP
#define LOG_CHANNEL_TYPE_TS1_DUMP (enum log_channel_type)(LOG_CHANNEL_TYPE_TS_DUMP + 2)
#define LOG_CHANNEL_TYPE_ISP0 LOG_CHANNEL_TYPE_ISP
#define LOG_CHANNEL_TYPE_ISP1 (enum log_channel_type)(LOG_CHANNEL_TYPE_ISP + 1)
#define LOG_CHANNEL_TYPE_ISP2 (enum log_channel_type)(LOG_CHANNEL_TYPE_ISP + 2)
#define LOG_CHANNEL_TYPE_ISP3 (enum log_channel_type)(LOG_CHANNEL_TYPE_ISP + 3)

#define LOG_CHANNEL_LEVEL_DEBUG 0
#define LOG_CHANNEL_LEVEL_INFO 1
#define LOG_CHANNEL_LEVEL_MAX 5

#define LOG_CHANNEL_CONN_SQCQ     0
#define LOG_CHANNEL_CONN_IPC      1
#define LOG_CHANNEL_CONN_SHAREMEM 2
#define LOG_CHANNEL_CONN_MAX      3

#define LOG_MSG_TYPE_DLOG 0
#define LOG_MSG_TYPE_SLOG 1
#define LOG_MSG_TYPE_MAX  2

#define LOG_TYPE_START 0
#define LOG_TYPE_RUN   1
#define LOG_TYPE_MAX   2

enum mailbox_cmd_type {
    CALC_CQSQ_CREATE = 0x1,
    CALC_CQSQ_RELEASE,
    LOG_CQSQ_CREATE,
    LOG_CQSQ_RELEASE,
    DBG_CQSQ_CREATE,
    DBG_CQSQ_RELEASE,
    PROFILE_CQSQ_CREATE,
    PROFILE_CQSQ_RELEASE
};

#define LOG_SEND_TS_CMD_TYPE_NUM 2
#define LOG_TS_CMD_TYPE 0
#define LOG_TS_DUMP_CMD_TYPE 1
struct log_cqsq_info {
    u32 sq_0_index;
    u32 cq_0_index;
    u32 cq_1_index;

    struct semaphore cq0_wait_sema[LOG_SEND_TS_CMD_TYPE_NUM];
    struct mutex cq0_mutex;
    struct mutex cq1_mutex;
};

#define LOG_MAX_TS_NUM 2
struct log_device_info {
    u32 cmd_verify;
    u32 device_state[LOG_MAX_TS_NUM];
    s32 device_id;
    struct log_cqsq_info cqsq_info[LOG_MAX_TS_NUM];
};

struct log_ioctl_para {
    s32 device_id;
    s32 channel_type;
    const s8 *slog_buf;
    s8 *dest_buf;
    s32 buf_size;
    s32 write_size;
    s32 timeout;
    s32 log_level;
    s32 compress;
    s32 byte_per_sec;
    s32 device_id_set[LOG_DEVICE_ID_MAX];
    s32 device_id_num;
    s32 channel_type_set[LOG_CHANNEL_NUM_MAX];
    s32 channel_type_num;
    u32 tsnum;
    s32 ret; /* return value of command */
};

struct log_channel_info {
    s32 channel_type;
    s32 channel_conn;
    s32 channel_state;
    u32 buf_size;
    uintptr_t phy_addr;
    u8 *vir_addr;
    u8 *vir_addr_kmalloc;
    s32 log_level;
    s32 log_type;
    s32 ret_val;
    s32 print_num;
    const struct log_channel_desc *log_desc;
    struct vm_struct *area;
    struct mutex cmd_mutex;
    s32 channel_id;
};


#define LOG_CHANNEL_BUFF_HEAD_REV1 14
#define LOG_CHANNEL_BUFF_HEAD_REV2 14
struct log_channel_buf_head {
    volatile u32 buf_read;
    volatile u32 buf_len;      /* the malloc buffer length, and include head structure */
    volatile u32 rev[LOG_CHANNEL_BUFF_HEAD_REV1];
    volatile u32 buf_write;
    volatile u32 log_level;    /* add for IMU/IMP */
    volatile u32 rev2[LOG_CHANNEL_BUFF_HEAD_REV2];
};

// Please carefully modify this structure as it will be sent to the slog module for parsing
struct log_msg_head {
    u16 version_value;
    u16 data_compressed : 1;
    u16 frame_begin : 1;
    u16 frame_end : 1;
    u16 smp_flag : 1;
    u16 slog_flag : 1;
    u16 reserved_bits : 11;
    s32 reserved;
    s32 data_len;
    s8 data[0];
};

struct log_msg_buf {
    s32 msg_type;
    s8 *buf_addr;
    s32 buf_size;
    s8 *head_addr;
    s32 head_size;
    s8 *data_addr;
    s32 data_size;
    s32 data_threshold;
};

struct log_compress_desc {
    s8 *alg_name;
    s32 type;
    s32 mask;
    struct crypto_comp *tfm;
};

#define LOG_SESSION_LOCAL  0xca /* by ioctl */
#define LOG_SID_LEN        32


typedef long (*log_server_create)(void);
typedef long (*log_server_destroy)(void);
typedef long (*log_accept)(s32 dev_id, s32 *session);
typedef long (*log_close)(void);
typedef long (*log_send_timeout)(s32 session, void *buf, s32 len, s32 timeout);
typedef s32 (*log_get_segment)(void);
typedef long (*log_get_run_env)(s32 *run_env);
typedef long (*log_epoll_alloc_fd)(void);
typedef long (*log_epoll_free_fd)(void);
typedef long (*log_epoll_ctl)(void);
typedef long (*log_epoll_wait)(void);

struct log_session_ops {
    log_server_create sess_create_func;
    log_server_destroy sess_destroy_func;
    log_accept sess_accept_func;
    log_close sess_close_func;
    log_send_timeout sess_send_timeout_func;
    log_get_segment sess_get_segment_func;
    log_get_run_env sess_get_run_env_func;
    log_epoll_alloc_fd sess_epoll_alloc_fd;
    log_epoll_free_fd sess_epoll_free_fd;
    log_epoll_ctl sess_epoll_ctl;
    log_epoll_wait sess_epoll_wait;
};

struct session_ioctl_info {
    struct semaphore accept_sem;

    wait_queue_head_t wq;

    void *log_buf;
    u32 buf_size;
    atomic_t status; /* 0:idle or read finish; 1:readable; 2: in reading */

    struct mutex sess_mutex;
};

struct log_hdc_session_desc {
    s32 service_type;
    s32 session;
    s8 session_id[LOG_SID_LEN];
    s32 segment_size;
    s32 created;
    s32 connected;
    s32 host_closed;
    struct mutex ops_mutex;

    struct log_session_ops *ops;
    struct session_ioctl_info sess_info;
};

enum log_session_status {
    LOG_SESSION_STATUS_INIT = 0, /* idle or read finish */
    LOG_SESSION_STATUS_READABLE,
    LOG_SESSION_STATUS_READING
};

enum log_device_status {
    LOG_DEVICE_STATUS_INIT = 0,
    LOG_DEVICE_STATUS_WAIT_HOST,
    LOG_DEVICE_STATUS_WORK,
    LOG_DEVICE_STATUS_MAX
};

enum log_session_run_env {
    LOG_SESSION_RUN_ENV_UNKNOW = 0,
    LOG_SESSION_RUN_ENV_HOST,
    LOG_SESSION_RUN_ENV_CONTAINER
};

typedef enum log_create_channels {
    LOG_CREATE_TS_CHANNELS = 0,
    LOG_CREATE_NORMAL_CHANNELS,
    LOG_CREATE_ALL_CHANNELS,
    LOG_CREATE_CHANNELS_MAX
}log_create_channels;

struct log_generate_rate {
    struct timer_list rate_timer;
    s32 timer_interval;
    atomic_t total_volume;
    atomic_t byte_per_sec;
};

struct log_device_ctx {
    struct log_device_info device_info;
    struct log_channel_info *channels_info;
    struct log_compress_desc compress_desc;
    struct log_msg_buf channel_log_buf;
    struct log_msg_buf channel_compress_buf;
    struct log_hdc_session_desc hdc_desc;
    struct mutex channel_buf_mutex;
    struct log_generate_rate generate_rate;
    atomic_t status;
    atomic_t compress_config;
    wait_queue_head_t wq_slog;
    wait_queue_head_t wq_channel_log;
};

struct log_channel_desc {
    s32 channel_type;
    s32 channel_conn;
    s32 channel_ids;
    s32 channel_ids_num;
    s32 log_level_support;
    uintptr_t buf_phy_addr;
    s32 buf_size;
    s32 channel_state;
    s32 log_level;
    s32 log_type;
    s32 reserve;  // add  tsid for TS

    s32 (*create)(struct log_device_ctx *, struct log_channel_info *);
    s32 (*del)(struct log_device_ctx *, struct log_channel_info *);
    s32 (*set_level)(struct log_device_ctx *, struct log_channel_info *, s32);
    s32 (*init)(struct log_device_ctx *, struct log_channel_info *);
};

struct log_hdc_epoll {
    s32 valid;
    s32 epfd;
    s32 magic_num;
};

struct log_drv_desc {
    // define kernel driver necessary variable
    struct CLASS_UT *drv_class;
    struct cdev drv_cdev;
    dev_t drv_devno;
    struct device *drv_device;

    s32 suspend; /* low power: 0-normal,LOG_SUSPEND-suspend */
    void *suspend_handle;

    // define log variable
    s32 device_num;
    s32 device_ids[LOG_DEVICE_ID_MAX];
    s32 cpu_ids[LOG_DEVICE_ID_MAX];
    u32 ts_num[LOG_DEVICE_ID_MAX];
    struct log_device_ctx *device_ctx[LOG_DEVICE_ID_MAX];
    struct task_struct *task_log_work[LOG_DEVICE_ID_MAX];
#ifdef CFG_SOC_PLATFORM_MDC_V51
    struct task_struct *task_xpc_work[LOG_DEVICE_ID_MAX];
    struct task_struct *task_ts_work[LOG_DEVICE_ID_MAX];
#endif
    struct task_struct *hdc_epoll_task;
    struct log_hdc_epoll hdc_epolls;

    s32 session_type;
    struct log_session_ops ops;
};

s32 log_send_set_level_cmd(struct log_device_ctx *device_ctx, s32 channel_type,
    struct log_channel_info *channel_info, struct log_ioctl_para *log_para);
extern bool tsdrv_is_ts_work(u32 devid, u32 tsid);

#endif /* _LOG_DRV_DEV_ */
