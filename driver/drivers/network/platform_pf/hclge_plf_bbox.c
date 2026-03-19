/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Description: bbox
 * Create: 2023-02-28
 */

#include <linux/io.h>
#include <linux/time64.h>
#include <linux/delay.h>
#include <linux/atomic.h>
#ifdef CONFIG_BOOTDOT
#include <linux/bootdot.h>
#else
#include "ascend_kernel_hal.h"

/*
 * 6.18 bring-up does not provide the legacy bootdot service as a
 * loadable dependency for hclgeplf. Keep blockdot call sites intact,
 * but degrade to no-op helpers so the NIC platform driver can still
 * link and run without that optional service.
 */
#define bootdot_init_blk hclge_plf_bootdot_init_blk
#define bootdot_set_blk hclge_plf_bootdot_set_blk

static int hclge_plf_bootdot_init_blk(u32 block_id, u32 magic, u32 execption_id,
    u32 expect_status)
{
    return 0;
}

static int hclge_plf_bootdot_set_blk(u32 block_id, u32 magic, u32 current_status)
{
    return 0;
}
#endif
#include "securec.h"
#include "hclge_plf_bbox.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

STATIC struct hclge_plf_bbox_cb g_hclge_plf_bbox_cb;
STATIC struct hclge_plf_dot_cb g_hclge_plf_dot_cb = {
    .blockdot_flag = false,
    .blockdot_status = 0,
    .blockdot_dev_cnt = 0,
    .blockdot_mutex = __MUTEX_INITIALIZER(g_hclge_plf_dot_cb.blockdot_mutex),
};

STATIC void hclge_plf_bbox_net_cb(u32 temp1, void *temp2)
{
    pr_info("bbox %s !\n", __func__);
}

STATIC char *hclge_plf_bbox_devid2mem(u32 devid)
{
    struct hclge_plf_bbox_cb *hclge_plf_bbox = &g_hclge_plf_bbox_cb;

    if (devid >= HCLGE_PLF_BBOX_DEV_NUM) {
        pr_err("bbox devid:%u invalid, valid range [0, %u)\n", devid, HCLGE_PLF_BBOX_DEV_NUM);
        return NULL;
    }

    return hclge_plf_bbox->va[devid];
}

STATIC bool hclge_plf_bbox_log_mem_is_valid(void)
{
    struct hclge_plf_bbox_cb *hclge_plf_bbox = &g_hclge_plf_bbox_cb;

    if ((hclge_plf_bbox->log_mem.log_addr & HCLGE_PLF_BBOX_LOG_MEM_ADDR_MASK) != HCLGE_PLF_BBOX_LOG_MEM_ADDR) {
        pr_err("check bbox share mem info failed, addr is invalid\n");
        return false;
    }

    if (hclge_plf_bbox->log_mem.log_len != HCLGE_PLF_BBOX_LOG_RESV_MEM_LEN) {
        pr_err("check bbox share mem info failed, len[%u] is invalid, expect[%u]\n",
               hclge_plf_bbox->log_mem.log_len, HCLGE_PLF_BBOX_LOG_RESV_MEM_LEN);
        return false;
    }

    return true;
}

STATIC void hclge_plf_bbox_excep_handler_nic(const struct bbox_dump_ops_info *info, const char *log_buf)
{
    struct hclge_plf_bbox_info *bbox_info = NULL;
    int ret;

    if (log_buf == NULL) {
        pr_err("bbox dump excep_id:0x%x fail, log_buf is null\n", info->excepid);
        return;
    }

    bbox_info = (struct hclge_plf_bbox_info *)hclge_plf_bbox_devid2mem(info->devid);
    if (bbox_info == NULL) {
        pr_err("bbox dump excep_id:0x%x fail, bbox_info is null\n", info->excepid);
        return;
    }

    bbox_info->head.magic = BBOX_MODULE_MAGIC;
    bbox_info->head.e_block_num = HCLGE_PLF_BBOX_BLOCK_NUM;
    bbox_info->head.block[HCLGE_PLF_BBOX_NIC_BLOCK_IDX].e_clock = info->time;
    bbox_info->head.block[HCLGE_PLF_BBOX_NIC_BLOCK_IDX].e_excepid = info->excepid;
    bbox_info->head.block[HCLGE_PLF_BBOX_NIC_BLOCK_IDX].e_block_offset = sizeof(struct bbox_module_ctrl);
    bbox_info->head.block[HCLGE_PLF_BBOX_NIC_BLOCK_IDX].e_block_len = HCLGE_PLF_BBOX_LOG_LEN_MAX;
    bbox_info->head.block[HCLGE_PLF_BBOX_NIC_BLOCK_IDX].e_info_len = HCLGE_PLF_BBOX_LOG_BUF_LEN;

    ret = snprintf_s(bbox_info->log, sizeof(bbox_info->log), sizeof(bbox_info->log) - 1, log_buf);
    if (ret == -1) {
        pr_err("bbox dump excep_id:0x%x fail, snprintf fail.\n", info->excepid);
    }

    return;
}

