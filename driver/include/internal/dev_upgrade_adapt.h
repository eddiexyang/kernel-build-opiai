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

#ifndef _DEV_UPGRADE_ADAPT_H_
#define _DEV_UPGRADE_ADAPT_H_

#include <linux/errno.h>
#include "dev_upgrade_def.h"

#ifdef STATIC_SKIP
#define STATIC
#else
#define STATIC static
#endif

#define FLASH_BLKS_ONCE 16
#define LINE_FEED_PRINT 16

#define DEV_FLASH_OFFSET_SHA256 0x2c
#define DEV_FLASH_OFFSET_VER    0x480

#define PART0_DEV  "/dev/mtd0"
#define PART1_DEV  "/dev/mtd1"
#define PART2_DEV  "/dev/mtd2"
#define PART3_DEV  "/dev/mtd3"
#define PART4_DEV  "/dev/mtd4"
#define PART5_DEV  "/dev/mtd5"
#define PART6_DEV  "/dev/mtd6"
#define PART7_DEV  "/dev/mtd7"
#define PART8_DEV  "/dev/mtd8"
#define PART9_DEV  "/dev/mtd9"
#define PART10_DEV "/dev/mtd10"

#define PART_NAME_BOOTROM   "Bootrom_1" /* 分区名相对于实际分区名加上_1，目的是不想被查找到 */
#define PART_NAME_NVE       "Config_info"
#define PART_NAME_NVE_P0    "Config_info_P0_N"
#define PART_NAME_NVE_P1    "Config_info_P1_N"
#define PART_NAME_NVE_P2    "Config_info_P2_N"
#define PART_NAME_NVE_P3    "Config_info_P3_N"
#define PART_NAME_XLOADER_1 "Xloader_1"
#define PART_NAME_XLOADER_2 "Xloader_2"
#define PART_NAME_UEFI      "BIOS"
#define PART_NAME_UEFI_1_P0 "BIOS_1_P0_N"
#define PART_NAME_UEFI_2_P0 "BIOS_2_P0_N"
#define PART_NAME_UEFI_1_P1 "BIOS_1_P1_N"
#define PART_NAME_UEFI_2_P1 "BIOS_2_P1_N"
#define PART_NAME_UEFI_1_P2 "BIOS_1_P2_N"
#define PART_NAME_UEFI_2_P2 "BIOS_2_P2_N"
#define PART_NAME_UEFI_1_P3 "BIOS_1_P3_N"
#define PART_NAME_UEFI_2_P3 "BIOS_2_P3_N"
#define PART_NAME_M3FW      "M3_FW"
#define PART_NAME_TEE       "TEE"
#define PART_NAME_TEE_1_P0  "TEE_1_P0_N"
#define PART_NAME_TEE_2_P0  "TEE_2_P0_N"
#define PART_NAME_TEE_1_P1  "TEE_1_P1_N"
#define PART_NAME_TEE_2_P1  "TEE_2_P1_N"
#define PART_NAME_TEE_1_P2  "TEE_1_P2_N"
#define PART_NAME_TEE_2_P2  "TEE_2_P2_N"
#define PART_NAME_TEE_1_P3  "TEE_1_P3_N"
#define PART_NAME_TEE_2_P3  "TEE_2_P3_N"
#define PART_NAME_KERNEL    "OS"
#define PART_NAME_DTB       "OS"
#define PART_NAME_ROOTFS    "OS"
#define PART_NAME_IMU_1_P0  "IMU_1_P0_N"
#define PART_NAME_IMU_2_P0  "IMU_2_P0_N"
#define PART_NAME_IMU_1_P1  "IMU_1_P1_N"
#define PART_NAME_IMU_2_P1  "IMU_2_P1_N"
#define PART_NAME_IMU_1_P2  "IMU_1_P2_N"
#define PART_NAME_IMU_2_P2  "IMU_2_P2_N"
#define PART_NAME_IMU_1_P3  "IMU_1_P3_N"
#define PART_NAME_IMU_2_P3  "IMU_2_P3_N"
#define PART_NAME_IMP_1_P0  "IMP_1_P0_N"
#define PART_NAME_IMP_2_P0  "IMP_2_P0_N"
#define PART_NAME_IMP_1_P1  "IMP_1_P1_N"
#define PART_NAME_IMP_2_P1  "IMP_2_P1_N"
#define PART_NAME_IMP_1_P2  "IMP_1_P2_N"
#define PART_NAME_IMP_2_P2  "IMP_2_P2_N"
#define PART_NAME_IMP_1_P3  "IMP_1_P3_N"
#define PART_NAME_IMP_2_P3  "IMP_2_P3_N"
#define PART_NAME_RESERVE_1 "Reserve_1"
#define PART_NAME_RESERVE_2 "Reserve_2"

