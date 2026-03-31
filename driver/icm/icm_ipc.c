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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/jiffies.h>
#include <linux/securec.h>
#include <linux/sched.h>
#include <linux/delay.h>

#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif
#include "icm_core.h"
#include "icm_ipc.h"

struct mutex peer_mutex[MAX_CHIP_NUM][IPC_OBJ_MAX];
struct icmdrv_ipc_res icm_ipc_resource[MAX_CHIP_NUM][HISI_RPROC_MAX] = {
    {
#if defined(CFG_SOC_PLATFORM_CLOUD)
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_IMU_MBX0, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_IMU_MBX1, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TS, HISI_RPROC_RX_TS_MBX2, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TS, HISI_RPROC_TX_TS_MBX17, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_TX_IMU_MBX24, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_TX_IMU_MBX25, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_IMU_MBX3, ICM_IPC_MSG_OLD_VER, 0},
#elif defined(CFG_SOC_PLATFORM_CLOUD_V2)
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TS, HISI_RPROC_RX_TS_MBX8, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TS, HISI_RPROC_RX_TS_MBX9, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TS, HISI_RPROC_RX_TS_MBX10, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TS, HISI_RPROC_RX_TS_MBX11, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TS, HISI_RPROC_TX_TS_MBX2, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TS, HISI_RPROC_TX_TS_MBX3, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TS, HISI_RPROC_TX_TS_MBX4, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TS, HISI_RPROC_TX_TS_MBX5, ICM_IPC_MSG_NEW_VER, 0},

        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_IMU_MBX20, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_IMU_MBX21, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_IMU_MBX22, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_IMU_MBX23, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_TX_IMU_MBX28, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_TX_IMU_MBX29, ICM_IPC_MSG_NEW_VER, 0},
#elif defined(CFG_SOC_PLATFORM_MINIV2)

#ifdef AOS_LLVM_BUILD
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_SI, HISI_RPROC_SI_Q_RX_RPID2, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_SI, HISI_RPROC_SI_Q_RX_RPID3, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_SI, HISI_RPROC_SI_Q_TX_RPID16, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_SI, HISI_RPROC_SI_Q_TX_RPID17, ICM_IPC_MSG_OLD_VER, 0},
#else
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_SI, HISI_RPROC_SI_Q_RX_RPID0, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_SI, HISI_RPROC_SI_Q_RX_RPID1, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_SI, HISI_RPROC_SI_Q_TX_RPID14, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_SI, HISI_RPROC_SI_Q_TX_RPID15, ICM_IPC_MSG_OLD_VER, 0},
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51_LITE
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_LP_Q_RX_RPID0, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_LP_Q_RX_RPID1, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_LP_Q_RX_RPID2, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_LP_Q_TX_RPID4_ACPU0, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_LP_Q_TX_RPID4_ACPU1, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_LP_Q_TX_RPID4_ACPU2, ICM_IPC_MSG_NEW_VER, 0},
#else
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_LP_Q_RX_RPID0, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_LP_Q_RX_RPID1, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_LP_Q_RX_RPID2, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_LP_Q_TX_RPID4_ACPU0, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_LP_Q_TX_RPID4_ACPU1, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_LP_Q_TX_RPID4_ACPU2, ICM_IPC_MSG_OLD_VER, 0},
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSC, HISI_RPROC_TSC_RX_RPID4, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSC, HISI_RPROC_TSC_RX_RPID5, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSC, HISI_RPROC_TSC_RX_RPID6, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSC, HISI_RPROC_TSC_RX_RPID7, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TSC, HISI_RPROC_TSC_TX_RPID0, ICM_IPC_MSG_IPCDRV_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TSC, HISI_RPROC_TSC_TX_RPID1, ICM_IPC_MSG_IPCDRV_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSV, HISI_RPROC_TSV_RX_RPID4, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSV, HISI_RPROC_TSV_RX_RPID5, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSV, HISI_RPROC_TSV_RX_RPID6, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSV, HISI_RPROC_TSV_RX_RPID7, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TSV, HISI_RPROC_TSV_TX_RPID0, ICM_IPC_MSG_IPCDRV_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TSV, HISI_RPROC_TSV_TX_RPID1, ICM_IPC_MSG_IPCDRV_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TSV, HISI_RPROC_TSC_TX_RPID2, ICM_IPC_MSG_IPCDRV_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TSV, HISI_RPROC_TSV_TX_RPID2, ICM_IPC_MSG_IPCDRV_VER, 0},
#ifdef CFG_SOC_PLATFORM_MDC_V51_LITE
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_LP_Q_RX_RPID3, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_LP_Q_TX_RPID4_ACPU3, ICM_IPC_MSG_NEW_VER, 0},
#else
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_LP_Q_RX_RPID3, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_LP_Q_TX_RPID4_ACPU3, ICM_IPC_MSG_OLD_VER, 0},
#endif

