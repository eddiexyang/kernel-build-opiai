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

#ifndef _DEVDRV_IPC_H_
#define _DEVDRV_IPC_H_

#include "drv_ipc.h"

#define IPC_RETRY_TIME 3

#define MAX_IPCDRV_MSG_LENGTH 32
#define MAX_IPCDRV_MSG_HEAD 4

#define DEVICE_TYPE_DDR 0
#define DEVICE_TYPE_HBM 2

enum IpcCmdObj {
    OBJ_AP_DMP = 1,  /* 1 */
    OBJ_AP,          /* 2 */
    OBJ_IMU_DMP,     /* 3 */
    OBJ_IMU,         /* 4 */
    OBJ_TS,          /* 5 */
    OBJ_AP_MNTN = 7, /* imu black_box exception report channel */
    OBJ_IMU_MNTN,    /* imu black_box exception report channel */
    MAX_CMD_OBJ
};

/* imu ipc */
#define IPC_IMU_ID 2
/* CMD TYPE0 */
#define IPC_IMU_SMOKE 1
#define IPC_IMU_TEMP 6
#define IPC_IMU_POWER 6
#define IPC_IMU_VOLTAGE 6

#if !defined(CFG_SOC_PLATFORM_MINIV2) || defined(CFG_FEATURE_IMU_ENABLE)
enum IpcCmdMode {
    CMD_ON,          /* 0 */
    CMD_OFF,         /* 1 */
    CMD_INQUIRY,     /* 2 */
    CMD_SETTING,     /* 3 */
    CMD_NOTIFY,      /* 4 */
    CMD_TEST,        /* 5 */
    CMD_IMP_ERRCODE, /* 6 */

    CMD_IMU_MCU_MSG = 5, /* used for OBJ_AP for mcu */
    CMD_IMU_SMOKE = 6, /* used with OBJ_AP */
    CMD_IMU_DMP_MSG = 7,

    MAX_CMD_MODE = 10
};

#define SUB_CMD_IMU_MCU_MSG 0

#define IPC_IMU_HEART_BEAT 7

#else
enum {
    CMD_INQUIRY = 1,
    CMD_SETTING = 2,
    CMD_NOTIFY  = 3,
    CMD_TEST = 4,
    CMD_ON          = 5,
    CMD_IMU_SMOKE = 6,
    CMD_HEART       = 7,
    CMD_DFT_TEST = 8,
    MAX_CMD_MODE
};

// cmdType define
enum {
    SUB_CMD_NO_TYPE = 0,
    SUB_CMD_SMOKE = 1,
    SUB_CMD_TEMPERATURE = 2,
    SUB_CMD_SLEEP = 3,
    SUB_CMD_RESUME = 4,
    SUB_CMD_FREQUENCE = 5,
    SUB_CMD_HEALTH_STATE = 6,
    SUB_CMD_HEALTH_EVENT = 7,
    SUB_CMD_HEART = 8,
    SUB_CMD_VERSION = 9,
    SUB_CMD_LOG = 10,
    SUB_CMD_EXCEPTION = 11,
    SUB_CMD_AICORE_LIMIT = 80,
    SUB_CMD_QUERY_SOC_TEMP,
    SUB_CMD_QUERY_VOLTAGE,
    SUB_CMD_QUERY_AICFREQ,
    SUB_CMD_QUERY_AIVFREQ,
    SUB_CMD_QUERY_CTRLCPUFREQ,
    SUB_CMD_QUERY_LP_HEALTH,
    SUB_CMD_QUERY_DDR_FREQUENCY,
    SUB_CMD_QUERY_DDR_ECC,
    SUB_CMD_QUERY_SOC_PWC,
    SUB_CMD_QUERY_DDR_TEMP,
    SUB_CMD_QUERY_VRD_TEMP,
    SUB_CMD_DDR_THRESHOLD,
    SUB_CMD_SOC_THRESHOLD = 93,
    SUB_CMD_AICORE_VOL_CURRENT,
    SUB_CMD_HYBIRD_VOL_CURRENT,
    SUB_CMD_TAISHAN_VOL_CURRENT,
    SUB_CMD_DDR_VOL_CURRENT,
    SUB_CMD_NOTIFY_LP_SUSPEND,
    SUB_CMD_NOTIFY_LP_RESUME = 99,
    SUB_CMD_QUERY_DDR_BASE_INFO = 100,
    SUB_CMD_QUERY_DDR_MANUFACTURES_INFO = 101,
    SUB_CMD_QUERY_LP_ACG = 102,
    SUB_CMD_NOTIFY_LP_BIST,
    SUB_CMD_INIT_DDR_STATISTIC = 104,
    SUB_CMD_QUERY_DDR_STATISTIC = 105,
};

