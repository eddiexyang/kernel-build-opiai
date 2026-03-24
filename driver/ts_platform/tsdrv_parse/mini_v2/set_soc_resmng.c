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
* Create: 2022-12-07
*/
#ifdef CFG_FEATURE_PROF_HWTS_THROUGH
#include "tsdrv_log.h"
#include "soc_res.h"
#include "set_soc_resmng.h"
#include "id_pool.h"

enum {
    TS_SYSCTRL_REG,
    TS_DOORBELL_REG,
    TS_HWTS_REG,
};

char ts_reg_name[][SOC_RESMNG_MAX_NAME_LEN] = {
    [TS_SYSCTRL_REG] = "TS_SYSCTRL_REG",
    [TS_DOORBELL_REG] = "TS_DOORBELL_REG",
    [TS_HWTS_REG] = "TS_HWTS_REG",
};

enum {
    TS_SRAM_MEM,
    TS_SQCQ_MEM,
    TS_SQCQ_MEM1,
    TS_MAINT_SQCQ_MEM,
};
char ts_mem_name[][SOC_RESMNG_MAX_NAME_LEN] = {
    [TS_SRAM_MEM] = "TS_SRAM_MEM",
    [TS_SQCQ_MEM] = "TS_SQCQ_MEM",
    [TS_SQCQ_MEM1] = "TS_SQCQ_MEM1",
    [TS_MAINT_SQCQ_MEM] = "TS_MAINT_SQCQ_MEM",
};

static inline int tsdrv_get_device_index(u32 devid)
{
#if defined(CFG_MANAGER_HOST_ENV)
    return devdrv_get_device_index(devid);
#else
    return (int)devid;
#endif
}

#define TS_HWTS_REG_BASE_ADDR                                    0xBA700000
#define TS_HWTS_REG_SIZE                              (0x9A108 + 32 * 0x20)

#define SQ_RESERVE_MEM_BASE 0xBB00000
#define SQ_RESERVE_MEM_SIZE (32 * 1024 * 1024) /* sqmem size is 32M per ts */

#define CQ_RESERVE_MEM_BASE (0x2B981000 + 0x200000) /* reserve mem: start addr + offset */
#define CQ_RESERVE_MEM_SIZE (16 * 1024 * 256)

#define MAINT_SQCQ_RESERVE_MEM_BASE 0x2C381000
#define MAINT_SQCQ_RESERVE_MEM_SIZE  0x600000  /* (0x2C981000 - 0x20000) 0x20000 reserved for callback cq buffer */

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

    io_base.io_base = TS_HWTS_REG_BASE_ADDR;
    io_base.io_base_size = TS_HWTS_REG_SIZE;
    ret = soc_resmng_set_reg_base(&inst, ts_reg_name[TS_HWTS_REG], &io_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set hwts reg failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    return 0;
}

static int set_soc_resmng_subsys_tscpu_rsv_mem(u32 devid, struct devdrv_ts_pdata *ts_pdata)
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

    rsv_mem.rsv_mem = SQ_RESERVE_MEM_BASE + ((long)(unsigned)ts_pdata->tsid * SQ_RESERVE_MEM_SIZE) +
        ((long)(unsigned)tsdrv_get_device_index(devid) * CHIP_BASEADDR_PA_OFFSET);
    rsv_mem.rsv_mem_size = SQ_RESERVE_MEM_SIZE;
    ret = soc_resmng_set_rsv_mem(&inst, ts_mem_name[TS_SQCQ_MEM], &rsv_mem);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rsv_mem failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    rsv_mem.rsv_mem = CQ_RESERVE_MEM_BASE + ((phys_addr_t)tsdrv_get_device_index(devid) * CHIP_BASEADDR_PA_OFFSET) +
        ((phys_addr_t)ts_pdata->tsid * (phys_addr_t)CQ_RESERVE_MEM_SIZE);
    rsv_mem.rsv_mem_size = CQ_RESERVE_MEM_SIZE;
    ret = soc_resmng_set_rsv_mem(&inst, ts_mem_name[TS_SQCQ_MEM1], &rsv_mem);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set rsv_mem failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    rsv_mem.rsv_mem = MAINT_SQCQ_RESERVE_MEM_BASE +
        ((phys_addr_t)tsdrv_get_device_index(devid) * CHIP_BASEADDR_PA_OFFSET);
    rsv_mem.rsv_mem_size = MAINT_SQCQ_RESERVE_MEM_SIZE;
    ret = soc_resmng_set_rsv_mem(&inst, ts_mem_name[TS_MAINT_SQCQ_MEM], &rsv_mem);
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

