/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/io.h>
#include <linux/of_irq.h>
#include <linux/version.h>
#ifdef AOS_LLVM_BUILD
#include <linux/nodemask.h>
#endif
#include "tsdrv_log.h"
#include "devdrv_platform.h"
#include "devdrv_manager.h"
#include "tsdrv_pdata.h"
#include "tsdrv_parse.h"
#include "soc_res.h"
#include "set_soc_resmng.h"

static int tsdrv_dev_pdata_addr_ioremap(struct platform_device *pdev,
    struct devdrv_platform_data *pdata)
{
    int ret;
    phys_addr_t gicv3_paddr;
    size_t gicv3_size;
    phys_addr_t disp_paddr;
    size_t disp_size;
    phys_addr_t sysctrl_paddr;
    size_t sysctrl_size;

    ret = tsdrv_get_gicv3_addr(pdata->dev_id, &gicv3_paddr, &gicv3_size);
    ret |= tsdrv_get_dispatch_addr(pdata->dev_id, &disp_paddr, &disp_size);
    ret |= tsdrv_get_sysctrl_addr(pdata->dev_id, &sysctrl_paddr, &sysctrl_size);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Get addr info failed. (devid=%u)\n", pdata->dev_id);
        return ret;
    }
    pdata->platform_info.gicv3_base = devm_ioremap(&pdev->dev, gicv3_paddr, gicv3_size);
    if (pdata->platform_info.gicv3_base == NULL) {
        TSDRV_PRINT_ERR("Ioremap gicv3 addr failed.\n");
        return -ENOMEM;
    }
    pdata->platform_info.disp_base = devm_ioremap(&pdev->dev, disp_paddr, disp_size);
    if (pdata->platform_info.disp_base == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Ioremap disp addr failed.\n");
        goto remap_disp_failed;
#endif
    }
    pdata->platform_info.sysctl_base = devm_ioremap(&pdev->dev, sysctrl_paddr, sysctrl_size);
    if (pdata->platform_info.sysctl_base == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Ioremap sysctl addr failed.\n");
        goto remap_sysctl_failed;
#endif
    }

    return 0;
#ifndef TSDRV_UT
remap_sysctl_failed:
    devm_iounmap(&pdev->dev, pdata->platform_info.disp_base);
    pdata->platform_info.disp_base = NULL;
remap_disp_failed:
    devm_iounmap(&pdev->dev, pdata->platform_info.gicv3_base);
    pdata->platform_info.gicv3_base = NULL;
    return -ENOMEM;
#endif
}

static void tsdrv_dev_pdata_addr_iounmap(struct platform_device *pdev, struct devdrv_platform_data *pdata)
{
    if (pdata->platform_info.sysctl_base != NULL) {
        devm_iounmap(&pdev->dev, pdata->platform_info.sysctl_base);
        pdata->platform_info.sysctl_base = NULL;
    }

    if (pdata->platform_info.disp_base != NULL) {
        devm_iounmap(&pdev->dev, pdata->platform_info.disp_base);
        pdata->platform_info.disp_base = NULL;
    }

    if (pdata->platform_info.gicv3_base != NULL) {
        devm_iounmap(&pdev->dev, pdata->platform_info.gicv3_base);
        pdata->platform_info.gicv3_base = NULL;
    }
}

static int ts_tsensor_shm_addr_ioremap(struct platform_device *pdev,
    struct devdrv_ts_pdata *ts_pdata)
{
    if (ts_pdata->tsensor_shm_paddr == 0) {
        return 0;
    }
    ts_pdata->tsensor_shm_vaddr =
        devm_ioremap_wc(&pdev->dev, ts_pdata->tsensor_shm_paddr,
            ts_pdata->tsensor_shm_size);
    if (ts_pdata->tsensor_shm_vaddr == NULL) {
        TSDRV_PRINT_ERR("Ioremap tsensor shm addr failed.\n");
        return -ENOMEM;
    }
    return 0;
}

static void ts_tsensor_shm_addr_iounmap(struct platform_device *pdev,
    struct devdrv_ts_pdata *ts_pdata)
{
    if (ts_pdata->tsensor_shm_vaddr != NULL) {
        devm_iounmap(&pdev->dev, ts_pdata->tsensor_shm_vaddr);
        ts_pdata->tsensor_shm_vaddr = NULL;
    }
}

