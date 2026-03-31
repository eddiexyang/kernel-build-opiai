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

#ifndef _HDCDRV_CORE_COM_H_
#define _HDCDRV_CORE_COM_H_

#include "hdcdrv_cmd.h"
#include "drv_log.h"
#include "hdcdrv_mem_com.h"

#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/export.h>
#include <linux/spinlock.h>
#include <linux/time.h>

#ifdef HDC_ENV_DEVICE
#include <linux/securec.h>
#include "kernel_version_adapt.h"
#else
#include <linux/version.h>
#include "securec.h"
#endif

#ifndef __GFP_ACCOUNT

#ifdef __GFP_KMEMCG
#define __GFP_ACCOUNT __GFP_KMEMCG /* for linux version 3.10 */
#endif

#ifdef __GFP_NOACCOUNT
#define __GFP_ACCOUNT 0 /* for linux version 4.1 */
#endif

#endif

#ifdef CFG_BUILD_DEBUG
#define EXPORT_SYMBOL_UNRELEASE(symbol) EXPORT_SYMBOL(symbol)
#else
#define EXPORT_SYMBOL_UNRELEASE(symbol)
#endif

#define module_hdcdrv "hdcdrv"

#define HDC_SHARE_LOG_START (0xE0000060000ULL)
#ifdef DRV_UT
#define drv_err_log(fmt, ...) do { \
    drv_err(module_hdcdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0)
#else
#ifdef HDC_ENV_DEVICE
#define drv_err_log(fmt, ...) do { \
    drv_err(module_hdcdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0)
#else
#define drv_err_log(fmt, ...) do { \
    drv_err(module_hdcdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    share_log_err(HDC_SHARE_LOG_START, fmt, ##__VA_ARGS__); \
} while (0)
#endif
#endif

static u64 g_log_count = 0;
#define LOG_MS_PER_SECOND  1000
#define LOG_US_PER_MSECOND 1000
#define LOG_NS_PER_MSECOND 1000000
#define LOG_LIMIT_RATE 30

static inline u64 hdc_get_time_interval(u64 *caller_jiffies, u64 *time_now)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    struct timespec64 now;
    ktime_get_real_ts64(&now);
    *time_now = (u64)(now.tv_sec * LOG_MS_PER_SECOND + now.tv_nsec / LOG_NS_PER_MSECOND);
#else
    struct timeval now;
    do_gettimeofday(&now);
    *time_now = (u64)(now.tv_sec * LOG_MS_PER_SECOND + now.tv_usec / LOG_US_PER_MSECOND);
#endif
    return (*time_now - *caller_jiffies);
}

static inline bool hdc_print_timed_ratelimit(u64 *caller_jiffies, u64 interval_msecs)
{
    u64 time_now, timer;

    timer =  hdc_get_time_interval(caller_jiffies, &time_now);
    if ((*caller_jiffies != 0) && (timer <= interval_msecs)) {
        return false;
    }
    *caller_jiffies = time_now;

    return true;
}

static inline bool hdc_log_rate_limit(u32 *count, u64 *last_jiffies, u32 branch_rate, u64 limit_time)
{
    if (hdc_print_timed_ratelimit(last_jiffies, limit_time)) {
        *count = 0;
        g_log_count = 0;
        return false;
    } else {
        (*count)++;
        g_log_count++;
        return  ((*count >= branch_rate)  || (g_log_count  >=  LOG_LIMIT_RATE));
    }
}

#define HDC_LOG_LIMIT_TIME 3000       /* 3s */
#define HDC_LOG_LIMIT_BRANCH_RATE 1   /* print 1 counts per 3s */
#define HDC_LOG_ERR_LIMIT(print_cnt, last_jiffies, fmt, ...) do { \
    if (!hdc_log_rate_limit(print_cnt, last_jiffies, HDC_LOG_LIMIT_BRANCH_RATE, HDC_LOG_LIMIT_TIME)) \
        drv_err_log(fmt, ##__VA_ARGS__); \
} while (0);

#define hdcdrv_err_limit(fmt, ...) do { \
    if (printk_ratelimit() != 0) \
        drv_err_log(fmt, ##__VA_ARGS__); \
} while (0);
#define hdcdrv_warn_limit(fmt, ...) do { \
    if (printk_ratelimit() != 0) \
        drv_warn(module_hdcdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0);
