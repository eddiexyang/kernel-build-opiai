/* SPDX-License-Identifier: GPL-2.0+ */
// Copyright (c) 2016-2017 Hisilicon Limited.

#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/io.h>
#include <linux/time.h>
#include <linux/delay.h>

#include "securec.h"
#include "hclge_fault_manager.h"
#include "hns3_bbox.h"
#include "network_custom.h"
#include "ascend_kernel_hal.h"

static struct hns_bbox_cb g_hns_bbox_cb = {0};

static void hns_bbox_net_cb(u32 temp1, void *temp2)
{
    pr_info("hns3: %s !", __func__);
}

struct hns_bbox_cb *hns_bbox_get_cb(void)
{
    return &g_hns_bbox_cb;
}

bool hns_is_addr_in_white_list(u64 addr)
{
    static u64 addr_white_list[] = {
        /* hns imp share mem addr */
        HNS_IMP_SHARE_MEM_ADDR,
        /* bbox log mem addr */
        HNS_BBOX_LOG_MEM_ADDR,
    };
    size_t addr_white_list_size = sizeof(addr_white_list) / sizeof(u64);
    size_t i;

    for (i = 0; i < addr_white_list_size; i++) {
        if (addr_white_list[i] == addr) {
            return true;
        }
    }

    return false;
}

bool hns_imp_share_mem_is_valid(u64 addr, u32 len)
{
    if (!hns_is_addr_in_white_list(addr & HNS_BBOX_LOG_MEM_ADDR_MASK)) {
        pr_err("check imp share mem info failed, addr is invalid\n");
        return false;
    }

    if (len != HNS_IMP_SHARE_MEM_LEN) {
        pr_err("check imp share mem info failed, len[%u] is invalid, expect[%u]\n",
               len, HNS_IMP_SHARE_MEM_LEN);
        return false;
    }

    return true;
}

bool hns_bbox_log_mem_is_valid(void)
{
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();
    if (!hns_is_addr_in_white_list(hns_bbox->log_mem.log_addr & HNS_BBOX_LOG_MEM_ADDR_MASK)) {
        pr_err("check bbox share mem info failed, addr is invalid\n");
        return false;
    }

    if (hns_bbox->log_mem.log_len != HNS_BBOX_LOG_RESV_MEM_LEN) {
        pr_err("check bbox share mem info failed, len[%u] is invalid, expect[%u]\n",
               hns_bbox->log_mem.log_len, HNS_BBOX_LOG_RESV_MEM_LEN);
        return false;
    }

    return true;
}

static char *hns_bbox_devid2va_bbox(u32 devid)
{
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();

    if (devid < HNS_BBOX_DEV_NUM) {
        return hns_bbox->va_bbox[devid];
    }

    pr_err("hns3: bbox param error! devid:%u invalid, expect [0, %u).\n", devid, HNS_BBOX_DEV_NUM);
    return NULL;
}

static char *hns_bbox_devid2va_imp(u32 devid)
{
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();

    if (devid < HNS_BBOX_DEV_NUM) {
        return hns_bbox->va_imp[devid];
    }

    pr_err("hns3: bbox param error! devid:%u invalid, expect [0, %u)\n", devid, HNS_BBOX_DEV_NUM);
    return NULL;
}

static int hns_bbox_excep_dump_nic_roce(u32 devid, u32 excepid, const char *log_buf)
{
    struct hns_bbox_info *bbox_info = NULL;
    int ret;

    if (log_buf == NULL) {
        pr_err("hns3: bbox dump excep_id:0x%x fail, log_buf is null\n", excepid);
        return -EINVAL;
    }

    bbox_info = (struct hns_bbox_info *)hns_bbox_devid2va_bbox(devid);
    if (bbox_info == NULL) {
        pr_err("hns3: bbox dump excep_id:0x%x fail, bbox_info is null\n", excepid);
        return -EIO;
    }

    if ((bbox_info->pi + 1) % HNS_BBOX_LOG_MSG_MAX == bbox_info->ci) {
        pr_err("hns3: log share mem full, pi:%u, ci:%u >= %u.\n", bbox_info->pi, bbox_info->ci, HNS_BBOX_LOG_MSG_MAX);
        return -EIO;
    }

    ret = snprintf_s(bbox_info->log + (bbox_info->pi * HNS_BBOX_LOG_BUF_LEN), (size_t)HNS_BBOX_LOG_BUF_LEN, \
                     (size_t)(HNS_BBOX_LOG_BUF_LEN - 1), log_buf);
    if (ret == -1) {
        pr_err("hns3: bbox dump excep_id:0x%x fail, snprintf fail\n", excepid);
        return -ENOMEM;
    }

    bbox_info->pi = (bbox_info->pi + 1) % HNS_BBOX_LOG_MSG_MAX;

    pr_info("hns3: bbox dump excep_id:0x%x log success\n", excepid);
    return 0;
}