#define PART_NAME_HBOOT1_A_0      "Hboot1_a_0"
#define PART_NAME_HBOOT1_A_1      "Hboot1_a_1"
#define PART_NAME_HBOOT1_B_0      "Hboot1_b_0"
#define PART_NAME_HBOOT1_B_1      "Hboot1_b_1"
#define PART_NAME_HBOOT2_0        "Hboot2_0"
#define PART_NAME_HBOOT2_1        "Hboot2_1"
#define PART_NAME_DDR_0           "DDR_0"
#define PART_NAME_DDR_1           "DDR_1"
#define PART_NAME_LP_0            "LP_0"
#define PART_NAME_LP_1            "LP_1"
#define PART_NAME_HSM_0           "HSM_0"
#define PART_NAME_HSM_1           "HSM_1"
#define PART_NAME_SAFETY_ISLAND_0 "SAFETY_ISLAND_0"
#define PART_NAME_SAFETY_ISLAND_1 "SAFETY_ISLAND_1"
#define PART_NAME_HLINK_0         "Hlink_0"
#define PART_NAME_HLINK_1         "Hlink_1"
#define PART_NAME_SYS_BASE_CONFIG_0         "SYSBASE_CFG_0"
#define PART_NAME_SYS_BASE_CONFIG_1         "SYSBASE_CFG_1"
#define PART_NAME_USER_BASE_CONFIG_0        "USERBASE_CFG_0"
#define PART_NAME_USER_BASE_CONFIG_1        "USERBASE_CFG_1"
#define PART_NAME_LOGIC_BIST_0    "LOGIC_BIST_0"
#define PART_NAME_LOGIC_BIST_1    "LOGIC_BIST_1"
#define PART_NAME_ATF_0           "ATF_0"
#define PART_NAME_ATF_1           "ATF_1"

#define PART_NAME_HILINK60_0 "Hilink60_0"
#define PART_NAME_HILINK60_1 "Hilink60_1"

#define PART_NAME_USER_CONFIG        PART_NAME_RESERVE_1
#define PART_NAME_USER_CONFIG_SIZE   (64 * 1024)
#define PART_NAME_USER_CONFIG_OFFSET 0

#define FLASH_BLOCK_SIZE (64 * 1024)

#define SEC_BOOT_HEAD_SIZE (8 * 1024)
#define OS_KERNEL_SIZE     (11 * 1024 * 1024)
#define OS_DTB_SIZE        (1 * 1024 * 1024)

#define DTB_VERSION_OFFSET    (SEC_BOOT_HEAD_SIZE + SEC_BOOT_HEAD_SIZE + OS_KERNEL_SIZE)
#define ROOTFS_VERSION_OFFSET (DTB_VERSION_OFFSET + (SEC_BOOT_HEAD_SIZE + SEC_BOOT_HEAD_SIZE + OS_DTB_SIZE))

#if (defined CFG_SOC_PLATFORM_MINIV2) || (defined CFG_SOC_PLATFORM_MDC_V51)
#define COMM_VERSION_OFFSET (0x2000 + 0x18)
#else
#define COMM_VERSION_OFFSET 0x480
#endif

#define COMM_VERSION_LENGTH 16

#define VER_STRING_MAX_LEN 64

#define BOOTROM_HEAD_OFFSET 0xc000

#define NVE_VERSION_OFFSET 64

#define HASH_OFFSET  0x2c
#define HASH_LENGTH  32
#define CHECK_FAILED 1
#define UPGRADE_HEX  16

#ifdef CFG_SOC_PLATFORM_CLOUD
#define SEC_HEAD_SIZE 0x10000
#else
#if (defined CFG_SOC_PLATFORM_MINIV2) || (defined CFG_SOC_PLATFORM_MDC_V51)
#define SEC_HEAD_SIZE (0x2000 + 0x100)
#else
#define SEC_HEAD_SIZE (0x2000 + 0x2000)
#endif
#endif

