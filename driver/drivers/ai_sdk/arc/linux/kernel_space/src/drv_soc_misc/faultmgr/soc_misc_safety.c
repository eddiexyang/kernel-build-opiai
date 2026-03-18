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
#include <linux/slab.h>

#include "soc_misc_safety.h"
#include "drvfault_common.h"
#include "drvfault_user_common.h"
#include "soc_misc_init.h"
#include "soc_misc_fault.h"
#include "soc_misc_err_info.h"
#include "dms_node_type.h"
#include "dms_sensor.h"
#include "soc_misc_mn.h"
#include "soc_misc_cpucluster.h"
#include "soc_misc_safety_conf.h"
#include "dfm_safety_report.h"
#include "dfm_dev_register.h"
#include "drv_notify.h"
#include "dms_node_type.h"

/**
 * safety fault src id
 * every fault has it's own src id
 */
#define EMU_SUB_IRQ_GRP_SIZE     (32u)
#define L3D_SAFETY_ERR_SRC_ID    (u32)((SUB_SRC_ID_17 / EMU_SUB_IRQ_GRP_SIZE) + 1)
#define L3T_SAFETY_ERR_SRC_ID    (u32)((SUB_SRC_ID_16 / EMU_SUB_IRQ_GRP_SIZE) + 1)
#define L3D_SAFETY_ERR_BIT_ID    (u32)(SUB_SRC_ID_17 % EMU_SUB_IRQ_GRP_SIZE)
#define L3T_SAFETY_ERR_BIT_ID    (u32)(SUB_SRC_ID_16 % EMU_SUB_IRQ_GRP_SIZE)

#define L3D_SAFETY_INT_BIT_ID    (u32)(SUB_SRC_ID_1 % EMU_SUB_IRQ_GRP_SIZE)
#define L3T_SAFETY_INT_BIT_ID    (u32)(SUB_SRC_ID_0 % EMU_SUB_IRQ_GRP_SIZE)

/**
 * These values must match the index of subsys_irq_vector.
 */

#define SOC_MISC_SAFETY_EVENT_MAX (32U)

#define NUM_TO_STR(num) #num

struct soc_misc_safety_irq_infos {
    u32 soc_platform;
    u32 irq_info_num;
    struct safety_irq_info *irq_infos;
};

STATIC struct soc_misc_safety_irq_infos g_safety_irq_table[] = {
#ifdef CFG_FEATURE_SAFETY_MANAGER
    { CHIP_TYPE_MDC_BS9SX1A, 2, g_int_fhi_mn },
#endif
};

int soc_misc_register_safety_notifier(struct safety_irq_info *safety_irq, unsigned int len)
{
    struct drv_soc_misc_ctrl *soc_misc_ctl = soc_misc_get_ctrl();
    int dev_id, irq_idx;
    int ret;

    if (soc_misc_ctl == NULL) {
        soc_misc_drv_err("soc misc ctrl has not initilized.\n");
        return -EFAULT;
    }

    for (dev_id = 0; dev_id < (int)soc_misc_ctl->dev_num; dev_id++) {
        for (irq_idx = 0; irq_idx < (int)len; irq_idx++) {
            /* RAS fault's interrupt is recept by BIOS, but need register
             * callback function to fault manager. when safety type fault
             * occur, we need to get fault status from fault manager */
            if ((safety_irq[irq_idx].irq_type != SAFETY_IRQ_TYPE_ARM_RAS) &&
                (drvfault_find_irq_in_dts(safety_irq[irq_idx].irq_name) != 0)) {
                soc_misc_drv_info("skip registration because irq is not found in dts.(irq_name=%.*s)\n",
                    NAME_LEN, safety_irq[irq_idx].irq_name);
                continue;
            }

            ret = drvfault_register_safety_irq_info(&safety_irq[irq_idx]);
            if (ret != 0) {
                soc_misc_drv_err("register safety fault notifier failed. devid(%u),"
                    "irq_index(%u), ret(%d)\n", dev_id, irq_idx, ret);
            }
        }
    }
    return 0;
}

