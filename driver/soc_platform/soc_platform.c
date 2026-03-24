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
 * Create: 2022-09-15
 */
#ifndef EMU_ST
#include <linux/module.h>

/* Stub register addresses for ascend310Brc */


#ifndef DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_EVENT_TBL_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_NOTIFY_TBL_BASE

#define DEVDRV_ADDR_TS_NOTIFY_TBL_BASE 0

#endif

#include <linux/platform_device.h>

/* Stub register addresses for ascend310Brc */


#ifndef DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_EVENT_TBL_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_NOTIFY_TBL_BASE

#define DEVDRV_ADDR_TS_NOTIFY_TBL_BASE 0

#endif

#include <linux/pci.h>

/* Stub register addresses for ascend310Brc */


#ifndef DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_EVENT_TBL_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_NOTIFY_TBL_BASE

#define DEVDRV_ADDR_TS_NOTIFY_TBL_BASE 0

#endif

#include <linux/delay.h>

/* Stub register addresses for ascend310Brc */


#ifndef DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_EVENT_TBL_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_NOTIFY_TBL_BASE

#define DEVDRV_ADDR_TS_NOTIFY_TBL_BASE 0

#endif


#include "devdrv_interface.h"

/* Stub register addresses for ascend310Brc */


#ifndef DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_EVENT_TBL_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_NOTIFY_TBL_BASE

#define DEVDRV_ADDR_TS_NOTIFY_TBL_BASE 0

#endif

#include "host_config.h"

/* Stub register addresses for ascend310Brc */


#ifndef DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_EVENT_TBL_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_NOTIFY_TBL_BASE

#define DEVDRV_ADDR_TS_NOTIFY_TBL_BASE 0

#endif

#include "soc_res.h"

/* Stub register addresses for ascend310Brc */


#ifndef DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_EVENT_TBL_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_NOTIFY_TBL_BASE

#define DEVDRV_ADDR_TS_NOTIFY_TBL_BASE 0

#endif

#include "dbl/uda.h"

/* Stub register addresses for ascend310Brc */


#ifndef DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_EVENT_TBL_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_NOTIFY_TBL_BASE

#define DEVDRV_ADDR_TS_NOTIFY_TBL_BASE 0

#endif


#include "soc_platform.h"

/* Stub register addresses for ascend310Brc */


#ifndef DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_EVENT_TBL_BASE
#define DEVDRV_ADDR_TS_EVENT_TBL_BASE 0
#endif
#ifndef DEVDRV_ADDR_TS_NOTIFY_TBL_BASE

#define DEVDRV_ADDR_TS_NOTIFY_TBL_BASE 0

#endif


#define PCI_VENDOR_ID_HUAWEI 0x19e5

