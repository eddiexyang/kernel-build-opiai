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

#ifndef DEVMNG_UT
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
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/kallsyms.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/gfp.h>

#ifdef CFG_USER_CFG_SUPPORT_COMPACT_SPACE
#include <linux/sort.h>
#endif

#include "ascend_platform.h"
#include "user_cfg_management.h"
#include "user_cfg_public.h"
#include "devdrv_user_common.h"
#include "dev_user_cfg.h"
#include "config.h"
#include "drv_whitelist.h"
#include "user_cfg_interface.h"
#include "drv_comm.h"
#include "kernel_version_adapt.h"
#ifdef CFG_USER_CFG_SUPPORT_SEC_FLASH
#include "hiss/hsm_info.h"
#endif

#ifdef CFG_SOC_PLATFORM_CLOUD
#include "hisfc300_def.h"
#endif

#ifdef CFG_SOC_PLATFORM_MINI
#include "devdrv_manager.h"
#include "devdrv_gpioirq_register.h"
#include "hisfc300_def.h"
#endif

static DEFINE_MUTEX(devdrv_flash_config_mutex_dev0);
static DEFINE_MUTEX(devdrv_flash_config_mutex_dev1);
static DEFINE_MUTEX(devdrv_flash_config_mutex_dev2);
static DEFINE_MUTEX(devdrv_flash_config_mutex_dev3);

STATIC s32 devdrv_config_write_blk_ops(u32 dev_id, u32 blk_index, u8 *block_data, u32 blk_size, u32 flag);
STATIC s32 devdrv_user_config_op(devdrv_cfg_para_t *config_para, s32 cmd, unsigned long arg);
STATIC s32 devdrv_config_write_one_block(u32 dev_id, u32 blk_index, u8 *block_data);
STATIC s32 devdrv_config_rebuild_item_content(u32 dev_id, u8 *head_blk);
STATIC s32 devdrv_config_sync_main_backup(u32 dev_id, u8 *head_blk, u32 blk_size, u32 sync_flg, u32 blk_index);
s32 devdrv_get_config_index_by_name(const char *name);

#ifdef CFG_USER_CFG_SUPPORT_MAC_INFO
STATIC s32 devdrv_config_set_mac_info(u32 dev_id, u8 *buf, u32 buf_size);
STATIC s32 devdrv_config_clear_mac_info(u32 dev_id);
int devdrv_config_get_mac_info(unsigned int dev_id, unsigned char *buf, unsigned int buf_size, unsigned int *info_size);
#endif

#if defined(CFG_FEATURE_PSS_SIGN) && defined(CFG_FEATURE_PKCS_SIGN)
STATIC devdrv_cfg_pss_t g_sign_type = {0};
#endif

#ifdef CFG_FEATURE_CPU_NUM_CFG
STATIC s32 devdrv_config_get_default_dts_cpu_config(u8 *buf, u32 *buf_size, const struct user_config_item *item);
#endif

STATIC struct user_config_item_default_func g_user_config_item_default_func_list[UC_ITEM_MAX_NUM] = {
    #ifdef CFG_FEATURE_CPU_NUM_CFG
        {"cpu_num_cfg", devdrv_config_get_default_dts_cpu_config},
    #endif
};

#ifdef CFG_SOC_PLATFORM_MINIV2
#define BOARDID_UNINITED (-1)
STATIC int g_devdrv_boardid = BOARDID_UNINITED;
STATIC void devdrv_get_boardid_by_reg(void)
{
    void __iomem *sysctl_base = NULL;

    if (g_devdrv_boardid == BOARDID_UNINITED) {
        sysctl_base = ioremap(SYSCTL_REG_BASE_ADDR, SYSCTL_REG_SIZE);
        if (sysctl_base == NULL) {
            DEV_USER_CFG_ERR("The ioremap function return NULL.\n");
            return;
        }
        g_devdrv_boardid = readl_relaxed((void __iomem *)(uintptr_t)((u64)(uintptr_t)sysctl_base +
            DRV_BOARD_ID_REG_OFFSET));
        iounmap(sysctl_base);
        sysctl_base = NULL;

        /* byte0 and byte1 used to board id */
        g_devdrv_boardid &= BOARD_ID_REG_VALUE_MASK;
    }

    DEV_USER_CFG_INFO("The value of board_id. (board_id=%d)\n", g_devdrv_boardid);
    return;
}
#endif
int devdrv_config_get_dev_num(unsigned int *dev_num)
{
#ifdef CFG_FEATURE_GET_DEV_NUM_FROM_ASCEND_CTL
    int ret;
    get_dev_num_handler_t get_dev_num_handle;
    get_dev_num_handle = (get_dev_num_handler_t)(uintptr_t)__symbol_get(ASCEND_CTL_GET_DEV_NUM_FUNC);
    if (get_dev_num_handle == NULL) {
        DEV_USER_CFG_ERR("The kallsyms_lookup_name function return NULL.\n");
        return UC_ERR_PARA;
    }

    ret = get_dev_num_handle(dev_num);
    __symbol_put(ASCEND_CTL_GET_DEV_NUM_FUNC);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The get_dev_num_handle function fail. (ret=%d)\n", ret);
        return ret;
    }

    return ret;
#else
    *dev_num = DEVDRV_UC_CHIP_MAX;
    return 0;
#endif
}

STATIC void devdrv_config_mutex_lock(u32 dev_id)
{
    switch (dev_id) {
        case DEVDRV_UC_DEV_ID_0:
            mutex_lock(&devdrv_flash_config_mutex_dev0);
            break;
        case DEVDRV_UC_DEV_ID_1:
            mutex_lock(&devdrv_flash_config_mutex_dev1);
            break;
        case DEVDRV_UC_DEV_ID_2:
            mutex_lock(&devdrv_flash_config_mutex_dev2);
            break;
        case DEVDRV_UC_DEV_ID_3:
            mutex_lock(&devdrv_flash_config_mutex_dev3);
            break;
        default:
            DEV_USER_CFG_ERR("Device id error for devdev_config_mutex_lock. (dev_id=%u)\n", dev_id);
            break;
    }
}

STATIC void devdrv_config_mutex_unlock(u32 dev_id)
{
    switch (dev_id) {
        case DEVDRV_UC_DEV_ID_0:
            mutex_unlock(&devdrv_flash_config_mutex_dev0);
            break;
        case DEVDRV_UC_DEV_ID_1:
            mutex_unlock(&devdrv_flash_config_mutex_dev1);
            break;
        case DEVDRV_UC_DEV_ID_2:
            mutex_unlock(&devdrv_flash_config_mutex_dev2);
            break;
        case DEVDRV_UC_DEV_ID_3:
            mutex_unlock(&devdrv_flash_config_mutex_dev3);
            break;
        default:
            DEV_USER_CFG_ERR("Device id error for devdrv_config_mutex_unlock. (device_id=%u)\n", dev_id);
            break;
    }
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
#include <crypto/hash.h>
STATIC s32 devdrv_calculate_block_sha256(const u8 *data, u32 data_len, u8 *sha256_code, u32 sha256_len)
{
    int ret;
    struct crypto_shash *sha256 = NULL;

    if (sha256_len != DEVDRV_SHA256_DIGEST) {
        DEV_USER_CFG_ERR("The length of sha256 is incorrect. (length=%u)\n", (u32)sha256_len);
        return UC_ERR_PARA;
    }

    ret = memset_s(sha256_code, DEVDRV_SHA256_DIGEST, 0, DEVDRV_SHA256_DIGEST);
    if (ret) {
        DEV_USER_CFG_ERR("The memset_s for sha256 code failed. (ret=%d)\n", ret);
        return UC_ERR_MEM_INIT;
    }

    sha256 = crypto_alloc_shash("sha256", 0, 0);
    if (IS_ERR_OR_NULL(sha256)) {
        DEV_USER_CFG_ERR("The crypto_alloc_shash function failed.\n");
        return UC_ERR_SHA256_CHECK;
    }

    do {
        SHASH_DESC_ON_STACK(shash, sha256);
        shash->tfm = sha256;

        ret = crypto_shash_init(shash);
        if (ret < 0) {
            DEV_USER_CFG_ERR("The crypto_shash_init function failed. (ret=%d)\n", ret);
            goto OUT;
        }
        ret = crypto_shash_update(shash, data, data_len);
        if (ret < 0) {
            DEV_USER_CFG_ERR("The crypto_shash_update function failed. (ret=%d)\n", ret);
            goto OUT;
        }
        ret = crypto_shash_final(shash, sha256_code);
        if (ret < 0) {
            DEV_USER_CFG_ERR("The crypto_shash_final function failed. (ret=%d)\n", ret);
        }
    } while (0);

OUT:
    crypto_free_shash(sha256);
    return ret;
}
#else
STATIC s32 devdrv_calculate_block_sha256(const u8 *data, u32 data_len, u8 *sha256_code, u32 sha256_len)
{
    int ret;
    struct scatterlist sg;
    struct hash_desc desc;

    if (sha256_len != DEVDRV_SHA256_DIGEST) {
        DEV_USER_CFG_ERR("The length of sha256 is incorrect. (length=%u)\n", (u32)sha256_len);
        return UC_ERR_PARA;
    }

    ret = memset_s(sha256_code, DEVDRV_SHA256_DIGEST, 0, DEVDRV_SHA256_DIGEST);
    if (ret) {
        DEV_USER_CFG_ERR("The memset_s for sha256 code failed. (ret=%d)\n", ret);
        return UC_ERR_MEM_INIT;
    }

    desc.flags = 0;
    desc.tfm = crypto_alloc_hash("sha256", 0, 0);

    if (IS_ERR_OR_NULL(desc.tfm)) {
        DEV_USER_CFG_ERR("The crypto_alloc_hash function failed.\n");
        return UC_ERR_SHA256_CHECK;
    }

    sg_init_one(&sg, data, data_len);

    ret = crypto_hash_init(&desc);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The crypto_hash_init function failed. (ret=%d)\n", ret);
        goto OUT;
    }
    ret = crypto_hash_update(&desc, &sg, data_len);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The crypto_hash_update function failed. (ret=%d)\n", ret);
        goto OUT;
    }
    ret = crypto_hash_final(&desc, sha256_code);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The crypto_hash_final function failed. (ret=%d)\n", ret);
    }
OUT:
    crypto_free_hash(desc.tfm);
    return ret;
}
#endif

/*
 *  get user configure partition name by device id
 */
