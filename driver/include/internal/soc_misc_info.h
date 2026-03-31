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

#ifndef SOC_MISC_INFO_H
#define SOC_MISC_INFO_H

#include "dms_define.h"

struct soc_misc_board_info {
    unsigned int board_id;
    unsigned int pcb_id;
    unsigned int bom_id;
    unsigned int slot_id;
};

struct soc_misc_cpu_info {
    unsigned int ccpu_num;
    unsigned int ccpu_os_sched;
    unsigned int dcpu_num;
    unsigned int dcpu_os_sched;
    unsigned int aicpu_num;
    unsigned int aicpu_os_sched;
    unsigned int tscpu_num;
    unsigned int tscpu_os_sched;
};

struct soc_misc_info_st {
    unsigned int dev_id;
    struct soc_misc_board_info board_info;
    struct soc_misc_cpu_info cpu_info;
};

struct soc_misc_info_cb {
    unsigned int dev_num;
    struct soc_misc_info_st soc_misc_info[DEVICE_NUM_MAX];
};

int soc_misc_dev_info_init(void *data);
struct soc_misc_info_st *soc_misc_get_soc_info(unsigned int dev_id);
int soc_misc_soc_info_init(void);
void soc_misc_soc_info_uninit(void);
int soc_misc_set_cpu_info(void *data);

#endif