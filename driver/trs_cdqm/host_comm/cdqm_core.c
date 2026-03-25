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
* Create: 2022-7-9
*/
#include <linux/slab.h>
#include <linux/io.h>

#include "securec.h"
#include "ascend_kernel_hal.h"
#include "trs_pub_def.h"
#include "devdrv_interface.h"
#include "soc_res.h"

#include "trs_cdqm.h"
#include "cdq_manager.h"
#include "cdqm_ioctl.h"
#include "cdqm_fops.h"
#include "cdqm_core.h"

#define DEVDRV_MAX_DAVINCI_NUM  64
#define DEVDRV_MAX_TS_NUM   1

struct cdqm_dev_manage *g_cdq_dev_manage[DEVDRV_MAX_DAVINCI_NUM][DEVDRV_MAX_TS_NUM] = {NULL, };

static struct cdqm_cdq_manage *cdqm_get_cdq_mng(u32 devid, u32 tsid, u32 cdq_id)
{
    return &g_cdq_dev_manage[devid][tsid]->cdq_manage[cdq_id];
}

struct cdqm_dev_manage *cdqm_get_cdq_dev(u32 devid, u32 tsid)
{
    if (g_cdq_dev_manage[devid][tsid] == NULL) {
        trs_warn("cdqm dev %u ts %u is NULL.\n", devid, tsid);
        return NULL;
    }
    return g_cdq_dev_manage[devid][tsid];
}

#define CDQM_F2NF_MAX_NS_NUM 16
static void cdqm_f2nf_set(u32 devid, u32 tsid, u32 irq)
{
    struct cdqm_dev_manage *cdq_dev = cdqm_get_cdq_dev(devid, tsid);
    u8 ns_num;

    for (ns_num = 0; ns_num < CDQM_F2NF_MAX_NS_NUM; ns_num++) {
        tsdrv_cdqm_set_int_irq(cdq_dev->int_io_base, irq, ns_num);
    }
}