static void hns_bbox_excep_handler_nic_roce(const struct bbox_dump_ops_info *info)
{
    struct hns_bbox_info *bbox_info = NULL;

    bbox_info = (struct hns_bbox_info *)hns_bbox_devid2va_bbox(info->devid);
    if (bbox_info == NULL) {
        pr_err("hns3: bbox dump excep_id:0x%x fail, bbox_info is null\n", info->excepid);
        return;
    }

    if (bbox_info->ci == bbox_info->pi || bbox_info->ci >= HNS_BBOX_LOG_MSG_MAX) {
        pr_err("hns3: bbox dump excep_id:0x%x fail, pi:%u = ci:%u or ci >= %u.\n",
               info->excepid, bbox_info->pi, bbox_info->ci, HNS_BBOX_LOG_MSG_MAX);
        return;
    }

    bbox_info->head.magic = BBOX_MODULE_MAGIC;
    bbox_info->head.e_block_num = HNS_BBOX_BLOCK_NUM;
    bbox_info->head.block[HNS_BBOX_NIC_ROCE_BLOCK_IDX].e_clock = info->time;
    bbox_info->head.block[HNS_BBOX_NIC_ROCE_BLOCK_IDX].e_excepid = info->excepid;
    bbox_info->head.block[HNS_BBOX_NIC_ROCE_BLOCK_IDX].e_block_offset = sizeof(struct bbox_module_ctrl) + \
                                                                        bbox_info->ci * HNS_BBOX_LOG_BUF_LEN;
    bbox_info->head.block[HNS_BBOX_NIC_ROCE_BLOCK_IDX].e_block_len = HNS_BBOX_LOG_LEN_MAX;
    bbox_info->head.block[HNS_BBOX_NIC_ROCE_BLOCK_IDX].e_info_len = HNS_BBOX_LOG_BUF_LEN;

    bbox_info->ci = (bbox_info->ci + 1) % HNS_BBOX_LOG_MSG_MAX;

    return;
}

static void hns_bbox_excep_handler_imp(const struct bbox_dump_ops_info *info)
{
    struct hns_bbox_info *bbox_info = NULL;
    struct hns_bbox_imp_info *imp_info = NULL;

    bbox_info = (struct hns_bbox_info *)hns_bbox_devid2va_bbox(info->devid);
    if (bbox_info == NULL) {
        pr_err("hns3: bbox dump excep_id:0x%x fail, bbox_info is null\n", info->excepid);
        return;
    }

    imp_info = (struct hns_bbox_imp_info *)((char *)bbox_info + HNS_BBOX_IMP_ADDR_OFFSET);
    if (imp_info->magic != BBOX_MODULE_MAGIC) {
        pr_err("hns3: bbox dump excep_id:0x%x fail, magic:0x%x invalid\n", info->excepid, imp_info->magic);
        return;
    }

    if (imp_info->ci == imp_info->pi || imp_info->ci >= HNS_BBOX_IMP_MSG_MAX) {
        pr_err("hns3: bbox dump excep_id:0x%x fail, pi:%u = ci:%u or ci >= %u.\n",
               info->excepid, imp_info->pi, imp_info->ci, HNS_BBOX_IMP_MSG_MAX);
        return;
    }

    bbox_info->head.magic = BBOX_MODULE_MAGIC;
    bbox_info->head.e_block_num = HNS_BBOX_BLOCK_NUM;
    bbox_info->head.block[HNS_BBOX_IMP_BLOCK_IDX].e_clock = info->time;
    bbox_info->head.block[HNS_BBOX_IMP_BLOCK_IDX].e_excepid = info->excepid;
    bbox_info->head.block[HNS_BBOX_IMP_BLOCK_IDX].e_block_offset = imp_info->mem[imp_info->ci].offset + \
                                                                   HNS_BBOX_IMP_ADDR_OFFSET;
    bbox_info->head.block[HNS_BBOX_IMP_BLOCK_IDX].e_block_len = HNS_BBOX_IMP_LOG_LEN_MAX;
    bbox_info->head.block[HNS_BBOX_IMP_BLOCK_IDX].e_info_len = imp_info->mem[imp_info->ci].len;

    imp_info->ci = (imp_info->ci + 1) % HNS_BBOX_IMP_MSG_MAX;

    return;
}

