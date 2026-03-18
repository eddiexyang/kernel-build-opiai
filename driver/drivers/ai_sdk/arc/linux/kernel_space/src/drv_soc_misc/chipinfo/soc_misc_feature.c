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

#include "soc_misc_feature.h"
#include "soc_misc_info.h"
#include "soc_misc_adapt.h"
#include "soc_misc_chip_info.h"
#include "soc_misc_init.h"
#include "soc_misc_board_info.h"

#include "dms_define.h"
#include "dms_template.h"
#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "boardinfo.h"
#include "soc_spec.h"
#include "slot_id.h"

#ifdef CFG_FEATURE_SRIOV
#include "devdrv_interface.h"
#endif

#ifdef CFG_FEATURE_DEVICE_DEVID_CONVERT
#include "devdrv_manager_container.h"
#endif

BEGIN_DMS_MODULE_DECLARATION(DMS_SOC_CMD_NAME)
BEGIN_FEATURE_COMMAND()
ADD_FEATURE_COMMAND(DMS_SOC_CMD_NAME, DMS_GET_BOARD_ID_CMD, ZERO_CMD, NULL, NULL,
                    DMS_SUPPORT_ALL, soc_misc_get_board_id)
ADD_FEATURE_COMMAND(DMS_SOC_CMD_NAME, DMS_MAIN_CMD_SOC, DMS_SUBCMD_GET_PCB_ID, NULL, NULL,
                    DMS_SUPPORT_ALL, soc_misc_get_pcb_id)
ADD_FEATURE_COMMAND(DMS_SOC_CMD_NAME, DMS_MAIN_CMD_SOC, DMS_SUBCMD_GET_BOM_ID, NULL, NULL,
                    DMS_SUPPORT_ALL, soc_misc_get_bom_id)
ADD_FEATURE_COMMAND(DMS_SOC_CMD_NAME, DMS_MAIN_CMD_SOC, DMS_SUBCMD_GET_SLOT_ID, NULL, NULL,
                    DMS_SUPPORT_ALL, soc_misc_get_slot_id)
ADD_FEATURE_COMMAND(DMS_SOC_CMD_NAME, DMS_MAIN_CMD_SOC, DMS_SUBCMD_GET_CPU_INFO, NULL, NULL,
                    DMS_SUPPORT_ALL, soc_misc_get_cpu_info)
ADD_FEATURE_COMMAND(DMS_SOC_CMD_NAME, DMS_GET_GET_DIE_ID_CMD, ZERO_CMD, NULL, NULL,
                    DMS_SUPPORT_ALL, soc_misc_feature_get_die_id)
#ifdef CFG_FEATURE_CHIP_EXPAND_VERSION
ADD_FEATURE_COMMAND(DMS_SOC_CMD_NAME, DMS_GET_CHIP_EXPAND_VERSION_CMD, ZERO_CMD, NULL, NULL,
                    DMS_SUPPORT_ALL, soc_misc_feature_get_chip_version)
#endif
END_FEATURE_COMMAND()
END_MODULE_DECLARATION()

void soc_misc_feature_init(void)
{
    CALL_INIT_MODULE(DMS_SOC_CMD_NAME);
}

void soc_misc_feature_uninit(void)
{
    CALL_EXIT_MODULE(DMS_SOC_CMD_NAME);
}

STATIC int soc_misc_feature_para_check(char *in, u32 in_len, u32 correct_in_len,
    char *out, u32 out_len, u32 correct_out_len)
{
    if ((in == NULL) || (in_len != correct_in_len)) {
        soc_misc_drv_err("Input char is NULL or in_len is wrong. (in_len=%u; correct_in_len=%u)\n",
            in_len, correct_in_len);
        return -EINVAL;
    }

    if ((out == NULL) || (out_len != correct_out_len)) {
        soc_misc_drv_err("Output char is NULL or out_len is wrong. (out_len=%u; correct_out_len=%u)\n",
            out_len, correct_out_len);
        return -EINVAL;
    }

    return 0;
}

