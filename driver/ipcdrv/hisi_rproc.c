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

#include <linux/wait.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/securec.h>
#include "hisi_rproc.h"
#include "hisi_rproc_adapt.h"
#include "hisi_mailbox_dev.h"

#define MODULE_NAME "hisi_rproc"

STATIC int g_isReady[MAX_IPCDEV_NUM] = {0};
#if (defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MINIV2)) && !defined(CFG_SOC_PLATFORM_MINIV3)
STATIC int g_rproc_table_num[MAX_IPCDEV_NUM] = {0};
#endif
#define READY(ipc_id) do { \
    g_isReady[ipc_id] = 1; \
} while (0)
#define NOT_READY(ipc_id) do { \
    g_isReady[ipc_id] = 0; \
} while (0)
#define IS_READY(ipc_id) ({ g_isReady[ipc_id]; })

typedef enum {
    ASYNC_CALL = 0,
    SYNC_CALL
} call_type_t;

STATIC struct hisi_rproc_info *get_rproc_buff(int ipc_id, int *rproc_num)
{
#if (defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MINIV2)) && !defined(CFG_SOC_PLATFORM_MINIV3)
    *rproc_num = g_rproc_table_num[ipc_id];
    return g_rproc_table[ipc_id];
#else
    *rproc_num = HISI_RPROC_MAX;
    return (&g_rproc_table[ipc_id][0]);
#endif
}

struct hisi_rproc_info *find_rproc(int ipc_id, rproc_id_t rproc_id)
{
    struct hisi_rproc_info *rproc = NULL;
    int i, rproc_num;

    if (ipc_id < 0 || ipc_id >= MAX_IPCDEV_NUM) {
        ipcdrv_drv_err("ipc_id %d out of range, max %d\n", ipc_id, MAX_IPCDEV_NUM);
        return NULL;
    }

    rproc = get_rproc_buff(ipc_id, &rproc_num);
    if (rproc == NULL) {
        ipcdrv_drv_err("ipc_id:%d no g_rproc_table registered\n", ipc_id);
        return NULL;
    }
    for (i = 0; i < rproc_num; i++) {
        if (rproc_id == rproc[i].rproc_id) {
            return (&rproc[i]);
        }
    }

    return NULL;
}

STATIC int rproc_xfer_params_check(int ipc_id, rproc_msg_t *msg, rproc_msg_len_t len)
{
    size_t len_tmp;

    if (msg == NULL) {
        ipcdrv_drv_err("Msg is null, ipc_id=%d\n", ipc_id);
        return -EINVAL;
    }

    if (ipc_id < 0 || ipc_id >= MAX_IPCDEV_NUM) {
        ipcdrv_drv_err("ipc_id %d out of range\n", ipc_id);
        return -EINVAL;
    }

    if (!IS_READY(ipc_id)) {
        ipcdrv_drv_err("ipc_id %d not ready\n",  ipc_id);
        return -EINVAL;
    }

    len_tmp = len * sizeof(rproc_msg_t);
    if ((len_tmp < sizeof(struct ipcdrv_msg_header)) || (len > MBOX_CHAN_DATA_SIZE)) {
        ipcdrv_drv_err("ipc_id %d msglen is invalid\n",  ipc_id);
        return -EINVAL;
    }

    return 0;
}

int rproc_xfer_async(int ipc_id, rproc_id_t rproc_id, rproc_msg_t *msg, rproc_msg_len_t len)
{
    mbox_ack_type_t ack_type = AUTO_ACK;
    struct hisi_mbox_task *tx_task = NULL;
    struct hisi_rproc_info *rproc = NULL;
    struct hisi_mbox *mbox = NULL;
    const char *rproc_name = NULL;
    int ret = 0;

    ret = rproc_xfer_params_check(ipc_id, msg, len);
    if (ret != 0) {
        ipcdrv_drv_err("ipc_id:%d rproc_id:%d invalid params.\n", ipc_id, (u32)rproc_id);
        ret = -EINVAL;
        goto out;
    }

    rproc = find_rproc(ipc_id, rproc_id);
    if (rproc == NULL) {
        ipcdrv_drv_err("ipc_id:%d rproc_id:%d invalid rproc xfer\n", ipc_id, (u32)rproc_id);
        ret = -EINVAL;
        goto out;
    }

    mbox = rproc->mbox;
    rproc_name = rproc->mbox_rp;

    /* collect all information to tx_task */
    tx_task = mbox_task_alloc(ipc_id, mbox, rproc_name, msg, len, (int)ack_type);
    if (tx_task == NULL) {
        ipcdrv_drv_err("no mem\n");
        ret = -ENOMEM;
        goto out;
    }

    ret = mbox_msg_send_async(mbox, tx_task);
    if (ret != 0) {
        ipcdrv_drv_warn("async send not success.(mdev=%s, rproc_id=%d, ret=%d)\n",
            mbox->tx->name, (u32)rproc_id, ret);
        mbox_task_free(ipc_id, &tx_task);
    }

out:
    return ret;
}
EXPORT_SYMBOL(rproc_xfer_async);

