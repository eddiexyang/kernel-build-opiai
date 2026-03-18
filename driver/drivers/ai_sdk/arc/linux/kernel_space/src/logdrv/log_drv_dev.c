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
 * Create: 2022-08-13
 */
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/types.h>
#include <linux/crypto.h>
#include <linux/slab.h>
#include <linux/acpi.h>
#include <linux/vmalloc.h>
#include <linux/kallsyms.h>
#include <linux/atomic.h>
#include <linux/version.h>
#include <linux/cpumask.h>
#include <linux/gfp.h>

#include "dbl/chip_config.h"
#include "dbl/uda.h"
#include "soc_res.h"

#include "log_drv_dev.h"
#include "log_drv_res.h"
#include "log_drv_res_common.h"
#include "devdrv_manager.h"
#include "devdrv_platform.h"

#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "drv_cpu_type.h"
#include "log_xpc.h"
#endif

#if (defined CFG_SOC_PLATFORM_MINIV2) || (defined CFG_SOC_PLATFORM_MDC_V51) || (defined CFG_SOC_PLATFORM_MINIV3) || \
    (defined CFG_SOC_PLATFORM_CLOUD) || (defined CFG_SOC_PLATFORM_CLOUD_V2)
#include "devdrv_parse_pdata.h"
#endif

STATIC struct log_drv_desc log_desc = {
    .drv_class = NULL,
    .drv_device = NULL,
    .device_num = 0,
    .device_ids = { 0 },
    .cpu_ids = { 0 },
    .ts_num = { 0 },
    .device_ctx = { NULL },
    .task_log_work = { NULL },
    .hdc_epoll_task = NULL,
    .hdc_epolls = { 0 },
    .session_type = 0
};

/* only TS and aicpu channel , need flush cache */
#define LOG_FLUSH_CACHE(channel_info, entire)
void *devdrv_manager_register_msg_pm(s32 (*suspend)(void), s32(*resume)(void));
void devdrv_manager_unregister_msg_pm(void *msg_pm_p);

void log_cq0_report(u32 device_id, u32 tsid, const u8 *cq_buf, const u8 *sq_buf);
void log_cq1_callback(u32 device_id, u32 tsid, const u8 *cq_buf, u8 *sq_buf);

STATIC void log_dev_session_init(s32 dev_id);

STATIC s32 log_hdc_wait_host(struct log_device_ctx *device_ctx);
STATIC s32 log_create_channel(struct log_device_ctx *device_ctx, const struct log_channel_desc *log_chan_desc);
STATIC s32 log_delete_channel(struct log_device_ctx *device_ctx, struct log_channel_info *channel_info);
STATIC s32 log_user_set_level(struct log_device_ctx *device_ctx, struct log_ioctl_para *log_para);
STATIC s32 log_read_channel(const struct log_device_ctx *device_ctx, s32 channel_id, struct log_msg_buf *recv_buf);
STATIC s32 log_read_ringbuffer(const struct log_channel_info *channel_info, s32 buf_read, s32 buf_write,
    struct log_msg_buf *recv_buf);
STATIC s32 log_create_all_channels(struct log_device_ctx *device_ctx, log_create_channels create_channel_type);
STATIC void log_delete_all_channels(struct log_device_ctx *device_ctx);
STATIC s32 log_read_multi_channels(struct log_device_ctx *device_ctx, s32 channel_ids, s32 channel_ids_num);
STATIC s32 log_msg_compress(const struct log_device_ctx *device_ctx, struct log_msg_head *msg_head, s32 msg_data_size,
    struct log_msg_buf *compress_buf);
STATIC s32 log_send_msg_to_host(struct log_device_ctx *device_ctx,
    struct log_msg_buf *send_buf,
    struct log_msg_buf *compress_buf,
    u32 timeout);
STATIC s32 log_work_init(struct log_device_ctx *device_ctx);
STATIC void log_work_uninit(struct log_device_ctx *device_ctx);
STATIC s32 log_hdc_prepare_to_work(struct log_device_ctx *device_ctx);
STATIC s32 log_work_thread(void *arg);
STATIC s32 log_user_get_channel_type(struct log_ioctl_para *log_para);
STATIC s32 log_user_get_device_id(struct log_ioctl_para *log_para);
STATIC ssize_t log_drv_ioctl(struct file *file, u32 cmd, uintptr_t arg);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
STATIC void log_calc_generate_rate(struct timer_list *arg);
#else
STATIC void log_calc_generate_rate(size_t arg);
#endif
STATIC s32 log_msg_buf_init(struct log_msg_buf *msg_buf, s32 msg_type, s32 buf_size, s32 threshold);
STATIC struct log_device_ctx *log_create_device_ctx(s32 device_id);
STATIC void log_destroy_device_ctx(struct log_device_ctx *device_ctx);
STATIC s32 log_drv_register_cdev(void);
STATIC void log_drv_free_cdev(void);
STATIC s32 log_create_hdc_server(struct log_device_ctx *device_ctx);
STATIC s32 log_drv_wait_ts(const struct log_device_ctx *device_ctx);
STATIC s32 log_drv_is_suspend(void);
#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC void log_stop_ts_threads(void);
STATIC void log_stop_xpc_threads(void);
STATIC void log_launch_ts_threads(void);
STATIC s32 g_log_user_level[LOG_CHANNEL_TYPE_MAX];
STATIC bool g_log_ts_thread_stop = false;
STATIC bool g_log_xpc_thread_stop = false;
#endif

STATIC u64 log_make_alloc_exact(u64 addr, u32 order, size_t size)
{
    if (addr != 0) {
        u64 alloc_end = addr + (u64)((1UL << order) * PAGE_SIZE);
        u64 used = addr + PAGE_ALIGN(size);

        split_page(virt_to_page((void *)(uintptr_t)addr), order);
        while (used < alloc_end) {
            free_page(used);
            used += PAGE_SIZE;
        }
    }
    return addr;
}

STATIC u64 log_alloc_pages_exact(s32 nid, size_t size, gfp_t gfp_mask)
{
    const struct page *alloc_page = NULL;
    u32 order = (u32)get_order(size);
    u64 addr;

    /* try to alloc pages by order, just so we can get continuous phy-pages */
    alloc_page = alloc_pages_node(nid, gfp_mask, order);
    if (alloc_page == NULL) {
        return 0;
    }

    addr = log_make_alloc_exact((u64)(uintptr_t)page_address(alloc_page), order, size);
    if (addr != 0) {
        if (memset_s((void *)(uintptr_t)addr, size, 0, size) != 0) {
#ifndef LOG_UT
            slog_drv_warn("Buf clear fail.(addr=%pK)\n", (void *)(uintptr_t)addr);
#endif
        }
    }

    return addr;
}

STATIC void log_free_pages_exact(u64 virt, size_t size)
{
    u64 addr = (u64)virt;
    u64 end = addr + PAGE_ALIGN(size);

    while (addr < end) {
        free_page(addr);
        addr += PAGE_SIZE;
    }
}

STATIC int log_copy_from_user_safe(void *to, void *from, unsigned long n)
{
    if ((from == NULL) || (to == NULL)) {
        slog_drv_err("User pointer is NULL.\n");
        return -EINVAL;
    }

    if (n == 0) {
        slog_drv_err("Arg n less than or equal to zero.\n");
        return -EINVAL;
    }

    if (copy_from_user(to, (void __user *)from, n) != 0)
        return -ENODEV;

    return 0;
}

STATIC int log_copy_to_user_safe(void *to, void *from, unsigned long n)
{
    if ((from == NULL) || (to == NULL)) {
        slog_drv_err("User pointer is NULL.\n");
        return -EINVAL;
    }

    if (n == 0) {
        slog_drv_err("Arg n less than or equal to zero.\n");
        return -EINVAL;
    }

    if (copy_to_user((void __user *)to, from, n) != 0)
        return -ENODEV;

    return 0;
}

void log_cq0_report(u32 device_id, u32 tsid, const u8 *cq_buf, const u8 *sq_buf)
{
    struct log_channel_info *channel_info = NULL;
    struct log_device_info *device_info = NULL;
    struct log_cq_scheduler *cq_report = NULL;
    struct log_device_ctx *device_ctx = NULL;
    struct log_cqsq_info *cqsq_info = NULL;
    s32 set_level_channel_type = 0;
    s32 channel_id;

    if (cq_buf == NULL) {
        slog_drv_err("Cq_buf is NULL. (device_id=%u)\n", device_id);
        return;
    }

    cq_report = (struct log_cq_scheduler *)cq_buf;

    LOG_DEBUG_CQ_REPORT_SHOW(cq_report);
    if (device_id >= (u32)(log_desc.device_num)) {
        slog_drv_err("Device_id is invalid. (device_id=%u)\n", device_id);
        return;
    }

    if (tsid > LOG_MDC_TS_ID) {
        slog_drv_err("Tsid is invalid. (device_id=%u; cq_channel_id=%u; tsid=%u)\n",
            device_id, cq_report->channel_id, tsid);
        return;
    }

    channel_id = log_channel_id_shift(device_id, tsid, (s32)cq_report->channel_id, &set_level_channel_type);
    if (log_is_channel_valid(channel_id) != 0) {
        slog_drv_err("Channel_id is invalid. (device_id=%u; channel_id=%d; cq_channel_id=%u)\n",
            device_id, channel_id, cq_report->channel_id);
        return;
    }

    device_ctx = log_desc.device_ctx[device_id];
    if (device_ctx == NULL) {
        slog_drv_err("Device_ctx is NULL. (device_id=%u)\n", device_id);
        return;
    }

    device_info = &device_ctx->device_info;
    cqsq_info = &device_info->cqsq_info[tsid];
    mutex_lock(&cqsq_info->cq0_mutex);

    if (device_info->cmd_verify != cq_report->cmd_verify) {
        slog_drv_err("Cmd_verify is invalid. (device_id=%u; cmd_verify=%u; cq_cmd_verify=%u)\n",
                     device_id, device_info->cmd_verify, cq_report->cmd_verify);

        mutex_unlock(&cqsq_info->cq0_mutex);
        return;
    }

    channel_info = &device_ctx->channels_info[channel_id];
    channel_info->ret_val = cq_report->ret_val;
    device_info->cmd_verify++;
    mutex_unlock(&cqsq_info->cq0_mutex);

    up(&(cqsq_info->cq0_wait_sema[set_level_channel_type]));
    LOG_DEBUG_CQ_REPORT_SHOW(cq_report);
}

void log_cq1_callback(u32 device_id, u32 tsid, const u8 *cq_buf, u8 *sq_buf)
{
    const struct log_channel_info *channel_info = NULL;
    const struct log_cq_scheduler *cq_report = NULL;
    struct log_device_ctx *device_ctx = NULL;
    s32 set_level_channel_type;
    s32 channel_id;
    s32 ret;
    u32 deviceid;
    u32 log_tsid;

    if (cq_buf == NULL) {
        slog_drv_err("Cq_buf is NULL. (device_id=%u)\n", device_id);
        return;
    }

    slog_drv_debug("Show details. (devide_id=%u; tsid=%u)\n", device_id, tsid);

    cq_report = (struct log_cq_scheduler *)cq_buf;
    deviceid = cq_report->device_id;
    channel_id = cq_report->channel_id;
    log_tsid = cq_report->tsid;

    if (deviceid >= (u32)(log_desc.device_num)) {
        slog_drv_err("Device_id is invalid. (device_id=%u)\n", device_id);
        return;
    }

    if (log_tsid > LOG_MDC_TS_ID) {
        slog_drv_err("Log_tsid is invalid. (device_id=%u; channel_id=%d; log_tsid=%u)\n",
            device_id, channel_id, log_tsid);
        return;
    }

    channel_id = log_channel_id_shift(deviceid, log_tsid, channel_id, &set_level_channel_type);
    if (log_is_channel_valid(channel_id) != (s32)LOG_RET_OK) {
        slog_drv_err("Channel_id is invalid. (device_id=%u; channel_id=%d)\n",
            device_id, channel_id);
        return;
    }

    device_ctx = log_desc.device_ctx[deviceid];
    if (device_ctx == NULL) {
        slog_drv_err("Device_ctx is NULL. (device_id=%u)\n", device_id);
        return;
    }

    // this function invoked by devdrv thread, so check device status before used.
    if (atomic_read(&device_ctx->status) != LOG_DEVICE_STATUS_WORK) {
        return;
    }

    channel_info = &device_ctx->channels_info[channel_id];

    if (unlikely(channel_info->channel_state != LOG_CHANNEL_ENABLE)) {
        slog_drv_err("Channel is not created. (device_id=%u; channel_id=%d)\n", device_id, channel_id);
        return;
    }

    mutex_lock(&device_ctx->channel_buf_mutex);
    ret = log_read_channel(device_ctx, channel_id, &device_ctx->channel_log_buf);
    if (ret == (s32)LOG_RET_EOF) {
        slog_drv_err("Cq1 callback read log failed. (device_id=%u; channel_id=%d; ret=LOG_RET_EOF)\n",
                     device_id, channel_id);
    }

    if (device_ctx->channel_log_buf.data_size > device_ctx->channel_log_buf.data_threshold) {
        ret = log_send_msg_to_host(device_ctx,
                                   &device_ctx->channel_log_buf,
                                   &device_ctx->channel_compress_buf,
                                   LOG_HDCDRV_SEND_TIMEOUT);
        if (ret != (s32)LOG_RET_OK) {
            slog_drv_err("Channel log send failed. "
                "(device_id=%u; channel_id=%d; data_size=%d; data_threshold=%d; ret=%d)\n",
                device_id, channel_id, device_ctx->channel_log_buf.data_size,
                device_ctx->channel_log_buf.data_threshold, ret);
        }
    }
    mutex_unlock(&device_ctx->channel_buf_mutex);

    LOG_DEBUG_CQ_REPORT_SHOW(cq_report);
    return;
}

