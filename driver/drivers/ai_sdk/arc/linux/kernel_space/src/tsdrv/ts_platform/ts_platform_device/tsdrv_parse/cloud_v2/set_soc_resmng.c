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
* Create: 2022-10-08
*/

#include <linux/io.h>

#include "tsdrv_log.h"
#include "devdrv_user_common.h"
#include "tsdrv_firmware_load.h"

#include "soc_res.h"
#include "set_soc_resmng.h"
#ifdef CFG_TRS_REFACTOR_FEATURE
#include "id_pool.h"

enum {
    TS_SYSCTRL_REG,
    TS_DOORBELL_REG,
    TS_STARS_RTSQ_SCHED_REG,
    TS_STARS_CQINT_REG,
    TS_STARS_CDQM_REG,
    TS_STARS_TOPIC_REG,
    TS_STARS_INT_REG,
    TS_STARS_RTSQ_CFG_REG,
    TS_STARS_NOTIFY_TBL_REG,
    TS_STARS_EVENT_TBL_NS_REG,
    TS_STARS_FFTS_CC2_CTRL_REG
};

char ts_reg_name[][SOC_RESMNG_MAX_NAME_LEN] = {
    [TS_SYSCTRL_REG] = "TS_SYSCTRL_REG",
    [TS_DOORBELL_REG] = "TS_DOORBELL_REG",
    [TS_STARS_RTSQ_SCHED_REG] = "TS_STARS_RTSQ_SCHED_REG",
    [TS_STARS_CQINT_REG] = "TS_STARS_CQINT_REG",
    [TS_STARS_CDQM_REG] = "TS_STARS_CDQM_REG",
    [TS_STARS_TOPIC_REG] = "TS_STARS_TOPIC_REG",
    [TS_STARS_INT_REG] = "TS_STARS_INT_REG",
    [TS_STARS_RTSQ_CFG_REG] = "TS_STARS_RTSQ_CFG_REG",
    [TS_STARS_NOTIFY_TBL_REG] = "TS_STARS_NOTIFY_TBL_REG",
    [TS_STARS_EVENT_TBL_NS_REG] = "TS_STARS_EVENT_TBL_NS_REG",
    [TS_STARS_FFTS_CC2_CTRL_REG] = "TS_STARS_FFTS_CC2_CTRL_REG",
};

enum {
    TS_SRAM_MEM,
    TS_SQCQ_MEM,
    TS_STARS_TOPIC_MAILBOX_MEM,
};
char ts_mem_name[][SOC_RESMNG_MAX_NAME_LEN] = {
    [TS_SRAM_MEM] = "TS_SRAM_MEM",
    [TS_SQCQ_MEM] = "TS_SQCQ_MEM",
    [TS_STARS_TOPIC_MAILBOX_MEM] = "TS_STARS_TOPIC_MAILBOX_MEM",
};

static inline int tsdrv_get_device_index(u32 devid)
{
#if defined(CFG_MANAGER_HOST_ENV)
    return devdrv_get_device_index(devid);
#else
    return (int)devid;
#endif
}

#define CDQM_F2NF_INTR_NS_CTRL_INDEX   11
#define TOPIC_SCHED_CPU_INTR_START_INDEX 12

#define TSFW_SHR_MEM_ADDR 0x1036EFDC00 /* 0x1036E00000 …… task shm(1k) + aicpu config(8k) */
#define TSFW_SHR_MEM_ADDR_SIZE 1024

static int set_soc_resmng_subsys_tscpu_reg_base(u32 devid, struct devdrv_ts_pdata *ts_pdata)
{
    struct res_inst_info inst;
    struct soc_reg_base_info io_base;
    int ret;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, ts_pdata->tsid);

    io_base.io_base = ts_pdata->ts_sysctl_paddr;
    io_base.io_base_size = ts_pdata->ts_sysctl_size;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_SYSCTRL_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set reg failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    io_base.io_base = ts_pdata->doorbell_paddr;
    io_base.io_base_size = ts_pdata->doorbell_size;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_DOORBELL_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set reg failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    return 0;
}