u8 soc_misc_get_node_type(u8 sensor_id)
{
    switch (sensor_id) {
        case SOC_MISC_SENSOR_CPUCORE:
            return DMS_DEV_TYPE_CPU_CORE;
        case SOC_MISC_SENSOR_RING:
            return DMS_DEV_TYPE_RING;
        case SOC_MISC_SENSOR_MN:
            return DMS_DEV_TYPE_MN;
        case SOC_MISC_SENSOR_RBRG:
            return DMS_DEV_TYPE_RBRG;
        default:
            return DMS_DEV_TYPE_MAX;
    }
}


const struct ras_fault_converge_item *soc_misc_report_safety_fault(
    u32 devid, u8 cluster_id, u8 sensor_id, u32 irq_status_bit)
{
    const struct ras_fault_converge_item *converge_node = NULL;
    struct ras_error error_info = { 0 };

    error_info.device_id = devid;
    error_info.sensor_id = sensor_id;
    error_info.module_id = soc_misc_get_node_type(sensor_id);
    error_info.sub_node_id = cluster_id;
    error_info.ras_err_type = 0;
    error_info.ras_err_severity = 0;
    error_info.sec_type = SOC_ERR_SECTOR_SAFETY;

    error_info.ras_code = irq_status_bit;
    converge_node = soc_misc_parse_table(&error_info, sensor_id);
    if (converge_node == NULL) {
        soc_misc_drv_warn("Unknow fault. bit_index = %u.\n", irq_status_bit);
        return NULL;
    }
    soc_misc_fault_event_handler(&error_info, sizeof(struct ras_error), sensor_id);
    return converge_node;
}

void soc_misc_report_ras_fault(u32 devid, u8 cluster_id, u8 sensor_id, u32 ras_ierr)
{
    const struct ras_fault_converge_item *converge_node = NULL;
    struct ras_error error_info = { 0 };

    error_info.device_id = devid;
    error_info.sensor_id = sensor_id;
    error_info.module_id = soc_misc_get_node_type(sensor_id);
    error_info.sub_node_id = cluster_id;
    error_info.ras_err_type = 0;
    error_info.ras_err_severity = 0;
    error_info.sec_type = SOC_ERR_SECTOR_RAS;

    error_info.ras_code = ras_ierr;
    converge_node = soc_misc_parse_table(&error_info, sensor_id);
    if (converge_node == NULL) {
        soc_misc_drv_warn("Unknow fault. bit_index = %u.\n", ras_ierr);
        return;
    }
    soc_misc_fault_event_handler(&error_info, sizeof(struct ras_error), sensor_id);
    return;
}

u8 soc_misc_get_node_id_by_emu(unsigned int emu_id, unsigned int src_id, unsigned int bit_id)
{
    if ((emu_id >= CPU_CLUSTER0_EMU_ID) &&
        (emu_id <= (CPU_CLUSTER0_EMU_ID + SOC_MISC_MAX_NODE_NUM))) {
        return (u8)(emu_id - CPU_CLUSTER0_EMU_ID);
    } else if (emu_id == AO_SUB_EMU_ID) {
        if (bit_id == AO_MN_INT_SAFETY_BIT_ID || bit_id == AO_MN_ERR_SAFETY_BIT_ID) {
            return 0;
        }
        return 0;
    } else if (emu_id == IO_SUB_EMU_ID) {
        if (bit_id == IO_MN_INT_SAFETY_BIT_ID || bit_id == IO_MN_ERR_SAFETY_BIT_ID) {
            return 0x1;   /* MN-A0's node id is 0, MN-IO's node id is 1 */
        }
        return 0;
    } else {
        return SOC_MISC_MAX_NODE_NUM;
    }
}

u8 get_cpu_cluster_sensor_id(unsigned int bit_id)
{
    if (bit_id == CPU_CLUSTER_INT_SAFETY_BIT_ID || bit_id == CPU_CLUSTER_ERR_SAFETY_BIT_ID) {
        return SOC_MISC_SENSOR_CPUCORE;
    } else {
        return SOC_MISC_SENSOR_ID_MAX;
    }
}