#else
        { ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSC, HISI_RPROC_TSC_RX_RPID4, ICM_IPC_MSG_OLD_VER, 0 },
        { ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSC, HISI_RPROC_TSC_RX_RPID5, ICM_IPC_MSG_OLD_VER, 0 },
        { ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSC, HISI_RPROC_TSC_RX_RPID6, ICM_IPC_MSG_OLD_VER, 0 },
        { ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSC, HISI_RPROC_TSC_RX_RPID7, ICM_IPC_MSG_OLD_VER, 0 },
        { ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TSC, HISI_RPROC_TSC_TX_RPID0, ICM_IPC_MSG_OLD_VER, 0 },
        { ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TSC, HISI_RPROC_TSC_TX_RPID1, ICM_IPC_MSG_OLD_VER, 0 },
        { ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSV, HISI_RPROC_TSV_RX_RPID4, ICM_IPC_MSG_OLD_VER, 0 },
        { ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSV, HISI_RPROC_TSV_RX_RPID5, ICM_IPC_MSG_OLD_VER, 0 },
        { ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSV, HISI_RPROC_TSV_RX_RPID6, ICM_IPC_MSG_OLD_VER, 0 },
        { ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TSV, HISI_RPROC_TSV_RX_RPID7, ICM_IPC_MSG_OLD_VER, 0 },
        { ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TSV, HISI_RPROC_TSV_TX_RPID0, ICM_IPC_MSG_OLD_VER, 0 },
        { ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TSV, HISI_RPROC_TSV_TX_RPID1, ICM_IPC_MSG_OLD_VER, 0 },
#endif
        /* IPC_ID: 4 */
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_ISP, HISI_RPROC_ISP0_IPC0_MBX0_RX_RPID0, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_ISP, HISI_RPROC_ISP0_IPC0_MBX6_TX_RPID1, ICM_IPC_MSG_OLD_VER, 0},
        /* IPC_ID: 5 */
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_ISP, HISI_RPROC_ISP0_IPC1_MBX0_RX_RPID0, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_ISP, HISI_RPROC_ISP0_IPC1_MBX6_TX_RPID1, ICM_IPC_MSG_OLD_VER, 0},
        /* IPC_ID: 6 */
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_ISP, HISI_RPROC_ISP1_IPC0_MBX0_RX_RPID0, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_ISP, HISI_RPROC_ISP1_IPC0_MBX6_TX_RPID1, ICM_IPC_MSG_OLD_VER, 0},
        /* IPC_ID: 7 */
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_ISP, HISI_RPROC_ISP1_IPC1_MBX0_RX_RPID0, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_ISP, HISI_RPROC_ISP1_IPC1_MBX6_TX_RPID6, ICM_IPC_MSG_OLD_VER, 0},
#elif defined(CFG_SOC_PLATFORM_MINIV3)
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_LP_ACPU0, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_LP_ACPU1, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_LP_ACPU2, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_LP_ACPU3, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_TX_LP_ACPU0, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_TX_LP_ACPU1, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_TX_LP_ACPU2, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_TX_LP_ACPU3, ICM_IPC_MSG_NEW_VER, 0},

        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TS, HISI_RPROC_RX_TS_ACPU0, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TS, HISI_RPROC_RX_TS_ACPU1, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TS, HISI_RPROC_RX_TS_ACPU2, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TS, HISI_RPROC_RX_TS_ACPU3, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TS, HISI_RPROC_TX_TS_ACPU0, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TS, HISI_RPROC_TX_TS_ACPU1, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TS, HISI_RPROC_TX_TS_ACPU2, ICM_IPC_MSG_NEW_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TS, HISI_RPROC_TX_TS_ACPU3, ICM_IPC_MSG_NEW_VER, 0},
#else
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_TS, HISI_RPROC_TX_TS, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_TX, IPC_OBJ_LP, HISI_RPROC_TX_LPM3, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TS, HISI_RPROC_RX_TS_MBX4, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_LPM3_MBX5, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TS, HISI_RPROC_RX_TS_MBX6, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_LPM3_MBX7, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TS, HISI_RPROC_RX_TS_MBX8, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_LPM3_MBX9, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_TS, HISI_RPROC_RX_TS_MBX10, ICM_IPC_MSG_OLD_VER, 0},
        {ICMDRV_FREE, ICM_IPC_RX, IPC_OBJ_LP, HISI_RPROC_RX_LPM3_MBX11, ICM_IPC_MSG_OLD_VER, 0},
#endif
    },
};

/* get current time in ms */
STATIC u64 icm_gettime_ms(void)
{
    struct timespec64 ts;
#define ICM_MS_PER_S 1000
#define ICM_NS_PER_MS (1000*1000)
    ktime_get_real_ts64(&ts);
    return (ts.tv_sec * ICM_MS_PER_S) + (ts.tv_nsec / ICM_NS_PER_MS);
}

STATIC int icm_ipc_chan_valid_check(u32 dev_id, u32 chan_id)
{
    if ((dev_id >= MAX_CHIP_NUM) || (chan_id >= HISI_RPROC_MAX)) {
        return -EINVAL;
    }

    return 0;
}

