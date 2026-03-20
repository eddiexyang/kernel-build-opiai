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

#include "vhdc_core.h"
#include "vhdc_agent.h"
#include "vhdc_mem.h"

#ifndef CFG_FEATURE_VFIO
static int hdccom_rx_comm_msg_para_check(u32 dev_id, u32 fid, const struct vmng_rx_msg_proc_info *proc_info)
{
    if ((proc_info == NULL) || (proc_info->real_out_len == NULL) || (proc_info->data == NULL) ||
        (dev_id >= VMNG_PDEV_MAX) || (fid >= VMNG_VDEV_MAX_PER_PDEV)) {
        hdcdrv_err("Input pararmeter is error. (dev_id=%u; fid=%u)\n", dev_id, fid);
        return HDCDRV_PARA_ERR;
    }

    if ((proc_info->in_data_len < (sizeof(enum VHDC_CTRL_MSG_TYPE) + sizeof(int))) ||
        (proc_info->out_data_len < (sizeof(enum VHDC_CTRL_MSG_TYPE) + sizeof(int)))) {
        hdcdrv_err("Input pararmeter is error. (in_data_len=%u; out_data_len=%u)\n",
            proc_info->in_data_len, proc_info->out_data_len);
        return HDCDRV_PARA_ERR;
    }

    return HDCDRV_OK;
}

static int hdccom_rx_vpc_msg_para_check(u32 dev_id, u32 fid, const struct vmng_rx_msg_proc_info *proc_info)
{
    if ((proc_info == NULL) || (proc_info->real_out_len == NULL) || (proc_info->data == NULL) ||
        (dev_id >= VMNG_PDEV_MAX) || (fid >= VMNG_VDEV_MAX_PER_PDEV)) {
        hdcdrv_err("Input pararmeter is error. (dev_id=%u; fid=%u)\n", dev_id, fid);
        return HDCDRV_PARA_ERR;
    }

    if ((proc_info->in_data_len < (sizeof(struct vhdc_ioctl_msg) - sizeof(union hdcdrv_cmd))) ||
        (proc_info->out_data_len < (sizeof(struct vhdc_ioctl_msg) - sizeof(union hdcdrv_cmd)))) {
        hdcdrv_err("Input pararmeter is error. (in_data_len=%u; out_data_len=%u)\n",
            proc_info->in_data_len, proc_info->out_data_len);
        return HDCDRV_PARA_ERR;
    }

    return HDCDRV_OK;
}

static int hdccom_rx_comm_msg_type_check(unsigned int cmd_min_len, const struct vmng_rx_msg_proc_info *proc_info)
{
    int ctrl_msg_head = sizeof(enum VHDC_CTRL_MSG_TYPE) + sizeof(int);

    if (((cmd_min_len + ctrl_msg_head) > proc_info->in_data_len) ||
        ((cmd_min_len + ctrl_msg_head) > proc_info->out_data_len)) {
        hdcdrv_err("Input pararmeter is error. (in_data_len=%u; out_data_len=%u)\n",
            proc_info->in_data_len, proc_info->out_data_len);
        return HDCDRV_PARA_ERR;
    }

    return HDCDRV_OK;
}
#endif


long vhdca_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct vhdca_vctx *vctx = NULL;
    void __user *argp = (void __user *)((uintptr_t)arg);
    struct vhdc_ioctl_msg iomsg = {0};
    long ret;

    if ((file == NULL) || (file->private_data == NULL)) {
        hdcdrv_err("Input pararmeter is error.\n");
        return HDCDRV_PARA_ERR;
    }

    if ((_IOC_TYPE(cmd) != HDCDRV_CMD_MAGIC) || (argp == NULL)) {
        hdcdrv_err("cmd type is error. (cmd=%x)\n", cmd);
        return HDCDRV_PARA_ERR;
    }

    if (copy_from_user(&iomsg.cmd_data, argp, sizeof(union hdcdrv_cmd)) != 0) {
        hdcdrv_err("Calling copy_from_user failed. (cmd=%u)\n", _IOC_NR(cmd));
        return HDCDRV_COPY_FROM_USER_FAIL;
    }

    vctx = (struct vhdca_vctx *)file->private_data;
    iomsg.cmd_data.cmd_com.pid = hdcdrv_get_pid();

    ret = vhdca_cmd_operation(vctx, cmd, &iomsg);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    if (((bool)iomsg.copy_flag) && (copy_to_user(argp, &iomsg.cmd_data, sizeof(union hdcdrv_cmd)) != 0)) {
        hdcdrv_err("Calling copy_to_user failed. (cmd=%d)\n", _IOC_NR(cmd));
        return HDCDRV_COPY_TO_USER_FAIL;
    }

    return HDCDRV_OK;
}