#ifndef UT_NOT_VCAST
int dms_trans_and_check_id(u32 logical_dev_id, u32 *physical_dev_id, u32 *vfid)
{
#ifdef CFG_FEATURE_DEVICE_DEVID_CONVERT
    if (devdrv_manager_container_logical_id_to_physical_id(logical_dev_id, physical_dev_id, vfid) != 0) {
        soc_misc_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u)\n", logical_dev_id);
        return -EINVAL;
    }

    if (VDAVINCI_IS_VDEV(*physical_dev_id)) {
        *physical_dev_id = VDAVINCI_GET_PFID(*physical_dev_id);
    }
#else
    *physical_dev_id = logical_dev_id;
    *vfid = 0;
#endif
    return 0;
}
#endif

int soc_misc_get_board_id(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct soc_misc_info_st *soc_info = NULL;
    u32 dev_id;
    int ret;
    u32 phys_id = 0, vfid = 0;

    ret = soc_misc_feature_para_check(in, in_len, sizeof(u32), out, out_len, sizeof(u32));
    if (ret != 0) {
        soc_misc_drv_err("Para check failed.\n");
        return ret;
    }

    dev_id = *(u32 *)in;
    ret = dms_trans_and_check_id(dev_id, &phys_id, &vfid);
    if (ret != 0) {
        soc_misc_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    soc_info = soc_misc_get_soc_info(phys_id);
    if (soc_info == NULL) {
        soc_misc_drv_err("Soc info is invalid. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    *(u32 *)out = soc_info->board_info.board_id;
    return 0;
}

int soc_misc_get_pcb_id(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct soc_misc_info_st *soc_info = NULL;
    u32 dev_id;
    int ret;
    u32 phys_id = 0, vfid = 0;

    ret = soc_misc_feature_para_check(in, in_len, sizeof(u32), out, out_len, sizeof(u32));
    if (ret != 0) {
        soc_misc_drv_err("Para check failed.\n");
        return ret;
    }

    dev_id = *(u32 *)in;
    ret = dms_trans_and_check_id(dev_id, &phys_id, &vfid);
    if (ret != 0) {
        soc_misc_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    soc_info = soc_misc_get_soc_info(phys_id);
    if (soc_info == NULL) {
        soc_misc_drv_err("Soc info is invalid. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    ret = soc_misc_init_pcb_id(soc_info);
    if (ret) {
        soc_misc_drv_err("Init pcb id failed. (ret=%d)\n", ret);
        return ret;
    }

    *(u32 *)out = soc_info->board_info.pcb_id;
    return 0;
}

int soc_misc_get_bom_id(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct soc_misc_info_st *soc_info = NULL;
    u32 dev_id;
    int ret;
    u32 phys_id = 0, vfid = 0;

    ret = soc_misc_feature_para_check(in, in_len, sizeof(u32), out, out_len, sizeof(u32));
    if (ret != 0) {
        soc_misc_drv_err("Para check failed.\n");
        return ret;
    }

    dev_id = *(u32 *)in;
    ret = dms_trans_and_check_id(dev_id, &phys_id, &vfid);
    if (ret != 0) {
        soc_misc_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    soc_info = soc_misc_get_soc_info(phys_id);
    if (soc_info == NULL) {
        soc_misc_drv_err("Soc info is invalid. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    ret = soc_misc_init_bom_id(soc_info);
    if (ret) {
        soc_misc_drv_err("Init bom id failed. (ret=%d)\n", ret);
        return ret;
    }

    *(u32 *)out = soc_info->board_info.bom_id;
    return 0;
}

int soc_misc_get_slot_id(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct soc_misc_info_st *soc_info = NULL;
    u32 dev_id;
    int ret;
    u32 phys_id = 0, vfid = 0;

    ret = soc_misc_feature_para_check(in, in_len, sizeof(u32), out, out_len, sizeof(u32));
    if (ret != 0) {
        soc_misc_drv_err("Para check failed.\n");
        return ret;
    }

    dev_id = *(u32 *)in;
    ret = dms_trans_and_check_id(dev_id, &phys_id, &vfid);
    if (ret != 0) {
        soc_misc_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    soc_info = soc_misc_get_soc_info(phys_id);
    if (soc_info == NULL) {
        soc_misc_drv_err("Soc info is invalid. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

#ifdef CFG_FEATURE_SLOT_ID_FROM_DEVINFO
    if (soc_misc_is_pcie_card(soc_info->board_info.board_id)) {
        ret = soc_misc_init_slot_id_from_devinfo(soc_info);
        if (ret != 0) {
            soc_misc_drv_err("Soc get slot_id from dev_info failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
            return -EINVAL;
        }
    }
#endif

    *(u32 *)out = soc_info->board_info.slot_id;
    return 0;
}

int soc_misc_get_cpu_info(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct soc_misc_info_st *soc_info = NULL;
    u32 dev_id;
    int ret;
    u32 phys_id = 0, vfid = 0;

    ret = soc_misc_feature_para_check(in, in_len, sizeof(u32), out, out_len, sizeof(struct soc_misc_cpu_info));
    if (ret != 0) {
        soc_misc_drv_err("Para check failed.\n");
        return ret;
    }

    dev_id = *(u32 *)in;
    ret = dms_trans_and_check_id(dev_id, &phys_id, &vfid);
    if (ret != 0) {
        soc_misc_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }
    soc_info = soc_misc_get_soc_info(phys_id);
    if (soc_info == NULL) {
        soc_misc_drv_err("Soc info is invalid. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    *(struct soc_misc_cpu_info *)out = soc_info->cpu_info;
    return 0;
}

int soc_misc_feature_get_die_id(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret;
    dms_soc_die_id_t die_id_info = {0};
    u32 dev_id = 0;
#ifdef CFG_FEATURE_SRIOV
    unsigned int pf_id = 0;
    unsigned int vf_id = 0;
#else
    u32 phys_id = 0, vfid = 0;
#endif

    ret = soc_misc_feature_para_check(in, in_len, sizeof(dms_soc_die_id_t), out, out_len, sizeof(dms_soc_die_id_t));
    if (ret != 0) {
        soc_misc_drv_err("Para check failed.\n");
        return ret;
    }

    die_id_info = *(dms_soc_die_id_t *)in;
    dev_id = die_id_info.dev_id;
#ifdef CFG_FEATURE_SRIOV
    ret = devdrv_get_pfvf_id_by_devid(die_id_info.dev_id, &pf_id, &vf_id);
    if (ret != 0) {
        soc_misc_drv_err("Get pf and vf id failed. (dev_id=%u)\n", die_id_info.dev_id);
        return ret;
    }

    die_id_info.dev_id = pf_id;
#else
    ret = dms_trans_and_check_id(die_id_info.dev_id, &phys_id, &vfid);
    if (ret != 0) {
        soc_misc_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n",
            die_id_info.dev_id, ret);
        return ret;
    }

    die_id_info.dev_id = phys_id;
#endif
    ret = soc_misc_get_die_id(&die_id_info);
    if (ret != 0) {
        soc_misc_drv_err("Soc info is invalid. (dev_id=%u, ret=%d)\n", die_id_info.dev_id, ret);
        return ret;
    }

    die_id_info.dev_id = dev_id;
    *(dms_soc_die_id_t *)out = die_id_info;
    return 0;
}

#ifdef CFG_FEATURE_CHIP_EXPAND_VERSION
int soc_misc_feature_get_chip_version(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret;
    u32 dev_id;
    u32 devnum = 0;
    u8 chip_version = 0;

    ret = soc_misc_feature_para_check(in, in_len, sizeof(u32), out, out_len, sizeof(u8));
    if (ret != 0) {
        soc_misc_drv_err("Para check failed.\n");
        return ret;
    }

    dev_id = *(u32 *)in;

    ret = devdrv_get_devnum(&devnum);
    if (ret != 0) {
        soc_misc_drv_err("Get device count failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    if (dev_id >= devnum) {
        soc_misc_drv_err("Dev_id is invalid. (dev_id=%u; devnum=%u)\n", dev_id, devnum);
        return -EINVAL;
    }

    ret = soc_misc_get_chip_version(dev_id, &chip_version);
    if (ret != 0) {
        soc_misc_drv_err("Get chip version failed. (ret=%d)\n", ret);
        return ret;
    }

    *(u8 *)out = chip_version;
    return 0;
}
#endif

