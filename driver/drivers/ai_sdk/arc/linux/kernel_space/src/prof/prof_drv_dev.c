/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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
#ifndef PROF_UNIT_TEST

#ifdef CONFIG_GENERIC_BUG
#undef CONFIG_GENERIC_BUG
#endif
#ifdef CONFIG_BUG
#undef CONFIG_BUG
#endif
#ifdef CONFIG_DEBUG_BUGVERBOSE
#undef CONFIG_DEBUG_BUGVERBOSE
#endif
#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/barrier.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/kallsyms.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/securec.h>
#include <linux/types.h>
#include <linux/cpumask.h>
#include <linux/vmalloc.h>
#include <linux/oom.h>
#ifdef AOS_LLVM_BUILD
#include <linux/hrtimer.h>
#endif
#include "drv_log.h"
#include "devdrv_functional_cqsq_api.h"
#include "prof_drv_dev.h"
#include "prof_def.h"
#include "dbl/chip_config.h"
#include "dbl/uda.h"

#ifdef CFG_FEATURE_SRIOV
#include "devdrv_interface.h"
#include "dvprof_init.h"
#endif

#ifdef CFG_FEATURE_HOST_COLLECTION
#include "prof_drv_hdc_dev.h"
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "tsdrv_ts_node.h"
#endif

#ifdef CFG_FEATURE_PROF_HWTS_THROUGH
#include "prof_hwts_init.h"
#endif

typedef struct char_device {
    struct class *dev_class;
    struct cdev cdev;
    dev_t devno;
    struct device *device;
} char_device_t;

#ifndef AOS_LLVM_BUILD
STATIC char_device_t char_dev_prof;
#endif
STATIC struct prof_device_info *prof_devices[PROF_DEVICE_NUM_VALUE] = {NULL};
struct prof_device_resource_info *device_sub_resource_info[PROF_DEVICE_NUM_VALUE] = {NULL};

#ifdef CFG_SOC_PLATFORM_MDC_V51
bool prof_is_ts_valid(void)
{
    int ts_num = devdrv_get_ts_node_num();
    return (ts_num == MDC_TSNUM) ? true : false;
}
STATIC u32 devdrv_manager_get_ts_num_stub(struct devdrv_info *dev_info)
{
    return MDC_TSNUM;
}
#else
STATIC u32 devdrv_manager_get_ts_num_stub(struct devdrv_info *dev_info)
{
    int ret;
#ifndef CFG_SOC_PLATFORM_MINI
    ret = devdrv_manager_get_ts_num(dev_info);
#else
    ret = 1;
#endif
    return ret;
}
#endif


struct prof_device_info *prof_device_info_get(u32 device_id)
{
    return prof_devices[device_id];
}

struct prof_channel_info *prof_get_channel_info_from_index(u32 device_id, u32 vfid,
    u32 channel_id)
{
    return &prof_device_info_get(device_id)->vf_info[vfid].channel_info[channel_id];
}

struct prof_device_resource_info *prof_get_device_resource_info(u32 device_id)
{
    return device_sub_resource_info[device_id];
}

struct prof_channel_resource_info *prof_get_channel_resource_info(u32 device_id,
    u32 channel_id)
{
    struct prof_device_resource_info *dev_res_info = prof_get_device_resource_info(device_id);
    return &(dev_res_info->channel_sub_resource_info[channel_id]);
}

struct prof_sub_channel_info *prof_get_sub_resource_info(u32 dev_id,
    u32 channel_id, u32 sub_channel_id)
{
    struct prof_device_resource_info *dev_res_info = prof_get_device_resource_info(dev_id);
    return &(dev_res_info->channel_sub_resource_info[channel_id].sub_resource_info[sub_channel_id]);
}

int prof_check_device_state(u32 device_id)
{
    struct prof_device_info *prof_device = NULL;

    if (device_id >= PROF_DEVICE_NUM_VALUE) {
        prof_err("Parameter [device_id] was invalid. (device_id=%u)\n", device_id);
        return PROF_ERROR;
    }

    prof_device = prof_device_info_get(device_id);
    if (prof_device == NULL) {
        prof_err("Device not valid. (device_id=%u)\n", device_id);
        return PROF_ERROR;
    }

    if (prof_device->device_state != DEV_USED) {
        prof_err("The device was used. Failed to operate. (device_id=%u; device_state=%u)\n",
            device_id, prof_device->device_state);
        return PROF_ERROR;
    }

    return PROF_OK;
}

STATIC int prof_distribute_cmd(struct prof_proc_ctx *ctx, struct prof_ioctl_para *para);

#define PROF_POLL_DEFAULT_TIMEOUT 5 /* HZ */

STATIC inline bool prof_is_valid_range(int channel_id)
{
    if ((channel_id < 0) || (channel_id >= CHANNEL_IDS_MAX)) {
        prof_err("Parameter [channel_id] was invalid. (channel_id=%d)\n", channel_id);
        return false;
    }

    return true;
}

STATIC inline bool prof_is_valid_virtual_tscpu_hw_flag(int channel_flag)
{
    return (channel_flag == PROF_VALID_TSCPU_HW_VIRTUAL_FLAG);
}

STATIC inline bool prof_is_valid_virtual_tscpu_sw_flag(int channel_flag)
{
    return (channel_flag == PROF_VALID_TSCPU_SW_VIRTUAL_FLAG);
}

STATIC inline bool prof_is_valid_virtual_tscpu_flag(int channel_flag)
{
    return (prof_is_valid_virtual_tscpu_hw_flag(channel_flag) || prof_is_valid_virtual_tscpu_sw_flag(channel_flag));
}

STATIC inline bool prof_is_valid_tscpu_flag(int channel_flag)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (!prof_is_ts_valid()) {
        return false;
    }
#endif
    return ((channel_flag == PROF_VALID_TSCPU_FLAG) || prof_is_valid_virtual_tscpu_flag(channel_flag));
}

STATIC inline bool prof_is_valid_virtual_peri_flag(int channel_flag)
{
    return (channel_flag == PROF_VALID_PERIPHERAL_VIRTUAL_FLAG);
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC inline bool prof_is_valid_peri_flag(int channel_id, int channel_flag)
{
    if (channel_flag == PROF_VALID_PERIPHERAL_FLAG || prof_is_valid_virtual_peri_flag(channel_flag)) {
        return prof_agent_sample_valid(channel_id);
    }
    return false;
}
#else
STATIC inline bool prof_is_valid_peri_flag(int channel_id, int channel_flag)
{
    return (channel_flag == PROF_VALID_PERIPHERAL_FLAG || prof_is_valid_virtual_peri_flag(channel_flag));
}
#endif

STATIC inline bool prof_is_valid_flag(int channel_id, int channel_flag)
{
    return (prof_is_valid_tscpu_flag(channel_flag) || prof_is_valid_peri_flag(channel_id, channel_flag));
}

bool prof_is_valid_virtual_tscpu_hw_channel(int channel_id)
{
    int *prof_hash = NULL;

    if (prof_is_valid_range(channel_id)) {
        prof_hash = prof_get_hash();
        return prof_is_valid_virtual_tscpu_hw_flag(prof_hash[channel_id]);
    }

    return false;
}

bool prof_is_valid_virtual_tscpu_channel(int channel_id)
{
    int *prof_hash = NULL;

    if (prof_is_valid_range(channel_id)) {
        prof_hash = prof_get_hash();
        return prof_is_valid_virtual_tscpu_flag(prof_hash[channel_id]);
    }

    return false;
}

bool prof_is_valid_virtual_peri_channel(int channel_id)
{
    int *prof_hash = NULL;

    if (prof_is_valid_range(channel_id)) {
        prof_hash = prof_get_hash();
        return prof_is_valid_virtual_peri_flag(prof_hash[channel_id]);
    }

    return false;
}

STATIC bool prof_is_valid_tscpu_channel(int channel_id)
{
    int *prof_hash = NULL;

    if (prof_is_valid_range(channel_id)) {
        prof_hash = prof_get_hash();
        return prof_is_valid_tscpu_flag(prof_hash[channel_id]);
    }

    return false;
}

STATIC bool prof_is_valid_peri_channel(int channel_id)
{
    int *prof_hash = NULL;

    if (prof_is_valid_range(channel_id)) {
        prof_hash = prof_get_hash();
        return prof_is_valid_peri_flag(channel_id, prof_hash[channel_id]);
    }

    return false;
}

bool prof_is_valid_channel(int channel_id)
{
    int *prof_hash = NULL;

    if (prof_is_valid_range(channel_id)) {
        prof_hash = prof_get_hash();
        return prof_is_valid_flag(channel_id, prof_hash[channel_id]);
    }

    return false;
}

int prof_check_channel_with_vfid(u32 device_id, u32 channel_id, u32 vfid)
{
#ifdef CFG_FEATURE_SRIOV
    u32 phy_device_id, tmp_vfid;

    (void)vmngd_get_pfvf_id_by_devid(device_id, &phy_device_id, &tmp_vfid);
    vfid = tmp_vfid;
#endif

    if (vfid == PROF_PHYSICAL_MACHINE_VFID) {
        if (prof_is_valid_channel(channel_id)) {
            return PROF_OK;
        }
    } else {
        if (prof_is_valid_virtual_tscpu_channel(channel_id) ||
            prof_is_valid_virtual_peri_channel(channel_id)) {
            return PROF_OK;
        }
    }

    return PROF_ERROR;
}

STATIC int prof_drv_wait_ts(u32 device_id)
{
    int cnt = 0;
    bool work_flag = false;
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (!prof_is_ts_valid()) {
        return PROF_OK;
    }
#endif
    while (cnt <= PROF_WAIT_TS_COUNTS) {
        work_flag = tsdrv_is_ts_work(device_id, 0);
        if (work_flag == true)
            break;

        cnt++;
        msleep(PROF_DELAY_MS);
    }
    if (work_flag == true) {
        prof_info("TS was working. Profile driver was waiting for the response of TS. (device_id=%u; cnt=%d)\n",
            device_id, cnt);
        return PROF_OK;
    }

    prof_err("TS was not working. Failed to operate. (device_id=%u; cnt=%d)\n", device_id, cnt);
    return PROF_ERROR;
}

#ifdef __aarch64__
void flush_cache(unsigned char *base, u32 len)
{
    int i;
    int num = len / CACHE_LINE_LEN;

    if ((len % CACHE_LINE_LEN) > 0) {
        num++;
    }
    base = (unsigned char *)(uintptr_t)(((unsigned long)(uintptr_t)base) & (~(CACHE_LINE_LEN - 1)));

    asm volatile("dsb st" : : : "memory");
    for (i = 0; i < num; i++) {
        asm volatile("DC CIVAC ,%x0" ::"r"(base + i * CACHE_LINE_LEN));
        mb();
    }

    asm volatile("dsb st" : : : "memory");
}

void invalidate_cache(unsigned char *base, u32 len)
{
    int i;
    int num = len / CACHE_LINE_LEN;

    if ((len % CACHE_LINE_LEN) > 0) {
        num++;
    }
    base = (unsigned char *)(uintptr_t)(((unsigned long)(uintptr_t)base) & (~(CACHE_LINE_LEN - 1)));

    asm volatile("dsb st" : : : "memory");
    for (i = 0; i < num; i++) {
        asm volatile("DC IVAC ,%x0" ::"r"(base + i * CACHE_LINE_LEN));
        mb();
    }

    asm volatile("dsb st" : : : "memory");
}
#endif

STATIC int copy_to_user_secure(void __user *to, const void *from, unsigned long n)
{
    if ((to == NULL) || (n == 0)) {
        prof_err("Parameter [to] was [NULL] or [n] was zero(0)."
            " ((to == NULL)=%d; n=%lu)\n", (to == NULL), n);
        return -EINVAL;
    }

    if (copy_to_user(to, (void *)from, n) != 0) {
        prof_err("Failed to invoke function [copy_to_user] to send data to user.\n");
        return -ENODEV;
    }

    return 0;
}

#ifndef CFG_SOC_PLATFORM_MDC_V51
/* when set tscpu/ipc irq affinity need adapt for diff os */
int prof_set_irq_affinity(unsigned int irq, const struct cpumask *cpumask)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) && !defined(AOS_LLVM_BUILD)
    if (cpumask != NULL) {
        return irq_force_affinity(irq, cpumask);
    }
    return PROF_ERROR;
