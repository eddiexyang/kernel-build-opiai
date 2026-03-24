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

#ifdef CONFIG_GENERIC_BUG
#undef CONFIG_GENERIC_BUG
#endif
#ifdef CONFIG_BUG
#undef CONFIG_BUG
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/jiffies.h>

#include "dbl/uda.h"

#include "hdcdrv_core.h"
#include "hdcdrv_device.h"
#include "kernel_version_adapt.h"
#include "ascend_hal_error.h"

STATIC check_hostpid g_hdcdrv_check_hostpid_func = NULL;
query_fid g_hdcdrv_get_fid_func = NULL;
STATIC query_localpid g_hdcdrv_get_localpid_func = NULL;
STATIC query_master_pid_by_host_slave g_hdcdrv_query_master_pid_by_host_slave_func = NULL;

int hdcdrv_get_link_status(struct devdrv_pcie_link_info_para *link_info)
{
    int link_status;
    int dev_id = 0;
    int ret;

    ret = devdrv_get_pcie_link_info((u32)dev_id, link_info);
    if (ret != 0) {
        hdcdrv_err("when query link status, get pcie status fail.(ret=%d, link_status=%d)\n",
            ret, link_info->link_status);
        return ret;
    }
    if (link_info->link_status != HDCDRV_LINK_NORMAL) {
        return HDCDRV_OK;
    }

    if (hdc_ctrl->segment == HDCDRV_INVALID_PACKET_SEGMENT) {
        link_info->link_status = HDCDRV_HDC_DISCONNECT;
        return HDCDRV_OK;
    }

    link_status = hdcdrv_get_device_status(dev_id);
    if (link_status == HDCDRV_VALID) {
        link_info->link_status = HDCDRV_LINK_NORMAL;
    } else {
        link_info->link_status = HDCDRV_HDC_DISCONNECT;
    }
    return HDCDRV_OK;
}

int hdcdrv_get_running_env(void)
{
    return HDCDRV_RUNNING_ENV_DAVICI;
}

enum devdrv_dma_direction hdcdrv_get_dma_direction(void)
{
    return DEVDRV_DMA_HOST_TO_DEVICE;
}

int hdcdrv_set_msg_chan_priv(void *msg_chan, void *priv)
{
    return agentdrv_set_msg_chan_priv(msg_chan, priv);
}

struct hdcdrv_msg_chan *hdcdrv_get_msg_chan_priv(void *msg_chan)
{
    return (struct hdcdrv_msg_chan *)agentdrv_get_msg_chan_priv(msg_chan);
}

struct hdcdrv_sq_desc *hdcdrv_get_w_sq_desc(void *msg_chan, u32 *tail)
{
    return (struct hdcdrv_sq_desc *)agentdrv_get_msg_chan_local_sq_tail(msg_chan, tail);
}

void hdcdrv_set_w_sq_desc_head(void *msg_chan, u32 head)
{
    agentdrv_set_msg_chan_local_sq_head(msg_chan, head);
}

void hdcdrv_copy_sq_desc_to_remote(struct hdcdrv_msg_chan *msg_dev, const struct hdcdrv_sq_desc *sq_desc,
    enum devdrv_dma_data_type data_type)
{
    void *msg_chan = msg_dev->chan;
    struct devdrv_asyn_dma_para_info para_info;
    dma_addr_t host_sq_addr;
    dma_addr_t local_sq_addr;
    int ret;

    host_sq_addr = agentdrv_get_msg_chan_host_sq_tail_dma_addr(msg_chan);
    local_sq_addr = agentdrv_get_msg_chan_local_sq_tail_dma_addr(msg_chan);

    para_info.interrupt_and_attr_flag = DEVDRV_REMOTE_IRQ_FLAG;
    para_info.priv = NULL;
    para_info.finish_notify = NULL;
    para_info.trans_id = 0;
    para_info.remote_msi_vector = agentdrv_get_remote_rx_msg_notify_irq(msg_chan);