int vhdca_open(struct inode *node, struct file *file)
{
    struct vhdca_vctx *vctx = NULL;

    vctx = kzalloc(sizeof(struct vhdca_vctx), GFP_KERNEL | __GFP_ACCOUNT);
    if (vctx == NULL) {
        hdcdrv_err("Calling kzalloc failed.\n");
        return HDCDRV_MEM_ALLOC_FAIL;
    }

    vctx->dev_id = VHDC_INVALID_DEV_ID;
    vctx->pid = (long long)hdcdrv_get_pid();
    vctx->hash = vhdca_hash_gen((u64)(uintptr_t)vctx, (u64)vctx->pid);
    vctx->file = file;

    spin_lock_init(&vctx->ctx_fmem.mem_lock);
    INIT_LIST_HEAD(&vctx->ctx_fmem.mlist.list);

    file->private_data = vctx;
    return HDCDRV_OK;
}

int vhdca_release(struct inode *node, struct file *file)
{
    struct vhdca_vctx *vctx = file->private_data;
    struct vmng_tx_msg_proc_info tx_info;
    struct vhdc_ctrl_msg msg;
    int ret;

    if (vctx == NULL) {
        hdcdrv_err("Input pararmeter is error. (task_pid=%llu)", hdcdrv_get_pid());
        return HDCDRV_PARA_ERR;
    }

    hdcdrv_release_free_mem(&vctx->ctx_fmem);

    if (vctx->dev_id != VHDC_INVALID_DEV_ID) {
        /* notify PM to free resource */
        msg.type = VHDC_CTRL_MSG_TYPE_RELEASE;
        msg.error_code = HDCDRV_ERR;
        msg.vhdc_release.hash = vctx->hash;

        tx_info.data = &msg;
        tx_info.in_data_len = sizeof(struct vhdc_ctrl_msg);
        tx_info.out_data_len = sizeof(struct vhdc_ctrl_msg);
        tx_info.real_out_len = 0;
        ret = vmnga_common_msg_send(vctx->dev_id, VMNG_MSG_COMMON_TYPE_HDC, &tx_info);
        if ((ret != 0) || (msg.error_code != HDCDRV_OK)) {
            hdcdrv_err("Calling vmnga_common_msg_send failed. (dev_id=%u; ret=%d; error_code=%d)\n",
                vctx->dev_id, ret, msg.error_code);
        }
    }

    /* free vctx */
    file->private_data = NULL;
    kfree(vctx);
    vctx = NULL;

    return HDCDRV_OK;
}

/*
 * 1. kernel virtual address max 48 bits, and it will not conflict
 * 2. 64 bits linux system pid max support 4194304 (23 bits)
 */
u64 vhdca_hash_gen(u64 va, u64 pid)
{
    return (((va << HDCDRV_RBTREE_PID) & HDCDRV_RBTREE_ADDR_MASK_L) | (pid & HDCDRV_RBTREE_PID_MASK));
}

