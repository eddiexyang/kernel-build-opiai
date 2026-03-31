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

/**
 * brief description about this document.
 * points to focus on.
 */

#include <linux/slab.h>
#include <linux/workqueue.h>

#include "soc_misc_common.h"
#include "dms_notifier.h"
#include "soc_misc_safety.h"
#include "soc_misc_dms_def.h"
#include "soc_misc_config.h"
#include "l3d_dms_init.h"
#include "l3t_dms_init.h"
#include "soc_misc_init.h"
#include "soc_misc_fault.h"
#ifdef CFG_FEATURE_FAULT_MANAGER
#ifdef CFG_FEATURE_DFM_RAS_FAULT
#include "soc_misc_ras.h"
#endif
#endif

#include "dms_dev_node.h"

STATIC struct drv_soc_misc_ctrl g_soc_misc_ctrl;

static inline bool soc_misc_dms_node_valid(const struct soc_misc_node *s_dev)
{
    if (s_dev == NULL || s_dev->dev_node == NULL) {
        return false;
    }

    return (s_dev->dev_node->owner_devid != -1) && (s_dev->dev_node->node_id != -1);
}

int soc_misc_register_dms_sensors(struct soc_misc_node *s_dev)
{
    int ret;
    unsigned int i;

    if (s_dev == NULL) {
        soc_misc_drv_err("Invalid para, s_dev is NULL.\n");
        return -EINVAL;
    }

    for (i = 0; i < s_dev->sensor_obj_num; i++) {
        INIT_LIST_HEAD(&s_dev->sensor_event_queue[i].list);
        mutex_init(&s_dev->sensor_event_queue[i].mutex);
        ret = dms_sensor_register(s_dev->dev_node, &s_dev->sensor_obj_table[i]);
        if (ret != 0) {
            soc_misc_drv_err("Register sensor failed. (sensor_idx=%u; ret=%d.)\n", i, ret);
            goto out;
        }

        ret = fpdc_register_fault_notifier(s_dev->dev_node->node_type, s_dev->fpdc_notify);
        if (ret != 0) {
            soc_misc_drv_err("fpdc_register_fault_notifier failed. (sensor_idx=%u; ret=%d.)\n", i, ret);
            i++; /* used for label "out: if (i > 0) {..."  */
            goto out;
        }
    }
    return 0;

out:
    if (i > 0) {
        (void)dms_sensor_node_unregister(s_dev->dev_node);
    }
    return ret;
}

void soc_misc_unregister_dms_sensors(struct soc_misc_node *s_dev)
{
    int ret;
    unsigned int i;

    if (!soc_misc_dms_node_valid(s_dev)) {
        soc_misc_drv_err("Invalid para, s_dev is NULL.\n");
        return;
    }

    ret = fpdc_unregister_fault_notifier(s_dev->dev_node->node_type);
    if (ret != 0) {
        soc_misc_drv_err("soc misc unregister fpdc failed, (node type=%d, ret=%d.)\n", s_dev->dev_node->node_type, ret);
    }

    ret = dms_sensor_node_unregister(s_dev->dev_node);
    if (ret != 0) {
        soc_misc_drv_err("soc misc unregister sensor node failed, (ret=%d.)\n", ret);
    }
    for (i = 0; i < s_dev->sensor_obj_num; i++) {
        soc_misc_fault_event_free(&s_dev->sensor_event_queue[i]);
    }
}

int soc_misc_register_sensors_in_cpu_cluster(unsigned int dev_id, unsigned int cluster_id,
    unsigned int *succ_num)
{
    struct drv_soc_misc_ctrl *soc_misc_ctl = soc_misc_get_ctrl();
    struct soc_misc_node *s_dev = NULL;
    unsigned int sensor_id;
    int ret;

    for (sensor_id = 0; sensor_id < SOC_MISC_SENSOR_ID_MAX; sensor_id++) {
        s_dev = &soc_misc_ctl->node[dev_id][cluster_id][sensor_id]; /* get each device dev_node */
        if (!soc_misc_dms_node_valid(s_dev)) {
            continue;
        }
        ret = soc_misc_register_dms_sensors(s_dev);
        if (ret != 0) {
            soc_misc_drv_err("Register soc misc fault failed. (ret=%d.)\n", ret);
            *succ_num = sensor_id;
            return ret;
        }
    }

    *succ_num = sensor_id;
    return 0;
}

