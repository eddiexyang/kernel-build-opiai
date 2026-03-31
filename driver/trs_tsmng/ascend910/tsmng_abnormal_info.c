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
 * Create: 2023-06-09
 */

#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/workqueue.h>

#include <securec.h>
#ifndef EMU_ST
#include "tsmng_log.h"
#ifdef CFG_FEATURE_TS_ABNORMAL_INFO
#include "tsmng_abnormal_info.h"
#include "trs_abnormal_info.h"
#endif
#include "trs_chip_def_comm.h"
#include "icm_interface.h"
#include "uda.h"

#ifdef CFG_SOC_PLATFORM_MINIV3
#define TSMNG_ABNORMAL_INFO_ACK_MBX HISI_RPROC_TX_TS_ACPU0
#else
#define TSMNG_ABNORMAL_INFO_ACK_MBX HISI_RPROC_TX_TS_MBX3
#endif

struct stars_abnormal_ack_info {
    u16 sqid;
    u16 task_id;
    u8 task_type;
    u8 err_type;
    int result;
};

struct tsmng_abnormal_handle {
    struct list_head list;
    int (*handle)(u32 devid, u32 tsid, void *data);
};

struct tsmng_abnormal_handle_list {
    u32 cur_num;
    struct mutex mutex_lock;
    struct list_head head;
};

struct tsmng_abnormal_work {
    u32 devid;
    void *data;
    struct work_struct abnornal_work;
};

struct tsmng_abnormal_handle_list abnormal_handle_list;
struct workqueue_struct *abnormal_handle_wq[TRS_DEVICE_MAX_PHY_DEV];

void tsmng_abnormal_handle_list_init()
{
    abnormal_handle_list.cur_num = 0;
    mutex_init(&abnormal_handle_list.mutex_lock);
    INIT_LIST_HEAD(&abnormal_handle_list.head);
}

void tsmng_abnormal_handle_list_uninit()
{
    struct tsmng_abnormal_handle *abnormal_handle = NULL;
    struct tsmng_abnormal_handle *tmp = NULL;

    mutex_lock(&abnormal_handle_list.mutex_lock);
    list_for_each_entry_safe(abnormal_handle, tmp, &abnormal_handle_list.head, list) {
        list_del(&abnormal_handle->list);
        abnormal_handle_list.cur_num--;
        kfree(abnormal_handle);
    }
    mutex_unlock(&abnormal_handle_list.mutex_lock);
}

int tsmng_abnormal_workqueue_init(u32 udevid)
{
    if (abnormal_handle_wq[udevid] == NULL) {
        abnormal_handle_wq[udevid] = alloc_workqueue("dev%u_abnormal_handle_wq", WQ_UNBOUND, 1, udevid);
        if (abnormal_handle_wq[udevid] == NULL) {
            tsmng_drv_err("Create workqueue failed. (devid=%u)\n", udevid);
            return -EFAULT;
        }
    }

    return 0;
}

void tsmng_abnormal_workqueue_uninit(u32 udevid)
{
    if (abnormal_handle_wq[udevid] != NULL) {
        flush_workqueue(abnormal_handle_wq[udevid]);
        destroy_workqueue(abnormal_handle_wq[udevid]);
    }
}

void tsmng_flush_abnormal_handle_wq(u32 udevid)
{
    flush_workqueue(abnormal_handle_wq[udevid]);   /* 310Brc dont support suspend with task send */
}

int tsmng_local_notifier_func(u32 udevid, enum uda_notified_action action)
{
    int ret = 0;

    if (action == UDA_INIT) {
        ret = tsmng_abnormal_workqueue_init(udevid);
    } else if (action == UDA_UNINIT) {
        tsmng_abnormal_workqueue_uninit(udevid);
    } else if (action == UDA_SUSPEND) {
        tsmng_flush_abnormal_handle_wq(udevid);
    } else {
        /* do nothing */
    }

    tsmng_drv_info("notifier action. (udevid=%u; action=%d)\n", udevid, action);
    return ret;
}

int tsmng_register_abnormal_proc_func(tsmng_abnormal_proc_func func)
{
    struct tsmng_abnormal_handle *abnormal_handle = NULL;

    if (func == NULL) {
        tsmng_drv_err("Invalid NULL func.\n");
        return -EINVAL;
    }

    abnormal_handle = kzalloc(sizeof(struct tsmng_abnormal_handle), GFP_ATOMIC | __GFP_ACCOUNT);
    if (abnormal_handle == NULL) {
        tsmng_drv_err("Kzalloc abnormal handle failed.\n");
        return -EFAULT;
    }

    abnormal_handle->handle = func;

    mutex_lock(&abnormal_handle_list.mutex_lock);
    abnormal_handle_list.cur_num++;
    list_add_tail(&abnormal_handle->list, &abnormal_handle_list.head);
    mutex_unlock(&abnormal_handle_list.mutex_lock);

    return 0;
}
EXPORT_SYMBOL(tsmng_register_abnormal_proc_func);

