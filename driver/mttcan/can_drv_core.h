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

#ifndef _CAN_DRV_CORE_H__
#define _CAN_DRV_CORE_H__

#include <linux/net_tstamp.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/gfp.h>
#include <linux/delay.h>
#include <linux/kfifo.h>
#ifdef RUN_IN_AOS
#include <linux/timer.h>
#include <linux/timecounter.h>
#include <linux/interrupt.h>
#include <linux/skbuff.h>
#else
#include <linux/mempool.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#endif

#include "can_drv_mttcan.h"
#include "can_drv_fault_mgr.h"
#include "can_drv_sysfs.h"

#define TIMEBASE_PRE_MAX       256
#define TIMEBASE_BITS          32
#define DEFAULT_BITRATE        250000
#define COUNTS_PER_BERR        8
#define RESET_REG_LEN          8
#define PLATFORM_REG_LEN       8

#define PLAT_TYPE_SHIFT        16
#define PLAT_TYPE_MASK         0xFFFF0000U
#define PLAT_TYPE_FPGA         0x0
#define PLAT_TYPE_EMU          0x1
#define PLAT_TYPE_ESL          0x2
#define PLAT_TYPE_ASIC         0x4

#define CHIP_VERSION_V1       1
#define CHIP_VERSION_V2       2

#define FPGA_HOST_CLOCK_FREQ 25000000
#define FPGA_CORE_CLOCK_FREQ 25000000

/* according to 5 kbps, need 284 ms */
#define BUS_OFF_SEQ_MS         300
#define TIMER_PERIOD_MS        100
#define RESET_WIDTH_US         2

#define CAN_CTRL_MODE_MASK    0x03
#define NORMAL_WORK_MODE      0 /* normal work mode */
#define EXT_LOOPBACK_MODE     1 /* external loopback mode */
#define BUS_MONITOR_MODE      2 /* listen only mode */
#define INT_LOOPBACK_MODE     3 /* internal loopback mode */

#define AUDIT_LOG_MAX_LEN     256
#define MAX_CAN_NUN   15

/* can_bittiming_const of nominal */
#define TSEG1_MIN      1
#define TSEG1_MAX      256
#define TSEG2_MIN      1
#define TSEG2_MAX      128
#define BRP_MIN        1
#define BRP_MAX        512
#define BRP_INC        1
#define SJW_MAX        128

/* can_bittiming_const of data */
#define FD_TSEG1_MIN  1
#define FD_TSEG1_MAX  32
#define FD_TSEG2_MIN  1
#define FD_TSEG2_MAX  16
#define FD_BRP_MIN    1
#define FD_BRP_MAX    32
#define FD_BRP_INC    1
#define FD_SJW_MAX    16

#define BUSOFF_RECONVER_TIMEOUT_US  1000000
#define ERROR_PASSIVE_ERR_COUNTER   128

#define CAN_SDF_STUFF_LEN     48    /* CAN standard framed stuff is 47, Adapter canoe add 1 */
#define CAN_EXF_STUFF_LEN     67
#define CANFD_SDF_LS_STUFF_LEN   29 /* CANFD standard framed stuff is 30, Adapter canoe sub 1 */
#define CANFD_SDF_HS_STUFF_LEN   33
#define CANFD_EXF_LS_STUFF_LEN   47 /* CANFD extended framed stuff is 49, Adapter canoe sub 2 */
#define CANFD_EXF_HS_STUFF_LEN   33

#define SKB_KFIFO_SIZE              1024
#define PERF_RECORD_SIZE            40

#define BUSLOAD_MULTIPLES 100
#define BUSLOAD_PERCENT_MAX 100
#define RECV_THRESHOLD      (15 * 1000)  /* ns */
#define XMIT_THRESHOLD      (280 * 1000)  /* ns */

#define S_TO_MS  1000
#define MS_TO_US 1000
#define US_TO_NS 1000

#define TDC_FLAG_AUTOADAPTER    0
#define TDC_FLAG_DISABLE        1
#define TDC_FLAG_ENABLE         2

typedef enum {
    CAN_SUSPEND_START = 0,
    CAN_SUSPEND_STOP_WORK,
    CAN_SUSPEND_RESET_KFIFO,
    CAN_SUSPEND_EXPECT = 32
} CAN_SNAPSHOT_SUSPEND;

typedef enum {
    CAN_RESUME_START = 0,
    CAN_RESUMU_MODULE_INIT,
    CAN_RESUMU_BITRATE,
    CAN_RESUMU_SET_WORKMODE,
    CAN_RESUME_EXPECT = 32
} CAN_SNAPSHOT_RESUME;

struct element_config_size {
    u16 rxf0; /* rx fifo 0 */
    u16 rxf1; /* rx fifo 1 */
    u16 rxb;  /* rx buffer */
    u16 txb;  /* tx buffer */
};

