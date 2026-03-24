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

#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/pm_runtime.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/mod_devicetable.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kern_levels.h>
#include <linux/msi.h>
#include <linux/property.h>
#include <linux/acpi.h>
#include <linux/cpumask.h>
#if (defined(CFG_SOC_PLATFORM_MDC_V51))
#include <linux/aos/cpu_domain_info.h>
#endif

#include "hisi_mailbox_dev.h"
#include "hisi_rproc.h"

#ifdef AOS_LLVM_BUILD
#include <linux/hashtable.h>
#include <linux/export.h>
#include "aos_dlib.h"
#define __raw_writel(value, addr) writel(value, addr)
#define __raw_readl(addr) readl(addr)
#endif

#define MODULE_NAME "hisi_mailbox_dev"
#define OUTPUT_LENGTH 3
#define FUNC_CHAN_MODE_INDEX 0
#define FUNC_SRC_MODE_INDEX 1
#define FUNC_DST_MODE_INDEX 2

#define IRQINDEX_LENGTH 4
#define CMBOX_GIC_1_INDEX 0
#define CMBOX_GIC_2_INDEX 1
#define CMBOX_GIC_3_INDEX 2
#define CMBOX_GIC_4_INDEX 3

/* MBX_MODE:
 * HiIPCV230 have a state machine, the state machine have 4 status:
 * 4'b0001 : IDLE_STATE
 * 4'b0010 : SOURCE_STATE
 * 4'b0100 : DEST_STATE
 * 4'b1000 : ACK_STATE
 */
STATIC struct list_head g_mbox_irq_list;
STATIC int g_ipc_mdevs_index = 0;

enum IPC_STATE_MACHINE {
    IDLE_STATE,
    SOURCE_STATE,
    DEST_STATE,
    ACK_STATE
};

STATIC int __ipc_write_with_check(void __iomem *addr, unsigned int value, unsigned int check_value)
{
    int retry_times = IPC_WRITE_CHECK_RETRY_TIMES;
    unsigned int reg;

    do {
        __raw_writel(value, addr);

        reg = __raw_readl(addr);
        if (reg == check_value) {
            return 0;
        }

        retry_times--;
    } while (retry_times);

    return -ENODEV;
}

STATIC inline void __ipc_lock(void __iomem *base, unsigned int lock_key)
{
    int ret;

    ret = __ipc_write_with_check(base + IPCLOCK(), lock_key, IPC_LOCKED);
    if (ret != 0) {
        ipcdrv_drv_warn("ipc lock abnormal\n");
    }
}

STATIC inline void __ipc_unlock(void __iomem *base, unsigned int unlock)
{
    int ret;

    ret = __ipc_write_with_check(base + IPCLOCK(), unlock, IPC_UNLOCKED);
    if (ret != 0) {
        ipcdrv_drv_warn("ipc unlock abnormal\n");
    }
}

STATIC inline unsigned int __ipc_lock_status(const void __iomem *base)
{
    return __raw_readl(base + IPCLOCK());
}

STATIC inline void __ipc_set_src(void __iomem *base, u32 source, unsigned int mdev)
{
    __raw_writel(IPCBITMASK(source), base + (long)(unsigned)IPCMBxSOURCE(mdev));
}

STATIC inline unsigned int __ipc_read_src(const void __iomem *base, unsigned int mdev)
{
    return __raw_readl(base + (long)(unsigned)IPCMBxSOURCE(mdev));
}

STATIC inline void __ipc_set_des(void __iomem *base, u32 source, unsigned int mdev)
{
    __raw_writel(IPCBITMASK(source), base + (long)(unsigned)IPCMBxDSET(mdev));
}

STATIC inline void __ipc_clr_des(void __iomem *base, u32 source, unsigned int mdev)
{
    __raw_writel(IPCBITMASK(source), base + (long)(unsigned)IPCMBxDCLR(mdev));
}

STATIC inline unsigned int __ipc_des_status(const void __iomem *base, unsigned int mdev)
{
    return __raw_readl(base + (long)(unsigned)IPCMBxDSTATUS(mdev));
}

STATIC inline void __ipc_send(void __iomem *base, unsigned int tosend, unsigned int mdev)
{
    __raw_writel(tosend, base + (long)(unsigned)IPCMBxSEND(mdev));
}

STATIC inline unsigned int __ipc_read(const void __iomem *base, unsigned int mdev, unsigned int index)
{
    return __raw_readl(base + (long)(unsigned)IPCMBxDATA(mdev, index));
}

STATIC inline void __ipc_write(void __iomem *base, u32 data, unsigned int mdev, unsigned int index)
{
    __raw_writel(data, base + (long)(unsigned)IPCMBxDATA(mdev, index));
}

STATIC inline unsigned int __ipc_cpu_imask_get(const void __iomem *base, unsigned int mdev)
{
    return __raw_readl(base + (long)(unsigned)IPCMBxIMASK(mdev));
}

STATIC inline void __ipc_cpu_imask_clr(void __iomem *base, unsigned int toclr, unsigned int mdev)
{
    unsigned int reg;
    int ret;

    reg = __raw_readl(base + (long)(unsigned)IPCMBxIMASK(mdev));
    reg = reg & (~(toclr));

    ret = __ipc_write_with_check(base + (long)(unsigned)IPCMBxIMASK(mdev), reg, reg);
    if (ret != 0) {
        ipcdrv_drv_warn("cpu imask abnormal.\n");
    }
}

STATIC inline void __ipc_cpu_imask_all(void __iomem *base, unsigned int mdev)
{
    __raw_writel((~0), base + (long)(unsigned)IPCMBxIMASK(mdev));
}

STATIC inline void __ipc_cpu_iclr(void __iomem *base, unsigned int toclr, unsigned int mdev)
{
    __raw_writel(toclr, base + (long)(unsigned)IPCMBxICLR(mdev));
}

STATIC inline int __ipc_cpu_istatus(const void __iomem *base, unsigned int mdev)
{
    return __raw_readl(base + (long)(unsigned)IPCMBxICLR(mdev));
}

STATIC inline unsigned int __ipc_mbox_istatus(const void __iomem *base, unsigned int cpu)
{
    return __raw_readl(base + (long)(unsigned)IPCCPUxIMST(cpu));
}

STATIC inline unsigned int __ipc_mbox_istatus_2(const void __iomem *base, unsigned int cpu, unsigned int n)
{
    return __raw_readl(base + (long)(unsigned)IPCCPUxIMST2(cpu, n >> 5));
}

STATIC inline unsigned int __ipc_mbox_irstatus(const void __iomem *base, unsigned int cpu)
{
    return __raw_readl(base + (long)(unsigned)IPCCPUxIRST(cpu));
}

STATIC inline unsigned int __ipc_status(const void __iomem *base, unsigned int mdev)
{
    return __raw_readl(base + (long)(unsigned)IPCMBxMODE(mdev));
}

STATIC inline void __ipc_mode(void __iomem *base, unsigned int mode, unsigned int mdev)
{
    __raw_writel(mode, base + (long)(unsigned)IPCMBxMODE(mdev));
}

STATIC void ipc_clear_lpi_active(void __iomem *base, unsigned int index)
{
#ifdef CFG_SOC_PLATFORM_CLOUD
    unsigned int mbix_offset;
    unsigned int value;

    /* Clear LPI interrupt */
    mbix_offset = ((index + MIIX_PIN_BASE) / 32) * 4;
    value = (1 << ((index + MIIX_PIN_BASE) % 32));

    __raw_writel(value, base + (long)mbix_offset);
#endif
    return;
}

void sync_ipc_clear_lpi(struct hisi_rproc_info *rproc)
{
#ifdef CFG_SOC_PLATFORM_CLOUD
    struct hisi_mbox_device_priv *priv = NULL;
    struct hisi_mbox_device *mdev = NULL;
    unsigned int index;

    if ((rproc == NULL) || (rproc->mbox == NULL) || (rproc->mbox->tx == NULL) ||
        (rproc->symmetry_id == HISI_RPROC_MAX)) {
        ipcdrv_drv_err("sync_ipc_clear_lpi failed. (rproc == NULL)=%d, (rproc->mbox == NULL)=%d.\n",
            (rproc == NULL), ((rproc == NULL) ? -EFAULT : (rproc->mbox == NULL)));
        return;
    }

    mdev = rproc->mbox->tx;
    if (((priv = mdev->priv) == NULL) || (priv->idev == NULL) || (priv->idev->lpi_base == NULL)) {
        ipcdrv_drv_err("get lpi base failed. (priv == NULL)=%d, (priv->idev == NULL)=%d.\n",
            (priv == NULL), ((priv == NULL) ? -EFAULT : (priv->idev == NULL)));
        return;
    }

    index = rproc->symmetry_id + MBIX_IPC_INT_OFFSET;
    ipc_clear_lpi_active(priv->idev->lpi_base, index);
#endif
    return;
}

STATIC int hisi_mdev_startup(struct hisi_mbox_device *mdev)
{
    /*
     * nothing won't be done during suspend & resume flow for HI3xxx IPC.
     * see dummy like SR function, mdev_suspend & mdev_resume.
     * reserve runtime power management proceeding for further modification,
     * if necessary.
     */
    return 0;
}

STATIC void hisi_mdev_shutdown(struct hisi_mbox_device *mdev)
{
    /*
     * nothing won't be done during suspend & resume flow for HI3xxx IPC.
     * see dummy like SR function, mdev_suspend & mdev_resume.
     * reserve runtime power management proceeding for further modification,
     * if necessary.
     */
    return;
}

STATIC void mdev_dump_status(struct hisi_mbox_device *mdev)
{
    struct hisi_mbox_device_priv *priv = mdev->priv;

    ipcdrv_drv_event("\n====  mdev-%d-%s registers dump  ====\n  "
        "[SOURCE] : 0x%08x\n  [DSTATUS]: 0x%08x\n  "
        "[MODE]   : 0x%08x\n  [IMASK]  : 0x%08x\n  [ICLR] : 0x%08x\n  "
        "[CPUIRST]: 0x%08x\n  [DATA0]  : 0x%08x\n  [DATA1]: 0x%08x\n"
        "==============  end  =============\n",
        priv->index, mdev->name, __ipc_read_src(priv->idev->base, priv->index),
        __ipc_des_status(priv->idev->base, priv->index), __ipc_status(priv->idev->base, priv->index),
        __ipc_cpu_imask_get(priv->idev->base, priv->index), __ipc_cpu_istatus(priv->idev->base, priv->index),
        __ipc_mbox_irstatus(priv->idev->base, priv->des), __ipc_read(priv->idev->base, priv->index, 0),
        __ipc_read(priv->idev->base, priv->index, 1));

    return;
}

/* check the mdev, src_name != des_name */
STATIC int hisi_mdev_check(struct hisi_mbox_device *mdev, mbox_mail_type_t mtype, const char *rp_name)
{
    struct hisi_mbox_device_priv *priv = mdev->priv;
    int ret = RPUNACCESSIBLE;

    if ((mtype == TX_MAIL) && (SOURCE_MBOX & priv->func) && !strcmp(rp_name, priv->dse_name)) {
        ret = RPACCESSIBLE;
    } else if ((mtype == RX_MAIL) && (DESTINATION_MBOX & priv->func) && !strcmp(rp_name, priv->src_name)) {
        ret = RPACCESSIBLE;
    }

    return ret;
}

