/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-11-4
*/

#ifndef TRS_SEC_EH_MSG_H
#define TRS_SEC_EH_MSG_H

#include <linux/types.h>

#define TRS_SEC_EH_VER 1
#define TRS_HW_SQE_SIZE 64

enum trs_sec_eh_cmd {
    TRS_SEC_EH_VER_NEGO,
    TRS_SEC_EH_MB_SEND,
    TRS_SEC_EH_SQE_UPDATE,
    TRS_SEC_EH_RES_CTRL,
    TRS_SEC_EH_MAX = 50 // for incompatible problem, if one day add another cmd.
};

struct trs_sec_eh_msg_head {
    enum trs_sec_eh_cmd cmd_type;
    int result;
    u32 tsid;

    u64 rsv;
};

/* vm与pm 版本协商使用 */
struct trs_sec_eh_ver_nego_info {
    struct trs_sec_eh_msg_head head;

    u32 ver;
};

struct trs_sec_eh_mbox_info {
    struct trs_sec_eh_msg_head head;

    u8 data[64]; // ts mailbox size is 64
};

struct trs_sec_eh_sq_update_info {
    struct trs_sec_eh_msg_head head;

    int pid;
    u32 sqid;
    u32 first_sqeid;
    u32 sqe_num;
    u8 data[256];
};

struct trs_sec_eh_res_ctrl_info {
    struct trs_sec_eh_msg_head head;

    u32 id_type;
    u32 id;
    u32 cmd;
    u64 rsv;
};

#endif
