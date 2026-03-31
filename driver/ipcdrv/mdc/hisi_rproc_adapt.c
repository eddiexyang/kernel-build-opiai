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

#include <linux/wait.h>
#include "hisi_rproc_adapt.h"

struct hisi_rproc_info* g_rproc_table[MAX_IPCDEV_NUM] = {NULL};
struct hisi_rproc_cfg g_rproc_cfg[HISI_RPROC_MAX] = {
    /* quick rx channels */
    {
        .rproc_id = HISI_RPROC_SI_Q_RX_RPID0,
        .mbox_rp = IPC_SI_MBX0_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_SI_Q_RX_RPID1,
        .mbox_rp = IPC_SI_MBX1_RPID1_Q_RX_ACPU1,
    },
    {
        .rproc_id = HISI_RPROC_SI_Q_RX_RPID2,
        .mbox_rp = IPC_SI_MBX2_RPID2_Q_RX_ACPU2,
    },

    {
        .rproc_id = HISI_RPROC_SI_Q_RX_RPID3,
        .mbox_rp = IPC_SI_MBX3_RPID3_Q_RX_ACPU3,
    },
    /* quick tx channels */
    {
        .rproc_id = HISI_RPROC_SI_Q_TX_RPID14,
        .mbox_rp = IPC_SI_MBX14_RPID14_Q_TX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_SI_Q_TX_RPID15,
        .mbox_rp = IPC_SI_MBX15_RPID15_Q_TX_ACPU1,
    },
    {
        .rproc_id = HISI_RPROC_SI_Q_TX_RPID16,
        .mbox_rp = IPC_SI_MBX16_RPID16_Q_TX_ACPU2,
    },
    {
        .rproc_id = HISI_RPROC_SI_Q_TX_RPID17,
        .mbox_rp = IPC_SI_MBX17_RPID17_Q_TX_ACPU3,
    },
    /* quick rx channels */
    {
        .rproc_id = HISI_RPROC_LP_Q_RX_RPID0,
        .mbox_rp = IPC_LP_MBX0_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_LP_Q_RX_RPID1,
        .mbox_rp = IPC_LP_MBX1_RPID1_Q_RX_ACPU1,
    },
    {
        .rproc_id = HISI_RPROC_LP_Q_RX_RPID2,
        .mbox_rp = IPC_LP_MBX2_RPID2_Q_RX_ACPU2,
    },
    {
        .rproc_id = HISI_RPROC_LP_Q_RX_RPID3,
        .mbox_rp = IPC_LP_MBX3_RPID3_Q_RX_ACPU3,
    },
    /* quick tx channels */
    {
        .rproc_id = HISI_RPROC_LP_Q_TX_RPID4_ACPU0,
        .mbox_rp = IPC_LP_MBX8_RPID4_Q_TX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_LP_Q_TX_RPID4_ACPU1,
        .mbox_rp = IPC_LP_MBX9_RPID4_Q_TX_ACPU1,
    },
    {
        .rproc_id = HISI_RPROC_LP_Q_TX_RPID4_ACPU2,
        .mbox_rp = IPC_LP_MBX10_RPID4_Q_TX_ACPU2,
    },
    {
        .rproc_id = HISI_RPROC_LP_Q_TX_RPID4_ACPU3,
        .mbox_rp = IPC_LP_MBX11_RPID4_Q_TX_ACPU3,
    },
    /* normal rx channels */
    {
        .rproc_id = HISI_RPROC_TSC_RX_RPID4,
        .mbox_rp = IPC_TSC_MBX5_RPID4_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_TSC_RX_RPID5,
        .mbox_rp = IPC_TSC_MBX6_RPID5_RX_ACPU1,
    },
    {
        .rproc_id = HISI_RPROC_TSC_RX_RPID6,
        .mbox_rp = IPC_TSC_MBX7_RPID6_RX_ACPU2,
    },
    {
        .rproc_id = HISI_RPROC_TSC_RX_RPID7,
        .mbox_rp = IPC_TSC_MBX8_RPID7_RX_ACPU3,
    },
    /* normal tx channels */
    {
        .rproc_id = HISI_RPROC_TSC_TX_RPID0,
        .mbox_rp = IPC_TSC_MBX1_RPID0_TX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_TSC_TX_RPID1,
        .mbox_rp = IPC_TSC_MBX2_RPID1_TX_ACPU1,
    },
    {
        .rproc_id = HISI_RPROC_TSC_TX_RPID2,
        .mbox_rp = IPC_TSC_MBX3_RPID2_TX_ACPU2,
    },
    {
        .rproc_id = HISI_RPROC_TSC_TX_RPID3,
        .mbox_rp = IPC_TSC_MBX4_RPID3_TX_ACPU3,
    },
    /* normal rx channels */
    {
        .rproc_id = HISI_RPROC_TSV_RX_RPID4,
        .mbox_rp = IPC_TSV_MBX5_RPID4_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_TSV_RX_RPID5,
        .mbox_rp = IPC_TSV_MBX6_RPID5_RX_ACPU1,
    },
    {
        .rproc_id = HISI_RPROC_TSV_RX_RPID6,
        .mbox_rp = IPC_TSV_MBX7_RPID6_RX_ACPU2,
    },
    {
        .rproc_id = HISI_RPROC_TSV_RX_RPID7,
        .mbox_rp = IPC_TSV_MBX8_RPID7_RX_ACPU3,
    },

