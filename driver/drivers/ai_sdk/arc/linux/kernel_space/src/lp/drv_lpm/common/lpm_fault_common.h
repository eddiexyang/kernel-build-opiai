/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
*/
#ifndef LPM_FAULT_COMMON_H
#define LPM_FAULT_COMMON_H

#include <linux/types.h>
#include <linux/printk.h>
#include <linux/securec.h>
#include <linux/compiler.h>
#include <linux/time64.h>
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include <linux/seq_file.h>
#endif
#include "drv_ipc.h"
#include "dms_pg_info.h"
#include "lpm_log_base.h"

#define MODULE_LPM "[DRV_LPM_FAULT]"
#define MODULE_LPM_MATCH "hisi,lpm_fault_manager"
#ifndef LPM_VPRINTK
#define lpm_log_err(...)   (void)printk(KERN_ERR MODULE_LPM __VA_ARGS__)
#define lpm_log_warn(...)  (void)printk(KERN_WARNING MODULE_LPM __VA_ARGS__)
#define lpm_log_info(...)  (void)printk(KERN_INFO MODULE_LPM __VA_ARGS__)
#else
#define lpm_log_err(...)   (void)lpm_log_vprintk_emit(LOGLEVEL_ERR,     MODULE_LPM __VA_ARGS__)
#define lpm_log_warn(...)  (void)lpm_log_vprintk_emit(LOGLEVEL_WARNING, MODULE_LPM __VA_ARGS__)
#define lpm_log_info(...)  (void)lpm_log_vprintk_emit(LOGLEVEL_INFO,    MODULE_LPM __VA_ARGS__)
#endif

#ifndef STATIC
#ifndef DRV_SOC_MISC_UT
#define STATIC static
#else
#define STATIC
#endif
#endif

#if defined(CFG_SOC_PLATFORM_HELPER) || defined(CFG_SOC_PLATFORM_MINIV2)
// multiple chips, each chip has one die
// or two chips share one os
#define LPM_FAULT_DIE_OFFSET        0ULL
#define LPM_FAULT_CHIP_OFFSET       0x8000000000ULL
#define LPM_DMS_NODE_MAX_NUM        2U

// config for sysctl reg
#define LPM_FAULT_SYSCTL_REG_BASE_ADDR      0x80000000ULL
#define LPM_FAULT_SYSCTL_REG_SIZE           0x10000ULL
#define LPM_FAULT_SYSCTL_VERSION_REG_OFFSET 0xFFFCULL

#define LPM_FAULT_SYSCTL_LP_STATUS_OFFSET        0xF0A8ULL
#define LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE1  0x1F1F1F1FU
#define LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE2  0x2F2F2F2FU

// config for heartbeat
#define LPM_FAULT_HEART_BEAT_CYCLE         3
// whether to check heartbeat broken when resume
#define LPM_FAULT_HEART_BEAT_RESUME_CHECK  false
// is need check heartbeat recovery
#define LPM_FAULT_HEART_BEAT_RECOVERY_CHECK  true
// is need check heartbeat seq read from sharemem
#define LPM_FAULT_HEART_BEAT_SHAREMEM_CHECK true

// time cycle check, unit:ms
#define LPM_HEARTBEAT_TIME_THRESHOLD_WARN   10000
#define LPM_HEARTBEAT_LOST_BBOX_CODE        0xA6192D15U

// ipc channel type
// tx ipc channel num
#define LPM_FAULT_IPC_TPROC_NUM     1
// tx ipc channel info
#define LPM_FAULT_IPC_TPROC_ID_0    HISI_RPROC_LP_Q_TX_RPID4_ACPU1 // for heartbeat

// rx ipc channel num
#define LPM_FAULT_IPC_RPROC_NUM     1
// rx ipc channel info
#define LPM_FAULT_IPC_RPROC_ID_0    HISI_RPROC_LP_Q_RX_RPID1 // for heartbeat

// CMD source & dest define
#define LP_CPU_ID_HB                4U // for heartbeat report
#define TAISHAN_CPU_ID_HB           0U

