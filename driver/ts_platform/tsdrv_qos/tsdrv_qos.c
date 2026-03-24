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
* Create: 2022-8-29
*/

#include "tsdrv_qos.h"
#include "icm_interface.h"
#include "tsdrv_log.h"
#include "devdrv_common.h"
#include "securec.h"

#ifdef TSDRV_UT
#define MBOX_ID 0
#else
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#define MBOX_ID HISI_RPROC_TX_TS_MBX3
#else
#define MBOX_ID HISI_RPROC_TX_TS_ACPU0
#endif
#endif

#define ENABLE 1
#define REPLACE_EN_MODE 0
#define VF_EN_MODE 1
#define SQE_SELF_MODE 2

static int tsdrv_set_replace_and_vf_en(int mode, struct ts_qos_mpam_cfg *qos_cfg)
{
    if (mode < REPLACE_EN_MODE || mode > SQE_SELF_MODE) {
        TSDRV_PRINT_ERR("Mode is invalid. (mode=%d)\n", mode);
        return -EINVAL;
    }
    qos_cfg->replace_en = (mode == REPLACE_EN_MODE) ? ENABLE : 0;
    qos_cfg->vf_en = (mode == VF_EN_MODE) ? ENABLE : 0;

    return 0;
}

int tsdrv_set_qos_cfg(int devid, const struct qos_master_config_type *cfg)
{
    int ret;
    struct icmdrv_ipc_msg_info ack_data = {0};
    struct icmdrv_ipc_msg ipc_msg = {0};
    struct ts_qos_mpam_cfg qos_cfg;
    struct ts_qos_mpam_cfg *qos_cfg_ptr = NULL;

    if (cfg == NULL) {
        TSDRV_PRINT_ERR("Cfg is NULL.\n");
        return -EINVAL;
    }
    ipc_msg.sub_cmd = ICM_SUB_CMD_SET_QOS;
    ipc_msg.cmd = ICM_MAIN_CMD_TS_NOTIFY;
    ipc_msg.cmd_dest = OBJ_CMD_TS;
    ipc_msg.cmd_src = OBJ_CMD_CCPU;
    ipc_msg.len = sizeof(struct ts_qos_mpam_cfg);

    qos_cfg.bitmap = cfg->bitmap[0];
    qos_cfg.mpam_id = cfg->mpamid;
    qos_cfg.pmg = cfg->pmg;
    qos_cfg.qos = cfg->qos;
    qos_cfg.type = cfg->type;

    ret = tsdrv_set_replace_and_vf_en(cfg->mode, &qos_cfg);
    if (ret != 0) {
        return -EINVAL;
    }

    ret = memcpy_s(ipc_msg.data, ICM_MSG_DATA_LENGTH, &qos_cfg, sizeof(struct ts_qos_mpam_cfg));
    if (ret != 0) {
        TSDRV_PRINT_ERR("Memcpy failed. (ret=%d)\n", ret);
        return -EINVAL;
    }
    ret = icm_msg_send_sync(ICM_FD_BUILD((u32)devid, MBOX_ID), (u32 *)&ipc_msg, IPCDRV_RPROC_MSG_LENGTH,
        (u32 *)&ack_data, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Send icm msg failed. (devid=%d; ret=%d)\n", devid, ret);
        return ret;
    }
    qos_cfg_ptr = (struct ts_qos_mpam_cfg*)ack_data.data;
    if (qos_cfg_ptr->err != 0) {
        TSDRV_PRINT_ERR("Set qos failed. (type=%u; ret=%u)\n", cfg->type, (u32)qos_cfg_ptr->err);
        return qos_cfg_ptr->err;
    }
    return 0;
}

