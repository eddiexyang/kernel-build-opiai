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

#ifndef __USER_CFG_MANAGEMENT_H
#define __USER_CFG_MANAGEMENT_H

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include "user_cfg_public.h"

#define DEVDRV_FLASH_BLOCK_SIZE UC_FLASH_PARTITION_SIZE /* 64KB */
#define DEVDRV_FLASH_FIRST_BLOCK 0
#define DEVDRV_FLASH_SECOND_BLOCK (UC_FLASH_PARTITION_NUM / 2)
#define DEVDRV_FLASH_INVALID_BLOCK 0xFF
#define DEVDRV_FLASH_INVALID 0
#define DEVDRV_FLASH_VALID 1

/* flag for recored valid head */
#define DEVDRV_FLASH_HEAD_INVALID 0
#define DEVDRV_FLASH_MAIN_HEAD_VALID (1 << 0)
#define DEVDRV_FLASH_BK_HEAD_VALID (1 << 1)
#define DEVDRV_FLASH_HEAD_ALL_VALID (DEVDRV_FLASH_MAIN_HEAD_VALID | DEVDRV_FLASH_BK_HEAD_VALID)

#define DEVDRV_SHA256_DIGEST 32
#define DEVDRV_PARTITION_DIV 2

#define DEVDRV_UC_HEAD_FOUND 1
#define DEVDRV_UC_HEAD_NOT_FOUND (-1)

#define DEVDRV_UC_FLASH_WR_TIMEOUT 3

#define DEVDRV_UC_HEAD_LEN_MAX (UC_BLK_INFO_OFFSET - UC_CFG_HEAD_ITEM_START)
#define DEVDRV_UC_HEAD_NUM_MAX (DEVDRV_UC_HEAD_LEN_MAX / sizeof(uc_cfg_head_t))

/* write flash flag */
#define DEVDRV_UC_FLASH_FLAG_MAIN (1 << 0)
#define DEVDRV_UC_FLASH_FLAG_BACKUP (1 << 1)
#define DEVDRV_UC_FLASH_FLAG_ALL (DEVDRV_UC_FLASH_FLAG_MAIN | DEVDRV_UC_FLASH_FLAG_BACKUP)

/* sync main and backup content flag */
#define DEVDRV_UC_SYNC_ALL 0x0
#define DEVDRV_UC_SYNC_ONE_BLOCK 0x1

#define DEVDRV_UC_SYNC_DONE 0x1
#define DEVDRV_UC_NOT_SYNC 0x0

#define DEVDRV_UC_NEED_SYNC 0x1
#define DEVDRV_UC_NO_NEED_SYNC 0x0

/* init first block op flag */
#define DEVDRV_UC_BLOCK_INIT_NO_WR 0
#define DEVDRV_UC_BLOCK_INIT_WR 1

#define UC_CFG_INDEX_DYNAMIC (-1)
#define UC_CFG_INDEX_FOUND 1
#define UC_CFG_INDEX_NOT_FOUND 0

/* device id */
#define DEVDRV_UC_DEV_ID_0 0
#define DEVDRV_UC_DEV_ID_1 1
#define DEVDRV_UC_DEV_ID_2 2
#define DEVDRV_UC_DEV_ID_3 3

#define DEVDRV_UC_FLASH_DEFAULT_VAL 0xFF
#define DEVDRV_UC_NAME_MAX 32

#define DEVDRV_UC_DATT_LEN_MIN 4
#define DEVDRV_UC_DATA_NUM_EVEN 2

/* process white list */
#ifdef CFG_USER_CFG_SUPPORT_HCCP
#define WHITE_LIST_PROCESS_NUM 2
#else
#define WHITE_LIST_PROCESS_NUM 1
#endif

#define PROCESS_NAME_DMP "dmp_daemon"
#define PROCESS_NAME_HCCP "hccp_service.bin"

#define FLASH_REMAP_BASE_ADDR 0x90a40000U
#define CHIP_REG_BASE_OFFSET 0x8000000000

#define SIGN_DEFAULT_VALUE (-1)
#define IS_FROM_MEMORY  1
#define IS_FROM_FLASH   0