/**
 * get sensor id by src id and bit id
 *
 * Currently, the ASCEND610 can distinguish the sensor type based on the BIT ID.
 * But The correct method is to distinguish the sensor type based on the
 * src ID and bit ID. This prevents the sensor type identified as same
 * when the src ID and bit ID are different.
 */
u8 soc_misc_get_sensor_id_by_src_id(unsigned int emu_id, unsigned int src_id, unsigned int bit_id)
{
    if ((emu_id >= CPU_CLUSTER0_EMU_ID) && (emu_id <= (CPU_CLUSTER0_EMU_ID + SOC_MISC_MAX_NODE_NUM))) {
        return get_cpu_cluster_sensor_id(bit_id);
    }

    if (emu_id == AO_SUB_EMU_ID) {
        if (bit_id == AO_MN_INT_SAFETY_BIT_ID || bit_id == AO_MN_ERR_SAFETY_BIT_ID) {
            return SOC_MISC_SENSOR_MN;
        } else {
            return SOC_MISC_SENSOR_ID_MAX;
        }
    }

    if (emu_id == IO_SUB_EMU_ID) {
        if (bit_id == IO_MN_INT_SAFETY_BIT_ID || bit_id == IO_MN_ERR_SAFETY_BIT_ID) {
            return SOC_MISC_SENSOR_MN;
        } else {
            return SOC_MISC_SENSOR_ID_MAX;
        }
    }

    return SOC_MISC_SENSOR_ID_MAX;
}

int soc_misc_set_fault_event(struct safety_event *fault_event,
    u32 devid, u8 clustrer_id, u8 sensor_id, u8 sensor_type, int error_type)
{
    static u8 g_event_serial = 0;
    unsigned int event_severity = 0;

    fault_event->node_type = soc_misc_get_node_type(sensor_id);
    if (fault_event->node_type >= DMS_DEV_TYPE_MAX) {
        soc_misc_drv_err("invalid not type.\n");
        return -EINVAL;
    }

    fault_event->sensor_type = sensor_type;
    fault_event->event_type = error_type;
    fault_event->node_id = clustrer_id;  /* the index of devices */
    fault_event->sub_node_type = 0;
    fault_event->sub_node_id = 0;
    if (dms_get_event_severity(fault_event->node_type, sensor_type, error_type, &event_severity) != 0) {
        soc_misc_drv_warn("event severity can't be found.(sensor_type=%u, err_type=%u)\n",
            DMS_SEN_TYPE_RAS_SENSOR, error_type);
    }
    fault_event->event_severity = (u8)event_severity;
    fault_event->event_assertion = 1;  /* 0:RESUME 1:OCCUR 2:ONE_TIME */
    fault_event->event_serial_num = g_event_serial++;

    return 0;
}

STATIC void soc_misc_notify_sensor_scan_immediately(unsigned int dev_id, u8 cluster_id, u8 sensor_id)
{
    struct drv_soc_misc_ctrl *soc_misc_ctl = soc_misc_get_ctrl();
#ifdef CFG_FEATURE_SAFETY_MANAGER
    struct soc_misc_node *node = &(soc_misc_ctl->node[dev_id][cluster_id][sensor_id]);
    u32 i = 0;

    for (; i < node->sensor_obj_num; ++i) {
        if (dms_sensor_event_notify(dev_id, &node->sensor_obj_table[i]) != 0) {
            soc_misc_drv_err("notify sensor scan event failed. "
                "(dev_id=%u, sensor_id=%u, sensor_idx=%u)\n", dev_id, sensor_id, i);
        }
    }
#else
    struct dms_sensor_object_cfg *sensor_obj =
        soc_misc_ctl->node[dev_id][cluster_id][sensor_id].sensor_obj_table;
    if (dms_sensor_event_notify(dev_id, &sensor_obj[0]) != 0) {
        soc_misc_drv_err("notify sensor scan event failed. (dev_id=%u, sensor_id=%u)\n", dev_id, sensor_id);
    }
#endif
}