STATIC void hisi_mdev_clr_ack(struct hisi_mbox_device *mdev)
{
    struct hisi_mbox_device_priv *priv = mdev->priv;
    unsigned int imask;
    unsigned int toclr;

    imask = __ipc_cpu_imask_get(priv->idev->base, priv->index);
#ifdef CFG_SOC_PLATFORM_MINIV2
    toclr = (IPCBITMASK(ACPU0) | IPCBITMASK(ACPU1) | IPCBITMASK(ACPU2) | IPCBITMASK(ACPU3) |
             IPCBITMASK(TS_ACPU0) | IPCBITMASK(TS_ACPU1) | IPCBITMASK(TS_ACPU2) | IPCBITMASK(TS_ACPU3)) & (~imask);
#elif defined(CFG_SOC_PLATFORM_MINIV3)
    if (mdev->rproc_id < HISI_RPROC_RX_TS_ACPU0) {
        toclr = (IPCBITMASK(ACPU0) | IPCBITMASK(ACPU1) | IPCBITMASK(ACPU2) | IPCBITMASK(ACPU3)) & (~imask);
    } else {
        toclr = (IPCBITMASK(TS_ACPU0) | IPCBITMASK(TS_ACPU1) | IPCBITMASK(TS_ACPU2) | IPCBITMASK(TS_ACPU3)) & (~imask);
    }
#elif defined(CFG_SOC_PLATFORM_CLOUD_V2)
    toclr = (IPCBITMASK(TS_ACPU0) | IPCBITMASK(TS_ACPU1) | IPCBITMASK(TS_ACPU2) | IPCBITMASK(TS_ACPU3) |
        IPCBITMASK(IMU_ACPU0) | IPCBITMASK(IMU_ACPU1) | IPCBITMASK(IMU_ACPU2) | IPCBITMASK(IMU_ACPU3)) & (~imask);
#else
    toclr = (IPCBITMASK(ACPU0) | IPCBITMASK(ACPU1) | IPCBITMASK(ACPU2) | IPCBITMASK(ACPU3)) & (~imask);
#endif
    __ipc_cpu_iclr(priv->idev->base, toclr, priv->index);
}

STATIC void hisi_mdev_clr_irq_and_ack(struct hisi_mbox_device *mdev)
{
    struct hisi_mbox_device_priv *priv = mdev->priv;
    unsigned int status;
    unsigned int imask;
    unsigned int todo;
    int i;

    /*
     * temporarily, local processor will clean msg register,
     * and ack zero for an ipc from remote processors.
     */
    for (i = 0; i < priv->capability; i++) {
        __ipc_write(priv->idev->base, IPCACKMSG, priv->index, i);
    }

    imask = __ipc_cpu_imask_get(priv->idev->base, priv->index);

    /* get the irq unmask core bits, and clear the irq according to the unmask core bits,
     * because the irq to be sure triggered to the unmasked cores
     */
#ifdef CFG_SOC_PLATFORM_MINIV2
    todo = (IPCBITMASK(ACPU0) | IPCBITMASK(ACPU1) | IPCBITMASK(ACPU2) | IPCBITMASK(ACPU3) |
            IPCBITMASK(TS_ACPU0) | IPCBITMASK(TS_ACPU1) | IPCBITMASK(TS_ACPU2) | IPCBITMASK(TS_ACPU3)) & (~imask);
#elif defined(CFG_SOC_PLATFORM_MINIV3)
    if (mdev->rproc_id < HISI_RPROC_RX_TS_ACPU0) {
        todo = (IPCBITMASK(ACPU0) | IPCBITMASK(ACPU1) | IPCBITMASK(ACPU2) | IPCBITMASK(ACPU3)) & (~imask);
    } else {
        todo = (IPCBITMASK(TS_ACPU0) | IPCBITMASK(TS_ACPU1) | IPCBITMASK(TS_ACPU2) | IPCBITMASK(TS_ACPU3)) & (~imask);
    }
#elif defined(CFG_SOC_PLATFORM_CLOUD_V2)
    todo = (IPCBITMASK(TS_ACPU0) | IPCBITMASK(TS_ACPU1) | IPCBITMASK(TS_ACPU2) | IPCBITMASK(TS_ACPU3) |
        IPCBITMASK(IMU_ACPU0) | IPCBITMASK(IMU_ACPU1) | IPCBITMASK(IMU_ACPU2) | IPCBITMASK(IMU_ACPU3)) & (~imask);
#else
    todo = (IPCBITMASK(ACPU0) | IPCBITMASK(ACPU1) | IPCBITMASK(ACPU2) | IPCBITMASK(ACPU3)) & (~imask);
#endif

    __ipc_cpu_iclr(priv->idev->base, todo, priv->index);
    ipc_clear_lpi_active(priv->idev->lpi_base, priv->index);
    status = __ipc_status(priv->idev->base, priv->index);
    /* MBX_SEND write back to source processor */
    if ((DESTINATION_STATUS & status) && (!(AUTOMATIC_ACK_CONFIG & status))) {
        __ipc_send(priv->idev->base, todo, priv->index);
    }
    return;
}

STATIC void hisi_mdev_ack(struct hisi_mbox_device *mdev, mbox_msg_t *msg, mbox_msg_len_t len)
{
    hisi_mdev_clr_irq_and_ack(mdev);
    return;
}

STATIC mbox_msg_len_t hisi_mdev_hw_read(struct hisi_mbox_device *mdev, mbox_msg_t *msg)
{
    struct hisi_mbox_device_priv *priv = mdev->priv;
    mbox_msg_len_t cap;
    int i;

    cap = priv->capability;
    for (i = 0; i < (int)cap; i++) {
        msg[i] = __ipc_read(priv->idev->base, priv->index, i);
    }

    return cap;
}

/* to judge the four kind machine status of the ip, they are idle,src,des,ack */
STATIC int hisi_mdev_is_stm(struct hisi_mbox_device *mdev, unsigned int stm)
{
    struct hisi_mbox_device_priv *priv = mdev->priv;
    int is_stm = 0;

    if ((stm & __ipc_status(priv->idev->base, priv->index))) {
        is_stm = 1;
    }

    return is_stm;
}

STATIC mbox_msg_len_t mdev_receive_msg(struct hisi_mbox_device *mdev, mbox_msg_t **buf)
{
    mbox_msg_t *_buf = NULL;
    mbox_msg_len_t len = 0;

    if (hisi_mdev_is_stm(mdev, ACK_STATUS)) {
        ipcdrv_drv_debug("mdev_receive_msg ACK_STATUS\n");
        _buf = mdev->ack_buffer;
    } else {
        _buf = mdev->rx_buffer;
    }

    if (_buf != NULL) {
        len = hisi_mdev_hw_read(mdev, _buf);
    }
    if (buf != NULL) {
        *buf = _buf;
    }

    return len;
}

STATIC int hisi_mdev_unlock(struct hisi_mbox_device *mdev)
{
    struct hisi_mbox_device_priv *priv = mdev->priv;
    int retry = 3;

    do {
        __ipc_unlock(priv->idev->base, priv->idev->unlock);
        if (__ipc_lock_status(priv->idev->base) == IPC_UNLOCKED) {
            break;
        }

        udelay(10);
        retry--;
    } while (retry);

    if (!retry) {
        return -ENODEV;
    }

    return 0;
}

STATIC int hisi_mdev_occupy(struct hisi_mbox_device *mdev)
{
    struct hisi_mbox_device_priv *priv = mdev->priv;
    int retry = 10;

    do {
        /*
         * Hardware lock
         * A hardware lock is needed here to lock a mailbox resource,
         * which could be used by another remote proccessor, such as
         * a HiIPCV230 common mailbox-25/mailbox-26.
         */
        if (!(__ipc_status(priv->idev->base, priv->index) & IDLE_STATUS)) {
            hisi_mdev_asm_wfe();
        } else {
            /* set the source processor bit, we set common mailbox's  source processor bit through dtsi */
            __ipc_set_src(priv->idev->base, priv->src, priv->index);
            if (__ipc_read_src(priv->idev->base, priv->index) & (unsigned)IPCBITMASK((unsigned)priv->src)) {
                break;
            }
        }
        retry--;
        /* Hardware unlock */
    } while (retry);

    if (!retry) {
        return -ENODEV;
    }

    return 0;
}

STATIC int hisi_mdev_hw_send(struct hisi_mbox_device *mdev, const mbox_msg_t *msg, mbox_msg_len_t len, int ack_mode)
{
    struct hisi_mbox_device_priv *priv = mdev->priv;
    unsigned int temp;
    int i;

    /* interrupts unmask */
    __ipc_cpu_imask_all(priv->idev->base, priv->index);

    if (ack_mode == AUTO_ACK) {
        temp = (unsigned)IPCBITMASK((unsigned)priv->des);
    } else {
        temp = (unsigned)IPCBITMASK((unsigned)priv->src) | (unsigned)IPCBITMASK((unsigned)priv->des);
    }
    __ipc_cpu_imask_clr(priv->idev->base, temp, priv->index);

    /* des config */
    __ipc_set_des(priv->idev->base, priv->des, priv->index);

    /* ipc mode config */
    if (ack_mode == AUTO_ACK) {
        temp = AUTOMATIC_ACK_CONFIG;
    } else {
        temp = NO_FUNC_CONFIG;
    }
    __ipc_mode(priv->idev->base, temp, priv->index);

    /* write data */
    for (i = 0; i < (int)((priv->capability < (int)len) ? priv->capability : len); i++) {
        __ipc_write(priv->idev->base, msg[i], priv->index, i);
    }

    /* enable sending */
    __ipc_send(priv->idev->base, (unsigned)IPCBITMASK((unsigned)priv->src), priv->index);

    return 0;
}

STATIC void mdev_ensure_channel(struct hisi_mbox_device *mdev)
{
    int timeout = 0;

    if (mdev->ops->is_stm(mdev, IDLE_STATUS)) {
        return;
    } else if (mdev->ops->is_stm(mdev, ACK_STATUS)) {
        /* the ack status is reached, just release,
         * (the sync and async is mutexed by by mdev->dev_lock.
         * ACK STATUS, release the channel directly
         */
        goto release;
    } else {
        /* DEST STATUS and SRC STATUS, the dest is processing, wait here
         * the worst situation is to delay 1000*5us+60*5ms = 305ms
         */
        while (timeout < MAILBOX_ASYNC_TIMEOUT_CNT) {
            if (timeout < MAILBOX_ASYNC_UDELAY_CNT) {
                udelay(5);
            } else {
                /* the hifi may power off when send ipc msg, so the ack status may wait 20ms */
                usleep_range(3000, 5000);
            }

            /* if chanal is always source state, need jump out and release the channel */
            if ((timeout > MAILBOX_ASYNC_UDELAY_CNT) && (mdev->ops->is_stm(mdev, SOURCE_STATUS))) {
                break;
            }

            /* if chanal is idle, need jump out */
            if (unlikely(mdev->ops->is_stm(mdev, IDLE_STATUS))) {
                return;
            }

            /* if the ack status is ready, break out */
            if (mdev->ops->is_stm(mdev, ACK_STATUS)) {
                break;
            }

            timeout++;
        }

        if (unlikely(timeout == MAILBOX_ASYNC_TIMEOUT_CNT)) {
            ipcdrv_drv_event("Wait %s ipc channel idle timeout...\n", mdev->name);
            if (mdev->ops->status != NULL) {
                mdev->ops->status(mdev);
            }
        }
        goto release;
    }

release:
    /* release the channel */
    mdev->ops->refresh(mdev);
}