#ifndef CFG_USER_CFG_SUPPORT_SEC_FLASH
STATIC s32 devdrv_config_get_partition_name(u32 dev_id, const u8 *name, u32 name_size)
{
    s32 ret;

    if (name_size > MAX_MTD_NAME) {
        DEV_USER_CFG_ERR("The name size is incorrect. (device_id=%u; name_size=%u)\n", dev_id, name_size);
        return UC_ERR_PARA;
    }

#ifdef CFG_SOC_PLATFORM_CLOUD
    switch (dev_id) {
        case DEVDRV_UC_DEV_ID_0:
            ret = strcpy_s((char *)name, name_size, USER_CON_P0);
            break;
        case DEVDRV_UC_DEV_ID_1:
            ret = strcpy_s((char *)name, name_size, USER_CON_P1);
            break;
        case DEVDRV_UC_DEV_ID_2:
            ret = strcpy_s((char *)name, name_size, USER_CON_P2);
            break;
        case DEVDRV_UC_DEV_ID_3:
            ret = strcpy_s((char *)name, name_size, USER_CON_P3);
            break;
        default:
            DEV_USER_CFG_ERR("The device id is incorrect. (device_id=%u)\n", dev_id);
            return UC_ERR_PARA;
    }
#else
    ret = strcpy_s((char *)name, name_size, PART_NAME_RESERVE_1);
#endif
    if (ret != 0) {
        DEV_USER_CFG_ERR("The strcpy_s function failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return ret;
}
#endif

STATIC s32 devdrv_config_read_block(u32 dev_id, u32 block, u8 *buf, u32 buf_size)
{
    int ret;
    u32 offset = 0;

#ifndef CFG_USER_CFG_SUPPORT_SEC_FLASH
    size_t retlen = 0;
    u8 partition_name[MAX_MTD_NAME] = {0};

    if (block >= UC_FLASH_PARTITION_NUM) {
        DEV_USER_CFG_ERR("The block of device id is out of range. (device_id=%u; block=%u)\n", dev_id, block);
        return UC_ERR_PARA;
    } else if (block >= UC_FLASH_PARTITION_MAIN_NUM) {
        offset = (UC_FLASH_BK_ADDR - UC_FLASH_MAIN_ADDR) +
            (DEVDRV_FLASH_BLOCK_SIZE * (block - UC_FLASH_PARTITION_MAIN_NUM));
    } else {
        offset = DEVDRV_FLASH_BLOCK_SIZE * block;
    }

    ret = devdrv_config_get_partition_name(dev_id, (const u8 *)partition_name, sizeof(partition_name));
    if (ret < 0) {
        DEV_USER_CFG_ERR("The devdrv_config_get_partition_name failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = hisi_sfc_ctl_flash_read((const unsigned char *)partition_name, offset, buf_size, &retlen, buf);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The hisi_sfc_ctl_flash_read failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }
#else

    if (block >= UC_FLASH_PARTITION_NUM) {
        DEV_USER_CFG_ERR("The block of device id is out of range. (device_id=%u; block=%u)\n", dev_id, block);
        return UC_ERR_PARA;
    } else if (block >= UC_FLASH_PARTITION_MAIN_NUM) {
        offset = UC_FLASH_BK_ADDR + (DEVDRV_FLASH_BLOCK_SIZE * (block - UC_FLASH_PARTITION_MAIN_NUM));
    } else {
        offset = UC_FLASH_MAIN_ADDR + (DEVDRV_FLASH_BLOCK_SIZE * block);
    }
#if !defined(CFG_SOC_PLATFORM_MDC_LITE_ESL) && !defined(CFG_SOC_MDC_V51_LITE)
    ret = sec_flash_read(dev_id, offset, buf, buf_size);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The sec_flash_read failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return UC_ERR_FLASH_OP_FAIL;
    }
#else
    ret = 0;
#endif
#endif

    return ret;
}

#ifndef CFG_USER_CFG_SUPPORT_SEC_FLASH
STATIC s32 devdrv_config_erase_block(u32 dev_id, u32 block)
{
    int ret;
    u32 offset = 0;

    u8 partition_name[MAX_MTD_NAME] = {0};

    if (block >= UC_FLASH_PARTITION_NUM) {
        DEV_USER_CFG_ERR("The block of device id is out of range. (device_id=%u; block=%u)\n", dev_id, block);
        return UC_ERR_PARA;
    }

    offset = DEVDRV_FLASH_BLOCK_SIZE * block;

    ret = devdrv_config_get_partition_name(dev_id, (const u8 *)partition_name, sizeof(partition_name));
    if (ret < 0) {
        DEV_USER_CFG_ERR("The devdrv_config_get_partition_name failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    DEV_USER_CFG_EVENT("The block of device id flash erase. (device_id=%u; block=%u)\n", dev_id, block);

    ret = hisi_sfc_ctl_flash_erase((const u8 *)partition_name, offset, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The hisi_sfc_ctl_flash_erase buff failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return ret;
}
#endif

STATIC s32 devdrv_config_write_block(u32 dev_id, u32 block, const u8 *buf, u32 buf_size)
{
#if !defined(CFG_SOC_PLATFORM_MDC_LITE_ESL) && !defined(CFG_SOC_MDC_V51_LITE)
    int ret;
#endif
    u32 offset = 0;

#ifndef CFG_USER_CFG_SUPPORT_SEC_FLASH
    size_t retlen = 0;
    u8 partition_name[MAX_MTD_NAME] = {0};

    if (block >= UC_FLASH_PARTITION_NUM) {
        DEV_USER_CFG_ERR("The block of device id is out of range. (device_id=%u; block=%u)\n", dev_id, block);
        return UC_ERR_PARA;
    } else if (block >= UC_FLASH_PARTITION_MAIN_NUM) {
        offset = (UC_FLASH_BK_ADDR - UC_FLASH_MAIN_ADDR) +
            (DEVDRV_FLASH_BLOCK_SIZE * (block - UC_FLASH_PARTITION_MAIN_NUM));
    } else {
        offset = DEVDRV_FLASH_BLOCK_SIZE * block;
    }

    ret = devdrv_config_erase_block(dev_id, block);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_erase_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = devdrv_config_get_partition_name(dev_id, (const u8 *)partition_name, sizeof(partition_name));
    if (ret < 0) {
        DEV_USER_CFG_ERR("The devdrv_config_get_partition_name failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = hisi_sfc_ctl_flash_write((const u8 *)partition_name, offset, buf_size, &retlen, buf);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The hisi_sfc_ctl_flash_write failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }
#else
    if (block >= UC_FLASH_PARTITION_NUM) {
        DEV_USER_CFG_ERR("The block of device id is out of range. (device_id=%u; block=%u)\n", dev_id, block);
        return UC_ERR_PARA;
    } else if (block >= UC_FLASH_PARTITION_MAIN_NUM) {
        offset = UC_FLASH_BK_ADDR + (DEVDRV_FLASH_BLOCK_SIZE * (block - UC_FLASH_PARTITION_MAIN_NUM));
    } else {
        offset = UC_FLASH_MAIN_ADDR + (DEVDRV_FLASH_BLOCK_SIZE * block);
    }

    DEV_USER_CFG_INFO("The sec_flash_write of device id. (device_id=%u)\n", dev_id);
#if !defined(CFG_SOC_PLATFORM_MDC_LITE_ESL) && !defined(CFG_SOC_MDC_V51_LITE)
    ret = sec_flash_write(dev_id, offset, buf, buf_size);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The sec_flash_write failed. (device_id=%u; ret=%d)", dev_id, ret);
        return UC_ERR_FLASH_OP_FAIL;
    }
#endif
#endif
    return UC_OK;
}

STATIC s32 devdrv_config_read_check_code(const u8 *buf, u32 buf_size, u8 *check, u32 check_size)
{
    u8 *check_code = NULL;
    s32 ret;

    if (buf_size != DEVDRV_FLASH_BLOCK_SIZE) {
        DEV_USER_CFG_ERR("Input buf size is invalid. (buf_size=%u; must_be=%d)\n",
            buf_size, DEVDRV_FLASH_BLOCK_SIZE);
        return UC_ERR_PARA;
    }

    if (check_size != DEVDRV_SHA256_DIGEST) {
        DEV_USER_CFG_ERR("Input check size is invalid. (check_size=%u; must_be=%d)\n",
            check_size, DEVDRV_SHA256_DIGEST);
        return UC_ERR_PARA;
    }

    /*
     * check_code offset is 64 * 1024 - 4 - 32
     * check_code is a uint8_t array which has 32 elements
     */
    check_code = (u8 *)(((uintptr_t)buf) + UC_CHECK_FLASH_OFFSET);

    ret = memcpy_s(check, DEVDRV_SHA256_DIGEST, check_code, DEVDRV_SHA256_DIGEST);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The memcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }

    return UC_OK;
}

STATIC s32 devdrv_config_read_valid_flag(const u8 *buf, u32 buf_size, u32 *valid_flag)
{
    u32 *flag = NULL;

    if (buf_size != DEVDRV_FLASH_BLOCK_SIZE) {
        DEV_USER_CFG_ERR("Input buf size is invalid. (buf_size=%u; must_be=%d)\n",
            buf_size, DEVDRV_FLASH_BLOCK_SIZE);
        return UC_ERR_PARA;
    }

    /*
     * valid flag offset is 64 * 1024 - 4
     * valid flag is a uint32_t
     */
    flag = (uint32_t *)(((uintptr_t)buf) + UC_VALID_FLASH_OFFSET);

    *valid_flag = *flag;
    return UC_OK;
}

STATIC s32 devdrv_config_calculate_check_code(const u8 *buf, u32 buf_size, u8 *check, u32 check_size)
{
    u32 size;
    s32 ret;

    if (buf_size != DEVDRV_FLASH_BLOCK_SIZE) {
        DEV_USER_CFG_ERR("Input buf size is invalid. (buf_size=%u; must_be=%d)\n",
            buf_size, DEVDRV_FLASH_BLOCK_SIZE);
        return UC_ERR_PARA;
    }

    if (check_size != DEVDRV_SHA256_DIGEST) {
        DEV_USER_CFG_ERR("Input check size is invalid. (check_size=%u; must_be=%d)\n",
            check_size, DEVDRV_SHA256_DIGEST);
        return UC_ERR_PARA;
    }

    size = UC_CHECK_FLASH_OFFSET;
    ret = devdrv_calculate_block_sha256(buf, size, check, DEVDRV_SHA256_DIGEST);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_calculate_block_sha256 failed. (ret=%d)\n", ret);
        return ret;
    }

    return UC_OK;
}

STATIC s32 devdrv_config_set_check_code(u8 *buf, u32 buf_size)
{
    u8 check[DEVDRV_SHA256_DIGEST] = {0};
    u8 *data = NULL;
    u32 size;
    s32 ret;
    s32 i;

    if (buf_size != DEVDRV_FLASH_BLOCK_SIZE) {
        DEV_USER_CFG_ERR("Input buf size is invalid. (buf_size=%u; must_be=%d)\n",
            buf_size, DEVDRV_FLASH_BLOCK_SIZE);
        return UC_ERR_PARA;
    }

    size = UC_CHECK_FLASH_OFFSET;
    ret = devdrv_calculate_block_sha256(buf, size, check, DEVDRV_SHA256_DIGEST);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_calculate_block_sha256 failed. (ret=%d)\n", ret);
        return ret;
    }

    data = (u8 *)((uintptr_t)buf + UC_CHECK_FLASH_OFFSET);

    for (i = 0; i < DEVDRV_SHA256_DIGEST; ++i) {
        data[i] = check[i];
    }

    return UC_OK;
}

STATIC s32 devdrv_config_str2hex(const char *str, u8 *hex, u32 size)
{
    const s8 *id_pos = NULL;
    s32 len;
    s32 ret;
    s32 i;

    len = strlen(str);
    if ((len < DEVDRV_UC_DATT_LEN_MIN) || (len > UC_ITEM_DATA_MAX_LEN)) {
        DEV_USER_CFG_ERR("The default data len is out of range. (len=%d)\n", len);
        return UC_ERR_PARA;
    }

    if (len % DEVDRV_UC_DATA_NUM_EVEN != 0) {
        DEV_USER_CFG_ERR("Default data must own a even number length. (len=%d)\n", len);
        return UC_ERR_PARA;
    }

    if (str[0] != '0' || (str[1] != 'x' && str[1] != 'X')) {
        DEV_USER_CFG_ERR("Default data is not start with \"0x\" or \"0X\". (str[0]=%c; str[1]=%c)\n",
            str[0], str[1]);
        return UC_ERR_PARA;
    }

    id_pos = str + DEVDRV_UC_DATA_NUM_EVEN;
    len -= DEVDRV_UC_DATA_NUM_EVEN; /* remove prefix "0x" */
    len /= DEVDRV_UC_DATA_NUM_EVEN; /* figure out number of hex */

    if ((id_pos == NULL) || ((int)size < len)) {
        DEV_USER_CFG_ERR("Default data has len hex, but output buf_size is only size. (len=%d; size=%u)\n", len, size);
        return UC_ERR_PARA;
    }

    for (i = 0; i < len; ++i) {
        ret = sscanf_s(id_pos, "%02hhx", &hex[i]);
        id_pos += DEVDRV_UC_DATA_NUM_EVEN;
        if ((id_pos == NULL) || (ret != 1)) {
            DEV_USER_CFG_ERR("The scanf_s failed. (ret=%d)\n", ret);
            return UC_ERR_PARA;
        }
    }

    ret = memset_s(&hex[len], UC_ITEM_DATA_MAX_LEN - len, 0, size);
    if (ret) {
        DEV_USER_CFG_ERR("The memset_s failed. (ret=%d)\n", ret);
        return ret;
    }

    return UC_OK;
}

/*
 * if current board_id is equal to default user config item,
 * match will set into 1
 */
STATIC s32 devdrv_config_get_default_extract(const struct user_config_item *item, u8 *buf, u32 *buf_size)
{
    u8 *hex = NULL;
    u32 i;
    s32 ret;

    hex = ka_vmalloc(UC_ITEM_DATA_MAX_LEN, GFP_KERNEL | __GFP_ACCOUNT, PAGE_KERNEL);
    if (hex == NULL) {
        DEV_USER_CFG_ERR("Alloc memory for buffer failed.\n");
        return UC_ERR_MEM_ALLOC;
    }

    for (i = 0; i < UC_ITEM_DATA_MAX_LEN; ++i) {
        hex[i] = 0xFF;
    }

    if ((int)*buf_size < (item->len - 1)) {
        DEV_USER_CFG_ERR("The input length is too small. (buff_size=%u; least=%d)\n", *buf_size, (item->len - 1));
        ret = UC_ERR_PARA;
        goto out;
    }

    *buf_size = item->len - 1;

    ret = devdrv_config_str2hex(item->default_data, hex, *buf_size);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_str2hex failed. (ret=%d)\n", ret);
        goto out;
    }

    for (i = 0; i < *buf_size; ++i) {
        buf[i] = hex[i];
    }

out:
    vfree(hex);
    hex = NULL;
    return ret;
}

STATIC s32 devdrv_config_get_default(const struct user_config_item *item, u8 *buf, u32 *buf_size)
{
    s32 ret;
    int i;

    for (i = 0; i < UC_ITEM_MAX_NUM; i++) {
        if (g_user_config_item_default_func_list[i].cfg_name != NULL &&
            strcmp(g_user_config_item_default_func_list[i].cfg_name, item->name) == 0) {
            if (g_user_config_item_default_func_list[i].get_para_post_process_func == NULL) {
                continue;
            }
            ret = g_user_config_item_default_func_list[i].get_para_post_process_func(buf, buf_size, item);
            if (ret != 0) {
                DEV_USER_CFG_ERR("Faild in excute get_para_post_process_func. (item_name=%s; ret=%d)\n",
                    item->name, ret);
            }
            return ret;
        }
    }

    ret = devdrv_config_get_default_extract(item, buf, buf_size);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The devdrv_config_get_default_extract failed. (ret=%d)\n", ret);
        return ret;
    }

    return UC_OK;
}

#ifdef CFG_FEATURE_CPU_NUM_CFG
STATIC s32 devdrv_config_get_default_dts_cpu_config(u8 *buf, u32 *buf_size, const struct user_config_item *item)
{
    int ret;
    int ctrl_cpu_num = 0, data_cpu_num = 0, ai_cpu_num = 0;
    int ctrl_cpu_num_index, data_cpu_num_index, ai_cpu_num_index;
    get_dts_cpu_config_handler_t get_dts_cpu_config_handle = NULL;

    ctrl_cpu_num_index = 0;
    data_cpu_num_index = ctrl_cpu_num_index + 1;
    ai_cpu_num_index = data_cpu_num_index + 1;

    get_dts_cpu_config_handle =
        (get_dts_cpu_config_handler_t)(uintptr_t)__symbol_get(ASCEND_CTL_GET_DTS_CPU_CONFIG_FUNC);
    if (get_dts_cpu_config_handle == NULL) {
        DEV_USER_CFG_ERR("The kallsyms_lookup_name return NULL.\n");
        return UC_ERR_PARA;
    }

    ret = get_dts_cpu_config_handle(&ctrl_cpu_num, &data_cpu_num, &ai_cpu_num);
    __symbol_put(ASCEND_CTL_GET_DTS_CPU_CONFIG_FUNC);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The get_dts_cpu_config_handle function fail. (ret=%d)\n", ret);
        return ret;
    }

    if ((int)*buf_size < (item->len - 1)) {
        DEV_USER_CFG_ERR("Input buf size is too small. (buff_size=%u; least=%d)\n", *buf_size, (item->len - 1));
        return UC_ERR_PARA;
    }
    *buf_size = item->len - 1;

    if (ai_cpu_num_index >= (item->len - 1)) {
        DEV_USER_CFG_ERR("Config index is out of range. (ai_cpu_num_index=%d; least=%d)\n", ai_cpu_num_index,
            (item->len - 1));
        return UC_ERR_PARA;
    }

    ret = memset_s(buf, *buf_size, 0, *buf_size);
    if (ret) {
        DEV_USER_CFG_ERR("The memset_s failed. (ret=%d)\n", ret);
        return ret;
    }

    buf[ctrl_cpu_num_index] = ctrl_cpu_num;
    buf[data_cpu_num_index] = data_cpu_num;
    buf[ai_cpu_num_index] = ai_cpu_num;
    return ret;
}
#endif
/*
 * if current board_id is equal to user config item,
 * match will set into 1
 */
STATIC s32 devdrv_config_get_item_extract(const struct user_config_item *item, const u8 *block, u32 block_size, u8 *buf,
                                          u32 *buf_size)
{
    u8 *valid = NULL;
    u8 *raw = NULL;
    u32 i;

    if ((int)*buf_size < (item->len - 1)) {
        DEV_USER_CFG_ERR("Input buf size is too small. (buff_size=%u; least=%d)\n", *buf_size, (item->len - 1));
        return UC_ERR_PARA;
    }

    *buf_size = item->len - 1;
    valid = (uint8_t *)((uintptr_t)block + UC_CONFIG_FLASH_OFFSET + item->offset);

    if (*valid != UC_ITEM_VALID_VALUE) {
        DEV_USER_CFG_INFO("The item in flash block is invalid, rewind to get default config.\n");
        /* no need to process if board id match default config */
        return devdrv_config_get_default(item, buf, buf_size);
    }

    /*
     * valid flag owns one byte length, followed by item data
     * data length is configured in user_cfg_version_1
     */
    raw = (uint8_t *)((uintptr_t)valid + 1);

    for (i = 0; i < *buf_size; ++i) {
        buf[i] = raw[i];
    }

    return UC_OK;
}

STATIC s32 devdrv_config_get_item(const u8 *block, u32 block_size, s32 cfg_index, u8 *buf, u32 *buf_size)
{
    if (block_size != DEVDRV_FLASH_BLOCK_SIZE) {
        DEV_USER_CFG_ERR("Input block buffer size is invalid. (block_size=%u; must_be=%d)\n",
            block_size, DEVDRV_FLASH_BLOCK_SIZE);
        return UC_ERR_PARA;
    }

    if ((cfg_index >= UC_ITEM_MAX_NUM) || (cfg_index < 0)) {
        DEV_USER_CFG_ERR("The input cfg_index is incorrect. (cfg_index=%d)\n", cfg_index);
        return UC_ERR_PARA;
    }

    return devdrv_config_get_item_extract(&user_cfg_version_1[cfg_index], block, block_size, buf, buf_size);
}

STATIC s32 devdrv_config_set_item(u8 *block, u32 block_size, s32 cfg_index, const u8 *buf, u32 buf_size, u8 effect)
{
    u8 *valid = NULL;
    u8 *data = NULL;
    u32 offset;
    u32 i;
    s32 ret;

    if (block_size != DEVDRV_FLASH_BLOCK_SIZE) {
        DEV_USER_CFG_ERR("Input block buffer size is invalid. (block_size=%u; must_be=%d)\n",
            block_size, DEVDRV_FLASH_BLOCK_SIZE);
        return UC_ERR_PARA;
    }

    if (cfg_index >= UC_ITEM_MAX_NUM) {
        DEV_USER_CFG_ERR("The input cfg_index is incorrect. (cfg_index=%d)\n", cfg_index);
        return UC_ERR_PARA;
    }

    if ((user_cfg_version_1[cfg_index].len - 1) != buf_size) {
        DEV_USER_CFG_ERR("Input buf size is invalid. (buf_size=%u; must_be=%d)\n",
            buf_size, (user_cfg_version_1[cfg_index].len - 1));
        return UC_ERR_PARA;
    }

    offset = user_cfg_version_1[cfg_index].offset;
    valid = (u8 *)((uintptr_t)block + UC_CONFIG_FLASH_OFFSET + offset);
    data = (u8 *)((uintptr_t)valid + 1);

    for (i = 0; i < buf_size; ++i) {
        data[i] = buf[i];
    }

    *valid = 0;
    if (effect == DEVDRV_FLASH_VALID) {
        *valid = UC_ITEM_VALID_VALUE;
    }

    /*
     * update sha256
     */
    ret = devdrv_config_set_check_code(block, block_size);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_set_check_code failed. (ret=%d)\n", ret);
        return ret;
    }

    return UC_OK;
}

STATIC s32 devdrv_config_check_valid(const u8 *block, u32 block_size, u32 *valid)
{
    u8 calculate_code[DEVDRV_SHA256_DIGEST] = {0};
    u8 check_code[DEVDRV_SHA256_DIGEST] = {0};
    u32 valid_flag;
    s32 ret;

    if (block_size != DEVDRV_FLASH_BLOCK_SIZE) {
        DEV_USER_CFG_ERR("Input block buffer size is invalid. (block_size=%u; must_be=%d)\n",
            block_size, DEVDRV_FLASH_BLOCK_SIZE);
        return UC_ERR_PARA;
    }

    ret = devdrv_config_read_valid_flag(block, block_size, &valid_flag);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_read_valid_flag failed. (ret=%d)\n", ret);
        return ret;
    }

    if (valid_flag != UC_VALID_FLAG_VALUE) {
        DEV_USER_CFG_WARN("The valid_flag is invalid. (valid_flag=%u; must_be=%u)\n", valid_flag, UC_VALID_FLAG_VALUE);
        *valid = DEVDRV_FLASH_INVALID;
        return UC_OK;
    }

    ret = devdrv_config_read_check_code(block, block_size, check_code, DEVDRV_SHA256_DIGEST);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_read_check_code failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = devdrv_config_calculate_check_code(block, block_size, calculate_code, DEVDRV_SHA256_DIGEST);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_calculate_check_code failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = memcmp(calculate_code, check_code, DEVDRV_SHA256_DIGEST);
    if (ret != 0) {
        DEV_USER_CFG_INFO("The sha256 is not equal to calculation in flash. (ret=%d)\n", ret);
        *valid = DEVDRV_FLASH_INVALID;
        return UC_OK;
    }

    *valid = DEVDRV_FLASH_VALID;
    return UC_OK;
}

/*
 *  description: check the input block data is valid or not, the block must be the head block
 */
STATIC s32 devdrv_config_check_blk_validity(u32 dev_id, u32 blk_id, u8 *blk_data, u32 *valid)
{
    s32 ret;

    ret = devdrv_config_read_block(dev_id, blk_id, blk_data, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; blk_id=%u; ret=%d)\n",
            dev_id, blk_id, ret);
        return ret;
    }

    ret = devdrv_config_check_valid(blk_data, DEVDRV_FLASH_BLOCK_SIZE, valid);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_check_valid failed. (device_id=%u; blk_id=%u; ret=%d)\n",
            dev_id, blk_id, ret);
        return ret;
    }

    return ret;
}

/*
 *  description: get valid block and sync valid block to another not valid one
 */
STATIC s32 devdrv_config_get_valid_block(u32 dev_id, u8 *block, u32 *valid, u32 *valid_block)
{
    s32 ret;
    u32 main_valid = DEVDRV_FLASH_INVALID;
    u32 bk_valid = DEVDRV_FLASH_INVALID;
    u32 sync_flg = DEVDRV_UC_NO_NEED_SYNC;
    u32 sync_block = 0;

    *valid = DEVDRV_FLASH_HEAD_INVALID;
    ret = devdrv_config_check_blk_validity(dev_id, DEVDRV_FLASH_FIRST_BLOCK, block, &main_valid);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_check_blk_validity failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = devdrv_config_check_blk_validity(dev_id, DEVDRV_FLASH_SECOND_BLOCK, block, &bk_valid);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_check_blk_validity failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    DEV_USER_CFG_INFO("The main valid, backup valid. (device_id=%u; main_valid=%u; bk_valid=%u)\n",
        dev_id, main_valid, bk_valid);

    /*
     * note: when main head and backup head are all valid, maybe their data are not the same.
     * so when getting item info from head, we must notice this condition
     */
    *valid_block = DEVDRV_FLASH_FIRST_BLOCK;
    if ((main_valid == DEVDRV_FLASH_VALID) && (bk_valid == DEVDRV_FLASH_VALID)) {
        *valid = DEVDRV_FLASH_HEAD_ALL_VALID;
    } else if ((main_valid == DEVDRV_FLASH_VALID) && (bk_valid == DEVDRV_FLASH_INVALID)) {
        *valid = DEVDRV_FLASH_MAIN_HEAD_VALID;
        sync_flg = DEVDRV_UC_NEED_SYNC;
        sync_block = DEVDRV_FLASH_SECOND_BLOCK;
    } else if ((main_valid == DEVDRV_FLASH_INVALID) && (bk_valid == DEVDRV_FLASH_VALID)) {
        *valid = DEVDRV_FLASH_BK_HEAD_VALID;
        *valid_block = DEVDRV_FLASH_SECOND_BLOCK;
        sync_flg = DEVDRV_UC_NEED_SYNC;
        sync_block = DEVDRV_FLASH_FIRST_BLOCK;
    } else {
        /*
         * do not return error here
         * because user may never flash user config into flash chip before
         */
        DEV_USER_CFG_INFO("Can not find any valid block, maybe user config is not exist in flash.\n");
    }

    /* read valid block data and sync main and backup data */
    if (*valid != DEVDRV_FLASH_HEAD_INVALID) {
        ret = devdrv_config_read_block(dev_id, *valid_block, block, DEVDRV_FLASH_BLOCK_SIZE);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; block=%u; ret=%d)\n",
                dev_id, *valid_block, ret);
            return ret;
        }

        /* if there is an invalid block, sync */
        if (sync_flg == DEVDRV_UC_NEED_SYNC) {
            ret = devdrv_config_write_one_block(dev_id, sync_block, block);
            if (ret) {
                DEV_USER_CFG_ERR("The devdrv_config_write_one_block failed. (device_id=%u; block=%u; ret=%d)\n",
                    dev_id, sync_block, ret);
                return ret;
            }
        }
    }
    return UC_OK;
}

STATIC s32 devdrv_config_init_block(u8 *block, u32 block_size)
{
    u32 *data = NULL;
    uc_blk_used_info_t *used_info = NULL;
    u32 i = 0;
    s32 ret;

    if (block_size != DEVDRV_FLASH_BLOCK_SIZE) {
        DEV_USER_CFG_ERR("Input block buffer size is invalid. (block_size=%u; must_be=%d).\n",
            block_size, DEVDRV_FLASH_BLOCK_SIZE);
        return UC_ERR_PARA;
    }

    ret = memset_s(block, block_size, 0, block_size);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The memset_s failed. (ret=%d)\n", ret);
        return ret;
    }

    /*
     * set version
     */
    data = (u32 *)block;
    *data = CURRENT_USER_CONFIG_VERSION;

    /*
     * set valid flag
     */
    data = (u32 *)((uintptr_t)block + UC_VALID_FLASH_OFFSET);
    *data = UC_VALID_FLAG_VALUE;

    /*
     * set dynamic cfg item head
     */
    for (i = UC_DYNAMIC_CFG_HEAD_OFFSET; i < UC_CHECK_FLASH_OFFSET; i++) {
        /* head len and data set to zero */
        *(block + i) = 0;
    }

    used_info = (uc_blk_used_info_t *)(block + UC_BLK_INFO_OFFSET);