int soc_misc_report_arm_ras_event(unsigned int dev_id, const struct safety_fault_status *fault,
    unsigned int *event_num, struct safety_event *event_list)
{
    u32 safety_status, safety_bit;
    u8 cluster_id, sensor_id;
    unsigned int event_index;
    const struct ras_fault_converge_item *coverage_node = NULL;

    soc_misc_drv_debug("safety fault handler: emu_id(%u), src_id(%u), bit_id(%u), status(0x%x).\n",
        fault->emu_id, fault->src_id, fault->bit_id, fault->fault_status);

    cluster_id = soc_misc_get_node_id_by_emu(fault->emu_id, fault->src_id, fault->bit_id);
    if (cluster_id >= SOC_MISC_MAX_NODE_NUM) {
        soc_misc_drv_err("emu id invalid. imu_id(%d)\n", fault->emu_id);
        return -EINVAL;
    }
    sensor_id = soc_misc_get_sensor_id_by_src_id(fault->emu_id, fault->src_id, fault->bit_id);
    if (sensor_id >= SOC_MISC_SENSOR_ID_MAX) {
        soc_misc_drv_err("src id invalid. src_id(%u), bit_id(%u)\n", fault->src_id, fault->bit_id);
        return -EINVAL;
    }

    safety_status = fault->fault_status;
    while (safety_status) {
        safety_bit = ffs(safety_status) - 1;
        safety_status &= ~(1U << safety_bit);

        coverage_node = soc_misc_report_safety_fault(dev_id, cluster_id, sensor_id, safety_bit);
        if (coverage_node == NULL) {
            soc_misc_drv_err("report safety fault to sensor failed. (sensor:%u, err_bit:%u)\n", sensor_id, safety_bit);
            continue;
        }

        event_index = *event_num;
        if (soc_misc_set_fault_event(&event_list[event_index],
            dev_id, cluster_id, sensor_id, coverage_node->sensor_type, coverage_node->error_type) != 0) {
            soc_misc_drv_err("set fault event failed. event index=%u\n", event_index);
            continue;
        }
        event_list[event_index].emu_id = fault->emu_id;
        event_list[event_index].src_id = fault->src_id;
        event_list[event_index].bit_id = fault->bit_id;

        *event_num = event_index + 1;
        if (*event_num >= SOC_MISC_SAFETY_EVENT_MAX) {
            soc_misc_drv_warn("event list full. capacity is %u\n", event_index);
            break;
        }
    }

    soc_misc_notify_sensor_scan_immediately(dev_id, cluster_id, sensor_id);
    return 0;
}

int soc_misc_subsys_irq_handler(struct safety_fault_info *fault_info,
    unsigned int *event_num, struct safety_event **event_list)
{
    unsigned int i;
    int ret;

    if (fault_info == NULL) {
        soc_misc_drv_err("fault_info info is null\n");
        return -EINVAL;
    } else if (event_num == NULL) {
        soc_misc_drv_err("event num is null\n");
        return -EINVAL;
    } else if (fault_info->fault_status_list == NULL) {
        soc_misc_drv_err("fault status list is null\n");
        return -EINVAL;
    }

    *event_list = (struct safety_event *)kmalloc(
        sizeof(struct safety_event) * SOC_MISC_SAFETY_EVENT_MAX, GFP_KERNEL | __GFP_ACCOUNT);
    if (*event_list == NULL) {
        soc_misc_drv_err("kmalloc safety event list failed\n");
        return -EINVAL;
    }

    for (i = 0; i < fault_info->fault_reg_num; ++i) {
        ret = soc_misc_report_arm_ras_event(fault_info->dev_id,
            &fault_info->fault_status_list[i], event_num, *event_list);
        if (ret) {
            soc_misc_drv_err("report arm ras event failed. (ret=%d)\n", ret);
            return ret;
        }
    }
    return 0;
}