int vhdca_com_msg_recv(u32 dev_id, u32 fid, struct vmng_rx_msg_proc_info *proc_info)
{
    struct vhdc_ctrl_msg *msg = NULL;
    int ret = HDCDRV_ERR;

    if (hdccom_rx_comm_msg_para_check(dev_id, fid, proc_info) != HDCDRV_OK) {
        hdcdrv_err("Calling hdccom_rx_comm_msg_para_check failed. (devid=%u; fid=%u)\n", dev_id, fid);
        return HDCDRV_PARA_ERR;
    }

    msg = (struct vhdc_ctrl_msg *)proc_info->data;

    switch (msg->type) {
        case VHDC_CTRL_MSG_TYPE_ALLOC_MEM:
            if (hdccom_rx_comm_msg_type_check(sizeof(struct vhdc_ctrl_msg_alloc_mempool), proc_info) != HDCDRV_OK) {
                hdcdrv_err("Calling hdccom_rx_comm_msg_type_check failed. (dev_id=%u; fid=%u)\n", dev_id, fid);
                return HDCDRV_PARA_ERR;
            }
            msg->alloc_mempool.mem_para.dev_id = (int)dev_id;
            msg->alloc_mempool.mem_para.fid = fid;
            ret = vhdca_alloc_mem(&msg->alloc_mempool.mem_para, &msg->alloc_mempool.buf,
                (dma_addr_t *)&msg->alloc_mempool.addr);
            if (ret != HDCDRV_OK) {
                hdcdrv_err("Calling vhdca_alloc_mem failed. (ret=%d)\n", ret);
                goto OUT_COM;
            }
            break;
        case VHDC_CTRL_MSG_TYPE_FREE_MEM:
            if (hdccom_rx_comm_msg_type_check(sizeof(struct vhdc_ctrl_msg_free_mempool), proc_info) != HDCDRV_OK) {
                hdcdrv_err("Calling hdccom_rx_comm_msg_type_check failed. (dev_id=%u; fid=%u)\n", dev_id, fid);
                return HDCDRV_PARA_ERR;
            }
            ret = vhdca_host_free_mem(msg->free_mempool.buf);
            if (ret != HDCDRV_OK) {
                hdcdrv_err("Calling vhdca_host_free_mem failed. (ret=%d)\n", ret);
                goto OUT_COM;
            }
            break;
        default:
            hdcdrv_err("msg_type is error. (msg type=%u)\n", msg->type);
            ret = HDCDRV_PARA_ERR;
            break;
    }

OUT_COM:
    *(proc_info->real_out_len) = sizeof(struct vhdc_ctrl_msg);
    msg->error_code = ret;

    return HDCDRV_OK;
}

int vhdca_vpc_msg_recv(u32 dev_id, u32 fid, struct vmng_rx_msg_proc_info *proc_info)
{
    struct vhdc_ioctl_msg *msg = NULL;
    int ret = HDCDRV_OK;

    if (hdccom_rx_vpc_msg_para_check(dev_id, fid, proc_info) != HDCDRV_OK) {
        hdcdrv_err("Calling hdccom_rx_vpc_msg_para_check error.\n");
        return HDCDRV_PARA_ERR;
    }

    msg = (struct vhdc_ioctl_msg *)proc_info->data;
    switch (msg->cmd) {
        default:
            hdcdrv_err("msg_type is error. (msg_type=%u)\n", _IOC_NR(msg->cmd));
            ret = HDCDRV_PARA_ERR;
        break;
    }

    *(proc_info->real_out_len) = sizeof(struct vhdc_ioctl_msg);
    msg->error_code = ret;

    return HDCDRV_OK;
}

STATIC long vhdca_get_config(struct hdcdrv_cmd_config *cmd)
{
    if (g_vhdca_ctrl->segment == HDCDRV_INVALID_PACKET_SEGMENT) {
        return HDCDRV_PARA_ERR;
    }

    if (cmd->segment > vhdca_mem_block_capacity()) {
        cmd->segment = vhdca_mem_block_capacity();
    }

    hdcdrv_info("Get segment value. (pid=%llu; use_segment=%d)\n", cmd->pid, cmd->segment);
    return HDCDRV_OK;
}