    ret = devdrv_dma_async_copy_plus(msg_dev->dev_id, data_type, HDCDRV_DMA_CHAN_INVALID_INSTANCE, local_sq_addr,
        host_sq_addr, HDCDRV_SQ_DESC_SIZE, DEVDRV_DMA_DEVICE_TO_HOST, &para_info);
    if (ret) {
        hdcdrv_err("dev_id %d send sq desc to host failed. (ret=%d)\n", msg_dev->dev_id, ret);
    }

    agentdrv_move_msg_chan_local_sq_tail(msg_chan);
}

bool hdcdrv_w_sq_full_check(void *msg_chan)
{
    return agentdrv_msg_chan_local_sq_full_check(msg_chan);
}

struct hdcdrv_sq_desc *hdcdrv_get_r_sq_desc(void *msg_chan, u32 *head)
{
    return (struct hdcdrv_sq_desc *)agentdrv_get_msg_chan_reserve_sq_head(msg_chan, head);
}

void hdcdrv_move_r_sq_desc(void *msg_chan)
{
    agentdrv_move_msg_chan_reserve_sq_head(msg_chan);
}

struct hdcdrv_cq_desc *hdcdrv_get_w_cq_desc(void *msg_chan)
{
    return (struct hdcdrv_cq_desc *)agentdrv_get_msg_chan_local_cq_tail(msg_chan);
}

void hdcdrv_copy_cq_desc_to_remote(struct hdcdrv_msg_chan *msg_dev, const struct hdcdrv_cq_desc *cq_desc,
                                   enum devdrv_dma_data_type data_type)
{
    void *msg_chan = msg_dev->chan;
    struct devdrv_asyn_dma_para_info para_info;
    dma_addr_t host_cq_addr;
    dma_addr_t local_cq_addr;
    int ret;

    host_cq_addr = agentdrv_get_msg_chan_host_cq_tail_dma_addr(msg_chan);
    local_cq_addr = agentdrv_get_msg_chan_local_cq_tail_dma_addr(msg_chan);

    para_info.interrupt_and_attr_flag = DEVDRV_REMOTE_IRQ_FLAG;
    para_info.priv = NULL;
    para_info.finish_notify = NULL;
    para_info.trans_id = 0;
    para_info.remote_msi_vector = agentdrv_get_remote_tx_finish_notify_irq(msg_chan);

    ret = devdrv_dma_async_copy_plus(msg_dev->dev_id, data_type, HDCDRV_DMA_CHAN_INVALID_INSTANCE, local_cq_addr,
        host_cq_addr, HDCDRV_CQ_DESC_SIZE, DEVDRV_DMA_DEVICE_TO_HOST, &para_info);
    if (ret) {
        hdcdrv_err("dev_id %d send cq desc to host failed. (ret=%d)\n", msg_dev->dev_id, ret);
    }

    agentdrv_move_msg_chan_local_cq_tail(msg_chan);
}

struct hdcdrv_cq_desc *hdcdrv_get_r_cq_desc(void *msg_chan)
{
    return (struct hdcdrv_cq_desc *)agentdrv_get_msg_chan_reserve_cq_head(msg_chan);
}

void hdcdrv_move_r_cq_desc(void *msg_chan)
{
    agentdrv_move_msg_chan_reserve_cq_head(msg_chan);
}

long hdcdrv_ctrl_msg_send(u32 devid, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len)
{
    return (long)agentdrv_common_msg_send(devid, data, in_data_len, out_data_len, real_out_len,
                                          AGENTDRV_COMMON_MSG_HDC);
}
EXPORT_SYMBOL_UNRELEASE(hdcdrv_ctrl_msg_send);

long hdcdrv_non_trans_ctrl_msg_send(u32 devid, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len)
{
    struct hdcdrv_dev *hdc_dev = &hdc_ctrl->devices[devid];

    return (long)agentdrv_sync_msg_send(hdc_dev->ctrl_msg_chan, data, in_data_len, out_data_len, real_out_len);
}
EXPORT_SYMBOL_UNRELEASE(hdcdrv_non_trans_ctrl_msg_send);