#ifdef CFG_USER_CFG_SUPPORT_MULTI_BLOCK
    for (i = 0; i < UC_FLASH_PARTITION_MAIN_NUM; i++) {
        if (i < UC_ITEM_CONTENT_BLK_START) {
            used_info->index = i;
            used_info->used = DEVDRV_FLASH_VALID;
            used_info->used_len = (u16)(DEVDRV_FLASH_BLOCK_SIZE - 1);
        } else {
            used_info->index = i;
            used_info->used = DEVDRV_FLASH_INVALID;
            used_info->used_len = 0;
        }

        used_info++;
    }
#else
    used_info->index = 0;
    used_info->used = DEVDRV_FLASH_VALID;
    used_info->used_len = UC_ITEM_CONTENT_BLK_OFFSET;
#endif

    return UC_OK;
}

STATIC s32 devdrv_config_update_item(u32 dev_id, u8 *block, u32 block_size, s32 cfg_index, const u8 *buf, u32 buf_size,
                                     u32 effect)
{
    s32 ret;

    ret = devdrv_config_set_item(block, block_size, cfg_index, buf, buf_size, effect);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_set_item failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = devdrv_config_write_blk_ops(dev_id, DEVDRV_FLASH_FIRST_BLOCK, block, block_size, DEVDRV_UC_FLASH_FLAG_ALL);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_write_blk_ops failed. (ret=%d)\n", ret);
        return ret;
    }

    return UC_OK;
}

STATIC s32 devdrv_config_update(u32 dev_id, s32 cfg_index, const u8 *buf, u32 buf_size, u32 effect)
{
    u8 *block = NULL;
    u32 valid_block;
    u32 valid;
    s32 ret;

    block = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (block == NULL) {
        DEV_USER_CFG_ERR("The ka_vmalloc function return NULL. (device_id=%u)\n", dev_id);
        return UC_ERR_MEM_ALLOC;
    }
    ret = devdrv_config_get_valid_block(dev_id, block, &valid, &valid_block);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_get_valid_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto OUT;
    }
    if (valid == DEVDRV_FLASH_HEAD_INVALID) {
        valid_block = DEVDRV_FLASH_FIRST_BLOCK;
        DEV_USER_CFG_WARN("Init new block data. (device_id=%u)\n", dev_id);
        ret = devdrv_config_init_block(block, DEVDRV_FLASH_BLOCK_SIZE);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_init_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
            goto OUT;
        }
    }
    ret = devdrv_config_update_item(dev_id, block, DEVDRV_FLASH_BLOCK_SIZE, cfg_index, buf, buf_size, effect);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_update_item failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto OUT;
    } else {
        DEV_USER_CFG_EVENT("Config update success. (device_id=%u; config_index=%d)\n", dev_id, cfg_index);
    }

OUT:
    vfree(block);
    block = NULL;
    return ret;
}

s32 devdrv_get_user_config_core(u32 dev_id, s32 cfg_index, u8 *buf, u32 *buf_size)
{
    u8 *block = NULL;
    u32 valid_block;
    u32 valid;
    s32 ret;

    if ((buf == NULL) || (buf_size == NULL)) {
        DEV_USER_CFG_ERR("Input buf handle is NULL. (device_id=%u)\n", dev_id);
        return UC_ERR_PARA;
    }

    block = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (block == NULL) {
        DEV_USER_CFG_ERR("The ka_vmalloc function return NULL. (device_id=%u)\n", dev_id);
        return UC_ERR_MEM_ALLOC;
    }

    /*
     * get a valid block
     * not care about which block is valid
     */
    ret = devdrv_config_get_valid_block(dev_id, block, &valid, &valid_block);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_get_valid_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto OUT;
    }
    if (valid == DEVDRV_FLASH_HEAD_INVALID) {
        vfree(block);
        block = NULL;
        /* get default config */
        DEV_USER_CFG_WARN("No valid flash block is found, "
                          "user config may be not existent, get default config.\n");
        if ((cfg_index >= UC_ITEM_MAX_NUM) || (cfg_index < 0)) {
            DEV_USER_CFG_ERR("Invalid cfg_index. (cfg_index=%d)\n", cfg_index);
            return UC_ERR_PARA;
        }
        return devdrv_config_get_default(&user_cfg_version_1[cfg_index], buf, buf_size);
    }
    /* get item from block data */
    ret = devdrv_config_get_item(block, DEVDRV_FLASH_BLOCK_SIZE, cfg_index, buf, buf_size);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_get_item failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto OUT;
    }

OUT:
    vfree(block);
    block = NULL;
    return ret;
}
EXPORT_SYMBOL(devdrv_get_user_config_core);

STATIC s32 check_config_para(struct user_cfg_ioctl_para *config_para)
{
    int ret;
    unsigned int dev_num = 0;

    ret = devdrv_config_get_dev_num(&dev_num);
    if ((ret != 0) || (dev_num > DEVDRV_UC_CHIP_MAX)) {
        DEV_USER_CFG_ERR("Get device num failed. (device_id=%u; dev_num=%u; ret=%d)\n",
            config_para->dev_id, dev_num, ret);
        return UC_ERR_PARA;
    }

    if (config_para->dev_id >= dev_num) {
        DEV_USER_CFG_ERR("The device id is incorrect. (device_id=%u)\n", config_para->dev_id);
        return UC_ERR_PARA;
    }

    if (config_para->cmd != DEVDRV_FLASH_CONFIG_CLEAR_CMD) {
        if ((config_para->buf_size == 0) || (config_para->buf_size > UC_ITEM_DATA_MAX_LEN) ||
            (config_para->buf == NULL)) {
            DEV_USER_CFG_ERR("The buf parameteris incorrect. (device_id=%u; buf_size=%u)\n",
                config_para->dev_id, config_para->buf_size);
            return UC_ERR_PARA;
        }
    }

    if ((config_para->cfg_index < UC_CFG_INDEX_DYNAMIC) || (config_para->cfg_index >= UC_ITEM_MAX_NUM)) {
        DEV_USER_CFG_ERR("The config index out of range. (device_id=%u; cfg_index=%d)\n",
            config_para->dev_id, config_para->cfg_index);
        return UC_ERR_PARA;
    }

    return UC_OK;
}

STATIC s32 check_authority_for_static_item(devdrv_cfg_para_t *config_para, u32 *is_root_item)
{
    *is_root_item = 0;
    if (user_cfg_version_1[config_para->cfg_index].authority_flag == UC_AUTHORITY_ROOT_WR) {
        /* first judge root authority */
        if (!capable(CAP_SYS_ADMIN)) {
            DEV_USER_CFG_ERR("Check write authority for root failed.\n");
            return UC_ERR_NO_AUTHORITY;
        }

        *is_root_item = 1;
    }
    if ((user_cfg_version_1[config_para->cfg_index].authority_flag == UC_AUTHORITY_USER_RO) &&
        ((config_para->cmd == DEVDRV_FLASH_CONFIG_WRITE_CMD) || (config_para->cmd == DEVDRV_FLASH_CONFIG_CLEAR_CMD))) {
        DEV_USER_CFG_ERR("The config authority error. (cfg_index=%d; cmd=%d)\n",
            config_para->cfg_index, config_para->cmd);
        return UC_ERR_NO_AUTHORITY;
    }
    return UC_OK;
}

/*
 * Get user cfg name info
 */
STATIC s32 devdrv_get_usr_cfg_name_info(uc_item_info_t *item_info, const s8 *name)
{
    s32 found = 0;
    u32 i = 0;
    u32 *head_len = NULL;
    u32 head_num;
    uc_cfg_head_t *cfg_head_pt = NULL;
    /* get total head length and head number */
    head_len = (u32 *)(item_info->head_blk + UC_CFG_HEAD_LEN_OFFSET);
    *head_len = *head_len - (*head_len % (sizeof(uc_cfg_head_t)));
    head_num = *head_len / sizeof(uc_cfg_head_t);

    if (head_num > DEVDRV_UC_HEAD_NUM_MAX) {
        DEV_USER_CFG_ERR("The head num error. (head_num=%u)\n", head_num);
        return UC_ERR_HEAD_NUM;
    }

    cfg_head_pt = (uc_cfg_head_t *)(item_info->head_blk + UC_CFG_HEAD_ITEM_START);

    /* compare item name */
    for (i = 0; i < head_num; i++) {
        if (strncmp((const s8 *)cfg_head_pt->item_name, name, UC_CFG_NAME_LEN_MAX) == 0) {
            if (cfg_head_pt->valid_flg != DEVDRV_FLASH_VALID) {
                DEV_USER_CFG_ERR("Find match item name but flag is invalid. (head_idx=%d, blk_idx=%d, flag=%u)\n",
                                 i, cfg_head_pt->blk_offset, cfg_head_pt->valid_flg);
            } else {
                DEV_USER_CFG_EVENT("Find match item name. head_idx=%d, blk=%d, offset=%d, size=%d\n",
                                   i, cfg_head_pt->blk_offset, cfg_head_pt->item_offset,  cfg_head_pt->item_bytes);
                found = 1;
                item_info->head_index = i;
            }
            break;
        }
        cfg_head_pt++;
    }

    if (found == 1) {
        item_info->head_ops = cfg_head_pt;
        item_info->content_blk_index = cfg_head_pt->blk_offset;
    } else {
        item_info->head_index = DEVDRV_UC_HEAD_NOT_FOUND;
    }

    return UC_OK;
}

/*
 * check cfg head invalid or not
 */
STATIC s32 devdrv_user_cfg_head_check(const uc_cfg_head_t *head_pt)
{
    u32 offset;

    if (head_pt == NULL) {
        DEV_USER_CFG_ERR("The input parameter is NULL.\n");
        return UC_ERR_PARA;
    }

    if (head_pt->blk_offset >= UC_FLASH_PARTITION_MAIN_NUM) {
        DEV_USER_CFG_ERR("The block offset is error. (blk_offset=%u)\n", head_pt->blk_offset);
        return UC_ERR_PARA;
    }

    if (head_pt->item_bytes > UC_ITEM_DATA_MAX_LEN) {
        DEV_USER_CFG_ERR("The item content length is error. (item_bytes=%u)\n", head_pt->item_bytes);
        return UC_ERR_PARA;
    }

    if (head_pt->item_offset >= UC_FLASH_PARTITION_SIZE) {
        DEV_USER_CFG_ERR("The item offset is error. (item_offset=%u)\n", head_pt->item_offset);
        return UC_ERR_PARA;
    }

    offset = head_pt->item_offset + head_pt->item_bytes + DEVDRV_SHA256_DIGEST;
    if (offset >= UC_FLASH_PARTITION_SIZE) {
        DEV_USER_CFG_ERR("The total offset is error. (offset=%u)\n", offset);
        return UC_ERR_PARA;
    }

    if (head_pt->authority_flg > UC_AUTHORITY_USER_RO) {
        DEV_USER_CFG_ERR("The authority value is error. (authority_flg=%u)\n", head_pt->authority_flg);
        return UC_ERR_NO_AUTHORITY;
    }

    if (head_pt->valid_flg != DEVDRV_FLASH_VALID) {
        DEV_USER_CFG_ERR("The valid flag is error. (valid_flg=%u)\n", head_pt->valid_flg);
        return UC_ERR_PARA;
    }

    return UC_OK;
}

/*
 * get cfg head item content
 */
STATIC s32 devdrv_config_get_item_content(u32 dev_id, uc_item_info_t *item_info)
{
    s32 ret;
    u8 *blk_data = NULL;
    u8 calculate_code[DEVDRV_SHA256_DIGEST] = {0};
    u8 *check_pt = NULL;
    u8 *content_pt = NULL;
    uc_cfg_head_t *head_pt = NULL;

    head_pt = item_info->head_ops;
    blk_data = item_info->content_blk;

    ret = devdrv_user_cfg_head_check(head_pt);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_user_cfg_head_check failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = devdrv_config_read_block(dev_id, item_info->content_blk_index, blk_data, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    content_pt = blk_data + head_pt->item_offset;

    /* check sha256 */
    ret = devdrv_calculate_block_sha256(content_pt, head_pt->item_bytes, calculate_code, DEVDRV_SHA256_DIGEST);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_calculate_block_sha256 failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    check_pt = item_info->content_blk + head_pt->item_offset + head_pt->item_bytes;
    ret = memcmp(calculate_code, check_pt, DEVDRV_SHA256_DIGEST);
    if (ret != 0) {
        /* if sha256 check err, check if valid block is first block */
        /* if not ,check backup partition and write it back to main partition */
        if (head_pt->blk_offset < UC_FLASH_PARTITION_MAIN_NUM) {
            ret = UC_ERR_GET_ITEM_FAIL;
        } else {
            ret = UC_ERR_MEM_CMP;
        }

        return ret;
    }

    /* copy data to buff */
    ret = memcpy_s(item_info->content_buf, UC_ITEM_DATA_MAX_LEN, content_pt, head_pt->item_bytes);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The memcpy_s failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return UC_ERR_MEM_CPY;
    }

    if (item_info->content_blk_index >= UC_FLASH_PARTITION_MAIN_NUM) {
        item_info->main_content_wr_flag = 1;
    }

    item_info->content_size = head_pt->item_bytes;

    return UC_OK;
}

/*
 * get cfg head item content
 */
STATIC s32 devdrv_config_get_item_from_head(u32 dev_id, uc_item_info_t *item_info, const char *name)
{
    s32 ret;

    /* check user cfg name and return block info */
    ret = devdrv_get_usr_cfg_name_info(item_info, (const char *)name);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_get_usr_cfg_name_info failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    if (item_info->head_index == DEVDRV_UC_HEAD_NOT_FOUND) {
        DEV_USER_CFG_EVENT("Can not find fix item name %.*s from item head.\n", UC_CFG_NAME_LEN_MAX, name);
        return UC_ERR_HEAD_NOT_FOUND;
    }

    ret = devdrv_config_get_item_content(dev_id, item_info);
    if (ret == UC_ERR_GET_ITEM_FAIL) {
        DEV_USER_CFG_ERR("The devdrv_config_get_item_content get content blk %d fail.\n", item_info->content_blk_index);
        if (item_info->valid_blk == DEVDRV_FLASH_FIRST_BLOCK) {
            item_info->content_blk_index += DEVDRV_FLASH_SECOND_BLOCK;
            DEV_USER_CFG_ERR("The devdrv_config_get_item_content get main fail,try backup\n");
            ret = devdrv_config_get_item_content(dev_id, item_info);
            if (ret) {
                DEV_USER_CFG_ERR("The devdrv_config_get_item_content get backup fail. (device_id=%u; ret=%d)\n",
                                 dev_id, ret);
                return ret;
            }
        }
    } else if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_get_item_content failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return UC_OK;
}

#ifdef CFG_USER_CFG_SUPPORT_COMPACT_SPACE
/* In the same block, the data is sorted by offset form low to high.
   In the different block, the data is sorted by block form low to high. */
STATIC s32 item_compare(const void *p1, const void *p2)
{
    uc_cfg_item_info_t *item1 = (uc_cfg_item_info_t *)p1;
    uc_cfg_item_info_t *item2 = (uc_cfg_item_info_t *)p2;
    if (item1->blk_offset == item2->blk_offset) {
        return item1->start_offset - item2->start_offset;
    }
    return item1->blk_offset - item2->blk_offset;
}