STATIC const struct hclge_plf_bbox_excep_ops g_hclge_plf_bbox_excep[] = {
    {
        {
            .e_excepid = HCLGE_PLF_EXCEPID_NIC_UNINIT,
            .e_excepid_end = HCLGE_PLF_EXCEPID_NIC_UNINIT,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "NIC_UNINIT",
            .e_desc = "NIC un-initialize.",
            .e_callback = hclge_plf_bbox_net_cb,
        },
        .handler = hclge_plf_bbox_excep_handler_nic,
        .log_buf = "Uninit: NIC",
    },

    {
        {
            .e_excepid = HCLGE_PLF_EXCEPID_NIC_GLOBAL_RESET,
            .e_excepid_end = HCLGE_PLF_EXCEPID_NIC_GLOBAL_RESET,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "NIC_GLOBAL_RST",
            .e_desc = "NIC global reset.",
            .e_callback = hclge_plf_bbox_net_cb,
        },
        .handler = hclge_plf_bbox_excep_handler_nic,
        .log_buf = "NIC global reset",
    },

    {
        {
            .e_excepid = HCLGE_PLF_EXCEPID_NIC_PORT_RESET,
            .e_excepid_end = HCLGE_PLF_EXCEPID_NIC_PORT_RESET,
            .e_notify_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_reset_core_mask = BBOX_COREID_MASK(BBOX_NETWORK),
            .e_exce_type = NETWORK_EXCEPTION,
            .e_from_core = BBOX_NETWORK,
            .e_process_priority = BBOX_MAJOR,
            .e_reboot_priority = BBOX_REBOOT_NO,
            .e_reentrant = BBOX_REENTRANT_ALLOW,
            .e_from_module = "NIC_PORT_RST",
            .e_desc = "NIC port reset.",
            .e_callback = hclge_plf_bbox_net_cb,
        },
        .handler = hclge_plf_bbox_excep_handler_nic,
        .log_buf = "NIC port reset",
    },
};

STATIC const size_t g_hclge_plf_bbox_excep_size = sizeof(g_hclge_plf_bbox_excep) /
                                                  sizeof(struct hclge_plf_bbox_excep_ops);

STATIC int hclge_plf_bbox_mmap(u32 devid)
{
    struct hclge_plf_bbox_cb *hclge_plf_bbox = &g_hclge_plf_bbox_cb;
    u64 paddr;

    if (devid >= HCLGE_PLF_BBOX_DEV_NUM) {
        pr_err("[hclge_plf]: invalid bbox_mmap devid(%u).\n", devid);
        return -EINVAL;
    }

    if (!hclge_plf_bbox_log_mem_is_valid()) {
        pr_err("bbox param error!\n");
        return -EINVAL;
    }

    paddr = hclge_plf_bbox->log_mem.log_addr + devid * HCLGE_PLF_BBOX_DDR_SIZE_PER_DEV;
#ifndef DEFINE_HNS_LLT
    hclge_plf_bbox->va[devid] = ioremap_wc(paddr, hclge_plf_bbox->log_mem.log_len);
#endif
    if (hclge_plf_bbox->va[devid] == NULL) {
        pr_err("bbox ioremap fail, devid=%u\n", devid);
        return -ENOMEM;
    }

    return 0;
}

STATIC void hclge_plf_bbox_unmap(void)
{
    struct hclge_plf_bbox_cb *hclge_plf_bbox = &g_hclge_plf_bbox_cb;
    u32 i;

    for (i = 0; i < HCLGE_PLF_BBOX_DEV_NUM; i++) {
        if (hclge_plf_bbox->va[i] != NULL) {
#ifndef DEFINE_HNS_LLT
            iounmap((void __iomem *)hclge_plf_bbox->va[i]);
#endif
            hclge_plf_bbox->va[i] = NULL;
        }
    }
}