ICM_HANDLE icm_ipc_msg_chan_alloc(u32 dev_id, u32 peer)
{
    struct icmdrv_ipc_res *res = NULL;
    u32 rproc_id = HISI_RPROC_MAX;
#ifndef AOS_LLVM_BUILD
    u64 used_cnt = U64_MAX;
#else
    u64 used_cnt = ((u64)~0ULL);
#endif
    u32 i;
    u32 index = 0;

    if (dev_id >= MAX_CHIP_NUM || peer >= IPC_OBJ_MAX) {
        icm_err("Input pararmeter is error, (dev_id=%d, peer=%d).\n", dev_id, peer);
        return ICM_INVAILED_FD;
    }

    mutex_lock(&peer_mutex[dev_id][peer]);
    for (i = 0; i < ARRAY_SIZE(icm_ipc_resource[dev_id]); i++) {
        res = &icm_ipc_resource[dev_id][i];
        if (res->direction == ICM_IPC_TX && res->peer == peer && res->status == ICMDRV_FREE) {
            if (res->used_cnt < used_cnt) {
                rproc_id = res->id;
                index = i;
                used_cnt = res->used_cnt;
            }
        }
    }

    if (rproc_id == HISI_RPROC_MAX) {
        mutex_unlock(&peer_mutex[dev_id][peer]);
        icm_err("No free chan, (dev_id=%d, peer=%d).\n", dev_id, peer);
        return ICM_INVAILED_FD;
    }

    icm_ipc_resource[dev_id][index].status = ICMDRV_ALLOC;
    icm_ipc_resource[dev_id][index].used_cnt++;
    mutex_unlock(&peer_mutex[dev_id][peer]);

    return ICM_FD_BUILD(dev_id, rproc_id);
}

void icm_ipc_msg_chan_free(ICM_HANDLE fd)
{
    struct icmdrv_ipc_res *res = NULL;
    u32 dev_id = fd >> ICM_DEV_ID_BEG;
    u32 chan_id = fd & ICM_MAILBOX_ID_MASK;
    u32 i;
    int ret;

    ret = icm_ipc_chan_valid_check(dev_id, chan_id);
    if (ret != 0) {
        icm_err("Input pararmeter is error, (dev_id=%d, chan_id=%d).\n", dev_id, chan_id);
        return;
    }

    for (i = 0; i < ARRAY_SIZE(icm_ipc_resource[dev_id]); i++) {
        res = &icm_ipc_resource[dev_id][i];
        if (res->direction == ICM_IPC_TX && res->id == chan_id && res->status == ICMDRV_ALLOC) {
            mutex_lock(&peer_mutex[dev_id][res->peer]);
            res->status = ICMDRV_FREE;
            mutex_unlock(&peer_mutex[dev_id][res->peer]);
        }
    }
}

void icm_ipc_msg_chan_get_id(u32 peer, u32 direction, u32 chan_id[], u32 *num)
{
    struct icmdrv_ipc_res *res = NULL;
    u32 i;
    int size = 0;

    for (i = 0; i < ARRAY_SIZE(icm_ipc_resource[0]); i++) {
        res = &icm_ipc_resource[0][i];
        if (res->peer == peer && res->direction == direction) {
            chan_id[size++] = res->id;
        }
    }

    *num = size;
}

u16 icm_crc16(const u8 *data, u16 len)
{
    u16 val = NULL_USHORT;
    const u16 poly = CRC_POLYNOMIAL;
    uint8_t ch;
    int i;

    while (len--) {
        ch = *(data++);
        val ^= (ch << BITS_PER_BYTE);
        for (i = 0; i < BITS_PER_BYTE; i++) {
            if (val & BIT15) {
                val = (val << 1) ^ poly;
            } else {
                val = val << 1;
            }
        }
    }

    return (val);
}
EXPORT_SYMBOL(icm_crc16);

int icm_ipc_wait_msg_response(u32 dev_id, u32 chan_id)
{
    struct icmdrv_msg_chan *chan = NULL;
    unsigned long wait_jiffies;

    chan = &icm_ctrl->msg_chan[dev_id][chan_id];
    wait_jiffies = msecs_to_jiffies(chan->wait_time);
    return wait_event_timeout(chan->wait_queue, (atomic_read(&chan->wait_flag) == 0), wait_jiffies);
}