/* find every bit of space to get the idle position. */
STATIC s32 devdrv_config_find_free_space(u8 *blk_data, uc_cfg_head_t *head_pt)
{
    u32 *head_len = NULL;
    u32 head_num;
    uc_cfg_head_t *headloop = NULL;
    u32 valid_num = 0;
    uc_cfg_item_info_t *item_info = NULL;
    u32 item_num_each_block[UC_FLASH_PARTITION_MAIN_NUM + 1] = {0};
    u32 want_len = head_pt->item_bytes + DEVDRV_SHA256_DIGEST;
    u32 i, j;
    u32 block_start_item = 0;
    u32 block_end_item;
    u32 pre_offset, cur_offset;
    uc_blk_used_info_t *used_info = NULL;
    used_info = (uc_blk_used_info_t *)(blk_data + UC_BLK_INFO_OFFSET);

    /* get total head length and head number */
    head_len = (u32 *)(blk_data + UC_CFG_HEAD_LEN_OFFSET);
    *head_len = *head_len - (*head_len % (sizeof(uc_cfg_head_t)));
    head_num = *head_len / sizeof(uc_cfg_head_t);

    item_info = ka_vmalloc(*head_len, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    headloop = (uc_cfg_head_t *)(blk_data + UC_CFG_HEAD_ITEM_START);
    head_pt->valid_flg = DEVDRV_FLASH_INVALID;
    for (i = 0; i < head_num; i++) {
        if (headloop->valid_flg == DEVDRV_FLASH_VALID) {
            (item_info + valid_num)->blk_offset = headloop->blk_offset;
            (item_info + valid_num)->start_offset = headloop->item_offset;
            (item_info + valid_num)->end_offset = headloop->item_offset + headloop->item_bytes + DEVDRV_SHA256_DIGEST;
            item_num_each_block[headloop->blk_offset]++;
            valid_num++;
        }
        DEV_USER_CFG_DEBUG("item[%d] name[%.*s] vaild[%d] block[%d], start[%d], size[%d], end[%d]\n",
                           i, UC_CFG_NAME_LEN_MAX, headloop->item_name, headloop->valid_flg, headloop->blk_offset,
                           headloop->item_offset, headloop->item_bytes + DEVDRV_SHA256_DIGEST,
                           headloop->item_offset + headloop->item_bytes + DEVDRV_SHA256_DIGEST);
        headloop++;
    }

    /* Sort all items and check whether there is a space between the next two items. */
    sort(item_info, valid_num, sizeof(uc_cfg_item_info_t), item_compare, NULL);
    for (i = UC_ITEM_CONTENT_BLK_START; i < UC_FLASH_PARTITION_MAIN_NUM; i++) {
        pre_offset = 0;
        cur_offset = 0;
        if (item_num_each_block[i] == 0) {
            /* block[i] have no item, item is written from the start address of the block */
            (used_info + i)->used_len = head_pt->item_bytes + DEVDRV_SHA256_DIGEST;
            head_pt->blk_offset = i;
            head_pt->item_offset = 0;
            vfree(item_info);
            return DEVDRV_FLASH_VALID;
        }

        block_end_item = block_start_item + item_num_each_block[i];
        for (j = block_start_item; j < block_end_item; j++) {
            /* Check whether there is space between two items. */
            cur_offset = (item_info + j)->start_offset;
            if (cur_offset - pre_offset >= want_len) {
                head_pt->blk_offset = i;
                head_pt->item_offset = pre_offset;
                vfree(item_info);
                return DEVDRV_FLASH_VALID;
            }
            pre_offset = (item_info + j)->end_offset;
        }
        /* The last item is a virtual item, it's start is the end of the block. */
        cur_offset = DEVDRV_FLASH_BLOCK_SIZE;
        if (cur_offset - pre_offset >= want_len) {
            head_pt->blk_offset = i;
            head_pt->item_offset = pre_offset;
            (used_info + i)->used_len = pre_offset + 1 + head_pt->item_bytes + DEVDRV_SHA256_DIGEST;
            vfree(item_info);
            return DEVDRV_FLASH_VALID;
        }
        block_start_item = block_end_item;
    }
    vfree(item_info);
    return DEVDRV_FLASH_INVALID;
}
#endif
/*
 * init head block data for user cfg partition.
 */
STATIC s32 devdrv_config_get_item_blk_info(u8 *blk_data, uc_cfg_head_t *head_pt)
{
    s32 result = 0;
#ifdef CFG_USER_CFG_SUPPORT_COMPACT_SPACE
    result = devdrv_config_find_free_space(blk_data, head_pt);
#else
    u32 i = 0;
    u32 free_len = 0;
    uc_blk_used_info_t *used_info = NULL;
    used_info = (uc_blk_used_info_t *)(blk_data + UC_BLK_INFO_OFFSET);
    used_info = used_info + UC_ITEM_CONTENT_BLK_START;
    for (i = UC_ITEM_CONTENT_BLK_START; i < UC_FLASH_PARTITION_MAIN_NUM; i++) {
        if (used_info->used == DEVDRV_FLASH_VALID) {
            /* valid content block is first block */
#ifndef CFG_USER_CFG_SUPPORT_MULTI_BLOCK
            free_len = UC_BLK_INFO_OFFSET - used_info->used_len;
#else
            free_len = DEVDRV_FLASH_BLOCK_SIZE - used_info->used_len;
#endif

            if (free_len > (head_pt->item_bytes + DEVDRV_SHA256_DIGEST)) {
                head_pt->blk_offset = i;
                head_pt->item_offset = used_info->used_len;
                used_info->used_len += head_pt->item_bytes + DEVDRV_SHA256_DIGEST;
                result = DEVDRV_FLASH_VALID;
                break;
            }
        } else {
            used_info->used = DEVDRV_FLASH_VALID;
            used_info->used_len = head_pt->item_bytes + DEVDRV_SHA256_DIGEST;
            head_pt->blk_offset = i;
            head_pt->item_offset = 0;
            result = DEVDRV_FLASH_VALID;
            break;
        }

        used_info++;
    }
#endif
    if (result == DEVDRV_FLASH_INVALID) {
        DEV_USER_CFG_ERR("Not enough space.\n");
        return UC_ERR_FLASH_FULL;
    } else {
        DEV_USER_CFG_EVENT("Find free space for content block success. (block=%u offset=%u size=%u)\n",
                           head_pt->blk_offset, head_pt->item_offset, head_pt->item_bytes + DEVDRV_SHA256_DIGEST);
    }

    return UC_OK;
}

/*
 * get a new item head and set cfg data into head info
 */
STATIC s32 devdrv_config_set_new_head(devdrv_cfg_para_t *config_para, u8 *block, uc_cfg_head_t **head)
{
    s32 ret;
    u32 i = 0;
    u32 head_num;
    u32 *item_head_len = NULL;
    uc_cfg_head_t *item_head = NULL;
    u32 valid_flag = DEVDRV_FLASH_INVALID;

    /* get item head total len */
    item_head_len = (u32 *)(block + UC_CFG_HEAD_LEN_OFFSET);
    item_head = (uc_cfg_head_t *)(block + UC_CFG_HEAD_ITEM_START);
    head_num = (*item_head_len) / sizeof(uc_cfg_head_t);

    if (*item_head_len < (DEVDRV_UC_HEAD_LEN_MAX - sizeof(uc_cfg_head_t))) {
        for (i = 0; i < head_num; i++) {
            if (item_head->valid_flg != DEVDRV_FLASH_VALID) {
                valid_flag = DEVDRV_FLASH_VALID;
                break;
            }

            item_head++;
        }

        if (valid_flag != DEVDRV_FLASH_VALID) {
            item_head = (uc_cfg_head_t *)(block + UC_CFG_HEAD_ITEM_START + *item_head_len);
            *item_head_len = *item_head_len + sizeof(uc_cfg_head_t);
        }

        item_head->item_bytes = config_para->buf_size;
        ret = memcpy_s(item_head->item_name, UC_CFG_NAME_LEN_MAX, config_para->name, UC_CFG_NAME_LEN_MAX);
        if (ret != 0) {
            DEV_USER_CFG_ERR("The memcpy_s failed. (ret=%d)\n", ret);
            return ret;
        }
    } else {
        DEV_USER_CFG_ERR("Item head too long. (head_length=%u)\n", *item_head_len);
        return UC_ERR_HEAD_NUM;
    }

    *head = item_head;
    return UC_OK;
}

/*
 * add user cfg item head
 */
STATIC s32 devdrv_config_add_item_head(devdrv_cfg_para_t *config_para, uc_item_info_t *item_info)
{
    s32 ret;
    uc_cfg_head_t head_tmp = {0};
    uc_cfg_head_t *add_head_pt = NULL;
    u8 *block = item_info->head_blk;
    u32 content_sync_flag = DEVDRV_UC_NOT_SYNC;

    head_tmp.item_bytes = config_para->buf_size;

    /* get item content info */
    ret = devdrv_config_get_item_blk_info(block, &head_tmp);
    if (ret) {
        /* if block space is not enough, rebuild content and try again */
        /* Note: head data can't had been changed before rebuild */
        DEV_USER_CFG_EVENT("block space is not enough, rebuild content and try again\n");
        ret = devdrv_config_rebuild_item_content(config_para->dev_id, block);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_rebuild_item_content failed. (ret=%d)\n", ret);
            return ret;
        }

        content_sync_flag = DEVDRV_UC_SYNC_DONE;
        ret = devdrv_config_get_item_blk_info(block, &head_tmp);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_uc_get_item_blk_info failed. (ret=%d)\n", ret);
            return ret;
        }
    }

    /* sync content block */
#ifdef CFG_USER_CFG_SUPPORT_MULTI_BLOCK
    if (content_sync_flag == DEVDRV_UC_NOT_SYNC) {
        ret = devdrv_config_sync_main_backup(config_para->dev_id, block, DEVDRV_FLASH_BLOCK_SIZE,
                                             DEVDRV_UC_SYNC_ONE_BLOCK, head_tmp.blk_offset);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_set_check_code failed. (ret=%d)\n", ret);
            return ret;
        }
    }
#endif

    /* get a new head and set configure info into head position */
    ret = devdrv_config_set_new_head(config_para, block, &add_head_pt);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_set_check_code failed. (ret=%d)\n", ret);
        return ret;
    }

    /* must set valid flag before caculate sha256 */
    add_head_pt->valid_flg = DEVDRV_FLASH_VALID;
    add_head_pt->blk_offset = head_tmp.blk_offset;
    add_head_pt->item_offset = head_tmp.item_offset;

    /* reset sha256 value */
    ret = devdrv_config_set_check_code(block, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_set_check_code failed. (ret=%d)\n", ret);
        return ret;
    }

    item_info->head_ops = add_head_pt;

    return UC_OK;
}

/*
 * write block para check
 */
STATIC s32 devdrv_config_write_blk_check(u32 dev_id, u32 blk_index, u8 *block_data, u32 blk_size, u32 flag)
{
    if (block_data == NULL) {
        DEV_USER_CFG_ERR("Block data is NULL. (device_id=%u)\n", dev_id);
        return UC_ERR_PARA;
    }

    if (dev_id >= DEVDRV_UC_CHIP_MAX) {
        DEV_USER_CFG_ERR("The device ID is not fixed. (device_id=%u)\n", dev_id);
        return UC_ERR_PARA;
    }

    if (((blk_index >= UC_FLASH_PARTITION_MAIN_NUM) && (flag == DEVDRV_UC_FLASH_FLAG_MAIN)) ||
        ((blk_index < UC_FLASH_PARTITION_MAIN_NUM) && (flag == DEVDRV_UC_FLASH_FLAG_BACKUP)) ||
        (blk_index >= UC_FLASH_PARTITION_NUM)) {
        DEV_USER_CFG_ERR("Index and flag not fixed. (blk_index=%u; flag=%u)\n", blk_index, flag);
        return UC_ERR_PARA;
    }

    if (blk_size != DEVDRV_FLASH_BLOCK_SIZE) {
        DEV_USER_CFG_ERR("Block size error. (blk_size=%u)\n", blk_size);
        return UC_ERR_PARA;
    }

    return UC_OK;
}

/*
 * write main and backup partition.
 */
STATIC s32 devdrv_config_write_one_block(u32 dev_id, u32 blk_index, u8 *block_data)
{
    s32 ret;
    u8 *read_buf = NULL;

    read_buf = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (read_buf == NULL) {
        DEV_USER_CFG_ERR("The ka_vmalloc failed. (device_id=%u)\n", dev_id);
        return UC_ERR_MEM_ALLOC;
    }

    ret = devdrv_config_read_block(dev_id, blk_index, read_buf, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto FREE_EXIT;
    }

    if (memcmp(block_data, read_buf, DEVDRV_FLASH_BLOCK_SIZE) == 0) {
        DEV_USER_CFG_INFO("The write data is the same as the current flash data. (device_id=%u)\n", dev_id);
        goto FREE_EXIT;
    }

    ret = devdrv_config_write_block(dev_id, blk_index, block_data, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_write_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto FREE_EXIT;
    }

    /* read data and compare */
    ret = devdrv_config_read_block(dev_id, blk_index, read_buf, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto FREE_EXIT;
    }

    if (memcmp(block_data, read_buf, DEVDRV_FLASH_BLOCK_SIZE)) {
        DEV_USER_CFG_ERR("The memcmp failed. (device_id=%u)\n", dev_id);
        ret = UC_ERR_FLASH_OP_FAIL;
        goto FREE_EXIT;
    }

FREE_EXIT:
    vfree(read_buf);
    read_buf = NULL;
    return ret;
}

/*
 * write main and backup partition.
 */
STATIC s32 devdrv_config_write_blk_ops(u32 dev_id, u32 blk_index, u8 *block_data, u32 blk_size, u32 flag)
{
    s32 ret;
    u32 i = 0;
    u32 cycle_time = 1;
    u32 blk_wr[DEVDRV_UC_DATA_NUM_EVEN] = {0};

    ret = devdrv_config_write_blk_check(dev_id, blk_index, block_data, blk_size, flag);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_write_blk_check failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    if (flag == DEVDRV_UC_FLASH_FLAG_ALL) {
        cycle_time++;
    }

    blk_wr[0] = blk_index;
    if (blk_index >= UC_FLASH_PARTITION_MAIN_NUM) {
        blk_wr[1] = blk_index % UC_FLASH_PARTITION_MAIN_NUM;
    } else {
        blk_wr[1] = blk_index + UC_FLASH_PARTITION_MAIN_NUM;
    }

    for (i = 0; i < cycle_time; i++) {
        ret = devdrv_config_write_one_block(dev_id, blk_wr[i], block_data);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_write_one_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
            return ret;
        }
    }

    return ret;
}

/*
 * add user cfg item content
 */
STATIC s32 devdrv_config_update_item_content(u32 dev_id, uc_item_info_t *item_info)
{
    s32 ret;
    u8 *wr_pos = NULL;
    u32 wr_blk = 0;
    u32 wr_offset;
    u32 wr_len;
    u8 *wr_sha256_pos = {0};
    uc_cfg_head_t *head_pt = item_info->head_ops;

    ret = devdrv_user_cfg_head_check(head_pt);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_user_cfg_head_check failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    if (item_info->valid_blk >= UC_FLASH_PARTITION_MAIN_NUM) {
        wr_blk = head_pt->blk_offset + UC_FLASH_PARTITION_MAIN_NUM;
    } else {
        wr_blk = head_pt->blk_offset;
    }

    item_info->content_blk_index = wr_blk;
    ret = devdrv_config_read_block(dev_id, wr_blk, item_info->content_blk, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    wr_offset = head_pt->item_offset;
    wr_len = head_pt->item_bytes;
    wr_pos = item_info->content_blk + wr_offset;
    wr_sha256_pos = wr_pos + wr_len;

    /* write item content */
    ret = memcpy_s(wr_pos, DEVDRV_FLASH_BLOCK_SIZE - wr_offset, item_info->content_buf, wr_len);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The memcpy_s failed. (device_id=%u; wr_len=%u; ret=%d)\n", dev_id, wr_len, ret);
        return ret;
    }

    /* write sha256 data */
    ret = devdrv_calculate_block_sha256(wr_pos, wr_len, wr_sha256_pos, DEVDRV_SHA256_DIGEST);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_calculate_block_sha256 failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return UC_OK;
}

/*
 * Updated user cfg item head
 */
STATIC s32 devdrv_config_update_item_head(devdrv_cfg_para_t *config_para, uc_item_info_t *item_info)
{
    s32 ret;
    u32 content_sync_flag = DEVDRV_UC_NOT_SYNC;
    uc_cfg_head_t *add_head_pt = NULL;
    u8 *block = item_info->head_blk;
    u32 old_buf_size;
    add_head_pt = item_info->head_ops;
    old_buf_size = add_head_pt->item_bytes;
    add_head_pt->item_bytes = config_para->buf_size;

    /* if config len larger than old, get a new position */
    if (config_para->buf_size > old_buf_size) {
        /* get item content info */
        ret = devdrv_config_get_item_blk_info(block, add_head_pt);
        if (ret) {
            add_head_pt->valid_flg = DEVDRV_FLASH_INVALID;

            /* if block space is not enough, rebuild content and try again */
            DEV_USER_CFG_EVENT("block space is not enough, rebuild content and try again\n");
            ret = devdrv_config_rebuild_item_content(config_para->dev_id, block);
            if (ret) {
                DEV_USER_CFG_ERR("The devdrv_config_rebuild_item_content failed. (ret=%d)\n", ret);
                return ret;
            }

            ret = devdrv_config_get_item_blk_info(block, add_head_pt);
            if (ret) {
                DEV_USER_CFG_ERR("The devdrv_uc_get_item_blk_info failed. (ret=%d)\n", ret);
                return ret;
            }
        }

        add_head_pt->valid_flg = DEVDRV_FLASH_VALID;
        content_sync_flag = DEVDRV_UC_SYNC_DONE;
    }

#ifdef CFG_USER_CFG_SUPPORT_MULTI_BLOCK
    if (content_sync_flag == DEVDRV_UC_NOT_SYNC) {
        /*
         * sync content block data, the setting item must set to invalid status.
         * because sync process will sync valid item.
         */
        DEV_USER_CFG_INFO("Sync content block.\n");
        add_head_pt->valid_flg = DEVDRV_FLASH_INVALID;
        ret = devdrv_config_sync_main_backup(config_para->dev_id, item_info->head_blk, DEVDRV_FLASH_BLOCK_SIZE,
                                             DEVDRV_UC_SYNC_ONE_BLOCK, add_head_pt->blk_offset);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_sync_main_backup failed. (ret=%d)\n", ret);
            return ret;
        }

        add_head_pt->valid_flg = DEVDRV_FLASH_VALID;
    }
#endif

    /* reset sha256 value */
    ret = devdrv_config_set_check_code(item_info->head_blk, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_set_check_code failed. (ret=%d)\n", ret);
        return ret;
    }

    item_info->head_ops = add_head_pt;
    return UC_OK;
}

/*
 * check user cfg item content
 */
STATIC s32 devdrv_config_check_item_content(u32 dev_id, uc_item_info_t *item_info, u32 *result)
{
    s32 ret;
    u32 rd_blk = 0;
    u8 *block = NULL;
    u8 *content_pt = NULL;
    u8 *check_pt = NULL;
    u8 calculate_code[DEVDRV_SHA256_DIGEST] = {0};
    uc_cfg_head_t *head_item = item_info->head_ops;

    ret = devdrv_user_cfg_head_check(head_item);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_user_cfg_head_check failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    if (item_info->valid_blk >= UC_FLASH_PARTITION_MAIN_NUM) {
        rd_blk = head_item->blk_offset + UC_FLASH_PARTITION_MAIN_NUM;
    } else {
        rd_blk = head_item->blk_offset;
    }

    *result = 0;
    block = item_info->content_blk;
    ret = devdrv_config_read_block(dev_id, rd_blk, block, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    content_pt = block + head_item->item_offset;
    ret = memcmp(item_info->content_buf, content_pt, head_item->item_bytes);
    if (ret) {
        DEV_USER_CFG_INFO("Item content is not the same. (device_id=%u; ret=%d)", dev_id, ret);
        return UC_OK;
    }

    /* check sha256 */
    ret = devdrv_calculate_block_sha256(content_pt, head_item->item_bytes, calculate_code, DEVDRV_SHA256_DIGEST);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_calculate_block_sha256 failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    check_pt = item_info->content_blk + head_item->item_offset + head_item->item_bytes;
    ret = memcmp(calculate_code, check_pt, DEVDRV_SHA256_DIGEST);
    if (ret) {
        DEV_USER_CFG_INFO("The sha256 value is incorrect. (device_id=%u; ret=%d)\n", dev_id, ret);
        return 0;
    }

    *result = 1;
    return UC_OK;
}

/*
 *  Initialize cfg block for first time read and write
 */
STATIC s32 devdrv_config_init_block_dynamic(u32 dev_id, u8 *block, u32 block_size, u32 init_flag)
{
    s32 ret;

    ret = devdrv_config_init_block(block, block_size);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_init_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    if (init_flag == DEVDRV_UC_BLOCK_INIT_WR) {
        ret = devdrv_config_write_blk_ops(dev_id, DEVDRV_FLASH_FIRST_BLOCK, block, DEVDRV_FLASH_BLOCK_SIZE,
                                          DEVDRV_UC_FLASH_FLAG_ALL);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_init_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
            return ret;
        }
    }

    return UC_OK;
}

/*
 *  move item content to tmp block space.
 */
STATIC s32 devdrv_config_move_item_content(u8 *content, u8 *tmp_content, uc_cfg_head_t *head, u32 *pos)
{
    int ret;
    u8 *cpy_from = NULL;
    u8 *cpy_to = NULL;
    u32 cpy_len;

    if (*pos >= DEVDRV_FLASH_BLOCK_SIZE) {
        DEV_USER_CFG_ERR("The start pos out of range. (pos=%u)\n", *pos);
        return UC_ERR_PARA;
    }
    ret = devdrv_user_cfg_head_check(head);
    if (ret != UC_OK) {
        DEV_USER_CFG_ERR("The devdrv_user_cfg_head_check failed. (ret=%d)\n", ret);
        return ret;
    }

    cpy_from = content + head->item_offset;
    cpy_to = tmp_content + *pos;
    cpy_len = head->item_bytes + DEVDRV_SHA256_DIGEST;
    ret = memcpy_s(cpy_to, DEVDRV_FLASH_BLOCK_SIZE - *pos, cpy_from, cpy_len);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The memcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }

    head->item_offset = *pos;
    *pos += cpy_len;
    return UC_OK;
}

/*
 *  rebuild block data
 */
