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


#include "devdrv_manager.h"
#include "devdrv_manager_time.h"
#include "devdrv_manager_freq.h"
#include "devdrv_ipc_msg.h"
#include "devdrv_ipc.h"
#include "devdrv_dfm.h"
#include "icm_interface.h"
#include "tsmng_interface.h"
#include <linux/delay.h>
#include <linux/types.h>
#include "dms_kernel_version_adapt.h"
#include "dms_device_time_zone.h"
#include "ascend_platform.h"

#if defined(CFG_FEATURE_LP_ENABLE)
#include "bbox_proxy.h"
#include "device/bbox_pub.h"
#include "device/bbox_pub_cloud.h"

#ifdef CFG_SOC_PLATFORM_MDC_V51
#include <linux/kallsyms.h>
#define LP_FAULT_STATE_MASK_8BIT 0xFF
struct mutex si_mutex[MAX_CHIP_NUM];
#endif
#endif
struct devdrv_ipc_cmd_data imu_dmp_data[MAX_CHIP_NUM];
struct mutex imu_mutex[MAX_CHIP_NUM];
struct notifier_block mbox_lp_nb;
struct notifier_block mbox_si_nb;


#ifdef CFG_FEATURE_IMU_ENABLE
STATIC void devdrv_ipc_move_data(u8 dst[], u8 dst_len, u8 src[], u8 src_len)
{
    u8 i;

    if (dst_len >= src_len) {
        for (i = 0; i < src_len; i++)
            dst[i] = src[i];
    }
}

STATIC void devdrv_imu_dmp_msg_rx(struct devdrv_ipc_cmd_data *cmd_data, u8 *msg, u8 len)
{
    struct cmd_imu_dmp_msg *ipc_msg = (struct cmd_imu_dmp_msg *)msg;

    if (((ipc_msg->offset != 0) && (ipc_msg->offset != CMD_DMP_MSG_MAX_LEN)) ||
        (ipc_msg->offset + ipc_msg->length > MAX_IPCDRV_MSG_LENGTH)) {
        devdrv_drv_err("ipc msg offset %d len %d error.\r\n", ipc_msg->offset, ipc_msg->length);
        return;
    }

    if (len < (ipc_msg->length + CMD_DMP_MSG_HEAD_LEN)) {
        devdrv_drv_err("ipc msg len %d len %d error.\r\n", len, ipc_msg->length);
        return;
    }

    if (ipc_msg->offset == 0) {
        devdrv_ipc_move_data(cmd_data->data, MAX_IPCDRV_MSG_LENGTH, ipc_msg->msg, ipc_msg->length);
    } else {
        devdrv_ipc_move_data(cmd_data->data + ipc_msg->offset, MAX_IPCDRV_MSG_LENGTH - ipc_msg->offset, ipc_msg->msg,
                             ipc_msg->length);
    }

    if (ipc_msg->finish == 1) {
        cmd_data->len = ipc_msg->offset + ipc_msg->length;
        cmd_data->seq = ipc_msg->seq;
        cmd_data->valid = 1;
#ifndef DEVMNG_UT
        devdrv_manage_wake_up_msg_poll();
#endif
    }
}
#endif

int devdrv_ipc_msg_check(void)
{
#if defined(CFG_FEATURE_IMU_ENABLE)
    struct devdrv_ipc_cmd_data *cmd_data = NULL;
    int i;

    for (i = 0; i < MAX_CHIP_NUM; i++) {
        cmd_data = &imu_dmp_data[i];
        if (cmd_data->valid == 1)
            return 1;
    }
#endif
    return 0;
}


#ifdef CFG_FEATURE_IPC_FORMAT_V2_CRC
STATIC int devdrv_imu_mbx0_notifier(u32 dev_id, unsigned long len, void *data)
{
    struct icmdrv_ipc_msg_info *msg = NULL;
    struct devdrv_info *info = NULL;
    u8 type;

    info = dev_manager_info->dev_info[dev_id];
    msg = (struct icmdrv_ipc_msg_info *)data;

    devdrv_drv_debug("Receive data. (len=%ld; sub_cmd=%2x; cmd=%2x; cmd_dest=%2x; cmd_src=%2x; data=%u)\n",
        len, msg->sub_cmd, msg->cmd, msg->cmd_dest, msg->cmd_src, msg->data[0]);

    type = msg->cmd;
    switch (type) {
        case ICM_MAIN_CMD_LP_HB:
            break;

        default:
            devdrv_drv_err("Don't supportted imu ipc message command. (type=%d)\n", type);
            break;
    }
    return 0;
}

STATIC int devdrv_imu_mbx1_notifier(u32 dev_id, unsigned long len, void *data)
{
    struct icmdrv_ipc_msg_info *msg = NULL;
    u8 msg_len = (u8)(sizeof(rproc_msg_t) * len);

    msg = (struct icmdrv_ipc_msg_info *)data;

    devdrv_drv_debug("Receive data. (len=%d; cmd=%2x; sub_cmd=%2x; target_id=%2x; source_id=%2x)\n",
        msg_len, msg->cmd, msg->sub_cmd, msg->cmd_dest, msg->cmd_src);
    devdrv_imu_dmp_msg_rx(&imu_dmp_data[dev_id], msg->data, (u8)(msg_len - MAX_IPCDRV_MSG_HEAD));

    return 0;
}

typedef struct imu_exception_data {
    u32 exception_id;
    u8 timestamp_sec[8];  /* 8 bytes for second */
    u8 timestamp_usec[8]; /* 8 bytes for microsecond */
} imu_exception_data_t;

