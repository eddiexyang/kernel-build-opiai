/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-8-29
*/

#include <linux/kallsyms.h>
#include "devdrv_common.h"
#include "icm_interface.h"
#include "tsmng_interface.h"
#include "tsdrv_qos.h"
#include "tsdrv_log.h"
#include "securec.h"


typedef int (*tsdrv_qos_register_func)(const struct qos_master_node *master);
typedef int (*tsdrv_qos_unregister_func)(const struct qos_master_node *master);

DEFINE_MUTEX(g_tsdrv_qos_mutex);
tsdrv_qos_register_func g_tsdrv_qos_register = NULL;
tsdrv_qos_unregister_func g_tsdrv_qos_unregister = NULL;

int tsdrv_qos_register(const struct qos_master_node *master)
{
    int ret = 0;
    mutex_lock(&g_tsdrv_qos_mutex);
    if (g_tsdrv_qos_register == NULL) {
        g_tsdrv_qos_register = (tsdrv_qos_register_func)__symbol_get("hal_kernel_qos_node_register");
    }
    if (g_tsdrv_qos_register != NULL) {
        ret = g_tsdrv_qos_register (master);
    }
    mutex_unlock(&g_tsdrv_qos_mutex);
    return ret;
}

int tsdrv_qos_unregister(const struct qos_master_node *master)
{
    int ret = 0;
    mutex_lock(&g_tsdrv_qos_mutex);
    if (g_tsdrv_qos_unregister == NULL) {
        g_tsdrv_qos_unregister = (tsdrv_qos_unregister_func)__symbol_get("hal_kernel_qos_node_unregister");
    }
    if (g_tsdrv_qos_unregister != NULL) {
        ret = g_tsdrv_qos_unregister(master);
    }
    mutex_unlock(&g_tsdrv_qos_mutex);
    return ret;
}

static void tsdrv_qos_ipc_pack(rproc_msg_t *msg, u32 msg_len, u32 cmd_type, u8 *data, u32 len)
{
    struct ipcdrv_message *ipc_msg = NULL;
    int ret;

    ipc_msg = (struct ipcdrv_message *)msg;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
    ipc_msg->ipc_msg_header.cmd_type = cmd_type;
    ipc_msg->ipc_msg_header.sync_type = IPCDRV_MSG_SYNC;
    ipc_msg->ipc_msg_header.reserved = 0;
    ipc_msg->ipc_msg_header.msg_length = len;
    ipc_msg->ipc_msg_header.msg_index = 0;

    ret = memcpy_s(ipc_msg->ipcdrv_payload, IPCDRV_MSG_LENGTH, data, len);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Memcpy_s failed. (ret=%d; len=%u)\n", ret, len);
    }
}

static int tsdrv_set_mpam(int devid, rproc_id_t id, const struct qos_master_config_type *cfg)
{
    int ret;
    struct ipcdrv_message ack_data = {};
    struct ts_qos_mpam_cfg qos_cfg = {};
    struct ts_qos_mpam_cfg *qos_cfg_ptr = NULL;
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH] = {};

    if (cfg == NULL) {
        TSDRV_PRINT_ERR("Cfg is NULL.\n");
        return -EINVAL;
    }

    qos_cfg.replace_en = 0;
    qos_cfg.vf_en = 0;

    qos_cfg.bitmap = cfg->bitmap[0];
    qos_cfg.mpam_id = cfg->mpamid;
    qos_cfg.pmg = cfg->pmg;
    qos_cfg.qos = cfg->qos;
    qos_cfg.type = cfg->type;

    tsdrv_qos_ipc_pack(msg, IPCDRV_RPROC_MSG_LENGTH, IPCDRV_TS_QOS_SET, (u8 *)&qos_cfg, sizeof(qos_cfg));
    ret = icm_msg_send_sync(ICM_FD_BUILD((u32)devid, id), msg, IPCDRV_RPROC_MSG_LENGTH,
        (u32 *)&ack_data, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Send icm msg failed. (devid=%d; ret=%d)\n", devid, ret);
        return ret;
    }
    qos_cfg_ptr = (struct ts_qos_mpam_cfg*)ack_data.ipcdrv_payload;
    if (qos_cfg_ptr->err != 0) {
        TSDRV_PRINT_ERR("Set qos failed. (type=%u; ret=%u)\n", cfg->type, qos_cfg_ptr->err);
        return (int)qos_cfg_ptr->err;
    }

    TSDRV_PRINT_DEBUG("tsdrv set qos cfg success. (bitmap=%llu; mpam_id=%u; pmg=%u; qos=%u; type=%u)\n",
        qos_cfg.bitmap, qos_cfg.mpam_id, qos_cfg.pmg, qos_cfg.qos, qos_cfg.type);
    return 0;
}

static int tsdrv_set_aic_mpam(int devid, const struct qos_master_config_type *cfg)
{
    return tsdrv_set_mpam(devid, HISI_RPROC_TSC_TX_RPID1, cfg);
}