STATIC s32 devdrv_config_content_cpy(u8 *content, uc_blk_used_info_t *used_info, u32 blk_index,
                                     uc_cfg_head_t *head, u32 head_num)
{
    s32 ret;
    u8 *tmp_blk = NULL;
    u32 i = 0;
    u32 start_pos = 0;

    tmp_blk = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (tmp_blk == NULL) {
        DEV_USER_CFG_ERR("The ka_vmalloc failed.\n");
        return UC_ERR_MEM_ALLOC;
    }

    if (blk_index == 0) {
        start_pos = UC_ITEM_CONTENT_BLK_OFFSET;
        ret = memcpy_s(tmp_blk, DEVDRV_FLASH_BLOCK_SIZE, content, DEVDRV_FLASH_BLOCK_SIZE);
        if (ret != 0) {
            DEV_USER_CFG_ERR("The memcpy_s failed. (ret=%d)\n", ret);
            goto out;
        }
    }

    for (i = 0; i < head_num; i++) {
        if ((head->blk_offset == blk_index) && (head->valid_flg == DEVDRV_FLASH_VALID)) {
            ret = devdrv_config_move_item_content(content, tmp_blk, head, &start_pos);
            if (ret != UC_OK) {
                DEV_USER_CFG_ERR("The devdrv_config_move_item_content failed. (ret=%d)\n", ret);
                goto out;
            }
        }

        head++;
    }

    /* update head block info */
    used_info->used_len = start_pos;

    ret = memcpy_s(content, DEVDRV_FLASH_BLOCK_SIZE, tmp_blk, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The memcpy_s failed. (ret=%d)\n", ret);
        goto out;
    }

out:
    vfree(tmp_blk);
    tmp_blk = NULL;
    return ret;
}

/*
 *  write rebuild head and content data to flash,
 *  order: 1. main content, block 2-7;
 *         2. main head, block 0;
 *         3. backup content, block 10-15;
 *         4. backup head, block 8.
 *  block 1 and block 9 is resevered, not for dynamic item, no need to write
 */
#ifdef CFG_USER_CFG_SUPPORT_MULTI_BLOCK
STATIC s32 devdrv_config_write_sync_data(u32 dev_id, u8 *head_blk, u8 *content_data)
{
    s32 ret;
    s32 i = 0;
    s32 data_offset;
    u8 wr_order[UC_FLASH_PARTITION_NUM] = {0};
    u32 wr_flag[UC_FLASH_PARTITION_NUM] = {0};
    u8 *wr_data[UC_FLASH_PARTITION_NUM] = {NULL};

    /* reset sha256 value for head block */
    ret = devdrv_config_set_check_code(head_blk, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_set_check_code failed. (ret=%d)\n", ret);
        return ret;
    }

    for (i = 0; i < UC_FLASH_PARTITION_NUM - UC_ITEM_CONTENT_BLK_START; i++) {
        /* fill main content index, write order 0-5 */
        if (i < (UC_FLASH_PARTITION_MAIN_NUM - UC_ITEM_CONTENT_BLK_START)) {
            wr_order[i] = UC_ITEM_CONTENT_BLK_START + i;
            wr_flag[i] = DEVDRV_UC_FLASH_FLAG_MAIN;
            data_offset = (unsigned int)(i * DEVDRV_FLASH_BLOCK_SIZE);
            wr_data[i] = (u8 *)&content_data[data_offset];
        } else if (i == (UC_FLASH_PARTITION_MAIN_NUM - UC_ITEM_CONTENT_BLK_START)) {
            /* main head block index, write order 6 */
            wr_order[i] = 0;
            wr_flag[i] = DEVDRV_UC_FLASH_FLAG_MAIN;
            wr_data[i] = head_blk;
        } else if ((i == UC_FLASH_PARTITION_NUM - UC_ITEM_CONTENT_BLK_START - 1)) {
            /* backup head index, write order 13 */
            wr_order[i] = UC_FLASH_PARTITION_MAIN_NUM;
            wr_flag[i] = DEVDRV_UC_FLASH_FLAG_BACKUP;
            wr_data[i] = head_blk;
        } else if (i > UC_FLASH_PARTITION_MAIN_NUM - UC_ITEM_CONTENT_BLK_START) {
            /* backup content index, write order 7-12 */
            wr_order[i] = UC_ITEM_CONTENT_BLK_START + i + 1;
            wr_flag[i] = DEVDRV_UC_FLASH_FLAG_BACKUP;
            data_offset = (unsigned int)((i + 1 - UC_FLASH_PARTITION_MAIN_NUM) * DEVDRV_FLASH_BLOCK_SIZE);
            wr_data[i] = (u8 *)&content_data[data_offset];
        }
    }

    /* write data to flash according to the filled order */
    for (i = 0; i < UC_FLASH_PARTITION_NUM - UC_ITEM_CONTENT_BLK_START; i++) {
        ret = devdrv_config_write_blk_ops(dev_id, wr_order[i], wr_data[i],
                                          DEVDRV_FLASH_BLOCK_SIZE, wr_flag[i]);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_uc_write_blk_ops failed. (device_id=%u; ret=%d)\n", dev_id, ret);
            return ret;
        }
    }

    return ret;
}

/*
 *  rebuild block data
 */
STATIC s32 devdrv_config_rebuild_item_data(u32 dev_id, u8 *head_blk, u8 *cfg_head, u32 head_len)
{
    s32 ret;
    u32 i = 0;
    u32 head_num;
    u8 *content_data = NULL;
    u8 *content_blk = NULL;
    uc_blk_used_info_t *used_info = NULL;
    uc_cfg_head_t *item_head = (uc_cfg_head_t *)cfg_head;

    /* alloc memory for content */
    head_num = head_len / sizeof(uc_cfg_head_t);
    if (head_num > DEVDRV_UC_HEAD_NUM_MAX) {
        DEV_USER_CFG_ERR("The head num error. (device_id=%u; head_num=%u)\n", dev_id, head_num);
        return UC_ERR_PARA;
    }

    content_data = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE * (UC_FLASH_PARTITION_MAIN_NUM - UC_ITEM_CONTENT_BLK_START),
        GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (content_data == NULL) {
        DEV_USER_CFG_ERR("The ka_vmalloc failed. (device_id=%u)\n", dev_id);
        return UC_ERR_MEM_ALLOC;
    }

    for (i = UC_ITEM_CONTENT_BLK_START; i < UC_FLASH_PARTITION_MAIN_NUM; i++) {
        content_blk = content_data + (u32)((i - UC_ITEM_CONTENT_BLK_START) * DEVDRV_FLASH_BLOCK_SIZE);
        used_info = (uc_blk_used_info_t *)(head_blk + UC_BLK_INFO_OFFSET + (sizeof(uc_blk_used_info_t) * i));

        /* read content block */
        ret = devdrv_config_read_block(dev_id, i, content_blk, DEVDRV_FLASH_BLOCK_SIZE);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
            goto out;
        }

        /* rebuild content */
        ret = devdrv_config_content_cpy(content_blk, used_info, i, item_head, head_num);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_content_cpy failed. (device_id=%u; ret=%d)\n", dev_id, ret);
            goto out;
        }
    }

    /* write back to flash */
    ret = devdrv_config_write_sync_data(dev_id, head_blk, content_data);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_write_sync_data failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto out;
    }

out:
    vfree(content_data);
    content_data = NULL;
    return ret;
}

/*
 *  sync item content
 */
STATIC s32 devdrv_config_cmp_sha256_data(u8 *data, u32 data_len, u8 *in, u32 in_len)
{
    s32 ret;
    u8 sha256_check[DEVDRV_SHA256_DIGEST] = {0};

    if (in_len < DEVDRV_SHA256_DIGEST) {
        DEV_USER_CFG_ERR("Input data len out of range. (input_len=%u)\n", in_len);
        return UC_ERR_PARA;
    }

    ret = devdrv_calculate_block_sha256(data, data_len, sha256_check, DEVDRV_SHA256_DIGEST);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_calculate_block_sha256 failed. (ret=%d)\n", ret);
        return UC_ERR_SHA256_CHECK;
    }

    ret = memcmp(sha256_check, in, DEVDRV_SHA256_DIGEST);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The sha256 data is not equal. (ret=%d)\n", ret);
        return UC_ERR_MEM_CMP;
    }

    return UC_OK;
}

/*
 *  sync item content
 */
STATIC s32 devdrv_config_sync_item_content(u8 *main_blk_pt, u8 *bk_blk_pt, uc_cfg_head_t *item_head,
                                           u32 *main_wr_flag, u32 *bk_wr_flag)
{
    int ret;
    u8 *check_pos_main = main_blk_pt + item_head->item_offset;
    u8 *sha256_pos_main = check_pos_main + item_head->item_bytes;
    u8 *check_pos_bk = bk_blk_pt + item_head->item_offset;
    u8 *sha256_pos_bk = check_pos_bk + item_head->item_bytes;

    if ((item_head->item_offset + item_head->item_bytes + DEVDRV_SHA256_DIGEST) >= DEVDRV_FLASH_BLOCK_SIZE) {
        DEV_USER_CFG_ERR("Item offset out of range. (offset=%u; item_bytes=%u)\n",
                         item_head->item_offset, item_head->item_bytes);
        return UC_ERR_PARA;
    }

    ret = devdrv_config_cmp_sha256_data(check_pos_main, item_head->item_bytes, sha256_pos_main, DEVDRV_SHA256_DIGEST);
    if (ret == UC_ERR_MEM_CMP) {
        ret = devdrv_config_cmp_sha256_data(check_pos_bk, item_head->item_bytes, sha256_pos_bk, DEVDRV_SHA256_DIGEST);
        if (ret == UC_ERR_MEM_CMP) {
            DEV_USER_CFG_INFO("Memcmp not equal, item invalid. (ret=%d)\n", ret);
            item_head->valid_flg = DEVDRV_FLASH_INVALID;
        } else if (ret == UC_OK) {
            ret = memcpy_s(check_pos_main, item_head->item_bytes + DEVDRV_SHA256_DIGEST, check_pos_bk,
                           item_head->item_bytes + DEVDRV_SHA256_DIGEST);
            if (ret != 0) {
                DEV_USER_CFG_ERR("The memcpy_s failed. (ret=%d)\n", ret);
                return ret;
            }

            *main_wr_flag = 1;
        }
    } else if (ret == UC_OK) {
        ret = memcmp(check_pos_bk, check_pos_main, item_head->item_bytes + DEVDRV_SHA256_DIGEST);
        if (ret != 0) {
            ret = memcpy_s(check_pos_bk, item_head->item_bytes + DEVDRV_SHA256_DIGEST, check_pos_main,
                           item_head->item_bytes + DEVDRV_SHA256_DIGEST);
            if (ret != 0) {
                DEV_USER_CFG_ERR("The memcpy_s failed. (ret=%d)\n", ret);
                return ret;
            }

            *bk_wr_flag = 1;
            return UC_OK;
        }

        DEV_USER_CFG_INFO("Not need sync.\n");
    }

    if (ret == UC_ERR_SHA256_CHECK) {
        DEV_USER_CFG_ERR("The devdrv_config_cmp_sha256_data failed. (ret=%d)\n", ret);
        return ret;
    }

    return UC_OK;
}

/*
 *  write data to flash if it's essential
 */
STATIC s32 devdrv_config_sync_data_to_flash(uc_sync_info_t *sync_info)
{
    s32 ret;

    if (sync_info->main_wr_flg == 1) {
        ret = devdrv_config_write_blk_ops(sync_info->dev_id, sync_info->blk_index, sync_info->main_blk,
                                          DEVDRV_FLASH_BLOCK_SIZE, DEVDRV_UC_FLASH_FLAG_MAIN);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_uc_write_blk_ops failed. (device_id=%u; ret=%d)\n", sync_info->dev_id, ret);
            return ret;
        }
    }

    if (sync_info->bk_wr_flg == 1) {
        ret = devdrv_config_write_blk_ops(sync_info->dev_id, sync_info->blk_index + UC_FLASH_PARTITION_MAIN_NUM,
                                          sync_info->backup_blk, DEVDRV_FLASH_BLOCK_SIZE, DEVDRV_UC_FLASH_FLAG_BACKUP);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_uc_write_blk_ops failed. (device_id=%u; ret=%d)\n", sync_info->dev_id, ret);
            return ret;
        }
    }

    return UC_OK;
}

/*
 *  sync main and backup data for one block
 */
STATIC s32 devdrv_config_sync_one_block(uc_sync_info_t *sync_info)
{
    s32 ret, item_num;
    s32 i = 0;
    u32 head_len;
    u32 dev_id = sync_info->dev_id;
    uc_cfg_head_t *item_head = NULL;
    head_len = *(u32 *)(sync_info->head_blk + UC_DYNAMIC_CFG_HEAD_OFFSET);
    if (head_len % sizeof(uc_cfg_head_t)) {
        DEV_USER_CFG_ERR("The head len data err. (device_id=%u; head_len=%u)\n", dev_id, head_len);
        return UC_ERR_PARA;
    }

    item_num = head_len / sizeof(uc_cfg_head_t);
    if (item_num > (s32)DEVDRV_UC_HEAD_NUM_MAX) {
        DEV_USER_CFG_ERR("The head_num out of range. (device_id=%u; item_num=%d)\n", dev_id, item_num);
        return UC_ERR_HEAD_NUM;
    } else if (item_num == 0) {
        DEV_USER_CFG_WARN("The item num is 0, no need to sync. (device_id=%u)\n", dev_id);
        return UC_OK;
    }

    DEV_USER_CFG_INFO("Device's head num. (device_id=%u; num=%d)\n", dev_id, item_num);
    item_head = (uc_cfg_head_t *)(sync_info->head_blk + UC_CFG_HEAD_ITEM_START);

    ret = devdrv_config_read_block(dev_id, sync_info->blk_index, sync_info->main_blk, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = devdrv_config_read_block(dev_id, sync_info->blk_index + UC_FLASH_PARTITION_MAIN_NUM,
                                   sync_info->backup_blk, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }
    for (i = 0; i < item_num; i++) {
        if ((item_head->blk_offset == sync_info->blk_index) && (item_head->valid_flg == DEVDRV_FLASH_VALID)) {
            ret = devdrv_config_sync_item_content(sync_info->main_blk, sync_info->backup_blk, item_head,
                                                  &sync_info->main_wr_flg, &sync_info->bk_wr_flg);
            if (ret) {
                DEV_USER_CFG_ERR("The devdrv_config_sync_item_content failed. (device_id=%u; ret=%d)\n", dev_id, ret);
                return ret;
            }
        }

        item_head++;
    }

    ret = devdrv_config_sync_data_to_flash(sync_info);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_sync_data_to_flash failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }
    return UC_OK;
}

/*
 *  sync main and backup data
 */
STATIC s32 devdrv_config_sync_main_backup(u32 dev_id, u8 *head_blk, u32 blk_size, u32 sync_flg, u32 blk_index)
{
    s32 ret;
    s32 i = 0;
    uc_sync_info_t sync_info = {0};

    if (blk_size != DEVDRV_FLASH_BLOCK_SIZE) {
        DEV_USER_CFG_ERR("The block size error. (device_id=%u; blk_size=%u)\n", dev_id, blk_size);
        return UC_ERR_PARA;
    }

    sync_info.main_blk = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (sync_info.main_blk == NULL) {
        DEV_USER_CFG_ERR("The ka_vmalloc failed. (device_id=%u)\n", dev_id);
        return UC_ERR_MEM_ALLOC;
    }

    sync_info.backup_blk = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (sync_info.backup_blk == NULL) {
        DEV_USER_CFG_ERR("The ka_vmalloc failed. (device_id=%u)\n", dev_id);
        ret = UC_ERR_MEM_ALLOC;
        goto OUT1;
    }

    sync_info.head_blk = head_blk;
    sync_info.dev_id = dev_id;
    if (sync_flg == DEVDRV_UC_SYNC_ALL) {
        for (i = UC_ITEM_CONTENT_BLK_START; i < UC_FLASH_PARTITION_MAIN_NUM; i++) {
            sync_info.blk_index = i;
            ret = devdrv_config_sync_one_block(&sync_info);
            if (ret) {
                DEV_USER_CFG_ERR("The devdrv_config_sync_one_block failed. (device_id=%u; ret=%d)\n",
                    dev_id, ret);
                goto OUT;
            }
        }
    } else if (sync_flg == DEVDRV_UC_SYNC_ONE_BLOCK) {
        sync_info.blk_index = blk_index;
        ret = devdrv_config_sync_one_block(&sync_info);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_sync_one_block failed. (device_id=%u; ret=%d)\n",
                dev_id, ret);
            goto OUT;
        }
    } else {
        DEV_USER_CFG_ERR("Sync flag error. (device_id=%u; flag=%u)\n", dev_id, sync_flg);
    }

    DEV_USER_CFG_INFO("Synchronizing the primary and backup blocks succeeded. (device_id=%u; flag=%u)\n",
        dev_id, sync_flg);
    ret = UC_OK;
OUT:
    vfree(sync_info.backup_blk);
    sync_info.backup_blk = NULL;
OUT1:
    vfree(sync_info.main_blk);
    sync_info.main_blk = NULL;
    return ret;
}

#else
/*
 *  rebuild block data
 */
STATIC s32 devdrv_config_rebuild_item_data(u32 dev_id, u8 *head_blk, u8 *cfg_head, u32 head_len)
{
    s32 ret;
    u32 i = 0;
    u32 head_num;
    u8 *content_blk = NULL;
    uc_blk_used_info_t *used_info = NULL;
    uc_cfg_head_t *item_head = (uc_cfg_head_t *)cfg_head;

    /* alloc memory for content */
    head_num = head_len / sizeof(uc_cfg_head_t);
    if ((head_num == 0) || (head_num > DEVDRV_UC_HEAD_NUM_MAX)) {
        DEV_USER_CFG_ERR("The head num error. (device_id=%u; head_num=%u)\n", dev_id, head_num);
        return UC_ERR_PARA;
    }

    /* content block is first block */
    /* rebuild content */
    used_info = (uc_blk_used_info_t *)(head_blk + UC_BLK_INFO_OFFSET);
    content_blk = head_blk;
    ret = devdrv_config_content_cpy(content_blk, used_info, i, item_head, head_num);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_content_cpy failed. (device_id=%u; ret=%d)\n", dev_id, ret);
    }

    return ret;
}
#endif

/*
 *  rebuild item data, including head and content
 */
STATIC s32 devdrv_config_rebuild_item_content(u32 dev_id, u8 *head_blk)
{
    s32 ret;
    u32 head_len;
    u8 *head_pt = NULL;

    head_len = *(u32 *)(head_blk + UC_DYNAMIC_CFG_HEAD_OFFSET);
    head_pt = head_blk + UC_CFG_HEAD_ITEM_START;

    if ((head_len % sizeof(uc_cfg_head_t)) != 0) {
        DEV_USER_CFG_ERR("The head len error. (device_id=%u; head_len=%u)\n", dev_id, head_len);
        return UC_ERR_PARA;
    }

    /* sync main partition and backup partition */
#ifdef CFG_USER_CFG_SUPPORT_MULTI_BLOCK
    ret = devdrv_config_sync_main_backup(dev_id, head_blk, DEVDRV_FLASH_BLOCK_SIZE, DEVDRV_UC_SYNC_ALL, 0);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_sync_main_backup failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }
#endif

    /* rebuild item content */
    ret = devdrv_config_rebuild_item_data(dev_id, head_blk, head_pt, head_len);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_sync_item_head_content failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return ret;
}

/*
 * add user cfg new item
 */
STATIC s32 devdrv_config_add_item(devdrv_cfg_para_t *config_para, uc_item_info_t *item_info)
{
    s32 ret;

    ret = devdrv_config_add_item_head(config_para, item_info);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_add_item_head failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = devdrv_config_update_item_content(config_para->dev_id, item_info);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_update_item_content failed. (ret=%d)\n", ret);
        return ret;
    }

    item_info->main_head_wr_flag = 1;
    item_info->main_content_wr_flag = 1;
    item_info->bk_head_wr_flag = 1;
    item_info->bk_content_wr_flag = 1;

    return UC_OK;
}

