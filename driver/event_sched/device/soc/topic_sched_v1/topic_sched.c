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

#ifndef EVENT_SCHED_UT

#include <linux/types.h>
#include <stdbool.h>
#include <asm/io.h>
#include <linux/delay.h>

#include "topic_sched.h"
#include "topic_sched_common.h"

#include "esched.h"

STATIC void esched_drv_reg_wr(const void __iomem *io_base, u32 offset, u32 val)
{
    sched_debug("Write. (offset=%x; data=%x)\n", offset, val);
    writel(val, (volatile void __iomem *)(io_base + offset));
}

STATIC void esched_drv_reg_rd(const void __iomem *io_base, u32 offset, u32 *val)
{
    *val = readl(io_base + offset);

    if (*val != 0) {
        sched_debug("Read. (offset=%x; data=%x)\n", offset, *val);
    }
}

void topic_sched_get_wait_mb_free_status(const void __iomem *io_base, u32 *val)
{
    esched_drv_reg_rd(io_base, (u32)STARS_TOPIC_DEVICE_CPU_FREE_BITMAP, val);
    *val &= 0xFF;
}

void topic_sched_reset_pool_dev_cpu(const void __iomem *io_base, u32 pool_id)
{
    esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_DEVICE_POOL_DISABLE_CTRL_NS(pool_id), 0xFFFFFFFFU);
    esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_ACPU_SLOT_POOL_DISABLE_CTRL_NS(pool_id), 0xFFFFFFFFU);
    esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_CCPU_SLOT_POOL_DISABLE_CTRL_NS(pool_id), 0xFFFFFFFFU);
}

void topic_sched_reset_pool_host_cpu(const void __iomem *io_base, u32 pool_id)
{
    esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_HOST_AICPU_POOL_DISABLE_CTRL0_NS(pool_id), 0xFFFFFFFFU);
    esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_HOST_AICPU_POOL_DISABLE_CTRL1_NS(pool_id), 0xFFFFFFFFU);
    esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_HOST_CTRLCPU_POOL_DISABLE_CTRL_NS(pool_id), 0xFFFFFFFFU);
}

void topic_sched_pool_add_mb(const void __iomem *io_base, u32 pool_id, u32 mask)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_DEVICE_POOL_ENABLE_CTRL_NS(pool_id), mask);
}

void topic_sched_pool_del_mb(const void __iomem *io_base, u32 pool_id, u32 mask)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_DEVICE_POOL_DISABLE_CTRL_NS(pool_id), mask);
}

void topic_sched_pool_add_aicpu_slot(const void __iomem *io_base, u32 pool_id, u32 mask)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_ACPU_SLOT_POOL_ENABLE_CTRL_NS(pool_id), mask);
}

void topic_sched_pool_del_aicpu_slot(const void __iomem *io_base, u32 pool_id, u32 mask)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_ACPU_SLOT_POOL_DISABLE_CTRL_NS(pool_id), mask);
}

void topic_sched_pool_add_ccpu_slot(const void __iomem *io_base, u32 pool_id, u32 mask)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_CCPU_SLOT_POOL_ENABLE_CTRL_NS(pool_id), mask);
}

void topic_sched_pool_del_ccpu_slot(const void __iomem *io_base, u32 pool_id, u32 mask)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_CCPU_SLOT_POOL_DISABLE_CTRL_NS(pool_id), mask);
}

void topic_sched_add_host_pool(const void __iomem *io_base, u32 cpu_type, u32 pool_id, u64 mask)
{
    u32 mask_high_val = (u32)(mask >> 32);

    if (cpu_type == ACPU_HOST) {
        esched_drv_reg_wr(io_base, STARS_TOPIC_HOST_AICPU_POOL_ENABLE_CTRL0_NS(pool_id), (u32)mask);
        esched_drv_reg_wr(io_base, STARS_TOPIC_HOST_AICPU_POOL_ENABLE_CTRL1_NS(pool_id), mask_high_val);
    }

    if (cpu_type == CCPU_HOST) {
        esched_drv_reg_wr(io_base, STARS_TOPIC_HOST_CTRLCPU_POOL_ENABLE_CTRL_NS(pool_id), (u32)mask);
    }
}

