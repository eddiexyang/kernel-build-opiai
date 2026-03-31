/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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

#ifndef _AGENTDRV_PCI_H_
#define _AGENTDRV_PCI_H_

#include "agentdrv_msg.h"
#include "agentdrv_unit.h"
extern struct agentdrv_dfx_log_info g_dfx_info;

#define AGENTDEV_DEV_IS_INIT 1
#define AGENTDEV_WAIT_DEV_READY_TIME 30000 /* 30s */

#define AGENTDRV_STATUS_WAIT_DEV_PROBE 1
#define AGENTDRV_STATUS_WAIT_LOCAL_DEV_ADD 2
#define AGENTDRV_STATUS_WAIT_HOST 3
#define AGENTDRV_STATUS_OK 4
#define AGENTDRV_STATUS_WAIT_DEV_RESUME 5
#define AGENTDRV_STATUS_WAIT_NOTIFY_RESUME 6

#define AGENTDRV_LOCAL_STATUS_OFFLINE 0
#define AGENTDRV_LOCAL_STATUS_ONLINE 1

#define AGENTDRV_DMA_BIT_MASK_64 64
#define AGENTDRV_DMA_BIT_MASK_32 32
#define AGENTDRV_TIMEOUT_DIV 1000
#define AGENTDRV_PCI_MSI_NUM 32
#define AGENTDRV_PARA_NUM 2
#define AGENTDRV_MSLEEP_2 2
#define AGENTDRV_USLEEP_2000 2000
#define AGENTDRV_USLEEP_RANGE_10 10

#define AGENTDRV_SECOND_TO_MSECOND 1000
#define AGENTDRV_GET_INIT_TIME_DELAY 2000
#define AGENTDRV_GET_INIT_TIME_RANGE 10

#define AGENTDRV_GET_EP_SUSPEND_STATUS_DELAY 20
#define AGENTDRV_GET_EP_SUSPEND_STATUS_TIMEOUT 500
#define AGENTDRV_SEND_EP_HANDSHAKE_MSG_TIMEOUT 5000

#define AGENTDRV_WAIT_STATE_STATUS_DELAY   10
#define AGENTDRV_WAIT_STATE_STATUS_TIMEOUT 100

#define AGENTDRV_SVM0_PCI_DEV_INDEX_BASE 1

#define AGENTDRV_PCI_BAR_2 2 /* IEP Bar 2, not host insight */
#ifdef CFG_FEATURE_PM
void agentdrv_dfm_dump(u32 excep_id, u32 etype, u32 module_id, char **black_box_info);
#else
void agentdrv_dfm_dump(u32 dev_id, u32 excep_id, u32 etype, u32 module_id, char **black_box_info);
#endif
int agentdrv_notify_dev_online(struct agentdrv_msg_dev *msg_dev, void *data);
int agentdrv_dma_proc_func_init(struct pci_dev *pdev, struct agentdrv_devctrl *agent_dev, u32 func_idx);
int agentdrv_get_ep_suspend_status(struct agentdrv_msg_dev *msg_dev, void *data);
void agentdrv_wake_up_ep_suspend(void);

typedef int (*svm_register_pci_device_syms)(struct device *dev, u32 key);
typedef int (*svm_unregister_pci_device_syms)(struct device *dev, u32 key);
typedef int (*svm_enable_sva_enforce_syms)(struct device *dev);

#endif
