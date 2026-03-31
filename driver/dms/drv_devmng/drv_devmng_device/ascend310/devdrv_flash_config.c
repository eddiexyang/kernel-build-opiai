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


#include <linux/capability.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/crypto.h>
#include <linux/fs.h>
#include <linux/kernel.h>

#include <linux/scatterlist.h>
#include <linux/version.h>
#include <linux/securec.h>
#include <linux/vmalloc.h>

#ifndef DEVMNG_UT
#include "devdrv_flash_config.h"
#include "user_cfg_public.h"
#include "devdrv_manager_common.h"
#include "devdrv_common.h"

#include "devdrv_user_common.h"
#include "config.h"
#include "hisfc300_def.h"
#include "config.h"

#include "drv_whitelist.h"

#define DEVDRV_FLASH_BLOCK_SIZE UC_FLASH_PARTITION_SIZE // 64KB
#define DEVDRV_FLASH_FIRST_BLOCK 0
#define DEVDRV_FLASH_SECOND_BLOCK 1
#define DEVDRV_FLASH_INVALID 0
#define DEVDRV_FLASH_VALID 1
#define DEVDRV_SHA256_DIGEST 32
static DEFINE_MUTEX(devdrv_flash_config_mutex);
extern int sscanf_s(const char *buffer, const char *format, ...);
extern int devdrv_get_boardid(void);


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
#include <crypto/hash.h>
STATIC int devdrv_calculate_block_sha256(const unsigned char *data, u32 data_len, u8 *sha256_code, u32 sha256_len)
{
    struct crypto_shash *sha256 = NULL;
    int i;

    if (sha256_len != DEVDRV_SHA256_DIGEST) {
        devdrv_drv_err("sha256 len err(%u).\n", (unsigned)sha256_len);
        return -1;
    }
    for (i = 0; i < DEVDRV_SHA256_DIGEST; i++)
        sha256_code[i] = 0;

    sha256 = crypto_alloc_shash("sha256", 0, 0);
    if (IS_ERR(sha256)) {
        devdrv_drv_err("crypto_alloc_shash failed.\n");
        return -1;
    }
    do {
        SHASH_DESC_ON_STACK(shash, sha256);
        shash->tfm = sha256;

        (void)crypto_shash_init(shash);
        (void)crypto_shash_update(shash, data, data_len);
        (void)crypto_shash_final(shash, sha256_code);
    } while (0);

    crypto_free_shash(sha256);

    return 0;
}


#else
STATIC int devdrv_calculate_block_sha256(const unsigned char *data, u32 data_len, u8 *sha256_code, u32 sha256_len)
{
    struct scatterlist sg;
    struct hash_desc desc;
    int ret = 0;
    int i;

    for (i = 0; i < DEVDRV_SHA256_DIGEST; i++)
        sha256_code[i] = 0;

    desc.flags = 0;
    desc.tfm = crypto_alloc_hash("sha256", 0, 0);
    if (IS_ERR_OR_NULL(desc.tfm)) {
        devdrv_drv_err(" crypto_alloc_hash failed.\n");
        return -1;
    }
    sg_init_one(&sg, data, data_len);

    (void)crypto_hash_init(&desc);
    (void)crypto_hash_update(&desc, &sg, data_len);
    (void)crypto_hash_final(&desc, sha256_code);
    crypto_free_hash(desc.tfm);
    return 0;
}

#endif


STATIC int devdrv_config_read_block(u32 block, u8 *buf, u32 buf_size)
{
    u32 offset = 0;
    size_t retlen = 0;
    int ret;

    if (block == DEVDRV_FLASH_FIRST_BLOCK) {
        offset = 0;
    } else {
        offset = DEVDRV_FLASH_BLOCK_SIZE;
    }

    ret = hisi_sfc_ctl_flash_read((unsigned char *)PART_NAME_RESERVE_1, offset, buf_size, &retlen, buf);
    if (ret != 0) {
        devdrv_drv_err("read buff err %d", ret);
        return -1;
    }
    return 0;
}

STATIC int devdrv_config_erase_block(u32 block)
{
    u32 offset = 0;
    int ret;

    if (block == DEVDRV_FLASH_FIRST_BLOCK) {
        offset = 0;
    } else {
        offset = DEVDRV_FLASH_BLOCK_SIZE;
    }
    devdrv_drv_info("dev_id flash erase\n");
    ret = hisi_sfc_ctl_flash_erase((unsigned char *)PART_NAME_RESERVE_1, offset, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret != 0) {
        devdrv_drv_err("erase buff err %d", ret);
        return -1;
    }
    return 0;
}

STATIC int devdrv_config_write_block(u32 block, const u8 *buf, u32 buf_size)
{
    u32 offset = 0;
    size_t retlen = 0;
    int ret;

    if (block == DEVDRV_FLASH_FIRST_BLOCK) {
        offset = 0;
    } else {
        offset = DEVDRV_FLASH_BLOCK_SIZE;
    }
    ret = devdrv_config_erase_block(block);
    if (ret) {
        devdrv_drv_err("erase buff err %d", ret);
        return -1;
    }

    ret = hisi_sfc_ctl_flash_write((unsigned char *)PART_NAME_RESERVE_1, offset, buf_size, &retlen, buf);
    if (ret != 0) {
        devdrv_drv_err("write buff err %d", ret);
        return -1;
    }
    return 0;
}