#define hdcdrv_info_limit(fmt, ...) do { \
    if (printk_ratelimit() != 0) \
        drv_info(module_hdcdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0);
#define hdcdrv_err(fmt, ...) do { \
    drv_err_log(fmt, ##__VA_ARGS__); \
} while (0);
#define hdcdrv_warn(fmt, ...) do { \
    drv_warn(module_hdcdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0);
#define hdcdrv_event(fmt, ...) do { \
    drv_event(module_hdcdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0);
#define hdcdrv_info(fmt, ...) do { \
    drv_info(module_hdcdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0);
#define hdcdrv_critical_info(fmt, ...) do { \
    drv_err_log(fmt, ##__VA_ARGS__); \
} while (0);

#define hdcdrv_dbg(fmt, ...) do { \
    drv_debug(module_hdcdrv, "[DEBUG]<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0);

#ifdef DRV_UT
#define STATIC
#else
#define STATIC static
#endif

#define hdcdrv_err_limit_spinlock(fmt, ...)
#define hdcdrv_warn_limit_spinlock(fmt, ...)
#define hdcdrv_info_limit_spinlock(fmt, ...)
#define hdcdrv_err_spinlock(fmt, ...)
#define hdcdrv_warn_spinlock(fmt, ...)
#define hdcdrv_info_spinlock(fmt, ...)
#define hdcdrv_critical_info_spinlock(fmt, ...)
#define hdcdrv_dbg_spinlock(fmt, ...)

#ifdef HDC_ENV_DEVICE
#define HDCDRV_DEV_MODE 0600
#else
#define HDCDRV_DEV_MODE 0640
#endif

#define HDCDRV_VALID 1
#define HDCDRV_INVALID 0
#define HDCDRV_HOTRESET_FLAG_SET 1
#define HDCDRV_HOTRESET_FLAG_UNSET 0

#define HDCDRV_CDEV_COUNT   1

#define HDCDRV_MEM_POOL_TYPE_TX 0
#define HDCDRV_MEM_POOL_TYPE_RX 1
#define HDCDRV_MEM_POOL_TYPE_NUM 2

#define HDCDRV_PACKET_SEGMENT (32 * 1024)
#define HDCDRV_INVALID_PACKET_SEGMENT (-1)
#define HDCDRV_INVALID_HDC_VERSION (-1)

#define HDCDRV_RBTREE_PID 16
#define HDCDRV_RBTREE_CNT_BIT 24
#define HDCDRV_RBTREE_PID_MASK 0x000000000000FFFFull
#define HDCDRV_RBTREE_ADDR_MASK_H 0x0000FFFFFFFFFFFFull
#define HDCDRV_RBTREE_ADDR_MASK_L 0xFFFFFFFFFFFF0000ull

#define HDCDRV_SMALL_PACKET_SEGMENT (4 * 1024)
#define HDCDRV_HUGE_PACKET_SEGMENT (512 * 1024)
#ifdef CFG_FEATURE_MIRROR
#define HDCDRV_SMALL_PACKET_NUM 512 /* power of 2 */
#else
#define HDCDRV_SMALL_PACKET_NUM 1024u /* power of 2 */
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V11
# define HDCDRV_HUGE_PACKET_NUM 2u    /* power of 2 */
#else
# define HDCDRV_HUGE_PACKET_NUM 64    /* power of 2 */
#endif


#define HDCDRV_USLEEP_RANGE_2000 2000
#define HDCDRV_USLEEP_RANGE_3000 3000

#define HDCDRV_LIST_CACHE   32

/*
 * 5a5a is magic
 * For example, add a new version number
 * #define HDC_VERSION_0002 0x5a5a0002u
 * #define HDC_VERSION HDC_VERSION_0002
 */
#define HDC_VERSION_0000 0
#define HDC_VERSION_0001 0x5a5a0001u    // HDC support mutiply pkt recv
#define HDC_VERSION_0002 0x5a5a0002u    // HDC support service query session dfx
#define HDC_VERSION HDC_VERSION_0002    // HDC Current Version

extern u32 hdcdrv_cmd_size_table[HDCDRV_CMD_MAX];

struct hdcdrv_cdev {
    struct cdev cdev;
    dev_t dev_no;
    struct class *cdev_class;
    struct device *dev;
};

enum VHDC_CTRL_MSG_TYPE {
    VHDC_CTRL_MSG_TYPE_SEGMENT = 1,
    VHDC_CTRL_MSG_TYPE_OPEN,
    VHDC_CTRL_MSG_TYPE_RELEASE,
    VHDC_CTRL_MSG_TYPE_ALLOC_MEM,
    VHDC_CTRL_MSG_TYPE_FREE_MEM,
    VHDC_CTRL_MSG_TYPE_ALLOC_HUGE,
    VHDC_CTRL_MSG_TYPE_FREE_HUGE,
    VHDC_CTRL_MSG_TYPE_MEMCOPY,
    VHDC_CTRL_MSG_TYPE_POOL_CHECK,
    VHDC_CTRL_MSG_TYPE_HDC_VERSION,
    VHDC_CTRL_MSG_TYPE_MAX
};

struct vhdc_ctrl_msg_get_segment {
    int segment;
};

struct vhdc_ctrl_msg_open {
};

struct vhdc_ctrl_msg_release {
    unsigned long long hash;
};

struct vhdca_alloc_mem_para {
    int pool_type;
    int dev_id;
    int len;
    u32 fid;
};

struct vhdc_ctrl_msg_alloc_mempool {
    struct vhdca_alloc_mem_para mem_para;
    void *buf;
    u64 addr;
};

struct vhdc_ctrl_msg_free_mempool {
    void *buf;
};

struct vhdc_ctrl_msg_alloc_huge {
};
struct vhdc_ctrl_msg_free_huge {
};

struct vhdc_ctrl_msg_memcopy {
    void *dest;
    unsigned long destMax;
    const void *src;
    unsigned long count;
    int mode;
};

struct vhdc_ctrl_msg_pool_check {
    u32 size;
    u32 segment;
    u32 sg_cnt;
    dma_addr_t addr[HDCDRV_HUGE_PACKET_NUM];
    unsigned char map[HDCDRV_HUGE_PACKET_NUM];
};

struct vhdc_ctrl_msg_hdc_version {
    int pm_version;
    int vm_version;
};

struct vhdc_ctrl_msg {
    enum VHDC_CTRL_MSG_TYPE type;
    int error_code;
    union {
        struct vhdc_ctrl_msg_get_segment vhdc_segment;
        struct vhdc_ctrl_msg_open vhdc_open;
        struct vhdc_ctrl_msg_release vhdc_release;
        struct vhdc_ctrl_msg_alloc_mempool alloc_mempool;
        struct vhdc_ctrl_msg_free_mempool free_mempool;
        struct vhdc_ctrl_msg_alloc_huge alloc_huge;
        struct vhdc_ctrl_msg_free_huge free_huge;
        struct vhdc_ctrl_msg_memcopy mem_copy;
        struct vhdc_ctrl_msg_pool_check pool_check;
        struct vhdc_ctrl_msg_hdc_version hdc_version;
    };
};

struct vhdc_ioctl_msg {
    unsigned int cmd;
    unsigned long long hash;
    int copy_flag;
    int error_code;
    union hdcdrv_cmd cmd_data;
};

struct hdcdrv_dma_mem {
    u64 addr;
    u32 len;
    u32 resv;
};

struct hdcdrv_ctrl_msg_sync_mem_info {
    int type;
    int error_code;
    int flag;
    int phy_addr_num;
    int mem_type;
    u32 alloc_len;
    u64 hash_va;
    long long pid;
#ifdef CFG_FEATURE_HDC_REG_MEM
    u32 align_size;
    u32 register_offset;
    u64 user_va;
#endif
    struct hdcdrv_dma_mem mem[0];
};

struct hdc_hotreset_task_info {
    int dev_id;
    int hdc_valid;
    int hotreset_flag;
    int msg_chan_refcnt;
    spinlock_t task_rw_lock;
};

/* ctrl msg type */
#define HDCDRV_CTRL_MSG_TYPE_CONNECT 1U
#define HDCDRV_CTRL_MSG_TYPE_CONNECT_REPLY 2
#define HDCDRV_CTRL_MSG_TYPE_CLOSE 3
#define HDCDRV_CTRL_MSG_TYPE_SYNC 4
#define HDCDRV_CTRL_MSG_TYPE_RESET 5
#define HDCDRV_CTRL_MSG_TYPE_CHAN_SET 6
#define HDCDRV_CTRL_MSG_TYPE_SYNC_MEM_INFO 7
#define HDCDRV_CTRL_MSG_TYPE_GET_DEV_SESSION_STAT 8
#define HDCDRV_CTRL_MSG_TYPE_GET_DEV_CHAN_STAT 9
#define HDCDRV_CTRL_MSG_TYPE_GET_DEV_LINK_STAT 10

/* ctrl msg status */
enum HDC_LINK_CTRL_MSG_STATUS_TYPE {
    HDCDRV_LINK_CTRL_MSG_SEND_SUCC = 0,
    HDCDRV_LINK_CTRL_MSG_SEND_FAIL = 1,
    HDCDRV_LINK_CTRL_MSG_RECV_SUCC = 2,
    HDCDRV_LINK_CTRL_MSG_RECV_FAIL = 3,
    HDCDRV_LINK_CTRL_MSG_WAIT_SUCC = 4,
    HDCDRV_LINK_CTRL_MSG_WAIT_TIMEOUT = 5,
    HDCDRV_LINK_CTRL_MSG_STATUS_MAX
};

struct hdcdrv_link_ctrl_msg_stats {
    u32 count[HDCDRV_LINK_CTRL_MSG_STATUS_MAX];
    int last_err[HDCDRV_LINK_CTRL_MSG_STATUS_MAX];
};

#ifdef CFG_FEATURE_SIMPLIFY_MODE
#define HDCDRV_NON_TRANS_MSG_S_DESC_SIZE 0x10000 /* 64k, pcie support */
#define HDCDRV_NON_TRANS_MSG_C_DESC_SIZE 0x10000 /* 64k, pcie support */
#else
#define HDCDRV_NON_TRANS_MSG_S_DESC_SIZE 0x100000 /* 1M */
#define HDCDRV_NON_TRANS_MSG_C_DESC_SIZE 0x100000 /* 1M */
#endif

/* reserved 1k mem for non trans msg head and hdc ctrl msg head */
#define HDCDRV_MAX_DMA_NODE ((int)((HDCDRV_NON_TRANS_MSG_S_DESC_SIZE - 0x400) / sizeof(struct hdcdrv_dma_mem)))
#define HDCDRV_CTRL_MEM_MAX_PHY_NUM (HDCDRV_CTRL_MEM_MAX_LEN / HDCDRV_MEM_MIN_LEN)
#define HDCDRV_MEM_MAX_PHY_NUM (HDCDRV_MAX_DMA_NODE - HDCDRV_CTRL_MEM_MAX_PHY_NUM)

#define HDCDRV_NODE_IDLE 0
#define HDCDRV_NODE_BUSY 1
#define HDCDRV_ADD_FLAG 0
#define HDCDRV_DEL_FLAG 1
#define HDCDRV_ADD_REGISTER_FLAG 2
#define HDCDRV_DEL_REGISTER_FLAG 3
#define HDCDRV_FALSE_FLAG 0
#define HDCDRV_TRUE_FLAG 1

#define HDCDRV_RBTREE_SIDE_LOCAL 0
#define HDCDRV_RBTREE_SIDE_REMOTE 1

#define HDCDRV_RUNNING_ENV_ARM_3559 0
#define HDCDRV_RUNNING_ENV_X86_NORMAL 1
#define HDCDRV_RUNNING_ENV_DAVICI 2

#define HDCDRV_KERNEL_DEFAULT_PID 0x7FFFFFFFULL
#define HDCDRV_INVALID_PID 0x7FFFFFFEULL
#define HDCDRV_RAW_PID_MASK 0xFFFFFFFFULL


#ifdef CFG_SOC_PLATFORM_ESL_FPGA
#define HDCDRV_NODE_BUSY_WARING     2000 /* 2s for fpga */
#define HDCDRV_NODE_BUSY_TIMEOUT    50000 /* 50s for fpga */

#define HDCDRV_NODE_WAIT_TIME_MIN   0
#define HDCDRV_NODE_WAIT_TIME_MAX   30000 /* 30s for fpga */

#define HDCDRV_SESSION_DEFAULT_TIMEOUT 120000 /* 120s for fpga */
#define HDCDRV_SESSION_REMOTE_CLOSED_TIMEOUT (10 * HZ) /* 10s for fpga */
#define HDCDRV_SESSION_REMOTE_CLOSED_TIMEOUT_MS 10000 /* 10s for fpga */
#define HDCDRV_CONN_TIMEOUT (100 * HZ) /* 100s for fpga */
#define HDCDRV_SESSION_RECLAIM_TIMEOUT (150 * HZ) /* 150s for fpga */

#define HDCDRV_TASKLET_STATUS_CHECK_TIME 10 /* 10s for fpga */
#else
#define HDCDRV_NODE_BUSY_WARING     100 /* ms */
#define HDCDRV_NODE_BUSY_TIMEOUT    5000 /* ms */

#define HDCDRV_NODE_WAIT_TIME_MIN   0
#define HDCDRV_NODE_WAIT_TIME_MAX   3000 /* ms */

#define HDCDRV_SESSION_DEFAULT_TIMEOUT 3000 /* ms */
#define HDCDRV_SESSION_REMOTE_CLOSED_TIMEOUT (3 * HZ) /* 3s */
#define HDCDRV_SESSION_REMOTE_CLOSED_TIMEOUT_MS 3000 /* 3s */
#define HDCDRV_CONN_TIMEOUT (30 * HZ) /* 30s */
#define HDCDRV_SESSION_RECLAIM_TIMEOUT (60 * HZ) /* 60s */

#define HDCDRV_TASKLET_STATUS_CHECK_TIME 3
#endif


extern u64 hdcdrv_get_hash(u64 user_va, u64 pid, u32 fid);
extern u32 hdcdrv_get_mem_list_len(void);
extern void hdcdrv_node_status_busy(struct hdcdrv_fast_node *node);
extern void hdcdrv_node_status_idle(struct hdcdrv_fast_node *node);
extern void hdcdrv_node_status_idle_by_mem(struct hdcdrv_fast_mem *f_mem);
extern bool hdcdrv_node_is_timeout(int node_stamp);
extern bool hdcdrv_node_is_busy(const struct hdcdrv_fast_node *node);
extern int hdcdrv_send_mem_info(struct hdcdrv_fast_mem *mem, int devid, int flag);
extern int hdcdrv_get_running_env(void);
extern void hdcdrv_unbind_mem_ctx(struct hdcdrv_fast_node *f_node);

extern long hdcdrv_non_trans_ctrl_msg_send(u32 devid, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len);

int hdccom_register_cdev(struct hdcdrv_cdev *hcdev, const struct file_operations *fops);
void hdccom_free_cdev(struct hdcdrv_cdev *hcdev);
u64 hdcdrv_get_pid(void);
u64 hdcdrv_get_ppid(void);
int hdcdrv_rebuild_raw_pid(u64 pid);
struct mutex *hdcdrv_get_sync_mem_lock(int dev_id);
extern bool hdcdrv_mem_is_notify(const struct hdcdrv_fast_mem *f_mem);
void* hdcdrv_get_sync_mem_buf(int dev_id);
void hdcdrv_release_free_mem(struct hdcdrv_ctx_fmem *ctx_fmem);
extern void hdcdrv_fast_mem_free_abnormal(const struct hdcdrv_fast_node *f_node);
long hdcdrv_bind_mem_ctx(struct hdcdrv_ctx_fmem *ctx_fmem, struct hdcdrv_fast_node *f_node);
int hdcdrv_mmap_param_check(const struct file *filep, const struct vm_area_struct *vma);

struct page *hdcdrv_alloc_pages_node(u32 dev_id, gfp_t gfp_mask, u32 order);
int hdcdrv_get_running_status(void);

#endif /* _HDCDRV_CORE_COM_H_ */
