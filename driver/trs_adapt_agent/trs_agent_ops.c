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
* Create: 2022-7-15
*/
#include <asm/page.h>

#include "soc_res.h"
#include "devdrv_manager_comm.h"
#include "devdrv_interface.h"

#include "trs_chip_def_comm.h"
#include "trs_device_comm.h"
#include "trs_device_agent.h"
#include "trs_core.h"
#include "trs_core_ops.h"
#include "trs_cdqm.h"

static struct trs_device_agent_ops agent_ops[TRS_DEVICE_MAX_DEV];

void trs_agent_register_ops(u32 devid, struct trs_device_agent_ops *ops)
{
    if ((devid < TRS_DEVICE_MAX_DEV) && (ops != NULL)) {
        agent_ops[devid] = *ops;
    }
}
EXPORT_SYMBOL(trs_agent_register_ops);

static void trs_agent_ops_reset(u32 devid)
{
    agent_ops[devid].trs_agent_get_id = NULL;
    agent_ops[devid].trs_agent_put_id = NULL;
    agent_ops[devid].trs_agent_get_cap = NULL;
    agent_ops[devid].trs_agent_get_phy_addr = NULL;
    agent_ops[devid].trs_agent_get_cq_group = NULL;
    agent_ops[devid].trs_agent_instance = NULL;
    agent_ops[devid].trs_agent_uninstance = NULL;
}

void trs_agent_unregister_ops(u32 devid)
{
    if (devid < TRS_DEVICE_MAX_DEV) {
        trs_agent_ops_reset(devid);
    }
}
EXPORT_SYMBOL(trs_agent_unregister_ops);

void trs_agent_ops_init(void)
{
    int i;

    for (i = 0; i < TRS_DEVICE_MAX_DEV; i++) {
        trs_agent_ops_reset(i);
    }
}

static int trs_agent_ops_get_ssid(u32 devid, struct trs_msg_data *msg)
{
    struct trs_msg_sync_ssid *data = (struct trs_msg_sync_ssid *)msg->payload;
    int ret = 0;

    data->ssid = trs_device_get_remote_ssid(devid, data->vfid, data->hpid);
    if (data->ssid < 0) {
        trs_err("Get remote ssid fail. (devid=%u; vfid=%u; ssid=%d)\n", devid, data->vfid, data->ssid);
        return -EINVAL;
    }

    if (agent_ops[devid].trs_agent_send_ssid_to_ts != NULL) {
        ret = agent_ops[devid].trs_agent_send_ssid_to_ts(devid, msg->header.tsid, data->ssid, data->hpid);
        if (ret != 0) {
            trs_err("Send ssid fail. (devid=%u; tsid=%u; ssid=%d; ret=%d)\n",
                devid, msg->header.tsid, data->ssid, ret);
        }
    }

    return ret;
}

static int trs_agent_ops_check_res_id(u32 devid, struct trs_msg_data *msg)
{
    struct trs_msg_res_id_check *data = (struct trs_msg_res_id_check *)msg->payload;
    struct trs_id_inst inst = { .devid = devid, .tsid = msg->header.tsid };
    int pid, ret;

    ret = devdrv_query_process_by_host_pid(data->hpid, devid, DEVDRV_PROCESS_CP1, 0, &pid);
    if (ret != 0) {
        trs_err("Get pid fail. (hpid=%u; devid=%u; vfid=%u; ret=%d)\n", data->hpid, devid, 0, ret);
        return ret;
    }

    if (!trs_res_is_belong_to_proc(&inst, pid, data->id_type, data->res_id)) {
        trs_err("Res id check fail. (devid=%u; type=%d; id=%u)\n", devid, data->id_type, data->res_id);
        return -EINVAL;
    }
    trs_debug("Res id check. (devid=%u; type=%d; id=%u)\n", devid, data->id_type, data->res_id);

    return 0;
}

static int trs_agent_ops_get_id(u32 devid, struct trs_msg_data *msg)
{
    if (agent_ops[devid].trs_agent_get_id == NULL) {
        trs_err("Invalid value. (devid=%u)\n", devid);
        return -EINVAL;
    }

    return agent_ops[devid].trs_agent_get_id(devid, msg->header.tsid, (struct trs_msg_id_sync *)msg->payload);
}

static int trs_agent_ops_put_id(u32 devid, struct trs_msg_data *msg)
{
    if (agent_ops[devid].trs_agent_put_id == NULL) {
        trs_err("Invalid value. (devid=%u)\n", devid);
        return -EINVAL;
    }

    return agent_ops[devid].trs_agent_put_id(devid, msg->header.tsid, (struct trs_msg_id_sync *)msg->payload);
}

static int trs_agent_ops_get_cap(u32 devid, struct trs_msg_data *msg)
{
    if (agent_ops[devid].trs_agent_get_cap == NULL) {
        trs_err("Invalid value. (devid=%u)\n", devid);
        return -EINVAL;
    }

    return agent_ops[devid].trs_agent_get_cap(devid, msg->header.tsid, (struct trs_msg_id_cap *)msg->payload);
}

