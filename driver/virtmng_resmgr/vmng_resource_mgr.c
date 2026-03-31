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
 * Create: 2022-09-28
 */

#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/semaphore.h>
#include <linux/bitmap.h>
#include <linux/cpumask.h>
#include <linux/nodemask.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/securec.h>

#include "kernel_version_adapt.h"
#include "davinci_api.h"
#include "virtmng_interface.h"
#include "resource_mgr_cmd.h"
#include "vmng_resource_mgr.h"

#define VMNGD_DEV_CGROUP_LIMIT_PATH  "/sys/fs/cgroup/memory/usermemory/memory.limit_in_bytes"
#define VMNGD_DEVICE_AUTHORITY 0440
#define VMNGD_MEMNFO_BUFF_LEN 128

struct vmnghd_resource_ctrl_node {
    struct list_head node_list;
    struct vmng_resource_mgr_ctl resource_ctrl;
};

struct vmnghd_private_data {
    struct vmnghd_resource_ctrl_node *ctrl_node;
    struct mutex proc_lock;
};

struct vmnghd_resource_ctrl_list {
    struct mutex node_lock;
    struct list_head list;
};

struct vmnghd_resource_ctrl {
    struct semaphore sema;
    struct vmnghd_resource_ctrl_list instance_list;
    struct vmnghd_resource_ctrl_list processing_list;
};

struct vmnghd_resource_ctrl g_vmngd_resource_mgr_ctrl;

static void vmngd_resource_mgr_struct_init(void)
{
    sema_init(&g_vmngd_resource_mgr_ctrl.sema, 0);
    INIT_LIST_HEAD(&g_vmngd_resource_mgr_ctrl.instance_list.list);
    INIT_LIST_HEAD(&g_vmngd_resource_mgr_ctrl.processing_list.list);
    mutex_init(&g_vmngd_resource_mgr_ctrl.instance_list.node_lock);
    mutex_init(&g_vmngd_resource_mgr_ctrl.processing_list.node_lock);
}

static void vmngd_resource_mgr_add_node(
    struct vmnghd_resource_ctrl_list *list,
    struct vmnghd_resource_ctrl_node *ctrl_node)
{
    mutex_lock(&list->node_lock);
    list_add_tail(&ctrl_node->node_list, &list->list);
    mutex_unlock(&list->node_lock);
}

static void vmngd_resource_mgr_delete_node(
    struct vmnghd_resource_ctrl_list *list,
    struct vmnghd_resource_ctrl_node *ctrl_node)
{
    mutex_lock(&list->node_lock);
    list_del(&ctrl_node->node_list);
    mutex_unlock(&list->node_lock);
}

static void vmngd_resource_mgr_list_free(struct vmnghd_resource_ctrl_list *list)
{
    struct vmnghd_resource_ctrl_node *ctrl_node = NULL;
    struct vmnghd_resource_ctrl_node *n = NULL;

    mutex_lock(&list->node_lock);
    list_for_each_entry_safe(ctrl_node, n, &list->list, node_list) {
        list_del(&ctrl_node->node_list);
        kfree(ctrl_node);
        ctrl_node = NULL;
    }
    mutex_unlock(&list->node_lock);

    return;
}

static void vmngd_resource_mgr_ctrl_free(void)
{
    vmngd_resource_mgr_list_free(&g_vmngd_resource_mgr_ctrl.instance_list);

    vmngd_resource_mgr_list_free(&g_vmngd_resource_mgr_ctrl.processing_list);

    return;
}

#ifndef EMU_ST
static int vmngd_get_cgroup_meminfo(u64 *size)
{
    char file_string[VMNGD_MEMNFO_BUFF_LEN] = {0};
    struct file *fp = NULL;
    loff_t pos = 0;
    int ret;

    fp = filp_open(VMNGD_DEV_CGROUP_LIMIT_PATH, O_RDONLY, VMNGD_DEVICE_AUTHORITY);
    if (IS_ERR((void const *)fp)) {
        vmng_res_err("Open failed. (file=%s; errno=%ld)\n", VMNGD_DEV_CGROUP_LIMIT_PATH, PTR_ERR((void const *)fp));
        return -EINVAL;
    }
    if (kernel_read(fp, file_string, VMNGD_MEMNFO_BUFF_LEN - 1, &pos) < 0) {
        vmng_res_err("Filestring not right. (file=%s; pos=%lld)\n", VMNGD_DEV_CGROUP_LIMIT_PATH, pos);
        (void)filp_close(fp, NULL);
        return -EINVAL;
    }

    ret = sscanf_s(file_string, "%llu", size);
    if (ret != 1) {
        vmng_res_err("Sscanf not right. (file=%s; ret=%d)\n", VMNGD_DEV_CGROUP_LIMIT_PATH, ret);
        (void)filp_close(fp, NULL);
        return -EINVAL;
    }

    (void)filp_close(fp, NULL);

    return 0;
}
#endif

