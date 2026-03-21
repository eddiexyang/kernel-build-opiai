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
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/printk.h>
#include <asm/atomic.h>
#include <linux/hashtable.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include <stdbool.h>

#include "ascend_kernel_hal.h"
#include "cdq_manager.h"
#include "securec.h"
#include "devdrv_user_common.h"
#include "devdrv_devinit.h"
#include "tsdrv_sync.h"
#include "tsdrv_device.h"
#include "tsdrv_interface.h"
#include "tsdrv_cdqm_module.h"

#ifdef CFG_FEATURE_CDQM
int tsdrv_msg_sync_ssid(u32 dev_id, u32 tsid, u32 hpid, u32 vfid);
struct cdqm_dev_manage *g_cdq_dev_manage[DEVDRV_MAX_DAVINCI_NUM][DEVDRV_MAX_TS_NUM] = {NULL, };

STATIC struct cdqm_cdq_manage *tsdrv_cdqm_get_cdq_info(u32 devid, u32 tsid, u32 cdq_id)
{
    return &g_cdq_dev_manage[devid][tsid]->cdq_manage[cdq_id];
}

STATIC struct cdqm_dev_manage *tsdrv_cdqm_get_cdq_dev(u32 devid, u32 tsid)
{
    if (g_cdq_dev_manage[devid][tsid] == NULL) {
        TSDRV_PRINT_WARN("cdqm dev %u ts %u is NULL.\n", devid, tsid);
        return NULL;
    }
    return g_cdq_dev_manage[devid][tsid];
}

#define CDQM_F2NF_MAX_NS_NUM 16
STATIC void tsdrv_cdqm_drv_f2nf_set(u32 devid, u32 tsid, u32 irq)
{
    u8 ns_num;

    for (ns_num = 0; ns_num < CDQM_F2NF_MAX_NS_NUM; ns_num++) {
        tsdrv_cdqm_set_int_irq(g_cdq_dev_manage[devid][tsid]->int_io_base, irq, ns_num);
    }
}

/* 采用name+pid或者name来查询cdq_id，如果没有pid，采用INVALID_PID */
STATIC int cdqm_drv_get_cdq_id(u32 devid, u32 tsid, int pid, char *name, u32 *cdq_id)
{
    struct cdqm_dev_manage *cdq_dev = tsdrv_cdqm_get_cdq_dev(devid, tsid);
    struct cdqm_cdq_manage *cdq_manage = cdq_dev->cdq_manage;
    int i;

    mutex_lock(&cdq_dev->dev_mutex);
    for (i = 0; i < MAX_CDQM_CDQ_NUM; i++) {
        if (cdq_manage[i].config_info.pid == pid && strcmp(cdq_manage[i].config_info.name, name) == 0 &&
            cdq_manage[i].valid == CDQ_VALID) {
            *cdq_id = cdq_manage[i].config_info.cdq_id;
            mutex_unlock(&cdq_dev->dev_mutex);
            return 0;
        }
    }
    mutex_unlock(&cdq_dev->dev_mutex);

    return (int)DRV_ERROR_CDQ_NOT_EXIST;
}

void cdqm_clear_cdq_info(struct cdqm_cdq_manage *cdq_manage)
{
    cdq_manage->valid = 0;
    cdq_manage->create_side = 0;
    cdq_manage->time_out_flag = 0;
    cdq_manage->config_info.pid = 0;
}

STATIC int cdqm_judge_cdq_full(struct cdqm_cdq_manage *cdq_manage)
{
    return (cdq_manage->use_batch_head == (cdq_manage->use_batch_tail + 1) % cdq_manage->config_info.batch_num) ? 1 : 0;
}

