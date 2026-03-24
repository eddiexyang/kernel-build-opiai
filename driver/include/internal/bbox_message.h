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

#ifndef BBOX_MESSAGE_H
#define BBOX_MESSAGE_H

#include <linux/list.h>
#include "device/bbox_types.h"
#include "bbox_communication.h"
#include "common/bbox_common.h"
#include "rdr/rdr_exception_core.h"

#define MAX_MESSAGE_SEQ  0xFFFF

enum bbox_dataset_type {
    BBOX_DATASET_FULL       = 1, // full set, DATA_MSG_T_FULL
    BBOX_DATASET_MIN        = 2, // mini set, DATA_MSG_T_MIN
    BBOX_DATASET_DDR        = 3, // ddr dump, DATA_MSG_T_DDR
    BBOX_DATASET_KLOG       = 4, // kernel log, DATA_MSG_T_KLOG
    BBOX_DATASET_NODDR      = 5, // no ddr data, DATA_MSG_T_NODDR
    BBOX_DATASET_DMA        = 6, // dma dump, DATA_MSG_T_DMA
    BBOX_DATASET_PMU        = 7, // DATA_MSG_T_PMU
    BBOX_DATASET_SRAM       = 8, // DATA_MSG_T_SRAM
    BBOX_DATASET_OOM        = 9, // DATA_MSG_T_OOM
    BBOX_DATASET_LOG        = 10, // DATA_MSG_T_LOG
    BBOX_DATASET_HDR        = 11, // DATA_MSG_T_HDR
    BBOX_DATASET_TSENSOR    = 12, // DATA_MSG_T_TSENSOR
    BBOX_DATASET_CDR        = 13, // DATA_MSG_T_CDR
    BBOX_DATASET_RECOVER    = 14, // DATA_MSG_T_RECOVER
    BBOX_DATASET_MAX
};

// enum type in struct bbox_msg_header
enum bbox_msg_type {
    BBOX_MSG_HELLO = 0x1,
    BBOX_MSG_EXCEPTION = 0x10,      // device exception msg 16 ~ 63
    BBOX_MSG_OOM,
    BBOX_MSG_EXCEPTION_MAX = 0x3F,
    BBOX_MSG_REBOOT = 0x40,
    BBOX_MSG_NOTIFY_MAX = 0x4F,     // device notify msg 64 ~ 79
    BBOX_MSG_ACK = 0x50,            // reply msg 80 ~ 95
    BBOX_MSG_NAK,
    BBOX_MSG_RPL_MAX = 0x5F,
    BBOX_MSG_MAX = 0xFF
};

struct bbox_msg_header {
    u32 type;   // message type
    u32 len;    // message length
};

struct bbox_reboot_msg {
    struct bbox_msg_header header;
    u32 reboot_type;
    u32 devmem_len;
    u64 devmem_addr;
};

struct bbox_hello_msg {
    struct bbox_msg_header header;
    u32 capacity;
    u32 flag;       // upward or downward flag
    u32 sequence;
    u32 reserve;
};

struct bbox_exception_msg {
    struct bbox_msg_header header;
    struct bbox_time tm; // time stamp
    u32 excepid;    // exception id
    u32 dtype;      // data set type
    u32 dlen;       // the length of data below
    u8 coreid;      // module id
    u8 etype;       // exception reason
    u16 pad;        // padding
    u8 data[0];     // data
};

struct bbox_reply_msg {
    struct bbox_msg_header header;
    u64 reserve;
};

#define MSG_NODE_TYPE_DYNAMIC   0
#define MSG_NODE_TYPE_STATIC    1

typedef void (*node_free_notify_ptr)(void *arg);

struct bbox_msg_node {
    struct list_head list;
    struct bbox_msg_header *msg;
    u32 devid;
    u32 excepid;
    u8 coreid;
    u8 node_type;
    node_free_notify_ptr notify;
    void *arg;  // dynamic application
};