#else
    return irq_set_affinity_hint(irq, cpumask);
#endif
}

int prof_clear_irq_affinity(unsigned int irq)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) && !defined(AOS_LLVM_BUILD)
    return 0;
#else
    return irq_set_affinity_hint(irq, NULL);
#endif
}

unsigned int prof_get_affinity_cpuid(u32 dev_id)
{
    unsigned int cpu_id;
    u32 node_per_cpu;
    u32 devnum;

    devnum = (u32)cpu_to_node(num_online_cpus() - 1) + 1;
    node_per_cpu = num_online_cpus() / devnum;
    cpu_id = node_per_cpu * dev_id;

    prof_info("Bind to cpu_id. (dev_id=%u; cpu_id=%u)\n", dev_id, cpu_id);
    return cpu_id;
}

int prof_init_affinity_cpuid_thread(u32 device_id, struct task_struct *thread)
{
#if defined(CFG_FEATURE_KTHREAD_BIND_CORE)
    unsigned int cpu_id;
    cpu_id = prof_get_affinity_cpuid(device_id);

    kthread_bind(thread, cpu_id);
    prof_info("The thread bound the thread with device initialization task successfully."
        " (device_id=%u; cpu_id=%u)\n", device_id, cpu_id);
#endif

    return PROF_OK;
}
#endif

enum prof_channel_type prof_get_channel_type(u32 channel_id)
{
    enum prof_channel_type channel_type = PROF_CHANNEL_TYPE_MAX;
    if (prof_is_valid_tscpu_channel(channel_id)) {
        channel_type = PROF_TS_TYPE;
    } else if (prof_is_valid_peri_channel(channel_id)) {
        channel_type = PROF_PERIPHERAL_TYPE;
    } else {
#ifndef CFG_SOC_PLATFORM_MDC_V51
        prof_debug("Profile detected unknown channel type. (channel_type=%d; channel_id=%u)\n",
            (int)channel_type, channel_id);
#endif
    }

    return channel_type;
}

void prof_channel_info_free(struct prof_sub_channel_info *sub_channel_info, enum prof_channel_type channel_type)
{
    u32 device_id = sub_channel_info->device_id;
    u32 vfid = sub_channel_info->vfid;
    u32 channel_id = sub_channel_info->channel_id;
    u32 sub_channel_id = sub_channel_info->sub_channel_id;
    int poll_fd_num;
    int dfx_flag = 0;

    if (channel_type == PROF_PERIPHERAL_TYPE) {
        if (sub_channel_info->vir_addr != NULL) {
            prof_peripheral_chan_free_mem(sub_channel_info->vir_addr, sub_channel_info->buf_len);
            sub_channel_info->vir_addr = NULL;
        }
        sub_channel_info->peri_channel.sample_thread = NULL;
    } else if (channel_type == PROF_TS_TYPE) {
        if (sub_channel_info->vir_addr != NULL) {
            prof_tscpu_free_channel_memory(sub_channel_info);
        }
    }

    spin_lock_bh(&sub_channel_info->spinlock);
    if (sub_channel_info->proc_ctx != NULL) {
        poll_fd_num = atomic_dec_return(&sub_channel_info->proc_ctx->poll.fd_num);
        dfx_flag = 1;
    }
    spin_unlock_bh(&sub_channel_info->spinlock);

    sub_channel_info->poll_flag = (int)POLL_INVALID;
    sub_channel_info->channel_state = (u32)PROF_CHANNEL_DISABLE;

    if (dfx_flag == 1) {
        prof_info("Profile received the channel information."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u; poll_fd_num=%d)\n",
            device_id, vfid, channel_id, sub_channel_id, poll_fd_num);
    }
}

void prof_hdc_session_channel_free(u32 device_id, u32 vfid,
    u32 channel_id, struct prof_proc_ctx *proc_ctx)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;

    sub_channel_info = prof_get_sub_channel_info_from_ctx(proc_ctx, device_id, vfid, channel_id);
    if (sub_channel_info == NULL) {
        return;
    }

    mutex_lock(&sub_channel_info->state_mutex);
    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_DISABLE) {
        mutex_unlock(&sub_channel_info->state_mutex);
        prof_free_sub_channel_resource(sub_channel_info);
        return;
    }
    prof_channel_info_free(sub_channel_info, prof_get_channel_type(channel_id));
    mutex_unlock(&sub_channel_info->state_mutex);
    prof_free_sub_channel_resource(sub_channel_info);

    prof_info("HDC session channel uninitialized successfully. (device_id=%u; vfid=%u; channel_id=%u)\n",
        device_id, vfid, channel_id);
}

int prof_wake_up_poll_wq(struct prof_sub_channel_info *sub_channel_info)
{
    struct prof_poll_info_kernel *prof_poll_mgr = NULL;

    if (sub_channel_info->poll_flag != (int)POLL_INVALID) {
        return PROF_OK;
    }

    spin_lock_bh(&sub_channel_info->spinlock);
    if (sub_channel_info->proc_ctx == NULL) {
        spin_unlock_bh(&sub_channel_info->spinlock);
        return PROF_ERROR;
    }

    spin_lock_bh(&(sub_channel_info->proc_ctx->poll.spinlock));
    prof_poll_mgr = &sub_channel_info->proc_ctx->poll;
    prof_poll_mgr->poll_box[prof_poll_mgr->poll_head].device_id = sub_channel_info->device_id;
    prof_poll_mgr->poll_box[prof_poll_mgr->poll_head].channel_id = sub_channel_info->channel_id;
    prof_poll_mgr->poll_head++;
    prof_poll_mgr->poll_head %= PROF_POLL_DEPTH;
    sub_channel_info->poll_flag = (int)POLL_VALID;
    spin_unlock_bh(&(sub_channel_info->proc_ctx->poll.spinlock));

    wake_up(&prof_poll_mgr->poll_wq);
    spin_unlock_bh(&sub_channel_info->spinlock);

    sub_channel_info->prof_dfx.prof_wake_up_poll_count++;

    return PROF_OK;
}

int prof_stop_wait_buff_read_over(struct prof_sub_channel_info *sub_channel_info, enum prof_channel_type type,
    struct prof_proc_ctx *proc_ctx)
{
    u32 total_data_len;
    int ret;

    if (type == PROF_PERIPHERAL_TYPE) {
        total_data_len = prof_agent_get_data_len(sub_channel_info);
    } else {
        total_data_len = prof_tscpu_get_data_len(sub_channel_info);
    }

    if (total_data_len == 0) {
        return PROF_OK;
    }

    prof_info("Reading data was not complete. Profile should read data completely."
        " (device_id=%u; vfid=%u; channel_id=%u; total_data_len=%u)\n",
        sub_channel_info->device_id, sub_channel_info->vfid,
        sub_channel_info->channel_id, total_data_len);

    sub_channel_info->channel_state = (u32)PROF_CHANNEL_STOPPING;
    (void)prof_wake_up_poll_wq(sub_channel_info);
    mutex_unlock(&sub_channel_info->state_mutex);
    ret = prof_wait_read_last_data(sub_channel_info);
    mutex_lock(&sub_channel_info->state_mutex);
    if (ret == PROF_OK) {
        return prof_stop_channel_status_check(sub_channel_info, proc_ctx);
    }

    sub_channel_info->channel_state = (u32)PROF_CHANNEL_ENABLE;
    return PROF_OK;
}

#ifdef AOS_LLVM_BUILD
unsigned char *prof_alloc_node_memory(u32 device_id, u32 size, u32 flags, enum prof_node_mem_type type)
{
    return NULL;
}
#else
typedef int (*dbl_get_nid_handle)(u32 devid, int nids[], int num);
struct page *prof_alloc_pages_node(u32 device_id, gfp_t flags, u32 order, enum prof_node_mem_type type)
{
    int nids[DBL_NUMA_ID_MAX_NUM] = { 0 };
    dbl_get_nid_handle dbl_get_nid = NULL;
    int node_num, i;