void icm_ipc_msg_packet_wrap(struct icmdrv_msg_chan *chan, struct icmdrv_ipc_msg *org_msg, u32 *rproc_msg)
{
    int i;
    struct icmdrv_ipc_msg_info *ipc_msg = (struct icmdrv_ipc_msg_info *)rproc_msg;

    if (chan->msg_version != ICM_IPC_MSG_NEW_VER) {
        if (chan->msg_version == ICM_IPC_MSG_IPCDRV_VER) {
            struct ipcdrv_msg_header *header = (struct ipcdrv_msg_header *)org_msg;
            ipc_msg->crc16 = icm_crc16((u8 *)rproc_msg, sizeof(struct ipcdrv_msg_header) + header->msg_length);
        }
        return;
    }
    chan->msg_seq++;
    ipc_msg->sub_cmd = org_msg->sub_cmd;
    ipc_msg->cmd = org_msg->cmd;
    ipc_msg->cmd_dest = org_msg->cmd_dest;
    ipc_msg->cmd_src = org_msg->cmd_src;
    ipc_msg->len = org_msg->len;
    ipc_msg->msg_seq = chan->msg_seq;
    ipc_msg->msg_type = org_msg->msg_type;
    ipc_msg->version = ICM_IPC_MSG_NEW_VER;
    for (i = 0; i < ICM_MSG_DATA_LENGTH; i++) {
        ipc_msg->data[i] = org_msg->data[i];
    }
    ipc_msg->crc16 = icm_crc16((u8 *)ipc_msg, ipc_msg->len + ICM_IPC_MSG_HEAD_LEN);
    return;
}

STATIC void icm_ipc_send_msg_store(struct icmdrv_msg_chan *chan, const u32 *send_msg, u32 send_len)
{
    int i;

    for (i = 0; i < IPCDRV_RPROC_MSG_LENGTH; i++) {
        chan->send_data[i] = send_msg[i];
    }

    return;
}

void icm_ipc_send_msg_clear(struct icmdrv_msg_chan *chan)
{
    int i;

    for (i = 0; i < IPCDRV_RPROC_MSG_LENGTH; i++) {
        chan->send_data[i] = 0;
    }

    return;
}

STATIC int icm_ipc_msg_check_and_wrap(u32 dev_id, u32 chan_id, u32 *msg, u32 send_len, u32 *send_msg)
{
    struct icmdrv_msg_chan *chan = NULL;
    int ret;
    u32 i;

    ret = icm_ipc_chan_valid_check(dev_id, chan_id);
    if (ret != 0) {
        icm_err("Input pararmeter is error, (dev_id=%d, chan_id=%d).\n", dev_id, chan_id);
        return ret;
    }
    if ((msg == NULL) || (send_len > IPCDRV_RPROC_MSG_LENGTH) || (send_len < 1)) {
        icm_err("Input pararmeter is error, (msg=%pK, send_len=%d, dev_id=%d, chan_id=%d).\n",
            msg, send_len, dev_id, chan_id);
        return -EINVAL;
    }

    for (i = 0; i < send_len; i++) {
        send_msg[i] = msg[i];
    }

    chan = &icm_ctrl->msg_chan[dev_id][chan_id];
    icm_ipc_msg_packet_wrap(chan, (struct icmdrv_ipc_msg *)msg, send_msg);

    return 0;
}

int icm_ipc_msg_send_async(ICM_HANDLE fd, u32 *msg, u32 send_len)
{
    u32 send_msg[IPCDRV_RPROC_MSG_LENGTH] = {0};
    u32 dev_id = fd >> ICM_DEV_ID_BEG;
    u32 chan_id = fd & ICM_MAILBOX_ID_MASK;
    int ret;

    ret = icm_ipc_msg_check_and_wrap(dev_id, chan_id, msg, send_len, send_msg);
    if (ret != 0) {
        icm_err("Input pararmeter is error, (dev_id=%d, chan_id=%d).\n", dev_id, chan_id);
        return ret;
    }

    ret = rproc_xfer_async((int)dev_id, (rproc_id_t)chan_id, send_msg, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        icm_err("rproc_xfer_async failed, (dev_id=%u, chan_id=%u).\n", dev_id, chan_id);
        return ret;
    }

    return 0;
}

int icm_ipc_msg_fast_send_async(ICM_HANDLE fd, u32 *msg, u32 send_len)
{
    u32 send_msg[IPCDRV_RPROC_MSG_LENGTH] = {0};
    u32 dev_id = fd >> ICM_DEV_ID_BEG;
    u32 chan_id = fd & ICM_MAILBOX_ID_MASK;
    int ret;

    ret = icm_ipc_msg_check_and_wrap(dev_id, chan_id, msg, send_len, send_msg);
    if (ret != 0) {
        icm_err("Input pararmeter is error, (dev_id=%d, chan_id=%d).\n", dev_id, chan_id);
        return ret;
    }

    ret = fast_rproc_xfer_async((int)dev_id, (rproc_id_t)chan_id, send_msg, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        icm_err("fast_rproc_xfer_async failed, (dev_id=%u, chan_id=%u).\n", dev_id, chan_id);
        return ret;
    }

    return 0;
}

int icm_ipc_msg_send_sync_para_check(u32 *msg, u32 send_len, u32 *ack_buf, u32 ack_len)
{
    if ((msg == NULL) || (ack_buf == NULL) || (send_len > (IPCDRV_RPROC_MSG_LENGTH << 1u)) ||
        (send_len < 1) || (ack_len > IPCDRV_RPROC_MSG_LENGTH) || (ack_len < 1)) {
        icm_err("Input pararmeter is error, msg(%pK),ack_buffer(%pK),send_len(%d),ack_len(%d).\n", msg, ack_buf,
            send_len, ack_len);
        return -EINVAL;
    }

    return 0;
}