    /* normal tx channels */
    {
        .rproc_id = HISI_RPROC_TSV_TX_RPID0,
        .mbox_rp = IPC_TSV_MBX1_RPID0_TX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_TSV_TX_RPID1,
        .mbox_rp = IPC_TSV_MBX2_RPID1_TX_ACPU1,
    },
    {
        .rproc_id = HISI_RPROC_TSV_TX_RPID2,
        .mbox_rp = IPC_TSV_MBX3_RPID2_TX_ACPU2,
    },
    {
        .rproc_id = HISI_RPROC_TSV_TX_RPID3,
        .mbox_rp = IPC_TSV_MBX4_RPID3_TX_ACPU3,
    },
/* isp0_ipc0 */
    {
        .rproc_id = HISI_RPROC_ISP0_IPC0_MBX0_RX_RPID0,
        .mbox_rp = IPC_ISP0_IPC0_MBX0_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP0_IPC0_MBX1_RX_RPID0,
        .mbox_rp = IPC_ISP0_IPC0_MBX1_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP0_IPC0_MBX2_RX_RPID0,
        .mbox_rp = IPC_ISP0_IPC0_MBX2_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP0_IPC0_MBX6_TX_RPID1,
        .mbox_rp = IPC_ISP0_IPC0_MBX6_RPID1_Q_TX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP0_IPC0_MBX7_TX_RPID2,
        .mbox_rp = IPC_ISP0_IPC0_MBX7_RPID2_Q_TX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP0_IPC0_MBX8_TX_RPID3,
        .mbox_rp = IPC_ISP0_IPC0_MBX8_RPID3_Q_TX_ACPU0,
    },
/* isp0_ipc1 */
    {
        .rproc_id = HISI_RPROC_ISP0_IPC1_MBX0_RX_RPID0,
        .mbox_rp = IPC_ISP0_IPC1_MBX0_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP0_IPC1_MBX1_RX_RPID0,
        .mbox_rp = IPC_ISP0_IPC1_MBX1_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP0_IPC1_MBX2_RX_RPID0,
        .mbox_rp = IPC_ISP0_IPC1_MBX2_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP0_IPC1_MBX6_TX_RPID1,
        .mbox_rp = IPC_ISP0_IPC1_MBX6_RPID1_Q_TX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP0_IPC1_MBX7_TX_RPID2,
        .mbox_rp = IPC_ISP0_IPC1_MBX7_RPID2_Q_TX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP0_IPC1_MBX8_TX_RPID3,
        .mbox_rp = IPC_ISP0_IPC1_MBX8_RPID3_Q_TX_ACPU0,
    },
/* isp1_ipc0 */
    {
        .rproc_id = HISI_RPROC_ISP1_IPC0_MBX0_RX_RPID0,
        .mbox_rp = IPC_ISP1_IPC0_MBX0_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP1_IPC0_MBX1_RX_RPID0,
        .mbox_rp = IPC_ISP1_IPC0_MBX1_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP1_IPC0_MBX2_RX_RPID0,
        .mbox_rp = IPC_ISP1_IPC0_MBX2_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP1_IPC0_MBX6_TX_RPID1,
        .mbox_rp = IPC_ISP1_IPC0_MBX6_RPID1_Q_TX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP1_IPC0_MBX7_TX_RPID2,
        .mbox_rp = IPC_ISP1_IPC0_MBX7_RPID2_Q_TX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP1_IPC0_MBX8_TX_RPID3,
        .mbox_rp = IPC_ISP1_IPC0_MBX8_RPID3_Q_TX_ACPU0,
    },
/* isp1_ipc1 */
    {
        .rproc_id = HISI_RPROC_ISP1_IPC1_MBX0_RX_RPID0,
        .mbox_rp = IPC_ISP1_IPC1_MBX0_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP1_IPC1_MBX1_RX_RPID0,
        .mbox_rp = IPC_ISP1_IPC1_MBX1_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP1_IPC1_MBX2_RX_RPID0,
        .mbox_rp = IPC_ISP1_IPC1_MBX2_RPID0_Q_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP1_IPC1_MBX6_TX_RPID6,
        .mbox_rp = IPC_ISP1_IPC1_MBX6_RPID1_Q_TX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP1_IPC1_MBX7_TX_RPID7,
        .mbox_rp = IPC_ISP1_IPC1_MBX7_RPID2_Q_TX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_ISP1_IPC1_MBX8_TX_RPID8,
        .mbox_rp = IPC_ISP1_IPC1_MBX8_RPID3_Q_TX_ACPU0,
    },
/* TaiShan */
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX0_ACPU1_RX_ACPU0,
        .mbox_rp = IPC_TAISHAN_MBX0_RPID1_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX0_ACPU2_RX_ACPU0,
        .mbox_rp = IPC_TAISHAN_MBX0_RPID2_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX0_ACPU3_RX_ACPU0,
        .mbox_rp = IPC_TAISHAN_MBX0_RPID3_RX_ACPU0,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX1_ACPU0_TX_ACPU1,
        .mbox_rp = IPC_TAISHAN_MBX1_ACPU0_TX_RPID1,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX2_ACPU0_TX_ACPU2,
        .mbox_rp = IPC_TAISHAN_MBX2_ACPU0_TX_RPID2,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX3_ACPU0_TX_ACPU3,
        .mbox_rp = IPC_TAISHAN_MBX3_ACPU0_TX_RPID3,
    },
    /* ACPU1 */
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX1_ACPU0_RX_ACPU1,
        .mbox_rp = IPC_TAISHAN_MBX1_RPID0_RX_ACPU1,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX1_ACPU2_RX_ACPU1,
        .mbox_rp = IPC_TAISHAN_MBX1_RPID2_RX_ACPU1,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX1_ACPU3_RX_ACPU1,
        .mbox_rp = IPC_TAISHAN_MBX1_RPID3_RX_ACPU1,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX0_ACPU1_TX_ACPU0,
        .mbox_rp = IPC_TAISHAN_MBX0_ACPU1_TX_RPID0,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX2_ACPU1_TX_ACPU2,
        .mbox_rp = IPC_TAISHAN_MBX2_ACPU1_TX_RPID2,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX3_ACPU1_TX_ACPU3,
        .mbox_rp = IPC_TAISHAN_MBX3_ACPU1_TX_RPID3,
    },
    /* ACPU2 */
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX2_ACPU0_RX_ACPU2,
        .mbox_rp = IPC_TAISHAN_MBX2_RPID0_RX_ACPU2,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX2_ACPU1_RX_ACPU2,
        .mbox_rp = IPC_TAISHAN_MBX2_RPID1_RX_ACPU2,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX2_ACPU3_RX_ACPU2,
        .mbox_rp = IPC_TAISHAN_MBX2_RPID3_RX_ACPU2,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX0_ACPU2_TX_ACPU0,
        .mbox_rp = IPC_TAISHAN_MBX0_ACPU2_TX_RPID0,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX1_ACPU2_TX_ACPU1,
        .mbox_rp = IPC_TAISHAN_MBX1_ACPU2_TX_RPID1,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX3_ACPU2_TX_ACPU3,
        .mbox_rp = IPC_TAISHAN_MBX3_ACPU2_TX_RPID3,
    },
    /* ACPU3 */
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX3_ACPU0_RX_ACPU3,
        .mbox_rp = IPC_TAISHAN_MBX3_RPID0_RX_ACPU3,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX3_ACPU1_RX_ACPU3,
        .mbox_rp = IPC_TAISHAN_MBX3_RPID1_RX_ACPU3,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX3_ACPU2_RX_ACPU3,
        .mbox_rp = IPC_TAISHAN_MBX3_RPID2_RX_ACPU3,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX0_ACPU3_TX_ACPU0,
        .mbox_rp = IPC_TAISHAN_MBX0_ACPU3_TX_RPID0,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX1_ACPU3_TX_ACPU1,
        .mbox_rp = IPC_TAISHAN_MBX1_ACPU3_TX_RPID1,
    },
    {
        .rproc_id = HISI_RPROC_TAISHAN_MBX2_ACPU3_TX_ACPU2,
        .mbox_rp = IPC_TAISHAN_MBX2_ACPU3_TX_RPID2,
    },
};

#if defined(UT_TEST) || defined(ST_TEST)
int for_ut_test(void)
{
    return 0;
}
#endif