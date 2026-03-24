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
* Create: 2022-7-15
*/

#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/mm.h>

#include "ascend_hal_define.h"

#include "davinci_api.h"

#include "trs_id.h"
#include "trs_proc_fs.h"
#include "trs_ts_inst.h"
#include "trs_cmd.h"
#include "trs_fops.h"

static int (*const trs_res_id_handles[TRS_MAX_CMD])(struct trs_proc_ctx *proc_ctx,
    struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para) = {
    [_IOC_NR(TRS_RES_ID_ALLOC)] = trs_res_id_alloc,
    [_IOC_NR(TRS_RES_ID_FREE)] = trs_res_id_free,
    [_IOC_NR(TRS_RES_ID_ENABLE)] = trs_res_id_enable,
    [_IOC_NR(TRS_RES_ID_DISABLE)] = trs_res_id_disable,
    [_IOC_NR(TRS_RES_ID_NUM_QUERY)] = trs_res_id_num_query,
    [_IOC_NR(TRS_RES_ID_MAX_QUERY)] = trs_res_id_max_query,
    [_IOC_NR(TRS_RES_ID_USED_NUM_QUERY)] = trs_res_id_used_query,
    [_IOC_NR(TRS_RES_ID_AVAIL_NUM_QUERY)] = trs_res_id_avail_query,
    [_IOC_NR(TRS_RES_ID_REG_OFFSET_QUERY)] = trs_res_id_reg_offset_query,
    [_IOC_NR(TRS_RES_ID_REG_SIZE_QUERY)] = trs_res_id_reg_size_query,
    [_IOC_NR(TRS_RES_ID_CFG)] = trs_res_id_cfg
};

static bool trs_is_id_query_cmd(unsigned int cmd)
{
    return ((cmd == TRS_RES_ID_NUM_QUERY) || (cmd == TRS_RES_ID_MAX_QUERY) || (cmd == TRS_RES_ID_USED_NUM_QUERY) ||
        (cmd == TRS_RES_ID_REG_OFFSET_QUERY) || (cmd == TRS_RES_ID_REG_SIZE_QUERY) ||
        (cmd == TRS_RES_ID_AVAIL_NUM_QUERY));
}

static int ioctl_trs_res_id_comm(struct trs_proc_ctx *proc_ctx, unsigned int cmd, unsigned long arg)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    struct trs_res_id_para *usr_para = (struct trs_res_id_para __user *)arg;
    struct trs_res_id_para para;
    int ret;

    ret = copy_from_user(&para, usr_para, sizeof(para));
    if (ret != 0) {
        trs_err("Copy from user failed. (cmd=%d, ret=%d)\n", _IOC_NR(cmd), ret);
        return ret;
    }

    if ((para.res_type < 0) || (trs_is_id_query_cmd(cmd) && (para.res_type >= TRS_CORE_MAX_ID_TYPE)) ||
        (!trs_is_id_query_cmd(cmd) && (para.res_type >= TRS_HW_SQ))) {
        trs_err("Invalid para. (cmd=%d; res_type=%d)\n", _IOC_NR(cmd), para.res_type);
        return -EINVAL;
    }

    ts_inst = trs_core_inst_get(proc_ctx->devid, para.tsid);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", proc_ctx->devid, para.tsid);
        return -EINVAL;
    }

    ret = trs_res_id_handles[_IOC_NR(cmd)](proc_ctx, ts_inst, &para);
    trs_core_inst_put(ts_inst);

    if (ret == 0) {
        if (cmd == TRS_RES_ID_ALLOC) {
            ret = put_user(para.id, &usr_para->id);
            if (ret != 0) {
#ifndef EMU_ST
                goto Exit;
#endif
            }
            ret = put_user(para.value[0], &usr_para->value[0]);
            if (ret != 0) {
#ifndef EMU_ST
                goto Exit;
#endif
            }
            ret = put_user(para.value[1], &usr_para->value[1]); /* for id reg addr */
            if (ret != 0) {
#ifndef EMU_ST
                goto Exit;
#endif
            }
        } else if (trs_is_id_query_cmd(cmd)) {
            ret = put_user(para.para, &usr_para->para);
        } else {
            /* do nothing */
        }
    }