int tsdrv_get_qos_cfg(int devid, struct qos_master_config_type *cfg)
{
    int ret;
    struct icmdrv_ipc_msg_info ack_data = {0};
    struct icmdrv_ipc_msg ipc_msg = {0};
    struct ts_qos_mpam_cfg qos_cfg;
    struct ts_qos_mpam_cfg *qos_cfg_ptr = NULL;

    if (cfg == NULL) {
        TSDRV_PRINT_ERR("Cfg is NULL.\n");
        return -EINVAL;
    }
    ipc_msg.sub_cmd = ICM_SUB_CMD_GET_QOS;
    ipc_msg.cmd = ICM_MAIN_CMD_TS_REQ;
    ipc_msg.cmd_dest = OBJ_CMD_TS;
    ipc_msg.cmd_src = OBJ_CMD_CCPU;
    ipc_msg.len = sizeof(struct ts_qos_mpam_cfg);

    qos_cfg.type = cfg->type;

    ret = memcpy_s(ipc_msg.data, ICM_MSG_DATA_LENGTH, &qos_cfg, sizeof(struct ts_qos_mpam_cfg));
    if (ret != 0) {
        TSDRV_PRINT_ERR("Memcpy failed. (ret=%d)\n", ret);
        return ret;
    }
    ret = icm_msg_send_sync(ICM_FD_BUILD((u32)devid, MBOX_ID), (u32 *)&ipc_msg, IPCDRV_RPROC_MSG_LENGTH,
        (u32 *)&ack_data, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Send icm msg failed. (devid=%d; ret=%d)\n", devid, ret);
        return ret;
    }
    qos_cfg_ptr = (struct ts_qos_mpam_cfg*)ack_data.data;
    if (qos_cfg_ptr->err != 0) {
        TSDRV_PRINT_ERR("Get qos config failed. (type=%u; ret=%u)\n", cfg->type, (u32)qos_cfg_ptr->err);
        return qos_cfg_ptr->err;
    }

    cfg->qos = qos_cfg_ptr->qos;
    cfg->pmg = qos_cfg_ptr->pmg;
    cfg->mpamid = qos_cfg_ptr->mpam_id;
    cfg->bitmap[0] = qos_cfg_ptr->bitmap;

    TSDRV_PRINT_DEBUG("Get qos info. (bitmap=0x%llx; mpam_id=%u; pmg=%u; qos=%u; type=%u\n",
        qos_cfg_ptr->bitmap, qos_cfg_ptr->mpam_id, qos_cfg_ptr->pmg, qos_cfg_ptr->qos, qos_cfg_ptr->type);
    return 0;
}

int tsdrv_set_otsd_cfg(int devid, const struct qos_otsd_config_type *cfg)
{
    struct icmdrv_ipc_msg_info ack_data = {};
    struct icmdrv_ipc_msg ipc_msg = {};
    struct ts_qos_cfg qos_cfg = {};
    struct ts_qos_cfg *qos_cfg_ptr = NULL;
    int ret, i;

    if (cfg == NULL) {
        TSDRV_PRINT_ERR("Cfg is NULL.\n");
        return -EINVAL;
    }
    qos_cfg.master_type = cfg->master;
    qos_cfg.op_type = TS_QOS_OP_SET;
    qos_cfg.qos_cfg_type = TS_QOS_CFG_OTSD;
    qos_cfg.otsd_cfg.mode = cfg->otsd_mode;
    for (i = 0; i < MAX_OTSD_LEVEL; i++) {
        qos_cfg.otsd_cfg.lvl[i] = cfg->otsd_lvl[i];
    }

    ipc_msg.sub_cmd = ICM_SUB_CMD_CFG_QOS;
    ipc_msg.cmd = ICM_MAIN_CMD_TS_NOTIFY;
    ipc_msg.cmd_dest = OBJ_CMD_TS;
    ipc_msg.cmd_src = OBJ_CMD_CCPU;
    ipc_msg.len = sizeof(struct ts_qos_cfg);
    ret = memcpy_s(ipc_msg.data, ICM_MSG_DATA_LENGTH, &qos_cfg, sizeof(struct ts_qos_cfg));
    if (ret != 0) {
        TSDRV_PRINT_ERR("Memcpy failed. (ret=%d)\n", ret);
        return -EINVAL;
    }
    ret = icm_msg_send_sync(ICM_FD_BUILD((u32)devid, MBOX_ID), (u32 *)&ipc_msg, IPCDRV_RPROC_MSG_LENGTH,
        (u32 *)&ack_data, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Send icm msg failed. (devid=%d; ret=%d)\n", devid, ret);
        return ret;
    }
    qos_cfg_ptr = (struct ts_qos_cfg*)ack_data.data;
    if (qos_cfg_ptr->status != 0U) {
        TSDRV_PRINT_ERR("Set qos otsd failed. (master=%u; otsd_mode=%u; lvl_0=%u; lvl_1=%u; ret=%u)\n",
            cfg->master, cfg->otsd_mode, cfg->otsd_lvl[0], cfg->otsd_lvl[1], qos_cfg_ptr->status);
        return (int)qos_cfg_ptr->status;
    }
    return 0;
}