#define LP_CPU_ID_FAULT             4U // for fault report
#define TAISHAN_CPU_ID_FAULT        0U // for fault report

// cmd define
#define LPM_FAULT_CMD_INQUIRY       1U
// sub_cmd define
#define LPM_FAULT_SUB_CMD_SET_HB    8U

// node type info
#define LPM_FAULT_NODE_TYPE_NUM     1U
#define LPM_FAULT_LPM_NODE_NUM      1U

// total node id num
#define LPM_FAULT_NODE_NUM (LPM_FAULT_LPM_NODE_NUM)

// all sensor num supported
#define LPM_FAULT_SENSOR_TYPE_NUM     1U

// total sensor num
#define LPM_FAULT_LPM_SENSOR_NUM      1U
#define LPM_FAULT_SENSOR_NUM (LPM_FAULT_LPM_SENSOR_NUM * LPM_FAULT_LPM_NODE_NUM)

// the type of module that needs to be sent and received by IPC
enum lpm_fault_ipc_tx_type {
	LPM_IPC_NOTIFY_HB,
	LPM_IPC_TX_TYPE_MAX
};

enum lpm_fault_ipc_rx_type {
	LPM_IPC_RX_HB,
	LPM_IPC_RX_TYPE_MAX
};

#elif defined(CFG_SOC_PLATFORM_CLOUD)
// multiple chip, each chip has one die
#define LPM_FAULT_DIE_OFFSET        0ULL
#define LPM_FAULT_CHIP_OFFSET       0x200000000000ULL
#define LPM_DMS_NODE_MAX_NUM        4U

// config for sysctl reg
#define LPM_FAULT_SYSCTL_REG_BASE_ADDR          0x80000000ULL
#define LPM_FAULT_SYSCTL_REG_SIZE               0x10000ULL
#define LPM_FAULT_SYSCTL_VERSION_REG_OFFSET     0xFFFCULL

#define LPM_FAULT_SYSCTL_LP_STATUS_OFFSET        0x0ULL      // no use
#define LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE1  0x1F1F1F1FU // no use
#define LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE2  0x2F2F2F2FU // no use

// config for heartbeat
#define LPM_FAULT_HEART_BEAT_CYCLE         3
// whether to check heartbeat broken when resume
#define LPM_FAULT_HEART_BEAT_RESUME_CHECK  false
// is need check heartbeat recovery
#define LPM_FAULT_HEART_BEAT_RECOVERY_CHECK  true
// is need check heartbeat seq read from sharemem
#define LPM_FAULT_HEART_BEAT_SHAREMEM_CHECK false

// time cycle check, unit:ms
#define LPM_HEARTBEAT_TIME_THRESHOLD_WARN   10000
#define LPM_HEARTBEAT_LOST_BBOX_CODE        0xA62FFFFFU

// ipc channel type
// tx ipc channel num
#define LPM_FAULT_IPC_TPROC_NUM     1
// tx ipc channel info
#define LPM_FAULT_IPC_TPROC_ID_0    HISI_RPROC_TX_IMU_MBX24 // for heartbeat

// rx ipc channel num
#define LPM_FAULT_IPC_RPROC_NUM     2
// rx ipc channel info
#define LPM_FAULT_IPC_RPROC_ID_0    HISI_RPROC_RX_IMU_MBX0 // for heartbeat
#define LPM_FAULT_IPC_RPROC_ID_1    HISI_RPROC_RX_IMU_MBX3 // for fault report

// CMD source & dest define
#define LP_CPU_ID_HB               4U // for heartbeat
#define TAISHAN_CPU_ID_HB          2U // for heartbeat

#define LP_CPU_ID_FAULT            8U // for fault report
#define TAISHAN_CPU_ID_FAULT       7U // for fault report

// cmd define
#define LPM_FAULT_CMD_INQUIRY       7U
// sub_cmd define
#define LPM_FAULT_SUB_CMD_SET_HB    1U

// node type info
#define LPM_FAULT_NODE_TYPE_NUM     1U
#define LPM_FAULT_LPM_NODE_NUM      1U