static bool vmngd_resource_mgr_check_processing(struct vmnghd_resource_ctrl_list *list,
    struct vmng_vdev_ctrl *vdev_ctrl)
{
    struct vmnghd_resource_ctrl_node *processing_ctrl_node = NULL;

    mutex_lock(&list->node_lock);
    /* check same vf processing completed or not */
    list_for_each_entry(processing_ctrl_node, &list->list, node_list) {
        if ((processing_ctrl_node->resource_ctrl.dev_id == vdev_ctrl->dev_id) &&
            (processing_ctrl_node->resource_ctrl.vfid == vdev_ctrl->vfid)) {
            mutex_unlock(&list->node_lock);
            return true;
        }
    }
    mutex_unlock(&list->node_lock);

    return false;
}

static bool vmngd_resource_mgr_check_instance(struct vmngd_client_instance *instance)
{
    return vmngd_resource_mgr_check_processing(&g_vmngd_resource_mgr_ctrl.instance_list, &instance->vdev_ctrl) ||
        vmngd_resource_mgr_check_processing(&g_vmngd_resource_mgr_ctrl.processing_list, &instance->vdev_ctrl);
}

static int vmngd_resource_mgr_init_instance(struct vmngd_client_instance *instance)
{
    struct vmnghd_resource_ctrl_node *ctrl_node = NULL;
    u64 mem_limit_size;
    int ret;

    if (vmngd_resource_mgr_check_instance(instance) == true) {
        vmng_res_err("The previous action is not complete, wait and try again.\n");
        return -EBUSY;
    }

    ret = vmngd_get_cgroup_meminfo(&mem_limit_size);
    if (ret != 0) {
        vmng_res_err("Get cgroup meminfo fail.\n");
        return ret;
    }

    ctrl_node = kzalloc(sizeof(struct vmnghd_resource_ctrl_node), GFP_KERNEL);
    if (ctrl_node == NULL) {
        vmng_res_err("Kzalloc failed.\n");
        return -ENOMEM;
    }

    ctrl_node->resource_ctrl.status = VMNG_RM_CLIENT_INIT;
    ctrl_node->resource_ctrl.dev_id = instance->vdev_ctrl.dev_id;
    ctrl_node->resource_ctrl.vfid = instance->vdev_ctrl.vfid;
    ctrl_node->resource_ctrl.aicpu_bitmap =
        instance->vdev_ctrl.vf_cfg.cpu.device_aicpu_bitmap;
    ctrl_node->resource_ctrl.mem_size = (mem_limit_size * (u64)instance->vdev_ctrl.core_num) /
        (u64)instance->vdev_ctrl.total_core_num;

    vmngd_resource_mgr_add_node(&g_vmngd_resource_mgr_ctrl.instance_list, ctrl_node);

    /* make sure vmngd_resource_mgr_get_vdev_event data is right */
    isb();

    up(&g_vmngd_resource_mgr_ctrl.sema);

    vmng_res_info("Instance info.(devid=%u; vfid=%u; mem_size=%llu; mem_limit_size=%llu; aicpu_bitmap=0x%llx;"
        " core_num=%u; total_core_num=%u)\n", instance->vdev_ctrl.dev_id, instance->vdev_ctrl.vfid,
        ctrl_node->resource_ctrl.mem_size, mem_limit_size, ctrl_node->resource_ctrl.aicpu_bitmap,
        instance->vdev_ctrl.core_num, instance->vdev_ctrl.total_core_num);

    return 0;
}

