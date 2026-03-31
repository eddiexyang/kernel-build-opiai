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

#ifndef TOPIC_SCHED_H
#define TOPIC_SCHED_H

#include "topic_sched_chip_def.h"

#define TOPIC_SCHED_PER_CPU_RSV_MEM_SIZE (4 * 1024)

#define TOPIC_SCHED_NS_REG_OFFSET 0x800000
#define TOPIC_SCHED_NS_REG_SIZE (8 * 1024 * 1024)

#define TOPIC_SCHED_CPU_INTR_START_INDEX 12

/* user process kernel type config */
#define STARS_TOPIC_KERNEL_TYPE_USER 0xe40

/* core group pool config reg */
#define TOPIC_SCHED_DCPU_POOL_CFG_OFFSET 8
#define TOPIC_SCHED_CCPU_POOL_CFG_OFFSET 16
#define TOPIC_SCHED_TSCPU_POOL_CFG_OFFSET 24
#define TOPIC_SCHED_DVPPCPU_POOL_CFG_OFFSET 24

#define STARS_TOPIC_DEVICE_CPU_FREE_BITMAP  0x830 /* 1:idle, 0 busy */

#define STARS_TOPIC_DEVICE_POOL_ENABLE_CTRL_NS(pool_id)  (0x840 + ((pool_id) * 0x4))
#define STARS_TOPIC_DEVICE_POOL_DISABLE_CTRL_NS(pool_id) (0x880 + ((pool_id) * 0x4))
#define STARS_TOPIC_DEVICE_POOL_STATUS_NS(pool_id)       (0x8c0 + ((pool_id) * 0x4))

/* cpu mailbox config reg */
#define TOPIC_SCHED_MAILBOX_NUM 8
#define STARS_TOPIC_DEVICE_WAIT_MAILBOX_ADDR_LOW_NS(mbx_id)      (0xc00 + ((mbx_id) * 0x10))
#define STARS_TOPIC_DEVICE_WAIT_MAILBOX_ADDR_HIGH_NS(mbx_id)     (0xc04 + ((mbx_id) * 0x10))
#define STARS_TOPIC_DEVICE_ACPU_GET_MAILBOX_ADDR_LOW_NS(mbx_id)  (0xc08 + ((mbx_id) * 0x10))
#define STARS_TOPIC_DEVICE_ACPU_GET_MAILBOX_ADDR_HIGH_NS(mbx_id) (0xc0c + ((mbx_id) * 0x10))

/* wait/get status and report reg */
#define STARS_TOPIC_DEVICE_CPU_STATUS_REPORT_NS(mbx_id)     (0x2000 + ((mbx_id) * 0x20))
#define STARS_TOPIC_DEVICE_CPU_ERRCODE_REPORT_NS(mbx_id)    (0x200C + ((mbx_id) * 0x20))
#define STARS_TOPIC_DEVICE_CPU_GET_STATUS_REPORT_NS(mbx_id) (0x2010 + ((mbx_id) * 0x20))
#define STARS_TOPIC_DEVICE_CPU_WAIT_TOPIC_NS(mbx_id)        (0x2004 + ((mbx_id) * 0x20))
#define STARS_TOPIC_DEVICE_CPU_GET_TOPIC_NS(mbx_id)         (0x2014 + ((mbx_id) * 0x20))
#define STARS_TOPIC_DEVICE_CPU_INT_EN_NS(mbx_id)            (0x2008 + ((mbx_id) * 0x20))

/* process table config */
#define TOPIC_SCHED_PID_TABLE_SIZE 64
#define STARS_TOPIC_PID_MAP_VALID(index)   (0x1000 + ((index) * 0x40))
#define STARS_TOPIC_HOST_PID(index)        (0x1014 + ((index) * 0x40))
#define STARS_TOPIC_VF_ID(index)           (0x1010 + ((index) * 0x40))
#define STARS_TOPIC_HOST_PID_STD(index)    (0x1018 + ((index) * 0x40))
#define STARS_TOPIC_HOST_PID_USER(index)   (0x101c + ((index) * 0x40))
#define STARS_TOPIC_DEVICE_PID_STD(index)  (0x1020 + ((index) * 0x40))
#define STARS_TOPIC_DEVICE_PID_USER(index) (0x1024 + ((index) * 0x40))
#define STARS_TOPIC_TYPE_PID(index, type)  (0x1018 + ((index) * 0x40) + ((type) * 0x4))
#define STARS_TOPIC_COREGROUP_ID(index)    (0x1028 + ((index) * 0x40))