// total node id num
#define LPM_FAULT_NODE_NUM (LPM_FAULT_LPM_NODE_NUM)

// all sensor num supported
#define LPM_FAULT_SENSOR_TYPE_NUM     5U

// total sensor num for each node type
#define LPM_FAULT_LPM_SENSOR_NUM      5U
#define LPM_FAULT_SENSOR_NUM (LPM_FAULT_LPM_SENSOR_NUM * LPM_FAULT_LPM_NODE_NUM)

// the type of module that needs to be sent and received by IPC
enum lpm_fault_ipc_tx_type {
	LPM_IPC_NOTIFY_HB,
	LPM_IPC_TX_TYPE_MAX
};

enum lpm_fault_ipc_rx_type {
	LPM_IPC_RX_FAULT,
	LPM_IPC_RX_HB,
	LPM_IPC_RX_TYPE_MAX
};

#elif defined(CFG_SOC_PLATFORM_MDC_V51)
// multiple chips, each chip has one die
// or two chips share one os
#define LPM_FAULT_DIE_OFFSET        0ULL
#define LPM_FAULT_CHIP_OFFSET       0x8000000000ULL
#define LPM_DMS_NODE_MAX_NUM        1U

// config for sysctl reg
#define LPM_FAULT_SYSCTL_REG_BASE_ADDR          0x80000000ULL
#define LPM_FAULT_SYSCTL_REG_SIZE               0x10000ULL
#define LPM_FAULT_SYSCTL_VERSION_REG_OFFSET     0xFFFCULL

#define LPM_FAULT_SYSCTL_LP_STATUS_OFFSET        0xF0A8ULL
#define LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE1  0x1F1F1F1FU
#define LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE2  0x2F2F2F2FU

// config for heartbeat
#define LPM_FAULT_HEART_BEAT_CYCLE         3
// whether to check heartbeat broken when resume
#define LPM_FAULT_HEART_BEAT_RESUME_CHECK  false
// is need check heartbeat recovery
#define LPM_FAULT_HEART_BEAT_RECOVERY_CHECK  true
// is need check heartbeat seq read from sharemem
#define LPM_FAULT_HEART_BEAT_SHAREMEM_CHECK false

// time cycle check, unit:ms
#define LPM_HEARTBEAT_TIME_THRESHOLD_WARN   10000
#define LPM_HEARTBEAT_LOST_BBOX_CODE        0xA6192D15U

// ipc channel type
#ifndef LPM_FAULT_RUN_IN_AOS
// tx ipc channel num
#define LPM_FAULT_IPC_TPROC_NUM     2
// rx ipc channel num
#define LPM_FAULT_IPC_RPROC_NUM     2

#else
// tx ipc channel num
#define LPM_FAULT_IPC_TPROC_NUM     1
// rx ipc channel num
#define LPM_FAULT_IPC_RPROC_NUM     1

#endif
// tx ipc channel info
#define LPM_FAULT_IPC_TPROC_ID_0    HISI_RPROC_LP_Q_TX_RPID4_ACPU1 // for linux heartbeat
#define LPM_FAULT_IPC_TPROC_ID_1    HISI_RPROC_LP_Q_TX_RPID4_ACPU0 // for linux fault report
#define LPM_FAULT_IPC_TPROC_ID_2    HISI_RPROC_LP_Q_TX_RPID4_ACPU3 // for aoscore fault report and heartbeat

// rx ipc channel info
#define LPM_FAULT_IPC_RPROC_ID_0    HISI_RPROC_LP_Q_RX_RPID1 // for linux heartbeat
#define LPM_FAULT_IPC_RPROC_ID_1    HISI_RPROC_LP_Q_RX_RPID2 // for linux fault report
#define LPM_FAULT_IPC_RPROC_ID_2    HISI_RPROC_LP_Q_RX_RPID3 // for aoscore fault report and heartbeat

// CMD source & dest define
#define LP_CPU_ID_HB                4U
#define TAISHAN_CPU_ID_HB           0U