int fast_rproc_xfer_async(int ipc_id, rproc_id_t rproc_id, rproc_msg_t *msg, rproc_msg_len_t len)
{
    struct hisi_rproc_info *rproc = NULL;
    struct hisi_mbox_device *mdev = NULL;
    int ret = 0;

    ret = rproc_xfer_params_check(ipc_id, msg, len);
    if (ret != 0) {
        ipcdrv_drv_err("ipc_id:%d rproc_id:%d invalid params.\n", ipc_id, (u32)rproc_id);
        return -EINVAL;
    }

    rproc = find_rproc(ipc_id, rproc_id);
    if (rproc == NULL) {
        ipcdrv_drv_err("ipc_id:%d rproc_id:%d invalid rproc xfer\n", ipc_id, (u32)rproc_id);
        return -EINVAL;
    }
    mdev = rproc->mbox->tx;

    mutex_lock(&mdev->dev_lock);
    mdev->ops->ensure_channel(mdev);
    ret = mdev->ops->send(mdev, msg, len, AUTO_ACK);
    if (ret != 0) {
        mutex_unlock(&mdev->dev_lock);
        ipcdrv_drv_err("can not be sended.(mdev=%s, rproc_id=%d)\n", mdev->name, (u32)rproc_id);
        return -EINVAL;
    }
    mutex_unlock(&mdev->dev_lock);

    return 0;
}
EXPORT_SYMBOL(fast_rproc_xfer_async);

STATIC int hisi_rproc_rx_notifier(struct notifier_block *nb, unsigned long len, void *msg)
{
    struct hisi_rproc_info *rproc = NULL;

    if ((nb == NULL) || (nb->notifier_call == NULL)) {
        ipcdrv_drv_err("nb is null pointer\n");
        return 0;
    }

    if (msg == NULL) {
        ipcdrv_drv_err("msg is null pointer\n");
        return 0;
    }

    rproc = container_of(nb, struct hisi_rproc_info, nb);
    if (rproc == NULL) {
        ipcdrv_drv_err("rproc is null pointer\n");
        return 0;
    }

    (void)atomic_notifier_call_chain(&rproc->notifier, len, msg);
    return 0;
}

int rproc_rx_register(int ipc_id, rproc_id_t rproc_id, struct notifier_block *nb)
{
    struct hisi_rproc_info *rproc = NULL;
    int ret = 0;

    if (nb == NULL) {
        ipcdrv_drv_err("null pointer nb\n");
        return -EINVAL;
    }

    if (ipc_id < 0 || ipc_id >= MAX_IPCDEV_NUM) {
        ipcdrv_drv_debug("ipc_id %d is invalid\n", ipc_id);
        return -EINVAL;
    }

    if (!IS_READY(ipc_id)) {
        ipcdrv_drv_debug("ipc_id %d is not ready\n", ipc_id);
        return -EINVAL;
    }

    rproc = find_rproc(ipc_id, rproc_id);
    if (rproc == NULL) {
        ipcdrv_drv_debug("ipc_id:%d rproc_id:%d invalid rproc xfer\n", ipc_id, (u32)rproc_id);
        ret = -EINVAL;
        goto out;
    }
    (void)atomic_notifier_chain_register(&rproc->notifier, nb);
out:
    return ret;
}

EXPORT_SYMBOL(rproc_rx_register);

