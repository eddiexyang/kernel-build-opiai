/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-01-03
 */
#ifdef CFG_FEATURE_CPUCORE_UNMASK
#include <linux/io.h>
#endif
#include "dfm_dev_register.h"
#include "dms_define.h"
#include "dms_sensor_type.h"
#include "soc_misc_dms_def.h"
#include "soc_misc_init.h"
#include "soc_misc_fault.h"
#include "soc_misc_err_info.h"
#include "drv_ras_common.h"
#include "ascend_kernel_hal.h"
#include "dfm_report.h"
#include "fpdc.h"
#include "cpucore_config.h"


#define CPUCORE_CACHE_LEVEL_OFFSET (16U)

#define CPUCORE_ERR_TYPE(cache_level, corrected) \
        ((((cache_level) << CPUCORE_CACHE_LEVEL_OFFSET) & 0x70000) | ((corrected) & 0x1))

#define CPUCORE_RAS_COVERAGE(sub_module, ce_flag, msg, ras_err) { \
    .subsys_id = DMS_DEV_TYPE_CPU_CORE,                           \
    .module_id = SOC_MISC_SENSOR_CPUCORE,                         \
    .section_type = RAS_SEC_ARM,                                  \
    .ras_code.int_status = CPUCORE_ERR_TYPE(sub_module, ce_flag), \
    .describe = msg,                                              \
    .sensor_type = DMS_SEN_TYPE_RAS_SENSOR,                       \
    .error_type = ras_err,                                        \
}

STATIC struct ras_fault_converge_item g_soc_misc_cpucore_converge_table[] = {
    CPUCORE_RAS_COVERAGE(SOC_MISC_IFU, SOC_MISC_RAS_ERR_CE, "parity error in IFU SRAM", RAS_ERROR_TYPE_PARITY),
    CPUCORE_RAS_COVERAGE(SOC_MISC_IFU, SOC_MISC_RAS_ERR_UC, "multi ECC err in IFU", RAS_ERROR_TYPE_MBECC),
    CPUCORE_RAS_COVERAGE(SOC_MISC_LSU, SOC_MISC_RAS_ERR_CE, "single-bit error in LSU", RAS_ERROR_TYPE_SBECCOverThold),
    CPUCORE_RAS_COVERAGE(SOC_MISC_LSU, SOC_MISC_RAS_ERR_UC, "multi-bit error in LSU", RAS_ERROR_TYPE_MBECC),
    CPUCORE_RAS_COVERAGE(SOC_MISC_MMU, SOC_MISC_RAS_ERR_CE, "parity error in MMU", RAS_ERROR_TYPE_PARITY),
    CPUCORE_RAS_COVERAGE(SOC_MISC_MMU, SOC_MISC_RAS_ERR_UC, "multi-bit error in MMU", RAS_ERROR_TYPE_MBECC),
    CPUCORE_RAS_COVERAGE(SOC_MISC_L2C, SOC_MISC_RAS_ERR_CE, "single-bit error in L2C", RAS_ERROR_TYPE_SBECCOverThold),
    CPUCORE_RAS_COVERAGE(SOC_MISC_L2C, SOC_MISC_RAS_ERR_UC, "multi-bit error in L2C", RAS_ERROR_TYPE_MBECC),
};

static struct dfm_struct g_soc_misc_cpucore_dms[SOC_MISC_MAX_DEV_NUM];

const struct ras_fault_converge_item *soc_misc_get_cpucore_converge_item(const struct ras_error *error_info)
{
    u32 num = sizeof(g_soc_misc_cpucore_converge_table) / sizeof(struct ras_fault_converge_item);
    return soc_misc_parse_table_handle(error_info, g_soc_misc_cpucore_converge_table, num);
}

int soc_misc_cpucore_ops_init(struct dms_node *device)
{
    soc_misc_drv_debug("soc_misc_cpucore_ops_init success.\n");
    return 0;
}

