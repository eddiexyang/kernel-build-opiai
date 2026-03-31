/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-7-15
*/

#ifndef TRS_CHAN_H
#define TRS_CHAN_H

#include <linux/interrupt.h>
#include <linux/module.h>

#include "trs_pub_def.h"
#include "trs_chan_kernel_interface.h"
#include "trs_abnormal_info.h"

#define CHAN_MEM_FLAG_LOCAL 0   /* output from sqcq mem alloc */

#define SQE_CACHE_SIZE 256
#define SQE_ALIGN_SIZE 8
#define CQE_CACHE_SIZE 128
#define CQE_ALIGN_SIZE 4

static const char *trs_chan_type_name[CHAN_TYPE_MAX][CHAN_SUB_TYPE_MAX] = {
    {"rts", "esched", "dvpp", "ts", "rsv_ts"},
    {"logic", "shm", "unknown", "unknown", "unknown"},
    {"log", "prof", "hb", "dbg", "unknown"},
    {"cb", "scb", "unknown", "unknown", "unknown"},
};

static inline const char *trs_chan_type_to_name(struct trs_chan_type *types)
{
    if ((types->type < CHAN_TYPE_MAX) && (types->sub_type < CHAN_SUB_TYPE_MAX)) {
        return trs_chan_type_name[types->type][types->sub_type];
    }
    return "unknown";
}

int trs_chan_send(struct trs_id_inst *inst, int chan_id, struct trs_chan_send_para *para);
int trs_chan_recv(struct trs_id_inst *inst, int chan_id, struct trs_chan_recv_para *para);

#define CHAN_CTRL_CMD_SQ_HEAD_UPDATE 0 /* para: sq head, update sw sq head */
#define CHAN_CTRL_CMD_SQ_HEAD_SET 1 /* para: sq head, set hw sq head */
#define CHAN_CTRL_CMD_SQ_STATUS_SET 2 /* para: 0: disable, 1: enable */
#define CHAN_CTRL_CMD_SQ_DISABLE_TO_ENABLE 3 /* para: timeout */
#define CHAN_CTRL_CMD_CQ_SCHED 4 /* no para */
#define CHAN_CTRL_CMD_NOT_NOTICE_TS 5 /* no para */
int trs_chan_ctrl(struct trs_id_inst *inst, int chan_id, u32 cmd, u32 para);

#define CHAN_QUERY_CMD_SQ_STATUS 0 /* value: sq status 0: disable, 1: enable */
#define CHAN_QUERY_CMD_SQ_HEAD 1 /* value: sq head */
#define CHAN_QUERY_CMD_SQ_TAIL 2 /* value: sq tail */

int trs_chan_query(struct trs_id_inst *inst, int chan_id, u32 cmd, u32 *value);

struct trs_chan_sq_info {
    u32 sqid;
    u64 sq_phy_addr;
    u64 head_addr;
    u64 tail_addr;
    u64 db_addr;
    struct trs_chan_sq_para sq_para;
};

struct trs_chan_cq_info {
    u32 cqid;
    int irq;
    u64 cq_phy_addr;
    struct trs_chan_cq_para cq_para;
};

int trs_chan_get_chan_id(struct trs_id_inst *inst, int res_type, u32 res_id, int *chan_id);
int trs_chan_get_sq_info(struct trs_id_inst *inst, int chan_id, struct trs_chan_sq_info *info);
int trs_chan_get_cq_info(struct trs_id_inst *inst, int chan_id, struct trs_chan_cq_info *info);
int trs_chan_to_string(struct trs_id_inst *inst, int chan_id, char *buff, u32 buff_len);

struct trs_chan_info {
    u32 op; /* 1 add, 0 del */
    u16 ssid;
    u8 master_pid_flag;
    pid_t pid;
    int irq_type;
    struct trs_chan_type types;
    struct trs_chan_sq_info sq_info;
    struct trs_chan_cq_info cq_info;
    u32 msg[SQCQ_INFO_LENGTH]; /* send to ts */
};