#define LP_CPU_ID_FAULT             4U // for fault report
#define TAISHAN_CPU_ID_FAULT        0U // for fault report

// cmd define
#define LPM_FAULT_CMD_NOTIFY        3U
#define LPM_FAULT_CMD_SETTING       2U
#define LPM_FAULT_CMD_INQUIRY       1U
// sub_cmd define
#define LPM_FAULT_SUB_CMD_SET_CHAN  13U
#define LPM_FAULT_SUB_CMD_SET_HB    8U
#define LPM_FAULT_SUB_CMD_HEALTH_STATE 6U

// node type info
#define LPM_FAULT_NODE_TYPE_NUM       9U
#define LPM_FAULT_LPM_NODE_NUM        1U
#define LPM_FAULT_PMU_NODE_NUM        1U
#define LPM_FAULT_VR_NODE_NUM         1U
#define LPM_FAULT_AO_NODE_NUM         1U
#define LPM_FAULT_DISP_NODE_NUM       1U
#define LPM_FAULT_AA_NODE_NUM         1U
#define LPM_FAULT_SCHE_NODE_NUM       1U
#define LPM_FAULT_SMMU_NODE_NUM       1U
#define LPM_FAULT_PG_SENSOR_NODE_NUM  2U

// total node id num
#define LPM_FAULT_NODE_NUM (LPM_FAULT_LPM_NODE_NUM + LPM_FAULT_PMU_NODE_NUM + LPM_FAULT_VR_NODE_NUM + \
	LPM_FAULT_AO_NODE_NUM + LPM_FAULT_DISP_NODE_NUM + LPM_FAULT_AA_NODE_NUM + LPM_FAULT_SCHE_NODE_NUM + \
	LPM_FAULT_SMMU_NODE_NUM + LPM_FAULT_PG_SENSOR_NODE_NUM)

// all sensor num supported
#define LPM_FAULT_SENSOR_TYPE_NUM     13U

// total sensor num
#define LPM_FAULT_LPM_SENSOR_NUM      9U
#define LPM_FAULT_PMU_SENSOR_NUM      6U
#define LPM_FAULT_VR_SENSOR_NUM       3U
#define LPM_FAULT_AO_SENSOR_NUM       3U
#define LPM_FAULT_DISP_SENSOR_NUM     3U
#define LPM_FAULT_AA_SENSOR_NUM       3U
#define LPM_FAULT_SCHE_SENSOR_NUM     2U
#define LPM_FAULT_SMMU_SENSOR_NUM     2U
#define LPM_FAULT_PG_SENSOR_NUM       1U
#define LPM_FAULT_SENSOR_NUM \
	((LPM_FAULT_LPM_SENSOR_NUM * LPM_FAULT_LPM_NODE_NUM) + (LPM_FAULT_PMU_SENSOR_NUM * LPM_FAULT_PMU_NODE_NUM) + \
	(LPM_FAULT_VR_SENSOR_NUM * LPM_FAULT_VR_NODE_NUM) + (LPM_FAULT_AO_NODE_NUM * LPM_FAULT_AO_SENSOR_NUM) + \
	(LPM_FAULT_DISP_SENSOR_NUM * LPM_FAULT_DISP_NODE_NUM) + (LPM_FAULT_AA_SENSOR_NUM * LPM_FAULT_AA_NODE_NUM) + \
	(LPM_FAULT_SCHE_SENSOR_NUM * LPM_FAULT_SCHE_NODE_NUM) + (LPM_FAULT_SMMU_SENSOR_NUM * LPM_FAULT_SMMU_NODE_NUM) + \
	(LPM_FAULT_PG_SENSOR_NUM * LPM_FAULT_PG_SENSOR_NODE_NUM))

#define LPM_AO_SYSCNT_VALUE_ADDR   0x80020008U
#define LPM_AO_SYSCNT_VALUE_LENGTH 8U

// the type of module that needs to be sent and received by IPC
enum lpm_fault_ipc_tx_type {
	LPM_IPC_SET_FAULT_CHAN,
	LPM_IPC_NOTIFY_HB,
	LPM_IPC_TX_TYPE_MAX
};