    if (type == CTRL_NODE_MEM_TYPE) {
        dbl_get_nid = dbl_get_ctrl_nid;
    } else if (type == TS_NODE_MEM_TYPE) {
        dbl_get_nid = dbl_get_ts_nid;
    }

    node_num = dbl_get_nid(device_id, nids, DBL_NUMA_ID_MAX_NUM);
    if ((node_num <= 0) || (node_num >= DBL_NUMA_ID_MAX_NUM)) {
        prof_err("Invalid hbm_nor node_num. (device_id=%u; node_num=%d; type=%d)\n", device_id, node_num, (int)type);
        return NULL;
    }
    prof_debug("Do alloc_pages_node. (device_id=%u; order=%u; type=%d)\n", device_id, order, (int)type);

    for (i = 0; i < node_num; i++) {
        struct page *p = alloc_pages_node(nids[i], flags | __GFP_NOWARN, order);
        prof_debug("Do alloc_pages_node. (nids[%d]=%d)\n", i, nids[i]);
        if (p != NULL) {
            return p;
        }
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    (void)oom_type_notifier_call(OOM_TYPE_CGROUP, NULL);
#else
    (void)hisi_oom_notifier_call(HISI_OOM_TYPE_CGROUP, NULL);
#endif

    for (i = 0; i < node_num; i++) {
        prof_err("Do alloc_pages_node failed. (device_id=%u; order=%u; type=%d; nids[%d]=%d)\n",
            device_id, order, (int)type, i, nids[i]);
    }

    return NULL;
}

unsigned char *prof_alloc_node_memory(u32 device_id, u32 size, gfp_t flags, enum prof_node_mem_type type)
{
    struct page *page = NULL;

#ifdef CFG_FEATURE_NO_SUPPORT_NODE_DIVIDE
    struct uda_mia_dev_para mia_para;
    int ret;
    if (!uda_is_phy_dev(device_id)) {
        ret = uda_udevid_to_mia_devid(device_id, &mia_para);
        if (ret != 0) {
            prof_err("Failed to get mia devid. (devid=%u)\n", device_id);
            return NULL;
        }
        device_id = mia_para.phy_devid;
    }
#endif

    page = prof_alloc_pages_node(device_id, flags, (u32)get_order(size), type);
    if (page == NULL) {
        prof_err("Failed to invoke alloc_pages_node. (device_id=%u; size=%u; type=%d)\n", device_id, size, (int)type);
        return NULL;
    }

    return page_to_virt(page);
}
#endif

int prof_alloc_device_info(u32 device_id)
{
    prof_devices[device_id] = (struct prof_device_info *)vzalloc(sizeof(struct prof_device_info));
    if (prof_devices[device_id] == NULL) {
        prof_err("Failed to invoke function [vzalloc] to apply for the memory.\n");
        return PROF_ERROR;
    }

    return PROF_OK;
}

void prof_free_device_info(u32 device_id)
{
    if (prof_devices[device_id] != NULL) {
        vfree((unsigned char *)prof_devices[device_id]);
        prof_devices[device_id] = NULL;
    }
}

STATIC void prof_add_to_sub_resource_info(struct prof_channel_info *channel_info,
    struct prof_proc_ctx *proc_ctx, u32 phy_device_id, u32 sub_channel_id)
{
    struct prof_channel_resource_info *channel_resource_info = NULL;
    struct prof_sub_channel_info *sub_channel_info = NULL;

    channel_resource_info = prof_get_channel_resource_info(phy_device_id, channel_info->channel_id);

    sub_channel_info = &(channel_resource_info->sub_resource_info[sub_channel_id]);
    spin_lock_bh(&sub_channel_info->spinlock);
    sub_channel_info->proc_ctx = proc_ctx;
    spin_unlock_bh(&sub_channel_info->spinlock);
    sub_channel_info->device_id = channel_info->device_id;
    sub_channel_info->vfid = channel_info->vfid;
    channel_info->sub_channel_info[sub_channel_id] = sub_channel_info;
    channel_info->channel_used_count++;

    if (prof_get_channel_type(sub_channel_id) == PROF_TS_TYPE) {
        sub_channel_info->ts_channel.ts_buff_free_flag = TS_BUFF_FREE;
    }
}

STATIC int prof_alloc_sub_resource_check(struct prof_channel_info *channel_info, struct prof_proc_ctx *proc_ctx,
    u32 phy_device_id)
{
    struct prof_device_resource_info *dev_res_info = NULL;
    struct prof_sub_channel_info *sub_channel_info = NULL;
    int i;

    dev_res_info = prof_get_device_resource_info(phy_device_id);
    if (dev_res_info == NULL) {
        prof_err("Failed to get dev res info. (phy_device_id=%u)", phy_device_id);
        return PROF_ERROR;
    }

    for (i = 0; i < PROF_SUB_CHANNEL_NUM_MAX; i++) {
        sub_channel_info = channel_info->sub_channel_info[i];
        if (sub_channel_info == NULL) {
            continue;
        }
        spin_lock_bh(&sub_channel_info->spinlock);
        if (sub_channel_info->proc_ctx == proc_ctx) {
            spin_unlock_bh(&sub_channel_info->spinlock);
            prof_err("Profile CTX had allocated the subordinate channel. Please do not allocate it again."
                " (device_id=%u, vfid=%u, channel_id=%u; sub_channel_id=%u)",
                channel_info->device_id, channel_info->vfid, channel_info->channel_id,
                sub_channel_info->sub_channel_id);
            return PROF_STARTED_ALREADY;
        }
        spin_unlock_bh(&sub_channel_info->spinlock);
    }

    return PROF_OK;
}

STATIC void prof_free_sub_channel(struct prof_sub_channel_res *sub_channel_res,
    u32 device_id, u32 channel_id, u32 sub_channel_id)
{
    struct prof_channel_resource_info *channel_resource_info = NULL;
    struct prof_device_resource_info *dev_res_info = NULL;
    u32 phy_device_id = device_id;
#ifdef CFG_FEATURE_SRIOV
    u32 vfid;
    (void)vmngd_get_pfvf_id_by_devid(device_id, &phy_device_id, &vfid);
#endif

    dev_res_info = prof_get_device_resource_info(phy_device_id);
    if (dev_res_info == NULL) {
        prof_err("Failed to get dev res info. (phy_device_id=%u)", phy_device_id);
        return;
    }

    channel_resource_info = prof_get_channel_resource_info(phy_device_id, channel_id);
    spin_lock_bh(&channel_resource_info->sub_resource_info[sub_channel_id].spinlock);
    channel_resource_info->sub_resource_info[sub_channel_id].proc_ctx = NULL;
    spin_unlock_bh(&channel_resource_info->sub_resource_info[sub_channel_id].spinlock);
    channel_resource_info->sub_resource_info[sub_channel_id].vfid = PROF_VFID_NUM_MAX;
    channel_resource_info->sub_resource_info[sub_channel_id].ts_channel.cmd_verify = 0;

    prof_free_sub_channel_id(sub_channel_res, device_id, phy_device_id, channel_id, sub_channel_id);
}

STATIC int prof_alloc_sub_channel_id(struct prof_ioctl_para *para, struct prof_proc_ctx *proc_ctx,
    struct prof_channel_info *channel_info, u32 *sub_channel_id)
{
    u32 device_id = para->device_id;
    u32 phy_device_id = device_id;
    u32 channel_id = para->channel_id;
    int ret;

#ifdef CFG_FEATURE_SRIOV
    u32 vfid;
    (void)vmngd_get_pfvf_id_by_devid(device_id, &phy_device_id, &vfid);
#endif

    ret = prof_alloc_sub_resource_check(channel_info, proc_ctx, phy_device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to check the CTX subordinate channel resources.\n");
        return ret;
    }

    ret = prof_get_available_sub_channel(para, proc_ctx, phy_device_id, sub_channel_id);
    if (ret != PROF_OK) {
        prof_err("Profile had no available subordinate channel. (phy_device_id=%u; vfid=%u; channel_id=%u)\n",
            phy_device_id, para->vfid, channel_id);
        return ret;
    }

    if (channel_info->channel_used_count >= channel_info->channel_used_num_max) {
        prof_err("Parameter [channel_used_count] was invalid. The used channel sum was over the maximum."
            " (device_id=%u; vfid=%u; channel_id=%u; channel_used_count=%u; channel_used_num_max=%u)\n",
            channel_info->device_id, channel_info->vfid, channel_info->channel_id,
            channel_info->channel_used_count, channel_info->channel_used_num_max);
        prof_free_sub_channel_id(&proc_ctx->sub_channel_res, device_id, phy_device_id, channel_id, *sub_channel_id);
        return PROF_VF_SUB_RESOURCE_FULL;
    }

#ifdef CFG_FEATURE_PROF_HWTS_THROUGH
    prof_set_device_flag_vm_or_phy(phy_device_id,
        para->vfid > PROF_PHYSICAL_MACHINE_VFID ? PROF_DEV_IS_VM : PROF_DEV_IS_PHY);
#endif

    prof_add_to_sub_resource_info(channel_info, proc_ctx, phy_device_id, *sub_channel_id);
    return PROF_OK;
}

struct prof_sub_channel_info *prof_alloc_sub_channel_resource(struct prof_ioctl_para *para,
    struct prof_proc_ctx *proc_ctx)
{
    struct prof_channel_info *channel_info = NULL;
    struct prof_sub_channel_info *sub_channel_info = NULL;
    u32 device_id = para->device_id;
    u32 vfid = para->vfid;
    u32 channel_id = para->channel_id;
    u32 sub_channel_id;
    int ret;

    channel_info = prof_get_channel_info_from_index(device_id, vfid, channel_id);

