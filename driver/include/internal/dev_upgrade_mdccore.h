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

#ifndef _DEV_UPGRADE_MDCCORE_H_
#define _DEV_UPGRADE_MDCCORE_H_

#include "linux/sched.h"
#include "linux/wait.h"

#include "dev_upgrade_public.h"
#include "dev_upgrade_adapt.h"
#include "dev_upgrade_core.h"
#include "pkicms_common.h"

#define UFS_FLAG_BASE_OFFSET   0x800000
#define UFS_COND_FLAG_OFFSET   0
#define UFS_AREA_FLAG_OFFSET   4
#define UFS_STATE_FLAG_OFFSET  8
#define UFS_UPGRADE_FLAG_LUN   0
#define UFS_START_CNT_MASK     8
#define UFS_MASTER_START_MAX   4
#define UFS_FLAG_BACKUP_OFFSET 0x10000
#define UFS_UPGRADE_END_FLAG   0xD6C55BC1
#define UFS_SYNC_END_FLAG      0xABCD1234
#define UFS_MASTER_NEW_FLAG    0xC11CB55B
#define UFS_BACKUP_NEW_FLAG    0x3443DAAD
#define UFS_WAIT_RST_FLAG      0x12344321
#define UFS_NORMAL_FLAG        0x5A5A5A5A
#define UFS_MASTER_AREA        0
#define UFS_BACKUP_AREA        1

#define UFS_SINGLE_AREA        0
#define UFS_DOUBLE_AREA        1
#define UFS_VALID_AREA         1
#define UFS_INVALID_AREA       0
#define RESTART_CNT_REG_ADDR   0xC6F4D804
#define RESTART_CNT_REG_LEN    4
#define RESTART_CNT_VAL        0
#define BACKUP_RESTART_CNT_VAL 4
#define MAX_MTD_NAME_SIZE      32
#define UFS_BLOCK_SIZE         4096
#define UFS_SYNC_SIZE          (4 * 1024 * 1024)
#define IMAGE_CHK_ENABLE       1
#define IMAGE_CHK_DISABLE      0
#define IMAGE_CHK_OK           0
#define IMAGE_CHK_ERR          1
#define SYNC_PROP_START        0
#define SYNC_PROP_HALF         50
#define SYNC_PROP_FINISHED     100
#define NONE_EMU_EVENT         1

#define UFS_RAWDATA_CUR_AREA_MASK    0xf
#define UFS_RO_SYSDRV_CUR_AREA_MASK  0x10

#define DISK_OP_TYPE_SYNC 0x0
#define DISK_OP_TYPE_UPDATE 0x1
#define DISK_OP_TYPE_SYNC_RECOVERY 0x2
#define DISK_OP_TYPE_SYNC_FIRMWARE 0x3

/* system type flag */
#define SYS_TYPE_FLAG_SINGLE 0xB1
#define SYS_TYPE_FLAG_MULTI 0xB2

/* flash offset for master and standby area */
#define MASTER_SYS_TYPE_BASE              0xE70000
#define BACKUP_SYS_TYPE_BASE              0xE80000
#define UFS_MASTER_SYS_TYPE_OFFSET        0x0
#define UFS_BACKUP_SYS_TYPE_OFFSET        0x4
#define UPGRADE_DEVICE_AUTHORITY          0440
#define UPGRADE_BUFFER_SIZE               1024
#define UPGRADE_RUNIMGLOCATION_DEFAULT    0x0
#define UPGRADE_CMDLINE_HEX_MODE          16

#define UPGRADE_SPI_FLASH_MEDIA           0
#define UPGRADE_UFS_MEDIA                 1
#define UPGRADE_PCIE_MEDIA                2
#define UPGRADE_SSD_MEDIA                 3
#define UPGRADE_EMMC_MEDIA                4
#define UPGRADE_CUR_MEDIA_MASK            0x80
#define CMDLINE_FILE_PATH                 "/proc/cmdline"
#define CMDLINE_PREFIX                    "runImgLocation="

