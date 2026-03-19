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

#ifndef TSDRV_KERNEL_UT
#include <linux/stat.h>
#ifndef AOS_LLVM_BUILD
#include <linux/debugfs.h>
#endif
#include <linux/slab.h>
#include <linux/atomic.h>

#include "securec.h"
#include "tsdrv_dev_dfx.h"
#include "tsdrv_heartbeat.h"
#include "tsdrv_pdata.h"
#include "devdrv_manager_common.h"

#define HB_DFX_BUF_LEN        (10 * 1024)
#define HB_DFX_BUF_MIN_LEN    30
#define HB_DFX_MAX_OPEN_NUM   128
#define HB_REPORT_SHOW_NUM    20

#ifndef CFG_TRS_REFACTOR_FEATURE

STATIC atomic_t g_hb_dfx_open_cnt = ATOMIC_INIT(0);

struct hb_dfx_private_data {
    u32 dfx_id;
    char *buff;
#ifdef AOS_LLVM_BUILD
    u32 finish_flag;
#endif
};

STATIC int tsdrv_hb_dfx_open(struct inode *inode, struct file *filp)
{
#ifndef TSDRV_UT
    struct hb_dfx_private_data *data = NULL;

    if (atomic_inc_return(&g_hb_dfx_open_cnt) > HB_DFX_MAX_OPEN_NUM) {
        atomic_dec(&g_hb_dfx_open_cnt);
        TSDRV_PRINT_ERR("over max hb dfx open num=%d\n", HB_DFX_MAX_OPEN_NUM);
        return -ENODEV;
    }

    data = kzalloc(sizeof(struct hb_dfx_private_data), GFP_KERNEL);
    if (data == NULL) {
        TSDRV_PRINT_ERR("data kzalloc failed\n");
        atomic_dec(&g_hb_dfx_open_cnt);
        return -ENODEV;
    }
#ifndef AOS_LLVM_BUILD
    data->dfx_id = (u32)(uintptr_t)inode->i_private;
#else
    data->dfx_id =  (u32)(uintptr_t)pde_data(inode);
    data->finish_flag = 0;
#endif
    data->buff = kzalloc(HB_DFX_BUF_LEN, GFP_KERNEL);
    if (data->buff == NULL) {
        TSDRV_PRINT_ERR("data->buff kzalloc failed\n");
        kfree(data);
        atomic_dec(&g_hb_dfx_open_cnt);
        return -ENODEV;
    }
    filp->private_data = (void *)data;
#endif
    return 0;
}

STATIC int tsdrv_hb_dfx_prepare(struct hb_dfx_private_data *data, struct tsdrv_heart_beat_info *hb)
{
#ifndef TSDRV_UT
    struct devdrv_functional_cq_info *cq_info = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    struct devdrv_heart_beat_cq *cq = NULL;
    int buf_size = HB_DFX_BUF_LEN;
    char *str = data->buff;
    u8 *slot_addr = NULL;
    u32 tmp_head, slot;
    int out_len = 0;
    int len, i;

    if (str == NULL) {
        TSDRV_PRINT_ERR("Buff is null.\n");
        return 0;
    }
    len = snprintf_s(str, buf_size, buf_size - 1, "send_number:%u  resp_number:%u  lost_cnt:%u\n"
        "sq_index:%4u  cq_index:%4u\n",
        hb->msg_sn, hb->resp_msg.sn, hb->lost_count, hb->msg_chan.sq_index, hb->msg_chan.cq_index);
    DRV_CHECK_EXP_ACT(len < 0, return 0, "snprintf_s fail, len(%d)\n", len);
    str += len;
    buf_size -= len;
    out_len += len;
    DRV_CHECK_EXP_ACT(buf_size < HB_DFX_BUF_MIN_LEN, return 0, "buf_size(%d) err, line:%d\n", buf_size, __LINE__);

    ts_res = tsdrv_get_ts_resoruce(hb->instance.devid, TSDRV_PM_FID, hb->instance.tsid);
    cq_info = &ts_res->functional_cqsq.cq_info[hb->msg_chan.cq_index];
    tmp_head = cq_info->head;
    len = snprintf_s(str, buf_size, buf_size - 1, "cur_head:%4u         cur_tail:%4u         cur_phase:%u\n",
        tmp_head, cq_info->tail, cq_info->phase);
    DRV_CHECK_EXP_ACT(len < 0, return 0, "snprintf_s fail, len(%d)\n", len);
    str += len;
    buf_size -= len;
    out_len += len;
    DRV_CHECK_EXP_ACT(buf_size < HB_DFX_BUF_MIN_LEN, return 0, "buf_size(%d) err,line:%d\n", buf_size, __LINE__);

    len = snprintf_s(str, buf_size, buf_size - 1, "slot       phase      number      status        type      except"
        "        time\n");
    DRV_CHECK_EXP_ACT(len < 0, return 0, "snprintf_s fail, len(%d)\n", len);
    str += len;
    buf_size -= len;
    out_len += len;
    DRV_CHECK_EXP_ACT(buf_size < HB_DFX_BUF_MIN_LEN, return 0, "buf_size(%d) err, line:%d\n", buf_size, __LINE__);

    for (i = 0; i < HB_REPORT_SHOW_NUM; ++i) {
        /* show range : head - HB_REPORT_SHOW_NUM / 2 ~ head + HB_REPORT_SHOW_NUM / 2 */
        slot = (cq_info->depth + tmp_head - HB_REPORT_SHOW_NUM / 2 + i) % cq_info->depth;
        slot_addr = cq_info->addr + (unsigned long)slot * cq_info->slot_len;
        cq = (struct devdrv_heart_beat_cq *)(slot_addr + DEVDRV_FUNCTIONAL_DETAILED_CQ_OFFSET);
        len = snprintf_s(str, buf_size, buf_size - 1, "%4u%12u%12u%12u%12u%12u%12lld\n", slot, slot_addr[0],
            cq->number, cq->ts_status, cq->report_type, cq->exception_code, cq->exception_time.tv_sec);
        DRV_CHECK_EXP_ACT(len < 0, return 0, "snprintf_s fail, len(%d)\n", len);
        str += len;
        buf_size -= len;
        out_len += len;
        DRV_CHECK_EXP_ACT(buf_size < HB_DFX_BUF_MIN_LEN, return 0, "buf_size(%d) err, line:%d\n", buf_size, __LINE__);
    }

    return out_len;
#endif
}