void topic_sched_del_host_pool(const void __iomem *io_base, u32 cpu_type, u32 pool_id, u64 mask)
{
    u32 mask_high_val = (u32)(mask >> 32);

    if (cpu_type == ACPU_HOST) {
        esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_HOST_AICPU_POOL_DISABLE_CTRL0_NS(pool_id), (u32)mask);
        esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_HOST_AICPU_POOL_DISABLE_CTRL1_NS(pool_id), mask_high_val);
    }

    if (cpu_type == CCPU_HOST) {
        esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_HOST_CTRLCPU_POOL_DISABLE_CTRL_NS(pool_id), (u32)mask);
    }
}

void topic_sched_set_cpu_port_head(const void __iomem *io_base, u32 port_id, u32 head)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_CPU_PORT_HEAD_CFG(port_id), head);
}

void topic_sched_get_cpu_port_head(const void __iomem *io_base, u32 port_id, u32 *head)
{
    esched_drv_reg_rd(io_base, STARS_TOPIC_CPU_PORT_HEAD_CFG(port_id), head);
}

void topic_sched_set_cpu_port_tail(const void __iomem *io_base, u32 port_id, u32 tail)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_CPU_PORT_DB(port_id), tail);
}

void topic_sched_init_cpu_port(const void __iomem *io_base, u32 port_id, u64 sq_base, u32 depth)
{
    /* set depth then enalbe */
    esched_drv_reg_wr(io_base, STARS_TOPIC_CPU_PORT_BASE_ADDR_LOW(port_id), (u32)sq_base);
    esched_drv_reg_wr(io_base, STARS_TOPIC_CPU_PORT_BASE_ADDR_HIGH(port_id), (u32)(sq_base >> 32));
    esched_drv_reg_wr(io_base, STARS_TOPIC_CPU_PORT_CFG2(port_id), depth);
}

void topic_sched_enable_cpu_port(const void __iomem *io_base, u32 port_id)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_CPU_PORT_CFG1(port_id), 1);
}

void topic_sched_disable_cpu_port(const void __iomem *io_base, u32 port_id)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_CPU_PORT_CFG1(port_id), 0);
}

#define CLR_WAIT_CNT 100
#define CPU_PORT_CLR_MASK (0x1)
#define CPU_PORT_PAUSE_MASK (0x1 << 1)
#define CPU_PORT_BUSY_MASK (0x1 << 2)
#define CPU_PORT_GET_MB_VALID_MASK 0x1U
void topic_sched_clr_cpu_port_task(const void __iomem *io_base, u32 port_id, u32 mb_id, u32 tail)
{
    u32 status, head;
    int cnt;

    esched_drv_reg_wr(io_base, STARS_TOPIC_CPU_TASK_CTRL0(port_id), CPU_PORT_PAUSE_MASK); /* pause */

    topic_sched_get_cpu_port_head(io_base, port_id, &head);
    topic_sched_set_cpu_port_tail(io_base, port_id, 0);
    topic_sched_set_cpu_port_head(io_base, port_id, 0);

    if (head != tail) {
        sched_warn("Cpu port still has events. (port_id=%u; head=%u; tail=%u)\n", port_id, head, tail);
    }

    /* wait pause success */
    cnt = CLR_WAIT_CNT;
    while (cnt-- > 0) {
        esched_drv_reg_rd(io_base, STARS_TOPIC_CPU_TASK_CTRL0(port_id), &status);
        if ((status & CPU_PORT_BUSY_MASK) == 0) {
            break;
        }
    }

    if (cnt < 0) {
        sched_err("Wait cpu port pause failed. (port_id=%u; status=%u; cnt=%d)\n", port_id, status, cnt);
        esched_drv_reg_wr(io_base, STARS_TOPIC_CPU_TASK_CTRL0(port_id), 0);
        return;
    }

    /* pause & clear */
    esched_drv_reg_wr(io_base, STARS_TOPIC_CPU_TASK_CTRL0(port_id), CPU_PORT_PAUSE_MASK || CPU_PORT_CLR_MASK);

    /* clear get mailbox */
    cnt = CLR_WAIT_CNT;
    while (cnt-- > 0) {
        esched_drv_reg_rd(io_base, STARS_TOPIC_DEVICE_CPU_GET_TOPIC_NS(mb_id), &status);
        if (status == 0) {
            break;
        }

        if ((status & CPU_PORT_GET_MB_VALID_MASK) == 1) {
            topic_sched_cpu_get_status_report(io_base, mb_id, TOPIC_FINISH_STATUS_NORMAL);
        } else {
            // do nothing
        }
    }

    if (cnt < 0) {
        sched_warn("Clear get mailbox warn. (port_id=%u; status=%u; cnt=%d)\n", port_id, status, cnt);
    }

    esched_drv_reg_wr(io_base, STARS_TOPIC_CPU_TASK_CTRL0(port_id), 0); /* stop pause */
}

