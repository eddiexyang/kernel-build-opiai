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
* Create: 2022-7-15
*/

#ifndef TRS_CORE_H
#define TRS_CORE_H

#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/module.h>

#include "trs_pub_def.h"
#include "trs_chan_kernel_interface.h"
#include "trs_res_id_def.h"
#include "trs_chan.h"

#define TRS_INST_ALL_FEATUR_MODE 0
#define TRS_INST_PART_FEATUR_MODE 1

/* res id ctrl cmd */
enum {
    TRS_RES_OP_RESET = 0,
    TRS_RES_OP_RECORD,
    TRS_RES_OP_ENABLE,
    TRS_RES_OP_DISABLE,
    TRS_RES_OP_CHECK_AND_RESET,
    TRS_RES_OP_MAX,
};

/* res id query cmd */
enum {
    TRS_RES_QUERY_ADDR = 0,
    TRS_RES_QUERY_MAX,
};

/* stars scenario define soft logic cqe */
struct trs_logic_cqe {
    u16 stream_id;
    u16 task_id;
    u32 errorCode;
    u8 errorType;
    u8 sqeType;
    u16 sqId;
    u16 sqHead;
    u16 match_flag : 1;
    u16 drop_flag : 1;
    u16 error_bit : 1;
    u16 acc_error : 1;
    u16 reserved0 : 12;
    union {
        u64 timeStamp;
        u16 sqe_index;
    };
    /* Union description:
     *  Internal: enque_timestamp temporarily used as dfx
     *  External: reserved1
     */
    union {
        u64 enque_timestamp;
        u64 reserved1;
    };
};

struct trs_sqcq_reg_map_para {
    u32 stream_id;
    u32 sqid;
    u32 cqid;
    pid_t host_pid;
};

struct trs_core_adapt_ops {
    struct module *owner;
    int (*proc_bind_smmu)(struct trs_id_inst *inst);
    void (*proc_unbind_smmu)(struct trs_id_inst *inst);
    int (*ssid_query)(struct trs_id_inst *inst, int *ssid);
    int (*res_id_check)(struct trs_id_inst *inst, int type, u32 id);
    int (*get_res_surport_proc_num)(struct trs_id_inst *inst, u32 *proc_num); /* not must */
    int (*id_alloc[TRS_CORE_MAX_ID_TYPE])(struct trs_id_inst *inst, u32 para, u32 *id); /* not must */
    int (*id_free[TRS_CORE_MAX_ID_TYPE])(struct trs_id_inst *inst, u32 id); /* not must */
    int (*get_sq_id_head_from_hw_cqe)(struct trs_id_inst *inst, void *hw_cqe, u32 *sqid, u32 *sq_head); /* not must */
    int (*get_stream_id_from_hw_cqe)(struct trs_id_inst *inst, void *hw_cqe, u32 *stream_id); /* not must */
    int (*hw_cqe_to_logic_cqe)(struct trs_id_inst *inst, void *hw_cqe, struct trs_logic_cqe *logic_cqe); /* not must */
    int (*notice_ts)(struct trs_id_inst *inst, u8 *msg, u32 len);
    int (*sqcq_reg_map)(struct trs_id_inst *inst, struct trs_sqcq_reg_map_para *para);
    int (*sqcq_reg_unmap)(struct trs_id_inst *inst, struct trs_sqcq_reg_map_para *para);
    int (*res_id_ctrl)(struct trs_id_inst *inst, u32 type, u32 id, u32 cmd);
    int (*res_id_query)(struct trs_id_inst *inst, u32 type, u32 id, u32 cmd, u64 *value);
    int (*get_res_reg_offset)(struct trs_id_inst *inst, int type, u32 id, u32 *offset);
    int (*get_res_reg_total_size)(struct trs_id_inst *inst, int type, u32 *total_size);
    int (*get_sq_trigger_irq)(struct trs_id_inst *inst, u32 *irq); /* not must */
    int (*get_trigger_sqid)(struct trs_id_inst *inst, u32 *sqid); /* not must */
    void (*set_trigger_irq_affinity)(struct trs_id_inst *inst, u32 irq, u32 op); /* not must */
    void (*set_thread_affinity)(struct trs_id_inst *inst, struct task_struct *thread);
    /* not must (tscpu phy device set) */
    int (*get_thread_bind_irq)(struct trs_id_inst *inst, u32 irq[], u32 irq_num, u32 *valid_irq_num);
    int (*get_ts_inst_status)(struct trs_id_inst *inst, u32 *status);
    void (*trace_sqe_fill)(struct trs_id_inst *inst, struct trs_chan_sq_trace *sq_trace, void *sqe);
    void (*trace_cqe_fill)(struct trs_id_inst *inst, struct trs_chan_cq_trace *cq_trace, void *cqe);
};

int trs_core_ts_inst_register(struct trs_id_inst *inst, int hw_type, struct trs_core_adapt_ops *ops);
void trs_core_ts_inst_unregister(struct trs_id_inst *inst);

int trs_stream_bind_remote_sqcq(struct trs_id_inst *inst, u32 stream_id, u32 sqid, u32 cqid, int host_pid);
int trs_set_sq_reg_vaddr(struct trs_id_inst *inst, u32 sqid, u64 va, size_t size);
int trs_get_sq_reg_vaddr(struct trs_id_inst *inst, u32 sqid, u64 *va, size_t *size);

int trs_res_id_get(struct trs_id_inst *inst, int res_type, u32 res_id);
int trs_res_id_put(struct trs_id_inst *inst, int res_type, u32 res_id);

bool trs_res_is_belong_to_proc(struct trs_id_inst *inst, int pid, int res_type, u32 res_id);

typedef bool (* trs_res_share_by_proc_ops_t)(struct trs_id_inst *inst, int pid, int res_type, u32 res_id);
void trs_res_share_proc_ops_register(int res_type, trs_res_share_by_proc_ops_t func);
int trs_set_ts_inst_feature_mode(struct trs_id_inst *inst, u32 mode, u32 force);

int trs_core_get_ssid(struct trs_id_inst *inst, int pid, u32 *passid);
int trs_res_id_check(struct trs_id_inst *inst, int res_type, u32 res_id);

#endif