int tsdrv_get_otsd_cfg(int devid, struct qos_otsd_config_type *cfg)
{
    struct icmdrv_ipc_msg_info ack_data = {};
    struct icmdrv_ipc_msg ipc_msg = {};
    struct ts_qos_cfg qos_cfg;
    struct ts_qos_cfg *qos_cfg_ptr = NULL;
    int ret, i;

    if (cfg == NULL) {
        TSDRV_PRINT_ERR("Cfg is NULL.\n");
        return -EINVAL;
    }
    qos_cfg.master_type = cfg->master;
    qos_cfg.op_type = TS_QOS_OP_GET;
    qos_cfg.qos_cfg_type = TS_QOS_CFG_OTSD;

    ret = memcpy_s(ipc_msg.data, ICM_MSG_DATA_LENGTH, &qos_cfg, sizeof(struct ts_qos_cfg));
    if (ret != 0) {
        TSDRV_PRINT_ERR("Memcpy failed. (ret=%d)\n", ret);
        return ret;
    }

    ipc_msg.sub_cmd = ICM_SUB_CMD_CFG_QOS;
    ipc_msg.cmd = ICM_MAIN_CMD_TS_REQ;
    ipc_msg.cmd_dest = OBJ_CMD_TS;
    ipc_msg.cmd_src = OBJ_CMD_CCPU;
    ipc_msg.len = sizeof(struct ts_qos_cfg);

    ret = icm_msg_send_sync(ICM_FD_BUILD((u32)devid, MBOX_ID), (u32 *)&ipc_msg, IPCDRV_RPROC_MSG_LENGTH,
        (u32 *)&ack_data, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Send icm msg failed. (devid=%d; ret=%d)\n", devid, ret);
        return ret;
    }

    qos_cfg_ptr = (struct ts_qos_cfg*)ack_data.data;
    if (qos_cfg_ptr->status != 0U) {
        TSDRV_PRINT_ERR("Get qos otsd config failed. (master=%u; ret=%u)\n",
            cfg->master, qos_cfg_ptr->status);
        return (int)qos_cfg_ptr->status;
    }

    cfg->otsd_mode = qos_cfg_ptr->otsd_cfg.mode;
    for (i = 0; i < MAX_OTSD_LEVEL; i++) {
        cfg->otsd_lvl[i] = qos_cfg_ptr->otsd_cfg.lvl[i];
    }

    TSDRV_PRINT_DEBUG("Get qos info. (master=0x%u; otsd_mode=%u; lvl_0=%u; lvl_1=%u\n",
        cfg->master, cfg->otsd_mode, cfg->otsd_lvl[0], cfg->otsd_lvl[1]);
    return 0;
}

int tsdrv_set_allow_cfg(int devid, const struct qos_allow_config_type *cfg)
{
    struct icmdrv_ipc_msg_info ack_data = {};
    struct icmdrv_ipc_msg ipc_msg = {};
    struct ts_qos_cfg qos_cfg = {};
    struct ts_qos_cfg *qos_cfg_ptr = NULL;
    int ret, i;

    if (cfg == NULL) {
        TSDRV_PRINT_ERR("Cfg is NULL.\n");
        return -EINVAL;
    }
    qos_cfg.master_type = cfg->master;
    qos_cfg.op_type = TS_QOS_OP_SET;
    qos_cfg.qos_cfg_type = TS_QOS_CFG_ALLOW;
    qos_cfg.allow_cfg.mode = cfg->qos_allow_mode;
    qos_cfg.allow_cfg.ctrl = cfg->qos_allow_ctrl;
    for (i = 0; i < MAX_QOS_ALLOW_LEVEL; i++) {
        if (cfg->qos_allow_lvl[i] > TS_QOS_MAX_ALLOW_LEVEL) {
            TSDRV_PRINT_ERR("Allow lvl out of range. (lvl=%u)\n", cfg->qos_allow_lvl[i]);
            return -EINVAL;
        }
        qos_cfg.allow_cfg.lvl[i] = cfg->qos_allow_lvl[i];
    }

    ipc_msg.sub_cmd = ICM_SUB_CMD_CFG_QOS;
    ipc_msg.cmd = ICM_MAIN_CMD_TS_NOTIFY;
    ipc_msg.cmd_dest = OBJ_CMD_TS;
    ipc_msg.cmd_src = OBJ_CMD_CCPU;
    ipc_msg.len = sizeof(struct ts_qos_cfg);
    ret = memcpy_s(ipc_msg.data, ICM_MSG_DATA_LENGTH, &qos_cfg, sizeof(struct ts_qos_cfg));
    if (ret != 0) {
        TSDRV_PRINT_ERR("Memcpy failed. (ret=%d)\n", ret);
        return -EINVAL;
    }
    ret = icm_msg_send_sync(ICM_FD_BUILD((u32)devid, MBOX_ID), (u32 *)&ipc_msg, IPCDRV_RPROC_MSG_LENGTH,
        (u32 *)&ack_data, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Send icm msg failed. (devid=%d; ret=%d)\n", devid, ret);
        return ret;
    }
    qos_cfg_ptr = (struct ts_qos_cfg*)ack_data.data;
    if (qos_cfg_ptr->status != 0U) {
        TSDRV_PRINT_ERR("Set qos allow failed. "
            "(master=%u; allow_mode=%u; ctrl=%u; lvl_0=%u; lvl_1=%u; lvl_2=%u; ret=%u)\n",
            cfg->master, cfg->qos_allow_mode, cfg->qos_allow_ctrl,
            cfg->qos_allow_lvl[0], cfg->qos_allow_lvl[1], cfg->qos_allow_lvl[2], qos_cfg_ptr->status); // allow[0-1-2]
        return (int)qos_cfg_ptr->status;
    }
    return 0;
}

