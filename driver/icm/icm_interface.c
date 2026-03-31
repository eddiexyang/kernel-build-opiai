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

#include <linux/delay.h>
#include <linux/types.h>
#include <linux/securec.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif
#include "icm_core.h"
#include "icm_interface.h"

ICM_HANDLE icm_msg_chan_alloc(u32 dev_id, u32 peer)
{
    if ((icm_ctrl->adapter == NULL) || (icm_ctrl->adapter->alloc == NULL)) {
        icm_err("ICM is not initialized\n");
        return ICM_INVAILED_FD;
    }
    return icm_ctrl->adapter->alloc(dev_id, peer);
}
EXPORT_SYMBOL(icm_msg_chan_alloc);

void icm_msg_chan_free(ICM_HANDLE fd)
{
    if ((icm_ctrl->adapter == NULL) || (icm_ctrl->adapter->free == NULL)) {
        icm_err("ICM is not initialized\n");
        return;
    }
    icm_ctrl->adapter->free(fd);
}
EXPORT_SYMBOL(icm_msg_chan_free);

int icm_msg_send_async(ICM_HANDLE fd, u32 *msg, u32 send_len)
{
    if ((icm_ctrl->adapter == NULL) || (icm_ctrl->adapter->send_async == NULL)) {
        icm_err("ICM is not initialized\n");
        return -EINVAL;
    }
    return icm_ctrl->adapter->send_async(fd, msg, send_len);
}
EXPORT_SYMBOL(icm_msg_send_async);

int icm_msg_fast_send_async(ICM_HANDLE fd, u32 *msg, u32 send_len)
{
    if ((icm_ctrl->adapter == NULL) || (icm_ctrl->adapter->fast_send_async == NULL)) {
        icm_err("ICM is not initialized\n");
        return -EINVAL;
    }
    return icm_ctrl->adapter->fast_send_async(fd, msg, send_len);
}
EXPORT_SYMBOL(icm_msg_fast_send_async);

int icm_msg_send_sync(ICM_HANDLE fd, u32 *msg, u32 send_len, u32 *ack_buf,
    u32 ack_len)
{
    if ((icm_ctrl->adapter == NULL) || (icm_ctrl->adapter->send_sync == NULL)) {
        icm_err("ICM is not initialized\n");
        return -EINVAL;
    }
    return icm_ctrl->adapter->send_sync(fd, msg, send_len, ack_buf, ack_len);
}
EXPORT_SYMBOL(icm_msg_send_sync);

int icm_rx_register(u32 dev_id, u32 chan_id, u32 peer, struct notifier_block *nb)
{
    if ((icm_ctrl->adapter == NULL) || (icm_ctrl->adapter->rx_register == NULL)) {
        icm_err("ICM is not initialized\n");
        return -EINVAL;
    }
    return icm_ctrl->adapter->rx_register(dev_id, chan_id, peer, nb);
}
EXPORT_SYMBOL(icm_rx_register);

int icm_rx_unregister(u32 dev_id, u32 chan_id, u32 peer, struct notifier_block *nb)
{
    if ((icm_ctrl->adapter == NULL) || (icm_ctrl->adapter->rx_unregister == NULL)) {
        icm_err("ICM is not initialized\n");
        return -EINVAL;
    }
    return icm_ctrl->adapter->rx_unregister(dev_id, chan_id, peer, nb);
}
EXPORT_SYMBOL(icm_rx_unregister);