static void trs_get_chip_id(struct devdrv_ts_pdata *ts_pdata, u32 *chip_id, u32 *die_id)
{
    void *info_reg = (u32 *)((unsigned long)(uintptr_t)ts_pdata->ts_sysctl_vaddr + 0x1c);
    u32 val = readl_relaxed(info_reg);

    *chip_id = (val >> 4) & 0xF; /* chip offset 4 */
    *die_id = (val >> 8) & 0x3; /* die id saves in 8~9 bit */
}

static int set_soc_resmng_subsys_tscpu_res_mem(u32 devid, struct devdrv_ts_pdata *ts_pdata)
{
    struct res_inst_info inst;
    struct soc_rsv_mem_info rsv_mem;
    u32 chip_id, die_id;
    int ret;

    trs_get_chip_id(ts_pdata, &chip_id, &die_id);
    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, ts_pdata->tsid);

    rsv_mem.rsv_mem = ts_pdata->sram_paddr;
    rsv_mem.rsv_mem_size = ts_pdata->sram_size;
    ret = soc_resmng_set_rsv_mem(&inst, ts_mem_name[TS_SRAM_MEM], &rsv_mem);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rsv_mem failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    TSDRV_PRINT_INFO("chip_id=%u; die_id=%u\n", chip_id, die_id);
    rsv_mem.rsv_mem = TSFW_SHR_MEM_ADDR + tsdrv_get_addr_chip_die_offset(devid, chip_id, die_id);
    rsv_mem.rsv_mem_size = TSFW_SHR_MEM_ADDR_SIZE;
    ret = soc_resmng_set_rsv_mem(&inst, "tsfw_shr_mem", &rsv_mem);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rsv_mem failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    return 0;
}

static int set_soc_resmng_subsys_tscpu_irq(u32 devid, struct devdrv_ts_pdata *ts_pdata)
{
    struct res_inst_info inst;
    int ret, i;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, ts_pdata->tsid);

    ret = soc_resmng_set_irq_num(&inst, TS_MAILBOX_ACK_IRQ, 1);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq_num failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }
    ret = soc_resmng_set_irq_by_index(&inst, TS_MAILBOX_ACK_IRQ, 0, ts_pdata->irq_mailbox_ack_request);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    ret = soc_resmng_set_irq_num(&inst, TS_FUNC_CQ_IRQ, 1);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq_num failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }
    ret = soc_resmng_set_irq_by_index(&inst, TS_FUNC_CQ_IRQ, 0, ts_pdata->irq_functional_cq_request);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }
    ret = soc_resmng_set_hwirq(&inst, TS_FUNC_CQ_IRQ, ts_pdata->irq_functional_cq_request, ts_pdata->irq_functional_cq);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set hwirq failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    ret = soc_resmng_set_irq_num(&inst, TS_STARS_CDQM_IRQ, 1);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq_num failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }
    ret = soc_resmng_set_irq_by_index(&inst, TS_STARS_CDQM_IRQ, 0, ts_pdata->irq_base + CDQM_F2NF_INTR_NS_CTRL_INDEX);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }
    ret = soc_resmng_set_hwirq(&inst, TS_STARS_CDQM_IRQ, ts_pdata->irq_base + CDQM_F2NF_INTR_NS_CTRL_INDEX,
        ts_pdata->irq_base + CDQM_F2NF_INTR_NS_CTRL_INDEX);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    ret = soc_resmng_set_irq_num(&inst, TS_STARS_TOPIC_IRQ, 1);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq_num failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }
    ret = soc_resmng_set_irq_by_index(&inst, TS_STARS_TOPIC_IRQ, 0, ts_pdata->irq_base +
        TOPIC_SCHED_CPU_INTR_START_INDEX);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    ret = soc_resmng_set_irq_num(&inst, TS_CQ_UPDATE_IRQ, ts_pdata->cq_irq_num);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq_num failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    for (i = 0; i < ts_pdata->cq_irq_num; i++) {
        ret = soc_resmng_set_irq_by_index(&inst, TS_CQ_UPDATE_IRQ, i, ts_pdata->irq_cq_update_request[i]);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Set irq failed. (devid=%u; tsid=%u; index=%d; irq=%d)\n", devid, ts_pdata->tsid, i,
                ts_pdata->irq_cq_update[i]);
            return ret;
        }
        ret = soc_resmng_set_hwirq(&inst, TS_CQ_UPDATE_IRQ, ts_pdata->irq_cq_update_request[i],
            ts_pdata->irq_cq_update[i]);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Set irq failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
            return ret;
        }
    }

    return 0;
}