#ifdef CFG_USER_CFG_SUPPORT_COMPACT_SPACE
typedef struct _uc_cfg_item_info {
    unsigned int blk_offset;
    unsigned int start_offset;
    unsigned int end_offset;
} uc_cfg_item_info_t;
#endif

typedef struct __devdrv_config_para {
    u32 dev_id;
    s32 cmd;
    s32 cfg_index;
    s8 name[DEVDRV_UC_NAME_MAX];
    u32 buf_size;
    void *buf;
} devdrv_cfg_para_t;

typedef struct __uc_add_item_info {
    u32 dev_id;
    u8 *head_blk;
    u8 *content_blk;
    u32 content_blk_index;
    u8 *content_buf;
    u32 content_size;
    u32 valid_blk;
    u32 head_valid_flag;
    s32 head_index;
    uc_cfg_head_t *head_ops;
    s32 main_head_wr_flag;
    s32 bk_head_wr_flag;
    s32 main_content_wr_flag;
    s32 bk_content_wr_flag;
} uc_item_info_t;

typedef struct __uc_sync_info {
    u32 dev_id;
    u32 blk_index;
    u8 *main_blk;
    u8 *backup_blk;
    u8 *head_blk;
    u32 main_wr_flg;
    u32 bk_wr_flg;
} uc_sync_info_t;

struct user_config_item_default_func {
    s8 *cfg_name;
    int (*get_para_post_process_func)(u8 *buf, u32* buf_size, const struct user_config_item *item);
};

typedef struct __devdrv_pss_para {
    u32 sign;
    struct mutex sign_mutex;
} devdrv_cfg_pss_t;

#define ASCEND_CTL_GET_DEV_NUM_FUNC "ascend_ctl_get_dev_num"
typedef int (*get_dev_num_handler_t)(unsigned int *dev_num);

#define ASCEND_CTL_GET_DTS_CPU_CONFIG_FUNC "ascend_ctl_get_dts_cpu_cfg_to_user_cfg"
typedef int (*get_dts_cpu_config_handler_t)(unsigned int *ctrl_cpu_num, unsigned int *data_cpu_num,
    unsigned int *ai_cpu_num);

/*
 *  description:    ioctl handler for normal user user configure operation
 *  return:         0 success?, not 0 fail
 */
int devdrv_flash_user_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

/*
 *  description:    ioctl handler for root user user configure operation
 *  return:         0 success?, not 0 fail
 */
int devdrv_config_ioctl(struct file *filep, u32 cmd, unsigned long arg);

/*
 *  description:    clear user information in user cfg partition
 *  @para:          dev_id       device id
 *  @para:          item_index   user config item index
 *  return:         0 success?? not 0 fail
 */
s32 devdrv_config_clear_user_item(u32 dev_id, u32 item_index);

/*
 *  description:    get cpu configuration in flash
 *  @para:          dev_id       device id
 *  @para:          buf          buffer for store data
 *  @para:          buf_size     buffer size
 *  return:         0 success, not 0 fail
 */
s32 devdrv_config_get_cpu_cfg(u32 dev_id, u8 *buf, u32 buf_size);

/*
 *  description:    get pss configuration in memory
 *  @para:          dev_id       device id
 *  @para:          sign         sign type
 *  return:         0 success, not 0 fail
 */
int devdrv_config_get_pss_cfg(unsigned int dev_id, int *sign);
/*
 *  description:    set pss configuration
 *  @para:          dev_id       device id
 *  @para:          sign         sign type
 *  return:         0 success, not 0 fail
 */
int devdrv_config_set_pss_cfg(unsigned int dev_id, int sign);
/*
 *  description:    to init pss configuration
 *  @para:          sign         sign type
 *  return:         0 success, not 0 fail
 */
s32 devdrv_config_pss_cfg_init(u32 sign);
/*
 *  description:    to uninit pss configuration
 */
void devdrv_config_pss_cfg_uninit(void);
/*
 *  description:    get device number
 *  @para:          dev_num       device number
 *  return:         0 success, not 0 fail
 */
int devdrv_config_get_dev_num(unsigned int *dev_num);
#endif
