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
* Create: 2023-8-28
*/

#include <linux/device.h>
#include <linux/iommu.h>
#include <linux/mutex.h>
#include <linux/vmalloc.h>
#include <linux/kref.h>

#include "uda.h"
#include "trs_core_smmu.h"

struct trs_ssid_node {
    pid_t pid;
    int ssid;
    struct iommu_sva *handle;
    struct list_head node;
};

struct trs_smmu_inst {
    struct trs_id_inst inst;
    struct list_head ssid_list_head;
    struct mutex mutex;
    struct kref ref;
    struct device *dev;
};

static DEFINE_MUTEX(trs_smmu_mutex);

static struct trs_smmu_inst *g_smmu_inst[TRS_DEV_MAX_NUM];

static int trs_smmu_inst_create(struct trs_id_inst *inst)
{
    struct trs_smmu_inst *smmu_inst = NULL;
    struct device *dev = NULL;
    u32 ts_inst = trs_id_inst_to_ts_inst(inst);
    int ret;

    dev = uda_get_device(inst->devid);
    if (dev == NULL) {
        trs_err("Dev is NULL. (devid=%u)\n", inst->devid);
        return -ENODEV;
    }

    smmu_inst = (struct trs_smmu_inst*)vzalloc(sizeof(struct trs_smmu_inst));
    if (smmu_inst == NULL) {
        trs_err("Vzalloc smmu inst failed. (devid=%u)\n", inst->devid);
        return -ENOMEM;
    }

    ret = iommu_dev_enable_feature(dev, IOMMU_DEV_FEAT_IOPF);
    if (ret != 0) {
        vfree(smmu_inst);
        trs_err("Enable IOPF feature failed. (devid=%u; ret=%d)\n", inst->devid, ret);
        return ret;
    }

    ret = iommu_dev_enable_feature(dev, IOMMU_DEV_FEAT_SVA);
    if (ret != 0) {
        (void)iommu_dev_disable_feature(dev, IOMMU_DEV_FEAT_IOPF);
        vfree(smmu_inst);
        trs_err("Enable SVA feature failed. (devid=%u; ret=%d)\n", inst->devid, ret);
        return ret;
    }

    smmu_inst->dev = dev;
    smmu_inst->inst = *inst;
    INIT_LIST_HEAD(&smmu_inst->ssid_list_head);
    mutex_init(&smmu_inst->mutex);
    kref_init(&smmu_inst->ref);

    mutex_lock(&trs_smmu_mutex);
    if (g_smmu_inst[ts_inst] != NULL) {
        (void)iommu_dev_disable_feature(dev, IOMMU_DEV_FEAT_SVA);
        (void)iommu_dev_disable_feature(dev, IOMMU_DEV_FEAT_IOPF);
        vfree(smmu_inst);
        mutex_unlock(&trs_smmu_mutex);
        return -ENODEV;
    }
    g_smmu_inst[ts_inst] = smmu_inst;
    mutex_unlock(&trs_smmu_mutex);

    return 0;
}

static void trs_smmu_inst_release(struct kref *kref)
{
    struct trs_smmu_inst *smmu_inst = container_of(kref, struct trs_smmu_inst, ref);

    vfree(smmu_inst);
}

static void trs_smmu_inst_destroy(struct trs_id_inst *inst)
{
    struct trs_smmu_inst *smmu_inst = NULL;
    u32 ts_inst = trs_id_inst_to_ts_inst(inst);

    mutex_lock(&trs_smmu_mutex);
    smmu_inst = g_smmu_inst[ts_inst];
    g_smmu_inst[ts_inst] = NULL;
    mutex_unlock(&trs_smmu_mutex);

    if (smmu_inst != NULL) {
        (void)iommu_dev_disable_feature(smmu_inst->dev, IOMMU_DEV_FEAT_SVA);
        (void)iommu_dev_disable_feature(smmu_inst->dev, IOMMU_DEV_FEAT_IOPF);
        kref_put(&smmu_inst->ref, trs_smmu_inst_release);
    }
}

static struct trs_smmu_inst *trs_smmu_inst_get(struct trs_id_inst *inst)
{
    u32 ts_inst = trs_id_inst_to_ts_inst(inst);
    struct trs_smmu_inst *smmu_inst = NULL;

    mutex_lock(&trs_smmu_mutex);
    smmu_inst = g_smmu_inst[ts_inst];
    if (smmu_inst != NULL) {
        kref_get(&smmu_inst->ref);
    }
    mutex_unlock(&trs_smmu_mutex);

    return smmu_inst;
}

static void trs_smmu_inst_put(struct trs_smmu_inst *smmu_inst)
{
    kref_put(&smmu_inst->ref, trs_smmu_inst_release);
}