STATIC s32 log_remap_noncache(struct log_channel_info *channel_info, s32 buf_size)
{
    struct vm_struct *area = NULL;
    struct page **pages = NULL;
    struct page *page = NULL;
    u32 i;
    u32 page_num;
    s32 ret;

    if (channel_info->channel_conn != (s32)LOG_CHANNEL_CONN_SQCQ) {
        return LOG_RET_OK;
    }
    page_num = (u32)(buf_size / (s32)PAGE_SIZE);
    page_num += (u32)((bool)(buf_size % (s32)PAGE_SIZE) ? 1 : 0);
    if (page_num > LOG_MAX_PAGE_NUM_OF_KZALLOC) {
        slog_drv_err("Buf_size too large. (page_num=%d; buf_size=%d)\n", page_num, buf_size);
        return LOG_RET_ERROR;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0)
    area = __get_vm_area((size_t)(u32)buf_size, VM_ALLOC, VMALLOC_START, VMALLOC_END);
#else
    area = __get_vm_area_caller((size_t)(u32)buf_size, VM_ALLOC,
        VMALLOC_START, VMALLOC_END, __builtin_return_address(0));
#endif
    if (area == NULL) {
        slog_drv_err("Cannot find vm struct address. (buf_size=%d)\n", buf_size);
        return LOG_RET_ERROR;
    }
    page = phys_to_page(channel_info->phy_addr);

    pages = kzalloc(sizeof(struct page *) * page_num, GFP_KERNEL);
    if (pages == NULL) {
        slog_drv_err("Malloc pages failed. (buf_size=%d; page_num=%u)\n", buf_size, page_num);
        free_vm_area(area);
        return LOG_RET_ERROR;
    }
    for (i = 0; i < page_num; i++) {
        pages[i] = nth_page(page, i);
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    ret = map_kernel_range((unsigned long)(uintptr_t)area->addr, get_vm_area_size(area),
        __pgprot(PROT_NORMAL_NC), pages);
    if (ret < 0) {
#else
    ret = map_vm_area(area, __pgprot(PROT_NORMAL_NC), pages);
    if (ret != (s32)LOG_RET_OK) {
#endif
        kfree(pages);
        pages = NULL;
        free_vm_area(area);
        slog_drv_err("Cannot map area. (buf_size=%d; ret=%d)\n", buf_size, ret);
        return LOG_RET_ERROR;
    }
    kfree(pages);
    pages = NULL;
    slog_drv_info("Remap succeeded. (area_size=%lu)\n", area->size);

    channel_info->area = area;
    channel_info->vir_addr_kmalloc = channel_info->vir_addr;
    channel_info->vir_addr = area->addr;

    return LOG_RET_OK;
}

STATIC void log_unmap_noncache(struct log_channel_info *channel_info)
{
    if (channel_info->channel_conn != LOG_CHANNEL_CONN_SQCQ) {
        return;
    }
    if (channel_info->area != NULL) {
        free_vm_area(channel_info->area);
        channel_info->area = NULL;
    }
    if (channel_info->vir_addr_kmalloc != NULL) {
        log_free_pages_exact((u64)(uintptr_t)channel_info->vir_addr_kmalloc, channel_info->buf_size);
        channel_info->vir_addr_kmalloc = NULL;
    }
    channel_info->vir_addr = NULL;
}

STATIC  void log_init_channel_common_info(struct log_channel_info *channel_info,
    const struct log_channel_desc *log_chan_desc)
{
    channel_info->log_desc = log_chan_desc;
    channel_info->channel_type = log_chan_desc->channel_type;
    channel_info->channel_conn = log_chan_desc->channel_conn;
    channel_info->log_type = log_chan_desc->log_type;
    channel_info->buf_size = log_chan_desc->buf_size;
    channel_info->log_level = log_chan_desc->log_level;
    channel_info->channel_id = log_chan_desc->channel_ids;
}

STATIC bool log_is_tscpu_channel(const struct log_channel_info *channel_info)
{
    if ((channel_info->channel_type == (s32)LOG_CHANNEL_TYPE_TS) ||
        (channel_info->channel_type == (s32)LOG_CHANNEL_TYPE_TS_DUMP)) {
        return true;
    }
    return false;
}

STATIC u32 log_get_chip_id(void)
{
    u32 chip_id = 0;
#ifdef CFG_SOC_PLATFORM_CLOUD_v2
    void __iomem *chip_id_addr = NULL;

    chip_id_addr = ioremap_cache(0x36EFD800, sizeof(u32));  /* 0x36EFD800 chip id addr */
    if (chip_id_addr == NULL) {
        return 0;
    }
    chip_id = readl(chip_id_addr) & 0xFF;

    iounmap(chip_id_addr);
    chip_id_addr = NULL;

    if (chip_id >= 8) { /* chip id < 8 */
        return 0;
    }
#endif
    return chip_id;
}

static bool log_is_pcie_connet(void)
{
    bool is_pcie_connet = true;
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    struct devdrv_hw_info {
        unsigned char chip_id;
        unsigned char multi_chip;
        unsigned char multi_die;
        unsigned char mainboard_id;
        unsigned short hccs_connect_status;
        unsigned short board_id;
        unsigned int reserved;
    };
    struct devdrv_hw_info *hw_data = NULL;

    /* 0x36EFD800 chip id addr */
    hw_data = (struct devdrv_hw_info *)ioremap_cache(0x36EFD800, sizeof(struct devdrv_hw_info));
    if (hw_data == NULL) {
        return true;
    }
    is_pcie_connet = (hw_data->hccs_connect_status == CONNECT_PROTOCOL_PCIE) ? true : false;
    iounmap(hw_data);
#endif
    return is_pcie_connet;
}

static u64 log_get_addr_pcie_offset(s32 devid, u64 addr)
{
    u32 chip_id = log_get_chip_id();
    u64 offset = chip_id * LOG_CHIP_OFFSET + (u32)devid * LOG_DEVICE_OFFSET;
    offset += (devid > 0) ? LOG_DIE_HIGH_OFFSET : 0;
    return offset;
}

#ifndef LOG_UT
static u64 log_get_addr_hccs_offset(s32 devid, u64 addr)
{
#define LOG_HCCS_DIE0_ADD_OFFSET_LIMIT     0x80000000ULL    /* For die 0, less this limit should add  */
#define LOG_HCCS_CHIP_DDR_PARTITION        0x20000000000UL
#define LOG_HCCS_BASE_OFFSET               0x200000000000ULL
    u64 hccs_base_offset, hccs_die_offset, hccs_chip_offset;

    hccs_base_offset =  ((devid == 0) && (addr < LOG_HCCS_DIE0_ADD_OFFSET_LIMIT)) ? 0 : LOG_HCCS_BASE_OFFSET;
    hccs_die_offset = devid * (LOG_DEVICE_OFFSET  + LOG_DIE_HIGH_OFFSET);
    hccs_chip_offset = ((devid == 0) && (addr < LOG_HCCS_DIE0_ADD_OFFSET_LIMIT)) ?
        0 : log_get_chip_id() * LOG_HCCS_CHIP_DDR_PARTITION;

    return hccs_base_offset + hccs_die_offset + hccs_chip_offset;
}
#endif

STATIC u64 log_get_addr_offset(s32 devid, u64 addr)
{
    if (log_is_pcie_connet()) {
        return log_get_addr_pcie_offset(devid, addr);
    } else {
#ifndef LOG_UT
        return log_get_addr_hccs_offset(devid, addr);
#else
        return 0;
#endif
    }
}

STATIC s32 log_alloc_channel_memory(const struct log_device_ctx *device_ctx, struct log_channel_info *channel_info)
{
    const struct log_channel_desc *desc = channel_info->log_desc;
    // 1.reserved mem, when it is multi soc, consider chip_offset
    if (desc->buf_phy_addr != 0) {
        channel_info->phy_addr = desc->buf_phy_addr +
            log_get_addr_offset(device_ctx->device_info.device_id, desc->buf_phy_addr);
        channel_info->vir_addr = devm_ioremap_wc(log_desc.drv_device, channel_info->phy_addr, (u64)(u32)desc->buf_size);
        if (channel_info->vir_addr == NULL) {
            slog_drv_err("Remap buffer error.\n");
            return LOG_RET_ERROR;
        }
        slog_drv_debug("Alloc channel memory successfully. (device_id=%d; channel_id=%d; phy_addr=%pK)\n",
            device_ctx->device_info.device_id, channel_info->channel_id, (void *)(uintptr_t)channel_info->phy_addr);
    // 2.dynamic mem
    } else {
        bool is_ts_channel = log_is_tscpu_channel(channel_info);
        gfp_t gfp_mask = GFP_KERNEL;
        s32 nid;

        gfp_mask |= is_ts_channel ? (u32)(__GFP_THISNODE | GFP_HIGHUSER_MOVABLE) : 0;
        nid = is_ts_channel ? dbl_get_ts_default_nid((u32)device_ctx->device_info.device_id) : NUMA_NO_NODE;

        channel_info->vir_addr = (u8 *)(uintptr_t)log_alloc_pages_exact(nid,
            (size_t)(u32)desc->buf_size, gfp_mask);
        if (channel_info->vir_addr == NULL) {
            slog_drv_err("Alloc memory failed. (buf_size=%d)\n", desc->buf_size);
            return LOG_RET_ERROR;
        }

        channel_info->phy_addr = virt_to_phys(channel_info->vir_addr);
        channel_info->buf_size = (u32)desc->buf_size;

        if (log_remap_noncache(channel_info, desc->buf_size) != (s32)LOG_RET_OK) {
            log_free_pages_exact((u64)(uintptr_t)channel_info->vir_addr, channel_info->buf_size);
            channel_info->vir_addr = NULL;
            slog_drv_err("Remap memory failed. (buf_size=%d)\n", desc->buf_size);
            return LOG_RET_ERROR;
        }
    }

    return LOG_RET_OK;
}

STATIC void log_free_channel_memory(struct log_channel_info *channel_info)
{
    const struct log_channel_desc *desc = channel_info->log_desc;

    if (desc->buf_phy_addr != 0) { // 1.reserved mem
        devm_iounmap(log_desc.drv_device, channel_info->vir_addr);
        channel_info->vir_addr = NULL;
    } else { // 2. dynamic mem need to be freed
        log_unmap_noncache(channel_info);
        if (channel_info->vir_addr != NULL) {
            log_free_pages_exact((u64)(uintptr_t)channel_info->vir_addr, channel_info->buf_size);
            channel_info->vir_addr = NULL;
        }
    }
}

STATIC s32 log_create_channel(struct log_device_ctx *device_ctx, const struct log_channel_desc *log_chan_desc)
{
    const struct log_device_info *device_info = NULL;
    struct log_channel_info *channel_info = NULL;
    s32 ret;

    if ((device_ctx == NULL) || (log_chan_desc == NULL)) {
        slog_drv_err("Input parameter error. "
            "(device_ctx_is_null=%d; log_chan_desc_is_null=%d)\n", (device_ctx == NULL), (log_chan_desc == NULL));
        return LOG_RET_ERROR;
    }

    device_info = &device_ctx->device_info;
    if (device_info->device_id >= log_desc.device_num) {
        slog_drv_err("Device_id is invalid. (device_id=%d)\n", device_info->device_id);
        return LOG_RET_ERROR;
    }

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    if ((device_info->device_id == 1) && (log_chan_desc->channel_type == LOG_CHANNEL_TYPE_HSM)) {
        return LOG_RET_ERROR;
    }
#endif

    if ((log_is_channel_valid(log_chan_desc->channel_ids) != 0) ||
        (log_chan_desc->buf_size <= 0)) {
        slog_drv_warn("Channel_id is invalid. "
            "(device_id=%d; channel_ids=%d)\n", device_info->device_id, log_chan_desc->channel_ids);
        return LOG_RET_ERROR;
    }

    channel_info = &device_ctx->channels_info[log_chan_desc->channel_ids];
    mutex_lock(&channel_info->cmd_mutex);
    if (channel_info->channel_state == LOG_CHANNEL_ENABLE) {
        mutex_unlock(&channel_info->cmd_mutex);
        slog_drv_err("Channel_id create repeatedly. "
            "(device_id=%d; channel_ids=%d)\n", device_info->device_id, log_chan_desc->channel_ids);
        return LOG_RET_ERROR;
    }

    // 1.init channel common info
    log_init_channel_common_info(channel_info, log_chan_desc);

    // 2.alloc or remap memory for channel
    ret = log_alloc_channel_memory(device_ctx, channel_info);
    if (ret != (s32)LOG_RET_OK) {
        mutex_unlock(&channel_info->cmd_mutex);
        slog_drv_err("Alloc memory failed. (device_id=%d; ret=%d)\n", device_info->device_id, ret);
        return ret;
    }

    // 3.init channel memory
    if (log_chan_desc->init != NULL) {
        log_chan_desc->init(device_ctx, channel_info);
    }

    /* 4.send log channel create cmd to other sub system */
    if (log_chan_desc->create != NULL) {
        ret = log_chan_desc->create(device_ctx, channel_info);
        if (ret != (s32)LOG_RET_OK) {
            mutex_unlock(&channel_info->cmd_mutex);
            slog_drv_err("Send creat cmd failed. (device_id=%d)\n", device_info->device_id);
            LOG_DEBUG_CHANINFO_SHOW(channel_info);
            log_free_channel_memory(channel_info);
            return ret;
        }
    }

    /* 5.send log channel default log level cmd to other sub system */
    if (log_chan_desc->set_level != NULL) {
#ifdef CFG_SOC_PLATFORM_MDC_V51
        ret = log_chan_desc->set_level(device_ctx, channel_info, g_log_user_level[log_chan_desc->channel_type]);
#else
        ret = log_chan_desc->set_level(device_ctx, channel_info, log_chan_desc->log_level);
#endif
        if (ret != (s32)LOG_RET_OK) {
            mutex_unlock(&channel_info->cmd_mutex);
            slog_drv_err("Log_send_setlevel_cmd_ts failed. (device_id=%d; channel_id=%d; ret=%d)\n",
                device_info->device_id, log_chan_desc->channel_ids, ret);
            log_free_channel_memory(channel_info);
            return ret;
        }
    }

    channel_info->channel_state = LOG_CHANNEL_ENABLE;
    mutex_unlock(&channel_info->cmd_mutex);

    LOG_DEBUG_BUFFHEAD_SHOW((struct log_channel_buf_head *)(channel_info->vir_addr));
    LOG_DEBUG_CHANINFO_SHOW(channel_info);

    return LOG_RET_OK;
}

STATIC s32 log_delete_channel(struct log_device_ctx *device_ctx, struct log_channel_info *channel_info)
{
    const struct log_channel_desc *desc;
    s32 channel_id;
    s32 ret;

    if ((device_ctx == NULL) || (channel_info == NULL) || (channel_info->log_desc == NULL)) {
        slog_drv_err("Input parameter or log_desc is NULL. "
            "(device_ctx_is_null=%d; channel_info_is_null=%d)\n", (device_ctx == NULL), (channel_info == NULL));
        return LOG_RET_ERROR;
    }

    desc = channel_info->log_desc;
    channel_id = channel_info->channel_type;

    mutex_lock(&channel_info->cmd_mutex);
    if (channel_info->channel_state != LOG_CHANNEL_ENABLE) {
        mutex_unlock(&channel_info->cmd_mutex);
        slog_drv_err("Channel has not been created. (channel_id=%d)\n", channel_id);
        return LOG_RET_ERROR;
    }

    LOG_DEBUG_CHANINFO_SHOW(channel_info);

    // 1. send delete cmd to sub system
    if (desc->del != NULL) {
        ret = desc->del(device_ctx, channel_info);
        if (ret != (s32)LOG_RET_OK) {
            mutex_unlock(&channel_info->cmd_mutex);
            slog_drv_err("Channel has not been created. (channel_id=%d)\n", channel_id);
            return ret;
        }
    }

    slog_drv_info("Log_delete_channel succeeded. (channel_id=%d)\n", channel_id);

    log_free_channel_memory(channel_info);
    channel_info->channel_state = LOG_CHANNEL_DISABLE;
    mutex_unlock(&channel_info->cmd_mutex);

    return LOG_RET_OK;
}

s32 log_send_set_level_cmd(struct log_device_ctx *device_ctx, s32 channel_type,
    struct log_channel_info *channel_info, struct log_ioctl_para *log_para)
{
    const struct log_channel_desc *log_chan_desc = NULL;
    s32 ret = LOG_RET_OK;

    if ((device_ctx == NULL) || (channel_info == NULL) || (channel_info->log_desc == NULL) || (log_para == NULL)) {
        slog_drv_err("Input parameter is NULL. "
            "(device_ctx_is_null=%d; channel_info_is_null=%d; log_para_is_null=%d)\n",
            (device_ctx == NULL), (channel_info == NULL), (log_para == NULL));
        return LOG_RET_ERROR;
    }

    log_chan_desc = channel_info->log_desc;
    LOG_DEBUG_BUFFHEAD_SHOW((struct log_channel_buf_head *)(channel_info->vir_addr));
    LOG_DEBUG_CHANINFO_SHOW(channel_info);

    // 1. check if set log level is supported
    if (!(s32)(log_chan_desc->log_level_support)) {
        slog_drv_debug("Show details. (device_id=%d; tmp_channel_id=%d; log_chan_desc->log_level_support=%d)\n",
            log_para->device_id, channel_info->channel_id, log_chan_desc->log_level_support);
        return LOG_RET_OK;
    }

    // 2. check if channel_type is user_channel_type
    if (channel_type != log_chan_desc->channel_type) {
        slog_drv_debug("Show details. (channel_id=%d; user_channel_type=%d; tmp_channel_type=%d)\n",
            channel_info->channel_id, channel_type, log_chan_desc->channel_type);
        return LOG_RET_OK;
    }

    // 3. set level to sub system
    if (log_chan_desc->set_level != NULL) {
        ret = log_chan_desc->set_level(device_ctx, channel_info, log_para->log_level);
        if (ret != (s32)LOG_RET_OK) {
            slog_drv_err("Set_level failed. (device_id=%u; tmp_channel_id=%d; ret=%d)\n",
                log_para->device_id, channel_info->channel_id, ret);
            return ret;
        }
    }

    LOG_DEBUG_BUFFHEAD_SHOW((struct log_channel_buf_head *)(channel_info->vir_addr));
    LOG_DEBUG_CHANINFO_SHOW(channel_info);
    return ret;
}

STATIC s32 log_user_set_level(struct log_device_ctx *device_ctx, struct log_ioctl_para *log_para)
{
    struct log_channel_info *channel_info = NULL;
    s32 channel_type;
    s32 ret = -1;
    s32 i;

    if (log_para == NULL) {
        slog_drv_err("Log_para is NULL.\n");
        return LOG_RET_ERROR;
    }

    if ((log_para->log_level >= LOG_CHANNEL_LEVEL_MAX) || (log_para->log_level < 0) ||
        (log_is_channel_valid(log_para->channel_type) != 0)) {
        slog_drv_err("Log_level or channel_type is invalid. "
            "(device_id=%u; channel_type=%d; log_level=%d)\n",
            log_para->device_id, log_para->channel_type, log_para->log_level);
        return LOG_RET_ERROR;
    }

    log_para->ret = LOG_RET_ERROR;

    channel_type = log_para->channel_type;
    if ((channel_type < (s32)LOG_CHANNEL_TYPE_TS) || (channel_type >= (s32)LOG_CHANNEL_TYPE_MAX)) {
        slog_drv_err("Channel_type out of range. (device_id=%d; channel_type=%d; channel_type_range=\"0-%d\")\n",
            log_para->device_id, channel_type, (s32)LOG_CHANNEL_TYPE_MAX - 1);
        return LOG_RET_ERROR;
    }

    for (i = 0; i < LOG_CHANNEL_NUM_MAX; i++) {
        channel_info = &device_ctx->channels_info[i];
        // 1.judge if channel info is valid.
        if (channel_info->channel_state != LOG_CHANNEL_ENABLE) {
#ifdef CFG_SOC_PLATFORM_MDC_V51
            mutex_lock(&channel_info->cmd_mutex);
            if (channel_info->channel_state != LOG_CHANNEL_ENABLE) {
                g_log_user_level[channel_type] = log_para->log_level;
            }
            mutex_unlock(&channel_info->cmd_mutex);
#endif
            continue;
        }
        mutex_lock(&channel_info->cmd_mutex);
        ret = log_send_set_level_cmd(device_ctx, channel_type, channel_info, log_para);
        if (ret != (s32)LOG_RET_OK) {
            mutex_unlock(&channel_info->cmd_mutex);
            log_para->ret = ret;
            return log_para->ret;
        }
        mutex_unlock(&channel_info->cmd_mutex);
    }

    log_para->ret = ret;
    slog_drv_event("Log_user_set_level succeeded. (device_id=%d; channel_type=%d; log_level=%d)\n",
        log_para->device_id, channel_type, log_para->log_level);

    return log_para->ret;
}

STATIC s32 log_read_channel(const struct log_device_ctx *device_ctx, s32 channel_id, struct log_msg_buf *recv_buf)
{
    struct log_channel_info *channel_info = NULL;
    const struct log_channel_buf_head *buf_ptr = NULL;
    u32 buf_read;
    u32 buf_write;
    s32 device_id, ret;

    device_id = device_ctx->device_info.device_id;
    if (unlikely((device_id < 0) || (device_id >= log_desc.device_num))) {
        slog_drv_err("Device_id is invalid. (device_id=%d)\n", device_id);
        return LOG_RET_ERROR;
    }

    if (unlikely(log_is_channel_valid(channel_id) != 0)) {
        slog_drv_err("Channel_id is invalid. (device_id=%d; channel_id=%d)\n",
            device_id, channel_id);
        return LOG_RET_ERROR;
    }

    channel_info = &device_ctx->channels_info[channel_id];

    mutex_lock(&channel_info->cmd_mutex);
    buf_ptr = (struct log_channel_buf_head *)channel_info->vir_addr;

#if (!defined CFG_SOC_PLATFORM_CLOUD) && (!defined CFG_SOC_PLATFORM_CLOUD_v2)
    // cloud's cache is half conherent. control cpu need not flush cache before read.
    LOG_FLUSH_CACHE(channel_info, 1);
#endif
    buf_read = buf_ptr->buf_read;
    buf_write = atomic_read((atomic_t *)&buf_ptr->buf_write);
    /* print_num is set to control log's print for every channel; and always go back; */
    if (unlikely((buf_write > (channel_info->buf_size - sizeof(struct log_channel_buf_head))) ||
                 (buf_read > (channel_info->buf_size - sizeof(struct log_channel_buf_head))))) {
        if (channel_info->print_num < LOG_PRINT_MAX) {
            slog_drv_err("Log's buff is error. "
                "(device_id=%d; channel_id=%d; len=%lu;buf_read=%u; buf_write=%u)\n", device_id, channel_id,
                (channel_info->buf_size - sizeof(struct log_channel_buf_head)), buf_read, buf_write);
            channel_info->print_num++;
        }
        mutex_unlock(&channel_info->cmd_mutex);
        return LOG_RET_ERROR;
    }

    if (buf_write == buf_read) {
        LOG_DEBUG_CHANINFO_SHOW(channel_info);
        LOG_DEBUG_BUFFHEAD_SHOW(buf_ptr);
        mutex_unlock(&channel_info->cmd_mutex);
        return LOG_RET_OK;
    }
    ret = log_read_ringbuffer(channel_info, buf_read, buf_write, recv_buf);
    mutex_unlock(&channel_info->cmd_mutex);

    return ret;
}

s32 log_read_ringbuffer_r2w(const struct log_channel_info *channel_info, s32 buf_read, s32 buf_write,
    struct log_msg_buf *recv_buf, const struct log_channel_buf_head *buf_ptr)
{
    s8 *out_buf = NULL;
    s32 out_avail_size;
    s32 buf_len;
    const s8 *base = NULL;
    s32 ret;

    buf_len = buf_write - buf_read;
    base = (s8 *)(channel_info->vir_addr + sizeof(struct log_channel_buf_head) + (u32)buf_read);
    out_buf = recv_buf->data_addr + recv_buf->data_size;
    out_avail_size = recv_buf->buf_addr + recv_buf->buf_size - out_buf;
    if (buf_len > out_avail_size) {
        return LOG_RET_EOF;
    }
    if (channel_info->channel_id == (s32)LOG_CHANNEL_TYPE_SIS) {
        memcpy_fromio((void *)out_buf, (void *)base, (size_t)(u32)buf_len);
    } else {
        ret = memcpy_s((void *)out_buf, (size_t)(u32)out_avail_size, (void *)base, (size_t)(u32)buf_len);
        if (ret != EOK) {
            slog_drv_err("Copy base to out_buf failed. (ret=%d)\n", ret);
            return ret;
        }
    }
    recv_buf->data_size += buf_len;

    atomic_set((atomic_t *)&buf_ptr->buf_read, buf_write);
    LOG_FLUSH_CACHE(channel_info, 0);

    LOG_DEBUG_READ_STOP_SHOW(channel_info, buf_ptr);

    return LOG_RET_OK;
}

STATIC s32 log_read_ringbuffer(const struct log_channel_info *channel_info, s32 buf_read, s32 buf_write,
    struct log_msg_buf *recv_buf)
{
    const struct log_channel_buf_head *buf_ptr = NULL;
    s8 *out_buf = NULL;
    s32 out_avail_size;
    s32 buf_len;
    const s8 *base = NULL;
    s32 ret;

    buf_ptr = (struct log_channel_buf_head *)channel_info->vir_addr;

    LOG_DEBUG_READ_START_SHOW(channel_info, buf_ptr);

    if (buf_write > buf_read) {
        return log_read_ringbuffer_r2w(channel_info, buf_read, buf_write, recv_buf, buf_ptr);
    }

    /* Save the scenario where the write pointer is smaller than the read pointer. */
    if (buf_read < (s32)(channel_info->buf_size - sizeof(struct log_channel_buf_head))) {
        buf_len = (s32)((channel_info->buf_size - sizeof(struct log_channel_buf_head)) - (u32)buf_read);
        base = (s8 *)(channel_info->vir_addr + sizeof(struct log_channel_buf_head) + (u32)buf_read);

        out_buf = recv_buf->data_addr + recv_buf->data_size;
        out_avail_size = recv_buf->buf_addr + recv_buf->buf_size - out_buf;
        if (buf_len > out_avail_size) {
            return LOG_RET_EOF;
        }

        if (channel_info->channel_id == (s32)LOG_CHANNEL_TYPE_SIS) {
            memcpy_fromio((void *)out_buf, (void *)base, (size_t)(u32)buf_len);
        } else {
            ret = memcpy_s((void *)out_buf, (size_t)(u32)out_avail_size, (void *)base, (size_t)(u32)buf_len);
            if (ret != EOK) {
                slog_drv_err("Copy base to out_buf failed. (ret=%d)\n", ret);
                return ret;
            }
        }

        recv_buf->data_size += buf_len;

        atomic_set((atomic_t *)&buf_ptr->buf_read, 0);
        /* The write pointer is 0. */
        if (buf_write == 0) {
            LOG_FLUSH_CACHE(channel_info, 0);

            LOG_DEBUG_READ_STOP_SHOW(channel_info, buf_ptr);

            return LOG_RET_OK;
        }
    } else {
        buf_len = 0;
    }

    /* notice user buffer's offset */
    base = (s8 *)(channel_info->vir_addr + sizeof(struct log_channel_buf_head));

    out_buf = recv_buf->data_addr + recv_buf->data_size;
    out_avail_size = recv_buf->buf_addr + recv_buf->buf_size - out_buf;
    if (buf_write > out_avail_size) {
        LOG_FLUSH_CACHE(channel_info, 0);
        return LOG_RET_EOF;
    }

    if (channel_info->channel_id == (s32)LOG_CHANNEL_TYPE_SIS) {
        memcpy_fromio((void *)out_buf, (void *)base, (size_t)(u32)buf_write);
    } else {
        ret = memcpy_s((void *)out_buf, (size_t)(u32)out_avail_size, (void *)base, (size_t)(u32)buf_write);
        if (ret != EOK) {
            slog_drv_err("Copy base to out_buf failed. (ret=%d)\n", ret);
            LOG_FLUSH_CACHE(channel_info, 0);
            return ret;
        }
    }
    recv_buf->data_size += buf_write;

    atomic_set((atomic_t *)&buf_ptr->buf_read, buf_write);
    LOG_FLUSH_CACHE(channel_info, 0);
    LOG_DEBUG_READ_STOP_SHOW(channel_info, buf_ptr);

    return LOG_RET_OK;
}

STATIC bool log_channel_need_create(s32 channel_type, log_create_channels create_channel_type)
{
    bool ret = true;
    switch (create_channel_type) {
        case LOG_CREATE_NORMAL_CHANNELS:
            if ((channel_type == LOG_CHANNEL_TYPE_TS) || (channel_type == LOG_CHANNEL_TYPE_TS_DUMP)) {
                ret = false;
            }
            break;
        case LOG_CREATE_TS_CHANNELS:
            if ((channel_type != LOG_CHANNEL_TYPE_TS) && (channel_type != LOG_CHANNEL_TYPE_TS_DUMP)) {
                ret = false;
            }
            break;
        case LOG_CREATE_ALL_CHANNELS:
            ret = true;
            break;
        default:
            ret = false;
            slog_drv_err("Create channel type is invalid. (create_channel_type=%d)\n", (s32)create_channel_type);
    }

    return ret;
}

STATIC s32 log_create_all_channels(struct log_device_ctx *device_ctx, log_create_channels create_channel_type)
{
    const struct log_channel_desc *log_chan_desc = log_get_channel_desc();
    s32 channel_sets_num;
    s32 device_id;
    s32 ret;
    s32 i;

    device_id = device_ctx->device_info.device_id;
    channel_sets_num = log_get_channel_num();
    if (channel_sets_num == 0) {
        slog_drv_err("Channel number is 0.\n");
        return LOG_RET_ERROR;
    }

    slog_drv_debug("Channel details. (device_id=%d; channel_sets_num=%d)\n", device_id, channel_sets_num);

    for (i = 0; i < channel_sets_num; i++) {
        if (!log_channel_need_create(log_chan_desc[i].channel_type, create_channel_type)) {
            continue;
        }
        ret = log_create_channel(device_ctx, &log_chan_desc[i]);
        if (ret != (s32)LOG_RET_OK) {
            slog_drv_warn("Channel creat failed. (device_id=%d; i=%d)\n", device_id, i);
        } else {
            slog_drv_info("Channel created successfully. (device_id=%d; i=%d)\n", device_id, i);
        }
    }

    return LOG_RET_OK;
}

STATIC void log_delete_all_channels(struct log_device_ctx *device_ctx)
{
    struct log_channel_info *channel_info = NULL;
    s32 channel_id;
    s32 ret;

    for (channel_id = 0; channel_id < LOG_CHANNEL_NUM_MAX; channel_id++) {
        channel_info = &device_ctx->channels_info[channel_id];

        mutex_lock(&channel_info->cmd_mutex);
        if ((log_is_channel_valid(channel_id) != 0) ||
            (channel_info->channel_state != LOG_CHANNEL_ENABLE)) {
            mutex_unlock(&channel_info->cmd_mutex);
            continue;
        }
        mutex_unlock(&channel_info->cmd_mutex);

        ret = log_delete_channel(device_ctx, channel_info);
        if (ret != (s32)LOG_RET_OK) {
            slog_drv_err("Channel delete failed. (channel_id=%d)\n", channel_id);
        } else {
            slog_drv_info("Channel delete successfully. (channel_id=%d)\n", channel_id);
        }
    }
}

// return next channel_id which not be read. this method is fair for all channels.
STATIC s32 log_read_multi_channels(struct log_device_ctx *device_ctx, s32 channel_ids, s32 channel_ids_num)
{
    struct log_channel_info *channel_info = NULL;
    s32 channel_id = channel_ids;
    struct log_msg_buf *recv_buf = NULL;
    s32 log_type;
    s32 ret;
    s32 i;

    for (log_type = LOG_TYPE_START; log_type < (s32)LOG_TYPE_MAX; log_type++) {
        for (i = 0; i < channel_ids_num; i++) {
            channel_id = (channel_ids + i) % LOG_CHANNEL_NUM_MAX;
            channel_info = &device_ctx->channels_info[channel_id];

#ifdef CFG_SOC_PLATFORM_MDC_V51
            if (mutex_trylock(&channel_info->cmd_mutex) == 0) {
                continue;
            }
#else
            mutex_lock(&channel_info->cmd_mutex);
#endif
            if ((channel_info->channel_state != LOG_CHANNEL_ENABLE) ||
                ((log_type == LOG_TYPE_START) && (channel_info->log_type != LOG_TYPE_START)) ||
                ((log_type == LOG_TYPE_RUN) && (channel_info->log_type != LOG_TYPE_RUN))) {
                mutex_unlock(&channel_info->cmd_mutex);
                continue;
            }

            mutex_unlock(&channel_info->cmd_mutex);
            recv_buf = &device_ctx->channel_log_buf;
            LOG_DEBUG_CHANINFO_SHOW(channel_info);
            ret = log_read_channel(device_ctx, channel_id, recv_buf);
            if (unlikely(ret == LOG_RET_EOF)) {
                // EOF main this channel is not read, so return this channel_id.
                return channel_id;
            }
        }
    }

    // last channel is read, so return next channel_id.
    return (channel_id + 1) % LOG_CHANNEL_NUM_MAX;
}

STATIC s32 log_msg_compress(const struct log_device_ctx *device_ctx, struct log_msg_head *msg_head, s32 msg_data_size,
    struct log_msg_buf *compress_buf)
{
    s32 compressed = 0;
    s32 ret;

    if (likely(atomic_read(&device_ctx->compress_config) && !IS_ERR(device_ctx->compress_desc.tfm))) {
        compress_buf->data_size = compress_buf->buf_size - compress_buf->head_size;
        ret = crypto_comp_compress(device_ctx->compress_desc.tfm, (void *)msg_head + sizeof(struct log_msg_head),
                                   (u32)msg_data_size, (u8 *)compress_buf->data_addr,
                                   (u32 *)&compress_buf->data_size);
        if (likely(ret == (s32)LOG_RET_OK)) {
            compressed = 1;
        } else {
            slog_drv_err("Failed to compress. (ret=%d)\n", ret);
        }
    }

    msg_head->data_compressed = (u16)compressed;
    if (compressed == 1) {
        msg_head->data_len = compress_buf->data_size;
        ret = memcpy_s((void *)compress_buf->head_addr, compress_buf->head_size, (void *)msg_head,
                       sizeof(struct log_msg_head));
        if (unlikely(ret != EOK)) {
            slog_drv_err("Copy msg_head to compress_buf->head_addr failed. (ret=%d)\n", ret);
            return LOG_RET_ERROR;
        }
    } else {
        msg_head->data_len = msg_data_size;
    }
    return LOG_RET_OK;
}

static void log_set_frame_end(s32 send_size, s32 total_size, struct log_msg_head *msg_head)
{
    if (send_size == total_size) {
        msg_head->frame_end = 1;
    } else {
        msg_head->frame_end = 0;
    }
}

STATIC struct log_msg_head *log_msg_head_init(const struct log_msg_buf *send_buf)
{
    struct log_msg_head *msg_head = NULL;
    s32 ret;

    msg_head = (struct log_msg_head *)send_buf->head_addr;
    ret = memset_s((void *)msg_head, sizeof(struct log_msg_head), 0, sizeof(struct log_msg_head));
    if (ret != EOK) {
        slog_drv_err("Memset_s error. (ret=%d)\n", ret);
        return NULL;
    }

    msg_head->version_value = LOG_MSG_VERSION;
    msg_head->smp_flag = (log_desc.device_num > 1) ? 1 : 0;
    msg_head->slog_flag = (send_buf->msg_type == (s32)LOG_MSG_TYPE_SLOG) ? 1 : 0;
    msg_head->frame_begin = 1;

    return msg_head;
}

STATIC s32 log_send_msg_to_host(struct log_device_ctx *device_ctx,
                                struct log_msg_buf *send_buf,
                                struct log_msg_buf *compress_buf,
                                u32 timeout)
{
    struct log_msg_head *msg_head = NULL;
    s32 total_data_size;
    s32 hdc_data_segment;
    s32 hdc_send_size;
    void *hdc_send_ptr = NULL;
    s32 ret;
    static u32 suspend_send_failed_count = 0;
    static u32 send_failed_count = 0;

    if (log_drv_is_suspend() != 0) {
        if (suspend_send_failed_count < LOG_PRINT_MAX) {
            slog_drv_warn("In low power state, device doesn't need to send message to host.\n");
            suspend_send_failed_count++;
        }
        msleep(LOG_SCHEDULE_INTERVAL);
        return LOG_RET_OK;
    }
    suspend_send_failed_count = 0;

    msg_head = log_msg_head_init(send_buf);
    if (msg_head == NULL) {
        slog_drv_err("Log_msg_head_init failed, msg_head is NULL.\n");
        return LOG_RET_ERROR;
    }

    total_data_size = send_buf->data_size;
    // hdc_desc.segment_size is insure > sizeof(struct log_msg_head), when init.
    hdc_data_segment = device_ctx->hdc_desc.segment_size - (s32)sizeof(struct log_msg_head);

    // lock the entire send process, avoid multiple threads sending packets alternately.
    mutex_lock(&device_ctx->hdc_desc.ops_mutex);
    if (unlikely(device_ctx->hdc_desc.connected == 0)) {
        slog_drv_err("Hdcdrv is not ready.\n");
        mutex_unlock(&device_ctx->hdc_desc.ops_mutex);
        return LOG_RET_ERROR;
    }

    while (total_data_size > 0) {
        hdc_send_size = (total_data_size < hdc_data_segment) ? total_data_size : hdc_data_segment;

        log_set_frame_end(hdc_send_size, total_data_size, msg_head);

        ret = log_msg_compress(device_ctx, msg_head, hdc_send_size, compress_buf);
        if (ret != (s32)LOG_RET_OK) {
            slog_drv_err("Log_msg_compress failed. (ret=%d)\n", ret);
            ret = (s32)LOG_RET_ERROR;
            break;
        }

        if ((msg_head->data_compressed) != 0) {
            hdc_send_ptr = (void *)compress_buf->head_addr;
        } else {
            hdc_send_ptr = (void *)msg_head;
        }

        ret = (s32)log_desc.ops.sess_send_timeout_func(device_ctx->hdc_desc.session,
            hdc_send_ptr,
            (size_t)(u32)(msg_head->data_len) + sizeof(struct log_msg_head),
            (s32)timeout);
        if (unlikely(ret != (s32)LOG_RET_OK)) {
            if ((ret != (s32)LOG_RET_TIMEOUT) && ((send_failed_count) < LOG_PRINT_MAX)) {
                slog_drv_err("Log send msg failed. (send_failed_count=%u; ret=%d)\n",
                    ++send_failed_count, ret);
            }
            break;
        }
        send_failed_count = 0;

        total_data_size -= hdc_send_size;
        if (total_data_size <= 0) {
            ret = LOG_RET_OK;
            break;
        }

        // slide the msg_head window
        ret = memmove_s((void *)msg_head + hdc_send_size, sizeof(struct log_msg_head), (void *)msg_head,
                        sizeof(struct log_msg_head));
        if (unlikely(ret != EOK)) {
            slog_drv_err("Move msg_head offset failed. (hdc_send_size=%d; ret=%d)\n", hdc_send_size, ret);
            ret = (s32)LOG_RET_ERROR;
            break;
        }
        msg_head = (struct log_msg_head *)((void *)msg_head + hdc_send_size);
        msg_head->frame_begin = 0;
    }

    mutex_unlock(&device_ctx->hdc_desc.ops_mutex);

    if (ret == LOG_RET_OK) {
        atomic_add(send_buf->data_size, &device_ctx->generate_rate.total_volume);

        send_buf->data_size = 0;
        compress_buf->data_size = 0;
    }

    return ret;
}

STATIC void log_send_ts_channel_info_to_host(struct log_device_ctx *device_ctx)
{
    struct log_channel_info *ts_channels_info = &device_ctx->channels_info[LOG_CHANNEL_TYPE_TS];
    const struct log_channel_desc *ts_channel_desc = ts_channels_info->log_desc;
    u32 devid, ts_mem_size;
    bool is_dynamic_alloc;
    u64 ts_pa;
    s32 ret;

    if (ts_channels_info->channel_state != LOG_CHANNEL_ENABLE) {
        slog_drv_warn("Ts channel is unable\n");
        return;
    }

    is_dynamic_alloc = (ts_channel_desc->buf_phy_addr == 0) ? true: false;
    devid = device_ctx->device_info.device_id;
    ts_pa = ts_channels_info->phy_addr;
    ts_mem_size = ts_channels_info->buf_size;
    ret = devdrv_manager_send_tslog_addr_to_host(devid, ts_pa, ts_mem_size, is_dynamic_alloc);
    if (ret != 0) {
        slog_drv_err("Send ts channel fail. (devid=%u)\n", devid);
    }
}

STATIC s32 log_work_init(struct log_device_ctx *device_ctx)
{
    struct log_device_info device_info;
    s32 device_id;
    s32 ret;

    if (device_ctx == NULL) {
        slog_drv_err("Device_ctx is NULL.\n");
        return LOG_RET_ERROR;
    }
    device_info = device_ctx->device_info;
    device_id = device_info.device_id;

    ret = log_create_hdc_server(device_ctx);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Log_create_hdc_server failed. (device_id=%d)\n", device_id);
        return LOG_RET_ERROR;
    }
    device_ctx->hdc_desc.created = 1;

#ifdef CFG_SOC_PLATFORM_MDC_V51
    log_launch_ts_threads();
    ret = log_create_all_channels(device_ctx, LOG_CREATE_NORMAL_CHANNELS);
#else
    ret = log_drv_wait_ts(device_ctx);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Log_drv_wait_ts failed, please check TS's status. (device_id=%d)\n", device_id);
        return LOG_RET_ERROR;
    }
    ret = log_create_all_channels(device_ctx, LOG_CREATE_ALL_CHANNELS);
#endif
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Log_create_all_channels failed. (device_id=%d)\n", device_id);
        return LOG_RET_ERROR;
    }

    log_send_ts_channel_info_to_host(device_ctx);

    return LOG_RET_OK;
}