/*
 * add user cfg new item
 */
STATIC s32 devdrv_config_update_item_dynamic(devdrv_cfg_para_t *config_para, uc_item_info_t *item_info)
{
    s32 ret;
    u32 cmp_result = 0;
    uc_cfg_head_t *head_pt = item_info->head_ops;

    /* check if setting item size is equal to current size, if equal, no need to update head */
    DEV_USER_CFG_INFO("devdrv_config_update_item_dynamic START. new_size=%d, old_size=%d\n",
                      config_para->buf_size, head_pt->item_bytes);
    if (config_para->buf_size != head_pt->item_bytes) {
        ret = devdrv_config_update_item_head(config_para, item_info);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_uc_update_item_head failed. (ret=%d)\n", ret);
            return ret;
        }
        item_info->main_head_wr_flag = 1;
        item_info->main_content_wr_flag = 1;
        item_info->bk_head_wr_flag = 1;
        item_info->bk_content_wr_flag = 1;
    } else {
        item_info->main_head_wr_flag = 0;
        item_info->main_content_wr_flag = 1;
        item_info->bk_head_wr_flag = 0;
        item_info->bk_content_wr_flag = 1;
        ret = devdrv_config_check_item_content(config_para->dev_id, item_info, &cmp_result);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_uc_check_item_content failed. (ret=%d)\n", ret);
            return ret;
        }
    }

    if (cmp_result == 0) {
        ret = devdrv_config_update_item_content(config_para->dev_id, item_info);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_uc_update_item_content failed. (ret=%d)\n", ret);
            return ret;
        }
    } else {
        item_info->main_head_wr_flag = 0;
        item_info->main_content_wr_flag = 0;
        item_info->bk_head_wr_flag = 0;
        item_info->bk_content_wr_flag = 0;
    }
    return UC_OK;
}

/*
 * write flash content and head according to the write flash flag
 */
STATIC s32 devdrv_config_write_flash_head_content(uc_item_info_t *item_info)
{
    s32 ret = 0;
    u32 main_content_index = 0;
    u32 bk_content_index = 0;

    if (item_info->content_blk_index >= DEVDRV_FLASH_SECOND_BLOCK) {
        main_content_index = item_info->content_blk_index % DEVDRV_FLASH_SECOND_BLOCK;
        bk_content_index = item_info->content_blk_index;
    } else {
        main_content_index = item_info->content_blk_index;
        bk_content_index = item_info->content_blk_index + DEVDRV_FLASH_SECOND_BLOCK;
    }

    /* write main content */
    if (item_info->main_content_wr_flag == 1) {
        ret = devdrv_config_write_blk_ops(item_info->dev_id, main_content_index, item_info->content_blk,
                                          DEVDRV_FLASH_BLOCK_SIZE, DEVDRV_UC_FLASH_FLAG_MAIN);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_write_blk_ops failed. (ret=%d)\n", ret);
            return ret;
        }
        DEV_USER_CFG_INFO("devdrv_config_write_flash_head_content main_content_wr_flag.\n");
    }

    /* write main head */
    if (item_info->main_head_wr_flag == 1) {
        ret = devdrv_config_write_blk_ops(item_info->dev_id, DEVDRV_FLASH_FIRST_BLOCK, item_info->head_blk,
                                          DEVDRV_FLASH_BLOCK_SIZE, DEVDRV_UC_FLASH_FLAG_MAIN);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_write_blk_ops failed. (ret=%d)\n", ret);
            return ret;
        }
        DEV_USER_CFG_INFO("devdrv_config_write_flash_head_content main_head_wr_flag.\n");
    }

    /* write backup content */
    if (item_info->bk_content_wr_flag == 1) {
        ret = devdrv_config_write_blk_ops(item_info->dev_id, bk_content_index, item_info->content_blk,
                                          DEVDRV_FLASH_BLOCK_SIZE, DEVDRV_UC_FLASH_FLAG_BACKUP);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_write_blk_ops failed. (ret=%d)\n", ret);
            return ret;
        }
        DEV_USER_CFG_INFO("devdrv_config_write_flash_head_content bk_content_wr_flag.\n");
    }

    /* write backup head */
    if (item_info->bk_head_wr_flag == 1) {
        ret = devdrv_config_write_blk_ops(item_info->dev_id, DEVDRV_FLASH_SECOND_BLOCK, item_info->head_blk,
                                          DEVDRV_FLASH_BLOCK_SIZE, DEVDRV_UC_FLASH_FLAG_BACKUP);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_write_blk_ops failed. (ret=%d)\n", ret);
            return ret;
        }
        DEV_USER_CFG_INFO("devdrv_config_write_flash_head_content bk_head_wr_flag.\n");
    }

    return ret;
}

STATIC s32 devdrv_config_clear_item_content(uc_item_info_t *item_info)
{
    s32 ret;
    u32 item_size = item_info->head_ops->item_bytes;

    if ((item_size > UC_ITEM_DATA_MAX_LEN) || (item_size == 0)) {
        DEV_USER_CFG_ERR("Item size out of range. (item_size=%u)\n", item_size);
        return UC_ERR_PARA;
    }

    item_info->content_buf = (u8 *)ka_vmalloc(item_size, GFP_KERNEL | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info->content_buf == NULL) {
        DEV_USER_CFG_ERR("ka_vmalloc content block failed.\n");
        return UC_ERR_MEM_ALLOC;
    }

    ret = memset_s(item_info->content_buf, item_size, DEVDRV_UC_FLASH_DEFAULT_VAL, item_size);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The memset_s failed. (ret=%d)\n", ret);
        goto out;
    }

    ret = devdrv_config_update_item_content(item_info->dev_id, item_info);
    if (ret != UC_OK) {
        DEV_USER_CFG_ERR("The devdrv_config_update_item_content failed. (ret=%d)\n", ret);
        goto out;
    }

out:
    vfree(item_info->content_buf);
    item_info->content_buf = NULL;
    return ret;
}

/*
 * Clear user configure dynamic item
 */
STATIC s32 devdrv_config_clear_item_head(uc_item_info_t *item_info)
{
    s32 ret;
    u8 *clr_pos = NULL;
    u8 *move_pos = NULL;
    u32 *head_len = NULL;
    u32 head_num;

    /* clear item head */
    clr_pos = item_info->head_blk + UC_CFG_HEAD_ITEM_START + sizeof(uc_cfg_head_t) * item_info->head_index;
    head_len = (unsigned int *)(item_info->head_blk + UC_CFG_HEAD_LEN_OFFSET);
    head_num = *head_len / sizeof(uc_cfg_head_t);
    if ((head_num == 0) || (head_num > DEVDRV_UC_HEAD_NUM_MAX)) {
        DEV_USER_CFG_ERR("The head num error. (head_num=%u)\n", head_num);
        *head_len = 0;
        return UC_ERR_HEAD_NUM;
    }

    /* move end item head to clear item head position */
    move_pos = item_info->head_blk + UC_CFG_HEAD_ITEM_START + sizeof(uc_cfg_head_t) * (head_num - 1);
    if ((head_num != 1) && (item_info->head_index != head_num - 1)) {
        ret = memcpy_s(clr_pos, sizeof(uc_cfg_head_t), move_pos, sizeof(uc_cfg_head_t));
        if (ret != 0) {
            DEV_USER_CFG_ERR("The memcpy_s failed. (ret=%d)\n", ret);
            return ret;
        }
    }

    *head_len -= sizeof(uc_cfg_head_t);

    /* reset sha256 value */
    ret = devdrv_config_set_check_code(item_info->head_blk, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_set_check_code failed. (ret=%d)\n", ret);
        return ret;
    }

    return UC_OK;
}

STATIC s32 devdrv_config_clear_item(uc_item_info_t *item_info)
{
    int ret;

    item_info->content_blk = (u8 *)ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT,
        PAGE_KERNEL);
    if (item_info->content_blk == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc content block failed.\n");
        return UC_ERR_MEM_ALLOC;
    }

    /* clear item content first */
    ret = devdrv_config_clear_item_content(item_info);
    if (ret != UC_OK) {
        DEV_USER_CFG_ERR("The devdrv_config_clear_item_content failed. (ret=%d)\n", ret);
        goto out;
    }

    /* clear item head */
    ret = devdrv_config_clear_item_head(item_info);
    if (ret != UC_OK) {
        DEV_USER_CFG_ERR("The devdrv_config_clear_item_head failed. (ret=%d)\n", ret);
        goto out;
    }

    item_info->main_head_wr_flag = 1;
    item_info->main_content_wr_flag = 1;
    item_info->bk_head_wr_flag = 1;
    item_info->bk_content_wr_flag = 1;

    ret = devdrv_config_write_flash_head_content(item_info);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_write_flash_head_content failed. (ret=%d)\n", ret);
    }

out:
    vfree(item_info->content_blk);
    item_info->content_blk = NULL;
    return ret;
}

/*
 * set user configure static item
 */
STATIC s32 devdrv_config_set_item_static(devdrv_cfg_para_t *cfg_para, u8 *buff)
{
    return devdrv_config_update(cfg_para->dev_id, cfg_para->cfg_index, buff, cfg_para->buf_size, DEVDRV_FLASH_VALID);
}

/*
 * Get user configure static item
 */
STATIC s32 devdrv_get_user_config_static(devdrv_cfg_para_t *config_para)
{
    s32 ret;
    u8 *data = NULL;
    u32 data_size = UC_ITEM_DATA_MAX_LEN;

    data = ka_vmalloc(UC_ITEM_DATA_MAX_LEN, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (data == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc failed.\n");
        ret = UC_ERR_MEM_ALLOC;
        goto OUT;
    }

#ifdef CFG_USER_CFG_SUPPORT_MAC_INFO
    if (config_para->cfg_index == UC_ITEM_INDEX_MAC_INFO) {
        ret = devdrv_config_get_mac_info(config_para->dev_id, data, UC_ITEM_DATA_MAX_LEN, &data_size);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_get_mac_info failed. (ret=%d)\n", ret);
            goto OUT;
        }
    } else {
        ret = devdrv_get_user_config_core(config_para->dev_id, config_para->cfg_index, data, &data_size);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv get static item failed. (cfg_index=%d; ret=%d)\n",
                config_para->cfg_index, ret);
            goto OUT;
        }
    }
#else
    ret = devdrv_get_user_config_core(config_para->dev_id, config_para->cfg_index, data, &data_size);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv get static item failed. (cfg_index=%d; ret=%d)\n", config_para->cfg_index, ret);
        goto OUT;
    }
#endif
    if (data_size != config_para->buf_size) {
        DEV_USER_CFG_ERR("Size error. (cfg_index=%d; buf_size=%u; data_size=%u)\n",
            config_para->cfg_index, config_para->buf_size, data_size);
        ret = UC_ERR_PARA;
        goto OUT;
    }

    config_para->buf_size = data_size;
    ret = copy_to_user((void *)config_para->buf, data, data_size);
    if (ret) {
        DEV_USER_CFG_ERR("The copy_to_user failed. (ret=%d)\n", ret);
        goto OUT;
    }
OUT:
    if (data != NULL) {
        vfree(data);
        data = NULL;
    }

    return ret;
}

/*
 * get data from user space and set user configure static item
 */
STATIC s32 devdrv_set_user_config_static(devdrv_cfg_para_t *cfg_para)
{
    s32 ret;
    u8 *data_buf = NULL;

    /* size had been checked before */
    data_buf = ka_vmalloc(UC_ITEM_DATA_MAX_LEN, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (data_buf == NULL) {
        DEV_USER_CFG_ERR("Alloc memory for block buffer failed.\n");
        return UC_ERR_MEM_ALLOC;
    }

    ret = copy_from_user((void *)data_buf, cfg_para->buf, cfg_para->buf_size);
    if (ret) {
        DEV_USER_CFG_ERR("The copy_to_user failed. (ret=%d)\n", ret);
        goto OUT;
    }

#ifdef CFG_USER_CFG_SUPPORT_MAC_INFO
    if (cfg_para->cfg_index == UC_ITEM_INDEX_MAC_INFO) {
        ret = devdrv_config_set_mac_info(cfg_para->dev_id, data_buf, cfg_para->buf_size);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_set_mac_info failed. (device_id=%u; ret=%d)\n", cfg_para->dev_id, ret);
        }

        goto OUT;
    }
#endif

    ret = devdrv_config_set_item_static(cfg_para, data_buf);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_set_item_static failed. (ret=%d)\n", ret);
        goto OUT;
    }

OUT:
    vfree(data_buf);
    data_buf = NULL;
    return ret;
}

STATIC s32 devdrv_clear_user_config_static(devdrv_cfg_para_t *config_para)
{
    u8 *buf = NULL;
    u32 buf_size;
    s32 ret;
    u32 i;

    if (user_cfg_version_1[config_para->cfg_index].len <= 1) { // config_para len must > 1
        DEV_USER_CFG_ERR("The config_para len invalid. (len=%d)\n", user_cfg_version_1[config_para->cfg_index].len);
        return UC_ERR_PARA;
    }

    buf_size = user_cfg_version_1[config_para->cfg_index].len - 1;  // for vaild flag
    buf = ka_vmalloc(buf_size,  GFP_KERNEL | __GFP_ACCOUNT, PAGE_KERNEL);
    if (buf == NULL) {
        DEV_USER_CFG_ERR("The ka_vmalloc failed.\n");
        return UC_ERR_MEM_ALLOC;
    }

    for (i = 0; i < buf_size; ++i) {
        buf[i] = 0xFF;
    }

#ifdef CFG_USER_CFG_SUPPORT_MAC_INFO
    if (config_para->cfg_index == UC_ITEM_INDEX_MAC_INFO) {
        ret = devdrv_config_clear_mac_info(config_para->dev_id);
        if (ret < 0) {
            DEV_USER_CFG_ERR("The devdrv_config_clear_mac_info failed. (device_id=%u; ret=%d)\n",
                config_para->dev_id, ret);
        }

        goto out;
    }
#endif

    ret = devdrv_config_update(config_para->dev_id, config_para->cfg_index, buf, buf_size, DEVDRV_FLASH_INVALID);
    if (ret < 0) {
        DEV_USER_CFG_ERR("The devdrv_config_update fail. (device_id=%u; ret=%d)\n", config_para->dev_id, ret);
        goto out;
    }

out:
    vfree(buf);
    buf = NULL;
    return ret;
}

/*
 * Copy data to user space for getting dynamic cfg item
 */
STATIC int devdrv_config_copy_data_to_user(devdrv_cfg_para_t *config_para, uc_item_info_t *item_info, unsigned long arg)
{
    s32 ret = UC_ERR_PARA;

    if ((void *)(uintptr_t)arg != NULL) {
        if (config_para->buf_size >= item_info->content_size) {
            config_para->buf_size = item_info->content_size;
            ret = copy_to_user((void *)config_para->buf, item_info->content_buf, item_info->content_size);
            if (ret) {
                DEV_USER_CFG_ERR("The copy_to_user failed. (ret=%d)\n", ret);
                return ret;
            }

            ret = copy_to_user((void *)&((struct user_cfg_ioctl_para *)(uintptr_t)arg)->buf_size,
                               &item_info->content_size, sizeof(u32));
            if (ret) {
                DEV_USER_CFG_ERR("The copy_to_user failed. (ret=%d)\n", ret);
                return ret;
            }
        } else {
            DEV_USER_CFG_ERR("The content size is not equal to the buf size."
                             "(content_size=%#x; buf_size=%#x; ret=%d)\n",
                             item_info->content_size, config_para->buf_size, ret);
            return UC_ERR_GET_ITEM_FAIL;
        }
    }

    /* check if need to sync main and backup */
    ret = devdrv_config_write_flash_head_content(item_info);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_write_flash_head_content failed. (ret=%d)\n", ret);
        return ret;
    }

    return UC_OK;
}

/*
 * Get user configure dynamic item
 */
STATIC int devdrv_config_get_item_dynamic(devdrv_cfg_para_t *config_para, uc_item_info_t *item_info)
{
    s32 ret;

    /* check valid block and read data */
    ret = devdrv_config_get_valid_block(config_para->dev_id, item_info->head_blk,
                                        &item_info->head_valid_flag, &item_info->valid_blk);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_get_valid_block failed. (ret=%d)\n", ret);
        return ret;
    }

    if (item_info->head_valid_flag == DEVDRV_FLASH_HEAD_INVALID) {
        DEV_USER_CFG_WARN("The block is invalid.\n");
        ret = UC_ERR_FLASH_INVALID;
        return ret;
    }

    /* get user cfg item content */
    ret = devdrv_config_get_item_from_head(config_para->dev_id, item_info, (const char *)config_para->name);
    if (ret) {
        /* if main and backup block are all valid, used backup head to check item */
        if ((item_info->valid_blk == DEVDRV_FLASH_FIRST_BLOCK) &&
            (item_info->head_valid_flag == DEVDRV_FLASH_HEAD_ALL_VALID)) {
            DEV_USER_CFG_INFO("Get item from backup head.\n");
            ret = devdrv_config_read_block(config_para->dev_id, DEVDRV_FLASH_SECOND_BLOCK,
                                           item_info->head_blk, DEVDRV_FLASH_BLOCK_SIZE);
            if (ret) {
                DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; ret=%d)\n",
                    config_para->dev_id, ret);
                return ret;
            }

            ret = devdrv_config_get_item_from_head(config_para->dev_id, item_info, (const char *)config_para->name);
            if (ret == UC_ERR_ITEM_NOT_SET) {
                DEV_USER_CFG_WARN("Item is not in the item head. (ret=%d)\n", ret);
                return ret;
            } else if (ret) {
                DEV_USER_CFG_ERR("The devdrv_config_get_item_from_head failed. (ret=%d)\n", ret);
                return ret;
            }
        }
    }

    /* check content size */
    if (config_para->buf_size < item_info->content_size) {
        DEV_USER_CFG_ERR("The content size is not equal to the buf size. (content_size=%#x; buf_size=%#x]; ret=%d)\n",
            item_info->content_size, config_para->buf_size, ret);
        ret = UC_ERR_GET_ITEM_FAIL;
    }

    return ret;
}

/*
 * Get user configure dynamic item
 */
STATIC int devdrv_get_user_config_dynamic(devdrv_cfg_para_t *config_para, unsigned long arg)
{
    s32 ret;
    uc_item_info_t item_info = {0};

    item_info.dev_id = config_para->dev_id;
    item_info.head_blk = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info.head_blk == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc head block failed.\n");
        return UC_ERR_MEM_ALLOC;
    }

    item_info.content_blk = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info.content_blk == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc content block failed.\n");
        ret = UC_ERR_MEM_ALLOC;
        goto out3;
    }

    item_info.content_buf = ka_vmalloc(UC_ITEM_DATA_MAX_LEN, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info.content_buf == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc content buf failed.\n");
        ret = UC_ERR_MEM_ALLOC;
        goto out2;
    }

    ret = devdrv_config_get_item_dynamic(config_para, &item_info);
    if (ret) {
        if (ret == UC_ERR_ITEM_NOT_SET) {
            DEV_USER_CFG_WARN("The getting item is not set before. (ret=%d)\n", ret);
        } else {
            DEV_USER_CFG_ERR("The devdrv_config_get_item_dynamic failed. (ret=%d)\n", ret);
        }
        goto out1;
    }

    /* return data */
    ret = devdrv_config_copy_data_to_user(config_para, &item_info, arg);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_copy_data_to_user failed. (ret=%d)\n", ret);
        goto out1;
    }

    DEV_USER_CFG_INFO("The devdrv_get_usr_config_dynamic success. (device_id=%u)\n", config_para->dev_id);