void soc_misc_cpucore_ops_uninit(struct dms_node *device)
{
    soc_misc_drv_debug("soc_misc_cpucore_ops_uninit success.\n");
    return;
}

#ifdef CFG_FEATURE_CPUCORE_UNMASK
struct ras_cpucore_unmask_hw_info {
    unsigned long long reg_addr;
    unsigned long long umask_offset;
    unsigned int unmask_val;
};

STATIC struct ras_cpucore_unmask_hw_info g_cpucore_unmask_hw_info[RAS_CPUCORE_MAX_NUM] = {
    { RAS_CPUCORE0_BASE_ADDR, RAS_CPUCORE0_UNMASK_OFFSET, RAS_CPUCORE0_UNMASK_VAL},
    { RAS_CPUCORE1_BASE_ADDR, RAS_CPUCORE1_UNMASK_OFFSET, RAS_CPUCORE1_UNMASK_VAL},
    { RAS_CPUCORE2_BASE_ADDR, RAS_CPUCORE2_UNMASK_OFFSET, RAS_CPUCORE2_UNMASK_VAL},
    { RAS_CPUCORE3_BASE_ADDR, RAS_CPUCORE3_UNMASK_OFFSET, RAS_CPUCORE3_UNMASK_VAL},
};
#endif

#define DEV_NODE_OFFSET 16
#define DEV_NODE_MASK 0xFFFF
#define SENSOR_TYPE_OFFSET (0)
#define SENSOR_TYPE_MASK   (0xFF)
#define SOC_MISC_DEVID_OFFSET    32
#define SOC_MISC_DEVID_MASK      0xFFFFFFFF

#define SENSOR_PRIV_DATA(_dev_id, _node_id, _sensor_type)               \
    (((u64)(_node_id & DEV_NODE_MASK) << DEV_NODE_OFFSET) |             \
    ((u64)(_sensor_type & SENSOR_TYPE_MASK) << SENSOR_TYPE_OFFSET) |    \
    ((u64)(_dev_id & SOC_MISC_DEVID_MASK) << SOC_MISC_DEVID_OFFSET))

STATIC int soc_scan_cpucore_event(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 dev_id;
    u32 node_id;
    u32 sensor_type;

    dev_id = (private_data >> SOC_MISC_DEVID_OFFSET) & SOC_MISC_DEVID_MASK;
    node_id = ((private_data >> DEV_NODE_OFFSET) & DEV_NODE_MASK);
    sensor_type = ((private_data >> SENSOR_TYPE_OFFSET) & SENSOR_TYPE_MASK);

    if (node_id >= g_soc_misc_cpucore_dms[dev_id].node_num) {
        soc_misc_drv_err("invalid node id(%u).\n", node_id);
        return -EINVAL;
    }
    (void)dfm_scan_events(&g_soc_misc_cpucore_dms[dev_id], node_id, sensor_type, data);
    return 0;
}

int soc_misc_cpucore_check_notify_data(const struct notify_data *pdata)
{
    if ((pdata == NULL) || (pdata->origin_data == NULL)) {
        soc_misc_drv_err("notify data is NULL\n");
        return -EINVAL;
    }

    if ((pdata->src_type != FPDC_SRC_RAS) || (pdata->node_type != DMS_DEV_TYPE_CPU_CORE)) {
        soc_misc_drv_err("invalid paramters: src_type(%u), node_type(%u)\n", pdata->src_type, pdata->node_type);
        return -EINVAL;
    }
    if (pdata->data_len < sizeof(struct cper_sec_proc_arm)) {
        soc_misc_drv_err("ras data is not too short, len(%u)\n", pdata->data_len);
        return -EINVAL;
    }
    return 0;
}

STATIC bool soc_misc_is_cpucore_error(u32 cache_level)
{
    return (cache_level <= SOC_MISC_L2C);
}