int tsdrv_get_allow_cfg(int devid, struct qos_allow_config_type *cfg)
{
    struct icmdrv_ipc_msg_info ack_data = {};
    struct icmdrv_ipc_msg ipc_msg = {};
    struct ts_qos_cfg qos_cfg;
    struct ts_qos_cfg *qos_cfg_ptr = NULL;
    int ret, i;

    if (cfg == NULL) {
        TSDRV_PRINT_ERR("Cfg is NULL.\n");
        return -EINVAL;
    }
    qos_cfg.master_type = cfg->master;
    qos_cfg.op_type = TS_QOS_OP_GET;
    qos_cfg.qos_cfg_type = TS_QOS_CFG_ALLOW;
    qos_cfg.allow_cfg.ctrl = cfg->qos_allow_ctrl;

    ipc_msg.sub_cmd = ICM_SUB_CMD_CFG_QOS;
    ipc_msg.cmd = ICM_MAIN_CMD_TS_REQ;
    ipc_msg.cmd_dest = OBJ_CMD_TS;
    ipc_msg.cmd_src = OBJ_CMD_CCPU;
    ipc_msg.len = sizeof(struct ts_qos_cfg);
    ret = memcpy_s(ipc_msg.data, ICM_MSG_DATA_LENGTH, &qos_cfg, sizeof(struct ts_qos_cfg));
    if (ret != 0) {
        TSDRV_PRINT_ERR("Memcpy failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = icm_msg_send_sync(ICM_FD_BUILD((u32)devid, MBOX_ID), (u32 *)&ipc_msg, IPCDRV_RPROC_MSG_LENGTH,
        (u32 *)&ack_data, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Send icm msg failed. (devid=%d; ret=%d)\n", devid, ret);
        return ret;
    }

    qos_cfg_ptr = (struct ts_qos_cfg*)ack_data.data;
    if (qos_cfg_ptr->status != 0U) {
        TSDRV_PRINT_ERR("Get qos allow config failed. (master=%u; ret=%u)\n", cfg->master, qos_cfg_ptr->status);
        return (int)qos_cfg_ptr->status;
    }

    cfg->qos_allow_mode = qos_cfg_ptr->allow_cfg.mode;
    for (i = 0; i < MAX_QOS_ALLOW_LEVEL; i++) {
        cfg->qos_allow_lvl[i] = qos_cfg_ptr->allow_cfg.lvl[i];
    }
    TSDRV_PRINT_DEBUG("Get qos info. (master=0x%u; allow_mode=%u; ctrl=%u; lvl_0=%u; lvl_1=%u; lvl_2=%u\n",
        cfg->master, cfg->qos_allow_mode, cfg->qos_allow_ctrl,
        cfg->qos_allow_lvl[0], cfg->qos_allow_lvl[1], cfg->qos_allow_lvl[2]); // print lvl[0,1,2]
    return 0;
}