STATIC ssize_t tsdrv_hb_dfx_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
#ifndef TSDRV_UT
    u32 devid, tsid, fid;
    struct tsdrv_heart_beat_info *hb = NULL;
    int output_len = 0;

    struct hb_dfx_private_data *data = (struct hb_dfx_private_data *)file->private_data;
    if (data == NULL) {
        TSDRV_PRINT_ERR("Private data is null.\n");
        return 0;
    }
    devid = tsdrv_dfx_id_to_devid(data->dfx_id);
    tsid = tsdrv_dfx_id_to_tsid(data->dfx_id);
    fid = tsdrv_dfx_id_to_fid(data->dfx_id);
#ifdef AOS_LLVM_BUILD
    if (data->finish_flag) {
        TSDRV_PRINT_INFO("read finish. return 0\n");
        return 0;
    }
    data->finish_flag = 1;
#endif

    if ((devid >= TSDRV_MAX_DAVINCI_NUM) || (tsid >= DEVDRV_MAX_TS_NUM) || (fid >= TSDRV_MAX_FID_NUM))
            return 0;

    hb = tsdrv_hb_get_heart_beat_info(devid, tsid);
    mutex_lock(&hb->lock);
    if (hb->init_state == TSDRV_HEARTBEAT_INITED) {
        output_len = tsdrv_hb_dfx_prepare(data, hb);
    }
    mutex_unlock(&hb->lock);
#ifndef AOS_LLVM_BUILD
    return simple_read_from_buffer_safe(user_buf, count, ppos, data->buff, output_len);
#else
    return simple_read_from_buffer_safe(user_buf, data->buff, output_len);
#endif
#endif
}

STATIC int tsdrv_hb_dfx_release(struct inode *inode, struct file *filep)
{
#ifndef TSDRV_UT
    struct hb_dfx_private_data *data = (struct hb_dfx_private_data *)filep->private_data;

    if (data == NULL) {
        TSDRV_PRINT_ERR("data is NULL.\n");
        return -ENOMEM;
    } else {
        if (data->buff != NULL) {
            kfree(data->buff);
        }
        kfree(data);
        atomic_dec(&g_hb_dfx_open_cnt);
        filep->private_data = NULL;
        return 0;
    }
#endif
}
#ifndef AOS_LLVM_BUILD
static const struct file_operations ops = {
    .open = tsdrv_hb_dfx_open,
    .read = tsdrv_hb_dfx_read,
    .release = tsdrv_hb_dfx_release,
};