enum {
    IPC_GET_EMU_FAULT_INFO,
    IPC_GET_SI_FAULE_INFO,
};

#define IPC_IMU_HEART_BEAT 1
#endif
#define IPC_IMU_INFO 8
#define IPC_IMU_DDR 8
#define IPC_IMU_HBM 9
#define IPC_IMU_DEBUG 9
#define IPC_IMU_AICORE 2

/* CMD TYPE1 */
#define IPC_IMU_POWER_1 14
#define IPC_IMU_TEMP_1 11
#define IPC_IMU_FREQ_1 10  // freq
#define IPC_IMU_ECC_1 7
#define IPC_IMU_VOLTAGE_1 12
#define IPC_IMU_HEART_BEAT_1 1
#define IPC_IMU_DEBUG_1 0

#if defined(CFG_SOC_PLATFORM_CLOUD)
typedef enum {
    MAILBOX_TO_IMU_MBX0,
    MAILBOX_TO_IMU_MBX1,
    MAILBOX_TO_TS_MBX,
    MAILBOX_TO_IMU_MBX3,
    MAILBOX_NUM
} mailbox_id;
#elif defined(CFG_SOC_PLATFORM_MINIV2)
typedef enum {
    MAILBOX_TO_TS_MBX,
    MAILBOX_TO_TS_MBX1,
    MAILBOX_TO_TS_MBX2,
    MAILBOX_TO_LP_MBX0,
    MAILBOX_TO_LP_MBX1,
    MAILBOX_TO_LP_MBX2,
    MAILBOX_TO_SI_MBX2,
    MAILBOX_TO_SI_MBX1,
    MAILBOX_TO_TSV_MBX,
    MAILBOX_NUM
} mailbox_id;

#define DEVDRV_SIL_MAX_CMD_DATA_LEN 22

struct devdrv_ipc_sil_bbox_msg {
    u8 cmdType;
    u8 cmd;
    u8 destAddr;
    u8 srcAddr;
    u16 reqSeq;
    u8 dataType : 1;  // 0:request  1:response
    u8 reserved : 3;  // reserved
    u8 chanType : 4;  // 1:uart  2:s_ipc  3:ns_ipc
    u8 len;
    u8 data[DEVDRV_SIL_MAX_CMD_DATA_LEN];  // The data field is variable length
    u16 crcVal;
};

#else
typedef enum {
    MAILBOX_TO_LPM3_MBX,
    MAILBOX_TO_TS_MBX,
    MAILBOX_TO_LPM3_MBX2,
    MAILBOX_NUM
} mailbox_id;
#endif

#define TO_LP_MAX_DATA_LEN  28

struct devdrv_ipc_imu {
    u8 cmd_type1;
    u8 cmd_type0;
    u8 target_id;
    u8 source_id;

    u8 cmd_para0;
    u8 cmd_para1;
    u8 cmd_para2;
    u8 cmd_para3;

    u8 cmd_data0;
    u8 cmd_data1;
    u8 cmd_data2;
    u8 cmd_data3;

    u32 data1;
    u32 data2;
    u32 data3;
    u32 data4;
    u32 data5;
};

#define IPC_CMDTYPE_SILS_PMUWDG_EN 0x11
#define IPC_CMDLEN_SILS_PMUWDG_EN 0x1
#define IPC_RSPLEN_SILS_PMUWDG_EN 0x1
typedef enum {
    IPC_SUBCMD_SILS_PMUWDG_DISABLE = 0,
    IPC_SUBCMD_SILS_PMUWDG_ENABLE = 1,
    IPC_SUBCMD_SILS_PMUWDG_STATUS = 2,
    IPC_SUBCMD_SILS_INVALID = 0xFF,
} IPC_SUBCMD_SILS;

#define IPC_GET_SAFETYISLAND_INFO_CMD_TYPE 0xb
#define IPC_GET_SAFETYISLAND_INFO_CMD 0x1
#define IPC_SET_SAFETYISLAND_INFO_CMD 0x2

#define IPC_GET_SAFETYISLAND_INFO_FUN_TYPE_LEN 0x1

#define IPC_GET_EMU_INFO_PAYLOAD_LEN 12
#define IPC_GET_SAFETYISLAND_INFO_PAYLOAD_LEN 2
#define IPC_SET_SAFETYISLAND_INFO_PAYLOAD_LEN 1

#define IPC_SAFETY_ISLAND_EMU_RSP_LEN 12
#define IPC_SAFETY_ISLAND_INFO_RSP_LEN 2

#define IPC_SAFETY_ISLAND_REQUEST 0
#define IPC_SAFETY_ISLAND_RESPONSE 1

