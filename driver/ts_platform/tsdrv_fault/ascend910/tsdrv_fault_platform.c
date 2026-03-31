/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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
#include "tsdrv_common_fault_platform.h"
#include "tsdrv_log.h"
#include "tsdrv_fault_comm.h"
#include "tsdrv_tscpu_fault.h"
#include "tsdrv_fault_platform.h"

#ifdef CFG_FEATURE_FAULT_FPDC
static struct ras_fault_converge_item fault_converge_table[] = {
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TSCPU, DMS_DEV_TYPE_TSCPU, RAS_SEC_OEM, RAS_CODE_TSCPU,
        "tscpu module err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_OEM, RAS_L2BUF_SRAM_MULTI_BIT_ECC_UER,
        "l2buf multi bit ecc err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_OEM, RAS_L2BUF_SRAM_SINGLE_BIT_ECC_CE,
        "l2buf single bit ecc err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_OEM, RAS_L2BUF_CFG_0,
        "l2buf invalid security attr", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_OEM, RAS_L2BUF_CFG_1,
        "l2buf invalid access request", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_OEM, RAS_L2BUF_CFG_2,
        "l2buf invalid read request", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_OEM, RAS_L2BUF_CFG_3,
        "l2buf invalid write request", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_OEM, RAS_L2BUF_CFG_4,
        "l2buf invalid access thr chan", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_OEM, RAS_L2BUF_CFG_5,
        "l2buf debug invalid access", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_OEM, RAS_L2BUF_CFG_6,
        "l2buf access addr out-of-bound", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_OEM, RAS_L2BUF_CFG_7,
        "l2buf invalid rsv request", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_OEM, RAS_L2BUF_CFG_8,
        "l2buf request attr err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_OEM, RAS_L2BUF_CFG_9,
        "l2buf access uninitialized", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_OEM, RAS_L2BUF_CFG_10,
        "l2buf atomic count err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
};

struct ras_fault_converge_item *tsdrv_get_fault_converge_table(void)
{
    return fault_converge_table;
}

int tsdrv_get_fault_converge_table_size(void)
{
    return (int)ARRAY_SIZE(fault_converge_table);
}
#endif

#define SENSOR_TABLE_OBJ(_devid) \
    { \
        /* HWTS */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "hwts", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_HWTS, \
            SENSOR_PRIV_DATA(_devid, 0, DMS_DEV_TYPE_HWTS_S_TS, DEV_NODE_HWTS_STARS, SENSOR_NODE_HWTS_STARS), \
            0xFFF, 0xFBF), \
        /* TS doorbell */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "ts", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_TS, \
            SENSOR_PRIV_DATA(_devid, 0, DMS_DEV_TYPE_TS, DEV_NODE_TS, SENSOR_NODE_TS), 0xFFF, 0xFBF), \
        /* AIC */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "aic", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_AIC, \
            SENSOR_PRIV_DATA(_devid, 0, DMS_DEV_TYPE_AIC, DEV_NODE_AIC, SENSOR_NODE_AIC), 0xFFF, 0xFBF), \
        /* SDMAM */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "sdma", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_SDMA, \
            SENSOR_PRIV_DATA(_devid, 0, DMS_DEV_TYPE_SDMA, DEV_NODE_SDMAM, SENSOR_NODE_SDMAM), 0xFFF, 0xFBF), \
        /* Invalid sensor obj */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
    }

static struct dms_sensor_object_cfg g_sensor_obj_table[TSDRV_MAX_DAVINCI_NUM][DMS_MAX_NODE_SENSOR_COUNT] = {
    SENSOR_TABLE_OBJ(0x0ULL), /* dev0 */
    SENSOR_TABLE_OBJ(0x1ULL), /* dev1 */
    SENSOR_TABLE_OBJ(0x2ULL), /* dev2 */
    SENSOR_TABLE_OBJ(0x3ULL), /* dev3 */
};

struct dms_sensor_object_cfg *tsdrv_get_sensor_obj_table(u32 devid)
{
    return g_sensor_obj_table[devid];
}

#ifdef CFG_FEATURE_FAULT_FPDC
void tsdrv_fill_tscpu_sensor_status(struct ras_error *error)
{
    error->node_id = 0;
    error->section_type = RAS_SEC_OEM;
    error->ras_code = RAS_CODE_TSCPU;
    error->sensor_status = RAS_ERROR_TYPE_PARITY;
}

/* offset between devices */
#define DEVICE_OFFSET 0x200000000000

#define L2BUF_UNMASK_IRQ_RES_OFF   0x2008ULL
#define L2BUF_UNMASK_IRQ_RES_SZ    0x20
#define L2BUF_UNMASK_IRQ_RES_VAL   0x50DU

/* L2buffer RAS common base address, control 16 l2buffer modules in each device */
#define L2BUF_COMMON_BASE_ADDR 0x898A0000

