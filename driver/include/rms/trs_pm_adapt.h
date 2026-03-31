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

#ifndef TRS_PM_ADAPT_H
#define TRS_PM_ADAPT_H

#include "trs_pub_def.h"
#include "trs_chan.h"
#include "trs_core.h"

#define TRS_CHAN_SQ_MEM_OFFSET      0U
#define TRS_CHAN_CQ_MEM_OFFSET      1U

#define TRS_CHAN_DEV_MEM         0U
#define TRS_CHAN_HOST_MEM        1U
#define TRS_CHAN_HOST_PHY_MEM    2U

int trs_get_host_irq_group(struct trs_id_inst *inst, u32 group[], u32 group_num, u32 *valid_group_num);
int trs_device_get_ssid(struct trs_id_inst *inst, int *ssid);

/* mailbox */
struct trs_chan_adapt_info {
    u8 app_type;
    u8 sq_side;
    u8 cq_side;
    u8 fid;
};

int trs_chan_mbox_send(struct trs_id_inst *inst, struct trs_chan_info *chan_info, struct trs_chan_adapt_info *adapt);

int trs_device_get_id_range(struct trs_id_inst *inst, int type, u32 *start, u32 *end);
int trs_device_get_id_total_num(struct trs_id_inst *inst, int type, u32 *total_num);
int trs_device_get_id_split(struct trs_id_inst *inst, int type, u32 *split);

/* chan ops */
void trs_chan_update_ssid(struct trs_id_inst *inst, struct trs_chan_info *chan_info);
void *trs_chan_ops_sq_mem_alloc(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_sq_para *sq_para, u64 *phy_addr, u32 *attr);
void trs_chan_ops_sq_mem_free(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_sq_para *sq_para, void *sq_addr, u64 phy_addr);
void *trs_chan_ops_cq_mem_alloc(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_cq_para *cq_para, u64 *phy_addr, u32 *attr);
void trs_chan_ops_cq_mem_free(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_cq_para *cq_para, void *cq_addr, u64 phy_addr);
void trs_chan_ops_flush_sqe_cache(struct trs_id_inst *inst,
    struct trs_chan_type *types, void *addr, u64 pa, u32 len);
void trs_chan_ops_invalid_cqe_cache(struct trs_id_inst *inst,
    struct trs_chan_type *types, void *addr, u64 pa, u32 len);
int trs_chan_get_irq(struct trs_id_inst *inst, u32 irq_type, u32 irq [], u32 irq_num, u32 *valid_irq_num);
int trs_chan_ops_request_irq(struct trs_id_inst *inst, u32 irq_type, int irq_index, void *para,
    void(*handler)(int irq_type, int irq_index, void *para, u32 cqid [], u32 cq_num));
int trs_chan_free_irq(struct trs_id_inst *inst, int irq_type, int irq_index, void *para);
int trs_chan_ops_sqe_update(struct trs_id_inst *inst, int pid, void *sqe);
int trs_chan_ops_cqe_update(struct trs_id_inst *inst, int pid, void *cqe);

int trs_chan_ops_get_valid_cq_list(struct trs_id_inst *inst, u32 group, u32 cqid[], u32 cq_id_num, u32 *valid_cq_num);
void trs_chan_ops_intr_mask_config(struct trs_id_inst *inst, u32 group, u32 irq, int val);
void trs_chan_ops_get_sq_head_in_cqe(struct trs_id_inst *inst, void *cqe, u32 *sq_head);
bool trs_chan_ops_cqe_is_valid(struct trs_id_inst *inst, void *cqe, u32 loop);
int trs_chan_ops_ctrl_sqcq(struct trs_id_inst *inst, struct trs_chan_type *types, u32 id, u32 cmd, u32 para);
int trs_chan_ops_query_sqcq(struct trs_id_inst *inst, struct trs_chan_type *types, u32 id, u32 cmd, u64 *value);

/* core ops */
int trs_core_ops_get_support_proc_num(struct trs_id_inst *inst, u32 *proc_num);
int trs_core_ops_set_sq_status(struct trs_id_inst *inst, u32 sqid, u32 status);
int trs_core_ops_get_sq_id_head_from_hw_cqe(struct trs_id_inst *inst, void *hw_cqe, u32 *sqid, u32 *sq_head);
int trs_core_ops_get_stream_from_cqe(struct trs_id_inst *inst, void *hw_cqe, u32 *stream_id);
int trs_core_ops_cqe_to_logic_cqe(struct trs_id_inst *inst, void *hw_cqe, struct trs_logic_cqe *logic_cqe);
int trs_core_ops_get_res_reg_offset(struct trs_id_inst *inst, int type, u32 id, u32 *offset);
int trs_core_ops_get_res_reg_total_size(struct trs_id_inst *inst, int type, u32 *total_size);
int trs_core_ops_res_id_query(struct trs_id_inst *inst, u32 type, u32 id, u32 cmd, u64 *value);

int trs_stars_res_id_ctrl(struct trs_id_inst *inst, u32 id_type, u32 id, u32 cmd);

int trs_core_ops_sqcq_reg_map(struct trs_id_inst *inst, struct trs_sqcq_reg_map_para *para);
int trs_core_ops_sqcq_reg_unmap(struct trs_id_inst *inst, struct trs_sqcq_reg_map_para *para);

int trs_core_ops_get_ts_inst_status(struct trs_id_inst *inst, u32 *status);
bool trs_is_support_uio_d(void);

struct trs_adapt_notice_ops {
    int (*set_ts_status)(u32 devid, u32 tsid, u32 status);
    int (*abnormal_proc)(u32 devid, u32 tsid, void* data);
    int (*sync_id_proc)(u32 devid, u32 tsid, int type);
};
struct trs_adapt_notice_ops *trs_adapt_get_notice_ops(void);
void trs_adapt_notice_ops_register(struct trs_adapt_notice_ops *register_ops);
void trs_adapt_notice_ops_unregister(void);

#endif

