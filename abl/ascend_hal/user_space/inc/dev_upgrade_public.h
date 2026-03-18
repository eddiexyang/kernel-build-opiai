/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#ifndef _DEV_UPGRADE_PUBLIC_H_
#define _DEV_UPGRADE_PUBLIC_H_

#include <linux/limits.h>

#ifdef CFG_SOC_PLATFORM_CLOUD
#define DEVICE_NUM_MAX 4
#elif (defined CFG_SOC_PLATFORM_MINIV2) || (defined CFG_SOC_PLATFORM_CLOUD_V2)
#define DEVICE_NUM_MAX 2
#else /* mini & mdc & mini v3 */
#define DEVICE_NUM_MAX 1
#endif

#define BITS_OF_INT       32
#define BITS_OF_SHORT     16
#define BITS_OF_CHAR      8
#define BITS_OF_HALF_CHAR 4

#define DRV_ENABLE        1
#define DRV_DISABLE       0
#define DEVICE_ID_INVALID 0xFF

#define DEV_UPGRADE_STOP_FLAG 0x4b4b4b4b
#define UPGRADE_FILE_SIZE_MAX (512 * 1024 * 1024)

#ifndef OK
#define OK 0
#endif

#ifndef ERROR
#define ERROR (-1)
#endif

#define DEV_MID_UPGRADE 0x01

/* 升级模块错误码生成
   通用格式 [15:8]模块 | [7:0]具体错误码 */
#define dev_errno_make(mode, errno) \
    ((((unsigned short)(mode)&0xff) << 8) | ((errno)&0xff))

#define upgrade_errno_make(errno) \
    dev_errno_make(DEV_MID_UPGRADE, errno)

/* [0x00 , 0x3f]通用错误码定义 */
#define ERRNO_NULL_POINT             0x01 /* 空指针 */
#define ERRNO_INVALID_PARAMS         0x02 /* 无效参数 */
#define ERRNO_INAVLID_ADDR           0x03 /* 无效地址 */
#define ERRNO_NONSUPPORT_ITEM        0x04 /* 不支持的选项 */
#define ERRNO_CRC                    0x05 /* CRC计算出错 */
#define ERRNO_INVALID_LENGTH         0x06 /* 参数长度错误 */
#define ERRNO_INVALID_INDEX          0x07 /* 非法的索引、数组下标等 */
#define ERRNO_MEM_MALLOC             0x08 /* 申请块内存 */
#define ERRNO_MEM_FREE               0x09 /* 释放块内存 */
#define ERRNO_NO_INIT                0x0a /* 没有初始化 */
#define ERRNO_REPEAT_INIT            0x0b /* 重复初始化 */
#define ERRNO_SOURCE_NO              0x0c /* 没有资源 */
#define ERRNO_SAME_VERSION           0x0d /* 版本相同 */
#define ERRNO_HASH_CHECK             0x0e /* HASH校验出错 */
#define ERRNO_HASH                   0x0f /* HASH计算出错 */
#define ERRNO_NOT_ALIGN              0x10 /* 非对齐错误 */
#define ERRNO_NOT_HAVE_VALID_PARTION 0x11 /* PART1的四个分区中无有效nve */
#define ERRNO_COPY_FROM_USER         0x12 /* copy_from_user错误 */
#define ERRNO_COPY_TO_USER           0x13 /* copy_to_user错误 */
#define ERRNO_FILE_TYPE              0x14 /* 文件类型不匹配 */
#define ERRNO_MEM_INIT               0x15 /* 内存初始化错误 */
#define ERRNO_PERMISSION_DENIED      0x16 /* 权限被拒绝 */
#define ERRNO_VERIFY_OR_REVOCATION   0x17 /* 认证或者吊销过程错误 */
#define ERRNO_DFM_REGISTER           0x18 /* 黑匣子注册错误 */
#define ERRNO_VERSION_NOT_MATCH      0x19 /* 版本回滚驱动拦截 */