int icm_ipc_msg_multiple_send_async(u32 dev_id, u32 chan_id, u32 *msg, u32 send_len)
{
    struct devdrv_ipc_cmd *ipc_msg = (struct devdrv_ipc_cmd *)msg;
    int ret;

    ipc_msg->cmdPara0[0] = 0;                   /* offset */
    ipc_msg->cmdPara0[1] = 0;                   /* finish */
    ipc_msg->cmdPara0[2] = CMD_DMP_MSG_MAX_LEN; /* length */
    ret = rproc_xfer_async((int)dev_id, (rproc_id_t)chan_id, (u32 *)ipc_msg, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        icm_err("First rproc_xfer_async failed, (dev_id=%u, chan_id=%u).\n", dev_id, chan_id);
        return ret;
    }

    ipc_msg++;
    ipc_msg->cmdPara0[0] = CMD_DMP_MSG_MAX_LEN; /* offset */
    ipc_msg->cmdPara0[1] = 1;                   /* finish */
    ipc_msg->cmdPara0[2] = (u8)(((send_len - IPCDRV_RPROC_MSG_LENGTH) * sizeof(u32)) - CMD_DMP_MSG_MAX_LEN); /* len */
    ret = rproc_xfer_async((int)dev_id, (rproc_id_t)chan_id, (u32 *)ipc_msg, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        icm_err("Second rproc_xfer_async failed, (dev_id=%u, chan_id=%u).\n", dev_id, chan_id);
        return ret;
    }

    return 0;
}

int icm_ipc_msg_send_sync_handle(u32 dev_id, u32 chan_id, u32 *msg, u32 send_len)
{
    struct icmdrv_msg_chan *chan = &icm_ctrl->msg_chan[dev_id][chan_id];
    u32 send_msg[IPCDRV_RPROC_MSG_LENGTH] = {0};
    int ret;
    u32 i;

    if (send_len <= IPCDRV_RPROC_MSG_LENGTH) {
        for (i = 0; i < send_len; i++) {
            send_msg[i] = msg[i];
        }
        icm_ipc_msg_packet_wrap(chan, (struct icmdrv_ipc_msg *)msg, send_msg);
        icm_ipc_send_msg_store(chan, msg, IPCDRV_RPROC_MSG_LENGTH);
        ret = rproc_xfer_async((int)dev_id, (rproc_id_t)chan_id, send_msg, IPCDRV_RPROC_MSG_LENGTH);
    } else if (send_len > IPCDRV_RPROC_MSG_LENGTH && chan->msg_version != ICM_IPC_MSG_NEW_VER) {
        icm_ipc_send_msg_store(chan, msg, IPCDRV_RPROC_MSG_LENGTH);
        ret = icm_ipc_msg_multiple_send_async(dev_id, chan_id, msg, send_len);
    } else {
        ret = -EINVAL;
    }

    return ret;
}

#define ICM_CHANNEL_LOCK_MAX_TRY_TIMES (300)
#define ICM_CHANNEL_RETYR_INTERVAL (10U)  // 10ms
static int icm_ipc_send_try_lock(struct mutex *msg_mutex)
{
    unsigned int i = 0;
    while (mutex_trylock(msg_mutex) == 0) {
        if (i >= ICM_CHANNEL_LOCK_MAX_TRY_TIMES) {
            return -EINVAL;
        }
        msleep(ICM_CHANNEL_RETYR_INTERVAL);
        i++;
    }

    return 0;
}