STATIC int devdrv_imu_mbx3_notifier(u32 dev_id, unsigned long len, void *data)
{
    struct icmdrv_ipc_msg_info *msg = NULL;
    u32 exception_id;
    excep_time timestamp;
    imu_exception_data_t *exception_data;

    msg = (struct icmdrv_ipc_msg_info *)data;

    devdrv_drv_debug("Receive data. (len=%ld; sub_cmd=%2x; cmd=%2x; cmd_dest=%2x; cmd_src=%2x; exception_id=0x%x)\n",
        len, msg->sub_cmd, msg->cmd, msg->cmd_dest, msg->cmd_src, *(u32 *)msg->data);

    exception_data = (imu_exception_data_t *)msg->data;
    exception_id = exception_data->exception_id;
    timestamp.tv_sec = *((u64 *)exception_data->timestamp_sec);
    timestamp.tv_usec = *((u64 *)exception_data->timestamp_usec);
    bbox_system_error(dev_id, exception_id, &timestamp, 0);
    devdrv_drv_err("Get mntn message. (exception_id=0x%x; tv_sec=%llu; tv_usec=%llu)\n",
        exception_id, timestamp.tv_sec, timestamp.tv_usec);

    return 0;
}

STATIC int devdrv_imu_mbx_notifier(struct notifier_block *nb, unsigned long len, void *data)
{
    struct icmdrv_msg_chan *chan_info = NULL;
    struct icmdrv_ipc_msg_info *msg = NULL;
    u8 msg_len = (u8)(sizeof(rproc_msg_t) * len);
    u32 dev_id;
    unsigned short crc16_value;
    int ret = 0;

    if ((nb == NULL) || (data == NULL)) {
        devdrv_drv_err("Invalid parameter, value is NULL.\n");
        return 0;
    }
    if (msg_len > sizeof(struct icmdrv_ipc_msg_info)) {
        devdrv_drv_err("Ipc message length error. (msg_len=%u)\n", msg_len);
        return 0;
    }
    chan_info = container_of(nb, struct icmdrv_msg_chan, icm_monitor);
    if (chan_info == NULL || chan_info->magic != ICM_MAGIC_WORD) {
        return 0;
    }
    dev_id = chan_info->dev_id;
    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("Device does not exist.\n");
        return 0;
    }

    msg = (struct icmdrv_ipc_msg_info *)data;
    crc16_value = sils_crc16((u8 *)msg, msg->len + ICM_IPC_MSG_HEAD_LEN);
    if (crc16_value != msg->crc16) {
        devdrv_drv_err("Crc16 check failed. (cmd_src=%d; cmd_dest=%d).\n", msg->cmd_src, msg->cmd_dest);
        return 0;
    }

    if (msg->cmd_src == OBJ_CMD_LP && msg->cmd_dest == OBJ_CMD_CCPU) {
        if (msg->cmd == ICM_MAIN_CMD_LP_HB) {
            ret = devdrv_imu_mbx0_notifier(dev_id, len, data);
        } else if ((msg->cmd == ICM_MAIN_CMD_CCPU_NOTIFY) && (msg->sub_cmd == ICM_SUB_CMD_HEALTH_EVENT)) {
            ret = devdrv_imu_mbx3_notifier(dev_id, len, data);
        } else if (msg->cmd == ICM_MAIN_CMD_CCPU_DMP) {
            ret = devdrv_imu_mbx1_notifier(dev_id, len, data);
        } else {
            devdrv_drv_err("Icm msg from imu not valid. (cmd=%u; sub_cmd=%u)\n", msg->cmd, msg->sub_cmd);
            return -EINVAL;
        }
    } else {
        devdrv_drv_err("Ipc message not from IMU. (cmd_src=%d; cmd_dest=%d)\n", msg->cmd_src, msg->cmd_dest);
    }

    return ret;
}
#endif

#if (defined CFG_FEATURE_IMU_ENABLE)
#ifndef CFG_FEATURE_IPC_FORMAT_V2_CRC
STATIC int devdrv_imu_mbx0_notifier(u32 dev_id, unsigned long len, void *data)
{
    struct devdrv_ipc_imu *msg = NULL;
    struct devdrv_info *info = NULL;
    u8 type;

    info = dev_manager_info->dev_info[dev_id];
    msg = (struct devdrv_ipc_imu *)data;

    devdrv_drv_debug("recve data(%lu):%2x %2x %2x %2x %2x %2x %2x %2x\n", len, msg->cmd_type1, msg->cmd_type0,
                     msg->target_id, msg->source_id, msg->cmd_para0, msg->cmd_para1, msg->cmd_para2, msg->cmd_para3);

    type = msg->cmd_type0;
    switch (type) {
        case IPC_IMU_HEART_BEAT:
            break;

        default:
            devdrv_drv_err("it's not a supportted imu ipc msg command type: %d.\n", type);
            break;
    }
    return 0;
}

STATIC int devdrv_imu_mbx1_notifier(unsigned long len, u32 dev_id, void *data)
{
    struct devdrv_ipc_cmd *msg = NULL;
    u8 msg_len = (u8)(sizeof(rproc_msg_t) * len);

    msg = (struct devdrv_ipc_cmd *)data;

    devdrv_drv_debug("recve data(%d):%2x %2x %2x %2x\n", msg_len, msg->cmd, msg->cmd_sub, msg->target_id,
                    msg->source_id);
    switch (msg->cmd) {
        case CMD_IMU_DMP_MSG:
            devdrv_imu_dmp_msg_rx(&imu_dmp_data[dev_id], msg->cmdPara0, (u8)(msg_len - MAX_IPCDRV_MSG_HEAD));
            break;

        default:
            devdrv_drv_err("imu ipc msg "
                           "command type: %d cmd_sub %d.\n",
                           msg->cmd, msg->cmd_sub);
            break;
    }
    return 0;
}