#define IMAGE_CODE_OFFSET     (SEC_HEAD_SIZE)
#define IMAGE_CODE_LEN_OFFSET 0x460
#define IMAGE_CODE_LEN        0x4

#define CMS_MAGIC_FLAG_OFFSET 0x490
#define CMS_MAGIC_FLAG_SIZE   0x8
#define CMS_MAGIC_FLAG_VALUE  0xABCD1234AA55AA55
#define FILE_TOTAL_LEN_OFFSET (CMS_MAGIC_FLAG_OFFSET + CMS_MAGIC_FLAG_SIZE)
#define FILE_TOTAL_LEN_SIZE   0x4

#define CMS_RESERVED_PAD_SIZE 0x10

#define CMS_TOTAL_SIZE        0x2000
#define BOOTUP_INI_TOTAL_SIZE 0x800
#define CRL_TOTAL_SIZE        0x4000
#define CRL_HEAD_SIZE         0x10

#define ALL_SEC_TAIL_TOTAL_SIZE (CMS_RESERVED_PAD_SIZE + CMS_TOTAL_SIZE + BOOTUP_INI_TOTAL_SIZE + CRL_HEAD_SIZE)

#define FLASH_ADDR_OFFSET_BOOTROM_0 0x00
#ifdef CFG_SOC_PLATFORM_CLOUD
#define FLASH_SIZE_BOOTROM_0 0x40000
#else
#define FLASH_SIZE_BOOTROM_0 0x10000
#endif

#define FLASH_ADDR_OFFSET_NVE_0 0x00
#define FLASH_SIZE_NVE_0        0x40000

#ifdef CFG_SOC_PLATFORM_CLOUD
#define FLASH_ADDR_OFFSET_NVE_1 0x40000
#define FLASH_SIZE_NVE_1        0x40000
#endif
#define FLASH_ADDR_OFFSET_XLOADER_0 0x00
#define FLASH_SIZE_XLOADER_0        0x40000

#define FLASH_ADDR_OFFSET_XLOADER_1 0x00
#define FLASH_SIZE_XLOADER_1        0x40000

#define FLASH_ADDR_OFFSET_M3_FW_0 0x00
#define FLASH_SIZE_M3_FW_0        0x40000

#define FLASH_ADDR_OFFSET_UEFI_0 0x00
#ifdef CFG_SOC_PLATFORM_CLOUD
#define FLASH_SIZE_UEFI_0 0x440000
#else
#define FLASH_SIZE_UEFI_0 0x400000
#endif

#ifdef CFG_SOC_PLATFORM_CLOUD
#define FLASH_ADDR_OFFSET_UEFI_1 0x00
#define FLASH_SIZE_UEFI_1        0x440000
#endif

#define FLASH_ADDR_OFFSET_TEE_0 0x00
#define FLASH_SIZE_TEE_0        0x80000

#ifdef CFG_SOC_PLATFORM_CLOUD
#define FLASH_ADDR_OFFSET_TEE_1 0x00
#define FLASH_SIZE_TEE_1        0x80000
#endif

#define FLASH_ADDR_OFFSET_KERNEL_0 0x00
#define FLASH_SIZE_KERNEL_0        0xb00000

#define FLASH_ADDR_OFFSET_DTB_0 0xb00000
#define FLASH_SIZE_DTB_0        0x100000

#define FLASH_ADDR_OFFSET_ROOTFS_0 0xc00000
#define FLASH_SIZE_ROOTFS_0        0x2800000

#define FLASH_ADDR_OFFSET_IMP_0 0x00
#define FLASH_SIZE_IMP_0        0x1e0000

#define FLASH_ADDR_OFFSET_IMP_1 0x00
#define FLASH_SIZE_IMP_1        0x1e0000

#define FLASH_ADDR_OFFSET_IMU_0 0x00
#define FLASH_SIZE_IMU_0        0x200000

#define FLASH_ADDR_OFFSET_IMU_1 0x00
#define FLASH_SIZE_IMU_1        0x200000