enum lpm_fault_ipc_rx_type {
	LPM_IPC_RX_FAULT,
	LPM_IPC_RX_HB,
	LPM_IPC_RX_TYPE_MAX
};

#elif defined(CFG_SOC_PLATFORM_MINIV3)
// one chip, each chip has one die
#define LPM_FAULT_DIE_OFFSET        0ULL
#define LPM_FAULT_CHIP_OFFSET       0ULL
#define LPM_DMS_NODE_MAX_NUM        1U

// config for sysctl reg
#define LPM_FAULT_SYSCTL_REG_BASE_ADDR          0xC0140000ULL
#define LPM_FAULT_SYSCTL_REG_SIZE               0x10000ULL
#define LPM_FAULT_SYSCTL_VERSION_REG_OFFSET     0xFFFCULL

#define LPM_FAULT_SYSCTL_LP_STATUS_OFFSET        0x0ULL      // no use
#define LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE1  0x1F1F1F1FU // no use
#define LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE2  0x2F2F2F2FU // no use

// config for heartbeat
#define LPM_FAULT_HEART_BEAT_CYCLE         3
// whether to check heartbeat broken when resume
#define LPM_FAULT_HEART_BEAT_RESUME_CHECK  false
// is need check heartbeat recovery
#define LPM_FAULT_HEART_BEAT_RECOVERY_CHECK  true
// is need check heartbeat seq read from sharemem
#define LPM_FAULT_HEART_BEAT_SHAREMEM_CHECK true

// time cycle check, unit:ms
#define LPM_HEARTBEAT_TIME_THRESHOLD_WARN   10000
#define LPM_HEARTBEAT_LOST_BBOX_CODE        0xA6192D15U

// ipc channel type
// tx ipc channel num
#define LPM_FAULT_IPC_TPROC_NUM     1
// tx ipc channel info
#define LPM_FAULT_IPC_TPROC_ID_0    HISI_RPROC_TX_LP_ACPU0 // for heartbeat

// rx ipc channel num
#define LPM_FAULT_IPC_RPROC_NUM     2
// rx ipc channel info
#define LPM_FAULT_IPC_RPROC_ID_0    HISI_RPROC_RX_LP_ACPU1 // for heartbeat
#define LPM_FAULT_IPC_RPROC_ID_1    HISI_RPROC_RX_LP_ACPU2 // for fault report

// CMD source & dest define
#define LP_CPU_ID_HB                OBJ_CMD_LP
#define TAISHAN_CPU_ID_HB           OBJ_CMD_CCPU

#define LP_CPU_ID_FAULT             OBJ_CMD_LP   // for fault report
#define TAISHAN_CPU_ID_FAULT        OBJ_CMD_CCPU // for fault report

// cmd define
#define LPM_FAULT_CMD_INQUIRY       ICM_MAIN_CMD_LP_HB
// sub_cmd define
#define LPM_FAULT_SUB_CMD_SET_HB    ICM_SUB_CMD_HEART

// node type info
#define LPM_FAULT_LPM_NODE_NUM      1U
#define LPM_FAULT_AO_NODE_NUM       1U
#define LPM_FAULT_DISP_NODE_NUM     1U
#define LPM_FAULT_VR_NODE_NUM       1U

// all sensor num supported
#define LPM_FAULT_SENSOR_TYPE_NUM     9U

// total sensor num
#define LPM_FAULT_LPM_SENSOR_NUM      8U
#define LPM_FAULT_AO_SENSOR_NUM       1U
#define LPM_FAULT_DISP_SENSOR_NUM     1U
#define LPM_FAULT_VR_SENSOR_NUM       3U

#if !defined(CFG_SOC_PLATFORM_MDC_V11)
#define LPM_FAULT_NODE_TYPE_NUM     4U

// total node id num
#define LPM_FAULT_NODE_NUM (LPM_FAULT_LPM_NODE_NUM + LPM_FAULT_DISP_NODE_NUM + LPM_FAULT_AO_NODE_NUM + \
	LPM_FAULT_VR_NODE_NUM)