/* [0x40 , 0x5f]操作系统任务相关错误码定义 */
#define ERRNO_TASK_CREATE       0x40 /* 任务创建 */
#define ERRNO_TASK_INIT         0x41 /* 任务初始化 */
#define ERRNO_TASK_DELETE       0x42 /* 任务删除 */
#define ERRNO_TASK_DELETE_FORCE 0x43 /* 任务强制删除 */
#define ERRNO_TASK_SUSPEND      0x44 /* 任务挂起 */
#define ERRNO_TASK_RESUME       0x45 /* 任务恢复 */
#define ERRNO_TASK_RESTART      0x46 /* 任务重启 */
#define ERRNO_TASK_PRIORITY_SET 0x47 /* 任务优先级设置 */
#define ERRNO_TASK_PRIORITY_GET 0x48 /* 任务优先级获取 */
#define ERRNO_TASK_LOCK         0x49 /* 任务上锁 */
#define ERRNO_TASK_UNLOCK       0x4a /* 任务解锁 */
#define ERRNO_TASK_DELAY        0x4b /* 任务切换 */

/* [0x60 , 0x7f]操作系统信号量相关错误码定义 */
#define ERRNO_SEM_GIVE         0x60 /* 信号量 释放 */
#define ERRNO_SEM_TRY          0x61 /* 信号量 尝试获取 */
#define ERRNO_SEM_TAKE         0x62 /* 信号量 获取 */
#define ERRNO_SEM_FLUSH        0x63 /* 信号量 解锁阻塞任务 */
#define ERRNO_SEM_DELETE       0x64 /* 信号量 删除 */
#define ERRNO_SEM_B_CREATE     0x65 /* 创建二进制信号量 */
#define ERRNO_SEM_C_CREATE     0x66 /* 创建计数信号量 */
#define ERRNO_SEM_M_CREATE     0x67 /* 创建互斥信号量 */
#define ERRNO_SEM_M_GIVE_FORCE 0x68 /* 释放互斥信号量 */
#define ERRNO_SEM_B_SM_CREATE  0x69 /* 创建共享内存二进制信号量 */
#define ERRNO_SEM_C_SM_CREATE  0x6a /* 创建共享内存计数信号量 */
#define ERRNO_SEM_SHOW         0x6b /* 信号量 显示 */

/* [0x80 , 0x9f] 文件系统操作类 通用错误码定义 */
#define ERRNO_FS_NOT_FOUND 0x80 /* 文件没有找到 */
#define ERRNO_FS_WRITE     0x81 /* 文件写入失败 */
#define ERRNO_FS_READ      0x82 /* 文件读取失败 */
#define ERRNO_FS_CONTENT   0x83 /* 内容无效 */
#define ERRNO_FS_FILEID    0x84 /* 文件ID无效 */
#define ERRNO_FS_MKDIR     0x85 /* 创建文件夹失败 */
#define ERRNO_FS_OPEN      0x86 /* 打开失败 */
#define ERRNO_FS_CLOSE     0x87 /* 关闭失败 */
#define ERRNO_FS_SEEK      0x88 /* 关闭失败 */
#define ERRNO_FS_ERASE     0x89 /* 擦除失败 */
#define ERRNO_FS_DELETE    0x90 /* 删除文件失败 */
#define ERRNO_FS_REALPATH  0x91 /* realpath失败 */
#define ERRNO_FS_IOCTL     0x92 /* ioctl失败 */

