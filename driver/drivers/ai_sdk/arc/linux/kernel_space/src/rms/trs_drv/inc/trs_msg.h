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
* Create: 2022-8-15
*/
#ifndef TRS_MSG_H
#define TRS_MSG_H

#include <linux/types.h>

struct trs_msg_head {
    u32 devid;
    u32 tsid;
    u32 cmdtype;
    u16 valid;  /* validity judgement, 0x5A5A is valide */
    s16 result; /* process result from rp, zero for succ, non zero for fail */
};

#define TRS_MSG_TOTAL_LEN       128
#define TRS_MSG_DATA_LEN        (TRS_MSG_TOTAL_LEN - sizeof(struct trs_msg_head))

#define TRS_MSG_SEND_MAGIC      0x5A5A
#define TRS_MSG_RCV_MAGIC       0xA5A5
#define TRS_MSG_INVALID_RESULT  0x1A

struct trs_msg_data {
    struct trs_msg_head header;
    char payload[TRS_MSG_DATA_LEN];
};

enum trs_msg_cmd_type {
    TRS_MSG_GET_SSID = 1,
    TRS_MSG_PUT_RES_ID,
    TRS_MSG_GET_RES_ID,
    TRS_MSG_GET_RES_CAP,
    TRS_MSG_GET_PHY_ADDR,
    TRS_MSG_GET_CQ_GROUP,
    TRS_MSG_GET_PROC_NUM,
    TRS_MSG_SQ_REG_MAP,
    TRS_MSG_SQ_REG_UNMAP,
    TRS_MSG_CDQM_INIT,
    TRS_MSG_CDQM_CREATE,
    TRS_MSG_CDQM_DESTROY,
    TRS_MSG_CDQM_BATCH_ABNORMAL,
    TRS_MSG_GET_RES_AVAIL_NUM,
    TRS_MSG_CHAN_ABNORMAL,
    TRS_MSG_SET_TS_STATUS,
    TRS_MSG_FLUSH_RES_ID,
    TRS_MSG_RES_ID_CHECK,
    TRS_MSG_MAX
};

struct trs_msg_id_cap {
    int type;
    u32 id_start;
    u32 id_end;
    u32 total_num;
    u32 split;
};

struct trs_msg_id_sync_head {
    int type;
    u16 req_num;    // request id num
    u16 ret_num;    // return id num
};

#define TRS_MSG_ID_SYNC_MAX_NUM ((TRS_MSG_DATA_LEN - sizeof(struct trs_msg_id_sync_head)) / sizeof(u16))
struct trs_msg_id_sync {
    struct trs_msg_id_sync_head head;
    u16 id[TRS_MSG_ID_SYNC_MAX_NUM];
};

struct trs_msg_stream_bind_sqcq {
    u32 stream_id;
};

struct trs_msg_map_stars_sq {
    u64     va;         /* stars va addr for cp proc  */
    size_t  size;
};

struct trs_msg_sqcq_sync {
    u32 sq_id;
    u32 cq_id;
    int remote_tgid;
    struct trs_msg_stream_bind_sqcq bind_sqcq;
    struct trs_msg_map_stars_sq sq_map;
};

struct trs_msg_sync_ssid {
    u32 hpid;
    u32 vfid;
    int ssid;
};

struct trs_msg_res_id_check {
    u32 hpid;
    int id_type;
    u32 res_id;
};

struct trs_msg_proc_num {
    u32 proc_num;
};

struct trs_msg_res_num {
    int type;
    u32 avail_num;
};

#define SYNC_MAX_NAME_LEN 64
struct trs_msg_get_phy_addr {
    u64 addr;
    u32 size;
    char name[SYNC_MAX_NAME_LEN];
};

#define MAX_GROUP_NUM 16
struct trs_msg_cq_group {
    u32 group_num;
    u32 group[MAX_GROUP_NUM];
};

typedef int (* trs_msg_send_func_t)(u32, struct trs_msg_data *);
typedef int (* trs_msg_rdv_func_t)(u32, struct trs_msg_data *);

#endif /* TRS_MSG_H */