    mutex_lock(&(channel_info->mutex));
    ret = prof_alloc_sub_channel_id(para, proc_ctx, channel_info, &sub_channel_id);
    if (ret != PROF_OK) {
        mutex_unlock(&(channel_info->mutex));
        prof_err("Failed to alloc sub channel id.\n");
        goto err;
    }

    mutex_unlock(&(channel_info->mutex));
    sub_channel_info = prof_get_sub_channel_info_from_index(device_id, vfid, channel_id, sub_channel_id);

    return sub_channel_info;

err:
    para->ret_val = ret;
    return NULL;
}

void prof_free_sub_channel_resource(struct prof_sub_channel_info *sub_channel_info)
{
    struct prof_channel_info *channel_info = prof_get_channel_info_from_index(sub_channel_info->device_id,
        sub_channel_info->vfid, sub_channel_info->channel_id);

    mutex_lock(&channel_info->mutex);
    if (channel_info->sub_channel_info[sub_channel_info->sub_channel_id] == sub_channel_info) {
        channel_info->sub_channel_info[sub_channel_info->sub_channel_id] = NULL;
        channel_info->channel_used_count--;
        mutex_unlock(&channel_info->mutex);
        prof_free_sub_channel(&sub_channel_info->proc_ctx->sub_channel_res,
            sub_channel_info->device_id, sub_channel_info->channel_id,
            sub_channel_info->sub_channel_id);
        return;
    }
    mutex_unlock(&channel_info->mutex);
}

void prof_sub_resource_info_init(u32 device_id, u32 channel_id,
    u32 sub_channel_id, struct prof_sub_channel_info *sub_resource_info)
{
    u32 memory_type;

    sub_resource_info->device_id = device_id;
    sub_resource_info->vfid = PROF_VFID_NUM_MAX;
    sub_resource_info->channel_id = channel_id;
    sub_resource_info->sub_channel_id = sub_channel_id;
    sub_resource_info->proc_ctx = NULL;
    prof_sub_resource_channel_state_init(sub_resource_info);
    memory_type = prof_get_mem_type(channel_id);
    sub_resource_info->peri_channel.sample_thread_flag = PROF_SAMPLE_THREAD_EXIT;

    mutex_init(&sub_resource_info->state_mutex);
    init_waitqueue_head(&sub_resource_info->channel_wq);
    spin_lock_init(&sub_resource_info->spinlock);
    prof_init_tsid(sub_resource_info);

    if (prof_get_channel_type(channel_id) == PROF_TS_TYPE) {
        sub_resource_info->ts_channel.ts_buff_free_flag = TS_BUFF_FREE;
        if (memory_type == MEM_RESERVE) {
            sub_resource_info->phy_addr = prof_get_reserved_mem(device_id, channel_id, sub_channel_id);
        }
    }
}

STATIC void prof_channel_info_init(u32 device_id, u32 vfid, u32 channel_id,
    u32 aicore_num, u32 total_core_num)
{
    struct prof_channel_info *channel_info = prof_get_channel_info_from_index(device_id, vfid, channel_id);
    int i;

    channel_info->device_id = device_id;
    channel_info->vfid = vfid;
    channel_info->channel_id = channel_id;
    channel_info->channel_used_count = 0;
    channel_info->channel_used_num_max = 1;
    mutex_init(&(channel_info->mutex));

    for (i = 0; i < PROF_SUB_CHANNEL_NUM_MAX; i++) {
        channel_info->sub_channel_info[i] = NULL;
    }

    prof_debug("(device_id=%u; vfid=%u; channel_id=%u; aicore_num=%u; total_core_num=%u; channel_used_num_max=%u)\n",
        device_id, vfid, channel_id, aicore_num, total_core_num, channel_info->channel_used_num_max);
}

STATIC void prof_channel_recycle(u32 device_id, u32 vfid_num, u32 channel_num)
{
    struct prof_channel_info *channel_info = NULL;
    u32 i, j;

    for (i = 0; i < vfid_num; i++) {
        for (j = 0; j < channel_num; j++) {
            channel_info = prof_get_channel_info_from_index(device_id, i, j);
            mutex_destroy(&(channel_info->mutex));
        }
    }

    return;
}

STATIC void prof_channel_all_init(u32 device_id, u32 aicore_num, u32 total_core_num)
{
    int i, j;

    for (i = 0; i < PROF_VFID_NUM_MAX; i++) {
        for (j = 0; j < PROF_CHANNEL_NUM; j++) {
            prof_channel_info_init(device_id, i, j, aicore_num, total_core_num);
        }
    }
}

STATIC void prof_channel_all_uninit(u32 device_id)
{
    prof_channel_recycle(device_id, PROF_VFID_NUM_MAX, PROF_CHANNEL_NUM);
}

int prof_poll_init(struct prof_poll_info_kernel *prof_poll_mgr)
{
    int ret;

    ret = memset_s(prof_poll_mgr, sizeof(struct prof_poll_info_kernel), 0, sizeof(struct prof_poll_info_kernel));
    if (ret != EOK) {
        prof_err("Failed to invoke function [memset_s] to clean the data. (ret=%d)\n", ret);
        return ret;
    }

    prof_poll_mgr->poll_box = (prof_poll_box_t *)vzalloc(sizeof(prof_poll_box_t) * PROF_POLL_DEPTH);
    if (prof_poll_mgr->poll_box == NULL) {
        prof_err("Failed to invoke function [vzalloc] to apply for memory.\n");
        return PROF_ERROR;
    }

    prof_poll_mgr->poll_head = 0;
    prof_poll_mgr->poll_tail = 0;
    prof_poll_mgr->status = 0;
    atomic_set(&prof_poll_mgr->fd_num, 0);
    spin_lock_init(&prof_poll_mgr->spinlock);
    init_waitqueue_head(&prof_poll_mgr->poll_wq);

    return PROF_OK;
}

void prof_poll_free(struct prof_poll_info_kernel *prof_poll_mgr)
{
    if (prof_poll_mgr->poll_box != NULL) {
        vfree(prof_poll_mgr->poll_box);
        prof_poll_mgr->poll_box = NULL;
    }
}

int prof_ctx_init(struct prof_proc_ctx *ctx)
{
    int ret;

    ret = prof_poll_init(&ctx->poll);
    if (ret != PROF_OK) {
        prof_err("Failed to initialize the profile poll.\n");
        return PROF_ERROR;
    }

    ret = prof_ctx_sub_channel_res_init(&ctx->sub_channel_res);
    if (ret != PROF_OK) {
        prof_poll_free(&ctx->poll);
        prof_err("Failed to initialize the profile CTX subordinate channel.\n");
        return PROF_ERROR;
    }

    return PROF_OK;
}

void prof_ctx_uninit(struct prof_proc_ctx *ctx)
{
    prof_poll_free(&ctx->poll);
    prof_ctx_sub_channel_res_uninit(&ctx->sub_channel_res);
}

int prof_wait_read_last_data(struct prof_sub_channel_info *sub_channel_info)
{
    long wait_ret = wait_event_interruptible_timeout(sub_channel_info->channel_wq,
        sub_channel_info->channel_state == (u32)PROF_CHANNEL_ENABLE, TS2DRV_TIMEOUT);
    if (wait_ret == PROF_WAIT_NOTHING) {
        prof_err("Waiting for reading the last data was timeout."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u; remain_time=%ld)\n",
            sub_channel_info->device_id, sub_channel_info->vfid, sub_channel_info->channel_id,
            sub_channel_info->sub_channel_id, wait_ret);
        return PROF_TIMEOUT;
    } else if (wait_ret == -ERESTARTSYS) {
        prof_warn("Function [wait_event_interruptible_timeout] had been awakened "
            "by the value returned from interruption function. (err_ret=%ld).\n", wait_ret);
        return PROF_ERROR;
    } else if (wait_ret < 0) {
        prof_err("Failed to get the returned value from function [wait_event_interruptible_timeout]."
            " (err_ret=%ld)\n", wait_ret);
        return PROF_ERROR;
    }

    return PROF_OK;
}

int prof_stop_channel_status_check(struct prof_sub_channel_info *sub_channel_info,
    struct prof_proc_ctx *proc_ctx)
{
    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_DISABLE ||
        sub_channel_info->channel_state == (u32)PROF_CHANNEL_IDLE ||
        sub_channel_info->channel_state == (u32)PROF_CHANNEL_STARTING) {
        prof_warn("The subordinate channel had been disabled. Please do not stop it again."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u; sub_channel_state=%u)\n",
            sub_channel_info->device_id, sub_channel_info->vfid, sub_channel_info->channel_id,
            sub_channel_info->sub_channel_id, sub_channel_info->channel_state);
        return PROF_STOPPED_ALREADY;
    }

    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_STOPPING ||
        sub_channel_info->channel_state == (u32)PROF_CHANNEL_STOP_WAIT_TS) {
        prof_warn("The subordinate channel was stopping. Please do not stop it again."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)\n",
            sub_channel_info->device_id, sub_channel_info->vfid,
            sub_channel_info->channel_id, sub_channel_info->sub_channel_id);
        return PROF_BUSY;
    }

    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_FLUSH) {
        prof_err("The subordinate channel was flusing. Please do not flush it again."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)\n",
            sub_channel_info->device_id, sub_channel_info->vfid,
            sub_channel_info->channel_id, sub_channel_info->sub_channel_id);
        return PROF_ERROR;
    }

    spin_lock_bh(&sub_channel_info->spinlock);
    if (sub_channel_info->proc_ctx != proc_ctx) {
        spin_unlock_bh(&sub_channel_info->spinlock);
        prof_err("Failed to disable other profile channels."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)\n",
            sub_channel_info->device_id, sub_channel_info->vfid,
            sub_channel_info->channel_id, sub_channel_info->sub_channel_id);
        return PROF_ERROR;
    }
    spin_unlock_bh(&sub_channel_info->spinlock);

    return PROF_OK;
}