int soc_misc_init_dms_sensors(void)
{
    struct drv_soc_misc_ctrl *soc_misc_ctl = soc_misc_get_ctrl();
    unsigned int dev_id, cluster_id, sensor_id;
    unsigned int successed_sensor_num;
    int i, j, k;
    int ret;

    for (dev_id = 0; dev_id < soc_misc_dev_num(soc_misc_ctl->dev_num); dev_id++) {
        for (cluster_id = 0; cluster_id < SOC_MISC_MAX_NODE_NUM; ++cluster_id) {
            ret = soc_misc_register_sensors_in_cpu_cluster(dev_id, cluster_id, &sensor_id);
            if (ret != 0) {
                goto out;
            }
        }
    }
    return 0;

out:
    for (i = dev_id; i >= 0; i--) {
        for (j = cluster_id; j >= 0; j--) {
            successed_sensor_num = (((i == (int)dev_id) && (j == (int)cluster_id)) ?
                sensor_id : SOC_MISC_SENSOR_ID_MAX);
            for (k = 0; k < (int)successed_sensor_num; k++) {
                soc_misc_unregister_dms_sensors(&soc_misc_ctl->node[i][j][k]);
            }
        }
    }
    return ret;
}

STATIC void soc_misc_uninit_sensors_in_cpu_cluster(int dev_id, int cluster_id)
{
    struct drv_soc_misc_ctrl *soc_misc_ctl = soc_misc_get_ctrl();
    int sensor_id;
    struct soc_misc_node *s_dev = NULL;

    for (sensor_id = 0; sensor_id < SOC_MISC_SENSOR_ID_MAX; sensor_id++) {
        s_dev = &soc_misc_ctl->node[dev_id][cluster_id][sensor_id];
        if (!soc_misc_dms_node_valid(s_dev)) {
            continue;
        }

        soc_misc_unregister_dms_sensors(s_dev);
    }
}

void soc_misc_uninit_dms_sensors(void)
{
    struct drv_soc_misc_ctrl *soc_misc_ctl = soc_misc_get_ctrl();
    unsigned int dev_id, cluster_id;

    for (dev_id = 0; dev_id < soc_misc_dev_num(soc_misc_ctl->dev_num); dev_id++) {
        for (cluster_id = 0; cluster_id < SOC_MISC_MAX_NODE_NUM; ++cluster_id) {
            soc_misc_uninit_sensors_in_cpu_cluster(dev_id, cluster_id);
        }
    }
}

struct drv_soc_misc_ctrl *soc_misc_get_ctrl(void)
{
    return &g_soc_misc_ctrl;
}

#ifdef CFG_FEATURE_FAULT_MANAGER

#define SOC_SENSOR_CFG(dev_id, node_id, sensor_id, sensor_cfg, fpdc_callback, fault_unmask_task, query_task) do { \
    node = &(g_soc_misc_ctrl.node[dev_id][node_id][sensor_id]);                 \
    node->die_id = dev_id;                                                      \
    node->dev_node = &g_soc_misc_dev_node_table[dev_id][sensor_id][node_id];    \
    node->sensor_obj_num = sizeof(sensor_cfg[dev_id][node_id]) / obj_len;       \
    node->sensor_obj_table = sensor_cfg[dev_id][node_id];                       \
    if (fault_unmask_task != NULL) {                                            \
        INIT_DELAYED_WORK(&node->unmask_irq_work, fault_unmask_task);           \
    }                                                                           \
    node->fpdc_notify = fpdc_callback;                                          \
    if (query_task != NULL) {                                                   \
        INIT_DELAYED_WORK(&node->query_work, query_task);                       \
        schedule_delayed_work(&node->query_work,                                \
            msecs_to_jiffies(SOC_MISC_DRV_QUERY_RAS_DEFAULT_TIME));             \
    }                                                                           \
} while (0)