static int ts_mailbox_addr_ioremap(struct platform_device *pdev,
    struct devdrv_ts_pdata *ts_pdata)
{
    ts_pdata->ts_mbox_send_vaddr =
        devm_ioremap(&pdev->dev, ts_pdata->ts_mbox_send_paddr,
            ts_pdata->ts_mbox_send_size);
    if (ts_pdata->ts_mbox_send_vaddr == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Ioremap mailbox send addr failed.\n");
        return -ENOMEM;
#endif
    }
    ts_pdata->ts_mbox_rcv_vaddr =
        devm_ioremap(&pdev->dev, ts_pdata->ts_mbox_rcv_paddr,
            ts_pdata->ts_mbox_rcv_size);
    if (ts_pdata->ts_mbox_rcv_vaddr == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Ioremap mailbox recv addr failed.\n");
        devm_iounmap(&pdev->dev, ts_pdata->ts_mbox_send_vaddr);
        ts_pdata->ts_mbox_send_vaddr = NULL;
        return -ENOMEM;
#endif
    }
    return 0;
}

static void ts_mailbox_addr_iounmap(struct platform_device *pdev,
    struct devdrv_ts_pdata *ts_pdata)
{
    if (ts_pdata->ts_mbox_rcv_vaddr != NULL) {
        devm_iounmap(&pdev->dev, ts_pdata->ts_mbox_rcv_vaddr);
        ts_pdata->ts_mbox_rcv_vaddr = NULL;
    }

    if (ts_pdata->ts_mbox_send_vaddr != NULL) {
        devm_iounmap(&pdev->dev, ts_pdata->ts_mbox_send_vaddr);
        ts_pdata->ts_mbox_send_vaddr = NULL;
    }
}

static int ts_doorbell_addr_ioremap(struct platform_device *pdev,
    struct devdrv_ts_pdata *ts_pdata)
{
    ts_pdata->doorbell_vaddr =
        devm_ioremap(&pdev->dev, ts_pdata->doorbell_paddr,
            ts_pdata->doorbell_size);
    if (ts_pdata->doorbell_vaddr == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Ioremap doorbell addr failed.\n");
        return -ENOMEM;
#endif
    }
    return 0;
}

static void ts_doorbell_addr_iounmap(struct platform_device *pdev,
    struct devdrv_ts_pdata *ts_pdata)
{
    if (ts_pdata->doorbell_vaddr != NULL) {
        devm_iounmap(&pdev->dev, ts_pdata->doorbell_vaddr);
        ts_pdata->doorbell_vaddr = NULL;
    }
}

static int ts_sysctl_addr_ioremap(struct platform_device *pdev,
    struct devdrv_ts_pdata *ts_pdata)
{
    ts_pdata->ts_sysctl_vaddr =
        devm_ioremap(&pdev->dev, ts_pdata->ts_sysctl_paddr,
            ts_pdata->ts_sysctl_size);
    if (ts_pdata->ts_sysctl_vaddr == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Ioremap ts sysctl addr failed.\n");
        return -ENOMEM;
#endif
    }
    return 0;
}

static void ts_sysctl_addr_iounmap(struct platform_device *pdev,
    struct devdrv_ts_pdata *ts_pdata)
{
    if (ts_pdata->ts_sysctl_vaddr != NULL) {
        devm_iounmap(&pdev->dev, ts_pdata->ts_sysctl_vaddr);
        ts_pdata->ts_sysctl_vaddr = NULL;
    }
}

static int ts_stars_rtsq_addr_ioremap(struct platform_device *pdev,
    struct devdrv_ts_pdata *ts_pdata)
{
    if (ts_pdata->stars_ctrl_paddr == 0) {
        return 0;
    }
    ts_pdata->stars_ctrl_vaddr =
        devm_ioremap(&pdev->dev, ts_pdata->stars_ctrl_paddr,
        ts_pdata->stars_ctrl_size);

    if (ts_pdata->stars_ctrl_vaddr == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Ioremap stars ctrl addr failed.\n");
        return -ENOMEM;
#endif
    }
    return 0;
}

