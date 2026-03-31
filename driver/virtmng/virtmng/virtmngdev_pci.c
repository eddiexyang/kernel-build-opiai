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

#include <linux/pci.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/semaphore.h>
#include <linux/delay.h>

#include "dbl/uda.h"

#include "runenv_config.h"
#include "virtmngdev_pci.h"
#include "virtmngdev_proc_fs.h"
#include "virtmngdev_resource.h"
#ifdef CFG_FEATURE_VIRTMNG_SUPPORT_UDA
#include "virtmngdev_mia_cfg.h"
#endif
#include "ascend_hal_define.h"
#include "devdrv_interface.h"
#include "kernel_version_adapt.h"

struct vmngd_ctrl vmngdev_ctrl = { 0 };

struct vmngd_client_instance g_vmngd_clients_instance[VMNGD_CLIENT_TYPE_MAX][VMNGD_SURPORT_MAX_DEV]
                                                     [VMNG_VDEV_MAX_PER_PDEV];
struct vmng_vdev_ctrl g_vdev_ctrl[VMNGD_SURPORT_MAX_DEV][VMNG_VDEV_MAX_PER_PDEV];
struct mutex g_vdev_ctrl_mutex[VMNGD_SURPORT_MAX_DEV][VMNG_VDEV_MAX_PER_PDEV];
struct vmngd_client *g_vmngd_clients[VMNGD_CLIENT_TYPE_MAX];
u32 vmngd_tsd_pid = 0;
submit_event g_vmngd_submit_event_func = NULL;
struct vmngd_clear_timer g_clear_timer;
int vmngd_ack_result = 0;
struct semaphore vmngd_sema;

static const struct chip_to_vf_max_num g_chip_to_vf_max_table[HISI_CHIP_NUM] = {
    {
        .chip_type = HISI_MINI_V1,
        .vf_max_num = 0 /* ascend 310 not support vdev */
    },
    {
        .chip_type = HISI_CLOUD_V1,
        .vf_max_num = 16 /* ascend 910 support vdev */
    },
    {
        .chip_type = HISI_MINI_V2,
        .vf_max_num = 8 /* ascend 310p support vdev */
    },
    {
        .chip_type = HISI_CLOUD_V2,
        .vf_max_num = 8 /* support vdev */
    },
    {
        .chip_type = HISI_MINI_V3,
        .vf_max_num = 4 /* not support vdev */
    }
};

int vmngd_msg_recv_sync(u32 devid, struct vmng_ctrl_msg *msg);
int vmngd_msg_recv_destory_vdev(u32 dev_id, struct vmng_ctrl_msg *msg);
int vmngd_msg_recv_alloc_vf(u32 dev_id, struct vmng_ctrl_msg *msg);
int vmngd_msg_recv_enquire_resource(u32 dev_id, struct vmng_ctrl_msg *msg);
int vmngd_msg_recv_create_vdev(u32 dev_id, struct vmng_ctrl_msg *msg);
int vmngd_msg_recv_refresh_vf(u32 dev_id, struct vmng_ctrl_msg *msg);
int vmngd_msg_recv_free_vf(u32 dev_id, struct vmng_ctrl_msg *msg);