STATIC int devdrv_config_read_check_code(const u8 *buf, u32 buf_size, u8 *check, u32 check_size)
{
    uint8_t *check_code = NULL;
    int i;

    if (buf == NULL) {
        devdrv_drv_err("input buf is NULL.\n");
        return -1;
    }
    if (buf_size != DEVDRV_FLASH_BLOCK_SIZE) {
        devdrv_drv_err("input buf_size is invalid, "
            "input is: %d, must be: %d.\n",
            buf_size, DEVDRV_FLASH_BLOCK_SIZE);
        return -1;
    }
    if (check == NULL) {
        devdrv_drv_err("input check handle is NULL.\n");
        return -1;
    }
    if (check_size != DEVDRV_SHA256_DIGEST) {
        devdrv_drv_err("input check_size is invalid, "
            "input is: %d, must be: %d.\n",
            check_size, DEVDRV_SHA256_DIGEST);
        return -1;
    }

    /*
     * check_code offset is 64 * 1024 - 4 - 32
     * check_code is a uint8_t array which has 32 elements
     */
    check_code = (uint8_t *)(((uintptr_t)buf) + UC_CHECK_FLASH_OFFSET);

    for (i = 0; i < DEVDRV_SHA256_DIGEST; ++i)
        check[i] = check_code[i];

    return 0;
}

STATIC int devdrv_config_read_valid_flag(const u8 *buf, u32 buf_size, u32 *valid_flag)
{
    uint32_t *flag = NULL;

    if (buf == NULL) {
        devdrv_drv_err("input buf is NULL.\n");
        return -1;
    }
    if (buf_size != DEVDRV_FLASH_BLOCK_SIZE) {
        devdrv_drv_err("input buf_size is invalid, "
            "input is: %d, must be: %d.\n",
            buf_size, DEVDRV_FLASH_BLOCK_SIZE);
        return -1;
    }
    if (valid_flag == NULL) {
        devdrv_drv_err("input check handle is NULL.\n");
        return -1;
    }

    /*
     * valid flag offset is 64 * 1024 - 4
     * valid flag is a uint32_t
     */
    flag = (uint32_t *)(((uintptr_t)buf) + UC_VALID_FLASH_OFFSET);

    *valid_flag = *flag;
    return 0;
}

STATIC int devdrv_config_calculate_check_code(const u8 *buf, u32 buf_size, u8 *check, u32 check_size)
{
    u32 size;
    int ret;

    if (buf == NULL) {
        devdrv_drv_err("input buf is NULL.\n");
        return -1;
    }
    if (buf_size != DEVDRV_FLASH_BLOCK_SIZE) {
        devdrv_drv_err("input buf_size is invalid, "
            "input is: %d, must be: %d.\n",
            buf_size, DEVDRV_FLASH_BLOCK_SIZE);
        return -1;
    }
    if (check == NULL) {
        devdrv_drv_err("input check is NULL.\n");
        return -1;
    }
    if (check_size != DEVDRV_SHA256_DIGEST) {
        devdrv_drv_err("input check_size is invalid, "
            "input is: %d, must be: %d.\n",
            check_size, DEVDRV_SHA256_DIGEST);
        return -1;
    }

    size = UC_CHECK_FLASH_OFFSET;
    ret = devdrv_calculate_block_sha256(buf, size, check, DEVDRV_SHA256_DIGEST);
    if (ret) {
        devdrv_drv_err("devdrv_calculate_block_sha256 return error, "
            "unable to calculate block sha256.\n");
        return -1;
    }

    return 0;
}


STATIC int devdrv_config_set_check_code(u8 *buf, u32 buf_size)
{
    u8 check[DEVDRV_SHA256_DIGEST];
    u8 *data = NULL;
    u32 size;
    int ret;
    int i;

    for (i = 0; i < DEVDRV_SHA256_DIGEST; ++i)
        check[i] = 0;

    if (buf == NULL) {
        devdrv_drv_err("input buf is NULL.\n");
        return -1;
    }
    if (buf_size != DEVDRV_FLASH_BLOCK_SIZE) {
        devdrv_drv_err("input buf_size is invalid, "
            "input is: %d, must be: %d.\n",
            buf_size, DEVDRV_FLASH_BLOCK_SIZE);
        return -1;
    }

    size = UC_CHECK_FLASH_OFFSET;
    ret = devdrv_calculate_block_sha256(buf, size, check, DEVDRV_SHA256_DIGEST);
    if (ret) {
        devdrv_drv_err("devdrv_calculate_block_sha256 return error, "
            "unable to calculate block sha256.\n");
        return -1;
    }

    data = (uint8_t *)((uintptr_t)buf + UC_CHECK_FLASH_OFFSET);

    for (i = 0; i < DEVDRV_SHA256_DIGEST; ++i)
        data[i] = check[i];

    return 0;
}