STATIC long vhdca_cmd_send_top_half(struct hdcdrv_cmd_send *cmd)
{
    struct vhdca_alloc_mem_para para;
    long ret;

    if (cmd->src_buf == NULL) {
        hdcdrv_err("Input pararmeter is error.\n");
        return HDCDRV_PARA_ERR;
    }

    if (((u32)cmd->dev_id >= VMNG_PDEV_MAX) || (cmd->len <= 0) || (cmd->len > vhdca_mem_block_capacity())) {
        hdcdrv_err("Input pararmeter is error. (devid=%d; len=%d)\n", cmd->dev_id, cmd->len);
        return HDCDRV_PARA_ERR;
    }

    para.dev_id = cmd->dev_id;
    para.pool_type = HDCDRV_MEM_POOL_TYPE_TX;
    para.len = cmd->len;

    ret = vhdca_alloc_mem(&para, &cmd->pool_buf, &cmd->pool_addr);
    if (ret != HDCDRV_OK) {
        hdcdrv_err("Calling vhdca_alloc_mem failed. (ret=%ld)\n", ret);
        return ret;
    }

    if (copy_from_user(cmd->pool_buf, (void __user *)cmd->src_buf, (unsigned long)cmd->len) != 0) {
        vhdca_free_mem(cmd->pool_buf);
        hdcdrv_err("Calling copy_from_user failed.\n");
        return HDCDRV_COPY_FROM_USER_FAIL;
    }

    return HDCDRV_CMD_CONTINUE;
}

STATIC long vhdca_cmd_recv_bottom_half(struct hdcdrv_cmd_recv *cmd)
{
    long ret = HDCDRV_OK, ret_free = HDCDRV_OK;
    int i = 0;
    unsigned int offset = 0;

    if (cmd->buf_count <= 0 || cmd->buf_count > HDCDRV_SESSION_RX_LIST_MAX_PKT) {
        hdcdrv_err("Input pararmeter is error. buf_count is invalid\n");
        return HDCDRV_PARA_ERR;
    }

    for (i = 0; i < cmd->buf_count; i++) {
        // If one buf is empty or out of capacity, release all buf in buf_list
        if ((cmd->buf_list[i] == NULL) || (cmd->buf_len[i] > vhdca_mem_block_capacity())) {
            hdcdrv_err("Input pararmeter is error. buf is NULL or len is illgeal. (index = %d; buf_len=%u; "
                "capacity=%d)\n", i, cmd->buf_len[i], vhdca_mem_block_capacity());
            ret = HDCDRV_PARA_ERR;
            goto FREE_POOL;
        }
    }

    if ((cmd->dst_buf == NULL) || (cmd->out_len > cmd->len)) {
        hdcdrv_err("Input pararmeter is error. dst_buf is NULL or out_len is illegal. (out_len=%d; buf_len=%d)\n",
            cmd->out_len, cmd->len);
        ret = HDCDRV_PARA_ERR;
        goto FREE_POOL;
    }

    for (i = 0; i < cmd->buf_count; i++) {
        if (copy_to_user((void __user *)cmd->dst_buf + offset, cmd->buf_list[i], (unsigned long)cmd->buf_len[i]) != 0) {
            hdcdrv_err("Calling copy_from_user failed.\n");
            ret = HDCDRV_COPY_TO_USER_FAIL;
            goto FREE_POOL;
        }
        offset += (unsigned int)cmd->buf_len[i];
    }

FREE_POOL:
    for (i = 0; i < cmd->buf_count; i++) {
        if (cmd->buf_list[i] != NULL) {
            ret_free = vhdca_free_mem(cmd->buf_list[i]);
            if (ret_free != HDCDRV_OK) {
                hdcdrv_err("Free mem is failed, buf_list index is %d, ret = %ld.\n", i, ret_free);
                ret = ret_free;
            }
            cmd->buf_list[i] = NULL;
        }
    }
    cmd->pool_buf = NULL;
    return ret;
}

STATIC long vhdca_epoll_wait_top_half(struct hdcdrv_cmd_epoll_wait *cmd)
{
    if (cmd->maxevents < 0) {
        hdcdrv_err("Input pararmeter is error. (maxevents=%d)\n", cmd->maxevents);
        return HDCDRV_PARA_ERR;
    }

    if (cmd->maxevents > HDCDRV_VEPOLL_EVENT_MAX) {
        cmd->maxevents = HDCDRV_VEPOLL_EVENT_MAX;
    }

    return HDCDRV_CMD_CONTINUE;
}

