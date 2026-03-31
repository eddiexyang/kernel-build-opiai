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

#define GET_DATA_WAIT_TIMEOUT 3000
#define MAX_IPCDRV_MSG_LENGTH 32
#define MAX_IPCDRV_MSG_HEAD 4

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

enum IpcCmdMode {
    CMD_ON,      /* 0 */
    CMD_OFF,     /* 1 */
    CMD_INQUIRY, /* 2 */
    CMD_SETTING, /* 3 */
    CMD_NOTIFY,  /* 4 */
    CMD_TEST,    /* 5 */

    CMD_IMU_SMOKE = 6, /* used with OBJ_AP */
    CMD_IMU_DMP_MSG = 7,

    MAX_CMD_MODE = 10
};

/* imu ipc */
#define IPC_IMU_ID 2
/* CMD TYPE0 */
#define IPC_IMU_SMOKE 1
#define IPC_IMU_TEMP 6
#define IPC_IMU_POWER 6
#define IPC_IMU_VOLTAGE 6
#define IPC_IMU_HEART_BEAT 7
#define IPC_IMU_INFO 8
#define IPC_IMU_DDR 8
#define IPC_IMU_HBM 9
#define IPC_IMU_DEBUG 9
#define IPC_IMU_AICORE 2


/* CMD TYPE1 */
#define IPC_IMU_POWER_1 14
#define IPC_IMU_TEMP_1 11
#define IPC_IMU_FREQ_1 10 // freq
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
#else
typedef enum {
    MAILBOX_TO_LPM3_MBX,
    MAILBOX_TO_TS_MBX,
    MAILBOX_TO_LPM3_MBX2,
    MAILBOX_NUM
} mailbox_id;
#endif

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

extern int ipc_mailbox_rx_register(u32 dev_id);
extern void ipc_mailbox_rx_unregister(u32 dev_id);
extern int devdrv_ipc_send_to_imu(unsigned long arg);
extern int devdrv_ipc_recv_from_imu(unsigned long arg);
extern int devdrv_ipc_msg_check(void);

#endif
