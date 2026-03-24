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
* Create: 2022-10-15
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
#include "trs_device_ts_db.h"
#include "trs_device_core.h"
#include "trs_device_tsfw_shr_mem.h"
#include "trs_device_comm.h"
#include "trs_abnormal_info.h"
#include "trs_core.h"
#include "trs_ts_status.h"

typedef int (* trs_config_list)(struct trs_id_inst *);
typedef void (* trs_decofnig_list)(struct trs_id_inst *);

static const trs_config_list g_trs_hw_init[] = {
    trs_ts_db_config,
    trs_mbox_config,
};

static const trs_decofnig_list g_trs_hw_uninit[] = {
    trs_ts_db_deconfig,
    trs_mbox_deconfig
};

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

static int trs_ts_hw_init(struct trs_id_inst *inst)
{
    int type, ret, i;

    for (type = 0; type < (int)ARRAY_SIZE(g_trs_hw_init); type++) {
        ret = g_trs_hw_init[type](inst);
        if (ret != 0) {
            for (i = type - 1; i >= 0; i--) {
                g_trs_hw_uninit[i](inst);
            }
            trs_err("Trs ts hw init fail. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, type);
            return ret;
        }
    }
    return 0;
}

static void trs_ts_hw_uninit(struct trs_id_inst *inst)
{
    int type;

    for (type = ARRAY_SIZE(g_trs_hw_uninit) - 1; type >= 0; type--) {
        g_trs_hw_uninit[type](inst);
    }
}

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
    int ret, type;

    if (trs_get_hw_type() == TRS_HW_TYPE_STARS) {
        ret = trs_tscpu_chan_create(inst);
        if (ret != 0) {
            trs_err("Trs id rsv config fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
            return ret;
        }
    }

    for (type = 0; type < TRS_CORE_MAX_ID_TYPE; type++) {
        ops.res_belong_proc[type] = trs_res_is_belong_to_proc;
    }

#ifdef CFG_FEATURE_TSCPU
    ops.res_belong_proc[TRS_CDQ] = NULL;
#else
#ifdef CFG_FEATURE_CDQM
    ops.res_belong_proc[TRS_CDQ] = cdqid_is_belong_to_proc;
#endif
#endif

    trs_res_ops_register(inst->devid, &ops);

    return 0;
}

static void trs_ts_adapt_uninit(struct trs_id_inst *inst)
{
    trs_res_ops_unregister(inst->devid);
    if (trs_get_hw_type() == TRS_HW_TYPE_STARS) {
        trs_tscpu_chan_destroy(inst);
    }
}

static int trs_ts_init(struct trs_id_inst *inst)
{
    int ret;

    ret = trs_ts_hw_init(inst);
    if (ret != 0) {
        return ret;
    }

    ret = trs_ts_func_init(inst);
    if (ret != 0) {
        trs_ts_hw_uninit(inst);
        return ret;
    }

    ret = trs_ts_adapt_init(inst);
    if (ret != 0) {
        trs_ts_func_uninit(inst);
        trs_ts_hw_uninit(inst);
        return ret;
    }

    trs_ts_status_mng_init(inst);
    return ret;
}

static void trs_ts_uninit(struct trs_id_inst *inst)
{
    trs_ts_status_mng_exit(inst);
    trs_ts_adapt_uninit(inst);
    trs_ts_func_uninit(inst);
    trs_ts_hw_uninit(inst);
}

int trs_device_init(u32 phy_devid)
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

    trs_tsfw_shr_mem_config(&inst);

    trs_debug("Init success. (devid=%u)\n", phy_devid);

    return 0;
}

void trs_device_uninit(u32 phy_devid)
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

#define TRS_LOCAL_NOTIFIER "trs_adapt"
static int trs_local_notifier_func(u32 udevid, enum uda_notified_action action)
{
    struct trs_id_inst pm_inst;
    int ret = 0;

    trs_id_inst_pack(&pm_inst, udevid, 0);
    if (action == UDA_INIT) {
        ret = trs_device_init(udevid);
    } else if (action == UDA_UNINIT) {
        trs_device_uninit(udevid);
    } else if ((action == UDA_TO_MIA) || (action == UDA_SUSPEND)) {
        u32 force = (action == UDA_TO_MIA) ? 0 : 1; /* 1 : suspend don't check whether proc exist. */
        ret = trs_set_ts_inst_feature_mode(&pm_inst, TRS_INST_PART_FEATUR_MODE, force);
        if (ret != 0) {
            trs_err("Failed to mia. (devid=%u; ret=%d)\n", udevid, ret);
            return ret;
        }
        if (action == UDA_TO_MIA) {
            trs_tscpu_chan_destroy(&pm_inst);
        }
    } else if ((action == UDA_TO_SIA) || (action == UDA_RESUME)) {
        if (action == UDA_TO_SIA) {
            ret = trs_tscpu_chan_create(&pm_inst);
        }
        (void)trs_set_ts_inst_feature_mode(&pm_inst, TRS_INST_ALL_FEATUR_MODE, 0);
    } else {
        /* do nothing */
    }

    trs_info("notifier action. (udevid=%u; action=%d)\n", udevid, action);

    return ret;
}

int __init init_trs_adapt(void)
{
    struct uda_dev_type type;
    int ret;

    uda_davinci_local_real_entity_type_pack(&type);
    ret = uda_notifier_register(TRS_LOCAL_NOTIFIER, &type, UDA_PRI1, trs_local_notifier_func);
    if (ret != 0) {
        trs_err("Register notifier failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = tsmng_register_abnormal_proc_func(trs_ts_adapt_abnormal_proc);
    if (ret != 0) {
        (void)uda_notifier_unregister(TRS_LOCAL_NOTIFIER, &type);
        trs_err("Register abnormal proc func failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}

void __exit exit_trs_adapt(void)
{
    struct uda_dev_type type;

    tsmng_unregister_abnormal_proc_func(trs_ts_adapt_abnormal_proc);
    uda_davinci_local_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(TRS_LOCAL_NOTIFIER, &type);
}

module_init(init_trs_adapt);
module_exit(exit_trs_adapt);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("TRS DEVICE PM ADAPT DRVIER");