Exit:
    if (ret != 0) {
        trs_err("Fail. (devid=%u; tsid=%u; cmd=%d; res_type=%d; id=%u; ret=%d)\n",
            proc_ctx->devid, para.tsid, _IOC_NR(cmd), para.res_type, para.id, ret);
    }

    return ret;
}

static int ioctl_trs_ssid_query(struct trs_proc_ctx *proc_ctx, unsigned int cmd, unsigned long arg)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    struct trs_ssid_query_para *usr_para = (struct trs_ssid_query_para __user *)arg;
    int ret;

    ts_inst = trs_core_inst_get(proc_ctx->devid, 0);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u)\n", proc_ctx->devid);
        return -EINVAL;
    }

    ret = ts_inst->ops.ssid_query(&ts_inst->inst, &proc_ctx->cp_ssid);
    trs_core_inst_put(ts_inst);

    if (ret == 0) {
        ret = put_user(0, &usr_para->ssid); /* invalid SSID returned to the user due to security reasons */
    }

    return ret;
}

static int ioctl_trs_hw_info_query(struct trs_proc_ctx *proc_ctx, unsigned int cmd, unsigned long arg)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    struct trs_hw_info_query_para *usr_para = (struct trs_hw_info_query_para __user *)arg;
    int ret, hw_type, tsid;
    int tsnum = 0;

    for (tsid = 0; tsid < TRS_TS_MAX_NUM; tsid++) {
        ts_inst = trs_core_inst_get(proc_ctx->devid, (u32)tsid);
        if (ts_inst != NULL) {
            hw_type = ts_inst->hw_type;
            tsnum++;
            trs_core_inst_put(ts_inst);
        }
    }
    if (tsnum == 0) {
        trs_err("Invalid para. (devid=%u)\n", proc_ctx->devid);
        return -EINVAL;
    }

    ret = put_user(hw_type, &usr_para->hw_type);
    ret |= put_user(tsnum, &usr_para->tsnum);

    return ret;
}

static int (*const trs_sqcq_alloc_handles[DRV_INVALID_TYPE])(struct trs_proc_ctx *proc_ctx,
    struct trs_core_ts_inst *ts_inst, struct halSqCqInputInfo *para) = {
    [DRV_NORMAL_TYPE] = trs_hw_sqcq_alloc,
    [DRV_CALLBACK_TYPE] = trs_cb_sqcq_alloc,
    [DRV_LOGIC_TYPE] = trs_logic_cq_alloc,
    [DRV_SHM_TYPE] = trs_shm_sqcq_alloc,
    [DRV_CTRL_TYPE] = trs_sw_sqcq_alloc
};

static int ioctl_trs_sqcq_alloc(struct trs_proc_ctx *proc_ctx, unsigned int cmd, unsigned long arg)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    struct halSqCqInputInfo para;
    struct trs_alloc_para *alloc_para = NULL;
    struct trs_uio_info *user_uio_info = NULL;
    struct trs_uio_info uio_info;
    int ret;

    ret = copy_from_user(&para, (struct halSqCqInputInfo __user *)arg, sizeof(para));
    if (ret != 0) {
        trs_err("Copy from user failed. (ret=%d)\n", ret);
        return ret;
    }

    alloc_para = get_alloc_para_addr(&para);
    user_uio_info = alloc_para->uio_info;
    ret = copy_from_user(&uio_info, (struct trs_uio_info __user *)user_uio_info, sizeof(uio_info));
    if (ret != 0) {
        trs_err("Copy from user uio info failed. (ret=%d)\n", ret);
        return ret;
    }

    if ((para.type < 0) || (para.type >= DRV_INVALID_TYPE)) {
        trs_err("Invalid value. (type=%d)\n", para.type);
        return -EINVAL;
    }

    ts_inst = trs_core_inst_get(proc_ctx->devid, para.tsId);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", proc_ctx->devid, para.tsId);
        return -EINVAL;
    }

    mutex_lock(&proc_ctx->ts_ctx[para.tsId].mutex);
    alloc_para->uio_info = &uio_info;
    ret = trs_sqcq_alloc_handles[para.type](proc_ctx, ts_inst, &para);
    alloc_para->uio_info = user_uio_info; /* restore use addr. otherwise, user-mode access will be abnormal. */
    mutex_unlock(&proc_ctx->ts_ctx[para.tsId].mutex);

    trs_core_inst_put(ts_inst);

    if (ret == 0) {
        ret = copy_to_user((struct halSqCqInputInfo __user *)arg, &para, sizeof(para));
        ret |= copy_to_user((struct trs_uio_info __user *)user_uio_info, &uio_info, sizeof(uio_info));
        if (ret != 0) {
            trs_err("Copy to user failed. (ret=%d)\n", ret);
        }
    }

    return ret;
}