#define FLASH_ADDR_OFFSET_FLASHBOOT_0           0x00
#define FLASH_SIZE_FLASHBOOT_0                  0x40000
#define FLASH_ADDR_OFFSET_HBOOT1_A_0            0x00
#ifdef CFG_SOC_PLATFORM_MDC_V11
#define FLASH_SIZE_HBOOT1_A_0                   0x20000
#define FLASH_SIZE_HBOOT1_A_1                   0x20000
#define FLASH_SIZE_DDR_0                        0x50000
#define FLASH_SIZE_DDR_1                        0x50000
#define FLASH_SIZE_HSM_0                        0x80000
#define FLASH_SIZE_HSM_1                        0x80000
#define FLASH_SIZE_SYS_BASE_CONFIG_0            0x60000
#define FLASH_SIZE_SYS_BASE_CONFIG_1            0x60000
#else
#define FLASH_SIZE_HBOOT1_A_0                   0x10000
#define FLASH_SIZE_HBOOT1_A_1                   0x40000
#define FLASH_SIZE_DDR_0                        0x40000
#define FLASH_SIZE_DDR_1                        0x40000
#define FLASH_SIZE_HSM_0                        0x40000
#define FLASH_SIZE_HSM_1                        0x40000
#define FLASH_SIZE_SYS_BASE_CONFIG_0            0x40000
#define FLASH_SIZE_SYS_BASE_CONFIG_1            0x40000
#endif
#define FLASH_ADDR_OFFSET_HBOOT1_A_1            0x00
#define FLASH_ADDR_OFFSET_HBOOT1_B_0            0x00
#define FLASH_SIZE_HBOOT1_B_0                   0x40000
#define FLASH_ADDR_OFFSET_HBOOT1_B_1            0x00
#define FLASH_SIZE_HBOOT1_B_1                   0x40000
#define FLASH_ADDR_OFFSET_HBOOT2_0              0x00
#define FLASH_SIZE_HBOOT2_0                     0x300000
#define FLASH_ADDR_OFFSET_HBOOT2_1              0x00
#define FLASH_SIZE_HBOOT2_1                     0x300000
#define FLASH_ADDR_OFFSET_DDR_0                 0x00
#define FLASH_ADDR_OFFSET_DDR_1                 0x00
#define FLASH_ADDR_OFFSET_LP_0                  0x00
#define FLASH_SIZE_LP_0                         0x40000
#define FLASH_ADDR_OFFSET_LP_1                  0x00
#define FLASH_SIZE_LP_1                         0x40000
#define FLASH_ADDR_OFFSET_HSM_0                 0x00
#define FLASH_ADDR_OFFSET_HSM_1                 0x00
#define FLASH_ADDR_OFFSET_SAFETY_ISLAND_0       0x00
#define FLASH_SIZE_SAFETY_ISLAND_0              0x40000
#define FLASH_ADDR_OFFSET_SAFETY_ISLAND_1       0x00
#define FLASH_SIZE_SAFETY_ISLAND_1              0x40000
#define FLASH_ADDR_OFFSET_HLINK                 0x00
#define FLASH_SIZE_HLINK                        0x30000
#define FLASH_ADDR_OFFSET_SYS_BASE_CONFIG_0     0x00
#define FLASH_ADDR_OFFSET_SYS_BASE_CONFIG_1     0x00
#define FLASH_ADDR_OFFSET_USER_BASE_CONFIG_0    0x00
#define FLASH_SIZE_USER_BASE_CONFIG_0           0x40000
#define FLASH_ADDR_OFFSET_USER_BASE_CONFIG_1    0x00
#define FLASH_SIZE_USER_BASE_CONFIG_1           0x40000
#define FLASH_ADDR_OFFSET_LOGIC_BIST_0          0x00
#define FLASH_SIZE_LOGIC_BIST_0                 0x40000
#define FLASH_ADDR_OFFSET_LOGIC_BIST_1          0x00
#define FLASH_SIZE_LOGIC_BIST_1                 0x40000
#define FLASH_ADDR_OFFSET_ATF_0                 0x00
#define FLASH_SIZE_ATF_0                        0x200000
#define FLASH_ADDR_OFFSET_ATF_1                 0x00
#define FLASH_SIZE_ATF_1                        0x200000

#define FLASH_ADDR_OFFSET_HILINK32              0xE0000
#define FLASH_SIZE_HILINK32                     0x40000
#define FLASH_ADDR_OFFSET_HILINK60              0x120000
#define FLASH_SIZE_HILINK60                     0x40000

#define FLASH_SIZE_MAX (64 * 1024 * 1024)
#define AICPU_SIZE_MAX (512 * 1024 * 1024)

#define LOG_LEVEL_PRINTF       8
#define NVE_WRITE_DATA_ADDRESS 0

