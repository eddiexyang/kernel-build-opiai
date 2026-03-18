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

#ifndef __HISI_MAILBOX_H__
#define __HISI_MAILBOX_H__

#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/kfifo.h>
#include <linux/notifier.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/time.h>
#include <linux/errno.h>

#ifdef AOS_LLVM_BUILD
#include <linux/wait.h>
#include <linux/hashtable.h>
#endif

#include "drv_log.h"
#include "hisi_rproc_adapt.h"
#include "drv_snapshot.h"

#ifdef STATIC_SKIP
#define STATIC
#else
#ifndef STATIC
#define STATIC static
#endif
#endif

/* IPC Log module */
#define module_ipcdrv "ipcdrv"
#define ipcdrv_drv_err(fmt...) do { \
    drv_err(module_ipcdrv, fmt);   \
} while (0)
#define ipcdrv_drv_warn(fmt...) do { \
    drv_warn(module_ipcdrv, fmt);   \
} while (0)
#define ipcdrv_drv_info(fmt...) do { \
    drv_info(module_ipcdrv, fmt);   \
} while (0)
#define ipcdrv_drv_debug(fmt...) do { \
    drv_debug(module_ipcdrv, fmt);   \
} while (0)
#define ipcdrv_drv_event(fmt...) do { \
    drv_event(module_ipcdrv, fmt);   \
} while (0)

#define ipcdrv_drv_err_spinlock(fmt...)
#define ipcdrv_drv_warn_spinlock(fmt...)
#define ipcdrv_drv_info_spinlock(fmt...)
#define ipcdrv_drv_debug_spinlock(fmt...)


#define IRQ_IRQ_NONE_RETRY_TIME 100 /* if a irq error 100 times, disable it as it will always print */

/* Error number */
#define EMDEVCLEAN  1
#define EMDEVDIRTY  2
#define ETIMEOUT    3

/* mailbox check status */
#define RPUNCERTAIN     2
#define RPACCESSIBLE    1
#define RPUNACCESSIBLE  0

#define MDEV_SYNC_SENDING   (1 << 0)
#define MDEV_ASYNC_ENQUEUE  (1u << 1)
#define MDEV_DEACTIVATED    (1u << 2)

/* mbox tx&rx delay static */
#define MBOX_DELAY_LEVEL1  10
#define MBOX_DELAY_LEVEL2  50
#define MBOX_DELAY_LEVEL3 100
#define MBOX_DELAY_STATIC_PERIOD  (60000 * 5) // 5min

#ifdef CFG_SOC_PLATFORM_MINIV2_MDC
#ifdef AOS_LLVM_BUILD
#define IPC_TX_TS_BIND_CPU_ID 1
#else
#define IPC_TX_TS_BIND_CPU_ID 11
#endif
#endif

/* Alloc a static buffer for the tx_task, the num can be adjust according to different platform.
 * 512 allow 4 mdev-kfifo to be full fille (one mdev-kfifo can cache 256 tx_task node).
 */
#define TX_TASK_DDR_NODE_NUM        512
#define TX_TASK_DDR_NODE_VALID      0x0
#define TX_TASK_DDR_NODE_OCCUPIED   0x1

#define CONTINUOUS_FAIL_CNT_MAX 50
#define CONTINUOUS_FAIL_JUDGE (likely(g_ContinuousFailCnt < CONTINUOUS_FAIL_CNT_MAX))

#ifdef CFG_SOC_PLATFORM_ESL_FPGA
#define MAILBOX_AUTOACK_TIMEOUT msecs_to_jiffies(15000)
#define MAILBOX_MANUACK_TIMEOUT msecs_to_jiffies(15000)
#else
#define MAILBOX_AUTOACK_TIMEOUT msecs_to_jiffies(1500)
#define MAILBOX_MANUACK_TIMEOUT msecs_to_jiffies(1500)
#endif

#define IDLE_STATUS         (1 << 4)
#define SOURCE_STATUS       (1 << 5)
#define DESTINATION_STATUS  (1u << 6)
#define ACK_STATUS          (1u << 7)