static int trs_agent_ops_get_phy_addr(u32 devid, struct trs_msg_data *msg)
{
    if (agent_ops[devid].trs_agent_get_phy_addr == NULL) {
        trs_err("Invalid value. (devid=%u)\n", devid);
        return -EINVAL;
    }

    return agent_ops[devid].trs_agent_get_phy_addr(devid, msg->header.tsid,
        (struct trs_msg_get_phy_addr *)msg->payload);
}

static int trs_agent_ops_get_cq_group(u32 devid, struct trs_msg_data *msg)
{
    if (agent_ops[devid].trs_agent_get_cq_group == NULL) {
        trs_err("Invalid value. (devid=%u)\n", devid);
        return -EINVAL;
    }

    return agent_ops[devid].trs_agent_get_cq_group(devid, msg->header.tsid, (struct trs_msg_cq_group *)msg->payload);
}

static int trs_agent_ops_get_proc_num(u32 devid, struct trs_msg_data *msg)
{
    if (agent_ops[devid].trs_agent_get_proc_num == NULL) {
        trs_err("invalid value. (devid=%u)\n", devid);
        return -EINVAL;
    }

    return agent_ops[devid].trs_agent_get_proc_num(devid, msg->header.tsid, (struct trs_msg_proc_num *)msg->payload);
}

static int trs_agent_ops_get_res_avail_num(u32 devid, struct trs_msg_data *msg)
{
    if (agent_ops[devid].trs_agent_get_res_avail_num == NULL) {
        trs_warn("Not support. (devid=%u)\n", devid);
        return -EINVAL;
    }

    return agent_ops[devid].trs_agent_get_res_avail_num(devid, msg->header.tsid,
        (struct trs_msg_res_num *)msg->payload);
}

static int trs_pm_agent_sq_reg_map(u32 devid, u32 tsid, struct trs_msg_sqcq_sync *data)
{
    struct trs_id_inst pm_inst;
    struct trs_id_inst inst;
    phys_addr_t pa;
    u32 vfid, pfid;
    pid_t devpid;
    size_t size;
    int ret;
    u64 va;

    trs_id_inst_pack(&inst, devid, tsid);
    if (trs_id_inst_check(&inst) != 0) {
        return -EINVAL;
    }

    ret = trs_stream_bind_remote_sqcq(&inst, data->bind_sqcq.stream_id, data->sq_id, data->cq_id, data->remote_tgid);
    if (ret != 0) {
        return ret;
    }

    data->sq_map.va = 0;
    data->sq_map.size = 0;
    ret = devdrv_get_pfvf_id_by_devid(devid, &pfid, &vfid);
    if (ret != 0) {
        trs_err("Get pfvf id fail. (ret=%d; devid=%u)\n", ret, devid);
        return ret;
    }

    /* Get Sq reg paddr should use pfid. Because Sq reg paddr is just initiated in physical device */
    trs_id_inst_pack(&pm_inst, pfid, tsid);
    ret = trs_core_ops_get_sq_reg_paddr(&pm_inst, data->sq_id, &pa, &size);
    if (ret != 0) {
        trs_debug("Get sq reg fail. (ret=%d; devid=%u; tsid=%u; sqid=%u; pfid=%u; vfid=%u)\n",
            ret, pm_inst.devid, pm_inst.tsid, data->sq_id, pfid, vfid);
        return 0;
    }

    /* Find devipid from hostpid, should use virtual devid. vfid should be 0 */
    ret = devdrv_query_process_by_host_pid(data->remote_tgid, devid, DEVDRV_PROCESS_CP1, 0, &devpid);
    if (ret != 0) {
        trs_debug("Slave pid is not bind. (hostpid=%d; devid=%u)\n", data->remote_tgid, devid);
        return -ESRCH;
    }

    ret = trs_core_ops_mmap(devpid, pa, size, &va);
    if (ret != 0) {
        return ret;
    }

    ret = trs_set_sq_reg_vaddr(&inst, data->sq_id, va, size);
    if (ret != 0) {
        trs_core_ops_unmap(devpid, va, size);
        return ret;
    }
    data->sq_map.va = va;
    data->sq_map.size = size;
    return 0;
}

static int trs_pm_agent_sq_reg_unmap(u32 devid, u32 tsid, struct trs_msg_sqcq_sync *data)
{
    struct trs_id_inst inst;
    pid_t devpid;
    size_t size;
    u64 va = 0;
    int ret;

    trs_id_inst_pack(&inst, devid, tsid);
    if (trs_id_inst_check(&inst) != 0) {
        return -EINVAL;
    }

    ret = trs_stream_bind_remote_sqcq(&inst, data->bind_sqcq.stream_id, (u32)-1, (u32)-1, -1);
    if (ret != 0) {
        return ret;
    }

    ret = trs_get_sq_reg_vaddr(&inst, data->sq_id, &va, &size);
    if ((ret != 0) || (va == 0)) {
        trs_debug("Sq reg vaddr is not set. (ret=%d; sqid=%u; devid=%u; tsid=%u)\n",
            ret, data->sq_id, inst.devid, inst.tsid);
        return 0;
    }

    /* Find devipid from hostpid, should use virtual devid. vfid should be 0 */
    ret = devdrv_query_process_by_host_pid(data->remote_tgid, devid, DEVDRV_PROCESS_CP1, 0, &devpid);
    if (ret != 0) {
        trs_debug("Slave pid is not bind. (hostpid=%d; devid=%u)\n", data->remote_tgid, devid);
        return 0;
    }

    ret = trs_core_ops_unmap(devpid, va, size);
    if (ret != 0) {
        trs_err("Unmap fail. (ret=%d; devpid=%d)\n", ret, devpid);
        return ret;
    }
    trs_set_sq_reg_vaddr(&inst, data->sq_id, 0, 0);
    return 0;
}