out1:
    vfree(item_info.content_buf);
    item_info.content_buf = NULL;
out2:
    vfree(item_info.content_blk);
    item_info.content_blk = NULL;
out3:
    vfree(item_info.head_blk);
    item_info.head_blk = NULL;
    return ret;
}

STATIC int devdrv_config_check_kpara(unsigned int dev_id, const char *item_name, u8 *data, u32 size)
{
    int ret;
    unsigned int dev_num = 0;

    if (item_name == NULL || data == NULL || size == 0 || size > UC_ITEM_DATA_MAX_LEN) {
        DEV_USER_CFG_ERR("invalid param. (device_id=%u; item_name=%d; data=%d; size=%d)\n",
            dev_id, item_name != NULL, data != NULL, size);
        return UC_ERR_PARA;
    }
    ret = devdrv_config_get_dev_num(&dev_num);
    if ((ret != 0) || (dev_num > DEVDRV_UC_CHIP_MAX)) {
        DEV_USER_CFG_ERR("Get device num failed. (device_id=%u; dev_num=%u; ret=%d)\n",
            dev_id, dev_num, ret);
        return UC_ERR_PARA;
    }
    if (dev_id >= dev_num) {
        DEV_USER_CFG_ERR("The device id is incorrect. (device_id=%u)\n", dev_id);
        return UC_ERR_PARA;
    }
    return UC_OK;
}

STATIC int devdrv_kernel_get_config_dynamic(devdrv_cfg_para_t *config_para, u8 *data, u32 in_len, u32 *out_len)
{
    int ret;
    uc_item_info_t item_info = {0};

    item_info.dev_id = config_para->dev_id;
    item_info.head_blk = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info.head_blk == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc head block failed. (device_id=%u)\n", config_para->dev_id);
        return UC_ERR_MEM_ALLOC;
    }

    item_info.content_blk = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info.content_blk == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc content block failed. (device_id=%u)\n", config_para->dev_id);
        ret = UC_ERR_MEM_ALLOC;
        goto out3;
    }

    item_info.content_buf = ka_vmalloc(UC_ITEM_DATA_MAX_LEN, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info.content_buf == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc content buf failed. (device_id=%u)\n", config_para->dev_id);
        ret = UC_ERR_MEM_ALLOC;
        goto out2;
    }
    ret = devdrv_config_get_item_dynamic(config_para, &item_info);
    if (ret != 0) {
        if (ret == UC_ERR_ITEM_NOT_SET) {
            DEV_USER_CFG_WARN("The getting item is not set before. (device_id=%u; ret=%d)\n",
                config_para->dev_id, ret);
        } else {
            DEV_USER_CFG_ERR("The devdrv_config_get_item_dynamic failed. (device_id=%u; ret=%d)\n",
                config_para->dev_id, ret);
        }
        goto out1;
    }
    ret = memcpy_s((void *)data, in_len, item_info.content_buf, item_info.content_size);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The memcpy_s failed. (device_id=%u; ret=%d)\n", config_para->dev_id, ret);
        ret = UC_ERR_PARA;
        goto out1;
    }
    *out_len = item_info.content_size;
out1:
    vfree(item_info.content_buf);
    item_info.content_buf = NULL;
out2:
    vfree(item_info.content_blk);
    item_info.content_blk = NULL;
out3:
    vfree(item_info.head_blk);
    item_info.head_blk = NULL;
    return ret;
}

int hal_kernel_get_user_config(unsigned int dev_id, const char *name, u8 *data, u32 in_len, u32 *out_len)
{
    int ret;
    devdrv_cfg_para_t config_para = {0};

    ret = devdrv_config_check_kpara(dev_id, name, data, in_len);
    if (ret != 0) {
        return ret;
    }
    if (out_len == NULL) {
        DEV_USER_CFG_ERR("Invalid param. (device_id=%u; out_len=%d)\n", dev_id, out_len != NULL);
        return UC_ERR_PARA;
    }

    config_para.buf_size = in_len;
    config_para.dev_id = dev_id;
    ret = strcpy_s((char *)config_para.name, DEVDRV_UC_NAME_MAX, name);
    if (ret != 0) {
        DEV_USER_CFG_ERR("strcpy_s failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return UC_ERR_PARA;
    }

    ret = devdrv_get_config_index_by_name(config_para.name);
    if ((ret == UC_CFG_INDEX_DYNAMIC) || (ret >= 0)) {
        config_para.cfg_index = ret;
    } else {
        DEV_USER_CFG_ERR("The devdrv_get_config_index_by_name failed. (ret=%d)\n", ret);
        return ret;
    }

    devdrv_config_mutex_lock(config_para.dev_id);
    if ((config_para.cfg_index >= 0) && (config_para.cfg_index < UC_ITEM_MAX_NUM)) {
        *out_len = in_len;
        ret = devdrv_get_user_config_core(config_para.dev_id, config_para.cfg_index, data, out_len);
        if (ret != 0) {
            DEV_USER_CFG_ERR("The devdrv get static item failed. (cfg_index=%d; ret=%d)\n",
                config_para.cfg_index, ret);
        }
    } else {
        ret = devdrv_kernel_get_config_dynamic(&config_para, data, in_len, out_len);
        if (ret != 0) {
            DEV_USER_CFG_WARN("Can't get dynamic item. (ret=%d)\n", ret);
        }
    }
    devdrv_config_mutex_unlock(config_para.dev_id);

    return ret;
}
EXPORT_SYMBOL(hal_kernel_get_user_config);

/*
 * Set user configure dynamic item
 */
STATIC s32 devdrv_config_set_item_dynamic(devdrv_cfg_para_t *config_para, uc_item_info_t *item_info)
{
    s32 ret;

    /* check valid block and read data */
    ret = devdrv_config_get_valid_block(config_para->dev_id, item_info->head_blk,
                                        &item_info->head_valid_flag, &item_info->valid_blk);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_get_valid_block failed. (ret=%d)\n", ret);
        return ret;
    }

    if (item_info->head_valid_flag == DEVDRV_FLASH_HEAD_INVALID) {
        DEV_USER_CFG_EVENT("Invalid block, init it. (device_id=%u)\n", config_para->dev_id);
        ret = devdrv_config_init_block_dynamic(config_para->dev_id, item_info->head_blk, DEVDRV_FLASH_BLOCK_SIZE,
                                               DEVDRV_UC_BLOCK_INIT_NO_WR);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_init_block_dynamic failed. (ret=%d)\n", ret);
            return ret;
        }
    }

    /* check user config name */
    ret = devdrv_get_usr_cfg_name_info(item_info, (const char *)config_para->name);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_get_usr_cfg_name_info failed. (ret=%d)\n", ret);
        return ret;
    }

    /* if item name not found, add one */
    if (item_info->head_index == DEVDRV_UC_HEAD_NOT_FOUND) {
        DEV_USER_CFG_EVENT("Add new item.\n");
        ret = devdrv_config_add_item(config_para, item_info);
        if (ret) {
            DEV_USER_CFG_ERR("devdrv_uc_add_item fail. (ret=%d)\n", ret);
            return ret;
        }
    } else {
        /* updated item content */
        DEV_USER_CFG_EVENT("Updated existing item.\n");
        ret = devdrv_config_update_item_dynamic(config_para, item_info);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_update_item_dynamic failed. (ret=%d)\n", ret);
            return ret;
        }
    }

    ret = devdrv_config_write_flash_head_content(item_info);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_write_flash_head_content failed. (ret=%d)\n", ret);
    }

    return ret;
}

STATIC int devdrv_kernel_set_config_dynamic(devdrv_cfg_para_t *config_para, u8 *data, u32 in_len)
{
    int ret;
    uc_item_info_t item_info = {0};

    item_info.head_index = DEVDRV_UC_HEAD_NOT_FOUND;
    item_info.content_size = config_para->buf_size;
    item_info.dev_id = config_para->dev_id;
    item_info.head_blk = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info.head_blk == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc head block failed. (device_id=%u)\n", item_info.dev_id);
        return UC_ERR_MEM_ALLOC;
    }

    item_info.content_blk = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info.content_blk == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc content block failed. (device_id=%u)\n", item_info.dev_id);
        ret = UC_ERR_MEM_ALLOC;
        goto out3;
    }

    item_info.content_buf = ka_vmalloc(UC_ITEM_DATA_MAX_LEN, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info.content_buf == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc content buf failed. (device_id=%u)\n", item_info.dev_id);
        ret = UC_ERR_MEM_ALLOC;
        goto out2;
    }

    ret = memcpy_s(item_info.content_buf, item_info.content_size, data, in_len);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The memcpy_s failed. (device_id=%u; ret=%d)\n", item_info.dev_id, ret);
        ret = UC_ERR_PARA;
        goto out1;
    }

    ret = devdrv_config_set_item_dynamic(config_para, &item_info);
    if (ret != 0) {
        DEV_USER_CFG_ERR("devdrv_config_set_item_dynamic failed. (device_id=%d; ret=%u)\n", ret, config_para->dev_id);
        goto out1;
    }

    DEV_USER_CFG_EVENT("Set dynamic cfg item success. (device_id=%u)\n", config_para->dev_id);

out1:
    vfree(item_info.content_buf);
    item_info.content_buf = NULL;
out2:
    vfree(item_info.content_blk);
    item_info.content_blk = NULL;
out3:
    vfree(item_info.head_blk);
    item_info.head_blk = NULL;
    return ret;
}

int hal_kernel_set_user_config(unsigned int dev_id, const char *name, u8 *data, u32 in_len)
{
    s32 ret;
    devdrv_cfg_para_t config_para = {0};

    ret = devdrv_config_check_kpara(dev_id, name, data, in_len);
    if (ret != 0) {
        return ret;
    }

    config_para.buf_size = in_len;
    config_para.dev_id = dev_id;
    ret = strcpy_s(config_para.name, DEVDRV_UC_NAME_MAX, name);
    if (ret != 0) {
        DEV_USER_CFG_ERR("strcpy_s failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return UC_ERR_PARA;
    }

    ret = devdrv_get_config_index_by_name(config_para.name);
    if ((ret == UC_CFG_INDEX_DYNAMIC) || (ret >= 0)) {
        config_para.cfg_index = ret;
    } else {
        DEV_USER_CFG_ERR("The devdrv_get_config_index_by_name failed. (ret=%d)\n", ret);
        return ret;
    }

    devdrv_config_mutex_lock(config_para.dev_id);
    if ((config_para.cfg_index >= 0) && (config_para.cfg_index < UC_ITEM_MAX_NUM)) {
        ret = devdrv_config_set_item_static(&config_para, data);
        if (ret != 0) {
            DEV_USER_CFG_ERR("The devdrv set static item failed. (cfg_index=%d; ret=%d)\n",
                config_para.cfg_index, ret);
        }
    } else {
        ret = devdrv_kernel_set_config_dynamic(&config_para, data, in_len);
        if (ret != 0) {
            DEV_USER_CFG_ERR("The devdrv set dynamic item failed. (ret=%d)\n", ret);
        }
    }
    devdrv_config_mutex_unlock(config_para.dev_id);
    /* Wait for the hardware operation to complete after 100 ms.  */
    msleep(100);

    return ret;
}
EXPORT_SYMBOL(hal_kernel_set_user_config);

/*
 * Set user configure dynamic item
 */
STATIC s32 devdrv_set_user_config_dynamic(devdrv_cfg_para_t *config_para)
{
    s32 ret;
    uc_item_info_t item_info = {0};

    /* size had been checked before */
    item_info.content_buf = ka_vmalloc(UC_ITEM_DATA_MAX_LEN, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info.content_buf == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc content_buf failed. (device_id=%u)\n", config_para->dev_id);
        return UC_ERR_MEM_ALLOC;
    }

    ret = copy_from_user((void *)item_info.content_buf, config_para->buf, config_para->buf_size);
    if (ret) {
        DEV_USER_CFG_ERR("The copy_from_user failed. (ret=%d)\n", ret);
        goto out;
    }

    item_info.head_index = DEVDRV_UC_HEAD_NOT_FOUND;
    item_info.content_size = config_para->buf_size;
    item_info.dev_id = config_para->dev_id;
    item_info.head_blk = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info.head_blk == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc head block failed.\n");
        ret = UC_ERR_MEM_ALLOC;
        goto out;
    }

    item_info.content_blk = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info.content_blk == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc content block failed.\n");
        ret = UC_ERR_MEM_ALLOC;
        goto out1;
    }

    ret = devdrv_config_set_item_dynamic(config_para, &item_info);
    if (ret) {
        DEV_USER_CFG_ERR("devdrv_config_set_item_dynamic failed. (device_id=%d; ret=%u)\n", ret, config_para->dev_id);
        goto out2;
    }

    DEV_USER_CFG_EVENT("Set dynamic cfg item success. (device_id=%u)\n", config_para->dev_id);

out2:
    vfree(item_info.content_blk);
    item_info.content_blk = NULL;
out1:
    vfree(item_info.head_blk);
    item_info.head_blk = NULL;
out:
    vfree(item_info.content_buf);
    item_info.content_buf = NULL;
    return ret;
}

/*
 * Clear user configure dynamic item
 */
STATIC s32 devdrv_clear_user_config_dynamic(devdrv_cfg_para_t *config_para)
{
    s32 ret;
    uc_item_info_t item_info = {0};

    item_info.head_index = DEVDRV_UC_HEAD_NOT_FOUND;
    item_info.dev_id = config_para->dev_id;
    item_info.head_blk = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (item_info.head_blk == NULL) {
        DEV_USER_CFG_ERR("Ka_vmalloc head block failed.\n");
        return UC_ERR_MEM_ALLOC;
    }

    /* check valid block and read data */
    ret = devdrv_config_get_valid_block(config_para->dev_id, item_info.head_blk,
                                        &item_info.head_valid_flag, &item_info.valid_blk);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_get_valid_block failed. (ret=%d)\n", ret);
        goto out;
    }

    if (item_info.head_valid_flag == DEVDRV_FLASH_HEAD_INVALID) {
        ret = devdrv_config_init_block_dynamic(config_para->dev_id, item_info.head_blk, DEVDRV_FLASH_BLOCK_SIZE,
                                               DEVDRV_UC_BLOCK_INIT_WR);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_init_block_dynamic failed. (ret=%d)\n", ret);
            goto out;
        }

        ret = UC_ERR_HEAD_NOT_FOUND;
        goto out;
    }

    /* check user config name */
    ret = devdrv_get_usr_cfg_name_info(&item_info, (const char *)config_para->name);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_get_usr_cfg_name_info failed. (ret=%d)\n", ret);
        goto out;
    }

    if (item_info.head_index != DEVDRV_UC_HEAD_NOT_FOUND) {
        ret = devdrv_config_clear_item(&item_info);
        if (ret) {
            DEV_USER_CFG_ERR("The devdrv_config_clear_item_head failed. (ret=%d)\n", ret);
            goto out;
        }
    } else {
        DEV_USER_CFG_ERR("Usr cfg name not found.\n");
        ret = UC_ERR_HEAD_NOT_FOUND;
    }

    DEV_USER_CFG_EVENT("Clear dynamic cfg item success. (device_id=%u)\n", config_para->dev_id);

out:
    vfree(item_info.head_blk);
    item_info.head_blk = NULL;
    return ret;
}

/*
 * user configure operation
 */
STATIC s32 devdrv_user_config_op(devdrv_cfg_para_t *config_para, s32 cmd, unsigned long arg)
{
    s32 ret;

    if ((cmd == DEVDRV_FLASH_CONFIG_WRITE_CMD) || (cmd == DEVDRV_FLASH_CONFIG_CLEAR_CMD)) {
        DEV_USER_CFG_EVENT("Custom item. (device_id=%u; config_index=%d; cmd=%d)\n",
            config_para->dev_id, config_para->cfg_index, cmd);
    }

    if ((config_para->cfg_index >= 0) && (config_para->cfg_index < UC_ITEM_MAX_NUM)) {
        switch (cmd) {
            case DEVDRV_FLASH_CONFIG_READ_CMD:
                ret = devdrv_get_user_config_static(config_para);
                break;
            case DEVDRV_FLASH_CONFIG_WRITE_CMD:
                ret = devdrv_set_user_config_static(config_para);
                break;
            case DEVDRV_FLASH_CONFIG_CLEAR_CMD:
                ret = devdrv_clear_user_config_static(config_para);
                break;
            default:
                DEV_USER_CFG_ERR("The cmd parameter error. (cmd=%d)\n", config_para->cmd);
                ret = UC_ERR_PARA;
                break;
        }
    } else {
        switch (cmd) {
            case DEVDRV_FLASH_CONFIG_READ_CMD:
                ret = devdrv_get_user_config_dynamic(config_para, arg);
                break;
            case DEVDRV_FLASH_CONFIG_WRITE_CMD:
                ret = devdrv_set_user_config_dynamic(config_para);
                break;
            case DEVDRV_FLASH_CONFIG_CLEAR_CMD:
                ret = devdrv_clear_user_config_dynamic(config_para);
                break;
            default:
                DEV_USER_CFG_ERR("The cmd parameter error. (cmd=%d)\n", config_para->cmd);
                ret = UC_ERR_PARA;
                break;
        }
    }

    return ret;
}

#ifdef CFG_USER_CFG_SUPPORT_MAC_INFO
/*
 *  description:    get mac information in user cfg partition
 *  @para:          dev_id       device id
 *  @para:          buf          buffer for store information
 *  @para:          buf_size     buffer max size
 *  @para:          info_size    actual info size
 *  return:         0 success�� not 0 fail
 */
int devdrv_config_get_mac_info(unsigned int dev_id, unsigned char *buf, unsigned int buf_size, unsigned int *info_size)
{
    int ret;
    u8 *blk_data = NULL;

    if ((buf == NULL) || (info_size == NULL) || (dev_id >= DEVDRV_UC_CHIP_MAX) || (buf_size < UC_MAC_INFO_SIZE)) {
        DEV_USER_CFG_ERR("Input parameter is incorrect. (device_id=%u; buf_size=%u)\n", dev_id, buf_size);
        return UC_ERR_PARA;
    }

    blk_data = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (blk_data == NULL) {
        DEV_USER_CFG_ERR("The ka_vmalloc failed. (device_id=%u)\n", dev_id);
        return UC_ERR_MEM_ALLOC;
    }

    ret = devdrv_config_read_block(dev_id, UC_BLK_INDEX_MAC_INFO, blk_data, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto out;
    }

    ret = memcpy_s(buf, buf_size, blk_data, UC_MAC_INFO_SIZE);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The memcpy_s failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto out;
    }

    *info_size = UC_MAC_INFO_SIZE;

out:
    vfree(blk_data);
    blk_data = NULL;
    return ret;
}

EXPORT_SYMBOL(devdrv_config_get_mac_info);

/*
 *  description:    set mac information in user cfg partition
 *  @para:          dev_id       device id
 *  @para:          buf          buffer for store information
 *  @para:          buf_size     buffer max size
 *  return:         0 success�� not 0 fail
 */