STATIC void hclge_plf_bbox_reset(const struct bbox_reset_ops_info *info)
{
    struct hclge_plf_bbox_cb *hclge_plf_bbox = &g_hclge_plf_bbox_cb;

    atomic_dec(&hclge_plf_bbox->report_cnt);
}

STATIC void hclge_plf_bbox_dump(const struct bbox_dump_ops_info *info, bbox_dump_done_ops done)
{
    u32 i;

    if (info->devid >= HCLGE_PLF_BBOX_DEV_NUM) {
        pr_err("bbox param error! devid:%u invalid, valid range [0, %u)\n", info->devid, HCLGE_PLF_BBOX_DEV_NUM);
        return;
    }

    if (info->coreid != BBOX_NETWORK || info->etype != NETWORK_EXCEPTION) {
        pr_err("bbox param error! core_id:%d != [%d], etype:%d != [%d]\n",
               info->coreid, BBOX_NETWORK, info->etype, NETWORK_EXCEPTION);
        return;
    }

    for (i = 0; i < g_hclge_plf_bbox_excep_size; i++) {
        if (info->excepid == g_hclge_plf_bbox_excep[i].info.e_excepid) {
            g_hclge_plf_bbox_excep[i].handler(info, g_hclge_plf_bbox_excep[i].log_buf);
            done((const struct bbox_dump_done_ops_info *)info);
            return;
        }
    }

    pr_err("bbox unknown exception! id:0x%x\n", info->excepid);
    return;
}

STATIC const struct bbox_module_info g_hclge_plf_bbox_module_info = {
    .coreid = BBOX_NETWORK,
    .ops_dump = hclge_plf_bbox_dump,
    .ops_reset = hclge_plf_bbox_reset,
};

STATIC void hclge_plf_bbox_unreg_excepts(u32 size)
{
    int ret;
    u32 i;

    for (i = 0; i < size; i++) {
        ret = bbox_unregister_exception(g_hclge_plf_bbox_excep[i].info.e_excepid);
        if (ret < 0) {
            pr_err("bbox unregister exception 0x%x fail! ret:%d\n", g_hclge_plf_bbox_excep[i].info.e_excepid, ret);
        }
    }
}

STATIC int hclge_plf_bbox_reg_excepts(void)
{
    u32 ret;
    u32 i;

    for (i = 0; i < g_hclge_plf_bbox_excep_size; i++) {
        ret = bbox_register_exception(&g_hclge_plf_bbox_excep[i].info);
        if (ret == 0) {
            pr_err("bbox register exception fail! index:%u\n", i);
            hclge_plf_bbox_unreg_excepts(i);
            return -EIO;
        }
    }

    return 0;
}

int hclge_plf_bbox_init(void)
{
    struct hclge_plf_bbox_cb *hclge_plf_bbox = &g_hclge_plf_bbox_cb;
    int ret;

    ret = bbox_register_module(&g_hclge_plf_bbox_module_info, &hclge_plf_bbox->log_mem);
    if (ret != 0) {
        pr_err("bbox register module fail! ret:%d\n", ret);
        return ret;
    }

    if (!hclge_plf_bbox_log_mem_is_valid()) {
        pr_err("bbox register module fail! log mem info is invalid\n");
        goto out;
    }

    ret = hclge_plf_bbox_reg_excepts();
    if (ret != 0) {
        pr_err("bbox reg exceptions fail! ret:%d\n", ret);
        goto out;
    }

    atomic_set(&hclge_plf_bbox->report_flag, 1);
    atomic_set(&hclge_plf_bbox->report_cnt, 0);

    pr_info("hclge_plf_bbox_init success.\n");
    return 0;

out:
    ret = bbox_unregister_module(BBOX_NETWORK);
    if (ret != 0) {
        pr_err("bbox unregister module fail! ret:%d\n", ret);
    }
    return -EIO;
}