STATIC int mdev_send_msg(struct hisi_mbox_device *mdev, mbox_msg_t *msg, mbox_msg_len_t len, int ack_mode)
{
    struct hisi_mbox_device_priv *priv = mdev->priv;
    int err = 0;

    /* all the mailbox channel is treated as fast-mailbox */
    if (DESTINATION_MBOX & priv->func) {
        ipcdrv_drv_err("mdev %s has no tx ability\n", mdev->name);
        err = -EMDEVCLEAN;
        goto out;
    }

    /*
     * Whenever an ipc starts,
     * ipc module has to be unlocked at the very beginning.
     */
    if (hisi_mdev_unlock(mdev)) {
        ipcdrv_drv_err("%s: mdev %s can not be unlocked\n", MODULE_NAME, mdev->name);
        err = -EMDEVCLEAN;
        goto out;
    }

    if (hisi_mdev_occupy(mdev)) {
        ipcdrv_drv_err("mdev %s can not be occupied\n", mdev->name);
        err = -EMDEVCLEAN;
        goto out;
    }

    (void)hisi_mdev_hw_send(mdev, msg, len, ack_mode);

out:
    return err;
}

STATIC void hisi_mdev_release(struct hisi_mbox_device *mdev)
{
    struct hisi_mbox_device_priv *priv = mdev->priv;
    volatile unsigned int src;

    __ipc_cpu_imask_all(priv->idev->base, priv->index);
    src = __ipc_read_src(priv->idev->base, priv->index);
    __raw_writel(src, priv->idev->base + (long)(unsigned)IPCMBxSOURCE(priv->index));
    hisi_mdev_asm_sev();
    return;
}

int hisi_mdev_is_irq_exist(int irq)
{
    struct hisi_mbox_irq_info *irq_info = NULL;

    if (list_empty(&g_mbox_irq_list) != 0) {
        return 0;
    }
    list_for_each_entry(irq_info, &g_mbox_irq_list, list) {
        if (irq_info->irq_no == irq) {
            return 1;
        }
    }

    return 0;
}

void mdev_add_irq_info(int irq, void *priv_data)
{
    struct hisi_mbox_irq_info *irq_info = NULL;

    irq_info = (struct hisi_mbox_irq_info *)kzalloc(sizeof(struct hisi_mbox_irq_info), GFP_KERNEL);
    if (irq_info == NULL) {
        ipcdrv_drv_err("irq %d kzalloc err\n", irq);
        return;
    }
    irq_info->irq_no = irq;
    irq_info->priv_data = priv_data;
    list_add_tail(&irq_info->list, &g_mbox_irq_list);
}

#if (defined(CFG_SOC_PLATFORM_MDC_V51))
STATIC void mdev_dump_domain_cpu_info(struct cpu_domain_info domain_info)
{
    ipcdrv_drv_err("domain_cpu_info:ctrl=%u,ctrlmap=%lu.\n"
        "domain_cpu_info:ai=%u,aimap=%lu.\n"
        "domain_cpu_info:data=%u,datamap=%lu.\n",
        domain_info.ctrlcpu_num, domain_info.ctrlcpu_bitmap,
        domain_info.aicpu_num, domain_info.aicpu_bitmap,
        domain_info.datacpu_num, domain_info.datacpu_bitmap);
}

STATIC int mdev_irq_check_domain_info_num(struct cpu_domain_info domain_info)
{
    /* ctrlcpu_num the minimum value is 1. */
    if (domain_info.ctrlcpu_num == 0U) {
        ipcdrv_drv_err("invalid, The value of ctrlcpu_num should be greater than 0, "
            "but the actual value is %u.\n", domain_info.ctrlcpu_num);
        return -EINVAL;
    }
    return 0;
}

STATIC int mdev_get_irq_cpu_aos(unsigned int *irq_cpu)
{
#define AOSCORE_CTRLCPU_ARR_SIZE 32
    int err;
    unsigned int i;
    unsigned int ctrlcpu_bitmap_num = 0;
    unsigned int ctrlcpu_arr[AOSCORE_CTRLCPU_ARR_SIZE] = {0};
    struct cpu_domain_info domain_info;
#ifdef AOS_LLVM_BUILD
    unsigned int tmp_irq = *irq_cpu;
#endif

    err = get_cpudomain_info(&domain_info);
    if (err != 0) {
        ipcdrv_drv_err("get_cpudomain_info failed.(err=%d).\n", err);
        return -EINVAL;
    }

    err = mdev_irq_check_domain_info_num(domain_info);
    if (err != 0) {
        ipcdrv_drv_err("irq check domain info failed\n");
        mdev_dump_domain_cpu_info(domain_info);
        return -EINVAL;
    }

    for (i = 0; i < AOSCORE_CTRLCPU_ARR_SIZE; i++) {
        if (((domain_info.ctrlcpu_bitmap >> i) & 0x1U) != 0) {
            ctrlcpu_arr[ctrlcpu_bitmap_num] = i;
            ctrlcpu_bitmap_num++;
        }
    }

    if (ctrlcpu_bitmap_num != domain_info.ctrlcpu_num) {
        for (i = 0; i < ctrlcpu_bitmap_num; i++) {
            ipcdrv_drv_err("crtrlcpu_arr[%u] = %u.\n", i, ctrlcpu_arr[i]);
        }
        ipcdrv_drv_err("ctrlcpu_bitmap_num-%u is not equal ctrlcpu_num-%u, failed\n",
            ctrlcpu_bitmap_num, domain_info.ctrlcpu_num);
        mdev_dump_domain_cpu_info(domain_info);
        return -EINVAL;
    }

    ipcdrv_drv_info("srcirq = %u. ctrlcpu_bitmap=%lu. ctrlcpu_num=%u.\n",
        *irq_cpu, domain_info.ctrlcpu_bitmap, domain_info.ctrlcpu_num);
    *irq_cpu = ctrlcpu_arr[*irq_cpu % ctrlcpu_bitmap_num];
#ifdef AOS_LLVM_BUILD
    int aos_first_cpu_num = get_boot_cpu_phyid();
    if ((aos_first_cpu_num < 0)) {
        ipcdrv_drv_err("get_boot_cpu_phyid failed aos_first_cpu_num = %d\n", aos_first_cpu_num);
        *irq_cpu = tmp_irq;
        return -EINVAL;
    }
    *irq_cpu += (unsigned int)aos_first_cpu_num;
#endif

    return 0;
}
#endif


STATIC int mdev_irq_request(struct hisi_mbox_device *mdev, irq_handler_t handler)
{
    struct hisi_mbox_device_priv *priv = mdev->priv;
    int ret;

    if (hisi_mdev_is_irq_exist(priv->irq) == 0 && priv->irq != -1) {
        ret = request_irq((u32)priv->irq, handler, 0, mdev->name, (void*)mdev);
        if (ret != 0) {
#ifndef UT_TEST
            ipcdrv_drv_err("irq %d request err. ret:%d\n", priv->irq, ret);
#endif
            return -EINVAL;
        }

#if (defined(CFG_SOC_PLATFORM_MDC_V51))
        if (priv->irq_affinity != IPC_CPU_AFFINITY_DEFAULT) {
            ret = mdev_get_irq_cpu_aos(&priv->irq_affinity);
            if (ret != 0) {
                ipcdrv_drv_warn("%s mdev_get_irq_cpu_aos did not complete as expected.\n", mdev->name);
            }
            ipcdrv_drv_info("bind irq = %u.\n", priv->irq_affinity);
            ret = irq_set_affinity_hint(priv->irq, get_cpu_mask(priv->irq_affinity));
            if (ret != 0) {
                ipcdrv_drv_warn("%s irq %d bind to cpu %u did not complete as expected.\n",
                    mdev->name, priv->irq, priv->irq_affinity);
            }
        }
#else
#ifndef AOS_LLVM_BUILD
        if (priv->irq_affinity != IPC_CPU_AFFINITY_DEFAULT) {
            ret = irq_set_affinity_hint(priv->irq, get_cpu_mask(priv->irq_affinity));
            if (ret != 0) {
                ipcdrv_drv_warn("%s irq %d bind to cpu %u did not complete as expected.\n",
                    mdev->name, priv->irq, priv->irq_affinity);
            }
        }
#endif
#endif
        mdev_add_irq_info(priv->irq, mdev);
    }
    return 0;
}

STATIC void mdev_irq_free(struct hisi_mbox_device *mdev, void *p)
{
    int ret;
    struct hisi_mbox_device_priv *priv = mdev->priv;
    struct hisi_mbox_irq_info *irq_info = NULL;
    struct hisi_mbox_irq_info *irq_info_next = NULL;

    if (list_empty(&g_mbox_irq_list) != 0) {
        return;
    }
    list_for_each_entry_safe(irq_info, irq_info_next, &g_mbox_irq_list, list) {
        if (priv->irq == irq_info->irq_no) {
            list_del(&irq_info->list);
#ifndef AOS_LLVM_BUILD
            if (priv->irq_affinity != IPC_CPU_AFFINITY_DEFAULT) {
                ret = irq_set_affinity_hint(priv->irq, NULL);
                if (ret != 0) {
                    ipcdrv_drv_warn("%s irq %d unbind cpu did not complete as expected\n", mdev->name, priv->irq);
                }
            }
#endif
            (void)free_irq(irq_info->irq_no, irq_info->priv_data);
            kfree(irq_info);
            irq_info = NULL;
        }
    }
    return;
}

STATIC void hisi_mdev_irq_enable(struct hisi_mbox_device *mdev)
{
    enable_irq((unsigned int)mdev->cur_irq);
}

STATIC void hisi_mdev_irq_disable(struct hisi_mbox_device *mdev)
{
    disable_irq_nosync((unsigned int)mdev->cur_irq);
}