#define LPM_FAULT_SENSOR_NUM  \
	((LPM_FAULT_LPM_SENSOR_NUM * LPM_FAULT_LPM_NODE_NUM) + (LPM_FAULT_DISP_NODE_NUM * LPM_FAULT_DISP_SENSOR_NUM) + \
	(LPM_FAULT_AO_SENSOR_NUM * LPM_FAULT_AO_NODE_NUM) + (LPM_FAULT_VR_SENSOR_NUM * LPM_FAULT_VR_NODE_NUM))

#else
#define LPM_FAULT_NODE_TYPE_NUM       6U
#define LPM_FAULT_AA_NODE_NUM         1U
#define LPM_FAULT_SMMU_NODE_NUM       1U

#define LPM_FAULT_AA_SENSOR_NUM       1U
#define LPM_FAULT_SMMU_SENSOR_NUM     1U

// total node id num
#define LPM_FAULT_NODE_NUM (LPM_FAULT_LPM_NODE_NUM + LPM_FAULT_DISP_NODE_NUM + LPM_FAULT_AO_NODE_NUM + \
	LPM_FAULT_VR_NODE_NUM + LPM_FAULT_AA_NODE_NUM + LPM_FAULT_SMMU_NODE_NUM)

#define LPM_FAULT_SENSOR_NUM  \
	((LPM_FAULT_LPM_SENSOR_NUM * LPM_FAULT_LPM_NODE_NUM) + (LPM_FAULT_DISP_NODE_NUM * LPM_FAULT_DISP_SENSOR_NUM) + \
	(LPM_FAULT_AO_SENSOR_NUM * LPM_FAULT_AO_NODE_NUM) + (LPM_FAULT_VR_SENSOR_NUM * LPM_FAULT_VR_NODE_NUM) + \
	(LPM_FAULT_AA_NODE_NUM * LPM_FAULT_AA_SENSOR_NUM) + (LPM_FAULT_SMMU_NODE_NUM * LPM_FAULT_SMMU_SENSOR_NUM))

#endif

// the type of module that needs to be sent and received by IPC
enum lpm_fault_ipc_tx_type {
	LPM_IPC_NOTIFY_HB,
	LPM_IPC_TX_TYPE_MAX
};

enum lpm_fault_ipc_rx_type {
	LPM_IPC_RX_FAULT,
	LPM_IPC_RX_HB,
	LPM_IPC_RX_TYPE_MAX
};

#elif defined(CFG_SOC_PLATFORM_CLOUD_V2)
// multiple chips, each chip has one die
// or multiple chips, each chip has two die
#define LPM_FAULT_DIE_OFFSET        0x10000000000ULL
#define LPM_FAULT_CHIP_OFFSET       0x80000000000ULL
#define LPM_DMS_NODE_MAX_NUM        2U

// config for sysctl reg
#define LPM_FAULT_SYSCTL_REG_BASE_ADDR          0x80000000ULL
#define LPM_FAULT_SYSCTL_REG_SIZE               0x10000ULL
#define LPM_FAULT_SYSCTL_VERSION_REG_OFFSET     0xFFFCULL

#define LPM_FAULT_SYSCTL_LP_STATUS_OFFSET        0x0ULL      // no use
#define LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE1  0x1F1F1F1FU // no use
#define LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE2  0x2F2F2F2FU // no use

// config for heartbeat
#define LPM_FAULT_HEART_BEAT_CYCLE         3
// whether to check heartbeat broken when resume
#define LPM_FAULT_HEART_BEAT_RESUME_CHECK  false
// is need check heartbeat recovery
#define LPM_FAULT_HEART_BEAT_RECOVERY_CHECK  true
// is need check heartbeat seq read from sharemem
#define LPM_FAULT_HEART_BEAT_SHAREMEM_CHECK true

// time cycle check, unit:ms
#define LPM_HEARTBEAT_TIME_THRESHOLD_WARN   10000
#define LPM_HEARTBEAT_LOST_BBOX_CODE        0xA6192D15U