#define TASK_DEBUG_ON(tx_task) do { \
} while (0)
#define TASK_DEBUG_OFF(tx_task) do { \
} while (0)
#define START_TTS(tx_task) do { \
} while (0)
#define SEND_TTS(tx_task) do { \
} while (0)
#define RECEIVE_TTS(tx_task) do { \
} while (0)
#define BH_TTS(tx_task) do { \
} while (0)
#define COMPLETE_TTS(tx_task) do { \
} while (0)
#define PRINT_TTS(tx_task) do { \
} while (0)

#define HISI_MBOX_WARN_ON(A) (A)

#define TX_FIFO_CELL_SIZE (sizeof(struct hisi_mbox_task *))

/* max 256 simultaneous tasks for every mailbox device */
#define MAILBOX_MAX_TX_BUFFER (256 * TX_FIFO_CELL_SIZE)

typedef enum {
    IPC_STARTUP_START = 0,
    IPC_STARTUP_PARSE_DTS_FINISH = 1,
    IPC_STARTUP_MBOX_REGISTER_FAIL = 2,
    IPC_STARTUP_RPROC_INIT_FAIL = 3,
    IPC_STARTUP_EXPECT = 50
} IPC_SNAPSHOT_STARTUP;

typedef enum {
    IPC_SUSPEND_START = 0,
    IPC_SUSPEND_EXPECT = 50
} IPC_SNAPSHOT_SUSPEND;

typedef enum {
    IPC_RESUME_START = 0,
    IPC_RESUME_EXPECT = 50
} IPC_SNAPSHOT_RESUME;

enum {
    NOCOMPLETION = 0,
    COMPLETING,
    COMPLETED
};

enum {
    TX_TASK = 0,
    RX_TASK
};

typedef enum {
    MANUAL_ACK = 0,
    AUTO_ACK
} mbox_ack_type_t;

typedef enum {
    TX_MAIL = 0,
    RX_MAIL,
    MAIL_TYPE_MAX
} mbox_mail_type_t;

typedef enum {
    ENABLE_IRQ_IN_INTERRUPT = 0,
    ENABLE_IRQ_IN_BH,
    ENABLE_IN_MAX
} mbox_irq_proc;

typedef u32 mbox_mesg_t;
typedef u32 mbox_msg_length;
typedef mbox_mesg_t rproc_mesg_t;
typedef mbox_msg_length rproc_msg_length;

struct hisi_mbox_task;
typedef void (*mbox_complete_t)(struct hisi_mbox_task *task);
typedef int (*mbox_irq_handler_t)(int irq, void *p);
typedef void (*rproc_complete_t)(rproc_mesg_t *ack_buffer, rproc_msg_length ack_buffer_len, int error, void *data);

struct mbox_proc_delay {
    unsigned int level1_num; /* [0 - 10ms) */
    unsigned int level2_num; /* [10 - 50ms) */
    unsigned int level3_num; /* [50 - 100ms) */
    unsigned int level4_num; /* over 100ms */
    unsigned int print_flag; /* if has over 10ms in period，print static */
};

struct hisi_mbox_task {
    /* use static memory to cache the async tx buffer */
    mbox_mesg_t tx_buffer[MBOX_CHAN_DATA_SIZE];
    /* alloc by mailbox core, shouldn't be free when a tx task complete by mailbox users */
    mbox_mesg_t *ack_buffer;
    mbox_msg_length tx_buffer_len;
    mbox_msg_length ack_buffer_len;
    int need_auto_ack;
    u8 tx_buffer_status;
    unsigned long send_timestamp;
};

struct hisi_mbox_recv_data {
    spinlock_t spinlock;
    struct list_head recv_data_list;
    u32 recv_data_num;
};

struct ipcdrv_recv_data {
    struct list_head list;
    u8 buffer[MAX_BUFFER_LEN];
    u32 size;
    unsigned long irq_timestamp;
};

struct hisi_mbox_device {
    struct hisi_mbox_dev_ops *ops;
    int ipc_id;
    rproc_id_t rproc_id;
    int thread_flag;
    int configured;
    int mbox_type;
    const char *name;
    struct list_head node;
    /* @stub_ipc_drv_01 */
    /* @stub_ipc_drv_01_TAIL */
    struct device *dev;

    void *priv;

    int cur_irq;
    volatile unsigned int status;
    spinlock_t status_lock;