static int (*const trs_sqcq_free_handles[DRV_INVALID_TYPE])(struct trs_proc_ctx *proc_ctx,
    struct trs_core_ts_inst *ts_inst, struct halSqCqFreeInfo *para) = {
    [DRV_NORMAL_TYPE] = trs_hw_sqcq_free,
    [DRV_CALLBACK_TYPE] = trs_cb_sqcq_free,
    [DRV_LOGIC_TYPE] = trs_logic_cq_free,
    [DRV_SHM_TYPE] = trs_shm_sqcq_free,
    [DRV_CTRL_TYPE] = trs_sw_sqcq_free
};

static int ioctl_trs_sqcq_free(struct trs_proc_ctx *proc_ctx, unsigned int cmd, unsigned long arg)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    struct halSqCqFreeInfo para;
    int ret;

    ret = copy_from_user(&para, (struct halSqCqFreeInfo __user *)arg, sizeof(para));
    if (ret != 0) {
        trs_err("Copy from user failed. (ret=%d)\n", ret);
        return ret;
    }

    if ((para.type < 0) || (para.type >= DRV_INVALID_TYPE)) {
        trs_err("Invalid value. (type=%d)\n", para.type);
        return -EINVAL;
    }

    ts_inst = trs_core_inst_get(proc_ctx->devid, para.tsId);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", proc_ctx->devid, para.tsId);
        return -EINVAL;
    }

    mutex_lock(&proc_ctx->ts_ctx[para.tsId].mutex);
    ret = trs_sqcq_free_handles[para.type](proc_ctx, ts_inst, &para);
    mutex_unlock(&proc_ctx->ts_ctx[para.tsId].mutex);

    trs_core_inst_put(ts_inst);
    return ret;
}

static int ioctl_trs_sqcq_config(struct trs_proc_ctx *proc_ctx, unsigned int cmd, unsigned long arg)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    struct halSqCqConfigInfo para;
    int ret;

    ret = copy_from_user(&para, (struct halSqCqConfigInfo __user *)arg, sizeof(para));
    if (ret != 0) {
        trs_err("Copy from user failed. (ret=%d)\n", ret);
        return ret;
    }

    if (para.type != DRV_NORMAL_TYPE) {
        trs_err("Invalid value. (type=%d; prop=%d)\n", para.type, para.prop);
        return -EINVAL;
    }

    ts_inst = trs_core_inst_get(proc_ctx->devid, para.tsId);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", proc_ctx->devid, para.tsId);
        return -EINVAL;
    }

    ret = trs_sqcq_config(proc_ctx, ts_inst, &para);

    trs_core_inst_put(ts_inst);
    return ret;
}

