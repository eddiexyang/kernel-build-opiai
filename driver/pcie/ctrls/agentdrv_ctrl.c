/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
#include <linux/securec.h>
#include <linux/delay.h>

#include "agentdrv_ctrl.h"
#include "agentdrv_multi_chip.h"
#include "agentdrv_unit.h"
#include "devdrv_util.h"
#include "devdrv_interface.h"
#include "agentdrv_platform.h"
#include "apb_comm_drv.h"
#include "resource_drv.h"
#include "kernel_version_adapt.h"

typedef int (*flr_uninstance)(u32 dev_id);
STATIC flr_uninstance flr_uninstance_func = NULL;

struct agentdrv_dev g_agent_devs[MAX_AGENTDEV_CNT];
struct agentdrv_client *g_agent_client[AGENTDRV_CLIENT_TYPE_MAX] = {
    NULL,
};
struct agentdrv_client_instance g_agent_instance_list[AGENTDRV_CLIENT_TYPE_MAX][MAX_AGENTDEV_CNT];
int g_agent_pci_manage_device_num = 0;
int g_agent_pci_manage_slot_num = 0;

int agentdrv_ctrl_init(void)
{
    int ret;

    ret = memset_s((void *)g_agent_devs, sizeof(g_agent_devs), 0, sizeof(g_agent_devs));
    if (ret) {
        devdrv_err("agentdrv memset_s failed. (ret=%d)\n", ret);
        return ret;
    }
    return 0;
}

void devdrv_init_dev_num(const struct pci_device_id agentdrv_tbl[], int id_num)
{
    struct pci_dev *pdev = NULL;
    int dev_num = 0;
    int i;

    for (i = 0; i < id_num; i++) {
        pdev = NULL;
        dev_num = -1;

        do {
            pdev = pci_get_device(agentdrv_tbl[i].vendor, agentdrv_tbl[i].device, pdev);
            dev_num++;
        } while (pdev != NULL);

        devdrv_info("PCIe device findout. (vendor=%x; device=%x; dev_num=%d)\n",
                    agentdrv_tbl[i].vendor, agentdrv_tbl[i].device, dev_num);

        if (dev_num > 0) {
            break;
        }
    }

    devdrv_info("Findout total PCIe devices end. (dev_num=%d)\n", dev_num);

    g_agent_pci_manage_device_num = dev_num;
}

bool devdrv_is_sriov_support(u32 dev_id)
{
    (void)dev_id;
    if (agentdrv_get_sriov_capability() == true) {
        return true;
    }
    return false;
}
EXPORT_SYMBOL(devdrv_is_sriov_support);