struct rdr_exception_msg_info {
    u8 coreid;
    u8 etype;
    u32 excepid;
    u32 devid;
    struct bbox_time tm;
    struct bbox_time tv;
};

static inline void bbox_set_msg_info(struct rdr_exception_msg_info *info, u8 coreid, u8 etype, u32 excepid, u32 devid)
{
    info->coreid  = coreid;
    info->etype   = etype;
    info->excepid = excepid;
    info->devid   = devid;
}

static inline void bbox_set_msg_time(struct rdr_exception_msg_info *info, const struct bbox_time *tm)
{
    info->tm.tv_sec  = tm->tv_sec;
    info->tm.tv_nsec = tm->tv_nsec;
#ifdef CFG_FEATURE_MDC_CLOCK_CONFIG
    info->tv.tv_sec  = tm->tv_sec;
    info->tv.tv_nsec = tm->tv_nsec;
#endif
}


#define RDR_SET_SUBMIT_INFO(_param, _devid, _excepid, _coreid, _etype, _time) do { \
    (_param).coreid  = (u8)(_coreid);            \
    (_param).etype   = (u8)(_etype);             \
    (_param).excepid = (u32)(_excepid);          \
    (_param).devid   = (u32)(_devid);            \
    (_param).tm.tv_sec  = (_time).tv_sec;  \
    (_param).tm.tv_nsec = (_time).tv_nsec; \
} while (0)

#define RDR_SET_SUBMIT_INFO_EX1(_param, _excepid, _coreid, _etype, _time) do { \
    (_param).coreid  = (_coreid);            \
    (_param).etype   = (_etype);             \
    (_param).excepid = (_excepid);           \
    (_param).devid   = DEFAULT_DEVICE_ID;    \
    (_param).tm.tv_sec  = (_time).tv_sec;  \
    (_param).tm.tv_nsec = (_time).tv_nsec; \
} while (0)

#define RDR_SET_SUBMIT_INFO_EX2(_param, _excepid, _coreid, _time) do { \
    (_param).coreid  = (_coreid);                         \
    (_param).etype   = rdr_exception_get_etype(_excepid); \
    (_param).excepid = (_excepid);                        \
    (_param).devid   = DEFAULT_DEVICE_ID;                 \
    (_param).tm.tv_sec  = (_time).tv_sec;               \
    (_param).tm.tv_nsec = (_time).tv_nsec;              \
} while (0)

#define RDR_SET_SUBMIT_INFO_EX3(_param, _devid, _excepid, _coreid, _time, _vtime) do { \
    (_param).coreid  = (_coreid);                         \
    (_param).etype   = rdr_exception_get_etype(_excepid); \
    (_param).excepid = (_excepid);                        \
    (_param).devid   = (_devid);                          \
    (_param).tm.tv_sec  = (_time).tv_sec;               \
    (_param).tm.tv_nsec = (_time).tv_nsec;              \
    (_param).tv.tv_sec  = (_vtime).tv_sec;               \
    (_param).tv.tv_nsec = (_vtime).tv_nsec;              \
} while (0)

s32 bbox_submit_reboot_message(u32 devid, u8 reboot_type, u64 addr, u32 len);
s32 bbox_submit_pmu_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len);
s32 bbox_submit_tsensor_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len);
s32 bbox_submit_sram_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len);
s32 bbox_submit_klog_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len);
s32 bbox_submit_log_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len);
s32 bbox_submit_ddr_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len);
s32 bbox_submit_module_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len);
s32 bbox_submit_module_reset_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len);
s32 bbox_submit_hdr_message(const struct rdr_exception_msg_info *info, const node_free_notify_ptr notify,
                            const char *data, u32 len);
s32 bbox_submit_cdr_message(const struct rdr_exception_msg_info *info, const node_free_notify_ptr notify,
                            const char *data, u32 len);

void bbox_message_set_time_seq(struct channel_info *channel, struct bbox_msg_header *hdr, u64 *tv_nsec);
void bbox_submit_message(struct bbox_msg_node *node);

#endif