STATIC void log_work_uninit(struct log_device_ctx *device_ctx)
{
    if (device_ctx == NULL) {
        slog_drv_err("Device_ctx is NULL.\n");
        return;
    }

    log_delete_all_channels(device_ctx);

    mutex_lock(&device_ctx->hdc_desc.ops_mutex);
    if (device_ctx->hdc_desc.connected == 1) {
        (void)log_desc.ops.sess_close_func();
        device_ctx->hdc_desc.connected = 0;
        device_ctx->hdc_desc.session = LOG_HDC_INVALID_SESSION;
    }

    if (device_ctx->hdc_desc.created == 1) {
        (void)log_desc.ops.sess_destroy_func();
        device_ctx->hdc_desc.created = 0;
    }
    mutex_unlock(&device_ctx->hdc_desc.ops_mutex);
}

STATIC s32 log_hdc_prepare_to_work(struct log_device_ctx *device_ctx)
{
    s32 device_id;
    if (device_ctx == NULL) {
        slog_drv_err("Device_ctx is NULL.\n");
        return LOG_RET_ERROR;
    }
    device_id = device_ctx->device_info.device_id;

    mutex_lock(&device_ctx->hdc_desc.ops_mutex);

    if (likely((device_ctx->hdc_desc.connected == 1) && (device_ctx->hdc_desc.host_closed == 0))) {
        mutex_unlock(&device_ctx->hdc_desc.ops_mutex);
        return LOG_RET_OK;
    }

    if (log_drv_is_suspend() != 0) {
        mutex_unlock(&device_ctx->hdc_desc.ops_mutex);
        slog_drv_warn("In low power state, device doesn't need to send message to the host. (device_id=%d)\n",
            device_id);
        msleep(LOG_SCHEDULE_INTERVAL);
        return LOG_RET_ERROR;
    }

    atomic_set(&device_ctx->status, (s32)LOG_DEVICE_STATUS_WAIT_HOST);

    if (device_ctx->hdc_desc.connected == 1) {
        slog_drv_warn("Old HDC session has broken, so hdcdrv_kernel_close is to be called. "
            "(device_id=%d; session=%d)\n", device_id, device_ctx->hdc_desc.session);
        // this func will send the close notification to host, so need to judge whether in low power state before it
        (void)log_desc.ops.sess_close_func();
        device_ctx->hdc_desc.connected = 0;
        device_ctx->hdc_desc.session = LOG_HDC_INVALID_SESSION;
    }

    mutex_unlock(&device_ctx->hdc_desc.ops_mutex);
    return log_hdc_wait_host(device_ctx);
}