static void ts_stars_rtsq_addr_iounmap(struct platform_device *pdev,
    struct devdrv_ts_pdata *ts_pdata)
{
    if (ts_pdata->stars_ctrl_vaddr != NULL) {
        devm_iounmap(&pdev->dev, ts_pdata->stars_ctrl_vaddr);
        ts_pdata->stars_ctrl_vaddr = NULL;
    }
}

static void tsdrv_ts_pdata_addr_iounmap(struct platform_device *pdev,
    struct devdrv_platform_data *pdata)
{
    u32 tsid;
    struct devdrv_ts_pdata *ts_pdata = NULL;

    for (tsid = 0; tsid < pdata->ts_num; tsid++) {
        ts_pdata = &pdata->ts_pdata[tsid];
        ts_tsensor_shm_addr_iounmap(pdev, ts_pdata);
        ts_doorbell_addr_iounmap(pdev, ts_pdata);
        ts_mailbox_addr_iounmap(pdev, ts_pdata);
        ts_sysctl_addr_iounmap(pdev, ts_pdata);
        ts_stars_rtsq_addr_iounmap(pdev, ts_pdata);
    }
}

static int tsdrv_ts_pdata_addr_ioremap(struct platform_device *pdev,
    struct devdrv_ts_pdata *ts_pdata)
{
    int ret;

    ret = ts_mailbox_addr_ioremap(pdev, ts_pdata);
    if (ret != 0) {
        return ret;
    }

    ret = ts_doorbell_addr_ioremap(pdev, ts_pdata);
    if (ret != 0) {
#ifndef TSDRV_UT
        goto doorbell_ioremap_fail;
#endif
    }

    ret = ts_tsensor_shm_addr_ioremap(pdev, ts_pdata);
    if (ret != 0) {
#ifndef TSDRV_UT
        goto tsensor_ioremap_fail;
#endif
    }

    ret = ts_sysctl_addr_ioremap(pdev, ts_pdata);
    if (ret != 0) {
#ifndef TSDRV_UT
        goto ts_sysctl_ioremap_fail;
#endif
    }

    ret = ts_stars_rtsq_addr_ioremap(pdev, ts_pdata);
    if (ret != 0) {
#ifndef TSDRV_UT
        goto stars_ioremap_fail;
#endif
    }

    return 0;
#ifndef TSDRV_UT
stars_ioremap_fail:
    ts_sysctl_addr_iounmap(pdev, ts_pdata);
ts_sysctl_ioremap_fail:
    ts_tsensor_shm_addr_iounmap(pdev, ts_pdata);
tsensor_ioremap_fail:
    ts_doorbell_addr_iounmap(pdev, ts_pdata);
doorbell_ioremap_fail:
    ts_mailbox_addr_iounmap(pdev, ts_pdata);
    return ret;
#endif
}

int get_ts_pdata_addr(u32 devid,
    struct devdrv_ts_pdata *ts_pdata)
{
    u32 tsid;
    int ret;

    tsid = ts_pdata->tsid;
    ret = tsdrv_get_ts_doorbell_addr(devid, tsid,
        (phys_addr_t *)&ts_pdata->doorbell_paddr, &ts_pdata->doorbell_size);
    ret |= tsdrv_get_ts_sram_addr(devid, tsid,
        (phys_addr_t *)&ts_pdata->sram_paddr, &ts_pdata->sram_size);
    ret |= tsdrv_get_ts_sysctrl_addr(devid, tsid,
        (phys_addr_t *)&ts_pdata->ts_sysctl_paddr, &ts_pdata->ts_sysctl_size);
    ret |= tsdrv_get_tsensor_shm_addr(devid, tsid,
        (phys_addr_t *)&ts_pdata->tsensor_shm_paddr, &ts_pdata->tsensor_shm_size);
    ret |= tsdrv_get_ts_stars_rtsq_addr(devid, tsid,
        (phys_addr_t *)&ts_pdata->stars_ctrl_paddr, &ts_pdata->stars_ctrl_size);
    if (ret != 0) {
        return ret;
    }

    ts_pdata->ts_mbox_send_paddr = ts_pdata->sram_paddr;
    ts_pdata->ts_mbox_send_size = DEVDRV_MAILBOX_PAYLOAD_LENGTH;
    ts_pdata->ts_mbox_rcv_paddr = ts_pdata->sram_paddr + DEVDRV_MAILBOX_PAYLOAD_LENGTH;
    ts_pdata->ts_mbox_rcv_size = DEVDRV_MAILBOX_PAYLOAD_LENGTH;
    return 0;
}

