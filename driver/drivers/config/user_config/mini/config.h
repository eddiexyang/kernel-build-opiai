/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This file contains user config related code.
 * Author: huawei
 * Create: 2020-05-15
 */
#ifndef __OPIAI_USER_CONFIG_MINI_H
#define __OPIAI_USER_CONFIG_MINI_H

#ifndef NULL
#define NULL    (0L)
#endif

#define CURRENT_USER_CONFIG_VERSION  1 /* 当前软件能支持的用户配置的版本号 */
#define UC_FLASH_BASE_ADDR           0x250000 /* 配置所在的flash偏移地址 */
#define UC_FLASH_MAIN_ADDR           0x250000 /* 配置所在的flash主区偏移地址 */
#define UC_FLASH_BK_ADDR             0x260000 /* 配置所在的flash备区偏移地址 */
#define UC_FLASH_PARTITION_NUM       2 /* 用户配置分区数量 */
#define UC_FLASH_PARTITION_MAIN_NUM  1 /* 用户配置区主区数量 */
#define UC_FLASH_PARTITION_BK_NUM    1 /* 用户配置区备区数量 */
#define UC_FLASH_PARTITION_SIZE      (64 * 1024) /* 用户配置分区大小(字节数) */

#define UC_VERION_FLASH_OFFSET       0  /* 配置版本号偏移 */
#define UC_CONFIG_FLASH_OFFSET       4  /* 配置内容偏移 */
#define UC_CHECK_FLASH_OFFSET        (64 * 1024 - 4 - 32) /* 配置校验码偏移 */
#define UC_VALID_FLASH_OFFSET        (64 * 1024 - 4) /* 配置分区有效标志偏移 */
#define UC_VALID_FLAG_VALUE          0x8a7b6c5d /* 配置分区有效标志值 */

#define UC_ITEM_VALID_VALUE  1  /* 配置项有效标志值 */
#define UC_ITEM_MAX_NUM      6  /* 配置项数量 */
#define UC_ITEM_DATA_MAX_LEN 2048 /*  配置内容的最大长度 */

/* 配置项数据字节数(最少为2，第1字节为有效性) */
#define UC_P2P_SIZE          2
#define UC_DDR_ECC_SIZE      2
#define UC_RESERVE0_CONFIG_SIZE    2
#define UC_USER_SHADOW_SIZE  (2048 + 1) /* 1个字节是有效性标志 */
#define UC_PUBLIC_TAG_SIZE  (1024 + 1) /* 1个字节是有效性标志 */
#define UC_AICPU_CONFIG_SIZE    2 /* AICPU与CTRLCPU配比 */
#define UC_SSH_STATUS_SIZE   2 /* SSH状态：ON\OFF */

/* 配置内容中各项偏移 */
#define UC_P2P_OFFSET          0   /* P2P用户配置偏移 */
#define UC_DDR_ECC_OFFSET      (UC_P2P_OFFSET + UC_P2P_SIZE)  /* DDE ECC使能用户配置偏移 */
#define UC_RESERVE0_CONFIG_OFFSET    (UC_DDR_ECC_OFFSET + UC_DDR_ECC_SIZE) /* 保留的配置偏移 */
#define UC_USERSHADOW_OFFSET   (UC_RESERVE0_CONFIG_OFFSET + UC_RESERVE0_CONFIG_SIZE) /* usershadow_0用户配置偏移 */
#define UC_PUBLIC_TAG_OFFSET   (UC_USERSHADOW_OFFSET + UC_USER_SHADOW_SIZE) /* public_key用户配置偏移 */
#define UC_AICPU_CONFIG_OFFSET   (UC_PUBLIC_TAG_OFFSET + UC_PUBLIC_TAG_SIZE) /* aicpu num用户配置偏移 */
#define UC_SSH_STATUS_OFFSET   (UC_AICPU_CONFIG_OFFSET + UC_AICPU_CONFIG_SIZE) /* SSH状态用户配置偏移 */

/* 配置项权限控制 */
#define UC_AUTHORITY_ROOT_WR    0 /* device侧：root 用户可读可写 普通用户无法访问；host侧root无法访问 */
#define UC_AUTHORITY_USER_WR    1 /* device侧：root 用户可读可写 普通用户可读可写；host侧root可读可写 */
#define UC_AUTHORITY_USER_RO    2 /* device侧：root 用户可读可写 普通用户只读    ；host侧root只读 */

#define UC_USER_PARA_ERR    -1
#define UC_USER_PARA_OK     0

/* 用户配置项信息 */
struct user_config_item {
    char *name;         /* 配置项名 */
    int  board_id;      /* 此配置属于board_id的单板，如果board_id为-1，那么所有board_id都使用 */
    int  offset;        /* 配置项数据地址偏移 */
    int  len;           /* 数据字节长度,至少为2，第1字节为配置有效性,1为有效 */
    int  authority_flag;  /* 配置项权限， 定义见UC_AUTHORITY_XXX */
    char *default_data;  /* 配置项默认值,数据为字符串,例如: "0x12,0x34,0x78,0xaa" */
    int (*check_para)(unsigned int buf_size, unsigned char *buf);
};