/* 采用name+pid或者name来查询cdq_id，如果没有pid，采用INVALID_PID */
static int cdqm_get_cdq_id(u32 devid, u32 tsid, int pid, char *name, u32 *cdq_id)
{
    struct cdqm_dev_manage *cdq_dev = cdqm_get_cdq_dev(devid, tsid);
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

inline void cdqm_clear_cdq_mng(struct cdqm_cdq_manage *cdq_manage)
{
    cdq_manage->valid = 0;
    cdq_manage->create_side = 0;
    cdq_manage->time_out_flag = 0;
    cdq_manage->config_info.pid = 0;
}

static inline int cdqm_judge_cdq_full(struct cdqm_cdq_manage *cdq_manage)
{
    return (cdq_manage->use_batch_head == (cdq_manage->use_batch_tail + 1) % cdq_manage->config_info.batch_num) ? 1 : 0;
}

static int cdqm_create_para_check(u32 devid, u32 tsid, struct tsdrv_cdqm_create_para *para, int pid)
{
    u32 cdq_id;
    u32 batch_num = para->batch_num;
    u32 batch_size = para->batch_size;
    size_t name_len;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM || tsid >= DEVDRV_MAX_TS_NUM || para->mem_addr == 0) {
        trs_err("devid(%u) tsid(%u) mem_addr(%pK) failed.\n", devid, tsid, (void *)para->mem_addr);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    if (batch_num > MAX_CDQ_DEPTH || batch_size > MAX_CDQ_DEPTH || batch_num <= 1 || batch_size == 0 ||
        batch_num * batch_size > MAX_CDQ_DEPTH) {
        trs_err("batch_size(%u) batch_num(%u) error.\n", batch_size, batch_num);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    name_len = strnlen(para->name, CDQ_NAME_LEN);
    if ((name_len == 0) || (name_len >= CDQ_NAME_LEN)) {
        trs_err("Length out of range. (name_len=%lu)\n", name_len);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    if (cdqm_get_cdq_id(devid, tsid, pid, para->name, &cdq_id) == 0) {
        trs_err("pid(%d) name(%s) repeated.\n", pid, para->name);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    return (int)DRV_ERROR_NONE;
}

static int cdqm_update_ssid(struct cdqm_dev_manage *cdq_dev, struct tsdrv_cdqm_create_para *para)
{
    struct trs_id_inst inst = {.devid = cdq_dev->devid, .tsid = cdq_dev->tsid};
    int proc_ssid, ret;

    ret = cdq_dev->ops.ssid_query(&inst, &proc_ssid);
    if ((ret != 0) || (proc_ssid < 0) || (para->ssid != 0)) {
        trs_err("Invalid para. (devid=%u; tsid=%u; proc_ssid=%d; ssid=%d)\n",
            inst.devid, inst.tsid, proc_ssid, para->ssid);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    para->ssid = proc_ssid;

    return (int)DRV_ERROR_NONE;
}

static int cdqm_cdq_id_para_check(u32 devid, u32 tsid, u32 cdq_id)
{
    struct cdqm_cdq_manage *cdq_manage = NULL;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM || tsid >= DEVDRV_MAX_TS_NUM || cdq_id >= MAX_CDQM_CDQ_NUM) {
        trs_err("devid(%u) tsid(%u) cdqid(%u) failed.\n", devid, tsid, cdq_id);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    cdq_manage = cdqm_get_cdq_mng(devid, tsid, cdq_id);
    if (cdq_manage->valid != CDQ_VALID) {
        trs_err("devid %u tsid %u cdqId %u valid %u\n", devid, tsid, cdq_id, (u32)cdq_manage->valid);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    return 0;
}

static int cdqm_exist_cdq_para_check(u32 devid, u32 tsid, u32 cdq_id, int pid)
{
    int ret;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    ret = cdqm_cdq_id_para_check(devid, tsid, cdq_id);
    if (ret != 0) {
        return ret;
    }

    cdq_manage = cdqm_get_cdq_mng(devid, tsid, cdq_id);
    if (cdq_manage->recycle_pid != 0) {
        return 0;
    }

    if (cdq_manage->config_info.pid != pid) {
        trs_err("devid %u tsid %u cdqId %u valid %u manage->pid %d current pid %d error.\n",
            devid, tsid, cdq_id, (u32)cdq_manage->valid, cdq_manage->config_info.pid, pid);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    return 0;
}


/* 接收消息处理函数 */
static int cdqm_create_msg_handle(u32 devid, u32 tsid, struct cdq_cfg_info *cdq_info)
{
#ifdef EMU_ST
    int i;
#endif
    int ret;
    struct tsdrv_cdqm_create_para para;
    struct cdqm_dev_manage *cdq_dev = NULL;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    para.mem_addr = cdq_info->mem_addr;
    para.batch_num = cdq_info->batch_num;
    para.batch_size = cdq_info->batch_size;
#ifndef EMU_ST
    ret = strcpy_s(para.name, sizeof(para.name), cdq_info->name);
    if (ret != 0) {
        trs_err("strcpy_s err, devid = %u, tsid = %u, ret = %d", devid, tsid, ret);
        return -EINVAL;
    }
#else
    for (i = 0; i < CDQ_NAME_LEN; i++) {
        para.name[i] = cdq_info->name[i];
    }
#endif
    ret = cdqm_create_para_check(devid, tsid, &para, cdq_info->pid);
    if (ret != 0) {
        trs_err("para check failed.\n");
        return ret;
    }

    cdq_dev = cdqm_get_cdq_dev(devid, tsid);
    ret = tsdrv_cdqm_create_msg_handle_by_side(cdq_dev, cdq_info);
    if (ret != 0) {
        return ret;
    }

    cdq_manage = cdqm_get_cdq_mng(devid, tsid, cdq_info->cdq_id);

    cdq_manage->config_info = *cdq_info;
    cdq_manage->use_batch_head = 0;
    cdq_manage->use_batch_tail = 0;
    cdq_manage->time_out_flag = 0;
    cdq_manage->recycle_pid = 0;
    cdq_manage->last_get_batch = cdq_info->batch_num - 1;

    trs_info("cdqm dev %u ts %u cdq %u addr(%pK) ssid %u batch_num %u batch_size %u side %u "
        "create handle ok.\n", devid, tsid, cdq_info->cdq_id, (void *)cdq_info->mem_addr,
        cdq_info->ssid, cdq_info->batch_num, cdq_info->batch_size, cdq_manage->create_side);

    return 0;
}

static int cdqm_destroy_msg_handle(u32 devid, u32 tsid, struct cdqm_destroy_msg *msg)
{
    int ret;
    struct cdqm_dev_manage *cdq_dev = NULL;

    ret = cdqm_exist_cdq_para_check(devid, tsid, msg->cdq_id, msg->pid);
    if (ret != 0) {
        trs_err("destroy msg para check fail.\n");
        return ret;
    }

    cdq_dev = cdqm_get_cdq_dev(devid, tsid);

    return tsdrv_cdqm_destroy_handle_by_side(msg->cdq_id, cdq_dev);
}

static int cdqm_batch_abnormal_msg_handle(u32 devid, u32 tsid, u32 cdq_id)
{
    int ret;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    ret = cdqm_cdq_id_para_check(devid, tsid, cdq_id);
    if (ret != 0) {
        trs_err("msg para check failed:%d.\n", ret);
        return ret;
    }

    cdq_manage = cdqm_get_cdq_mng(devid, tsid, cdq_id);

    cdq_manage->time_out_flag = 1;

    wake_up_interruptible(&cdq_manage->wq);

    return 0;
}

int cdqm_sync_msg_init(struct cdqm_dev_manage *cdq_dev)
{
    struct trs_msg_data msg_head = { 0 };
    struct tsdrv_cdqm_init_msg *irq_msg = (struct tsdrv_cdqm_init_msg *)msg_head.payload;
    int ret;

    msg_head.header.cmdtype = TRS_MSG_CDQM_INIT;
    msg_head.header.valid = TRS_MSG_SEND_MAGIC;
    msg_head.header.result = TRS_MSG_INVALID_RESULT;
    msg_head.header.tsid = cdq_dev->tsid;

    irq_msg->f2nf_irq = (u32)cdq_dev->f2nf_hwirq;

    ret = cdq_dev->ops.send_sync_msg(cdq_dev->devid, (void *)&msg_head, sizeof(struct trs_msg_data));
    if ((ret != 0) || (msg_head.header.result != 0) || (msg_head.header.valid != TRS_MSG_RCV_MAGIC)) {
        trs_err("device(%u) msg chan send failed, ret = %d, result = %u, valid = 0x%x.\n",
            cdq_dev->devid, ret, (u32)msg_head.header.result, (u32)msg_head.header.valid);
        return -EFAULT;
    }
    return 0;
}

int cdqm_sync_msg_create(u32 devid, u32 tsid, struct cdq_cfg_info *para)
{
    struct trs_msg_data msg_head;
    struct cdq_cfg_info *msg = (struct cdq_cfg_info *)msg_head.payload;
    struct cdqm_dev_manage *cdq_dev = cdqm_get_cdq_dev(devid, tsid);
    int ret;

    msg_head.header.cmdtype = TRS_MSG_CDQM_CREATE;
    msg_head.header.valid = TRS_MSG_SEND_MAGIC;
    msg_head.header.result = TRS_MSG_INVALID_RESULT;
    msg_head.header.tsid = tsid;

    *msg = *para;

    if (cdq_dev->ops.send_sync_msg != NULL) {
        ret = cdq_dev->ops.send_sync_msg(devid, (void *)&msg_head, sizeof(struct trs_msg_data));
        if ((ret != 0) || (msg_head.header.result != 0) || (msg_head.header.valid != TRS_MSG_RCV_MAGIC)) {
            trs_err("device(%u) msg chan send failed, ret = %d, result = %u, valid = 0x%x.\n",
                devid, ret, (u32)msg_head.header.result, (u32)msg_head.header.valid);
            return -EFAULT;
        }
    }

    para->cdq_id = msg->cdq_id;

    return 0;
}

int cdqm_sync_msg_destroy(u32 devid, u32 tsid, struct cdqm_destroy_msg *para)
{
    int ret;
    struct trs_msg_data msg_head;
    struct cdqm_destroy_msg *msg = (struct cdqm_destroy_msg *)msg_head.payload;
    struct cdqm_dev_manage *cdq_dev = cdqm_get_cdq_dev(devid, tsid);

    msg_head.header.cmdtype = TRS_MSG_CDQM_DESTROY;
    msg_head.header.valid = TRS_MSG_SEND_MAGIC;
    msg_head.header.result = TRS_MSG_INVALID_RESULT;
    msg_head.header.tsid = tsid;

    msg->cdq_id = para->cdq_id;
    msg->pid = para->pid;

    if (cdq_dev->ops.send_sync_msg != NULL) {
        ret = cdq_dev->ops.send_sync_msg(devid, (void *)&msg_head, sizeof(struct trs_msg_data));
        if ((ret != 0) || (msg_head.header.result != 0) || (msg_head.header.valid != TRS_MSG_RCV_MAGIC)) {
            trs_err("device(%u) msg chan send failed, ret = %d, result = %u, valid = 0x%x.\n",
                devid, ret, (u32)msg_head.header.result, (u32)msg_head.header.valid);
            return -EFAULT;
        }
    }

    return 0;
}

static int cdqm_sync_msg_batch_abnormal(u32 devid, u32 tsid, u32 cdq_id)
{
    int ret;
    struct trs_msg_data msg_head;
    struct cdqm_batch_abnormal_msg *msg = (struct cdqm_batch_abnormal_msg *)msg_head.payload;
    struct cdqm_dev_manage *cdq_dev = cdqm_get_cdq_dev(devid, tsid);

    msg_head.header.cmdtype = TRS_MSG_CDQM_BATCH_ABNORMAL;
    msg_head.header.valid = TRS_MSG_SEND_MAGIC;
    msg_head.header.result = TRS_MSG_INVALID_RESULT;
    msg_head.header.tsid = tsid;

    msg->cdq_id = cdq_id;
    msg->devid = devid;

    if (cdq_dev->ops.send_sync_msg != NULL) {
        ret = cdq_dev->ops.send_sync_msg(devid, (void *)&msg_head, sizeof(struct trs_msg_data));
        if ((ret != 0) || (msg_head.header.result != 0) || (msg_head.header.valid != TRS_MSG_RCV_MAGIC)) {
            trs_err("device(%u) msg chan send failed, ret = %d, result = %u, valid = 0x%x.\n",
                devid, ret, (u32)msg_head.header.result, (u32)msg_head.header.valid);
            return -EFAULT;
        }
    }

    return 0;
}

static int cdqm_proc_sync_msg_init(u32 devid, struct trs_msg_data *msg_head)
{
    struct tsdrv_cdqm_init_msg *msg = NULL;
    u32 tsid;

    msg = (struct tsdrv_cdqm_init_msg *)msg_head->payload;
    tsid = msg_head->header.tsid;
    trs_info("init proc devid %u, ts %u, irq %u.\n", devid, tsid, msg->f2nf_irq);

    if (msg_head->header.valid != TRS_MSG_SEND_MAGIC) {
        trs_err("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
    }
    cdqm_f2nf_set(msg_head->header.devid, msg_head->header.tsid, msg->f2nf_irq);

    msg_head->header.valid = TRS_MSG_RCV_MAGIC;
    msg_head->header.result = 0;

    return 0;
}

static int cdqm_proc_sync_msg_create(u32 devid, struct trs_msg_data *msg_head)
{
    struct cdq_cfg_info *msg = (struct cdq_cfg_info *)msg_head->payload;
    int ret;
    u32 tsid;

    tsid = msg_head->header.tsid;
    if (msg_head->header.valid != TRS_MSG_SEND_MAGIC) {
        trs_err("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
    }

    msg_head->header.valid = TRS_MSG_RCV_MAGIC;

    ret = cdqm_create_msg_handle(devid, msg_head->header.tsid, msg);
    if (ret != 0) {
        trs_err("create cdq failed:%d.\n", ret);
        msg_head->header.result = (u16)ret;
        return ret;
    }
    msg_head->header.result = 0;

    return 0;
}

static int cdqm_proc_sync_msg_destroy(u32 devid, struct trs_msg_data *msg_head)
{
    struct cdqm_destroy_msg *msg = (struct cdqm_destroy_msg *)msg_head->payload;
    int ret;

    if (msg_head->header.valid != TRS_MSG_SEND_MAGIC) {
        trs_err("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
    }

    msg_head->header.valid = TRS_MSG_RCV_MAGIC;

    ret = cdqm_destroy_msg_handle(devid, msg_head->header.tsid, msg);
    if (ret != 0) {
        trs_err("destroy cdq %u failed:%d.\n", msg->cdq_id, ret);
        msg_head->header.result = ret;
        return ret;
    }
    msg_head->header.result = 0;

    trs_info("destroy proc ok, dev%u ts %u cdq %u\n", devid, msg_head->header.tsid, msg->cdq_id);

    return 0;
}

int cdqm_proc_sync_msg_batch_abnormal(u32 devid, struct trs_msg_data *msg_head)
{
    struct cdqm_batch_abnormal_msg *msg = (struct cdqm_batch_abnormal_msg *)msg_head->payload;
    int ret;

    if (msg_head->header.valid != TRS_MSG_SEND_MAGIC) {
        trs_err("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
    }

    msg_head->header.valid = TRS_MSG_RCV_MAGIC;

    ret = cdqm_batch_abnormal_msg_handle(devid, msg_head->header.tsid, msg->cdq_id);
    if (ret != 0) {
        trs_err("set timeout cdq %u failed:%d.\n", msg->cdq_id, ret);
        msg_head->header.result = ret;
        return ret;
    }
    msg_head->header.result = 0;

    return 0;
}

int (*const cdqm_sync_msg_handlers[TRS_MSG_MAX])(u32 devid, struct trs_msg_data *msg) = {
    [TRS_MSG_CDQM_INIT] = cdqm_proc_sync_msg_init,
    [TRS_MSG_CDQM_CREATE] = cdqm_proc_sync_msg_create,
    [TRS_MSG_CDQM_DESTROY] = cdqm_proc_sync_msg_destroy,
    [TRS_MSG_CDQM_BATCH_ABNORMAL] = cdqm_proc_sync_msg_batch_abnormal,
};

int cdqm_proc_sync_msg(u32 devid, struct trs_msg_data *msg)
{
    struct trs_msg_data *trs_msg_data = NULL;
    u32 cmdtype;

    return -EOPNOTSUPP;

    if (msg == NULL) {
        trs_err("msg(%pK) is NULL\n", msg);
        return -EINVAL;
    }

    trs_msg_data = (struct trs_msg_data *)msg;
    cmdtype = trs_msg_data->header.cmdtype;

    if (cmdtype >= TRS_MSG_MAX) {
        trs_err("invalid msg_id=%u, devid=%u\n", cmdtype, devid);
        return -EINVAL;
    }

    return cdqm_sync_msg_handlers[cmdtype](devid, trs_msg_data);
}
EXPORT_SYMBOL(cdqm_proc_sync_msg);

static void cdqm_init_cdq_info(u32 devid, u32 tsid, int pid,
    struct tsdrv_cdqm_create_para *para, struct cdq_cfg_info *cdq_info)
{
    if (strcpy_s(cdq_info->name, CDQ_NAME_LEN, para->name) != EOK) {
        trs_err("Strcpy err. (devid=%u; tsid=%u; name=%s).\n", devid, tsid, para->name);
    }

    cdq_info->name[CDQ_NAME_LEN - 1] = '\0';
    cdq_info->batch_num = para->batch_num;
    cdq_info->batch_size = para->batch_size;
    cdq_info->tsid = tsid;
    cdq_info->mem_addr = para->mem_addr;
    cdq_info->sid = CDQM_STREAM_ID;
    cdq_info->ssid = para->ssid;
    cdq_info->pid = pid;
}

/* 主流程 */
static int cdqm_drv_create_cdq(u32 devid, u32 tsid, int pid, struct tsdrv_cdqm_create_para *para)
{
    struct cdqm_dev_manage *cdq_dev = NULL;
    struct cdqm_cdq_manage *cdq_manage = NULL;
    struct cdq_cfg_info cdq_info;
    int ret;

    ret = cdqm_create_para_check(devid, tsid, para, pid);
    if (ret != 0) {
        return ret;
    }

    cdq_dev = cdqm_get_cdq_dev(devid, tsid);
    ret = cdqm_update_ssid(cdq_dev, para);
    if (ret != 0) {
        return ret;
    }

    cdqm_init_cdq_info(devid, tsid, pid, para, &cdq_info);

    ret = tsdrv_cdqm_create_cdq_id(cdq_dev, &cdq_info);
    if (ret != 0) {
        trs_err("dev %u ts %u name %s alloc cdq id failed:%d.\n", devid, tsid, cdq_info.name, ret);
        return ret;
    }

    cdq_manage = &cdq_dev->cdq_manage[cdq_info.cdq_id];

    mutex_lock(&cdq_manage->cdq_mutex);

    ret = tsdrv_cdqm_sync_cdq_info(cdq_dev, &cdq_info);
    if (ret != 0) {
        tsdrv_cdqm_uninit_cdq_hardware(cdq_dev, cdq_info.cdq_id);
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

    para->qid = cdq_info.cdq_id;

    trs_info("cdqm dev %u ts %u cdq %u addr(%pK) ssid %u batch_num %u batch_size %u pid %d create success.\n",
        devid, tsid, para->qid, (void *)para->mem_addr, para->ssid, para->batch_num, para->batch_size, cdq_info.pid);

    return 0;
}

static int cdqm_drv_destroy_cdq(u32 devid, u32 tsid, int pid, struct tsdrv_cdqm_destroy_para *para)
{
    int ret;
    u32 cdq_id = para->qid;
    struct cdqm_dev_manage *cdq_dev = NULL;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    ret = cdqm_exist_cdq_para_check(devid, tsid, cdq_id, pid);
    if (ret != 0) {
        return ret;
    }

    cdq_dev = cdqm_get_cdq_dev(devid, tsid);
    cdq_manage = &cdq_dev->cdq_manage[cdq_id];

    if (cdq_manage->create_side != tsdrv_cdqm_get_local_side()) {
        trs_err("dev %u ts %u cdq %u destroy by wrong side, create_side:%u.\n", devid, tsid, cdq_id,
            (u32)cdq_manage->create_side);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    mutex_lock(&cdq_manage->cdq_mutex);

    wake_up_interruptible(&cdq_manage->wq);

    // uninit hardware send message
    ret = tsdrv_cdqm_uninit_cdq_by_side(cdq_dev, cdq_id);
    if (ret != 0) {
        mutex_unlock(&cdq_manage->cdq_mutex);
        return ret;
    }
    cdqm_clear_cdq_mng(cdq_manage);

    mutex_unlock(&cdq_manage->cdq_mutex);

    trs_info("cdqm dev %u ts %u cdq %u destroy success.\n", devid, tsid, para->qid);

    return 0;
}

static int cdqm_drv_query_cdq(u32 devid, u32 tsid, struct tsdrv_cdqm_query_para *para)
{
    size_t name_len;
    int ret;

    name_len = strnlen(para->name, CDQ_NAME_LEN);
    if ((name_len == 0) || (name_len >= CDQ_NAME_LEN)) {
        trs_err("Length out of range. (name_len=%lu)\n", name_len);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    ret = cdqm_get_cdq_id(devid, tsid, para->host_pid, para->name, &para->qid);
    if (ret != 0) {
        trs_err("devid %u ts %u cdq %u name:%s pid %d not found:%d.\n",
            devid, tsid, para->qid, para->name, para->host_pid, ret);
        return ret;
    }

    trs_info("devid %u ts %u cdq %u name:%s .\n", devid, tsid, para->qid, para->name);

    return 0;
}

static int cdqm_drv_alloc_batch(u32 devid, u32 tsid, int pid, struct tsdrv_cdqm_alloc_batch_para *para)
{
    int ret;
    u32 cdq_id = para->qid;
    u32 jf_timeout;
    u32 cdq_head;
    u32 batch_num;
    struct cdqm_dev_manage *cdq_dev = NULL;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    ret = cdqm_exist_cdq_para_check(devid, tsid, cdq_id, pid);
    if (ret != 0) {
        trs_err("alloc para check failed,dev %u ts %u cdq %u.\n", devid, tsid, cdq_id);
        return ret;
    }

    cdq_dev = cdqm_get_cdq_dev(devid, tsid);
    cdq_manage = cdqm_get_cdq_mng(devid, tsid, cdq_id);

    mutex_lock(&cdq_manage->cdq_mutex);
    if (cdq_manage->create_side != tsdrv_cdqm_get_local_side()) {
        trs_err("cdq %u created by %u, now side %d.\n", cdq_id, (u32)cdq_manage->create_side,
            tsdrv_cdqm_get_local_side());
        goto err;
    }

    if (cdq_manage->time_out_flag != 0) {
        trs_err("cdq %u has timeout batch.\n", cdq_id);
        ret = (int)DRV_ERROR_CDQ_ABNORMAL;
        goto err;
    }

    batch_num = cdq_manage->config_info.batch_num;
    tsdrv_cdqm_get_cdq_rdy_idx(cdq_dev->io_base, cdq_id, batch_num, &cdq_manage->use_batch_head);

    if (cdqm_judge_cdq_full(cdq_manage) != 0) {
        cdq_head = cdq_manage->use_batch_head;
        if (para->timeout == 0) {
            trs_warn("devid %u cdq %u full.\n", devid, cdq_id);
            ret = (int)DRV_ERROR_NO_CDQ_RESOURCES;
            goto err;
        }
        tsdrv_cdqm_set_batch_get_notify(cdq_dev->io_base, cdq_id, 1);

        mutex_unlock(&cdq_manage->cdq_mutex);

        jf_timeout = msecs_to_jiffies((u32)para->timeout);
        ret = wait_event_interruptible_timeout(cdq_manage->wq,
            ((cdq_manage->valid == CDQ_INVALID) || (cdq_manage->use_batch_head != cdq_head)), jf_timeout);

        if (cdq_manage->valid == CDQ_INVALID) {
            ret = (int)DRV_ERROR_CDQ_QUIT;
            trs_info("dev %u ts %u cdq %u alloc batch destroyed.\n", devid, tsid, cdq_id);
            return (int)DRV_ERROR_CDQ_QUIT;
        }

        mutex_lock(&cdq_manage->cdq_mutex);

        if (ret == 0) {
            mutex_unlock(&cdq_manage->cdq_mutex);
            trs_info("dev %u ts %u cdq %u alloc batch timeout.\n", devid, tsid, cdq_id);
            return (int)DRV_ERROR_WAIT_TIMEOUT;
        } else if (ret == -ERESTARTSYS) {
            mutex_unlock(&cdq_manage->cdq_mutex);
            trs_info("dev %u ts %u cdq %u alloc batch interrupted.\n", devid, tsid, cdq_id);
            return (int)DRV_ERROR_WAIT_INTERRUPT;
        }
    }

    para->batch_id = cdq_manage->use_batch_tail;
    cdq_manage->use_batch_tail = (cdq_manage->use_batch_tail + 1) % batch_num;

    mutex_unlock(&cdq_manage->cdq_mutex);

    trs_info("cdqm alloc batch success. dev:%u ts:%u cdq:%u batch id:%u.head:%u tail:%u\n", devid, tsid,
        cdq_id, para->batch_id, cdq_manage->use_batch_head, cdq_manage->use_batch_tail);

    return 0;

err:
    mutex_unlock(&cdq_manage->cdq_mutex);
    return ret;
}

static int cdqm_drv_get_ready_batch(u32 devid, u32 tsid, struct tsdrv_cdqm_get_ready_batch_para *para)
{
    u32 cdq_id = para->qid;
    u32 rdy_num, rdy_idx, batch_num;
    struct cdqm_dev_manage *cdq_dev = NULL;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    if (cdqm_cdq_id_para_check(devid, tsid, cdq_id) != 0) {
        trs_err("devid %u cdq id %u not valid.\n", devid, para->qid);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    cdq_dev = cdqm_get_cdq_dev(devid, tsid);
    tsdrv_cdqm_get_cdq_rdy_num(cdq_dev->io_base, cdq_id, &rdy_num);
    if (rdy_num == 0) {
        trs_info("devid %u  cdq %u not ready.\n", devid, cdq_id);
        tsdrv_cdqm_set_batch_rdy_notify(cdq_dev->io_base, cdq_id);
        tsdrv_cdqm_get_cdq_rdy_num(cdq_dev->io_base, cdq_id, &rdy_num);
        if (rdy_num == 0) { /* read rdy_num again to make sure no batch ready during set rdy_notify register */
            return (int)DRV_ERROR_NO_CDQ_RESOURCES;
        }
    }

    cdq_manage = &cdq_dev->cdq_manage[cdq_id];
    batch_num = cdq_manage->config_info.batch_num;
    tsdrv_cdqm_get_cdq_rdy_idx(cdq_dev->io_base, cdq_id, batch_num, &rdy_idx);

    if (rdy_idx == cdq_manage->last_get_batch) {
        trs_err("devid %u cdq %u batchId(%u) repeat get.\n", devid, cdq_id, cdq_manage->last_get_batch);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    if (rdy_idx != (cdq_manage->last_get_batch + 1) % batch_num) {
        trs_err("devid %u cdq %u rdy_idx %u last_get_batch(%u).\n", devid, cdq_id, rdy_idx,
            cdq_manage->last_get_batch);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    para->batch_size = cdq_manage->config_info.batch_size;
    para->batch_addr = cdq_manage->config_info.mem_addr + para->batch_size * rdy_idx * CDQE_SIZE;

    cdq_manage->last_get_batch = rdy_idx;

    trs_info("get devid %u cdq %u batchId(%u) addr(%pK).\n", devid, cdq_id, rdy_idx, (void *)para->batch_addr);

    return 0;
}

static int cdqm_drv_free_batch(u32 devid, u32 tsid, struct tsdrv_cdqm_free_batch_para *para)
{
    int ret;
    u32 cdq_id = para->qid;
    struct cdqm_dev_manage *cdq_dev = NULL;

    ret = cdqm_cdq_id_para_check(devid, tsid, cdq_id);
    if (ret != 0) {
        trs_err("para check failed,devid %u tsid %u qid %u.\n", devid, tsid, cdq_id);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    cdq_dev = cdqm_get_cdq_dev(devid, tsid);
    tsdrv_cdqm_set_cdq_batch_get(cdq_dev->io_base, para->qid, 1);

    trs_info("dev %u ts %u cdq %u free batch ok.\n", devid, tsid, para->qid);

    return 0;
}

static int cdqm_drv_batch_abnormal(u32 devid, u32 tsid, struct tsdrv_cdqm_batch_abnormal *para)
{
    int ret;
    u32 cdq_id = para->qid;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    ret = cdqm_cdq_id_para_check(devid, tsid, cdq_id);
    if (ret != 0) {
        trs_err("timeout para check failed.\n");
        return ret;
    }

    cdq_manage = cdqm_get_cdq_mng(devid, tsid, cdq_id);
    cdq_manage->time_out_flag = ENABLE_TIMEOUT_ERR;

    ret = cdqm_sync_msg_batch_abnormal(devid, tsid, cdq_id);
    if (ret != 0) {
        trs_err("other side set dev %u ts %u cdq %u timeout failed:%d.\n", devid, tsid, cdq_id, ret);
        return ret;
    }

    return 0;
}

int cdqm_ioctl_comm(u32 devid, int pid, struct cdqm_ioctl_arg *arg)
{
    int ret;

    switch (arg->cdqm_para.type) {
        case CDQM_CREATE_CDQ:
            ret = cdqm_drv_create_cdq(devid, arg->tsid, pid, &arg->cdqm_para.create_para);
            break;
        case CDQM_DESTROY_CDQ:
            ret = cdqm_drv_destroy_cdq(devid, arg->tsid, pid, &arg->cdqm_para.destroy_para);
            break;
        case CDQM_QUERY_CDQ:
            ret = cdqm_drv_query_cdq(devid, arg->tsid, &arg->cdqm_para.query_para);
            break;
        case CDQM_ALLOC_BATCH:
            ret = cdqm_drv_alloc_batch(devid, arg->tsid, pid, &arg->cdqm_para.alloc_para);
            break;
        case CDQM_GET_READY_BATCH:
            ret = cdqm_drv_get_ready_batch(devid, arg->tsid, &arg->cdqm_para.rdy_para);
            break;
        case CDQM_FREE_BATCH:
            ret = cdqm_drv_free_batch(devid, arg->tsid, &arg->cdqm_para.free_para);
            break;
        case CDQM_BATCH_ABNORMAL:
            ret = cdqm_drv_batch_abnormal(devid, arg->tsid, &arg->cdqm_para.timeout_para);
            break;
        default:
            trs_err("ioctl handel out of rage, type:%d", (int)arg->cdqm_para.type);
            ret = (int)DRV_ERROR_INVALID_HANDLE;
    }

    return ret;
}

int tsdrv_cdqm_set_topic_id(u32 devid, u32 topic_id)
{
    struct cdqm_dev_manage *cdq_dev = NULL;
    u32 tsid;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        trs_err("dev %u invalid.\n", devid);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    for (tsid = 0; tsid < DEVDRV_MAX_TS_NUM; tsid++) {
        cdq_dev = cdqm_get_cdq_dev(devid, tsid);
        if (cdq_dev != NULL) {
            tsdrv_cdqm_write_topic_id(cdq_dev->io_base, topic_id);
        }
    }

    return 0;
}
EXPORT_SYMBOL(tsdrv_cdqm_set_topic_id);

u32 tsdrv_cdqm_get_instance_by_cdqid(u32 devid, u32 tsid, u32 cdq_id)
{
    return CDQ_INSTANCE(devid, tsid, cdq_id);
}
EXPORT_SYMBOL(tsdrv_cdqm_get_instance_by_cdqid);

int tsdrv_cdqm_get_name_by_cdqid(u32 devid, u32 tsid, u32 cdq_id, char *name, int buf_len)
{
    int ret;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM || tsid >= DEVDRV_MAX_TS_NUM || cdqm_get_cdq_dev(devid, tsid) == NULL ||
        cdq_id >= MAX_CDQM_CDQ_NUM) {
        trs_err("dev %u ts %u cdqid %u invalid.\n", devid, tsid, cdq_id);
        return (int)DRV_ERROR_PARA_ERROR;
    }
    if (name == NULL || buf_len < CDQ_NAME_LEN) {
        trs_err("name ptr invalid.\n");
        return (int)DRV_ERROR_PARA_ERROR;
    }

    cdq_manage = cdqm_get_cdq_mng(devid, tsid, cdq_id);
    ret = strcpy_s(name, (size_t)buf_len, cdq_manage->config_info.name);
    if (ret != EOK) {
        trs_err("Strcpy error. (dev=%u; ts=%u; cdq=%u; ret:%d)\n", devid, tsid, cdq_id, ret);
        return ret;
    }

    return ret;
}
EXPORT_SYMBOL(tsdrv_cdqm_get_name_by_cdqid);

int cdqm_recycle_cdq(u32 devid, u32 tsid, int pid)
{
    struct cdqm_dev_manage *cdq_dev = NULL;
    struct cdqm_cdq_manage *cdq_manage = NULL;
    struct tsdrv_cdqm_destroy_para destroy_para;
    int i, ret;

    cdq_dev = cdqm_get_cdq_dev(devid, tsid);
    if (cdq_dev == NULL) {
        trs_warn("dev %u ts %u release manage info NULL.\n", devid, tsid);
        return 0;
    }

    mutex_lock(&cdq_dev->dev_mutex);

    cdq_manage = cdq_dev->cdq_manage;

    for (i = 0; i < MAX_CDQM_CDQ_NUM; i++) {
        if (cdq_manage[i].config_info.pid == pid && cdq_manage[i].valid == CDQ_VALID) {
            destroy_para.qid = (u32)i;
            destroy_para.tsid = tsid;
            cdq_manage[i].recycle_pid = (u32)pid;

            ret = cdqm_drv_destroy_cdq(devid, tsid, pid, &destroy_para);
            if (ret != 0) {
                trs_warn("cdqm dev %u ts %u cdq %d destroy by release failed:%d.\n", devid, tsid, i, ret);
            } else {
                trs_info("cdqm dev %u ts %u cdq %d destroy by release.\n", devid, tsid, i);
            }
        }
    }

    mutex_unlock(&cdq_dev->dev_mutex);
    trs_info("cdqm dev %u ts %u recycle cdq finished.\n", devid, tsid);

    return 0;
}

/* 后续tsdrv实现？ */
enum tsdrv_env_type {
    TSDRV_ENV_OFFLINE = 0,  /* do not change the value, the value is used for mbox communication */
    TSDRV_ENV_ONLINE = 1
};

static inline enum tsdrv_env_type tsdrv_get_env_type(void)
{
#if defined(CFG_MANAGER_HOST_ENV)
    return TSDRV_ENV_ONLINE;
#else
    return TSDRV_ENV_OFFLINE;
#endif
}

irqreturn_t cdqm_f2nf_handle(int irq, void *data)
{
    struct cdqm_dev_manage *cdq_dev = (struct cdqm_dev_manage *)data;

    if (tsdrv_get_env_type() == TSDRV_ENV_OFFLINE) {
        tsdrv_cdqm_f2nf_int_mask_l1(cdq_dev->io_base);
    }
    tasklet_schedule(&cdq_dev->f2nf_task);

    return IRQ_HANDLED;
}

static int cdqm_get_f2nf_irq(u32 devid, u32 *hwirq, u32 *irq)
{
    struct res_inst_info inst = {.devid = devid, .sub_type = TS_SUBSYS, .subid = 0};
    int ret;

    ret = soc_resmng_get_irq_by_index(&inst, TS_STARS_CDQM_IRQ, 0, irq);
    if (ret != 0) {
        trs_err("dev(%d) get irq failed(%d).\n", devid, ret);
        return ret;
    }

    ret = soc_resmng_get_hwirq(&inst, TS_STARS_CDQM_IRQ, *irq, hwirq);
    if (ret != 0) {
        trs_err("dev(%d) get hwirq failed(%d) hwirq %d.\n", devid, ret, *hwirq);
        return ret;
    }

    return 0;
}

static int cdqm_get_hw_info(u32 devid, u32 tsid, struct cdqm_dev_manage *cdq_dev)
{
    struct res_inst_info inst;
    struct soc_reg_base_info reg_base;
    int ret;

    soc_resmng_inst_pack(&inst, devid, TS_SUBSYS, tsid);
    ret = soc_resmng_get_reg_base(&inst, "TS_STARS_CDQM_REG", &reg_base);
    if (ret != 0) {
        trs_err("dev(%u) ts(%u) cdqm get addr info failed(%d).\n", devid, tsid, ret);
        return ret;
    }

    cdq_dev->io_base = ioremap(reg_base.io_base, reg_base.io_base_size);
    if (cdq_dev->io_base == NULL) {
        trs_err("dev(%u) ts(%u) cdqm ioremap failed(%d).\n", devid, tsid, ret);
        return -EINVAL;
    }

    if (devdrv_get_dev_chip_type(devid) == HISI_MINI_V3 && tsdrv_get_env_type() == TSDRV_ENV_ONLINE) {
        cdq_dev->int_io_base = 0;
    } else {
        ret = soc_resmng_get_reg_base(&inst, "TS_STARS_INT_REG", &reg_base);
        if (ret != 0) {
            trs_err("dev(%u) ts(%u) cdqm get irq reg failed(%d).\n", devid, tsid, ret);
            return ret;
        }

        cdq_dev->int_io_base = ioremap(reg_base.io_base, reg_base.io_base_size);
        if (cdq_dev->int_io_base == NULL) {
            trs_err("dev(%u) ts(%u) cdqm ioremap failed(%d).\n", devid, tsid, ret);
            return -EINVAL;
        }
    }

    ret = cdqm_get_f2nf_irq(devid, &cdq_dev->f2nf_hwirq, &cdq_dev->f2nf_irq);
    if (ret != 0) {
        trs_err("dev(%u) ts(%u) cdqm get irq failed(%d).\n", devid, tsid, ret);
        return ret;
    }
    return 0;
}

static void cdqm_update_dev_ops(struct cdqm_dev_manage *cdq_dev, struct cdqm_adapt_ops *ops)
{
    if (cdq_dev->ops.send_sync_msg == NULL) {
        cdq_dev->ops.send_sync_msg = ops->send_sync_msg;
    }
}

static int cdqm_dev_init(u32 devid, u32 tsid, struct cdqm_adapt_ops *ops)
{
    struct cdqm_dev_manage *cdq_dev = NULL;
    int i, ret;

    cdq_dev = g_cdq_dev_manage[devid][tsid];
    if (cdq_dev != NULL) {
        if (ops != NULL) {
            cdqm_update_dev_ops(cdq_dev, ops);
        }
        return 0;
    }

    cdq_dev = kzalloc(sizeof(struct cdqm_dev_manage), GFP_KERNEL);
    if (cdq_dev == NULL) {
        trs_err("devid(%u) tsid(%u)kamlloc cdq_dev failed.\n", devid, tsid);
        return -ENOMEM;
    }

    cdq_dev->devid = devid;
    cdq_dev->tsid = tsid;
    if (ops != NULL) {
        cdq_dev->ops = *ops;
    }

    mutex_init(&cdq_dev->dev_mutex);
    hash_init(cdq_dev->proc_hash_table);

    for (i = 0; i < MAX_CDQM_CDQ_NUM; i++) {
        mutex_init(&cdq_dev->cdq_manage[i].cdq_mutex);
        init_waitqueue_head(&cdq_dev->cdq_manage[i].wq);
    }

    ret = cdqm_get_hw_info(devid, tsid, cdq_dev);
    if (ret != 0) {
        goto out;
    }

    f2nf_tasklet_init(cdq_dev);
    ret = request_irq(cdq_dev->f2nf_irq, cdqm_f2nf_handle, 0, "cdqm_f2nf", (void *)cdq_dev);
    if (ret != 0) {
        tasklet_kill(&cdq_dev->f2nf_task);
        trs_err("request irq fail:%d\n", ret);
        ret = (int)DRV_ERROR_INNER_ERR;
        goto out;
    }
    (void)irq_set_affinity_hint(cdq_dev->f2nf_irq, NULL);

    ret = cdqm_dev_adapt_init(cdq_dev);
    if (ret != 0) {
        tasklet_kill(&cdq_dev->f2nf_task);
        (void)free_irq(cdq_dev->f2nf_irq, cdq_dev);
        goto out;
    }

    g_cdq_dev_manage[devid][tsid] = cdq_dev;

    trs_info("Init finish. (dev=%u; tsid=%u; f2nf_vector_id=%u; irq=%u; io_base=0x%pK; int_io_base=0x%pK)\n",
        devid, tsid, cdq_dev->f2nf_hwirq, cdq_dev->f2nf_irq, (void *)(uintptr_t)cdq_dev->io_base,
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

static void cdqm_dev_uninit(u32 devid, u32 tsid)
{
    struct cdqm_dev_manage *cdq_dev = NULL;

    cdq_dev = g_cdq_dev_manage[devid][tsid];
    if (cdq_dev == NULL) {
        return;
    }

    tasklet_kill(&cdq_dev->f2nf_task);
    (void)irq_set_affinity_hint(cdq_dev->f2nf_irq, NULL);
    (void)free_irq(cdq_dev->f2nf_irq, cdq_dev);
    iounmap(cdq_dev->io_base);
    iounmap(cdq_dev->int_io_base);
    kfree(cdq_dev);
    g_cdq_dev_manage[devid][tsid] = NULL;
}

/* 后续显示包含对应头文件 */
int devmng_get_vdavinci_info(u32 vdev_id, u32 *phy_id, u32 *vfid);
int cdqm_ts_inst_register(struct trs_id_inst *inst, struct cdqm_adapt_ops *ops)
{
    u32 devid, fid;
    int ret;

    ret = trs_id_inst_check(inst);
    if (ret != 0) {
        return ret;
    }

    ret = devmng_get_vdavinci_info(inst->devid, &devid, &fid);
    if ((ret != 0) || (devid >= CDQM_MAX_DAVINCI_NUM)) {
        trs_err("Invalid para. (vdevid=%u; devid=%u; fid=%u)\n", inst->devid, devid, fid);
        return -EINVAL;
    }

    ret = cdqm_dev_init(devid, inst->tsid, ops);
    if (ret != 0) {
        trs_err("cdqm init failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(cdqm_ts_inst_register);

int cdqm_ts_inst_unregister(struct trs_id_inst *inst)
{
    u32 devid, fid;
    int ret;

    ret = trs_id_inst_check(inst);
    if (ret != 0) {
        return ret;
    }

    ret = devmng_get_vdavinci_info(inst->devid, &devid, &fid);
    if ((ret != 0) || (devid >= CDQM_MAX_DAVINCI_NUM)) {
        trs_err("Invalid para. (vdevid=%u; devid=%u; fid=%u)\n", inst->devid, devid, fid);
        return -EINVAL;
    }

    cdqm_dev_uninit(devid, inst->tsid);

    return 0;
}
EXPORT_SYMBOL(cdqm_ts_inst_unregister);

bool cdqid_is_belong_to_proc(struct trs_id_inst *inst, pid_t tgid, int res_type, u32 id)
{
    struct cdqm_cdq_manage *cdq_manage = NULL;
    struct cdqm_dev_manage *cdq_dev = NULL;

    if ((inst->devid >= CDQM_MAX_DAVINCI_NUM) || (inst->tsid >= DEVDRV_MAX_TS_NUM) || (id >= MAX_CDQM_CDQ_NUM)) {
        trs_err("Invalid id_inst. (devid=%u; cdq_id=%u; tsid=%u)\n", inst->devid, id, inst->tsid);
        return false;
    }

    cdq_dev = cdqm_get_cdq_dev(inst->devid, inst->tsid);
    if (cdq_dev == NULL) {
        trs_err("Invalid id_inst. (devid=%u; cdq_id=%u; tsid=%u)\n", inst->devid, id, inst->tsid);
        return false;
    }

    cdq_manage = cdqm_get_cdq_mng(inst->devid, inst->tsid, id);
    mutex_lock(&cdq_manage->cdq_mutex);
    if (cdq_manage->valid != CDQ_VALID) {
        mutex_unlock(&cdq_manage->cdq_mutex);
        trs_err("Valid. (devid=%u; tsid=%u; cdqId=%u)\n", inst->devid, inst->tsid, id);
        return false;
    }

    if (cdq_manage->config_info.pid != tgid) {
        mutex_unlock(&cdq_manage->cdq_mutex);
        trs_err("Valid. (devid=%u; tsid=%u; cdqId=%u; pid=%d)\n", inst->devid, inst->tsid, id, tgid);
        return false;
    }
    mutex_unlock(&cdq_manage->cdq_mutex);

    return true;
}
EXPORT_SYMBOL(cdqid_is_belong_to_proc);