STATIC int hdcdrv_non_trans_ctrl_msg_recv(void *msg_chan, void *data, u32 in_data_len,
    u32 out_data_len, u32 *real_out_len)
{
    u32 devid = (u32)agentdrv_get_msg_chan_devid(msg_chan);
    return hdcdrv_ctrl_msg_recv(devid, data, in_data_len, out_data_len, real_out_len);
}

u32 hdcdrv_get_container_id(void)
{
    /* device not support docker */
    return HDCDRV_PHY_HOST_ID;
}

u32 hdcdrv_get_vmid_from_pid(u64 pid)
{
    /* device not support virtual */
    return HDCDRV_DEFAULT_VM_ID;
}

int hdcdrv_check_hostpid(int hostpid, u32 devid, u32 vfid)
{
    if (hostpid == HDCDRV_INVALID_PEER_PID) {
        return HDCDRV_OK;
    }

    if (g_hdcdrv_check_hostpid_func == NULL) {
        g_hdcdrv_check_hostpid_func = (check_hostpid)(uintptr_t)__kallsyms_lookup_name("devdrv_check_hostpid");
    }

    if (g_hdcdrv_check_hostpid_func != NULL) {
        return g_hdcdrv_check_hostpid_func(hostpid, devid, vfid);
    }

    return HDCDRV_OK;
}

u32 hdcdrv_get_fid(u64 pid)
{
#ifdef CFG_FEATURE_VFIO_DEVICE
    int host_pid, chip_id, vfid, process_type;
    int ret = HDCDRV_ERR;

    if (g_hdcdrv_get_fid_func == NULL) {
        g_hdcdrv_get_fid_func = (query_fid)(uintptr_t)__kallsyms_lookup_name("devdrv_query_process_host_pid");
    }

    if (g_hdcdrv_get_fid_func != NULL) {
        ret = g_hdcdrv_get_fid_func(pid, &chip_id, &vfid, &host_pid, &process_type);
    }

    if (ret == HDCDRV_OK) {
        return vfid;
    }

    return HDCDRV_DEFAULT_PM_FID;
#else
    return HDCDRV_DEFAULT_PM_FID;
#endif
}

int hdcdrv_get_localpid(u32 hostpid, u32 chip_id, int processs_type, u32 vfid, int *pid)
{
    int ret = HDCDRV_ERR;

    if (g_hdcdrv_get_localpid_func == NULL) {
        g_hdcdrv_get_localpid_func =
            (query_localpid)(uintptr_t)__kallsyms_lookup_name("devdrv_query_process_by_host_pid");
    }

    if (g_hdcdrv_get_localpid_func != NULL) {
        ret = g_hdcdrv_get_localpid_func(hostpid, chip_id, processs_type, vfid, pid);
    }

    return ret;
}

STATIC int hdcdrv_query_master_pid_by_host_slave(int slave_pid, u32 *master_pid)
{
    int ret = HDCDRV_ERR;

    if (g_hdcdrv_query_master_pid_by_host_slave_func == NULL) {
        g_hdcdrv_query_master_pid_by_host_slave_func =
            (query_master_pid_by_host_slave)(uintptr_t)__kallsyms_lookup_name("devdrv_query_master_pid_by_host_slave");
    }

    if (g_hdcdrv_query_master_pid_by_host_slave_func != NULL) {
        ret = g_hdcdrv_query_master_pid_by_host_slave_func(slave_pid, master_pid);
    }

    return ret;
}