// name VERSION reserve numofnvbins sha256support age
#define NVE_SHA256_OFFSET (12 + 4 + 4 + 4 + 4 + 4)

// name VERSION reserve numofnvbins sha256support age SHA256
#define NVE_RESERVE_HEAD_OFFSET (12 + 4 + 4 + 4 + 4 + 4 + 32)

// nv_number nv_name valid_size board_id
#define NVE_CRC_START_ADDRESS (4 + 12 + 4 + 4)

#define NVE_CRC_LENGTH 32

// name VERSION reserve numofnvbins
#define NVE_CRC_SUPPORT (12 + 4 + 4 + 4)

// nv_number nv_name valid_size board_id sha256
#define NVE_PROTO_DATA_ADDRESS (4 + 12 + 4 + 4 + 32)
#define NVE_PROTO_DATA_LENGTH  104

// nv_number nv_name valid_size board_id sha256 proto_data
#define NVE_BLOCK_SIZE (4 + 12 + 4 + 4 + 32 + 104)

#define NVE_VERSION_START_ADDRESS 12

// name VERSION reserve
#define NVE_VALID_ITEMS_START_ADDRESS (12 + 4)
#define NVE_VALID_ITEMS_LENGTH        4

// name VERSION reserve numofnvbins sha256_support age SHA256 reserve
#define NVE_HEAD_LENGTH (12 + 4 + 4 + 4 + 4 + 4 + 32 + 64)

// boardid offset size
#define NVE_BIN_ENTRY_LEN    (4 + 4 + 4)
#define NVE_BIN_ENTRY_OFFSET 4
#define NVE_BIN_SIZE_OFFSET  (4 + 4)
#define NVE_AGE_OFFSET       (12 + 4 + 4 + 4 + 4)
#define NVAE_AGE_LEN         4

// nv_number nv_name valid_size board_id sha256 proto_data
#define NVE_BIN_FILE_LEN (4 + 12 + 4 + 4 + 32 + 104)

#define NVE_MAX_VALID_ITEM_NUM ((256 * 1024 - (NVE_HEAD_LENGTH) - (NVE_BIN_ENTRY_LEN)) / (NVE_BIN_FILE_LEN))

#define DEV_UPDATE_AREA_MAIN    0x0
#define DEV_UPDATE_AREA_STANDBY 0x1

#define COMPONENT_LIST_MAX 32
#define MTD_NAME_MAX       32

#define FLASH_AREA_0 0
#define FLASH_AREA_1 1

#define SINGLE_AREA  1
#define DOUBLE_AREA  2

#define UPGRADE_SCHEDULE_PERCENTAGE_100 100

#define SIZE_OF_64K      0x10000
#define SYSCTRL_REG_BASE 0x1100C0000
#define SC_SOFT_POR_RSV3 0xFF2C
#define SC_BAK_DATA14    0x3448

#define CPU_ISRAM_BASE    0xA0D10000
#define ISRAM_IMU_OFFSET  0x0280
#define ISRAM_IMP_OFFSET  0x0284
#define ISRAM_UEFI_OFFSET 0x0288
#define ISRAM_ACPI_OFFSET 0x028C
#define ISRAM_NVE_OFFSET  0x0290
#define CHIP_REG_OFFSET   0x200000000000

#define DEV_UPDATE_XLOADER_AREA0 0x0
#define DEV_UPDATE_XLOADER_AREA1 0x1
#define DEV_UPDATE_XLOADER_BOTH  0x2

#define DEV_GET_CURR_BOOT_AREA  0
#define DEV_CLEAR_BOOT_COUNT    1
#define DEV_GET_BOOT_FAIL_COUNT 2

#define DEV_FLASH_DOUBLE_AREAS  2

#define CMS_NAME_LEN 12
#define EFI_TYPE_LEN 16

#define COVER_FILE_WITHOUT_HEAD (1<<0)
#define HBOOT1_A_MASK       0x100
#define HILINK_MASK         0x200
#define HBOOT1_B_MASK       0x400
#define HBOOT2_MASK         0x800
#define DDR_MASK            0x1000
#define LP_MASK             0x2000
#define HSM_MASK            0x4000
#define SAFETY_ISLAND_MASK  0x8000
#define SYS_BASE_CONFIG_MASK  0x10000
#define HILINK2_MASK        0x40000
#define ATF_MASK     0x80000
#define LOGIC_BIST_MASK     0x100000

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef enum {
    BOOT_FROM_ONCHIPROM,
    BOOT_FROM_AREA0,
    BOOT_FROM_AREA1,
    BOOT_FROM_BOTTOM
} BOOT_MODE;