STATIC int devdrv_config_str2hex(const char *str, u8 *hex, u32 size)
{
    const char *id_pos = NULL;
    int len;
    int ret;
    int i;

    if (str == NULL) {
        devdrv_drv_err("input str is NULL.\n");
        return -1;
    }
    if (hex == NULL) {
        devdrv_drv_err("input hex is NULL.\n");
        return -1;
    }
    len = strlen(str);
    if ((len < DEVDRV_UC_DATT_LEN_MIN) || (len > UC_ITEM_DATA_MAX_LEN)) {
        devdrv_drv_err("default data len[%u] is out of range.\n", len);
        return -1;
    }
    if (len % 2 != 0) {
        devdrv_drv_err("default data must own a "
            "even number length.\n");
        return -1;
    }
    if (str[0] != '0' || (str[1] != 'x' && str[1] != 'X')) {
        devdrv_drv_err("default data is not start "
            "with \"0x\" or \"0X\".\n");
        return -1;
    }

    id_pos = str + 2;
    len -= 2; /* remove prefix "0x" */
    len /= 2; /* figure out number of hex */

    if (id_pos == NULL || (int)size < len) {
        devdrv_drv_err("default data has %d hex, "
            "but output buf_size is only %d.\n",
            len, size);
        return -1;
    }

    for (i = 0; i < len; ++i) {
        ret = sscanf_s(id_pos, "%02hhx", &hex[i]);
        id_pos += 2;
        if (id_pos == NULL || ret != 1) {
            devdrv_drv_err("scanf_s return error: %d, "
                "unable to get change default data into hex.\n",
                ret);
            return -1;
        }
    }

    for (i = len; i < (int)size; ++i)
        hex[i] = 0;

    return 0;
}

/*
 * if current board_id is equal to default user config item,
 * match will set into 1
 */
STATIC int devdrv_config_get_default_extract(const struct user_config_item *item, uint8_t *buf, uint32_t *buf_size)
{
    uint8_t *hex = NULL;
    uint32_t i;
    int ret;

    hex = vmalloc(UC_ITEM_DATA_MAX_LEN);
    if (hex == NULL) {
        devdrv_drv_err("alloc memory for buffer failed.\n");
        return -1;
    }

    for (i = 0; i < UC_ITEM_DATA_MAX_LEN; ++i)
        hex[i] = 0xFF;

    if (item == NULL) {
        vfree(hex);
        hex = NULL;
        devdrv_drv_err("input config item is NULL.\n");
        return -1;
    }
    if (buf == NULL) {
        vfree(hex);
        hex = NULL;
        devdrv_drv_err("input buf handle is NULL.\n");
        return -1;
    }
    if (buf_size == NULL) {
        vfree(hex);
        hex = NULL;
        devdrv_drv_err("input buf_size handle is NULL.\n");
        return -1;
    }

    if ((int)*buf_size < (item->len - 1)) {
        vfree(hex);
        hex = NULL;
        devdrv_drv_err("input buf_size is too small, "
            "input is %d, must be %d at least.\n",
            *buf_size, (item->len - 1));
        return -1;
    }

    *buf_size = item->len - 1;

    ret = devdrv_config_str2hex(item->default_data, hex, *buf_size);
    if (ret) {
        vfree(hex);
        hex = NULL;
        devdrv_drv_err("devdrv_config_str2hex return error, "
            "unable to change default from char* into hex, "
            "ret: %d\n",
            ret);
        return -1;
    }

    for (i = 0; i < *buf_size; ++i)
        buf[i] = hex[i];

    vfree(hex);
    hex = NULL;
    return 0;
}
STATIC int devdrv_config_get_default(int cfg_index, u8 *buf, u32 *buf_size)
{
    int ret;

    if ((cfg_index >= UC_ITEM_MAX_NUM) || (cfg_index < 0)) {
        devdrv_drv_err("cfg_index  is err %d.\n", cfg_index);
        return -1;
    }
    if (buf == NULL) {
        devdrv_drv_err("input buf handle is NULL.\n");
        return -1;
    }
    if (buf_size == NULL) {
        devdrv_drv_err("input buf_size handle is NULL.\n");
        return -1;
    }

    ret = devdrv_config_get_default_extract(&user_cfg_version_1[cfg_index], buf, buf_size);
    if (ret) {
        devdrv_drv_err("devdrv_config_get_default_extract failed, "
            "unable to extract config.\n");
        return -1;
    }
    return 0;
}

/*
 * if current board_id is equal to user config item,
 * match will set into 1
 */
STATIC int devdrv_config_get_item_extract(const struct user_config_item *item, const u8 *block, u32 block_size, u8 *buf,
    u32 *buf_size)
{
    u8 *valid = NULL;
    u8 *raw = NULL;
    u32 i;

    if (item == NULL) {
        devdrv_drv_err("input config item is NULL.\n");
        return -1;
    }

    if ((int)*buf_size < (item->len - 1)) {
        devdrv_drv_err("input buf_size is too small, "
            "input is %d, must be %d at least.\n",
            *buf_size, (item->len - 1));
        return -1;
    }

    *buf_size = item->len - 1;

    valid = (uint8_t *)((uintptr_t)block + UC_CONFIG_FLASH_OFFSET + item->offset);

    if (*valid != UC_ITEM_VALID_VALUE) {
        devdrv_drv_info("item in falsh block is invalid, "
            "rewind to get default config.\n");
        /* no need to process if board id match default config */
        return devdrv_config_get_default_extract(item, buf, buf_size);
    }

    /*
     * valid flag owns one byte length, followed by item data
     * data length is configured in user_cfg_version_1
     */
    raw = (uint8_t *)((uintptr_t)valid + 1);

    for (i = 0; i < *buf_size; ++i)
        buf[i] = raw[i];

    return 0;
}