#define UPGRADE_THREAD_CMD_DEFAULT 0
#define UPGRADE_THREAD_CMD_UPDATE 1
#define UPGRADE_THREAD_CMD_SYNC 2
#define UPGRADE_THREAD_CMD_GET_VER_AND_STATE 3
#define UPGRADE_THREAD_CMD_SYNC_RECOVERY 4
#define UPGRADE_THREAD_CMD_CRL_UPDATE 5
#define UPGRADE_THREAD_CMD_SYNC_FIRMWARE 6


#define UPGRADE_THREAD_RESULT_DEFAULT 0
#define UPGRADE_THREAD_RESULT_SUCCESS 1
#define UPGRADE_THREAD_RESULT_FAIL 2

#define UPGRADE_SYNC_PROC_TIMEOUT 250 /* totally timeout for 500s */
#define UPGRADE_SYS_FILE_BLOCK_SIZE 512
#define UPGRADE_SYNC_PROC_DELAY 2000
#define GET_VER_AND_STATE_PROC_TIMEOUT 1000 /* totally timeout for 5000ms */
#define GET_VER_AND_STATE_PROC_DELAY 5
#define UPGRADE_CRL_UPDATE_TIMEOUT 600 /* totally timeout for 3000ms */
#define UPGRADE_CRL_UPDATE_DELAY 5

#define READ_CONTINUE 1
#define READ_END 0
#define READ_ERROR (-1)

#define VER_AND_STATE_VALID 1
#define VER_AND_STATE_INVALID 0

#define NORMAL_UPGRADE_SCENES 0
#define PXE_UPGRADE_SCENES 1

enum dev_upgrade_fault_level {
    UPGRADE_FAULT_LEVLE_NORMAL = 0,
    UPGRADE_FAULT_LEVLE_MINOR = 1,
    UPGRADE_FAULT_LEVLE_MAJOR = 2,
    UPGRADE_FAULT_LEVLE_CRITICAL = 3,
    UPGRADE_FAULT_LEVEL_MAX
};

enum dev_upgrade_fault_state {
    UPGRADE_FAULT_STATE_OCCUR = 0,
    UPGRADE_FAULT_STATE_RESUME = 1,
    UPGRADE_FAULT_STATE_MAX
};

enum dev_upgrade_fault_id {
    UPGRADE_FAULT_ID_UFS_CHK_ERR = 0,
    UPGRADE_FAULT_ID_MAX
};

enum dev_upgrade_error_no {
    UPGRADE_ERROR_NO_START_FAULT_EVENT = 0x200,
    UPGRADE_ERROR_NO_MAX
};

enum dev_upgrade_query_mode {
    QUERY_FROM_CMDLINE = 0,
    QUERY_FROM_BOOTSTRAP = 1,
    QUERY_FROM_MAX
};

typedef int (*disk_write_handle)(unsigned int flags, u64 offset, const char *data, u64 bytes);
typedef int (*disk_read_handle)(unsigned int flags, u64 offset, char *data, u64 bytes);

typedef struct __disk_ops {
    disk_write_handle cur_boot_disk_write;
    disk_read_handle cur_boot_disk_read;
    disk_write_handle bootstrap_disk_write;
    disk_read_handle bootstrap_disk_read;
} disk_ops_t;

struct dev_upgrade_lun_node {
    u64 node_offset;
    u64 node_len;
};

struct dev_upgrade_lun_info {
    u64 lun_len;
    struct dev_upgrade_lun_node node_info[MAX_MDC_PKT_NUM];
    u32 file_cnt;
    u32 lun;
};

typedef struct __upgrade_ver_and_state {
    unsigned int dev_id;
    int is_valid;
    unsigned char main_zone_ver[DSMI_COMPONENT_TYPE_MAX][COMM_VERSION_LENGTH];
    unsigned char backup_zone_ver[DSMI_COMPONENT_TYPE_MAX][COMM_VERSION_LENGTH];
    int state;
} upgrade_ver_and_state_t;

typedef struct dev_upgrade_crl_info {
    unsigned char *crl_data;
    unsigned int size;
} dev_upgrade_crl_info_t;