void prof_show_dfx_info(struct prof_sub_channel_info *sub_channel_info)
{
    enum prof_channel_type channel_type = prof_get_channel_type(sub_channel_info->channel_id);
    if (channel_type == PROF_TS_TYPE) {
        prof_event("Profile showed TS_type DFX information."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u; "
            "data_buf_len=%u; read_ptr=%u; write_ptr=%u; prof_out_total_size=%llu;"
            " prof_cq1_counts_from_ts=%u; prof_wake_up_poll_count=%u; "
            "prof_read_count=%u; prof_read_flipped_count=%u; "
            "buf_not_enough_count=%u\n",
            sub_channel_info->device_id, sub_channel_info->vfid,
            sub_channel_info->channel_id, sub_channel_info->sub_channel_id,
            sub_channel_info->prof_dfx.data_buf_len,
            sub_channel_info->prof_dfx.read_ptr, sub_channel_info->prof_dfx.write_ptr,
            sub_channel_info->prof_dfx.prof_out_total_size,
            sub_channel_info->prof_dfx.prof_cq1_counts_from_ts,
            sub_channel_info->prof_dfx.prof_wake_up_poll_count,
            sub_channel_info->prof_dfx.prof_read_count,
            sub_channel_info->prof_dfx.prof_read_flipped_count,
            sub_channel_info->prof_dfx.buf_not_enough_count);
    } else {
        prof_event("Profile showed Peri_type DFX information."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u; channel_state=%u; poll_flag=%d; "
            "prof_in_total_size=%llu; prof_out_total_size=%llu; collect_cycles_count=%u; read_count=%u;",
            sub_channel_info->device_id, sub_channel_info->vfid, sub_channel_info->channel_id,
            sub_channel_info->sub_channel_id, sub_channel_info->channel_state,
            sub_channel_info->poll_flag,
            sub_channel_info->prof_dfx.prof_in_total_size,
            sub_channel_info->prof_dfx.prof_out_total_size,
            sub_channel_info->prof_dfx.prof_wake_up_poll_count,
            sub_channel_info->prof_dfx.prof_read_count);
    }
}

void prof_dev_dfx_load(prof_dev_dfx_info_t *dev_dfx, struct prof_ioctl_para *prof_para)
{
    if (prof_para->buf_len == sizeof(prof_dev_dfx_info_t)) {
        (void)prof_send_data(prof_para->out_buf, dev_dfx, sizeof(prof_dev_dfx_info_t), prof_para);
    }
}