static int tsdrv_set_aiv_mpam(int devid, const struct qos_master_config_type *cfg)
{
    return tsdrv_set_mpam(devid, HISI_RPROC_TSV_TX_RPID1, cfg);
}

static int tsdrv_get_mpam(int devid, rproc_id_t id, struct qos_master_config_type *cfg)
{
    int ret;
    struct ipcdrv_message ack_data = {};
    struct ts_qos_mpam_cfg qos_cfg = {};
    struct ts_qos_mpam_cfg *qos_cfg_ptr = NULL;
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH] = {};

    if (cfg == NULL) {
        TSDRV_PRINT_ERR("Cfg is NULL.\n");
        return -EINVAL;
    }

    qos_cfg.bitmap = cfg->bitmap[0];
    qos_cfg.type = cfg->type;

    tsdrv_qos_ipc_pack(msg, IPCDRV_RPROC_MSG_LENGTH, IPCDRV_TS_QOS_GET, (u8 *)&qos_cfg, sizeof(qos_cfg));
    ret = icm_msg_send_sync(ICM_FD_BUILD((u32)devid, id), msg, IPCDRV_RPROC_MSG_LENGTH,
        (u32 *)&ack_data, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Send icm msg failed. (devid=%d; ret=%d)\n", devid, ret);
        return ret;
    }

    qos_cfg_ptr = (struct ts_qos_mpam_cfg*)ack_data.ipcdrv_payload;
    if (qos_cfg_ptr->err != 0) {
        TSDRV_PRINT_ERR("Get qos config failed. (type=%u; ret=%u)\n", cfg->type, qos_cfg_ptr->err);
        return (int)qos_cfg_ptr->err;
    }

    cfg->qos = qos_cfg_ptr->qos;
    cfg->pmg = qos_cfg_ptr->pmg;
    cfg->mpamid = qos_cfg_ptr->mpam_id;

    TSDRV_PRINT_DEBUG("Get qos info. (bitmap=0x%llx; mpam_id=%u; pmg=%u; qos=%u; type=%u)\n",
        qos_cfg.bitmap, qos_cfg_ptr->mpam_id, qos_cfg_ptr->pmg, qos_cfg_ptr->qos, qos_cfg.type);
    return 0;
}

static int tsdrv_get_aic_mpam(int devid, struct qos_master_config_type *cfg)
{
    return tsdrv_get_mpam(devid, HISI_RPROC_TSC_TX_RPID1, cfg);
}

static int tsdrv_get_aiv_mpam(int devid, struct qos_master_config_type *cfg)
{
    return tsdrv_get_mpam(devid, HISI_RPROC_TSV_TX_RPID1, cfg);
}

enum tsdrv_qos_node {
    TSDRV_QOS_NODE_AIC_DAT,
    TSDRV_QOS_NODE_AIC_INS,
    TSDRV_QOS_NODE_AIV_DAT,
    TSDRV_QOS_NODE_AIV_INS,
    TSDRV_QOS_NODE_MAX
};

static struct qos_master_node g_qos_node[TSDRV_QOS_NODE_MAX] = {
    [TSDRV_QOS_NODE_AIC_DAT] = {
        .name = "AIC_DAT_QOS", .cfg.type = MASTER_AIC_DAT, .set = tsdrv_set_aic_mpam, .get = tsdrv_get_aic_mpam,
    },
    [TSDRV_QOS_NODE_AIC_INS] = {
        .name = "AIC_INS_QOS", .cfg.type = MASTER_AIC_INS, .set = tsdrv_set_aic_mpam, .get = tsdrv_get_aic_mpam,
    },
    [TSDRV_QOS_NODE_AIV_DAT] = {
        .name = "AIV_DAT_QOS", .cfg.type = MASTER_AIV_DAT, .set = tsdrv_set_aiv_mpam, .get = tsdrv_get_aiv_mpam,
    },
    [TSDRV_QOS_NODE_AIV_INS] = {
        .name = "AIV_INS_QOS", .cfg.type = MASTER_AIV_INS, .set = tsdrv_set_aiv_mpam, .get = tsdrv_get_aiv_mpam,
    }
};

int tsdrv_qos_node_register(void)
{
    int ret, idx, i;
    for (idx = 0; idx < TSDRV_QOS_NODE_MAX; idx++) {
        ret = tsdrv_qos_register(&g_qos_node[idx]);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Qos node resigster failed. (idx=%d; ret=%d)\n", idx, ret);
            goto err_qos_node_resigster;
        }
    }
    return 0;

err_qos_node_resigster:
    for (i = 0; i < idx; i++) {
        (void)tsdrv_qos_unregister(&g_qos_node[i]);
    }
    return -EINVAL;
}

void tsdrv_qos_node_unregister(void)
{
    int idx;
    for (idx = 0; idx < TSDRV_QOS_NODE_MAX; idx++) {
        (void)tsdrv_qos_unregister(&g_qos_node[idx]);
    }
}