// ipc channel type
// tx ipc channel num
#define LPM_FAULT_IPC_TPROC_NUM     1
// tx ipc channel info
#define LPM_FAULT_IPC_TPROC_ID_0    HISI_RPROC_TX_IMU_MBX28 // for heartbeat

// rx ipc channel num
#define LPM_FAULT_IPC_RPROC_NUM     2
// rx ipc channel info
#define LPM_FAULT_IPC_RPROC_ID_0    HISI_RPROC_RX_IMU_MBX20 // for heartbeat
#define LPM_FAULT_IPC_RPROC_ID_1    HISI_RPROC_RX_IMU_MBX22 // for fault report

// CMD source & dest define
#define LP_CPU_ID_HB                OBJ_CMD_LP
#define TAISHAN_CPU_ID_HB           OBJ_CMD_CCPU

#define LP_CPU_ID_FAULT             OBJ_CMD_LP   // for fault report
#define TAISHAN_CPU_ID_FAULT        OBJ_CMD_CCPU // for fault report

// cmd define
#define LPM_FAULT_CMD_INQUIRY       ICM_MAIN_CMD_LP_HB // 0x10
// sub_cmd define
#define LPM_FAULT_SUB_CMD_SET_HB    ICM_SUB_CMD_HEART // 73

// node type info
#define LPM_FAULT_NODE_TYPE_NUM     1U
#define LPM_FAULT_LPM_NODE_NUM      1U
// total node id num
#define LPM_FAULT_NODE_NUM (LPM_FAULT_LPM_NODE_NUM)

// all sensor num supported
#define LPM_FAULT_SENSOR_TYPE_NUM     5U

// total sensor num
#define LPM_FAULT_LPM_SENSOR_NUM      5U
#define LPM_FAULT_SENSOR_NUM (LPM_FAULT_LPM_SENSOR_NUM * LPM_FAULT_LPM_NODE_NUM)

// the type of module that needs to be sent and received by IPC
enum lpm_fault_ipc_tx_type {
	LPM_IPC_NOTIFY_HB,
	LPM_IPC_TX_TYPE_MAX
};

enum lpm_fault_ipc_rx_type {
	LPM_IPC_RX_FAULT,
	LPM_IPC_RX_HB,
	LPM_IPC_RX_TYPE_MAX
};

#elif defined(CFG_SOC_PLATFORM_MINI)
// one chip, each chip has one die
#define LPM_FAULT_DIE_OFFSET            0ULL
#define LPM_FAULT_CHIP_OFFSET           0ULL
#define LPM_DMS_NODE_MAX_NUM            1U

// config for sysctl reg
#define LPM_FAULT_SYSCTL_REG_BASE_ADDR          0x1100c0000ULL
#define LPM_FAULT_SYSCTL_REG_SIZE               0x10000ULL
#define LPM_FAULT_SYSCTL_VERSION_REG_OFFSET     0xFFFCULL

#define LPM_FAULT_SYSCTL_LP_STATUS_OFFSET        0x0ULL      // no use
#define LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE1  0x1F1F1F1FU // no use
#define LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE2  0x2F2F2F2FU // no use

// config for heartbeat
#define LPM_FAULT_HEART_BEAT_CYCLE         6
// whether to check heartbeat broken when resume
#define LPM_FAULT_HEART_BEAT_RESUME_CHECK  true
// is need check heartbeat recovery
#define LPM_FAULT_HEART_BEAT_RECOVERY_CHECK  false
// is need check heartbeat seq read from sharemem
#define LPM_FAULT_HEART_BEAT_SHAREMEM_CHECK false

// time cycle check, unit:ms
#define LPM_HEARTBEAT_TIME_THRESHOLD_WARN   20000
#define LPM_HEARTBEAT_LOST_BBOX_CODE        0xA619FFFFU

// ipc channel type
// tx ipc channel num
#define LPM_FAULT_IPC_TPROC_NUM     1
// tx ipc channel info
#define LPM_FAULT_IPC_TPROC_ID_0    HISI_RPROC_TX_LPM3 // for heartbeat

