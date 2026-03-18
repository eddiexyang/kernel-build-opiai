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
 * Create: 2023-2-1
 */

#include <linux/io.h>

#include "soc_res.h"
#include "trs_mailbox_def.h"
#include "trs_mia_info_ts.h"

#define TRS_INFO(var_struct, var_name, format) trs_info(#var_name ":" #format "\n", (var_struct.var_name))
static void trs_mia_print_cfg(const vmng_vf_cfg_t *cfg, u32 vfid)
{
    trs_info("Print vf%u info to ts begin =======================\n", vfid);
    TRS_INFO(cfg->accelerator, c_core_bitmap, "%#x");
    TRS_INFO(cfg->accelerator, dsa_bitmap, "%#x");
    TRS_INFO(cfg->accelerator, ffts_bitmap, "%#x");
    TRS_INFO(cfg->accelerator, sdma_bitmap, "%#x");
    TRS_INFO(cfg->accelerator, pcie_dma_bitmap, "%#x");
    TRS_INFO(cfg->accelerator, acsq_slice_bitmap, "%#x");
    TRS_INFO(cfg->accelerator, rtsq_slice_bitmap, "%#x");
    TRS_INFO(cfg->accelerator, event_slice_bitmap, "%#x");
    TRS_INFO(cfg->accelerator, notify_slice_bitmap, "%#x");
    TRS_INFO(cfg->accelerator, cdq_slice_bitmap, "%#x");
    TRS_INFO(cfg->accelerator, aic_bitmap, "%#x");
    TRS_INFO(cfg->accelerator, aiv_bitmap, "%#llx");
    TRS_INFO(cfg->dvpp, jpegd_bitmap, "%#x");
    TRS_INFO(cfg->dvpp, jpege_bitmap, "%#x");
    TRS_INFO(cfg->dvpp, vpc_bitmap, "%#x");
    TRS_INFO(cfg->dvpp, vdec_bitmap, "%#x");
    TRS_INFO(cfg->dvpp, pngd_bitmap, "%#x");
    TRS_INFO(cfg->dvpp, venc_bitmap, "%#x");
    TRS_INFO(cfg->cpu, topic_aicpu_slot_bitmap, "%#x");
    TRS_INFO(cfg->cpu, topic_ctrl_cpu_slot_bitmap, "%#x");
    TRS_INFO(cfg->cpu, host_ctrl_cpu_bitmap, "%#x");
    TRS_INFO(cfg->cpu, device_aicpu_bitmap, "%#x");
    TRS_INFO(cfg->cpu, host_aicpu_bitmap, "%#llx");
    TRS_INFO(cfg->id, vf_id, "%u");
    TRS_INFO(cfg->id, vfg_mode, "%u");
    TRS_INFO(cfg->id, vfg_id, "%u");
    TRS_INFO(cfg->id, vip, "%u");
    TRS_INFO(cfg->id, token, "%#llx");
    TRS_INFO(cfg->id, token_max, "%#llx");
    TRS_INFO(cfg->id, task_timeout, "%#llx");

    trs_info("Print vf%u info to ts end =======================\n", vfid);
}

static long trs_mia_get_sram_offset(u32 vfid)
{
    return (((vfid + 1) * 0x800) + 0x80);
}

static int trs_mia_write_info_to_sram(struct trs_id_inst *inst, vmng_vf_cfg_t *cfg)
{
    struct res_inst_info res_inst;
    struct soc_rsv_mem_info sram;
    void __iomem *dst_addr = NULL;
    int ret;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_rsv_mem(&res_inst, "TS_SRAM_MEM", &sram);
    if (ret != 0) {
        trs_err("Get sram addr failed. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    /* one VF need 2K size */
    dst_addr = ioremap((sram.rsv_mem + trs_mia_get_sram_offset(cfg->id.vf_id)), PAGE_SIZE / 2);
    if (dst_addr == NULL) {
        trs_err("Iomem remap fail. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -ENOMEM;
    }

    memcpy_toio(dst_addr, cfg, sizeof(vmng_vf_cfg_t));

    iounmap(dst_addr);
    dst_addr = NULL;

    return 0;
}

static int _trs_mia_notice_ts(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct trs_mb_header *header = (struct trs_mb_header *)(void *)msg;
    int ret = trs_mbox_send(inst, 0, msg, len, TRS_DEVICE_CHAN_MBOX_TIMEOUT_MS);
    if (((ret != 0) || (header->result != 0))) {
        trs_err("Ts resp failed. (devid=%u; tsid=%u; cmd_type=%u; ret=%d; result=%u)\n",
            inst->devid, inst->tsid, header->cmd_type, ret, header->result);
    }

    return ((ret == 0) && (header->result == 0)) ? 0 : -EFAULT;
}

static int trs_mia_get_mbox_cmd(u32 op)
{
    return (op != 0) ? TRS_MBOX_CREATE_MIA : TRS_MBOX_DESTROY_MIA;
}

int trs_mia_notice_ts(struct trs_id_inst *ts_inst, u32 vfid, struct vmng_vdev_ctrl *cfg, u32 op)
{
    struct trs_mia_cfg_msg msg = { 0 };
    int ret;

    ret = trs_mia_write_info_to_sram(ts_inst, &cfg->vf_cfg);
    if (ret) {
        trs_err("Failed write mia cfg to sram.\n");
        return ret;
    }

    trs_mia_print_cfg(&cfg->vf_cfg, vfid);

    trs_mbox_init_header(&msg.header, trs_mia_get_mbox_cmd(op));
    msg.vfid = vfid - 1;

    return _trs_mia_notice_ts(ts_inst, (u8 *)&msg, sizeof(msg));
}