static int ioctl_trs_sqcq_query(struct trs_proc_ctx *proc_ctx, unsigned int cmd, unsigned long arg)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    struct halSqCqQueryInfo *usr_para = (struct halSqCqQueryInfo __user *)arg;
    struct halSqCqQueryInfo para;
    int ret;

    ret = (int)copy_from_user(&para, usr_para, sizeof(para));
    if (ret != 0) {
        trs_err("Copy from user failed. (ret=%d)\n", ret);
        return ret;
    }

    if (para.type != DRV_NORMAL_TYPE) {
        trs_err("Invalid value. (type=%d; prop=%d)\n", para.type, para.prop);
        return -EINVAL;
    }

    ts_inst = trs_core_inst_get(proc_ctx->devid, para.tsId);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", proc_ctx->devid, para.tsId);
        return -EINVAL;
    }

    ret = trs_sqcq_query(proc_ctx, ts_inst, &para);

    trs_core_inst_put(ts_inst);

    if (ret == 0) {
        ret = (int)put_user(para.value[0], &usr_para->value[0]);
        if (para.prop == DRV_SQCQ_PROP_SQ_REG_BASE) {
            ret |= (int)put_user(para.value[1], &usr_para->value[1]);
            ret |= (int)put_user(para.value[2], &usr_para->value[2]); /* 2 return sq reg size */
        }
    }

    return ret;
}

static int (*const trs_sqcq_send_handles[DRV_INVALID_TYPE])(struct trs_proc_ctx *proc_ctx,
    struct trs_core_ts_inst *ts_inst, struct halTaskSendInfo *para) = {
    [DRV_NORMAL_TYPE] = trs_hw_sqcq_send,
    [DRV_CALLBACK_TYPE] = trs_cb_sqcq_send,
};

static int ioctl_trs_sqcq_send(struct trs_proc_ctx *proc_ctx, unsigned int cmd, unsigned long arg)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    struct halTaskSendInfo __user *usr_para = (struct halTaskSendInfo __user *)arg;
    struct halTaskSendInfo para;
    int ret;

    ret = copy_from_user(&para, usr_para, sizeof(para));
    if (ret != 0) {
        trs_err("Copy from user failed. (ret=%d)\n", ret);
        return ret;
    }

    if ((para.type < 0) || (para.type >= DRV_INVALID_TYPE) || (trs_sqcq_send_handles[para.type] == NULL) ||
        (para.sqe_num == 0) || (para.sqe_addr == NULL)) {
        trs_err("Invalid value. (type=%d; sqe_num=%u)\n", para.type, para.sqe_num);
        return -EINVAL;
    }

    ts_inst = trs_core_inst_get(proc_ctx->devid, para.tsId);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", proc_ctx->devid, para.tsId);
        return -EINVAL;
    }

    ret = trs_sqcq_send_handles[para.type](proc_ctx, ts_inst, &para);

    trs_core_inst_put(ts_inst);

    if ((ret == 0) && (para.type == DRV_NORMAL_TYPE)) {
        ret = put_user(para.pos, &usr_para->pos);
        if (ret != 0) {
            trs_err("Put to user fail. (devid=%u; tsid=%u; sqId=%u)\n", proc_ctx->devid, para.tsId, para.sqId);
        }
    }

    return ret;
}

static int (*const trs_sqcq_recv_handles[DRV_INVALID_TYPE])(struct trs_proc_ctx *proc_ctx,
    struct trs_core_ts_inst *ts_inst, struct halReportRecvInfo *para) = {
    [DRV_NORMAL_TYPE] = trs_hw_sqcq_recv,
    [DRV_LOGIC_TYPE] = trs_logic_cq_recv,
};

static int ioctl_trs_sqcq_recv(struct trs_proc_ctx *proc_ctx, unsigned int cmd, unsigned long arg)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    struct halReportRecvInfo *usr_para = (struct halReportRecvInfo __user *)arg;
    struct halReportRecvInfo para;
    int ret;

    ret = copy_from_user(&para, usr_para, sizeof(para));
    if (ret != 0) {
        trs_err("Copy from user failed. (ret=%d)\n", ret);
        return ret;
    }

    if ((para.type < 0) || (para.type >= DRV_INVALID_TYPE) || (trs_sqcq_recv_handles[para.type] == NULL) ||
        (para.cqe_num == 0) || (para.cqe_addr == NULL)) {
        trs_err("Invalid value. (type=%d; cqe_num=%u)\n", para.type, para.cqe_num);
        return -EINVAL;
    }

    ts_inst = trs_core_inst_get(proc_ctx->devid, para.tsId);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", proc_ctx->devid, para.tsId);
        return -EINVAL;
    }

    ret = trs_sqcq_recv_handles[para.type](proc_ctx, ts_inst, &para);
    if (ret == 0) {
        ret = put_user(para.report_cqe_num, &usr_para->report_cqe_num);
        if (ret != 0) {
            trs_err("Put to user fail. (devid=%u; tsid=%u; cqId=%u)\n", proc_ctx->devid, para.tsId, para.cqId);
        }
    } else {
        u32 ts_status;
        if (ts_inst->ops.get_ts_inst_status(&ts_inst->inst, &ts_status) == 0) {
            ret = (ts_status == TRS_INST_STATUS_ABNORMAL) ? -EBUSY : ret;
        }
    }

    trs_core_inst_put(ts_inst);

    return ret;
}