static int set_soc_resmng_subsys_tscpu_prof_irq(u32 devid, struct devdrv_ts_pdata *ts_pdata)
{
    struct res_inst_info inst;
    int ret;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, ts_pdata->tsid);

    ret = soc_resmng_set_irq_num(&inst, TS_PROF_AICORE_IRQ, 1);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq_num failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }
    ret = soc_resmng_set_irq_by_index(&inst, TS_PROF_AICORE_IRQ, 0, ts_pdata->irq_prof_aicore_request);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    ret = soc_resmng_set_irq_num(&inst, TS_PROF_HWTS_LOG_IRQ, 1);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq_num failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }
    ret = soc_resmng_set_irq_by_index(&inst, TS_PROF_HWTS_LOG_IRQ, 0, ts_pdata->irq_prof_hwts_log_request);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Set irq failed. (devid=%u; tsid=%u)\n", devid, ts_pdata->tsid);
        return ret;
    }

    TSDRV_PRINT_DEBUG("Set soc res irq. (aic irq=%d; hwts log irq=%d)\n",
        ts_pdata->irq_prof_aicore_request, ts_pdata->irq_prof_hwts_log_request);
    return 0;
}

int set_soc_resmng_subsys_tscpu(u32 devid, struct devdrv_ts_pdata *ts_pdata)
{
    int ret;

    ret = set_soc_resmng_subsys_tscpu_reg_base(devid, ts_pdata);
    ret |= set_soc_resmng_subsys_tscpu_rsv_mem(devid, ts_pdata);
    ret |= set_soc_resmng_subsys_tscpu_irq(devid, ts_pdata);
    ret |= set_soc_resmng_subsys_tscpu_prof_irq(devid, ts_pdata);

    return ret;
}
#endif

#ifdef CFG_TRS_REFACTOR_FEATURE
int set_soc_resmng_subsys_stars(u32 devid, struct devdrv_platform_data *pdata)
{
    int ret;

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
    int ret;

    id_pool_inst_pack(&pool_inst, devid, tsid);
    attr.id_start = 0;
    attr.id_total_num = 1024; /* stream 1024 */
    ret = id_pool_register(&pool_inst, TRS_STREAM_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    attr.id_start = 0;
    attr.id_total_num = 1024; /* event id 1024 */
    ret = id_pool_register(&pool_inst, TRS_EVENT_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    attr.id_start = 0;
    attr.id_total_num = 1024; /* notify 1024 */
    ret = id_pool_register(&pool_inst, TRS_NOTIFY_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    attr.id_start = 0;
    attr.id_total_num = 1024; /* model 1024 = stream num */
    ret = id_pool_register(&pool_inst, TRS_MODEL_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    attr.id_start = 0;
    attr.id_total_num = 512; /* hw sq 512 */
    ret = id_pool_register(&pool_inst, TRS_HW_SQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    attr.id_start = 0;
    attr.id_total_num = 334; /* hw cq 334 */
    ret = id_pool_register(&pool_inst, TRS_HW_CQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    attr.id_start = 351; /* start from 351 */
    attr.id_total_num = 1; /* task sched cq 2 (cb) */
    ret = id_pool_register(&pool_inst, TRS_TASK_SCHED_CQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    attr.id_start = 0;
    attr.id_total_num = 4; /* maint sq 4 */
    ret = id_pool_register(&pool_inst, TRS_MAINT_SQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    attr.id_start = 0;
    attr.id_total_num = 10; /* maint cq 10 */
    ret = id_pool_register(&pool_inst, TRS_MAINT_CQ_ID, &attr);
    if (ret != 0) {
        return ret;
    }

    return 0;
}
#endif

/* delete later */
#ifdef TSDRV_UT
void set_soc_resmng_mini_v2_stub_test(void)
{
}
#endif