void hclge_plf_bbox_uninit(void)
{
    struct hclge_plf_bbox_cb *hclge_plf_bbox = &g_hclge_plf_bbox_cb;
    int timeout_cnt;
    int ret;

    atomic_set(&hclge_plf_bbox->report_flag, 0);
    ret = atomic_read(&hclge_plf_bbox->report_cnt);
    timeout_cnt = ret;
    while (ret != 0 && timeout_cnt != 0) {
        pr_warn("bbox report_cnt:%d and timeout_cnt:%d is not 0.\n", ret, timeout_cnt);
        ret = atomic_read(&hclge_plf_bbox->report_cnt);
        msleep(HCLGE_PLF_BBOX_MSLEEP_TIME);
        timeout_cnt--;
    }

    hclge_plf_bbox_unmap();

    ret = bbox_unregister_module(BBOX_NETWORK);
    if (ret != 0) {
        pr_err("bbox unregister module fail, ret:%d\n", ret);
        return;
    }

    hclge_plf_bbox_unreg_excepts(g_hclge_plf_bbox_excep_size);

    pr_info("bbox uninit success!\n");
    return;
}

STATIC int hclge_plf_bbox_report(const struct bbox_report_info *info)
{
    struct hclge_plf_bbox_cb *hclge_plf_bbox = &g_hclge_plf_bbox_cb;
    int ret;
    u32 i;

    if (atomic_read(&hclge_plf_bbox->report_flag) == 0) {
        pr_warn("report_flag is 0, not allowed to report to bbox.\n");
        return -EIO;
    }

    for (i = 0; i < g_hclge_plf_bbox_excep_size; i++) {
        if (info->excepid == g_hclge_plf_bbox_excep[i].info.e_excepid) {
            ret = bbox_exception_report(info);
            if (ret == 1) {
                pr_warn("bbox report exception id:0x%x, exception non-reentrant, dropped.\n", info->excepid);
                return ret;
            } else if (ret == 0) {
                atomic_inc(&hclge_plf_bbox->report_cnt);
                return ret;
            } else if (ret < 0) {
                pr_err("bbox report exception id:0x%x fail, ret = %d.\n", info->excepid, ret);
                return ret;
            } else {
                pr_err("bbox report exception id:0x%x unknown return value, ret = %d.\n", info->excepid, ret);
                return ret;
            }
        }
    }
    pr_err("bbox unknown exception! id:0x%x\n", info->excepid);
    return -EINVAL;
}

void hclge_plf_bbox_excep_report(u32 devid, u32 except_id)
{
    excep_time timestamp = { HCLGE_PLF_BBOX_EXCEP_TIME_INIT, HCLGE_PLF_BBOX_EXCEP_TIME_INIT };
    struct timespec64 times_temp;
    struct hclge_plf_bbox_cb *hclge_plf_bbox = &g_hclge_plf_bbox_cb;
    struct bbox_report_info info = {0};

    if (devid >= HCLGE_PLF_BBOX_DEV_NUM) {
        pr_err("bbox exception report param error! devid:%u, expect [0, %d).\n", devid, HCLGE_PLF_BBOX_DEV_NUM);
        return;
    }

    if (hclge_plf_bbox->va[devid] == NULL) {
        if (hclge_plf_bbox_mmap(devid) != 0) {
            pr_err("hclge_plf_bbox_mmap error! devid:%u\n", devid);
            return;
        }
    }

    ktime_get_real_ts64(&times_temp);
    timestamp.tv_sec = times_temp.tv_sec;
    timestamp.tv_usec = times_temp.tv_nsec / NSEC_PER_USEC;

    info.devid = devid;
    info.excepid = except_id;
    info.time = timestamp;
    hclge_plf_bbox_report(&info);
}

STATIC bool hclge_plf_bbox_get_blockdot_flag(void)
{
    return g_hclge_plf_dot_cb.blockdot_flag;
}

STATIC void hclge_plf_bbox_set_blockdot_flag(bool flag)
{
    // only the last one can disable
    if (flag == HCLGE_PLF_BLOCKDOT_FLAG_DISABLE && g_hclge_plf_dot_cb.blockdot_dev_cnt != 0) {
        return;
    }

    g_hclge_plf_dot_cb.blockdot_flag = flag;
}

STATIC u32 hclge_plf_bbox_get_block_id(void)
{
    return HCLGE_PLF_BLOCKDOT_BLOCK_ID;
}