static void hns_bbox_excep_handler_ras(const struct bbox_dump_ops_info *info)
{
    struct hns_bbox_info *bbox_info = NULL;
    struct hns_bbox_ras_info *ras_info = NULL;

    bbox_info = (struct hns_bbox_info *)hns_bbox_devid2va_bbox(info->devid);
    if (bbox_info == NULL) {
        pr_err("hns3: bbox dump excep_id:0x%x fail, bbox_info is null\n", info->excepid);
        return;
    }

    ras_info = (struct hns_bbox_ras_info *)((char *)bbox_info + HNS_BBOX_RAS_ADDR_OFFSET);
    if (ras_info->ci == ras_info->pi || ras_info->ci >= HNS_BBOX_RAS_MSG_MAX) {
        pr_err("hns3: bbox dump excep_id:0x%x fail, pi:%u = ci:%u or ci >= %u.\n",
               info->excepid, ras_info->pi, ras_info->ci, HNS_BBOX_RAS_MSG_MAX);
        return ;
    }

    bbox_info->head.magic = BBOX_MODULE_MAGIC;
    bbox_info->head.e_block_num = HNS_BBOX_BLOCK_NUM;
    bbox_info->head.block[HNS_BBOX_RAS_BLOCK_IDX].e_clock = info->time;
    bbox_info->head.block[HNS_BBOX_RAS_BLOCK_IDX].e_excepid = info->excepid;
    bbox_info->head.block[HNS_BBOX_RAS_BLOCK_IDX].e_block_offset = HNS_BBOX_RAS_ADDR_OFFSET + \
                                                                   sizeof(*ras_info) + \
                                                                   ras_info->ci * HNS_BBOX_RAS_BUF_LEN;
    bbox_info->head.block[HNS_BBOX_RAS_BLOCK_IDX].e_block_len = HNS_BBOX_RAS_BUF_LEN;
    bbox_info->head.block[HNS_BBOX_RAS_BLOCK_IDX].e_info_len = HNS_BBOX_RAS_BUF_LEN;

    ras_info->ci = (ras_info->ci + 1) % HNS_BBOX_RAS_MSG_MAX;

    return;
}