void prof_init_dfx_info(struct prof_sub_channel_info *sub_channel_info)
{
    sub_channel_info->prof_dfx.prof_in_total_size = 0;
    sub_channel_info->prof_dfx.prof_out_total_size = 0;
    sub_channel_info->prof_dfx.prof_cq1_counts_from_ts = 0;
    sub_channel_info->prof_dfx.prof_wake_up_poll_count = 0;
    sub_channel_info->prof_dfx.prof_read_count = 0;
    sub_channel_info->prof_dfx.prof_read_flipped_count = 0;
    sub_channel_info->prof_dfx.buf_not_enough_count = 0;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC int prof_read_exception(struct prof_ioctl_para *para)
{
    para->ret_val = PROF_NOT_SUPPORT;
    return PROF_OK;
}
#else
STATIC int prof_read_exception(struct prof_ioctl_para *para)
{
    prof_err("Channel_id was invalid. (device_id=%u; channel_id=%u)\n", para->device_id, para->channel_id);
    return PROF_ERROR;
}
#endif

int prof_drv_read(struct prof_proc_ctx *proc_ctx,
    struct prof_ioctl_para *prof_para)
{
    enum prof_channel_type channel_type = prof_get_channel_type(prof_para->channel_id);

    int ret = prof_check_device_state(prof_para->device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to check the Profile device status. (device_id=%u; ret=%d).\n", prof_para->device_id, ret);
        return PROF_ERROR;
    }

    if (prof_para->out_buf == NULL) {
        prof_err("The variable out_buf is NULL. (device_id=%u; vfid=%u; channel_id=%u)\n",
            prof_para->device_id, prof_para->vfid, prof_para->channel_id);
        return PROF_ERROR;
    }

    if (prof_para->buf_len == 0) {
        prof_warn("Parameter [buf_len] was invalid. (device_id=%u; vfid=%u; channel_id=%u; buf_len=0)\n",
            prof_para->device_id, prof_para->vfid, prof_para->channel_id);
        return PROF_NOT_ENOUGH_BUF;
    }

    if (channel_type == PROF_PERIPHERAL_TYPE) {
        ret = prof_agent_read(proc_ctx, prof_para);
    } else if (channel_type == PROF_TS_TYPE) {
        ret = prof_tscpu_read(proc_ctx, prof_para);
    } else {
        return prof_read_exception(prof_para);
    }

    return ret;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC int prof_stop_exception(struct prof_ioctl_para *para)
{
    para->ret_val = PROF_NOT_SUPPORT;
    return PROF_OK;
}
#else
STATIC int prof_stop_exception(struct prof_ioctl_para *para)
{
    prof_err("Parameter [channel_id] was invalid. (device_id=%u; channel_id=%u)\n",
        para->device_id, para->channel_id);
    return PROF_ERROR;
}
#endif

int prof_drv_stop(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *prof_para)
{
    int ret = PROF_ERROR;
    int channel_type;

    channel_type = prof_get_channel_type(prof_para->channel_id);
    if (channel_type == PROF_PERIPHERAL_TYPE) {
        ret = prof_agent_stop(proc_ctx, prof_para);
    } else if (channel_type == PROF_TS_TYPE) {
        ret = prof_tscpu_stop(proc_ctx, prof_para);
    } else {
        return prof_stop_exception(prof_para);
    }

    if (atomic_read(&proc_ctx->poll.fd_num) <= 0) {
        wake_up(&(proc_ctx->poll.poll_wq));
    }

    return ret;
}

struct prof_sub_channel_info *prof_get_sub_channel_info_from_ctx_no_lock(struct prof_proc_ctx *proc_ctx,
    u32 device_id, u32 vfid, u32 channel_id)
{
    struct prof_channel_info *channel_info = NULL;
    struct prof_sub_channel_info *sub_channel_info = NULL;
    int i;

    channel_info = prof_get_channel_info_from_index(device_id, vfid, channel_id);
    for (i = 0; i < PROF_SUB_CHANNEL_NUM_MAX; i++) {
        sub_channel_info = channel_info->sub_channel_info[i];
        if (sub_channel_info == NULL) {
            continue;
        }
        if (sub_channel_info->proc_ctx == proc_ctx) {
            return (channel_info->sub_channel_info[i]);
        }
    }

    return NULL;
}

STATIC void prof_pollflag_init(struct prof_poll_info_kernel *prof_poll_mgr, u32 vfid, int start,
    int end, struct prof_proc_ctx *proc_ctx)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;
    u32 channel_id;
    u32 device_id;
    int idx;

    for (idx = start; idx < end; idx++) {
        device_id = prof_poll_mgr->poll_box[idx].device_id;
        channel_id = prof_poll_mgr->poll_box[idx].channel_id;
        sub_channel_info = prof_get_sub_channel_info_from_ctx_no_lock(proc_ctx, device_id, vfid, channel_id);
        if (sub_channel_info != NULL) {
            sub_channel_info->poll_flag = (int)POLL_INVALID;
        }
    }

    return;
}

STATIC int prof_poll_copy(struct prof_poll_info_kernel *prof_poll_mgr,
    struct prof_ioctl_para *para, struct prof_proc_ctx *proc_ctx)
{
    int ret;
    int size = 0;
    int all_size = 0;
    int copy_size = 0;
    struct prof_poll_box *tmp_box = NULL;
    int poll_box_size = sizeof(prof_poll_box_t) * para->poll_number;

    tmp_box = (prof_poll_box_t *)vzalloc(poll_box_size);
    if (tmp_box == NULL) {
        prof_err("Failed to invoke function [vzalloc] to apply for memory.\n");
        return PROF_ERROR;
    }

    if (para->out_buf == NULL) {
        prof_err("Parameter [out_buf] was invalid. (out_buf=NULL)\n");
        vfree(tmp_box);
        return PROF_ERROR;
    }

    para->buf_len = para->poll_number * sizeof(prof_poll_box_t);
    spin_lock_bh(&prof_poll_mgr->spinlock);
    if (prof_poll_mgr->poll_head == prof_poll_mgr->poll_tail) {
        spin_unlock_bh(&prof_poll_mgr->spinlock);
        para->ret_val = 0;
        vfree(tmp_box);
        return PROF_OK;
    }

    if (prof_poll_mgr->poll_head > prof_poll_mgr->poll_tail) {
        size = prof_poll_mgr->poll_head - prof_poll_mgr->poll_tail;
        copy_size = sizeof(prof_poll_box_t) * size;
        if (copy_size > poll_box_size) {
            copy_size = poll_box_size;
            size = para->poll_number;
        }

        ret = memcpy_s(tmp_box, poll_box_size,
                       &prof_poll_mgr->poll_box[prof_poll_mgr->poll_tail], copy_size);
        if (ret != EOK)
            goto error;

        prof_pollflag_init(prof_poll_mgr, para->vfid, prof_poll_mgr->poll_tail,
            prof_poll_mgr->poll_tail + size, proc_ctx);
        prof_poll_mgr->poll_tail = prof_poll_mgr->poll_tail + size;
        goto back_user;
    }

    if (prof_poll_mgr->poll_tail < PROF_POLL_DEPTH) {
        size = PROF_POLL_DEPTH - prof_poll_mgr->poll_tail;

        if (size > para->poll_number) {
            size = para->poll_number;
        }

        ret = memcpy_s(tmp_box, poll_box_size, &prof_poll_mgr->poll_box[prof_poll_mgr->poll_tail],
                       (size_t)(sizeof(prof_poll_box_t) * size));
        if (ret != EOK)
            goto error;

        prof_pollflag_init(prof_poll_mgr, para->vfid, prof_poll_mgr->poll_tail,
            prof_poll_mgr->poll_tail + size, proc_ctx);

        if ((size >= para->poll_number) || (prof_poll_mgr->poll_head == 0)) {
            prof_poll_mgr->poll_tail = (prof_poll_mgr->poll_tail + size) % PROF_POLL_DEPTH;
            copy_size = sizeof(prof_poll_box_t) * size;
            goto back_user;
        }

        all_size = size;
    }

    size = prof_poll_mgr->poll_head;

    // enhance condition for static check
    if ((size + all_size) > para->poll_number) {
        size = para->poll_number - all_size;
    }

    /* notice user buffer's offset */
    ret = memcpy_s((void *)((uintptr_t)tmp_box + ((unsigned long)sizeof(prof_poll_box_t) * all_size)),
                   poll_box_size - (sizeof(prof_poll_box_t) * all_size),
                   (void *)prof_poll_mgr->poll_box, (size_t)(sizeof(prof_poll_box_t) * size));
    if (ret != EOK) {
        size = size + all_size;
        goto error;
    }

    prof_pollflag_init(prof_poll_mgr, para->vfid, 0, size, proc_ctx);
    prof_poll_mgr->poll_tail = size;
    copy_size = sizeof(prof_poll_box_t) * (size + all_size);

back_user:
    spin_unlock_bh(&prof_poll_mgr->spinlock);
    ret = prof_send_data(para->out_buf, (void *)tmp_box, copy_size, para);
    if (ret != PROF_OK) {
        para->ret_val = 0;
        prof_err("Failed to make profile send data. (ret=%d; copy_size=%d)\n", ret, copy_size);
        vfree(tmp_box);
        return PROF_ERROR;
    }
    para->ret_val = copy_size / (int)sizeof(prof_poll_box_t);
    vfree(tmp_box);

    return PROF_OK;

error:
    spin_unlock_bh(&prof_poll_mgr->spinlock);
    para->ret_val = 0;
    copy_size = sizeof(prof_poll_box_t) * size;
    prof_err("Failed to invoke function [memcpy_s]. (ret=%d; copy_size=%d)\n", ret, copy_size);
    vfree(tmp_box);

    return PROF_ERROR;
}

int prof_poll(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para)
{
    int ret = PROF_ERROR;
    long wait_ret = 0;
    long time_remain = (long)para->timeout * TIME_UNIT;
    struct prof_poll_info_kernel *prof_poll_mgr = &proc_ctx->poll;

    if ((para->poll_number > PROF_CHANNEL_NUM) || (para->poll_number <= 0)) {
        prof_err("Parameter [poll_number] was [NULL]. (number=%d)\n", para->poll_number);
        return PROF_ERROR;
    }

    if ((para->timeout <= 0) || (para->timeout > PROF_POLL_DEFAULT_TIMEOUT)) {
        prof_warn("The default timeout value was set. (timeout=%dHZ; PROF_POLL_DEFAULT_TIMEOUT=%dHZ)\n",
            para->timeout, PROF_POLL_DEFAULT_TIMEOUT);
        time_remain = PROF_POLL_DEFAULT_TIMEOUT * TIME_UNIT;
    }

    if (atomic_read(&prof_poll_mgr->fd_num) <= 0) {
        para->ret_val = PROF_STOPPED_ALREADY;
        return PROF_OK;
    }

    if (prof_poll_mgr->poll_head == prof_poll_mgr->poll_tail) {
        wait_ret = wait_event_interruptible_timeout(prof_poll_mgr->poll_wq,
            ((prof_poll_mgr->poll_head != prof_poll_mgr->poll_tail) || (prof_poll_mgr->status != 0) ||
            (atomic_read(&prof_poll_mgr->fd_num) <= 0)), time_remain);
        if (wait_ret == -ERESTARTSYS) {
            prof_warn("Received the value returned from function [wait_event_interruptible_timeout]."
                " (err_ret=%ld)\n", wait_ret);
            para->ret_val = 0;
            return PROF_OK;
        }

        if (prof_poll_mgr->status != 0) {
            para->ret_val = 0;
            return PROF_OK;
        }

        if (wait_ret == PROF_WAIT_NOTHING) {
            para->ret_val = 0;
            return PROF_OK;
        } else if (wait_ret < 0) {
            prof_warn("Received the value returned from function [wait_event_interruptible_timeout]."
                " (err_ret=%ld)\n", wait_ret);
            return PROF_ERROR;
        }
    }

    ret = prof_poll_copy(prof_poll_mgr, para, proc_ctx);
    if (ret != PROF_OK) {
        prof_err("Failed to make profile poll copy data. (ret=%d; mode=%u)\n", ret, para->use_mode);
        return ret;
    }

    return ret;
}

STATIC void prof_channel_list_init(channel_list_t *channel_list, u32 device_id, u32 vfid)
{
    int i;
    channel_list->channel_num = 0;

    for (i = 0; i < CHANNEL_IDS_MAX; i++) {
        if (prof_check_channel_with_vfid(device_id, i, vfid) == PROF_OK) {
            channel_list->channel[channel_list->channel_num].channel_id = i;
            channel_list->channel_num++;
        }
    }
}

int prof_get_channel_list(struct prof_ioctl_para *para)
{
    channel_list_t *channel_list = NULL;
    int ret;

    ret = prof_check_device_state(para->device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to check the status of devices. (device_id=%u; ret=%d)\n", para->device_id, ret);
        return PROF_ERROR;
    }

    if (para->buf_len < sizeof(channel_list_t)) {
        prof_err("Parameter [buf_len] was less than [sizeof(channel_list_t)]."
            " (device_id=%u; vfid=%u; buf_len=%u; sizeof(channel_list_t)=%lu)\n",
            para->device_id, para->vfid, para->buf_len, sizeof(channel_list_t));
        return PROF_ERROR;
    }

    if (para->out_buf == NULL) {
        prof_err("Parameter [buf_len] was invalid. (device_id=%u; vfid=%u; buf_len=NULL)\n",
            para->device_id, para->vfid);
        return PROF_ERROR;
    }

    channel_list = vzalloc(sizeof(channel_list_t));
    if (channel_list == NULL) {
        prof_err("Failed to invoke function [vzalloc] to apply for memory. (device_id=%u; vfid=%u)\n",
            para->device_id, para->vfid);
        return PROF_ERROR;
    }

    prof_channel_list_init(channel_list, para->device_id, para->vfid);

    ret = prof_send_data(para->out_buf, (void *)channel_list, sizeof(channel_list_t), para);
    if (ret != PROF_OK) {
        para->ret_val = PROF_ERROR;
        prof_err("Failed to make profile send data. (device_id=%u; vfid=%u; ret=%d; copy_size=%lu)\n",
            para->device_id, para->vfid, ret, sizeof(channel_list_t));
        vfree(channel_list);
        return PROF_ERROR;
    }

    vfree(channel_list);
    para->ret_val = PROF_OK;
    return PROF_OK;
}

struct prof_sub_channel_info *prof_get_sub_channel_info_from_ctx(struct prof_proc_ctx *proc_ctx,
    u32 device_id, u32 vfid, u32 channel_id)
{
    struct prof_channel_info *channel_info = NULL;
    struct prof_sub_channel_info *sub_channel_info = NULL;
    int i;

    channel_info = prof_get_channel_info_from_index(device_id, vfid, channel_id);
    for (i = 0; i < PROF_SUB_CHANNEL_NUM_MAX; i++) {
        sub_channel_info = channel_info->sub_channel_info[i];
        if (sub_channel_info == NULL) {
            continue;
        }
        spin_lock_bh(&sub_channel_info->spinlock);
        if (sub_channel_info->proc_ctx == proc_ctx) {
            spin_unlock_bh(&sub_channel_info->spinlock);
            return (channel_info->sub_channel_info[i]);
        }
        spin_unlock_bh(&sub_channel_info->spinlock);
    }

    return NULL;
}

struct prof_sub_channel_info *prof_get_sub_channel_info_from_index(u32 device_id,
    u32 vfid, u32 channel_id, u32 sub_channel_id)
{
    return prof_device_info_get(device_id)->vf_info[vfid].channel_info[channel_id].sub_channel_info[sub_channel_id];
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC int prof_start_exception(struct prof_ioctl_para *para, enum prof_channel_type channel_type)
{
    para->ret_val = PROF_NOT_SUPPORT;
    return PROF_OK;
}
#else
STATIC int prof_start_exception(struct prof_ioctl_para *para, enum prof_channel_type channel_type)
{
    prof_err("Failed to get information of channel and device."
        " (device_id=%u; channel_id=%u; channel_type=%d)\n",
        para->device_id, para->channel_id, (int)channel_type);
    return PROF_ERROR;
}
#endif

/* 后续显式包含对应头文件 */
int devdrv_manager_container_check_devid_in_container_ns(u32 devid, struct task_struct *tsk);
STATIC int prof_distribute_cmd(struct prof_proc_ctx *ctx, struct prof_ioctl_para *para)
{
    u32 cmd = para->cmd;
    enum prof_channel_type channel_type;
    int ret = PROF_OK;

    if (cmd == PROF_POLL) {
        (void)prof_poll(ctx, para);
        return ret;
    }

#ifndef AOS_LLVM_BUILD
    ret = devdrv_manager_container_check_devid_in_container_ns(para->device_id, current);
    if (ret != 0) {
        prof_err("Device is not in container. (devid=%u)\n", para->device_id);
        return -EINVAL;
    }
#endif

    ret = prof_check_device_state(para->device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to check the status of device. (device_id=%u; ret=%d)\n", para->device_id, ret);
        return PROF_ERROR;
    }

    switch (cmd) {
        case PROF_START:
            ret = prof_check_channel_with_vfid(para->device_id, para->channel_id, para->vfid);
            if (ret != PROF_OK) {
                break;
            }
            channel_type = prof_get_channel_type(para->channel_id);
            if (channel_type == PROF_TS_TYPE) {
                (void)prof_tscpu_start(ctx, para);
            } else if (channel_type == PROF_PERIPHERAL_TYPE) {
                (void)prof_agent_start(ctx, para);
            } else {
                ret = prof_start_exception(para, channel_type);
            }
            break;
        case PROF_STOP:
            (void)prof_drv_stop(ctx, para);
            break;
        case PROF_READ:
            (void)prof_drv_read(ctx, para);
            break;
        case PROF_GET_CHANNEL_LIST:
            (void)prof_get_channel_list(para);
            break;
        case PROF_DATA_FLUSH:
            (void)prof_data_len_flush(ctx, para);
            break;
        default:
            prof_err("Parameter [cmd] was invaild. (cmd=%u)\n", cmd);
            return PROF_ERROR;
    }

    return ret;
}

int prof_send_data(void *outbuf, const void *buf, u32 size,
                   struct prof_ioctl_para *para)
{
    if ((outbuf == NULL) || (buf == NULL) || (size == 0)) {
        return PROF_ERROR;
    }

    /* In order to be compatible with the stored code,
     * the USER scene retains the copy_to_user operation,
     * and pre-embeds the subsequent KERNEL scene.
     */
    if (para->use_mode == PROF_MODE_USER) {
        if (copy_to_user_secure((void __user *)outbuf, buf, size) != 0) {
            prof_err("Failed to invoke function [copy_to_user_secure] to send data to user."
                " (read_mode=%u, device_id=%u; channel_id=%u; size=%u)\n",
                para->use_mode, para->device_id, para->channel_id, size);
            return PROF_ERROR;
        }
    } else {
        if (memcpy_s(outbuf, para->buf_len, (void *)buf, size) != EOK) {
            prof_err("Failed to invoke function [memcpy_s] to send data to user."
                " (read_mode=%u; device_id=%u; channel_id=%u; size=%u)\n",
                para->use_mode, para->device_id, para->channel_id, size);
            return PROF_ERROR;
        }
    }
    return PROF_OK;
}

STATIC long prof_drv_ioctl(struct file *file, u32 cmd, unsigned long arg)
{
    struct prof_proc_ctx *ctx = (struct prof_proc_ctx *)file->private_data;
    struct prof_ioctl_para prof_para = {0};
    void __user *parg = NULL;
    u32 drv_cmd = _IOC_NR(cmd);
    int ret;

    if (drv_cmd >= PROF_CMD_MAX || drv_cmd < PROF_GET_PLATFORM) {
        prof_err("Parameter [drv_cmd] was invaild. (cmd=%u)\n", drv_cmd);
        return PROF_ERROR;
    }

    parg = (void __user *)(uintptr_t)arg;
    if (parg == NULL) {
        prof_err("Parameter [arg] was invalid. (parg=NULL)\n");
        return PROF_ERROR;
    }

    (void)memset_s(&prof_para, sizeof(prof_para), 0, sizeof(struct prof_ioctl_para));
    if (copy_from_user(&prof_para, parg, sizeof(struct prof_ioctl_para)) != 0) {
        prof_err("Failed to invoke function [copy_from_user] to get data from user.\n");
        return PROF_ERROR;
    }

    prof_para.ret_val = PROF_ERROR;
    prof_para.cmd = drv_cmd;
    prof_para.vfid = PROF_PHYSICAL_MACHINE_VFID;  /* vfid = 0, physical machine */
    prof_para.use_mode = PROF_MODE_USER;

    ret = prof_distribute_cmd(ctx, &prof_para);
    if (ret != PROF_OK) {
        prof_err("Command error. Failed to excutive the command. (ret=%d; drv_cmd=%u)\n", ret, drv_cmd);
    }

    if (prof_send_data(parg, &prof_para, sizeof(struct prof_ioctl_para), &prof_para) != 0) {
        prof_err("Failed to make profile send data.\n");
        return PROF_ERROR;
    }

    return ret;
}

STATIC int prof_drv_open(struct inode *inode, struct file *filp)
{
    struct prof_proc_ctx *ctx = NULL;
    int ret;

    ctx = kzalloc(sizeof(struct prof_proc_ctx), GFP_KERNEL | __GFP_ACCOUNT);
    if (ctx == NULL) {
        prof_err("Failed to invoke function [kzalloc] to apply for memory.\n");
        return PROF_ERROR;
    }

    filp->private_data = ctx;
    ctx->collect_target_pid = (u32)current->tgid;
    ret = prof_ctx_init(ctx);
    if (ret != PROF_OK) {
        prof_err("Failed to initialize profile CTX.\n");
        kfree(ctx);
        ctx = NULL;
        return PROF_ERROR;
    }

    prof_event("Profile driver opened successfully.\n");
    return PROF_OK;
}

STATIC void prof_ctx_channels_stop(struct prof_proc_ctx *ctx, u32 device_id, u32 vfid)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;
    struct prof_ioctl_para prof_para = {0};
    int ret;
    int k;

    prof_para.device_id = device_id;
    prof_para.vfid = vfid;

    for (k = 0; k < PROF_CHANNEL_NUM; k++) {
        sub_channel_info = prof_get_sub_channel_info_from_ctx(ctx, device_id, vfid, k);
        if (sub_channel_info == NULL) {
            continue;
        }
        spin_lock_bh(&sub_channel_info->spinlock);
        if (sub_channel_info->proc_ctx != ctx) {
            spin_unlock_bh(&sub_channel_info->spinlock);
            continue;
        }
        spin_unlock_bh(&sub_channel_info->spinlock);

        prof_para.channel_id = k;
        prof_para.use_mode = PROF_MODE_USER;
        ret = prof_drv_stop(ctx, &prof_para);
        if (ret != PROF_OK) {
            prof_err("Failed to stop CTX channel. (device_id=%u; vfid=%u; channel_id=%d)\n",
                device_id, vfid, k);
        }
    }
}

STATIC int prof_drv_release(struct inode *inode, struct file *filp)
{
    struct prof_proc_ctx *ctx = filp->private_data;
    int i, j;

    for (i = 0; i < PROF_DEVICE_NUM_VALUE; i++) {
        if (prof_device_info_get(i) == NULL) {
            continue;
        }

        for (j = 0; j < PROF_VFID_NUM_MAX; j++) {
            prof_ctx_channels_stop(ctx, i, j);
        }
    }

    isb();
    prof_ctx_uninit(ctx);
    kfree(ctx);

    prof_event("Profile driver released successfully.\n");

    return PROF_OK;
}
STATIC const struct file_operations prof_drv_fops = {
    .owner = THIS_MODULE,
#ifndef AOS_LLVM_BUILD
    .unlocked_ioctl = prof_drv_ioctl,
#else
    .ioctl = prof_drv_ioctl,
#endif
    .open = prof_drv_open,
    .release = prof_drv_release,
};

#ifndef AOS_LLVM_BUILD
STATIC char *prof_devnode(struct device *dev, umode_t *mode)
{
    return NULL;
}

STATIC int prof_drv_register_cdev(void)
{
    int ret;
    u32 major_dev;
    dev_t devno;
    struct char_device *priv = (struct char_device *)&char_dev_prof;

    priv->devno = 0;
    ret = alloc_chrdev_region(&priv->devno, 0, 1, CHAR_DRIVER_NAME);
    if (ret < 0) {
        prof_err("Failed to invoke function [alloc_chrdev_region]. (ret=%d)\n", ret);
        return PROF_ERROR;
    }

    /* init and add char device */
    major_dev = MAJOR(priv->devno);
    devno = MKDEV(major_dev, 0);
    cdev_init(&priv->cdev, &prof_drv_fops);
    priv->cdev.owner = THIS_MODULE;

    if (cdev_add(&priv->cdev, devno, 1) != 0) {
        prof_err("Failed to invoke function [cdev_add]. (devno=%u)\n", devno);
        unregister_chrdev_region(devno, 1);
        return PROF_ERROR;
    }

    priv->dev_class = class_create(THIS_MODULE, CHAR_DRIVER_NAME);
    if (IS_ERR(priv->dev_class)) {
        prof_err("Failed to invoke function [class_create]. (devno=%u)\n", devno);
        unregister_chrdev_region(devno, 1);
        cdev_del(&priv->cdev);
        return PROF_ERROR;
    }
    if (priv->dev_class != NULL)
        priv->dev_class->devnode = prof_devnode;

    char_dev_prof.device = device_create(priv->dev_class, NULL, devno, NULL, CHAR_DRIVER_NAME);

    return PROF_OK;
}

STATIC void prof_drv_unregister_cdev(void)
{
    struct char_device *priv = &char_dev_prof;

    device_destroy(priv->dev_class, priv->devno);
    class_destroy(priv->dev_class);
    unregister_chrdev_region(priv->devno, 1);
    cdev_del(&priv->cdev);
}
#else
struct devdrv_info g_prof_stub_dev_info = { .dev_id = 0, };
struct devdrv_info *devdrv_manager_get_devdrv_info(u32 device_id)
{
    if (device_id != 0) {
        return NULL;
    }
    return &g_prof_stub_dev_info;
}

STATIC int prof_drv_register_cdev(void)
{
    int ret;
    ret = register_driver(CHAR_DRIVER_NAME, &prof_drv_fops, PROF_NONE_ROOT_ACCESS, NULL);
    return ret;
}

STATIC void prof_drv_unregister_cdev(void)
{
    unregister_driver(CHAR_DRIVER_NAME);
}
#endif

STATIC int prof_alloc_device_resource(u32 device_id, u32 aicore_num, u32 total_core_num)
{
    int ret;

    prof_channel_all_init(device_id, aicore_num, total_core_num);

    ret = prof_alloc_all_tscpu_common_bufs(device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to allocate the buffer for TS channels. (device_id=%u)\n", device_id);
        prof_channel_all_uninit(device_id);
        return PROF_ERROR;
    }

    return PROF_OK;
}

STATIC void prof_free_device_resource(u32 device_id)
{
    prof_channel_all_uninit(device_id);
    prof_free_all_tscpu_common_bufs(device_id);
}

void prof_each_device_uninit(u32 device_id)
{
    int ret;

    ret = prof_check_device_state(device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to check the state of device. (device_id=%u; ret=%d)\n", device_id, ret);
        return;
    }

    prof_device_info_get(device_id)->device_state = DEV_UNUSED;
    prof_dev_tscpu_all_stop(device_id);
    prof_dev_agent_all_stop(device_id);
#ifdef CFG_FEATURE_HOST_COLLECTION
    prof_hdc_uninit_each_device(device_id);
#endif
    prof_free_device_resource(device_id);
    prof_info("Profile device uninitialization was success. (device_id=%u)\n", device_id);
}

int prof_each_device_init(u32 device_id, u32 aicore_num, u32 total_core_num)
{
    int ret;

    ret = prof_alloc_device_resource(device_id, aicore_num, total_core_num);
    if (ret != PROF_OK) {
        prof_err("Failed to allocate the device resources. (device_id=%u)\n", device_id);
        return ret;
    }

#ifdef CFG_FEATURE_HOST_COLLECTION
    ret = prof_hdc_init_each_device(device_id);
    if (ret != PROF_OK) {
        prof_free_device_resource(device_id);
        prof_err("Failed to initialize the hdc server. (device_id=%u; ret=%d)\n", device_id, ret);
        return ret;
    }
#endif

    prof_device_info_get(device_id)->device_state = DEV_USED;
    prof_info("Profile device initialization was success. (device_id=%u; ret=%d)\n", device_id, ret);
    return ret;
}

void prof_phydev_sub_channel_resource_init(u32 device_id)
{
    struct prof_device_resource_info *dev_res_info =
        prof_get_device_resource_info(device_id);
    struct prof_channel_resource_info *resource_info = NULL;
    int j, k;

    for (j = 0; j < CHANNEL_IDS_MAX; j++) {
        resource_info = prof_get_channel_resource_info(device_id, j);
        mutex_init(&resource_info->mutex);
        for (k = 0; k < PROF_SUB_CHANNEL_NUM_MAX; k++) {
            prof_sub_resource_info_init(device_id, j, k, &(resource_info->sub_resource_info[k]));
        }
    }
    mutex_init(&dev_res_info->mutex);
}

void prof_phydev_sub_channel_resource_uninit(u32 device_id)
{
    struct prof_device_resource_info *dev_res_info =
        prof_get_device_resource_info(device_id);
    struct prof_channel_resource_info *channel_resource_info = NULL;
    int j, k;

    for (j = 0; j < CHANNEL_IDS_MAX; j++) {
        channel_resource_info = prof_get_channel_resource_info(device_id, j);
        mutex_destroy(&channel_resource_info->mutex);
        for (k = 0; k < PROF_SUB_CHANNEL_NUM_MAX; k++) {
            mutex_destroy(&channel_resource_info->sub_resource_info[k].state_mutex);
        }
    }
    mutex_destroy(&dev_res_info->mutex);
}

STATIC int prof_alloc_device_sub_res_info(u32 dev_id)
{
    device_sub_resource_info[dev_id] =
        (struct prof_device_resource_info *)vzalloc(sizeof(struct prof_device_resource_info));
    if (device_sub_resource_info[dev_id] == NULL) {
        prof_err("Failed to invoke function [vzalloc] to apply the memory for subordinate resources. (dev_id=%u)\n",
            dev_id);
        return PROF_ERROR;
    }
    return PROF_OK;
}

STATIC void prof_free_device_sub_res_info(u32 dev_id)
{
    if (device_sub_resource_info[dev_id] != NULL) {
        vfree((unsigned char *)device_sub_resource_info[dev_id]);
        device_sub_resource_info[dev_id] = NULL;
    }
}

STATIC int prof_each_phy_device_init(u32 device_id)
{
    struct prof_device_info *prof_device = NULL;
    int ret;
    u32 i;
    u32 j;

    ret = prof_alloc_device_info(device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to alloc device info. (device_id=%u)\n", device_id);
        return ret;
    }

    ret = prof_alloc_device_sub_res_info(device_id);
    if (ret != PROF_OK) {
        prof_free_device_info(device_id);
        return ret;
    }

    prof_device = prof_device_info_get(device_id);
    prof_device->ts_num =
        devdrv_manager_get_ts_num_stub(devdrv_manager_get_devdrv_info(device_id));

    if (prof_device->ts_num == 0 || prof_device->ts_num > MDC_TSNUM) {
        prof_err("Invalid ts num. (device_id=%u; ts_num=%u)\n", device_id,
            prof_device->ts_num);
        prof_free_device_sub_res_info(device_id);
        prof_free_device_info(device_id);
        return PROF_ERROR;
    }

    ret = prof_drv_wait_ts(device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to make profile driver wait for TS response. Please check the status of TS."
            " (device_id=%u)\n", device_id);
        prof_free_device_sub_res_info(device_id);
        prof_free_device_info(device_id);
        return ret;
    }

    for (i = 0; i < prof_device->ts_num; i++) {
        ret = prof_cqsq_init(device_id, i);
        if (ret != PROF_OK) {
            prof_err("Failed to initialize the profile CQ and SQ. (device_id=%u)\n", device_id);
            goto cqsq_uninit;
        }
        mutex_init(&prof_device->cqsq_info[i].sq_mutex);
    }

    prof_phydev_sub_channel_resource_init(device_id);
    ret = prof_each_device_init(device_id, 1, 1);
    if (ret != PROF_OK) {
        prof_phydev_sub_channel_resource_uninit(device_id);
        prof_err("Failed to initialize device. (device_id=%u)\n", device_id);
        goto cqsq_uninit;
    }

#ifdef CFG_FEATURE_PROF_HWTS_THROUGH
    ret = prof_init_almost_full_irq(device_id, 0);
    if (ret != PROF_OK) {
        prof_each_device_uninit(device_id);
        prof_phydev_sub_channel_resource_uninit(device_id);
        prof_err("Failed to initialize the almost full irq. (device_id=%d)\n", device_id);
        goto cqsq_uninit;
    }
#endif

    return ret;

cqsq_uninit:
    for (j = i, i = 0; i < j; i++) {
        prof_cqsq_uninit(device_id, i);
        mutex_destroy(&prof_device->cqsq_info[i].sq_mutex);
    }
    prof_free_device_sub_res_info(device_id);
    prof_free_device_info(device_id);
    return ret;
}