STATIC int vmngd_bw_get_vfio_barspace(u32 dev_id, u32 vfid, struct vf_bandwidth_ctrl_remote **vfio_base)
{
    if (vmngdev_ctrl.devices[dev_id].bw_ctrl.io_base_bwctrl == NULL) {
        vmng_err("io_base_bwctrl is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    *vfio_base = vmngdev_ctrl.devices[dev_id].bw_ctrl.io_base_bwctrl + (vfid - 1);

    return 0;
}

int vmngd_ack_event(unsigned int devid, unsigned int subevent_id, const char *msg, unsigned int msg_len, void *priv)
{
    int ret;

    if (msg == NULL) {
        vmng_err("Input parameter is error.\n");
        return -EINVAL;
    }
    if ((subevent_id != VMNGD_EVENT_CREATE_VF) && (subevent_id != VMNGD_EVENT_DESTROY_VF)) {
        vmng_err("subevent_id check failed. (subevent_id=%u)\n", subevent_id);
        return -EINVAL;
    }

    ret = memcpy_s(&vmngd_ack_result, sizeof(int), msg, msg_len);
    if (ret != 0) {
        vmng_err("Call memcpy_s failed. (msg_len=%u)\n", msg_len);
        return -EINVAL;
    }

    up(&vmngd_sema);

    return 0;
}

int vmngd_get_device_status(int devid)
{
    struct vmngd_dev *dev = &vmngdev_ctrl.devices[devid];
    return dev->valid;
}

STATIC int vmngd_bw_data_clear(u32 dev_id, u32 vfid)
{
    struct vf_bandwidth_ctrl_remote *vf_ptr = NULL;
    int ret;

    ret = vmngd_bw_get_vfio_barspace(dev_id, vfid, &vf_ptr);
    if (ret) {
        vmng_err("Get vfio base failed. (dev_id=%u; fid=%u)\n", dev_id, vfid);
        return ret;
    }

    vf_ptr->ctrlcpu_flow_cnt[VMNG_PCIE_FLOW_H2D] = 0;
    vf_ptr->ctrlcpu_flow_cnt[VMNG_PCIE_FLOW_D2H] = 0;
    vf_ptr->ctrlcpu_pack_cnt[VMNG_PCIE_FLOW_H2D] = 0;
    vf_ptr->ctrlcpu_pack_cnt[VMNG_PCIE_FLOW_D2H] = 0;

    vf_ptr->tscpu_flow_cnt[VMNG_PCIE_FLOW_H2D] = 0;
    vf_ptr->tscpu_flow_cnt[VMNG_PCIE_FLOW_D2H] = 0;
    vf_ptr->tscpu_pack_cnt[VMNG_PCIE_FLOW_H2D] = 0;
    vf_ptr->tscpu_pack_cnt[VMNG_PCIE_FLOW_D2H] = 0;

    return 0;
}

STATIC enum hrtimer_restart vmngd_bw_data_clear_event(struct hrtimer *t)
{
    u32 dev_id, vfid;
    int ret;

    for (dev_id = 0; dev_id < VMNGD_SURPORT_MAX_DEV; dev_id++) {
        if (vmngd_get_device_status(dev_id) != VMNG_VALID) {
            continue;
        }
        for (vfid = VMNG_VDEV_FIRST_VFID; vfid < VMNG_VDEV_MAX_PER_PDEV; vfid++) {
            ret = vmngd_bw_data_clear(dev_id, vfid);
            if (ret) {
                vmng_err("Bandwidth ctrl data clear failed. (dev_id=%u; fid=%u)\n", dev_id, vfid);
                return HRTIMER_NORESTART;
            }
        }
    }
    hrtimer_forward_now(&g_clear_timer.timer, g_clear_timer.kt);
    return HRTIMER_RESTART;
}

STATIC void vmngd_bw_data_clear_timer_init(void)
{
    g_clear_timer.vaild_dev++;
    if (g_clear_timer.vaild_dev > 1) {
        return;
    }

    hrtimer_init(&g_clear_timer.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    g_clear_timer.timer.function = vmngd_bw_data_clear_event;
    g_clear_timer.kt = ktime_set(BANDWIDTH_CLEAR_INTERVAL, 0);
    hrtimer_start(&g_clear_timer.timer, g_clear_timer.kt, HRTIMER_MODE_REL);

    vmng_info("Used bandwidth clear timer init finish.\n");
}

STATIC void vmngd_bw_data_clear_timer_uninit(void)
{
    if (g_clear_timer.vaild_dev <= 0) {
        return;
    }

    g_clear_timer.vaild_dev--;
    if (g_clear_timer.vaild_dev > 0) {
        return;
    }

    hrtimer_cancel(&g_clear_timer.timer);

    vmng_info("Used bandwidth clear timer canceled.\n");
}

enum vmng_split_mode vmng_get_device_split_mode(u32 dev_id)
{
    u32 pf_id = dev_id;
    u32 vf_id = 0;
    int ret;

    if (vmngd_get_pfvf_type_by_devid(dev_id) == VMNGD_TYPE_VF) {
        ret = vmngd_get_pfvf_id_by_devid(dev_id, &pf_id, &vf_id);
        if (ret != 0) {
            vmng_err("Trans dev_id to phy_id and vf_id failed. (dev_id=%u)\n", dev_id);
            return VMNG_INVALID_SPLIT_MODE;
        }
    }

    if (pf_id >= VMNGD_SURPORT_MAX_DEV) {
        vmng_err("Invalid dev_id. (dev_id=%u)\n", pf_id);
        return VMNG_INVALID_SPLIT_MODE;
    }

    return vmngdev_ctrl.devices[pf_id].split_mode;
}
EXPORT_SYMBOL(vmng_get_device_split_mode);

void vmng_set_device_split_mode(u32 dev_id, enum vmng_split_mode split_mode)
{
    vmngdev_ctrl.devices[dev_id].split_mode = split_mode;
}

STATIC void vmngd_init_split_mode(u32 dev_id, u32 status)
{
    mutex_lock(&vmngdev_ctrl.devices[dev_id].mutex);
    if (vmngdev_ctrl.devices[dev_id].vf_num == 0) {
        vmng_set_device_split_mode(dev_id, status);
    }
    vmngdev_ctrl.devices[dev_id].vf_num++;
    mutex_unlock(&vmngdev_ctrl.devices[dev_id].mutex);
}

STATIC void vmngd_uninit_split_mode(u32 dev_id)
{
    mutex_lock(&vmngdev_ctrl.devices[dev_id].mutex);
    vmngdev_ctrl.devices[dev_id].vf_num--;
    if (vmngdev_ctrl.devices[dev_id].vf_num == 0) {
        vmng_set_device_split_mode(dev_id, VMNG_NORMAL_NONE_SPLIT_MODE);
    }
    mutex_unlock(&vmngdev_ctrl.devices[dev_id].mutex);
}

int vmngd_get_device_vf_max(u32 dev_id, u32 *vf_max_num)
{
    u32 chip_type, i;

    if (vf_max_num == NULL) {
        vmng_err("Invalid parameter.\n");
        return -EINVAL;
    }

    chip_type = vmngd_get_dev_chip_type(dev_id);
    for (i = 0; i < HISI_CHIP_NUM; i++) {
        if (chip_type == g_chip_to_vf_max_table[i].chip_type) {
            *vf_max_num = g_chip_to_vf_max_table[i].vf_max_num;
            return 0;
        }
    }

    *vf_max_num = 0;
    vmng_info("Invalid chip type. (dev_id=%u; chip_type=%u)\n", dev_id, chip_type);
    return 0;
}
EXPORT_SYMBOL(vmngd_get_device_vf_max);

int vmngd_get_device_vf_list(u32 dev_id, u32 *vf_list, u32 list_len, u32 *vf_num)
{
    u32 vf_count = 0;
    u32 i;

    if ((dev_id >= VMNGD_SURPORT_MAX_DEV) || (vf_list == NULL) || (vf_num == NULL)) {
        vmng_err("Invalid parameter. (dev_id=%u; vf_list=%s; vf_num=%s;)\n", dev_id, vf_list == NULL ? "NULL" : "OK",
            vf_num == NULL ? "NULL" : "OK");
        return -EINVAL;
    }

    for (i = 0; i < VMNG_VDEV_MAX_PER_PDEV; i++) {
        if (g_vdev_ctrl[dev_id][i].status != VMNG_VDEV_STATUS_CLIENT_INIT) {
            continue;
        }
        if (vf_count >= list_len) {
            vmng_err("Invalid parameter. (dev_id=%u; list_len=%u; vf_count=%u)\n", dev_id, list_len, vf_count);
            return -EINVAL;
        }
        vf_list[vf_count++] = g_vdev_ctrl[dev_id][i].vfid;
    }

    *vf_num = vf_count;
    return 0;
}
EXPORT_SYMBOL(vmngd_get_device_vf_list);

int vmngd_get_device_vf_core_info(u32 dev_id, u32 vf_id, u32 *total_core, u32 *core_count, u64 *mem_size)
{
    if (dev_id >= VMNGD_SURPORT_MAX_DEV) {
        vmng_err("Input parameter is error. (dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }
    if (vf_id >= VMNG_VDEV_MAX_PER_PDEV) {
        vmng_err("Input parameter is error. (vfid=%u)\n", vf_id);
        return VMNG_ERR;
    }
    if ((total_core == NULL) || (core_count == NULL) || (mem_size == NULL)) {
        vmng_err("Input parameter is error.\n");
        return VMNG_ERR;
    }
    if (g_vdev_ctrl[dev_id][vf_id].status != VMNG_VDEV_STATUS_CLIENT_INIT) {
        vmng_err("vdev not create. (dev_id=%u; vfid=%u)\n", dev_id, vf_id);
        return VMNG_ERR;
    }

    *total_core = g_vdev_ctrl[dev_id][vf_id].total_core_num;
    *core_count = g_vdev_ctrl[dev_id][vf_id].core_num;
    *mem_size = g_vdev_ctrl[dev_id][vf_id].mem_size;
    return 0;
}
EXPORT_SYMBOL(vmngd_get_device_vf_core_info);

int vmngd_sumit_event_func(u32 dev_id, struct sched_published_event *event)
{
    if (g_vmngd_submit_event_func == NULL) {
        g_vmngd_submit_event_func = (submit_event)(uintptr_t)__kallsyms_lookup_name("sched_submit_event");
    }

    if (g_vmngd_submit_event_func != NULL) {
        return g_vmngd_submit_event_func(dev_id, event);
    }

    return -EINVAL;
}

int vmngd_sumit_event(int event_type, const struct vmngd_client_instance *instance)
{
    u32 dev_id = instance->vdev_ctrl.dev_id;
    u32 vfid = instance->vdev_ctrl.vfid;
    struct sched_published_event event = {0};
    struct vmngd_event_msg event_msg;
    int ret;

    event_msg.dev_id = instance->vdev_ctrl.dev_id;
    event_msg.vfid = instance->vdev_ctrl.vfid;
    event_msg.core_num = instance->vdev_ctrl.core_num;
    event_msg.total_core_num = instance->vdev_ctrl.total_core_num;

    event.event_info.pid = vmngd_tsd_pid;
    event.event_info.gid = 0;
    event.event_info.event_id = EVENT_CCPU_CTRL_MSG;
    event.event_info.subevent_id = event_type;
    event.event_info.msg_len = sizeof(struct vmngd_event_msg);
    event.event_info.msg = (char *)&event_msg;
    event.event_func.event_ack_func = vmngd_ack_event;
    event.event_func.event_finish_func = NULL;
    event.event_info.dst_engine = CCPU_DEVICE;

    ret = vmngd_sumit_event_func(0, &event);
    if (ret != 0) {
        vmng_err("Submit event failed. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return -EINVAL;
    }

    ret = down_timeout(&vmngd_sema, VMNGD_WAIT_TIMEOUT);
    if (ret) {
        vmng_err("Submit event down timeout. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return -EINVAL;
    }

    if (vmngd_ack_result != 0) {
        vmng_err("Ack result not ok. (dev_id=%u; vfid=%u; ack_result=%d)\n", dev_id, vfid, vmngd_ack_result);
        return -EINVAL;
    }

    vmng_info("Get event_type value. (dev_id=%u; vfid=%u; event_type=%d)\n", dev_id, event_msg.vfid, event_type);
    return 0;
}

int vmngd_notify_tsd_create_vdev(struct vmngd_client_instance *instance)
{
    return vmngd_sumit_event(VMNGD_EVENT_CREATE_VF, instance);
}
int vmngd_notify_tsd_destory_vdev(struct vmngd_client_instance *instance)
{
    return vmngd_sumit_event(VMNGD_EVENT_DESTROY_VF, instance);
}

struct vmngd_client tsd_client = {
    .type = VMNGD_CLIENT_TYPE_TSD,
    .init_instance = vmngd_notify_tsd_create_vdev,
    .uninit_instance = vmngd_notify_tsd_destory_vdev,
};

int vmngd_register_vmng_client(void)
{
    vmngd_tsd_pid = current->tgid;

    g_vmngd_clients[VMNGD_CLIENT_TYPE_TSD] = &tsd_client;

    vmng_info("Get vmngd_tsd_pid value. (pid=%u)\n", vmngd_tsd_pid);

    return 0;
}
EXPORT_SYMBOL(vmngd_register_vmng_client);

static int vmngd_uninit_instance_proc(u32 dev_id, u32 vfid, struct vmngd_client_instance *instance,
    const struct vmngd_client *client)
{
    int ret;

    /* if client have not been registered. */
    if (client == NULL) {
        vmng_debug("Input parameter is error. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return 0;
    }

    if (client->uninit_instance == NULL) {
        vmng_debug("Client is no need to uninit instance. (dev_id=%u; vfid=%u; client_type=%d)\n", dev_id, vfid,
            (int)client->type);
        return 0;
    }
    mutex_lock(&instance->mutex);
    instance->flag = VMNG_INSTANCE_FLAG_UNINIT;
    mutex_unlock(&instance->mutex);
    vmng_info("Client uninit begin. (dev_id=%u; vfid=%u; client_type=%d)\n", dev_id, vfid, (int)client->type);
    ret = client->uninit_instance(instance);
    if (ret != 0) {
        vmng_err("Client uninit failed. (dev_id=%u; vfid=%u; client_type=%d; ret=%d)\n", dev_id, vfid,
            (int)client->type, ret);
    }
    vmng_info("Client uninit success. (dev_id=%u; vfid=%u; client_type=%d)\n", dev_id, vfid, (int)client->type);

    return ret;
}

static int vmngd_init_instance_proc(u32 dev_id, u32 vfid, struct vmngd_client_instance *instance,
    const struct vmngd_client *client)
{
    int ret;

    /* if client have not been registered. */
    if (client == NULL) {
        vmng_debug("Input parameter is error. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return 0;
    }
    if (client->init_instance == NULL) {
        vmng_debug("Client is no need to init instance. (dev_id=%u; vfid=%u; client_type=%d)\n", dev_id, vfid,
            (int)client->type);
        return 0;
    }
    mutex_lock(&instance->mutex);
    if (instance->flag == VMNG_INSTANCE_FLAG_UNINIT) {
        instance->flag = VMNG_INSTANCE_FLAG_INIT;
        mutex_unlock(&instance->mutex);
        vmng_info("Client init begin. (dev_id=%u; vfid=%u; client_type=%d)\n", dev_id, vfid, (int)client->type);
        ret = client->init_instance(instance);
        if (ret != 0) {
            mutex_lock(&instance->mutex);
            instance->flag = VMNG_INSTANCE_FLAG_UNINIT;
            mutex_unlock(&instance->mutex);
            vmng_err("Client init failed. (dev_id=%u; vfid=%u; client_type=%d; ret=%d)\n", dev_id, vfid,
                (int)client->type, ret);
            return ret;
        }
        vmng_info("Client init success. (dev_id=%u; vfid=%u; client_type=%d)\n", dev_id, vfid, (int)client->type);
    } else {
        vmng_info("Client already init. (dev_id=%u; vfid=%u; client_type=%d)\n", dev_id, vfid, (int)client->type);
        mutex_unlock(&instance->mutex);
    }

    return 0;
}

STATIC int vmngd_update_vfg_proc(u32 dev_id, u32 vfid, const struct vmngd_client *client)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);
    vmngd_vfg_cfg_info_t *vfg = NULL;
    vmngd_resource_unit_t *vf = NULL;
    struct vmng_soc_resource_enquire info;
    vmngd_resource_unit_t *node;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;
    int chip_type;
    int ret;

    chip_type = vmngd_get_dev_chip_type(dev_id);
    if (chip_type != HISI_MINI_V2) {
        return 0;
    }

    if ((client == NULL) || (client->type != VMNGD_CLIENT_TYPE_DVPP)) {
        return 0;
    }

    vf = &resource_info->vf_cfg_each[vfid - VMNGD_VF_NUM_MIN];
    vfg = vf->cfg_info.vfg_id >= BASE_VFG_NUM ? NULL :
          &resource_info->vfg_cfg_each[vf->cfg_info.vfg_id];
    if (vfg == NULL) {
        return 0;
    }
    list_for_each_safe(cur, next, &vfg->vf_list_head)
    {
        node = (vmngd_resource_unit_t *)list_entry(cur, vmngd_resource_unit_t, vf_node);
        (void)memset_s(&info, sizeof(info), 0, sizeof(info));
        ret = vmngd_enquire_soc_resource(dev_id, node->cfg_info.vfid, &info);
        if (ret != VMNG_OK) {
            vmng_err("Enquire vf resource info failed, (ret=%d).\n", ret);
            return VMNG_ERR;
        }
        if (client->update_vfg != NULL) {
            vmng_info("Start update vfg,(type=%d;devid=%u;vfid=%u;aic=%u;aicpu=%x;vfgid=%u;vfg_aic=%u;vfg_aicpu=%x)\n",
                (int)client->type, dev_id, info.each.vfid, info.each.stars_static.aic,
                info.each.stars_refresh.device_aicpu, info.each.vfg.vfg_id, info.vfg.stars_static.aic,
                info.vfg.stars_refresh.device_aicpu);
            ret = client->update_vfg(&info);
            if (ret != 0) {
                vmng_err("Update vfg resource info failed, (ret=%d, type=%d).\n", ret, (int)client->type);
                return VMNG_ERR;
            }
            vmng_info("End update vfg.\n");
        }
    }
    return 0;
}

int vmngd_init_instance_all_client(u32 dev_id, const struct vmng_vdev_ctrl *vd_ctrl)
{
    struct vmngd_client_instance *instance = NULL;
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    int type = 0;
    int ret;
    int i;
    u32 vfid = vd_ctrl->vfid;

    if (res_ops->unify_devid_to_pfvf_id != NULL) {
        ret = res_ops->unify_devid_to_pfvf_id(vd_ctrl->dev_id, &dev_id, &vfid);
        if (ret != VMNG_OK) {
            vmng_err("Unify_devid_to_pfvf_id err.(dev_id=%u, ret=%d)\n", dev_id, ret);
            return VMNG_ERR;
        }
    }

    if (res_ops->bind_vf_to_vfg != NULL) {
        ret = res_ops->bind_vf_to_vfg(dev_id, vfid);
        if (ret != VMNG_OK) {
            vmng_err("Bind_vf_to_vfg err.(dev_id=%u, ret=%d)\n", dev_id, ret);
            return VMNG_ERR;
        }
    }

    for (type = 0; type < VMNGD_CLIENT_TYPE_MAX; type++) {
        instance = &g_vmngd_clients_instance[type][dev_id][vfid];
        instance->vdev_ctrl = *vd_ctrl;
        ret = vmngd_init_instance_proc(dev_id, vfid, instance, g_vmngd_clients[type]);
        if (ret != 0) {
            vmng_err("Client init failed. (dev_id=%u; vfid=%u; client_type=%d; ret=%d)\n", dev_id, vfid, type, ret);
            goto OUT;
        }

        ret = vmngd_update_vfg_proc(dev_id, vfid, g_vmngd_clients[type]);
        if (ret != 0) {
            vmng_err("Update vfg resource info failed, (ret=%d, type=%d).\n", ret, type);
            (void)vmngd_uninit_instance_proc(dev_id, vfid, instance, g_vmngd_clients[type]);
            goto OUT;
        }
    }

    vmng_info("Init all client ok. (dev_id=%u; vfid=%u)\n", dev_id, vfid);

    return VMNG_OK;
OUT:
    if (res_ops->unbind_vf_to_vfg != NULL) {
        res_ops->unbind_vf_to_vfg(dev_id, vfid);
    }
    for (i = type - 1; i >= 0; --i) {
        instance = &g_vmngd_clients_instance[i][dev_id][vfid];
        (void)vmngd_update_vfg_proc(dev_id, vfid, g_vmngd_clients[i]);
        (void)vmngd_uninit_instance_proc(dev_id, vfid, instance, g_vmngd_clients[i]);
    }

    return ret;
}

void vmngd_init_instance_all_vdev(struct vmngd_client *client)
{
    int ret;
    struct vmngd_client_instance *instance = NULL;
    u32 dev_id;
    u32 vfid;

    for (dev_id = 0; dev_id < VMNGD_SURPORT_MAX_DEV; dev_id++) {
        for (vfid = 0; vfid < VMNG_VDEV_MAX_PER_PDEV; vfid++) {
            if (g_vdev_ctrl[dev_id][vfid].status != VMNG_VDEV_STATUS_ALLOC) {
                continue;
            }
            instance = &g_vmngd_clients_instance[client->type][dev_id][vfid];
            instance->vdev_ctrl = g_vdev_ctrl[dev_id][vfid];
            ret = vmngd_init_instance_proc(dev_id, vfid, instance, client);
            if (ret != 0) {
                vmng_err("Client init failed. (dev_id=%u; vfid=%u; client_type=%d; ret=%d)\n", dev_id, vfid,
                    (int)client->type, ret);
                continue;
            }
        }
    }
}

int vmngd_register_client(struct vmngd_client *client)
{
    if (client == NULL) {
        vmng_err("Input parameter is error.\n");
        return -EINVAL;
    }
    if (client->type >= VMNGD_CLIENT_TYPE_MAX) {
        vmng_err("Input parameter is error. (client_type=%d)\n", (int)client->type);
        return -EINVAL;
    }
    if (g_vmngd_clients[client->type] != NULL) {
        vmng_err("Client is already registered. (client_type=%d)\n", (int)client->type);
        return -EALREADY;
    }

    vmngd_init_instance_all_vdev(client);

    g_vmngd_clients[client->type] = client;

    vmng_info("Register client. (client_type=%d)\n", (int)client->type);
    return 0;
}
EXPORT_SYMBOL(vmngd_register_client);

void vmngd_uninit_instance_all_vdev(struct vmngd_client *client)
{
    int ret;
    struct vmngd_client_instance *instance = NULL;
    u32 dev_id, vfid;

    for (dev_id = 0; dev_id < VMNGD_SURPORT_MAX_DEV; dev_id++) {
        for (vfid = 0; vfid < VMNG_VDEV_MAX_PER_PDEV; vfid++) {
            if (g_vdev_ctrl[dev_id][vfid].status != VMNG_VDEV_STATUS_ALLOC) {
                continue;
            }
            instance = &g_vmngd_clients_instance[client->type][dev_id][vfid];
            ret = vmngd_uninit_instance_proc(dev_id, vfid, instance, client);
            if (ret != 0) {
                vmng_err("Client uninit failed. (dev_id=%u; vfid=%u; client_type=%d; ret=%d)\n", dev_id, vfid,
                    (int)client->type, ret);
                continue;
            }
        }
    }
}

int vmngd_unregister_client(struct vmngd_client *client)
{
    if (client == NULL) {
        vmng_err("Input parameter is error.\n.");
        return -EINVAL;
    }
    if (client->type >= VMNGD_CLIENT_TYPE_MAX) {
        vmng_err("Input parameter is error. (client_type=%d)\n", (int)client->type);
        return -EINVAL;
    }

    vmngd_uninit_instance_all_vdev(client);

    g_vmngd_clients[client->type] = NULL;

    vmng_info("Unregister client. (client_type=%d)\n", (int)client->type);

    return 0;
}
EXPORT_SYMBOL(vmngd_unregister_client);

int vmngd_get_dtype(u32 dev_id, u32 vfid, u32 *dtype)
{
    if (dev_id >= VMNGD_SURPORT_MAX_DEV) {
        vmng_err("Input parameter is error. (dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }
    if (vfid >= VMNG_VDEV_MAX_PER_PDEV) {
        vmng_err("Input parameter is error. (vfid=%u)\n", vfid);
        return VMNG_ERR;
    }

    if (dtype == NULL) {
        vmng_err("Input parameter is error.\n");
        return VMNG_ERR;
    }

    if (g_vdev_ctrl[dev_id][vfid].status != VMNG_VDEV_STATUS_ALLOC) {
        vmng_err("vdev not create. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    *dtype = g_vdev_ctrl[dev_id][vfid].dtype;

    return VMNG_OK;
}
EXPORT_SYMBOL(vmngd_get_dtype);

void vmngd_set_device_status(int devid, u32 valid)
{
    struct vmngd_dev *dev = &vmngdev_ctrl.devices[devid];
    dev->valid = valid;
    vmng_info("Set device status finished. (devid=%d; status=%u)\n", devid, valid);
}

void vmngd_set_peer_dev_id(int dev_id, int peer_dev_id)
{
    vmngdev_ctrl.devices[dev_id].peer_dev_id = peer_dev_id;
}

int vmngd_msg_recv_sync(u32 devid, struct vmng_ctrl_msg *msg)
{
#ifndef CFG_FEATURE_SOC_VIRTMNG
    int peer_dev_id = 0;

    peer_dev_id = agentdrv_get_host_devid(devid);
    vmngd_set_peer_dev_id(devid, peer_dev_id);

    /* notice host device devid */
    msg->sync_msg.dev_id = (int)devid;

    vmng_info("Call msg_recv_sync success. (dev_id=%u; peer_dev_id=%d)\n", devid, peer_dev_id);
#endif
    return VMNG_OK;
}

void vmngd_msg_recv_vdev_status_change(u32 dev_id, u32 vfid, u32 status)
{
    g_vdev_ctrl[dev_id][vfid].status = status;
}

int vmngd_msg_info_check(const struct vmng_ctrl_msg *msg)
{
    u32 vfid = msg->info_msg.vfid;

    if ((vfid >= VMNG_VDEV_MAX_PER_PDEV) || (vfid == 0)) {
        vmng_err("Input parameter is error. (vfid %u)\n", vfid);
        return VMNG_ERR;
    }

    return VMNG_OK;
}

STATIC int vmngd_register_ctrls(struct vmng_ctrl_msg_info *info)
{
    struct vmng_vdev_ctrl *vdev_ctrl = NULL;
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    int ret;

    vdev_ctrl = &g_vdev_ctrl[info->dev_id][info->vfid];
    mutex_lock(&g_vdev_ctrl_mutex[info->dev_id][info->vfid]);
    ret = vmngd_get_devid_by_pfvf_id(info->dev_id, info->vfid, &vdev_ctrl->dev_id);
    if (ret != 0) {
        vmng_err("Get vdevid by pfvf for ts error.(dev_id=%u; vfid=%u;ret=%d)\n", info->dev_id, info->vfid, ret);
        mutex_unlock(&g_vdev_ctrl_mutex[info->dev_id][info->vfid]);
        return ret;
    }

    vdev_ctrl->vfid = info->vfid;
    vdev_ctrl->dtype = info->dtype;
    vdev_ctrl->core_num = info->core_num;
    vdev_ctrl->total_core_num = info->total_core_num;
    if (res_ops->update_vdev_ctrl != NULL) {
        res_ops->update_vdev_ctrl(info->dev_id, info->vfid, vdev_ctrl);
        if (memcpy_s(&info->vf_cfg, sizeof(vmng_vf_cfg_t), &vdev_ctrl->vf_cfg, sizeof(vmng_vf_cfg_t)) != EOK) {
            mutex_unlock(&g_vdev_ctrl_mutex[info->dev_id][info->vfid]);
            vmng_err("Call memcpy_s error.(dev_id=%u; vfid=%u)\n", info->dev_id, info->vfid);
            return VMNG_ERR;
        }
    }
    mutex_unlock(&g_vdev_ctrl_mutex[info->dev_id][info->vfid]);
    vmng_info("vmngd_register_ctrls.(dev_id=%u; vfid=%u; vdev_id=%u)\n", info->dev_id, info->vfid, vdev_ctrl->dev_id);
    return VMNG_OK;
}

STATIC void vmngd_unregister_ctrls(const struct vmng_ctrl_msg_info *info)
{
    struct vmng_vdev_ctrl *vdev_ctrl = NULL;
    u32 old_status;

    vdev_ctrl = &g_vdev_ctrl[info->dev_id][info->vfid];
    old_status = vdev_ctrl->status;

    mutex_lock(&g_vdev_ctrl_mutex[info->dev_id][info->vfid]);
    (void)memset_s(vdev_ctrl, sizeof(struct vmng_vdev_ctrl), 0, sizeof(struct vmng_vdev_ctrl));
    vdev_ctrl->status = old_status;
    mutex_unlock(&g_vdev_ctrl_mutex[info->dev_id][info->vfid]);
}

/* call when vd_dev remove. */
void vmngd_uninit_instance_all_client(u32 dev_id, const struct vmng_vdev_ctrl *vd_ctrl)
{
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    struct vmngd_client_instance *instance = NULL;
    int type;
    int ret;
    u32 vfid = vd_ctrl->vfid;

    if (res_ops->unify_devid_to_pfvf_id != NULL) {
        ret = res_ops->unify_devid_to_pfvf_id(vd_ctrl->dev_id, &dev_id, &vfid);
        if (ret != VMNG_OK) {
            vmng_err("Unify_devid_to_pfvf_id err.(dev_id=%u, ret=%d)\n", dev_id, ret);
            return;
        }
    }

    if (res_ops->unbind_vf_to_vfg != NULL) {
        res_ops->unbind_vf_to_vfg(dev_id, vfid);
    }

    for (type = VMNGD_CLIENT_TYPE_MAX - 1; type >= 0; type--) {
        ret = vmngd_update_vfg_proc(dev_id, vfid, g_vmngd_clients[type]);
        if (ret) {
            vmng_err("Update vfg resource info failed, (ret=%d, type=%d).\n", ret, type);
        }

        instance = &g_vmngd_clients_instance[type][dev_id][vfid];
        instance->vdev_ctrl = *vd_ctrl;
        ret = vmngd_uninit_instance_proc(dev_id, vfid, instance, g_vmngd_clients[type]);
        if (ret != 0) {
            vmng_err("Client uninit failed. (dev_id=%u; vfid=%u; type=%d; ret=%d)\n", dev_id, vfid, type, ret);
            continue;
        }
    }
    vmng_info("Uninit all client ok. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
}

static int vmngd_set_pcie_msg_dev_status(u32 dev_id, int status)
{
#ifndef CFG_FEATURE_SOC_VIRTMNG
    if (vmngd_get_pfvf_type_by_devid(dev_id) == VMNGD_TYPE_VF) {
        return agentdrv_set_msg_dev_status(dev_id, status);
    }
#endif
    return 0;
}

int vmngd_execute_client_offline(u32 dev_id, struct vmng_ctrl_msg_info *info)
{
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    struct vmng_vdev_ctrl *vdev_ctrl = NULL;
    u32 vfid;
    int ret = VMNG_OK;

    vfid = info->vfid;
    vdev_ctrl = &g_vdev_ctrl[dev_id][vfid];
    if ((vdev_ctrl->status != VMNG_VDEV_STATUS_RESET) && (vdev_ctrl->status != VMNG_VDEV_STATUS_CLIENT_INIT)) {
        vmng_err("vdev status check failed. (dev_id=%u; vfid=%u; status=%u)\n", dev_id, vfid, vdev_ctrl->status);
        return VMNG_ERR;
    }

    (void)vmngd_set_pcie_msg_dev_status(vdev_ctrl->dev_id, DEVDRV_DEV_OFFLINE);
    vmngd_uninit_instance_all_client(dev_id, vdev_ctrl);
    if (res_ops->pci_client_offline != NULL) {
        ret = res_ops->pci_client_offline(dev_id, vfid, vdev_ctrl);
        if (ret != VMNG_OK) {
            vmng_err("pci_client_offline error. (dev_id=%u;vfid=%u;vdev_id=%u;ret=%d)\n", dev_id, vfid,
                vdev_ctrl->dev_id, ret);
        }
    }

    if (res_ops->alloc_vf == NULL) {
        vmngd_unregister_ctrls(info);
    }

    vmngd_msg_recv_vdev_status_change(dev_id, vfid, VMNG_VDEV_STATUS_CLIENT_UNINIT);
    vmng_info("Client unint. (dev_id=%u; vfid=%u)\n", dev_id, vfid);

    return ret;
}

int vmngd_msg_recv_destory_vdev(u32 dev_id, struct vmng_ctrl_msg *msg)
{
    int ret;

    if (vmngd_msg_info_check(msg)) {
        vmng_err("Message information vfid check failed. (dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }

    if (vmngd_is_sriov_support(dev_id) == false) {
        vmngd_bw_data_clear_timer_uninit();
    }

    ret = vmngd_execute_client_offline(dev_id, &msg->info_msg);
    if (ret != 0) {
        vmng_err("Client offline err. (dev_id=%u; vfid=%u)\n", dev_id, msg->info_msg.vfid);
        return VMNG_ERR;
    }

    vmng_info("Destory vdev. (dev_id=%u; vfid=%u)\n", dev_id, msg->info_msg.vfid);
    return VMNG_OK;
}

int vmngd_execute_alloc_vf(u32 dev_id, struct vmng_ctrl_msg_info *info)
{
    u32 vfid;
    int ret;
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    struct vmng_vdev_ctrl *vdev_ctrl = NULL;

    if (res_ops->alloc_vf == NULL) {
        vmng_err("Not support alloc VF. (dev_id=%u; vfid=%u)\n", dev_id, info->vfid);
        return VMNG_ERR;
    }

    vfid = info->vfid;
    if ((vfid >= VMNG_VDEV_MAX_PER_PDEV) || (vfid == 0)) {
        vmng_err("vfid parameter is error. (vfid %u)\n", vfid);
        return VMNG_ERR;
    }

    vdev_ctrl = &g_vdev_ctrl[dev_id][vfid];
    if (vdev_ctrl->status != VMNG_VDEV_STATUS_FREE) {
        vmng_err("vdev status check failed. (dev_id=%u; vfid=%u; status=%u)\n", dev_id, vfid, vdev_ctrl->status);
        return VMNG_ERR;
    }

    if (res_ops->alloc_vf(info) != VMNG_OK) {
        vmng_err("Alloc vf err. (dev_id=%u; vfid=%u)\n", dev_id, info->vfid);
        return VMNG_ERR;
    }

    vmngd_msg_recv_vdev_status_change(dev_id, vfid, VMNG_VDEV_STATUS_ALLOC);
    ret = vmngd_register_ctrls(info);
    if (ret != 0) {
        vmng_err("Register ctrls err. (dev_id=%u; vfid=%u)\n", dev_id, info->vfid);
        goto EXIT;
    }
    return VMNG_OK;

EXIT:
    if (res_ops->free_vf != NULL) {
        (void)res_ops->free_vf(dev_id, vfid);
    }
    return VMNG_ERR;
}

int vmngd_msg_recv_alloc_vf(u32 dev_id, struct vmng_ctrl_msg *msg)
{
    int ret;

    ret = vmngd_execute_alloc_vf(dev_id, &msg->info_msg);
    if (ret != 0) {
        vmng_err("Alloc vdev failed. (dev_id=%u; vfid=%u)\n", dev_id, msg->info_msg.vfid);
        return VMNG_ERR;
    }

    vmng_info("Alloc vdev. (dev_id=%u; vfid=%u)\n", dev_id, msg->info_msg.vfid);
    return VMNG_OK;
}

int vmngd_msg_recv_enquire_resource(u32 dev_id, struct vmng_ctrl_msg *msg)
{
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    int ret;

    if (res_ops->res_enquire == NULL) {
        vmng_err("Not support enquire VF. (dev_id=%u; vfid=%u)\n", dev_id, msg->info_msg.vfid);
        return VMNG_ERR;
    }

    if (msg->info_msg.vfid >= VMNG_VDEV_MAX_PER_PDEV) {
        vmng_err("Message information vfid check failed. (dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }

    if ((res_ops->bind_vf_to_vfg != NULL) && (msg->info_msg.vfid != 0)) {
        ret = res_ops->bind_vf_to_vfg(dev_id, msg->info_msg.vfid);
        if (ret != VMNG_OK) {
            vmng_err("Bind_vf_to_vfg err.(dev_id=%u, ret=%d)\n", dev_id, ret);
            return VMNG_ERR;
        }
    }

    ret = memset_s(&msg->info_msg.enquire, sizeof(msg->info_msg.enquire), 0xff, sizeof(msg->info_msg.enquire));
    if (ret != 0) {
        vmng_err("memset_s failed, (ret=%d)\n", ret);
        return VMNG_ERR;
    }
    (void)memset_s(msg->info_msg.enquire.each.name, VMNG_VF_TEMP_NAME_LEN, 0, VMNG_VF_TEMP_NAME_LEN);
    if (res_ops->res_enquire(dev_id, msg->info_msg.vfid, &msg->info_msg.enquire) != VMNG_OK) {
        vmng_err("enquire soc source info err. (dev_id=%u; vfid=%u)\n", dev_id, msg->info_msg.vfid);
        return VMNG_ERR;
    }

    return VMNG_OK;
}

int vmngd_enquire_vfg_resource(u32 dev_id, u32 vfid, struct vmng_soc_res_info *vfg_info)
{
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    u32 pf_id = dev_id;
    u32 vf_id = vfid;

    if (res_ops->res_enquire_vfg == NULL) {
        vmng_err("Not support enquire VFG. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    if (pf_id >= VMNGD_SURPORT_MAX_DEV || vf_id > BASE_VF_NUM || vf_id == 0) {
        vmng_err("vfid check failed. (dev_id=%u, pf_id=%u, vfid=%u)\n", dev_id, pf_id, vfid);
        return VMNG_ERR;
    }

    if (res_ops->res_enquire_vfg(pf_id, vf_id, vfg_info) != VMNG_OK) {
        vmng_err("enquire vfg resource info err. (dev_id=%u; vfid=%u)\n", dev_id, vf_id);
        return VMNG_ERR;
    }

    return VMNG_OK;
}
EXPORT_SYMBOL(vmngd_enquire_vfg_resource);

int vmngd_execute_client_online(u32 dev_id, struct vmng_ctrl_msg_info *info)
{
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    struct vmng_vdev_ctrl *vdev_ctrl = NULL;
    u32 vfid;
    int ret;

    vfid = info->vfid;
    vdev_ctrl = &g_vdev_ctrl[dev_id][vfid];
    if ((vdev_ctrl->status != VMNG_VDEV_STATUS_ALLOC) && (vdev_ctrl->status != VMNG_VDEV_STATUS_CLIENT_UNINIT) &&
        (vdev_ctrl->status != VMNG_VDEV_STATUS_REFRESH)) {
        vmng_err("vdev status check failed. (dev_id=%u; vfid=%u; status=%u)\n", dev_id, vfid, vdev_ctrl->status);
        return VMNG_ERR;
    }

    if (res_ops->alloc_vf == NULL) {
        ret = vmngd_register_ctrls(info);
        if (ret != 0) {
            vmng_err("Register ctrls err. (dev_id=%u; vfid=%u)\n", dev_id, info->vfid);
            return ret;
        }
    }

    if ((res_ops->pci_client_online != NULL) &&
        (res_ops->pci_client_online(dev_id, vfid, vdev_ctrl) != VMNG_OK)) {
        vmng_err("pci_client_online err. (dev_id=%u; vfid=%u)\n", dev_id, info->vfid);
        return VMNG_ERR;
    }

    ret = vmngd_init_instance_all_client(dev_id, vdev_ctrl);
    if (ret != 0) {
        vmng_err("Instance all client failed. (dev_id=%u; vfid=%u; ret=%d)\n", dev_id, vfid, ret);
        if ((res_ops->pci_client_offline != NULL)) {
            (void)res_ops->pci_client_offline(dev_id, vfid, vdev_ctrl);
        }
        if (res_ops->alloc_vf == NULL) {
            vmngd_unregister_ctrls(info);
        }
        return ret;
    }

    vmngd_msg_recv_vdev_status_change(dev_id, vfid, VMNG_VDEV_STATUS_CLIENT_INIT);

    return VMNG_OK;
}

int vmngd_msg_recv_create_vdev(u32 dev_id, struct vmng_ctrl_msg *msg)
{
    struct vmng_vdev_ctrl *vdev_ctrl = NULL;
    u32 vfid;
    int ret;

    if (vmngd_msg_info_check(msg)) {
        vmng_err("Message information vfid check failed. (dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }

    vfid = msg->info_msg.vfid;
    vdev_ctrl = &g_vdev_ctrl[dev_id][vfid];

    if (g_vmngd_clients[VMNGD_CLIENT_TYPE_TSD] == NULL &&
        (vmngd_is_sriov_support(dev_id) == false)) {
        vmng_err("Not registered client. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    (void)vmngd_set_pcie_msg_dev_status(vdev_ctrl->dev_id, DEVDRV_DEV_ONLINE);
    ret = vmngd_execute_client_online(dev_id, &msg->info_msg);
    if (ret != 0) {
        vmng_err("Client online err. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return ret;
    }

    if (vmngd_is_sriov_support(dev_id) == false) {
        (void)vmngd_bw_data_clear(dev_id, vfid);
        vmngd_bw_data_clear_timer_init();
    }
    vmng_info("Get vdev_ctrl value. (dev_id=%u; vfid=%u; vdev_id=%u; dtype=%u; core_num=%u; total_core_num=%u)\n",
        dev_id, vfid, vdev_ctrl->dev_id, vdev_ctrl->dtype, vdev_ctrl->core_num, vdev_ctrl->total_core_num);

    return VMNG_OK;
}

int vmngd_ctrl_msg_para_check(u32 devid, void *data, u32 in_data_len, u32 out_data_len, const u32 *p_real_out_len)
{
    struct vmng_ctrl_msg *msg = (struct vmng_ctrl_msg *)data;
    u32 len_min;

    if ((devid >= VMNGD_SURPORT_MAX_DEV) || (msg == NULL) || (p_real_out_len == NULL)) {
        vmng_err("Invalid value. (devid=%u)\n", devid);
        return -EINVAL;
    }

    len_min = sizeof(struct vmng_ctrl_msg);
    if ((in_data_len < len_min) || (out_data_len < len_min)) {
        vmng_err("data_len is invalid. (dev_id=%u; in_len=%u; min_size=%u; out_len=%u)\n", devid, in_data_len, len_min,
            out_data_len);
        return -EINVAL;
    }

    if (vmngd_get_device_status(devid) != VMNG_VALID) {
        vmng_warn("Device is not init instance. (devid=%u)\n", devid);
        return -ENODEV;
    }
    return VMNG_OK;
}

int vmngd_msg_recv_refresh_vf(u32 dev_id, struct vmng_ctrl_msg *msg)
{
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    struct vmng_vdev_ctrl *vdev_ctrl = NULL;
    u32 vfid;

    if (res_ops->refresh_vf == NULL) {
        vmng_err("Not support refresh VF. (dev_id=%u; vfid=%u)\n", dev_id, msg->info_msg.vfid);
        return VMNG_ERR;
    }

    if (vmngd_msg_info_check(msg)) {
        vmng_err("Message information vfid check failed. (dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }

    vfid = msg->info_msg.vfid;
    vdev_ctrl = &g_vdev_ctrl[dev_id][vfid];
    if ((vdev_ctrl->status != VMNG_VDEV_STATUS_ALLOC) && (vdev_ctrl->status != VMNG_VDEV_STATUS_CLIENT_UNINIT)) {
        vmng_err("vdev status check failed. (dev_id=%u; vfid=%u; status=%u)\n", dev_id, vfid, vdev_ctrl->status);
        return VMNG_ERR;
    }

    if (res_ops->refresh_vf(&msg->info_msg) != VMNG_OK) {
        vmng_err("Refresh vf err. (dev_id=%u; cmd=%d)\n", dev_id, msg->type);
        return VMNG_ERR;
    }

    vmngd_msg_recv_vdev_status_change(dev_id, vfid, VMNG_VDEV_STATUS_REFRESH);

    return VMNG_OK;
}

int vmngd_execute_free_vf(u32 dev_id, struct vmng_ctrl_msg_info *info)
{
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    struct vmng_vdev_ctrl *vdev_ctrl = NULL;
    u32 vfid;

    if (res_ops->free_vf == NULL) {
        vmng_err("Not support free VF. (dev_id=%u; vfid=%u)\n", dev_id, info->vfid);
        return VMNG_ERR;
    }

    vfid = info->vfid;
    vdev_ctrl = &g_vdev_ctrl[dev_id][vfid];
    if ((vdev_ctrl->status != VMNG_VDEV_STATUS_ALLOC) && (vdev_ctrl->status != VMNG_VDEV_STATUS_CLIENT_UNINIT) &&
        (vdev_ctrl->status != VMNG_VDEV_STATUS_REFRESH)) {
        vmng_err("vdev status check failed. (dev_id=%u; vfid=%u; status=%u)\n", dev_id, vfid, vdev_ctrl->status);
        return VMNG_ERR;
    }

    if (res_ops->free_vf(dev_id, vfid) != VMNG_OK) {
        vmng_err("Free vf err. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    if (res_ops->free_vfid != NULL) {
        res_ops->free_vfid(dev_id, vfid);
    }

    vmngd_unregister_ctrls(info);
    vmngd_msg_recv_vdev_status_change(dev_id, vfid, VMNG_VDEV_STATUS_FREE);

    vmng_info("Free vf ok.(dev_id=%u, vfid=%u)\n", dev_id, vfid);

    return VMNG_OK;
}

int vmngd_msg_recv_free_vf(u32 dev_id, struct vmng_ctrl_msg *msg)
{
    int ret;

    if (vmngd_msg_info_check(msg)) {
        vmng_err("Message information vfid check failed. (dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }

    ret = vmngd_execute_free_vf(dev_id, &msg->info_msg);
    if (ret != 0) {
        vmng_err("Free vf err. (dev_id=%u; vfid=%u)\n", dev_id, msg->info_msg.vfid);
        return VMNG_ERR;
    }

    return VMNG_OK;
}

int vmngd_msg_recv_sriov_info(u32 dev_id, struct vmng_ctrl_msg *msg)
{
    struct vmng_sriov_info sriov_info = {0};
    struct vmngd_client *client;
    int type;
    int ret;

    sriov_info.dev_id = dev_id;
    sriov_info.sriov_status = msg->info_msg.sriov_status;

    for (type = 0; type < VMNGD_CLIENT_TYPE_MAX; type++) {
        client = g_vmngd_clients[type];
        if (client == NULL || client->sriov_instance == NULL) {
            vmng_info("Client not register sriov_instance function. (type=%d).\n", type);
            continue;
        }

        ret = client->sriov_instance(&sriov_info);
        if (ret != 0) {
            vmng_err("Client sriov instance failed. (dev_id=%u;client_type=%d;ret=%d;status=%d)\n",
                     dev_id, type, ret, (int)sriov_info.sriov_status);
            goto FAILED;
        }
    }

    ret = vmngd_sriov_modify_soc_res(dev_id, sriov_info.sriov_status);
    if (ret != 0) {
        vmng_err("Modify soc resource failed. (dev_id=%u;status=%d)\n", dev_id, sriov_info.sriov_status);
        goto FAILED;
    }

    vmng_info("Sriov inform success.(dev_id=%u, sriov_status=%d)\n", dev_id, (int)sriov_info.sriov_status);

    return VMNG_OK;

FAILED:
    sriov_info.sriov_status = (sriov_info.sriov_status == VMNGH_PF_SRIOV_DISABLE) ?
        VMNGH_PF_SRIOV_ENABLE : VMNGH_PF_SRIOV_DISABLE;   // ENABLE(1) <--> DISABLE(0)
    for (type--; type >= 0; --type) {
        client = g_vmngd_clients[type];
        if ((client != NULL) && (client->sriov_instance != NULL)) {
            (void)client->sriov_instance(&sriov_info);
        }
    }
    return VMNG_ERR;
}

int vmngd_msg_recv_mdev_iova_info(u32 dev_id, struct vmng_ctrl_msg *msg)
{
    struct vmng_mdev_iova_info *iova_info = NULL;
    struct vmng_mdev_iova *dev_iova = NULL;
    unsigned int vdev_id = 0;
    unsigned int vfid;
    int ret;

    vfid = msg->info_msg.vfid;
    iova_info = (struct vmng_mdev_iova_info *)&msg->info_msg.iova_info;

    ret = vmngd_get_devid_by_pfvf_id(dev_id, vfid, &vdev_id);
    if (ret != 0) {
        vmng_err("Get vdevid failed. (dev_id=%u;vfid=%u;ret=%d)\n", dev_id, vfid, ret);
        return ret;
    }

    if (vfid >= VMNG_VDEV_MAX_PER_PDEV) {
        vmng_err("Invalid vfid. (dev_id=%u;vfid=%u)\n", dev_id, vfid);
        return -EINVAL;
    }

    dev_iova = &vmngdev_ctrl.devices[dev_id].mdev_iova[vfid];
    dev_iova->iova_base = iova_info->iova_base;
    dev_iova->iova_end = iova_info->iova_base + iova_info->size;
    if (dev_iova->iova_base > dev_iova->iova_end) {
        vmng_err("Invalid iova info. (vdev_id=%u)\n", vdev_id);
        return -EINVAL;
    }
#ifndef CFG_FEATURE_SOC_VIRTMNG
    // set pcie iova
    ret = devdrv_mdev_set_pm_iova_addr_range(vdev_id, iova_info->iova_base, iova_info->size);
    if (ret != 0) {
        vmng_err("Set iova info to pcie failed.(vdev_id=%u;ret=%d)\n", vdev_id, ret);
        return ret;
    }
#endif
    return VMNG_OK;
}

int vmngd_msg_sync_remote_id_info(u32 dev_id, struct vmng_ctrl_msg *msg)
{
    struct vmng_vf_sync_remote_id *id_info = NULL;
    struct uda_mia_dev_para mia_para = {0};
    u32 udevid;
    u32 vfid;
    int ret;

    vfid = msg->info_msg.vfid;

    if (vfid == 0) {
        udevid = dev_id;
    } else {
        mia_para.phy_devid = dev_id;
        mia_para.sub_devid = vfid - 1;
        ret = uda_mia_devid_to_udevid(&mia_para, &udevid);
        if (ret != 0) {
            vmng_err("Get udevid failed. (dev_id=%u;vfid=%u;ret=%d)\n", dev_id, vfid, ret);
            return ret;
        }
    }

    id_info = (struct vmng_vf_sync_remote_id *)&msg->info_msg.id_info;
    /* set remote udevid to uda */
    ret = uda_dev_set_remote_udevid(udevid, id_info->udevid);
    if (ret != 0) {
        vmng_err("Set remote udevid failed.(udevid=%u;ret=%d)\n", udevid, ret);
        return ret;
    }

    return VMNG_OK;
}
int vmng_check_vdev_iova_address(unsigned int dev_id, dma_addr_t iova_addr, size_t size)
{
    struct vmng_mdev_iova *dev_iova = NULL;
    unsigned int pf_id = 0;
    unsigned int vf_id = 0;
    int ret;

    if (vmngd_get_pfvf_type_by_devid(dev_id) == DEVDRV_SRIOV_TYPE_PF) { // PF no need to check iova
        return 0;
    }

    ret = vmngd_get_pfvf_id_by_devid(dev_id, &pf_id, &vf_id);
    if (ret != 0) {
        vmng_err("Get pfid and vfid by devid failed. (dev_id=%u;ret=%d)\n", dev_id, ret);
        return ret;
    }
    if (pf_id >= VMNGD_SURPORT_MAX_DEV || vf_id >= VMNG_VDEV_MAX_PER_PDEV) {
        vmng_err("pfid or vfid is invalid. (pfid=%u;vfid=%u)\n", pf_id, vf_id);
        return -EINVAL;
    }

    dev_iova = &vmngdev_ctrl.devices[pf_id].mdev_iova[vf_id];
    if (iova_addr + size <= iova_addr) {
        return -EINVAL;
    }
    if ((iova_addr >= dev_iova->iova_base) && (iova_addr < dev_iova->iova_end)) {
        return -EINVAL;
    }
    if ((iova_addr + size > dev_iova->iova_base) && (iova_addr + size <= dev_iova->iova_end)) {
        return -EINVAL;
    }
    return 0;
}
EXPORT_SYMBOL(vmng_check_vdev_iova_address);

typedef int (*vmngd_msg_proc)(u32 devid, struct vmng_ctrl_msg *msg);
vmngd_msg_proc g_vmngd_msg_proc[] = {
    [VMNG_CTRL_MSG_TYPE_SYNC] = vmngd_msg_recv_sync,
    [VMNG_CTRL_MSG_TYPE_INIT_CLIENT] = vmngd_msg_recv_create_vdev,
    [VMNG_CTRL_MSG_TYPE_UNINIT_CLIENT] = vmngd_msg_recv_destory_vdev,
    [VMNG_CTRL_MSG_TYPE_ALLOC_VF] = vmngd_msg_recv_alloc_vf,
    [VMNG_CTRL_MSG_TYPE_FREE_VF] = vmngd_msg_recv_free_vf,
    [VMNG_CTRL_MSG_TYPE_ENQUIRE_VF] = vmngd_msg_recv_enquire_resource,
    [VMNG_CTRL_MSG_TYPE_REFRESH_VF] = vmngd_msg_recv_refresh_vf,
    [VMNG_CTRL_MSG_TYPE_SRIOV_INFO] = vmngd_msg_recv_sriov_info,
    [VMNG_CTRL_MSG_TYPE_IOVA_INFO] = vmngd_msg_recv_mdev_iova_info,
    [VMNG_CTRL_MSG_TYPE_SYNC_ID] = vmngd_msg_sync_remote_id_info
};

int vmngd_ctrl_msg_recv(u32 devid, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len)
{
    struct vmng_ctrl_msg *msg = (struct vmng_ctrl_msg *)data;

    if (vmngd_ctrl_msg_para_check(devid, data, in_data_len, out_data_len, real_out_len) != VMNG_OK) {
        vmng_warn("Ctrl msg parameters is illegal or device is not ready. (dev_id=%u)\n", devid);
        return -EINVAL;
    }

    msg->error_code = VMNG_OK;
    *real_out_len = (sizeof(struct vmng_ctrl_msg));
    msg->info_msg.dev_id = devid;

    if ((msg->type >= VMNG_CTRL_MSG_TYPE_MAX) || (msg->type < 0) || (g_vmngd_msg_proc[msg->type] == NULL)) {
        vmng_err("Command is illegal. (dev_id=%u; cmd=%d)\n", devid, msg->type);
        return -EINVAL;
    }
    if (msg->type == VMNG_CTRL_MSG_TYPE_SYNC) {
        msg->sync_msg.dev_id = (int)devid;
    } else {
        msg->info_msg.dev_id = devid;
    }

    msg->error_code = g_vmngd_msg_proc[msg->type](devid, msg);
    if (msg->error_code != VMNG_OK) {
        vmng_err("Command executed failed. (dev_id=%u; cmd=%d; error_code=%d)\n", devid, msg->type, msg->error_code);
    }

    return VMNG_OK;
}

struct agentdrv_common_msg_client vmngd_device_comm_msg_client = {
    .type = AGENTDRV_COMMON_MSG_VMNG,
    .common_msg_recv = vmngd_ctrl_msg_recv,
};

STATIC int vmngd_bw_excess_bandwidth_judge(u32 dev_id, u32 vfid, u32 dir, bool *bandwidth_full)
{
    struct vf_bandwidth_ctrl_remote *vf_ptr = NULL;
    u64 single_flow, total_flow, single_pack;
    int ret;

    ret = vmngd_bw_get_vfio_barspace(dev_id, vfid, &vf_ptr);
    if (ret) {
        vmng_err("Get vfio base failed. (dev_id=%u; fid=%u)\n", dev_id, vfid);
        return ret;
    }

    if (dir == VMNG_PCIE_FLOW_H2D) {
        single_flow = vf_ptr->hostcpu_flow_cnt[VMNG_PCIE_FLOW_H2D] + vf_ptr->ctrlcpu_flow_cnt[VMNG_PCIE_FLOW_H2D] +
            vf_ptr->tscpu_flow_cnt[VMNG_PCIE_FLOW_H2D];
        total_flow = vf_ptr->hostcpu_flow_cnt[VMNG_PCIE_FLOW_D2H] + vf_ptr->ctrlcpu_flow_cnt[VMNG_PCIE_FLOW_D2H] +
            vf_ptr->tscpu_flow_cnt[VMNG_PCIE_FLOW_D2H] + single_flow;
        single_pack = vf_ptr->hostcpu_pack_cnt[VMNG_PCIE_FLOW_H2D] + vf_ptr->ctrlcpu_pack_cnt[VMNG_PCIE_FLOW_H2D] +
            vf_ptr->tscpu_pack_cnt[VMNG_PCIE_FLOW_H2D];
    } else {
        single_flow = vf_ptr->hostcpu_flow_cnt[VMNG_PCIE_FLOW_D2H] + vf_ptr->ctrlcpu_flow_cnt[VMNG_PCIE_FLOW_D2H] +
            vf_ptr->tscpu_flow_cnt[VMNG_PCIE_FLOW_D2H];
        total_flow = vf_ptr->hostcpu_flow_cnt[VMNG_PCIE_FLOW_H2D] + vf_ptr->ctrlcpu_flow_cnt[VMNG_PCIE_FLOW_H2D] +
            vf_ptr->tscpu_flow_cnt[VMNG_PCIE_FLOW_H2D] + single_flow;
        single_pack = vf_ptr->hostcpu_pack_cnt[VMNG_PCIE_FLOW_D2H] + vf_ptr->ctrlcpu_pack_cnt[VMNG_PCIE_FLOW_D2H] +
            vf_ptr->tscpu_pack_cnt[VMNG_PCIE_FLOW_D2H];
    }

    if ((single_flow >= vf_ptr->flow_limit) || (single_pack >= vf_ptr->pack_limit) ||
        (total_flow >= (vf_ptr->flow_limit * VMNG_BANDW_DUPLEX_PERCENTAGE / VMNG_BANDW_PERCENTAGE_BASE))) {
        *bandwidth_full = true;
    } else {
        *bandwidth_full = false;
    }

    return 0;
}

STATIC int vmngd_bw_set_ctrlcpu_data(u32 dev_id, u32 vfid, u32 dir, u64 data_len, u32 node_cnt)
{
    struct vf_bandwidth_ctrl_remote *vf_ptr = NULL;
    int ret;

    ret = vmngd_bw_get_vfio_barspace(dev_id, vfid, &vf_ptr);
    if (ret) {
        vmng_err("Get vfio base failed. (dev_id=%u; fid=%u)\n", dev_id, vfid);
        return ret;
    }

    vf_ptr->ctrlcpu_flow_cnt[dir] += data_len;
    vf_ptr->ctrlcpu_pack_cnt[dir] += node_cnt;

    return 0;
}

int vmng_bandwidth_limit_check(struct vmng_bandwidth_check_info *info)
{
    bool bandwidth_full = false;
    int retry_cnt = 0;
    int ret;

    if (info == NULL) {
        vmng_err("Input para error, info is NULL\n");
        return -EINVAL;
    }

    if ((info->dev_id >= VMNGD_SURPORT_MAX_DEV) || (info->vfid >= VMNG_VDEV_MAX_PER_PDEV) ||
        (info->dir > VMNG_PCIE_FLOW_D2H)) {
        vmng_err("Input parameter is error. (dev_id=%u; vfid=%u; dir=%u)\n", info->dev_id, info->vfid, info->dir);
        return -EINVAL;
    }

    /* no need to judge physical machine or length less than 1K */
    if ((info->vfid < VMNG_VDEV_FIRST_VFID) || (info->data_len < VMNG_BW_BANDWIDTH_CHECK_LEN)) {
        return 0;
    }

RETRY_CHECK:
    ret = vmngd_bw_excess_bandwidth_judge(info->dev_id, info->vfid, info->dir, &bandwidth_full);
    if (ret) {
        vmng_err("Update host bandwidth error, excess judge fail. (ret=%d)\n", ret);
        return ret;
    }

    if (bandwidth_full) {
        if ((retry_cnt > VMNG_BW_BANDWIDTH_CHECK_MAX_CNT) || (info->handle_mode == VMNG_BW_BANDWIDTH_CHECK_NON_SLEEP)) {
            return -EBUSY;
        }
        msleep(VMNG_BW_BANDWIDTH_CHECK_WAIT_TIME);
        retry_cnt++;
        goto RETRY_CHECK;
    } else {
        ret = vmngd_bw_set_ctrlcpu_data(info->dev_id, info->vfid, info->dir, info->data_len, info->node_cnt);
        if (ret) {
            vmng_err("Set ctrlcpu data error. (ret=%d)\n", ret);
            return ret;
        }
    }

    return 0;
}
EXPORT_SYMBOL(vmng_bandwidth_limit_check);

int vmngd_enquire_soc_resource(u32 dev_id, u32 vfid, struct vmng_soc_resource_enquire *info)
{
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    u32 pf_id = dev_id;
    u32 vf_id = vfid;

    if (res_ops->res_enquire == NULL) {
        vmng_warn("res_enquire is NULL. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_OK;
    }

    if ((res_ops->is_vf != NULL) && res_ops->is_vf(dev_id)) {
        if (res_ops->unify_devid_to_pfvf_id != NULL) {
            res_ops->unify_devid_to_pfvf_id(dev_id, &pf_id, &vf_id);
        }
    }

    if ((pf_id >= VMNGD_SURPORT_MAX_DEV) || (vf_id > BASE_VF_NUM)) {
        vmng_err("vfid check failed. (dev_id=%u,vfid=%u)\n", pf_id, vf_id);
        return VMNG_ERR;
    }

    if (res_ops->res_enquire(pf_id, vf_id, info) != VMNG_OK) {
        vmng_err("enquire soc resource info err. (dev_id=%u; vfid=%u)\n", pf_id, vf_id);
        return VMNG_ERR;
    }

    return VMNG_OK;
}
EXPORT_SYMBOL(vmngd_enquire_soc_resource);

STATIC void vmngd_set_alloc_vf_para(u32 dev_id, u32 dtype, u32 vfid, struct vmng_vf_res_info *vf_resource,
                                    struct vmng_ctrl_msg_info *info)
{
    info->dev_id = dev_id;
    info->vfid = vfid;
    info->dtype = dtype;
    if (memcpy_s(&info->vf_cfg, sizeof(struct vmng_vf_res_info), vf_resource, sizeof(struct vmng_vf_res_info)) != EOK) {
        vmng_err("Call memcpy_s failed.\n");
        return;
    }
}

struct vmng_vdev_ctrl *vmngd_get_ctrl(u32 dev_id, u32 vfid)
{
    if (dev_id >= VMNGD_SURPORT_MAX_DEV) {
        vmng_err("Input parameter is error. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return NULL;
    }
    if (vfid > BASE_VF_NUM) {
        vmng_err("Input parameter is error. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return NULL;
    }

    return &g_vdev_ctrl[dev_id][vfid];
}

struct mutex *vmngd_get_ctrl_mutex(u32 dev_id, u32 vfid)
{
    return &g_vdev_ctrl_mutex[dev_id][vfid];
}

int vmngd_dev_id_check(u32 dev_id, u32 fid)
{
    if (dev_id >= VMNGD_SURPORT_MAX_DEV) {
        vmng_err("Input parameter is error. (dev_id=%u; fid=%u)\n", dev_id, fid);
        return -EINVAL;
    }
    if (fid > BASE_VF_NUM) {
        vmng_err("Input parameter is error. (dev_id=%u; fid=%u)\n", dev_id, fid);
        return -EINVAL;
    }
    if (fid == 0) {
        vmng_err("Input parameter is error. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    return 0;
}

STATIC int vmngd_resource_alloc_vfid(u32 dev_id, u32 dtype, u32 *fid)
{
    if (*fid > BASE_VF_NUM) {
        vmng_err("Invalid fid. (fid=%u)\n", *fid);
        return VMNG_ERR;
    }

    if (*fid == 0) {
        if (vmngd_alloc_vfid_dynamic(dev_id, fid) != VMNG_OK) {
            vmng_err("Alloc_vfid_dynamic err. (dev_id=%u)\n", dev_id);
            return VMNG_ERR;
        }
    } else {
        if (vmngd_alloc_vfid_static(dev_id, *fid) != VMNG_OK) {
            vmng_err("Alloc_vfid_static err. (dev_id=%u)\n", dev_id);
            return VMNG_ERR;
        }
    }

    vmng_debug("dev_id=%u, *fid=%u, dtype=%u\n", dev_id, *fid, dtype);

    return VMNG_OK;
}

STATIC void vmngd_free_vdev_ctrl(u32 dev_id, u32 vfid)
{
    struct vmng_vdev_ctrl *ctrl = vmngd_get_ctrl(dev_id, vfid);
    struct mutex *ctrl_mutex = vmngd_get_ctrl_mutex(dev_id, vfid);

    mutex_lock(ctrl_mutex);
    ctrl->dev_id = 0;
    ctrl->vfid = 0;
    ctrl->dtype = 0;
    ctrl->core_num = 0;
    ctrl->total_core_num = 0;
    ctrl->status = VMNG_VDEV_STATUS_FREE;
    mutex_unlock(ctrl_mutex);
}

STATIC void vmngd_resource_free_vfid(u32 dev_id, u32 vfid)
{
    if (vmngd_dev_id_check(dev_id, vfid) != 0) {
        vmng_err("Parameter check failed. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return;
    }

    vmngd_free_vdev_ctrl(dev_id, vfid);
}

STATIC int vmngd_check_create_container_vdev_parameter(u32 dev_id, u32 dtype, const u32 *vfid,
    struct vmng_vf_res_info *vf_resource)
{
    u32 dtype_min = VMNG_DTYPE_RANGE_FROM;
    u32 dtype_max = VMNG_DTYPE_RANGE_TO;
    if ((vfid == NULL) || (vf_resource == NULL)) {
        vmng_err("Input parameter is error.\n");
        return VMNG_ERR;
    }

    if ((dev_id >= VMNGD_SURPORT_MAX_DEV) || (dtype < dtype_min) || (dtype > dtype_max)) {
        vmng_err("Input parameter is error. (dev_id=%u; dtype=%u)\n", dev_id, dtype);
        return VMNG_ERR;
    }

    if (vmngd_get_device_status(dev_id) != VMNG_VALID) {
        vmng_err("Device is not ready. (dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }
    return VMNG_OK;
}

int vmng_create_container_vdev(u32 dev_id, u32 dtype, u32 *vfid, struct vmng_vf_res_info *vf_resource)
{
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    struct vmng_ctrl_msg_info info;
    int ret;

    // add input param check
    if (vmngd_check_create_container_vdev_parameter(dev_id, dtype, vfid, vf_resource) != VMNG_OK) {
        return VMNG_ERR;
    }

    // alloc vfid (dynamic/static)
    ret = vmngd_resource_alloc_vfid(dev_id, dtype, vfid);
    if (ret != VMNG_OK) {
        vmng_err("Alloc vfid err. (dev_id=%d;dtype=%d;ret=%d)\n", dev_id, dtype, ret);
        return VMNG_ERR;
    }

    // get vf_resource data and write to struct vmng_ctrl_msg_info
    vf_resource->vfid = *vfid;
    if (memset_s(&info, sizeof(info), 0, sizeof(info)) != EOK) {
        vmng_err("Call memset_s failed.\n");
        return VMNG_ERR;
    }
    vmngd_set_alloc_vf_para(dev_id, dtype, *vfid, vf_resource, &info);

    // alloc vf (stars resource / vfg)
    ret = vmngd_execute_alloc_vf(dev_id, &info);
    if (ret != 0) {
        vmng_err("Execute alloc vf failed. (dev_id=%u; vfid=%u; ret=%d)\n", dev_id, *vfid, ret);
        goto FREE_VFID;
    }
    vf_resource->vfg.vfg_id = info.vf_cfg.vfg.vfg_id;

    // client online
    ret = vmngd_execute_client_online(dev_id, &info);
    if (ret != 0) {
        vmng_err("Execute client online failed. (dev_id=%u; vfid=%u; ret=%d)\n", dev_id, *vfid, ret);
        goto FREE_VF;
    }
    vmngd_init_split_mode(dev_id, VMNG_CONTAINER_SPLIT_MODE);

    vmng_info("vmng create container vdev sucess. (dev_id=%d, dtype=%d)\n", dev_id, dtype);
    return VMNG_OK;

FREE_VF:
    if (res_ops->free_vf != NULL) {
        (void)res_ops->free_vf(dev_id, *vfid);
    }
FREE_VFID:
    vmngd_resource_free_vfid(dev_id, *vfid);
    return VMNG_ERR;
}
EXPORT_SYMBOL(vmng_create_container_vdev);

STATIC int vmngd_check_destory_container_vdev_parameter(u32 dev_id, u32 vfid)
{
    if (dev_id >= VMNGD_SURPORT_MAX_DEV) {
        vmng_err("Input parameter is error. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    if ((vfid > BASE_VF_NUM) || (vfid == 0)) {
        vmng_err("Input parameter is error. (vfid %u)\n", vfid);
        return VMNG_ERR;
    }

    if (vmngd_get_device_status(dev_id) != VMNG_VALID) {
        vmng_err("Device is not ready. (dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }
    return VMNG_OK;
}

int vmng_destory_container_vdev(u32 dev_id, u32 vfid)
{
    struct vmng_ctrl_msg_info info;
    int ret;

    if (vmngd_check_destory_container_vdev_parameter(dev_id, vfid) != VMNG_OK) {
        return VMNG_ERR;
    }

    if (memset_s(&info, sizeof(struct vmng_ctrl_msg_info), 0, sizeof(struct vmng_ctrl_msg_info)) != EOK) {
        vmng_err("Call memset_s failed.\n");
        return VMNG_ERR;
    }

    info.dev_id = dev_id;
    info.vfid = vfid;

    ret = vmngd_execute_client_offline(dev_id, &info);
    if (ret != VMNG_OK) {
        vmng_err("Offline client failed. (dev_id=%u; vfid=%u; ret=%d)\n", dev_id, vfid, ret);
        return VMNG_ERR;
    }

    ret = vmngd_execute_free_vf(dev_id, &info);
    if (ret != VMNG_OK) {
        vmng_err("Free vf failed. (dev_id=%u; vfid=%u; ret=%d)\n", dev_id, vfid, ret);
        return VMNG_ERR;
    }
    vmngd_uninit_split_mode(dev_id);

    return VMNG_OK;
}
EXPORT_SYMBOL(vmng_destory_container_vdev);

STATIC int vmngd_res_init_instance(u32 dev_id)
{
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    int ret = 0;

    if (res_ops->res_init != NULL) {
        ret = res_ops->res_init(dev_id);
    }
    if (ret != 0) {
        vmng_err("Res init error. (dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }

    vmngd_set_device_status(dev_id, VMNG_VALID);

    return VMNG_OK;
}

STATIC void vmngd_res_uninit_instance(u32 dev_id)
{
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    if (res_ops->is_vf != NULL) {
        if (res_ops->is_vf(dev_id) == true) {
            return;
        }
    }

    if (dev_id >= VMNGD_SURPORT_MAX_DEV) {
        vmng_err("Parameter dev_is is error. (dev_id=%u)\n", dev_id);
        return;
    }

    if (res_ops->res_uninit != NULL) {
        res_ops->res_uninit(dev_id);
    }

    vmngd_set_device_status(dev_id, VMNG_INVALID);
    vmngdev_ctrl.devices[dev_id].dev = NULL;

    vmng_info("Res uninit instance. (dev_id=%u)\n", dev_id);
}

#ifndef CFG_FEATURE_SOC_VIRTMNG
STATIC void vmngd_bw_ctrl_info_init(u32 dev_id)
{
    size_t size = 0;
    u64 addr = 0;
    int ret;

    ret = agentdrv_get_addr_info(dev_id, DEVDRV_ADDR_VF_BANDWIDTH_BASE, 0, &addr, &size);
    if ((ret != 0) || (addr == 0) || (size == 0)) {
        vmngdev_ctrl.devices[dev_id].bw_ctrl.io_base_bwctrl = NULL;
        vmng_warn("Not support bandwidth ctrl. (ret=%d; dev_id=%u; addr=%llu; size=%lu)\n", ret, dev_id, addr, size);
        return;
    }
    vmngdev_ctrl.devices[dev_id].bw_ctrl.io_base_bwctrl =
        (struct vf_bandwidth_ctrl_remote*)devm_ioremap(vmngdev_ctrl.devices[dev_id].dev, addr, size);
    if (vmngdev_ctrl.devices[dev_id].bw_ctrl.io_base_bwctrl == NULL) {
        vmng_err("Bandwidth ctrl ioremap failed. (dev_id=%u)\n", dev_id);
        return;
    }

    return;
}

STATIC void vmngd_bw_ctrl_info_uninit(u32 dev_id)
{
    if (vmngdev_ctrl.devices[dev_id].bw_ctrl.io_base_bwctrl != NULL) {
        devm_iounmap(vmngdev_ctrl.devices[dev_id].dev, vmngdev_ctrl.devices[dev_id].bw_ctrl.io_base_bwctrl);
        vmngdev_ctrl.devices[dev_id].bw_ctrl.io_base_bwctrl = NULL;
    }

    return;
}

STATIC int vmngd_pci_init_instance(u32 dev_id, struct device *dev)
{
    vmngdev_ctrl.devices[dev_id].dev = dev;

    if (vmngd_is_sriov_support(dev_id) == false) {
        vmngd_bw_ctrl_info_init(dev_id);
    }

    vmngd_set_device_status(dev_id, VMNG_VALID);
    vmng_info("Init instance. (dev_id=%u)\n", dev_id);

    return 0;
}

STATIC void vmngd_pci_uninit_instance(u32 dev_id)
{
    if (vmngd_is_sriov_support(dev_id) == false) {
        vmngd_bw_ctrl_info_uninit(dev_id);
    }

    vmngd_set_device_status(dev_id, VMNG_INVALID);

    vmngdev_ctrl.devices[dev_id].dev = NULL;

    vmng_info("Uninit instance. (dev_id=%u)\n", dev_id);
}

typedef int (*flr_uninstance)(u32 dev_id);
void agentdrv_dev_reg_flr_uninstance_func(flr_uninstance func);
int vmngd_pci_flr_uninstance(u32 vdev_id)
{
    struct vmngd_resource_ops *ops = &vmngdev_ctrl.res_ops;
    u32 pf_id = 0;
    u32 vf_id = 0;
    int ret;

    if (vmngd_get_pfvf_type_by_devid(vdev_id) != VMNGD_TYPE_VF) {
        vmng_err("Pf not support flr error. (dev_id=%u)\n", vdev_id);
        return VMNG_ERR;
    }

    ret = vmngd_get_pfvf_id_by_devid(vdev_id, &pf_id, &vf_id);
    if (ret != VMNG_OK) {
        vmng_err("Get_pfvf_id_by_devid error. (dev_id=%u)\n", vdev_id);
        return ret;
    }

    if (ops->reset_vf != NULL) {
        ret = ops->reset_vf(pf_id, vf_id);
        if (ret != VMNG_OK) {
            vmng_err("Reset vf error.(dev_id=%u;vfid=%u;ret=%d)\n", pf_id, vf_id, ret);
            return ret;
        }
    }

    vmng_info("Reset instance Done. (dev_id=%u)\n", vdev_id);

    return VMNG_OK;
}

#define MIA_MNG_NOTIFIER "mia_mng"
static int vmngd_notifier_func(u32 udevid, enum uda_notified_action action)
{
    struct device *dev = NULL;
    int ret = 0;

    if (udevid >= VMNGD_SURPORT_MAX_DEV) {
        vmng_err("Invalid para. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    dev = uda_get_agent_device(udevid);
    if (dev != NULL) { /* obp virtual dev is null, not need to init virtual dev */
        if (action == UDA_INIT) {
            ret = vmngd_pci_init_instance(udevid, dev);
        } else if (action == UDA_UNINIT) {
            vmngd_pci_uninit_instance(udevid);
        }
    }

    vmng_info("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);

    return ret;
}

#ifdef CFG_FEATURE_VIRTMNG_SUPPORT_UDA
#define MIA_MNG_VIRTUAL_NOTIFIER "mia_mng_virtual"
static int vmngd_notifier_virtual_func(u32 udevid, enum uda_notified_action action)
{
    u32 phyid = 0;
    u32 vfid = 0;
    int ret = 0;
    if (action == UDA_INIT) {
        if (uda_is_phy_dev(udevid)) {
            phyid = udevid;
            vfid = 0;
        } else {
            struct uda_mia_dev_para mia_para;
            ret = uda_udevid_to_mia_devid(udevid, &mia_para);
            if (ret != 0) {
                vmng_err("Query mia dev failed. (udevid=%u)\n",  udevid);
                return ret;
            }

            phyid = mia_para.phy_devid;
            vfid = mia_para.sub_devid + 1; /* vfid start from 1 */
        }

        ret = vmngd_config_soc_res(phyid, vfid);
    }
    vmng_info("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);
    return ret;
}
#endif

/* end ifndef CFG_FEATURE_SOC_VIRTMNG */
#endif
STATIC int vmngd_dev_init_instance(u32 dev_id)
{
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;
    int ret = 0;

    if (res_ops->res_init != NULL) {
        ret = res_ops->res_init(dev_id);
    }
    if (ret != 0) {
        vmng_err("Res init error. (dev_id=%u;ret=%d)\n", dev_id, ret);
        return VMNG_ERR;
    }

    vmng_info("Init dev instance. (dev_id=%u)\n", dev_id);
    return 0;
}

STATIC void vmngd_dev_uninit_instance(u32 dev_id)
{
    struct vmngd_resource_ops *res_ops = &vmngdev_ctrl.res_ops;

    if (res_ops->res_uninit != NULL) {
        res_ops->res_uninit(dev_id);
    }
    vmng_info("Uninit dev instance. (dev_id=%u)\n", dev_id);
}
#define MIA_MNG_LOCAL_NOTIFIER "mia_mng_local"
static int vmngd_local_notifier_func(u32 udevid, enum uda_notified_action action)
{
    int ret = 0;

    if (udevid >= VMNGD_SURPORT_MAX_DEV) {
        vmng_err("Invalid para. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    if (action == UDA_INIT) {
        ret = vmngd_dev_init_instance(udevid);
    } else if (action == UDA_UNINIT) {
        vmngd_dev_uninit_instance(udevid);
    }

    vmng_info("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);

    return ret;
}

int vmngd_init(void)
{
    u32 type;
    u32 dev_id;
    u32 vfid;

    if ((memset_s(g_vmngd_clients, sizeof(g_vmngd_clients), 0, sizeof(g_vmngd_clients)) != EOK) ||
        (memset_s(g_vdev_ctrl, sizeof(g_vdev_ctrl), 0, sizeof(g_vdev_ctrl)) != EOK) ||
        (memset_s(&g_clear_timer, sizeof(g_clear_timer), 0, sizeof(g_clear_timer)) != EOK) ||
        (memset_s(g_vmngd_clients_instance, sizeof(g_vmngd_clients_instance), 0, sizeof(g_vmngd_clients_instance)) !=
        EOK) ||
        (memset_s(&vmngdev_ctrl, sizeof(vmngdev_ctrl), 0, sizeof(vmngdev_ctrl)) != EOK)) {
        vmng_err("Call memset_s failed.\n");
        return -EINVAL;
    }

    for (dev_id = 0; dev_id < VMNGD_SURPORT_MAX_DEV; dev_id++) {
        for (vfid = 0; vfid < VMNG_VDEV_MAX_PER_PDEV; vfid++) {
            g_vdev_ctrl[dev_id][vfid].status = VMNGD_VDEV_INIT_STATUS;
            mutex_init(&g_vdev_ctrl_mutex[dev_id][vfid]);
        }
    }

    for (type = 0; type < (u32)VMNGD_CLIENT_TYPE_MAX; type++) {
        for (dev_id = 0; dev_id < VMNGD_SURPORT_MAX_DEV; dev_id++) {
            for (vfid = 0; vfid < VMNG_VDEV_MAX_PER_PDEV; vfid++) {
                g_vmngd_clients_instance[type][dev_id][vfid].type = type;
                g_vmngd_clients_instance[type][dev_id][vfid].flag = VMNG_INSTANCE_FLAG_UNINIT;
                mutex_init(&g_vmngd_clients_instance[type][dev_id][vfid].mutex);
            }
        }
    }

    sema_init(&vmngd_sema, 0);
    return 0;
}

STATIC int __init vmngd_init_module(void)
{
    struct uda_dev_type type;
    int ret;
    u32 mode;

    vmng_info("vmngd init module start.\n");
    ret = vmngd_init();
    if (ret != 0) {
        vmng_err("Call vmngd_init failed. (ret=%d)\n", ret);
        return ret;
    }

    if (vmngd_res_ops_init(&vmngdev_ctrl.res_ops) != VMNG_OK) {
        vmng_err("Res_ops_init failed.\n");
        return -EINVAL;
    }

    mode = dbl_get_rc_ep_mode();
    if (mode == DBL_RC_MODE) {
        ret = vmngd_res_init_instance(0); // mini v3 dev_id = 0;
        if ((ret == 0) && (vmngd_proc_fs_init() == 0)) {
            vmngdev_ctrl.procfs_valid = VMNG_PROCFS_VALID;
            vmng_info("Proc fs init finish.\n");
        }
        return ret;
    }

    uda_davinci_local_real_entity_type_pack(&type);
    ret = uda_notifier_register(MIA_MNG_LOCAL_NOTIFIER, &type, UDA_PRI3, vmngd_local_notifier_func);
    if (ret != 0) {
        vmng_err("Register local notifier failed. (ret=%d)\n", ret);
        return ret;
    }

#ifndef CFG_FEATURE_SOC_VIRTMNG
    uda_davinci_local_real_agent_type_pack(&type);
    ret = uda_notifier_register(MIA_MNG_NOTIFIER, &type, UDA_PRI3, vmngd_notifier_func);
    if (ret != 0) {
        vmng_err("Register pcie ko client failed. (ret=%d)\n", ret);
        uda_davinci_local_real_entity_type_pack(&type);
        (void)uda_notifier_unregister(MIA_MNG_LOCAL_NOTIFIER, &type);
        return ret;
    }
#ifdef CFG_FEATURE_VIRTMNG_SUPPORT_UDA
    uda_davinci_local_virtual_entity_type_pack(&type);
    ret = uda_notifier_register(MIA_MNG_VIRTUAL_NOTIFIER, &type, UDA_PRI1, vmngd_notifier_virtual_func);
    if (ret != 0) {
        vmng_err("Register pcie ko client failed. (ret=%d)\n", ret);
        uda_davinci_local_real_agent_type_pack(&type);
        (void)uda_notifier_unregister(MIA_MNG_NOTIFIER, &type);
        uda_davinci_local_real_entity_type_pack(&type);
        (void)uda_notifier_unregister(MIA_MNG_LOCAL_NOTIFIER, &type);
        return ret;
    }
#endif
    ret = agentdrv_register_common_msg_client(&vmngd_device_comm_msg_client);
    if (ret != 0) {
        uda_davinci_local_real_agent_type_pack(&type);
        (void)uda_notifier_unregister(MIA_MNG_NOTIFIER, &type);
        uda_davinci_local_real_entity_type_pack(&type);
        (void)uda_notifier_unregister(MIA_MNG_LOCAL_NOTIFIER, &type);
#ifdef CFG_FEATURE_VIRTMNG_SUPPORT_UDA
        uda_davinci_local_real_agent_type_pack(&type);
        (void)uda_notifier_unregister(MIA_MNG_VIRTUAL_NOTIFIER, &type);
#endif
        vmng_err("Call devdrv_register_common_msg_client failed.\n");
        return ret;
    }

    agentdrv_dev_reg_flr_uninstance_func(vmngd_pci_flr_uninstance);
#endif

    ret = vmngd_proc_fs_init();
    if (ret == 0) {
        vmngdev_ctrl.procfs_valid = VMNG_PROCFS_VALID;
        vmng_info("Proc fs init finish.\n");
    }
    vmng_info("Call vmngd_init_module finish.\n");
    return 0;
}
module_init(vmngd_init_module);

STATIC void __exit vmngd_exit_module(void)
{
    struct uda_dev_type type;
    u32 mode;
    if (vmngdev_ctrl.procfs_valid == VMNG_PROCFS_VALID) {
        vmngd_proc_fs_uninit();
        vmngdev_ctrl.procfs_valid = VMNG_PROCFS_INVALID;
        vmng_info("Proc fs uninit finish.\n");
    }

    mode = dbl_get_rc_ep_mode();
    if (mode == DBL_RC_MODE) {
        vmngd_res_uninit_instance(0); // mini v3 dev_id = 0;
        (void)vmngd_res_ops_uninit(&vmngdev_ctrl.res_ops);
        return;
    }

#ifndef CFG_FEATURE_SOC_VIRTMNG
    (void)agentdrv_unregister_common_msg_client(&vmngd_device_comm_msg_client);
    uda_davinci_local_real_agent_type_pack(&type);
    (void)uda_notifier_unregister(MIA_MNG_NOTIFIER, &type);
#endif
    uda_davinci_local_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(MIA_MNG_LOCAL_NOTIFIER, &type);

    if (vmngd_res_ops_uninit(&vmngdev_ctrl.res_ops) != VMNG_OK) {
        vmng_err("Res_ops_uninit failed.\n");
    }

    vmng_info("Call vmngd_exit_module finish.\n");
}
module_exit(vmngd_exit_module);

MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("virt mng dev driver");
MODULE_LICENSE("GPL");