static int (*const trs_ioctl_handles[TRS_MAX_CMD])(struct trs_proc_ctx *ctx, unsigned int cmd, unsigned long arg) = {
    [_IOC_NR(TRS_RES_ID_ALLOC)] = ioctl_trs_res_id_comm,
    [_IOC_NR(TRS_RES_ID_FREE)] = ioctl_trs_res_id_comm,
    [_IOC_NR(TRS_RES_ID_ENABLE)] = ioctl_trs_res_id_comm,
    [_IOC_NR(TRS_RES_ID_DISABLE)] = ioctl_trs_res_id_comm,
    [_IOC_NR(TRS_RES_ID_NUM_QUERY)] = ioctl_trs_res_id_comm,
    [_IOC_NR(TRS_RES_ID_MAX_QUERY)] = ioctl_trs_res_id_comm,
    [_IOC_NR(TRS_RES_ID_USED_NUM_QUERY)] = ioctl_trs_res_id_comm,
    [_IOC_NR(TRS_RES_ID_AVAIL_NUM_QUERY)] = ioctl_trs_res_id_comm,
    [_IOC_NR(TRS_RES_ID_REG_OFFSET_QUERY)] = ioctl_trs_res_id_comm,
    [_IOC_NR(TRS_RES_ID_REG_SIZE_QUERY)] = ioctl_trs_res_id_comm,
    [_IOC_NR(TRS_RES_ID_CFG)] = ioctl_trs_res_id_comm,
    [_IOC_NR(TRS_SSID_QUERY)] = ioctl_trs_ssid_query,
    [_IOC_NR(TRS_HW_INFO_QUERY)] = ioctl_trs_hw_info_query,
    [_IOC_NR(TRS_SQCQ_ALLOC)] = ioctl_trs_sqcq_alloc,
    [_IOC_NR(TRS_SQCQ_FREE)] = ioctl_trs_sqcq_free,
    [_IOC_NR(TRS_SQCQ_CONFIG)] = ioctl_trs_sqcq_config,
    [_IOC_NR(TRS_SQCQ_QUERY)] = ioctl_trs_sqcq_query,
    [_IOC_NR(TRS_SQCQ_SEND)] = ioctl_trs_sqcq_send,
    [_IOC_NR(TRS_SQCQ_RECV)] = ioctl_trs_sqcq_recv
};

static bool trs_ioctl_cmd_is_support(u32 devid, int cmd)
{
    struct trs_core_ts_inst *ts_inst = NULL;

    ts_inst = trs_core_inst_get(devid, 0);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u)\n", devid);
        return false;
    }

    if ((ts_inst->featur_mode == TRS_INST_PART_FEATUR_MODE) && (!trs_is_id_query_cmd(cmd))) {
        trs_core_inst_put(ts_inst);
        trs_err("Unsupported command. (devid=%u; cmd=%u)\n", devid, _IOC_NR(cmd));
        return false;
    }

    trs_core_inst_put(ts_inst);
    return true;
}