STATIC long vhdca_epoll_wait_bottom_half(struct hdcdrv_cmd_epoll_wait *cmd, struct vhdc_ioctl_msg *iomsg)
{
    u64 copy_size;

    if (cmd->event == NULL) {
        hdcdrv_err("Input pararmeter is error.\n");
        return HDCDRV_PARA_ERR;
    }

    if ((cmd->ready_event < 0) || (cmd->ready_event > cmd->maxevents) ||
        (cmd->ready_event > HDCDRV_VEPOLL_EVENT_MAX)) {
        hdcdrv_err("Input pararmeter is error. (ready_event=%d)\n", cmd->ready_event);
        return HDCDRV_PARA_ERR;
    }

    copy_size = (u64)sizeof(struct hdcdrv_event) * (unsigned int)cmd->ready_event;

    if ((copy_size > 0) && (copy_to_user((void __user *)cmd->event, (void *)&cmd->vevent[0], copy_size) != 0)) {
        hdcdrv_err("Calling copy_to_user failed. (size=%lld)\n", copy_size);
        return HDCDRV_COPY_TO_USER_FAIL;
    }

    return HDCDRV_OK;
}

STATIC void vhdca_vctx_bind_devid(struct vhdca_vctx *vctx, u32 dev_id)
{
    if (vctx->dev_id == VHDC_INVALID_DEV_ID) {
        vctx->dev_id = dev_id;
    }
}

long vhdca_cmd_deliver(struct vhdca_vctx *vctx, unsigned int cmd, struct vhdc_ioctl_msg *iomsg)
{
    struct vmng_tx_msg_proc_info tx_info;
    bool block_flag;
    u32 vpc_timeout;
    long ret;

    iomsg->cmd = cmd;
    iomsg->hash = vctx->hash;
    iomsg->error_code = HDCDRV_ERR;

    tx_info.data = iomsg;
    tx_info.in_data_len = sizeof(struct vhdc_ioctl_msg);
    tx_info.out_data_len = sizeof(struct vhdc_ioctl_msg);
    tx_info.real_out_len = 0;

    block_flag = vhdca_cmd_block_flag(cmd);
    vpc_timeout = block_flag ? VPC_BLK_MODE_TIMEOUT : VPC_DEFAULT_TIMEOUT;
    ret = vhdca_vpc_msg_send((u32)iomsg->cmd_data.cmd_com.dev_id, VMNG_VPC_TYPE_HDC, &tx_info, vpc_timeout);
    if (ret != 0) {
        /* no need check iomsg->error_code */
        hdcdrv_warn_limit("Calling vmnga_vpc_msg_send failed. (cmd=0x%x; ret=%ld; error_code=%d)\n",
            _IOC_NR(cmd), ret, iomsg->error_code);
        return HDCDRV_ERR;
    }

    return iomsg->error_code;
}
EXPORT_SYMBOL_UNRELEASE(vhdca_cmd_deliver);

STATIC long vhdca_operation_top_half(struct vhdca_vctx *vctx, unsigned int drv_cmd, struct vhdc_ioctl_msg *iomsg)
{
    long ret = HDCDRV_CMD_CONTINUE;

    switch (drv_cmd) {
        case HDCDRV_CMD_CONFIG:
            ret = vhdca_get_config(&iomsg->cmd_data.config);
            iomsg->copy_flag = true;
            break;
        case HDCDRV_CMD_SEND:
            ret = vhdca_cmd_send_top_half(&iomsg->cmd_data.send);
            break;
        case HDCDRV_CMD_SERVER_CREATE:
            vhdca_vctx_bind_devid(vctx, (u32)iomsg->cmd_data.server_create.dev_id);
            break;
        default:
            break;
    }

    return ret;
}