void tsmng_unregister_abnormal_proc_func(tsmng_abnormal_proc_func func)
{
    struct tsmng_abnormal_handle *abnormal_handle = NULL;
    struct tsmng_abnormal_handle *tmp = NULL;

    mutex_lock(&abnormal_handle_list.mutex_lock);
    list_for_each_entry_safe(abnormal_handle, tmp, &abnormal_handle_list.head, list) {
        if (abnormal_handle->handle == func) {
            list_del(&abnormal_handle->list);
            abnormal_handle_list.cur_num--;
            kfree(abnormal_handle);
        }
    }
    mutex_unlock(&abnormal_handle_list.mutex_lock);
}
EXPORT_SYMBOL(tsmng_unregister_abnormal_proc_func);

static void tsmng_abnormal_proc(struct work_struct *p_work)
{
    struct tsmng_abnormal_work *abnormal_work = container_of(p_work, struct tsmng_abnormal_work, abnornal_work);
    struct icmdrv_ipc_msg_info *ipc_rx = (struct icmdrv_ipc_msg_info *)abnormal_work->data;
    struct stars_abnormal_info *abnormal_info = (struct stars_abnormal_info *)ipc_rx->data;
    struct icmdrv_ipc_msg ipc_tx = {0};
    struct stars_abnormal_ack_info *ack_info = (struct stars_abnormal_ack_info *)ipc_tx.data;
    struct tsmng_abnormal_handle *abnormal_handle = NULL;
    struct uda_mia_dev_para mia_para;
    u32 udevid = abnormal_work->devid;
    int ret = 0;

    if (abnormal_info->vfid >= 1) {
        uda_mia_dev_para_pack(&mia_para, abnormal_work->devid, abnormal_info->vfid - 1);
        ret = uda_mia_devid_to_udevid(&mia_para, &udevid);
        if (ret != 0) {
            tsmng_drv_err("Get udevid failed.(dev_id=%u; vfid=%u; ret=%d)\n",
                abnormal_work->devid, abnormal_info->vfid, ret);
            goto exit;
        }
    }

    tsmng_drv_info("Recv ts abnormal. (udevid=%u; vfid=%u; sqid=%u; sqe_id=%u; task_err=%u; err_type=%u)\n",
        udevid, abnormal_info->vfid, abnormal_info->sqid, abnormal_info->sqe_id,
        abnormal_info->task_type, abnormal_info->err_type);

    mutex_lock(&abnormal_handle_list.mutex_lock);
    list_for_each_entry(abnormal_handle, &abnormal_handle_list.head, list) {
        (void)abnormal_handle->handle(udevid, 0, (void *)abnormal_info);
    }
    mutex_unlock(&abnormal_handle_list.mutex_lock);

    ipc_tx.sub_cmd = ICM_SUB_CMD_INFO;
    ipc_tx.cmd = ICM_MAIN_CMD_TS_REQ;
    ipc_tx.cmd_dest = OBJ_CMD_TS;
    ipc_tx.cmd_src = OBJ_CMD_CCPU;
    ipc_tx.len = sizeof(struct stars_abnormal_ack_info);

    ack_info->sqid = abnormal_info->sqid;
    ack_info->task_id = abnormal_info->task_id;
    ack_info->task_type = abnormal_info->task_type;
    ack_info->err_type = abnormal_info->err_type;
    ack_info->result = ret;

    ret = icm_msg_send_async(ICM_FD_BUILD(abnormal_work->devid, TSMNG_ABNORMAL_INFO_ACK_MBX),
        (rproc_msg_t *)&ipc_tx, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        tsmng_drv_err("Send icm ipc msg failed. (devid=%u; ret=%d)\n", abnormal_work->devid, ret);
    }

exit:
    kfree(abnormal_work->data);
    kfree(abnormal_work);
}

int tsmng_ts_req_abnormal(void *data, u32 devid)
{
    struct tsmng_abnormal_work *node = NULL;

    node = kzalloc(sizeof(struct tsmng_abnormal_work), GFP_ATOMIC | __GFP_ACCOUNT);
    if (node == NULL) {
        tsmng_drv_err("Kzalloc abnormal work failed. (devid=%u)\n", devid);
        return -EFAULT;
    }

    node->devid = devid;
    node->data = kzalloc(sizeof(struct icmdrv_ipc_msg_info), GFP_ATOMIC | __GFP_ACCOUNT);
    if (node->data == NULL) {
        kfree(node);
        tsmng_drv_err("kzalloc msg data failed. (devid=%u)\n", devid);
        return -EFAULT;
    }

    (void)memcpy_s(node->data, sizeof(struct icmdrv_ipc_msg_info), data, sizeof(struct icmdrv_ipc_msg_info));

    INIT_WORK(&node->abnornal_work, tsmng_abnormal_proc);

    queue_work(abnormal_handle_wq[devid], &node->abnornal_work);

    return 0;
}
#else
void tsmng_abnormal_info_ut_stub(void)
{
    return;
}
#endif
