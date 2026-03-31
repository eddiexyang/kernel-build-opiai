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
* Create: 2022-08-18
*/
#include "tsdrv_log.h"
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
    TS_STARS_EVENT_TBL_NS_REG
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

#ifdef CFG_MEMORY_OPTIMIZE
#define DEVDRV_RESERVE_MEM_BASE 0xCA00000
#define DEVDRV_RESERVE_MEM_SIZE (21233664)   // 20.25 * 1024 * 1024
#else
#define DEVDRV_RESERVE_MEM_BASE 0x24240000
#define DEVDRV_RESERVE_MEM_SIZE (72 * 1024 * 1024)
#endif
#define CHIP_BASEADDR_PA_OFFSET             (0x200000000000ULL)

#ifdef CFG_MEMORY_OPTIMIZE
#define TSFW_SHR_MEM_ADDR 0xBB3DC00 // (0xBB40000(base) - 9k
#else
#define TSFW_SHR_MEM_ADDR 0x2223dc00 // (0x22240000(base) - 9k
#endif
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

static int set_soc_resmng_subsys_tscpu_res_mem(u32 devid, struct devdrv_ts_pdata *ts_pdata)
{
    struct res_inst_info inst;
    struct soc_rsv_mem_info rsv_mem;
    int ret;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, ts_pdata->tsid);

    rsv_mem.rsv_mem = ts_pdata->sram_paddr;
    rsv_mem.rsv_mem_size = ts_pdata->sram_size;
    ret = soc_resmng_set_rsv_mem(&inst, ts_mem_name[TS_SRAM_MEM], &rsv_mem);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rsv_mem failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    rsv_mem.rsv_mem = DEVDRV_RESERVE_MEM_BASE + ((long)(unsigned)ts_pdata->tsid * DEVDRV_RESERVE_MEM_SIZE) +
        ((long)(unsigned)tsdrv_get_device_index(devid) * CHIP_BASEADDR_PA_OFFSET);
    rsv_mem.rsv_mem_size = DEVDRV_RESERVE_MEM_SIZE;
    ret = soc_resmng_set_rsv_mem(&inst, ts_mem_name[TS_SQCQ_MEM], &rsv_mem);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rsv_mem failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    rsv_mem.rsv_mem = TSFW_SHR_MEM_ADDR;
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
            TSDRV_PRINT_ERR("Set irq failed. (devid=%u; tsid=%u; index=%d; irq=%u)\n", devid, ts_pdata->tsid, i,
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

static int set_soc_resmng_subsys_trigger_irq(u32 devid, struct devdrv_ts_pdata *ts_pdata)
{
    struct res_inst_info inst;
    int ret, i;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, ts_pdata->tsid);
    ret = soc_resmng_set_irq_num(&inst, TS_SQ_SEND_TRIGGER_IRQ, DEVDRV_TRIGGER_IRQ_NUM);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set trigger irq_num failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    for (i = 0; i < DEVDRV_TRIGGER_IRQ_NUM; i++) {
        ret = soc_resmng_set_irq_by_index(&inst, TS_SQ_SEND_TRIGGER_IRQ, i, ts_pdata->irq_sq_trigger_request[i]);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Set trigger irq failed. (devid=%u; tsid=%u; index=%d; irq=%u)\n",
                devid, ts_pdata->tsid, i, ts_pdata->irq_cq_update[i]);
            return ret;
        }
        ret = soc_resmng_set_hwirq(&inst, TS_SQ_SEND_TRIGGER_IRQ, ts_pdata->irq_sq_trigger_request[i],
            ts_pdata->irq_sq_trigger[i]);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Set trigger irq failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
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
    ret |= set_soc_resmng_subsys_trigger_irq(devid, ts_pdata);
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