/* Get mailbox channel by irq */
STATIC struct hisi_mbox_device *mdev_irq_to_mdev(struct hisi_mbox_device *_mdev, struct list_head *list, int irq)
{
    struct hisi_mbox_device *mdev = NULL;
    struct hisi_mbox_device_priv *priv = NULL;

    if ((list_empty(list)) || (_mdev == NULL)) {
        ipcdrv_drv_err("invalid input: list_empty(list)=%d, (_mdev == NULL)=%d.\n",
            list_empty(list), (_mdev == NULL));
        goto out;
    }

    list_for_each_entry(mdev, list, node) {
        priv = mdev->priv;
        if ((priv->irq == irq) && (mdev->configured)) {
#ifdef CFG_SOC_PLATFORM_MINIV2_MDC
            if (__ipc_read_src(priv->idev->base, priv->index) != IPCBITMASK((u32)priv->src)) {
                continue;
            }
#endif
            if (priv->func & SOURCE_MBOX) {
                if (mdev->ops->is_stm(mdev, ACK_STATUS)) {
                    goto out;
                } else {
                    continue;
                }
            } else { /* DESTINATION_MBOX */
                if (mdev->ops->is_stm(mdev, DESTINATION_STATUS)) {
                    goto out;
                } else {
                    continue;
                }
            }
        }
    }
    mdev = NULL;
out:
    /* it is nearly occured */
    /* if can't find mdev, return NULL */
    return mdev;
}

struct hisi_mbox_dev_ops hisi_mdev_ops = {
    .startup = hisi_mdev_startup,
    .shutdown = hisi_mdev_shutdown,
    .check = hisi_mdev_check,
    .recv = mdev_receive_msg,
    .send = mdev_send_msg,
    .ack = hisi_mdev_ack,
    .refresh = hisi_mdev_release,
#ifdef AOS_LLVM_BUILD
    .hisi_request_irq = mdev_irq_request,
#else
    .request_irq = mdev_irq_request,
#endif
    .free_irq = mdev_irq_free,
    .enable_irq = hisi_mdev_irq_enable,
    .disable_irq = hisi_mdev_irq_disable,
    .irq_to_mdev = mdev_irq_to_mdev,
    .is_stm = hisi_mdev_is_stm,
    .clr_ack = hisi_mdev_clr_ack,
    .ensure_channel = mdev_ensure_channel,
    .status = mdev_dump_status,
};

STATIC void hisi_mdev_put(struct platform_device *pdev, struct hisi_ipc_device *idev)
{
    struct hisi_mbox_device **list = idev->mdev_res;
    struct hisi_mbox_device *mdev = NULL;
    int i;

#if defined(CFG_SOC_PLATFORM_CLOUD)
    devm_iounmap(&pdev->dev, idev->lpi_base);
    idev->lpi_base = NULL;
    devm_iounmap(&pdev->dev, idev->base);
    idev->base = NULL;
#else
    iounmap(idev->base);
    idev->base = NULL;
#endif

    kfree(idev->cmbox_info);
    kfree(idev->buf_pool);

    idev->cmbox_info = NULL;
    idev->buf_pool = NULL;

    for (i = 0; (mdev = list[i]); i++) {
        kfree(mdev->priv);
        mdev->priv = NULL;
        kfree(mdev);
        mdev = NULL;
    }

    return;
}

STATIC void hisi_mdev_remove(struct platform_device *pdev)
{
    struct hisi_ipc_device *idev = platform_get_drvdata(pdev);

    platform_set_drvdata(pdev, NULL);

    if (idev != NULL) {
        // fix this bug
        hisi_rproc_exit(idev->mdev_res[0]->ipc_id);

        (void)mbox_device_unregister(idev->mdev_res);
        hisi_mdev_put(pdev, idev);
        kfree(idev->mdev_res);
        idev->mdev_res = NULL;
        kfree(idev);
        idev = NULL;
    }
    
}

#ifdef CFG_SOC_PLATFORM_MINIV2
#define SC_CHIP_INF_ADDR 0x8000fff8

static u32 g_hisi_mdev_chip_type = HISI_MINIV2_V1;

u32 hisi_mdev_get_chip_type(void)
{
    return g_hisi_mdev_chip_type;
}

void mdev_init_chip_type(void)
{
    void __iomem *rd_base = NULL;
    u32 regval;

    rd_base = ioremap(SC_CHIP_INF_ADDR, sizeof(u32));
    if (rd_base == NULL) {
        ipcdrv_drv_warn("ioremap results, not get chip type base\n");
        return;
    }
    regval = readl(rd_base);
    if ((regval & 0xf) == HISI_MINIV2_V2) {
        g_hisi_mdev_chip_type = HISI_MINIV2_V2;
    } /* else use default cfg v1 */
    iounmap(rd_base);
    ipcdrv_drv_info("chip type v%u\n", g_hisi_mdev_chip_type);
}

STATIC int hisi_mdev_get_irq_from_srcbit(const struct hisi_ipc_device *idev, remote_processor_type_t src_bit)
{
    switch (src_bit) {
        case ACPU0:
        case TS_ACPU0:
            return idev->cmbox_info->cmbox_gic_1_irq;
        case ACPU1:
        case TS_ACPU1:
            return idev->cmbox_info->cmbox_gic_2_irq;
        case ACPU2:
        case TS_ACPU2:
            return idev->cmbox_info->cmbox_gic_3_irq;
        case ACPU3:
        case TS_ACPU3:
            return idev->cmbox_info->cmbox_gic_4_irq;
        default:
            return -EINVAL;
    }
}

#elif defined(CFG_SOC_PLATFORM_CLOUD_V2)
STATIC int hisi_mdev_get_irq_from_srcbit(struct hisi_ipc_device *idev, remote_processor_type_t src_bit)
{
    switch (src_bit) {
        case TS_ACPU0:
        case IMU_ACPU0:
            return idev->cmbox_info->cmbox_gic_1_irq;
        case TS_ACPU1:
        case IMU_ACPU1:
            return idev->cmbox_info->cmbox_gic_2_irq;
        case TS_ACPU2:
        case IMU_ACPU2:
            return idev->cmbox_info->cmbox_gic_3_irq;
        case TS_ACPU3:
        case IMU_ACPU3:
            return idev->cmbox_info->cmbox_gic_4_irq;
        default:
            return -EINVAL;
    }
}
#elif defined(CFG_SOC_PLATFORM_MINIV3)
STATIC int hisi_mdev_get_irq_from_srcbit(struct hisi_ipc_device *idev, remote_processor_type_t src_bit)
{
    switch (src_bit) {
        case ACPU0:
        case TS_ACPU0:
            return idev->cmbox_info->cmbox_gic_1_irq;
        case ACPU1:
        case TS_ACPU1:
            return idev->cmbox_info->cmbox_gic_2_irq;
        case ACPU2:
        case TS_ACPU2:
            return idev->cmbox_info->cmbox_gic_3_irq;
        case ACPU3:
        case TS_ACPU3:
            return idev->cmbox_info->cmbox_gic_4_irq;
        default:
            return -EINVAL;
    }
}
#else
STATIC int hisi_mdev_get_irq_from_srcbit(struct hisi_ipc_device *idev, remote_processor_type_t src_bit)
{
    switch (src_bit) {
        case ACPU0:
            return idev->cmbox_info->cmbox_gic_1_irq;
        case ACPU1:
            return idev->cmbox_info->cmbox_gic_2_irq;
        case ACPU2:
            return idev->cmbox_info->cmbox_gic_3_irq;
        case ACPU3:
            return idev->cmbox_info->cmbox_gic_4_irq;
        default:
            return -EINVAL;
    }
}
#endif /* end of miniv2 */

#ifdef CFG_SOC_PLATFORM_CLOUD
STATIC int mdev_get_func(struct hisi_ipc_device *idev, struct fwnode_handle *fwnode, struct hisi_mbox_device *mdev,
    u8 *func, int *irq, remote_processor_type_t src_bit, const int *lpi_irq)
{
    u32 output[OUTPUT_LENGTH] = {0};
    u32 irq_idex = 0;
    int ret;

    ret = fwnode_property_read_u32_array(fwnode, "func", output, OUTPUT_LENGTH);
    if (ret != 0) {
        return ret;
    }

    *func |= (output[0] ? FAST_MBOX : COMM_MBOX);

    *func |= (output[1] ? SOURCE_MBOX : 0);

    *func |= (output[2] ? DESTINATION_MBOX : 0);

    if ((FAST_MBOX & (*func)) && (DESTINATION_MBOX & (*func))) {
        ret = fwnode_property_read_u32(fwnode, "interrupts", &irq_idex);
        if ((irq_idex >= HISI_MAX_LPI_IRQ_NUM) || (ret != 0)) {
            ipcdrv_drv_err("interrupts read error:%d\n", ret);
            return ret;
        }
        *irq = lpi_irq[irq_idex];
    } else if ((FAST_MBOX & (*func)) && (SOURCE_MBOX & (*func))) {
        *irq = hisi_mdev_get_irq_from_srcbit(idev, src_bit);
        /* set the cmdev, the cmdev will be used in acore't interrupts */
        if (idev->cmbox_info->cmdev == NULL) {
            idev->cmbox_info->cmdev = mdev;
        }
    } else {
        ipcdrv_drv_err(" xxxxxxxxx we don't use comm-mailbox , we use it as fast-mailbox\n");
        ret = -EINVAL;
    }

    return ret;
}

STATIC int mdev_get_child(int ipc_id, struct hisi_ipc_device *idev, struct hisi_mbox_device **mdevs,
    const struct device *node, int capability, const int *lpi_irq, int lpi_irq_num)
{
    struct hisi_mbox_device_priv *priv = NULL;
    mbox_msg_t *buf_pool = idev->buf_pool;
    struct hisi_mbox_device *mdev = NULL;
    struct acpi_device *cdev_tmp = NULL;
    struct acpi_device *device = NULL;
    mbox_msg_t *ack_buffer = NULL;
    mbox_msg_t *rx_buffer = NULL;
    const char *src_name = NULL;
    const char *dse_name = NULL;
    u32 des_bit = 0;
    u32 src_bit = 0;
    int ret = 0;
    int irq = 0;
    int i = 0;
    u32 index;
    u8 func;

    device = ACPI_COMPANION(node);
    if (device == NULL) {
        ipcdrv_drv_err("ACPI_COMPANION return is NULL\n");
        return -1;
    }

    list_for_each_entry(cdev_tmp, &device->children, node)
    {
        mdev = NULL;
        priv = NULL;
        func = 0;
        src_name = NULL;
        dse_name = NULL;
        index = U32_MAX;
        rx_buffer = NULL;
        ack_buffer = NULL;

        mdev = kzalloc(sizeof(*mdev), GFP_KERNEL);
        if (mdev == NULL) {
            ret = -ENOMEM;
            goto to_break;
        }

        priv = kzalloc(sizeof(*priv), GFP_KERNEL);
        if (priv == NULL) {
            ret = -ENOMEM;
            goto free_mdev;
        }

        ret = fwnode_property_read_string(&cdev_tmp->fwnode, "src_remote_processor", &src_name);
        if (ret != 0) {
            goto free_priv;
        }

        ret = fwnode_property_read_string(&cdev_tmp->fwnode, "des_remote_processor", &dse_name);
        if (ret != 0) {
            goto free_priv;
        }

        ret = fwnode_property_read_u32(&cdev_tmp->fwnode, "src_bit", &src_bit);
        if (ret != 0) {
            goto free_priv;
        }

        ret = fwnode_property_read_u32(&cdev_tmp->fwnode, "des_bit", &des_bit);
        if (ret != 0) {
            goto free_priv;
        }

        ret = fwnode_property_read_u32(&cdev_tmp->fwnode, "index", &index);
        if (ret != 0) {
            goto free_priv;
        }

        ret = mdev_get_func(idev, &cdev_tmp->fwnode, mdev, &func, &irq, (remote_processor_type_t)src_bit, lpi_irq);
        if (ret != 0) {
            goto free_priv;
        }

        rx_buffer = buf_pool + capability * RX_BUFFER_TYPE;
        ack_buffer = buf_pool + capability * ACK_BUFFER_TYPE;
        buf_pool = buf_pool + capability * MBOX_BUFFER_TYPE_MAX;

        priv->capability = capability;
        priv->func = func;
        priv->src_name = src_name;
        priv->dse_name = dse_name;
        priv->src = (remote_processor_type_t)src_bit;
        priv->des = (remote_processor_type_t)des_bit;
        priv->irq = irq;
        priv->irq_affinity = IPC_CPU_AFFINITY_DEFAULT;
        priv->index = index;
        priv->idev = idev;

        mdev->name = src_name;  // dev_name;
        mdev->priv = priv;
        mdev->rx_buffer = rx_buffer;
        mdev->ack_buffer = ack_buffer;
        mdev->ops = &hisi_mdev_ops;
        mdev->configured = 0;
        mdev->ipc_id = ipc_id;
        mdev->thread_flag = 0;
        mdev->enable_irq_stage = ENABLE_IRQ_IN_INTERRUPT;
        mdev->rx_fifo_full_cnt = 0;
        mdevs[i++] = mdev;

        continue;
    free_priv:
        kfree(priv);
        priv = NULL;
    free_mdev:
        kfree(mdev);
        mdev = NULL;
    to_break:
        break;
    }

    if (ret != 0) {
        goto deinit_mdevs;
    }
    return ret;

deinit_mdevs:
    while (i--) {
        kfree(mdevs[i]->priv);
        mdevs[i]->priv = NULL;
        kfree(mdevs[i]);
        mdevs[i] = NULL;
    }
    return ret;
}
static void devdrv_write_lpi_msgs(struct msi_desc *desc, struct msi_msg *msg)
{
}