STATIC int devdrv_imu_mbx3_notifier(u32 dev_id, unsigned long len, void *data)
{
    struct devdrv_ipc_imu *msg = NULL;
    struct timespec64 interval;
    u32 excep_id;
    excep_time timestamp;

    msg = (struct devdrv_ipc_imu *)data;

    devdrv_drv_debug("recve data(%lu):%2x %2x %2x %2x %2x %2x %2x %2x\n", len, msg->cmd_type1, msg->cmd_type0,
                     msg->target_id, msg->source_id, msg->cmd_para0, msg->cmd_para1, msg->cmd_para2, msg->cmd_para3);

    switch (msg->cmd_type0) {
        case CMD_NOTIFY:
        case CMD_IMP_ERRCODE:
            dms_get_time_interval(&interval);
            excep_id = (((u32)msg->cmd_para3) << 24) | (((u32)msg->cmd_para2) << 16) | (((u32)msg->cmd_para1) << 8) |
                       (msg->cmd_para0);

            timestamp.tv_sec = (((u64)msg->data1) << 32) |
                               ((((u64)msg->cmd_data3) << 24) | (((u64)msg->cmd_data2) << 16) |
                                (((u64)msg->cmd_data1) << 8) | (msg->cmd_data0));
            timestamp.tv_usec = (((u64)msg->data3) << 32) | (msg->data2);
            if (timestamp.tv_sec >= interval.tv_sec) {
                timestamp.tv_sec -= interval.tv_sec;
            } else {
                devdrv_drv_warn("Timestamp is not synchronize.\n");
            }

#ifndef CFG_FEATURE_LP_ENABLE
            bbox_system_error(dev_id, excep_id, &timestamp, 0);
#endif /* CFG_FEATURE_LP_ENABLE */
            devdrv_drv_err("drv get mntn message, excep_id = 0x%x,"
                           "tv_sec = %llu, tv_usec = %llu.\n",
                           excep_id, timestamp.tv_sec, timestamp.tv_usec);
            break;
    }

    return 0;
}

STATIC int devdrv_imu_mbx_notifier(struct notifier_block *nb, unsigned long len, void *data)
{
    struct icmdrv_msg_chan *chan_info = NULL;
    struct devdrv_ipc_imu *msg = NULL;
    u8 msg_len = (u8)(sizeof(rproc_msg_t) * len);
    u32 dev_id;
    int ret = 0;

    if ((nb == NULL) || (data == NULL)) {
        devdrv_drv_err("data NULL=%d; nb NULL=%d\n", (data==NULL), (nb==NULL));
        return 0;
    }
    if (msg_len > MAX_IPCDRV_MSG_LENGTH) {
        devdrv_drv_err("ipc msg len %d error.\n", msg_len);
        return 0;
    }
    chan_info = container_of(nb, struct icmdrv_msg_chan, icm_monitor);
    if (chan_info == NULL || chan_info->magic != ICM_MAGIC_WORD) {
        return 0;
    }
    dev_id = chan_info->dev_id;
    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("device does not exist.\n");
        return 0;
    }

    msg = (struct devdrv_ipc_imu *)data;
    if (msg->source_id == OBJ_IMU && msg->target_id == OBJ_AP) {
        ret = devdrv_imu_mbx0_notifier(dev_id, len, data);
    } else if (msg->source_id == OBJ_IMU_DMP && msg->target_id == OBJ_AP_DMP) {
        ret = devdrv_imu_mbx1_notifier(len, dev_id, data);
    } else if (msg->source_id == OBJ_IMU_MNTN && msg->target_id == OBJ_AP_MNTN) {
        ret = devdrv_imu_mbx3_notifier(dev_id, len, data);
    } else {
        devdrv_drv_err("ipc msg not from IMU, source_id (%d), target_id(%d).\n", msg->source_id, msg->target_id);
    }

    return ret;
}
#endif /* #ifndef CFG_FEATURE_IPC_FORMAT_V2_CRC */
#endif /* #ifdef CFG_FEATURE_IMU_ENABLE */

#if defined(CFG_FEATURE_LP_ENABLE)
#define LP_SOURCE_ID 4
#define LP_TARGET_ID 0

STATIC void devdrv_lp_get_exception_id(const struct devdrv_ipc_imu *msg, u32 *exception_id)
{
    *exception_id = (((u32)msg->cmd_para3) << IPC_PARA_OFFSET_24BIT) |
                (((u32)msg->cmd_para2) << IPC_PARA_OFFSET_16BIT) |
                (((u32)msg->cmd_para1) << IPC_PARA_OFFSET_8BIT) | (msg->cmd_para0);

    return;
}

STATIC void devdrv_lp_get_timestamp(const struct devdrv_ipc_imu *msg, excep_time *timestamp, time64_t interval_sec)
{
    timestamp->tv_sec = (((u64)msg->data1) << IPC_PARA_OFFSET_32BIT) |
        ((((u64)msg->cmd_data3) << IPC_PARA_OFFSET_24BIT) | (((u64)msg->cmd_data2) << IPC_PARA_OFFSET_16BIT) |
        (((u64)msg->cmd_data1) << IPC_PARA_OFFSET_8BIT) | (msg->cmd_data0));
    timestamp->tv_usec = (((u64)msg->data3) << IPC_PARA_OFFSET_32BIT) | (msg->data2);
#if (!defined(CFG_SOC_PLATFORM_MDC_V51)) && (!defined(CFG_SOC_PLATFORM_MDC_V11))
    if (timestamp->tv_sec >= (u64)interval_sec) {
        timestamp->tv_sec -= (u64)interval_sec;
    } else {
        devdrv_drv_warn("Timestamp is not synchronize.\n");
    }
#endif
    return;
}