typedef struct __upgrade_thread_trans {
    unsigned int dev_id;
    dev_upgrade_core_ctrl *upgrade_ctrl;
    dev_upgrade_crl_info_t *crl_info;
    struct task_struct *thread;
    wait_queue_head_t wait_queue;
    atomic_t cmd;
    atomic_t result;
} upgrade_thread_trans_t;

struct local_image_verify {
    int img_id;
    struct file *fp;
    char *file_name;
    char *img_buff;
    unsigned long long buff_size;
    loff_t img_len;
    loff_t img_loff;
};

int dev_upgrade_components_fill(dev_upgrade_core_ctrl *upgrade_ctrl, struct upgrade_start_in *start_in);
int dev_upgrade_local_components_fill(const struct upgrade_ufs_local_component_start_in *start_in);
void dev_upgrade_clear_localcfg(int dev_id);
int dev_upgrade_mdc_init(void);
void dev_upgrade_mdc_exit(void);
int dev_upgrade_sync_ufs_image(int dev_id);
int dev_upgrade_ufs_image_check(int dev_id);
int dev_upgrade_check_proc(struct upgrade_check_image_in *check_image_in);
unsigned int dev_upgrade_ufs_check_status_get(int dev_id);
void dev_upgrade_ufs_check_status_set(int dev_id, unsigned int status);
int dev_upgrade_set_mdc_flag(int mode, unsigned int lun, unsigned int lun_base_addr,
    unsigned int offset, unsigned int val);
int dev_upgrade_get_mdc_flag(int mode, unsigned int lun, unsigned int lun_base_addr,
    unsigned int offset, unsigned int *val);
int dev_upgrade_ufs_component_core_info_init(void);
void dev_upgrade_ufs_component_core_info_uninit(void);
upgrade_ufs_component_info *dev_upgrade_ufs_get_local_component(unsigned int component_type);
/**
 * Chk file parameters in component.
 Conditon 1 : total length of all files is small than lun length;
 Condition 2 : each file does not overwrite each other.
*/
int dev_upgrade_component_para_chk(int dev_id, DSMI_COMPONENT_TYPE component_type,
    upgrade_sub_file *sub_file, int op_flag);
int dev_upgrade_mdc_update_crl_proc(int dev_id, unsigned char *src_crl_data, unsigned int src_size);
int dev_upgrade_disk_write(int mode, const u8 lun, const u64 offset, const char *buff, const u32 len);
int dev_upgrade_disk_read(int mode, const u8 lun, const u64 offset, char *buff, const u32 len);
int dev_upgrade_get_disk_img_version(dev_upgrade_core_ctrl *upgrade_ctrl, unsigned int component_type,
    unsigned int cur_flag, unsigned char *version_str, unsigned int max_len);
int dev_upgrade_get_disk_img_update_flag(dev_upgrade_core_ctrl *upgrade_ctrl, unsigned int *update_flag);
int dev_upgrade_mdc_update_sync_proc(int op_type, dev_upgrade_core_ctrl *upgrade_ctrl);
int dev_upgrade_get_media(int mode, unsigned int *stat);
int dev_upgrade_mdc_check_update_start_in_para(struct upgrade_start_in *start_in);
int dev_upgrade_mdc_check_update_proc_para(dev_upgrade_core_ctrl *upgrade_ctrl);
int dev_upgrade_mdc_check_sync_start_in_para(struct upgrade_ufs_local_component_start_in *start_in);
int dev_upgrade_mdc_check_sync_proc_para(upgrade_ufs_component_info *local_component);
int dev_upgrade_mdc_flag_update(dev_upgrade_core_ctrl *upgrade_ctrl);
long long dev_upgrade_get_lun_size(int op_flag, u32 lun);
void dev_upgrade_local_components_release(void);
int dev_upgrade_refresh_register_program_package_version(dev_upgrade_core_ctrl *upgrade_ctrl);

#endif /* _DEV_UPGRADE_MDCCORE_H_ */
