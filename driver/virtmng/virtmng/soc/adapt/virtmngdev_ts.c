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
* Create: 2022-04-06
*/

#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/bitmap.h>
#include <linux/list.h>
#include <asm/io.h>

#include "virtmngdev_ts.h"

STATIC long vmngd_get_sram_offset(u32 vfid)
{
    return (((vfid + 1) * 0x800) + 0x80);
}

#define CHIP_ID_INFO_ADDR 0x36EFD800 /* 0 is p0, 1 is p1,...7 is p7 */
#define CHIP_OFFSET  0x80000000000ULL
#define DIE_OFFSET 0x10000000000U
#define MAX_CHIP_ID 8 /* max is 8p */
#define BASE_VF_NUM 8
#define DIE_NUM_OF_CHIP 2
#define VMNGD_SURPORT_MAX_DEV 1

STATIC int vmngd_get_chip_id(void)
{
    void __iomem *chip_id_addr = NULL;
    static int chip_id = -1;

    if (chip_id >= 0 && chip_id < MAX_CHIP_ID) {
        return chip_id;
    }

    chip_id_addr = ioremap_cache(CHIP_ID_INFO_ADDR, sizeof(u32));
    if (chip_id_addr == NULL) {
        vmng_err("Ioremap chip id address error.\n");
        return -1;
    }
    chip_id = readl(chip_id_addr) & 0xFF;

    iounmap(chip_id_addr);
    chip_id_addr = NULL;

    if (chip_id >= MAX_CHIP_ID) {
        vmng_err("Get chip id(%d) is invalid.\n", chip_id);
        return -1;
    }

    vmng_info("Get chip id in 4p or 8p (chip_id=%d)\n", chip_id);
    return chip_id;
}

STATIC int vmngd_write_vf_info_to_sram(u32 dev_id, vmng_vf_cfg_t *cfg)
{
    unsigned char *write_data_addr = NULL;
    void __iomem *sram_remap_base = NULL;
    int chip_id;
    u32 i;

    if ((cfg == NULL) || (cfg->id.vf_id >= BASE_VF_NUM)) {
        vmng_err("Param error.\n");
        return VMNG_ERR;
    }

    chip_id = vmngd_get_chip_id();
    if (chip_id < 0) {
        vmng_err("Get chip id failed. (chip_id=%d)\n", chip_id);
        return -EINVAL;
    }
    sram_remap_base = ioremap(CHIP_OFFSET * chip_id + DIE_OFFSET * (dev_id % DIE_NUM_OF_CHIP) +
        STARS_RESOURCE_CFG_SRAM_BASE_ADDR, STARS_RESOURCE_CFG_SRAM_MAP_SIZE);
    if (sram_remap_base == NULL) {
        vmng_err("Ioremap sram fail.\n");
        return -ENOMEM;
    }

    for (i = 0; i < sizeof(vmng_vf_cfg_t); i++) {
        write_data_addr = (unsigned char *)cfg + i;
        writeb(*write_data_addr, (void __iomem *)(uintptr_t)((long)(uintptr_t)sram_remap_base +
            vmngd_get_sram_offset(cfg->id.vf_id) + i));
    }

    iounmap(sram_remap_base);
    sram_remap_base = NULL;

    return VMNG_OK;
}

STATIC int vmngd_ipc_msg_proc(u32 dev_id, u32 vfid, u32 cmd_type)
{
    int ret;
    ICM_HANDLE handle;
    vmng_ts_msg_t ts_vf_msg;
    u32 ack_data[IPCDRV_RPROC_MSG_LENGTH];
    struct icmdrv_ipc_msg ipc_msg = { 0 };

    ipc_msg.sub_cmd = (u8)cmd_type;
    ipc_msg.cmd = ICM_MAIN_CMD_TS_REQ;
    ipc_msg.cmd_dest = OBJ_CMD_TS;
    ipc_msg.cmd_src = OBJ_CMD_CCPU;
    ipc_msg.len = sizeof(vmng_ts_msg_t);
    ts_vf_msg.vfid = (u8)(vfid - 1);
    ret = memcpy_s(ipc_msg.data, ICM_MSG_DATA_LENGTH, &ts_vf_msg, sizeof(ts_vf_msg));
    if (ret != 0) {
        vmng_err("Vf ts msg memcpy_s failed.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return ret;
    }

    handle = ICM_FD_BUILD(dev_id, VMNGD_ICM_MBX_CHAN);
    if ((handle == ICM_INVAILED_FD) || (handle == -EINVAL)) {
        vmng_err("Alloc icm msg chan failed.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    ret =
        icm_msg_send_sync(handle, (rproc_msg_t *)&ipc_msg, IPCDRV_RPROC_MSG_LENGTH, ack_data, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != VMNG_OK) {
        vmng_warn("Icm msg send fail.(dev_id=%u; vfid=%u; sub_cmd=%d; ret=%d)\n", dev_id, vfid, ipc_msg.sub_cmd, ret);
    }

    handle = ICM_INVAILED_FD;

    return VMNG_OK;
}

int vmngd_send_msg_to_ts_create(u32 dev_id, u32 vfid, vmng_vf_cfg_t *cfg)
{
    int ret;

    ret = vmngd_write_vf_info_to_sram(dev_id, cfg);
    if (ret != VMNG_OK) {
        vmng_err("Write sram fail.(dev_id=%u; vfid=%u; ret=%d)\n", dev_id, vfid, ret);
        return VMNG_ERR;
    }

    ret = vmngd_ipc_msg_proc(dev_id, vfid, ICM_SUB_CMD_CREAT_VF);
    if (ret != VMNG_OK) {
        vmng_err("Ipc creat fail.(dev_id=%u; vfid=%u; ret=%d)\n", dev_id, vfid, ret);
        return VMNG_ERR;
    }

    return VMNG_OK;
}

int vmngd_send_msg_to_ts_destory(u32 dev_id, u32 vfid, vmng_vf_cfg_t *cfg)
{
    int ret;

    ret = vmngd_write_vf_info_to_sram(dev_id, cfg);
    if (ret != VMNG_OK) {
        vmng_err("Write sram fail.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    ret = vmngd_ipc_msg_proc(dev_id, vfid, ICM_SUB_CMD_DESTORY_VF);
    if (ret != VMNG_OK) {
        vmng_err("Ipc destory fail.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    return VMNG_OK;
}

int vmngd_send_msg_to_ts_reset(u32 dev_id, u32 vfid)
{
    if ((vfid > BASE_VF_NUM) || (vfid == 0) || (dev_id >= VMNGD_SURPORT_MAX_DEV)) {
        vmng_err("Param NULL.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    if (vmngd_ipc_msg_proc(dev_id, vfid, ICM_SUB_CMD_RESET_VF) != VMNG_OK) {
        vmng_err("Ipc reset fail.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    return VMNG_OK;
}