STATIC int topic_sched_get_host_pid_table_id(const void __iomem *io_base, u32 vfid, u32 host_pid)
{
    u32 host_pid_reg, vfid_reg, valid_reg;
    int i;

    /* check is host pid has been config */
    for (i = 0; i < TOPIC_SCHED_PID_TABLE_SIZE; i++) {
        esched_drv_reg_rd(io_base, (u32)STARS_TOPIC_PID_MAP_VALID(i), &valid_reg);
        esched_drv_reg_rd(io_base, (u32)STARS_TOPIC_HOST_PID(i), &host_pid_reg);
        esched_drv_reg_rd(io_base, (u32)STARS_TOPIC_VF_ID(i), &vfid_reg);
        if (((valid_reg & 0x1) == 1) && (vfid_reg == vfid) && (host_pid_reg == host_pid)) {
            break;
        }
    }

    return i;
}

int topic_sched_config_pid(const void __iomem *io_base, u32 identity, u32 host_pid, int pid_type, int pid)
{
    u32 valid_reg;
    int i;
    u32 vfid = identity & 0xFFFF; /* low 16 bit is vfid */
    u32 poolid = (identity >> 16) & 0xFFFF; /* hign 16 bit is poolid */

    /* topic vfid start from 0 */
    if (vfid >= 1) {
        vfid--;
    }

    i = topic_sched_get_host_pid_table_id(io_base, vfid, host_pid);
    if (i >= TOPIC_SCHED_PID_TABLE_SIZE) {
        for (i = 0; i < TOPIC_SCHED_PID_TABLE_SIZE; i++) {
            esched_drv_reg_rd(io_base, (u32)STARS_TOPIC_PID_MAP_VALID(i), &valid_reg);
            if ((valid_reg & 0x1) == 0) {
                break;
            }
        }

        if (i >= TOPIC_SCHED_PID_TABLE_SIZE) {
            return -EFAULT;
        }

        esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_HOST_PID(i), host_pid);
        esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_VF_ID(i), vfid);
        esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_COREGROUP_ID(i), poolid);
        esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_PID_MAP_VALID(i), 1);

        sched_info("Add pid mapping table entry. (vfid=%u; host_pid=%u; table_id=%d)\n", vfid, host_pid, i);
    }

    if (pid_type == HOST_STD_PROC) {
        esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_HOST_PID_STD(i), (u32)pid);
    } else if (pid_type == HOST_USER_PROC) {
        esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_HOST_PID_USER(i), (u32)pid);
    } else if (pid_type == DEVICE_STD_PROC) {
        esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_DEVICE_PID_STD(i), (u32)pid);
    } else {
        esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_DEVICE_PID_USER(i), (u32)pid);
    }

    sched_info("Update pid mapping table entry. (vfid=%u; host_pid=%u; table_id=%d; pid_type=%d; pid=%d)\n",
        vfid, host_pid, i, pid_type, pid);
    return 0;
}