#define TOPIC_SCHED_RSV_MEM_SIZE (32 * 1024)
#ifdef CFG_MEMORY_OPTIMIZE
#define TOPIC_SCHED_RSV_MEM_BASE_ADDR 0xB900000ULL
#else
#define TOPIC_SCHED_RSV_MEM_BASE_ADDR 0x22000000ULL
#endif
int set_soc_resmng_subsys_stars(u32 devid, struct devdrv_platform_data *pdata)
{
    struct res_inst_info inst;
    struct soc_reg_base_info io_base;
    struct soc_rsv_mem_info rsv_mem;
    int ret;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, 0);

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_CQ_INT_REG_OFFSET;
    io_base.io_base_size = STARS_CQ_INT_REG_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_CQINT_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set reg failed. (devid=%u; tsid=%u)\n", devid, 0);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_CDQM_REG_ADDR;
    io_base.io_base_size = STARS_CDQM_REG_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_CDQM_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set reg failed. (devid=%u; tsid=%u)\n", devid, 0);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + TOPIC_SCHED_NS_REG_OFFSET;
    io_base.io_base_size = TOPIC_SCHED_NS_REG_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_TOPIC_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set reg failed. (devid=%u; tsid=%u)\n", devid, 0);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_INT_NS_REG_OFFSET;
    io_base.io_base_size = STARS_INT_NS_REG_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_INT_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set reg failed. (devid=%u; tsid=%u)\n", devid, 0);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_RTSQ_SCHED_OFFSET;
    io_base.io_base_size = STARS_RTSQ_SCHED_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_RTSQ_SCHED_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set reg failed. (devid=%u; tsid=%u)\n", devid, 0);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_RTSQ_CFG_OFFSET;
    io_base.io_base_size = STARS_RTSQ_CFG_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_RTSQ_CFG_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rtsq cfg reg failed. (devid=%u; tsid=%u; type=%u)\n",
            devid, 0, TS_STARS_RTSQ_CFG_REG);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_NOTIFY_TBL_OFFSET;
    io_base.io_base_size = STARS_NOTIFY_TBL_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_NOTIFY_TBL_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rtsq cfg reg failed. (devid=%u; tsid=%u; type=%u)\n",
            devid, 0, TS_STARS_NOTIFY_TBL_REG);
        return ret;
    }

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_STARS_INDEX] + STARS_EVENT_TBL_NS_OFFSET;
    io_base.io_base_size = STARS_EVENT_TBL_NS_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_STARS_EVENT_TBL_NS_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rtsq cfg reg failed. (devid=%u; tsid=%u; type=%u)\n",
            devid, 0, TS_STARS_EVENT_TBL_NS_REG);
        return ret;
    }

    rsv_mem.rsv_mem = TOPIC_SCHED_RSV_MEM_BASE_ADDR + devid * TOPIC_SCHED_RSV_MEM_SIZE;
    rsv_mem.rsv_mem_size = TOPIC_SCHED_RSV_MEM_SIZE;
    ret = soc_resmng_set_rsv_mem(&inst, ts_mem_name[TS_STARS_TOPIC_MAILBOX_MEM], &rsv_mem);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rsv_mem failed. (devid=%u; tsid=%u)\n", devid, 0);
        return ret;
    }

    ret = soc_resmng_subsys_set_num(devid, TS_SUBSYS, pdata->ts_num);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set ts_num failed. (devid=%u; ts_num=%u)\n", devid, pdata->ts_num);
        return ret;
    }

    return 0;
}