static void __iomem *l2buf_io_base[TSDRV_MAX_DAVINCI_NUM];
bool fault_is_enable_unmask_ras_irp(u32 devid)
{
    return true;
}

void fault_unmask_ras_irq(struct work_struct *work)
{
    struct dfm_node *node = container_of(work, struct dfm_node, unmask_irq_work.work);
    u32 devid = node->devid;
    int node_type = node->node_type;

    if (node_type == DMS_DEV_TYPE_L2BUF) {
        writel(L2BUF_UNMASK_IRQ_RES_VAL, (volatile void *)l2buf_io_base[node->devid]);
        TSDRV_PRINT_DEBUG("Unmask l2buf irq success. (devid=%u)\n", node->devid);
    }

    if (node_type == DMS_DEV_TYPE_TSCPU) {
        TSDRV_PRINT_DEBUG("Unmask tscpu irq. (devid=%u; node_type=0x%x)\n", devid, node_type);
    }
}

int fault_remap_l2buf(u32 devid)
{
    u64 res_phy_addr = L2BUF_COMMON_BASE_ADDR + L2BUF_UNMASK_IRQ_RES_OFF + DEVICE_OFFSET * devid;

    l2buf_io_base[devid] = ioremap(res_phy_addr, L2BUF_UNMASK_IRQ_RES_SZ);
    if (l2buf_io_base[devid] == NULL) {
        TSDRV_PRINT_ERR("Failed to ioremap unmask irq register. (devid=%u)\n", devid);
        return -EFAULT;
    }
    TSDRV_PRINT_INFO("Remap l2buf address success. (devid=%u)\n", devid);
    return 0;
}

void fault_unmap_l2buf(u32 devid)
{
    if (l2buf_io_base[devid] != NULL) {
        iounmap(l2buf_io_base[devid]);
        l2buf_io_base[devid] = NULL;
    }
    TSDRV_PRINT_INFO("Unmap l2buf success. (devid=%u)\n", devid);
}

static struct tsdrv_ras_node_info ras_nodes_info[] = {
    {DMS_DEV_TYPE_L2BUF, DMS_DEV_TYPE_L2BUF, L2BUFF_NUM},
    {DMS_DEV_TYPE_TSCPU, DMS_DEV_TYPE_TSCPU, TSCPU_NUM}
};

void tsdrv_get_ras_node_types(struct tsdrv_ras_node_info **ras_nodes, int *num)
{
    *ras_nodes = ras_nodes_info;
    *num = ARRAY_SIZE(ras_nodes_info);
}

static struct dfm_struct l2buff_dms[TSDRV_MAX_DAVINCI_NUM];
static struct dfm_struct *tsdrv_get_l2buff_dms(u32 devid)
{
    return &l2buff_dms[devid];
}

static struct dms_node g_l2buff_dms_nodes[TSDRV_MAX_DAVINCI_NUM][L2BUFF_NUM];
static struct dms_node *tsdrv_get_l2buff_dms_nodes(u32 devid, u32 node_id)
{
    return &g_l2buff_dms_nodes[devid][node_id];
}

static struct dms_sensor_object_cfg g_l2buff_sensor_cfg[] = {
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "l2buff", DMS_DISCRETE_SENSOR_CLASS,
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL,
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, tsdrv_ras_event_scan,
        SENSOR_PRIV_DATA(0, 0, 0, 0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
};
static struct dms_sensor_object_cfg *tsdrv_get_l2buff_sensor_cfg(u32 *sensor_num)
{
    *sensor_num = sizeof(g_l2buff_sensor_cfg) / sizeof(g_l2buff_sensor_cfg[0]);
    return g_l2buff_sensor_cfg;
}

struct dfm_struct *tsdrv_get_dfm_by_node_type(u32 devid, int node_type)
{
    switch (node_type) {
        case DMS_DEV_TYPE_L2BUF:
            return tsdrv_get_l2buff_dms(devid);
        case DMS_DEV_TYPE_TSCPU:
            return tsdrv_get_tscpu_dms(devid);
        default:
            break;
    }

    return NULL;
}


struct dms_node *tsdrv_get_dms_node(u32 devid, int node_type, u32 node_id)
{
    switch (node_type) {
        case DMS_DEV_TYPE_L2BUF:
            return tsdrv_get_l2buff_dms_nodes(devid, node_id);
        case DMS_DEV_TYPE_TSCPU:
            return tsdrv_get_tscpu_dms_nodes(devid, node_id);
        default:
            break;
    }

    return NULL;
}

struct dms_sensor_object_cfg *tsdrv_get_sensor_cfg(int node_type, u32 *sensor_num)
{
    switch (node_type) {
        case DMS_DEV_TYPE_L2BUF:
            return tsdrv_get_l2buff_sensor_cfg(sensor_num);
        case DMS_DEV_TYPE_TSCPU:
            return tsdrv_get_tscpu_sensor_cfg(sensor_num);
        default:
            *sensor_num = 0;
            return NULL;
    }
}
#endif