static void ipcdev_free_msis(void *data)
{
    struct device *dev = data;
    platform_msi_domain_free_irqs(dev);
}

STATIC int mdev_get(int ipc_id, struct hisi_ipc_device *idev, struct hisi_mbox_device **mdevs, struct device *dev)
{
    struct hisi_common_mbox_info *cmbox_info = NULL;
    int ipc_lpi_irq[HISI_MAX_LPI_IRQ_NUM] = {0};
    mbox_msg_len_t buf_pool_len = 0;
    void __iomem *ipc_base = NULL;
    void __iomem *lpi_base = NULL;
    struct msi_desc *desc = NULL;
    mbox_msg_t *buf_pool = NULL;
    u64 ipc_mbigen_reg_addr = 0;
    u64 ipc_reg_addr = 0;
    u64 ipc_reg_len = 0;
    u32 capability = 0;
    u32 mdev_num = 0;
    int rp_num = 0;
    u32 unlock = 0;
    int ret;

    ret = device_property_read_u64(dev, "ipc_reg_addr", &ipc_reg_addr);
    if (ret != 0) {
        ipcdrv_drv_err("ipc_base_addr read error:%d\n", ret);
        return (-ENODEV);
    }

    ret = device_property_read_u64(dev, "ipc_reg_len", &ipc_reg_len);
    if (ret != 0) {
        ipcdrv_drv_err("ipc_base_addr read error:%d\n", ret);
        return (-ENODEV);
    }

    ret = device_property_read_u64(dev, "ipc_mbigen_reg_base", &ipc_mbigen_reg_addr);
    if (ret != 0) {
        ipcdrv_drv_err("ipc_mbigen_reg_base read error:%d\n", ret);
        return (-ENODEV);
    }
    lpi_base = (void __iomem *)devm_ioremap(dev, (ipc_mbigen_reg_addr + MBIX_INT_CLR_REG), 0x10);
    if (lpi_base == NULL) {
        ipcdrv_drv_err("devm_ioremap for ipc_reg failed.\n");
        return (-ENODEV);
    }
    ipc_base = (void __iomem *)devm_ioremap(dev, ipc_reg_addr, ipc_reg_len);
    if (ipc_base == NULL) {
        ipcdrv_drv_err("devm_ioremap for ipc_reg failed.\n");
        devm_iounmap(dev, lpi_base);
        return (-ENODEV);
    }

    ret = device_property_read_u32(dev, "capability", &capability);
    if (ret != 0) {
        ipcdrv_drv_err("prop \"capability\" error %d\n", ret);
        ret = -ENODEV;
        goto to_iounmap;
    }

    ret = device_property_read_u32(dev, "unlock_word", &unlock);
    if (ret != 0) {
        ipcdrv_drv_err("prop \"unlock_word\" error %d\n", ret);
        ret = -ENODEV;
        goto to_iounmap;
    }

    ret = device_property_read_u32(dev, "mailboxes", &mdev_num);
    if (ret != 0) {
        ipcdrv_drv_err("%s: prop \"mailboxes\" error %d\n", MODULE_NAME, ret);
        ret = -ENODEV;
        goto to_iounmap;
    }

    cmbox_info = kmalloc(sizeof(*cmbox_info), GFP_KERNEL);
    if (cmbox_info == NULL) {
        ret = -ENOMEM;
        goto to_iounmap;
    }

    buf_pool_len = capability * MBOX_BUFFER_TYPE_MAX * mdev_num;
    buf_pool = kzalloc(sizeof(mbox_msg_t) * buf_pool_len, GFP_KERNEL);
    if (buf_pool == NULL) {
        ret = -ENOMEM;
        goto free_cmbox;
    }

    /* get lpi irq number */
    ret = platform_msi_domain_alloc_irqs(dev, HISI_MAX_LPI_IRQ_NUM, devdrv_write_lpi_msgs);
    if (ret != 0) {
        ipcdrv_drv_err("failed to allocate LPI(%d)\n", ret);
        ret = -ENOMEM;
        goto deinit_mdevs;
    }
    for_each_msi_entry(desc, dev) {
        if (desc->platform.msi_index >= HISI_MAX_LPI_IRQ_NUM) {
            break;
        }
        ipc_lpi_irq[desc->platform.msi_index] = (int)desc->irq;
    }

    devm_add_action(dev, ipcdev_free_msis, dev);

    cmbox_info->gic_1_irq_requested = 0;
    cmbox_info->gic_2_irq_requested = 0;
    cmbox_info->gic_3_irq_requested = 0;
    cmbox_info->gic_4_irq_requested = 0;
    cmbox_info->cmbox_gic_1_irq = ipc_lpi_irq[HISI_ACPU0_ACK_IRQ_IDX];
    cmbox_info->cmbox_gic_2_irq = ipc_lpi_irq[HISI_ACPU1_ACK_IRQ_IDX];
    cmbox_info->cmbox_gic_3_irq = ipc_lpi_irq[HISI_ACPU2_ACK_IRQ_IDX];
    cmbox_info->cmbox_gic_4_irq = ipc_lpi_irq[HISI_ACPU3_ACK_IRQ_IDX];
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    cmbox_info->cmbox_gic_5_irq = ipc_lpi_irq[HISI_ACPU4_ACK_IRQ_IDX];
#endif
    cmbox_info->cmdev = NULL;

    idev->cmbox_info = cmbox_info;
    idev->unlock = unlock;
    idev->base = ipc_base;
    idev->lpi_base = lpi_base;
    idev->mdev_res = mdevs;
    idev->buf_pool = buf_pool;
    idev->rp_num = rp_num;

    ret = mdev_get_child(ipc_id, idev, mdevs, dev, (int)capability, ipc_lpi_irq, HISI_MAX_LPI_IRQ_NUM);
    if (ret != 0) {
        goto deinit_mdevs;
    }
    set_mbx_int_start_id(ipc_id, ipc_lpi_irq[0]);

    return ret;

deinit_mdevs:
    kfree(buf_pool);
    buf_pool = NULL;
free_cmbox:
    kfree(cmbox_info);
    cmbox_info = NULL;
to_iounmap:
    devm_iounmap(dev, lpi_base);
    devm_iounmap(dev, ipc_base);
    return ret;
}

#else

#ifdef IPC_ACPI_SUPPORT
STATIC int mdev_get_gic_irq(const struct fwnode_handle *handle,
    struct hisi_common_mbox_info *cmbox_info, const int *irq)
{
    u32 output[IRQINDEX_LENGTH] = {0};
    int ret;
    cmbox_info->gic_1_irq_requested = 0;
    cmbox_info->gic_2_irq_requested = 0;
    cmbox_info->gic_3_irq_requested = 0;
    cmbox_info->gic_4_irq_requested = 0;
    ret = fwnode_property_read_u32_array(handle, "gic_index", output, IRQINDEX_LENGTH);
    if (ret != 0) {
        ipcdrv_drv_err("fwnode_property_read_u32_array failed :%d\n", ret);
        return ret;
    }
    cmbox_info->cmbox_gic_1_irq = irq[output[CMBOX_GIC_1_INDEX]];
    cmbox_info->cmbox_gic_2_irq = irq[output[CMBOX_GIC_2_INDEX]];
    cmbox_info->cmbox_gic_3_irq = irq[output[CMBOX_GIC_3_INDEX]];
    cmbox_info->cmbox_gic_4_irq = irq[output[CMBOX_GIC_4_INDEX]];
    return 0;
}

STATIC int mdev_parse_child(const struct fwnode_handle *fwnode,
    struct hisi_mbox_device_priv *priv)
{
    u32 output[OUTPUT_LENGTH] = {0};
    const char *src_name = NULL;
    const char *dse_name = NULL;
    u32 des_bit = 0;
    u32 src_bit = 0;
    u32 cpu_affinity = IPC_CPU_AFFINITY_DEFAULT;
    u32 index;
    int ret;
    u8 func = 0;
    ret = fwnode_property_read_string(fwnode, "src_remote_processor", &src_name);
    if (ret != 0) {
        ipcdrv_drv_err("fwnode_property_read_string failed:%d\n", ret);
        return ret;
    }
    ret = fwnode_property_read_string(fwnode, "des_remote_processor", &dse_name);
    if (ret != 0) {
        ipcdrv_drv_err("fwnode_property_read_string failed:%d\n", ret);
        return ret;
    }
    ret = fwnode_property_read_u32(fwnode, "src_bit", (u32 *)&src_bit);
    if (ret != 0) {
        ipcdrv_drv_err("fwnode_property_read_u32 failed:%d\n", ret);
        return ret;
    }

    ret = fwnode_property_read_u32(fwnode, "des_bit", (u32 *)&des_bit);
    if (ret != 0) {
        ipcdrv_drv_err("fwnode_property_read_u32 failed:%d\n", ret);
        return ret;
    }

    ret = fwnode_property_read_u32(fwnode, "index", (u32 *)&index);
    if (ret != 0) {
        ipcdrv_drv_err("fwnode_property_read_u32 failed:%d\n", ret);
        return ret;
    }