typedef enum {
    STORE_IN_FLASH,
    STORE_IN_FILESYSTEM
} STORE_POS;

typedef enum {
    CHECK_ALL_THE_SAME,
    CHECK_IMAGE_BAD,
    CHECK_HASH_DIFF,
    CHECK_VER_DIFF,
    CHECK_IMAGE_OK,
    CHECK_UNKNOWN
} CHECK_RES;

typedef struct componet_boot_area_info_st {
        unsigned int component_type;
        unsigned int area_mask;
} componet_boot_area_info;

typedef struct tag_component_area_info_st {
    unsigned int component_type; /* 组件id */
    unsigned int area;           /* flash区域标识，如0区、1区、2区... */
    char dev_part[MTD_NAME_MAX]; /* 组件对应的分区 */
    const char *part_name;       /* 分区名称 */
    unsigned int offset;         /* 分区内偏移地址 */
    unsigned int size;           /* 占用空间大小 */
    unsigned int store_in;       /* 存储介质 */
} component_area_info;

#pragma pack(1)
typedef struct tag_cms_struct_info_st {
    char name[CMS_NAME_LEN];
    unsigned int len;
    unsigned char buf[1];
} cms_struct_info;
#pragma pack()

typedef struct tag_dev_component_list_ctrl_st {
    int dev_id;
    unsigned int bitmap;
    unsigned int count;
    component_area_info components[COMPONENT_LIST_MAX];
} dev_component_list_ctrl;

typedef struct {
    u8 b[EFI_TYPE_LEN];
} upgrade_efi_guid_t;

typedef struct tag_upg_comm_para_st {
    int dev_id;
    unsigned int type;
    unsigned int area;
    unsigned int file_size;
    unsigned char *file_content;
} upg_comm_para;

typedef struct tag_bios_info_st_st {
    long base_addr;
    unsigned int boot_area_ofs;
    unsigned int boot_cnt_ofs;
    unsigned int boot_area_shift;
    unsigned int boot_area_mask;
    unsigned int boot_cnt_mask;
} boot_area_info;

#define EFUSE_NS_MAGIC          0x1a4a5252

#if ((!defined CFG_SOC_PLATFORM_MINIV2) && (!defined CFG_SOC_PLATFORM_MDC_V51) && (!defined CFG_SOC_PLATFORM_MINIV3))
#define EFUSE_CTRL_BASE         0x2002C0000
#define HW_HASH_OFFSET          (EFUSE_CTRL_BASE + 0xE408)
#define HW_CATEGORY_OFFSET      (EFUSE_CTRL_BASE + 0xE428)
#define SUBKEYID_MASK1_OFFSET   (EFUSE_CTRL_BASE + 0xE490)
#define EFUSE_NS_FORBID_OFFSET  (EFUSE_CTRL_BASE + 0xE4A0)
#define SMP_REG_BASE_OFFSET     0x200000000000
#endif

#define EFUSE_REG_READ_LEN      4
#define EFUSE_HASH_LEN          32
#define EFUSE_HASH_WORDS        8
#define INVALID_NVCNT 0xFFFFFFFF

typedef struct {
    uint32_t need_check;
    uint32_t subkey_category;
    uint32_t subkey_id;
    uint32_t nv_cnt;
    uint8_t  root_pk_hash[EFUSE_HASH_LEN];
    uint32_t pss_only; /* disable rsa pkcs padding mode */
} IMG_CHECK_INFO;

#define XLOADER_FILE_TYPE_CHECK_OFFSET 8
#define XLOADER_FILE_TYPE_CHECK_LEN    8
#define XLOADER_FILE_TYPE_CHECK_FLAG   "XLOADER!"

#define UPGRADE_EFI_GUID(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7)                                  \
    ((upgrade_efi_guid_t) { { (a)&0xff, ((a) >> 8) & 0xff, ((a) >> 16) & 0xff, ((a) >> 24) & 0xff, \
        (b)&0xff, ((b) >> 8) & 0xff,                                                       \
        (c)&0xff, ((c) >> 8) & 0xff,                                                       \
        (d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7) } })