STATIC bool soc_misc_exist_irq_in_dts(const struct soc_misc_safety_cfg *irq_cfg)
{
#ifdef CFG_FEATURE_SAFETY_MANAGER
    u32 i;

    /* If the irq has not configured in dts,
     * maybe not support in current enviroment */
    for (i = 0; i < irq_cfg->irq_cfg_num; ++i) {
        if (drvfault_is_ras_irq_type(DF_INFO_BASE(irq_cfg->safety_irq_cfgs)->irq_type)) {
            if (drvfault_find_irq_in_dts(irq_cfg->ras_irq_cfgs[i].irq_name) == 0) {
                soc_misc_drv_info("ras irq found in dts.(irq name=%.*s)\n",
                    NAME_LEN, irq_cfg->ras_irq_cfgs[i].irq_name);
                return true;
            }
        } else {
            if (drvfault_find_irq_in_dts(irq_cfg->safety_irq_cfgs[i].irq_name) == 0) {
                soc_misc_drv_info("safety irq found in dts.(irq name=%.*s)\n",
                    NAME_LEN, irq_cfg->safety_irq_cfgs[i].irq_name);
                return true;
            }
        }
    }
    return false;
#else
    return true;
#endif
}

STATIC void soc_misc_unregister_node(struct soc_misc_safety_cfg *safety_cfg, u32 devid)
{
    bool is_ras_irq;

    is_ras_irq = drvfault_is_ras_irq_type(DF_INFO_BASE(safety_cfg->safety_irq_cfgs)->irq_type);
    if (is_ras_irq) {
        dfm_unregister_ras_irq_hwinfo(devid, safety_cfg->ras_irq_cfgs, safety_cfg->irq_cfg_num);
    } else {
        dfm_unregister_safety_irq_hwinfo(devid, safety_cfg->safety_irq_cfgs, safety_cfg->irq_cfg_num);
    }
}

STATIC void soc_misc_unregister_irq(struct soc_misc_safety_cfg *safety_cfg)
{
    if (safety_cfg->register_dms_node != NULL) {
        dfm_unregister_nodes(safety_cfg->dfm);
        dfm_struct_final(safety_cfg->dfm, safety_cfg->dms_node_num, 1);
    }
}

STATIC int soc_misc_register_dms_node_and_safety_irq(struct soc_misc_safety_cfg *safety_cfg, u32 devid)
{
    /* register DMS node to devmng if the dms node has not registered */
    if (safety_cfg->register_dms_node != NULL) {
        if (safety_cfg->register_dms_node(devid) != 0) {
            soc_misc_drv_err("register device node failed.(devid=%u)\n", devid);
            return -EFAULT;
        }
    }

    /* register interrupt to faultmng */
    if (drvfault_is_ras_irq_type(DF_INFO_BASE(safety_cfg->safety_irq_cfgs)->irq_type)) {
        if (dfm_register_ras_irq_hwinfo(devid, safety_cfg->ras_irq_cfgs, safety_cfg->irq_cfg_num) != 0) {
            soc_misc_drv_err("register safety irq info failed.(devid=%u)\n", devid);
            goto init_irq_fail;
        }
    } else {
        if (dfm_register_safety_irq_hwinfo(devid, safety_cfg->safety_irq_cfgs, safety_cfg->irq_cfg_num) != 0) {
            soc_misc_drv_err("register safety irq info failed.(devid=%u)\n", devid);
            goto init_irq_fail;
        }
    }

    /* In addition to common initialization, some modules have special initialization operations.
     * For example, the MBIGEN PERI needs to register the STL fault reporting to the operating system.
     * The specific initialization function of each module can be implemented through post-processing */
    if (safety_cfg->post_init_process != NULL) {
        if (safety_cfg->post_init_process(devid) != 0) {
            soc_misc_drv_err("post process of initialization failed.(devid=%u)\n", devid);
            goto init_process_fail;
        }
    }

    return 0;

init_process_fail:
    soc_misc_unregister_irq(safety_cfg);
init_irq_fail:
    soc_misc_unregister_node(safety_cfg, devid);

    return -EFAULT;
}

void soc_misc_init_safety_modules(u32 devid)
{
    struct soc_misc_safety_cfg **safety_cfgs = NULL;
    u32 module_num = 0;
    u32 i;

    safety_cfgs = soc_misc_get_safety_cfg_array(&module_num);

    for (i = 0; i < module_num; ++i) {
        if (!soc_misc_exist_irq_in_dts(safety_cfgs[i])) {
            soc_misc_drv_info(
                "skip registration because irq is not found in dts.(devid=%u, module_idx=%u)\n", devid, i);
            continue;
        }

        if (soc_misc_register_dms_node_and_safety_irq(safety_cfgs[i], devid) != 0) {
            soc_misc_drv_err("register dms node and safety interrupt failed.(devid=%u, module_idx=%u)\n", devid, i);
            continue;
        }
    }
}