    ret = fwnode_property_read_u32(fwnode, "irq_affinity", (u32 *)&cpu_affinity);
    if (ret != 0) {
        cpu_affinity = IPC_CPU_AFFINITY_DEFAULT;
        /* continue */
    }

    priv->src_name = src_name;
    priv->dse_name = dse_name;
    priv->src = (remote_processor_type_t)src_bit;
    priv->des = (remote_processor_type_t)des_bit;
    priv->index = index;
    priv->irq_affinity = cpu_affinity;

    ret = fwnode_property_read_u32_array(fwnode, "func", output, OUTPUT_LENGTH);
    if (ret != 0) {
        ipcdrv_drv_err("fwnode_property_read_u32_array failed:%d\n", ret);
        return ret;
    }

    func |= (output[FUNC_CHAN_MODE_INDEX] ? FAST_MBOX : COMM_MBOX);

    func |= (output[FUNC_SRC_MODE_INDEX] ? SOURCE_MBOX : 0);

    func |= (output[FUNC_DST_MODE_INDEX] ? DESTINATION_MBOX : 0);

    priv->func = func;
    return 0;
}
STATIC int mdev_get_child_acpi(int ipc_id,
    struct hisi_ipc_device *idev,
    const struct fwnode_handle *fwnode,
    int capability, const int *child_irq, int irq_num)
{
    struct hisi_mbox_device_priv *priv = NULL;
    struct hisi_mbox_device **mdevs = idev->mdev_res;
    mbox_msg_t *buf_pool = idev->buf_pool;
    struct hisi_mbox_device *mdev = NULL;
    struct fwnode_handle *child = NULL;
    mbox_msg_t *ack_buffer = NULL;
    mbox_msg_t *rx_buffer = NULL;
    int ret = 0;
    int irq_idex = 0;
    int i = 0;

    fwnode_for_each_child_node(fwnode, child) {
        mdev = NULL;
        priv = NULL;
        rx_buffer = NULL;
        ack_buffer = NULL;

        mdev = kzalloc(sizeof(*mdev), GFP_KERNEL);
        if (mdev == NULL) {
            ret = -ENOMEM;
            ipcdrv_drv_err("kzalloc failed \n");
            goto to_break;
        }

        priv = kzalloc(sizeof(*priv), GFP_KERNEL);
        if (priv == NULL) {
            ret = -ENOMEM;
            ipcdrv_drv_err("kzalloc failed \n");
            goto free_mdev;
        }
        ret = mdev_parse_child(child, priv);
        if (ret != 0) {
            ipcdrv_drv_err("mdev_parse_child failed:%d\n", ret);
            goto free_priv;
        }
        /* mbx is enough, either used for recv or send */
        if (DESTINATION_MBOX & (priv->func)) {
            ret = fwnode_property_read_u32(child, "interrupts", (u32 *)&irq_idex);
            if ((irq_idex >= irq_num) || ret) {
                ipcdrv_drv_err("interrupts read error:%d\n", ret);
                goto free_priv;
            }
            priv->irq = child_irq[irq_idex];
        } else if (SOURCE_MBOX & (priv->func)) {
            /* tx channel need ack interrupt, they use common interrupt */
            priv->irq = hisi_mdev_get_irq_from_srcbit(idev, priv->src);
            /* set the cmdev, the cmdev will be used in acore't interrupts */
            if (idev->cmbox_info->cmdev == NULL) {
                idev->cmbox_info->cmdev = mdev;
            }
        } else {
            ipcdrv_drv_err("The dts config file may not correct\n");
            goto free_priv;
        }

        rx_buffer = buf_pool + capability * RX_BUFFER_TYPE;
        ack_buffer = buf_pool + capability * ACK_BUFFER_TYPE;
        buf_pool = buf_pool + capability * MBOX_BUFFER_TYPE_MAX;
        priv->idev = idev;
        priv->capability = capability;
        mdev->name = priv->src_name;  // dev_name;
        mdev->priv = priv;
        mdev->rx_buffer = rx_buffer;
        mdev->ack_buffer = ack_buffer;
        mdev->ops = &hisi_mdev_ops;
        mdev->configured = 0;
        mdev->ipc_id = ipc_id;
        mdev->thread_flag = 0;
        mdev->enable_irq_stage = ENABLE_IRQ_IN_INTERRUPT;
        mdev->rx_fifo_full_cnt = 0;
        mdevs[g_ipc_mdevs_index++] = mdev;
        i++;
        continue;
    free_priv:
        kfree(priv);
        priv = NULL;
    free_mdev:
        kfree(mdev);
        mdev = NULL;
    to_break:
        break;
    }

    if (ret != 0) {
        goto deinit_mdevs;
    }
    return ret;

deinit_mdevs:
    while (i--) {
        kfree(mdevs[i]->priv);
        mdevs[i]->priv = NULL;
        kfree(mdevs[i]);
        mdevs[i] = NULL;
    }
    return ret;
}

STATIC int mdev_get_by_acpi(int ipc_id,
    struct hisi_ipc_device *idev_ptr,
    struct hisi_mbox_device **mdevs,
    struct platform_device *pdev)
{
    struct hisi_common_mbox_info *cmbox_info = NULL;
    int ipc_ack_irq[HISI_HELPER_MAX_LPI_IRQ_NUM] = {0};
    mbox_msg_len_t buf_pool_len = 0;
    void __iomem *ipc_base = NULL;
    struct fwnode_handle *fwnode = NULL;
    struct hisi_ipc_device *idev = idev_ptr;
    mbox_msg_t *buf_pool = NULL;
    u64 ipc_reg_addr = 0;
    u64 ipc_reg_len = 0;
    u32 irq_num = 0;
    u32 capability = 0;
    u32 mdev_num = 0;
    int rp_num = 0;
    u32 unlock = 0;
    int ret = 0;
    u32 i;

    device_for_each_child_node(&pdev->dev, fwnode) {
        ret = fwnode_property_read_u64(fwnode, "ipc_reg_addr", &ipc_reg_addr);
        if (ret != 0) {
            ipcdrv_drv_err("ipc_base_addr read error:%d\n", ret);
            return (-ENODEV);
        }
        ret = fwnode_property_read_u64(fwnode, "ipc_reg_len", &ipc_reg_len);
        if (ret != 0) {
            ipcdrv_drv_err("ipc_base_addr read error:%d\n", ret);
            return (-ENODEV);
        }
        ipc_base = (void __iomem *)devm_ioremap(&pdev->dev, ipc_reg_addr, ipc_reg_len);
        if (ipc_base == NULL) {
            ipcdrv_drv_err("devm_ioremap for ipc_reg failed.\n");
            return (-ENODEV);
        }
        ret = fwnode_property_read_u32(fwnode, "capability", &capability);
        if (ret != 0) {
            ipcdrv_drv_err("prop \"capability\" error %d\n", ret);
            ret = -ENODEV;
            goto to_iounmap;
        }

        ret = fwnode_property_read_u32(fwnode, "unlock_word", &unlock);
        if (ret != 0) {
            ipcdrv_drv_err("prop \"unlock_word\" error %d\n", ret);
            ret = -ENODEV;
            goto to_iounmap;
        }

        ret = fwnode_property_read_u32(fwnode, "mailboxes", &mdev_num);
        if (ret != 0) {
            ipcdrv_drv_err("%s: prop \"mailboxes\" error %d\n", MODULE_NAME, ret);
            ret = -ENODEV;
            goto to_iounmap;
        }

        ret = fwnode_property_read_u32(fwnode, "irq_num", &irq_num);
        if (ret != 0) {
            ipcdrv_drv_err("%s: prop \"mailboxes\" error %d\n", MODULE_NAME, ret);
            ret = -ENODEV;
            goto to_iounmap;
        }

        cmbox_info = kzalloc(sizeof(*cmbox_info), GFP_KERNEL);
        if (cmbox_info == NULL) {
            ret = -ENOMEM;
            ipcdrv_drv_err("kzalloc failed\n");
            goto to_iounmap;
        }

        buf_pool_len = capability * MBOX_BUFFER_TYPE_MAX * mdev_num;
        buf_pool = kzalloc(sizeof(mbox_msg_t) * buf_pool_len, GFP_KERNEL);
        if (buf_pool == NULL) {
            ret = -ENOMEM;
            ipcdrv_drv_err("kzalloc failed\n");
            goto free_cmbox;
        }

        for (i = 0; i < irq_num; i++) {
            ipc_ack_irq[i] = fwnode_irq_get(fwnode, i);
        }

        ret = mdev_get_gic_irq(fwnode, cmbox_info, ipc_ack_irq);
        if (ret != 0) {
            ipcdrv_drv_err("mdev_get_gic_irq failed :%d\n", ret);
            goto deinit_mdevs;
        }
        cmbox_info->cmdev = NULL;

        idev->cmbox_info = cmbox_info;
        idev->unlock = unlock;
        idev->base = ipc_base;
        idev->lpi_base = 0;
        idev->mdev_res = mdevs;
        idev->buf_pool = buf_pool;
        idev->rp_num = rp_num;

        ret = mdev_get_child_acpi(ipc_id, idev,
            fwnode, (int)capability,
            ipc_ack_irq, HISI_HELPER_MAX_LPI_IRQ_NUM);
        if (ret != 0) {
            ipcdrv_drv_err("mdev_get_child_acpi failed :%d\n", ret);
            goto deinit_mdevs;
        }
        idev++;
    }
    return ret;

deinit_mdevs:
    kfree(buf_pool);
    buf_pool = NULL;
free_cmbox:
    kfree(cmbox_info);
    cmbox_info = NULL;
to_iounmap:
    devm_iounmap(&pdev->dev, ipc_base);
    return ret;
}
#endif
STATIC int mdev_get_func(struct hisi_ipc_device *idev, struct device_node *son, struct hisi_mbox_device *mdev,
    u8 *func, int *irq, remote_processor_type_t src_bit)
{
    u32 output[OUTPUT_LENGTH] = {0};
    int ret;

    ret = of_property_read_u32_array(son, "func", output, OUTPUT_LENGTH);
    if (ret != 0) {
        return ret;
    }

    *func |= (output[0] ? FAST_MBOX : COMM_MBOX);

    *func |= (output[1] ? SOURCE_MBOX : 0);

    *func |= (output[2] ? DESTINATION_MBOX : 0);

/* cloud and mini don't use common, only fast channel  */
#if !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3) && !defined(CFG_SOC_PLATFORM_CLOUD_V2)
    if (COMM_MBOX & (*func)) {
        /* normal mbx */
        ipcdrv_drv_err(" xxxxxxxxx we don't use comm-mailbox , we use it as fast-mailbox\n");
        return -EINVAL;
    }
#endif

    /* mbx is enough, either used for recv or send */
    if (DESTINATION_MBOX & (*func)) {
        *irq = (int)irq_of_parse_and_map(son, 0);
    } else if (SOURCE_MBOX & (*func)) {
        /* tx channel need ack interrupt, they use common interrupt */
        *irq = hisi_mdev_get_irq_from_srcbit(idev, src_bit);
        /* set the cmdev, the cmdev will be used in acore't interrupts */
        if (idev->cmbox_info->cmdev == NULL) {
            idev->cmbox_info->cmdev = mdev;
        }
    } else {
        ipcdrv_drv_err("The dts config file may not correct\n");
        ret = -EINVAL;
    }

    return ret;
}