// rx ipc channel num
#define LPM_FAULT_IPC_RPROC_NUM     1
// rx ipc channel info
#define LPM_FAULT_IPC_RPROC_ID_0    HISI_RPROC_RX_LPM3_MBX5 // for heartbeat

// CMD source & dest define
#define LP_CPU_ID_HB            2U
#define TAISHAN_CPU_ID_HB       1U

// cmd define
#define LPM_FAULT_CMD_INQUIRY   8U
// sub_cmd define
#define LPM_FAULT_SUB_CMD_SET_HB  25U

// the type of module that needs to be sent and received by IPC
enum lpm_fault_ipc_tx_type {
	LPM_IPC_NOTIFY_HB,
	LPM_IPC_TX_TYPE_MAX
};

enum lpm_fault_ipc_rx_type {
	LPM_IPC_RX_HB,
	LPM_IPC_RX_TYPE_MAX
};

#endif
#define LPM_TIMER_BIND_CPU   0

#define LPM_MINUTES_TO_SECOND 60
#define LPM_NS_TO_US 1000 // 1us = 1000ns

#define LPM_FAULT_SINGLE_OS_TYPE_LINUX     0U
#define LPM_FAULT_MULTIPLE_OS_TYPE_LINUX   1U
#define LPM_FAULT_MULTIPLE_OS_TYPE_AOSCORE 2U

// fpga is 0x0, emu is 0x1
#define LPM_FAULT_PLAT_TYPE_ESL           0x2
#define LPM_FAULT_PLAT_TYPE_ASIC          0x3
#define LPM_FAULT_PLAT_TYPE_INVALID       0xFF

#define LPM_FAULT_PLAT_MASK               0x000F0000
#define LPM_FAULT_PLAT_OFFSET             16

#define LPM_PG_INFO_MASK_INVAILID 0xffffffffffffffffULL

enum lpm_pg_module_type {
	LPM_PG_TYPE_CPU,
	LPM_PG_TYPE_AIC,
	LPM_PG_TYPE_AIV,
	LPM_PG_TYPE_DDR,
	LPM_PG_TYPE_OTHER,
};

struct lpm_pg_info {
	uint64_t cpu_mask;
	uint64_t aic_mask;
	uint64_t aiv_mask;
	uint64_t aic_freq;
};

struct lpm_fault_common_dev_info {
	uint32_t chip_id;
	uint32_t die_id;
	uint32_t env_type; // read from version reg
	void __iomem *syscnt_addr;
	void __iomem *sysctl_base;
};

typedef int32_t (*fn_devdrv_get_chip_die_id)(uint32_t dev_id, uint32_t *chip_id, uint32_t *die_id);

struct lpm_fault_common_fn_hook {
	fn_devdrv_get_chip_die_id fn_get_chip_die_id;
};

struct lpm_fault_common_priv {
	uint32_t dev_num;
	struct lpm_fault_common_fn_hook fn_hook;
	struct lpm_fault_common_dev_info dev_priv[LPM_DMS_NODE_MAX_NUM];
};

int32_t lpm_fault_common_init(uint32_t dev_num);
int32_t lpm_fault_common_exit(uint32_t dev_num);
uint32_t lpm_common_get_dev_num(void);
bool lpm_common_check_dev_id(uint32_t dev_id);
uint64_t lpm_common_syscount_get_timestamp(uint32_t dev_id);
bool lpm_fault_query_lp_startup_status(uint32_t dev_id);
uint32_t lpm_common_get_env_type(uint32_t dev_id);
void lpm_common_get_current_time(struct timespec64 *os_time);
void lpm_common_get_time_interval(struct timespec64 *curr_time);
void lpm_common_get_chip_die_id(uint32_t dev_id, uint32_t *chip_id, uint32_t *die_id);
uint16_t lpm_common_crc16(const uint8_t *data, uint16_t len);
#ifdef CFG_FEATURE_PARTIAL_GOOD
struct lpm_pg_info *lpm_crg_monitor_assemble_pginfo(void);
#endif
#endif