static long trs_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct trs_proc_ctx *proc_ctx = file->private_data;
    int cmd_nr = _IOC_NR(cmd);
    if ((cmd_nr < 0) || (cmd_nr >= TRS_MAX_CMD) || (trs_ioctl_handles[cmd_nr] == NULL)) {
        trs_err("Unsupported command. (cmd_nr=%d)\n", cmd_nr);
        return -EINVAL;
    }

    if ((proc_ctx == NULL) || (proc_ctx->status != TRS_PROC_STATUS_NORMAL)) {
        trs_err("Proc is exiting. (cmd_nr=%d; status=%d)\n", cmd_nr, (proc_ctx == NULL) ? -1 : proc_ctx->status);
        return -EINVAL;
    }

    if (!trs_ioctl_cmd_is_support(proc_ctx->devid, (int)cmd)) {
        trs_err("Unsupported command. (devid=%u; cmd_nr=%d)\n", proc_ctx->devid, cmd_nr);
        return -EINVAL;
    }

    return (long)trs_ioctl_handles[cmd_nr](proc_ctx, cmd, arg);
}

int devdrv_manager_container_check_devid_in_container_ns(u32 devid, struct task_struct *tsk);
static int trs_open(struct inode *inode, struct file *file)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    struct trs_proc_ctx *proc_ctx = NULL;
    u32 devid = drv_davinci_get_device_id(file);
    int ret;

    file->private_data = NULL;

    if (davinci_intf_confirm_user() == false) {
        trs_err("The user is not allowed to open. (devid=%u)\n", devid);
        return -ENODEV;
    }

    ret = devdrv_manager_container_check_devid_in_container_ns(devid, current);
    if (ret != 0) {
        trs_err("Device is not in container. (devid=%u)\n", devid);
        return -EINVAL;
    }

    ts_inst = trs_core_inst_get(devid, 0);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u)\n", devid);
        return -EINVAL;
    }

    /* When core ts inst is obtained, the module reference counting of ops must be added. */
    if (!try_module_get(ts_inst->ops.owner)) {
#ifndef EMU_ST
        trs_core_inst_put(ts_inst);
        trs_err("Get trs core module failed. (devid=%u)\n", devid);
        return -EINVAL;
#endif
    }

    /*
     * When a process is reclaiming resources,
     * Prevent starting processes with the same pid.
     */
    ret = trs_proc_wait_for_exit(ts_inst, current->tgid);
    if (ret != 0) {
        module_put(ts_inst->ops.owner);
        trs_core_inst_put(ts_inst);
        trs_err("Wait for proc exit fail. (devid=%u; pid=%d; ret=%d)\n", devid, current->tgid, ret);
        return ret;
    }

    down_write(&ts_inst->sem);
    proc_ctx = trs_proc_ctx_find(ts_inst, current->tgid);
    if (proc_ctx != NULL) {
        up_write(&ts_inst->sem);
        module_put(ts_inst->ops.owner);
        trs_core_inst_put(ts_inst);
        trs_err("Repeat init. (devid=%u; pid=%d)\n", devid, current->tgid);
        return -EEXIST;
    }
    proc_ctx = trs_proc_ctx_create(ts_inst);
    if (proc_ctx == NULL) {
        up_write(&ts_inst->sem);
        module_put(ts_inst->ops.owner);
        trs_core_inst_put(ts_inst);
        trs_err("Proc ctx create failed. (devid=%u)\n", devid);
        return -ENOMEM;
    }

    if (ts_inst->ops.proc_bind_smmu != NULL) {
        ret = ts_inst->ops.proc_bind_smmu(&ts_inst->inst);
        if (ret != 0) {
            up_write(&ts_inst->sem);
            trs_proc_ctx_destroy(proc_ctx);
            module_put(ts_inst->ops.owner);
            trs_core_inst_put(ts_inst);
            trs_err("Proc bind smmu failed. (devid=%u)\n", devid);
            return ret;
        }
    }

    list_add_tail(&proc_ctx->node, &ts_inst->proc_list_head);
    up_write(&ts_inst->sem);
    trs_core_inst_put(ts_inst);

    file->private_data = proc_ctx;

    trs_debug("Proc open success. (devid=%u; task_id=%u)\n", devid, proc_ctx->task_id);

    return 0;
}

static void trs_try_flush_id_to_pool(struct trs_core_ts_inst *ts_inst)
{
    if (!trs_still_has_proc(ts_inst)) {
        trs_info("Flush id to pool. (devid=%u; tsid=%u)\n", ts_inst->inst.devid, ts_inst->inst.tsid);
        (void)trs_id_flush_to_pool(&ts_inst->inst);
    }
}

