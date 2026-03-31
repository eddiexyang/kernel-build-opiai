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

#ifndef DEV_UPGRADE_CORE_H
#define DEV_UPGRADE_CORE_H

#include <linux/mutex.h>
#include "dev_upgrade_def.h"
#include "dev_upgrade_public.h"

#ifndef CHECK_PROCESS_DMP
#define CHECK_PROCESS_DMP "dmp_daemon"
#endif

#define CHECK_YES 1
#define CHECK_NO  0

#define SOURCE_FLASH 0
#define SOURCE_DDR   1

#define CRL_FILE_SIZE_MAX 0x200000
#define CRL_FLASH_MAIN_ADDR_1 0xC20000
#define CRL_FLASH_MAIN_ADDR_2 0xC30000
#define CRL_FLASH_BK_ADDR_1   0xE20000
#define CRL_FLASH_BK_ADDR_2   0xE30000
#define FLASH_AREA_NUM 2
#define TAG_SIZE 12

typedef enum {
    PHASE_UPGRADE,
    PHASE_SYNC,
    PHASE_NONE
} UPGRADE_PHASE;

#define IMG_MAIN_OK    (0 << 0)
#define IMG_SLAVE_OK   (0 << 1)
#define IMG_MAIN_BAD   (1 << 0)
#define IMG_SLAVE_BAD  (1 << 1)

#define IMG_STAT_DOUBLE_OK      (IMG_MAIN_OK | IMG_SLAVE_OK)
#define IMG_STAT_M_BAD_S_OK     (IMG_MAIN_BAD | IMG_SLAVE_OK)
#define IMG_STAT_M_OK_S_BAD     (IMG_MAIN_OK | IMG_SLAVE_BAD)
#define IMG_STAT_DOUBLE_BAD     (IMG_MAIN_BAD | IMG_SLAVE_BAD)

typedef enum {
    UPGRADE_DOUBLE,
    UPGRADE_MASTER,
    UPGRADE_STANDBY,
    UPGRADE_NONE
} UPGRADE_AREA_FLAG;

#if (defined CFG_SOC_PLATFORM_MDC_V51)
struct upgrade_component {
    int dev_id;
    unsigned int component_type;
    unsigned int total_areas;
    unsigned int finish_areas;
    unsigned int main_area;
    unsigned int weight; /* 用于进度计算校正，默认值都是1，如果双区只升级1个区，则动态设置为2 */
    unsigned int system_type;
    unsigned int file_size;
    unsigned char *file_content; /* Flash升级使用，UFS不使用 */
    upgrade_sub_file sub_pkt;    /* UFS升级使用，flash不使用 */
    int (*pre_check)(struct upgrade_component *component);
    int (*update)(struct upgrade_component *component);
};
#else
struct upgrade_component {
    int dev_id;
    unsigned int component_type;
    unsigned int total_areas;
    unsigned int finish_areas;
    unsigned int main_area;
    unsigned int weight; /* 用于进度计算校正，默认值都是1，如果双区只升级1个区，则动态设置为2 */
    unsigned int file_size;
    unsigned char *file_content;
    int (*pre_check)(struct upgrade_component *component);
    int (*update)(struct upgrade_component *component);
};
#endif

#define BLACK_BOX_BUF_LEN (256 * 1024)
#define MAX_INFO_LEN 128

struct black_box_buff_info {
    char *buf;
    struct mutex lock;
    unsigned int w_offset;
    bool flag;
};


typedef struct tag_dev_upgrade_core_ctrl_st {
    int dev_id;
    struct mutex lock;
    unsigned int total_component;
    unsigned int total_size;
    unsigned int finish_size;
    unsigned int schedule;
    unsigned int stop_flag;
    unsigned int clr_localcfg;
    unsigned int system_type;
    struct upgrade_component *component_list[DSMI_COMPONENT_TYPE_MAX];
    struct black_box_buff_info black_box_info;
} dev_upgrade_core_ctrl;

int dev_upgrade_sync_proc(dev_upgrade_core_ctrl *upgrade_ctrl);
int dev_upgrade_firmware_sync_proc(dev_upgrade_core_ctrl *upgrade_ctrl);
void dev_upgrade_localcfg_proc(dev_upgrade_core_ctrl *upgrade_ctrl, struct upgrade_component *component);
PROG_ALL_PKG_INFO *dev_upgrade_get_register_package_info(void);
int dev_upgrade_mdc_register_program_package_init(void);
void dev_upgrade_mdc_register_program_package_uninit(void);
extern PROG_ALL_PKG_INFO *g_upgrade_register_pkg_info;
#endif /* _DEV_UPGRADE_H_ */