STATIC int mdev_get_child(int ipc_id, struct hisi_ipc_device *idev, struct hisi_mbox_device **mdevs,
    struct device_node *node, int capability)
{
    struct hisi_mbox_device_priv *priv = NULL;
    u32 des_bit = 0;
    u32 src_bit = 0;
    const char *src_name = NULL;
    const char *dse_name = NULL;
    struct hisi_mbox_device *mdev = NULL;
    struct device_node *son = NULL;
    const char *mdev_name = NULL;
    mbox_msg_t *rx_buffer = NULL;
    mbox_msg_t *ack_buffer = NULL;
    mbox_msg_t *buf_pool = idev->buf_pool;
    u32 cpu_affinity = IPC_CPU_AFFINITY_DEFAULT;
    u8 func = 0;
    int ret = 0;
    int i = 0;
    u32 index;
    int irq = 0;

    for (i = 0; (son = of_get_next_child(node, son)); i++) {  //lint !e441
        mdev = NULL;
        priv = NULL;
        mdev_name = NULL;
        func = 0;
        src_name = NULL;
        dse_name = NULL;
        index = 0;
        rx_buffer = NULL;
        ack_buffer = NULL;

        mdev = kzalloc(sizeof(*mdev), GFP_KERNEL);
        if (mdev == NULL) {
            ret = -ENOMEM;
            goto to_break;
        }

        priv = kzalloc(sizeof(*priv), GFP_KERNEL);
        if (priv == NULL) {
            ret = -ENOMEM;
            goto free_mdev;
        }
#ifdef AOS_LLVM_BUILD
        mdev_name = of_get_name(son);
#else
        mdev_name = son->name;
#endif
        ret = of_property_read_string(son, "src_remote_processor", &src_name);
        if (ret != 0) {
            goto free_priv;
        }

        ret = of_property_read_string(son, "des_remote_processor", &dse_name);
        if (ret != 0) {
            goto free_priv;
        }
        ret = of_property_read_u32(son, "src_bit", (u32 *)&src_bit);
        if (ret != 0) {
            goto free_priv;
        }

        ret = of_property_read_u32(son, "des_bit", (u32 *)&des_bit);
        if (ret != 0) {
            goto free_priv;
        }

        ret = of_property_read_u32(son, "index", (u32 *)&index);
        if (ret != 0) {
            goto free_priv;
        }

        ret = of_property_read_u32(son, "irq_affinity", (u32 *)&cpu_affinity);
        if (ret != 0) {
            cpu_affinity = IPC_CPU_AFFINITY_DEFAULT;
            /* continue */
        }

        ret = mdev_get_func(idev, son, mdev, &func, &irq, (remote_processor_type_t)src_bit);
        if (ret != 0) {
            goto free_priv;
        }

        /* Alloc Buffer for every mailbox channel,
         * rx and tx all alloc memory pool (capability)
         */
        rx_buffer = buf_pool + capability * RX_BUFFER_TYPE;
        ack_buffer = buf_pool + capability * ACK_BUFFER_TYPE;
        buf_pool = buf_pool + (long)capability * MBOX_BUFFER_TYPE_MAX;

        priv->capability = capability;
        priv->func = func;
        priv->src_name = src_name;
        priv->dse_name = dse_name;
        priv->src = (remote_processor_type_t)src_bit;
        priv->des = (remote_processor_type_t)des_bit;
        priv->irq = irq;
        priv->irq_affinity = cpu_affinity;
        priv->index = index;
        priv->idev = idev;

        mdev->name = mdev_name;
        mdev->priv = priv;
        mdev->rx_buffer = rx_buffer;
        mdev->ack_buffer = ack_buffer;
        mdev->ops = &hisi_mdev_ops;
        mdev->configured = 0;
        mdev->ipc_id = ipc_id;
        mdev->thread_flag = 0;
        mdev->enable_irq_stage = ENABLE_IRQ_IN_INTERRUPT;
        mdev->rx_fifo_full_cnt = 0;
        mdevs[g_ipc_mdevs_index] = mdev;
        g_ipc_mdevs_index++;

        continue;
    free_priv:
        kfree(priv);
        priv = NULL;
    free_mdev:
        kfree(mdev);
        mdev = NULL;
    to_break:
        break;
    }

    if (ret != 0) {
        goto deinit_mdevs;
    }
    return ret;

deinit_mdevs:
    while (i--) {
        kfree(mdevs[i]->priv);
        kfree(mdevs[i]);
        mdevs[i] = NULL;
    }
    return ret;
}

STATIC int mdev_get(int ipc_id, struct hisi_ipc_device *idev_ptr,
    struct hisi_mbox_device **mdevs, struct device *dev)
{
    struct hisi_common_mbox_info *cmbox_info = NULL;
    int ipc_lpi_ack_irq[HISI_MAX_LPI_IRQ_NUM] = {0};
    struct hisi_ipc_device *idev = idev_ptr;
    struct device_node *dev_node = NULL;
    struct device_node *node = NULL;
    void __iomem *ipc_base = NULL;
    mbox_msg_t *buf_pool = NULL;
    mbox_msg_len_t buf_pool_len = 0;
    u32 ack_lpi_flag = 0;
    u32 capability = 0;
    int irq_offset = 0;
    u32 mdev_num = 0;
    u32 unlock = 0;
    int rp_num = 0;
    int ret = 0;
    int i = 0;

#if defined(CFG_SOC_PLATFORM_MINIV2) || defined(CFG_SOC_PLATFORM_MINIV3) || defined(CFG_SOC_PLATFORM_CLOUD_V2)
#ifndef UT_TEST
    dev_node = dev->of_node;
    for_each_child_of_node(dev_node, node) {
#endif
#else
    node = dev->of_node;
    (void)dev_node;
#endif
        ipc_base = of_iomap(node, 0);
        if (ipc_base == NULL) {
            ipcdrv_drv_err("ipc_id %d: iomap ipc_base fail\n", ipc_id);
            ret = -ENOMEM;
            goto out;
        }

        ret = of_property_read_u32(node, "capability", &capability);
        if (ret != 0) {
            ipcdrv_drv_err("ipc_id %d: prop \"capability\" error %d\n", ipc_id, ret);
            ret = -ENODEV;
            goto to_iounmap;
        }

        ret = of_property_read_u32(node, "unlock_key", &unlock);
        if (ret != 0) {
            ipcdrv_drv_err("ipc_id %d: prop \"unlock\" error %d\n", ipc_id, ret);
            ret = -ENODEV;
            goto to_iounmap;
        }

        ret = of_property_read_u32(node, "mailboxes", &mdev_num);
        if (ret != 0) {
            ipcdrv_drv_err("ipc_id %d: prop \"mailboxes\" error %d\n", ipc_id, ret);
            ret = -ENODEV;
            goto to_iounmap;
        }

        ret = of_property_read_u32(node, "ack_lpi_irq", (u32 *)&ack_lpi_flag);
        if (ret != 0) {
            ack_lpi_flag = 0;
            ipcdrv_drv_debug("ipc_id %d do not has \"ack_lpi_irq\", set ack_lpi_flag=0\n", ipc_id);
        }

        cmbox_info = kzalloc(sizeof(*cmbox_info), GFP_KERNEL);
        if (cmbox_info == NULL) {
            ipcdrv_drv_err("Alloc cmbox_info failed. ipc_id %d\n", ipc_id);
            ret = -ENOMEM;
            goto to_iounmap;
        }

        /* every ipc channel has capabilty data registers */
        buf_pool_len = capability * MBOX_BUFFER_TYPE_MAX * mdev_num;
        buf_pool = kzalloc(sizeof(mbox_msg_t) * buf_pool_len, GFP_KERNEL);
        if (buf_pool == NULL) {
            ipcdrv_drv_err("Alloc buf pool failed. ipc_id %d\n", ipc_id);
            ret = -ENOMEM;
            goto free_cmbox;
        }

        irq_offset = 0;
        for (i = 0; i < HISI_MAX_LPI_IRQ_NUM; i++) {
            if ((ack_lpi_flag != 0) && (ack_lpi_flag != 1) && (i == HISI_ACPU0_ACK_IRQ_IDX)) {
#ifndef UT_TEST
                irq_offset = HISI_ACPU0_ACK_IRQ_IDX;
                break;
#endif
            }

            ipc_lpi_ack_irq[i] = (int)irq_of_parse_and_map(node, i);
        }
        if (ack_lpi_flag == 1) {
            set_mbx_int_start_id(ipc_id, ipc_lpi_ack_irq[0]);
        }

        cmbox_info->cmbox_gic_1_irq = ipc_lpi_ack_irq[HISI_ACPU0_ACK_IRQ_IDX - irq_offset];
        cmbox_info->cmbox_gic_2_irq = ipc_lpi_ack_irq[HISI_ACPU1_ACK_IRQ_IDX - irq_offset];
        cmbox_info->cmbox_gic_3_irq = ipc_lpi_ack_irq[HISI_ACPU2_ACK_IRQ_IDX - irq_offset];
        cmbox_info->cmbox_gic_4_irq = ipc_lpi_ack_irq[HISI_ACPU3_ACK_IRQ_IDX - irq_offset];

        cmbox_info->gic_1_irq_requested = 0;
        cmbox_info->gic_2_irq_requested = 0;
        cmbox_info->gic_3_irq_requested = 0;
        cmbox_info->gic_4_irq_requested = 0;

        cmbox_info->cmdev = NULL;

        idev->cmbox_info = cmbox_info;
        idev->unlock = unlock;
        idev->base = ipc_base;
        idev->mdev_res = mdevs;
        idev->buf_pool = buf_pool;
        idev->rp_num = rp_num;

        ret = mdev_get_child(ipc_id, idev, mdevs, node, (int)capability);
        if (ret != 0) {
            ipcdrv_drv_err("ipc_id %d get child failed\n", ipc_id);
            goto deinit_mdevs;
        }
#if defined(CFG_SOC_PLATFORM_MINIV2) || defined(CFG_SOC_PLATFORM_MINIV3) || defined(CFG_SOC_PLATFORM_CLOUD_V2)
#ifndef UT_TEST
        idev++;
    }
#endif
#endif

    return ret;

deinit_mdevs:
    kfree(idev->buf_pool);
    idev->buf_pool = NULL;
free_cmbox:
    kfree(cmbox_info);
    cmbox_info = NULL;
to_iounmap:
    iounmap(ipc_base);
out:
    return ret;
}
#endif