static struct trs_ssid_node *trs_smmu_find_ssid_node(struct trs_smmu_inst *smmu_inst, pid_t pid)
{
    struct trs_ssid_node *ssid_node = NULL;
    struct trs_ssid_node *tmp = NULL;

    mutex_lock(&smmu_inst->mutex);
    list_for_each_entry_safe(ssid_node, tmp, &smmu_inst->ssid_list_head, node) {
        if (ssid_node->pid == pid) {
            mutex_unlock(&smmu_inst->mutex);
            return ssid_node;
        }
    }
    mutex_unlock(&smmu_inst->mutex);

    return NULL;
}

static int trs_smmu_proc_bind(struct trs_smmu_inst *smmu_inst)
{
    struct trs_ssid_node *ssid_node = NULL;
    struct iommu_sva *handle = NULL;
    int ssid;

    ssid_node = trs_smmu_find_ssid_node(smmu_inst, current->tgid);
    if (ssid_node != NULL) {
        trs_warn("Proc repeat bind smmu.\n");
        return 0;
    }

    ssid_node = (struct trs_ssid_node *)vzalloc(sizeof(struct trs_ssid_node));
    if (ssid_node == NULL) {
        trs_err("Vzalloc failed. (devid=%u)\n", smmu_inst->inst.devid);
        return -ENOMEM;
    }

    handle = iommu_sva_bind_device(smmu_inst->dev, current->mm, NULL);
    if (handle == NULL) {
        trs_err("Proc bind smmu failed. (devid=%u)\n", smmu_inst->inst.devid);
        vfree(ssid_node);
        return -ENODEV;
    }

    ssid = iommu_sva_get_pasid(handle);
    if (ssid == IOMMU_PASID_INVALID) {
        trs_err("Get ssid failed. (devid=%u)\n", smmu_inst->inst.devid);
        iommu_sva_unbind_device(handle);
        vfree(ssid_node);
        return -EINVAL;
    }

    ssid_node->pid = current->tgid;
    ssid_node->ssid = ssid;
    ssid_node->handle = handle;

    mutex_lock(&smmu_inst->mutex);
    list_add_tail(&ssid_node->node, &smmu_inst->ssid_list_head);
    mutex_unlock(&smmu_inst->mutex);

    return 0;
}

static void trs_smmu_proc_unbind(struct trs_smmu_inst *smmu_inst)
{
    struct trs_ssid_node *ssid_node = NULL;

    ssid_node = trs_smmu_find_ssid_node(smmu_inst, current->tgid);
    if (ssid_node == NULL) {
        trs_err("Proc unbind smmu faild.\n");
        return;
    }

    iommu_sva_unbind_device(ssid_node->handle);

    mutex_lock(&smmu_inst->mutex);
    list_del(&ssid_node->node);
    mutex_unlock(&smmu_inst->mutex);

    vfree(ssid_node);
}

static int trs_smmu_query_ssid(struct trs_smmu_inst *smmu_inst, int *ssid)
{
    struct trs_ssid_node *ssid_node = NULL;

    ssid_node = trs_smmu_find_ssid_node(smmu_inst, current->tgid);
    if (ssid_node == NULL) {
        trs_err("Query ssid failed. (devid=%u)\n", smmu_inst->inst.devid);
        return -EINVAL;
    }

    *ssid = ssid_node->ssid;
    return 0;
}

int trs_core_ops_proc_bind_smmu(struct trs_id_inst *inst)
{
    struct trs_smmu_inst *smmu_inst = NULL;
    int ret = 0;

    smmu_inst = trs_smmu_inst_get(inst);
    if (smmu_inst != NULL) {
        ret = trs_smmu_proc_bind(smmu_inst);
        trs_smmu_inst_put(smmu_inst);
    }

    return ret;
}

void trs_core_ops_proc_unbind_smmu(struct trs_id_inst *inst)
{
    struct trs_smmu_inst *smmu_inst = NULL;

    smmu_inst = trs_smmu_inst_get(inst);
    if (smmu_inst != NULL) {
        trs_smmu_proc_unbind(smmu_inst);
        trs_smmu_inst_put(smmu_inst);
    }
}

int trs_core_ops_get_ssid(struct trs_id_inst *inst, int *ssid)
{
    struct trs_smmu_inst *smmu_inst = NULL;
    int ret = -EINVAL;

    smmu_inst = trs_smmu_inst_get(inst);
    if (smmu_inst != NULL) {
        ret = trs_smmu_query_ssid(smmu_inst, ssid);
        trs_smmu_inst_put(smmu_inst);
    }

    return ret;
}

int trs_smmu_config(struct trs_id_inst *inst)
{
    return trs_smmu_inst_create(inst);
}

void trs_smmu_deconfig(struct trs_id_inst *inst)
{
    trs_smmu_inst_destroy(inst);
}