static int vmngd_resource_mgr_uninit_instance(struct vmngd_client_instance *instance)
{
    struct vmnghd_resource_ctrl_node *ctrl_node = NULL;

    if (vmngd_resource_mgr_check_instance(instance) == true) {
        vmng_res_err("The previous action is not complete, wait and try again.\n");
        return -EBUSY;
    }

    ctrl_node = kzalloc(sizeof(struct vmnghd_resource_ctrl_node), GFP_KERNEL);
    if (ctrl_node == NULL) {
        vmng_res_err("Kzalloc failed.\n");
        return -ENOMEM;
    }
    ctrl_node->resource_ctrl.status = VMNG_RM_CLIENT_UNINIT;
    ctrl_node->resource_ctrl.dev_id = instance->vdev_ctrl.dev_id;
    ctrl_node->resource_ctrl.vfid = instance->vdev_ctrl.vfid;

    vmngd_resource_mgr_add_node(&g_vmngd_resource_mgr_ctrl.instance_list, ctrl_node);

    /* make sure vmngd_resource_mgr_get_vdev_event data is right */
    isb();
    vmng_res_info("Uninstance info.(devid=%u; vfid=%u)\n", instance->vdev_ctrl.dev_id, instance->vdev_ctrl.vfid);
    up(&g_vmngd_resource_mgr_ctrl.sema);

    return 0;
}

#define VMNGD_CLOUD_V2_AICPU 0XFC
#define VMNGD_CLOUD_V2_DCPU 0X2
static int vmngd_resource_mgr_sriov_instance(struct vmng_sriov_info *sriov_info)
{
    struct vmnghd_resource_ctrl_node *ctrl_node = NULL;

    ctrl_node = kzalloc(sizeof(struct vmnghd_resource_ctrl_node), GFP_KERNEL | __GFP_ACCOUNT);
    if (ctrl_node == NULL) {
        vmng_res_err("Kzalloc failed.\n");
        return -ENOMEM;
    }
    if (sriov_info->sriov_status == VMNGH_PF_SRIOV_ENABLE) {
        ctrl_node->resource_ctrl.status = VMNG_RM_CLIENT_SRIOV_ENABLE;
        ctrl_node->resource_ctrl.aicpu_bitmap = VMNGD_CLOUD_V2_AICPU | VMNGD_CLOUD_V2_DCPU;
        ctrl_node->resource_ctrl.datacpu_bitmap = 0x0;
    } else if (sriov_info->sriov_status == VMNGH_PF_SRIOV_DISABLE) {
        ctrl_node->resource_ctrl.status = VMNG_RM_CLIENT_SRIOV_DISABLE;
        ctrl_node->resource_ctrl.aicpu_bitmap = VMNGD_CLOUD_V2_AICPU;
        ctrl_node->resource_ctrl.datacpu_bitmap = VMNGD_CLOUD_V2_DCPU;
    }
    ctrl_node->resource_ctrl.dev_id = sriov_info->dev_id;
    ctrl_node->resource_ctrl.vfid = 0;

    vmngd_resource_mgr_add_node(&g_vmngd_resource_mgr_ctrl.instance_list, ctrl_node);

    /* make sure vmngd_resource_mgr_get_vdev_event data is right */
    isb();
    vmng_res_info("sriov info.(devid=%u; status=%d)\n", sriov_info->dev_id, sriov_info->sriov_status);
    up(&g_vmngd_resource_mgr_ctrl.sema);

    // Wait for resource mgr set cgroup finish, or event_sched will get the incorrect cpu info
    msleep(500);     // 500ms

    return 0;
}

static struct vmngd_client vmngd_resource_mgr_client = {
    .type = VMNGD_CLIENT_TYPE_VRESOURCE_MGR,
    .init_instance = vmngd_resource_mgr_init_instance,
    .uninit_instance = vmngd_resource_mgr_uninit_instance,
    .sriov_instance = vmngd_resource_mgr_sriov_instance,
};

void vmngd_resource_mgr_client_init(void)
{
    int ret;

    ret = vmngd_register_client(&vmngd_resource_mgr_client);
    if (ret) {
        vmng_res_err("Vmngd_register_client failed. (ret=%d)\n", ret);
        return;
    }

    vmng_res_info("Register vmngd client success.\n");
}

void vmngd_resource_mgr_client_uninit(void)
{
    int ret;

    ret = vmngd_unregister_client(&vmngd_resource_mgr_client);
    if (ret) {
        vmng_res_err("Vmngd_unregister_client failed. (ret=%d)\n", ret);
        return;
    }

    vmng_res_info("Unregister vmngd client success.\n");
}