/* pool-slot config reg */
#define STARS_TOPIC_ACPU_SLOT_POOL_ENABLE_CTRL_NS(pool_id) (0x20000 + ((pool_id) * 0x4))
#define STARS_TOPIC_ACPU_SLOT_POOL_DISABLE_CTRL_NS(pool_id) (0x20040 + ((pool_id) * 0x4))
#define STARS_TOPIC_ACPU_SLOT_POOL_STATUS_NS(pool_id)       (0x20080 + ((pool_id) * 0x4))
#define STARS_TOPIC_CCPU_SLOT_POOL_ENABLE_CTRL_NS(pool_id) (0x20100 + ((pool_id) * 0x4))
#define STARS_TOPIC_CCPU_SLOT_POOL_DISABLE_CTRL_NS(pool_id) (0x20140 + ((pool_id) * 0x4))
#define STARS_TOPIC_CCPU_SLOT_POOL_STATUS_NS(pool_id)       (0x20180 + ((pool_id) * 0x4))

/* submit split task to topic sched port */
#define STARS_TOPIC_CPU_PORT_NUM 8
#define STARS_TOPIC_CPU_PORT_BASE_ADDR_LOW(port_id)  (0x02400 + ((port_id) * 0x4))
#define STARS_TOPIC_CPU_PORT_BASE_ADDR_HIGH(port_id) (0x02460 + ((port_id) * 0x4))
#define STARS_TOPIC_CPU_PORT_DB(port_id)             (0x024C0 + ((port_id) * 0x4))
#define STARS_TOPIC_CPU_PORT_HEAD_CFG(port_id)       (0x02520 + ((port_id) * 0x4))
#define STARS_TOPIC_CPU_PORT_CFG1(port_id)           (0x02580 + ((port_id) * 0x4))
#define STARS_TOPIC_CPU_PORT_CFG2(port_id)           (0x02640 + ((port_id) * 0x4))
#define STARS_TOPIC_CPU_TASK_CTRL0(port_id)          (0x026A0 + ((port_id) * 0x4))

/* host core group pool config reg */
#define TOPIC_SCHED_HOST_POOL_NUM 8
#define STARS_TOPIC_HOST_AICPU_POOL_ENABLE_CTRL0_NS(pool_id)         (0x900 + ((pool_id) * 0x10))
#define STARS_TOPIC_HOST_AICPU_POOL_ENABLE_CTRL1_NS(pool_id)         (0x904 + ((pool_id) * 0x10))
#define STARS_TOPIC_HOST_CTRLCPU_POOL_ENABLE_CTRL_NS(pool_id)        (0x908 + ((pool_id) * 0x10))

#define STARS_TOPIC_HOST_AICPU_POOL_DISABLE_CTRL0_NS(pool_id)        (0xa00 + ((pool_id) * 0x10))
#define STARS_TOPIC_HOST_AICPU_POOL_DISABLE_CTRL1_NS(pool_id)        (0xa04 + ((pool_id) * 0x10))
#define STARS_TOPIC_HOST_CTRLCPU_POOL_DISABLE_CTRL_NS(pool_id)       (0xa08 + ((pool_id) * 0x10))

/* host cpu mailbox config reg */
#define TOPIC_SCHED_HOST_MAILBOX_NUM 80   /* aicpu: 64; ctrlcpu: 16 */
#define STARS_TOPIC_HOST_CPU_WAIT_MAILBOX_ADDR_LOW_NS(vf_id)        (0x00CA0 + ((vf_id) * 0x10))
#define STARS_TOPIC_HOST_CPU_WAIT_MAILBOX_ADDR_HIGH_NS(vf_id)       (0x00CA4 + ((vf_id) * 0x10))

#define STARS_TOPIC_MAX_VF_NUM       16

#define TOPIC_INT_NS_REG_OFFSET 0x8000
#define TOPIC_INT_NS_REG_SIZE (32 * 1024)

#define STARS_INT_CPU_STATUS_RAW_NS     0x04000
#define STARS_INT_CPU_STATUS_FORCE_NS   0x04004
#define STARS_INT_CPU_STATUS_CLR_NS     0x04008
#define STARS_INT_CPU_STATUS_STS_NS     0x0400C
#define STARS_INT_CPU_STATUS_MASK_NS    0x04010
#define STARS_TOPIC_HOST_CPU_INT_CFG_NS(vf_id) (((vf_id) * 0x10000) + 0x400A60)

#define STARS_INT_HTIC_TOPIC_HCPU_VF_NUM_OFFSET  11
#define STARS_INT_HTIC_TOPIC_HACPU_INT_ID(vf_id) (((vf_id) * 0x4) + 0xA30)
#define STARS_INT_HTIC_TOPIC_HCCPU_INT_ID(vf_id) (((vf_id) * 0x4) + 0xA70)

#define STARS_HTIC_INT_VF_NS_OFFSET    11
#define STARS_HTIC_INT_PF_NS_OFFSET    19

#define STARS_INT_HTIC_INTR_ID_NS      0x0B00

#endif