STATIC s32 log_hdc_wait_host(struct log_device_ctx *device_ctx)
{
    s32 device_id;
    s32 run_env = 0;
    s32 ret;

    if (device_ctx == NULL) {
        slog_drv_err("Device_ctx is NULL.\n");
        return LOG_RET_ERROR;
    }
    device_id = device_ctx->device_info.device_id;

    /*
     * block here, until accept hdc client connected. do not lock it,
     * because other thread could break it by destroy function.
     */
    slog_drv_info("Begin to hdcdrv_kernel_accept. (device_id=%d)\n", device_id);
    ret = (s32)log_desc.ops.sess_accept_func(device_id, &device_ctx->hdc_desc.session);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Hdcdrv_kernel_accept failed. (device_id=%d)\n", device_id);
        return LOG_RET_ERROR;
    }

    ret = (s32)log_desc.ops.sess_get_run_env_func(&run_env);
    if ((ret == (s32)LOG_RET_OK) && (run_env == (s32)LOG_SESSION_RUN_ENV_CONTAINER)) {
        slog_drv_warn("The connection came from the container, so close it. (session=%d)\n",
            device_ctx->hdc_desc.session);
        (void)log_desc.ops.sess_close_func();
        device_ctx->hdc_desc.session = LOG_HDC_INVALID_SESSION;
        return LOG_RET_NOT_SUPPORT;
    }

    mutex_lock(&device_ctx->hdc_desc.ops_mutex);
    device_ctx->hdc_desc.connected = 1;
    device_ctx->hdc_desc.host_closed = 0;
    mutex_unlock(&device_ctx->hdc_desc.ops_mutex);

    // thread ready to work
    atomic_set(&device_ctx->status, (s32)LOG_DEVICE_STATUS_WORK);

    slog_drv_info("Hdcdrv_kernel_accept successfully . "
        "(device_id=%d; session=%d)\n", device_id, device_ctx->hdc_desc.session);

    return LOG_RET_OK;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