int rproc_rx_unregister(int ipc_id, rproc_id_t rproc_id, struct notifier_block *nb)
{
    struct hisi_rproc_info *rproc = NULL;
    int ret = 0;

    if (nb == NULL) {
        ipcdrv_drv_err("null pointer nb\n");
        return -EINVAL;
    }

    if (ipc_id < 0 || ipc_id >= MAX_IPCDEV_NUM) {
        ipcdrv_drv_debug("ipc_id %d is invalid\n", ipc_id);
        return -EINVAL;
    }

    if (!IS_READY(ipc_id)) {
        ipcdrv_drv_debug("ipc_id %d is not ready\n", ipc_id);
        return -EINVAL;
    }

    rproc = find_rproc(ipc_id, rproc_id);
    if (rproc == NULL) {
        ipcdrv_drv_err("invalid rproc xfer\n");
        ret = -EINVAL;
        goto out;
    }
    (void)atomic_notifier_chain_unregister(&rproc->notifier, nb);
out:
    return ret;
}
EXPORT_SYMBOL(rproc_rx_unregister);

/*
 * Function name:hs_rproc_put.
 * Discription:release the ipc channel's structure, it's usually called by  module_exit function, but the module_exit
 * function should never be used  . Parameters:
 * @ rproc_id_t
 * return value:
 * @ -ENODEV-->failed, other-->succeed.
 */
int hs_rproc_put(int ipc_id, rproc_id_t rproc_id)
{
    struct hisi_rproc_info *rproc = NULL;
    int i, rproc_num;

    if (ipc_id < 0 || ipc_id >= MAX_IPCDEV_NUM) {
        return -ENODEV;
    }

    rproc = get_rproc_buff(ipc_id, &rproc_num);
    if (rproc == NULL) {
        ipcdrv_drv_err("ipc_id:%d no g_rproc_table registered\n", ipc_id);
        return -ENODEV;
    }
    for (i = 0; i < rproc_num; i++) {
        if ((rproc[i].mbox != NULL) && (rproc_id == rproc[i].rproc_id)) {
            hisi_mbox_put(&rproc[i].mbox);
            break;
        }
    }
    if (unlikely(i == HISI_RPROC_MAX)) {
        ipcdrv_drv_err("release the ipc channel %d 's structure failed\n", (int)rproc_id);
        return -ENODEV;
    }
    return 0;
}

/*
 * Function name:rproc_flush_tx.
 * Discription:flush the tx_work queue.
 * Parameters:
 *	@ rproc_id_t
 * return value:
 *	@ -ENODEV-->failed, other-->succeed.
 */
int rproc_flush_tx(int ipc_id, rproc_id_t rproc_id)
{
    struct hisi_rproc_info *rproc = NULL;
    int i;
    int rproc_num;

    if ((ipc_id < 0) || (ipc_id >= MAX_IPCDEV_NUM) || (IS_READY(ipc_id) == 0)) {
        ipcdrv_drv_err("ipc_id %d is invalid.\n", ipc_id);
        return -ENODEV;
    }

    rproc = get_rproc_buff(ipc_id, &rproc_num);
    if (rproc == NULL) {
        ipcdrv_drv_err("ipc_id:%d no g_rproc_table registered\n", ipc_id);
        return -ENODEV;
    }
    for (i = 0; i < rproc_num; i++) {
        if ((rproc[i].mbox->tx != NULL) && (rproc_id == rproc[i].rproc_id)) {
            mbox_empty_task(rproc[i].mbox->tx);
            break;
        }
    }

    if (unlikely(i == HISI_RPROC_MAX)) {
        return -ENODEV;
    }
    return 0;
}
EXPORT_SYMBOL(rproc_flush_tx);

#if defined (CFG_SOC_PLATFORM_MINIV2) && !defined (AOS_LLVM_BUILD)
int hisi_rproc_v2_chan(rproc_id_t id)
{
    if ((id == HISI_RPROC_ISP1_IPC0_MBX0_RX_RPID0) || (id == HISI_RPROC_ISP1_IPC0_MBX1_RX_RPID0) ||
        (id == HISI_RPROC_ISP1_IPC0_MBX2_RX_RPID0) || (id == HISI_RPROC_ISP1_IPC0_MBX6_TX_RPID1) ||
        (id == HISI_RPROC_ISP1_IPC0_MBX7_TX_RPID2) || (id == HISI_RPROC_ISP1_IPC0_MBX8_TX_RPID3) ||
        (id == HISI_RPROC_ISP1_IPC1_MBX0_RX_RPID0) || (id == HISI_RPROC_ISP1_IPC1_MBX1_RX_RPID0) ||
        (id == HISI_RPROC_ISP1_IPC1_MBX2_RX_RPID0) || (id == HISI_RPROC_ISP1_IPC1_MBX6_TX_RPID6) ||
        (id == HISI_RPROC_ISP1_IPC1_MBX7_TX_RPID7) || (id == HISI_RPROC_ISP1_IPC1_MBX8_TX_RPID8)) {
        return 0;
    }
    return -1;
}
#endif /* end of MINIV2 */