STATIC int devdrv_lp_mbx2_notifier(u32 dev_id, unsigned long len, void *data)
{
    struct devdrv_ipc_imu *msg = NULL;
    struct timespec64 interval;
    excep_time timestamp;
    u32 exception_id;

    msg = (struct devdrv_ipc_imu *)data;

    devdrv_drv_debug("recve data(%ld):%2x %2x %2x %2x %2x %2x %2x %2x\n",
        len, msg->cmd_type1, msg->cmd_type0, msg->target_id, msg->source_id, msg->cmd_para0,
        msg->cmd_para1, msg->cmd_para2, msg->cmd_para3);

    if (msg->cmd_type1 == SUB_CMD_HEALTH_EVENT) {
        dms_get_time_interval(&interval);
        devdrv_lp_get_exception_id(msg, &exception_id);
        devdrv_lp_get_timestamp(msg, &timestamp, interval.tv_sec);
        bbox_system_error(dev_id, exception_id, &timestamp, 0);
        devdrv_drv_err("drv get mntn message, exception_id = 0x%x, tv_sec = %lld, tv_usec = %lld.\n",
            exception_id, timestamp.tv_sec, timestamp.tv_usec);
    }

    return 0;
}

int devdrv_lp_heart_beat_msg(u32 dev_id, unsigned long len, void *data)
{
    struct devdrv_ipc_imu *msg = NULL;
    struct devdrv_info *dev_info = NULL;
    u8 type;

    dev_info = dev_manager_info->dev_info[dev_id];
    msg = (struct devdrv_ipc_imu *)data;

    type = msg->cmd_type1;
    if (type != SUB_CMD_HEART) {
        devdrv_drv_err("Unkonwn cmd type, type(%d)\n", type);
        return -EINVAL;
    }
    return 0;
}

STATIC int devdrv_lp_notifier(struct notifier_block *nb, unsigned long len, void *data)
{
    struct icmdrv_msg_chan *chan_info = NULL;
    struct devdrv_ipc_imu *msg = NULL;
    u8 msg_len = (u8)(sizeof(rproc_msg_t) * len);
    u32 dev_id;
    int ret = 0;

    if ((nb == NULL) || (data == NULL)) {
        devdrv_drv_err("data NULL=%d; nb NULL=%d\n", (data==NULL), (nb==NULL));
        return 0;
    }
    if (msg_len > MAX_IPCDRV_MSG_LENGTH) {
        devdrv_drv_err("ipc msg len %d error.\n", msg_len);
        return 0;
    }
    chan_info = container_of(nb, struct icmdrv_msg_chan, icm_monitor);
    if (chan_info == NULL || chan_info->magic != ICM_MAGIC_WORD) {
        return 0;
    }
    dev_id = chan_info->dev_id;
    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("device does not exist.\n");
        return 0;
    }

    msg = (struct devdrv_ipc_imu *)data;
    if (msg->source_id == OBJ_IMU && msg->target_id == 0 &&
        chan_info->chan_id == HISI_RPROC_LP_Q_RX_RPID1) {
        ret = devdrv_lp_heart_beat_msg(dev_id, len, data);
    } else if (msg->source_id == LP_SOURCE_ID && msg->target_id == LP_TARGET_ID &&
        chan_info->chan_id == HISI_RPROC_LP_Q_RX_RPID2) {
        ret = devdrv_lp_mbx2_notifier(dev_id, len, data);
    } else {
        devdrv_drv_err("ipc msg not from lp, source_id (%d), target_id(%d).\n", msg->source_id, msg->target_id);
    }

    return ret;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC int devdrv_sil_mbx2_bbox_notifier_para_check(struct notifier_block *nb, unsigned long len, void *data)
{
    const int IPC_MAX_DATA_LEN = 8;

    if (nb == NULL) {
        devdrv_drv_err("sil bbox notifier nb is NULL\n");
        return -EINVAL;
    }
    if (len > IPC_MAX_DATA_LEN) {
        devdrv_drv_err("sil bbox, recv ipc_msg_len %lu err\n", len);
        return -EINVAL;
    }
    if (data == NULL) {
        devdrv_drv_err("sil bbox, ipc data is NULL\n");
        return -EINVAL;
    }
    return 0;
}

#define DEVDRV_BBOX_SIL_DDR_BASE 0x30FC0000
#define DEVDRV_BBOX_SIL_DDR_SIZE 0x50000
#define DEVDRV_BBOX_SIL_SRAM_BASE 0xC6F2F500
#define DEVDRV_BBOX_SIL_SRAM_SIZE 0x50000
#define DEVDRV_SIL_SRAM_MAGIC 0x13141314

struct devdrv_sil_bbox_log_info {
    u32 silBboxMagic;       /* safety magic num */
    u32 silBboxTail;        /* bbox log tail point */
    u32 silBboxBlockPre;    /* taishan is moveing bbox from sram to ddr */
    u32 silBboxBlockPost;   /* taishan has moved bbox from sram to ddr */
    u32 excepId;            /* 异常id */
    u32 processPriority;    /* 异常处理级别 */
    u32 excepType;          /* 异常类型 */
    unsigned long long int timeStamp; /* bbox timeStamp */
    char data[0];
};