void soc_misc_cpucore_notify_sensor_scan(struct dfm_struct *dfm, u32 node_idx, u8 sensor_type)
{
    struct dfm_node *dev_node = &dfm->dev_nodes[node_idx];
    u32 i;
    for (i = 0; i < dev_node->sensor_num; ++i) {
        if (dev_node->sensors[i].dms_sensor.sensor_type == sensor_type) {
            /* notify the sensor to scan event immediately */
            if (dms_sensor_event_notify(0, &dev_node->sensors[i].dms_sensor) != 0) {
                soc_misc_drv_warn("send notify to dms sensor.\n");
            }
        }
    }
}

#ifdef CFG_FEATURE_CPUCORE_UNMASK
#define RAS_CPUCORE_UNMASK_REMAP_SIZE 0x4

STATIC bool soc_misc_cpucore_check_unmask(unsigned char cpucore_id, unsigned dev_id)
{
    devdrv_hardware_info_t hardware_info = {0};
    unsigned long long reg_phy_addr;
    void __iomem *reg_virt_addr = NULL;
    unsigned int unmask_val = g_cpucore_unmask_hw_info[cpucore_id].unmask_val;
    unsigned int i;
    bool is_mask = false;
    int ret;

    ret = hal_kernel_get_hardware_info(dev_id, &hardware_info);
    if (ret != 0) {
        soc_misc_drv_err("Failed to invoke hal_kernel_get_hardware_info. (devid=%u)\n", dev_id);
        return ret;
    }

    reg_phy_addr = hardware_info.phy_addr_offset +
        g_cpucore_unmask_hw_info[cpucore_id].reg_addr + g_cpucore_unmask_hw_info[cpucore_id].umask_offset;

    for (i = 0; i < RAS_INT_TYPE_NUM_MAX; i++) {
        if ((RAS_INT_TYPE_ENABLE_MASK & (0x01 << i)) != 0) {
            reg_virt_addr = ioremap(reg_phy_addr + (i * RAS_INT_TYPE_OFFSET), RAS_CPUCORE_UNMASK_REMAP_SIZE);
            if (reg_virt_addr == NULL) {
                soc_misc_drv_err("Failed to ioremap cpucore unmask irq register. (devid=%u)\n", dev_id);
                return false;
            }

            unmask_val = readl(reg_virt_addr);
            unmask_val = unmask_val & g_cpucore_unmask_hw_info[cpucore_id].unmask_val;

            iounmap(reg_virt_addr);
            reg_virt_addr = NULL;
            is_mask = (unmask_val != g_cpucore_unmask_hw_info[cpucore_id].unmask_val) ? true : false;
            if (is_mask) {
                break;
            }
        }
    }
    return is_mask;
}

STATIC void write_cpucore_reg32(unsigned int dev_id, unsigned long long reg_phy_addr, unsigned int value)
{
    (void)ras_int_unmask_reg_write(reg_phy_addr, value, value);
    soc_misc_drv_event("Unmask cpucore local ras irq success. (devid=%u)\n", dev_id);
}

STATIC void soc_misc_cpucore_fault_unmask_multi_type(unsigned int dev_id,
    unsigned long long reg_addr, unsigned int reg_val)
{
    unsigned int i;

    for (i = 0; i < RAS_INT_TYPE_NUM_MAX; i++) {
        if ((RAS_INT_TYPE_ENABLE_MASK & (0x01 << i)) != 0) {
            write_cpucore_reg32(dev_id, reg_addr + (i * RAS_INT_TYPE_OFFSET), reg_val);
        }
    }
}