int icm_ipc_msg_send_sync(ICM_HANDLE fd, u32 *msg, u32 send_len, u32 *ack_buf, u32 ack_len)
{
    struct icmdrv_msg_chan *chan = NULL;
    u32 dev_id = fd >> ICM_DEV_ID_BEG;
    u32 chan_id = fd & ICM_MAILBOX_ID_MASK;
    int ret;
    u64 wait_resp_start;
    u64 wait_resp_end;
    u64 wait_time_cost;

    ret = icm_ipc_chan_valid_check(dev_id, chan_id);
    if (ret != 0) {
        icm_err("Input pararmeter is error, (dev_id=%d, chan_id=%d).\n", dev_id, chan_id);
        return ret;
    }

    ret = icm_ipc_msg_send_sync_para_check(msg, send_len, ack_buf, ack_len);
    if (ret != 0) {
        return ret;
    }

    chan = &icm_ctrl->msg_chan[dev_id][chan_id];

    if (icm_ipc_send_try_lock(&chan->psci_mutex) != 0) {
        icm_err("msg send sync timout, (dev_id=%u, chan_id=%d, send_len=%u).\n", dev_id, chan_id, send_len);
        return ETIMEDOUT;
    }

    atomic_set(&chan->wait_flag, 1);
    ret = icm_ipc_msg_send_sync_handle(dev_id, chan_id, msg, send_len);
    if (ret != 0) {
        mutex_unlock(&chan->psci_mutex);
        icm_err("Send sync failed, (dev_id=%u, chan_id=%d, send_len=%u).\n", dev_id, chan_id, send_len);
        return ret;
    }

    wait_resp_start = icm_gettime_ms();
    ret = icm_ipc_wait_msg_response(dev_id, chan_id);
    wait_resp_end = icm_gettime_ms();
    if (ret <= 0) {
        chan->wait_timeout_count++;
        if (chan->wait_timeout_count >= ICM_TIMEOUT_TIMES) {
            chan->wait_time = IPC_TIME_OUT; // if faild greater than 3,then chang time to 100
        }
        mutex_unlock(&chan->psci_mutex);
        icm_warn("Wait event timeout, (ret=%d, count=%d, wait_time=%d, chan_id=%d).\n", ret,
            chan->wait_timeout_count, chan->wait_time, chan_id);
        return ETIMEDOUT;
    }

    wait_time_cost = wait_resp_end - wait_resp_start;
    // if the ipc response time exceeds 200ms, record the log
    if (wait_time_cost > 200) {
        icm_warn("wait response time too long.(time=%llu(ms), chan_id=%d)\n", wait_time_cost, chan_id);
    }

    chan->wait_timeout_count = 0;
    chan->wait_time = ICM_WAIT_TIMEOUT;

    ret = memcpy_s(ack_buf, ack_len * sizeof(u32), chan->ack_data, ack_len * sizeof(u32));
    if (ret != 0) {
        mutex_unlock(&chan->psci_mutex);
        icm_err("Copy to ack_buffer failed.\n");
        return ret;
    }

    icm_ipc_send_msg_clear(chan);

    mutex_unlock(&chan->psci_mutex);

    return 0;
}
EXPORT_SYMBOL(icm_ipc_msg_send_sync);

int icm_ipc_check_msg_cmd(struct icmdrv_msg_chan *chan, const void *msg)
{
    u8 *send_data = (u8*)chan->send_data;
    u8 *ack_data = (u8*)msg;

    /* check cmd for ts sync mailbox only */
    if (chan->peer == IPC_OBJ_TS || chan->peer == IPC_OBJ_TSC || chan->peer == IPC_OBJ_TSV) {
        /* for struct ipcdrv_message, check cmd */
        if ((send_data[0] & ICM_TS_CMD_MASK) != (ack_data[0] & ICM_TS_CMD_MASK)) {
            return ICM_RX_CONTINUE;
        }
    } else {
        /* check cmd and sub cmd for sync mailbox */
        if ((send_data[0] != ack_data[0]) || (send_data[1] != ack_data[1])) {
            return ICM_RX_CONTINUE;
        }
    }

    return 0;
}

STATIC int icm_ipc_check_msg_vaild(const struct icmdrv_msg_chan *chan, const void *msg)
{
    struct icmdrv_ipc_msg_info *ipc_msg = (struct icmdrv_ipc_msg_info *)msg;
    u16 crc_val;

    if (chan->msg_version == ICM_IPC_MSG_NEW_VER) {
        crc_val = icm_crc16((u8 *)msg, (u16)(ipc_msg->len + ICM_IPC_MSG_HEAD_LEN));
        if (crc_val != ipc_msg->crc16) {
            icm_err("Msg crc check failed, (local crc=0x%x, remote msg crc=0x%x).\n", crc_val, ipc_msg->crc16);
            return -EINVAL;
        }
    }

    return 0;
}

int icm_ipc_chan_msg_recv_init(struct icmdrv_msg_chan *chan, struct icmdrv_ipc_res *res, notifier_call func)
{
    ATOMIC_INIT_NOTIFIER_HEAD(&chan->notifier);

    chan->ipc_monitor.notifier_call = func;
    chan->ipc_monitor.next = NULL;
    chan->ipc_monitor.priority = 0;
    return rproc_rx_register((int)chan->dev_id, (rproc_id_t)chan->chan_id, &chan->ipc_monitor);
}

STATIC struct icmdrv_msg_chan *icm_ipc_recv_get_tx_chan(const struct icmdrv_msg_chan *chan, void *data)
{
    struct icmdrv_msg_chan *tx_chan = NULL;
    u32 chan_id[HISI_RPROC_MAX] = {0};
    u32 num = 0;
    u32 i;
    int ret;

    icm_ipc_msg_chan_get_id(chan->peer, ICM_IPC_TX, chan_id, &num);
    for (i = 0; i < num; i++) {
        tx_chan = &icm_ctrl->msg_chan[chan->dev_id][chan_id[i]];
        ret = icm_ipc_check_msg_cmd(tx_chan, data);
        if (ret == 0) {
            break;
        } else {
            tx_chan = NULL;
        }
    }

    return tx_chan;
}