int devdrv_get_connect_protocol(u32 dev_id)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;

    if (dev_id >= MAX_AGENTDEV_CNT) {
        devdrv_err("Dev_id is invalid. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev((int)chip_id);
    if (agent_dev == NULL || agent_dev->shr_para[func_id] == NULL) {
        devdrv_err("agent_dev or shr_para is invalid. (dev_id=%u)\n", dev_id);
        return CONNECT_PROTOCOL_UNKNOWN;
    }

    return agent_dev->shr_para[func_id]->connect_protocol;
}
EXPORT_SYMBOL(devdrv_get_connect_protocol);

int devdrv_get_devid_by_pfvf_id(u32 pf_id, u32 vf_id, u32 *dev_id)
{
    if ((pf_id >= MAX_AGENTCHIP_CNT) || (vf_id >= MAX_AGENTFUNC_CNT)) {
        devdrv_err("Params is null. (pf_id=%u, vf_id=%u)\n", pf_id, vf_id);
        return -EINVAL;
    }

#ifdef CFG_FEATURE_SRIOV
    if (vf_id > 0) {
        *dev_id = (pf_id * (MAX_AGENTFUNC_CNT - 1)) + (vf_id - 1) + AGENTDRV_SRIOV_VF_DEVID_START;
    } else {
        *dev_id = pf_id;
    }
#else
    *dev_id = pf_id;
#endif
    return 0;
}
EXPORT_SYMBOL(devdrv_get_devid_by_pfvf_id);

int devdrv_get_pfvf_id_by_devid(u32 dev_id, u32 *pf_id, u32 *vf_id)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;

    if ((pf_id == NULL) || (vf_id == NULL)) {
        devdrv_err("Params is null. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    if (dev_id >= MAX_AGENTDEV_CNT) {
        devdrv_err("Dev_id is invalid. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agent_dev->shr_para[func_id] == NULL)) {
        devdrv_err("Agent_dev is NULL. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    *vf_id = agent_dev->shr_para[func_id]->vf_id;
    if (*vf_id > 0) {
        *pf_id = chip_id;
    } else {
        *pf_id = dev_id;
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_get_pfvf_id_by_devid);

/* only for symbol table, return false in device npu */
bool devdrv_is_mdev_vm_boot_mode(u32 dev_id)
{
    (void)dev_id;
    return false;
}
EXPORT_SYMBOL(devdrv_is_mdev_vm_boot_mode);

/* 0 is pf; 1 is vf */
int devdrv_get_pfvf_type_by_devid(u32 dev_id)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;
    u32 pfvf_type;

    if (dev_id >= MAX_AGENTDEV_CNT) {
        devdrv_err("Dev_id is invalid. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agent_dev->shr_para[func_id] == NULL)) {
        devdrv_err("Agent_dev is NULL. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    if (agent_dev->shr_para[func_id]->vf_id) {
        pfvf_type = DEVDRV_SRIOV_TYPE_VF;
    } else {
        pfvf_type = DEVDRV_SRIOV_TYPE_PF;
    }

    return pfvf_type;
}
EXPORT_SYMBOL(devdrv_get_pfvf_type_by_devid);

bool agentdrv_is_mdev_vm_full_spec(u32 dev_id)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;

    if (dev_id >= MAX_AGENTDEV_CNT) {
        devdrv_err("Dev_id is invalid. (dev_id=%u)\n", dev_id);
        return false;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if (agent_dev == NULL) {
        return false;
    }

    if (agent_dev->vm_full_spec_flag == 1) {
        return true;
    } else {
        return false;
    }
}
EXPORT_SYMBOL(agentdrv_is_mdev_vm_full_spec);

int devdrv_get_host_pfvf_id_by_devid(u32 dev_id, u32 *pf_id, u32 *vf_id)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    int host_dev_id;
    u32 chip_id;
    u32 func_id;

    if (dev_id >= MAX_AGENTDEV_CNT) {
        devdrv_err("Device ID is invalid.(dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev(chip_id);
    if ((agent_dev == NULL) || (agent_dev->shr_para[func_id] == NULL)) {
        devdrv_err("Get agent_dev failed. (dev_id=%u; chip_id=%u)\n", dev_id, chip_id);
        return -EINVAL;
    }

    if (agentdrv_is_dev_valid(agent_dev, func_id) == false) {
        devdrv_err("Get device failed, agent_dev is invalid. (dev_id=%u; chip_id=%u)\n", dev_id, chip_id);
        return -EINVAL;
    }

    host_dev_id = agent_dev->shr_para[func_id]->host_dev_id;

#ifdef CFG_FEATURE_SRIOV
    *vf_id = func_id;
    *pf_id = ((host_dev_id - DEVDRV_SRIOV_HOST_VF_DEVID_START) + 1 - *vf_id) / (MAX_AGENTFUNC_CNT - 1);
#else
    *vf_id = func_id;
    *pf_id = host_dev_id;
#endif
    return 0;
}
EXPORT_SYMBOL(devdrv_get_host_pfvf_id_by_devid);

void agentdrv_msg_release(struct agentdrv_devctrl *agent_dev, u32 func_id)
{
    struct agentdrv_msg_dev *msg_dev = NULL;
    struct devdrv_free_queue_cmd msg_cmd_data = {0};
    u32 i;

    msg_dev = agent_dev->p_agentdrv_msg_dev[func_id];
    if (msg_dev == NULL) {
        devdrv_err("msg_dev is NULL. (dev_id=%u; func_id=%u)\n", agent_dev->agent_id, func_id);
        return;
    }

    /* when flr reset or vf offline, need set vf's admin sq base 0 to reinit */
    msg_dev->io_chan[AGENTDRV_ADMIN_CHAN_ID].sq.base_h = 0;

    /* admin msg chan no need free */
    for (i = 1; i < msg_dev->io_chan_cnt; i++) {
        msg_cmd_data.queue_id = i;
        (void)agentdrv_msg_free_msg_queue(msg_dev, &msg_cmd_data);
    }
}

int agentdrv_sriov_init_instance(u32 dev_id, u32 vm_full_spec_enable, u32 computility, u32 total,
    unsigned long *dma_bitmap)
{
    struct agentdrv_cpu_data drv_cpu_info = {0};
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;
    int ret;

    if (devdrv_get_pfvf_type_by_devid(dev_id) != DEVDRV_SRIOV_TYPE_VF) {
        devdrv_err("Pf instance only by pcie. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev((int)chip_id);
    if (agent_dev == NULL) {
        devdrv_err("Agent_dev is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    agent_dev->vm_full_spec_flag = vm_full_spec_enable;

    ret = agentdrv_sriov_init_dma(dev_id, computility, total, dma_bitmap);
    if (ret != 0) {
        devdrv_err("Sriov init dma failed. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    agentdrv_get_cpu_data_info(agent_dev, func_id, &drv_cpu_info);
    agentdrv_dma_bind_irq(agent_dev, func_id, &drv_cpu_info);

    ret = agentdrv_dev_register(agent_dev, (int)func_id);
    if (ret != 0) {
        agentdrv_sriov_uninit_dma(dev_id);
        devdrv_err("Sriov agent driver register failed. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(agentdrv_sriov_init_instance);

int agentdrv_sriov_uninit_instance(u32 dev_id)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;

    if (devdrv_get_pfvf_type_by_devid(dev_id) != DEVDRV_SRIOV_TYPE_VF) {
        devdrv_err("Pf uninstance only by pcie. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev((int)chip_id);
    if (agent_dev == NULL) {
        devdrv_err("Agent_dev is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    agentdrv_dev_unregister(agent_dev, (int)func_id);
    agentdrv_dma_unbind_irq(agent_dev, func_id);
    agentdrv_sriov_uninit_dma(dev_id);

    agentdrv_msg_release(agent_dev, func_id);

    return 0;
}
EXPORT_SYMBOL(agentdrv_sriov_uninit_instance);

int devdrv_mdev_set_pm_iova_addr_range(int devid, dma_addr_t iova_base, u64 iova_size)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;

    devdrv_dev2chipfunc((u32)devid, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if (agent_dev == NULL) {
        devdrv_err("Agent_dev is NULL. (dev_id=%d)\n", devid);
        return -EINVAL;
    }

    agent_dev->iova_range[func_id].start_addr = iova_base;
    agent_dev->iova_range[func_id].end_addr = iova_base + iova_size;
    if ((iova_base == 0) && (iova_size == 0)) {
        agent_dev->iova_range[func_id].init_flag = DEVDRV_DMA_IOVA_RANGE_UNINIT;
        devdrv_info("Uninit iova addr range success. (dev_id=%d)\n", devid);
    } else {
        agent_dev->iova_range[func_id].init_flag = DEVDRV_DMA_IOVA_RANGE_INIT;
        devdrv_info("Init iova addr range success. (dev_id=%d)\n", devid);
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_mdev_set_pm_iova_addr_range);

int devdrv_get_davinci_dev_num(void)
{
    return g_agent_pci_manage_slot_num;
}
EXPORT_SYMBOL(devdrv_get_davinci_dev_num);

int devdrv_get_dev_num(void)
{
    return g_agent_pci_manage_device_num;
}
EXPORT_SYMBOL(devdrv_get_dev_num);

int devdrv_get_slot_num(void)
{
    return g_agent_pci_manage_slot_num;
}
EXPORT_SYMBOL(devdrv_get_slot_num);

void agentdrv_res_set_slot_num(void)
{
    int func_totl = (int)agentdrv_res_get_func_total();
    int dev_totl = devdrv_get_dev_num();

    g_agent_pci_manage_slot_num = func_totl * dev_totl;
    devdrv_info("Seting slot number finish. (dev_totl=%d; func_totl=%d; slot_num=%d)\n",
        dev_totl, func_totl, g_agent_pci_manage_slot_num);
}

int devdrv_get_irq_vector(u32 devid, u32 entry, unsigned int *irq)
{
    *irq = entry;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_irq_vector);

STATIC void agentdrv_dev_register_instance_proc(struct agentdrv_devctrl *agent_dev,
    const struct agentdrv_client *client, int chip_id)
{
    struct agentdrv_client_instance *instance = NULL;
    int func_index;
    int dev_id;
    u32 ret;

    for (func_index = 0; func_index < (int)agent_dev->func_totl_num; func_index++) {
        if ((agentdrv_is_dev_valid(agent_dev, (u32)func_index) == true) &&
            (agent_dev->shr_para[func_index]->host_dev_id >= 0)) {
            devdrv_chipfunc2dev(&dev_id, chip_id, func_index);
            mutex_lock(&agent_dev->mutex);
            g_agent_devs[dev_id].agent_id = dev_id;
            g_agent_devs[dev_id].pdev = agent_dev->pdev;
            instance = &g_agent_instance_list[client->type][dev_id];
            instance->adev = &g_agent_devs[dev_id];
            if (client->init_instance == NULL) {
                mutex_unlock(&agent_dev->mutex);
                continue;
            }
            devdrv_info("Agent driver register client, before init instance. (type=%u, func_idx=%d, host_dev_id=%d)\n",
                (u32)client->type, func_index, agent_dev->shr_para[func_index]->host_dev_id);
            ret = (u32)client->init_instance(instance);
            devdrv_info("Agent driver register client, after init instance. (type=%u, func_idx=%d, host_dev_id=%d)\n",
                (u32)client->type, func_index, agent_dev->shr_para[func_index]->host_dev_id);
            if (ret != 0) {
                devdrv_err("Agent driver register client, init instance error. (type=%u, func_idx=%d)\n",
                    (u32)client->type, func_index);
            }
            mutex_unlock(&agent_dev->mutex);
        }
    }
}

int agentdrv_register_client(struct agentdrv_client *client)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    int chip_index;

    if (client == NULL) {
        devdrv_err("Input parameter is error.\n");
        return -EINVAL;
    }

    if ((u32)client->type >= AGENTDRV_CLIENT_TYPE_MAX) {
        devdrv_err("Register client failed, client type error. (type=%d)\n", client->type);
        return -EINVAL;
    }

    for (chip_index = 0; chip_index < MAX_AGENTCHIP_CNT; chip_index++) {
        agent_dev = agentdrv_get_dev((int)chip_index);
        if (agent_dev != NULL) {
            agentdrv_dev_register_instance_proc(agent_dev, client, chip_index);
        }
    }

    g_agent_client[client->type] = client;

    return 0;
}
EXPORT_SYMBOL(agentdrv_register_client);

int devdrv_get_hccs_link_status_and_group_id(u32 devid, u32 *hccs_status, u32 hccs_group_id[], u32 group_id_num)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;
    u32 i = 0;

    if ((devid >= MAX_AGENTDEV_CNT) || (group_id_num > HCCS_GROUP_SUPPORT_MAX_CHIPNUM)) {
        devdrv_err("Invalid device id or group_id_num. (dev_id=%u)\n", devid);
        return -EINVAL;
    }

    if ((hccs_status == NULL) || (hccs_group_id == NULL)) {
        devdrv_err("Hccs_status or hccs_group_id is null. (dev_id=%u)\n", devid);
        return -EINVAL;
    }

    devdrv_dev2chipfunc(devid, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if (agent_dev == NULL) {
        devdrv_err("Got device failed, agent_dev is NULL.(dev_id=%u)\n", devid);
        return -EINVAL;
    }

    if (!agentdrv_is_dev_valid(agent_dev, func_id)) {
        devdrv_err("Get device failed, agent_dev is invalid. (dev_id=%u; chip_id=%u)\n", devid, chip_id);
        return -EINVAL;
    }

    *hccs_status = agent_dev->shr_para[func_id]->hccs_status;
    for (i = 0; i < group_id_num; i++) {
        hccs_group_id[i] = agent_dev->shr_para[func_id]->hccs_group_id[i];
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_get_hccs_link_status_and_group_id);

int agentdrv_set_heartbeat_count(u32 devid, u64 count)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;

    if (devid >= MAX_AGENTDEV_CNT) {
        devdrv_err("Invalid device ID. (dev_id=%u)\n", devid);
        return -EINVAL;
    }
    devdrv_dev2chipfunc(devid, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev((int)chip_id);
    if (agent_dev == NULL) {
        devdrv_err("Got device failed, agent_dev is NULL.\n");
        return -EINVAL;
    }

    if (!agentdrv_is_dev_valid(agent_dev, func_id)) {
        devdrv_err("Get device failed, agent_dev is invalid. (dev_id=%u; chip_id=%u)\n", devid, chip_id);
        return -EINVAL;
    }
    agent_dev->shr_para[func_id]->heartbeat_count = count;

    return 0;
}
EXPORT_SYMBOL(agentdrv_set_heartbeat_count); //lint !e508

void agentdrv_unregister_client(const struct agentdrv_client *client)
{
    struct agentdrv_client_instance *instance = NULL;
    int i;

    if (client == NULL) {
        devdrv_err("Input parameter is error.\n");
        return;
    }

    if ((u32)client->type >= AGENTDRV_CLIENT_TYPE_MAX) {
        devdrv_err("Unregister client failed, client type invaild. (type=%d)\n", client->type);
        return;
    }

    for (i = 0; i < MAX_AGENTDEV_CNT; i++) {
        instance = &g_agent_instance_list[client->type][i];
        if (instance->adev != NULL) {
            if (client->uninit_instance != NULL) {
                client->uninit_instance(instance);
            }
            instance->adev = NULL;
        }
    }

    g_agent_client[client->type] = NULL;
}
EXPORT_SYMBOL(agentdrv_unregister_client);

int agentdrv_dev_register(struct agentdrv_devctrl *agent_dev, int func_id)
{
    struct agentdrv_client_instance *instance = NULL;
    int ret;
    u32 i;
    int dev_id = 0;

    if (agent_dev->agent_id >= MAX_AGENTCHIP_CNT) {
        devdrv_err("Invalid agent ID. (agent_id=%u)!\n", agent_dev->agent_id);
        return -EINVAL;
    }

    devdrv_chipfunc2dev(&dev_id, (int)agent_dev->agent_id, func_id);
    g_agent_devs[dev_id].pdev = agent_dev->pdev;
    g_agent_devs[dev_id].agent_id = (u32)dev_id;

    mutex_lock(&agent_dev->mutex);
    for (i = 0; i < AGENTDRV_CLIENT_TYPE_MAX; i++) {
        if (g_agent_client[i] == NULL || agent_dev->func_rdy[func_id] == true) {
            continue;
        }
        instance = &g_agent_instance_list[g_agent_client[i]->type][dev_id];
        if (instance->adev != NULL) {
            continue;
        }

        instance->adev = &g_agent_devs[dev_id];
        if (g_agent_client[i]->init_instance == NULL) {
            continue;
        }
        devdrv_info("Agent driver device register, before init_instance. (dev_id=%d; type=%u)\n", dev_id, i);
        ret = g_agent_client[i]->init_instance(instance);
        devdrv_info("Agent driver device register, after init_instance. (dev_id=%d; type=%u)\n", dev_id, i);
        if (ret != 0) {
            devdrv_err("Driver device init failed. (dev_id=%d; type=%u)\n", dev_id, i);
        }
    }
    agent_dev->func_rdy[func_id] = true;
    mutex_unlock(&agent_dev->mutex);

    devdrv_debug("Agent driver register success. (agent_id=%d)\n", agent_dev->agent_id);
    return 0;
}

void agentdrv_dev_unregister(struct agentdrv_devctrl *agent_dev, int func_id)
{
    struct agentdrv_client_instance *instance = NULL;
    u32 i;
    int dev_id;

    mutex_lock(&agent_dev->mutex);
    devdrv_chipfunc2dev(&dev_id, (int)agent_dev->agent_id, func_id);
    for (i = 0; i < AGENTDRV_CLIENT_TYPE_MAX; i++) {
        devdrv_info("Show device information. (dev_id=%d; type=%u)\n", dev_id, i);
        if (g_agent_client[i] == NULL || agent_dev->func_rdy[func_id] == false) {
            continue;
        }
        instance = &g_agent_instance_list[g_agent_client[i]->type][dev_id];
        if (instance->adev == NULL) {
            continue;
        }

        if (g_agent_client[i]->uninit_instance == NULL) {
            continue;
        }
        devdrv_info("Device before uninit_instance. (dev_id=%d; type=%u)\n", dev_id, i);
        g_agent_client[i]->uninit_instance(instance);
        instance->adev = NULL;
    }
    agent_dev->func_rdy[func_id] = false;
    mutex_unlock(&agent_dev->mutex);
}

void agentdrv_dev_flr_uninstance(struct agentdrv_devctrl *agent_dev, int func_id)
{
    int dev_id;

    devdrv_chipfunc2dev(&dev_id, (int)agent_dev->agent_id, func_id);

    if (devdrv_get_pfvf_type_by_devid((u32)dev_id) == DEVDRV_SRIOV_TYPE_PF) {
        devdrv_info("Pf uninstance only by pcie. (dev_id=%d)\n", dev_id);
        return;
    }

    if (flr_uninstance_func == NULL) {
        flr_uninstance_func = (flr_uninstance)(uintptr_t)__kallsyms_lookup_name("vmngd_pci_flr_uninstance");
    }

    if (flr_uninstance_func != NULL) {
        (void)flr_uninstance_func((u32)dev_id);
    }
}

void agentdrv_dev_reg_flr_uninstance_func(flr_uninstance func)
{
    flr_uninstance_func = func;
}
EXPORT_SYMBOL(agentdrv_dev_reg_flr_uninstance_func);

int devdrv_get_atu_info(u32 dev_id, int atu_type, struct devdrv_iob_atu **atu, phys_addr_t *host_phy_base)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;

    if (dev_id >= MAX_AGENTDEV_CNT) {
        devdrv_err("Device ID is invalid. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agentdrv_is_dev_valid(agent_dev, func_id) == false)) {
        devdrv_err("agent_dev is invalid. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    switch (atu_type) {
        case ATU_TYPE_RX_MEM:
            *atu = agent_dev->mem_rx_atu;
            *host_phy_base = agent_dev->shr_para[func_id]->host_mem_bar_base;
            break;
        default:
            devdrv_err("Device type not surport. (dev_id=%u; atu_type=%d)\n", dev_id, atu_type);
            return -EINVAL;
    }
    return 0;
}

#ifdef CFG_FEATURE_P2P
void agentdrv_atu_init(struct agentdrv_devctrl *agent_dev)
{
    struct devdrv_shr_para __iomem *shr_para = agent_dev->shr_para[0];
    int i;
    u32 idx_func;
    u32 func_num = agentdrv_res_get_func_total();

    for (idx_func = 0; idx_func < func_num; idx_func++) {
        shr_para = agent_dev->shr_para[idx_func];
        shr_para->tx_atu_base_size1 = 0;
        shr_para->tx_atu_base_size2 = 0;

        (void)agentdrv_get_tx_atu_addr(agent_dev->apb_pdev, idx_func, AGENTDRV_ATU_REGION1_BASE_INDEX,
            &shr_para->tx_atu_base_addr1, &shr_para->tx_atu_base_size1);

        (void)agentdrv_get_tx_atu_addr(agent_dev->apb_pdev, idx_func, AGENTDRV_ATU_REGION2_BASE_INDEX,
            &shr_para->tx_atu_base_addr2, &shr_para->tx_atu_base_size2);

        for (i = 0; i < DEVDRV_P2P_SUPPORT_MAX_DEVICE; i++) {
            shr_para->p2p_msg_base_addr[i] = 0;
            shr_para->p2p_db_base_addr[i] = 0;
        }
    }
}
#endif

void agentdrv_atu_rx_init(struct agentdrv_devctrl *agent_dev)
{
    (void)devdrv_mem_rx_atu_init(agent_dev->agent_id, agent_dev->apb_base,
        agent_dev->mem_rx_atu, DEVDRV_MAX_RX_ATU_NUM);
    devdrv_io_rx_atu_show(agent_dev->agent_id, agent_dev->apb_base);
    devdrv_rsv_mem_rx_atu_show(agent_dev->agent_id, agent_dev->apb_base);
}

void agentdrv_atu_proc(struct agentdrv_devctrl *agent_dev)
{
#ifdef CFG_FEATURE_P2P
    /* tx_atu config */
    agentdrv_atu_init(agent_dev);
#endif

    /* rx atu init */
    agentdrv_atu_rx_init(agent_dev);
}

int agentdrv_get_rx_atu(struct agentdrv_msg_dev *msg_dev, void *data)
{
    struct devdrv_admin_msg_reply *reply = (struct devdrv_admin_msg_reply *)data;
    struct devdrv_get_rx_atu_cmd *cmd_data = (struct devdrv_get_rx_atu_cmd *)data;
    struct agentdrv_devctrl *agent_dev = agentdrv_get_dev(msg_dev->dev_id);
    u32 pf_num;

    if (agent_dev == NULL) {
        devdrv_err("Device ID out of range. (dev_id=%d)\n", msg_dev->dev_id);
        return -EINVAL;
    }
    pf_num = devdrv_get_nvme_pf_num(msg_dev->dev_id);
    devdrv_rx_atu_init(agent_dev->apb_base, pf_num, cmd_data->bar_num,
        (struct devdrv_iob_atu*)reply->data, DEVDRV_MAX_RX_ATU_NUM);
    reply->len = sizeof(agent_dev->mem_rx_atu);

    return 0;
}

bool agentdrv_check_is_flr_finish(u32 dev_id)
{
    if (agentdrv_check_flr_reset_finish(dev_id) == 0) {
        return true;
    }
    return false;
}
EXPORT_SYMBOL(agentdrv_check_is_flr_finish);