#define UEFI_FILE_TYPE_CHECK_OFFSET_EUID 0X10
#define UEFI_FILE_TYPE_CHECK_EUID_LEN    (sizeof(upgrade_efi_guid_t))
#define UEFI_FILE_TYPE_CHECK_FLAG_EUID2  UPGRADE_EFI_GUID(0x8c8ce578, 0x8a3d, 0x4f1c, 0x99, 0x35, \
                                            0x89, 0x61, 0x85, 0xc3, 0x2d, 0xd3)
#define UEFI_FILE_TYPE_CHECK_FLAG_EUID3  UPGRADE_EFI_GUID(0x5473c07a, 0x3dcb, 0x4dca, 0xbd, 0x6f, \
                                            0x1e, 0x96, 0x89, 0xe7, 0x34, 0x9a)

#define UEFI_FILE_TYPE_CHECK_OFFSET_SIGNATURE (UEFI_FILE_TYPE_CHECK_OFFSET_EUID + UEFI_FILE_TYPE_CHECK_EUID_LEN + 8)
#define UEFI_FILE_TYPE_CHECK_SIGNATURE_LEN    4
#define UEFI_FILE_TYPE_CHECK_FLAG_SIGNATURE   "_FVH"

#ifdef CFG_SOC_PLATFORM_CLOUD
#define NVE_FILE_TYPE_CHECK_FLAG   "Davinci-NVE"
#define NVE_FILE_TYPE_CHECK_LEN    11
#define IMU_FILE_TYPE_CHECK_FLAG   "imu_image"
#define IMU_FILE_TYPE_CHECK_OFFSET 0x70000
#define IMU_FILE_TYPE_CHECK_LEN    9
#define IMP_FILE_TYPE_CHECK_FLAG   "10.01.01T08"
#define IMP_FILE_TYPE_CHECK_OFFSET 0x4FFE0
#define IMP_FILE_TYPE_CHECK_LEN    11
#endif

#define NVE_AREA_CONT   4
#define NVE_NAME_LEN    12
#define NVE_NAME_OFFSET 0
#define NVE_VER_OFFSET  NVE_NAME_LEN
#define NVE_AREA_LEN    (256 * 1024)
#define NVE_VER_LEN     4
#define NVE_HEADER_NAME "Davinci-NVE"
#define NVE_BNI_NUM_MAX 0x1000
typedef struct NVE_PARTION_HEADER_TAG {
    unsigned char name[NVE_NAME_LEN + 1];
    unsigned char nve_version[NVE_VER_LEN];
    unsigned int nvbin_number;
    unsigned int nve_crc_suppprt;
    unsigned int nve_age;
    unsigned int belonged_are;
} NVE_PARTION_HEADER;

#define DEV_UPGRADE_VFREE(p) do {                     \
        if ((p) != NULL) {   \
            vfree(p);        \
            p = NULL;        \
        }                    \
    } while (0)

#define DEV_UPGRADE_KFREE(p) \
    do {                     \
        kfree(p);            \
        p = NULL;            \
    } while (0)

typedef void (*dev_upgrade_scheule_handle)(int dev_id, unsigned int component_type,
                                           unsigned int total_size, unsigned int finish_size);

int dev_upgrade_component_version_get(upg_comm_para *comm_para, unsigned int is_memory,
    unsigned char *o_buf, unsigned int o_len);
int dev_upgrade_component_space_get(int dev_id, unsigned int component_type, unsigned int *space_size);
int dev_upgrade_get_image_size(upg_comm_para *comm_para, unsigned int *length);
int dev_upgrade_nve_sha256_check(unsigned char *buff_addr, unsigned int valid_items, unsigned int len);

int dev_upgrade_component_image_copy(int dev_id, unsigned int component_type, unsigned int dest_area,
                                     unsigned int src_area);
int dev_upgrade_component_info_match(int dev_id, unsigned int *match_nums);
int dev_upgrade_get_component_bitmap(int dev_id, unsigned int *bitmap);
int dev_upgrade_sec_verification(int dev_id, unsigned int component_type, unsigned char *file_content,
                                 unsigned int file_size);
