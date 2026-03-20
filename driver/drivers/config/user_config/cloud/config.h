/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description: This file contains user config related code.
 * Author: huawei
 * Create: 2019-08-06
 */
#ifndef __OPIAI_USER_CONFIG_CLOUD_H
#define __OPIAI_USER_CONFIG_CLOUD_H

#ifndef NULL
#define NULL    (0L)
#endif
/*
 * 用户配置区静态选项
 */
#define CURRENT_USER_CONFIG_VERSION     1                       /* 当前软件能支持的用户配置的版本号 */
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#define UC_FLASH_MAIN_ADDR              0x3E00000                /* 配置所在的flash主区偏移地址 */
#define UC_FLASH_BK_ADDR                0x3E80000                /* 配置所在的flash备区偏移地址 */
#else
#define UC_FLASH_MAIN_ADDR              0x250000                /* 配置所在的flash主区偏移地址 */
#define UC_FLASH_BK_ADDR                0x2D0000                /* 配置所在的flash备区偏移地址 */
#endif
#define UC_FLASH_PARTITION_NUM          16                      /* 用户配置分区数量 */
#define UC_FLASH_PARTITION_MAIN_NUM     8                       /* 用户配置区主区数量 */
#define UC_FLASH_PARTITION_BK_NUM       8                       /* 用户配置区备区数量 */
#define UC_FLASH_PARTITION_SIZE         (64 * 1024)             /* 用户配置分区大小(字节数) */

#define UC_VERION_FLASH_OFFSET          0                       /* 配置版本号偏移 */
#define UC_CONFIG_FLASH_OFFSET          4                       /* 配置内容偏移 */
#define UC_CHECK_FLASH_OFFSET           (64 * 1024 - 4 - 32)    /* 配置校验码偏移 */
#define UC_VALID_FLASH_OFFSET           (64 * 1024 - 4)         /* 配置分区有效标志偏移 */
#define UC_VALID_FLAG_VALUE             0x8a7b6c5d              /* 配置分区有效标志值 */

#define UC_ITEM_VALID_VALUE             1                       /* 配置项有效标志值 */
#define UC_ITEM_MAX_NUM                 2                       /* 配置项数量 */
#define UC_ITEM_DATA_MAX_LEN            0x8000                  /* 配置内容的最大长度,32KB */

/* 配置项数据字节数(最少为2，第1字节为有效性) */
#define UC_P2P_SIZE                     2
#define UC_DDR_ECC_SIZE                 2
#define UC_LOG_LEVEL_SIZE               2
#define UC_USER_SHADOW_SIZE             (2048 + 1)              /* 1个字节是有效性标志 */
#define UC_PUBLIC_KEY_SIZE              (1024 + 1)              /* 1个字节是有效性标志 */
#define UC_MAC_INFO_SIZE                16                      /* 16个字节MAC信息存储 */
#define UC_PSS_CFG_SIZE                 2                       /* PSS/PKCS签名配置大小，已删除 */

/* 配置内容中各项偏移 */
#define UC_P2P_OFFSET          0                                /* P2P用户配置偏移 */
#define UC_DDR_ECC_OFFSET      (UC_P2P_OFFSET + UC_P2P_SIZE)    /* DDE ECC使能用户配置偏移 */
#define UC_LOG_LEVEL_OFFSET    (UC_DDR_ECC_OFFSET + UC_DDR_ECC_SIZE) /* LOG级别用户配置偏移 */
#define UC_USERSHADOW_OFFSET   (UC_LOG_LEVEL_OFFSET + UC_LOG_LEVEL_SIZE) /* usershadow_0用户配置偏移 */
#define UC_PUBLIC_KEY_OFFSET   (UC_USERSHADOW_OFFSET + UC_USER_SHADOW_SIZE) /* usershadow_0用户配置偏移 */
#define UC_PSS_CFG_OFFSET      (UC_PUBLIC_KEY_OFFSET + UC_PUBLIC_KEY_SIZE)  /* PSS/PKCS 用户配置偏移，已删除 */
#define UC_MAC_INFO_OFFSET     0  /* MAC INFO用户配置偏移 */