    /* tx attributes */
    spinlock_t fifo_lock;
    struct kfifo fifo;

    struct mutex dev_lock;
    int enable_irq_stage;

    /* rx attributes */
    mbox_mesg_t *rx_buffer;
    mbox_mesg_t *ack_buffer;

    struct atomic_notifier_head notifier;
#ifdef AOS_LLVM_BUILD
    struct task_struct_out *tx_kthread;
#else
    struct task_struct *tx_kthread;
#endif
    struct tasklet_struct rx_bh;

    struct hisi_mbox_recv_data recv_data;

    wait_queue_head_t tx_wait;

    unsigned int rx_fifo_full_cnt;
    unsigned long pre_rx_print_timestamp;
    unsigned long pre_tx_print_timestamp;
    struct mbox_proc_delay rx_sched_delay;
    struct mbox_proc_delay rx_callback_delay;
    struct mbox_proc_delay tx_delay;
    struct mbox_proc_delay tx_sched_delay;
};

struct hisi_mbox_dev_ops {
    /* get ready */
    int (*startup)(struct hisi_mbox_device *mdev);
    void (*shutdown)(struct hisi_mbox_device *mdev);
    int (*check)(struct hisi_mbox_device *mdev, mbox_mail_type_t mtype, const char *rp_name);
    /* communication */
    mbox_msg_length (*recv)(struct hisi_mbox_device *mdev, mbox_mesg_t **msg);
    int (*send)(struct hisi_mbox_device *mdev, mbox_mesg_t *msg, mbox_msg_length len, int ack_mode);
    void (*ack)(struct hisi_mbox_device *mdev, mbox_mesg_t *msg, mbox_msg_length len);
    void (*refresh)(struct hisi_mbox_device *mdev);

    /* irq */
#ifdef AOS_LLVM_BUILD
    int (*hisi_request_irq)(struct hisi_mbox_device *mdev, irq_handler_t handler);
#else
    int (*request_irq)(struct hisi_mbox_device *mdev, irq_handler_t handler);
#endif
    void (*free_irq)(struct hisi_mbox_device *mdev, void *p);
    void (*enable_irq)(struct hisi_mbox_device *mdev);
    void (*disable_irq)(struct hisi_mbox_device *mdev);
    struct hisi_mbox_device *(*irq_to_mdev)(struct hisi_mbox_device *mdev, struct list_head *list, int irq);
    int (*is_stm)(struct hisi_mbox_device *mdev, unsigned int stm);
    void (*clr_ack)(struct hisi_mbox_device *mdev);
    void (*ensure_channel)(struct hisi_mbox_device *mdev);
    /* mntn */
    void (*status)(struct hisi_mbox_device *mdev);
};

struct hisi_mbox {
    const char *name;
    struct hisi_mbox_device *tx;
    struct hisi_mbox_device *rx;
    struct notifier_block *nb;
};

void mbox_task_free(int ipc_id, struct hisi_mbox_task **tx_task);
struct hisi_mbox_task *mbox_task_alloc(int ipc_id, const struct hisi_mbox *mbox, const char *rp_name,
    const mbox_mesg_t *tx_buffer, mbox_msg_length tx_buffer_len, int need_auto_ack);

/*
 * atomic context function
 */
int mbox_msg_send_async(struct hisi_mbox *mbox, const struct hisi_mbox_task *tx_task);

struct hisi_mbox *mbox_get(int ipc_id, const char *name, struct notifier_block *nb);
void hisi_mbox_put(struct hisi_mbox **mbox);

void mbox_device_activate(struct hisi_mbox_device **mdevs);
void mbox_device_deactivate(struct hisi_mbox_device **list);

int mbox_device_register(int ipc_id, struct device *parent, struct hisi_mbox_device **mdevs);
int mbox_device_unregister(struct hisi_mbox_device **list);

void mbox_empty_task(struct hisi_mbox_device *mdev);
void set_mbx_int_start_id(int dev_id, int var);
int mbox_init(void);
void hisi_mbox_exit(void);

void hisi_mdev_asm_sev(void);
void hisi_mdev_asm_wfe(void);

int mbox_is_exist_by_rp(int ipc_id, const char *rp_name);

#endif /* __HISI_MAILBOX_H__ */