STATIC long vhdca_cfg_operation_top_half(struct vhdca_vctx *vctx, unsigned int drv_cmd, struct vhdc_ioctl_msg *iomsg)
{
    long ret = HDCDRV_CMD_CONTINUE;

    switch (drv_cmd) {
        case HDCDRV_CMD_SET_SESSION_OWNER:
            vhdca_vctx_bind_devid(vctx, (u32)iomsg->cmd_data.set_owner.dev_id);
            iomsg->cmd_data.set_owner.ppid = hdcdrv_get_ppid();
            break;
        case HDCDRV_CMD_GET_PAGE_SIZE:
            ret = hdcdrv_get_page_size(&iomsg->cmd_data.get_page_size);
            iomsg->copy_flag = true;
            break;
        case HDCDRV_CMD_GET_SESSION_ATTR:
            if (iomsg->cmd_data.get_session_attr.cmd_type == HDCDRV_SESSION_ATTR_DFX) {
                if (g_vhdca_ctrl->pm_version < HDC_VERSION_0002) {
                    hdcdrv_warn("If the dfx function is needed, update the VM version.\n");
                    return HDCDRV_NOT_SUPPORT;
                }
            }
        default:
            break;
    }

    return ret;
}

STATIC long vhdca_fast_operation_top_half(struct vhdca_vctx *vctx, unsigned int drv_cmd, struct vhdc_ioctl_msg *iomsg)
{
    long ret = HDCDRV_CMD_CONTINUE;

    switch (drv_cmd) {
        /* fast mem cmd alloc/free/map/unmap/remap only process in vm. */
        case HDCDRV_CMD_ALLOC_MEM:
            ret = vhdca_fast_alloc_mem(vctx, &iomsg->cmd_data.alloc_mem);
            break;
        case HDCDRV_CMD_FREE_MEM:
            ret = hdcdrv_fast_free_mem((void *)vctx, &iomsg->cmd_data.free_mem);
            iomsg->copy_flag = true;
            break;
        case HDCDRV_CMD_DMA_MAP:
            ret = hdcdrv_fast_dma_map(&iomsg->cmd_data.dma_map);
            break;
        case HDCDRV_CMD_DMA_UNMAP:
            ret = hdcdrv_fast_dma_unmap(&iomsg->cmd_data.dma_unmap);
            break;
        case HDCDRV_CMD_DMA_REMAP:
            ret = hdcdrv_fast_dma_remap(&iomsg->cmd_data.dma_remap);
            break;
        default:
            break;
    }

    return ret;
}

STATIC long vhdca_epoll_operation_top_half(struct vhdca_vctx *vctx, unsigned int drv_cmd, struct vhdc_ioctl_msg *iomsg)
{
    long ret = HDCDRV_CMD_CONTINUE;

    switch (drv_cmd) {
        case HDCDRV_CMD_EPOLL_ALLOC_FD:
            vhdca_vctx_bind_devid(vctx, (u32)iomsg->cmd_data.epoll_wait.dev_id);
            break;
        case HDCDRV_CMD_EPOLL_CTL:
            if ((iomsg->cmd_data.epoll_ctl.event.events & HDCDRV_EPOLL_CONN_IN) != 0) {
                iomsg->cmd_data.cmd_com.dev_id = iomsg->cmd_data.epoll_ctl.para1;
            }
            break;
        case HDCDRV_CMD_EPOLL_WAIT:
            ret = vhdca_epoll_wait_top_half(&iomsg->cmd_data.epoll_wait);
            break;
        default:
            break;
    }

    return ret;
}

STATIC long vhdca_cmd_top_half(struct vhdca_vctx *vctx, unsigned int drv_cmd, struct vhdc_ioctl_msg *iomsg)
{
    long ret;

    if (drv_cmd < HDCDRV_CMD_SET_SESSION_OWNER) {
        ret = vhdca_operation_top_half(vctx, drv_cmd, iomsg);
    } else if (drv_cmd < HDCDRV_CMD_ALLOC_MEM) {
        ret = vhdca_cfg_operation_top_half(vctx, drv_cmd, iomsg);
    } else if (drv_cmd < HDCDRV_CMD_EPOLL_ALLOC_FD) {
        ret = vhdca_fast_operation_top_half(vctx, drv_cmd, iomsg);
    } else {
        ret = vhdca_epoll_operation_top_half(vctx, drv_cmd, iomsg);
    }

    return ret;
}