static int vmngd_resource_mgr_ensure_get_event_consecutive(
    struct vmnghd_private_data *private_data,
    struct vmnghd_resource_ctrl_node *ctrl_node)
{
    if (private_data == NULL) {
        vmng_res_err("private_data is NULL.\n");
        return -EINVAL;
    }
    mutex_lock(&private_data->proc_lock);
    if (private_data->ctrl_node != NULL) {
        mutex_unlock(&private_data->proc_lock);
        vmng_res_err("Just support consecutive processing.\n");
        return -EAGAIN;
    }
    private_data->ctrl_node = ctrl_node;
    mutex_unlock(&private_data->proc_lock);

    return 0;
}

static struct vmnghd_resource_ctrl_node *vmngd_resource_mgr_get_first_instance_node(void)
{
    struct vmnghd_resource_ctrl_node *ctrl_node = NULL;

    mutex_lock(&g_vmngd_resource_mgr_ctrl.instance_list.node_lock);
    if (list_empty(&g_vmngd_resource_mgr_ctrl.instance_list.list) == true) {
        mutex_unlock(&g_vmngd_resource_mgr_ctrl.instance_list.node_lock);
        return NULL;
    }
    ctrl_node = list_first_entry(&g_vmngd_resource_mgr_ctrl.instance_list.list,
        struct vmnghd_resource_ctrl_node, node_list);
    list_del(&ctrl_node->node_list);
    mutex_unlock(&g_vmngd_resource_mgr_ctrl.instance_list.node_lock);

    return ctrl_node;
}

static int vmngd_resource_mgr_get_vdev_event(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct vmnghd_private_data *private_data = (struct vmnghd_private_data *)filep->private_data;
    struct vmnghd_resource_ctrl_node *ctrl_node = NULL;
    int ret;

    ret = down_killable(&g_vmngd_resource_mgr_ctrl.sema);
    if (ret != 0) {
        return ret;
    }

    ctrl_node = vmngd_resource_mgr_get_first_instance_node();
    if (ctrl_node == NULL) {
        return 0;
    }

    ret = vmngd_resource_mgr_ensure_get_event_consecutive(private_data, ctrl_node);
    if (ret != 0) {
        /* add to list to other thread process */
        vmngd_resource_mgr_add_node(&g_vmngd_resource_mgr_ctrl.instance_list, ctrl_node);
        up(&g_vmngd_resource_mgr_ctrl.sema); /* to up sema again to other thread process */

        return -EAGAIN;
    }

    ret = copy_to_user((void __user *)(uintptr_t)arg,
        &ctrl_node->resource_ctrl, sizeof(struct vmng_resource_mgr_ctl));
    if (ret != 0) {
        vmng_res_err("Copy_to_user failed. (ret=%d).\n", ret);
        private_data->ctrl_node = NULL;
        /* add to list to other thread process */
        vmngd_resource_mgr_add_node(&g_vmngd_resource_mgr_ctrl.instance_list, ctrl_node);
        up(&g_vmngd_resource_mgr_ctrl.sema); /* to up sema again to other thread process */

        return ret;
    }

    vmngd_resource_mgr_add_node(&g_vmngd_resource_mgr_ctrl.processing_list, ctrl_node);

    vmng_res_info("Get vdev event info.(devid=%u; vfid=%u; state=%u)\n",
        ctrl_node->resource_ctrl.dev_id, ctrl_node->resource_ctrl.vfid, ctrl_node->resource_ctrl.status);

    return 0;
}

static struct vmnghd_resource_ctrl_node* vmngd_resource_mgr_ensure_set_event_consecutive(
    struct vmnghd_private_data *private_data)
{
    struct vmnghd_resource_ctrl_node *ctrl_node = NULL;

    mutex_lock(&private_data->proc_lock);
    if (private_data->ctrl_node == NULL) {
        mutex_unlock(&private_data->proc_lock);
        return NULL;
    }
    ctrl_node = private_data->ctrl_node;
    private_data->ctrl_node = NULL;
    mutex_unlock(&private_data->proc_lock);

    return ctrl_node;
}

static int vmngd_resource_mgr_set_vdev_event_done(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct vmnghd_private_data *private_data = (struct vmnghd_private_data *)filep->private_data;
    struct vmnghd_resource_ctrl_node *ctrl_node = NULL;

    ctrl_node = vmngd_resource_mgr_ensure_set_event_consecutive(private_data);
    if (ctrl_node == NULL) {
        return 0;
    }

    vmngd_resource_mgr_delete_node(&g_vmngd_resource_mgr_ctrl.processing_list, ctrl_node);

    vmng_res_info("Set vdev event done info.(devid=%u; vfid=%u; state=%u)\n",
        ctrl_node->resource_ctrl.dev_id, ctrl_node->resource_ctrl.vfid, ctrl_node->resource_ctrl.status);

    kfree(ctrl_node);
    ctrl_node = NULL;

    return 0;
}