// DSMI_COMPONENT_TYPE 枚举类型修改时需要与对外头文件中的同一个定义对齐
typedef enum {
    DSMI_COMPONENT_TYPE_NVE,
    DSMI_COMPONENT_TYPE_XLOADER,
    DSMI_COMPONENT_TYPE_M3FW,
    DSMI_COMPONENT_TYPE_UEFI,
    DSMI_COMPONENT_TYPE_TEE,
    DSMI_COMPONENT_TYPE_KERNEL,
    DSMI_COMPONENT_TYPE_DTB,
    DSMI_COMPONENT_TYPE_ROOTFS,
    DSMI_COMPONENT_TYPE_IMU,
    DSMI_COMPONENT_TYPE_IMP,
    DSMI_COMPONENT_TYPE_AICPU,
    DSMI_COMPONENT_TYPE_HBOOT1_A,
    DSMI_COMPONENT_TYPE_HBOOT1_B,
    DSMI_COMPONENT_TYPE_HBOOT2,
    DSMI_COMPONENT_TYPE_DDR,
    DSMI_COMPONENT_TYPE_LP,
    DSMI_COMPONENT_TYPE_HSM,
    DSMI_COMPONENT_TYPE_SAFETY_ISLAND,
    DSMI_COMPONENT_TYPE_HILINK,
    DSMI_COMPONENT_TYPE_RAWDATA,
    DSMI_COMPONENT_TYPE_SYSDRV,
    DSMI_COMPONENT_TYPE_ADSAPP,
    DSMI_COMPONENT_TYPE_COMISOLATOR,
    DSMI_COMPONENT_TYPE_CLUSTER,
    DSMI_COMPONENT_TYPE_CUSTOMIZED,
    DSMI_COMPONENT_TYPE_SYS_BASE_CONFIG,
    DSMI_COMPONENT_TYPE_RECOVERY,
    DSMI_COMPONENT_TYPE_HILINK2,
    DSMI_COMPONENT_TYPE_LOGIC_BIST,
    DSMI_COMPONENT_TYPE_MEMORY_BIST,
    DSMI_COMPONENT_TYPE_ATF,
    DSMI_COMPONENT_TYPE_USER_BASE_CONFIG,
    DSMI_COMPONENT_TYPE_BOOTROM,
    DSMI_COMPONENT_TYPE_MAX,
    UPGRADE_ALL_IMAGE_COMPONENT = 0xFFFFFFFD,
    UPGRADE_ALL_FIRMWARE_COMPONENT = 0xFFFFFFFE,
    UPGRADE_ALL_COMPONENT = 0xFFFFFFFF
} DSMI_COMPONENT_TYPE;

typedef enum {
    INDEX_0,
    INDEX_1,
    INDEX_2,
    INDEX_3
} COMMON_INDEX;


#define DAVINCI_LENGTH_FILE_PATH (PATH_MAX - NAME_MAX)

/* Use 'C' as magic number */
#define UPGRADE_CTL_IOC_MAGIC      'C'
#define UPGRADE_FIRMWARE_START     _IOR(UPGRADE_CTL_IOC_MAGIC, 1, int)
#define UPGRADE_READ_SCHEDULE_CTL  _IOR(UPGRADE_CTL_IOC_MAGIC, 2, int)
#define UPGRADE_GET_FIREWARE_VER   _IOR(UPGRADE_CTL_IOC_MAGIC, 3, int)
#define UPGRADE_GET_COMPONENT_LIST _IOR(UPGRADE_CTL_IOC_MAGIC, 4, int)
#define UPGRADE_SYNC_IMAGE_COPY    _IOR(UPGRADE_CTL_IOC_MAGIC, 5, int)
#define UPGRADE_FIRMWARE_STOP      _IOR(UPGRADE_CTL_IOC_MAGIC, 6, int)
#define UPGRADE_CLR_LOCALCFG       _IOR(UPGRADE_CTL_IOC_MAGIC, 7, int)
#define UPGRADE_SEC_REVOCATION     _IOR(UPGRADE_CTL_IOC_MAGIC, 8, int)
#define UPGRADE_SYNC_LOCAL_COMP    _IOR(UPGRADE_CTL_IOC_MAGIC, 9, int)
#define UPGRADE_IMAGE_CHECK        _IOR(UPGRADE_CTL_IOC_MAGIC, 10, int)
#define UPGRADE_GET_STATE_FLAG     _IOR(UPGRADE_CTL_IOC_MAGIC, 11, int)
#define UPGRADE_GET_DEV_INFO       _IOR(UPGRADE_CTL_IOC_MAGIC, 12, int)
#define UPGRADE_VERIFY_IMG         _IOR(UPGRADE_CTL_IOC_MAGIC, 13, int)
#define UPGRADE_SET_RCVR_FLAG          _IOR(UPGRADE_CTL_IOC_MAGIC, 14, int)
#define UPGRADE_GET_RCVR_FLAG          _IOR(UPGRADE_CTL_IOC_MAGIC, 15, int)
#define UPGRADE_CLEAN_RCVR_FLAG        _IOR(UPGRADE_CTL_IOC_MAGIC, 16, int)
#define UPGRADE_RESET_RCVR_BOOT_CNT    _IOR(UPGRADE_CTL_IOC_MAGIC, 17, int)
#define UPGRADE_SET_RCVR_STATUS        _IOR(UPGRADE_CTL_IOC_MAGIC, 18, int)
#define UPGRADE_SET_DEV_INFO           _IOR(UPGRADE_CTL_IOC_MAGIC, 19, int)
#define UPGRADE_GET_CHECK_INFO         _IOR(UPGRADE_CTL_IOC_MAGIC, 20, int)