STATIC s32 devdrv_config_set_mac_info(u32 dev_id, u8 *buf, u32 buf_size)
{
    s32 ret;
    u8 *blk_data = NULL;

    if ((buf == NULL) || (dev_id >= DEVDRV_UC_CHIP_MAX) || (buf_size != UC_MAC_INFO_SIZE)) {
        DEV_USER_CFG_ERR("Input parameter is incorrect. (device_id=%u; buf_size=%u)\n", dev_id, buf_size);
        return UC_ERR_PARA;
    }

    blk_data = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (blk_data == NULL) {
        DEV_USER_CFG_ERR("The ka_vmalloc failed. (device_id=%u)\n", dev_id);
        return UC_ERR_MEM_ALLOC;
    }

    ret = devdrv_config_read_block(dev_id, UC_BLK_INDEX_MAC_INFO, blk_data, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret < 0) {
        DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto out;
    }

    ret = memcmp(blk_data, buf, UC_MAC_INFO_SIZE);
    if (!ret) {
        DEV_USER_CFG_INFO("The content not change, no need to write. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto out;
    }

    ret = memcpy_s(blk_data, DEVDRV_FLASH_BLOCK_SIZE, buf, UC_MAC_INFO_SIZE);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The memcpy_s failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto out;
    }

    ret = devdrv_config_write_blk_ops(dev_id, UC_BLK_INDEX_MAC_INFO, blk_data, DEVDRV_FLASH_BLOCK_SIZE,
                                      DEVDRV_UC_FLASH_FLAG_MAIN);
    if (ret < 0) {
        DEV_USER_CFG_ERR("devdrv_config_write_blk_ops failed, dev_id[%u] ret: %d.\n", dev_id, ret);
        goto out;
    }

out:
    vfree(blk_data);
    blk_data = NULL;
    return ret;
}

/*
 *  description:    clear mac information in usr cfg partition
 *  @para:          dev_id       device id
 *  return:         0 success�� not 0 fail
 */
STATIC s32 devdrv_config_clear_mac_info(u32 dev_id)
{
    s32 ret;
    u32 i = 0;
    u8 *blk_data = NULL;

    blk_data = ka_vmalloc(DEVDRV_FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (blk_data == NULL) {
        DEV_USER_CFG_ERR("The ka_vmalloc failed. (device_id=%u)\n", dev_id);
        return UC_ERR_MEM_ALLOC;
    }

    ret = devdrv_config_read_block(dev_id, UC_BLK_INDEX_MAC_INFO, blk_data, DEVDRV_FLASH_BLOCK_SIZE);
    if (ret < 0) {
        DEV_USER_CFG_ERR("The devdrv_config_read_block failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto out;
    }

    for (i = 0; i < UC_MAC_INFO_SIZE; i++) {
        *(blk_data + i) = DEVDRV_UC_FLASH_DEFAULT_VAL;
    }

    ret = devdrv_config_write_blk_ops(dev_id, UC_BLK_INDEX_MAC_INFO, blk_data, DEVDRV_FLASH_BLOCK_SIZE,
                                      DEVDRV_UC_FLASH_FLAG_MAIN);
    if (ret < 0) {
        DEV_USER_CFG_ERR("The devdrv_config_write_blk_ops failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        goto out;
    }

out:
    vfree(blk_data);
    blk_data = NULL;

    return ret;
}

/*
 * check authority
 */
STATIC s32 devdrv_config_check_authority(devdrv_cfg_para_t *cfg_para)
{
    s32 ret;
    u32 is_root_item = 0;

    u32 i = 0;
    u32 cert_cmp_result = 0;
    u32 process_num = 1;
    const char *process_name[WHITE_LIST_PROCESS_NUM] = {PROCESS_NAME_DMP, PROCESS_NAME_HCCP};
    const char *sec_item_name[NETWORK_ITEM_NAME_NUM] = {CERT_ITEM_NAME_S0, CERT_ITEM_NAME_S1,
        CERT_ITEM_NAME_S2, CERT_ITEM_NAME_S3, DIGITAL_ITEM_NAME_S0, DIGITAL_ITEM_NAME_S1,
        DIGITAL_ITEM_NAME_S2};

    /* if it's static item, check authority validity */
    if (cfg_para->cfg_index >= 0) {
        ret = check_authority_for_static_item(cfg_para, &is_root_item);
        if (ret) {
            DEV_USER_CFG_ERR("Check user parameter error. (ret=%d)\n", ret);
            return ret;
        }
    }

    /* compare item name and list name, if found then break */
    for (i = 0; i < NETWORK_ITEM_NAME_NUM; i++) {
        if (!strcmp(cfg_para->name, sec_item_name[i])) {
            cert_cmp_result = 1;
            break;
        }
    }

    /* set index by if it's cert item */
    if (cfg_para->cmd == DEVDRV_FLASH_CONFIG_READ_CMD) {
        if (cert_cmp_result == 1) {
            /* read cert item only support for dmp and hccp process */
            process_num = WHITE_LIST_PROCESS_NUM;
        } else {
            process_num = 0;
        }
    }

    if ((process_num > 0) && (is_root_item == 0)) {
        ret = whitelist_process_handler(process_name, process_num);
        if (ret) {
            DEV_USER_CFG_ERR("The whitelist_process_handler failed. (ret=%d)\n", ret);
            return UC_ERR_NO_AUTHORITY;
        }
    }

    return UC_OK;
}

s32 devdrv_config_get_cpu_cfg(u32 dev_id, u8 *buf, u32 buf_size)
{
    return 0;
}
#else
/*
 * check authority
 */
STATIC s32 devdrv_config_check_authority(devdrv_cfg_para_t *cfg_para)
{
    s32 ret;
    u32 is_root_item = 0;
    const char *process_name = PROCESS_NAME_DMP;

    /* if it's static item, check authority validity */
    if (cfg_para->cfg_index >= 0) {
        ret = check_authority_for_static_item(cfg_para, &is_root_item);
        if (ret) {
            DEV_USER_CFG_ERR("Check user parameter error. (ret=%d)\n", ret);
            return ret;
        }
    }

    /* whitelist check for write and clear cfg item */
    if (cfg_para->cmd != DEVDRV_FLASH_CONFIG_READ_CMD) {
        if (is_root_item == 0) {
            ret = whitelist_process_handler(&process_name, WHITE_LIST_PROCESS_NUM);
            if (ret) {
                DEV_USER_CFG_ERR("The whitelist_process_handler failed. (ret=%d)\n", ret);
                return UC_ERR_NO_AUTHORITY;
            }
        }
    }

    return UC_OK;
}

s32 devdrv_config_get_cpu_cfg(u32 dev_id, u8 *buf, u32 buf_size)
{
    devdrv_cfg_para_t config_para = {0};
    s32 ret;
    u32 size = buf_size;

    if ((dev_id >= DEVDRV_UC_CHIP_MAX) || (buf == NULL)) {
        DEV_USER_CFG_ERR("Input parameter is incorrect. (device_id=%u)\n", dev_id);
        return UC_ERR_PARA;
    }

    ret = devdrv_get_config_index_by_name(CPU_NUM_CONFIG_NAME);
    if ((ret == UC_CFG_INDEX_DYNAMIC) || (ret >= 0)) {
        config_para.cfg_index = ret;
    } else {
        DEV_USER_CFG_ERR("The devdrv_get_config_index_by_name failed. (ret=%d)\n", ret);
        return ret;
    }

    config_para.buf = buf;
    config_para.buf_size = buf_size;
    config_para.cmd = DEVDRV_FLASH_CONFIG_READ_CMD;
    config_para.dev_id = dev_id;

    ret = devdrv_get_user_config_core(config_para.dev_id, config_para.cfg_index, buf, &size);
    if (ret) {
        DEV_USER_CFG_ERR("Devdrv get static item failed. (cfg_index=%d; ret=%d)\n", config_para.cfg_index, ret);
    }

    return ret;
}

#endif

/*
 *  description:    operate user information in user cfg partition
 *  @para:          dev_id       device id
 *  @para:          cmd          get/set/clear cmd for operation
 *  @para:          buf          buffer for store information
 *  @para:          buf_size     buffer max size
 *  @para:          info_size    actual info size
 *  @para:          item_index   user config item index
 *  return:         0 success�� not 0 fail
 */
s32 devdrv_config_user_item_op(u32 dev_id, u32 cmd, u8 *buf, u32 buf_size, u32 *info_size, u32 item_index)
{
    devdrv_cfg_para_t config_para = {0};
    s32 ret;

    if ((dev_id >= DEVDRV_UC_CHIP_MAX) || (buf == NULL) || (info_size == NULL)) {
        DEV_USER_CFG_ERR("Input parameter is incorrect. (device_id=%u)\n", dev_id);
        return UC_ERR_PARA;
    }

    config_para.buf = buf;
    config_para.buf_size = buf_size;
    config_para.cmd = cmd;
    config_para.dev_id = dev_id;
    config_para.cfg_index = item_index;

    devdrv_config_mutex_lock(config_para.dev_id);
    ret = devdrv_user_config_op(&config_para, config_para.cmd, 0);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_user_config_op failed. (device_id=%u; ret=%d)\n", dev_id, ret);
    }

    devdrv_config_mutex_unlock(config_para.dev_id);
    return ret;
}
/*
 *  description:    clear user information in user cfg partition
 *  @para:          dev_id       device id
 *  @para:          item_index   user config item index
 *  return:         0 success�� not 0 fail
 */
s32 devdrv_config_clear_user_item(u32 dev_id, u32 item_index)
{
    s32 ret;
    u8 buf = 0;
    u32 buf_size = 0;
    u32 info_size = 0;

    ret = devdrv_config_user_item_op(dev_id, DEVDRV_FLASH_CONFIG_CLEAR_CMD, &buf, buf_size, &info_size, item_index);
    if (ret < 0) {
        DEV_USER_CFG_ERR("The devdrv_config_user_item_op failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return UC_OK;
}
EXPORT_SYMBOL(devdrv_config_clear_user_item);

/*
 *  description:    get pss configuration in memory
 *  @para:          dev_id       device id
 *  @para:          sign         sign type
 *  return:         0 success, not 0 fail
 */
int devdrv_config_get_pss_cfg(unsigned int dev_id, int *sign)
{
    if (sign == NULL) {
        DEV_USER_CFG_ERR("The value of dev_id or sign is invalid. (dev_id=%u; sign_is_null=%d)\n",
            dev_id, (sign == NULL));
        return -EINVAL;
    }
#if defined(CFG_FEATURE_PSS_SIGN) && defined(CFG_FEATURE_PKCS_SIGN)
    mutex_lock(&g_sign_type.sign_mutex);
    *sign = g_sign_type.sign;
    mutex_unlock(&g_sign_type.sign_mutex);
#elif defined CFG_FEATURE_PSS_SIGN
    *sign = PKCS_SIGN_TYPE_OFF;
#else
    *sign = PKCS_SIGN_TYPE_ON;
#endif

    DEV_USER_CFG_EVENT("The sign details. (sign=%d)\n", *sign);
    return 0;
}
EXPORT_SYMBOL(devdrv_config_get_pss_cfg);
/*
 *  description:    set pss configuration in memory
 *  @para:          dev_id       device id
 *  @para:          sign         sign type
 *  return:         0 success, not 0 fail
 */
int devdrv_config_set_pss_cfg(unsigned int dev_id, int sign)
{
#if defined(CFG_FEATURE_PSS_SIGN) && defined(CFG_FEATURE_PKCS_SIGN)
    if (sign != PKCS_SIGN_TYPE_ON && sign != PKCS_SIGN_TYPE_OFF) {
        DEV_USER_CFG_ERR("The value of dev_id or sign is invalid. (dev_id=%u; sign=%d)\n", dev_id, sign);
        return -EINVAL;
    }

    mutex_lock(&g_sign_type.sign_mutex);
    g_sign_type.sign = sign;
    mutex_unlock(&g_sign_type.sign_mutex);
#endif

    return 0;
}
EXPORT_SYMBOL(devdrv_config_set_pss_cfg);
s32 devdrv_config_pss_cfg_init(u32 sign)
{
#if defined(CFG_FEATURE_PSS_SIGN) && defined(CFG_FEATURE_PKCS_SIGN)
    g_sign_type.sign = sign;
    mutex_init(&g_sign_type.sign_mutex);
#endif

    return 0;
}
void devdrv_config_pss_cfg_uninit(void)
{
#if defined(CFG_FEATURE_PSS_SIGN) && defined(CFG_FEATURE_PKCS_SIGN)
    mutex_destroy(&g_sign_type.sign_mutex);
#endif

    return;
}

/*
 *  description:    check user mode input para and process info
 *  @para:          dev_id       device id
 *  return:         0 success�� not 0 fail
 */
STATIC s32 devdrv_config_check_para(struct user_cfg_ioctl_para *ioctl_para,
                                    devdrv_cfg_para_t *cfg_para)
{
    s32 ret;

    ret = devdrv_get_config_index_by_name(ioctl_para->name);
    if ((ret == UC_CFG_INDEX_DYNAMIC) || (ret >= 0)) {
        ioctl_para->cfg_index = ret;
    } else {
        DEV_USER_CFG_ERR("The devdrv_get_config_index_by_name failed. (ret=%d)\n", ret);
        return ret;
    }

    /* check parameter validity */
    ret = check_config_para(ioctl_para);
    if (ret) {
        DEV_USER_CFG_ERR("Check para failed. (ret=%d)\n", ret);
        return ret;
    }

    /* save parameter to configure data structure */
    cfg_para->buf = ioctl_para->buf;
    cfg_para->buf_size = ioctl_para->buf_size;
    cfg_para->cmd = ioctl_para->cmd;
    cfg_para->dev_id = ioctl_para->dev_id;

    ret = memcpy_s(cfg_para->name, DEVDRV_UC_NAME_MAX, ioctl_para->name, DEVDRV_UC_NAME_MAX);
    if (ret != 0) {
        DEV_USER_CFG_ERR("The memcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }

    cfg_para->cfg_index  = ioctl_para->cfg_index;

    return UC_OK;
}

/*
 * ioctl process
 */
s32 devdrv_config_ioctl(struct file *filep, u32 cmd, unsigned long arg)
{
#ifndef FEATURE_USER_CFG_NOT_SUPPORT
    struct user_cfg_ioctl_para ioctl_para = {0};
    devdrv_cfg_para_t config_para = {0};
    s32 ret;

    if ((!arg) || (filep == NULL)) {
        DEV_USER_CFG_ERR("Input parameter is incorrect. (arg=%lu)\n", arg);
        return UC_ERR_PARA;
    }

    ret = copy_from_user(&ioctl_para, (void *)((uintptr_t)arg), sizeof(struct user_cfg_ioctl_para));
    if (ret) {
        DEV_USER_CFG_ERR("The copy_from_user failed. (ret=%d)\n", ret);
        return ret;
    }

    /* check parameter is valid or not */
    ret = devdrv_config_check_para(&ioctl_para, &config_para);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_check_para failed. (ret=%d)\n", ret);
        return ret;
    }

    /* check excute authority */
    ret = devdrv_config_check_authority(&config_para);
    if (ret) {
        DEV_USER_CFG_ERR("The devdrv_config_check_authority failed. (device_id=%u; ret=%d)\n",
            config_para.dev_id, ret);
        return ret;
    }

    /* user configure operation */
    devdrv_config_mutex_lock(config_para.dev_id);
    ret = devdrv_user_config_op(&config_para, config_para.cmd, arg);
    if (ret) {
        if (ret == UC_ERR_ITEM_NOT_SET) {
            DEV_USER_CFG_WARN("The devdrv_user_config_op not find item. (device_id=%u; ret=%d)\n",
                config_para.dev_id, ret);
        } else {
            DEV_USER_CFG_ERR("The devdrv_user_config_op failed. (device_id=%u; ret=%d)\n",
                config_para.dev_id, ret);
        }
    }

    devdrv_config_mutex_unlock(config_para.dev_id);

    return ret;
#else
    return UC_ERR_ITEM_NOT_SET;
#endif
}

s32 devdrv_get_config_index_by_name(const char *name)
{
    int found = UC_CFG_INDEX_NOT_FOUND;
    int i;

#if ((defined CFG_SOC_PLATFORM_MINI) || (defined CFG_SOC_PLATFORM_MINIV2))
    int board_id;
#if (defined CFG_SOC_PLATFORM_MINI)
    board_id = devdrv_get_boardid();
#else
    devdrv_get_boardid_by_reg();
    board_id = g_devdrv_boardid;
#endif
    if (board_id < 0) {
        DEV_USER_CFG_ERR("Unable to get board id.\n");
        return -ENODEV;
    }

    for (i = 0; i < UC_ITEM_MAX_NUM; i++) {
        if (strncmp(user_cfg_version_1[i].name, name, DEVDRV_UC_NAME_MAX) == 0) {
            if (user_cfg_version_1[i].board_id == board_id || user_cfg_version_1[i].board_id == -1) {
                DEV_USER_CFG_INFO("Board ID matchs. (board_id=%u; config_board_id=%d)\n",
                    board_id, user_cfg_version_1[i].board_id);
                found = UC_CFG_INDEX_FOUND;
                break;
            }
        }
    }
#else
    for (i = 0; i < UC_ITEM_MAX_NUM; i++) {
        if (strncmp(user_cfg_version_1[i].name, name, DEVDRV_UC_NAME_MAX) == 0) {
            found = UC_CFG_INDEX_FOUND;
            break;
        }
    }
#endif

    if (found == UC_CFG_INDEX_FOUND) {
        return i;
    } else {
#ifdef CFG_USER_CFG_SUPPORT_DYNAMIC_ITEM
        return UC_CFG_INDEX_DYNAMIC;
#else
        DEV_USER_CFG_WARN("Board ID isn't matchs. (board_id=%u)\n", board_id);
        return -ENODEV;
#endif
    }
}
EXPORT_SYMBOL(devdrv_get_config_index_by_name);

#ifdef CFG_SOC_PLATFORM_MINI
int devdrv_flash_get_aicpu_config(const char *name, unsigned int *aicpu_num)
{
    unsigned char aicpu_config[UC_AICPU_CONFIG_SIZE] = {0};
    unsigned int aicpu_size = UC_AICPU_CONFIG_SIZE;
    int index;
    int ret;

    if (name == NULL || aicpu_num == NULL) {
        DEV_USER_CFG_ERR("Input parameter is NULL.\n");
        return -ENODEV;
    }
    index = devdrv_get_config_index_by_name(name);
    if (index < 0) {
        DEV_USER_CFG_ERR("Can't find config name, or board id isn't support. (name=%s)\n", name);
        return -ENODEV;
    }

    ret = devdrv_get_user_config_core(DEVDRV_UC_DEV_ID_0, index, aicpu_config, &aicpu_size);
    if (ret) {
        DEV_USER_CFG_ERR("Unable to get config in flash. (name=%s)\n", name);
        return -ENODEV;
    }

    *aicpu_num = (unsigned int)aicpu_config[0];
    if (*aicpu_num == 0) {
        DEV_USER_CFG_INFO("Not set the aicpu config in flash.\n");
        return -EINVAL;
    }
    if (*aicpu_num != AICPU2_CTRLCPU6_CONFIG && *aicpu_num != AICPU4_CTRLCPU4_CONFIG
                                            && *aicpu_num != AICPU6_CTRLCPU2_CONFIG) {
        DEV_USER_CFG_ERR("The aicpu config is invalid in flash. (aicpu_config=%u)\n", *aicpu_num);
        return -ENODEV;
    }
    return 0;
}
#endif
#else
int devdrv_config_get_dev_num(unsigned int *dev_num)
{
    return 0;
}
#endif