#define LOG_WAIT_XPC_OPEN_STOP 300
STATIC void log_wait_ts_threads_stop(void)
{
    s32 cnt;

    cnt = 0;
    while ((g_log_ts_thread_stop != true) && (cnt <= LOG_WAIT_XPC_OPEN_STOP)) {
        if (g_log_ts_thread_stop == true) {
            break;
        }
        cnt++;
        msleep(200); /* 200ms */
    }

    return;
}

STATIC void log_wait_xpc_threads_stop(void)
{
    s32 cnt;

    log_stop_xpc_threads();

    cnt = 0;
    while ((g_log_xpc_thread_stop != true) && (cnt <= LOG_WAIT_XPC_OPEN_STOP)) {
        if (g_log_xpc_thread_stop == true) {
            break;
        }
        cnt++;
        msleep(200); /* 200ms */
    }

    return;
}
#endif

/* thread execution function, its prototype: int (*threadfn)(void *arg) */
STATIC s32 log_work_thread(void *arg)
{
    s32 device_id;
    s32 channel_ids = 0;
    struct log_device_ctx *device_ctx = NULL;
    s32 ret;
    u32 chlog_send_failed_count = 0;

    if (arg == NULL) {
        slog_drv_err("Input parameter arg is NULL.\n");
        return LOG_RET_ERROR;
    }

    /* add for clearing alarm, param(arg) is required for the prototype of thread execution function */
    device_id = *(s32 *)(arg);
    *(s32 *)(arg) = device_id;

    slog_drv_info("Enter log_work_thread. (device_id=%d)\n", device_id);

    device_ctx = log_create_device_ctx(device_id);
    if (device_ctx == NULL) {
        slog_drv_err("Log_create_device_ctx failed. (device_id=%d)\n", device_id);
        return LOG_RET_ERROR;
    }
    log_desc.device_ctx[device_id] = device_ctx;

    log_dev_session_init(device_id);

    ret = log_work_init(device_ctx);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Log_work_init failed. (device_id=%d)\n", device_id);
        goto undo_log_create_device_ctx;
    }

    // switch status to wait hdc connect
    atomic_set(&device_ctx->status, (s32)LOG_DEVICE_STATUS_WAIT_HOST);

    // work loop
    while (!kthread_should_stop()) {
        if (unlikely(atomic_read(&device_ctx->status) == LOG_DEVICE_STATUS_INIT)) {
            slog_drv_err("The member status in device_ctx is set to INIT, break here. (device_id=%d)\n",
                device_id);
            break;
        }

        ret = log_hdc_prepare_to_work(device_ctx);
        if (unlikely(ret != (s32)LOG_RET_OK)) {
            msleep(LOG_DELAY_TIME);
            continue;
        }

        mutex_lock(&device_ctx->channel_buf_mutex);
        channel_ids = log_read_multi_channels(device_ctx, channel_ids, LOG_CHANNEL_NUM_MAX);

        if (device_ctx->channel_log_buf.data_size > 0) {
            ret = log_send_msg_to_host(device_ctx,
                                       &device_ctx->channel_log_buf,
                                       &device_ctx->channel_compress_buf,
                                       LOG_HDCDRV_SEND_TIMEOUT);
            if (unlikely(ret != (s32)LOG_RET_OK)) {
                if ((ret != LOG_RET_TIMEOUT) && ((chlog_send_failed_count) < LOG_PRINT_MAX)) {
                    slog_drv_err("Channels log send failed. "
                        "(device_id=%d; channle_id=%d; count=%u; data_size=%d; data_threshold=%d; ret=%d)\n",
                        device_id, channel_ids, ++chlog_send_failed_count, device_ctx->channel_log_buf.data_size,
                        device_ctx->channel_log_buf.data_threshold, ret);
                }
            } else {
                chlog_send_failed_count = 0;
            }
        }
        mutex_unlock(&device_ctx->channel_buf_mutex);
        msleep(1);
    }

    atomic_set(&device_ctx->status, (s32)LOG_DEVICE_STATUS_INIT);

undo_log_create_device_ctx:
#ifdef CFG_SOC_PLATFORM_MDC_V51
    log_wait_ts_threads_stop();
#endif
    log_work_uninit(device_ctx);
#ifdef CFG_SOC_PLATFORM_MDC_V51
    log_wait_xpc_threads_stop();
#endif
    mutex_destroy(&(device_ctx->hdc_desc.sess_info.sess_mutex));
    log_destroy_device_ctx(device_ctx);
    device_ctx = NULL;
    log_desc.device_ctx[device_id] = NULL;

    return ret;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51

#define XPC_POLL_CHL_NUM 2
#define XPC_POLL_TIME_OUT 3000  // 3000 ms
#define XPC_READ_BUF_LEN 256
s32 log_xpc_work_thread(void *arg)
{
    int ret, res_num, i;
    unsigned int read_len, pack_type;
    unsigned char out_buf[XPC_READ_BUF_LEN];
    struct chl_poll_ret *poll_chl = NULL;
    struct chl_poll_ret poll_ret[XPC_POLL_CHL_NUM] = {};
    int poll_chl_id[XPC_POLL_CHL_NUM] = {};

    poll_chl_id[0] = log_xpc_get_chl_id(XPC_CHANNEL_TYPE_CQ0);
    poll_chl_id[1] = log_xpc_get_chl_id(XPC_CHANNEL_TYPE_CQ1);
    slog_drv_info("xpc thread work, chl=%d, %d\n", poll_chl_id[0], poll_chl_id[1]);
    while (!kthread_should_stop()) {
        res_num = 0;
        ret = xpcshm_poll(poll_chl_id, XPC_POLL_CHL_NUM, poll_ret, &res_num, XPC_POLL_TIME_OUT);
        if (ret < 0) {
            if (ret != XPC_ERR_POLL_TIMEOUT && ret != XPC_ERR_POLL_CANCELED_BY_OTHERS) {
                slog_drv_err("xpc shm poll err, ret=%d\n", ret);
            }
            continue;
        }
        for (i = 0; i < res_num; i++) {
            poll_chl = &poll_ret[i];
            read_len = XPC_READ_BUF_LEN;
            ret = xpcshm_read(poll_chl->id, out_buf, &read_len, &pack_type);
            if (ret < 0) {
                slog_drv_err("xpc shm read err, ret=%d\n", ret);
                continue;
            }
            if (poll_chl->id == log_xpc_get_chl_id(XPC_CHANNEL_TYPE_CQ0)) {
                log_xpc_parse_cq_data(out_buf, read_len, XPC_CHANNEL_TYPE_CQ0);
            } else if (poll_chl->id == log_xpc_get_chl_id(XPC_CHANNEL_TYPE_CQ1)) {
                log_xpc_parse_cq_data(out_buf, read_len, XPC_CHANNEL_TYPE_CQ1);
            }
        }
    }

    g_log_xpc_thread_stop = true;
    return 0;
}
#endif

