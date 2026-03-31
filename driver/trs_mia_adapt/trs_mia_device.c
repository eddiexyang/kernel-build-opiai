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
* Create: 2022-10-12
*/
#include <linux/module.h>

#include "devdrv_interface.h"
#include "virtmng_interface.h"
#include "soc_res.h"

#include "trs_chip_def.h"
#include "trs_pub_def.h"
#include "trs_mia_cfg.h"
#include "trs_mia_device_id.h"
#include "trs_mia_device_chan.h"
#include "trs_mia_device_core.h"
#include "trs_mia_info_ts.h"
#include "trs_chip_def_comm.h"
#include "trs_device_comm.h"
#include "trs_chan_update.h"
#include "trs_cdqm.h"
#include "trs_mia_device_db.h"
#include "trs_ts_status.h"
typedef int (* trs_mia_config_list)(struct trs_id_inst *);
typedef void (* trs_mia_decofnig_list)(struct trs_id_inst *);
static const trs_mia_config_list g_trs_mia_hw_init[] = {
    trs_mia_ts_db_config,
};

static const trs_mia_decofnig_list g_trs_mia_hw_uninit[] = {
    trs_mia_ts_db_deconfig,
};

static const trs_mia_config_list g_trs_mia_func_init[] = {
    trs_mia_device_id_config,
    trs_mia_device_chan_config,
    trs_mia_device_core_config
};

static const trs_mia_decofnig_list g_trs_mia_func_uninit[] = {
    trs_mia_device_id_deconfig,
    trs_mia_device_chan_deconfig,
    trs_mia_device_core_deconfig
};

static int trs_mia_hw_init(struct trs_id_inst *inst)
{
    int type, ret, i;

    for (type = 0; type < (int)ARRAY_SIZE(g_trs_mia_hw_init); type++) {
        ret = g_trs_mia_hw_init[type](inst);
        if (ret != 0) {
            for (i = type - 1; i >= 0; i--) {
                g_trs_mia_hw_uninit[i](inst);
            }
            return ret;
        }
    }

    return 0;
}

static void trs_mia_hw_uninit(struct trs_id_inst *inst)
{
    int type;

    for (type = ARRAY_SIZE(g_trs_mia_hw_uninit) - 1; type >= 0; type--) {
        g_trs_mia_hw_uninit[type](inst);
    }
}

static int trs_mia_func_init(struct trs_id_inst *inst)
{
    int type, ret, i;

    for (type = 0; type < (int)ARRAY_SIZE(g_trs_mia_func_init); type++) {
        ret = g_trs_mia_func_init[type](inst);
        if (ret != 0) {
            for (i = type - 1; i >= 0; i--) {
                g_trs_mia_func_uninit[i](inst);
            }
            return ret;
        }
    }

    return 0;
}

static void trs_mia_func_uninit(struct trs_id_inst *inst)
{
    int type;

    for (type = ARRAY_SIZE(g_trs_mia_func_uninit) - 1; type >= 0; type--) {
        g_trs_mia_func_uninit[type](inst);
    }
}

static int trs_mia_adapt_init(struct trs_id_inst *inst)
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

static void trs_mia_adapt_uninit(struct trs_id_inst *inst)
{
    if (trs_get_hw_type() == TRS_HW_TYPE_STARS) {
        trs_tscpu_chan_destroy(inst);
    }
}

int trs_mia_device_init(struct trs_id_inst *inst)
{
    int ret;

    ret = trs_mia_hw_init(inst);
    if (ret != 0) {
        return ret;
    }

    ret = trs_mia_func_init(inst);
    if (ret != 0) {
        trs_mia_hw_uninit(inst);
        return ret;
    }

    ret = trs_mia_adapt_init(inst);
    if (ret != 0) {
        trs_mia_hw_uninit(inst);
        trs_mia_func_uninit(inst);
        return ret;
    }

    trs_ts_status_mng_init(inst);

    return ret;
}

void trs_mia_device_uninit(struct trs_id_inst *inst)
{
    trs_ts_status_mng_exit(inst);
    trs_mia_adapt_uninit(inst);
    trs_mia_func_uninit(inst);
    trs_mia_hw_uninit(inst);
}

static int trs_mia_device_instance_check(struct vmngd_client_instance *instance)
{
    if (instance == NULL) {
        trs_err("The instance is NULL.\n");
        return -EINVAL;
    }

    if (instance->type != VMNGD_CLIENT_TYPE_TSDRV) {
        trs_err("The instance type is not correct. (type=%d)\n", instance->type);
        return -EINVAL;
    }

    if ((instance->vdev_ctrl.dev_id >= TRS_DEVICE_MAX_DEV) || (instance->vdev_ctrl.vfid >= TRS_DEVICE_VIR_DEV_NUM)) {
        trs_err("The devid or vfid is invalid. (devid=%u; vfid=%u)\n",
            instance->vdev_ctrl.dev_id, instance->vdev_ctrl.vfid);
        return -EINVAL;
    }

    trs_info("Start to init VF instance. (devid=%u; dtype=%u; id_bitmap=0x%x; aic_bitmap=0x%x)\n",
        instance->vdev_ctrl.dev_id, instance->vdev_ctrl.dtype,
        instance->vdev_ctrl.vf_cfg.accelerator.rtsq_slice_bitmap,
        instance->vdev_ctrl.vf_cfg.accelerator.aic_bitmap);
    return 0;
}