struct element_config_param {
    u32 off; /* unit bytes */
    u32 num; /* unit elements */
};

struct frame_stats {
    u64 can_ext_packets;
    u64 can_std_packets;
    u64 can_payload_bits;

    u64 canfd_ext_brs_packets;
    u64 canfd_ext_brs_bits;
    u64 canfd_ext_normal_packets;
    u64 canfd_ext_normal_bits;

    u64 canfd_std_brs_packets;
    u64 canfd_std_brs_bits;
    u64 canfd_std_normal_packets;
    u64 canfd_std_normal_bits;

    s64 busload_last_time;
};

struct busload_warning_param {
    struct timer_list cheking_timer;
    u16 threshold;      /* percent   */
    u16 interval;       /* msec unit */
    u16 idle_times_to_clear;
    bool busload_warning;
    unsigned long continuous_idle_nums;
    struct frame_stats stats;  /* current can frame count */
};

struct busoff_config_param {
    u32 busoff_quick;
    u32 busoff_slow;
    u32 busoff_quick_times;
    u32 busoff_report_threshold;
};

typedef enum {
    CAN_RECV,
    CAN_XMIT,
    MTTCAN_ACTION_MAX,
} MTTCAN_PERF_ACTION;

struct delay_data {
    ktime_t start_time;
    s64 latency;
};

struct perf_record {
    s64 latency_threshold;
    struct delay_data data[PERF_RECORD_SIZE];
    u32 index;
};

struct mttcan_priv {
    struct can_priv can; /* can_priv must be first */
    struct device *dev;
    struct net_device *ndev;
    struct timer_list timer;
    struct cyclecounter cc;
    struct timecounter tc;
    raw_spinlock_t tc_lock; /* lock to protect timecounter tc */
    raw_spinlock_t tx_complete; /* lock to protect timecounter tc */
    spinlock_t tx_lock;     /* lock to get tx index */
    int rx_irq;
    int tx_irq;
    u32 status_ir0;
    u32 hpm_rxf1_mask;
    u32 dedicated_txb_mask;
    u64 timebase_rollover_ms;
    s64 busoff_ts;
    int busoff_restart_ms;
    u32 plat_type;
    u32 chip_version;
    struct mttcan_base_addr addr;
    struct element_config_size elmt_size; /* unit bytes */
    struct element_config_param elmt_param[ELMT_TYPE_MAX];
    u64 tx_obj;
    struct mttcan_config cfg;
    struct run_status_cnt status_cnt;
    struct run_status_cnt last_status_cnt;
    u32 busoff_counter;
    struct busoff_config_param busoff_cfg_param;
    bool busoff_warning;
#ifdef SUPPORT_MTTCAN_FAULT_RECOVERY
    struct timer_list fault_revovery_timer;
    u32  last_busy_tx_idx;
    long unsigned int last_busy_tx_time;
    long unsigned int last_netif_stop_time;

    struct list_head fault_event_list;
    struct work_struct fault_report_work;
#endif

#ifdef SUPPORT_MTTCAN_DFX
    struct busload_warning_param busload;
    u32    audit_log_count[CAN_LOG_MAX_VALUE];
    unsigned long  audit_log_last_time[CAN_LOG_MAX_VALUE];
    char   audit_log_last_msg[CAN_LOG_MAX_VALUE][AUDIT_LOG_MAX_LEN];
#endif

#ifdef RUN_IN_AOS
    struct kfifo rx_skb_kfifo;
    struct kfifo tx_skb_kfifo;
#else
    DECLARE_KFIFO_PTR(rx_skb_kfifo, struct sk_buff*);
    DECLARE_KFIFO_PTR(tx_skb_kfifo, struct sk_buff*);
#endif
    struct tasklet_struct rx_tasklet;
    struct tasklet_struct tx_tasklet;
    struct timer_list echo_sysfs_log_timer;
    struct perf_record can_perf_record[MTTCAN_ACTION_MAX];
    u32 tx_ts_lost_count;
    unsigned char tdc_flag;
    unsigned char tdco;
    unsigned char tdcf;
    unsigned char ktime_type;
};

struct net_device* get_net_devices(u32 can_id);

int mttcan_set_mram_parameter(struct mttcan_priv *priv, const struct mttcan_config *cfg);

int mttcan_init(const struct net_device *ndev);
bool mttcan_is_data_size_valid(long dszie);

int get_err_counter(const struct net_device *ndev, u32 *rx_err_cnt, u32 *tx_err_cnt);

static inline bool mttcan_tx_fifo_full(const struct mttcan_priv *priv)
{
    u32 txbrp = mttcan_read_reg(&priv->addr, REG_TXBRP);
    if (((~txbrp) & (priv->dedicated_txb_mask)) != 0) {
        return false;
    }
    return !!(mttcan_read_reg(&priv->addr, REG_TXFQS) & TXFQS_TFQF_BIT);
}