#ifdef CFG_MEMORY_OPTIMIZE
int set_id_pool(u32 devid, u32 tsid)
{
    struct id_pool_inst pool_inst;
    struct id_pool_attr attr;
    int ret, i;

    id_pool_inst_pack(&pool_inst, devid, tsid);
    id_pool_attr_pack(&attr, 0, 0, 128);    /* stream 128 */
    ret = id_pool_register(&pool_inst, TRS_STREAM_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 64 * 1024);  /* event id 64 * 1024 rsv */
    ret = id_pool_register(&pool_inst, TRS_EVENT_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 2048);   /* notify 2048 */
    ret = id_pool_register(&pool_inst, TRS_NOTIFY_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 128);    /* model 128 = stream num */
    ret = id_pool_register(&pool_inst, TRS_MODEL_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 64 * 1024 - 1); /* cmo 64 * 1024 - 1 */
    ret = id_pool_register(&pool_inst, TRS_CMO_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    for (i = 0; i < 16; i++) {  /* rtsq slice num 16 */
        u32 id_start = i * 9;                         /* rtsq num per slice 9 */
        id_pool_attr_pack(&attr, 0, id_start, 8);     /* first colum id pool node total num 8 */
        ret = id_pool_register(&pool_inst, TRS_HW_SQ_ID, &attr);
        id_start = attr.id_start + attr.id_total_num;
        id_pool_attr_pack(&attr, 1, id_start, 0);       /* vf online adjustable rtsq num 0 */
        ret |= id_pool_register(&pool_inst, TRS_HW_SQ_ID, &attr);
        id_start = attr.id_start + attr.id_total_num;
        id_pool_attr_pack(&attr, 0, id_start, 1);       /* global rsv rtsq num per slice 1 */
        ret = id_pool_register(&pool_inst, TRS_RSV_HW_SQ_ID, &attr);
        if (ret != 0) {
            return ret;
        }
    }

    for (i = 0; i < 16; i++) {  /* rtsq slice num 16 */
        u32 id_start = i * 9;                         /* rtsq num per slice 9 */
        id_pool_attr_pack(&attr, 0, id_start, 8);     /* first colum id pool node total num 8 */
        ret = id_pool_register(&pool_inst, TRS_HW_CQ_ID, &attr);
        id_start = attr.id_start + attr.id_total_num;
        id_pool_attr_pack(&attr, 1, id_start, 0);       /* vf online adjustable rtsq num 0 */
        ret |= id_pool_register(&pool_inst, TRS_HW_CQ_ID, &attr);
        id_start = attr.id_start + attr.id_total_num;
        id_pool_attr_pack(&attr, 0, id_start, 1);       /* global rsv rtsq num per slice 1 */
        ret = id_pool_register(&pool_inst, TRS_RSV_HW_CQ_ID, &attr);
        if (ret != 0) {
            return ret;
        }
    }

    id_pool_attr_pack(&attr, 0, 0, 16); /* 16 set it later */
    ret = id_pool_register(&pool_inst, TRS_SW_SQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 16); /* 16 set it later */
    ret = id_pool_register(&pool_inst, TRS_SW_CQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 4);  /* maint sq 4 */
    ret = id_pool_register(&pool_inst, TRS_MAINT_SQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 10);  /* maint cq 10 */
    ret = id_pool_register(&pool_inst, TRS_MAINT_CQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 16); /* cdq 16 */
    ret = id_pool_register(&pool_inst, TRS_CDQM_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    return 0;
}
#else
int set_id_pool(u32 devid, u32 tsid)
{
    struct id_pool_inst pool_inst;
    struct id_pool_attr attr;
    int ret, i;

    id_pool_inst_pack(&pool_inst, devid, tsid);
    id_pool_attr_pack(&attr, 0, 0, 512);    /* stream 512 */
    ret = id_pool_register(&pool_inst, TRS_STREAM_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 64 * 1024);  /* event id 64 * 1024 rsv */
    ret = id_pool_register(&pool_inst, TRS_EVENT_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 2048);   /* notify 2048 */
    ret = id_pool_register(&pool_inst, TRS_NOTIFY_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 512);    /* model 512 = stream num */
    ret = id_pool_register(&pool_inst, TRS_MODEL_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 64 * 1024 - 1); /* cmo 64 * 1024 - 1 */
    ret = id_pool_register(&pool_inst, TRS_CMO_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    for (i = 0; i < 16; i++) {  /* rtsq slice num 16 */
        u32 id_start = i * 32;                         /* rtsq num per slice 32 */
        id_pool_attr_pack(&attr, 0, id_start, 22);     /* first colum id pool node total num 22 */
        ret = id_pool_register(&pool_inst, TRS_HW_SQ_ID, &attr);
        id_start = attr.id_start + attr.id_total_num;
        id_pool_attr_pack(&attr, 1, id_start, 7);       /* vf online adjustable rtsq num 7 */
        ret |= id_pool_register(&pool_inst, TRS_HW_SQ_ID, &attr);
        id_start = attr.id_start + attr.id_total_num;
        id_pool_attr_pack(&attr, 0, id_start, 3);       /* global rsv rtsq num per slice 3 */
        ret = id_pool_register(&pool_inst, TRS_RSV_HW_SQ_ID, &attr);
        if (ret != 0) {
            return ret;
        }
    }

    for (i = 0; i < 16; i++) {  /* rtsq slice num 16 */
        u32 id_start = i * 32;                         /* rtsq num per slice 32 */
        id_pool_attr_pack(&attr, 0, id_start, 22);     /* first colum id pool node total num 22 */
        ret = id_pool_register(&pool_inst, TRS_HW_CQ_ID, &attr);
        id_start = attr.id_start + attr.id_total_num;
        id_pool_attr_pack(&attr, 1, id_start, 7);       /* vf online adjustable rtsq num 7 */
        ret |= id_pool_register(&pool_inst, TRS_HW_CQ_ID, &attr);
        id_start = attr.id_start + attr.id_total_num;
        id_pool_attr_pack(&attr, 0, id_start, 3);       /* global rsv rtsq num per slice 3 */
        ret = id_pool_register(&pool_inst, TRS_RSV_HW_CQ_ID, &attr);
        if (ret != 0) {
            return ret;
        }
    }

    id_pool_attr_pack(&attr, 0, 0, 16); /* 16 set it later */
    ret = id_pool_register(&pool_inst, TRS_SW_SQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 16); /* 16 set it later */
    ret = id_pool_register(&pool_inst, TRS_SW_CQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 4);  /* maint sq 4 */
    ret = id_pool_register(&pool_inst, TRS_MAINT_SQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 10);  /* maint cq 10 */
    ret = id_pool_register(&pool_inst, TRS_MAINT_CQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    id_pool_attr_pack(&attr, 0, 0, 16); /* cdq 16 */
    ret = id_pool_register(&pool_inst, TRS_CDQM_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    return 0;
}
#endif

int set_soc_resmng_bitmap_res(u32 devid, u32 tsid)
{
    int ret;
    struct res_inst_info inst = {0};
    inst.devid = devid;
    inst.sub_type = TS_SUBSYS;
    inst.subid = tsid;
    ret = soc_resmng_set_mia_res(&inst, MIA_STARS_EVENT, 0xffff, 4096);    /* 16: bitnum, 4096: num_per_bit */
    if (ret != 0) {
        return ret;
    }
    ret = soc_resmng_set_mia_res(&inst, MIA_STARS_NOTIFY, 0xffff, 128);    /* 16: bitnum, 128: num_per_bit */
    if (ret != 0) {
        return ret;
    }
    ret = soc_resmng_set_mia_res(&inst, MIA_STARS_CMO, 0xffff, 4095);    /* 16: bitnum, 4095: num_per_bit */
    if (ret != 0) {
        return ret;
    }
    ret = soc_resmng_set_mia_res(&inst, MIA_STARS_RTSQ, 0xffff, 32);    /* 16: bitnum, 32: num_per_bit */
    if (ret != 0) {
        return ret;
    }
    ret = soc_resmng_set_mia_res(&inst, MIA_STARS_CDQ, 0xffff, 1);     /* 16: bitnum, 1: num_per_bit */
    if (ret != 0) {
        return ret;
    }
    return 0;
}
#endif

enum {
    ARM_RAS_REG_INDEX,
};

char ras_reg_name[][SOC_RESMNG_MAX_NAME_LEN] = {
    [ARM_RAS_REG_INDEX] = "ARM_RAS_REG",
};

#define RAS_REGS_SIZE 0x10000

int set_soc_resmng_subsys_ras(u32 devid, struct devdrv_platform_data *pdata)
{
    struct res_inst_info inst;
    struct soc_reg_base_info io_base;
    int ret;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, 0);

    io_base.io_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_ARM_RAS_INDEX];
    io_base.io_base_size = RAS_REGS_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ras_reg_name[ARM_RAS_REG_INDEX], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set arm ras reg failed. (devid=%u)\n", devid);
        return ret;
    }

    return 0;
}

/* delete later */
#ifdef TSDRV_UT
void set_soc_resmng_stub_test(void)
{
}
#endif