struct devdrv_sil_bbox_ddr_info {
    struct bbox_proxy_module_ctrl ctrl;
};

struct devdrv_sil_bbox_ddr_info *g_sil_d_info = NULL;
struct devdrv_sil_bbox_log_info *g_sil_s_info = NULL;

STATIC int devdrv_sil_bbox_init(u32 dev_id, u32 mid)
{
    if (dev_id != 0) {
        return 0;
    }

    mutex_lock(&si_mutex[dev_id]);
    if ((g_sil_d_info == NULL) && (g_sil_s_info == NULL)) {
        g_sil_d_info = (struct devdrv_sil_bbox_ddr_info *)ioremap(DEVDRV_BBOX_SIL_DDR_BASE, DEVDRV_BBOX_SIL_DDR_SIZE);
        if (g_sil_d_info == NULL) {
            mutex_unlock(&si_mutex[dev_id]);
            devdrv_drv_err("sil bbox ddr ioremap err\n");
            return -ENOMEM;
        }
        g_sil_s_info = (struct devdrv_sil_bbox_log_info *)ioremap(DEVDRV_BBOX_SIL_SRAM_BASE, DEVDRV_BBOX_SIL_SRAM_SIZE);
        if (g_sil_s_info == NULL) {
            iounmap(g_sil_d_info);
            g_sil_d_info = NULL;
            mutex_unlock(&si_mutex[dev_id]);
            devdrv_drv_err("sil bbox sram ioremap err\n");
            return -ENOMEM;
        }
    }

    mutex_unlock(&si_mutex[dev_id]);
    return 0;
}

STATIC void devdrv_sil_bbox_uninit(u32 dev_id, u32 mid)
{
    if (dev_id == 0) {
        mutex_lock(&si_mutex[dev_id]);
        if (g_sil_s_info != NULL) {
            iounmap(g_sil_s_info);
            g_sil_s_info = NULL;
        }
        if (g_sil_d_info != NULL) {
            iounmap(g_sil_d_info);
            g_sil_d_info = NULL;
        }
        mutex_unlock(&si_mutex[dev_id]);
    }
}

STATIC int devdrv_sil_bbox_check_dump_status(const struct devdrv_sil_bbox_ddr_info *d_info,
    const struct devdrv_sil_bbox_log_info *s_info, struct mutex *mutex)
{
    const int SIL_SRAM_HAVE_DATA = 1;
    int i;

    mutex_lock(mutex);
    if (s_info->silBboxBlockPost != SIL_SRAM_HAVE_DATA) {
        mutex_unlock(mutex);
        devdrv_drv_err("sil sram have no data\n");
        return -EINVAL;
    }

    for (i = 0; i < BBOX_PROXY_CTRL_NUM; i++) {
        if ((d_info->ctrl.block[i].e_dump_status == PROXY_STATUS_INIT) &&
            (d_info->ctrl.block[i].e_save_status == PROXY_STATUS_INIT)) {
            goto OUT;
        }
    }
    mutex_unlock(mutex);
    devdrv_drv_err("can't find a block to save...\n");
    return -EINVAL;

OUT:
    mutex_unlock(mutex);
    return i;
}

STATIC int devdrv_sil_bbox_dump(struct devdrv_sil_bbox_ddr_info *d_info, struct devdrv_sil_bbox_log_info *s_info,
    int blk_idx, struct mutex *mutex)
{
    struct bbox_proxy_exception_ctrl *e_ctrl = &(d_info->ctrl.block[blk_idx]);
    struct bbox_proxy_block_info *e_blk = &(d_info->ctrl.config.block_info[blk_idx]);
    struct bbox_report_info info;
    const u32 SIL_DUMP_RELEASE = 0;
    const u32 SIL_DUMP_OCCUPIED = 1;
    struct timeval tv;
    size_t len;
    int ret;

    do_gettimeofday(&tv);

    mutex_lock(mutex);
    /* occupy sram */
    s_info->silBboxBlockPre = SIL_DUMP_OCCUPIED;

    /* occupy ddr */
    e_ctrl->e_dump_status = PROXY_STATUS_DOING;

    e_ctrl->e_clock.tv_sec = tv.tv_sec;
    e_ctrl->e_clock.tv_usec = tv.tv_usec;
    e_ctrl->e_main_excepid = s_info->excepId;
    e_ctrl->e_info_offset = e_blk->info_offset;

    /* data copy */
    len = s_info->silBboxTail;  /* note: tail is length of data */
    if ((e_blk->info_offset > DEVDRV_BBOX_SIL_DDR_SIZE) ||
        (e_blk->info_block_len > DEVDRV_BBOX_SIL_DDR_SIZE) ||
        ((e_blk->info_offset + e_blk->info_block_len) > DEVDRV_BBOX_SIL_DDR_SIZE) ||
        (len > e_blk->info_block_len)) {
        mutex_unlock(mutex);
        devdrv_drv_err("e_blek %u err\n", e_blk->info_block_len);
        return -EINVAL;
    }

    ret = memcpy_s((void *)((uintptr_t)d_info + e_blk->info_offset), e_blk->info_block_len, s_info->data, len);
    if (ret != 0) {
        mutex_unlock(mutex);
        devdrv_drv_err("memcpy failed, ret(%d)\n", ret);
        return ret;
    }
    e_ctrl->e_info_len = s_info->silBboxTail;

    /* release sram */
    s_info->silBboxBlockPre = SIL_DUMP_RELEASE;

    info.devid = 0;
    info.excepid = s_info->excepId;
    info.time.tv_usec = e_ctrl->e_clock.tv_usec;
    info.time.tv_sec = e_ctrl->e_clock.tv_sec;
    info.arg = 0;

    /* wait for bbox to dump log. when dump done, bbox set dump_status to init */
    e_ctrl->e_dump_status = PROXY_STATUS_DONE;

    wmb();
    /* report exception to bbox */
    (void)bbox_exception_report(&info);

    s_info->silBboxBlockPost = SIL_DUMP_RELEASE;
    mutex_unlock(mutex);

    return 0;
}