int icm_ipc_recv_pre_handle(struct icmdrv_msg_chan *chan, void *data, unsigned long len)
{
    struct icmdrv_msg_chan *tx_chan = NULL;
    int ret;

    tx_chan = icm_ipc_recv_get_tx_chan(chan, data);
    if (tx_chan == NULL) {
        return ICM_RX_CONTINUE;
    }

    ret = icm_ipc_check_msg_vaild(tx_chan, data);
    if (ret != 0) {
        icm_err("Seq check failed, drop this packet, (dev_id=%d, chan_id=%d).\n", tx_chan->dev_id, tx_chan->chan_id);
        return ICM_RX_CONTINUE;
    }
    if (atomic_read(&tx_chan->wait_flag) == 1) {
        ret = icm_ipc_check_msg_cmd(tx_chan, data);
        if (ret != 0) {
            return ICM_RX_CONTINUE;
        }
        ret = memcpy_s(tx_chan->ack_data, IPCDRV_RPROC_MSG_LENGTH * sizeof(u32), data, len * sizeof(u32));
        if (ret != 0) {
            icm_err("Copy to data failed.\n");
            return ICM_RX_FINISH;
        }
        atomic_set(&tx_chan->wait_flag, 0);
        wmb();
        wake_up(&tx_chan->wait_queue);
        return ICM_RX_FINISH;
    }

    return ICM_RX_CONTINUE;
}

int icm_ipc_chan_msg_recv_handle(struct notifier_block *nb, unsigned long len, void *data)
{
    struct icmdrv_msg_chan *chan = NULL;
    int ret;

    if ((nb == NULL) || (data == NULL)) {
        icm_err("Nb or data is null.\n");
        return -EINVAL;
    }

    chan = container_of(nb, struct icmdrv_msg_chan, ipc_monitor);
    ret = icm_ipc_recv_pre_handle(chan, data, len);
    if (ret == ICM_RX_FINISH) {
        return 0;
    }

    (void)atomic_notifier_call_chain(&chan->notifier, len, data);
    return 0;
}

void icm_ipc_res_init(void)
{
    struct icmdrv_ipc_res *res = NULL;
    struct icmdrv_msg_chan *chan = NULL;
    u32 i, j;

    for (i = 0; i < MAX_CHIP_NUM; i++) {
        for (j = 0; j < ARRAY_SIZE(icm_ipc_resource[0]); j++) {
            res = &icm_ipc_resource[0][j];
            if (res->direction == ICM_IPC_RX || res->direction == ICM_IPC_TX) {
                chan = &icm_ctrl->msg_chan[i][res->id];
                chan->msg_version = res->msg_version;
                chan->peer = res->peer;
            }
            if (res->direction == ICM_IPC_RX) {
                (void)icm_ipc_chan_msg_recv_init(chan, res, icm_ipc_chan_msg_recv_handle);
            }
            if (i != 0) {
                icm_ipc_resource[i][j] = icm_ipc_resource[0][j]; /* init other dev res */
            }
        }
    }

    for (i = 0; i < MAX_CHIP_NUM; i++) {
        for (j = 0; j < IPC_OBJ_MAX; j++) {
            mutex_init(&peer_mutex[i][j]);
        }
    }
}

void icm_ipc_res_uninit(void)
{
    struct icmdrv_ipc_res *res = NULL;
    struct icmdrv_msg_chan *chan = NULL;
    u32 i, j;

    for (i = 0; i < MAX_CHIP_NUM; i++) {
        for (j = 0; j < ARRAY_SIZE(icm_ipc_resource[0]); j++) {
            res = &icm_ipc_resource[0][j];
            if (res->direction == ICM_IPC_RX) {
                chan = &icm_ctrl->msg_chan[i][res->id];
                (void)rproc_rx_unregister((int)chan->dev_id, (rproc_id_t)chan->chan_id, &chan->ipc_monitor);
            }
        }
    }

    for (i = 0; i < MAX_CHIP_NUM; i++) {
        for (j = 0; j < IPC_OBJ_MAX; j++) {
            mutex_destroy(&peer_mutex[i][j]);
        }
    }
}

int icm_ipc_peer_rx_register(u32 dev_id, u32 peer, notifier_call call_func)
{
    struct icmdrv_msg_chan *chan = NULL;
    u32 chan_id[HISI_RPROC_MAX] = {0};
    u32 num = 0;
    u32 i;

    icm_ipc_msg_chan_get_id(peer, ICM_IPC_RX, chan_id, &num);

    for (i = 0; i < num; i++) {
        chan = &icm_ctrl->msg_chan[dev_id][chan_id[i]];
        chan->icm_monitor.notifier_call = call_func;
        chan->icm_monitor.next = NULL;
        chan->icm_monitor.priority = 0;
        (void)atomic_notifier_chain_register(&chan->notifier, &chan->icm_monitor);
    }

    icm_info("Peer rx register. (dev_id=%u; peer=%u; num=%u)", dev_id, peer, num);
    return 0;
}