u64 hdcdrv_get_peer_pid(u32 devid, u64 host_pid, u32 fid, u64 peer_pid, int service_type)
{
    int local_pid;

    /* in this case the peer_pid from connect_msg is incredible */
    if (fid != HDCDRV_DEFAULT_PM_FID) {
        if ((service_type != HDCDRV_SERVICE_TYPE_TDT) && (service_type != HDCDRV_SERVICE_TYPE_DVPP)
            && (service_type != HDCDRV_SERVICE_TYPE_QUEUE)) {
            return HDCDRV_INVALID_PID;
        }

        if (hdcdrv_get_localpid(host_pid, devid, 0, fid, &local_pid) == HDCDRV_OK) {
            return (u64)local_pid;
        }
        hdcdrv_warn("devid %u fid %u hostpid %llu get localpid not success\n", devid, fid, host_pid);
        return HDCDRV_INVALID_PID;
    }

    if (((service_type == HDCDRV_SERVICE_TYPE_DVPP) || (service_type == HDCDRV_SERVICE_TYPE_QUEUE)) &&
        (peer_pid == HDCDRV_INVALID_PEER_PID)) {
        u32 master_pid = (u32)host_pid;

        if (service_type == HDCDRV_SERVICE_TYPE_QUEUE) {
            /* check if host pid is host slave proc, get it`s master pid */
            if (hdcdrv_query_master_pid_by_host_slave((int)host_pid, &master_pid) == 0) {
                hdcdrv_info("Host slave query success. (host_pid=%u; master_pid=%u)\n", (u32)host_pid, master_pid);
            }
        }

        /* query cp tgid from app master tgid */
        if (hdcdrv_get_localpid(master_pid, devid, 0, fid, &local_pid) == HDCDRV_OK) {
            return (u64)local_pid;
        }

        hdcdrv_warn("Get not success. (devid=%u; fid=%u; hostpid=%llu; master_pid=%u)\n",
            devid, fid, host_pid, master_pid);
    }
    return peer_pid;
}

int hdcdrv_service_scope_init(int service_type)
{
    if ((service_type == HDCDRV_SERVICE_TYPE_DVPP) || (service_type == HDCDRV_SERVICE_TYPE_QUEUE)) {
        return HDCDRV_SERVICE_SCOPE_PROCESS;
    }

#ifdef CFG_FEATURE_VFIO_DEVICE
    if (service_type == HDCDRV_SERVICE_TYPE_TDT) {
        return HDCDRV_SERVICE_SCOPE_PROCESS;
    }
#endif

    return HDCDRV_SERVICE_SCOPE_GLOBAL;
}

void hdcdrv_alloc_session_chan(int dev_id, int fid, int service_type, u32 *normal_chan_id, u32 *fast_chan_id)
{
#ifdef CFG_FEATURE_VFIO_DEVICE
    *fast_chan_id = HDCDRV_INVALID_CHAN_ID;
    *normal_chan_id = HDCDRV_INVALID_CHAN_ID;
#else
    *normal_chan_id = (u32)service_type % hdc_ctrl->devices[dev_id].normal_chan_num;
    *fast_chan_id = hdcdrv_alloc_fast_msg_chan(dev_id, service_type);
#endif
}

u64 hdcdrv_rebuild_pid(u32 devid, u32 fid, u64 pid)
{
    (void)devid;
    (void)fid;
    return pid;
}

STATIC int hdcdrv_init_trans_msg_chan_notify(void *msg_chan)
{
    struct hdcdrv_dev *hdc_dev = NULL;
    int dev_id;

    if (msg_chan == NULL) {
        return HDCDRV_ERR;
    }

    dev_id = agentdrv_get_msg_chan_devid(msg_chan);
    if ((dev_id < 0) || (dev_id >= hdcdrv_get_max_support_dev())) {
        hdcdrv_err("dev_id %d out of range\n", dev_id);
        return HDCDRV_ERR;
    }
    hdc_dev = &hdc_ctrl->devices[dev_id];

#ifndef HDCDRV_UT_TEST
    if (hdcdrv_add_msg_chan_to_dev(hdc_dev->dev_id, msg_chan)) {
        hdcdrv_err("dev_id %d add msg chan to dev failed.\n", hdc_dev->dev_id);
        return HDCDRV_ERR;
    }

    if (hdcdrv_get_running_status() == HDCDRV_RUNNING_RESUME) {
        hdcdrv_set_running_status(HDCDRV_RUNNING_NORMAL);
    }
#endif
    return 0;
}