STATIC s32 log_user_get_channel_type(struct log_ioctl_para *log_para)
{
    const struct log_channel_desc *log_chan_desc = log_get_channel_desc();
    s32 channel_sets_num;
    s32 i;
    s32 valid_channel_num = 0;
    s32 ret;

    channel_sets_num = log_get_channel_num();
    for (i = 0; i < channel_sets_num; i++) {
        ret = log_is_channel_valid (log_chan_desc[i].channel_ids);
        if ((ret == (s32)LOG_RET_OK) && (log_chan_desc[i].channel_ids_num > 0)) {
            log_para->channel_type_set[valid_channel_num] = log_chan_desc[i].channel_type;
            slog_drv_debug("Loop details. (valid_channel_num=%d; channel_type_set[valid_channel_num]=%d)\n",
                valid_channel_num, log_para->channel_type_set[valid_channel_num]);
            valid_channel_num++;
        }
    }
    slog_drv_debug("Show details about valid_channel_num. (valid_channel_num=%d)\n", valid_channel_num);
    log_para->channel_type_num = valid_channel_num;
    log_para->ret = LOG_RET_OK;

    return LOG_RET_OK;
}

STATIC s32 log_user_get_device_id(struct log_ioctl_para *log_para)
{
    s32 ret;

    ret = memcpy_s((void *)log_para->device_id_set,
                   ((size_t)LOG_DEVICE_ID_MAX) * sizeof(s32), (void *)log_desc.device_ids,
                   ((size_t)(u32)log_desc.device_num) * sizeof(s32));
    if (ret != EOK) {
        slog_drv_err("Copy log_desc.device_ids to log_para->device_id_set failed. (ret=%d)\n", ret);
        return ret;
    }
    log_para->device_id_num = log_desc.device_num;

    log_para->ret = LOG_RET_OK;

    return LOG_RET_OK;
}

STATIC ssize_t log_sess_server_create(void)
{
    return LOG_RET_OK;
}

STATIC ssize_t log_sess_server_destroy(void)
{
    return LOG_RET_OK;
}

STATIC ssize_t log_sess_close(void)
{
    return LOG_RET_OK;
}

STATIC s32 log_sess_get_segment(void)
{
    return LOG_SEGMENT_SIZE;
}

STATIC ssize_t log_sess_get_run_env(s32 *run_env)
{
    *run_env = LOG_SESSION_RUN_ENV_HOST;
    return LOG_RET_OK;
}

STATIC ssize_t log_sess_epoll_alloc_fd(void)
{
    return LOG_RET_OK;
}

STATIC ssize_t log_sess_epoll_free_fd(void)
{
    return LOG_RET_OK;
}

STATIC ssize_t log_sess_epoll_ctl(void)
{
    return LOG_RET_OK;
}

STATIC ssize_t log_sess_epoll_wait(void)
{
    return LOG_RET_OK;
}

STATIC ssize_t log_sess_accept(s32 dev_id, s32 *session)
{
    struct log_device_ctx *device_ctx = log_desc.device_ctx[dev_id];
    s32 ret;

    if (device_ctx->hdc_desc.connected == 1) {
        slog_drv_info("It is already connected. (device_id=%d)\n", dev_id);
        return LOG_RET_OK;
    }

    ret = down_interruptible(&device_ctx->hdc_desc.sess_info.accept_sem);
    if (ret == -EINTR) {
        slog_drv_warn("Interrupted. (device_id=%d)\n", dev_id);
        return (ssize_t)LOG_RET_ERROR;
    }

    device_ctx->hdc_desc.connected = 1;

    *session = dev_id;

    slog_drv_info("Log_sess_accept succeeded. (device_id=%d)\n", dev_id);

    return LOG_RET_OK;
}

/* timeout: msec, negative values mean 'infinite timeout' */
STATIC ssize_t log_sess_send_timeout(s32 session, void *buf, s32 len, s32 timeout)
{
    s32 ret;
    s32 dev_id = session;
    struct log_device_ctx *device_ctx = log_desc.device_ctx[dev_id];
    struct session_ioctl_info *sess_info = NULL;
    size_t tmp;

    if (device_ctx->hdc_desc.connected != 1) {
        slog_drv_err("Invalid connected. "
            "(device_id=%d; connected=%d)\n", dev_id, device_ctx->hdc_desc.connected);
        return (ssize_t)LOG_RET_ERROR;
    }

    sess_info = &device_ctx->hdc_desc.sess_info;

    mutex_lock(&sess_info->sess_mutex);

    sess_info->log_buf = buf;
    sess_info->buf_size = len;
    wmb();
    atomic_set(&sess_info->status, (s32)LOG_SESSION_STATUS_READABLE);
    wake_up(&sess_info->wq);

    tmp = msecs_to_jiffies((u32)timeout);
    ret = wait_event_interruptible_timeout(sess_info->wq,
        (sess_info->status.counter == (s32)LOG_SESSION_STATUS_INIT), tmp);
    if (ret > 0) {
        ret = LOG_RET_OK;
    } else {
        slog_drv_warn("Waiting timeout. (device_id=%d; timeout=%dms; ret=%d)\n", dev_id, timeout, ret);
        if (LOG_SESSION_STATUS_READABLE != atomic_cmpxchg(&sess_info->status,
            (s32)LOG_SESSION_STATUS_READABLE, (s32)LOG_SESSION_STATUS_INIT)) {
            /* race condition: we are timeout, at the same time, log_user_read judge status = 1.
            wait some time again
             */
            slog_drv_warn("Wait for reply again. (device_id=%d)\n", dev_id);
            ret = wait_event_interruptible_timeout(sess_info->wq,
                (sess_info->status.counter == (s32)LOG_SESSION_STATUS_INIT), tmp);
#ifndef LOG_UT
            if (ret > 0) {
                ret = LOG_RET_OK;
            } else {
                slog_drv_warn("Timeout again. (device_id=%d; ret=%d)\n", dev_id, ret);
                ret = LOG_RET_TIMEOUT;
            }
#endif
            atomic_set(&sess_info->status, (s32)LOG_SESSION_STATUS_INIT);
        }
    }

    mutex_unlock(&sess_info->sess_mutex);

    return ret;
}

STATIC s32 log_local_mode_read(struct log_device_ctx *device_ctx, struct log_ioctl_para *log_para)
{
    struct session_ioctl_info *sess_info = NULL;
    size_t tmp;
    s32 ret;

    sess_info = &device_ctx->hdc_desc.sess_info;

    if (device_ctx->hdc_desc.connected != 1) {
        slog_drv_info("It has not been connected, accept_sem is needed to up. (device_id=%d; connected=%d)\n",
            log_para->device_id, device_ctx->hdc_desc.connected);
        up(&sess_info->accept_sem);
    }

wait_again:
    if (log_para->timeout == -1) {
        ret = wait_event_interruptible(sess_info->wq, (sess_info->status.counter == (s32)LOG_SESSION_STATUS_READABLE));
    } else if (log_para->timeout > 0) {
        tmp = msecs_to_jiffies((u32)log_para->timeout);
        ret = wait_event_interruptible_timeout(sess_info->wq,
            (sess_info->status.counter == (s32)LOG_SESSION_STATUS_READABLE), tmp);
        if (ret == 0) {
            log_para->ret = (s32)LOG_RET_TIMEOUT;
            return LOG_RET_OK;
        }
    } else {
        ret = -1;
    }

    if (ret >= 0) {
        if (LOG_SESSION_STATUS_READABLE == atomic_cmpxchg(&sess_info->status,
            (s32)LOG_SESSION_STATUS_READABLE, (s32)LOG_SESSION_STATUS_READING)) {
            ret = log_copy_to_user_safe(log_para->dest_buf, sess_info->log_buf, sess_info->buf_size);
            if (ret != 0) {
                slog_drv_err("Copy_to_user_safe failed. (device_id=%d; buf_size=%u; ret=%d)\n",
                    log_para->device_id, sess_info->buf_size, ret);
                log_para->ret = (s32)LOG_RET_ERROR;
                return LOG_RET_ERROR;
            }
            log_para->write_size = sess_info->buf_size;
            wmb();
            atomic_set(&sess_info->status, (s32)LOG_SESSION_STATUS_INIT);
            wake_up(&sess_info->wq);
        } else {
            slog_drv_warn("Wait for reply again. (device_id=%d; ret=%d)\n", log_para->device_id, ret);
            goto wait_again;
        }
    } else {
        slog_drv_warn("Show details about log_para. (device_id=%d; timeout=%dms; ret=%d)\n", log_para->device_id,
            log_para->timeout, ret);
    }

    log_para->ret = ret;

    return ret;
}

STATIC s32 log_user_read(struct log_device_ctx *device_ctx, struct log_ioctl_para *log_para)
{
    s32 ret;

    if (log_para->dest_buf == NULL) {
        slog_drv_err("The member dest_buf in log_para is NULL. (device_id=%d)\n", log_para->device_id);
        log_para->ret = (s32)LOG_RET_ERROR;
        return LOG_RET_ERROR;
    }

    if (log_para->buf_size < device_ctx->hdc_desc.segment_size) {
        slog_drv_err("Buf_size is invalid. (device_id=%d; buf_size=%u)\n",
            log_para->device_id, log_para->buf_size);
        log_para->ret = (s32)LOG_RET_ERROR;
        return LOG_RET_ERROR;
    }

    ret = log_local_mode_read(device_ctx, log_para);

    return ret;
}

STATIC void log_dev_session_init(s32 dev_id)
{
    struct session_ioctl_info *sess_info = NULL;
    struct log_device_ctx *device_ctx = log_desc.device_ctx[dev_id];

    device_ctx->hdc_desc.ops = &log_desc.ops;

    sess_info = &device_ctx->hdc_desc.sess_info;

    sess_info->log_buf = NULL;
    sess_info->buf_size = 0;
    atomic_set(&sess_info->status, (s32)LOG_SESSION_STATUS_INIT);
    init_waitqueue_head(&sess_info->wq);

    mutex_init(&sess_info->sess_mutex);
    sema_init(&sess_info->accept_sem, 0);

    return;
}

STATIC s32 log_session_init(void)
{
    slog_drv_info("Log_session is local.\n");
    log_desc.session_type = LOG_SESSION_LOCAL;

    log_desc.ops.sess_create_func = log_sess_server_create;
    log_desc.ops.sess_destroy_func = log_sess_server_destroy;
    log_desc.ops.sess_accept_func = log_sess_accept;
    log_desc.ops.sess_close_func = log_sess_close;
    log_desc.ops.sess_send_timeout_func = log_sess_send_timeout;
    log_desc.ops.sess_get_segment_func = log_sess_get_segment;
    log_desc.ops.sess_get_run_env_func = log_sess_get_run_env;
    log_desc.ops.sess_epoll_alloc_fd = log_sess_epoll_alloc_fd;
    log_desc.ops.sess_epoll_free_fd = log_sess_epoll_free_fd;
    log_desc.ops.sess_epoll_ctl = log_sess_epoll_ctl;
    log_desc.ops.sess_epoll_wait = log_sess_epoll_wait;

    return LOG_RET_OK;
}

static s32 log_device_id_check(s32 device_id)
{
    if ((device_id < 0) || (device_id >= log_desc.device_num)) {
        slog_drv_err("Device_id is invalid. (device_id=%d)\n", device_id);
        return LOG_RET_ERROR;
    }
    return LOG_RET_OK;
}

STATIC s32 log_drv_ioctrl_distribute_cmd(u32 drv_cmd,
    struct log_device_ctx *device_ctx, struct log_ioctl_para *log_para)
{
    s32 ret = 0;
    switch (drv_cmd) {
        case LOG_USER_MSG_SET_LEVEL:
            ret = log_user_set_level(device_ctx, log_para);
            break;
        case LOG_USER_MSG_GET_CHANNEL_TYPE:
            ret = log_user_get_channel_type(log_para);
            break;
        case LOG_USER_MSG_GET_DEVICE_ID:
            ret = log_user_get_device_id(log_para);
            break;
        case LOG_USER_MSG_GET_DATA:
            ret = log_user_read(device_ctx, log_para);
            break;
        default:
            slog_drv_err("Log ioctl cmd is invalid. (device_id=%d; cmd=%d)\n", log_para->device_id, drv_cmd);
            return LOG_RET_ERROR;
    }
    return ret;
}