STATIC int mdev_parse_dt(struct platform_device *pdev,
    u32 *mdev_num,
    int *ipc_id,
    int *ipc_phy_device_cnt)
{
    int ret;
    struct device_node *node = pdev->dev.of_node;
    if (node == NULL) {
        ipcdrv_drv_err("dts[%s] node not found\n", "hisilicon,HiIPCV230");
        return -ENODEV;
    }
    ret = of_property_read_u32(node, "mailboxes", mdev_num);
    if (ret != 0) {
        ipcdrv_drv_err("ret=%d, no mailboxes resources.\n", ret);
        return -ENODEV;
    }
#if defined(CFG_SOC_PLATFORM_MINIV2) || defined(CFG_SOC_PLATFORM_MINIV3) || defined(CFG_SOC_PLATFORM_CLOUD_V2)
    ret = of_property_read_u32(node, "ipc_id", (u32*)ipc_id);
    if (ret != 0) {
        ipcdrv_drv_err("no ipc_id resources\n");
        return -ENODEV;
    }
    *ipc_phy_device_cnt = of_get_child_count(node);
    if (*ipc_phy_device_cnt == 0) {
        ipcdrv_drv_err("ipc_id %d, get phy device count faild\n", *ipc_id);
        return -ENODEV;
    }
#endif
    return 0;
}

STATIC int mdev_parse_acpi(struct platform_device *pdev,
    u32 *mdev_num,
    int *ipc_id,
    int *ipc_phy_device_cnt)
{
#if defined(IPC_ACPI_SUPPORT) || defined(CFG_SOC_PLATFORM_CLOUD)
    int ret;
    if (!ACPI_COMPANION(&pdev->dev)) {
        return -ENODEV;
    }
    ret = device_property_read_u32(&pdev->dev, "ipc_id", (u32 *)ipc_id);
    if (ret != 0) {
        ipcdrv_drv_err("device_property_read_u32 read error:%d\n", ret);
        return -ENODEV;
    }
    ret = device_property_read_u32(&pdev->dev, "mailboxes", mdev_num);
    if (ret != 0) {
        ipcdrv_drv_err("ret=%d, no mailboxes resources.\n", ret);
        return -ENODEV;
    }
#if defined(CFG_SOC_PLATFORM_MINIV2) || defined(CFG_SOC_PLATFORM_MINIV3)
    *ipc_phy_device_cnt = (int)device_get_child_node_count(&pdev->dev);
    if (*ipc_phy_device_cnt == 0) {
        ipcdrv_drv_err("ipc_id %d, get phy device count faild\n", *ipc_id);
        return -ENODEV;
    }
#endif
#endif
    return 0;
}

STATIC int mdev_probe(struct platform_device *pdev)
{
    struct hisi_mbox_device **mdev_res = NULL;
    struct hisi_ipc_device *idev = NULL;
    int ipc_phy_device_cnt = 1;
    u32 mdev_num = 0;
    int ipc_id = 0;
    int ret;

    drv_snapshot_bootdot_init(IPC_MODULE_ID, SNAPSHOT_STATUS_STARTUP, IPC_STARTUP_EXPECT);
    drv_snapshot_bootdot_set(IPC_MODULE_ID, IPC_STARTUP_START);
#ifdef AOS_LLVM_BUILD
    ret= mdev_parse_dt(pdev, &mdev_num, &ipc_id, &ipc_phy_device_cnt);
#else

    if (acpi_disabled && (pdev->dev.of_node != NULL)) {
        ret= mdev_parse_dt(pdev, &mdev_num, &ipc_id, &ipc_phy_device_cnt);
    } else if (!acpi_disabled) {
        ret = mdev_parse_acpi(pdev, &mdev_num, &ipc_id, &ipc_phy_device_cnt);
    } else {
        return -EINVAL;
    }
#endif
    if (ret != 0) {
        ipcdrv_drv_err("ret=%d, mdev parse failed.\n", ret);
        return -ENODEV;
    }

    idev = kzalloc(sizeof(*idev) * (size_t)ipc_phy_device_cnt, GFP_KERNEL);
    if (idev == NULL) {
        ipcdrv_drv_err("ipc_id %d, no mem for ipc resouce\n", ipc_id);
        ret = -ENOMEM;
        goto out;
    }
    mdev_res = kzalloc((sizeof(char *) * (mdev_num + 1)), GFP_KERNEL); /* pointer type size, use sizeof(char *). */
    if (mdev_res == NULL) {
        ipcdrv_drv_err("ipc_id %d probe, kzalloc mdev resource failed\n", ipc_id);
        ret = -ENOMEM;
        goto free_idev;
    }

    g_ipc_mdevs_index = 0;
    mdev_res[mdev_num] = NULL;
#ifdef AOS_LLVM_BUILD
    ret = mdev_get(ipc_id, idev, mdev_res, &pdev->dev);
#else

    if (acpi_disabled && (pdev->dev.of_node != NULL)) {
        ret = mdev_get(ipc_id, idev, mdev_res, &pdev->dev);
    } else {
#if defined(CFG_SOC_PLATFORM_CLOUD)
        ret = mdev_get(ipc_id, idev, mdev_res, &pdev->dev);
#elif defined(IPC_ACPI_SUPPORT)
        ret = mdev_get_by_acpi(ipc_id, idev, mdev_res, pdev);
#else
        ret = -ENODEV;
#endif
    }
#endif
    if (ret) {
        ipcdrv_drv_err("ret=%d, get ipc resource failed\n", ret);
        ret = -ENODEV;
        goto free_mdevs;
    }
    drv_snapshot_bootdot_set(IPC_MODULE_ID, IPC_STARTUP_PARSE_DTS_FINISH);

    ret = mbox_device_register(ipc_id, &pdev->dev, mdev_res);
    if (ret) {
        drv_snapshot_bootdot_set(IPC_MODULE_ID, IPC_STARTUP_MBOX_REGISTER_FAIL);
        ipcdrv_drv_err("ret=%d, mdevs register failed\n", ret);
        ret = -ENODEV;
        goto put_res;
    }

    if ((rproc_init(ipc_id, (int)mdev_num)) != 0) {
        drv_snapshot_bootdot_set(IPC_MODULE_ID, IPC_STARTUP_RPROC_INIT_FAIL);
        ipcdrv_drv_err("ipc_id %d, rproc init failed\n", ipc_id);
        ret = -ENODEV;
        goto hisi_rproc_init_failed;
    }

    platform_set_drvdata(pdev, idev);

    ipcdrv_drv_info("ipc_id %d mdev_num %d probe success\n", ipc_id, mdev_num);
    drv_snapshot_bootdot_set(IPC_MODULE_ID, IPC_STARTUP_EXPECT);
    (void)dev_set_name(&pdev->dev, "ipcdrv");

    return 0;

hisi_rproc_init_failed:
    (void)mbox_device_unregister(mdev_res);
put_res:
    hisi_mdev_put(pdev, idev);
free_mdevs:
    kfree(mdev_res);
    mdev_res = NULL;
free_idev:
    kfree(idev);
    idev = NULL;
out:
    return ret;
}

STATIC int mdev_suspend(struct device *dev)
{
    struct platform_device *pdev = container_of(dev, struct platform_device, dev);
    struct hisi_ipc_device *idev = platform_get_drvdata(pdev);

    drv_snapshot_bootdot_init(IPC_MODULE_ID, SNAPSHOT_STATUS_SUSPEND, IPC_SUSPEND_EXPECT);
    drv_snapshot_bootdot_set(IPC_MODULE_ID, IPC_SUSPEND_START);

    ipcdrv_drv_info("%s: suspend start\n", __func__);
    if (idev != NULL) {
        mbox_device_deactivate(idev->mdev_res);
    }
    ipcdrv_drv_info("%s: suspend success\n", __func__);
    drv_snapshot_bootdot_set(IPC_MODULE_ID, IPC_SUSPEND_EXPECT);
    return 0;
}

STATIC int mdev_resume(struct device *dev)
{
    struct platform_device *pdev = container_of(dev, struct platform_device, dev);
    struct hisi_ipc_device *idev = platform_get_drvdata(pdev);

    drv_snapshot_bootdot_init(IPC_MODULE_ID, SNAPSHOT_STATUS_RESUME, IPC_RESUME_EXPECT);
    drv_snapshot_bootdot_set(IPC_MODULE_ID, IPC_RESUME_START);

    ipcdrv_drv_info("%s: resume start\n", __func__);
    if (idev != NULL) {
        mbox_device_activate(idev->mdev_res);
    }
    ipcdrv_drv_info("%s: resume success\n", __func__);
    drv_snapshot_bootdot_set(IPC_MODULE_ID, IPC_RESUME_EXPECT);
    return 0;
}
STATIC void mdev_shutdown(struct platform_device *pdev)
{
    (void)mdev_suspend(&pdev->dev);
}

static const struct dev_pm_ops hisi_mdev_pm_ops = {
    .suspend_late = mdev_suspend,
    .resume_early = mdev_resume,
};
#ifndef AOS_LLVM_BUILD
static const struct acpi_device_id hisi_mdev_acpi_match[] = {{ "IPCA0000", 0 }, {}};
MODULE_DEVICE_TABLE(acpi, hisi_mdev_acpi_match);
#endif
static const struct of_device_id g_hisi_mdev_of_match[] = {
    {
        .compatible = "hisi,hi-cloudv2-ipcdrv",
    },
    {
        .compatible = "hisi,hi-miniv2-ipcdrv",
    },
    {
        .compatible = "hisi,hi-miniv2-bs9sx1a-ipcdrv",
    },
    {
        .compatible = "hisi,hi-miniv3-ipcdrv",
    },
    {
        .compatible = "hisi,mini-ipcdrv",
    },
    {},
};

MODULE_DEVICE_TABLE(of, g_hisi_mdev_of_match);
static struct platform_driver hisi_mdev_driver = {
    .probe = mdev_probe,
    .remove = hisi_mdev_remove,
    .shutdown = mdev_shutdown,
    .driver = {
        .name = "HiIPCV230-mailbox",
#ifndef AOS_LLVM_BUILD
        .owner = THIS_MODULE,
        .acpi_match_table = ACPI_PTR(hisi_mdev_acpi_match),
#endif
        .of_match_table = of_match_ptr(g_hisi_mdev_of_match),
        .pm = &hisi_mdev_pm_ops,
    },
};


STATIC int __init mdev_init(void)
{
#ifdef CFG_SOC_PLATFORM_MINIV2
    mdev_init_chip_type();
#endif
    (void)mbox_init();
    INIT_LIST_HEAD(&g_mbox_irq_list);
    if (platform_driver_register(&hisi_mdev_driver)) {
        hisi_mbox_exit();
        ipcdrv_drv_err("platform driver register failed\n");
        return -ENODEV;
    }

    return 0;
}
#if defined(CFG_SOC_PLATFORM_MINIV2) || defined(CFG_SOC_PLATFORM_CLOUD_V2)
#ifdef AOS_LLVM_BUILD
module_init(mdev_init);
#else
subsys_initcall(mdev_init);
#endif
#else
module_init(mdev_init);
#endif

STATIC void __exit hisi_mdev_exit(void)
{
    platform_driver_unregister(&hisi_mdev_driver);
    hisi_mbox_exit();
    return;
}
module_exit(hisi_mdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("HiIPCV230 ipc, mailbox device driver");