void topic_sched_del_host_pid(const void __iomem *io_base, u32 identity, u32 host_pid, u32 side, u32 pid)
{
    u32 vfid_reg;
    int i, j;
    u32 vfid = identity & 0xFFFF; /* low 16 bit is vfid */
    int pid_type = PID_TYPE_MAX;

    /* topic vfid start from 0 */
    if (vfid >= 1) {
        vfid--;
    }

    i = topic_sched_get_host_pid_table_id(io_base, vfid, host_pid);
    if (i >= TOPIC_SCHED_PID_TABLE_SIZE) {
        sched_info("None such host_pid in topic table, deletion not needed. (vfid=%u; host_pid=%u)\n", vfid, host_pid);
        return;
    }

    if (side == HOST_SIDE_SET_PID) {
        esched_drv_reg_rd(io_base, (u32)STARS_TOPIC_HOST_PID_STD(i), &vfid_reg);
        if (vfid_reg == pid) {
            esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_HOST_PID_STD(i), 0);
            pid_type = HOST_STD_PROC;
            goto check_app_exit;
        }

        esched_drv_reg_rd(io_base, (u32)STARS_TOPIC_HOST_PID_USER(i), &vfid_reg);
        if (vfid_reg == pid) {
            esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_HOST_PID_USER(i), 0);
            pid_type = HOST_USER_PROC;
            goto check_app_exit;
        }
    } else {
        /* when device cp exit, set pid map to invalid */
        esched_drv_reg_rd(io_base, (u32)STARS_TOPIC_DEVICE_PID_STD(i), &vfid_reg);
        if (vfid_reg == pid) {
            for (j = 0; j < PID_TYPE_MAX; j++) {
                esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_TYPE_PID(i, j), 0);
            }
            /* The stars compares the host PID and VFID to confirm the table entry.
            * If the host PID and VFID are cleared before the invaild table entry,
            * the stars may find multiple entries whose host PID and VFID are 0. */
            esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_PID_MAP_VALID(i), 0);
            sched_info("Del pid mapping table entry. (vfid=%u; host_pid=%u; table_id=%u; pid=%u)\n",
                vfid, host_pid, i, pid);
            return;
        }

        esched_drv_reg_rd(io_base, (u32)STARS_TOPIC_DEVICE_PID_USER(i), &vfid_reg);
        if (vfid_reg == pid) {
            esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_DEVICE_PID_USER(i), 0);
            pid_type = DEVICE_USER_PROC;
            goto check_app_exit;
        }
    }

check_app_exit:
    for (j = 0; j < PID_TYPE_MAX; j++) {
        esched_drv_reg_rd(io_base, (u32)STARS_TOPIC_TYPE_PID(i, j), &vfid_reg);
        if (vfid_reg != 0) {
            break;
        }
    }

    if (j == PID_TYPE_MAX) {
        esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_PID_MAP_VALID(i), 0);
        sched_info("Del pid mapping table entry. (vfid=%u; host_pid=%u; table_id=%u; pid_type=%d; pid=%u)\n",
            vfid, host_pid, i, pid_type, pid);
    } else {
        sched_info("Del a pid from mapping table entry. (vfid=%u; host_pid=%u; table_id=%d; pid_type=%d; pid=%u)\n",
            vfid, host_pid, i, pid_type, pid);
    }

    return;
}

bool topic_sched_cpu_is_mb_valid(const void __iomem *io_base, u32 mb_id)
{
    u32 val;

    esched_drv_reg_rd(io_base, STARS_TOPIC_DEVICE_CPU_WAIT_TOPIC_NS(mb_id), &val);

    return ((val & 0x1) == 1);
}

void topic_sched_cpu_status_report(const void __iomem *io_base, u32 mb_id, u32 status)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_DEVICE_CPU_STATUS_REPORT_NS(mb_id), status);
}

void topic_sched_cpu_errcode_report(const void __iomem *io_base, u32 mb_id, u32 error_code)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_DEVICE_CPU_ERRCODE_REPORT_NS(mb_id), error_code);
}