/*
 * user config配置项
 */
#define DDR_ECC_CONFIG_NAME      "ddr_ecc_enable"
#define DDR_ECC_CONFIG_BUF_SIZE  1
#define DDR_ECC_ENABLE           1
#define DDR_ECC_DISENABLE        0

#define AICPU_CONFIG_NAME   "aicpu_config"
#define AICPU_CONFIG_BUF_SIZE  1
#define AICPU2_CTRLCPU6_CONFIG  0xc0
#define AICPU4_CTRLCPU4_CONFIG  0xf0
#define AICPU6_CTRLCPU2_CONFIG  0xfc

#define SSH_CONFIG_NAME   "ssh_status"
#define SSH_CONFIG_BUF_SIZE  1
#define SSH_ENABLE           1
#define SSH_DISENABLE        0

#define CPU_NUM_CONFIG_NAME         "cpu_num_cfg"
#define P2P_MEM_CONFIG_NAME         "p2p_mem_cfg"
#define CCPU_USR_CERT_CONFIG_NAME   "ccpu_usr_cert_hash"

static int ddr_ecc_enable_check_para(unsigned int buf_size, unsigned char *buf);
static int aicpu_config_check_para(unsigned int buf_size, unsigned char *buf);
static int ssh_status_check_para(unsigned int buf_size, unsigned char *buf);


/*
 * default_data字段必须保证，以"0x"开始，每两个字符表示一个字节
 * 例如: 数值0，1表示为 0x0001
 */

static const struct user_config_item user_cfg_version_1[UC_ITEM_MAX_NUM] = {
    {"ddr_ecc_enable", -1, UC_DDR_ECC_OFFSET, UC_DDR_ECC_SIZE, UC_AUTHORITY_USER_WR, "0x00", ddr_ecc_enable_check_para},
    {"reserve0_config", -1, UC_RESERVE0_CONFIG_OFFSET, UC_RESERVE0_CONFIG_SIZE, UC_AUTHORITY_USER_WR, "0x00", NULL},
    {"usershadow", -1, UC_USERSHADOW_OFFSET, UC_USER_SHADOW_SIZE, UC_AUTHORITY_ROOT_WR, "0x00", NULL},
    {"public_key", -1, UC_PUBLIC_TAG_OFFSET, UC_PUBLIC_TAG_SIZE, UC_AUTHORITY_USER_RO, "0x00", NULL},
    {"aicpu_config", -1, UC_AICPU_CONFIG_OFFSET, UC_AICPU_CONFIG_SIZE, UC_AUTHORITY_USER_WR, "0xf0",
        aicpu_config_check_para},
    {"ssh_status", -1, UC_SSH_STATUS_OFFSET, UC_SSH_STATUS_SIZE, UC_AUTHORITY_USER_WR, "0x00", ssh_status_check_para},
};


/*
 * 校验set user config时输入的参数
 * 例如: ddr_ecc_enable_check_para，输入buf_size仅限为（UC_DDR_ECC_SIZE-1），*buf仅为合法值DDR_ECC_ENABLE或DDR_ECC_DISENABLE
 */

static int ddr_ecc_enable_check_para(unsigned int buf_size, unsigned char *buf)
{
    if (buf == NULL) {
        return UC_USER_PARA_ERR;
    }
    if (buf_size >= UC_DDR_ECC_SIZE) {
        return UC_USER_PARA_ERR;
    }
    if ((buf[0] != DDR_ECC_ENABLE) && (buf[0] != DDR_ECC_DISENABLE)) {
        return UC_USER_PARA_ERR;
    }
    return UC_USER_PARA_OK;
}

static int aicpu_config_check_para(unsigned int buf_size, unsigned char *buf)
{
    if (buf == NULL) {
        return UC_USER_PARA_ERR;
    }
    if (buf_size >= UC_AICPU_CONFIG_SIZE) {
        return UC_USER_PARA_ERR;
    }
    if ((buf[0] == AICPU2_CTRLCPU6_CONFIG) || (buf[0] == AICPU4_CTRLCPU4_CONFIG) ||
        (buf[0] == AICPU6_CTRLCPU2_CONFIG)) {
        return UC_USER_PARA_OK;
    }
    return UC_USER_PARA_ERR;
}

static int ssh_status_check_para(unsigned int buf_size, unsigned char *buf)
{
    if (buf == NULL) {
        return UC_USER_PARA_ERR;
    }
    if (buf_size >= UC_SSH_STATUS_SIZE) {
        return UC_USER_PARA_ERR;
    }
    if ((buf[0] != SSH_ENABLE) && (buf[0] != SSH_DISENABLE)) {
        return UC_USER_PARA_ERR;
    }
    return UC_USER_PARA_OK;
}

#endif