STATIC int devdrv_config_get_item(const u8 *block, u32 block_size, int cfg_index, u8 *buf, u32 *buf_size)
{
    if (block == NULL) {
        devdrv_drv_err("input block buffer is NULL.\n");
        return -1;
    }
    if (block_size != DEVDRV_FLASH_BLOCK_SIZE) {
        devdrv_drv_err("input block buffer size is invalid, input is: %d, "
            "valid size is: %d.\n",
            block_size, DEVDRV_FLASH_BLOCK_SIZE);
        return -1;
    }
    if (cfg_index >= UC_ITEM_MAX_NUM) {
        devdrv_drv_err("input cfg_index is err %d .\n", cfg_index);
        return -1;
    }
    if (buf == NULL) {
        devdrv_drv_err("input buf handle is NULL.\n");
        return -1;
    }
    if (buf_size == NULL) {
        devdrv_drv_err("input buf_size handle is NULL.\n");
        return -1;
    }


    return devdrv_config_get_item_extract(&user_cfg_version_1[cfg_index], block, block_size, buf, buf_size);
}

STATIC int devdrv_config_set_item(u8 *block, u32 block_size, int cfg_index, const u8 *buf, u32 buf_size, u8 effect)
{
    u8 *valid = NULL;
    u8 *data = NULL;
    u32 offset;
    u32 i;
    int ret;

    if (block == NULL) {
        devdrv_drv_err("input buf is NULL.\n");
        return -1;
    }
    if (block_size != DEVDRV_FLASH_BLOCK_SIZE) {
        devdrv_drv_err("input buf_size is invalid, "
            "input is: %d, must be: %d.\n",
            block_size, DEVDRV_FLASH_BLOCK_SIZE);
        return -1;
    }
    if (cfg_index >= UC_ITEM_MAX_NUM) {
        devdrv_drv_err("input name is NULL.\n");
        return -1;
    }

    if (buf == NULL) {
        devdrv_drv_err("input buf is NULL.\n");
        return -1;
    }
    if ((user_cfg_version_1[cfg_index].len - 1) != buf_size) {
        devdrv_drv_err("input buf_size is invalid, "
            "input is %d, must be %d.\n",
            buf_size, (user_cfg_version_1[cfg_index].len - 1));
        return -1;
    }

    offset = user_cfg_version_1[cfg_index].offset;
    valid = (uint8_t *)((uintptr_t)block + UC_CONFIG_FLASH_OFFSET + offset);
    data = (uint8_t *)((uintptr_t)valid + 1);

    for (i = 0; i < buf_size; ++i)
        data[i] = buf[i];

    *valid = 0;
    if (effect == DEVDRV_FLASH_VALID) {
        *valid = UC_ITEM_VALID_VALUE;
    }

    /*
     * update sha256
     */
    ret = devdrv_config_set_check_code(block, block_size);
    if (ret) {
        devdrv_drv_err("unable to set check code into block buffer.\n");
        return -1;
    }

    return 0;
}


STATIC int devdrv_config_check_valid(const u8 *block, u32 block_size, u32 *valid)
{
    u8 calculate_code[DEVDRV_SHA256_DIGEST];
    u8 check_code[DEVDRV_SHA256_DIGEST];
    u32 valid_flag;
    int ret;
    int i;

    for (i = 0; i < DEVDRV_SHA256_DIGEST; ++i) {
        calculate_code[i] = 0;
        check_code[i] = 0;
    }

    if (block == NULL) {
        devdrv_drv_err("input buf is NULL.\n");
        return -1;
    }
    if (block_size != DEVDRV_FLASH_BLOCK_SIZE) {
        devdrv_drv_err("input buf_size is invalid, "
            "input is: %d, must be: %d.\n",
            block_size, DEVDRV_FLASH_BLOCK_SIZE);
        return -1;
    }
    if (valid == NULL) {
        devdrv_drv_err("input valid handle is NULL.\n");
        return -1;
    }

    ret = devdrv_config_read_valid_flag(block, block_size, &valid_flag);
    if (ret) {
        devdrv_drv_err("unable to get valid flag.\n");
        return -1;
    }

    ret = devdrv_config_read_check_code(block, block_size, check_code, DEVDRV_SHA256_DIGEST);
    if (ret) {
        devdrv_drv_err("unable to get check_code.\n");
        return -1;
    }

    ret = devdrv_config_calculate_check_code(block, block_size, calculate_code, DEVDRV_SHA256_DIGEST);
    if (ret) {
        devdrv_drv_err("calculate check code failed.\n");
        return -1;
    }

    if (valid_flag != UC_VALID_FLAG_VALUE) {
        devdrv_drv_warn("valid_flag %d is invalid, must be %d.\n", valid_flag, UC_VALID_FLAG_VALUE);
        *valid = DEVDRV_FLASH_INVALID;
        return 0;
    }

    ret = memcmp(calculate_code, check_code, DEVDRV_SHA256_DIGEST);
    if (ret != 0) {
        devdrv_drv_info("sha256 in flash is not equal to calculation.\n");
        *valid = DEVDRV_FLASH_INVALID;
        return 0;
    }

    *valid = DEVDRV_FLASH_VALID;
    return 0;
}