#define CTRL_CMD_SQ_HEAD_UPDATE 0 /* para: sq head */
#define CTRL_CMD_SQ_TAIL_UPDATE 1 /* para: sq tail */
#define CTRL_CMD_CQ_HEAD_UPDATE 2 /* para: cq head */
#define CTRL_CMD_SQ_STATUS_SET 3 /* para: 0: disable, 1: enable */
#define CTRL_CMD_SQ_DISABLE_TO_ENABLE 4 /* para: timeout */
#define CTRL_CMD_MAX 5

#define QUERY_CMD_SQ_HEAD 0 /* value: head */
#define QUERY_CMD_SQ_TAIL 1 /* value: tail */
#define QUERY_CMD_CQ_HEAD 2 /* value: head */
#define QUERY_CMD_CQ_TAIL 3 /* value: tail */
#define QUERY_CMD_SQ_STATUS 4 /* value: sq status */
#define QUERY_CMD_SQ_HEAD_PADDR 5 /* value: head head phy addr, for uio */
#define QUERY_CMD_SQ_TAIL_PADDR 6 /* value: head tail phy addr, for uio */
#define QUERY_CMD_SQ_DB_PADDR 7 /* value: sq db phy addr, for uio */
#define QUERY_CMD_MAX 8

struct trs_chan_adapt_ops {
    struct module *owner;
    void* (*sq_mem_alloc)(struct trs_id_inst *inst, struct trs_chan_type *types, struct trs_chan_sq_para *sq_para,
        u64 *phy_addr, u32 *attr);
    void (*sq_mem_free)(struct trs_id_inst *inst, struct trs_chan_type *types, struct trs_chan_sq_para *sq_para,
        void *sq_addr, u64 phy_addr);
    void* (*cq_mem_alloc)(struct trs_id_inst *inst, struct trs_chan_type *types, struct trs_chan_cq_para *cq_para,
        u64 *phy_addr, u32 *attr);
    void (*cq_mem_free)(struct trs_id_inst *inst, struct trs_chan_type *types, struct trs_chan_cq_para *cq_para,
        void *cq_addr, u64 phy_addr);

    void (*flush_cache)(struct trs_id_inst *inst, struct trs_chan_type *types, void *addr, u64 pa, u32 len);
    void (*invalid_cache)(struct trs_id_inst *inst, struct trs_chan_type *types, void *addr, u64 pa, u32 len);

    bool (*cqe_is_valid)(struct trs_id_inst *inst, void *cqe, u32 loop);
    void (*get_sq_head_in_cqe)(struct trs_id_inst *inst, void *cqe, u32 *sq_head);
    int (*sqe_update)(struct trs_id_inst *inst, int pid, u32 sqid, u32 sqeid, void *sqe); /* not must */
    int (*cqe_update)(struct trs_id_inst *inst, int pid, void *cqe); /* not must */
    int (*sqcq_ctrl)(struct trs_id_inst *inst, struct trs_chan_type *types, u32 id, u32 cmd, u32 para);
    int (*sqcq_query)(struct trs_id_inst *inst, struct trs_chan_type *types, u32 id, u32 cmd, u64 *value);
    int (*notice_ts)(struct trs_id_inst *inst, struct trs_chan_info *chan_info);

    int (*get_irq)(struct trs_id_inst *inst, u32 irq_type, u32 irq[], u32 irq_num, u32 *valid_irq_num);
    int (*get_cq_affinity_irq)(struct trs_id_inst *inst, u32 cq_id, u32 *irq_index);
    int (*request_irq)(struct trs_id_inst *inst, u32 irq_type, int irq_index,
        void *para, void (*handler)(int irq_type, int irq_index, void *para, u32 cqid[], u32 cq_num));
    int (*free_irq)(struct trs_id_inst *inst, int irq_type, int irq_index, void *para);
};

int trs_chan_ts_inst_register(struct trs_id_inst *inst, int hw_type, struct trs_chan_adapt_ops *ops);
void trs_chan_ts_inst_unregister(struct trs_id_inst *inst);

typedef int (*trs_chan_abnormal_handle)(struct trs_id_inst *inst, u32 sqid, void *sqe, void *info);
int trs_chan_register_abnormal_handle(u32 task_type, trs_chan_abnormal_handle handle);
int trs_chan_unregister_abnormal_handle(u32 task_type);

int trs_chan_abnormal_proc(struct trs_id_inst *inst, struct stars_abnormal_info *abnormal_info);

#endif