/* 配置项权限控制 */
#define UC_AUTHORITY_ROOT_WR	(0) /* root 用户可读可写 普通用户无法访问 */
#define UC_AUTHORITY_USER_WR	(1) /* root 用户可读可写 普通用户可读可写 */
#define UC_AUTHORITY_USER_RO	(2) /* root 用户可读可写 普通用户只读 */
#define UC_USER_PARA_ERR    -1
#define UC_USER_PARA_OK     0

/* 用户配置项信息 */
struct user_config_item {
    char *name;             /* 配置项名 */
    int  board_id;          /* 此配置属于board_id的单板，如果board_id为-1，那么所有board_id都使用 */
    int  offset;            /* 配置项数据地址偏移 */
    int  len;               /* 数据字节长度,至少为2，第1字节为配置有效性,1为有效 */
    int  authority_flag;    /* 配置项权限， 定义见UC_AUTHORITY_XXX */
    char *default_data;     /* 配置项默认值,数据为字符串,例如: "0x12,0x34,0x78,0xaa" */
    int (*check_para)(unsigned int buf_size, unsigned char *buf);
};

/*
 * user config配置项
 */
#define DDR_ECC_CONFIG_NAME      "ddr_ecc_enable"
#define DDR_ECC_CONFIG_BUF_SIZE  1
#define DDR_ECC_ENABLE           1
#define DDR_ECC_DISENABLE        0

#define SSH_CONFIG_NAME             "ssh_status"
#define CPU_NUM_CONFIG_NAME         "cpu_num_cfg"
#define P2P_MEM_CONFIG_NAME         "p2p_mem_cfg"
#define CCPU_USR_CERT_CONFIG_NAME   "ccpu_usr_cert_hash"

/*
 * default_data字段必须保证，以"0x"开始，每两个字符表示一个字节
 * 例如: 数值0，1表示为 0x0001
 */
static const struct user_config_item user_cfg_version_1[UC_ITEM_MAX_NUM] = {
    {"usershadow", -1, UC_USERSHADOW_OFFSET, UC_USER_SHADOW_SIZE, UC_AUTHORITY_ROOT_WR, "0x00", NULL},
    {"mac_info", -1, UC_MAC_INFO_OFFSET, UC_MAC_INFO_SIZE, UC_AUTHORITY_USER_WR, "0x00", NULL},
};

#define UC_ITEM_INDEX_MAC_INFO             1
#define UC_ITEM_INDEX_USER_SHADOW          0

/*
 * 用户配置区动态选项
 */
/* 动态配置头地址 */
#define UC_DYNAMIC_CFG_HEAD_OFFSET          0x2000  // 8KB
#define UC_CFG_HEAD_LEN_OFFSET              UC_DYNAMIC_CFG_HEAD_OFFSET
#define UC_CFG_HEAD_BYTES                   4       // 4Bytes
#define UC_CFG_HEAD_ITEM_START              (UC_DYNAMIC_CFG_HEAD_OFFSET + UC_CFG_HEAD_BYTES)
#define UC_CFG_NAME_LEN_MAX                 32
#define UC_SHA256_DIGEST                    32

typedef struct _uc_cfg_head {
    unsigned char item_name[UC_CFG_NAME_LEN_MAX];
    unsigned int blk_offset;
    unsigned int item_offset;
    unsigned int item_bytes;
    unsigned int authority_flg;
    unsigned int valid_flg;
} uc_cfg_head_t;

typedef struct _uc_blk_used_info {
    unsigned char index;
    unsigned char used;
    unsigned short used_len;
} uc_blk_used_info_t;

#define UC_BLK_INFO_OFFSET           (UC_CHECK_FLASH_OFFSET - sizeof(uc_blk_used_info_t)*UC_FLASH_PARTITION_MAIN_NUM)
#define UC_BLK_INFO_BYTES	         (sizeof(uc_blk_used_info_t)*UC_FLASH_PARTITION_MAIN_NUM)
#define UC_ITEM_CONTENT_BLK_START    2
#define UC_BLK_INDEX_MAC_INFO        1
#define UC_ITEM_CONTENT_BLK_OFFSET   0   // first block offset

#endif