void soc_misc_unregister_dms_node_and_safety_irq(struct soc_misc_safety_cfg *safety_cfg, u32 devid)
{
    soc_misc_unregister_node(safety_cfg, devid);
    soc_misc_unregister_irq(safety_cfg);

    if (safety_cfg->post_uninit_process != NULL) {
        safety_cfg->post_uninit_process(devid);
    }
}

void soc_misc_uninit_safety_modules(u32 devid)
{
    struct soc_misc_safety_cfg **safety_cfgs = NULL;
    u32 module_num = 0;
    u32 i;

    safety_cfgs = soc_misc_get_safety_cfg_array(&module_num);

    for (i = 0; i < module_num; ++i) {
        struct soc_misc_safety_cfg *safety_cfg = safety_cfgs[i];
        if (!soc_misc_exist_irq_in_dts(safety_cfgs[i])) {
            soc_misc_drv_info("irq can't be found in dts.(devid=%u, module_idx=%u)\n", devid, i);
            continue;
        }

        soc_misc_unregister_dms_node_and_safety_irq(safety_cfg, devid);
    }
}

int soc_misc_init_safety_irq(void)
{
    int  irq_idx;
    int ret = 0;
    int num = sizeof(g_safety_irq_table) / sizeof(struct soc_misc_safety_irq_infos);
    u32 soc_type = soc_misc_get_chip_type();

    for (irq_idx = 0; irq_idx < num; ++irq_idx) {
        if ((g_safety_irq_table[irq_idx].soc_platform & soc_type) == 0) {
            soc_misc_drv_info("irq[%d] is not support in this soc, ship to register\n", irq_idx);
            continue;
        }

        ret = soc_misc_register_safety_notifier(
            g_safety_irq_table[irq_idx].irq_infos, g_safety_irq_table[irq_idx].irq_info_num);
        if (ret != 0) {
            soc_misc_drv_err("register ras irq[%d] info failed. ret(%d)\n",
                irq_idx, ret);
            return ret;
        }
    }

    if ((soc_type & CHIP_TYPE_MDC_BS9SX1A) != 0) {
        soc_misc_init_safety_modules(0);
    }

    return ret;
}

STATIC void soc_misc_unregister_safety_irq(unsigned int dev_id)
{
    unsigned int hwinfo_idx, irq_idx;
    struct safety_irq_info *safety_irq = NULL;
    int ret;
    int num = (int)(sizeof(g_safety_irq_table) / sizeof(struct soc_misc_safety_irq_infos));

    for (irq_idx = 0; irq_idx < num; ++irq_idx) {
        for (hwinfo_idx = 0; hwinfo_idx < g_safety_irq_table[irq_idx].irq_info_num; hwinfo_idx++) {
            safety_irq = &g_safety_irq_table[irq_idx].irq_infos[hwinfo_idx];
            ret = drvfault_unregister_safety_irq_info(safety_irq->dev_id, safety_irq->irq_name);
            if (ret != 0) {
                soc_misc_drv_err("register safety fault notifier failed."
                    "(devid=%u, cpu_cluster=%u, irq_index=%u, ret=%d)\n",
                    dev_id, hwinfo_idx, irq_idx, ret);
            }
        }
    }
}

void soc_misc_uninit_safety_irq(void)
{
    struct drv_soc_misc_ctrl *soc_misc_ctl = soc_misc_get_ctrl();
    unsigned int dev_id;

    if (soc_misc_ctl == NULL) {
        soc_misc_drv_err("soc misc has uninited.\n");
        return;
    }

    for (dev_id = 0; dev_id < soc_misc_ctl->dev_num; dev_id++) {
        soc_misc_unregister_safety_irq(dev_id);
    }
    if ((soc_misc_get_chip_type() & CHIP_TYPE_MDC_BS9SX1A) != 0) {
        soc_misc_uninit_safety_modules(0);
    }
}