STATIC int devdrv_config_get_valid_block(u8 *block, u32 block_size, u32 *valid, u32 *valid_block)
{
    u32 block_id;
    int ret;

    if (block == NULL) {
        devdrv_drv_err("input block buffer is NULL.\n");
        return -1;
    }
    if (block_size != DEVDRV_FLASH_BLOCK_SIZE) {
        devdrv_drv_err("input block buffer size is invalid, input is: %d, "
            "valid size is: %d.\n",
            block_size, DEVDRV_FLASH_BLOCK_SIZE);
        return -1;
    }
    if (valid == NULL) {
        devdrv_drv_err("input valid handler is NULL.\n");
        return -1;
    }
    if (valid_block == NULL) {
        devdrv_drv_err("input valid_block handler is NULL.\n");
        return -1;
    }

    *valid = DEVDRV_FLASH_INVALID;

    for (block_id = DEVDRV_FLASH_FIRST_BLOCK; block_id <= DEVDRV_FLASH_SECOND_BLOCK; ++block_id) {
        ret = devdrv_config_read_block(block_id, block, DEVDRV_FLASH_BLOCK_SIZE);
        if (ret) {
            devdrv_drv_warn("devdrv_config_read_block return error: %d,"
                "unable to read block %d.\n",
                ret, block_id);
            return -1;
        }

        ret = devdrv_config_check_valid(block, DEVDRV_FLASH_BLOCK_SIZE, valid);
        if (ret) {
            devdrv_drv_err("devdrv_config_check_valid return error: %d,"
                "unable to check valid, block %d.\n",
                ret, block_id);
            return -1;
        }

        if (*valid != DEVDRV_FLASH_INVALID) {
            devdrv_drv_info("find valid block, block id is %d.\n", block_id);
            *valid_block = block_id;
            break;
        }
    }

    if (*valid == DEVDRV_FLASH_INVALID) {
        /*
         * do not return error here
         * because user may never flash user config into flash chip before
         */
        devdrv_drv_info("can not find any valid block, "
            "maybe user config is not existent in flash.\n");
    }

    return 0;
}


STATIC int devdrv_config_init_block(uint8_t *block, uint32_t block_size)
{
    uint32_t *data = NULL;
    int ret;

    if (block == NULL) {
        devdrv_drv_err("input block buffer is NULL.\n");
        return -1;
    }
    if (block_size != DEVDRV_FLASH_BLOCK_SIZE) {
        devdrv_drv_err("input block buffer size is invalid, input is: %d, "
            "valid size is: %d.\n",
            block_size, DEVDRV_FLASH_BLOCK_SIZE);
        return -1;
    }

    ret = memset_s(block, block_size, 0, block_size);
    if (ret != 0) {
        devdrv_drv_err("memset_s return error: %d.\n", ret);
        return -1;
    }

    /*
     * first, set version into 1
     */
    data = (uint32_t *)block;
    *data = CURRENT_USER_CONFIG_VERSION;

    /*
     * second, set valid flag
     */
    data = (uint32_t *)((uintptr_t)block + UC_VALID_FLASH_OFFSET);
    *data = UC_VALID_FLAG_VALUE;

    return 0;
}


STATIC int devdrv_config_update_item(u8 *block, u32 block_size, int cfg_index, const u8 *buf, u32 buf_size, u32 effect)
{
    int ret;

    ret = devdrv_config_set_item(block, block_size, cfg_index, buf, buf_size, effect);
    if (ret) {
        devdrv_drv_err("devdrv_config_set_item return NULL, "
            "unable to set new item data into block buffer.\n");
        return -1;
    }

    ret = devdrv_config_write_block(DEVDRV_FLASH_FIRST_BLOCK, block, block_size);
    if (ret) {
        devdrv_drv_err("devdrv_config_write_block return error, "
            "unable to update flash user config block 0.\n");
        return -1;
    }

    ret = devdrv_config_write_block(DEVDRV_FLASH_SECOND_BLOCK, block, block_size);
    if (ret) {
        devdrv_drv_err("devdrv_config_write_block return error, "
            "unable to update flash user config block 1.\n");
        return -1;
    }

    return 0;
}


STATIC int devdrv_config_update(int cfg_index, const uint8_t *buf, uint32_t buf_size, uint32_t effect)
{
    u8 *block = NULL;
    u32 valid_block;
    u32 valid;
    int ret;

    block = vzalloc(DEVDRV_FLASH_BLOCK_SIZE);
    if (block == NULL) {
        devdrv_drv_err("vmalloc return NULL, "
            "unable to alloc buffer for flash block.\n");
        return -1;
    }
    ret = devdrv_config_get_valid_block(block, DEVDRV_FLASH_BLOCK_SIZE, &valid, &valid_block);

    if (ret) {
        vfree(block);
        block = NULL;
        devdrv_drv_err("devdrv_config_get_valid_block return error, "
            "can not get flash block data.\n");
        return -1;
    }
    if (valid == DEVDRV_FLASH_INVALID) {
        valid_block = DEVDRV_FLASH_FIRST_BLOCK;
        devdrv_drv_warn("init new block data.\n");
        ret = devdrv_config_init_block(block, DEVDRV_FLASH_BLOCK_SIZE);
        if (ret) {
            vfree(block);
            block = NULL;
            devdrv_drv_err("devdrv_config_init_block return error, "
                "unable to init block data.\n");
            return -1;
        }
    }
    ret = devdrv_config_update_item(block, DEVDRV_FLASH_BLOCK_SIZE, cfg_index, buf, buf_size, effect);
    if (ret) {
        vfree(block);
        block = NULL;
        devdrv_drv_err("devdrv_config_update return error, "
            "failed to update block data into flash.\n");
        return -1;
    } else {
        devdrv_drv_event("config update block data into flash, cfg_index = %d.\n", cfg_index);
    }

    vfree(block);
    block = NULL;
    return 0;
}