static int trs_agent_ops_sq_reg_mmap(u32 devid, struct trs_msg_data *msg)
{
    return trs_pm_agent_sq_reg_map(devid, msg->header.tsid, (struct trs_msg_sqcq_sync *)msg->payload);
}

static int trs_agent_ops_sq_reg_unmap(u32 devid, struct trs_msg_data *msg)
{
    return trs_pm_agent_sq_reg_unmap(devid, msg->header.tsid, (struct trs_msg_sqcq_sync *)msg->payload);
}

int trs_agent_ops_dev_instance(u32 devid, u32 tsid)
{
    if (agent_ops[devid].trs_agent_instance == NULL) {
        trs_warn("Inst. (devid=%u; tsid=%u)\n", devid, tsid);
        return 0;
    }

    return agent_ops[devid].trs_agent_instance(devid, tsid);
}

void trs_agent_ops_dev_uninstance(u32 devid, u32 tsid)
{
    if (agent_ops[devid].trs_agent_uninstance == NULL) {
        trs_warn("Unins. (devid=%u; tsid=%u)\n", devid, tsid);
        return;
    }

    agent_ops[devid].trs_agent_uninstance(devid, tsid);
}

static const trs_msg_rdv_func_t rcv_ops[TRS_MSG_MAX] = {
    [TRS_MSG_GET_SSID] = trs_agent_ops_get_ssid,
    [TRS_MSG_RES_ID_CHECK] = trs_agent_ops_check_res_id,
    [TRS_MSG_GET_RES_ID] = trs_agent_ops_get_id,
    [TRS_MSG_PUT_RES_ID] = trs_agent_ops_put_id,
    [TRS_MSG_GET_RES_CAP] = trs_agent_ops_get_cap,
    [TRS_MSG_GET_PHY_ADDR] = trs_agent_ops_get_phy_addr,
    [TRS_MSG_GET_CQ_GROUP] = trs_agent_ops_get_cq_group,
    [TRS_MSG_GET_PROC_NUM] = trs_agent_ops_get_proc_num,
    [TRS_MSG_SQ_REG_MAP] = trs_agent_ops_sq_reg_mmap,
    [TRS_MSG_SQ_REG_UNMAP] = trs_agent_ops_sq_reg_unmap,
    [TRS_MSG_GET_RES_AVAIL_NUM] = trs_agent_ops_get_res_avail_num,
#ifdef CFG_FEATURE_CDQM
    [TRS_MSG_CDQM_INIT] = cdqm_proc_sync_msg,
    [TRS_MSG_CDQM_CREATE] = cdqm_proc_sync_msg,
    [TRS_MSG_CDQM_DESTROY] = cdqm_proc_sync_msg,
    [TRS_MSG_CDQM_BATCH_ABNORMAL] = cdqm_proc_sync_msg
#endif
};

static int trs_agent_rcv_msg_check(u32 devid, struct trs_msg_data *data, size_t size)
{
    if (data == NULL) {
        trs_err("Msg is Null. (devid=%u)\n", devid);
        return -EINVAL;
    }

    if (size != sizeof(struct trs_msg_data)) {
        trs_err("Invalid size=%lu\n", size);
        return -EINVAL;
    }

    if (data->header.cmdtype >= TRS_MSG_MAX) {
        trs_err("Invalid cmd. (devid=%u; cmd=%u)\n", devid, data->header.cmdtype);
        return -EINVAL;
    }

    if (data->header.valid != TRS_MSG_SEND_MAGIC) {
        trs_err("Invalid magic. (devid=%u; magic=0x%x)\n", devid, data->header.valid);
        return -EINVAL;
    }

    if (data->header.tsid >= TRS_TS_MAX_NUM) {
        trs_err("Invalid tsid. (tsid=%u)\n", data->header.tsid);
        return -EINVAL;
    }

    return 0;
}

int trs_agent_rcv_handle(u32 devid, void *msg, size_t size)
{
    struct trs_msg_data *data = (struct trs_msg_data *)msg;
    u32 cmd;

    if (trs_agent_rcv_msg_check(devid, data, size) != 0) {
        return -EINVAL;
    }
    cmd = data->header.cmdtype;

    if (rcv_ops[cmd] != NULL) {
        int ret = rcv_ops[cmd](devid, data);
        data->header.valid = TRS_MSG_RCV_MAGIC;
        data->header.result = ret;
    }

    return 0;
}