static const struct pci_device_id soc_driver_tbl[] = {
    {PCI_VDEVICE(HUAWEI, 0xd100), 0},
    {PCI_VDEVICE(HUAWEI, 0xd105), 0},
    {PCI_VDEVICE(HUAWEI, PCI_DEVICE_CLOUD), 0},
    {PCI_VDEVICE(HUAWEI, 0xd801), 0},
    {PCI_VDEVICE(HUAWEI, 0xd500), 0},
    {PCI_VDEVICE(HUAWEI, 0xd501), 0},
    {PCI_VDEVICE(HUAWEI, 0xd802), 0},
    {PCI_VDEVICE(HUAWEI, 0xd803), 0},
    {PCI_VDEVICE(HUAWEI, 0xd804), 0},
    {PCI_VDEVICE(HUAWEI, 0xd805), 0},
    {DEVDRV_DIVERSITY_PCIE_VENDOR_ID, 0xd500, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {}
};
MODULE_DEVICE_TABLE(pci, soc_driver_tbl);

static int soc_platform_set_cdqm_f2nf_irq(u32 devid, u32 tsid)
{
    struct res_inst_info inst;
    int ret;
    u32 irq, irq_request;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, tsid);

    ret = soc_resmng_set_irq_num(&inst, TS_STARS_CDQM_IRQ, 1);
    if (ret != 0) {
        soc_err("Set irq_num failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = devdrv_get_cdqm_irq_vector_id(devid, 0, &irq);
    if (ret != 0) {
        soc_warn("Cdqm not support. (devid=%u; tsid=%u)\n", devid, tsid);
        return 0;
    }

    ret = devdrv_get_irq_vector(devid, irq, &irq_request);
    if (ret != 0) {
        soc_err("Get mb irq vector failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = soc_resmng_set_irq_by_index(&inst, TS_STARS_CDQM_IRQ, 0, irq_request);
    if (ret != 0) {
        soc_err("Set irq failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = soc_resmng_set_hwirq(&inst, TS_STARS_CDQM_IRQ, irq_request, irq);
    if (ret != 0) {
        soc_err("Set irq failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    return 0;
}

static int soc_platform_set_mb_irq(u32 devid, u32 tsid)
{
    struct res_inst_info inst;
    int ret;
    u32 irq, irq_request;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, tsid);

    ret = soc_resmng_set_irq_num(&inst, TS_MAILBOX_ACK_IRQ, 1);
    if (ret != 0) {
        soc_err("Set irq_num failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = devdrv_get_ts_drv_irq_vector_id(devid, 0, &irq);
    if (ret != 0) {
        soc_err("Get mb irq failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = devdrv_get_irq_vector(devid, irq, &irq_request);
    if (ret != 0) {
        soc_err("Get mb irq vector failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = soc_resmng_set_irq_by_index(&inst, TS_MAILBOX_ACK_IRQ, 0, irq_request);
    if (ret != 0) {
        soc_err("Set irq failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = soc_resmng_set_hwirq(&inst, TS_MAILBOX_ACK_IRQ, irq_request, irq);
    if (ret != 0) {
        soc_err("Set irq failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    return 0;
}

static int soc_platform_set_sq_send_trigger_irq(u32 devid, u32 tsid)
{
    struct res_inst_info inst;
    int ret;
    u32 irq, irq_request;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, tsid);

    ret = soc_resmng_set_irq_num(&inst, TS_SQ_SEND_TRIGGER_IRQ, 1);
    if (ret != 0) {
        soc_err("Set irq_num failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = devdrv_get_ts_drv_irq_vector_id(devid, 1, &irq);
    if (ret != 0) {
        soc_err("Get mb irq failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = devdrv_get_irq_vector(devid, irq, &irq_request);
    if (ret != 0) {
        soc_err("Get mb irq vector failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = soc_resmng_set_irq_by_index(&inst, TS_SQ_SEND_TRIGGER_IRQ, 0, irq_request);
    if (ret != 0) {
        soc_err("Set irq failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = soc_resmng_set_hwirq(&inst, TS_SQ_SEND_TRIGGER_IRQ, irq_request, irq);
    if (ret != 0) {
        soc_err("Set irq failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    return 0;
}

#define CQ_UPDATE_IRQ_NUM 16
#define VDEV_START_ID 100
static int soc_platform_set_cq_update_irq(u32 devid, u32 tsid)
{
    struct res_inst_info inst;
    int ret, i, index;
    u32 irq, irq_request;
    int irq_num = (devid < VDEV_START_ID) ? CQ_UPDATE_IRQ_NUM : CQ_UPDATE_IRQ_NUM / 2;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, tsid);

    ret = soc_resmng_set_irq_num(&inst, TS_CQ_UPDATE_IRQ, irq_num);
    if (ret != 0) {
        soc_err("Set irq_num failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    index = 2; /* cq update start from 2 */
    for (i = 0; i < irq_num; i++, index++) {
        ret = devdrv_get_ts_drv_irq_vector_id(devid, index, &irq);
        if (ret != 0) {
            soc_err("Get mb irq failed. (devid=%u; tsid=%u)\n", devid, tsid);
            return ret;
        }

        ret = devdrv_get_irq_vector(devid, irq, &irq_request);
        if (ret != 0) {
            soc_err("Get mb irq vector failed. (devid=%u; tsid=%u)\n", devid, tsid);
            return ret;
        }

        ret = soc_resmng_set_irq_by_index(&inst, TS_CQ_UPDATE_IRQ, i, irq_request);
        if (ret != 0) {
            soc_err("Set irq failed. (devid=%u; tsid=%u; index=%d; irq=%u)\n", devid, tsid, i, irq_request);
            return ret;
        }

        ret = soc_resmng_set_hwirq(&inst, TS_CQ_UPDATE_IRQ, irq_request, irq);
        if (ret != 0) {
            soc_err("Set irq failed. (devid=%u; tsid=%u)\n", devid, tsid);
            return ret;
        }
    }

    return 0;
}

static int soc_platform_set_func_cq_irq(u32 devid, u32 tsid)
{
    struct res_inst_info inst;
    u32 irq, irq_request;
    int ret;

    ret = devdrv_get_ts_drv_irq_vector_id(devid, CQ_UPDATE_IRQ_NUM + 2, &irq); /* use msi-x index 16 + 2 in phy */
    if (ret != 0) {
        soc_info("Not support func cq irq. (devid=%u; tsid=%u)\n", devid, tsid);
        return 0;
    }

    ret = devdrv_get_irq_vector(devid, irq, &irq_request);
    if (ret != 0) {
        soc_err("Get func irq vector failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, tsid);
    ret = soc_resmng_set_irq_num(&inst, TS_FUNC_CQ_IRQ, 1);
    if (ret != 0) {
        soc_err("Set func irq_num failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = soc_resmng_set_irq_by_index(&inst, TS_FUNC_CQ_IRQ, 0, irq_request);
    if (ret != 0) {
        soc_err("Set func irq failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = soc_resmng_set_hwirq(&inst, TS_FUNC_CQ_IRQ, irq_request, irq);
    if (ret != 0) {
        soc_err("Set func irq failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    soc_info("Set func cq irq success. (devid=%u; tsid=%u)\n", devid, tsid);

    return 0;
}

static int soc_platform_set_irq(u32 devid, u32 tsid)
{
    int ret = soc_platform_set_mb_irq(devid, tsid);
    ret |= soc_platform_set_sq_send_trigger_irq(devid, tsid);
    ret |= soc_platform_set_cq_update_irq(devid, tsid);
    ret |= soc_platform_set_cdqm_f2nf_irq(devid, tsid);
    ret |= soc_platform_set_func_cq_irq(devid, tsid);
    return ret;
}

static int soc_platform_set_reg_base(u32 devid, u32 tsid)
{
    struct res_inst_info inst;
    struct soc_reg_base_info io_base;
    int ret;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, tsid);

    ret = devdrv_get_addr_info(devid, DEVDRV_ADDR_TS_DOORBELL, 0, (u64 *)&io_base.io_base, &io_base.io_base_size);
    if (ret != 0) {
        soc_err("Get db addr failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = soc_resmng_set_reg_base(&inst, "TS_DOORBELL_REG", &io_base);
    if (ret != 0) {
        soc_err("Set reg failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = devdrv_get_addr_info(devid, DEVDRV_ADDR_STARS_SQCQ_BASE, 0, (u64 *)&io_base.io_base, &io_base.io_base_size);
    if (ret != 0) {
        soc_err("Get db addr failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = soc_resmng_set_reg_base(&inst, "TS_STARS_RTSQ_SCHED_REG", &io_base);
    if (ret != 0) {
        soc_err("Set reg failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = devdrv_get_addr_info(devid, DEVDRV_ADDR_STARS_SQCQ_INTR_BASE, 0,
        (u64 *)&io_base.io_base, &io_base.io_base_size);
    if (ret != 0) {
        soc_err("Get stars sqcq int addr failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = soc_resmng_set_reg_base(&inst, "TS_STARS_CQINT_REG", &io_base);
    if (ret != 0) {
        soc_err("Set reg failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = devdrv_get_addr_info(devid, DEVDRV_ADDR_STARS_CDQM_BASE, 0, (u64 *)&io_base.io_base, &io_base.io_base_size);
    if (ret != 0) {
        soc_err("dev(%d) get addr info failed(%d).\n", devid, ret);
        return ret;
    }

    ret = soc_resmng_set_reg_base(&inst, "TS_STARS_CDQM_REG", &io_base);
    if (ret != 0) {
        soc_err("Set reg failed. (devid=%u; tsid=%u)\n", devid, 0);
        return ret;
    }

    ret = devdrv_get_addr_info(devid, DEVDRV_ADDR_STARS_INTR_BASE, 0, (u64 *)&io_base.io_base, &io_base.io_base_size);
    if (ret != 0) {
        soc_warn("Without stars intr. (devid=%u; ret=%d)\n", devid, ret);
    } else {
        ret = soc_resmng_set_reg_base(&inst, "TS_STARS_INT_REG", &io_base);
        if (ret != 0) {
            soc_err("Set reg failed. (devid=%u; tsid=%u)\n", devid, 0);
            return ret;
        }
    }

    ret = devdrv_get_addr_info(devid, DEVDRV_ADDR_TS_NOTIFY_TBL_BASE, 0, (u64 *)&io_base.io_base,
        &io_base.io_base_size);
    if (ret != 0) {
        soc_warn("Without notify table addr. (devid=%u; ret=%d)\n", devid, ret);
    } else {
        ret = soc_resmng_set_reg_base(&inst, "TS_STARS_NOTIFY_TBL_REG", &io_base);
        if (ret != 0) {
            soc_err("Set reg failed. (devid=%u; tsid=%u)\n", devid, 0);
            return ret;
        }
    }

    ret = devdrv_get_addr_info(devid, DEVDRV_ADDR_TS_EVENT_TBL_NS_BASE, 0, (u64 *)&io_base.io_base,
        &io_base.io_base_size);
    if (ret != 0) {
        soc_warn("Without event table addr. (devid=%u; ret=%d)\n", devid, ret);
    } else {
        ret = soc_resmng_set_reg_base(&inst, "TS_STARS_EVENT_TBL_NS_REG", &io_base);
        if (ret != 0) {
            soc_err("Set reg failed. (devid=%u; tsid=%u)\n", devid, 0);
            return ret;
        }
    }

    return 0;
}

static int soc_platform_set_rsv_mem(u32 devid, u32 tsid)
{
    struct res_inst_info inst;
    struct soc_rsv_mem_info rsv_mem;
    int ret;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, tsid);

    ret = devdrv_get_addr_info(devid, DEVDRV_ADDR_TS_SRAM, 0, (u64 *)&rsv_mem.rsv_mem, &rsv_mem.rsv_mem_size);
    if (ret == 0) {
        ret = soc_resmng_set_rsv_mem(&inst, "TS_SRAM_MEM", &rsv_mem);
        if (ret != 0) {
            soc_err("Set rsv_mem failed. (devid=%u; tsid=%u)\n", devid, tsid);
            return ret;
        }
    }

    ret = devdrv_get_addr_info(devid, DEVDRV_ADDR_TS_SQ_BASE, 0, (u64 *)&rsv_mem.rsv_mem, &rsv_mem.rsv_mem_size);
    if (ret == 0) {
        ret = soc_resmng_set_rsv_mem(&inst, "TS_SQCQ_MEM", &rsv_mem);
        if (ret != 0) {
            soc_err("Set rsv_mem failed. (devid=%u; tsid=%u)\n", devid, tsid);
            return ret;
        }
    }

    return 0;
}

static int soc_platform_init_instance(u32 devid)
{
    u32 tsid;
    int ret;

    tsid = 0;
    ret = soc_platform_set_irq(devid, tsid);
    ret |= soc_platform_set_reg_base(devid, tsid);
    ret |= soc_platform_set_rsv_mem(devid, tsid);
    ret |= soc_resmng_subsys_set_num(devid, TS_SUBSYS, 1);

    return ret;
}

#define SOC_PLATFORM_HOST_NOTIFIER "soc_platform"
static int soc_platform_host_notifier_func(u32 udevid, enum uda_notified_action action)
{
    int ret = 0;

    if (action == UDA_INIT) {
        ret = soc_platform_init_instance(udevid);
    }

    soc_info("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);

    return ret;
}

static int __init soc_platform_init(void)
{
    struct uda_dev_type type;
    int ret;
    uda_davinci_near_real_entity_type_pack(&type);
    ret = uda_notifier_register(SOC_PLATFORM_HOST_NOTIFIER, &type, UDA_PRI0, soc_platform_host_notifier_func);
    if (ret != 0) {
        soc_err("Reg notifier failed. (ret=%d)\n", ret);
        return ret;
    }

    uda_dev_type_pack(&type, UDA_DAVINCI, UDA_ENTITY, UDA_NEAR, UDA_REAL_SEC_EH);
    ret = uda_notifier_register(SOC_PLATFORM_HOST_NOTIFIER, &type, UDA_PRI0, soc_platform_host_notifier_func);
    if (ret != 0) {
        uda_davinci_near_real_entity_type_pack(&type);
        (void)uda_notifier_unregister(SOC_PLATFORM_HOST_NOTIFIER, &type);
        soc_err("Reg sec eh notifier failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}
module_init(soc_platform_init);

static void __exit soc_platform_exit(void)
{
    struct uda_dev_type type;

    uda_davinci_near_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(SOC_PLATFORM_HOST_NOTIFIER, &type);
    uda_dev_type_pack(&type, UDA_DAVINCI, UDA_ENTITY, UDA_NEAR, UDA_REAL_SEC_EH);
    (void)uda_notifier_unregister(SOC_PLATFORM_HOST_NOTIFIER, &type);
}
module_exit(soc_platform_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("soc platform driver");
#else
void soc_platform_stub_test(void)
{
}
#endif