void topic_sched_cpu_get_status_report(const void __iomem *io_base, u32 mb_id, u32 status)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_DEVICE_CPU_GET_STATUS_REPORT_NS(mb_id),
        status);
}

bool topic_sched_aicpu_is_get_mb_valid(const void __iomem *io_base, u32 mb_id)
{
    u32 val;

    esched_drv_reg_rd(io_base, STARS_TOPIC_DEVICE_CPU_GET_TOPIC_NS(mb_id), &val);

    return ((val & 0x1) == 1);
}

void topic_sched_set_user_kernel_type(const void __iomem *io_base, u32 user_kernel_type)
{
    esched_drv_reg_wr(io_base, (u32)STARS_TOPIC_KERNEL_TYPE_USER, user_kernel_type);
}

void topic_sched_uninit_cpu_port(const void __iomem *io_base, u32 port_id)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_CPU_PORT_CFG1(port_id), 0);
}

void topic_sched_init_cpu_mailbox(const void __iomem *io_base, u32 mb_id, u64 mb_addr)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_DEVICE_WAIT_MAILBOX_ADDR_LOW_NS(mb_id), (u32)mb_addr);
    esched_drv_reg_wr(io_base, STARS_TOPIC_DEVICE_WAIT_MAILBOX_ADDR_HIGH_NS(mb_id), (u32)(mb_addr >> 32));
}

void topic_sched_init_aicpu_get_mailbox(const void __iomem *io_base, u32 mb_id, u64 mb_addr)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_DEVICE_ACPU_GET_MAILBOX_ADDR_LOW_NS(mb_id), (u32)mb_addr);
    esched_drv_reg_wr(io_base, STARS_TOPIC_DEVICE_ACPU_GET_MAILBOX_ADDR_HIGH_NS(mb_id), (u32)(mb_addr >> 32));
}

/* Only need to configure BASE_ADDR once, and the different mailboxe are differentiated by SHIFT.
   ESL B606 test, do not modify the original process

   The address of the first mailbox_id mailbox in the same VF is:
       host_cpu_wait_mailbox_base_addr + (mailbox_id << host_cpu_wait_mailbox_shift)

   host_cpu_wait_mailbox_shift: bit20 ~ bit25
   0x00600000: every mailbox length is 64B
 */
#define HOST_CPU_WAIT_MAILBOX_SHIFT 0x00600000

void topic_sched_init_host_cpu_mailbox(const void __iomem *io_base, u64 mb_addr, u32 vf_id)
{
    u32 addr_high_val = ((u32)(mb_addr >> 32)) | HOST_CPU_WAIT_MAILBOX_SHIFT;

    esched_drv_reg_wr(io_base, STARS_TOPIC_HOST_CPU_WAIT_MAILBOX_ADDR_LOW_NS(vf_id), (u32)mb_addr);
    esched_drv_reg_wr(io_base, STARS_TOPIC_HOST_CPU_WAIT_MAILBOX_ADDR_HIGH_NS(vf_id), addr_high_val);
}

/* No longer distinguish between aicpu, ctrlcpu and datacpu */
void topic_sched_mb_intr_clr(const void __iomem *io_base, u32 mb_id)
{
    esched_drv_reg_wr(io_base, STARS_INT_CPU_STATUS_CLR_NS, (0x1U << mb_id));
}

void topic_sched_conf_host_intr(const void __iomem *io_base, u32 int_value, u32 vf_id)
{
    esched_drv_reg_wr(io_base, STARS_INT_HTIC_TOPIC_HACPU_INT_ID(vf_id), int_value);
    esched_drv_reg_wr(io_base, STARS_INT_HTIC_TOPIC_HCCPU_INT_ID(vf_id), int_value);
}

void topic_sched_cpu_intr_enable(const void __iomem *io_base, u32 mb_id)
{
    esched_drv_reg_wr(io_base, STARS_TOPIC_DEVICE_CPU_INT_EN_NS(mb_id), 0x1);
}

#else
int tmp_esched_drv_reg_rd(void)
{
    return 0;
}
#endif