int set_soc_resmng_subsys_tscpu(u32 devid, struct devdrv_ts_pdata *ts_pdata)
{
    int ret;

    ret = set_soc_resmng_subsys_tscpu_reg_base(devid, ts_pdata);
    ret |= set_soc_resmng_subsys_tscpu_res_mem(devid, ts_pdata);
    ret |= set_soc_resmng_subsys_tscpu_irq(devid, ts_pdata);

    return ret;
}

#define TOPIC_SCHED_NS_REG_OFFSET 0x800000
#define TOPIC_SCHED_NS_REG_SIZE (8 * 1024 * 1024)
#define STARS_CDQM_REG_ADDR 0x1000000
#define STARS_CDQM_REG_SIZE (16 * 1024 * 1024)
#define STARS_INT_NS_REG_OFFSET 0x8000
#define STARS_INT_NS_REG_SIZE (32 * 1024)
#define STARS_CQ_INT_REG_OFFSET 0x28000
#define STARS_CQ_INT_REG_SIZE (16 * 1024)
#define STARS_RTSQ_SCHED_OFFSET 0x8000000
#define STARS_RTSQ_SCHED_SIZE (128 * 1024 * 1024)
#define STARS_RTSQ_CFG_OFFSET 0x4000
#define STARS_RTSQ_CFG_SIZE 0x4000
#define STARS_NOTIFY_TBL_OFFSET 0x100000
#define STARS_NOTIFY_TBL_SIZE 0x100000
#define STARS_EVENT_TBL_NS_OFFSET 0x200000
#define STARS_EVENT_TBL_NS_SIZE 0x100000

#define STARS_FFTS_BASE_ADDR 0x03C00000
#define STARS_FFTS_CC2_CTRL 0x200000
#define STARS_FFTS_CC2_CTRL_SIZE 0x20

#define TOPIC_SCHED_RSV_MEM_SIZE (32 * 1024)
#define TOPIC_SCHED_RSV_MEM_BASE_ADDR 0x22000000ULL
int set_soc_resmng_subsys_stars(u32 devid, struct devdrv_platform_data *pdata)
{
    struct res_inst_info inst;
    struct soc_reg_base_info io_base;
    struct soc_rsv_mem_info rsv_mem;
    u32 tsid = 0;
    int ret;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, 0);

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_CQ_INT_REG_OFFSET;
    io_base.io_base_size = STARS_CQ_INT_REG_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_CQINT_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set reg failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_CDQM_REG_ADDR;
    io_base.io_base_size = STARS_CDQM_REG_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_CDQM_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set reg failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + TOPIC_SCHED_NS_REG_OFFSET;
    io_base.io_base_size = TOPIC_SCHED_NS_REG_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_TOPIC_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set reg failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_INT_NS_REG_OFFSET;
    io_base.io_base_size = STARS_INT_NS_REG_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_INT_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set reg failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_RTSQ_SCHED_OFFSET;
    io_base.io_base_size = STARS_RTSQ_SCHED_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_RTSQ_SCHED_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set reg failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_RTSQ_CFG_OFFSET;
    io_base.io_base_size = STARS_RTSQ_CFG_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_RTSQ_CFG_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rtsq cfg reg failed. (devid=%u; tsid=%u; type=%u)\n",
            devid, tsid, TS_STARS_RTSQ_CFG_REG);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_NOTIFY_TBL_OFFSET;
    io_base.io_base_size = STARS_NOTIFY_TBL_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_NOTIFY_TBL_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rtsq cfg reg failed. (devid=%u; tsid=%u; type=%u)\n",
            devid, tsid, TS_STARS_NOTIFY_TBL_REG);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_EVENT_TBL_NS_OFFSET;
    io_base.io_base_size = STARS_EVENT_TBL_NS_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_EVENT_TBL_NS_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rtsq cfg reg failed. (devid=%u; tsid=%u; type=%u)\n",
            devid, tsid, TS_STARS_EVENT_TBL_NS_REG);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] +
        STARS_FFTS_BASE_ADDR + STARS_FFTS_CC2_CTRL;
    io_base.io_base_size = STARS_FFTS_CC2_CTRL_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_FFTS_CC2_CTRL_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set cc2 ctrl reg failed. (devid=%u; tsid=%u; type=%u)\n",
            devid, tsid, TS_STARS_FFTS_CC2_CTRL_REG);
        return ret;
    }

    rsv_mem.rsv_mem = TOPIC_SCHED_RSV_MEM_BASE_ADDR + devid * TOPIC_SCHED_RSV_MEM_SIZE;
    rsv_mem.rsv_mem_size = TOPIC_SCHED_RSV_MEM_SIZE;
    ret = soc_resmng_set_rsv_mem(&inst, ts_mem_name[TS_STARS_TOPIC_MAILBOX_MEM], &rsv_mem);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rsv_mem failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = soc_resmng_subsys_set_num(devid, TS_SUBSYS, pdata->ts_num);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set ts_num failed. (devid=%u; ts_num=%u)\n", devid, pdata->ts_num);
        return ret;
    }

    return 0;
}