int dev_upgrade_sec_rim_data(int dev_id, const unsigned char *rim_buf, unsigned int rim_size);
int dev_upgrade_update_cnt(int dev_id);
int dev_upgrade_file_content_check(const char *file_name, const char *buf, unsigned int size, unsigned int *check_res);
int dev_upgrade_write_fs_file(const char *file_name, const char *buf, unsigned int size, int mode);
int dev_upgrade_read_fs_file_fp(struct file *filp, loff_t offset, char *buf, unsigned int size);
int dev_upgrade_read_fs_file(const char *file_name, loff_t offset, char *buf, unsigned int size);
int dev_upgrade_get_fs_file_size_fp(struct file *filp, unsigned int *file_size);
int dev_upgrade_get_fs_file_size(const char *file_name, unsigned int *file_size);
int dev_upgrade_verify_image(int dev_id, const char *file_path, int mode);
int dev_upgrade_component_boot_area_op(int dev_id, unsigned int component_type, unsigned int op_flag,
    unsigned int *op_area);
int dev_upgrade_get_check_info(int dev_id, IMG_CHECK_INFO *info);
int init_get_nvcnt(void);

#define force_inline static inline __attribute__((always_inline))
#if (!defined SUPPORT_TEE_SECURE_UPGRADE)
int dev_upgrade_read_flash_image(upg_comm_para *comm_para, unsigned char *buff, unsigned int length);
int dev_upgrade_save_to_flash(upg_comm_para *comm_para, dev_upgrade_scheule_handle update_progress);
#else
/**
 * In the secure upgrade scenario, the following functions directly call the API of
 * the flash memory to read and write the flash memory. These functions are not used
 * in the DC and MDC scenarios, but are still used in the cloud and mini scenarios.
 *
 * These definition is used to avoid code isolation due to a large number of
 * macro definitions in the original code.
 */
force_inline int dev_upgrade_save_to_flash(upg_comm_para *comm_para,
    dev_upgrade_scheule_handle update_progress)
{
    dev_upgrade_err("%s: function not supported in TEE upgrade mode.\n", __func__);
    return -EACCES;
}

force_inline int dev_upgrade_read_flash_image(upg_comm_para *comm_para,
                                              unsigned char *buff, unsigned int length)
{
    dev_upgrade_err("%s: function not supported in TEE upgrade mode.\n", __func__);
    return -EACCES;
}

int dev_upgrade_ufs_valid_chk(unsigned int component_type);
int dev_upgrade_sync_pre_check(int dev_id);
#endif

#ifndef CFG_SOC_PLATFORM_MDC_V51
void dev_upgrade_clear_localcfg(int dev_id);
#else
/* MDC not supprot clear localcfg */
force_inline void dev_upgrade_clear_localcfg(int dev_id)
{
    dev_upgrade_err("%s: function not supported in TEE upgrade mode.\n", __func__);
}
#endif

#ifdef CFG_FEATURE_ROLL_BACK
#define FLASH_OFFSET_PRIMARY_MAGIC       0x1940000       /*  主区魔术字  */
#define FLASH_OFFSET_PRIMARY_VERSION     0x1940004       /*  主区支持最低版本  */
#define FLASH_OFFSET_PRIMARY_CRC         0x194001C       /*  主区CRC校验位  */
#define FLASH_OFFSET_SECONDARY_MAGIC     0x1980000       /*  备区魔术字  */
#define FLASH_OFFSET_SECONDARY_VERSION   0x1980004       /*  备区支持最低版本  */
#define FLASH_OFFSET_SECONDARY_CRC       0x198001C       /*  备区CRC校验位  */
#define FIRMWARE_BASE                    0x2000
#define FIRMWARE_MAGIC_OFFSET            0x84
#define FIRMWARE_VERSION_OFFSET          0x86
#define EP_FILE_SIZE                     (8 * 1024 + 256)
#define CHECK_LEN                        4
#define VERSION_LEN                      4
#define MAX_DEVICE_NUM                   4
#define MAGIC_NUMBER                     0x55AAAA55
#define FIRMWARE_MAGIC_NUMBER            0xA5
#define OLD_COMPONENT                    0      /*  旧器件  */
#define NEW_COMPONENT                    1      /*  新器件  */
#define UNKNOWN_COMPONENT                2      /*  CRC校验失败  */

#pragma pack(4)
struct flash_version_info_stru {
    unsigned int magic_num;
    unsigned int version;
    unsigned int reserv[5];
    unsigned int crc;
};
#pragma pack()

int dev_upgrade_roll_back_check(int dev_id, unsigned char *file_content, unsigned int file_size);
#endif

#endif /* _DEV_UPGRADE_ADAPT_H_ */
