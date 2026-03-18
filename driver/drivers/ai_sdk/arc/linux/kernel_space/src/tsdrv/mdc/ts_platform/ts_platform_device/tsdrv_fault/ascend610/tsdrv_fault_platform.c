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
#include <linux/io.h>
#include "tsdrv_fault_init.h"
#include "tsdrv_log.h"

#define SENSOR_TABLE_OBJ(_devid) \
    { \
        /* HWTS */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "hwts_ras", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_HWTS, \
            SENSOR_PRIV_DATA(_devid, 0, DEV_NODE_HWTS, SENSOR_NODE_HWTS), 0xFFF, 0xFBF), \
        /* A55 */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "a55_ras", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_A55, \
            SENSOR_PRIV_DATA(_devid, 0, DEV_NODE_A55, SENSOR_NODE_A55), 0xFFF, 0xFBF), \
        /* AIC */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "aic_ras", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_AIC, \
            SENSOR_PRIV_DATA(_devid, 0, DEV_NODE_AIC, SENSOR_NODE_AIC), 0xFFF, 0xFBF), \
        /* SDMAM */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "sdma_ras", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_SDMA, \
            SENSOR_PRIV_DATA(_devid, 0, DEV_NODE_SDMAM, SENSOR_NODE_SDMAM), 0xFFF, 0xFBF), \
        /* TS0 HEARTBEAT */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_HEARTBEAT, "ts0_heartbeat", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_HWTS, \
            SENSOR_PRIV_DATA(_devid, 0, DEV_NODE_TSC, SENSOR_NODE_HEARTBEAT), 0xFFF, 0xFBF), \
        /* TS1 HEARTBEAT */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_HEARTBEAT, "ts1_heartbeat", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_HWTS, \
            SENSOR_PRIV_DATA(_devid, 1, DEV_NODE_TSV, SENSOR_NODE_HEARTBEAT), 0xFFF, 0xFBF), \
        /* AIV */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "aiv_ras", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_AIC, \
            SENSOR_PRIV_DATA(_devid, 0, DEV_NODE_AIV, SENSOR_NODE_AIV), 0xFFF, 0xFBF), \
        /* Invalid sensor obj */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
    }

static struct dms_sensor_object_cfg g_sensor_obj_table[TSDRV_MAX_DAVINCI_NUM][DMS_MAX_NODE_SENSOR_COUNT] = {
    SENSOR_TABLE_OBJ(0x0ULL), /* dev0 */
    SENSOR_TABLE_OBJ(0x1ULL), /* dev1: dc only */
};

struct dms_sensor_object_cfg *tsdrv_get_sensor_obj_table(u32 devid)
{
    return g_sensor_obj_table[devid];
}

#ifdef CFG_FEATURE_FAULT_FPDC
bool tsdrv_fault_is_need_converged(DMS_DEVICE_NODE_TYPE node_type)
{
    return (node_type == DMS_DEV_TYPE_L2BUF);
}

/* offset between devices */
#define DEVICE_OFFSET 0x8000000000

#define L2BUF_UNMASK_IRQ_RES_OFF   0x2008ULL
#define L2BUF_UNMASK_IRQ_RES_SZ    0x1
#define L2BUF_UNMASK_IRQ_RES_VAL   0x50DU

/* L2buffer RAS common base address, control 16 l2buffer modules in each device */
#define L2BUF_COMMON_BASE_ADDR 0xA0F20000U

static void __iomem *l2buf_io_base[TSDRV_MAX_DAVINCI_NUM];

void fault_unmask_l2buf_irq(struct work_struct *work)
{
#ifndef TSDRV_UT
    struct fault_dev *dev = container_of(work, struct fault_dev, unmask_irq_work.work);
    writel(L2BUF_UNMASK_IRQ_RES_VAL, (volatile void *)l2buf_io_base[dev->devid]);
    TSDRV_PRINT_INFO("Unmask l2buf irq success. (devid=%u)\n", dev->devid);
#endif
}
int fault_remap_l2buf(u32 devid)
{
#ifndef TSDRV_UT
    u64 res_phy_addr = L2BUF_COMMON_BASE_ADDR + L2BUF_UNMASK_IRQ_RES_OFF + DEVICE_OFFSET * devid;

    l2buf_io_base[devid] = ioremap(res_phy_addr, L2BUF_UNMASK_IRQ_RES_SZ);
    if (l2buf_io_base[devid] == NULL) {
        TSDRV_PRINT_ERR("Failed to ioremap unmask irq register. (devid=%u)\n", devid);
        return -EFAULT;
    }
    TSDRV_PRINT_INFO("Remap l2buf address success. (devid=%u)\n", devid);
    return 0;
#endif
}
void fault_unmap_l2buf(u32 devid)
{
#ifndef TSDRV_UT
    if (l2buf_io_base[devid] != NULL) {
        iounmap(l2buf_io_base[devid]);
        l2buf_io_base[devid] = NULL;
    }
    TSDRV_PRINT_INFO("Unmask l2buf success. (devid=%u)\n", devid);
#endif
}
#endif