STATIC int soc_misc_ctrl_init(void)
{
    struct soc_misc_node *node = NULL;
    unsigned int obj_len = sizeof(struct dms_sensor_object_cfg);
    unsigned int dev_id, node_id;
    int ret;

    ret = devdrv_get_devnum(&g_soc_misc_ctrl.dev_num);
    if (ret != 0) {
        soc_misc_drv_err("Get dev_num failed. (ret=%d)\n", ret);
        return ret;
    }

    for (dev_id = 0; dev_id < soc_misc_dev_num(g_soc_misc_ctrl.dev_num); dev_id++) {
        for (node_id = 0; node_id < SOC_MISC_MAX_NODE_NUM; ++node_id) {
#ifdef CFG_FEATURE_RING_FAULT
            SOC_SENSOR_CFG(dev_id, node_id, SOC_MISC_SENSOR_RING,
                g_soc_misc_ring_sensor_table, soc_misc_ring_fault_handler, NULL, NULL);
#endif // CFG_FEATURE_RING_FAULT

#ifdef CFG_FEATURE_PCIE_LOCAL_FAULT
            SOC_SENSOR_CFG(dev_id, node_id, SOC_MISC_SENSOR_PCIE,
                g_soc_misc_pcie_sensor_table, soc_misc_pcie_fault_handler,
                soc_misc_pcie_local_unmask_task, NULL);
#endif // CFG_FEATURE_PCIE_LOCAL_FAULT
#ifdef CFG_FEATURE_PCIE_HISI_COMM_FAULT
#ifdef CFG_FEATURE_DRIVER_QUERY_RAS_ERROR
        SOC_SENSOR_CFG(dev_id, node_id, SOC_MISC_SENSOR_PCIE,
            g_soc_misc_pcie_sensor_table, soc_misc_pcie_fault_handler,
            soc_misc_pcie_hisi_comm_unmask_task, soc_misc_pcie_query_ras_error);
#else
        SOC_SENSOR_CFG(dev_id, node_id, SOC_MISC_SENSOR_PCIE,
            g_soc_misc_pcie_sensor_table, soc_misc_pcie_fault_handler,
            soc_misc_pcie_hisi_comm_unmask_task, NULL);
#endif // CFG_FEATURE_DRIVER_QUERY_RAS_ERROR
#endif // CFG_FEATURE_PCIE_HISI_COMM_FAULT

#ifdef CFG_FEATURE_MN_FAULT
            SOC_SENSOR_CFG(dev_id, node_id, SOC_MISC_SENSOR_MN,
                g_soc_misc_mn_sensor_table, soc_misc_mn_fault_handler, NULL, NULL);
#endif // CFG_FEATURE_MN_FAULT

#ifdef CFG_FEATURE_PCIE_DISP_FAULT
        SOC_SENSOR_CFG(dev_id, node_id, SOC_MISC_SENSOR_PCIE_DISP,
            g_soc_misc_pcie_disp_sensor_table, soc_misc_pcie_fault_handler, NULL, NULL);
#endif // CFG_FEATURE_PCIE_DISP_FAULT

#ifdef CFG_FEATURE_EMMC_HISI_COMM_FAULT
        SOC_SENSOR_CFG(dev_id, node_id, SOC_MISC_SENSOR_EMMC,
            g_soc_misc_emmc_sensor_table, soc_misc_emmc_fault_handler, NULL, soc_misc_emmc_query_ras_error);
#endif // CFG_FEATURE_EMMC_HISI_COMM_FAULT
        }
    }
    return 0;
}

#ifdef AOS_LLVM_BUILD
static inline int delayed_work_pending(struct delayed_work *work)
{
    return true;
}
#endif

#define SOC_SENSOR_CFG_UNINIT(dev_id, node_id, sensor_id) do { \
    node = &(g_soc_misc_ctrl.node[dev_id][node_id][sensor_id]);                 \
    node->die_id = 0;                                                           \
    node->dev_node = NULL;                                                      \
    node->sensor_obj_num = 0;                                                   \
    node->sensor_obj_table = 0;                                                 \
    if (node->unmask_irq_work.work.func != NULL) {                              \
        (void)cancel_delayed_work_sync(&node->unmask_irq_work);                 \
    }                                                                           \
    node->fpdc_notify = NULL;                                                   \
    if (node->query_work.work.func != NULL) {                                   \
        (void)cancel_delayed_work_sync(&node->query_work);                      \
    }                                                                           \
} while (0)