static int get_cq_update_irq(u32 devid,
    struct devdrv_ts_pdata *ts_pdata)
{
    int i;
    u32 tsid;
    int ret;

    tsid = ts_pdata->tsid;
    ts_pdata->cq_irq_num = tsdrv_get_cq_update_irq_num();
    if ((ts_pdata->cq_irq_num == 0) ||
        (ts_pdata->cq_irq_num > DEVDRV_CQ_IRQ_NUM)) {
        TSDRV_PRINT_ERR("Get cq update irq num failed. (devid=%u; tsid=%u)\n",
            devid, tsid);
        return -EINVAL;
    }

    ret = tsdrv_get_cq_update_irq(devid, ts_pdata->tsid,
        (u32 *)&ts_pdata->irq_cq_update_request[0]);
    if (ret != 0) {
        return ret;
    }

    ret = tsdrv_get_cq_update_hwirq((u32 *)&ts_pdata->irq_cq_update_request[0],
        (u32 *)&ts_pdata->irq_cq_update[0]);
    if (ret != 0) {
        return ret;
    }

    for (i = 0; i < ts_pdata->cq_irq_num; i++) {
        TSDRV_PRINT_DEBUG("Get cq update irq. (index=%d; irq=%d; hwirq=%d)\n",
            i, ts_pdata->irq_cq_update_request[i], ts_pdata->irq_cq_update[i]);
    }

    return 0;
}

static int get_mailbox_ack_irq(u32 devid,
    struct devdrv_ts_pdata *ts_pdata)
{
    u32 tsid;
    int ret;

    tsid = ts_pdata->tsid;
    ret = tsdrv_get_mailbox_ack_irq(devid, tsid,
        (u32 *)&ts_pdata->irq_mailbox_ack_request);
    if (ret != 0) {
        return ret;
    }

    ret = tsdrv_get_mailbox_ack_hwirq(ts_pdata->irq_mailbox_ack_request,
        (u32 *)&ts_pdata->irq_mailbox_ack);
    if (ret != 0) {
        return ret;
    }

    TSDRV_PRINT_DEBUG("Get mailbox ack irq. (irq=%d; hwirq=%d)\n",
        ts_pdata->irq_mailbox_ack_request, ts_pdata->irq_mailbox_ack);
    return 0;
}

static int get_mailbox_data_ack_irq(u32 devid,
    struct devdrv_ts_pdata *ts_pdata)
{
    u32 tsid;
    int ret;
    u32 irq_num;

    irq_num = tsdrv_get_mailbox_data_ack_irq_num();
    if (irq_num == 0) {
        return 0;
    }

    tsid = ts_pdata->tsid;
    ret = tsdrv_get_mailbox_data_ack_irq(devid, tsid,
        (u32 *)&ts_pdata->irq_mailbox_data_ack_request);
    if (ret != 0) {
        return ret;
    }

    ret = tsdrv_get_mailbox_data_ack_hwirq(ts_pdata->irq_mailbox_data_ack_request,
        (u32 *)&ts_pdata->irq_mailbox_data_ack);
    if (ret != 0) {
        return ret;
    }

    TSDRV_PRINT_DEBUG("Get mailbox data ack irq. (irq=%d; hwirq=%d)\n",
        ts_pdata->irq_mailbox_data_ack_request, ts_pdata->irq_mailbox_data_ack);
    return 0;
}

static int get_func_cq_irq(u32 devid,
    struct devdrv_ts_pdata *ts_pdata)
{
    u32 tsid;
    int ret;

    tsid = ts_pdata->tsid;
    ret = tsdrv_get_func_cq_irq(devid, tsid,
        (u32 *)&ts_pdata->irq_functional_cq_request);
    if (ret != 0) {
        return ret;
    }

    ret = tsdrv_get_func_cq_hwirq(ts_pdata->irq_functional_cq_request,
        (u32 *)&ts_pdata->irq_functional_cq);
    if (ret != 0) {
        return ret;
    }

