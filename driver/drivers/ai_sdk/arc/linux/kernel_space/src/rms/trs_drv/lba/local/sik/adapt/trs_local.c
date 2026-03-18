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
* Create: 2023-8-7
*/
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>

#include "soc_res.h"
#include "dbl/uda.h"

#include "trs_cdqm.h"
#include "trs_chan_update.h"
#include "trs_chip_def.h"

#include "trs_device_mbox.h"
#include "trs_device_id.h"
#include "trs_device_chan.h"
#include "trs_core_smmu.h"
#include "trs_device_core.h"
#include "trs_abnormal_info.h"
#include "trs_core.h"

typedef int (* trs_config_list)(struct trs_id_inst *);
typedef void (* trs_decofnig_list)(struct trs_id_inst *);

static const trs_config_list g_trs_func_init[] = {
    trs_id_config,
    trs_chan_config,
    trs_core_config
};

static const trs_decofnig_list g_trs_func_uninit[] = {
    trs_id_deconfig,
    trs_chan_deconfig,
    trs_core_deconfig
};

static int trs_ts_func_init(struct trs_id_inst *inst)
{
    int type, ret, i;

    for (type = 0; type < (int)ARRAY_SIZE(g_trs_func_init); type++) {
        ret = g_trs_func_init[type](inst);
        if (ret != 0) {
            for (i = type - 1; i >= 0; i--) {
                g_trs_func_uninit[i](inst);
            }
            trs_err("Trs ts func init fail. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, type);
            return ret;
        }
    }
    return 0;
}

static void trs_ts_func_uninit(struct trs_id_inst *inst)
{
    int type;

    for (type = ARRAY_SIZE(g_trs_func_uninit) - 1; type >= 0; type--) {
        g_trs_func_uninit[type](inst);
    }
}

static int trs_ts_adapt_init(struct trs_id_inst *inst)
{
    struct trs_res_ops ops = {0};
    int type;

    for (type = 0; type < TRS_CORE_MAX_ID_TYPE; type++) {
        ops.res_belong_proc[type] = trs_res_is_belong_to_proc;
    }

    trs_res_ops_register(inst->devid, &ops);

    return 0;
}

static void trs_ts_adapt_uninit(struct trs_id_inst *inst)
{
    trs_res_ops_unregister(inst->devid);
}

static int trs_ts_init(struct trs_id_inst *inst)
{
    int ret;

    ret = trs_smmu_config(inst);
    if (ret != 0) {
        return ret;
    }

    (void)trs_mbox_config(inst);

    ret = trs_ts_func_init(inst);
    if (ret != 0) {
        trs_mbox_deconfig(inst);
        trs_smmu_deconfig(inst);
        return ret;
    }

    ret = trs_ts_adapt_init(inst);
    if (ret != 0) {
        trs_ts_func_uninit(inst);
        trs_mbox_deconfig(inst);
        trs_smmu_deconfig(inst);
    }

    return ret;
}

static void trs_ts_uninit(struct trs_id_inst *inst)
{
    trs_ts_adapt_uninit(inst);
    trs_ts_func_uninit(inst);
    trs_mbox_deconfig(inst);
    trs_smmu_deconfig(inst);
}

static int trs_device_init(u32 phy_devid)
{
    struct trs_id_inst inst;
    u32 ts_num, tsid, i;
    int ret;

    ret = soc_resmng_subsys_get_num(phy_devid, TS_SUBSYS, &ts_num);
    if ((ret != 0) || (ts_num == 0) || (ts_num > TRS_TS_MAX_NUM)) {
        trs_err("Init get ts num failed. (devid=%u; ts_num=%u; ret=%d)\n", phy_devid, ts_num, ret);
        return -EFAULT;
    }

    for (tsid = 0; tsid < ts_num; tsid++) {
        trs_id_inst_pack(&inst, phy_devid, tsid);
        ret = trs_ts_init(&inst);
        if (ret != 0) {
            for (i = 0; i < tsid; i++) {
                trs_id_inst_pack(&inst, phy_devid, i);
                trs_ts_uninit(&inst);
            }
            trs_err("Init ts init failed. (devid=%u; tsid=%u; ret=%d)\n", phy_devid, tsid, ret);
            return -ENODEV;
        }
    }

    trs_info("Init success. (devid=%u)\n", phy_devid);

    return 0;
}

static void trs_device_uninit(u32 phy_devid)
{
    struct trs_id_inst inst;
    u32 ts_num, tsid;
    int ret;

    ret = soc_resmng_subsys_get_num(phy_devid, TS_SUBSYS, &ts_num);
    if ((ret != 0) || (ts_num == 0) || (ts_num > TRS_TS_MAX_NUM)) {
        return;
    }

    for (tsid = 0; tsid < ts_num; tsid++) {
        trs_id_inst_pack(&inst, phy_devid, tsid);
        trs_ts_uninit(&inst);
    }
}

#define TRS_SIK_NOTIFIER "trs_sik_adapt"
int trs_sik_adapt_notifier_func(u32 udevid, enum uda_notified_action action)
{
    struct trs_id_inst pm_inst;
    int ret = 0;

    trs_id_inst_pack(&pm_inst, udevid, 0);
    if (action == UDA_INIT) {
        ret = trs_device_init(udevid);
    }
    if (action == UDA_UNINIT) {
        trs_device_uninit(udevid);
    }

    trs_info("notifier action. (udevid=%u; action=%d)\n", udevid, action);

    return ret;
}

int __init init_trs_sik_adapt(void)
{
    struct uda_dev_type type;
    int ret;

    uda_dev_type_pack(&type, UDA_KUNPENG, UDA_ENTITY, UDA_LOCAL, UDA_REAL);
    ret = uda_notifier_register(TRS_SIK_NOTIFIER, &type, UDA_PRI1, trs_sik_adapt_notifier_func);
    if (ret != 0) {
        trs_err("Register notifier failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}

void __exit exit_trs_sik_adapt(void)
{
    struct uda_dev_type type;

    uda_dev_type_pack(&type, UDA_KUNPENG, UDA_ENTITY, UDA_LOCAL, UDA_REAL);
    (void)uda_notifier_unregister(TRS_SIK_NOTIFIER, &type);
}
module_init(init_trs_sik_adapt);
module_exit(exit_trs_sik_adapt);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("KUNPENG TRS DEVICE PM ADAPT DRVIER");