static int trs_release(struct inode *inode, struct file *file)
{
    struct trs_proc_ctx *proc_ctx = file->private_data;
    struct trs_core_ts_inst *ts_inst = NULL;
    int ret = 0;

    if (proc_ctx == NULL) { /* ioctl open fail */
        return 0;
    }

    file->private_data = NULL;

    trs_debug("Proc release. (pid=%d; task_id=%u)\n", proc_ctx->pid, proc_ctx->task_id);

    ts_inst = trs_core_inst_get(proc_ctx->devid, 0);
    if (ts_inst != NULL) {
        ret = trs_proc_release(proc_ctx);
        if (ret != 0) {
            ret = trs_proc_recycle(proc_ctx);
        }

        if (ret == 0) {
            down_write(&ts_inst->sem);
            list_del(&proc_ctx->node);
            trs_try_flush_id_to_pool(ts_inst);
            up_write(&ts_inst->sem);
        } else {
            down_write(&ts_inst->sem);
            list_move(&proc_ctx->node, &ts_inst->exit_proc_list_head);
            up_write(&ts_inst->sem);
        }
        module_put(ts_inst->ops.owner);
        trs_core_inst_put(ts_inst);
    }

    if (ret == 0) {
        trs_debug("Proc destroy. (pid=%d; task_id=%u)\n", proc_ctx->pid, proc_ctx->task_id);
        trs_proc_ctx_destroy(proc_ctx);
    }

    return 0;
}

static int trs_mmap(struct file *file, struct vm_area_struct *vma)
{
    vm_flags_set(vma, VM_DONTEXPAND);
    vm_flags_set(vma, VM_LOCKED);

    vma->vm_private_data = file->private_data;

    return 0;
}

static struct file_operations trs_fops = {
    .owner = THIS_MODULE,
    .open = trs_open,
    .release = trs_release,
    .unlocked_ioctl = trs_ioctl,
    .mmap = trs_mmap,
};

static int trs_release_prepare(struct file *file, unsigned long mode)
{
    struct trs_proc_ctx *proc_ctx = file->private_data;
    struct trs_core_ts_inst *ts_inst = NULL;

    if (proc_ctx == NULL) { /* ioctl open fail */
        return 0;
    }

    /* unbind smmu should do in proc ctx. */
    ts_inst = trs_core_inst_get(proc_ctx->devid, 0);
    if (ts_inst != NULL) {
        if (ts_inst->ops.proc_unbind_smmu != NULL) {
            ts_inst->ops.proc_unbind_smmu(&ts_inst->inst);
        }
        trs_core_inst_put(ts_inst);
    }

    trs_debug("Proc release prepare. (devid=%u; task_id=%u)\n", proc_ctx->devid, proc_ctx->task_id);
    return 0;
}

const struct notifier_operations trs_notifier_ops = {
    .notifier_call =  trs_release_prepare,
};

#define TRS_MODULE_NAME "TSDRV"
int __init trs_core_init_module(void)
{
    int ret;

    ret = drv_davinci_register_sub_parallel_module(TRS_MODULE_NAME, &trs_fops);
    if (ret != 0) {
        trs_err("Register sub module fail. (ret=%d)\n", ret);
        return ret;
    }

    ret = drv_ascend_register_notify(TRS_MODULE_NAME, &trs_notifier_ops);
    if (ret != 0) {
        (void)drv_ascend_unregister_sub_module(TRS_MODULE_NAME);
        trs_err("Register notify fail. (ret=%d)\n", ret);
        return ret;
    }

    trs_proc_fs_init();
    trs_core_inst_init();

    return 0;
}

void __exit trs_core_exit_module(void)
{
    trs_core_inst_uninit();
    trs_proc_fs_uninit();
    (void)drv_ascend_unregister_sub_module(TRS_MODULE_NAME);
}

module_init(trs_core_init_module);
module_exit(trs_core_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("TRS CORE DRIVER");