/* subsystem id define */
enum drvfault_msg_sub_id {
    TAISHAN0_SUBSYS = 0,
    TAISHAN1_SUBSYS = 1,
    TAISHAN0_SUBSYS_AOS = 2,
    TAISHAN1_SUBSYS_AOS = 3,
    LP_SUBSYS = 4,
    TSCPU0_SUBSYS = 5,
    TSCPU1_SUBSYS = 6,
    SAFETY_ISLAND = 7,
    HISS_SUBSYS = 8,
    ISP0_SUBSYS = 9,
    ISP1_SUBSYS = 10,
    ISP2_SUBSYS = 11,
    ISP3_SUBSYS = 12,
    HIFI_DSP_SUBSYS = 13,
    SENSOR_HUB_SUBSYS = 14,
    OPPOSITE_SAFETY_ISLAND = 15,
    MCU_SYS = 16,
    OOPPOSITE_MCU_SYS = 17,
    TOTAL_SUBSYS_NUM = 18,
};

typedef enum {
    GET_SAFETY_ISLAND_INFO = 0,
    GET_EMU_INFO,
} GET_SAFETYISLAND_INFO_FUN_TYPE;

struct drv_safety_island_ipc_info {
    unsigned char cmd_type;
    unsigned char cmd;
    unsigned char cmd_dest;
    unsigned char cmd_src;
    unsigned short seq;
    unsigned char data_type : 1; // 0:request  1:response
    unsigned char reserved : 3;  // reserved
    unsigned char chan_type : 4; // 1:uart  2:s_ipc  3:ns_ipc
    unsigned char len;
    unsigned char payload[DRVFAULT_IPC_MSG_LENGTH];
    unsigned short crc;
};

#define PRINT_IPC_MSG_FROM_SAFETYISLAND(ipc_msg) do {           \
    int i;\
    devdrv_drv_err("cmd_type   = 0x%x\n", ipc_msg->cmd_type);\
    devdrv_drv_err("cmd        = 0x%x\n", ipc_msg->cmd);\
    devdrv_drv_err("cmd_dest   = 0x%x\n", ipc_msg->cmd_dest);\
    devdrv_drv_err("cmd_src    = 0x%x\n", ipc_msg->cmd_src);\
    devdrv_drv_err("seq        = 0x%x\n", ipc_msg->seq);\
    devdrv_drv_err("data_type  = 0x%x\n", ipc_msg->data_type);\
    devdrv_drv_err("reserved   = 0x%x\n", ipc_msg->reserved);\
    devdrv_drv_err("chan_type  = 0x%x\n", ipc_msg->chan_type);\
    devdrv_drv_err("len        = 0x%x\n", ipc_msg->len);\
    devdrv_drv_err("crc        = 0x%x\n", ipc_msg->crc);\
    for (i = 0; i < DRVFAULT_IPC_MSG_LENGTH; i++) {\
        devdrv_drv_err("payload[%d] = 0x%x\n", i, ipc_msg->payload[i]);\
    }\
} while (0)

#define IPC_PARA_OFFSET_8BIT 8
#define IPC_PARA_OFFSET_16BIT 16
#define IPC_PARA_OFFSET_24BIT 24
#define IPC_PARA_OFFSET_32BIT 32

#define CMD_PARA_LEN 4

#define CMD_DMP_MSG_HEAD_LEN 4
/* 24 bytes: ipc mail size 32 bytes, 4 bytes ipc cmd head, 4 bytes dmp ipc msg head */
#define CMD_DMP_MSG_MAX_LEN (MAX_IPCDRV_MSG_LENGTH - MAX_IPCDRV_MSG_HEAD - CMD_DMP_MSG_HEAD_LEN)

struct cmd_imu_dmp_msg {
    u8 offset;
    u8 finish;
    u8 length;
    u8 seq;
    u8 msg[CMD_DMP_MSG_MAX_LEN];
};

struct devdrv_ipc_cmd_data {
    u8 valid;
    u8 len;
    u8 seq;
    u8 data[MAX_IPCDRV_MSG_LENGTH];
};

enum {
    IPC_CTRL_CPU_TO_TS_AIVECTORE,
    IPC_CTRL_CPU_TO_TS_AICORE,
    IPC_CTRL_CPU_TO_LOW_POWER
};

typedef int (*notifier_call_func)(struct notifier_block *nb, unsigned long len, void *data);

extern int ipc_mailbox_rx_register(u32 dev_id);
extern void ipc_mailbox_rx_unregister(u32 dev_id);
extern int devdrv_ipc_send_to_imu(unsigned long arg);
extern int devdrv_ipc_recv_from_imu(unsigned long arg);
extern void rproc_print_ipc_status(int ipc_id, rproc_id_t rproc_id);
extern struct devdrv_manager_info *dev_manager_info;
extern int devdrv_ipc_msg_check(void);

#endif
