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
 * Create: 2022-5-31
 */

#ifndef __SOC_RES_H
#define __SOC_RES_H

#include <linux/types.h>

#define SOC_RESMNG_MAX_NAME_LEN 32

enum soc_mia_res_type {
    MIA_AC_AIV = 0,
    MIA_AC_AIC,
    MIA_AC_C_CORE,
    MIA_AC_DSA,
    MIA_AC_FFTS,
    MIA_AC_SDMA,
    MIA_AC_PCIE_DMA,

    MIA_STARS_RTSQ,
    MIA_STARS_STREAM,
    MIA_STARS_EVENT,
    MIA_STARS_NOTIFY,
    MIA_STARS_MODEL,
    MIA_STARS_CMO,
    MIA_STARS_CDQ,
    MIA_STARS_ACSQ,
    MIA_STARS_TOPIC_ACPU_SLOT,
    MIA_STARS_TOPIC_CCPU_SLOT,

    MIA_MEM_NUMA,
    MIA_SYS_MEM,

    MIA_CPU_HOST_CCPU,
    MIA_CPU_HOST_ACPU,
    MIA_CPU_DEV_ACPU,

    MIA_DVPP_JPEGD,
    MIA_DVPP_JPEGE,
    MIA_DVPP_VPC,
    MIA_DVPP_VDEC,
    MIA_DVPP_PNGD,
    MIA_DVPP_VENC,

    MIA_MAX_RES_TYPE
};

struct soc_mia_res_info {
    u64 bitmap;
    u32 unit_per_bit;
};

struct soc_reg_base_info {
    phys_addr_t io_base;
    size_t io_base_size;
};

struct soc_rsv_mem_info {
    phys_addr_t rsv_mem;
    size_t rsv_mem_size;
};

/* 定义细化到子模块级别，如stars的cdqm及topic */
#define TS_MAILBOX_ACK_IRQ            0
#define TS_FUNC_CQ_IRQ                1
#define TS_DISP_NFE_IRQ               2  // for devmng
#define TS_CQ_UPDATE_IRQ              3
#define TS_CQ_UPDATE_BIND_THREAD_IRQ  4
#define TS_STARS_CDQM_IRQ             5
#define TS_STARS_TOPIC_IRQ            6
#define TS_SQ_SEND_TRIGGER_IRQ        7
#define TS_PROF_AICORE_IRQ            8
#define TS_PROF_HWTS_LOG_IRQ          9
#define TS_IRQ_TYPE_MAX              10

// for dev
#define DEV_IRQ_TYPE_MAX      0

// interface
enum soc_sub_type {
    TS_SUBSYS = 0,
    MAX_SOC_SUBSYS_TYPE,
};

struct res_inst_info {
    u32 devid;
    enum soc_sub_type sub_type;
    u32 subid;
};

static inline enum soc_sub_type soc_resmng_subsys_type(enum soc_mia_res_type res_type)
{
    if (res_type >= MIA_STARS_RTSQ && res_type <= MIA_STARS_TOPIC_CCPU_SLOT) {
        return TS_SUBSYS;
    }
    return MAX_SOC_SUBSYS_TYPE;
}

static inline void soc_resmng_inst_pack(struct res_inst_info *inst, u32 devid, enum soc_sub_type sub_type, u32 subid)
{
    inst->devid = devid;
    inst->sub_type = sub_type;
    inst->subid = subid;
}

/* for subsys */
int soc_resmng_set_rsv_mem(struct res_inst_info *inst, const char *name, struct soc_rsv_mem_info *rsv_mem);
int soc_resmng_get_rsv_mem(struct res_inst_info *inst, const char *name, struct soc_rsv_mem_info *rsv_mem);

int soc_resmng_set_reg_base(struct res_inst_info *inst, const char *name, struct soc_reg_base_info *io_base);
int soc_resmng_get_reg_base(struct res_inst_info *inst, const char *name, struct soc_reg_base_info *io_base);

int soc_resmng_set_irq_num(struct res_inst_info *inst, u32 irq_type, u32 irq_num);
int soc_resmng_get_irq_num(struct res_inst_info *inst, u32 irq_type, u32 *irq_num);
int soc_resmng_set_irq_by_index(struct res_inst_info *inst, u32 irq_type, u32 index, u32 irq);
int soc_resmng_get_irq_by_index(struct res_inst_info *inst, u32 irq_type, u32 index, u32 *irq);

int soc_resmng_set_hwirq(struct res_inst_info *inst, u32 irq_type, u32 irq, u32 hwirq);
int soc_resmng_get_hwirq(struct res_inst_info *inst, u32 irq_type, u32 irq, u32 *hwirq);

int soc_resmng_set_ts_status(struct res_inst_info *inst, u32 status);
int soc_resmng_get_ts_status(struct res_inst_info *inst, u32 *status);

int soc_resmng_set_mia_res(struct res_inst_info *inst, enum soc_mia_res_type type, u64 bitmap, u32 unit_per_bit);
int soc_resmng_get_mia_res(struct res_inst_info *inst, enum soc_mia_res_type type, u64 *bitmap, u32 *unit_per_bit);

int soc_resmng_subsys_set_num(u32 devid, enum soc_sub_type type, u32 subnum);
int soc_resmng_subsys_get_num(u32 devid, enum soc_sub_type type, u32 *subnum);

/* for dev */
int soc_resmng_dev_set_rsv_mem(u32 devid, const char *name, struct soc_rsv_mem_info *rsv_mem);
int soc_resmng_dev_get_rsv_mem(u32 devid, const char *name, struct soc_rsv_mem_info *rsv_mem);

int soc_resmng_dev_set_reg_base(u32 devid, const char *name, struct soc_reg_base_info *io_base);
int soc_resmng_dev_get_reg_base(u32 devid, const char *name, struct soc_reg_base_info *io_base);

int soc_resmng_dev_set_irq_num(u32 devid, u32 irq_type, u32 irq_num);
int soc_resmng_dev_get_irq_num(u32 devid, u32 irq_type, u32 *irq_num);
int soc_resmng_dev_set_irq_by_index(u32 devid, u32 irq_type, u32 index, u32 irq);
int soc_resmng_dev_get_irq_by_index(u32 devid, u32 irq_type, u32 index, u32 *irq);

int soc_resmng_dev_set_mia_res(u32 devid, enum soc_mia_res_type type, u64 bitmap, u32 unit_per_bit);
int soc_resmng_dev_get_mia_res(u32 devid, enum soc_mia_res_type type, u64 *bitmap, u32 *unit_per_bit);

int soc_resmng_dev_set_mia_spec(u32 devid, u32 vfg_num, u32 vf_num);
int soc_resmng_dev_get_mia_spec(u32 devid, u32 *vfg_num, u32 *vf_num);

int soc_resmng_dev_set_mia_info(u32 devid, u32 vfgid);
int soc_resmng_dev_get_mia_info(u32 devid, u32 *vfgid);

#endif /* __SOC_RES_H */