#if (defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MINIV2)) && !defined(CFG_SOC_PLATFORM_MINIV3)
int rproc_id_init(int ipc_id, int max_mdev_num, struct hisi_rproc_info *rproc)
{
    int mbox_num = 0;
    u32 i;

    for (i = 0; i < HISI_RPROC_MAX; i++) {
        if (mbox_is_exist_by_rp(ipc_id, g_rproc_cfg[i].mbox_rp) == -1) {
            continue;
        }
        if (mbox_num >= max_mdev_num) {
            ipcdrv_drv_err("mbox init match fail.(ipc_id=%d, mbox_num=%d, max_mdev_num=%d)\n",
                ipc_id, mbox_num, max_mdev_num);
            return 0;
        }
        ipcdrv_drv_info("mbox init success.(ipc_id=%d, rproc_id=%d, mbox_rp=%s)\n",
            ipc_id, (u32)g_rproc_cfg[i].rproc_id, g_rproc_cfg[i].mbox_rp);

        rproc[mbox_num].rproc_id = g_rproc_cfg[i].rproc_id;
        rproc[mbox_num].mbox_rp = g_rproc_cfg[i].mbox_rp;
        rproc[mbox_num].mbox = NULL;
        mbox_num++;
    }
    if (mbox_num != max_mdev_num) {
        ipcdrv_drv_err("mbox num not match.(ipc_id=%d, mbox_num=%d, max_mdev_num=%d)\n",
            ipc_id, mbox_num, max_mdev_num);
    }
    return mbox_num;
}
#endif

STATIC void rproc_table_init(int ipc_id, int mdev_num)
{
#if (defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MINIV2)) && !defined(CFG_SOC_PLATFORM_MINIV3)
    int ret_rproc_num;
    struct hisi_rproc_info *rproc = kzalloc(sizeof(struct hisi_rproc_info) * mdev_num, GFP_KERNEL);
    if (rproc == NULL) {
        return;
    }
    ret_rproc_num = rproc_id_init(ipc_id, mdev_num, rproc);
    if (ret_rproc_num <= 0) {
        ipcdrv_drv_err("ipc_id %d init match fail \n", ipc_id);
        kfree(rproc);
        return;
    }
    g_rproc_table_num[ipc_id] = ret_rproc_num;
    g_rproc_table[ipc_id] = rproc;
#endif
    return;
}

STATIC void rproc_mbx_clear(int ipc_id, int rproc_num, struct hisi_rproc_info *rproc)
{
    int i;

    for (i = 0; i < rproc_num; i++) {
        if (rproc[i].mbox != NULL) {
            hisi_mbox_put(&rproc[i].mbox);
        }
        mutex_destroy(&rproc[i].rproc_mutex);
    }

#if (defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MINIV2)) && !defined(CFG_SOC_PLATFORM_MINIV3)
    if (rproc != NULL) {
        kfree(rproc);
    }
    g_rproc_table_num[ipc_id] = 0;
#endif
    return;
}