STATIC ssize_t log_drv_ioctl(struct file *file, u32 cmd, uintptr_t arg)
{
    struct log_ioctl_para log_para;
    struct log_device_ctx *device_ctx = NULL;
    s32 device_id;
    u32 drv_cmd;
    void __user *parg = NULL;
    s32 ret;

    drv_cmd = _IOC_NR(cmd);
    if ((drv_cmd >= (u32)LOG_USER_MSG_MAX) || ((drv_cmd > (u32)LOG_USER_MSG_SET_LEVEL) &&
        (drv_cmd < (u32)LOG_USER_MSG_GET_CHANNEL_TYPE))) {
        slog_drv_err("Log ioctl cmd is invalid. (cmd=%u)\n", drv_cmd);
        return (ssize_t)LOG_RET_ERROR;
    }

    (void)memset_s(&log_para, sizeof(log_para), 0, sizeof(struct log_ioctl_para));
    parg = (void __user *)(uintptr_t)arg;
    if (log_copy_from_user_safe(&log_para, parg, sizeof(struct log_ioctl_para)) != 0) {
        slog_drv_err("Copy_from_user failed.\n");
        return (ssize_t)LOG_RET_ERROR;
    }

    device_id = log_para.device_id;
    if (log_device_id_check(device_id) != 0) {
        slog_drv_err("Log_device_id_check failed. (device_id=%d)\n", device_id);
        return (ssize_t)LOG_RET_ERROR;
    }

    device_ctx = log_desc.device_ctx[device_id];

    if (device_ctx == NULL) {
        slog_drv_err("Device_ctx is NULL. (device_id=%d)\n", device_id);
        return (ssize_t)LOG_RET_ERROR;
    }

    ret = log_drv_ioctrl_distribute_cmd(drv_cmd, device_ctx, &log_para);

    if (log_copy_to_user_safe((void __user *)(uintptr_t)arg, (void *)&log_para,
                          sizeof(struct log_ioctl_para)) != 0) {
        slog_drv_err("Copy_to_user_safe failed. (device_id=%d)\n", device_id);
        return (ssize_t)LOG_RET_ERROR;
    }
    return (ssize_t)ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
STATIC void log_calc_generate_rate(struct timer_list *arg)
#else
void log_calc_generate_rate(size_t arg)
#endif
{
    s32 total_volume;
    s32 byte_per_sec;
    struct log_generate_rate *generate_rate = NULL;

    if ((void *)(uintptr_t)arg == NULL) {
        slog_drv_err("Timer callback input variable arg is NULL.\n");
        return;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    generate_rate = from_timer(generate_rate, arg, rate_timer);
#else
    generate_rate = (struct log_generate_rate *)arg;
#endif

    total_volume = atomic_read(&generate_rate->total_volume);
    byte_per_sec = total_volume / generate_rate->timer_interval;
    atomic_set(&generate_rate->byte_per_sec, byte_per_sec);

    atomic_set(&generate_rate->total_volume, 0);
    generate_rate->rate_timer.expires = ((size_t)jiffies +
        (((size_t)(u32)generate_rate->timer_interval) * (size_t)LOG_CALC_GENERATE_RATE_UNIT));
    add_timer(&generate_rate->rate_timer);
}

STATIC s32 log_msg_buf_init(struct log_msg_buf *msg_buf, s32 msg_type, s32 buf_size, s32 threshold)
{
    // msg_buf and buf_size is insure valid by invoker.
    msg_buf->buf_addr = kzalloc((size_t)(u32)buf_size, GFP_KERNEL);
    if (msg_buf->buf_addr == NULL) {
        slog_drv_err("Kzalloc msg_buf->buf_addr failed. (size=%d)\n", buf_size);
        return LOG_RET_ERROR;
    }
    msg_buf->msg_type = msg_type;
    msg_buf->buf_size = buf_size;
    msg_buf->head_addr = msg_buf->buf_addr;
    msg_buf->head_size = (s32)sizeof(struct log_msg_head);
    msg_buf->data_addr = msg_buf->head_addr + msg_buf->head_size;
    msg_buf->data_size = 0;
    msg_buf->data_threshold = threshold;

    return LOG_RET_OK;
}

STATIC struct log_device_ctx *log_create_device_ctx(s32 device_id)
{
    struct log_device_ctx *device_ctx = NULL;
    struct log_cqsq_info *cqsq_info = NULL;
    u32 ts_num;
    s32 ret;
    u32 i, j;

    device_ctx = (struct log_device_ctx *)kzalloc(sizeof(struct log_device_ctx), GFP_KERNEL);
    if (device_ctx == NULL) {
        slog_drv_err("Kzalloc memory for device_ctx failed. (device_id=%d)\n", device_id);
        return NULL;
    }

    atomic_set(&device_ctx->status, (s32)LOG_DEVICE_STATUS_INIT);
    atomic_set(&device_ctx->compress_config, 1);
    mutex_init(&device_ctx->channel_buf_mutex);
    init_waitqueue_head(&device_ctx->wq_slog);
    init_waitqueue_head(&device_ctx->wq_channel_log);

    device_ctx->device_info.cmd_verify = 1;
    device_ctx->device_info.device_id = device_id;
    ts_num = log_desc.ts_num[device_id];
    for (i = 0; i < ts_num; i++) {
        device_ctx->device_info.device_state[i] = DEV_UNUSED;
        cqsq_info = &(device_ctx->device_info.cqsq_info[i]);
        for (j = 0; j < LOG_SEND_TS_CMD_TYPE_NUM; j++) {
            sema_init(&cqsq_info->cq0_wait_sema[j], 0);
        }
        mutex_init(&cqsq_info->cq0_mutex);
        mutex_init(&cqsq_info->cq1_mutex);
    }

    device_ctx->channels_info = (struct log_channel_info *)kzalloc(sizeof(struct log_channel_info) *
                                                                   LOG_CHANNEL_NUM_MAX, GFP_KERNEL);
    if (device_ctx->channels_info == NULL) {
        slog_drv_err("Kzalloc memory for channels_info failed. (device_id=%d)\n", device_id);
        log_destroy_device_ctx(device_ctx);
        return NULL;
    }

    for (i = 0; i < LOG_CHANNEL_NUM_MAX; i++)
        mutex_init(&device_ctx->channels_info[i].cmd_mutex);

    mutex_init(&device_ctx->hdc_desc.ops_mutex);
    device_ctx->hdc_desc.segment_size = 0;
    device_ctx->hdc_desc.service_type = LOG_HDCDRV_SERVICE_TYPE_LOG;
    device_ctx->hdc_desc.connected = 0;
    device_ctx->hdc_desc.host_closed = 0;
    device_ctx->hdc_desc.session = LOG_HDC_INVALID_SESSION;

    ret = log_msg_buf_init(&device_ctx->channel_log_buf, LOG_MSG_TYPE_DLOG, LOG_COLLECT_BUF_SIZE,
                           (s32)LOG_COLLECT_BUF_THRESHOLD_SIZE);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Channel_log_buf init failed. (device_id=%d; ret=%d)\n", device_id, ret);
        log_destroy_device_ctx(device_ctx);
        return NULL;
    }

    ret = log_msg_buf_init(&device_ctx->channel_compress_buf, LOG_MSG_TYPE_DLOG, LOG_SEGMENT_SIZE_MAX, 0);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Channel_compress_buf init failed. (device_id=%d)\n", device_id);
        log_destroy_device_ctx(device_ctx);
        return NULL;
    }

#if defined(CFG_SOC_PLATFORM_CLOUD) || defined(CFG_SOC_PLATFORM_CLOUD_V2)
    // Allocate transform for zlib-deflate
    device_ctx->compress_desc.alg_name = LOG_COMPRESS_HW_ALG_NAME;
    device_ctx->compress_desc.type = CRYPTO_ALG_TYPE_COMPRESS;
    device_ctx->compress_desc.mask = CRYPTO_ALG_TYPE_MASK;
    device_ctx->compress_desc.tfm = crypto_alloc_comp(device_ctx->compress_desc.alg_name,
                                                      device_ctx->compress_desc.type, device_ctx->compress_desc.mask);
    if (IS_ERR(device_ctx->compress_desc.tfm)) {
        slog_drv_warn("Doesn't support log compression: tfm is invalid. (device_id=%d)\n", device_id);
    }
#else
    device_ctx->compress_desc.tfm = ERR_PTR(-EINVAL);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    timer_setup(&device_ctx->generate_rate.rate_timer, log_calc_generate_rate, 0);
#else
    setup_timer(&device_ctx->generate_rate.rate_timer, log_calc_generate_rate,
                (size_t)&device_ctx->generate_rate);
#endif
    device_ctx->generate_rate.timer_interval = LOG_CALC_GENERATE_RATE_COUNT;
    device_ctx->generate_rate.rate_timer.expires = (size_t)((size_t)jiffies +
        (((size_t)(u32)device_ctx->generate_rate.timer_interval) *
        (size_t)LOG_CALC_GENERATE_RATE_UNIT));
    add_timer(&device_ctx->generate_rate.rate_timer);
    atomic_set(&device_ctx->generate_rate.total_volume, 0);
    atomic_set(&device_ctx->generate_rate.byte_per_sec, 0);

    slog_drv_info("Device_ctx succeeded. (device_id=%d)\n", device_id);

    return device_ctx;
}

STATIC void log_destroy_device_ctx(struct log_device_ctx *device_ctx)
{
    if (device_ctx == NULL) {
        slog_drv_err("Device_ctx is NULL, log_destroy_device_ctx failed.\n");
        return;
    }

    (void)del_timer_sync(&device_ctx->generate_rate.rate_timer);

    if (!IS_ERR(device_ctx->compress_desc.tfm)) {
        crypto_free_comp(device_ctx->compress_desc.tfm);
        device_ctx->compress_desc.tfm = NULL;
        slog_drv_info("Free device_ctx->compress_desc.tfm succeeded.\n");
    }

    if (device_ctx->channel_compress_buf.buf_addr != NULL) {
        kfree((void *)device_ctx->channel_compress_buf.buf_addr);
        device_ctx->channel_compress_buf.buf_addr = NULL;
        slog_drv_info("Free device_ctx->channel_compress_buf.buf_addr succeeded.\n");
    }

    if (device_ctx->channel_log_buf.buf_addr != NULL) {
        kfree((void *)device_ctx->channel_log_buf.buf_addr);
        device_ctx->channel_log_buf.buf_addr = NULL;
        slog_drv_info("Free device_ctx->channel_log_buf.buf_addr succeeded.\n");
    }

    if (device_ctx->channels_info != NULL) {
        kfree((void *)device_ctx->channels_info);
        device_ctx->channels_info = NULL;
        slog_drv_info("Free device_ctx->channels_info succeeded.\n");
    }

    kfree((void *)device_ctx);
    slog_drv_info("Log_destroy_device_ctx succeeded.\n");

    return;
}

STATIC const struct file_operations log_drv_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = log_drv_ioctl,
};

STATIC s32 log_drv_register_cdev(void)
{
    s32 ret;
    u32 major_dev;
    dev_t devno;
    struct log_drv_desc *priv = (struct log_drv_desc *)&log_desc;

    priv->drv_devno = 0;
    ret = alloc_chrdev_region(&priv->drv_devno, 0, 1, CHAR_DRIVER_NAME);
    if (ret < 0) {
        return LOG_RET_ERROR;
    }

    /* init and add s8 device */
    major_dev = MAJOR(priv->drv_devno);
    devno = MKDEV(major_dev, 0);
    cdev_init(&priv->drv_cdev, &log_drv_fops);
    priv->drv_cdev.owner = THIS_MODULE;
    priv->drv_cdev.ops = &log_drv_fops;

    if (cdev_add(&priv->drv_cdev, devno, 1) != 0) {
        unregister_chrdev_region(devno, 1);
        return LOG_RET_ERROR;
    }

    priv->drv_class = class_create(THIS_MODULE, CHAR_DRIVER_NAME);
    if (IS_ERR(priv->drv_class)) {
        unregister_chrdev_region(devno, 1);
        cdev_del(&priv->drv_cdev);
        return LOG_RET_ERROR;
    }

    priv->drv_device = device_create(priv->drv_class, NULL, devno, NULL,
                                     CHAR_DRIVER_NAME);
    if (IS_ERR(priv->drv_device)) {
        slog_drv_err("Device_create failed. (devno=%d)\n", devno);
        class_destroy(priv->drv_class);
        unregister_chrdev_region(devno, 1);
        cdev_del(&priv->drv_cdev);
        return LOG_RET_ERROR;
    }

    return LOG_RET_OK;
}

STATIC void log_drv_free_cdev(void)
{
    struct log_drv_desc *priv = &log_desc;

    device_destroy(priv->drv_class, priv->drv_devno);
    class_destroy(priv->drv_class);
    unregister_chrdev_region(priv->drv_devno, 1);
    cdev_del(&priv->drv_cdev);
}

STATIC s32 log_create_hdc_server(struct log_device_ctx *device_ctx)
{
    s32 cnt = 0;
    s32 device_id;
    s32 ret = LOG_RET_ERROR;

    device_id = device_ctx->device_info.device_id;
    if ((device_id < 0) || (device_id >= LOG_DEVICE_ID_MAX)) {
        slog_drv_err("Device_id is invalid. (device_id=%d)\n", device_id);
        return LOG_RET_ERROR;
    }

    while (cnt <= LOG_WAIT_HDC_NUM) {
        ret = (s32)log_desc.ops.sess_create_func();
        if (ret == (s32)LOG_RET_OK) {
            break;
        }
        cnt++;
        msleep(200); /* 200ms */
    }

    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("HDC is not ready to work. (device_id=%d; times=%d; ret=%d)\n", device_id,
            cnt, ret);
        return LOG_RET_ERROR;
    }

    device_ctx->hdc_desc.segment_size = log_desc.ops.sess_get_segment_func();
    slog_drv_info("Show details about hdc_desc. (device_id=%d; segment_size=%d)\n",
        device_id, device_ctx->hdc_desc.segment_size);
    if (device_ctx->hdc_desc.segment_size <= (s32)sizeof(struct log_msg_head)) {
        slog_drv_err("The member segment_size in hdc_desc is invalid. (device_id=%d; segment_size=%d)\n",
            device_id, device_ctx->hdc_desc.segment_size);
        return LOG_RET_ERROR;
    }
    if (device_ctx->hdc_desc.segment_size > LOG_SEGMENT_SIZE_MAX) {
        slog_drv_info("The member segment_size in hdc_desc is out of range. "
            "(device_id=%d; segment_size=%d; segment_size_max=%d)\n",
            device_id, device_ctx->hdc_desc.segment_size, LOG_SEGMENT_SIZE_MAX);
        device_ctx->hdc_desc.segment_size = LOG_SEGMENT_SIZE_MAX;
    }

    slog_drv_info("Hdc serve create succeeded. (device_id=%d)\n", device_id);
    return LOG_RET_OK;
}

STATIC s32 log_drv_wait_ts(const struct log_device_ctx *device_ctx)
{
    bool work_flag = false;
    s32 device_id;
    s32 cnt = 0;

#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (!log_is_ts_valid()) {
        return LOG_RET_OK;
    }