int set_id_pool(u32 devid, u32 tsid)
{
    struct id_pool_inst pool_inst;
    struct id_pool_attr attr;
    int ret, i;

    id_pool_inst_pack(&pool_inst, devid, tsid);
    id_pool_attr_pack(&attr, 0, 0, 2048);   /* stream 2048 */
    ret = id_pool_register(&pool_inst, TRS_STREAM_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 64 * 1024);  /* event id 64 * 1024 rsv */
    ret = id_pool_register(&pool_inst, TRS_EVENT_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 8192);   /* notify 8192 */
    ret = id_pool_register(&pool_inst, TRS_NOTIFY_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 2048);   /* model 2048 */
    ret = id_pool_register(&pool_inst, TRS_MODEL_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    for (i = 0; i < 16; i++) {  /* rtsq slice num 16 */
        u32 id_start = i * 128;                         /* rtsq num per slice 128 */
        id_pool_attr_pack(&attr, 0, id_start, 119);     /* first colum id pool node total num 119 */
        ret = id_pool_register(&pool_inst, TRS_HW_SQ_ID, &attr);
        id_start = attr.id_start + attr.id_total_num;
        id_pool_attr_pack(&attr, 1, id_start, 5);       /* vf online adjustable rtsq num 5 */
        ret |= id_pool_register(&pool_inst, TRS_HW_SQ_ID, &attr);
        id_start = attr.id_start + attr.id_total_num;
        id_pool_attr_pack(&attr, 0, id_start, 4);       /* global rsv rtsq num per slice 4 */
        ret = id_pool_register(&pool_inst, TRS_RSV_HW_SQ_ID, &attr);
        if (ret != 0) {
            return ret;
        }
    }

    for (i = 0; i < 16; i++) {  /* rtsq slice num 16 */
        u32 id_start = i * 128;                         /* rtsq num per slice 128 */
        id_pool_attr_pack(&attr, 0, id_start, 119);     /* first colum id pool node total num 119 */
        ret = id_pool_register(&pool_inst, TRS_HW_CQ_ID, &attr);
        id_start = attr.id_start + attr.id_total_num;
        id_pool_attr_pack(&attr, 1, id_start, 5);       /* vf online adjustable rtsq num 5 */
        ret |= id_pool_register(&pool_inst, TRS_HW_CQ_ID, &attr);
        id_start = attr.id_start + attr.id_total_num;
        id_pool_attr_pack(&attr, 0, id_start, 4);       /* global rsv rtsq num per slice 4 */
        ret = id_pool_register(&pool_inst, TRS_RSV_HW_CQ_ID, &attr);
        if (ret != 0) {
            return ret;
        }
    }

    id_pool_attr_pack(&attr, 0, 0, 16);     /* 16 set it later */
    ret = id_pool_register(&pool_inst, TRS_SW_SQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 16);     /* 16 set it later */
    ret = id_pool_register(&pool_inst, TRS_SW_CQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 4);      /* maint sq 4 */
    ret = id_pool_register(&pool_inst, TRS_MAINT_SQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 10);     /* maint cq 10 */
    ret = id_pool_register(&pool_inst, TRS_MAINT_CQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 128);    /* cdq 128 */
    ret = id_pool_register(&pool_inst, TRS_CDQM_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

struct platform_soc_res {
    enum soc_mia_res_type type;
    u32 unit_per_bit;
    u64 bitmap;
} soc_res_list[] = {
    {MIA_AC_AIC,                    1,      0x1FFFFFF}, // aic has pg version which has only 24 core or 20 core
    {MIA_AC_AIV,                    1,      0x3FFFFFFFFFFFF},
    {MIA_AC_C_CORE,                 1,      0xFF},
    {MIA_AC_DSA,                    1,      0xFF},
    {MIA_AC_FFTS,                   1,      0xFFFF},
    {MIA_AC_SDMA,                   1,      0xFFFFFFFF},
    {MIA_AC_PCIE_DMA,               1,      0x1FFE000},
    {MIA_STARS_ACSQ,                8,      0xFFFF},
    {MIA_STARS_CDQ,                 8,      0xFFFF},
    {MIA_STARS_RTSQ,                128,    0xFFFF},
    {MIA_STARS_EVENT,               4096,   0xFFFF},
    {MIA_STARS_NOTIFY,              512,    0xFFFF},
    {MIA_DVPP_JPEGD,                1,      0xFFFFFFF},
    {MIA_DVPP_JPEGE,                1,      0xF},
    {MIA_DVPP_VPC,                  1,      0x3FF},
    {MIA_DVPP_VENC,                 1,      0x0},
    {MIA_DVPP_VDEC,                 1,      0x3},
    {MIA_STARS_TOPIC_ACPU_SLOT,     1,      0xFFFFFFFF},
    {MIA_CPU_HOST_ACPU,             1,      0xFFFFFFFFFFFFFFFF},
    {MIA_CPU_DEV_ACPU,              1,      0xFC},
    {MIA_SYS_MEM,                   16384,     0xF}
};

static int set_platform_soc_res(u32 devid, u32 res_type, u64 bitmap, u32 unit_per_bit)
{
    struct res_inst_info inst = {0};
    enum soc_sub_type sub_type;
    sub_type = soc_resmng_subsys_type(res_type);
    if (sub_type == MAX_SOC_SUBSYS_TYPE) {
        return soc_resmng_dev_set_mia_res(devid, res_type, bitmap, unit_per_bit);
    }

    inst.devid = devid;
    inst.sub_type = sub_type;
    inst.subid = 0; // default subid is 0
    return soc_resmng_set_mia_res(&inst, res_type, bitmap, unit_per_bit);
}

int set_soc_resmng_bitmap_res(u32 devid, u32 tsid)
{
    struct platform_soc_res *res;
    int ret;
    u32 i;

    for (i = 0; i < sizeof(soc_res_list) / sizeof(soc_res_list[0]); ++i) {
        res = &soc_res_list[i];
        ret = set_platform_soc_res(devid, res->type, res->bitmap, res->unit_per_bit);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Set soc_res failed. (devid=%u; type=%u; ret=%d)\n", devid, i, ret);
            return ret;
        }
    }
    return 0;
}
#endif

enum {
    RAS0_REG_INDEX,
    RAS2_REG_INDEX
};

char ras_reg_name[][SOC_RESMNG_MAX_NAME_LEN] = {
    [RAS0_REG_INDEX] = "RAS0_REG",
    [RAS2_REG_INDEX] = "RAS2_REG",
};

#define RAS0_1_REG_SIZE 0x20000
#define RAS2_3_REG_SIZE 0x20000

int set_soc_resmng_subsys_ras(u32 devid, struct devdrv_platform_data *pdata)
{
    struct res_inst_info inst;
    struct soc_reg_base_info io_base;
    int ret;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, 0);
    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_RAS0_INDEX];
    io_base.io_base_size = RAS0_1_REG_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ras_reg_name[RAS0_REG_INDEX], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set ras0 reg failed. (devid=%u)\n", devid);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_RAS2_INDEX];
    io_base.io_base_size = RAS2_3_REG_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ras_reg_name[RAS2_REG_INDEX], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set ras2 reg failed. (devid=%u)\n", devid);
        return ret;
    }
    TSDRV_PRINT_INFO("Set ras subsys success. (devid=%u)\n", devid);

    return 0;
}

/* delete later */
#ifdef TSDRV_UT
void set_soc_resmng_cloud_v2_stub_test(void)
{
}
#endif