static int trs_mia_device_init_instance(struct vmngd_client_instance *instance)
{
    struct trs_id_inst inst;
    u32 devid, ts_num, tsid, i, pf_id, vf_id;
    int ret;

    ret = trs_mia_device_instance_check(instance);
    if (ret != 0) {
        return ret;
    }

    devid = instance->vdev_ctrl.dev_id;
    ret = vmngd_get_pfvf_id_by_devid(devid, &pf_id, &vf_id);
    if (ret != 0) {
        trs_err("Get pf vf id failed. (devid=%u; pf_id=%u; vf_id=%u)\n", devid, pf_id, vf_id);
        return ret;
    }

    ret = soc_resmng_subsys_get_num(pf_id, TS_SUBSYS, &ts_num);
    if ((ret != 0) || (ts_num == 0) || (ts_num > TRS_TS_MAX_NUM)) {
        return -EINVAL;
    }

    /* for miniv3 */
    if ((instance->vdev_ctrl.dtype >= VMNG_HW_TYPE_C1_4) &&
        (instance->vdev_ctrl.dtype < VMNG_HW_TYPE_EXTRA_MAX)) {
        for (tsid = 0; tsid < ts_num; tsid++) {
            trs_id_inst_pack(&inst, pf_id, tsid);
            ret = trs_mia_notice_ts(&inst, vf_id, &instance->vdev_ctrl, 1);
            if (ret != 0) {
                return ret;
            }
        }
    }

    for (tsid = 0; tsid < ts_num; tsid++) {
        trs_id_inst_pack(&inst, devid, tsid);
        ret = trs_mia_cfg_create(&inst, instance);
        if (ret != 0) {
            goto return_err;
        }

        ret = trs_mia_device_init(&inst);
        if (ret != 0) {
            trs_mia_cfg_destroy(&inst);
            goto return_err;
        }
    }

    return 0;

return_err:
    for (i = 0; i < tsid; i++) {
        trs_id_inst_pack(&inst, devid, i);
        trs_mia_device_uninit(&inst);
        trs_mia_cfg_destroy(&inst);
    }
    return ret;
}

static int trs_mia_device_uninit_instance(struct vmngd_client_instance *instance)
{
    struct trs_id_inst inst;
    u32 devid, ts_num, tsid, pf_id, vf_id;
    int ret;

    ret = trs_mia_device_instance_check(instance);
    if (ret != 0) {
        return ret;
    }

    devid = instance->vdev_ctrl.dev_id;
    ret = vmngd_get_pfvf_id_by_devid(devid, &pf_id, &vf_id);
    if (ret != 0) {
        trs_err("Get pf vf id failed. (devid=%u)\n", devid);
        return ret;
    }

    ret = soc_resmng_subsys_get_num(pf_id, TS_SUBSYS, &ts_num);
    if ((ret != 0) || (ts_num == 0) || (ts_num > TRS_TS_MAX_NUM)) {
        return ret;
    }

    for (tsid = 0; tsid < ts_num; tsid++) {
        trs_id_inst_pack(&inst, devid, tsid);
        trs_mia_device_uninit(&inst);
        trs_mia_cfg_destroy(&inst);
    }

    /* for miniv3 */
    if ((instance->vdev_ctrl.dtype >= VMNG_HW_TYPE_C1_4) &&
    (instance->vdev_ctrl.dtype < VMNG_HW_TYPE_EXTRA_MAX)) {
        for (tsid = 0; tsid < ts_num; tsid++) {
            trs_id_inst_pack(&inst, pf_id, tsid);
            (void)trs_mia_notice_ts(&inst, vf_id, &instance->vdev_ctrl, 0);
        }
    }

    return 0;
}

static struct vmngd_client trs_mia_device_client = {
    .type = VMNGD_CLIENT_TYPE_TSDRV,
    .init_instance = trs_mia_device_init_instance,
    .uninit_instance = trs_mia_device_uninit_instance,
};

void trs_mia_device_register_client(void)
{
    int ret;

    ret = vmngd_register_client(&trs_mia_device_client);
    if (ret != 0) {
        trs_err("Vmngd_register_client failed. (ret=%d)\n", ret);
        return;
    }
}

void trs_mia_device_unregister_client(void)
{
    int ret;

    ret = vmngd_unregister_client(&trs_mia_device_client);
    if (ret != 0) {
        trs_err("Vmngd_unregister_client failed. (ret=%d)\n", ret);
        return;
    }

    trs_info("Unregister vmngd client success.\n");
}

int __init trs_mia_device_adapt_init(void)
{
    trs_mia_device_register_client();
    return 0;
}

void __exit trs_mia_device_adapt_exit(void)
{
    trs_info("Exit trs mia device\n");

    trs_mia_device_unregister_client();
}

module_init(trs_mia_device_adapt_init);
module_exit(trs_mia_device_adapt_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("TRS DEVICE MIA ADAPT DRVIER");