#endif

    device_id = device_ctx->device_info.device_id;
    if ((device_id < 0) || (device_id >= LOG_DEVICE_ID_MAX)) {
        slog_drv_err("Device_id is invalid. (device_id=%d)\n", device_id);
        return LOG_RET_ERROR;
    }

    while ((work_flag == false) && (cnt <= LOG_WAIT_TS_NUM)) {
        work_flag = tsdrv_is_ts_work((u32)device_id, 0);
        if (work_flag == true) {
            break;
        }

        cnt++;
        msleep(200);
    }

    if (work_flag == true) {
        slog_drv_info("Wait for TS several times. (device_id=%d; times=%d)\n", device_id, cnt);
        return LOG_RET_OK;
    } else {
        slog_drv_err("TS is not working. (device_id=%d; times=%d)\n", device_id, cnt);
        return LOG_RET_ERROR;
    }
}

s32 log_get_ts_num(struct devdrv_info *dev_info)
{
    s32 tsnum;
#if (defined CFG_SOC_PLATFORM_MDC_V51)
    tsnum = MDC_TSNUM;
#elif (defined CFG_SOC_PLATFORM_MINIV2) || (defined CFG_SOC_PLATFORM_MINIV3) || \
    (defined CFG_SOC_PLATFORM_CLOUD) || (defined CFG_SOC_PLATFORM_CLOUD_V2)
    tsnum = (s32)devdrv_manager_get_ts_num(dev_info);
#else
    tsnum = 1;
#endif
    return tsnum;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC void log_stop_ts_threads(void)
{
    s32 i;
    s32 device_id;
    struct log_device_ctx *device_ctx = NULL;

    slog_drv_info("Stop log xpc open threads.\n");
    for (i = 0; i < log_desc.device_num; i++) {
        device_id = log_desc.device_ids[i];
        device_ctx = log_desc.device_ctx[device_id];
        if (device_ctx == NULL) {
            slog_drv_err("Device_ctx is NULL. (device_id=%d)\n", device_id);
            continue;
        }

        if (!IS_ERR_OR_NULL(log_desc.task_ts_work[device_id])) {
            (void)kthread_stop(log_desc.task_ts_work[device_id]);
        }
        log_desc.task_ts_work[device_id] = NULL;
    }
}

STATIC void log_stop_xpc_threads(void)
{
    s32 i;
    s32 device_id;
    struct log_device_ctx *device_ctx = NULL;

    slog_drv_info("Stop log xpc threads.\n");
    for (i = 0; i < log_desc.device_num; i++) {
        device_id = log_desc.device_ids[i];
        device_ctx = log_desc.device_ctx[device_id];
        if (device_ctx == NULL) {
            slog_drv_err("Device_ctx is NULL. (device_id=%d)\n", device_id);
            continue;
        }

        if (!IS_ERR_OR_NULL(log_desc.task_xpc_work[device_id])) {
            (void)kthread_stop(log_desc.task_xpc_work[device_id]);
        }
        log_desc.task_xpc_work[device_id] = NULL;
    }
}
#endif

STATIC void log_device_ctx_sess_destroy(struct log_device_ctx *device_ctx,
    log_server_destroy sess_destroy_func)
{
    s32 ret;

    if ((atomic_read(&device_ctx->status) != (s32)LOG_DEVICE_STATUS_INIT)) {
        atomic_set(&device_ctx->status, (s32)LOG_DEVICE_STATUS_INIT);
        // close hdc accept. if not, thread would be blocked.
        mutex_lock(&device_ctx->hdc_desc.ops_mutex);
        if ((device_ctx->hdc_desc.created == 1) && (device_ctx->hdc_desc.connected == 0)) {
            ret = sess_destroy_func();
            if (ret == 0) {
                device_ctx->hdc_desc.created = 0;
                slog_drv_info("Destroy hdc instance for interrupt accepting.\n");
            } else {
                slog_drv_err("Destroy hdc instance failed. (ret=%d)\n", ret);
            }
        }
        mutex_unlock(&device_ctx->hdc_desc.ops_mutex);
    }
}

STATIC void log_stop_work_threads(u32 device_id)
{
    struct log_device_ctx *device_ctx = NULL;

    device_ctx = log_desc.device_ctx[device_id];
    if (device_ctx == NULL) {
        slog_drv_err("Device_ctx is NULL. (device_id=%d)\n", device_id);
        return;
    }

    log_device_ctx_sess_destroy(device_ctx, log_desc.ops.sess_destroy_func);

    if (!IS_ERR(log_desc.task_log_work[device_id])) {
        (void)kthread_stop(log_desc.task_log_work[device_id]);
    }
    log_desc.task_log_work[device_id] = NULL;
    log_desc.device_ctx[device_id] = NULL;
}

#ifndef LOG_UT
static int log_get_affinity_cpuid(u32 dev_id)
{
#if (!defined CFG_SOC_PLATFORM_CLOUD) && (!defined CFG_SOC_PLATFORM_CLOUD_v2)
    return -1;
#else
    u32 node_per_cpu;
    u32 devnum;

    devnum = (u32)cpu_to_node(num_online_cpus() - 1) + 1;
    node_per_cpu = num_online_cpus() / devnum;
    slog_drv_info("Bind to cpu_id. (device_id=%d; cpu_id=%d) \n", dev_id, node_per_cpu * dev_id);

    return node_per_cpu * dev_id;
#endif
}

#ifndef CFG_SOC_PLATFORM_MDC_V51
STATIC void log_work_thread_bind_core(s32 i, struct task_struct *thread, s32 device_id)
{
    log_desc.cpu_ids[i] = log_get_affinity_cpuid((u32)device_id);
    if (log_desc.cpu_ids[i] != -1) {
        kthread_bind(thread, (u32)log_desc.cpu_ids[i]);
        slog_drv_info("Kthread_bind device_id to cpu_id. (device_id=%d; cpu_id=%d) \n", device_id,
            log_desc.cpu_ids[i]);
    }
}
#else
STATIC void log_work_thread_bind_core(s32 i, struct task_struct *thread, s32 device_id)
{
    cpumask_t ctrlcpu = {};
    drv_get_ctrlcpu_mask(&ctrlcpu);
    if (cpumask_empty(&ctrlcpu)) {
        slog_drv_warn("ctrl cpu mask is empty\n");
        return;
    }

    log_desc.cpu_ids[i] = log_get_affinity_cpuid((u32)device_id);
    if (cpumask_available(&ctrlcpu)) {
        set_cpus_allowed_ptr(thread, &ctrlcpu);
    }
}
#endif
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC void log_init_user_level(void)
{
    int i;

    for (i = 0; i < LOG_CHANNEL_TYPE_MAX; i++) {
        g_log_user_level[i] = (s32)LOG_CHANNEL_LEVEL_INFO;
    }

    return;
}

STATIC void log_create_xpc_work_thread(void)
{
    s32 device_id;
    s32 i;

    for (i = 0; i < log_desc.device_num; i++) {
        device_id = log_desc.device_ids[i];
        log_desc.task_xpc_work[device_id] = kthread_create(log_xpc_work_thread, (void *)&log_desc.device_ids[i],
                                                           "drvlog_xpc_%d", device_id);
        if (IS_ERR(log_desc.task_xpc_work[device_id])) {
            slog_drv_err("Kthread_run xpc failed. (device_id=%d)\n", device_id);
            goto undo_kthread_xpc_run;
        }
#ifndef LOG_UT
        log_work_thread_bind_core(i, log_desc.task_xpc_work[device_id], device_id);
#endif
        (void)wake_up_process(log_desc.task_xpc_work[device_id]);
    }

    return;

undo_kthread_xpc_run:
    log_stop_xpc_threads();
}

STATIC s32 log_ts_work_thread(void *arg)
{
    s32 device_id;
    s32 ret;
    struct log_device_ctx *device_ctx = NULL;

    device_id = *(s32 *)(arg);

    if (!log_is_ts_valid()) {
        log_xpc_open_all_channel();
        log_create_xpc_work_thread();
    }

    device_ctx = log_desc.device_ctx[device_id];
    if (device_ctx == NULL) {
        slog_drv_err("Log device_ctx is NULL. (device_id=%d)\n", device_id);
        goto undo_ts_work_thread;
    }

    ret = log_drv_wait_ts(device_ctx);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Log_drv_wait_ts failed, please check TS's status. (device_id=%d)\n", device_id);
        goto undo_ts_work_thread;
    }

    ret = log_create_all_channels(device_ctx, LOG_CREATE_TS_CHANNELS);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Log_create_ts_channels failed. (device_id=%d)\n", device_id);
        goto undo_ts_work_thread;
    }

    g_log_ts_thread_stop = true;
    log_desc.task_ts_work[device_id] = NULL;
    return (s32)LOG_RET_OK;

undo_ts_work_thread:
    g_log_ts_thread_stop = true;
    log_desc.task_ts_work[device_id] = NULL;
    return (s32)LOG_RET_ERROR;
}

STATIC void log_launch_ts_threads(void)
{
    s32 device_id;
    s32 i;
#ifdef CFG_SOC_MDC_V51_LITE_FPGA
    if (!log_is_ts_valid()) {
        return;
    }
#endif
    for (i = 0; i < log_desc.device_num; i++) {
        device_id = log_desc.device_ids[i];
        log_desc.task_ts_work[device_id] = kthread_create(log_ts_work_thread, (void *)&log_desc.device_ids[i],
                                                          "drvlog_ts_%d", device_id);
        if (IS_ERR(log_desc.task_ts_work[device_id])) {
            slog_drv_err("Kthread_run ts open failed. (device_id=%d)\n", device_id);
            goto undo_kthread_xpc_open_run;
        }
#ifndef LOG_UT
        log_work_thread_bind_core(i, log_desc.task_ts_work[device_id], device_id);
#endif
        (void)wake_up_process(log_desc.task_ts_work[device_id]);
    }

    return;

undo_kthread_xpc_open_run:
    log_stop_ts_threads();
}
#endif

STATIC s32 log_launch_work_threads(u32 device_id)
{
    s32 ret;

    ret = soc_resmng_subsys_get_num(device_id, TS_SUBSYS, &log_desc.ts_num[device_id]);
    if (ret != 0) {
        slog_drv_err("Get ts num failed. (device_id=%d)\n", device_id);
        return ret;
    }

    log_desc.device_ids[device_id] = device_id;
    log_desc.task_log_work[device_id] = kthread_create(log_work_thread, (void *)&log_desc.device_ids[device_id],
                                                       "drvlog_work_%d", device_id);
    if (IS_ERR(log_desc.task_log_work[device_id])) {
        slog_drv_err("Kthread_run failed. (device_id=%d)\n", device_id);
        return (s32)LOG_RET_ERROR;
    }
#ifndef LOG_UT
    log_work_thread_bind_core(device_id, log_desc.task_log_work[device_id], device_id);
#endif
    (void)wake_up_process(log_desc.task_log_work[device_id]);

    return LOG_RET_OK;
}

STATIC s32 log_drv_suspend(void)
{
    log_desc.suspend = LOG_SUSPEND;
    return 0;
}

STATIC s32 log_drv_resume(void)
{
    log_desc.suspend = 0;
    return 0;
}

STATIC s32 log_drv_is_suspend(void)
{
    return (log_desc.suspend == LOG_SUSPEND);
}

#define LOG_NOTIFIER "log"
STATIC int log_notifier_func(u32 udevid, enum uda_notified_action action)
{
    int ret = 0;

    if (udevid >= LOG_DEVICE_ID_MAX) {
        slog_drv_err("Invalid para. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    if (action == UDA_INIT) {
        log_desc.device_num++;
        ret = log_launch_work_threads(udevid);
    } else if (action == UDA_UNINIT) {
        log_stop_work_threads(udevid);
    } else if (action == UDA_SUSPEND) {
        ret = log_drv_suspend();
    } else if (action == UDA_RESUME) {
        ret = log_drv_resume();
    }

    return ret;
}

STATIC s32 __init log_drv_module_init(void)
{
    struct uda_dev_type type;
    s32 ret;

    slog_drv_info("Drv_log begin to init.\n");

    ret = memset_s((void *)&log_desc, sizeof(struct log_drv_desc), 0, sizeof(struct log_drv_desc));
    if (ret != EOK) {
        slog_drv_err("Memset_s failed. (ret=%d)\n", ret);
        return LOG_RET_ERROR;
    }

#ifdef CFG_SOC_PLATFORM_MDC_V51
    log_init_user_level();
#endif

    log_init_channel_desc();

    ret = log_drv_register_cdev();
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Create character device failed.\n");
        goto func_end;
    }

    ret = log_session_init();
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Log_session_init failed. (ret=%d)\n", ret);
        goto free_cdev;
    }

    uda_davinci_local_real_entity_type_pack(&type);
    ret = uda_notifier_register(LOG_NOTIFIER, &type, UDA_PRI2, log_notifier_func);
    if (ret != 0) {
        slog_drv_err("Register real notifier failed. (ret=%d)\n", ret);
        goto free_cdev;
    }

    return LOG_RET_OK;

free_cdev:
    log_drv_free_cdev();

func_end:
    return ret;
}

STATIC void __exit log_drv_module_exit(void)
{
    struct uda_dev_type type;

#ifdef CFG_SOC_PLATFORM_MDC_V51
    log_stop_xpc_threads();
    log_stop_ts_threads();
    log_xpc_close_all_channel();
#endif
    uda_davinci_local_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(LOG_NOTIFIER, &type);
    log_drv_free_cdev();
}

module_init(log_drv_module_init);

module_exit(log_drv_module_exit);

MODULE_DESCRIPTION("log driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