STATIC void prof_each_phy_device_uninit(u32 device_id)
{
    struct prof_device_info *prof_device = prof_device_info_get(device_id);
    u32 i;

#ifdef CFG_FEATURE_PROF_HWTS_THROUGH
    prof_uninit_almost_full_irq(device_id, 0);
#endif

    prof_each_device_uninit(device_id);
    for (i = 0; i < prof_device->ts_num; i++) {
        prof_cqsq_uninit(device_id, i);
        mutex_destroy(&prof_device->cqsq_info[i].sq_mutex);
    }
    prof_phydev_sub_channel_resource_uninit(device_id);
    prof_free_device_sub_res_info(device_id);
    prof_free_device_info(device_id);
}

#define PROF_NOTIFIER "prof"
static int prof_notifier_func(u32 udevid, enum uda_notified_action action)
{
    int ret = 0;

    if (udevid >= PROF_DEVICE_NUM_VALUE) {
        prof_err("Invalid para. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    if (action == UDA_INIT) {
        ret = prof_each_phy_device_init(udevid);
    } else if (action == UDA_UNINIT) {
        prof_each_phy_device_uninit(udevid);
    }

    return ret;
}

int __init prof_drv_module_init(void)
{
#ifndef AOS_LLVM_BUILD
    struct uda_dev_type type;
#endif
    int ret;

    prof_init_hash();

#ifdef CFG_FEATURE_HOST_COLLECTION
    ret = prof_hdc_agent_init();
    if (ret != 0) {
        prof_err("Failed to initialize profile host agent. (ret=%d)\n", ret);
        goto prof_hdc_agent_init_fail;
    }
#endif

#ifndef AOS_LLVM_BUILD
    uda_davinci_local_real_entity_type_pack(&type);
    ret = uda_notifier_register(PROF_NOTIFIER, &type, UDA_PRI2, prof_notifier_func);
#else
    ret = prof_notifier_func(0, UDA_INIT);
#endif
    if (ret != 0) {
        prof_err("Register real notifier failed. (ret=%d)\n", ret);
        goto uda_notifier_register_fail;
    }

    ret = prof_drv_register_cdev();
    if (ret != PROF_OK) {
        prof_err("Failed to invoke function [prof_drv_register_cdev] to create character device.\n");
        goto prof_drv_register_cdev_fail;
    }

#ifdef CFG_FEATURE_SRIOV
    ret = dvprof_init();
    if (ret != PROF_OK) {
        prof_err("Failed to dvprof_init. (ret=%d)\n", ret);
        goto dvprof_init_fail;
    }
#endif

    prof_event("Profile driver module installation was success.\n");
    return PROF_OK;

#ifdef CFG_FEATURE_SRIOV
dvprof_init_fail:
    prof_drv_unregister_cdev();
#endif
prof_drv_register_cdev_fail:
#ifndef AOS_LLVM_BUILD
    (void)uda_notifier_unregister(PROF_NOTIFIER, &type);
#else
    prof_notifier_func(0, UDA_UNINIT);
#endif
uda_notifier_register_fail:
#ifdef CFG_FEATURE_HOST_COLLECTION
    prof_hdc_agent_uninit();
prof_hdc_agent_init_fail:
#endif

    return PROF_ERROR;
}

void __exit prof_drv_module_exit(void)
{
#ifndef AOS_LLVM_BUILD
    struct uda_dev_type type;
#endif

#ifdef CFG_FEATURE_SRIOV
    dvprof_uninit();
#endif

#ifndef AOS_LLVM_BUILD
    uda_davinci_local_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(PROF_NOTIFIER, &type);
#else
    prof_notifier_func(0, UDA_UNINIT);
#endif

#ifdef CFG_FEATURE_HOST_COLLECTION
    prof_hdc_agent_uninit();
#endif

    prof_drv_unregister_cdev();
    prof_event("Profile driver module uninstallation was success.\n");
}

module_init(prof_drv_module_init);
module_exit(prof_drv_module_exit);

MODULE_DESCRIPTION("prof driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
#else
int prof_drv_dev_ut_test(void)
{
    return 0;
}
module_init(prof_drv_dev_ut_test);
#endif