    TSDRV_PRINT_DEBUG("Get func cq irq. (irq=%d; hwirq=%d)\n",
        ts_pdata->irq_functional_cq_request, ts_pdata->irq_functional_cq);
    return 0;
}

static int get_disp_nfe_irq(u32 devid,
    struct devdrv_ts_pdata *ts_pdata)
{
    u32 tsid;
    int ret;
    u32 irq_num;

    irq_num = tsdrv_get_disp_nfe_irq_num();
    if (irq_num == 0) {
        return 0;
    }

    tsid = ts_pdata->tsid;
    ret = tsdrv_get_disp_nfe_irq(devid, tsid,
        (u32 *)&ts_pdata->disp_nfe_irq);
    if (ret != 0) {
        return ret;
    }

    TSDRV_PRINT_DEBUG("Get disp nfe irq. (irq=%d)\n", ts_pdata->disp_nfe_irq);
    return 0;
}

static int get_ts_irq_base(u32 devid,
    struct devdrv_ts_pdata *ts_pdata)
{
    u32 tsid;
    int ret;

    tsid = ts_pdata->tsid;
    ret = tsdrv_get_ts_irq_base(devid, tsid, (u32 *)&ts_pdata->irq_base);
    if (ret != 0) {
        return ret;
    }

    TSDRV_PRINT_DEBUG("Get irq base. (irq=%d)\n", ts_pdata->irq_base);
    return 0;
}

static int get_ts_irq(u32 devid, struct devdrv_ts_pdata *ts_pdata)
{
    int ret;

    ret = tsdrv_alloc_irqs(devid);
    if (ret != 0) {
        return ret;
    }

    ret = get_cq_update_irq(devid, ts_pdata);
    ret |= get_mailbox_ack_irq(devid, ts_pdata);
    ret |= get_func_cq_irq(devid, ts_pdata);
    ret |= get_mailbox_data_ack_irq(devid, ts_pdata);
    ret |= get_disp_nfe_irq(devid, ts_pdata);
    ret |= get_ts_irq_base(devid, ts_pdata);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Get ts irq failed. (devid=%u; tsid=%u)\n",
            devid, ts_pdata->tsid);
        return ret;
    }

    return 0;
}
#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC int get_ts_stl_flag_from_device_node(u32 devid, u32 tsid, struct devdrv_ts_pdata *ts_pdata)
{
    u32 of_tsid;
    u32 stl_enable_flag;
    struct device_node *node = NULL;
    struct device_node *son = NULL;
    struct platform_device *pdev = NULL;

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
#endif
        return -ENODEV;
    }

    node = pdev->dev.of_node;
    for_each_child_of_node(node, son) {
        if (of_property_read_u32(son, "tsid", &of_tsid) != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Get ts id failed. (devid=%u)\n", devid);
#endif
            continue;
        }

        if (of_tsid == tsid) {
            if (of_property_read_u32(son, "ts_stl_enable_flag", &stl_enable_flag) != 0) {
                TSDRV_PRINT_INFO("The ts_stl_enable_flag is not configured in dts. (devid=%u; tsid=%u)\n",
                    devid, tsid);
                stl_enable_flag = 0;
            }
            ts_pdata->stl_enable_flag = stl_enable_flag;
            TSDRV_PRINT_INFO("Get ts stl enable flag. (devid=%u; tsid=%u; stl_enable_flag=%u)\n",
                devid, tsid, stl_enable_flag);
            return 0;
        }
    }

    return -EINVAL;
}

// Function: Get the Period of Software Test Library Safety test from the device tree node.
unsigned int get_ts_stl_test_period_from_device_node(u32 devid, u32 tsid)
{
    u32 of_tsid;
    unsigned int stl_test_period = 0;    // The initialization period is an invalid value
    struct device_node *node = NULL;
    struct device_node *son = NULL;
    struct platform_device *pdev = NULL;

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("get_ts_stl_test_period: Get platform device failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return stl_test_period;
    }

    node = pdev->dev.of_node;
    for_each_child_of_node(node, son) { // Find the specified device tree child node based on tsid
        if (of_property_read_u32(son, "tsid", &of_tsid) != 0) { // Read tsid
            TSDRV_PRINT_ERR("get_ts_stl_test_period: Get ts id failed. (devid=%u; tsid=%u)\n", devid, tsid);
            continue;
        }

        if (of_tsid == tsid) { // Find the child node success
            if (of_property_read_u32(son, "ts_stl_test_period", &stl_test_period) != 0) {
#ifndef TSDRV_UT
                TSDRV_PRINT_INFO("The ts_stl_test_period is not configured in dts. (devid=%u; tsid=%u)\n",
                    devid, tsid);
#endif
            }

            break;
        }
    }

    TSDRV_PRINT_INFO("get_ts_stl_test_period. (devid=%u; tsid=%u; stl_test_period=%u)\n", devid, tsid, stl_test_period);
    return stl_test_period;
}
#endif