STATIC void soc_misc_ctrl_uninit(void)
{
    struct soc_misc_node *node = NULL;
    unsigned int dev_id, node_id;
    int ret;

    ret = devdrv_get_devnum(&g_soc_misc_ctrl.dev_num);
    if (ret != 0) {
        soc_misc_drv_err("Get dev_num failed. (ret=%d)\n", ret);
        return;
    }

    for (dev_id = 0; dev_id < soc_misc_dev_num(g_soc_misc_ctrl.dev_num); dev_id++) {
        for (node_id = 0; node_id < SOC_MISC_MAX_NODE_NUM; ++node_id) {
#ifdef CFG_FEATURE_RING_FAULT
            SOC_SENSOR_CFG_UNINIT(dev_id, node_id, SOC_MISC_SENSOR_RING);
#endif // CFG_FEATURE_RING_FAULT
#if ((defined CFG_FEATURE_PCIE_LOCAL_FAULT) || (defined CFG_FEATURE_PCIE_HISI_COMM_FAULT))
            SOC_SENSOR_CFG_UNINIT(dev_id, node_id, SOC_MISC_SENSOR_PCIE);
#endif
#ifdef CFG_FEATURE_MN_FAULT
            SOC_SENSOR_CFG_UNINIT(dev_id, node_id, SOC_MISC_SENSOR_MN);
#endif
#ifdef CFG_FEATURE_PCIE_DISP_FAULT
            SOC_SENSOR_CFG_UNINIT(dev_id, node_id, SOC_MISC_SENSOR_PCIE_DISP);
#endif // CFG_FEATURE_PCIE_DISP_FAULT
#ifdef CFG_FEATURE_EMMC_HISI_COMM_FAULT
         SOC_SENSOR_CFG_UNINIT(dev_id, node_id, SOC_MISC_SENSOR_EMMC);
#endif // CFG_FEATURE_EMMC_HISI_COMM_FAULT
        }
    }
}

STATIC int soc_misc_register_dms_node(struct soc_misc_node *s_dev)
{
    int ret;

    if (s_dev == NULL) {
        soc_misc_drv_err("Invalid para, h_dev is NULL.\n");
        return -EINVAL;
    }

    ret = dms_register_dev_node(s_dev->dev_node);
    if (ret != 0) {
        soc_misc_drv_err("Register dev_node failed. (ret=%d.)\n", ret);
    }

    return ret;
}

STATIC void soc_misc_unregister_dms_node(struct soc_misc_node *s_dev)
{
    int ret;

    if (!soc_misc_dms_node_valid(s_dev)) {
        soc_misc_drv_err("Invalid para, h_dev is NULL.\n");
        return;
    }

    ret = dms_unregister_dev_node(s_dev->dev_node);
    if (ret != 0) {
        soc_misc_drv_warn("SOC unregister dev node, (ret=%d.)\n", ret);
    }
}

STATIC int soc_misc_init_nodes_in_cpu_cluster(unsigned int dev_id,
    unsigned int node_id, unsigned int *succ_num)
{
    struct drv_soc_misc_ctrl *soc_misc_ctl = soc_misc_get_ctrl();
    struct soc_misc_node *s_dev = NULL;
    unsigned int sensor_id;
    int ret;

    for (sensor_id = 0; sensor_id < SOC_MISC_SENSOR_ID_MAX; sensor_id++) {
        s_dev = &soc_misc_ctl->node[dev_id][node_id][sensor_id]; /* get each device dev_node */
        if (!soc_misc_dms_node_valid(s_dev)) {
            soc_misc_drv_info("This dms node is not used in cpu cluster. (dev_id=%u; sensor_id=%u)\n",
                dev_id, sensor_id);
            continue;
        }

        ret = soc_misc_register_dms_node(s_dev);
        if (ret != 0) {
            soc_misc_drv_err("Register dev_node failed. (dev_id=%u; sensor_id=%u; ret=%d)\n",
                dev_id, sensor_id, ret);
            *succ_num = sensor_id;
            return ret;
        }
    }

    *succ_num = sensor_id;
    return 0;
}

STATIC int soc_misc_init_dms_nodes(void)
{
    struct drv_soc_misc_ctrl *soc_misc_ctl = soc_misc_get_ctrl();
    unsigned int dev_id, cluster_id, sensor_id;
    unsigned int successed_sensor_num;
    int i, j, k;
    int ret;

    for (dev_id = 0; dev_id < soc_misc_dev_num(soc_misc_ctl->dev_num); dev_id++) {
        for (cluster_id = 0; cluster_id < SOC_MISC_MAX_NODE_NUM; ++cluster_id) {
            ret = soc_misc_init_nodes_in_cpu_cluster(dev_id, cluster_id, &sensor_id);
            if (ret) {
                goto out;
            }
        }
    }
    return 0;

out:
    for (i = dev_id; i>= 0; i--) {
        for (j = cluster_id; j >= 0; j--) {
            successed_sensor_num = (((i == (int)dev_id) && (j == (int)cluster_id)) ?
                sensor_id : SOC_MISC_SENSOR_ID_MAX);
            for (k = 0; k < (int)successed_sensor_num; k++) {
                soc_misc_unregister_dms_node(&soc_misc_ctl->node[i][j][k]);
            }
        }
    }
    return ret;
}