STATIC void vhdca_top_half_free(struct vhdca_vctx *vctx, unsigned int drv_cmd, struct vhdc_ioctl_msg *iomsg)
{
    switch (drv_cmd) {
        case HDCDRV_CMD_SEND:
            vhdca_delay_free_mem(iomsg->cmd_data.send.pool_buf);
            break;

        default:
            break;
    }

    return;
}

STATIC long vhdca_cmd_bottom_half(struct vhdca_vctx *vctx, unsigned int drv_cmd, struct vhdc_ioctl_msg *iomsg)
{
    long ret = HDCDRV_ERR;

    switch (drv_cmd) {
        case HDCDRV_CMD_RECV:
            ret = vhdca_cmd_recv_bottom_half(&iomsg->cmd_data.recv);
            break;
        case HDCDRV_CMD_EPOLL_WAIT:
            ret = vhdca_epoll_wait_bottom_half(&iomsg->cmd_data.epoll_wait, iomsg);
            break;

        default:
            break;
    }

    return ret;
}

long vhdca_cmd_operation(struct vhdca_vctx *vctx, u32 cmd, struct vhdc_ioctl_msg *iomsg)
{
    u32 drv_cmd = _IOC_NR(cmd);
    long ret;

    ret = vhdca_cmd_top_half(vctx, drv_cmd, iomsg);
    if (ret != HDCDRV_CMD_CONTINUE) {
        return ret;
    }

    /* send to PM */
    ret = vhdca_cmd_deliver(vctx, cmd, iomsg);
    if (ret != HDCDRV_CMD_CONTINUE) {
        if (ret != HDCDRV_OK) {
            vhdca_top_half_free(vctx, drv_cmd, iomsg);
        }
        return ret;
    }

    ret = vhdca_cmd_bottom_half(vctx, drv_cmd, iomsg);
    if (ret != HDCDRV_OK) {
        hdcdrv_err("Calling vhdca_cmd_bottom_half failed. (drv_cmd=%d; ret=%ld)\n", drv_cmd, ret);
        return ret;
    }

    return ret;
}

bool vhdca_cmd_block_flag(unsigned int cmd)
{
    u32 drv_cmd = _IOC_NR(cmd);
    bool ret = false;

    switch (drv_cmd) {
        case HDCDRV_CMD_ACCEPT:
            /* full through */
        case HDCDRV_CMD_CONNECT:
            /* full through */
        case HDCDRV_CMD_SEND:
            /* full through */
        case HDCDRV_CMD_RECV_PEEK:
            /* full through */
        case HDCDRV_CMD_FAST_SEND:
            /* full through */
        case HDCDRV_CMD_FAST_RECV:
            /* full through */
        case HDCDRV_CMD_EPOLL_WAIT:
            ret = true;
            break;
        default:
            ret = false;
            break;
    }

    return ret;
}

long vhdca_fast_alloc_mem(struct vhdca_vctx *vctx, struct hdcdrv_cmd_alloc_mem *cmd)
{
    long ret;
    struct hdcdrv_fast_node *f_node = NULL;

    ret = hdccom_fast_alloc_mem((void *)vctx, cmd, &f_node);
    if ((ret != HDCDRV_OK) || (f_node == NULL)) {
        hdcdrv_err("Calling hdccom_fast_alloc_mem failed. (dev=%d)\n", cmd->dev_id);
        return ret;
    }

    if ((vctx != NULL) && (vctx != VHDC_KERNEL_WITHOUT_CTX)) {
        ret = hdcdrv_bind_mem_ctx(&vctx->ctx_fmem, f_node);
        if (ret != HDCDRV_OK) {
            hdcdrv_err("Calling hdcdrv_bind_mem_ctx failed. (dev=%d)\n", cmd->dev_id);
            hdcdrv_fast_unalloc_mem(cmd, f_node);
            return ret;
        }
    }

    f_node->ctx = (void *)vctx;

    return HDCDRV_OK;
}