static int tsdrv_parse_ts_pdata(struct platform_device *pdev,
    struct devdrv_platform_data *pdata)
{
    int ret;
    u32 devid;
    u32 tsid;
    struct devdrv_ts_pdata *ts_pdata = NULL;

    devid = pdata->dev_id;
    pdata->ts_num = tsdrv_get_ts_num(devid);
    if (pdata->ts_num > DEVDRV_MAX_TS_NUM) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Ts number is invalid.(devid=%u; tsnum=%u)\n", devid, pdata->ts_num);
        return -EINVAL;
#endif
    }
    for (tsid = 0; tsid < pdata->ts_num; tsid++) {
        ts_pdata = &pdata->ts_pdata[tsid];
        ts_pdata->tsid = tsid;
        ts_pdata->ts_cpu_core_num = devdrv_get_cpu_number(devid, CPU_TYPE_OF_TS);
        ret = get_ts_pdata_addr(devid, ts_pdata);
        if (ret != 0) {
#ifndef TSDRV_UT
            goto get_addr_fail;
#endif
        }
#ifdef CFG_SOC_PLATFORM_MDC_V51
        ret = get_ts_stl_flag_from_device_node(devid, tsid, ts_pdata);
        if (ret != 0) {
#ifndef TSDRV_UT
            goto get_addr_fail;
#endif
        }
#endif
        ret = get_ts_irq(pdata->dev_id, ts_pdata);
        if (ret != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Get ts irq failed. (devid=%u; tsid=%u)\n",
                devid, tsid);
            goto get_irq_fail;
#endif
        }

        ret = tsdrv_ts_pdata_addr_ioremap(pdev, ts_pdata);
        if (ret != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Ioremap ts addr failed. (devid=%u; tsid=%u)\n",
                devid, tsid);
            goto ioremap_fail;
#endif
        }

#ifdef CFG_TRS_REFACTOR_FEATURE
        ret = set_soc_resmng_subsys_tscpu(devid, ts_pdata);
        if (ret != 0) {
            goto get_irq_fail;
        }

        ret = set_soc_resmng_subsys_stars(devid, pdata);
        if (ret != 0) {
            goto get_irq_fail;
        }

        ret = set_id_pool(devid, tsid);
        if (ret != 0) {
            goto get_irq_fail;
        }
#endif
        TSDRV_PRINT_INFO("Parse ts data info. (devid=%u; tsid=%u; tscpu_num=%u)\n",
            devid, tsid, ts_pdata->ts_cpu_core_num);
    }

    return 0;
#ifndef TSDRV_UT

ioremap_fail:
get_irq_fail:
get_addr_fail:
    tsdrv_ts_pdata_addr_iounmap(pdev, pdata);
    return ret;
#endif
}

#ifndef CFG_SOC_PLATFORM_CLOUD_V2
static void tsdrv_set_ts_mem_restrict_valid(u32 dev_id, u32 *valid)
{
    /* check the node num to determine whether enable TS access memory limit */
    if (num_online_nodes() == tsdrv_get_numa_num_per_dev() * TSDRV_MDC_SLOT_NUM) {
        *valid = TS_MEM_RESTRICT_VALID;
    } else {
        *valid = 0;
    }
    TSDRV_PRINT_INFO("Set ts mem restrict. (devid=%u; valid=%u; online_nodes=%d)\n",
        dev_id, *valid, num_online_nodes());
}
#endif