void soc_misc_cpucore_fault_unmask_task(struct work_struct *work)
{
    struct dfm_node *dev_node = container_of(work, struct dfm_node, unmask_irq_work.work);
    devdrv_hardware_info_t hardware_info = {0};
    unsigned long long reg_phy_addr;
    int ret;
    unsigned char node_id = dev_node->node_id;

    ret = hal_kernel_get_hardware_info(dev_node->devid, &hardware_info);
    if (ret != 0) {
        soc_misc_drv_err("Failed to invoke hal_kernel_get_hardware_info. (devid=%u; ret=%d)\n", dev_node->devid, ret);
        return;
    }

    if (soc_misc_cpucore_check_unmask(node_id, dev_node->devid)) {
        reg_phy_addr = hardware_info.phy_addr_offset +
            g_cpucore_unmask_hw_info[node_id].reg_addr + g_cpucore_unmask_hw_info[node_id].umask_offset;
        soc_misc_cpucore_fault_unmask_multi_type(dev_node->devid, reg_phy_addr,
                                                 g_cpucore_unmask_hw_info[node_id].unmask_val);
    }
}

STATIC void cpucore_relieve_suppresion(unsigned char dev_id, unsigned int node_id)
{
    struct dfm_node *dev_node = &g_soc_misc_cpucore_dms[dev_id].dev_nodes[node_id];
    /* check whether the cpu core node is masked */
    if (soc_misc_cpucore_check_unmask(node_id, dev_id)) {
        (void)schedule_delayed_work(&dev_node->unmask_irq_work, msecs_to_jiffies(300000)); /* 300000:5 minutes */
    }
}
#endif /* CFG_FEATURE_CPUCORE_UNMASK */

void soc_misc_cpucore_fault_handler(const struct notify_data *pdata)
{
    struct ras_error error_info = {0};
    struct cper_sec_proc_arm *pdata_ras = NULL;
    struct cper_arm_err_info *err_info = NULL;
    struct cper_arm_cache_err *cache_err = NULL;
    const dfm_event *event =  NULL;
    u8 cluster_id = 0;
    u16 idx;
    u32 node_id = 0;
    int ret;

    if (soc_misc_cpucore_check_notify_data(pdata)) {
        soc_misc_drv_err("fault notify data invalid.\n");
        return;
    }

    pdata_ras = (struct cper_sec_proc_arm *)pdata->origin_data;
    err_info = (struct cper_arm_err_info *)(pdata->origin_data + sizeof(struct cper_sec_proc_arm));
    for (idx = 0; idx < pdata_ras->err_info_num; idx++) {
        err_info += idx;
        cache_err = (struct cper_arm_cache_err *)(&err_info->error_info);
        if (!soc_misc_is_cpucore_error((u32)cache_err->cache_level)) {
            soc_misc_drv_debug("the cache level [%u] is not in cpu core\n", idx);
            continue;
        }

        /* Prevent the BIOS from obtaining the correct cluster ID due to chip exceptions.
         * If the cluster ID is invalid, use the first CPU cluster to report the fault.
         */
        ret = soc_misc_get_cpu_cluster_id(pdata_ras, &cluster_id);
        if (ret != 0) {
            soc_misc_drv_warn("get cluster failed. ret=%d, cluster_id: %u\n", ret, cluster_id);
            cluster_id = 0;
        }
        node_id = cluster_id;
        error_info.device_id = pdata->chip_id;
        error_info.sensor_id = SOC_MISC_SENSOR_CPUCORE;
        error_info.module_id = SOC_MISC_SENSOR_CPUCORE;
        error_info.sub_node_id = cluster_id;
        error_info.ras_code = CPUCORE_ERR_TYPE(cache_err->cache_level, cache_err->corrected);
        error_info.ras_err_severity = cache_err->cache_level;
        error_info.sec_type = RAS_SEC_ARM;
        event = soc_misc_get_cpucore_converge_item(&error_info);
        if (event == NULL) {
            soc_misc_drv_warn("cpucore[%d] get converge item failed.\n", cluster_id);
            continue;
        }
        ret = dfm_add_event(&g_soc_misc_cpucore_dms[error_info.device_id], node_id, event);
        if (ret != 0) {
            if (ret != -EEXIST) {
                soc_misc_drv_err("dfm_add_event failed.  ret = %d .\n", ret);
                return;
            }
#if (defined CFG_SOC_PLATFORM_MDC_V51) || (defined CFG_SOC_PLATFORM_MDC_V11)
        } else {
            soc_misc_cpucore_notify_sensor_scan(&g_soc_misc_cpucore_dms[error_info.device_id], node_id,
                event->sensor_type);
#endif
        }
    }
#ifdef CFG_FEATURE_CPUCORE_UNMASK
    cpucore_relieve_suppresion(error_info.device_id, node_id);
#endif
    return;
}