#define HISFC_PART_NUM_MAX 128

/* RootBubK related */
#define FIRMWARE_HEAD_LEN     0x477
#define ROOT_BUBK_LEN_OFFSET  0x58
#define ROOT_PUBK_OFFSET      0x5C
#define ROOT_PUBK_LEN_VAL     512
#define ROOT_PUBK_TOTAL_SIZE  1024
#define REVOCATE_FILE_LEN     544

/* revocation type */
#define UPGRADE_REVOCATION_TYPE_SOC 0
#define UPGRADE_REVOCATION_TYPE_CMS 1

/* master and standby area */
#define UPGRADE_MASTER_AREA 0
#define UPGRADE_STANDBY_AREA 1

/* command for get upgrade device information */
#define CMD_GET_VERSION     0
/* command for get current partition or set boot partition */
#define CMD_PARTITION_OPS   1

/* system type for mdc */
#define SYSTEM_TYPE_SINGLE 0
#define SYSTEM_TYPE_MULTI 1
#define SYSTEM_TYPE_INVALID 0xFF

#ifdef CFG_SOC_PLATFORM_MDC_V51
#define MAX_MDC_PKT_NUM       32
#define MAX_NAME_LEN          32
#define MAX_PATH_LEN          64
#define MAX_TYPE_LEN          6
#define SUB_FILE_MAX_PATH_LEN (MAX_NAME_LEN + DAVINCI_LENGTH_FILE_PATH)

/**
 * struct mdc_file_info - MDC upgraded component information
 * @path: upgraded file full path
 * @size: upgraded file size
 * @type: file system format. such as ext4/f2fs/nat
 *
 * When MDC upgrading, read mdc configuration and fill in this struct.
 */
struct mdc_file_info {
    char   path[MAX_PATH_LEN];    /* file path in mdc package */
    char   type[MAX_TYPE_LEN];
};

/**
* struct mdc_file_storage - MDC UFS storage information
* @start: Start position of the file to be burnt on the device
* @len: Actual size of the partition
* @sign_ltart: Location of the file signature information in the file
* @sign_len: Length of the file signature information
* @back: Start position of the backup file on the storage device.
*        The value is 0 if there is no backup file.
* @lun: Main logical partition ID saved in the file
* @back_lun: Number of the standby logical partition for which the file is saved
*
* During the MDC upgrade, the upgrade package of each component needs to be
* saved to the UFS. This structure stores the component storage information,
* such as the logical unit number, storage offset, and occupied space.
*/
struct mdc_file_storage {
    unsigned long long    start;
    unsigned long long    len;
    unsigned long long    back;
    unsigned int    lun;
    unsigned int    back_lun;
};

struct  mdc_package {
    char   name[MAX_NAME_LEN];  /* file name: from mdcpack.xml */
    char   file_name[SUB_FILE_MAX_PATH_LEN];
    struct mdc_file_info file_info;
    struct mdc_file_storage storage;
};
typedef struct mdc_package mdc_package_t;

typedef struct tag_upgrade_sub_file_st {
    unsigned int sub_file_count;
    mdc_package_t *sub_file_info[MAX_MDC_PKT_NUM];
} upgrade_sub_file;

struct component_file_info {
    unsigned int component_type;
    unsigned int file_size;
    unsigned char *file_content;
    upgrade_sub_file sub_pkt;
};

typedef struct tag_upgrade_ufs_component_info_st {
    unsigned int component_type;
    unsigned int valid;       // 0:invalid 1:valid
    unsigned int area_flag;   // 0:single 1:double
    unsigned int area_check;  // 0:ok 1:err
    unsigned int cur_flag;    // 0:master 1:backup
    unsigned int system_type; // 0:singleOS 1:multiOS
    char component_name[MAX_NAME_LEN];
    upgrade_sub_file sub_pkt_m;
    upgrade_sub_file sub_pkt_b;
} upgrade_ufs_component_info;

typedef struct tag_upgrade_ufs_single_area_info_st {
    unsigned int component_type;
    unsigned int valid;       // 0:invalid 1:valid
    unsigned int area_flag;   // 0:single 1:double
    unsigned int master_flag; // 0:master 1:backup
    unsigned int cur_flag;    // 0:master 1:backup
    upgrade_sub_file sub_pkt;
} upgrade_ufs_single_area_info;