void hclge_plf_blockdot_begin(u32 hdev_id, u32 id_map, u32 except_id, u32 dot_id)
{
    u32 status = 0;
    u32 block_id;
    u32 port_id;
    int ret;

    if (hdev_id >= HCLGE_PLF_BLOCKDOT_DEV_NUM) {
        pr_warn("blockdot %s param err, hdev_id:%u invalid, expect [0, %u)\n",
                __func__, hdev_id, HCLGE_PLF_BLOCKDOT_DEV_NUM);
        return;
    }

    // bootdot has been inited, skip
    mutex_lock(&g_hclge_plf_dot_cb.blockdot_mutex);
    g_hclge_plf_dot_cb.blockdot_dev_cnt++;
    if (hclge_plf_bbox_get_blockdot_flag()) {
        mutex_unlock(&g_hclge_plf_dot_cb.blockdot_mutex);
        return;
    }

    // only the first one need to init the block, init one block for all ports
    for (port_id = 0; id_map != 0 && port_id < HCLGE_PLF_BLOCKDOT_DEV_NUM; port_id++, id_map >>= 1) {
        if ((id_map & 0x1) == 0x0) {
            continue;
        }

        status |= (HCLGE_PLF_BLOCKDOT_STATUS_OFFSET(dot_id, port_id));
    }

    block_id = hclge_plf_bbox_get_block_id();
    ret = bootdot_init_blk(block_id, HCLGE_PLF_BLOCKDOT_MAGIC_NUM, except_id, status);
    if (ret != 0) {
        pr_err("blockdot bootdot_init_blk failed, block_id=0x%x, except_id=0x%x, status=0x%x, ret=%d\n",
               block_id, except_id, status, ret);
        mutex_unlock(&g_hclge_plf_dot_cb.blockdot_mutex);
        return;
    }

    // blockdot begin, set blockdot_flag enable
    hclge_plf_bbox_set_blockdot_flag(HCLGE_PLF_BLOCKDOT_FLAG_ENABLE);
    mutex_unlock(&g_hclge_plf_dot_cb.blockdot_mutex);
}

void hclge_plf_blockdot_record(u32 hdev_id, u32 dot_id)
{
    u32 block_id;
    u32 status;
    int ret;

    if (hdev_id >= HCLGE_PLF_BLOCKDOT_DEV_NUM) {
        pr_warn("blockdot %s param err, hdev_id:%u invalid, expect [0, %u)\n",
                __func__, hdev_id, HCLGE_PLF_BLOCKDOT_DEV_NUM);
        return;
    }

    // not blockdot scenes, no need to record
    mutex_lock(&g_hclge_plf_dot_cb.blockdot_mutex);
    if (!hclge_plf_bbox_get_blockdot_flag()) {
        mutex_unlock(&g_hclge_plf_dot_cb.blockdot_mutex);
        return;
    }

    // get the last time status, clear and set corresponding port bits
    status = g_hclge_plf_dot_cb.blockdot_status;
    status &= HCLGE_PLF_BLOCKDOT_STATUS_CLEAR_DOT(hdev_id);
    status |= HCLGE_PLF_BLOCKDOT_STATUS_OFFSET(dot_id, hdev_id);
    block_id = hclge_plf_bbox_get_block_id();
    ret = bootdot_set_blk(block_id, HCLGE_PLF_BLOCKDOT_MAGIC_NUM, status);
    if (ret != 0) {
        pr_err("blockdot bootdot_set_blk failed, block_id=0x%x, status=0x%x, ret=%d\n",
               block_id, status, ret);
    } else {
        g_hclge_plf_dot_cb.blockdot_status = status;
    }

    mutex_unlock(&g_hclge_plf_dot_cb.blockdot_mutex);
    return;
}

void hclge_plf_blockdot_end(u32 hdev_id, u32 dot_id)
{
    if (hdev_id >= HCLGE_PLF_BLOCKDOT_DEV_NUM) {
        pr_warn("blockdot %s param err, hdev_id:%u invalid, expect [0, %u)\n",
                __func__, hdev_id, HCLGE_PLF_BLOCKDOT_DEV_NUM);
        return;
    }

    hclge_plf_blockdot_record(hdev_id, dot_id);

    mutex_lock(&g_hclge_plf_dot_cb.blockdot_mutex);
    g_hclge_plf_dot_cb.blockdot_dev_cnt--;
    hclge_plf_bbox_set_blockdot_flag(HCLGE_PLF_BLOCKDOT_FLAG_DISABLE);
    mutex_unlock(&g_hclge_plf_dot_cb.blockdot_mutex);
}