int devdrv_get_user_config_core(int cfg_index, unsigned char *buf, unsigned int *buf_size)
{
    u8 *block = NULL;
    u32 valid_block;
    u32 valid;
    int ret;

    if (buf == NULL) {
        devdrv_drv_err("input buf handle is NULL.\n");
        return -1;
    }
    if (buf_size == NULL) {
        devdrv_drv_err("input buf_size handle is NULL.\n");
        return -1;
    }

    block = vzalloc(DEVDRV_FLASH_BLOCK_SIZE);
    if (block == NULL) {
        devdrv_drv_err("vmalloc return NULL, "
            "unable to alloc buffer for flash block.\n");
        return -1;
    }
    /*
     * get a valid block
     * not care about which block is valid
     */
    ret = devdrv_config_get_valid_block(block, DEVDRV_FLASH_BLOCK_SIZE, &valid, &valid_block);
    if (ret) {
        vfree(block);
        block = NULL;
        devdrv_drv_err("devdrv_config_get_valid_block return error, "
            "can not get flash block data.\n");
        return -1;
    }

    if (valid == DEVDRV_FLASH_INVALID) {
        vfree(block);
        block = NULL;
        /* get default config */
        devdrv_drv_warn("no valid falsh block is found, "
            "user config may be not existent, get default config.\n");

        return devdrv_config_get_default(cfg_index, buf, buf_size);
    }
    /* get item from block data */
    ret = devdrv_config_get_item(block, DEVDRV_FLASH_BLOCK_SIZE, cfg_index, buf, buf_size);
    if (ret) {
        vfree(block);
        block = NULL;
        devdrv_drv_err("devdrv_config_get_item return error, "
            "unable to get config item.\n");
        return -1;
    }

    vfree(block);
    block = NULL;
    return 0;
}
STATIC int devdrv_get_user_config(struct devdrv_flash_config_ioctl_para *config_para)
{
    unsigned char *data = NULL;

    unsigned int data_size = UC_ITEM_DATA_MAX_LEN;
    int ret;
    data = vzalloc(UC_ITEM_DATA_MAX_LEN);
    if (data == NULL) {
        devdrv_drv_err("unable to malloc block mem\n");
        return -1;
    }

    ret = devdrv_get_user_config_core(config_para->cfg_index, data, &data_size);
    if (ret) {
        vfree(data);
        data = NULL;
        devdrv_drv_err("unable to get config[%d] in flash.\n", config_para->cfg_index);
        return -1;
    }
    if (data_size != config_para->buf_size) {
        vfree(data);
        data = NULL;
        devdrv_drv_err("sizeerror %d %u %u.\n", config_para->cfg_index, config_para->buf_size, data_size);
        return -1;
    }
    config_para->buf_size = data_size;
    ret = copy_to_user_safe((void *)config_para->buf, data, data_size);
    if (ret) {
        vfree(data);
        data = NULL;
        devdrv_drv_err("copy_from_user return error: %d.\n", ret);
        return -1;
    }
    vfree(data);
    data = NULL;
    return 0;
}
STATIC int devdrv_set_user_config(struct devdrv_flash_config_ioctl_para *config_para)
{
    unsigned char *data = NULL;
    unsigned char *block = NULL;
    unsigned char *kernel_buff = NULL;
    int ret;
    unsigned int i;
    unsigned int data_size;
    unsigned int valid_block;
    unsigned int valid;
    int same;

    /* size had been checked before */
    kernel_buff = vmalloc(UC_ITEM_DATA_MAX_LEN);
    if (kernel_buff == NULL) {
        devdrv_drv_err("alloc memory for block buffer failed.\n");
        return -1;
    }
    for (i = 0; i < UC_ITEM_DATA_MAX_LEN; ++i)
        kernel_buff[i] = 0;

    ret = copy_from_user_safe((void *)kernel_buff, config_para->buf, config_para->buf_size);
    if (ret) {
        vfree(kernel_buff);
        kernel_buff = NULL;
        devdrv_drv_err("copy_from_user return error: %d.\n", ret);
        return -1;
    }

    /* first, check if flash config is valid or not */

    block = vmalloc(DEVDRV_FLASH_BLOCK_SIZE);
    if (block == NULL) {
        vfree(kernel_buff);
        kernel_buff = NULL;
        devdrv_drv_err("alloc memory for block buffer failed.\n");
        return -1;
    }
    for (i = 0; i < DEVDRV_FLASH_BLOCK_SIZE; ++i)
        block[i] = 0;

    valid = DEVDRV_FLASH_INVALID;
    ret = devdrv_config_get_valid_block(block, DEVDRV_FLASH_BLOCK_SIZE, &valid, &valid_block);
    vfree(block);
    block = NULL;

    if (ret) {
        vfree(kernel_buff);
        kernel_buff = NULL;
        devdrv_drv_err("devdrv_config_get_valid_block return error, "
            "can not get flash block data.\n");
        return -1;
    }

    if (valid == DEVDRV_FLASH_INVALID) {
        devdrv_drv_info("no valid falsh block is found, "
            "user config is not existent, just set config.\n");
        ret = devdrv_config_update(config_para->cfg_index, kernel_buff, config_para->buf_size, DEVDRV_FLASH_VALID);
        vfree(kernel_buff);
        kernel_buff = NULL;
        return ret;
    }

    devdrv_drv_info("valid falsh block is found, continue to check if update or not.\n");

    data = vmalloc(UC_ITEM_DATA_MAX_LEN);
    if (data == NULL) {
        vfree(kernel_buff);
        kernel_buff = NULL;
        devdrv_drv_err("alloc memory for buffer failed.\n");
        return -1;
    }

    for (i = 0; i < UC_ITEM_DATA_MAX_LEN; ++i)
        data[i] = 0xFF;

    data_size = UC_ITEM_DATA_MAX_LEN;
    ret = devdrv_get_user_config_core(config_para->cfg_index, data, &data_size);
    if (ret) {
        vfree(kernel_buff);
        kernel_buff = NULL;
        vfree(data);
        data = NULL;
        devdrv_drv_err("unable to get config[%d] in flash.\n", config_para->cfg_index);
        return -1;
    }

    if (config_para->buf_size != data_size) {
        vfree(kernel_buff);
        kernel_buff = NULL;
        vfree(data);
        data = NULL;
        devdrv_drv_err("input config[%d] size[%d] is not equal to "
            "its length[%d] in flash.\n",
            config_para->cfg_index, config_para->buf_size, data_size);
        return -1;
    }

    same = 1;
    for (i = 0; i < config_para->buf_size; ++i) {
        if (kernel_buff[i] != data[i]) {
            same = 0;
            break;
        }
    }


    vfree(data);
    data = NULL;
    if (same) {
        devdrv_drv_info("input config[%d] is same as its value in flash, "
            "will not update.\n",
            config_para->cfg_index);
        vfree(kernel_buff);
        kernel_buff = NULL;
        return 0;
    }

    devdrv_drv_info("input config[%d] is not the same as its value in flash, "
        "just update.\n",
        config_para->cfg_index);

    ret = devdrv_config_update(config_para->cfg_index, kernel_buff, config_para->buf_size, DEVDRV_FLASH_VALID);
    vfree(kernel_buff);
    kernel_buff = NULL;
    return ret;
}
STATIC int devdrv_clear_user_config(struct devdrv_flash_config_ioctl_para *config_para)
{
    u8 *buf = NULL;
    u32 buf_size;
    int ret;
    u32 i;

    buf_size = user_cfg_version_1[config_para->cfg_index].len - 1; // for vaild flag
    buf = vmalloc(buf_size);
    if (buf == NULL) {
        devdrv_drv_err("alloc memory for buffer failed.\n");
        return -1;
    }

    for (i = 0; i < buf_size; ++i)
        buf[i] = 0xFF;
    ret = devdrv_config_update(config_para->cfg_index, buf, buf_size, DEVDRV_FLASH_INVALID);
    vfree(buf);
    buf = NULL;
    return ret;
}