int tsdrv_hb_dfx_file_create(u32 devid, u32 fid)
{
    struct tsdrv_dfx_fid *dfx_fid = tsdrv_get_fid_dfx(devid, fid);
    struct dentry *ts_id_dentry[DEVDRV_MAX_TS_NUM] = {NULL};
    u32 tsnum = tsdrv_get_ts_num(devid);
    char name[TSDRV_DFX_NAME_LEN] = {0};
    struct dentry *hb_dir = NULL;
    u32 tsid, i, dfx_id;
    int err;

    if (tsnum > DEVDRV_MAX_TS_NUM) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Ts number is invalid.(devid=%u; tsnum=%u)\n", devid, tsnum);
        return -EINVAL;
#endif
    }

    if (dfx_fid->fid_dentry == NULL) {
        TSDRV_PRINT_ERR("fid entry is NULL, devid(%u) fid(%u)\n", devid, fid);
        return -ENODEV;
    }

    hb_dir = debugfs_create_dir("ts_heart_beat", dfx_fid->fid_dentry);
    if (hb_dir == NULL) {
        TSDRV_PRINT_ERR("ts_heart_beat dir create fail, devid=%u fid=%u\n", devid, fid);
        return -EFAULT;
    }

    for (tsid = 0; tsid < tsnum; tsid++) {
        err = snprintf_s(name, TSDRV_DFX_NAME_LEN, TSDRV_DFX_NAME_LEN - 1, "hb_ts%u", tsid);
        if (err < 0) {
            TSDRV_PRINT_ERR("copy dfx name fail, err(%d)\n", err);
            goto ERR;
        }

        dfx_id = tsdrv_pack_dfx_id(devid, fid, tsid);
        ts_id_dentry[tsid] = debugfs_create_file(name, S_IRUGO, hb_dir, (void *)(uintptr_t)dfx_id, &ops);
        if (ts_id_dentry[tsid] == NULL) {
            TSDRV_PRINT_ERR("create hb_ts%u file fail, devid(%u) fid(%u)", tsid, devid, fid);
            goto ERR;
        }
    }
    return 0;
ERR:
    for (i = 0; i < tsid; i++) {
        if (ts_id_dentry[i] != NULL) {
            debugfs_remove_recursive(ts_id_dentry[i]);
        }
    }
#ifndef TSDRV_UT
    debugfs_remove_recursive(hb_dir);
#endif
    return -ENODEV;
}
#else

ssize_t tsdrv_hb_proc_dfx_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    return tsdrv_hb_dfx_read(file, buf, count, NULL);
}

static const struct file_operations proc_ops = {
    .open = tsdrv_hb_dfx_open,
    .read = tsdrv_hb_proc_dfx_read,
    .release = tsdrv_hb_dfx_release,
};

int tsdrv_hb_dfx_file_create(u32 devid, u32 fid)
{
    struct tsdrv_proc_dfx_fid *dfx_fid = tsdrv_get_fid_dfx(devid, fid);
    struct proc_dir_entry *ts_id_dentry[DEVDRV_MAX_TS_NUM] = {NULL};
    u32 tsnum = tsdrv_get_ts_num(devid);
    char name[TSDRV_DFX_NAME_LEN] = {0};
    struct proc_dir_entry *hb_dir = NULL;
    u32 tsid, i, dfx_id;
    int err;

    if (tsnum > DEVDRV_MAX_TS_NUM) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Ts number is invalid.(devid=%u; tsnum=%u)\n", devid, tsnum);
        return -EINVAL;
#endif
    }
    if (dfx_fid->fid_dentry == NULL) {
        TSDRV_PRINT_ERR("fid entry is NULL, devid(%u) fid(%u)\n", devid, fid);
        return -ENODEV;
    }

    hb_dir = proc_mkdir("ts_heart_beat", dfx_fid->fid_dentry);
    if (hb_dir == NULL) {
        TSDRV_PRINT_ERR("ts_heart_beat dir create fail, devid=%u fid=%u\n", devid, fid);
        return -EFAULT;
    }

    for (tsid = 0; tsid < tsnum; tsid++) {
        err = snprintf_s(name, TSDRV_DFX_NAME_LEN, TSDRV_DFX_NAME_LEN - 1, "hb_ts%u", tsid);
        if (err < 0) {
            TSDRV_PRINT_ERR("copy dfx name fail, err(%d)\n", err);
            goto ERR;
        }

        dfx_id = tsdrv_pack_dfx_id(devid, fid, tsid);
        ts_id_dentry[tsid] = proc_create_data(name, TS_DRV_ATTR_RD, hb_dir, &proc_ops, (void *)(uintptr_t)dfx_id);
        if (ts_id_dentry[tsid] == NULL) {
            TSDRV_PRINT_ERR("create hb_ts%u file fail, devid(%u) fid(%u)", tsid, devid, fid);
            goto ERR;
        }
    }
    return 0;

ERR:
    for (i = 0; i < tsid; i++) {
        if (ts_id_dentry[i] != NULL) {
            proc_remove(ts_id_dentry[i]);
        }
    }
    proc_remove(hb_dir);
    return -ENODEV;
}
#endif
#endif
#else
void tsdrv_hb_dfx(void)
{
    return;
}
#endif