static int tsdrv_parse_basic_pdata(struct platform_device *pdev, struct devdrv_platform_data *pdata)
{
    int ret;
    u32 devid;

    ret = tsdrv_get_chip_id(pdev, &devid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Get chip id failed. (ret=%d)\n", ret);
        return ret;
    }

    pdata->dev_id = devid;
    /* ccpu & aicpu cluster should be 0 */
    pdata->platform_info.sclid = tsdrv_get_cpu_sclid(devid);
    pdata->platform_info.ccpu_cluster = tsdrv_get_cpu_cluster(devid, TSDRV_CCPU);
    pdata->platform_info.aicpu_cluster = tsdrv_get_cpu_cluster(devid, TSDRV_AICPU);
    pdata->platform_info.ts_cluster = tsdrv_get_cpu_cluster(devid, TSDRV_TSCPU);

    ret = tsdrv_get_board_slot_id(devid,
        &pdata->platform_info.board_id, &pdata->platform_info.slot_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Get board slot id failed. (devid=%u; ret=%d)\n",
            devid, ret);
        return ret;
    }
#ifndef CFG_SOC_PLATFORM_MDC_V51
    (void)tsdrv_get_partial_good(devid, &pdata->platform_info.aicpu_partial_good_enable);
#endif

#ifndef CFG_SOC_PLATFORM_CLOUD_V2
    tsdrv_set_ts_mem_restrict_valid(pdata->dev_id, &pdata->ts_mem_restrict_valid);
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
    ret = tsdrv_get_specification(pdev, &pdata->ts_spec);
    if (ret != 0) {
        TSDRV_PRINT_ERR("get hwts specification failed. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }
#endif
    return 0;
}

static int tsdrv_parse_dev_pdata(struct platform_device *pdev,
    struct devdrv_platform_data *pdata)
{
    int ret;

    ret = devdrv_get_base_addr_info(pdev, pdata);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Get dts addr failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = tsdrv_dev_pdata_addr_ioremap(pdev, pdata);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Ioremap device addr failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}

struct devdrv_platform_data *devdrv_parse_pdata(struct platform_device *pdev)
{
    struct devdrv_platform_data *pdata = NULL;
    int ret;

    pdata = devm_kzalloc(&pdev->dev, sizeof(struct devdrv_platform_data), GFP_KERNEL);
    if (pdata == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Alloc platform data failed.\n");
        return ERR_PTR(-ENOMEM);
#endif
    }

    ret = tsdrv_parse_basic_pdata(pdev, pdata);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Parse basic data failed. (devid=%u; ret=%d)\n",
            pdata->dev_id, ret);
        goto parse_basic_pdata_fail;
#endif
    }

    ret = tsdrv_parse_dev_pdata(pdev, pdata);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Parse device data failed. (devid=%u; ret=%d)\n",
            pdata->dev_id, ret);
        goto parse_dev_pdata_fail;
#endif
    }

    ret = tsdrv_get_plat_info(pdata);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Get plat info failed. (devid=%u; ret=%d)\n",
            pdata->dev_id, ret);
        goto get_plat_info_fail;
#endif
    }

    ret = tsdrv_get_aicpu_occupy_bitmap(pdata->dev_id, pdata->env_type,
        &pdata->platform_info.occupy_bitmap);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Get aicpu bitmap failed. (devid=%u; ret=%d)\n",
            pdata->dev_id, ret);
        goto get_aicpu_bitmap_fail;
#endif
    }

    ret = tsdrv_parse_ts_pdata(pdev, pdata);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Parse ts data failed. (devid=%u; ret=%d)\n",
            pdata->dev_id, ret);
        goto parse_ts_pdata_fail;
#endif
    }

    tsdrv_dfx_cq_irq_bind_core(pdata);
    return pdata;
#ifndef TSDRV_UT
parse_ts_pdata_fail:
get_aicpu_bitmap_fail:
get_plat_info_fail:
    tsdrv_dev_pdata_addr_iounmap(pdev, pdata);
parse_dev_pdata_fail:
parse_basic_pdata_fail:
    devm_kfree(&pdev->dev, pdata);
    pdata = NULL;
    return ERR_PTR(ret);
#endif
}

void devdrv_destroy_pdata(struct platform_device *pdev,
    struct devdrv_platform_data *pdata)
{
    tsdrv_dev_pdata_addr_iounmap(pdev, pdata);
    tsdrv_ts_pdata_addr_iounmap(pdev, pdata);
    devm_kfree(&pdev->dev, pdata);
    pdata = NULL;
}