const struct hns_bbox_excep_ops g_hns_bbox_excep[] = {
    {
        {
            .e_excepid = HNS_EXCEPID_NIC_INIT,
            .e_excepid_end = HNS_EXCEPID_NIC_INIT,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "HNS_NIC_INIT",
            .e_desc = "HNS NIC initialize.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_nic_roce,
        .dump_log_buf = hns_bbox_excep_dump_nic_roce,
        .log_buf = "Init: HNS NIC",
    },

    {
        {
            .e_excepid = HNS_EXCEPID_NIC_UNINIT,
            .e_excepid_end = HNS_EXCEPID_NIC_UNINIT,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "HNS_NIC_UNINIT",
            .e_desc = "HNS NIC un-initialize.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_nic_roce,
        .dump_log_buf = hns_bbox_excep_dump_nic_roce,
        .log_buf = "Uninit: HNS NIC",
    },

    {
        {
            .e_excepid = HNS_EXCEPID_CUSTOM_INIT,
            .e_excepid_end = HNS_EXCEPID_CUSTOM_INIT,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "HNS_CUSTOM_INIT",
            .e_desc = "HNS CUSTOM initialize.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_nic_roce,
        .dump_log_buf = hns_bbox_excep_dump_nic_roce,
        .log_buf = "Init: HNS CUSTOM",
    },

    {
        {
            .e_excepid = HNS_EXCEPID_CUSTOM_UNINIT,
            .e_excepid_end = HNS_EXCEPID_CUSTOM_UNINIT,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "HNS_CUSTOM_UNINIT",
            .e_desc = "HNS CUSTOM un-initialize.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_nic_roce,
        .dump_log_buf = hns_bbox_excep_dump_nic_roce,
        .log_buf = "Uninit: HNS CUSTOM",
    },

    {
        {
            .e_excepid = HNS_EXCEPID_RoCE_INIT,
            .e_excepid_end = HNS_EXCEPID_RoCE_INIT,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "HNS_RoCE_INIT",
            .e_desc = "HNS RoCE initialize.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_nic_roce,
        .dump_log_buf = hns_bbox_excep_dump_nic_roce,
        .log_buf = "Init: HNS RoCE",
    },

    {
        {
            .e_excepid = HNS_EXCEPID_RoCE_UNINIT,
            .e_excepid_end = HNS_EXCEPID_RoCE_UNINIT,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "HNS_RoCE_UNINIT",
            .e_desc = "HNS RoCE un-initialize.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_nic_roce,
        .dump_log_buf = hns_bbox_excep_dump_nic_roce,
        .log_buf = "Uninit: HNS RoCE",
    },

    {
        {
            .e_excepid = HNS_EXCEPID_ABNM_CQ_CAUSE_CQE_ERR,
            .e_excepid_end = HNS_EXCEPID_ABNM_CQ_CAUSE_CQE_ERR,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "ABNM_CQ_CQE_Err",
            .e_desc = "Get Abnormal CQ Error.",
            .e_callback = NULL,
        },
        .handler = hns_bbox_excep_handler_nic_roce,
        .dump_log_buf = hns_bbox_excep_dump_nic_roce,
        .log_buf = "RoCE Get Abnormal CQ Error",
    },

    {
        {
            .e_excepid = HNS_EXCEPID_INV_REQ_LOCAL_WQ_ERR,
            .e_excepid_end = HNS_EXCEPID_INV_REQ_LOCAL_WQ_ERR,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "WQE_CONTENT_Err",
            .e_desc = "Get Disorder Roce Frame Error.",
            .e_callback = NULL,
        },
        .handler = hns_bbox_excep_handler_nic_roce,
        .dump_log_buf = hns_bbox_excep_dump_nic_roce,
        .log_buf = "RoCE Get invalid opcode Error",
    },

    {
        {
            .e_excepid = HNS_EXCEPID_LOC_WQ_ACCESS_ERR,
            .e_excepid_end = HNS_EXCEPID_LOC_WQ_ACCESS_ERR,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "LOC_WQ_ACS_Err",
            .e_desc = "Get Loc WQ Access Error.",
            .e_callback = NULL,
        },
        .handler = hns_bbox_excep_handler_nic_roce,
        .dump_log_buf = hns_bbox_excep_dump_nic_roce,
        .log_buf = "RoCE Get Loc WQ Access Error",
    },

    {
        {
            .e_excepid = HNS_EXCEPID_CQ_OVERFLOW_ERR,
            .e_excepid_end = HNS_EXCEPID_CQ_OVERFLOW_ERR,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "CQ_OVERFLOW_Err",
            .e_desc = "Get CQ Overflow Error.",
            .e_callback = NULL,
        },
        .handler = hns_bbox_excep_handler_nic_roce,
        .dump_log_buf = hns_bbox_excep_dump_nic_roce,
        .log_buf = "RoCE Get CQ overflow Error",
    },

    {
        {
            .e_excepid = HNS_EXCEPID_DB_OVERFLOW_ERR,
            .e_excepid_end = HNS_EXCEPID_DB_OVERFLOW_ERR,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "DB_OVERFLOW_Err",
            .e_desc = "Get DB Overflow Error.",
            .e_callback = NULL,
        },
        .handler = hns_bbox_excep_handler_nic_roce,
        .dump_log_buf = hns_bbox_excep_dump_nic_roce,
        .log_buf = "RoCE Get DB overflow Error",
    },

    {
        {
            .e_excepid = HNS_EXCEPID_IMP_LOG_FULL,
            .e_excepid_end = HNS_EXCEPID_IMP_LOG_FULL,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_NOTICE,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "HNS_IMP_LOG_FULL",
            .e_desc = "HNS IMP Log Full.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_imp,
    },

    {
        {
            .e_excepid = HNS_EXCEPID_IMP_RESET,
            .e_excepid_end = HNS_EXCEPID_IMP_RESET,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MINOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "HNS_IMP_RESET",
            .e_desc = "HNS IMP except reset.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_imp,
    },

    {
        {
            .e_excepid = HNS_EXCEPID_IMP_NMI_INTER,
            .e_excepid_end = HNS_EXCEPID_IMP_NMI_INTER,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MINOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "NMI_INTER",
            .e_desc = "HNS IMP recv nmi interrupt.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_imp,
        .is_fault_report = true,
    },

    {
        {
            .e_excepid = HNS_EXCEPID_IMP_HARD_FAULT,
            .e_excepid_end = HNS_EXCEPID_IMP_HARD_FAULT,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MINOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "HARD_FAULT",
            .e_desc = "HNS IMP recv hard fault interrupt.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_imp,
        .is_fault_report = true,
    },

    {
        {
            .e_excepid = HNS_EXCEPID_IMP_MEM_FAULT,
            .e_excepid_end = HNS_EXCEPID_IMP_MEM_FAULT,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MINOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "MEM_FAULT",
            .e_desc = "HNS IMP recv mem managerment fault interrupt.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_imp,
        .is_fault_report = true,
    },

    {
        {
            .e_excepid = HNS_EXCEPID_IMP_BUS_FAULT,
            .e_excepid_end = HNS_EXCEPID_IMP_BUS_FAULT,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MINOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "BUS_FAULT",
            .e_desc = "HNS IMP recv bus fault interrupt.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_imp,
        .is_fault_report = true,
    },

    {
        {
            .e_excepid = HNS_EXCEPID_IMP_USAGE_FAULT,
            .e_excepid_end = HNS_EXCEPID_IMP_USAGE_FAULT,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MINOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "IMP_USAGE_FAULT",
            .e_desc = "HNS IMP recv usage fault interrupt.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_imp,
        .is_fault_report = true,
    },

    {
        {
            .e_excepid = HNS_EXCEPID_IMP_PENDSV,
            .e_excepid_end = HNS_EXCEPID_IMP_PENDSV,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MINOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "IMP_PENDSV",
            .e_desc = "HNS IMP recv pendsv fault interrupt.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_imp,
        .is_fault_report = true,
    },

    {
        {
            .e_excepid = HNS_EXCEPID_IMP_DEBUG_HANDLE,
            .e_excepid_end = HNS_EXCEPID_IMP_DEBUG_HANDLE,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MINOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "DEBUG_HANDLE",
            .e_desc = "HNS IMP recv debug handle interrupt.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_imp,
        .is_fault_report = true,
    },

    {
        {
            .e_excepid = HNS_EXCEPID_IMP_UNHANDLE_FAULT,
            .e_excepid_end = HNS_EXCEPID_IMP_UNHANDLE_FAULT,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MINOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "UNHANDLE_FAULT",
            .e_desc = "HNS IMP recv unhandle fault interrupt.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_imp,
        .is_fault_report = true,
    },

    {
        {
            .e_excepid = HNS_EXCEPID_IMP_PFC_STORM,
            .e_excepid_end = HNS_EXCEPID_IMP_PFC_STORM,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MINOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "UNHANDLE_FAULT",
            .e_desc = "HNS IMP check pfc storm.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_imp,
    },

    {
        {
            .e_excepid = HNS_EXCEPID_IMP_PAUSE_STORM,
            .e_excepid_end = HNS_EXCEPID_IMP_PAUSE_STORM,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MINOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "UNHANDLE_FAULT",
            .e_desc = "HNS IMP check pause storm.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_imp,
    },

    {
        {
            .e_excepid = HNS_EXCEPID_NIC_RAS_ERR,
            .e_excepid_end = HNS_EXCEPID_NIC_RAS_ERR,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "NIC/RoCE RAS",
            .e_desc = "NIC/RoCE RAS error interrupt.",
            .e_callback = hns_bbox_net_cb,
        },
        .handler = hns_bbox_excep_handler_ras,
    },
};

const size_t g_hns_bbox_excep_size = sizeof(g_hns_bbox_excep) / sizeof(struct hns_bbox_excep_ops);

static int hns_bbox_mmap(u32 devid)
{
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();
    devdrv_hardware_info_t hardware_info = {0};
    u8 connect_type;
    u64 paddr;
    u32 len;
    int ret;

    if (!hns_bbox_log_mem_is_valid()) {
        pr_err("hns3: bbox param error!\n");
        return -EINVAL;
    }

    ret = hal_kernel_get_hardware_info(devid, &hardware_info);
    if (ret != 0) {
        pr_err("hns3: bbox get hardware info failed, ret=%d\n", ret);
        return -EINVAL;
    }
    connect_type = hardware_info.base_hw_info.connect_type;

    paddr = (BBOX_ADDR_HIGH_OFFSET(hns_bbox->chip_id, devid, connect_type) << 32UL) | hns_bbox->log_mem.log_addr;
    hns_bbox->va_bbox[devid] = ioremap_wc(paddr, hns_bbox->log_mem.log_len);
    if (hns_bbox->va_bbox[devid] == NULL) {
        pr_err("hns3: bbox ioremap fail, devid=%u\n", devid);
        return -ENOMEM;
    }

    len = hns_bbox->log_mem.log_len > HNS_BBOX_IMP_ADDR_OFFSET ? HNS_BBOX_IMP_ADDR_OFFSET : hns_bbox->log_mem.log_len;
    ret = memset_s(hns_bbox->va_bbox[devid], len, 0x0, len);
    if (ret != 0) {
        iounmap((void __iomem *)hns_bbox->va_bbox[devid]);
        hns_bbox->va_bbox[devid] = NULL;
        pr_err("hns3: bbox memset 0 error, ret=%d\n", ret);
        return ret;
    }

    return 0;
}

static int hns_imp_mmap(u32 devid)
{
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();
    int ret;

    if (!hns_imp_share_mem_is_valid(hns_bbox->hns_imp_share_mem_addr, hns_bbox->hns_imp_share_mem_len)) {
        pr_err("hns3: bbox param error!\n");
        return -EINVAL;
    }

    hns_bbox->va_imp[devid] = ioremap_wc(hns_bbox->hns_imp_share_mem_addr, hns_bbox->hns_imp_share_mem_len);
    if (hns_bbox->va_imp[devid] == NULL) {
        pr_err("hns3: bbox ioremap fail, devid=%u\n", devid);
        return -ENOMEM;
    }

    ret = memset_s(hns_bbox->va_imp[devid], hns_bbox->hns_imp_share_mem_len, 0x0, hns_bbox->hns_imp_share_mem_len);
    if (ret != 0) {
        iounmap((void __iomem *)hns_bbox->va_imp[devid]);
        hns_bbox->va_imp[devid] = NULL;
        pr_err("hns3: bbox memset 0 error, ret=%d\n", ret);
        return ret;
    }

    return 0;
}

static void hns_bbox_dump(const struct bbox_dump_ops_info *info, bbox_dump_done_ops done)
{
    int i;

    if (info->devid >= HNS_BBOX_DEV_NUM) {
        pr_err("hns3: bbox param error! devid:%u invalid, valid range [0, %u)\n", info->devid, HNS_BBOX_DEV_NUM);
        return;
    }

    if (info->coreid != BBOX_NETWORK || info->etype != NETWORK_EXCEPTION) {
        pr_err("hns3: bbox param error! core_id:%d != [%d], etype:%d != [%d]\n",
               info->coreid, BBOX_NETWORK, info->etype, NETWORK_EXCEPTION);
        return;
    }

    for (i = 0; i < g_hns_bbox_excep_size; i++) {
        if (info->excepid == g_hns_bbox_excep[i].info.e_excepid) {
            g_hns_bbox_excep[i].handler(info);
            done((const struct bbox_dump_done_ops_info *)info);
            return;
        }
    }

    pr_err("hns3: bbox unknown exception! id:0x%x\n", info->excepid);
    return;
}

static void hns_bbox_reset(const struct bbox_reset_ops_info *info)
{
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();

    atomic_dec(&hns_bbox->report_cnt);
}

const struct bbox_module_info g_hns_bbox_module_info = {
    .coreid = BBOX_NETWORK,
    .ops_dump = hns_bbox_dump,
    .ops_reset = hns_bbox_reset,
};

static void hns_bbox_unmap(void)
{
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();
    u32 i;

    for (i = 0; i < HNS_BBOX_DEV_NUM; i++) {
        if (hns_bbox->va_bbox[i] != NULL) {
            iounmap((void __iomem *)hns_bbox->va_bbox[i]);
            hns_bbox->va_bbox[i] = NULL;
        }

        if (hns_bbox->va_imp[i] != NULL) {
            iounmap((void __iomem *)hns_bbox->va_imp[i]);
            hns_bbox->va_imp[i] = NULL;
        }
    }
}

static void hns_bbox_unreg_excepts(u32 size)
{
    u32 i;
    int ret;

    for (i = 0; i < size; i++) {
        pr_info("hns3: bbox unregister exception! excepid:0x%x\n", g_hns_bbox_excep[i].info.e_excepid);
        ret = bbox_unregister_exception(g_hns_bbox_excep[i].info.e_excepid);
        if (ret < 0) {
            pr_err("hns3: bbox unregister exception 0x%x fail! ret:%d\n", g_hns_bbox_excep[i].info.e_excepid, ret);
        }
    }
}

static int hns_bbox_reg_excepts(void)
{
    u32 i;
    u32 ret;
    for (i = 0; i < g_hns_bbox_excep_size; i++) {
        ret = bbox_register_exception(&g_hns_bbox_excep[i].info);
        if (ret == 0) {
            pr_err("hns3: bbox register exception fail! index:%u\n", i);
            hns_bbox_unreg_excepts(i);
            return -EIO;
        }
    }

    return 0;
}

int hns_bbox_init(u32 chip_id)
{
    int ret;
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();

    if (chip_id >= HNS_BBOX_CHIP_NUM) {
        pr_err("hns3: param error! chip_id:%u, expect [0, %d).\n", chip_id, HNS_BBOX_CHIP_NUM);
        return -EINVAL;
    }
    hns_bbox->chip_id = chip_id;

    ret = bbox_register_module(&g_hns_bbox_module_info, &hns_bbox->log_mem);
    if (ret != 0) {
        pr_err("hns3: bbox register module fail! ret:%d\n", ret);
        return ret;
    }

    if (!hns_bbox_log_mem_is_valid()) {
        pr_err("hns3: bbox register module fail! log mem info is invalid\n");
        goto out;
    }

    ret = hns_bbox_reg_excepts();
    if (ret != 0) {
        pr_err("hns3: bbox reg exceptions fail! ret:%d\n", ret);
        goto out;
    }

    atomic_set(&hns_bbox->report_flag, 1);
    atomic_set(&hns_bbox->report_cnt, 0);
    pr_info("hns3: hns_bbox_init success.\n");
    return 0;

out:
    ret = bbox_unregister_module(BBOX_NETWORK);
    if (ret != 0) {
        pr_err("hns3: bbox unregister module fail! ret:%d\n", ret);
    }
    return -EIO;
}

void hns_bbox_uninit(void)
{
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();
    int ret;
    int timeout_cnt;

    pr_info("hns3: bbox uninit!\n");

    atomic_set(&hns_bbox->report_flag, 0);
    ret = atomic_read(&hns_bbox->report_cnt);
    timeout_cnt = ret;
    while (ret != 0 && timeout_cnt != 0) {
        pr_warn("hns3: bbox report_cnt:%d and timeout_cnt:%d is not 0.\n", ret, timeout_cnt);
        ret = atomic_read(&hns_bbox->report_cnt);
        msleep(HNS_BBOX_MSLEEP_TIME);
        timeout_cnt--;
    }

    hns_bbox_unmap();

    ret = bbox_unregister_module(BBOX_NETWORK);
    if (ret != 0) {
        pr_err("hns3: bbox unregister module fail, ret:%d\n", ret);
        return;
    }

    hns_bbox_unreg_excepts(g_hns_bbox_excep_size);

    return;
}

static int hns_bbox_report(const struct bbox_report_info *info)
{
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();
    int ret = 0;
    u32 i;

    if (atomic_read(&hns_bbox->report_flag) == 0) {
        pr_warn("hns3: report_flag is 0, not allowed to report to bbox.\n");
        return -EIO;
    }

    for (i = 0; i < g_hns_bbox_excep_size; i++) {
        if (info->excepid == g_hns_bbox_excep[i].info.e_excepid) {
            if (g_hns_bbox_excep[i].dump_log_buf != NULL) {
                ret = g_hns_bbox_excep[i].dump_log_buf(info->devid, info->excepid, g_hns_bbox_excep[i].log_buf);
            }

            if (ret != 0) {
                pr_err("hns3: bbox report exception id:0x%x fail, ret=%d.\n", info->excepid, ret);
                return -EIO;
            }

            bbox_exception_report(info);
            atomic_inc(&hns_bbox->report_cnt);
            if (g_hns_bbox_excep[i].is_fault_report) {
                hclge_fault_report_excep(hns_bbox->chip_id, info->excepid);
            }
            return 0;
        }
    }

    pr_err("hns3: bbox unknown exception! id:0x%x\n", info->excepid);
    return -EINVAL;
}

void hns_bbox_excep_report(u32 devid, u32 except_id)
{
    excep_time timestamp = {HNS_BBOX_EXCEP_TIME_INIT, HNS_BBOX_EXCEP_TIME_INIT};
    struct bbox_report_info info = {0};
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();

    if (devid >= HNS_BBOX_DEV_NUM) {
        pr_err("hns3: bbox exception report param error! devid:%u, expect [0, %d).\n", devid, HNS_BBOX_DEV_NUM);
        return;
    }

    if (hns_bbox->va_bbox[devid] == NULL) {
        if (hns_bbox_mmap(devid) != 0) {
            pr_err("hns3: hns_bbox_mmap error! devid:%u\n", devid);
            return;
        }
    }

    ktime_get_real_ts64((struct timespec64 *)&timestamp);
    timestamp.tv_usec /= HNS_BBOX_EXCEP_TIME_CUTOFF;
    info.devid = devid;
    info.excepid = except_id;
    info.time = timestamp;
    hns_bbox_report(&info);
}
EXPORT_SYMBOL(hns_bbox_excep_report);

void hns_imp_bbox_report(u32 devid)
{
    struct hns_imp_share_mem *mem = NULL;
    struct bbox_report_info info = {0};
    int ret;

    if (devid >= HNS_BBOX_DEV_NUM) {
        pr_err("hns3: bbox exception report param error! devid:%u, expect [0, %d).\n", devid, HNS_BBOX_DEV_NUM);
        return;
    }

    if (hns_bbox_devid2va_imp(devid) == NULL) {
        if (hns_imp_mmap(devid) != 0) {
            pr_err("hns3: hns_imp_mmap error! devid:%u.\n", devid);
            return;
        }
    }

    mem = (struct hns_imp_share_mem *)hns_bbox_devid2va_imp(devid);
    if (mem == NULL) {
        pr_err("hns3: bbox exception report fail, mem is null.\n");
        return;
    }

    // not need to report, skip
    if (mem->pi == mem->ci) {
        return;
    }

    if (mem->magic != HNS_IMP_SHARE_BUF_MAGIC) {
        pr_err("hns3: bbox exception report fail, magic:0x%x invalid.\n", mem->magic);
        return;
    }

    if (mem->ci >= HNS_BBOX_IMP_MSG_MAX) {
        pr_err("hns3: share mem full, pi:%u, ci:%u >= %u.\n", mem->pi, mem->ci, HNS_BBOX_IMP_MSG_MAX);
        return;
    }

    info.devid = devid;
    info.excepid = mem->msg[mem->ci].except_id;
    info.time.tv_sec = mem->msg[mem->ci].sec;
    info.time.tv_usec = mem->msg[mem->ci].usec;

    ret = hns_bbox_report(&info);
    if (ret != 0) {
        pr_err("hns3: hns_bbox_report fail, ret:%d.\n", ret);
    }

    mem->ci = (mem->ci + 1) % HNS_BBOX_IMP_MSG_MAX;

    return;
}

void hns_bbox_ras_excep_report(u32 devid, const char *format, ...)
{
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();
    struct hns_bbox_info *bbox_info = NULL;
    struct hns_bbox_ras_info *ras_info = NULL;
    char *log_buf = NULL;
    excep_time timestamp = {HNS_BBOX_EXCEP_TIME_INIT, HNS_BBOX_EXCEP_TIME_INIT};
    struct bbox_report_info info = {0};
    va_list args;
    int ret;

    if (devid >= HNS_BBOX_DEV_NUM) {
        pr_err("hns3: bbox exception report param error! devid:%u, expect [0, %d).\n", devid, HNS_BBOX_DEV_NUM);
        return;
    }

    if (atomic_read(&hns_bbox->report_flag) == 0) {
        pr_warn("hns3: report_flag is 0, not allowed to report to bbox.\n");
        return;
    }

    if (hns_bbox->va_bbox[devid] == NULL) {
        if (hns_bbox_mmap(devid) != 0) {
            pr_err("hns3: hns_bbox_mmap error! devid:%u\n", devid);
            return;
        }
    }

    bbox_info = (struct hns_bbox_info *)hns_bbox_devid2va_bbox(devid);
    if (bbox_info == NULL) {
        pr_err("hns3: bbox dump excep_id:0x%x fail, bbox_info is null\n", HNS_EXCEPID_NIC_RAS_ERR);
        return;
    }

    ras_info = (struct hns_bbox_ras_info *)((char *)bbox_info + HNS_BBOX_RAS_ADDR_OFFSET);
    if ((ras_info->pi + 1) % HNS_BBOX_RAS_MSG_MAX == ras_info->ci) {
        pr_err("hns3: ras share mem full, pi:%u, ci:%u >= %u.\n", ras_info->pi, ras_info->ci, HNS_BBOX_RAS_MSG_MAX);
        return;
    }

    log_buf = (char *)bbox_info + HNS_BBOX_RAS_ADDR_OFFSET + sizeof(*ras_info) + ras_info->pi * HNS_BBOX_RAS_BUF_LEN;
    va_start(args, format);
    ret = vsnprintf_s(log_buf, (size_t)HNS_BBOX_RAS_BUF_LEN, (size_t)(HNS_BBOX_RAS_BUF_LEN - 1), format, args);
    if (ret == -1) {
        va_end(args);
        (void)args;
        return;
    }

    va_end(args);
    (void)args;

    ras_info->pi = (ras_info->pi + 1) % HNS_BBOX_RAS_MSG_MAX;

    ktime_get_real_ts64((struct timespec64 *)&timestamp);
    timestamp.tv_usec /= HNS_BBOX_EXCEP_TIME_CUTOFF;
    info.devid = devid;
    info.excepid = HNS_EXCEPID_NIC_RAS_ERR;
    info.time = timestamp;

    bbox_exception_report(&info);
    atomic_inc(&hns_bbox->report_cnt);
}

static int hns_get_nic_bbox_except_id(u32 sub_id, u32 *except_id)
{
    if (except_id == NULL) {
        pr_err("hns3: get bbox except_id fail, except_id is null\n");
        return -EINVAL;
    }

    switch(sub_id) {
        case HNS_BBOX_NIC_INIT_NOTIFY:
            *except_id = HNS_EXCEPID_NIC_INIT;
            break;
        case HNS_BBOX_NIC_UNINIT_NOTIFY:
            *except_id = HNS_EXCEPID_NIC_UNINIT;
            break;
        case HNS_BBOX_NIC_RAS_E:
            *except_id = HNS_EXCEPID_NIC_RAS_ERR;
            break;
        default:
            pr_warn("hns3: get func_id[%u] bbox except_id fail, sub_id %u not support\n", HNS_BBOX_FUNC_NIC, sub_id);
            return -EOPNOTSUPP;
    }

    return 0;
}

static int hns_get_roce_bbox_except_id(u32 sub_id, u32 *except_id)
{
    if (except_id == NULL) {
        pr_err("hns3: get bbox except_id fail, except_id is null\n");
        return -EINVAL;
    }

    switch(sub_id) {
        case HNS_BBOX_ROCE_INIT_NOTIFY:
            *except_id = HNS_EXCEPID_RoCE_INIT;
            break;
        case HNS_BBOX_ROCE_UNINIT_NOTIFY:
            *except_id = HNS_EXCEPID_RoCE_UNINIT;
            break;
        case HNS_BBOX_ROCE_WQE_COTNENT_E:
            *except_id = HNS_EXCEPID_ABNM_CQ_CAUSE_CQE_ERR;
            break;
        case HNS_BBOX_ROCE_SQ_DRAINED_E:
            *except_id = HNS_EXCEPID_SQ_DRAINED_ERR;
            break;
        case HNS_BBOX_ROCE_REQ_INVALID_OPCODE_E:
            *except_id = HNS_EXCEPID_INV_REQ_LOCAL_WQ_ERR;
            break;
        case HNS_BBOX_ROCE_LOC_WQ_ACCESS_E:
            *except_id = HNS_EXCEPID_LOC_WQ_ACCESS_ERR;
            break;
        case HNS_BBOX_ROCE_SRQ_CATAS_E:
            *except_id = HNS_EXCEPID_SRQ_CATAS_ERR;
            break;
        case HNS_BBOX_ROCE_CQ_OVERFLOW_E:
            *except_id = HNS_EXCEPID_CQ_OVERFLOW_ERR;
            break;
        case HNS_BBOX_ROCE_DB_OVERFLOW_E:
            *except_id = HNS_EXCEPID_DB_OVERFLOW_ERR;
            break;
        default:
            pr_warn("hns3: get func_id[%u] bbox except_id fail, sub_id %u not support\n", HNS_BBOX_FUNC_ROCE, sub_id);
            return -EOPNOTSUPP;
    }

    return 0;
}

int hns_get_bbox_excepid(u32 func_id, u32 sub_id, u32 *except_id)
{
    int ret;

    if (except_id == NULL) {
        pr_err("hns3: get bbox except_id fail, except_id is null\n");
        return -EINVAL;
    }

    switch(func_id) {
        case HNS_BBOX_FUNC_NIC:
            ret = hns_get_nic_bbox_except_id(sub_id, except_id);
            break;
        case HNS_BBOX_FUNC_ROCE:
            ret = hns_get_roce_bbox_except_id(sub_id, except_id);
            break;
        default:
            pr_warn("hns3: get bbox except_id fail, func_id %u not support\n", func_id);
            return -EOPNOTSUPP;
    }

    return ret;
}
EXPORT_SYMBOL(hns_get_bbox_excepid);