STATIC int hdcdrv_uninit_trans_msg_chan_notify(void *msg_chan)
{
    struct hdcdrv_dev *hdc_dev = NULL;
    int dev_id;

    if (msg_chan == NULL) {
        return HDCDRV_ERR;
    }
    if (hdcdrv_get_running_status() != HDCDRV_RUNNING_NORMAL) {
        hdcdrv_err("suspend period\n");
        return HDCDRV_OK;
    }

    dev_id = agentdrv_get_msg_chan_devid(msg_chan);
    if ((dev_id < 0) || (dev_id >= hdcdrv_get_max_support_dev())) {
        hdcdrv_err("dev_id %d out of range\n", dev_id);
        return HDCDRV_ERR;
    }
    hdc_dev = &hdc_ctrl->devices[dev_id];

    if (hdc_dev->msg_chan_cnt == 0) {
        hdcdrv_info("dev_id %d msg_chan has been uninit\n", dev_id);
        return 0;
    }

    (void)agentdrv_set_msg_chan_priv(msg_chan, NULL);

    mutex_lock(&hdc_dev->mutex);
    hdc_dev->msg_chan_cnt--;
    mutex_unlock(&hdc_dev->mutex);

    if (hdc_dev->msg_chan_cnt == 0) {
        hdcdrv_uninit_mem_pool(dev_id);
    }

    return 0;
}

STATIC int hdcdrv_init_non_trans_msg_chan_notify(void *msg_chan)
{
    u32 dev_id;
    int ret;

    if (msg_chan == NULL) {
        hdcdrv_err("msg_chan is null\n");
        return HDCDRV_ERR;
    }

    dev_id = agentdrv_get_msg_chan_devid(msg_chan);

    ret = hdcdrv_add_ctrl_msg_chan_to_dev(dev_id, msg_chan);
    if (ret == HDCDRV_OK) {
        hdcdrv_set_device_status(dev_id, HDCDRV_VALID);
    }

    return ret;
}

int hdcdrv_init_instance(u32 dev_id, struct device *dev)
{
    struct hdcdrv_dev *hdc_dev = NULL;

    hdc_dev = hdcdrv_add_dev(dev, dev_id);
    if (hdc_dev == NULL) {
        hdcdrv_err("dev_id %d add failed", dev_id);
        return HDCDRV_ERR;
    }

    hdcdrv_info("init instance dev_id %d.\n", dev_id);

    return 0;
}

int hdcdrv_uninit_instance(u32 dev_id)
{
    hdcdrv_del_dev(dev_id);
    hdcdrv_info("uninit instance dev_id %d.\n", dev_id);
    return 0;
}