STATIC void soc_misc_uninit_dms_nodes(void)
{
    struct drv_soc_misc_ctrl *soc_misc_ctl = soc_misc_get_ctrl();
    unsigned int dev_id, node_id, sensor_id;

    for (dev_id = 0; dev_id < soc_misc_dev_num(soc_misc_ctl->dev_num); dev_id++) {
        for (node_id = 0; node_id < SOC_MISC_MAX_NODE_NUM; ++node_id) {
            for (sensor_id = 0; sensor_id < SOC_MISC_SENSOR_ID_MAX; sensor_id++) {
                soc_misc_unregister_dms_node(&soc_misc_ctl->node[dev_id][node_id][sensor_id]);
            }
        }
    }
}

STATIC int soc_misc_init_modules_fault(void)
{
    int ret;

    ret = l3d_fault_manager_init();
    if (ret != 0) {
        soc_misc_drv_err("register fault of l3d failed. (ret=%d)\n", ret);
        return ret;
    }
    ret = l3t_fault_manager_init();
    if (ret != 0) {
        soc_misc_drv_err("register fault of l3t failed. (ret=%d)\n", ret);
        goto l3t_fault_init_fail;
    }
    return 0;

l3t_fault_init_fail:
    l3d_fault_manager_uninit();

    return ret;
}

STATIC void soc_misc_uninit_modules_fault(void)
{
    l3d_fault_manager_uninit();
    l3t_fault_manager_uninit();
}

STATIC int soc_misc_init_notifier(struct notifier_block *nb, unsigned long mode, void *data)
{
    soc_misc_drv_info("Soc misc received notify.\n");
    return 0;
}

STATIC struct notifier_block g_soc_misc_notifier = {
    .notifier_call = soc_misc_init_notifier,
};
#endif

int soc_misc_fault_init(void)
{
#ifdef CFG_FEATURE_FAULT_MANAGER
    int ret;

    ret = soc_misc_ctrl_init();
    if (ret != 0) {
        soc_misc_drv_err("SOC ctrl init failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = dms_register_notifier(&g_soc_misc_notifier);
    if (ret != 0) {
        soc_misc_drv_err("SOC register dms notifier failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = soc_misc_init_dms_nodes();
    if (ret != 0) {
        soc_misc_drv_err("SOC dev_node init failed. (ret=%d)\n", ret);
        goto node_init_fail;
    }

    ret = soc_misc_init_dms_sensors();
    if (ret != 0) {
        soc_misc_drv_err("SOC fault init failed. (ret=%d)\n", ret);
        goto fault_init_fail;
    }

    ret = soc_misc_init_modules_fault();
    if (ret != 0) {
        soc_misc_drv_err("soc_misc_init_fault failed. (ret=%d)\n", ret);
        goto init_modules_fault_fail;
    }

    ret = soc_misc_init_safety_irq();
    if (ret != 0) {
        soc_misc_drv_err("SOC safety init failed. (ret=%d)\n", ret);
        goto init_safety_irq_fail;
    }

#ifdef CFG_FEATURE_DFM_RAS_FAULT
    ret = soc_misc_init_ras_fault();
    if (ret != 0) {
        soc_misc_drv_err("SOC ras fault init failed. (ret=%d)\n", ret);
        goto init_ras_fault_fail;
    }
#endif

    return 0;

#ifdef CFG_FEATURE_DFM_RAS_FAULT
init_ras_fault_fail:
#endif
    soc_misc_uninit_safety_irq();
init_safety_irq_fail:
    soc_misc_uninit_modules_fault();
init_modules_fault_fail:
    soc_misc_uninit_dms_sensors();
fault_init_fail:
    soc_misc_uninit_dms_nodes();
node_init_fail:
    dms_unregister_notifier(&g_soc_misc_notifier);

    return ret;
#endif
}

void soc_misc_fault_exit(void)
{
#ifdef CFG_FEATURE_FAULT_MANAGER
    soc_misc_uninit_safety_irq();
#ifdef CFG_FEATURE_DFM_RAS_FAULT
    soc_misc_uninit_ras_fault();
#endif
    soc_misc_uninit_modules_fault();
    soc_misc_uninit_dms_sensors();
    soc_misc_uninit_dms_nodes();
    dms_unregister_notifier(&g_soc_misc_notifier);
    soc_misc_ctrl_uninit();
#endif
}
