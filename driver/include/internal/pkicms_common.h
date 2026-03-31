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

#ifndef PKICMS_COMMON_H_
#define PKICMS_COMMON_H_
#include <linux/types.h>


#define CURR_MODULE "pkicms"

#ifndef PKICMS_UT_TEST
#include "drv_log.h"

#define pkicms_err(fmt, ...) drv_err(CURR_MODULE, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define pkicms_warn(fmt, ...) drv_warn(CURR_MODULE, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define pkicms_info(fmt, ...) drv_info(CURR_MODULE, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define pkicms_event(fmt, ...) drv_event(CURR_MODULE, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define pkicms_debug(fmt, ...) drv_pr_debug(CURR_MODULE, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#else
#define pkicms_err(fmt, ...)                                                                              \
    do {                                                                                                  \
        printk(KERN_ERR "[ERROR][%s][%s %d]: " fmt, CURR_MODULE, __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#define pkicms_warn(fmt, ...)                                                                          \
    do {                                                                                               \
        printk(KERN_WARNING "[%s][%s %d]: " fmt, CURR_MODULE, __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#define pkicms_info(fmt, ...)                                                                       \
    do {                                                                                            \
        printk(KERN_INFO "[%s][%s %d]: " fmt, CURR_MODULE, __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#define pkicms_debug(fmt, ...)                                                                       \
    do {                                                                                             \
        printk(KERN_DEBUG "[%s][%s %d]: " fmt, CURR_MODULE, __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#endif

#if defined(STATIC_SKIP)
#define STATIC
#else
#define STATIC static
#endif

/**
 * read file return code
 */
#define READ_CONTINUE               1
#define READ_END                    0
#define READ_ERROR                  (-1)

#define SEC_IMG_VERIFY_MAX_SIZE     0x400000

/*
 * ESBC Header field length
 */
#define ESBC_ROOTKEY_LEN               512
#define ESBC_USER_DATA_SIZE            32
#define ESBC_HASH_SIZE                 32
#define ESBC_ENCRYPT_IV_LEN            16
#define ESBC_DERIVE_SEED_LEN           32
#define ESBC_ENCRYPT_TAG_LEN           16
#define ESBC_CMC_TAG_LEN               12
#define ESBC_HEAD_TAG_LEN              12
#define ESBC_RESERVED_LEN              8
#define ESBC_RESERVED_1_LEN            88
#define ESBC_RESERVED_2_LEN            20
#define ESBC_ENCRYPT_ADD_LEN           16

// efuse read macro
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#define EFUSE0_CTRL_BASE             0x703B40000
#define EFUSE_NS_FORBID_OFFSET       (EFUSE0_CTRL_BASE + 0xE384)    /* secure check enable flag */
#define EFUSE1_CTRL_BASE             0x703B50000
#define NS_FORBID_BIT_OFFSET         23
#define SMP_REG_BASE_OFFSET          0x80000000000
#define DIE_REG_BASE_OFFSET          0x10000000000
#else
#define EFUSE0_CTRL_BASE             0x81260000U
#define EFUSE_NS_FORBID_OFFSET       (EFUSE0_CTRL_BASE + 0xE080)    /* secure check enable flag */
#define EFUSE1_CTRL_BASE             0x81270000U
#define SMP_REG_BASE_OFFSET          0x8000000000
#endif
#define EFUSE_ROTPK1_OFFSET          (EFUSE1_CTRL_BASE + 0xE228)    /* efuse pub key */
#define EFUSE_HW_CATEGORY_OFFSET     (EFUSE1_CTRL_BASE + 0xE248)    /* sub key category */
#define EFUSE_SUBKEYID_MASK          (EFUSE1_CTRL_BASE + 0xE21C)    /* key revocated mask */

#define EFUSE_REG_READ_LEN      4
#define EFUSE_HASH_LEN          32
#define EFUSE_HASH_WORDS        8

#define PKICMS_SEC_CHECK_ENABLE 1
#define PKICMS_SEC_CHECK_DISABLE 0

// efuse read second head
#define IMG_MAGIC_LEN                     4
#define IMG_TAG_LEN                       16
#define IMG_NVCNT_LEN                     4
#define IMG_VER_LEN                       16
#ifdef CFG_FEATURE_SIGN_VERSION_1
#define SECOND_HEAD_LEN                   120
#else
#define SECOND_HEAD_LEN                   (IMG_MAGIC_LEN + IMG_TAG_LEN + IMG_NVCNT_LEN + IMG_VER_LEN)
#endif
#define IMG_VER_MAGIC                     0x3a3aaa33
#define BIT_POS_MAX                       32
#define IMG_NVCNT_VER_MAX                 31
#define IMG_HASH_LEN                      64
#define IMG_RESERVED_LEN                  8

enum PKICMS_ERROR {
    PKICMS_ERR_PARAM = 1,
    PKICMS_ERR_MEMORY,
    PKICMS_ERR_HASH,
    PKICMS_ERR_KERNEL,
};

typedef struct image_file_operations {
    uintptr_t (*open)(const char *filename);
    int (*read)(uintptr_t fp, char *buff, u32 len, int *read_len);
    loff_t (*lseek)(uintptr_t fp, loff_t offset, int whence);
    void (*release)(uintptr_t fp);
    loff_t (*cur_offset)(uintptr_t fp);
} image_fop_t;

/*
 * rsa256 hash api
 */
typedef struct shash_sha256 {
    struct crypto_shash *shash;
    struct shash_desc *desc;
} shash_sha256_t;

struct image_file {
    char file_name[256]; /* file name for 256 bytes */

    /*
     * file content offset, valid when great than 0
     */
    loff_t content_offset;

    /*
     * file content length, 0xFFFFFFFF means to end
     */
    unsigned long content_length;
};
typedef struct image_file image_file_t;

/*
 * calculate the file's digest
 * @filename: file name, include file path
 * @data_offset: data offset from start of file to calculate degist
 * @data_len: length of data to calculate degist
 * @hashcode: result of data digest will storage in this buffer
 * @hash_len: hashcode buffer len
 * @op: file operations, if use local file system. init with funciton
 * set_local_file_operations
 *
 * the function calculate digest of the data in the file. The algorithm used is SHA256.
 */
int pkicms_calc_digest(const char *filename, loff_t data_offset, unsigned long data_len,
    u8 *hashcode, u32 hash_len, image_fop_t *op);

/*
* esbc heander struct
*/
#ifdef CFG_FEATURE_SIGN_VERSION_1
#define ESBC_PADDING_LEN 104
typedef struct esbc_header {
    u32 preamble; /* 0x55aa55aa */
    u8 head_ver_tag[ESBC_HEAD_TAG_LEN];
    u8 reserved[ESBC_RESERVED_LEN];
    u32 head_len; /* don't check during secure booting */
    u32 user_len;
    u8 user_define_data[ESBC_USER_DATA_SIZE]; /* don't check during secure booting */
    u8 src_hash[ESBC_HASH_SIZE];              /* only user for unsecurity boot */
    u32 subkey_cert_offset;
    u32 subkey_cert_len;
    u32 root_hash_alg;
    u32 sign_alg;
    u32 root_pub_k_len;
    u8 root_pub_key[ESBC_ROOTKEY_LEN];   /* _e valuse & fixed to 65537 */
    u8 root_pub_key_e[ESBC_ROOTKEY_LEN]; /* _n value, length is _root_pub_k_len */
    u32 code_offset;                     /* offset between image header */
    u32 code_len;
    u32 sign_offset;
    u32 sign_len;
    u32 encrypt_flag;
    u32 encrypt_alg;
    u8 derive_seed[ESBC_DERIVE_SEED_LEN];
    u32 km_iretation_cnt;
    u8 encrypt_iv[ESBC_ENCRYPT_IV_LEN];
    u8 encrypt_tag[ESBC_ENCRYPT_TAG_LEN];
    u8 encrypt_aad[ESBC_ENCRYPT_ADD_LEN];
    u8 reserved1[ESBC_RESERVED_1_LEN];
    u32 h2c_enable;
    u32 h2c_cert_len;
    u32 h2c_cert_offset;
    u32 root_pub_key_info;
    u8 reserved2[ESBC_RESERVED_2_LEN];
    u32 head_magic; /* 0x33cc33cc */
    u8 head_hash[ESBC_HASH_SIZE];
    u8 cms_tag[ESBC_CMC_TAG_LEN];
    u32 img_len;
    u64 code_len_extend;  /* only used on (>4G) rootfs or appimg */
    u8 padding[ESBC_PADDING_LEN];
} esbc_header_t;

typedef struct {
    u8 tag[IMG_VER_LEN];
    u32 nv_cnt;
    u32 hash_alg;
    u8 hash[IMG_HASH_LEN];
    u32 offset;
    u32 len;
    u8 reserved[IMG_RESERVED_LEN];
    u8 ver[IMG_VER_LEN];
    u32 sig_enable_field;
    u32 hashtree_offset;
} img_ver_header;
#else
typedef struct esbc_header {
    u32 preamble; /* 0x55aa55aa */
    u32 head_len; /* don't check during secure booting */
    u32 user_len;
    u8 user_define_data[ESBC_USER_DATA_SIZE]; /* don't check during secure booting */
    u8 src_hash[ESBC_HASH_SIZE];              /* only user for unsecurity boot */
    u32 subkey_cert_offset;
    u32 sign_alg;
    u32 root_pub_k_len;
    u8 root_pub_key[ESBC_ROOTKEY_LEN];   /* _e valuse & fixed to 65537 */
    u8 root_pub_key_e[ESBC_ROOTKEY_LEN]; /* _n value, length is _root_pub_k_len */
    u32 code_offset;                     /* offset between image header */
    u32 code_len;
    u32 sign_offset;
    u32 encrypt_flag;
    u32 encrypt_alg;
    u8 encrypt_iv[ESBC_ENCRYPT_IV_LEN];
    u8 derive_seed[ESBC_DERIVE_SEED_LEN];
    u8 encrypt_tag[ESBC_ENCRYPT_TAG_LEN];
    u32 h2c_enable;
    u32 h2c_cert_len;
    u32 h2c_cert_offset;
    u32 head_magic; /* 0x33cc33cc */
    u8 head_hash[ESBC_HASH_SIZE];
    u32 reserved;
    u8 cms_tag[ESBC_CMC_TAG_LEN];
    u32 img_len;
    u64 code_len_extend;  /* only used on (>4G) rootfs or appimg */
} esbc_header_t;

typedef struct {
    u32 magic_num;
    u8 tag[IMG_VER_LEN];
    u32 nv_cnt;
    u8 ver[IMG_VER_LEN];
    u32 sig_enable_field;
    u32 hashtree_offset;
} img_ver_header;
#endif

int pkicms_sha256_string(const void *data, u32 len, u8 *hash_code, u32 *hash_len);
int pkicms_sha256_init(shash_sha256_t *sha256);
int pkicms_sha256_update(shash_sha256_t *sha256, const void *data, u32 len);
int pkicms_sha256_final(shash_sha256_t *sha256, void *sha256_code, u32 *hash_len);
int pkicms_get_sec_check_enable_flag(int *flag);

/**
 * init op with local file operations
 * @op: can't be null
 *
 * if the file is local file in OS.
 * the method init the local file operations.
 */
int set_local_file_operations(image_fop_t *op);

int pkicms_read_efuse_reg(u32 dev_id, u64 efuse_offset, u32 *out_value);
int pkicms_nvcnt_read(u32 dev_id, u32 *hw_l2_nv_cnt);
int pkicms_check_img_tag(int verify_type, int img_id, const img_ver_header *img_ver);
int pkicms_ver_check(int verify_type, int img_id, const img_ver_header *img_ver, u32 hw_cnt);
int pkicms_write_file_buf(const char *path, u8 *buf, u32 len);

#endif
