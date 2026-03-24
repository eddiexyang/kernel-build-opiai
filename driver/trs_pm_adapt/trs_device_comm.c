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
* Create: 2022-8-15
*/
#include <linux/compiler_types.h>

#include "devdrv_manager_comm.h"
#include "dbl/chip_config.h"
#include "trs_id.h"
#include "trs_pm_adapt.h"
#include "trs_device_comm.h"
#include "trs_pub_def.h"
#include "trs_mailbox_def.h"
#include "trs_chan.h"
#include "trs_abnormal_info.h"

#define TRS_CACHELINE_SIZE           (64)
#define TRS_CACHELINE_MASK           (TRS_CACHELINE_SIZE - 1)

int trs_tscpu_chan[TRS_TS_INST_MAX_NUM];
int svm_get_pasid(pid_t vpid, int dev_id __maybe_unused);

static int _trs_device_get_ssid(u32 devid, pid_t pid)
{
    return svm_get_pasid(pid, devid);
}

int trs_device_get_ssid(struct trs_id_inst *inst, int *ssid)
{
    pid_t vnr = task_tgid_vnr(current);

    *ssid = _trs_device_get_ssid(inst->devid, vnr);
    return (*ssid < 0) ? *ssid : 0;
}
EXPORT_SYMBOL(trs_device_get_ssid);

int trs_device_get_remote_ssid(u32 devid, u32 vfid, pid_t hpid)
{
    int ssid, pid, ret;

    ret = devdrv_query_process_by_host_pid(hpid, devid, DEVDRV_PROCESS_CP1, vfid, &pid);
    if (ret != 0) {
        trs_err("Get pid fail. (hpid=%u; devid=%u; vfid=%u; ret=%d)\n", hpid, devid, vfid, ret);
        return ret;
    }
    ssid = _trs_device_get_ssid(devid, pid);
    if (ssid < 0) {
        trs_err("Get ssid fail. (devid=%u; pid=%d)\n", devid, pid);
    }
    return ssid;
}
EXPORT_SYMBOL(trs_device_get_remote_ssid);

void trs_flush_cache(u64 base, u32 len)
{
#ifdef __aarch64__
    u64 addr_loop, addr_end;

    addr_loop = base & (~TRS_CACHELINE_MASK);
    addr_end = (base + len + TRS_CACHELINE_MASK) & (~TRS_CACHELINE_MASK);

    asm volatile("dsb st"
                 :
                 :
                 : "memory");
    for (; addr_loop < addr_end;) {
        asm volatile("DC CIVAC ,%x0" ::"r"(addr_loop));
        mb();
        addr_loop += TRS_CACHELINE_SIZE;
    }
    asm volatile("dsb st"
                 :
                 :
                 : "memory");
#endif
}
EXPORT_SYMBOL(trs_flush_cache);

void trs_invalid_cache(u64 base, u32 len)
{
#ifdef __aarch64__
    u64 addr_loop, addr_end;

    addr_loop = base & (~TRS_CACHELINE_MASK);
    addr_end = (base + len + TRS_CACHELINE_MASK) & (~TRS_CACHELINE_MASK);

    asm volatile("dsb st"
                 :
                 :
                 : "memory");
    for (; addr_loop < addr_end;) {
        asm volatile("DC CIVAC ,%x0" ::"r"(addr_loop));
        mb();
        addr_loop += TRS_CACHELINE_SIZE;
    }
    asm volatile("dsb st"
                 :
                 :
                 : "memory");
#endif
}
EXPORT_SYMBOL(trs_invalid_cache);

static void trs_set_tscpu_chan(struct trs_id_inst *inst, int chan_id)
{
    trs_tscpu_chan[trs_id_inst_to_ts_inst(inst)] = chan_id;
}

static int trs_get_tscpu_chan(struct trs_id_inst *inst)
{
    return trs_tscpu_chan[trs_id_inst_to_ts_inst(inst)];
}

int trs_tscpu_chan_create(struct trs_id_inst *inst)
{
    struct trs_chan_para chan_para = {0};
    int ret, chan_id;

    chan_para.types.type = CHAN_TYPE_HW;
    chan_para.types.sub_type = CHAN_SUB_TYPE_HW_RSV_TS;

    chan_para.ops.cqe_is_valid = NULL;
    chan_para.ops.get_sq_head_in_cqe = NULL;
    chan_para.ops.cq_recv = NULL;
    chan_para.ops.abnormal_proc = NULL;

    chan_para.flag |= (0x1 << CHAN_FLAG_NO_ALLOC_SQCQ_MEM_BIT);
    chan_para.flag |= (0x1 << CHAN_FLAG_ALLOC_SQ_BIT);
    chan_para.flag |= (0x1 << CHAN_FLAG_ALLOC_CQ_BIT);
    chan_para.flag |= (0x1 << CHAN_FLAG_NOTICE_TS_BIT);
    ret = hal_kernel_trs_chan_create(inst, &chan_para, &chan_id);
    if (ret != 0) {
        trs_err("Chan create fail. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    trs_set_tscpu_chan(inst, chan_id);

    return 0;
}
EXPORT_SYMBOL(trs_tscpu_chan_create);

void trs_tscpu_chan_destroy(struct trs_id_inst *inst)
{
    hal_kernel_trs_chan_destroy(inst, trs_get_tscpu_chan(inst));
}
EXPORT_SYMBOL(trs_tscpu_chan_destroy);

int trs_get_ts_nid(u32 devid)
{
    int nid_cnt = 1, nid = NUMA_NO_NODE;

    (void)dbl_get_ts_nid(devid, &nid, nid_cnt);
    return nid;
}

int trs_ts_adapt_abnormal_proc(u32 devid, u32 tsid, void *data)
{
#ifndef EMU_ST
    struct stars_abnormal_info *abnormal_info = (struct stars_abnormal_info *)data;
    struct trs_id_inst inst;

    if (abnormal_info == NULL) {
        trs_err("Data is NULL. (udevid=%u; tsid=%u)\n", devid, tsid);
        return -EFAULT;
    }

    trs_id_inst_pack(&inst, devid, tsid);
    return trs_chan_abnormal_proc(&inst, abnormal_info);
#else
    return 0;
#endif
}

static u32 feature_mode = ALL_FEATURE_MODE;

void trs_set_feature_mode(void)
{
    feature_mode = PART_FEATURE_MODE;
}
EXPORT_SYMBOL(trs_set_feature_mode);

u32 trs_get_feature_mode(void)
{
    return feature_mode;
}
EXPORT_SYMBOL(trs_get_feature_mode);