#define HDCDRV_DEVICE_NOTIFIER "hdc_device"
#ifndef DRV_UT
STATIC int hdcdrv_device_notifier_func(u32 udevid, enum uda_notified_action action)
{
    struct device *dev = NULL;
    int ret = 0;

    if (udevid >= HDCDRV_SUPPORT_MAX_DEV) {
        hdcdrv_err("Invalid para. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    dev = uda_get_agent_device(udevid);
    if (dev != NULL) { /* obp virtual dev is null, not need to init virtual dev */
        if (action == UDA_INIT) {
            ret = hdcdrv_init_instance(udevid, dev);
        } else if (action == UDA_UNINIT) {
            ret = hdcdrv_uninit_instance(udevid);
        } else if (action == UDA_SUSPEND) {
            ret = hdcdrv_suspend(udevid);
        } else if (action == UDA_RESUME) {
            ret = hdcdrv_resume(udevid, dev);
        } else {
            hdcdrv_warn("notifier action unkown. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);
        }
    }

    hdcdrv_info("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);

    return ret;
}
#endif
static struct agentdrv_trans_msg_client hdcdrv_device_msg_client = {
    .type = agentdrv_msg_client_hdc,
    .init_trans_msg_chan = hdcdrv_init_trans_msg_chan_notify,
    .uninit_trans_msg_chan = hdcdrv_uninit_trans_msg_chan_notify,
    .rx_trans_msg_notify = hdcdrv_rx_msg_notify,
    .tx_trans_finish_notify = hdcdrv_tx_finish_notify,
};

STATIC const struct agentdrv_non_trans_msg_client hdcdrv_device_non_trans_msg_client = {
    .type = agentdrv_msg_client_hdc,
    .flag = 0,
    .init_non_trans_msg_chan = hdcdrv_init_non_trans_msg_chan_notify,
    .uninit_non_trans_msg_chan = NULL,
    .non_trans_msg_process = hdcdrv_non_trans_ctrl_msg_recv,
};

struct agentdrv_common_msg_client hdcdrv_device_comm_msg_client = {
    .type = AGENTDRV_COMMON_MSG_HDC,
    .common_msg_recv = hdcdrv_ctrl_msg_recv,
};

STATIC int __init hdcdrv_init_module(void)
{
    struct uda_dev_type type;
    int ret;

    ret = hdcdrv_init();
    if (ret) {
        hdcdrv_err("call hdcdrv_init failed");
        return ret;
    }

    hdcdrv_set_segment(HDCDRV_INVALID_PACKET_SEGMENT);

#ifndef DRV_UT
    uda_davinci_local_real_agent_type_pack(&type);
    ret = uda_real_virtual_notifier_register(HDCDRV_DEVICE_NOTIFIER, &type, UDA_PRI1, hdcdrv_device_notifier_func);
    if (ret != 0) {
        hdcdrv_uninit();
        hdcdrv_err("uda_real_virtual_notifier_register failed\n");
        return ret;
    }
#endif

    ret = agentdrv_register_trans_msg_client(&hdcdrv_device_msg_client);
    if (ret != 0) {
        (void)uda_real_virtual_notifier_unregister(HDCDRV_DEVICE_NOTIFIER, &type);
        hdcdrv_uninit();
        hdcdrv_err("agentdrv_register_trans_msg_client failed\n");
        return ret;
    }

    ret = agentdrv_register_non_trans_msg_client(&hdcdrv_device_non_trans_msg_client);
    if (ret != 0) {
        (void)agentdrv_unregister_trans_msg_client(&hdcdrv_device_msg_client);
        (void)uda_real_virtual_notifier_unregister(HDCDRV_DEVICE_NOTIFIER, &type);
        hdcdrv_uninit();
        hdcdrv_err("call devdrv_register_common_msg_client failed");
        return ret;
    }

    ret = agentdrv_register_common_msg_client(&hdcdrv_device_comm_msg_client);
    if (ret) {
        (void)agentdrv_unregister_non_trans_msg_client(&hdcdrv_device_non_trans_msg_client);
        (void)agentdrv_unregister_trans_msg_client(&hdcdrv_device_msg_client);
        (void)uda_real_virtual_notifier_unregister(HDCDRV_DEVICE_NOTIFIER, &type);
        hdcdrv_uninit();
        hdcdrv_err("call devdrv_register_common_msg_client failed");
        return ret;
    }

    hdcdrv_info("hdcdrv_init_module success\n");
    return HDCDRV_OK;
}

STATIC void __exit hdcdrv_exit_module(void)
{
    struct uda_dev_type type;

    uda_davinci_local_real_agent_type_pack(&type);
    (void)agentdrv_unregister_common_msg_client(&hdcdrv_device_comm_msg_client);
    (void)agentdrv_unregister_non_trans_msg_client(&hdcdrv_device_non_trans_msg_client);
    (void)agentdrv_unregister_trans_msg_client(&hdcdrv_device_msg_client);
    (void)uda_real_virtual_notifier_unregister(HDCDRV_DEVICE_NOTIFIER, &type);
    hdcdrv_uninit();
}

module_init(hdcdrv_init_module);
module_exit(hdcdrv_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("hdcdrv device driver");