STATIC int tsdrv_cdqm_create_para_check(u32 devid, u32 tsid, struct tsdrv_cdqm_create_para *para, int pid)
{
    u32 cdq_id;
    u32 batch_num = para->batch_num;
    u32 batch_size = para->batch_size;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM || tsid >= DEVDRV_MAX_TS_NUM || para->mem_addr == 0) {
        TSDRV_PRINT_ERR("devid(%u) tsid(%u) mem_addr(%pK) failed.\n", devid, tsid, (void *)para->mem_addr);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    if (batch_num > MAX_CDQ_DEPTH || batch_size > MAX_CDQ_DEPTH || batch_num <= 1 || batch_size == 0 ||
        batch_num * batch_size > MAX_CDQ_DEPTH) {
        TSDRV_PRINT_ERR("batch_size(%u) batch_num(%u) error.\n", batch_size, batch_num);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    if (cdqm_drv_get_cdq_id(devid, tsid, pid, para->name, &cdq_id) == 0) {
        TSDRV_PRINT_ERR("pid(%d) name(%s) repeated.\n", pid, para->name);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    return (int)DRV_ERROR_NONE;
}

static int tsdrv_cdqm_update_ssid(u32 devid, u32 tsid, struct tsdrv_cdqm_create_para *para, int pid)
{
    int proc_ssid = tsdrv_msg_sync_ssid(devid, tsid, pid, 0);
    if ((proc_ssid == -1) || (para->ssid != 0)) {
        TSDRV_PRINT_ERR("Invalid para. (devid=%u; tsid=%u; proc_ssid=%d; ssid=%u)\n",
            devid, tsid, proc_ssid, para->ssid);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    para->ssid = proc_ssid;

    return (int)DRV_ERROR_NONE;
}

STATIC int tsdrv_cdqm_cdq_id_para_check(u32 devid, u32 tsid, u32 cdq_id)
{
    struct cdqm_cdq_manage *cdq_manage = NULL;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM || tsid >= DEVDRV_MAX_TS_NUM || cdq_id >= MAX_CDQM_CDQ_NUM) {
        TSDRV_PRINT_ERR("devid(%u) tsid(%u) failed.\n", devid, tsid);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    cdq_manage = tsdrv_cdqm_get_cdq_info(devid, tsid, cdq_id);
    if (cdq_manage->valid != CDQ_VALID) {
        TSDRV_PRINT_ERR("devid %u tsid %u cdqId %u valid %u\n", devid, tsid, cdq_id, (u32)cdq_manage->valid);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    return 0;
}

STATIC int tsdrv_cdqm_exist_cdq_para_check(u32 devid, u32 tsid, u32 cdq_id, int pid)
{
    int ret;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    ret = tsdrv_cdqm_cdq_id_para_check(devid, tsid, cdq_id);
    if (ret != 0) {
        return ret;
    }

    cdq_manage = tsdrv_cdqm_get_cdq_info(devid, tsid, cdq_id);
    if (cdq_manage->recycle_pid != 0) {
        return 0;
    }

    if (cdq_manage->config_info.pid != pid) {
        TSDRV_PRINT_ERR("devid %u tsid %u cdqId %u valid %u manage->pid %d current pid %d error.\n",
            devid, tsid, cdq_id, (u32)cdq_manage->valid, cdq_manage->config_info.pid, pid);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    return 0;
}


/* 接收消息处理函数 */
STATIC int tsdrv_cdqm_drv_create_msg_handle(u32 devid, u32 tsid, struct cdq_cfg_info *cdq_info)
{
    int ret;
    struct tsdrv_cdqm_create_para para;
    struct cdqm_dev_manage *cdq_dev = NULL;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    para.mem_addr = cdq_info->mem_addr;
    para.batch_num = cdq_info->batch_num;
    para.batch_size = cdq_info->batch_size;
    ret = strcpy_s(para.name, sizeof(para.name), cdq_info->name);
    if (ret != 0) {
        TSDRV_PRINT_ERR("strcpy_s err, devid = %u, tsid = %u, ret = %d", devid, tsid, ret);
        return -EINVAL;
    }

    ret = tsdrv_cdqm_create_para_check(devid, tsid, &para, cdq_info->pid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("para check failed.\n");
        return ret;
    }

    cdq_dev = g_cdq_dev_manage[devid][tsid];

    ret = tsdrv_cdqm_create_msg_handle_by_side(cdq_dev, cdq_info);
    if (ret != 0) {
        return ret;
    }

    cdq_manage = tsdrv_cdqm_get_cdq_info(devid, tsid, cdq_info->cdq_id);

    cdq_manage->config_info = *cdq_info;
    cdq_manage->use_batch_head = 0;
    cdq_manage->use_batch_tail = 0;
    cdq_manage->time_out_flag = 0;
    cdq_manage->recycle_pid = 0;
    cdq_manage->last_get_batch = cdq_info->batch_num - 1;

    TSDRV_PRINT_INFO("cdqm dev %u ts %u cdq %u addr(%pK) ssid %u batch_num %u batch_size %u "
        "create handle ok.\n", devid, tsid, cdq_info->cdq_id, (void *)cdq_info->mem_addr,
        cdq_info->ssid, cdq_info->batch_num, cdq_info->batch_size);

    return 0;
}

STATIC int tsdrv_cdqm_drv_destroy_msg_handle(u32 devid, u32 tsid, struct cdqm_destroy_msg *msg)
{
    int ret;
    struct cdqm_dev_manage *cdq_dev = NULL;

    ret = tsdrv_cdqm_exist_cdq_para_check(devid, tsid, msg->cdq_id, msg->pid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("destroy msg para check fail.\n");
        return ret;
    }

    cdq_dev = tsdrv_cdqm_get_cdq_dev(devid, tsid);

    return tsdrv_cdqm_destroy_handle_by_side(devid, tsid, msg->cdq_id, cdq_dev);
}

STATIC int tsdrv_cdqm_drv_batch_abnormal_msg_handle(u32 devid, u32 tsid, u32 cdq_id)
{
    int ret;
    struct cdqm_cdq_manage *cdq_info = NULL;

    ret = tsdrv_cdqm_cdq_id_para_check(devid, tsid, cdq_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("msg para check failed:%d.\n", ret);
        return ret;
    }

    cdq_info = tsdrv_cdqm_get_cdq_info(devid, tsid, cdq_id);

    cdq_info->time_out_flag = 1;

    wake_up_interruptible(&cdq_info->wq);

    return 0;
}

s32 tsdrv_sync_cdqm_init_proc(u32 devid, struct tsdrv_msg_info *msg_head, u32 *ack_len)
{
    struct tsdrv_cdqm_init_msg *msg = NULL;
    u32 tsid;

    msg = (struct tsdrv_cdqm_init_msg *)msg_head->payload;
    tsid = msg_head->header.tsid;
    TSDRV_PRINT_INFO("init proc devid %u, ts %u, irq %u.\n", devid, tsid, msg->f2nf_irq);

    if (msg_head->header.valid != TSDRV_MSG_SYNC_MAGIC) {
        TSDRV_PRINT_ERR("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
    }
    tsdrv_cdqm_drv_f2nf_set(msg_head->header.dev_id, msg_head->header.tsid, msg->f2nf_irq);
    *ack_len = (u32)sizeof(struct tsdrv_msg_info);

    msg_head->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    msg_head->header.result = 0;

    return 0;
}

s32 tsdrv_sync_cdqm_create_proc(u32 devid, struct tsdrv_msg_info *msg_head, u32 *ack_len)
{
    struct cdq_cfg_info *msg = (struct cdq_cfg_info *)msg_head->payload;
    int ret;
    u32 tsid;

    tsid = msg_head->header.tsid;
    if (msg_head->header.valid != TSDRV_MSG_SYNC_MAGIC) {
        devdrv_drv_err("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
    }

    msg_head->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;

    ret = tsdrv_cdqm_drv_create_msg_handle(devid, msg_head->header.tsid, msg);
    if (ret != 0) {
        devdrv_drv_err("create cdq failed:%d.\n", ret);
        msg_head->header.result = (u16)ret;
        return ret;
    }

    *ack_len = (u32)sizeof(struct tsdrv_msg_info);
    msg_head->header.result = 0;

    return 0;
}

s32 tsdrv_sync_cdqm_destroy_proc(u32 devid, struct tsdrv_msg_info *msg_head, u32 *ack_len)
{
    struct cdqm_destroy_msg *msg = (struct cdqm_destroy_msg *)msg_head->payload;
    int ret;

    if (msg_head->header.valid != TSDRV_MSG_SYNC_MAGIC) {
        devdrv_drv_err("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
    }

    msg_head->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;

    ret = tsdrv_cdqm_drv_destroy_msg_handle(devid, msg_head->header.tsid, msg);
    if (ret != 0) {
        devdrv_drv_err("destroy cdq %u failed:%d.\n", msg->cdq_id, ret);
        msg_head->header.result = ret;
        return ret;
    }

    *ack_len = (u32)sizeof(struct tsdrv_msg_info);
    msg_head->header.result = 0;

    TSDRV_PRINT_INFO("destroy proc ok, dev%u ts %u cdq %u\n", devid, msg_head->header.tsid, msg->cdq_id);

    return 0;
}

s32 tsdrv_sync_cdqm_batch_abnormal_proc(u32 devid, struct tsdrv_msg_info *msg_head, u32 *ack_len)
{
    struct cdqm_batch_abnormal_msg *msg = (struct cdqm_batch_abnormal_msg *)msg_head->payload;
    int ret;

    if (msg_head->header.valid != TSDRV_MSG_SYNC_MAGIC) {
        devdrv_drv_err("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
    }

    msg_head->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;

    ret = tsdrv_cdqm_drv_batch_abnormal_msg_handle(devid, msg_head->header.tsid, msg->cdq_id);
    if (ret != 0) {
        devdrv_drv_err("set timeout cdq %u failed:%d.\n", msg->cdq_id, ret);
        msg_head->header.result = ret;
        return ret;
    }

    *ack_len = (u32)sizeof(struct tsdrv_msg_info);

    msg_head->header.result = 0;

    return 0;
}

STATIC int tsdrv_cdqm_drv_init_cdq_info(u32 devid, u32 tsid, int pid,
    struct tsdrv_cdqm_create_para *para, struct cdq_cfg_info *cdq_info)
{
    int ret;

    ret = strcpy_s(cdq_info->name, CDQ_NAME_LEN, para->name);
    if (ret != EOK) {
        TSDRV_PRINT_ERR("cdq_info strcpy failed: %d\n", ret);
        return (int)DRV_ERROR_INNER_ERR;
    }

    cdq_info->name[CDQ_NAME_LEN - 1] = '\0';
    cdq_info->batch_num = para->batch_num;
    cdq_info->batch_size = para->batch_size;
    cdq_info->ts_id = tsid;
    cdq_info->mem_addr = para->mem_addr;
    cdq_info->sid = CDQM_STREAM_ID;
    cdq_info->ssid = para->ssid;
    cdq_info->pid = pid;

    return 0;
}

/* 主流程 */
STATIC int tsdrv_cdqm_drv_create_cdq(u32 devid, u32 tsid, struct tsdrv_cdqm_create_para *para)
{
    int ret;
    int pid = current->tgid;
    u32 cdq_id;
    struct cdqm_dev_manage *cdq_dev = NULL;
    struct cdq_cfg_info cdq_info;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    ret = tsdrv_cdqm_create_para_check(devid, tsid, para, pid);
    if (ret != 0) {
        return ret;
    }

    ret = tsdrv_cdqm_update_ssid(devid, tsid, para, pid);
    if (ret != 0) {
        return ret;
    }

    ret = tsdrv_cdqm_drv_init_cdq_info(devid, tsid, pid, para, &cdq_info);
    if (ret != 0) {
        return ret;
    }

    cdq_dev = tsdrv_cdqm_get_cdq_dev(devid, tsid);
    ret = tsdrv_cdqm_create_cdq_id(cdq_dev, &cdq_info, &cdq_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev %u ts %u name %s alloc cdq id failed:%d.\n", devid, tsid, cdq_info.name, ret);
        return ret;
    }

    cdq_manage = &cdq_dev->cdq_manage[cdq_id];

    mutex_lock(&cdq_manage->cdq_mutex);

    ret = tsdrv_cdqm_sync_cdq_info(cdq_dev, &cdq_info, cdq_id);
    if (ret != 0) {
        tsdrv_cdqm_uninit_cdq_hardware(cdq_dev, cdq_id);
        mutex_unlock(&cdq_manage->cdq_mutex);
        return ret;
    }

    cdq_manage->config_info = cdq_info;
    cdq_manage->use_batch_head = 0;
    cdq_manage->use_batch_tail = 0;
    cdq_manage->time_out_flag = 0;
    cdq_manage->recycle_pid = 0;
    cdq_manage->create_side = tsdrv_cdqm_get_local_side();
    cdq_manage->valid = CDQ_VALID;
    cdq_manage->last_get_batch = cdq_info.batch_num - 1;

    mutex_unlock(&cdq_manage->cdq_mutex);

    para->qid = cdq_id;

    TSDRV_PRINT_INFO("cdqm dev %u ts %u cdq %u addr(%pK) ssid %u batch_num %u batch_size %u create success.\n",
        devid, tsid, para->qid, (void *)para->mem_addr, para->ssid, para->batch_num, para->batch_size);

    return 0;
}

STATIC int tsdrv_cdqm_drv_destroy_cdq(u32 devid, u32 tsid, struct tsdrv_cdqm_destroy_para *para)
{
    int ret;
    int pid = current->tgid;
    u32 cdq_id = para->qid;
    struct cdqm_dev_manage *cdq_dev = NULL;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    ret = tsdrv_cdqm_exist_cdq_para_check(devid, tsid, cdq_id, pid);
    if (ret != 0) {
        return ret;
    }

    cdq_dev = tsdrv_cdqm_get_cdq_dev(devid, tsid);
    cdq_manage = &cdq_dev->cdq_manage[cdq_id];

    if (cdq_manage->create_side != tsdrv_cdqm_get_local_side()) {
        TSDRV_PRINT_ERR("dev %u ts %u cdq %u destroy by wrong side, create_side:%u.\n", devid, tsid, cdq_id,
            (u32)cdq_manage->create_side);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    mutex_lock(&cdq_manage->cdq_mutex);

    wake_up_interruptible(&cdq_manage->wq);
    TSDRV_PRINT_INFO("wake up wq, dev %u ts %u cdq %u.\n", devid, tsid, cdq_id);

    // uninit hardware send message
    ret = tsdrv_cdqm_uninit_cdq_by_side(devid, tsid, para->qid, cdq_dev);
    if (ret != 0) {
        mutex_unlock(&cdq_manage->cdq_mutex);
        return ret;
    }
    cdqm_clear_cdq_info(cdq_manage);

    mutex_unlock(&cdq_manage->cdq_mutex);

    TSDRV_PRINT_INFO("cdqm dev %u ts %u cdq %u destroy success.\n", devid, tsid, para->qid);

    return 0;
}

STATIC int tsdrv_cdqm_drv_query_cdq(u32 devid, u32 tsid, struct tsdrv_cdqm_query_para *para)
{
    int ret;
    struct cdqm_dev_manage *cdq_dev = NULL;

    if (strlen(para->name) > CDQ_NAME_LEN) {
        TSDRV_PRINT_ERR("qid %u name invalid.\n", para->qid);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    ret = cdqm_drv_get_cdq_id(devid, tsid, para->host_pid, para->name, &para->qid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("devid %u ts %u cdq %u name:%s not found:%d.\n", devid, tsid, para->qid, para->name, ret);
        return ret;
    }

    cdq_dev = tsdrv_cdqm_get_cdq_dev(devid, tsid);

    TSDRV_PRINT_INFO("devid %u ts %u cdq %u name:%s .\n", devid, tsid, para->qid, para->name);

    return 0;
}

STATIC int tsdrv_cdqm_drv_alloc_batch(u32 devid, u32 tsid, struct tsdrv_cdqm_alloc_batch_para *para)
{
    int ret;
    int pid = current->tgid;
    u32 cdq_id = para->qid;
    u32 batch_num;
    u32 jf_timeout;
    int side;
    u32 cdq_head;
    struct cdqm_dev_manage *cdq_dev = NULL;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    ret = tsdrv_cdqm_exist_cdq_para_check(devid, tsid, cdq_id, pid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("alloc para check failed,dev %u ts %u cdq %u.\n", devid, tsid, cdq_id);
        return ret;
    }

    cdq_dev = tsdrv_cdqm_get_cdq_dev(devid, tsid);
    cdq_manage = tsdrv_cdqm_get_cdq_info(devid, tsid, cdq_id);

    mutex_lock(&cdq_manage->cdq_mutex);
    side = tsdrv_cdqm_get_local_side();
    if (cdq_manage->create_side != side) {
        TSDRV_PRINT_ERR("cdq %u created by %u, now side %d.\n", cdq_id, (u32)cdq_manage->create_side, side);
        goto err;
    }

    if (cdq_manage->time_out_flag) {
        TSDRV_PRINT_ERR("cdq %u has timeout batch.\n", cdq_id);
        ret = (int)DRV_ERROR_CDQ_ABNORMAL;
        goto err;
    }

    tsdrv_cdqm_get_cdq_rdy_idx(cdq_dev->io_base, cdq_id, cdq_manage->config_info.batch_num,
        &cdq_manage->use_batch_head);

    batch_num = cdq_manage->config_info.batch_num;
    jf_timeout = msecs_to_jiffies((u32)para->timeout);

    if (cdqm_judge_cdq_full(cdq_manage)) {
        cdq_head = cdq_manage->use_batch_head;
        if (para->timeout == 0) {
            TSDRV_PRINT_WARN("devid %u cdq %u full.\n", devid, cdq_id);
            ret = (int)DRV_ERROR_NO_CDQ_RESOURCES;
            goto err;
        }
        tsdrv_cdqm_set_batch_get_notify(cdq_dev->io_base, cdq_id, 1);

        mutex_unlock(&cdq_manage->cdq_mutex);

        ret = wait_event_interruptible_timeout(cdq_manage->wq,
            ((cdq_manage->valid == CDQ_INVALID) || (cdq_manage->use_batch_head != cdq_head)), jf_timeout);

        if (cdq_manage->valid == CDQ_INVALID) {
            ret = (int)DRV_ERROR_CDQ_QUIT;
            TSDRV_PRINT_INFO("dev %u ts %u cdq %u alloc batch destroyed.\n", devid, tsid, cdq_id);
            return (int)DRV_ERROR_CDQ_QUIT;
        }

        mutex_lock(&cdq_manage->cdq_mutex);

        if (ret == 0) {
            mutex_unlock(&cdq_manage->cdq_mutex);
            TSDRV_PRINT_INFO("dev %u ts %u cdq %u alloc batch timeout.\n", devid, tsid, cdq_id);
            return (int)DRV_ERROR_WAIT_TIMEOUT;
        } else if (ret == -ERESTARTSYS) {
            mutex_unlock(&cdq_manage->cdq_mutex);
            TSDRV_PRINT_INFO("dev %u ts %u cdq %u alloc batch interrupted.\n", devid, tsid, cdq_id);
            return (int)DRV_ERROR_WAIT_INTERRUPT;
        }
    }

    para->batch_id = cdq_manage->use_batch_tail;
    cdq_manage->use_batch_tail = (cdq_manage->use_batch_tail + 1) % cdq_manage->config_info.batch_num;

    mutex_unlock(&cdq_manage->cdq_mutex);

    TSDRV_PRINT_INFO("cdqm alloc batch success. dev:%u ts:%u cdq:%u batch id:%u.head:%u tail:%u\n", devid, tsid,
        cdq_id, para->batch_id, cdq_manage->use_batch_head, cdq_manage->use_batch_tail);

    return 0;

err:
    mutex_unlock(&cdq_manage->cdq_mutex);
    return ret;
}

STATIC int tsdrv_cdqm_drv_get_ready_batch(u32 devid, u32 tsid, struct tsdrv_cdqm_get_ready_batch_para *para)
{
    u32 cdq_id = para->qid;
    u32 rdy_num;
    u32 rdy_idx;
    struct cdqm_dev_manage *cdq_dev = NULL;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    if (tsdrv_cdqm_cdq_id_para_check(devid, tsid, cdq_id) != 0) {
        TSDRV_PRINT_ERR("devid %u cdq id %u not valid.\n", devid, para->qid);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    cdq_dev = tsdrv_cdqm_get_cdq_dev(devid, tsid);

    tsdrv_cdqm_get_cdq_rdy_num(cdq_dev->io_base, cdq_id, &rdy_num);

    if (rdy_num == 0) {
        TSDRV_PRINT_INFO("devid %u  cdq %u not ready.\n", devid, cdq_id);
        tsdrv_cdqm_set_batch_rdy_notify(cdq_dev->io_base, cdq_id);
        tsdrv_cdqm_get_cdq_rdy_num(cdq_dev->io_base, cdq_id, &rdy_num);
        if (rdy_num == 0) { /* read rdy_num again to make sure no batch ready during set rdy_notify register */
            return (int)DRV_ERROR_NO_CDQ_RESOURCES;
        }
    }

    cdq_manage = &cdq_dev->cdq_manage[cdq_id];

    tsdrv_cdqm_get_cdq_rdy_idx(cdq_dev->io_base, cdq_id, cdq_manage->config_info.batch_num, &rdy_idx);

    if (rdy_idx == cdq_manage->last_get_batch) {
        TSDRV_PRINT_ERR("devid %u cdq %u batchId(%u) repeat get.\n", devid, cdq_id, cdq_manage->last_get_batch);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    if (rdy_idx != (cdq_manage->last_get_batch + 1) % cdq_manage->config_info.batch_num) {
        TSDRV_PRINT_ERR("devid %u cdq %u rdy_idx %u last_get_batch(%u).\n", devid, cdq_id, rdy_idx,
            cdq_manage->last_get_batch);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    para->batch_size = cdq_manage->config_info.batch_size;
    para->batch_addr = cdq_manage->config_info.mem_addr + para->batch_size * rdy_idx * CDQE_SIZE;

    cdq_manage->last_get_batch = rdy_idx;

    TSDRV_PRINT_INFO("get devid %u cdq %u batchId(%u) addr(%pK).\n", devid, cdq_id, rdy_idx, (void *)para->batch_addr);

    return 0;
}

STATIC int tsdrv_cdqm_drv_free_batch(u32 devid, u32 tsid, struct tsdrv_cdqm_free_batch_para *para)
{
    int ret;
    u32 cdq_id = para->qid;
    struct cdqm_dev_manage *cdq_dev = NULL;

    ret = tsdrv_cdqm_cdq_id_para_check(devid, tsid, cdq_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("para check failed,devid %u tsid %u qid %u.\n", devid, tsid, cdq_id);
        return (int)DRV_ERROR_PARA_ERROR;
    }
    cdq_dev = tsdrv_cdqm_get_cdq_dev(devid, tsid);

    tsdrv_cdqm_set_cdq_batch_get(cdq_dev->io_base, para->qid, 1);

    TSDRV_PRINT_INFO("dev %u ts %u cdq %u free batch ok.\n", devid, tsid, para->qid);

    return 0;
}

STATIC int tsdrv_cdqm_drv_batch_abnormal(u32 devid, u32 tsid, struct tsdrv_cdqm_batch_abnormal *para)
{
    int ret;
    u32 cdq_id = para->qid;
    struct cdqm_cdq_manage *cdq_info = NULL;

    ret = tsdrv_cdqm_cdq_id_para_check(devid, tsid, cdq_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("timeout para check failed.\n");
        return ret;
    }

    cdq_info = tsdrv_cdqm_get_cdq_info(devid, tsid, cdq_id);
    cdq_info->time_out_flag = ENABLE_TIMEOUT_ERR;

    ret = tsdrv_msg_sync_cdqm_batch_abnormal(devid, tsid, cdq_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("other side set dev %u ts %u cdq %u timeout failed:%d.\n", devid, tsid, cdq_id, ret);
        return ret;
    }
    return 0;
}

int tsdrv_ioctl_cdqm_handlers(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg)
{
    int ret;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);

    switch (arg->cdqm_para.type) {
        case CDQM_CREATE_CDQ:
            ret = tsdrv_cdqm_drv_create_cdq(devid, arg->tsid, &arg->cdqm_para.create_para);
            break;
        case CDQM_DESTROY_CDQ:
            ret = tsdrv_cdqm_drv_destroy_cdq(devid, arg->tsid, &arg->cdqm_para.destroy_para);
            break;
        case CDQM_QUERY_CDQ:
            ret = tsdrv_cdqm_drv_query_cdq(devid, arg->tsid, &arg->cdqm_para.query_para);
            break;
        case CDQM_ALLOC_BATCH:
            ret = tsdrv_cdqm_drv_alloc_batch(devid, arg->tsid, &arg->cdqm_para.alloc_para);
            break;
        case CDQM_GET_READY_BATCH:
            ret = tsdrv_cdqm_drv_get_ready_batch(devid, arg->tsid, &arg->cdqm_para.rdy_para);
            break;
        case CDQM_FREE_BATCH:
            ret = tsdrv_cdqm_drv_free_batch(devid, arg->tsid, &arg->cdqm_para.free_para);
            break;
        case CDQM_BATCH_ABNORMAL:
            ret = tsdrv_cdqm_drv_batch_abnormal(devid, arg->tsid, &arg->cdqm_para.timeout_para);
            break;
        default:
            TSDRV_PRINT_ERR("ioctl handel out of rage, type:%d", (int)arg->cdqm_para.type);
            ret = (int)DRV_ERROR_INVALID_HANDLE;
    }

    return ret;
}

int tsdrv_cdqm_query_que_num(u32 devid, u32 tsid)
{
    struct cdqm_cdq_manage *cdq_manage = NULL;
    struct cdqm_dev_manage *cdq_dev = NULL;
    int available_num = 0;
    int i;

    cdq_dev = tsdrv_cdqm_get_cdq_dev(devid, tsid);
    if (cdq_dev == NULL) {
        TSDRV_PRINT_WARN("dev %u ts %u release manage info NULL.\n", devid, tsid);
        return 0;
    }
    cdq_manage = cdq_dev->cdq_manage;

    mutex_lock(&cdq_dev->dev_mutex);
    for (i = 0; i < MAX_CDQM_CDQ_NUM; i++) {
        if (cdq_manage[i].valid != CDQ_VALID) {
            available_num++;
        }
    }
    mutex_unlock(&cdq_dev->dev_mutex);

    return available_num;
}

int tsdrv_cdqm_set_topic_id(u32 devid, u32 topic_id)
{
    struct cdqm_dev_manage *cdq_dev = NULL;
    u32 tsid;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        TSDRV_PRINT_ERR("dev %u invalid.\n", devid);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    for (tsid = 0; tsid < DEVDRV_MAX_TS_NUM; tsid++) {
        cdq_dev = tsdrv_cdqm_get_cdq_dev(devid, tsid);
        if (cdq_dev != NULL) {
            tsdrv_cdqm_write_topic_id(cdq_dev->io_base, topic_id);
        }
    }

    return 0;
}
TSDRV_EXPORT_SYMBOL(tsdrv_cdqm_set_topic_id);

u32 tsdrv_cdqm_get_instance_by_cdqid(u32 devid, u32 tsid, u32 cdq_id)
{
    return CDQ_INSTANCE(devid, tsid, cdq_id);
}
TSDRV_EXPORT_SYMBOL(tsdrv_cdqm_get_instance_by_cdqid);

int tsdrv_cdqm_get_name_by_cdqid(u32 devid, u32 tsid, u32 cdq_id, char *name, int buf_len)
{
    int ret;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    if (devid > DEVDRV_MAX_DAVINCI_NUM || tsid > DEVDRV_MAX_TS_NUM || tsdrv_cdqm_get_cdq_dev(devid, tsid) == NULL ||
        cdq_id >= MAX_CDQM_CDQ_NUM) {
        TSDRV_PRINT_ERR("dev %u ts %u cdqid %u invalid.\n", devid, tsid, cdq_id);
        return 0;
    }
    if (name == NULL || buf_len < CDQ_NAME_LEN) {
        TSDRV_PRINT_ERR("name ptr invalid.\n");
        return 0;
    }

    cdq_manage = tsdrv_cdqm_get_cdq_info(devid, tsid, cdq_id);

    ret = strcpy_s(name, (size_t)buf_len, cdq_manage->config_info.name);
    if (ret < 0) {
        TSDRV_PRINT_ERR("dev %u ts %u cdq %u strcpy error:%d", devid, tsid, cdq_id, ret);
        return ret;
    }

    return ret;
}
TSDRV_EXPORT_SYMBOL(tsdrv_cdqm_get_name_by_cdqid);

int tsdrv_cdqid_is_belong_to_proc(struct tsdrv_id_inst *id_inst, pid_t tgid, u32 id)
{
    struct cdqm_cdq_manage *cdq_manage = NULL;

    if (id_inst == NULL) {
        TSDRV_PRINT_ERR("id_inst is null.\n");
        return (int)DRV_ERROR_PARA_ERROR;
    }

    if (id_inst->devid >= TSDRV_MAX_DAVINCI_NUM || id >= MAX_CDQM_CDQ_NUM ||
        id_inst->fid != TSDRV_PM_FID || id_inst->tsid >= tsdrv_get_dev_tsnum(id_inst->devid)) {
        TSDRV_PRINT_ERR("Invalid id_inst. (devid=%u; cdq_id=%u; fid=%u; tsid=%u)\n",
            id_inst->devid, id, id_inst->fid, id_inst->tsid);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    cdq_manage = tsdrv_cdqm_get_cdq_info(id_inst->devid, id_inst->tsid, id);
    mutex_lock(&cdq_manage->cdq_mutex);
    if (cdq_manage->valid != CDQ_VALID) {
        mutex_unlock(&cdq_manage->cdq_mutex);
        TSDRV_PRINT_ERR("devid %u tsid %u cdqId %u\n",
            id_inst->devid, id_inst->tsid, id);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    if (cdq_manage->config_info.pid != tgid) {
        mutex_unlock(&cdq_manage->cdq_mutex);
        TSDRV_PRINT_ERR("devid %u tsid %u cdqId %u pid %d error.\n",
            id_inst->devid, id_inst->tsid, id, tgid);
        return (int)DRV_ERROR_PARA_ERROR;
    }
    mutex_unlock(&cdq_manage->cdq_mutex);

    return 0;
}
EXPORT_SYMBOL(tsdrv_cdqid_is_belong_to_proc);

int tsdrv_cdqm_recycle_cdq(u32 devid, u32 tsid, int pid)
{
    struct cdqm_dev_manage *cdq_dev = NULL;
    struct cdqm_cdq_manage *cdq_manage = NULL;
    struct tsdrv_cdqm_destroy_para destroy_para;
    int i;
    int ret;

    cdq_dev = tsdrv_cdqm_get_cdq_dev(devid, tsid);
    if (cdq_dev == NULL) {
        TSDRV_PRINT_WARN("dev %u ts %u release manage info NULL.\n", devid, tsid);
        return 0;
    }

    mutex_lock(&cdq_dev->dev_mutex);

    cdq_manage = cdq_dev->cdq_manage;

    for (i = 0; i < MAX_CDQM_CDQ_NUM; i++) {
        if (cdq_manage[i].config_info.pid == pid && cdq_manage[i].valid == CDQ_VALID) {
            destroy_para.qid = (u32)i;
            destroy_para.ts_id = tsid;
            cdq_manage[i].recycle_pid = (u32)pid;

            ret = tsdrv_cdqm_drv_destroy_cdq(devid, tsid, &destroy_para);
            if (ret != 0) {
                TSDRV_PRINT_WARN("cdqm dev %u ts %u cdq %d destroy by release failed:%d.\n", devid, tsid, i, ret);
            } else {
                TSDRV_PRINT_INFO("cdqm dev %u ts %u cdq %d destroy by release.\n", devid, tsid, i);
            }
        }
    }

    mutex_unlock(&cdq_dev->dev_mutex);
    TSDRV_PRINT_INFO("cdqm dev %u ts %u recycle cdq finished.\n", devid, tsid);

    return 0;
}

STATIC void tsdrv_cdqm_f2nf_task(unsigned long data)
{
    struct cdqm_dev_manage *cdq_dev = (struct cdqm_dev_manage *)data;
    struct cdqm_cdq_manage *cdq_manage = NULL;
    u32 sts2_l2;
    u32 cdq_ps, qid;
    u32 cdq_s = 0;

    // 轮询所有CDQ资源组的STS2_L1寄存器
    for (cdq_s = 0; cdq_s < MAX_CDQ_SLICE; cdq_s++) {
        tsdrv_cdqm_read_f2nf_int_sts2_l2(cdq_dev->io_base, cdq_s, &sts2_l2);
        if (sts2_l2 == 0) {
            continue;
        }

        tsdrv_cdqm_f2nf_int_clr2(cdq_dev->io_base, cdq_s, sts2_l2);
        TSDRV_PRINT_DEBUG("slice_id %u sts2_l2 0x%x\n", cdq_s, sts2_l2);

        for (cdq_ps = 0; cdq_ps < SLICE_CDQ_NUM; cdq_ps++) {
            if ((sts2_l2 & (1U << cdq_ps)) == 0) {
                continue;
            }
            // wakeup对应队列的wait_event
            qid = cdq_ps + cdq_s * SLICE_CDQ_NUM;
            cdq_manage = &cdq_dev->cdq_manage[qid];

            tsdrv_cdqm_get_cdq_rdy_idx(cdq_dev->io_base, qid, cdq_manage->config_info.batch_num,
                &cdq_manage->use_batch_head);

            wake_up_interruptible(&cdq_manage->wq);

            TSDRV_PRINT_DEBUG("dev %u ts %u cdq %u waked\n", cdq_dev->devid, cdq_dev->tsid, qid);
        }
        tsdrv_cdqm_set_int_f2nf_clr(cdq_dev->io_base, cdq_s);
    }
}

STATIC irqreturn_t tsdrv_cdqm_f2nf_handle(int irq, void *data)
{
    struct cdqm_dev_manage *cdq_dev = (struct cdqm_dev_manage *)data;

    tasklet_schedule(&cdq_dev->f2nf_task);

    return IRQ_HANDLED;
}

STATIC int tsdrv_cdqm_drv_dev_init(u32 devid, u32 ts_id)
{
    int ret;
    struct cdqm_dev_manage *cdq_dev = NULL;
    int i;
    u64 base_addr;
    size_t reg_size;
    u32 chip_type;

    cdq_dev = kzalloc(sizeof(struct cdqm_dev_manage), GFP_KERNEL);
    if (cdq_dev == NULL) {
        TSDRV_PRINT_ERR("devid(%u) tsid(%u)kamlloc cdq_dev failed.\n", devid, ts_id);
        return (int)DRV_ERROR_INNER_ERR;
    }

    cdq_dev->devid = devid;
    cdq_dev->tsid = ts_id;

    mutex_init(&cdq_dev->dev_mutex);

    for (i = 0; i < MAX_CDQM_CDQ_NUM; i++) {
        mutex_init(&cdq_dev->cdq_manage[i].cdq_mutex);
        init_waitqueue_head(&cdq_dev->cdq_manage[i].wq);
    }

    ret = tsdrv_cdqm_get_cdqm_reg(devid, &base_addr, &reg_size);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%u) ts(%u) cdqm get addr info failed(%d).\n", devid, ts_id, ret);
        goto out;
    }

    cdq_dev->io_base = ioremap(base_addr, reg_size);
    if (cdq_dev->io_base == NULL) {
        TSDRV_PRINT_ERR("dev(%u) ts(%u) cdqm ioremap failed(%d).\n", devid, ts_id, ret);
        ret = (int)DRV_ERROR_INNER_ERR;
        goto out;
    }
#ifndef AOS_LLVM_BUILD
    chip_type = devdrv_get_dev_chip_type(devid);
#else
    chip_type = HISI_MINI_V2;
#endif
    if (chip_type == HISI_MINI_V3 && tsdrv_get_env_type() == TSDRV_ENV_ONLINE) {
        cdq_dev->int_io_base = 0;
    } else {
        ret = tsdrv_cdqm_get_intr_reg(devid, &base_addr, &reg_size);
        if (ret != 0) {
            TSDRV_PRINT_ERR("dev(%u) ts(%u) cdqm get irq reg failed(%d).\n", devid, ts_id, ret);
            goto out;
        }

        cdq_dev->int_io_base = ioremap(base_addr, reg_size);
        if (cdq_dev->int_io_base == NULL) {
            TSDRV_PRINT_ERR("dev(%u) ts(%u) cdqm ioremap failed(%d).\n", devid, ts_id, ret);
            ret = (int)DRV_ERROR_INNER_ERR;
            goto out;
        }
    }

    ret = tsdrv_cdqm_get_f2nf_irq(devid, &cdq_dev->f2nf_vector_id, &cdq_dev->f2nf_irq);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%u) ts(%u) cdqm get irq failed(%d).\n", devid, ts_id, ret);
        goto out;
    }

    tasklet_init(&cdq_dev->f2nf_task, tsdrv_cdqm_f2nf_task, (unsigned long)(uintptr_t)cdq_dev);
    ret = request_irq(cdq_dev->f2nf_irq, tsdrv_cdqm_f2nf_handle, 0, "cdqm_f2nf", (void *)cdq_dev);
    if (ret != 0) {
        tasklet_kill(&cdq_dev->f2nf_task);
        TSDRV_PRINT_ERR("request irq fail:%d\n", ret);
        ret = (int)DRV_ERROR_INNER_ERR;
        goto out;
    }

    (void)irq_set_affinity_hint(cdq_dev->f2nf_irq, NULL);

    ret = tsdrv_cdqm_drv_manage_adapt_init(cdq_dev);
    if (ret != 0) {
        tasklet_kill(&cdq_dev->f2nf_task);
        (void)free_irq(cdq_dev->f2nf_irq, cdq_dev);
        goto out;
    }

    g_cdq_dev_manage[devid][ts_id] = cdq_dev;

    TSDRV_PRINT_INFO("Init finish. (dev=%u; tsid=%u; f2nf_vector_id=%u; irq=%u; io_base=0x%pK; int_io_base=0x%pK)\n",
        devid, ts_id, cdq_dev->f2nf_vector_id, cdq_dev->f2nf_irq, (void *)(uintptr_t)cdq_dev->io_base,
        (void *)(uintptr_t)cdq_dev->int_io_base);

    return 0;

out:

    if (cdq_dev->io_base != NULL) {
        iounmap(cdq_dev->io_base);
    }

    if (cdq_dev->int_io_base != NULL) {
        iounmap(cdq_dev->int_io_base);
    }

    kfree(cdq_dev);

    return ret;
}
int tsdrv_cdqm_drv_init(u32 devid, u32 tsnum)
{
    int err;
    u32 tsid;

    for (tsid = 0; tsid < tsnum; tsid++) {
        err = tsdrv_cdqm_drv_dev_init(devid, tsid);
        if (err != 0) {
            TSDRV_PRINT_ERR("cdqm init failed\n");
            return err;
        }
    }
    return 0;
}

void tsdrv_cdqm_drv_uninit(u32 devid, u32 tsnum)
{
    struct cdqm_dev_manage *cdq_dev = NULL;
    u32 ts_id;

    for (ts_id = 0; ts_id < tsnum; ts_id++) {
        cdq_dev = g_cdq_dev_manage[devid][ts_id];

        tasklet_kill(&cdq_dev->f2nf_task);
        (void)free_irq(cdq_dev->f2nf_irq, cdq_dev);
        iounmap(cdq_dev->io_base);
        iounmap(cdq_dev->int_io_base);
        kfree(cdq_dev);
        g_cdq_dev_manage[devid][ts_id] = NULL;
    }
}
#else /* !CFG_FEATURE_CDQM */
u32 tsdrv_cdqm_get_instance_by_cdqid(u32 devid, u32 tsid, u32 cdq_id)
{
    TSDRV_PRINT_WARN("CDQM is not supported. (devid=%u; tsid=%u; cdq_id=%u)\n", devid, tsid, cdq_id);
    return 0;
}
TSDRV_EXPORT_SYMBOL(tsdrv_cdqm_get_instance_by_cdqid);

int tsdrv_cdqm_get_name_by_cdqid(u32 devid, u32 tsid, u32 cdq_id, char *name, int buf_len)
{
    TSDRV_PRINT_WARN("CDQM is not supported. (devid=%u; tsid=%u; cdq_id=%u)\n", devid, tsid, cdq_id);
    return -ENODEV;
}
TSDRV_EXPORT_SYMBOL(tsdrv_cdqm_get_name_by_cdqid);

int tsdrv_cdqm_set_topic_id(u32 devid, u32 topic_id)
{
    TSDRV_PRINT_WARN("CDQM is not supported. (devid=%u; topic_id=%u)\n", devid, topic_id);
    return -ENODEV;
}
TSDRV_EXPORT_SYMBOL(tsdrv_cdqm_set_topic_id);
#endif /* CFG_FEATURE_CDQM */