STATIC int check_config_para(struct devdrv_flash_config_ioctl_para *config_para)
{
    if (config_para->dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("devid error %u .\n", config_para->dev_id);
        return -1;
    }


    if (config_para->cmd != DEVDRV_FLASH_CONFIG_CLEAR_CMD) {
        if (config_para->buf_size == 0 || config_para->buf_size > UC_ITEM_DATA_MAX_LEN || config_para->buf == NULL) {
            devdrv_drv_err("buf_size error %u %pK .\n", config_para->buf_size, config_para->buf);
            return -1;
        }
    }
    if ((config_para->cfg_index >= UC_ITEM_MAX_NUM) || (config_para->cfg_index < 0)) {
        devdrv_drv_err("cfg index error %d .\n", config_para->cfg_index);
        return -1;
    }
    devdrv_drv_debug("check_config_para ok cfg index : %d, name %s\n", config_para->cfg_index,
        user_cfg_version_1[config_para->cfg_index].name);
    return 0;
}

// NOT for root config
STATIC int check_user_config_para(struct devdrv_flash_config_ioctl_para *config_para)
{
    if (user_cfg_version_1[config_para->cfg_index].authority_flag == UC_AUTHORITY_ROOT_WR) {
        devdrv_drv_err("config authority error %d.\n", config_para->cfg_index);
        return -1;
    }
    if ((user_cfg_version_1[config_para->cfg_index].authority_flag == UC_AUTHORITY_USER_RO) &&
        ((config_para->cmd == DEVDRV_FLASH_CONFIG_WRITE_CMD) || (config_para->cmd == DEVDRV_FLASH_CONFIG_CLEAR_CMD))) {
        devdrv_drv_err("config authority error %d.\n", config_para->cfg_index);
        return -1;
    }
    return 0;
}

STATIC int devdrv_config_check_authority(struct devdrv_flash_config_ioctl_para *cfg_para)
{
    s32 ret;
    const char *process_name[WHITE_LIST_PROCESS_NUM] = {PROCESS_NAME_DMP};

    /* set index by if it's cert item */
    if (cfg_para->cmd != DEVDRV_FLASH_CONFIG_READ_CMD) {
        ret = whitelist_process_handler(process_name, WHITE_LIST_PROCESS_NUM);
        if (ret) {
            devdrv_drv_err("whitelist_process_handler error, ret=%d.\n", ret);
            return -1;
        }
    }

    return 0;
}