int icm_ipc_para_check(u32 dev_id, u32 peer, u32 chan_id)
{
    struct icmdrv_msg_chan *chan = NULL;

    if ((dev_id >= MAX_CHIP_NUM) || (peer >= IPC_OBJ_MAX) || (chan_id > HISI_RPROC_MAX)) {
        icm_err("Input para err. (devid=%u; peer=%u; chan_id=%u; max=%u)", dev_id, peer, chan_id, (u32)HISI_RPROC_MAX);
        return -EINVAL;
    }

    if (chan_id < HISI_RPROC_MAX) {
        chan = &icm_ctrl->msg_chan[dev_id][chan_id];
        if (chan->peer != peer) {
            icm_err("Peer check failed. (peer=%u; chan->peer=%u)", peer, chan->peer);
            return -EINVAL;
        }
    }

    return 0;
}

int icm_ipc_rx_register(u32 dev_id, u32 chan_id, u32 peer, struct notifier_block *nb)
{
    struct icmdrv_msg_chan *chan = NULL;
    int ret;

    ret = icm_ipc_para_check(dev_id, peer, chan_id);
    if (ret != 0) {
        icm_err("Rx register failed.");
        return -EINVAL;
    }

    if ((nb == NULL) || (nb->notifier_call == NULL)) {
        icm_err("Nb or nb->notifier_call is NULL. ");
        return -EINVAL;
    }

    if (chan_id == HISI_RPROC_MAX) {
        (void)icm_ipc_peer_rx_register(dev_id, peer, nb->notifier_call);
    } else {
        chan = &icm_ctrl->msg_chan[dev_id][chan_id];
        (void)atomic_notifier_chain_register(&chan->notifier, nb);
        icm_info("Rx register, (dev_id=%u, peer=%u, chan_id=%u)", dev_id, peer, chan_id);
    }

    return 0;
}
EXPORT_SYMBOL(icm_ipc_rx_register);

int icm_ipc_peer_rx_unregister(u32 dev_id, u32 peer)
{
    struct icmdrv_msg_chan *chan = NULL;
    u32 chan_id[HISI_RPROC_MAX] = {0};
    u32 num = 0;
    u32 i;

    icm_ipc_msg_chan_get_id(peer, ICM_IPC_RX, chan_id, &num);

    for (i = 0; i < num; i++) {
        chan = &icm_ctrl->msg_chan[dev_id][chan_id[i]];
        (void)atomic_notifier_chain_unregister(&chan->notifier, &chan->icm_monitor);
    }

    icm_info("Peer rx unregister. (dev_id=%u; peer=%u; num=%u)", dev_id, peer, num);
    return 0;
}

int icm_ipc_rx_unregister(u32 dev_id, u32 chan_id, u32 peer, struct notifier_block *nb)
{
    struct icmdrv_msg_chan *chan = NULL;
    int ret;

    ret = icm_ipc_para_check(dev_id, peer, chan_id);
    if (ret != 0) {
        icm_err("Rx unregister failed. ");
        return -EINVAL;
    }

    if (nb == NULL) {
        icm_err("Nb is NULL. ");
        return -EINVAL;
    }

    if (chan_id == HISI_RPROC_MAX) {
        (void)icm_ipc_peer_rx_unregister(dev_id, peer);
    } else {
        chan = &icm_ctrl->msg_chan[dev_id][chan_id];
        (void)atomic_notifier_chain_unregister(&chan->notifier, nb);
        icm_info("Rx unregister, (dev id=%d, peer=%d, chan_id=%d)", dev_id, peer, chan_id);
    }

    return 0;
}
EXPORT_SYMBOL(icm_ipc_rx_unregister);

u32 icm_get_msg_version(u32 dev_id, u32 chan_id)
{
    struct icmdrv_msg_chan *chan = NULL;
    u32 version = ICM_IPC_MSG_UNKNOW_VER;

    if ((dev_id >= MAX_CHIP_NUM) || (chan_id > HISI_RPROC_MAX)) {
        icm_err("Input para err. (devid=%u; chan_id=%u)\n", dev_id, chan_id);
        return ICM_IPC_MSG_UNKNOW_VER;
    }

    if (chan_id == HISI_RPROC_MAX) {
#if defined(CFG_SOC_PLATFORM_CLOUD_V2) || defined(CFG_SOC_PLATFORM_MINIV3)
        version = ICM_IPC_MSG_NEW_VER;
#else
        version = ICM_IPC_MSG_OLD_VER;
#endif
    } else {
        chan = &icm_ctrl->msg_chan[dev_id][chan_id];
        if (chan == NULL) {
            icm_err("Get chan is null. (devid=%u; chan_id=%u)\n", dev_id, chan_id);
            return ICM_IPC_MSG_UNKNOW_VER;
        }
        version = chan->msg_version;
    }

    return version;
}
EXPORT_SYMBOL(icm_get_msg_version);