STATIC int devdrv_sil_copy_exception_to_bbox_ddr(struct devdrv_sil_bbox_ddr_info *d_info,
    struct devdrv_sil_bbox_log_info *s_info)
{
    int d_dump_blk_idx;
    struct mutex *mutex = NULL;
    int ret;

    mutex = &si_mutex[0];
    if ((d_info == NULL) || (s_info == NULL)) {
        devdrv_drv_err("sil bbox memory not ready\n");
        ret = -ENOMEM;
        goto OUT;
    }

    if (d_info->ctrl.magic != BBOX_PROXY_MAGIC) {
        devdrv_drv_err("sil bbox ddr not match\n");
        ret = -EINVAL;
        goto OUT;
    }
    if (s_info->silBboxMagic != DEVDRV_SIL_SRAM_MAGIC) {
        devdrv_drv_err("sil bbox sram not match\n");
        ret = -EINVAL;
        goto OUT;
    }

    /* wait sil post 1, and bbox save status: init, dump status: init */
    d_dump_blk_idx = devdrv_sil_bbox_check_dump_status(d_info, s_info, mutex);
    if (d_dump_blk_idx < 0) {
        devdrv_drv_err("sil bbox wait dump status timeout...\n");
        ret = -EINVAL;
        goto OUT;
    }

    /* set dump head, and copy data */
    ret = devdrv_sil_bbox_dump(d_info, s_info, d_dump_blk_idx, mutex);
    if (ret != 0) {
        devdrv_drv_err("sil bbox dump err, ret(%d)\n", ret);
        ret = -EINVAL;
    }

OUT:
    return ret;
}

#define RESPONSE_FLAG 1
#define DATA_LEN 1
#define DEV_ID 0
STATIC int devdrv_sil_reply_bbox(const struct devdrv_ipc_sil_bbox_msg *recv_msg)
{
    struct devdrv_ipc_sil_bbox_msg msg = {0};
    int ret;
    int rproc_id;

    msg.cmdType = recv_msg->cmdType;
    msg.cmd = recv_msg->cmd;
    msg.destAddr = recv_msg->srcAddr;
    msg.srcAddr = recv_msg->destAddr;
    msg.reqSeq = recv_msg->reqSeq;
    msg.dataType = RESPONSE_FLAG;
    msg.len = DATA_LEN;
    msg.crcVal = sils_crc16((unsigned char *)&msg, msg.len + SILS_IPC_HEAD_LEN);

#ifdef AOS_LLVM_BUILD
    rproc_id = HISI_RPROC_SI_Q_TX_RPID17;
#else
    rproc_id = HISI_RPROC_SI_Q_TX_RPID15;
#endif

    ret = icm_msg_send_async(ICM_FD_BUILD(DEV_ID, rproc_id), (rproc_msg_t *)&msg,
        sizeof(msg) / sizeof(rproc_msg_len_t));
    if (ret != 0) {
        devdrv_drv_err("sil bbox reply to sil err..., ret(%d)\n", ret);
    }
    return ret;
}

STATIC int devdrv_sil_mbx2_bbox_notifier(struct notifier_block *nb, unsigned long len, void *data)
{
    struct devdrv_ipc_sil_bbox_msg *msg = NULL;
    int ret;

    ret = devdrv_sil_mbx2_bbox_notifier_para_check(nb, len, data);
    if (ret != 0) {
        devdrv_drv_err("sil bbox, ipc msg para check err. ret(%d)\n", ret);
        return ret;
    }

    msg = (struct devdrv_ipc_sil_bbox_msg *)data;

    /* reply to sil, otherwise sil will report three times */
    ret = devdrv_sil_reply_bbox(msg);
    if (ret != 0) {
        devdrv_drv_err("sil bbox reply failed\n");
        return ret;
    }

    /* copy exceptions data from sram to ddr, report to bbox */
    ret = devdrv_sil_copy_exception_to_bbox_ddr(g_sil_d_info, g_sil_s_info);
    if (ret != 0) {
        devdrv_drv_err("sil bbox report failed\n");
        return ret;
    }

    return 0;
}

int devdrv_safetyisland_notifier(struct notifier_block *nb, unsigned long len, void *data)
{
    int ret;
    u32 dev_id;
    struct icmdrv_msg_chan *mbx_info = NULL;
    struct drv_safety_island_ipc_info *msg = NULL;

    if ((data == NULL) || (nb == NULL)) {
        devdrv_drv_err("invalid parameter\n");
        return -EINVAL;
    }

    mbx_info = container_of(nb, struct icmdrv_msg_chan, icm_monitor);
    if (mbx_info == NULL || mbx_info->magic != ICM_MAGIC_WORD) {
        return -EINVAL;
    }
    dev_id = mbx_info->dev_id;

    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("device does not exist.\n");
        return -EINVAL;
    }

    msg = (struct drv_safety_island_ipc_info *)data;
    switch (msg->cmd_type) {
        case MSGTYPE_MS_STATUS:
            devdrv_safetyisland_ipc_bh(dev_id, mbx_info->chan_id, msg);
            break;
        case MSGTYPE_LOG_BBOX:
            ret = devdrv_sil_mbx2_bbox_notifier(nb, len, msg);
            if (ret != 0) {
                devdrv_drv_err("[dev_id: %u] sils bbox error, cmd_type:%u, ret:%d.\n", dev_id, msg->cmd_type, ret);
                return ret;
            }
            break;
        default:
            break;
    }

    return 0;
}
#endif /* CFG_SOC_PLATFORM_MDC_V51 */
#endif /* CFG_FEATURE_LP_ENABLE */