int soc_misc_cpucore_register(u32 dev_id)
{
    struct dms_sensor_object_cfg sensor_cfg[] = {
        SOC_MISC_SENOR_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "CPUCore", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, soc_scan_cpucore_event,
            SENSOR_PRIV_DATA(dev_id, 0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
    };
    u32 sensor_num = (u32)ARRAY_SIZE(sensor_cfg);
    u32 sensor_idx;
    int i;

    g_soc_misc_cpucore_dms[dev_id].dev_id = dev_id;

    if (dfm_struct_init(&g_soc_misc_cpucore_dms[dev_id], SOC_MISC_CPUCORE_NODE_NUM, sensor_num) != 0) {
        soc_misc_drv_err("init soc_misc_cpucore dfm data failed.\n");
        goto _fail;
    }

    for (i = 0; i < (int)SOC_MISC_CPUCORE_NODE_NUM; ++i) {
        g_soc_misc_cpucore_dms[dev_id].dev_nodes[i].node = &g_soc_misc_cpucore_dms_nodes[dev_id][i];
        g_soc_misc_cpucore_dms[dev_id].dev_nodes[i].post_proc = NULL;
        g_soc_misc_cpucore_dms[dev_id].dev_nodes[i].fpdc_notify = soc_misc_cpucore_fault_handler;
        g_soc_misc_cpucore_dms[dev_id].dev_nodes[i].get_converage_node = NULL;
#ifdef CFG_FEATURE_CPUCORE_UNMASK
        INIT_DELAYED_WORK(&g_soc_misc_cpucore_dms[dev_id].dev_nodes[i].unmask_irq_work,
            soc_misc_cpucore_fault_unmask_task);
#endif
        for (sensor_idx = 0; sensor_idx < sensor_num; ++sensor_idx) {
            sensor_cfg[sensor_idx].private_data = SENSOR_PRIV_DATA(dev_id, i, sensor_cfg[sensor_idx].sensor_type);
            g_soc_misc_cpucore_dms[dev_id].dev_nodes[i].sensors[sensor_idx].dms_sensor = sensor_cfg[sensor_idx];
        }
    }
    if (dfm_register_nodes(&g_soc_misc_cpucore_dms[dev_id]) != 0) {
        soc_misc_drv_err("register soc_misc_cpucore dms node failed. (dev_id=%u)\n", dev_id);
        goto _fail;
    }

    return 0;

_fail:
#ifdef CFG_FEATURE_CPUCORE_UNMASK
    for (i = 0; i < (int)SOC_MISC_CPUCORE_NODE_NUM; ++i) {
        (void)cancel_delayed_work_sync(&g_soc_misc_cpucore_dms[dev_id].dev_nodes[i].unmask_irq_work);
    }
#endif
    dfm_struct_final(&g_soc_misc_cpucore_dms[dev_id], SOC_MISC_CPUCORE_NODE_NUM, 1);
    return -EFAULT;
}

void soc_misc_cpucore_unregister(void)
{
    u32 dev_id;
    for (dev_id = 0; dev_id < SOC_MISC_MAX_DEV_NUM; dev_id++) {
        if (g_soc_misc_cpucore_dms[dev_id].node_num > 0) {
            dfm_unregister_nodes(&g_soc_misc_cpucore_dms[dev_id]);
            dfm_struct_final(&g_soc_misc_cpucore_dms[dev_id], SOC_MISC_CPUCORE_NODE_NUM, 1);
        }
    }
}