STATIC int vmngd_resource_mgr_open(struct inode *inode, struct file *filep)
{
    struct vmnghd_private_data *private_data;
    if (filep->private_data != NULL) {
        vmng_res_err("Kalloc failed.\n");
        return -EINVAL;
    }
    private_data = ka_vmalloc(sizeof(struct vmnghd_private_data), GFP_KERNEL | __GFP_ACCOUNT, PAGE_KERNEL);
    if (private_data == NULL) {
        vmng_res_err("Kalloc failed.\n");
        return -ENOMEM;
    }
    mutex_init(&private_data->proc_lock);
    private_data->ctrl_node = NULL;
    filep->private_data = (void *)private_data;

    return 0;
}

STATIC int vmngd_resource_mgr_release(struct inode *inode, struct file *filep)
{
    struct vmnghd_resource_ctrl_node *ctrl_node = NULL;
    struct vmnghd_private_data *private_data = NULL;
    if (filep->private_data == NULL) {
        return 0;
    }

    /* The OS ensures that release does not concur with the ioctl */
    private_data = (struct vmnghd_private_data *)filep->private_data;
    filep->private_data = NULL;

    ctrl_node = private_data->ctrl_node;
    vfree(private_data);

    if (ctrl_node != NULL) {
        vmngd_resource_mgr_delete_node(&g_vmngd_resource_mgr_ctrl.processing_list, ctrl_node);

        kfree(ctrl_node);
        ctrl_node = NULL;
    }

    return 0;
}

static int (*const vmngd_ioctl_handlers[VMNG_CMD_MAX_NR])(struct file *filep, u32 cmd, unsigned long arg) = {
    [_IOC_NR(VMNG_GET_VF_EVENT_BLOCK)] = vmngd_resource_mgr_get_vdev_event,
    [_IOC_NR(VMNG_SET_VF_EVENT_BLOCK_DONE)] = vmngd_resource_mgr_set_vdev_event_done,
};

STATIC long vmngd_resource_mgr_ioctl(struct file *filep, u32 cmd, unsigned long arg)
{
    if (arg == 0) {
        vmng_res_err("Invalid parameter, arg is null.\n");
        return -EINVAL;
    }

    if ((_IOC_TYPE(cmd) != VMNG_MAGIC) || (_IOC_NR(cmd) >= VMNG_CMD_MAX_NR)) {
        vmng_res_err("Cmd out of range. (cmd=%u)\n", _IOC_NR(cmd));
        return -EINVAL;
    }

    if (vmngd_ioctl_handlers[_IOC_NR(cmd)] == NULL) {
        vmng_res_err("Not support cmd. (cmd=%u)\n", _IOC_NR(cmd));
        return -EINVAL;
    }

    return vmngd_ioctl_handlers[_IOC_NR(cmd)](filep, cmd, arg);
}

const struct file_operations vmngd_file_operations = {
    .owner = THIS_MODULE,
    .open = vmngd_resource_mgr_open,
    .release = vmngd_resource_mgr_release,
    .unlocked_ioctl = vmngd_resource_mgr_ioctl,
};

STATIC int __init vmngd_resource_mgr_init(void)
{
    int ret;

    vmng_res_info("Init module.\n");
    vmngd_resource_mgr_struct_init();
    ret = drv_davinci_register_sub_module(DAVINCI_INTF_MODULE_VRESOURCE_MGR, &vmngd_file_operations);
    if (ret) {
        vmng_res_err("Drv_davinci_register_sub_module failed! (ret=%d)\n", ret);
        return ret;
    }
    vmngd_resource_mgr_client_init();

    return 0;
}
module_init(vmngd_resource_mgr_init);

STATIC void __exit vmngd_resource_mgr_uninit(void)
{
    vmngd_resource_mgr_client_uninit();
    vmngd_resource_mgr_ctrl_free();
    (void)drv_ascend_unregister_sub_module(DAVINCI_INTF_MODULE_VRESOURCE_MGR);
    vmng_res_info("Exit module finish.\n");

    return;
}
module_exit(vmngd_resource_mgr_uninit);

MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("virt resource mgr driver");
MODULE_LICENSE("GPL");