#ifdef CFG_FEATURE_IMU_ENABLE
STATIC void devdrv_fill_cmd_imu_dmp_msg(struct cmd_imu_dmp_msg *msg, u8 *data, u8 len, u8 offset, u8 finish, u8 seq)
{
    msg->offset = offset;
    msg->finish = finish;
    msg->length = len;
    msg->seq = seq;

    devdrv_ipc_move_data(msg->msg, CMD_DMP_MSG_MAX_LEN, data, len);
}

#ifndef CFG_FEATURE_IPC_FORMAT_V2_CRC
STATIC int devdrv_ipc_send_dmp_msg(unsigned int dev_id, struct cmd_imu_dmp_msg *dmp_msg)
{
    int ret;
    struct devdrv_ipc_cmd ipc = {0};

    ret = memcpy_s(ipc.cmdPara0, (sizeof(struct devdrv_ipc_cmd) - sizeof(unsigned int)),
        dmp_msg, sizeof(struct cmd_imu_dmp_msg));
    if (ret != 0) {
        devdrv_drv_err("Copy msg to ipc buffer failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ipc.cmd_sub = 0;
    ipc.cmd = CMD_IMU_DMP_MSG;
    ipc.target_id = OBJ_IMU_DMP;
    ipc.source_id = OBJ_AP;
    ret = icm_msg_send_async(ICM_FD_BUILD(dev_id, (unsigned int)HISI_RPROC_TX_IMU_MBX25),
        (rproc_msg_t *)&ipc, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        devdrv_drv_err("Send dmp msg to imu failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return 0;
}
#else
STATIC int devdrv_ipc_send_dmp_msg(unsigned int dev_id, struct cmd_imu_dmp_msg *dmp_msg)
{
    int ret;
    struct icmdrv_ipc_msg ipc = {0};

    ret = memcpy_s(ipc.data, ICM_MSG_DATA_LENGTH, dmp_msg, sizeof(struct cmd_imu_dmp_msg));
    if (ret != 0) {
        devdrv_drv_err("Copy msg to ipc buffer failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ipc.sub_cmd = ICM_SUB_CMD_NO_TYPE;
    ipc.cmd = ICM_MAIN_CMD_CCPU_DMP;
    ipc.cmd_dest = OBJ_CMD_LP;
    ipc.cmd_src = OBJ_CMD_CCPU;
    ipc.len = ICM_MSG_DATA_LENGTH;
    ipc.msg_type = 1;

    ret = icm_msg_send_async(ICM_FD_BUILD(dev_id, (unsigned int)DMS_LP_IPC_CHAN_ID),
        (rproc_msg_t *)&ipc, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        devdrv_drv_err("Send dmp msg to imu failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return 0;
}
#endif
#endif

int devdrv_ipc_send_to_imu(unsigned long arg)
{
#ifdef CFG_FEATURE_IMU_ENABLE
    u8 buf[MAX_IPCDRV_MSG_LENGTH + 2] = {0};  /* 2 buf[0] devid buf[1] len buf[2]~ data */
    u8 len, seq;
    u32 dev_id;
    int ret;
    u8 *data = NULL;
    struct cmd_imu_dmp_msg dmp_msg = {0};

    ret = copy_from_user_safe(buf, (void *)((uintptr_t)arg), sizeof(buf));
    if (ret != 0) {
        devdrv_drv_err("copy_from_user failed.\n");
        return ret;
    }

    dev_id = (u32)buf[0];
    len = buf[1];
    data = &buf[2];
    if ((dev_id >= MAX_CHIP_NUM) || (len > MAX_IPCDRV_MSG_LENGTH)) {
        devdrv_drv_err("Message data is invalid. (device_id=%u; len=%u)\n", dev_id, len);
        return -EINVAL;
    }

    mutex_lock(&imu_mutex[dev_id]);
    seq = imu_dmp_data[dev_id].seq;

    if (len <= CMD_DMP_MSG_MAX_LEN) {
        devdrv_fill_cmd_imu_dmp_msg(&dmp_msg, data, len, 0, 1, seq);
        ret = devdrv_ipc_send_dmp_msg(dev_id, &dmp_msg);
        if (ret != 0)
            goto err;
    } else {
        devdrv_fill_cmd_imu_dmp_msg(&dmp_msg, data, CMD_DMP_MSG_MAX_LEN, 0, 0, seq);
        ret = devdrv_ipc_send_dmp_msg(dev_id, &dmp_msg);
        if (ret != 0)
            goto err;

        devdrv_fill_cmd_imu_dmp_msg(&dmp_msg, data + CMD_DMP_MSG_MAX_LEN,
            len - CMD_DMP_MSG_MAX_LEN, CMD_DMP_MSG_MAX_LEN, 1, seq);
        ret = devdrv_ipc_send_dmp_msg(dev_id, &dmp_msg);
        if (ret != 0)
            goto err;
    }

    mutex_unlock(&imu_mutex[dev_id]);

    return 0;
err:
    mutex_unlock(&imu_mutex[dev_id]);
    devdrv_drv_err("send fail ret %d\n", ret);
    return ret;
#else
    return -EINVAL;
#endif
}

int devdrv_ipc_recv_from_imu(unsigned long arg)
{
#ifdef CFG_FEATURE_IMU_ENABLE
    u8 buf[MAX_IPCDRV_MSG_LENGTH + 2] = {0};  // 2 // buf[0] devid buf[1] len buf[2]~ data
    u32 dev_id;
    int ret;
    u8 *data = NULL;
    struct devdrv_ipc_cmd_data *cmd_data = NULL;

    ret = copy_from_user_safe(buf, (void *)((uintptr_t)arg), sizeof(buf));
    if (ret != 0) {
        devdrv_drv_err("copy_from_user failed.\n");
        return ret;
    }

    dev_id = (u32)buf[0];
    data = &buf[2];

    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("dev_id %u error\n", dev_id);
        return -EINVAL;
    }

    cmd_data = &imu_dmp_data[dev_id];

    if (cmd_data->valid == 0) {
        return -EAGAIN;
    }

    buf[1] = cmd_data->len;
    devdrv_ipc_move_data(data, MAX_IPCDRV_MSG_LENGTH, cmd_data->data, cmd_data->len);

    ret = copy_to_user_safe((void *)((uintptr_t)arg), buf, sizeof(buf));
    if (ret != 0) {
        devdrv_drv_err("copy_to_user failed.\n");
        return ret;
    }

    cmd_data->valid = 0;

    return 0;
#else
    return -EINVAL;
#endif
}

STATIC void devdrv_ipc_manager_init(u32 dev_id)
{
    int i;

    mutex_init(&imu_mutex[dev_id]);

    imu_dmp_data[dev_id].seq = 0;
    imu_dmp_data[dev_id].len = 0;
    imu_dmp_data[dev_id].valid = 0;
    for (i = 0; i < MAX_IPCDRV_MSG_LENGTH; i++) {
        imu_dmp_data[dev_id].data[i] = 0;
    }

#ifdef CFG_SOC_PLATFORM_MDC_V51
    mutex_init(&si_mutex[dev_id]);
    devdrv_safetyisland_ipc_init(dev_id);
    if (devdrv_sil_bbox_init(dev_id, MAILBOX_TO_SI_MBX2) != 0) {
        devdrv_drv_err("sil bbox init faield...dev_id:%d\n", dev_id);
        devdrv_sil_bbox_uninit(dev_id, MAILBOX_TO_SI_MBX2);
        return;
    }
#endif
}

STATIC void devdrv_ipc_manager_exit(u32 dev_id)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    devdrv_safetyisland_ipc_exit(dev_id);
    devdrv_sil_bbox_uninit(dev_id, MAILBOX_TO_SI_MBX2);
#endif
}

#if defined(CFG_FEATURE_IMU_ENABLE)
int ipc_mailbox_rx_register(u32 dev_id)
{
    int ret;
    devdrv_ipc_manager_init(dev_id);

    mbox_lp_nb.notifier_call = devdrv_imu_mbx_notifier;
    if ((ret = icm_rx_register(dev_id, HISI_RPROC_MAX, IPC_OBJ_LP, &mbox_lp_nb)) != 0) {
        devdrv_ipc_manager_exit(dev_id);
        devdrv_drv_err("mailbox(%u %d) register fail...err:%d\n", dev_id, IPC_OBJ_LP, ret);
        return ret;
    }

    return ret;
}

void ipc_mailbox_rx_unregister(u32 dev_id)
{
    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("device id %u is more than %d\n", dev_id, MAX_CHIP_NUM);
        return;
    }

    devdrv_ipc_manager_exit(dev_id);

    (void)icm_rx_unregister(dev_id, HISI_RPROC_MAX, IPC_OBJ_LP, &mbox_lp_nb);
}

#elif defined(CFG_FEATURE_LP_ENABLE)
int ipc_mailbox_rx_register(u32 dev_id)
{
    int ret;

    devdrv_ipc_manager_init(dev_id);

    mbox_lp_nb.notifier_call = devdrv_lp_notifier;
    ret = icm_rx_register(dev_id, HISI_RPROC_MAX, IPC_OBJ_LP, &mbox_lp_nb);
    if (ret != 0) {
        devdrv_drv_err("ts mailbox register fail...err:%d\n", ret);
        return ret;
    }

#ifdef CFG_SOC_PLATFORM_MDC_V51
    mbox_si_nb.notifier_call = devdrv_safetyisland_notifier;
    ret = icm_rx_register(dev_id, HISI_RPROC_MAX, IPC_OBJ_SI, &mbox_si_nb);
    if (ret != 0) {
        devdrv_drv_err("si mailxbox register faild...err:%d\n", ret);
        (void)icm_rx_unregister(dev_id, HISI_RPROC_MAX, IPC_OBJ_LP, &mbox_lp_nb);
        return ret;
    }
#endif

    return ret;
}

void ipc_mailbox_rx_unregister(u32 dev_id)
{
    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("device id %d is more than %d\n", dev_id, MAX_CHIP_NUM);
        return;
    }
#ifdef CFG_SOC_PLATFORM_MDC_V51
    (void)icm_rx_unregister(dev_id, HISI_RPROC_MAX, IPC_OBJ_SI, &mbox_si_nb);
#endif
    (void)icm_rx_unregister(dev_id, HISI_RPROC_MAX, IPC_OBJ_LP, &mbox_lp_nb);
    devdrv_ipc_manager_exit(dev_id);
}
#endif