int devdrv_flash_user_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_flash_config_ioctl_para config_para = {0};
    int ret;

    if (!arg || filep == NULL) {
        devdrv_drv_err("arg = %lu, filep = %pK\n", arg, filep);
        return -1;
    }
    ret = copy_from_user_safe(&config_para, (void *)((uintptr_t)arg), sizeof(struct devdrv_flash_config_ioctl_para));
    if (ret) {
        devdrv_drv_err("copy_from_user return error: %d.\n", ret);
        return -1;
    }
    ret = check_config_para(&config_para);
    if (ret) {
        devdrv_drv_err("check para error: %d.\n", ret);
        return -1;
    }

    ret = check_user_config_para(&config_para);
    if (ret) {
        devdrv_drv_err("check user para error: %d.\n", ret);
        return -1;
    }

    ret = devdrv_config_check_authority(&config_para);
    if (ret) {
        devdrv_drv_err("check authority config error: %d.\n", ret);
        return -1;
    }

    mutex_lock(&devdrv_flash_config_mutex);

    switch (config_para.cmd) {
        case DEVDRV_FLASH_CONFIG_READ_CMD: {
            ret = devdrv_get_user_config(&config_para);
            break;
        }

        case DEVDRV_FLASH_CONFIG_WRITE_CMD: {
            ret = devdrv_set_user_config(&config_para);
            break;
        }

        case DEVDRV_FLASH_CONFIG_CLEAR_CMD: {
            ret = devdrv_clear_user_config(&config_para);
            break;
        }

        default: {
            devdrv_drv_err("cmd para error: %d.\n", config_para.cmd);
            ret = -1;
            break;
        }
    }
    mutex_unlock(&devdrv_flash_config_mutex);
    return ret;
}
int devdrv_flash_root_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_flash_config_ioctl_para config_para = {0};
    int ret;

    if (!arg || filep == NULL) {
        devdrv_drv_err("arg = %lu, filep = %pK\n", arg, filep);
        return -1;
    }

    // first judge root authority
    if (!capable(CAP_SYS_ADMIN)) {
        devdrv_drv_err("check authority fail\n");
        return -1;
    }

    ret = copy_from_user_safe(&config_para, (void *)((uintptr_t)arg), sizeof(struct devdrv_flash_config_ioctl_para));
    if (ret) {
        devdrv_drv_err("copy_from_user return error: %d.\n", ret);
        return -1;
    }

    ret = check_config_para(&config_para);
    if (ret) {
        devdrv_drv_err("check para error: %d.\n", ret);
        return -1;
    }
    mutex_lock(&devdrv_flash_config_mutex);


    switch (config_para.cmd) {
        case DEVDRV_FLASH_CONFIG_READ_CMD: {
            ret = devdrv_get_user_config(&config_para);
            break;
        }

        case DEVDRV_FLASH_CONFIG_WRITE_CMD: {
            ret = devdrv_set_user_config(&config_para);
            break;
        }

        case DEVDRV_FLASH_CONFIG_CLEAR_CMD: {
            ret = devdrv_clear_user_config(&config_para);
            break;
        }
        default: {
            devdrv_drv_err("cmd para error: %d.\n", config_para.cmd);
            ret = -1;
            break;
        }
    }
    mutex_unlock(&devdrv_flash_config_mutex);

    return ret;
}

int devdrv_get_config_index_by_name(const char *name)
{
    int board_id;
    int i;

    board_id = devdrv_get_boardid();
    if (board_id < 0) {
        devdrv_drv_err("unable to get board id.\n");
        return -ENODEV;
    }

    for (i = 0; i < UC_ITEM_MAX_NUM; i++) {
        if (strncmp(user_cfg_version_1[i].name, name, UC_ITEM_DATA_MAX_LEN) == 0) {
            if (user_cfg_version_1[i].board_id == board_id || user_cfg_version_1[i].board_id == -1) {
                devdrv_drv_info("board id matchs, board id:%u, config board id:%d\n", board_id,
                    user_cfg_version_1[i].board_id);
                return i;
            }
        }
    }
    devdrv_drv_warn("board id isn't matchs, board id:%u, name:%s\n", board_id, name);
    return -ENODEV;
}


int devdrv_flash_get_aicpu_config(const char *name, unsigned int *aicpu_num)
{
    unsigned char aicpu_config[UC_AICPU_CONFIG_SIZE] = {0};
    unsigned int aicpu_size = UC_AICPU_CONFIG_SIZE;
    int index;
    int ret;

    if (name == NULL || aicpu_num == NULL) {
        devdrv_drv_err("name or aicpu_num is NULL\n");
        return -ENODEV;
    }
    index = devdrv_get_config_index_by_name(name);
    if (index < 0) {
        devdrv_drv_err("can't find config name %s, or board id isn't support\n", name);
        return -ENODEV;
    }

    ret = devdrv_get_user_config_core(index, aicpu_config, &aicpu_size);
    if (ret) {
        devdrv_drv_err("unable to get config(%s) in flash.\n", name);
        return -ENODEV;
    }

    *aicpu_num = (unsigned int)aicpu_config[0];
    if (*aicpu_num == 0) {
        devdrv_drv_info("not set the aicpu config in flash\n");
        return -EINVAL;
    }
    if (*aicpu_num != AICPU2_CTRLCPU6_CONFIG && *aicpu_num != AICPU4_CTRLCPU4_CONFIG &&
        *aicpu_num != AICPU6_CTRLCPU2_CONFIG) {
        devdrv_drv_err("the aicpu config is invalid in flash, aicpu config:%u\n", *aicpu_num);
        return -ENODEV;
    }
    return 0;
}
EXPORT_SYMBOL(devdrv_flash_get_aicpu_config);
#else
int devdrv_flash_get_aicpu_config(const char *name, unsigned int *aicpu_num)
{
    return 0;
}
#endif