static inline u16 mttcan_tx_fifo_free_level(const struct mttcan_priv *priv)
{
    return (mttcan_read_reg(&priv->addr, REG_TXFQS) & TXFQS_TFFL_MASK);
}

static inline bool mttcan_tx_buff_free_less_one(const struct mttcan_priv *priv)
{
    u32 txbrp = mttcan_read_reg(&priv->addr, REG_TXBRP);
    u32 ded_txb_free = (~txbrp) & priv->dedicated_txb_mask;

    if (ded_txb_free == 0 || (ffs(ded_txb_free) == fls(ded_txb_free))) {
        return true;
    }
    return false;
}

static inline bool mttcan_working(const struct mttcan_priv *priv)
{
    return priv->can.state == CAN_STATE_ERROR_ACTIVE ||
        priv->can.state == CAN_STATE_ERROR_PASSIVE ||
        priv->can.state == CAN_STATE_ERROR_WARNING;
}

#ifdef SUPPORT_MTTCAN_DFX
/**
* need_print_audit_log - print audit log to log file
* @log_item [in] see enum can_log_item
*
* Collect statistics on each type of error.
* If an error occurs within one second, the log of the error is printed only once.
* @Return: whether to record logs.
*/
bool need_print_audit_log(struct mttcan_priv *priv, enum can_log_item log_item);

/**
* mtcan_print_oplog: print operation log
*/
#define mttcan_print_oplog(priv, log_item, fmt, ...) do {                                                    \
        if (snprintf_s(priv->audit_log_last_msg[log_item], AUDIT_LOG_MAX_LEN, AUDIT_LOG_MAX_LEN - 1,        \
                "[ascend][mttcan][%s:%d][%s] opcode:0x%x, times:%u, detail:" fmt, __func__, __LINE__,  \
                priv->ndev->name, TO_OPLOG(log_item), priv->audit_log_count[log_item]++, ##__VA_ARGS__) >= 0) { \
            if (need_print_audit_log(priv, log_item)) {                                                      \
                mttcan_info("%s", priv->audit_log_last_msg[log_item]);                                       \
                (void)memset_s(priv->audit_log_last_msg[log_item], AUDIT_LOG_MAX_LEN, 0, AUDIT_LOG_MAX_LEN); \
            }                                                                                               \
        }                                                                                                   \
    } while (0)

/**
* mtcan_print_oplog: print security log
*/
#define mttcan_print_seclog(priv, log_item, fmt, ...) do {                                                  \
        if (snprintf_s(priv->audit_log_last_msg[log_item], AUDIT_LOG_MAX_LEN, AUDIT_LOG_MAX_LEN - 1,        \
                "[ascend][mttcan][%s:%d][%s] opcode:0x%x, times:%u, detail:" fmt, __func__, __LINE__,       \
                priv->ndev->name, TO_SECLOG(log_item), priv->audit_log_count[log_item]++, ##__VA_ARGS__) >= 0) { \
            if (need_print_audit_log(priv, log_item)) {                                                      \
                mttcan_info("%s", priv->audit_log_last_msg[log_item]);                                       \
                (void)memset_s(priv->audit_log_last_msg[log_item], AUDIT_LOG_MAX_LEN, 0, AUDIT_LOG_MAX_LEN); \
            }                                                                                               \
        }                                                                                                   \
    } while (0)

/**
* mtcan_print_runlog: print run log
*/
#define mttcan_print_runlog(priv, log_item, fmt, ...) do {                                                  \
        if (snprintf_s(priv->audit_log_last_msg[log_item], AUDIT_LOG_MAX_LEN, AUDIT_LOG_MAX_LEN - 1,        \
                "[ascend][mttcan][%s:%d][%s] opcode:0x%x, times:%u, detail:" fmt, __func__, __LINE__,       \
                priv->ndev->name, TO_RUNLOG(log_item), priv->audit_log_count[log_item]++, ##__VA_ARGS__) >= 0) { \
            if (need_print_audit_log(priv, log_item)) {                                                      \
                mttcan_info("%s", priv->audit_log_last_msg[log_item]);                                       \
                (void)memset_s(priv->audit_log_last_msg[log_item], AUDIT_LOG_MAX_LEN, 0, AUDIT_LOG_MAX_LEN); \
            }                                                                                               \
        }                                                                                                   \
    } while (0)

#else
#define mttcan_print_oplog(priv, log_item, fmt, ...)  \
    mttcan_info(fmt, ##__VA_ARGS__)
#define mttcan_print_seclog(priv, log_item, fmt, ...) \
    mttcan_info(fmt, ##__VA_ARGS__)
#define mttcan_print_runlog(priv, log_item, fmt, ...) \
    mttcan_info(fmt, ##__VA_ARGS__)
#endif

#endif    /* _CAN_DRV_CORE_H__ */