struct upgrade_ufs_local_component_start_in {
    int dev_id;
    unsigned int component_cnt;
    upgrade_ufs_single_area_info component_info[DSMI_COMPONENT_TYPE_MAX];
};

struct upgrade_ufs_local_component_start_out {
    int result;
};

#else
struct component_file_info {
    unsigned int component_type;
    unsigned int file_size;
    unsigned char *file_content;
};
#endif

struct upgrade_start_in {
    int dev_id;
    unsigned int count;
    unsigned int system_type;
    struct component_file_info file_list[DSMI_COMPONENT_TYPE_MAX];
};

struct upgrade_start_out {
    int result;
};

struct upgrade_read_schedule_in {
    int dev_id;
};

struct upgrade_read_schedule_out {
    unsigned int schedule;
};

struct upgrade_get_component_list_in {
    int dev_id;
};

struct upgrade_get_component_list_out {
    unsigned int bitmap;
};

struct upgrade_sync_image_in {
    int dev_id;
    unsigned int component_type;
};

struct upgrade_sync_image_out {
    int result;
};

struct upgrade_check_image_in {
    int dev_id;
    unsigned int flash_chk;
    unsigned int ufs_chk;
};

struct upgrade_check_image_out {
    int result;
    unsigned int check_result;
};

struct upgrade_get_state_flag_in {
    int dev_id;
};

struct upgrade_get_state_flag_out {
    int result;
    unsigned int state_flag;
};

struct upgrade_stop_in {
    int dev_id;
};

struct upgrade_stop_out {
    int result;
};

struct upgrade_clr_localcfg_in {
    int dev_id;
};

struct upgrade_clr_localcfg_out {
    int result;
};

struct upgrade_get_version_in {
    int dev_id;
    unsigned int component_type;
    unsigned int buffLen;
    void *outBuff;
};

struct upgrade_get_version_out {
    int result;
};

struct upgrade_recovery_status_in {
    unsigned int op;
    unsigned int part;
};

typedef enum {
    DSMI_UPGRADE_MAIN_TYPE_VERSION = 0,
    DSMI_UPGRADE_MAIN_TYPE_PARTITION,
    DSMI_UPGRADE_MAIN_TYPE_REG_PROG_PKG_INFO, // register program package infomation
    DSMI_UPGRADE_SUB_TYPE_FW_VERIFY,
    DSMI_UPGRADE_MAIN_TYPE_MAX,
} DSMI_UPGRADE_MAIN_TYPE;

/* partition type */
typedef enum {
    PARTITION_A = 0,
    PARTITION_B,
} PARTITION_TYPE;

/* package type, Security information is at the head or tail of the file. */
typedef enum {
    SEC_INFO_AT_HEAD = 0,
    SEC_INFO_AT_TAIL,
} PACKAGE_TYPE;

#define PROG_PKG_INFO_RESERVE_LEN 10
/* Format of the input parameter of the DSMI_UPGRADE_MAIN_TYPE_REG_PROG_PKG_INFO */
typedef struct program_pkg_info {
    unsigned int component_id;
    PARTITION_TYPE partition;
    unsigned int lun_id;
    unsigned long long lun_offset;
    PACKAGE_TYPE pkg_type;
    unsigned int reserve[PROG_PKG_INFO_RESERVE_LEN];
} PROG_PKG_INFO;

struct upgrade_dev_info_in {
    int dev_id;
    unsigned int cmd;
    unsigned int size;
    void *buf;
};

struct upgrade_dev_info_out {
    unsigned int size;
    int result;
};

struct upgrade_revocation_in {
    int dev_id;
    unsigned int type;
    unsigned int file_size;
    void *file_data;
};

struct upgrade_revocation_out {
    int result;
};

struct upgrade_verify_img_in {
    int dev_id;
    char *path;
    unsigned int size;
    int mode;
};

struct upgrade_verify_img_out {
    int result;
};

struct upgrade_ioctl_msg {
    int dev_id;
    signed int in_size;
    signed int out_size;
    void *in;
    void *out;
};

struct upgrade_get_check_info_in {
    int dev_id;
    unsigned int buffLen;
    void *outBuff;
};

struct upgrade_get_check_info_out {
    int result;
};

#endif /* _DEV_UPGRADE_PUBLIC_H_ */