int rproc_init(int ipc_id, int mdev_num)
{
    struct hisi_mbox_task *ptask = NULL;
    struct hisi_rproc_info *rproc = NULL;
    struct hisi_rproc_info *rproc_local = NULL;

    int ret = 0;
    int i;
    int rproc_num = mdev_num;

    if (ipc_id < 0 || ipc_id >= MAX_IPCDEV_NUM) {
        return -ENODEV;
    }

    rproc_table_init(ipc_id, rproc_num);
    rproc = get_rproc_buff(ipc_id, &rproc_num);
    if (rproc == NULL) {
        ipcdrv_drv_err("ipc_id:%d no g_rproc_table registered\n", ipc_id);
        return -ENODEV;
    }

    for (i = 0; i < rproc_num; i++) {
        rproc_local = &rproc[i];
#if defined (CFG_SOC_PLATFORM_MINIV2) && !defined (AOS_LLVM_BUILD)
        if ((hisi_mdev_get_chip_type() == HISI_MINIV2_V2) && (hisi_rproc_v2_chan(rproc_local->rproc_id) == 0)) {
            continue;
        }
#endif
        if ((rproc_local->mbox == NULL) && (rproc_local->mbox_rp != NULL)) {
            ATOMIC_INIT_NOTIFIER_HEAD(&rproc_local->notifier);

            rproc_local->nb.next = NULL;
            mutex_init(&rproc_local->rproc_mutex);
            rproc_local->register_flag = 0;

            rproc_local->nb.notifier_call = hisi_rproc_rx_notifier;
            rproc_local->mbox = mbox_get(ipc_id, rproc_local->mbox_rp, &rproc_local->nb);
            if (rproc_local->mbox == NULL) {
                ipcdrv_drv_err("ipc_id %d rproc %d will get later \n", ipc_id, (int)rproc_local->rproc_id);
                ret = -ENODEV;
                goto hisi_mbx_init_failed;
            }
            if (rproc_local->mbox->rx != NULL) {
                rproc_local->mbox->rx->rproc_id = rproc_local->rproc_id;
            }
        }
        /* the last rproc info has been initialize, set the rproc ready */
        if ((i == (rproc_num - 1)) && (rproc_local->mbox != NULL)) {
            READY(ipc_id);
            ipcdrv_drv_info("ipc_id %d  rproc_num %d mdev_num %d rproc initialized\n", ipc_id, rproc_num, mdev_num);
        }
    }

    if (!g_TxTaskBuffer[ipc_id]) {
        g_TxTaskBuffer[ipc_id] = (struct hisi_mbox_task *)kzalloc(TX_TASK_DDR_NODE_NUM * sizeof(struct hisi_mbox_task),
                                                                  GFP_KERNEL);
        if (g_TxTaskBuffer[ipc_id] == NULL) {
            ipcdrv_drv_err("failed to get g_TxTaskBuffer\n");
            ret = -ENOMEM;
            goto hisi_txbuffer_init_failed;
        }
        ptask = g_TxTaskBuffer[ipc_id];
        for (i = 0; i < TX_TASK_DDR_NODE_NUM; i++) {
            /* init the tx buffer 's node , set the flag to available */
            ptask->tx_buffer_status = TX_TASK_DDR_NODE_VALID;
            ptask++;
        }
    }

    return ret;

hisi_txbuffer_init_failed:
hisi_mbx_init_failed:
    rproc_mbx_clear(ipc_id, rproc_num, rproc);

    return ret;
}

EXPORT_SYMBOL(rproc_init);

void hisi_rproc_exit(int ipc_id)
{
    struct hisi_rproc_info *rproc = NULL;
    int rproc_num;

    if (ipc_id < 0 || ipc_id >= MAX_IPCDEV_NUM) {
        return;
    }

    NOT_READY(ipc_id);

    rproc = get_rproc_buff(ipc_id, &rproc_num);
    if (rproc == NULL) {
        rproc_num = 0;
        ipcdrv_drv_err("ipc_id:%d no g_rproc_table registered\n", ipc_id);
    }
    rproc_mbx_clear(ipc_id, rproc_num, rproc);

    if (g_TxTaskBuffer[ipc_id] != NULL) {
        kfree(g_TxTaskBuffer[ipc_id]);
    }

    g_TxTaskBuffer[ipc_id] = NULL;
    return;
}
EXPORT_SYMBOL(hisi_rproc_exit);

void rproc_print_ipc_status(int ipc_id, rproc_id_t rproc_id)
{
    struct hisi_mbox_device *mdev_tx = NULL;
    struct hisi_mbox_device *mdev_rx = NULL;
    struct hisi_rproc_info *rproc = NULL;
    struct hisi_mbox *mbox = NULL;

    if ((ipc_id < 0) || (ipc_id >= MAX_IPCDEV_NUM) || (!IS_READY(ipc_id))) {
        ipcdrv_drv_err("ipc_id %d is invalid.\n", ipc_id);
        return ;
    }
    rproc = find_rproc(ipc_id, rproc_id);
    if (rproc == NULL) {
        ipcdrv_drv_err("invalid rproc, rproc id = %d.\n", (int)rproc_id);
        return ;
    }
    mbox = rproc->mbox;
    if (mbox == NULL) {
        ipcdrv_drv_err("invalid mbox.\n");
        return ;
    }
    mdev_tx = mbox->tx;
    mdev_rx = mbox->rx;

    if ((mdev_tx != NULL) && (mdev_tx->ops != NULL) && (mdev_tx->ops->status != NULL)) {
        mdev_tx->ops->status(mdev_tx);
    }
    if ((mdev_rx != NULL) && (mdev_rx->ops != NULL) && (mdev_rx->ops->status != NULL)) {
        mdev_rx->ops->status(mdev_rx);
    }
}
EXPORT_SYMBOL(rproc_print_ipc_status);

