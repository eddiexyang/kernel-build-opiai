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
#include <linux/irq.h>
#include <linux/wait.h>

#include "hisi_rproc_adapt.h"

#define MBIX_CLEAR_REG_BASE   0xC015A000
#define MBIX_CLEAR_REG_SIZE   4
#define MBIX_CLEAR_REG_WIDTH  32
#define MBIX_CLEAR_REG_STEP   4

#if defined(UT_TEST) || defined(ST_TEST)
int for_ut_test(void)
{
    return 0;
}
#else
struct hisi_rproc_info g_rproc_table[MAX_IPCDEV_NUM][HISI_RPROC_MAX] = {
    {
        /* lp normal rx channels */
        {
            .rproc_id = HISI_RPROC_RX_LP_ACPU0,
            .mbox_rp = IPC_LP_MBX0_RPID0_RX_ACPU0,
        },
        {
            .rproc_id = HISI_RPROC_RX_LP_ACPU1,
            .mbox_rp = IPC_LP_MBX1_RPID1_RX_ACPU1,
        },
        {
            .rproc_id = HISI_RPROC_RX_LP_ACPU2,
            .mbox_rp = IPC_LP_MBX2_RPID2_RX_ACPU2,
        },
        {
            .rproc_id = HISI_RPROC_RX_LP_ACPU3,
            .mbox_rp = IPC_LP_MBX3_RPID3_RX_ACPU3,
        },

        /* lp normal tx channels */
        {
            .rproc_id = HISI_RPROC_TX_LP_ACPU0,
            .mbox_rp = IPC_LP_MBX5_RPID4_TX_ACPU0,
        },
        {
            .rproc_id = HISI_RPROC_TX_LP_ACPU1,
            .mbox_rp = IPC_LP_MBX6_RPID4_TX_ACPU1,
        },
        {
            .rproc_id = HISI_RPROC_TX_LP_ACPU2,
            .mbox_rp = IPC_LP_MBX7_RPID4_TX_ACPU2,
        },
        {
            .rproc_id = HISI_RPROC_TX_LP_ACPU3,
            .mbox_rp = IPC_LP_MBX8_RPID4_TX_ACPU3,
        },

        /* ts normal rx channels */
        {
            .rproc_id = HISI_RPROC_RX_TS_ACPU0,
            .mbox_rp = IPC_TS_MBX8_RPID8_RX_ACPU0,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_ACPU1,
            .mbox_rp = IPC_TS_MBX9_RPID9_RX_ACPU1,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_ACPU2,
            .mbox_rp = IPC_TS_MBX10_RPID10_RX_ACPU2,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_ACPU3,
            .mbox_rp = IPC_TS_MBX11_RPID11_RX_ACPU3,
        },

        /* ts normal tx channels */
        {
            .rproc_id = HISI_RPROC_TX_TS_ACPU0,
            .mbox_rp = IPC_TS_MBX0_RPID0_TX_ACPU0,
        },
        {
            .rproc_id = HISI_RPROC_TX_TS_ACPU1,
            .mbox_rp = IPC_TS_MBX1_RPID1_TX_ACPU1,
        },
        {
            .rproc_id = HISI_RPROC_TX_TS_ACPU2,
            .mbox_rp = IPC_TS_MBX2_RPID2_TX_ACPU2,
        },
        {
            .rproc_id = HISI_RPROC_TX_TS_ACPU3,
            .mbox_rp = IPC_TS_MBX3_RPID3_TX_ACPU3,
        },
    },
};

void mbox_clear_mbix_eoi_irq(u32 irq)
{
    void __iomem *mbox_mbix_base = NULL;
    struct irq_data *irq_data = NULL;
    u32 mask, offset;

    irq_data = irq_get_irq_data(irq);
    mask = 1 << (irq_data->hwirq % MBIX_CLEAR_REG_WIDTH);
    offset = (irq_data->hwirq / MBIX_CLEAR_REG_WIDTH) * MBIX_CLEAR_REG_STEP;

    mbox_mbix_base = ioremap(MBIX_CLEAR_REG_BASE + offset, MBIX_CLEAR_REG_SIZE);
    if (mbox_mbix_base == NULL) {
        return;
    }
    writel_relaxed(mask, mbox_mbix_base);

    iounmap(mbox_mbix_base);
    mbox_mbix_base = NULL;
}
#endif
