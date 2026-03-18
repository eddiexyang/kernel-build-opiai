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

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/securec.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/gfp.h>
#include "dev_upgrade_public.h"
#include "dev_upgrade_core.h"
#include "dev_upgrade_adapt.h"
#include "devdrv_dfm.h"
#include "drv_whitelist.h"
#include "ascend_hal_error.h"

#ifdef SUPPORT_TEE_SECURE_UPGRADE
#include "dev_upgrade_sec.h"
#endif

#if (defined CFG_SOC_PLATFORM_MDC_V51)
#include "dev_upgrade_mdccore.h"
#include "dev_upgrade_ufs.h"
#include "dev_upgrade_crl.h"
#endif

STATIC int dev_upgrade_open(struct inode *pnode, struct file *pfile);
STATIC int dev_upgrade_release(struct inode *pnode, struct file *pfile);
STATIC long dev_upgrade_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#ifdef CFG_SOC_PLATFORM_MINI
STATIC int dev_upgrade_pre_check_nve(struct upgrade_component *component);
STATIC int dev_upgrade_update_single_area(struct upgrade_component *component);
STATIC int dev_upgrade_update_single_area_stub(struct upgrade_component *component);
#endif

#if (!defined CFG_SOC_PLATFORM_MDC_V51)
STATIC int dev_upgrade_pre_check_common(struct upgrade_component *component);
STATIC int dev_upgrade_update_double_area(struct upgrade_component *component);
#endif

typedef int (*upgrade_cmd_func)(struct upgrade_ioctl_msg *upgrade_msg);

typedef struct _upgrade_cmd_map_t {
    unsigned int cmd;
    int permission_check_flag;
    upgrade_cmd_func upgrade_cmd_handler;
} upgrade_cmd_map_t;

typedef struct st_devtype {
    unsigned int dev_major;
    unsigned int dev_minor;
    struct cdev cdev; /* Char device structure */
    struct class *dev_class;
} CDEV_ST;

CDEV_ST *g_infra_upgrade_dev = NULL;
PROG_ALL_PKG_INFO *g_upgrade_register_pkg_info = NULL;

struct file_operations g_infra_upgrade_fops = {
    .owner = THIS_MODULE,
    .open = dev_upgrade_open,
    .release = dev_upgrade_release,
    .unlocked_ioctl = dev_upgrade_ioctl,
};

/* dev_id mask: enable,0: disable, default enable */
#define DEV_ID_MASK 0xf
unsigned int g_dev_id_mask = 0;
dev_upgrade_core_ctrl g_upgrade_core_ctrl[DEVICE_NUM_MAX] = {{0}};

#define UPGRADE_DEV_CLASS "upgrade_dev_class"
#define DEV_UPGRADE_NAME_KER "upgrade"
#define UPGRADE_DEV_MAX_COUNT 1
#define RECOVERY_FLAG_VALUE 0x464F5243
#define RECOVERY_FLAG_ENABLE 0x1
#define RECOVERY_FLAG_DISABLE 0x0
#define NEED_PERMISSION_CHECK 1
#define NOT_NEED_PERMISSION_CHECK 0
#define TEE_ERROR_NOT_SUPPORTED 0xFFFF000A
#define BUFF_SIZE_16 16

#define CHECK_PARA_SIZE_RETURN(size, para_size)            \
    do {                                                   \
        if ((size) != (para_size)) {                       \
            dev_upgrade_err("prarameter_size is error\n"); \
            return (-EINVAL);                              \
        }                                                  \
    } while (0)

#if (!defined CFG_SOC_PLATFORM_MDC_V51)
#define ANOTHER_AREA_GET(a) (((a) == FLASH_AREA_0) ? (FLASH_AREA_1) : (FLASH_AREA_0))

struct upgrade_component g_upgrade_component_list[] = {
#if (defined CFG_SOC_PLATFORM_CLOUD)
    {0,
        DSMI_COMPONENT_TYPE_NVE,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_UEFI,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_IMU,
        DOUBLE_AREA,
        0,
        FLASH_AREA_1,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_IMP,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
#elif (defined CFG_SOC_PLATFORM_MINI)
    {0,
        DSMI_COMPONENT_TYPE_NVE,
        SINGLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_nve,
        dev_upgrade_update_single_area},
    {0,
        DSMI_COMPONENT_TYPE_XLOADER,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_M3FW,
        SINGLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_single_area_stub},
    {0,
        DSMI_COMPONENT_TYPE_UEFI,
        SINGLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_single_area},
    {0,
        DSMI_COMPONENT_TYPE_TEE,
        SINGLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_single_area_stub},
    {0,
        DSMI_COMPONENT_TYPE_DTB,
        SINGLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_single_area_stub},
    {0,
        DSMI_COMPONENT_TYPE_KERNEL,
        SINGLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_single_area_stub},
    {0,
        DSMI_COMPONENT_TYPE_ROOTFS,
        SINGLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_single_area_stub},
#elif (defined CFG_SOC_PLATFORM_HELPER)
    {0,
        DSMI_COMPONENT_TYPE_HBOOT1_A,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HBOOT1_B,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HBOOT2,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_DDR,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_LP,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HSM,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HILINK,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
#elif (defined CFG_SOC_PLATFORM_MINIV3)
    {0,
        DSMI_COMPONENT_TYPE_HBOOT1_A,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HBOOT1_B,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HILINK,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
#if (defined CFG_FEATURE_RC_MODE)
    {0,
        DSMI_COMPONENT_TYPE_HSM,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_DDR,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_ATF,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HBOOT2,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_SYS_BASE_CONFIG,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_USER_BASE_CONFIG,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
#endif
#elif (defined CFG_SOC_PLATFORM_MINIV2)
    {0,
        DSMI_COMPONENT_TYPE_HBOOT1_A,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HBOOT1_B,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HILINK,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
#elif (defined CFG_SOC_PLATFORM_CLOUD_V2)
    {0,
        DSMI_COMPONENT_TYPE_HBOOT1_A,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HBOOT1_B,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HILINK,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HILINK2,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
#else
    {0,
        DSMI_COMPONENT_TYPE_HBOOT1_A,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HBOOT1_B,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HBOOT2,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_DDR,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_LP,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HSM,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_SAFETY_ISLAND,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
    {0,
        DSMI_COMPONENT_TYPE_HILINK,
        DOUBLE_AREA,
        0,
        FLASH_AREA_0,
        1,
        0,
        NULL,
        dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area},
#endif
    {0, DSMI_COMPONENT_TYPE_MAX, 0, 0, FLASH_AREA_0, 0, 0, NULL, NULL, NULL}};

#ifdef CFG_SOC_PLATFORM_CLOUD

STATIC void alloc_black_box(void)
{
    unsigned int i;
    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        mutex_init(&g_upgrade_core_ctrl[i].black_box_info.lock);
        g_upgrade_core_ctrl[i].black_box_info.flag = false;
        g_upgrade_core_ctrl[i].black_box_info.buf = (char *)kzalloc(BLACK_BOX_BUF_LEN, GFP_KERNEL | __GFP_ACCOUNT);
        if (g_upgrade_core_ctrl[i].black_box_info.buf == NULL) {
            dev_upgrade_warn("device id %d alloc black box mem fail\n", g_upgrade_core_ctrl[i].dev_id);
            continue;
        }
    }
}

STATIC void free_black_box(void)
{
    unsigned int i;
    struct black_box_buff_info *buff_info = NULL;

    (void)dfm_unregister_module(DFM_MODULE_ID_DRIVER, DFM_SUBMODULE_ID_UPGRADE);

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        buff_info = &g_upgrade_core_ctrl[i].black_box_info;
        mutex_destroy(&buff_info->lock);
        if (buff_info->buf != NULL) {
            kfree(buff_info->buf);
            buff_info->buf = NULL;
        }
    }
}

STATIC void print_to_black_box_info(int dev_id, const char *data)
{
    int ret;
    int offset;
    int remain_size;
    struct black_box_buff_info *buff_info = NULL;
    DRV_CHECK_RET((data != NULL));
    DRV_CHECK_RET((dev_id >= 0));
    DRV_CHECK_RET((dev_id < DEVICE_NUM_MAX));
    DRV_CHECK_RET((g_upgrade_core_ctrl[dev_id].black_box_info.buf != NULL));

    buff_info = &g_upgrade_core_ctrl[dev_id].black_box_info;
    mutex_lock(&buff_info->lock);
    buff_info->flag = true;
    remain_size = BLACK_BOX_BUF_LEN - buff_info->w_offset;

    if (remain_size <= 0) {
        buff_info->w_offset = 0;
        ret = memset_s(buff_info->buf, BLACK_BOX_BUF_LEN, 0, BLACK_BOX_BUF_LEN);
        if (ret != 0) {
            dev_upgrade_warn("memset error ret = %d\n", ret);
        }
    }
    offset = snprintf_s(buff_info->buf + buff_info->w_offset,
        remain_size,
        remain_size - 1,
        "device_id = %d, fail info: %s\n ",
        dev_id,
        data);
    if (offset == -1) {
        mutex_unlock(&buff_info->lock);
        dev_upgrade_err("write print_to_black_box_buf failed, only partial datas are writed\n");
        buff_info->w_offset = 0;
        ret = memset_s(buff_info->buf, BLACK_BOX_BUF_LEN, 0, BLACK_BOX_BUF_LEN);
        if (ret != 0) {
            dev_upgrade_err("memset error ret = %d\n", ret);
        }
        return;
    }

    dev_upgrade_info("abnormal info %s\n", buff_info->buf + buff_info->w_offset);
    buff_info->w_offset = buff_info->w_offset + offset;
    mutex_unlock(&buff_info->lock);
}

STATIC void upgrade_bbox_dump(
    unsigned int dev_id, unsigned int excep_id, unsigned int etype, unsigned int module_id, char **black_box_info)
{
    struct black_box_buff_info *box_info = NULL;
    DRV_CHECK_RET(black_box_info != NULL);
    DRV_CHECK_RET((dev_id < DEVICE_NUM_MAX));

    box_info = &g_upgrade_core_ctrl[dev_id].black_box_info;
    mutex_lock(&box_info->lock);
    *black_box_info = box_info->buf;
    dev_upgrade_info("black read info %s\n", *black_box_info);
    box_info->w_offset = 0;
    mutex_unlock(&box_info->lock);
    return;
}

STATIC int register_black_box(void)
{
    int ret;

    struct dfm_module_register black_module_info = {0};

    alloc_black_box();
    black_module_info.module_id = DFM_MODULE_ID_DRIVER;
    black_module_info.sub_module_id = DFM_SUBMODULE_ID_UPGRADE;
    black_module_info.ops_dump = upgrade_bbox_dump;
    dev_upgrade_info("upgrade sub module id = %u\n", black_module_info.sub_module_id);
    ret = dfm_register_module(&black_module_info);
    if (ret != 0) {
        dev_upgrade_err("register black fail, ret = %d\n", ret);
        free_black_box();
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_DFM_REGISTER);
    }

    return 0;
}

#endif
#endif

STATIC void dev_upgrade_devid_enable(int dev_id, unsigned int enable)
{
    if (enable) {
        g_dev_id_mask |= ((1U << (unsigned int)dev_id) & DEV_ID_MASK);
    } else {
        g_dev_id_mask &= ~((1U << (unsigned int)dev_id) & DEV_ID_MASK);
    }

    return;
}

STATIC int dev_upgrade_check_call_process(void)
{
#ifdef CFG_SOC_PLATFORM_MDC_V11
    return 0;
#else
    int ret;
    const char *wl_process_name = CHECK_PROCESS_DMP;

    /* check process name and related bin/so excutable section sha256 */
    ret = whitelist_process_handler(&wl_process_name, 1);
    if (ret) {
        dev_upgrade_err("Permission denied! ret = %d.\n", ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_PERMISSION_DENIED);
    }

    return 0;
#endif
}

#define CHECK_PROCESS_PERMISSION()                                           \
    do {                                                                     \
        if (dev_upgrade_check_call_process() != 0) {                         \
            dev_upgrade_err("Permission denied!\n");                         \
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_PERMISSION_DENIED); \
        }                                                                    \
    } while (0)

STATIC int dev_upgrade_devid_enable_check(int dev_id)
{
    return (g_dev_id_mask & (1U << (unsigned int)dev_id)) ? TRUE : FALSE;
}

STATIC dev_upgrade_core_ctrl *dev_upgrade_get_upgrade_core_ctrl(int dev_id)
{
    if ((dev_id >= DEVICE_NUM_MAX) || (dev_id < 0)) {
        dev_upgrade_err("dev(%d) invalid\n", dev_id);
        return NULL;
    }

    return &g_upgrade_core_ctrl[dev_id];
}

#ifndef SUPPORT_TEE_SECURE_UPGRADE
STATIC int dev_upgrade_image_integrity_check(upg_comm_para *comm_para, unsigned int *is_integrity)
{
    int ret;
    unsigned int length = 0;
    unsigned char *buff = NULL;

    *is_integrity = CHECK_NO;

    ret = dev_upgrade_get_image_size(comm_para, &length);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_get_image_size fail, ret: %d\n", comm_para->dev_id, ret);
        return ret;
    }

    if (length > FLASH_SIZE_MAX || length <= SEC_HEAD_SIZE) {
        dev_upgrade_err("dev(%d) length=%u is invalid\n", comm_para->dev_id, length);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    buff = (unsigned char *)vzalloc(length);
    if (buff == NULL) {
        dev_upgrade_err("dev(%d) vzalloc fail.\n", comm_para->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    ret = dev_upgrade_read_flash_image(comm_para, buff, length);
    if (ret != 0) {
        DEV_UPGRADE_VFREE(buff);
        dev_upgrade_err("dev(%d) dev_upgrade_read_flash_image fail, ret: %d\n", comm_para->dev_id, ret);
        return ret;
    }

    if (dev_upgrade_sec_verification(comm_para->dev_id, comm_para->type, buff, length) != 0) {
        DEV_UPGRADE_VFREE(buff);
        dev_upgrade_err("dev(%d) dev_upgrade_sec_verification fail.\n", comm_para->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_HASH_CHECK);
    }

    DEV_UPGRADE_VFREE(buff);
    *is_integrity = CHECK_YES;

    return OK;
}
#endif

int dev_upgrade_schedule_update(
    int dev_id, unsigned int component_type, unsigned int total_size, unsigned int finish_size)
{
    unsigned int i;
    unsigned char schedule_calc;
    unsigned int curr_comp_finish_size;
    struct upgrade_component *component = NULL;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;

    dev_upgrade_info("devid=%d, component type=%u, total size=%u, finished size=%u\n",
        dev_id,
        component_type,
        total_size,
        finish_size);

    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(dev_id);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }

    dev_upgrade_info("upgrade_ctrl->total_size=%u, upgrade_ctrl->finish_size=%u\n",
        upgrade_ctrl->total_size,
        upgrade_ctrl->finish_size);
    for (i = 0; i < upgrade_ctrl->total_component; i++) {
        if (upgrade_ctrl->component_list[i]->component_type == component_type) {
            break;
        }
    }

    if (i == upgrade_ctrl->total_component) {
        dev_upgrade_err("component_type invalid, dev(%d) component_type: %u\n", dev_id, component_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }

    component = upgrade_ctrl->component_list[i];
    dev_upgrade_info("component->finish_areas=%u, component->weight=%u\n", component->finish_areas, component->weight);

    curr_comp_finish_size = (component->finish_areas * total_size + component->weight * finish_size);
    DRV_CHECK_RETV(upgrade_ctrl->total_size != 0, ERRNO_INVALID_LENGTH);
    schedule_calc =
        (unsigned char)(((curr_comp_finish_size + upgrade_ctrl->finish_size) * UPGRADE_SCHEDULE_PERCENTAGE_100) /
                        upgrade_ctrl->total_size);

    if (schedule_calc >= UPGRADE_SCHEDULE_PERCENTAGE_100) {
        dev_upgrade_warn("dev_id %d dev_upgrade_calc_schedule warn, schedule_calc : %u\n", dev_id, schedule_calc);
        schedule_calc = UPGRADE_SCHEDULE_PERCENTAGE_100 - 1;
    }
    dev_upgrade_info(
        "curr_comp_finish_size=%u, upgrade schedule=%u\n", curr_comp_finish_size, (unsigned int)schedule_calc);
    upgrade_ctrl->schedule = schedule_calc;

    return 0;
}

#if (!defined CFG_SOC_PLATFORM_MDC_V51)
STATIC struct upgrade_component *dev_component_list_search(unsigned int component_type)
{
    unsigned int i = 0;
    struct upgrade_component *component_list = &g_upgrade_component_list[0];
    unsigned int count = sizeof(g_upgrade_component_list) / sizeof(struct upgrade_component);

    for (i = 0; i < count; i++) {
        if (component_type == component_list->component_type) {
            return component_list;
        }
        component_list++;
    }

    return NULL;
}

#ifdef CFG_SOC_PLATFORM_MINI
STATIC int dev_upgrade_pre_check_nve(struct upgrade_component *component)
{
    unsigned char *buff = NULL;
    unsigned int file_size;
    unsigned int valid_item;
    unsigned int *p_valid_item = NULL;

    if (component == NULL) {
        dev_upgrade_err("para is null.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    buff = component->file_content;
    file_size = component->file_size;

    if (file_size < NVE_CRC_SUPPORT + 1) {
        dev_upgrade_err("file_size error, file_size: %u\n", file_size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    p_valid_item = (unsigned int *)&buff[NVE_VALID_ITEMS_START_ADDRESS];
    dev_upgrade_info("valid_itme = %u\n", *p_valid_item);
    valid_item = *p_valid_item;
    if (valid_item > NVE_MAX_VALID_ITEM_NUM || valid_item == 0) {
        dev_upgrade_err("valid_item invalid! valid_item = %d\n", valid_item);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if (dev_upgrade_nve_sha256_check(buff, valid_item, file_size)) {
        dev_upgrade_err("dev_upgrade_nve_sha256_check fail!\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_CRC);
    }

    return OK;
}
#endif

STATIC int dev_upgrade_pre_check_common(struct upgrade_component *component)
{
    /**
     * DC&MDC does not currently support security upgrades
     */
#ifdef SUPPORT_TEE_SECURE_UPGRADE
    return OK;
#else
    if (component == NULL) {
        dev_upgrade_err("para is null.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (dev_upgrade_sec_verification(
        component->dev_id, component->component_type, component->file_content, component->file_size) != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_sec_verification fail.\n", component->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_HASH_CHECK);
    }

    return OK;
#endif
}

STATIC int dev_upgrade_version_check(upg_comm_para *comm_para, unsigned int phase, unsigned int *is_same)
{
    int ret;
    unsigned char src_ver[COMM_VERSION_LENGTH] = {0};
    unsigned char dest_ver[COMM_VERSION_LENGTH] = {0};
    unsigned int is_memory = 0;
    upg_comm_para comm_para_tmp = *comm_para;

    /* get version from flash boot area */
    ret = dev_upgrade_component_version_get(&comm_para_tmp, is_memory, src_ver, COMM_VERSION_LENGTH);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) get ver from flash fail, ret:%x\n", comm_para_tmp.dev_id, (unsigned int)ret);
        return ret;
    }

    if (phase == PHASE_SYNC) {
        /* comm_para_tmp->area switch to another */
        comm_para_tmp.area = ANOTHER_AREA_GET(comm_para->area);
    } else {
        is_memory = 1;
    }

    /* get version from flash another area or mem */
    ret = dev_upgrade_component_version_get(&comm_para_tmp, is_memory, dest_ver, COMM_VERSION_LENGTH);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) get ver from mem fail, ret:%x\n", comm_para_tmp.dev_id, (unsigned int)ret);
        return ret;
    }

    if (!memcmp(src_ver, dest_ver, COMM_VERSION_LENGTH)) {
        *is_same = CHECK_YES;
    } else {
        *is_same = CHECK_NO;
    }

    return 0;
}

STATIC unsigned char *dev_upgrade_get_content(upg_comm_para *comm_para, unsigned int *length)
{
    int ret;
    unsigned char *content = NULL;

    ret = dev_upgrade_get_image_size(comm_para, length);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) get image size fail, ret: %d\n", comm_para->dev_id, ret);
        return NULL;
    }

    if (*length > FLASH_SIZE_MAX || *length <= SEC_HEAD_SIZE) {
        dev_upgrade_err("dev(%d) length=%u is invalid\n", comm_para->dev_id, *length);
        return NULL;
    }

    content = (unsigned char *)vzalloc(*length);
    if (content == NULL) {
        dev_upgrade_err("dev(%d) vzalloc fail.\n", comm_para->dev_id);
        return NULL;
    }

    ret = dev_upgrade_read_flash_image(comm_para, content, *length);
    if (ret != 0) {
        DEV_UPGRADE_VFREE(content);
        dev_upgrade_err("dev(%d) read flash image fail, ret: %d\n", comm_para->dev_id, ret);
        return NULL;
    }

    return content;
}

STATIC int dev_upgrade_content_check(upg_comm_para *comm_para, unsigned int phase, unsigned int *is_same)
{
    unsigned int src_len = 0;
    unsigned int dst_len = 0;
    unsigned char *src_buf = NULL;
    unsigned char *dst_buf = NULL;
    upg_comm_para comm_para_tmp = *comm_para;

    /* get contont from boot area */
    src_buf = dev_upgrade_get_content(&comm_para_tmp, &src_len);
    if (src_buf == NULL) {
        dev_upgrade_err("dev(%d) get content fail\n", comm_para->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    if (phase == PHASE_SYNC) {
        comm_para_tmp.area = ANOTHER_AREA_GET(comm_para->area);
        /* get contont from another area */
        dst_buf = dev_upgrade_get_content(&comm_para_tmp, &dst_len);
    } else {
        /* contont from mem */
        dst_buf = comm_para->file_content;
        dst_len = comm_para->file_size;
    }
    if (dst_buf == NULL) {
        DEV_UPGRADE_VFREE(src_buf);
        dev_upgrade_err("dev(%d) dst buf is NULL\n", comm_para->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    if (src_len != dst_len) {
        *is_same = CHECK_NO;
    } else {
        *is_same = (!memcmp(src_buf, dst_buf, src_len)) ? CHECK_YES : CHECK_NO;
    }

    DEV_UPGRADE_VFREE(src_buf);

    if (phase == PHASE_SYNC) {
        DEV_UPGRADE_VFREE(dst_buf);
    }

    return 0;
}

STATIC int dev_upgrade_ver_and_content_check(upg_comm_para *comm_para, unsigned int phase, unsigned int *is_same)
{
    int ret;
    unsigned int is_same_version = CHECK_NO;
    unsigned int is_same_content = CHECK_NO;

    ret = dev_upgrade_version_check(comm_para, phase, &is_same_version);
    if (ret != 0) {
        dev_upgrade_err("version check fail, ret: %x\n", (unsigned int)ret);
        return ret;
    }

    ret = dev_upgrade_content_check(comm_para, phase, &is_same_content);
    if (ret != 0) {
        dev_upgrade_err("content check fail, ret: %x\n", (unsigned int)ret);
        return ret;
    }

    if ((is_same_version == CHECK_YES) && (is_same_content == CHECK_YES)) {
        *is_same = CHECK_YES;
        dev_upgrade_info("the version and content are the same.\n");
    } else {
        *is_same = CHECK_NO;
    }
    return 0;
}

STATIC int dev_upgrade_write_flash_and_check(upg_comm_para *comm_para)
{
    int ret;
    unsigned char *read_buff = NULL;

    ret = dev_upgrade_save_to_flash(comm_para, (dev_upgrade_scheule_handle)dev_upgrade_schedule_update);
    if (ret != 0) {
        dev_upgrade_err(
            "dev(%d) update fail, component%u area : %u\n", comm_para->dev_id, comm_para->type, comm_para->area);
        return ret;
    }

    read_buff = (unsigned char *)vzalloc(comm_para->file_size);
    if (read_buff == NULL) {
        dev_upgrade_err("dev(%d) vzalloc fail.\n", comm_para->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    ret = dev_upgrade_read_flash_image(comm_para, read_buff, comm_para->file_size);
    if (ret != 0) {
        DEV_UPGRADE_VFREE(read_buff);
        dev_upgrade_err("dev(%d) dev_upgrade_read_flash_image fail, ret: %d\n", comm_para->dev_id, ret);
        return ret;
    }

    ret = memcmp(read_buff, comm_para->file_content, comm_para->file_size);
    if (ret != 0) {
        DEV_UPGRADE_VFREE(read_buff);
        dev_upgrade_err("dev(%d) flash image check fail, ret: %x.\n", comm_para->dev_id, (unsigned int)ret);
        return ret;
    }

    DEV_UPGRADE_VFREE(read_buff);

    return OK;
}

#ifdef CFG_SOC_PLATFORM_MINI

/* This is for compatibility with earlier versions. */
STATIC int dev_upgrade_update_single_area_stub(struct upgrade_component *component)
{
    dev_upgrade_info("dev(%d) component(%u) cannot be upgraded.\n", component->dev_id, component->component_type);
    return 0;
}

STATIC int dev_upgrade_update_single_area(struct upgrade_component *component)
{
    int ret;
    unsigned int is_same = CHECK_NO;
    upg_comm_para comm_para = {0};

    if (component->total_areas != SINGLE_AREA) {
        dev_upgrade_err("dev(%u) type(%u) total_areas error.\n", component->dev_id, component->component_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    comm_para.dev_id = component->dev_id;
    comm_para.type = component->component_type;
    comm_para.area = FLASH_AREA_0;
    comm_para.file_content = component->file_content;
    comm_para.file_size = component->file_size;

    if (comm_para.type != DSMI_COMPONENT_TYPE_NVE) {
        ret = dev_upgrade_ver_and_content_check(&comm_para, PHASE_UPGRADE, &is_same);
        if (ret != 0) {
            dev_upgrade_err("dev(%d) ver and content check fail, ret:%x\n", comm_para.dev_id, ret);
            return ret;
        }

        if (is_same == CHECK_YES) {
            dev_upgrade_warn("dev(%d) the version and content are the same.\n", comm_para.dev_id);
            return 0;
        }
    }

    /* upgrade partitions and read back validation */
    component->finish_areas = 0;
    ret = dev_upgrade_write_flash_and_check(&comm_para);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) component(%u) upgrade fail, ret: %d\n", comm_para.dev_id, comm_para.type, ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
    }

    component->finish_areas++;
    dev_upgrade_info("dev(%d) component(%u) upgrade succ.\n", comm_para.dev_id, comm_para.type);

    return ret;
}
#endif

STATIC int dev_upgrade_double_image_check(upg_comm_para *comm_para, unsigned int *img_state)
{
    /**
     * DC&MDC does not currently support security upgrades
     */
#ifdef SUPPORT_TEE_SECURE_UPGRADE
    *img_state = IMG_STAT_DOUBLE_BAD;
    return 0;
#else
    int ret;
    unsigned int index;
    unsigned int check_res = CHECK_YES;
    upg_comm_para comm_para_tmp = *comm_para;

    /* comm_para->area default is main */
    for (index = 0; index < DEV_FLASH_DOUBLE_AREAS; index++) {
        ret = dev_upgrade_image_integrity_check(&comm_para_tmp, &check_res);
        if (ret != 0) {
            dev_upgrade_warn(
                "dev(%d) dev_upgrade_image_integrity_check fail, ret:%x\n", comm_para->dev_id, (unsigned int)ret);
        }

        /* bit0 -> main area, bit1 -> standby area */
        *img_state |= (check_res == CHECK_NO) ? (1U << index) : 0;

        /* comm_para->area switch to standby */
        comm_para_tmp.area = ANOTHER_AREA_GET(comm_para->area);
    }

    dev_upgrade_info("dev(%d) component(%u) image state: %u\n", comm_para->dev_id, comm_para->type, *img_state);

    return 0;
#endif
}

STATIC int dev_upgrade_update_image(
    struct upgrade_component *component, upg_comm_para *comm_para, unsigned int upgrade_flag)
{
    int ret;
    unsigned int fail_cnt = 0;
    unsigned int op_area = FLASH_AREA_0;
    unsigned int index = 0;
    upg_comm_para comm_para_tmp = *comm_para;

    if (upgrade_flag == UPGRADE_DOUBLE) {
        component->finish_areas = 0;
        for (index = 0; index < DEV_FLASH_DOUBLE_AREAS; index++) {
            comm_para_tmp.area = index;
            ret = dev_upgrade_write_flash_and_check(&comm_para_tmp);
            if (ret != 0) {
                fail_cnt++;
                dev_upgrade_err("dev_upgrade_write_flash_and_check is failed. (dev=%d)\n", component->dev_id);
            }

            component->finish_areas++;
        }

        if (fail_cnt != 0) {
            dev_upgrade_info("Upgrading double area is abnormal. (dev=%d; component=%u)\n",
                component->dev_id,
                component->component_type);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
        }
    } else if (upgrade_flag == UPGRADE_MASTER) {
        component->finish_areas = 0;
        component->weight = DEV_FLASH_DOUBLE_AREAS;
        ret = dev_upgrade_write_flash_and_check(&comm_para_tmp);
        if (ret != 0) {
            dev_upgrade_err("Failed to upgrade the component and would stop upgrading. (dev=%d; component=%u)\n",
                component->dev_id,
                component->component_type);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
        }

        component->finish_areas = component->total_areas;
        dev_upgrade_info("The component was upgraded successfully. (dev=%d; component=%u; area=%u)\n",
            component->dev_id,
            component->component_type,
            component->main_area);
    }

    /* clear start fail count for selecting boot from main area after reboot */
    ret = dev_upgrade_component_boot_area_op(
        component->dev_id, component->component_type, DEV_CLEAR_BOOT_COUNT, &op_area);
    if (ret != 0) {
        dev_upgrade_err("Failed to clear boot-count. (ret=%x)\n", (unsigned int)ret);
        return ret;
    }

    return ret;
}

STATIC int dev_upgrade_update_double_area(struct upgrade_component *component)
{
    int ret;
    unsigned int img_state = 0;
    unsigned int standby_area;
    unsigned int is_same = 0;
    unsigned int upgrade_flag = UPGRADE_MASTER;
    upg_comm_para comm_para = {0};

    if (component->total_areas != DOUBLE_AREA) {
        dev_upgrade_err("dev(%d) type(%u) total_areas error.\n", component->dev_id, component->component_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    comm_para.dev_id = component->dev_id;
    comm_para.type = component->component_type;
    comm_para.area = component->main_area;
    comm_para.file_content = component->file_content;
    comm_para.file_size = component->file_size;
    standby_area = ANOTHER_AREA_GET(component->main_area);

    ret = dev_upgrade_double_image_check(&comm_para, &img_state);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_double_image_check fail, ret:%x\n", (unsigned int)ret);
        return ret;
    }

    if (img_state == IMG_STAT_DOUBLE_OK) {
        ret = dev_upgrade_ver_and_content_check(&comm_para, PHASE_UPGRADE, &is_same);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_ver_and_content_check fail, ret:%x\n", (unsigned int)ret);
            return ret;
        }
        upgrade_flag = (is_same == CHECK_YES) ? UPGRADE_NONE : UPGRADE_MASTER;
    } else if (img_state == IMG_STAT_M_OK_S_BAD) {
        /* sync the main area to the standby area first, and then upgrade the main area */
        ret = dev_upgrade_component_image_copy(comm_para.dev_id, comm_para.type, standby_area, comm_para.area);
        if (ret != 0) {
            dev_upgrade_err("dev(%d) component(%u) copy from area(%u) to area(%u) fail, ret: %d\n",
                comm_para.dev_id,
                comm_para.type,
                comm_para.area,
                standby_area,
                ret);
            return ret;
        }
    } else if (img_state == IMG_STAT_DOUBLE_BAD) {
        /* both areas are damaged, and both areas are upgraded */
        upgrade_flag = UPGRADE_DOUBLE;
    } else {
        /* upgrade main area by default, nothing to do here */
    }

    ret = dev_upgrade_update_image(component, &comm_para, upgrade_flag);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_update_image fail, ret:%x\n", component->dev_id, (unsigned int)ret);
        return ret;
    }

    return 0;
}

void dev_upgrade_core_resource_release(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    unsigned int index = 0;

    for (index = 0; index < upgrade_ctrl->total_component; index++) {
        if (upgrade_ctrl->component_list[index] != NULL) {
            if (upgrade_ctrl->component_list[index]->file_content != NULL) {
                DEV_UPGRADE_VFREE(upgrade_ctrl->component_list[index]->file_content);
            }
            DEV_UPGRADE_VFREE(upgrade_ctrl->component_list[index]);
        }
    }

    upgrade_ctrl->clr_localcfg = 0;
    upgrade_ctrl->total_component = 0;
    upgrade_ctrl->stop_flag = 0;

    return;
}

void dev_upgrade_localcfg_proc(dev_upgrade_core_ctrl *upgrade_ctrl, struct upgrade_component *component)
{
#ifdef CFG_SOC_PLATFORM_CLOUD
    unsigned int bios_type = DSMI_COMPONENT_TYPE_IMU;
#elif (defined CFG_SOC_PLATFORM_MINIV2) || (defined CFG_SOC_PLATFORM_MINIV3)
    unsigned int bios_type = DSMI_COMPONENT_TYPE_HILINK;
#else
    unsigned int bios_type = DSMI_COMPONENT_TYPE_XLOADER;
#endif

    if (upgrade_ctrl->clr_localcfg && (component->component_type == bios_type)) {
        dev_upgrade_clear_localcfg(upgrade_ctrl->dev_id);
        upgrade_ctrl->clr_localcfg = 0;
    }
}
#endif

#ifndef SUPPORT_TEE_SECURE_UPGRADE
STATIC int dev_upgrade_unsec_update_proc(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int ret = 0;
    unsigned int index;
    unsigned int finish = 0;
    struct upgrade_component *component = NULL;

    /* before writing flash, validate all files */
    for (index = 0; index < upgrade_ctrl->total_component; index++) {
        component = upgrade_ctrl->component_list[index];
        if (component == NULL) {
            dev_upgrade_err("component index%u is null\n", index);
            goto out;
        }

        if (component->pre_check != NULL) {
            ret = component->pre_check(component);
            if (ret != 0) {
                dev_upgrade_err("pre_check err, ret = %d, index = %u\n", ret, index);
                goto out;
            }
        }

        if (upgrade_ctrl->stop_flag == DEV_UPGRADE_STOP_FLAG) {
            dev_upgrade_warn("update stop! flash has not been written yet\n");
            goto out;
        }
    }

    upgrade_ctrl->finish_size = 0;
    for (finish = 0; finish < upgrade_ctrl->total_component; finish++) {
        component = upgrade_ctrl->component_list[finish];
        if (component->update != NULL) {
            ret = component->update(component);
            if (ret != 0) {
                dev_upgrade_err("update err, ret = %d, index = %u\n", ret, finish);
                goto out;
            }
        }

        dev_upgrade_localcfg_proc(upgrade_ctrl, component);
        upgrade_ctrl->finish_size += (component->file_size * component->total_areas);
    }
out:
    if (finish == upgrade_ctrl->total_component) {
        dev_upgrade_info("upgrade %u files succ\n", upgrade_ctrl->total_component);
        upgrade_ctrl->schedule = UPGRADE_SCHEDULE_PERCENTAGE_100;
        ret = OK;
    } else {
        dev_upgrade_err("upgrade fail, %u/%u files finish!\n", finish, upgrade_ctrl->total_component);
        ret = (ret == dev_errno_make(DEV_MID_UPGRADE, ERRNO_VERSION_NOT_MATCH)) ?
            dev_errno_make(DEV_MID_UPGRADE, ERRNO_VERSION_NOT_MATCH) : ERROR;
    }

    dev_upgrade_core_resource_release(upgrade_ctrl);

    return ret;
}
#endif

STATIC int dev_upgrade_update_proc(dev_upgrade_core_ctrl *upgrade_ctrl)
{
#if (defined SUPPORT_TEE_SECURE_UPGRADE)

#ifndef CFG_SOC_PLATFORM_MDC_V51
    int ret;

    ret = dev_upgrade_sec_upgrade_proc(upgrade_ctrl);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_sec_upgrade_proc fail, ret=%d.\n", upgrade_ctrl->dev_id, ret);
        return ret;
    }

    return ret;
#else
    return dev_upgrade_mdc_update_sync_proc(DISK_OP_TYPE_UPDATE, upgrade_ctrl);
#endif

#else
    return dev_upgrade_unsec_update_proc(upgrade_ctrl);
#endif
}

#ifndef SUPPORT_TEE_SECURE_UPGRADE
STATIC void dev_upgrade_sync_info_print(upg_comm_para *comm_para)
{
    int ret;
    unsigned char src_ver[COMM_VERSION_LENGTH] = {0};
    unsigned char dest_ver[COMM_VERSION_LENGTH] = {0};
    unsigned int *pver_src = (unsigned int *)&src_ver;
    unsigned int *pver_dest = (unsigned int *)&dest_ver;
    upg_comm_para comm_para_tmp = *comm_para;

    ret = dev_upgrade_component_version_get(&comm_para_tmp, 0, src_ver, COMM_VERSION_LENGTH);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) get ver from flash fail, ret:%x\n", comm_para_tmp.dev_id, (unsigned int)ret);
        return;
    }
    dev_upgrade_info("[M.F.R.P.B] dev%d component(%u) area%u version is %x.%x.%x.%x.%x\n",
        comm_para_tmp.dev_id,
        comm_para_tmp.type,
        comm_para_tmp.area,
        pver_src[INDEX_0],
        pver_src[INDEX_1],
        pver_src[INDEX_2],
        ((pver_src[INDEX_3] >> BITS_OF_SHORT) & 0xffff),
        (pver_src[INDEX_3] & 0xffff));

    /* msg->area switch to another */
    comm_para_tmp.area = ANOTHER_AREA_GET(comm_para->area);
    ret = dev_upgrade_component_version_get(&comm_para_tmp, 0, dest_ver, COMM_VERSION_LENGTH);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) get ver from flash fail, ret:%x\n", comm_para_tmp.dev_id, (unsigned int)ret);
        return;
    }
    dev_upgrade_info("[M.F.R.P.B] dev%d component(%u) area%u version is %x.%x.%x.%x.%x\n",
        comm_para_tmp.dev_id,
        comm_para_tmp.type,
        comm_para_tmp.area,
        pver_dest[INDEX_0],
        pver_dest[INDEX_1],
        pver_dest[INDEX_2],
        ((pver_dest[INDEX_3] >> BITS_OF_SHORT) & 0xffff),
        (pver_dest[INDEX_3] & 0xffff));
}
#endif

#ifdef CFG_SOC_PLATFORM_CLOUD

STATIC void check_start_up_from_back(int dev_id, unsigned int component_type, unsigned int from_area)
{
    int offset;
    bool start_flag = true;
    char temp_buff[MAX_INFO_LEN] = {0};

    /* start from back area */
    if ((component_type == DSMI_COMPONENT_TYPE_IMU && from_area == FLASH_AREA_0) ||
        (component_type != DSMI_COMPONENT_TYPE_IMU && from_area == FLASH_AREA_1)) {
        start_flag = false;
    }
    if (start_flag == false) {
        offset = snprintf_s(temp_buff,
            MAX_INFO_LEN,
            MAX_INFO_LEN - 1,
            "device_id = %d, componet type  = %u start up from back area,need rollback\n",
            dev_id,
            component_type);
        if (offset == -1) {
            dev_upgrade_err("snprintf_s error ret = %d\n", offset);
            return;
        }
        print_to_black_box_info(dev_id, temp_buff);
    }
    return;
}

STATIC void sync_fail_write_black_box(
    int dev_id, unsigned int component_type, unsigned int from_area, unsigned int to_area)
{
    int offset;
    char temp_buff[MAX_INFO_LEN] = {0};

    offset = snprintf_s(temp_buff,
        MAX_INFO_LEN,
        MAX_INFO_LEN - 1,
        "device_id = %d, componet type  = %u sync from %u area to %u area fail\n",
        dev_id,
        component_type,
        from_area,
        to_area);
    if (offset == -1) {
        dev_upgrade_err("snprintf_s error ret = %d\n", offset);
        return;
    }
    print_to_black_box_info(dev_id, temp_buff);
    return;
}

#endif

#ifndef SUPPORT_TEE_SECURE_UPGRADE
STATIC int dev_upgrade_component_sync_check(upg_comm_para *comm_para, unsigned int *need_sync)
{
    int ret = 0;
    unsigned int fail_count = 0;
    unsigned int is_same = CHECK_NO;

    *need_sync = CHECK_NO;
    if (comm_para->type == DSMI_COMPONENT_TYPE_IMP) {
        ret = dev_upgrade_component_boot_area_op(
            comm_para->dev_id, comm_para->type, DEV_GET_BOOT_FAIL_COUNT, &fail_count);
        if (ret != 0) {
            dev_upgrade_err("dev(%d) get imp main boot fail count fail, ret: %d\n", comm_para->dev_id, ret);
            return ret;
        }
        if (fail_count != 0) {
            dev_upgrade_warn("dev(%d) curr imp exception, not sync, fail_count: %u\n", comm_para->dev_id, fail_count);
            *need_sync = CHECK_NO;
            return 0;
        }
    }

    ret = dev_upgrade_ver_and_content_check(comm_para, PHASE_SYNC, &is_same);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_ver_and_content_check fail, ret:%x\n", (unsigned int)ret);
        return ret;
    }

    *need_sync = (is_same == CHECK_YES) ? CHECK_NO : CHECK_YES;

    if (*need_sync == CHECK_YES) {
        dev_upgrade_sync_info_print(comm_para);
    }

    return 0;
}

STATIC int dev_upgrade_sync_image(int dev_id, unsigned int component_type)
{
    int ret;
    unsigned int from_area = 0;
    unsigned int to_area;
    unsigned int need_sync = 0;
    unsigned int check_res = CHECK_YES;
    upg_comm_para comm_para = {0};

    ret = dev_upgrade_component_boot_area_op(dev_id, component_type, DEV_GET_CURR_BOOT_AREA, &from_area);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) get curr boot area fail, boot_area: %u, component type: %u, ret: %d.\n",
            dev_id,
            from_area,
            component_type,
            ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    to_area = ANOTHER_AREA_GET(from_area);

#ifdef CFG_SOC_PLATFORM_CLOUD
    check_start_up_from_back(dev_id, component_type, from_area);
#endif

    comm_para.dev_id = dev_id;
    comm_para.type = component_type;
    comm_para.area = from_area;
    /* before sync, verify the signature of the current area */
    ret = dev_upgrade_image_integrity_check(&comm_para, &check_res);
    if ((ret != 0) || (check_res != CHECK_YES)) {
        dev_upgrade_err("dev(%d) component(%u) arae%u image check fail, need to report alarm, ret: %d\n",
            dev_id,
            component_type,
            from_area,
            ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_HASH_CHECK);
    }

    ret = dev_upgrade_component_sync_check(&comm_para, &need_sync);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_component_sync_check fail, ret:%x\n", dev_id, (unsigned int)ret);
        return ret;
    }
    if (need_sync) {
        ret = dev_upgrade_component_image_copy(dev_id, component_type, to_area, from_area);
        if (ret != 0) {
            dev_upgrade_err(
                "dev(%d) component(%u) sync from area%u to area%u fail.\n", dev_id, component_type, from_area, to_area);
#ifdef CFG_SOC_PLATFORM_CLOUD
            sync_fail_write_black_box(dev_id, component_type, from_area, to_area);
#endif
            return ret;
        } else {
            dev_upgrade_info("dev(%d) component(%u) sync from area%u to area%u success.\n",
                dev_id,
                component_type,
                from_area,
                to_area);
        }
    }

    if (component_type != DSMI_COMPONENT_TYPE_IMP) {
        ret = dev_upgrade_component_boot_area_op(dev_id, component_type, DEV_CLEAR_BOOT_COUNT, NULL);
        if (ret != 0) {
            dev_upgrade_warn("dev(%d) clear boot count fail, ret:%x\n", dev_id, (unsigned int)ret);
            return ret;
        }
    }

    return OK;
}

int dev_upgrade_sync_proc(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int ret;
    unsigned int i = 0;
    unsigned int fail_cnt = 0;
    unsigned int bitmap = 0;
    struct upgrade_component *component = NULL;

    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("upgrade_ctrl is NULL\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    ret = dev_upgrade_get_component_bitmap(upgrade_ctrl->dev_id, &bitmap);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_get_component_bitmap fail\n", upgrade_ctrl->dev_id);
        return ret;
    }

    for (i = 0; i < DSMI_COMPONENT_TYPE_MAX; i++) {
        component = dev_component_list_search(i);
        if (component == NULL) {
            continue;
        }
        if ((((bitmap >> i) & 0x1) == 0x1) && (component->total_areas == DOUBLE_AREA)) {
            ret = dev_upgrade_sync_image(upgrade_ctrl->dev_id, i);
            if (ret != 0) {
                fail_cnt++;
                dev_upgrade_warn("dev_id: %d component%u sync image fail, ret: %d\n", upgrade_ctrl->dev_id, i, ret);
                continue;
            }
        }
    }
#ifdef CFG_SOC_PLATFORM_CLOUD
    if (upgrade_ctrl->black_box_info.flag == true) {
        dfm_system_error_report(upgrade_ctrl->dev_id, FW_SYNC_FAIL, fail_cnt);
        upgrade_ctrl->black_box_info.flag = false;
    }
#endif
    return ((fail_cnt == 0) ? 0 : -EINVAL);
}
#else

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_img_sync_and_efuse_update(unsigned int dev_id)
{
    dev_upgrade_err("sec_img_sync_and_efuse_update error, esl don't support tee");
    return 0;
}
#endif

int dev_upgrade_firmware_sync_proc(dev_upgrade_core_ctrl *upgrade_ctrl)
{
#if (defined CFG_SOC_PLATFORM_MDC_V51)
    int ret;

    ret = dev_upgrade_sync_pre_check(upgrade_ctrl->dev_id);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_sync_pre_check fail, ret=%d.\n", upgrade_ctrl->dev_id, ret);
        return ret;
    }

    upgrade_ctrl->schedule = SYNC_PROP_START;

    ret = sec_img_sync_and_efuse_update(upgrade_ctrl->dev_id);
    dev_upgrade_debug("dev(%d) sec_img_sync_and_efuse_update ret = %d.\n", upgrade_ctrl->dev_id, ret);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) sec_img_sync_and_efuse_update fail.\n", upgrade_ctrl->dev_id);
        return ret;
    }
    upgrade_ctrl->schedule = SYNC_PROP_FINISHED;

    return ret;
#endif
    dev_upgrade_err("not support to only sync firmware\n");
    return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
}

int dev_upgrade_sync_proc(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int ret;

    ret = dev_upgrade_sync_pre_check(upgrade_ctrl->dev_id);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_sync_pre_check fail, ret=%d.\n", upgrade_ctrl->dev_id, ret);
        return ret;
    }

#if (defined CFG_SOC_PLATFORM_MDC_V51)
    upgrade_ctrl->schedule = SYNC_PROP_START;

    ret = dev_upgrade_sync_ufs_image(upgrade_ctrl->dev_id);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_sync_ufs_image fail.\n", upgrade_ctrl->dev_id);
        return ret;
    }

    upgrade_ctrl->schedule = SYNC_PROP_HALF;

    ret = sec_img_sync_and_efuse_update(upgrade_ctrl->dev_id);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) sec_img_sync_and_efuse_update fail.\n", upgrade_ctrl->dev_id);
        return ret;
    }

    ret = dev_upgrade_set_mdc_flag(
        QUERY_FROM_CMDLINE, UFS_UPGRADE_FLAG_LUN, UFS_FLAG_BASE_OFFSET, UFS_STATE_FLAG_OFFSET, UFS_NORMAL_FLAG);
    if (ret != OK) {
        dev_upgrade_err("set LU(%d) addr(0x%x) state_flag(0x%x) fail, ret: %d\n",
            UFS_UPGRADE_FLAG_LUN,
            UFS_FLAG_BASE_OFFSET + UFS_STATE_FLAG_OFFSET,
            UFS_NORMAL_FLAG,
            ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }
    upgrade_ctrl->schedule = SYNC_PROP_FINISHED;

    dev_upgrade_info("set LU(%d) addr(0x%x) state_flag(0x%x) success.\n",
        UFS_UPGRADE_FLAG_LUN,
        UFS_FLAG_BASE_OFFSET + UFS_STATE_FLAG_OFFSET,
        UFS_NORMAL_FLAG);
#else
    ret = sec_img_sync_and_efuse_update(upgrade_ctrl->dev_id);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) sec_img_sync_and_efuse_update fail.\n", upgrade_ctrl->dev_id);
        return ret;
    }
#endif
    return OK;
}
#endif
#if (!defined CFG_SOC_PLATFORM_MDC_V51)
STATIC struct upgrade_component *dev_upgrade_component_find(int dev_id, unsigned int type, unsigned int file_size)
{
    int ret;
    unsigned int flash_space_size;
    struct upgrade_component *component = NULL;

    ret = dev_upgrade_component_space_get(dev_id, type, &flash_space_size);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) component(%u) check space fail, ret: %x\n", dev_id, type, (unsigned int)ret);
        return NULL;
    }

    if (file_size > flash_space_size || file_size == 0) {
        dev_upgrade_err("dev(%d) component(%u) file_size invalid, file_size: %u, flash_space_size: %u\n",
            dev_id,
            type,
            file_size,
            flash_space_size);
        return NULL;
    }

    component = dev_component_list_search(type);
    if (component == NULL) {
        dev_upgrade_err("dev(%d) component(%u) find component fail\n", dev_id, type);
        return NULL;
    }

    return component;
}

STATIC int dev_upgrade_components_fill(dev_upgrade_core_ctrl *upgrade_ctrl, struct upgrade_start_in *start_in)
{
    unsigned int i, j, file_size, type;
    struct upgrade_component *component = NULL;
    struct upgrade_component *component_find = NULL;

    for (i = 0; i < start_in->count; i++) {
        type = start_in->file_list[i].component_type;
        file_size = start_in->file_list[i].file_size;

        if (start_in->file_list[i].file_content == NULL) {
            dev_upgrade_err("dev(%d) component(%u) check file_content is null\n", upgrade_ctrl->dev_id, type);
            break;
        }

        component_find = dev_upgrade_component_find(upgrade_ctrl->dev_id, type, file_size);
        if (component_find == NULL) {
            dev_upgrade_err("dev(%d) find component fail, component_type: %u\n", upgrade_ctrl->dev_id, type);
            break;
        }

        component = (struct upgrade_component *)(void *)vzalloc(sizeof(struct upgrade_component));
        if (component == NULL) {
            dev_upgrade_err("dev(%d) malloc fail\n", upgrade_ctrl->dev_id);
            break;
        }

        *component = *component_find;
        component->dev_id = upgrade_ctrl->dev_id;
        component->file_size = file_size;
        component->file_content = (unsigned char *)(void *)vzalloc(file_size);
        if (component->file_content == NULL) {
            dev_upgrade_err("dev(%d) vzalloc failed!\n", upgrade_ctrl->dev_id);
            DEV_UPGRADE_VFREE(component);
            break;
        }

        if (copy_from_user((void *)component->file_content, (void *)start_in->file_list[i].file_content, file_size)) {
            dev_upgrade_err("dev(%d) copy_from_user failed\n", upgrade_ctrl->dev_id);
            DEV_UPGRADE_VFREE(component->file_content);
            DEV_UPGRADE_VFREE(component);
            break;
        }

        upgrade_ctrl->component_list[i] = component;
        upgrade_ctrl->total_size += (component->file_size * component->total_areas);
    }

    if (i != start_in->count) {
        for (j = 0; j < i; j++) {
            DEV_UPGRADE_VFREE(upgrade_ctrl->component_list[j]->file_content);
            DEV_UPGRADE_VFREE(upgrade_ctrl->component_list[j]);
        }
        dev_upgrade_err("dev(%d) rollback %u files\n", upgrade_ctrl->dev_id, i);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    return 0;
}
#endif

STATIC int dev_upgrade_ioctl_firmware_start(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;
    struct upgrade_start_in *fw_start_in = NULL;
    struct upgrade_start_out fw_start_out = {0};

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_start_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_start_out));

    fw_start_in = (struct upgrade_start_in *)vzalloc(sizeof(struct upgrade_start_in));
    if (fw_start_in == NULL) {
        dev_upgrade_err("fw_start_in calloc fail\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    if (copy_from_user((void *)fw_start_in, (void *)upgrade_msg->in, sizeof(struct upgrade_start_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        DEV_UPGRADE_VFREE(fw_start_in);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    if (fw_start_in->count > DSMI_COMPONENT_TYPE_MAX) {
        dev_upgrade_err("fw_start_in component type count(%u) invalid.\n", fw_start_in->count);
        DEV_UPGRADE_VFREE(fw_start_in);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(fw_start_in->dev_id);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", fw_start_in->dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
        goto out;
    }

    mutex_lock(&upgrade_ctrl->lock);

    upgrade_ctrl->total_size = 0;
    upgrade_ctrl->finish_size = 0;
    upgrade_ctrl->schedule = 0;
    upgrade_ctrl->total_component = fw_start_in->count;
    dev_upgrade_info("upgrade start, dev(%d), count: %u\n", upgrade_ctrl->dev_id, upgrade_ctrl->total_component);

    ret = dev_upgrade_components_fill(upgrade_ctrl, fw_start_in);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_components_fill fail, ret: %x\n", upgrade_ctrl->dev_id, (unsigned int)ret);
        mutex_unlock(&upgrade_ctrl->lock);
        goto out;
    }

    ret = dev_upgrade_update_proc(upgrade_ctrl);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_update_proc fail, ret: %x\n", upgrade_ctrl->dev_id, (unsigned int)ret);
        mutex_unlock(&upgrade_ctrl->lock);
        goto out;
    }

    ret = 0;
    mutex_unlock(&upgrade_ctrl->lock);

out:
    fw_start_out.result = ret;
    if (copy_to_user((void *)upgrade_msg->out, (void *)&fw_start_out, sizeof(struct upgrade_start_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", fw_start_in->dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }
    DEV_UPGRADE_VFREE(fw_start_in);
    return ret;
}

#ifndef SUPPORT_TEE_SECURE_UPGRADE
STATIC int dev_upgrade_version_get(
    int dev_id, unsigned int type, unsigned int area, unsigned char *buf, unsigned int len)
{
    int ret;
    upg_comm_para comm_para = {0};
    unsigned int boot_area = FLASH_AREA_0;
    struct upgrade_component *component = NULL;

    component = dev_component_list_search(type);
    if (component == NULL) {
        dev_upgrade_err("find component fail, component_type: %u\n", type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    /* In the case of two areas, we need to get the current boot area */
    if (component->total_areas == DEV_FLASH_DOUBLE_AREAS) {
        ret = dev_upgrade_component_boot_area_op(dev_id, type, DEV_GET_CURR_BOOT_AREA, &boot_area);
        if (ret != 0) {
            dev_upgrade_warn("get boot area fail. default area0!!\n");
            boot_area = FLASH_AREA_0;
        }
    }

    comm_para.dev_id = dev_id;
    comm_para.type = type;
    comm_para.area = boot_area;
    comm_para.file_content = NULL;
    comm_para.file_size = 0;

    /* get version from flash */
    ret = dev_upgrade_component_version_get(&comm_para, 0, buf, len);
    if (ret != 0) {
        dev_upgrade_err("get ver fail,component(%u), area: %u\n", comm_para.type, comm_para.area);
        return ret;
    }

    return 0;
}

STATIC int dev_upgrade_get_device_info(int dev_id, unsigned int cmd, unsigned char *in_buf, unsigned int *size)
{
    dev_upgrade_err("dev_id(%d) dev_upgrade_get_device_info not support!\n", dev_id);
    return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
}

#ifndef DEV_UPGRADE_UT
STATIC int dev_upgrade_set_device_info(int dev_id, unsigned int cmd, void *in_buf, unsigned int size)
{
    dev_upgrade_err("dev_id(%d) dev_upgrade_set_device_info not support!\n", dev_id);
    return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
}
#endif

#else

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_flash_get_version(unsigned int dev_id, unsigned int img_id, unsigned char *boot_version, unsigned int max_len,
    unsigned int area_check)
{
    dev_upgrade_err("sec_flash_get_version error, esl don't support tee");
    return 0;
}
#endif

STATIC int dev_upgrade_version_get(
    int dev_id, unsigned int type, unsigned int area, unsigned char *buf, unsigned int len)
{
    int ret;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;

    if (type >= DSMI_COMPONENT_TYPE_MAX) {
        dev_upgrade_err("componet(%u) invalid.\n", type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }

    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(dev_id);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }

#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (dev_upgrade_ufs_valid_chk(type) == OK) {
        ret = dev_upgrade_get_disk_img_version(upgrade_ctrl, type, area, buf, len);
    } else {
        ret = sec_flash_get_version(dev_id, type, (char *)buf, len, area);
    }
#else
    ret = sec_flash_get_version(dev_id, type, (char *)buf, len, area);
#endif
    if (ret != OK) {
        dev_upgrade_warn("get component(0x%x) version fail.\n", type);
        return ret;
    }

    return OK;
}

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_get_fw_verify_result(unsigned int dev_id, unsigned int sub_type, unsigned int *result)
{
    dev_upgrade_err("sec_get_fw_verify_result error, esl don't support tee");
    return 0;
}
#endif

STATIC int dev_upgrade_fw_verify(int dev_id, unsigned char sub_type, unsigned int *result, int size)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    int ret;
    ret = sec_get_fw_verify_result(dev_id, sub_type, result);
    dev_upgrade_debug(
        "dev_id(%d) sec_get_fw_verify_result ret = %d, sub_type = %u, *result = %d.\n", dev_id, ret, sub_type, *result);
    if (ret != OK) {
        dev_upgrade_err(
            "dev_id(%d) sec_get_fw_verify_result failed, ret = %d, sub_type = %u.\n", dev_id, ret, sub_type);
        return ret;
    }

    return ret;
#endif
    dev_upgrade_err("not support to get fw verify\n");
    return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
}

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_read_ufs_resetcnt(unsigned int dev_id, unsigned int *out_value)
{
    dev_upgrade_err("sec_read_ufs_resetcnt error, esl don't support tee");
    return 0;
}
#endif

STATIC int dev_upgrade_get_current_partition(int dev_id, unsigned char sub_type, unsigned int *part_type, int size)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    int ret;
    unsigned int boot_count = 0;

    if ((unsigned int)sub_type != SUB_TYPE_GET_PARTITION) {
        dev_upgrade_err("not support for sub_type(%d), dev_id: %d\n", (unsigned int)sub_type, dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }

    ret = sec_read_ufs_resetcnt((unsigned int)dev_id, &boot_count);
    if (ret != OK) {
        dev_upgrade_err("dev_id(%d) sec_read_ufs_resetcnt failed, ret = %d.\n", dev_id, ret);
        return ret;
    }

    *part_type = ((boot_count % UFS_START_CNT_MASK) < UFS_MASTER_START_MAX) ? PARTITION_A : PARTITION_B;
    dev_upgrade_info("get current partition success, partition = %u\n", *part_type);

    return 0;
#else
    dev_upgrade_err("not support to get current partition type\n");
    return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
#endif
}

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_write_ufs_resetcnt(unsigned int dev_id, unsigned int value)
{
    dev_upgrade_err("sec_write_ufs_resetcnt error, esl don't support tee");
    return 0;
}
#endif

STATIC int dev_upgrade_set_boot_partition(
    int dev_id, unsigned char sub_type, unsigned int *part_type_user, unsigned int size)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    int ret;
    unsigned int part_type;
    if (copy_from_user((void *)&part_type, (void *)part_type_user, sizeof(unsigned int))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    if (size != sizeof(unsigned int)) {
        dev_upgrade_err("buf size(%u) is invalid\n", size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    if ((unsigned int)sub_type != SUB_TYPE_SET_PARTITION) {
        dev_upgrade_err("not support for sub_type(%d), dev_id: %d\n", (unsigned int)sub_type, dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }

    switch (part_type) {
        case PARTITION_A:
            ret = sec_write_ufs_resetcnt(dev_id, RESTART_CNT_VAL);
            break;
        case PARTITION_B:
            ret = sec_write_ufs_resetcnt(dev_id, BACKUP_RESTART_CNT_VAL);
            break;
        default:
            dev_upgrade_err("dev_id(%d) partition type(%u) err\n", dev_id, part_type);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }
    if (ret != OK) {
        dev_upgrade_err("dev_id(%d) set boot partition (%u) failed, ret = %d.\n", dev_id, part_type, ret);
        return ret;
    }
    dev_upgrade_info("dev_id(%d) set partition type(%u) success\n", dev_id, part_type);

    return 0;
#else
    dev_upgrade_err("not support to set boot partition\n");
    return 0;
#endif
}

STATIC int dev_upgrade_get_device_info(int dev_id, unsigned int cmd, void *buf, unsigned int *size)
{
    int ret;
    unsigned int area;
    void *buf_tmp = NULL;
    unsigned short cmd_main;
    unsigned short cmd_minor;
    unsigned char sub_type;
    unsigned int size_tmp = U16_MAX;

    buf_tmp = (void *)vzalloc(BUFF_SIZE_16);
    if (buf_tmp == NULL) {
        dev_upgrade_err("dev_id: %d vzalloc failed.\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    /* high 16 bytes means main cmd, low 16 bytes for minor cmd */
    cmd_main = (unsigned short)((cmd >> BITS_OF_SHORT) & U16_MAX);
    cmd_minor = (unsigned short)(cmd & U16_MAX);

    /* mini cmd high 8 bytes for sub type, low 8 bytes for area */
    sub_type = (unsigned char)((cmd_minor >> BITS_OF_CHAR) & U8_MAX);
    area = (unsigned char)(cmd_minor & U8_MAX);

    switch (cmd_main) {
        case CMD_GET_VERSION:
            if (area > (unsigned char)UPGRADE_STANDBY_AREA) {
                dev_upgrade_err("dev(%d) select area[%u] is invalid.\n", dev_id, area);
                DEV_UPGRADE_VFREE(buf_tmp);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
            }
            ret = dev_upgrade_version_get(dev_id, sub_type, area, (unsigned char *)buf_tmp, COMM_VERSION_LENGTH);
            size_tmp = COMM_VERSION_LENGTH;
            break;
        case CMD_PARTITION_OPS:
            ret = dev_upgrade_get_current_partition(dev_id, sub_type, (unsigned int *)buf_tmp, sizeof(unsigned int));
            size_tmp = sizeof(unsigned int);
            break;
        case CMD_FW_VERIFY_OPS:
            if (sub_type > (unsigned char)UPGRADE_STANDBY_AREA) {
                dev_upgrade_err("dev(%d) select area sub_type = [%u] is invalid.\n", dev_id, sub_type);
                DEV_UPGRADE_VFREE(buf_tmp);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
            }
            ret = dev_upgrade_fw_verify(dev_id, sub_type, (unsigned int *)buf_tmp, sizeof(unsigned int));
            size_tmp = sizeof(unsigned int);
            break;
        default:
            dev_upgrade_err("dev_id:%d illegal cmd, main cmd = 0x%x, minor cmd = 0x%x.\n", dev_id, cmd_main, cmd_minor);
            DEV_UPGRADE_VFREE(buf_tmp);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if (ret != OK) {
        dev_upgrade_err("dev_id: %d get device info failed, cmd=0x%x, ret=%d.\n", dev_id, cmd, ret);
        DEV_UPGRADE_VFREE(buf_tmp);
        return ret;
    }

    if (*size < size_tmp) {
        dev_upgrade_err("info size[%u] is larger than input buff size[%u].\n", size_tmp, *size);
        DEV_UPGRADE_VFREE(buf_tmp);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    if (copy_to_user((void *)buf, (void *)buf_tmp, size_tmp)) {
        dev_upgrade_err("dev(%d) copy_to_user failed.\n", dev_id);
        DEV_UPGRADE_VFREE(buf_tmp);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }

    *size = size_tmp;
    DEV_UPGRADE_VFREE(buf_tmp);
    return ret;
}

void dev_upgrade_mdc_register_program_package_uninit(void)
{
    DEV_UPGRADE_VFREE(g_upgrade_register_pkg_info);
}

int dev_upgrade_mdc_register_program_package_init(void)
{
    PROG_ALL_PKG_INFO *pkg_info = NULL;

    pkg_info = (PROG_ALL_PKG_INFO *)vzalloc(sizeof(PROG_ALL_PKG_INFO));
    if (pkg_info == NULL) {
        dev_upgrade_err("vzalloc pkg_info fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }
    g_upgrade_register_pkg_info = pkg_info;

    return OK;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC int dev_upgrade_program_package_info_check(PROG_PKG_INFO *pkg_info)
{
    if (pkg_info->component_id >= DSMI_COMPONENT_TYPE_MAX) {
        dev_upgrade_err("get component_id failed.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }
    if ((pkg_info->partition != PARTITION_A) && (pkg_info->partition != PARTITION_B)) {
        dev_upgrade_err("get partition info failed.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }
    if ((pkg_info->pkg_type != SEC_INFO_AT_HEAD) && (pkg_info->pkg_type != SEC_INFO_AT_TAIL)) {
        dev_upgrade_err("get pkg_type info failed.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }
    return 0;
}

PROG_ALL_PKG_INFO *dev_upgrade_get_register_package_info(void)
{
    return g_upgrade_register_pkg_info;
}

STATIC void dev_upgrade_set_pkg_info(PROG_PKG_INFO *pkg_info)
{
    PROG_ALL_PKG_INFO *all_pkg_info = NULL;

    all_pkg_info = dev_upgrade_get_register_package_info();
    if (pkg_info->partition == PARTITION_A) {
        all_pkg_info->pkg_info_a[pkg_info->component_id].component_id = pkg_info->component_id;
        all_pkg_info->pkg_info_a[pkg_info->component_id].partition = pkg_info->partition;
        all_pkg_info->pkg_info_a[pkg_info->component_id].lun_id = pkg_info->lun_id;
        all_pkg_info->pkg_info_a[pkg_info->component_id].lun_offset = pkg_info->lun_offset;
        all_pkg_info->pkg_info_a[pkg_info->component_id].pkg_type = pkg_info->pkg_type;
        all_pkg_info->pkg_info_a[pkg_info->component_id].flag = DRV_ENABLE;
    }
    if (pkg_info->partition == PARTITION_B) {
        all_pkg_info->pkg_info_b[pkg_info->component_id].component_id = pkg_info->component_id;
        all_pkg_info->pkg_info_b[pkg_info->component_id].partition = pkg_info->partition;
        all_pkg_info->pkg_info_b[pkg_info->component_id].lun_id = pkg_info->lun_id;
        all_pkg_info->pkg_info_b[pkg_info->component_id].lun_offset = pkg_info->lun_offset;
        all_pkg_info->pkg_info_b[pkg_info->component_id].pkg_type = pkg_info->pkg_type;
        all_pkg_info->pkg_info_b[pkg_info->component_id].flag = DRV_ENABLE;
    }
    all_pkg_info->flag = DRV_ENABLE;
}
#endif

STATIC int dev_upgrade_register_program_package_info(void *in_buf)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    PROG_PKG_INFO pkg_info = {0};
    int ret;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;

    if (copy_from_user((void *)&pkg_info, (void *)in_buf, sizeof(PROG_PKG_INFO))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    if (dev_upgrade_program_package_info_check(&pkg_info) != 0) {
        dev_upgrade_err(
            "dev_upgrade_program_package_info_check failed. pkg_info.component_id(%#x),"
            "pkg_info.partition(%#x), pkg_info.lun_id(%#x), pkg_info.lun_offset(%llu), pkg_info.pkg_type(%#x).\n",
            pkg_info.component_id,
            pkg_info.partition,
            pkg_info.lun_id,
            pkg_info.lun_offset,
            pkg_info.pkg_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(0);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err(
            "get ctrl handle null, pkg_info.component_id(%#x),"
            "pkg_info.partition(%#x), pkg_info.lun_id(%#x), pkg_info.lun_offset(%llu), pkg_info.pkg_type(%#x).\n",
            pkg_info.component_id,
            pkg_info.partition,
            pkg_info.lun_id,
            pkg_info.lun_offset,
            pkg_info.pkg_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }

    mutex_lock(&upgrade_ctrl->lock);
    dev_upgrade_set_pkg_info(&pkg_info);

    ret = dev_upgrade_refresh_register_program_package_version(upgrade_ctrl);
    if (ret != OK) {
        dev_upgrade_err(
            "device upgrade refresh register program package version fail, pkg_info.component_id(%#x),"
            "pkg_info.partition(%#x), pkg_info.lun_id(%#x), pkg_info.lun_offset(%llu), pkg_info.pkg_type(%#x).\n",
            pkg_info.component_id,
            pkg_info.partition,
            pkg_info.lun_id,
            pkg_info.lun_offset,
            pkg_info.pkg_type);
    } else {
        dev_upgrade_info(
            "register program package info, pkg_info.component_id(%#x),"
            "pkg_info.partition(%#x), pkg_info.lun_id(%#x), pkg_info.lun_offset(%llu), pkg_info.pkg_type(%#x).\n",
            pkg_info.component_id,
            pkg_info.partition,
            pkg_info.lun_id,
            pkg_info.lun_offset,
            pkg_info.pkg_type);
    }
    mutex_unlock(&upgrade_ctrl->lock);

    return ret;
#else
    dev_upgrade_err("not support to register program package info.\n");
    return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
#endif
}

STATIC int dev_upgrade_set_device_info(int dev_id, unsigned int cmd, void *buf, unsigned int size)
{
    int ret;
    unsigned int part_type;
    unsigned short cmd_main;
    unsigned short cmd_minor;
    unsigned char sub_type;

    /* high 16 bytes means main cmd, low 16 bytes for minor cmd */
    cmd_main = (unsigned short)((cmd >> BITS_OF_SHORT) & U16_MAX);
    cmd_minor = (unsigned short)(cmd & U16_MAX);

    /* mini cmd high 8 bytes for sub type, low 8 bytes for area */
    sub_type = (unsigned char)((cmd_minor >> BITS_OF_CHAR) & U8_MAX);
    part_type = (unsigned char)(cmd_minor & U8_MAX);

    switch (cmd_main) {
        case DSMI_UPGRADE_MAIN_TYPE_PARTITION:
            ret = dev_upgrade_set_boot_partition(dev_id, sub_type, (unsigned int *)buf, size);
            break;
        case DSMI_UPGRADE_MAIN_TYPE_REG_PROG_PKG_INFO:
            ret = dev_upgrade_register_program_package_info(buf);
            break;
        default:
            dev_upgrade_err("dev_id:%d illegal cmd, main cmd = 0x%x, minor cmd = 0x%x.\n", dev_id, cmd_main, cmd_minor);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }
    if (ret != OK) {
        dev_upgrade_err("dev_id: %d set device info failed, cmd=0x%x, ret=%d.\n", dev_id, cmd, ret);
        return ret;
    }

    return 0;
}

#endif

STATIC int dev_upgrade_ioctl_get_version(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    unsigned char ver_buf[COMM_VERSION_LENGTH] = {0};
    struct upgrade_get_version_in ver_in = {0};
    struct upgrade_get_version_out ver_out = {0};
    unsigned int cur_flag = UPGRADE_MASTER_AREA;

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_get_version_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_get_version_out));

    if (copy_from_user((void *)&ver_in, (void *)upgrade_msg->in, sizeof(struct upgrade_get_version_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }
    if (ver_in.dev_id >= DEVICE_NUM_MAX || ver_in.dev_id < 0) {
        dev_upgrade_err("dev_id(%d) is invalid\n", ver_in.dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }
    if (ver_in.outBuff == NULL) {
        dev_upgrade_err("dev_id(%d) outBuff is NULL\n", ver_in.dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }
    if (ver_in.component_type >= DSMI_COMPONENT_TYPE_MAX) {
        dev_upgrade_err("dev_id(%d) type(%u) is invalid\n", ver_in.dev_id, ver_in.component_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }

    if (ver_in.buffLen < COMM_VERSION_LENGTH) {
        dev_upgrade_err("buf_len: %u invalid, component_type: %u\n", ver_in.buffLen, ver_in.component_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

#ifdef SUPPORT_TEE_SECURE_UPGRADE
    if (ver_in.component_type != DSMI_COMPONENT_TYPE_RECOVERY) {
        ret =
            dev_upgrade_component_boot_area_op(ver_in.dev_id, ver_in.component_type, DEV_GET_CURR_BOOT_AREA, &cur_flag);
        if (ret != 0) {
            dev_upgrade_err(
                "dev_id(%d) type(%u) get boot area failed, ret = (%d).\n", ver_in.dev_id, ver_in.component_type, ret);
            goto out;
        }
    }
#endif

    /* get version from flash */
    ret = dev_upgrade_version_get(ver_in.dev_id, ver_in.component_type, cur_flag, ver_buf, COMM_VERSION_LENGTH);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) get ver fail,component(%u)\n", ver_in.dev_id, ver_in.component_type);
        goto out;
    }

    ret = copy_to_user(ver_in.outBuff, (void *)ver_buf, COMM_VERSION_LENGTH);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) copy_to_user failed, ret: %d\n", ver_in.dev_id, ret);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }
out:
    ver_out.result = ret;
#ifdef CFG_SOC_PLATFORM_MDC_V51
    /* return not supported when tee return 0xFFFF000A (only for lbist) */
    if (ret == TEE_ERROR_NOT_SUPPORTED) {
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }
#endif
    if (copy_to_user((void *)upgrade_msg->out, (void *)&ver_out, sizeof(struct upgrade_get_version_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", ver_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }
    return ret;
}

STATIC int dev_upgrade_ioctl_get_schedule(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret = 0;
    struct upgrade_read_schedule_in schedule_in = {0};
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_read_schedule_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_read_schedule_out));

    if (copy_from_user((void *)&schedule_in, (void *)upgrade_msg->in, sizeof(struct upgrade_read_schedule_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(schedule_in.dev_id);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", schedule_in.dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }

    if (copy_to_user(
        (void *)upgrade_msg->out, (void *)&upgrade_ctrl->schedule, sizeof(struct upgrade_read_schedule_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", schedule_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }

    return ret;
}

STATIC int dev_upgrade_ioctl_get_component_list(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    unsigned int bitmap = 0;
    struct upgrade_get_component_list_in list_in = {0};

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_get_component_list_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_get_component_list_out));

    if (copy_from_user((void *)&list_in, (void *)upgrade_msg->in, sizeof(struct upgrade_get_component_list_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }
    if (list_in.dev_id >= DEVICE_NUM_MAX || list_in.dev_id < 0) {
        dev_upgrade_err("dev(%d) is invalid\n", list_in.dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }
    ret = dev_upgrade_get_component_bitmap(list_in.dev_id, &bitmap);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_get_component_bitmap failed, ret: %d\n", list_in.dev_id, ret);
        return -EFAULT;
    }

    if (copy_to_user((void *)upgrade_msg->out, (void *)&bitmap, sizeof(struct upgrade_get_component_list_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", list_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }

    return ret;
}

STATIC int dev_upgrade_ioctl_sync_image_copy(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;
    struct upgrade_sync_image_in sync_image_in = {0};
    struct upgrade_sync_image_out sync_image_out = {0};

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_sync_image_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_sync_image_out));

    if (copy_from_user((void *)&sync_image_in, (void *)upgrade_msg->in, sizeof(struct upgrade_sync_image_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(sync_image_in.dev_id);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", sync_image_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
        goto out;
    }

    mutex_lock(&upgrade_ctrl->lock);
#if (defined CFG_SOC_PLATFORM_MDC_V51)
    if (sync_image_in.component_type == DSMI_COMPONENT_TYPE_RECOVERY) {
        ret = dev_upgrade_mdc_update_sync_proc(DISK_OP_TYPE_SYNC_RECOVERY, upgrade_ctrl);
    } else if (sync_image_in.component_type == UPGRADE_ALL_FIRMWARE_COMPONENT) {
        ret = dev_upgrade_mdc_update_sync_proc(DISK_OP_TYPE_SYNC_FIRMWARE, upgrade_ctrl);
    } else {
        ret = dev_upgrade_mdc_update_sync_proc(DISK_OP_TYPE_SYNC, upgrade_ctrl);
    }
#else
    ret = dev_upgrade_sync_proc(upgrade_ctrl);
#endif
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_sync_proc fail, ret: %d\n", sync_image_in.dev_id, ret);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
        mutex_unlock(&upgrade_ctrl->lock);
        goto out;
    }

#ifdef CFG_SOC_PLATFORM_CLOUD
    ret = dev_upgrade_update_cnt(sync_image_in.dev_id);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_update_cnt fail, ret: %d\n", sync_image_in.dev_id, ret);
        mutex_unlock(&upgrade_ctrl->lock);
        goto out;
    }
#endif

    dev_upgrade_info("dev(%d) dev_upgrade_sync_proc success.\n", sync_image_in.dev_id);

    mutex_unlock(&upgrade_ctrl->lock);
out:
    sync_image_out.result = ret;
    if (copy_to_user((void *)upgrade_msg->out, (void *)&sync_image_out, sizeof(struct upgrade_sync_image_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", sync_image_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }
    return ret;
}

STATIC int dev_upgrade_ioctl_stop(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret = 0;
    struct upgrade_stop_in stop_in = {0};
    struct upgrade_stop_out stop_out = {0};
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_stop_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_stop_out));

    if (copy_from_user((void *)&stop_in, (void *)upgrade_msg->in, sizeof(struct upgrade_stop_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }
    if (stop_in.dev_id >= DEVICE_NUM_MAX || stop_in.dev_id < 0) {
        dev_upgrade_err("dev_id(%d) is invalid\n", stop_in.dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }
    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(stop_in.dev_id);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", stop_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
        goto out;
    }

    upgrade_ctrl->stop_flag = DEV_UPGRADE_STOP_FLAG;

    dev_upgrade_info("dev(%d) stop upgrade!!!\n", stop_in.dev_id);

out:
    stop_out.result = ret;
    if (copy_to_user((void *)upgrade_msg->out, (void *)&stop_out, sizeof(struct upgrade_stop_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", stop_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }
    return ret;
}

STATIC int dev_upgrade_ioctl_clr_localcfg(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret = 0;
    struct upgrade_clr_localcfg_in clr_localcfg_in = {0};
    struct upgrade_clr_localcfg_out clr_localcfg_out = {0};
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_clr_localcfg_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_clr_localcfg_out));

    if (copy_from_user((void *)&clr_localcfg_in, (void *)upgrade_msg->in, sizeof(struct upgrade_clr_localcfg_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }
    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(clr_localcfg_in.dev_id);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", clr_localcfg_in.dev_id);
        ret = -EFAULT;
        goto out;
    }

    upgrade_ctrl->clr_localcfg = 1;

    dev_upgrade_info("dev(%d) set clear localcfg flag!!!\n", clr_localcfg_in.dev_id);

out:
    clr_localcfg_out.result = ret;
    if (copy_to_user((void *)upgrade_msg->out, (void *)&clr_localcfg_out, sizeof(struct upgrade_clr_localcfg_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", clr_localcfg_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }
    return ret;
}

#if (!defined CFG_SOC_PLATFORM_MINI)

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_rim_update(unsigned int dev_id, unsigned char *rim_buf, unsigned int file_size)
{
    dev_upgrade_err("sec_rim_update error, esl don't support tee");
    return 0;
}
#endif

STATIC int dev_upgrade_soc_process(struct upgrade_revocation_in *in)
{
    int ret;
    unsigned char *rim_buf = NULL;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;

    if (in->file_size != REVOCATE_FILE_LEN) {
        dev_upgrade_err("dev(%d) input size[%u] err.\n", in->dev_id, in->file_size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(in->dev_id);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", in->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    rim_buf = (unsigned char *)kzalloc(REVOCATE_FILE_LEN, GFP_KERNEL | __GFP_ACCOUNT);
    if (rim_buf == NULL) {
        dev_upgrade_err("dev(%d) kzalloc fail.\n", in->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (copy_from_user((void *)rim_buf, (void *)in->file_data, REVOCATE_FILE_LEN)) {
        dev_upgrade_err("dev(%d) copy_from_user failed\n", in->dev_id);
        DEV_UPGRADE_KFREE(rim_buf);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    mutex_lock(&upgrade_ctrl->lock);

#ifdef CFG_SOC_PLATFORM_CLOUD
    /* revocation process */
    ret = dev_upgrade_sec_rim_data(in->dev_id, (const unsigned char *)rim_buf, in->file_size);
#else
    ret = sec_rim_update(in->dev_id, rim_buf, in->file_size);
#endif
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_sec_rim_data fail, ret = %d.\n", in->dev_id, ret);
        DEV_UPGRADE_KFREE(rim_buf);
        mutex_unlock(&upgrade_ctrl->lock);
        return ret;
    }

    mutex_unlock(&upgrade_ctrl->lock);
    DEV_UPGRADE_KFREE(rim_buf);

    return OK;
}
#endif

#ifdef CFG_FEATURE_UPGRADE_CRL
#ifndef CFG_SOC_PLATFORM_MDC_V51

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_flash_read(unsigned int dev_id, unsigned int flash_offset, unsigned char *buf, unsigned int buf_len)
{
    dev_upgrade_err("sec_flash_read error, esl don't support tee");
    return 0;
}
#endif

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_flash_write(unsigned int dev_id, unsigned int offset, unsigned char *buf, unsigned int buf_len)
{
    dev_upgrade_err("sec_flash_write error, esl don't support tee");
    return 0;
}
#endif

STATIC int dev_upgrade_crl_write_flash(
    int dev_id, unsigned int offset, const unsigned char *write_buf, unsigned int size)
{
    int ret;
    unsigned char *read_buf = NULL;

    read_buf = (unsigned char *)kzalloc(FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ACCOUNT);
    if (read_buf == NULL) {
        dev_upgrade_err("kzalloc for write data fail. (dev_id=%d)\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    ret = sec_flash_read(dev_id, offset, read_buf, size);
    if (ret != 0) {
        dev_upgrade_err("Read flagsh fail. (dev_id=%d; ret=%d)\n", dev_id, ret);
        goto free_read_buf_exit;
    }

    if (memcmp(write_buf, read_buf, size) == 0) {
        dev_upgrade_info("The write data is the same as the current flash data. (dev_id=%d)\n", dev_id);
        goto free_read_buf_exit;
    }

    ret = sec_flash_write(dev_id, offset, write_buf, size);
    if (ret != 0) {
        dev_upgrade_err("Write flash fail. (dev_id=%d; ret=%d)\n", dev_id, ret);
        goto free_read_buf_exit;
    }

    ret = sec_flash_read(dev_id, offset, read_buf, size);
    if (ret != 0) {
        dev_upgrade_err("Read flagsh fail. (dev_id=%d; ret=%d)\n", dev_id, ret);
        goto free_read_buf_exit;
    }

    if (memcmp(write_buf, read_buf, size) != 0) {
        dev_upgrade_err("The read data is not equal to write data. (dev_id=%d)\n", dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
    }

free_read_buf_exit:
    DEV_UPGRADE_KFREE(read_buf);
    return ret;
}

STATIC int dev_upgrade_update_crl_proc(
    int dev_id, unsigned int revocation_type, unsigned char *src_crl_data, unsigned int src_size)
{
    int i;
    int ret;
    unsigned char *write_buf = NULL;
    unsigned char tag[][TAG_SIZE] = {{0}, "dbx", "crl"};
    unsigned int write_size = src_size + sizeof(src_size) + TAG_SIZE;
    unsigned int crl_address[][FLASH_AREA_NUM] = {
        {0, 0}, {CRL_FLASH_MAIN_ADDR_1, CRL_FLASH_BK_ADDR_1}, {CRL_FLASH_MAIN_ADDR_2, CRL_FLASH_BK_ADDR_2}};

    if (write_size > FLASH_BLOCK_SIZE) {
        dev_upgrade_err("The size of the CRL file is incorrect. (dev_id=%d; size=%u)\n", dev_id, src_size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    write_buf = (unsigned char *)kzalloc(FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ACCOUNT);
    if (write_buf == NULL) {
        dev_upgrade_err("kzalloc for write data fail. (dev_id=%d)\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    ret = memcpy_s(write_buf, FLASH_BLOCK_SIZE, tag[revocation_type], TAG_SIZE);
    if (ret != 0) {
        dev_upgrade_err("memcpy_s fail. (dev_id=%d; ret=%d)\n", dev_id, ret);
        goto free_write_buf_exit;
    }
    ret = memcpy_s(write_buf + TAG_SIZE, FLASH_BLOCK_SIZE - TAG_SIZE, &src_size, sizeof(src_size));
    if (ret != 0) {
        dev_upgrade_err("memcpy_s fail. (dev_id=%d; ret=%d)\n", dev_id, ret);
        goto free_write_buf_exit;
    }
    ret = memcpy_s(write_buf + TAG_SIZE + sizeof(src_size),
        FLASH_BLOCK_SIZE - TAG_SIZE - sizeof(src_size),
        src_crl_data,
        src_size);
    if (ret != 0) {
        dev_upgrade_err("memcpy_s fail. (dev_id=%d; ret=%d)\n", dev_id, ret);
        goto free_write_buf_exit;
    }

    /* 0: write main area  1: write backup area */
    for (i = 0; i <= 1; i++) {
        ret = dev_upgrade_crl_write_flash(dev_id, crl_address[revocation_type][i], write_buf, write_size);
        if (ret != 0) {
            dev_upgrade_err("Failed to write the CRL file to mian area. (dev_id=%d; ret=%d)\n", dev_id, ret);
            goto free_write_buf_exit;
        }
    }

free_write_buf_exit:
    DEV_UPGRADE_KFREE(write_buf);
    return ret;
}
#endif

STATIC int dev_upgrade_cms_crl_process(
    int dev_id, unsigned int revocation_type, unsigned char *user_file_data, unsigned int size)
{
    int ret;
    unsigned char *crl_data = NULL;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;

    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(dev_id);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (size > (unsigned int)CRL_FILE_SIZE_MAX) {
        dev_upgrade_err("size(%u) is out of range.\n", size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    crl_data = (unsigned char *)vzalloc(size);
    if (crl_data == NULL) {
        dev_upgrade_err("crl data vzalloc fail, size=%u.\n", size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (copy_from_user((void *)crl_data, (void *)user_file_data, size)) {
        dev_upgrade_err("copy_from_user failed, size=%u.\n", size);
        DEV_UPGRADE_VFREE(crl_data);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    mutex_lock(&upgrade_ctrl->lock);

#ifdef CFG_SOC_PLATFORM_MDC_V51
    ret = dev_upgrade_mdc_update_crl_proc(dev_id, crl_data, size);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_mdc_update_crl_proc fail, ret=0x%x, size=%u.\n", dev_id, ret, size);
        DEV_UPGRADE_VFREE(crl_data);
        mutex_unlock(&upgrade_ctrl->lock);
        return ret;
    }
#else
    ret = dev_upgrade_update_crl_proc(dev_id, revocation_type, crl_data, size);
    if (ret != 0) {
        dev_upgrade_err("Update crl process fail. (dev_id=%d;ret=0x%x;size=%u)\n", dev_id, ret, size);
        DEV_UPGRADE_VFREE(crl_data);
        mutex_unlock(&upgrade_ctrl->lock);
        return ret;
    }
#endif

    mutex_unlock(&upgrade_ctrl->lock);
    DEV_UPGRADE_VFREE(crl_data);
    return OK;
}
#endif

STATIC int dev_upgrade_ioctl_sec_revocation_process(struct upgrade_ioctl_msg *upgrade_msg)
{
#ifndef CFG_SOC_PLATFORM_MINI
    int ret;
    struct upgrade_revocation_in revocation_in = {0};
    struct upgrade_revocation_out revocation_out = {0};

    dev_upgrade_event("enter dev_upgrade_sec_revocation_process.\n");

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_revocation_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_revocation_out));

    if (copy_from_user((void *)&revocation_in, (void *)upgrade_msg->in, sizeof(struct upgrade_revocation_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    if (revocation_in.dev_id >= DEVICE_NUM_MAX || revocation_in.dev_id < 0) {
        dev_upgrade_err("dev(%d) is invalid.\n", revocation_in.dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if (revocation_in.file_data == NULL) {
        dev_upgrade_err("dev(%d) file_data is NULL.\n", revocation_in.dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    switch (revocation_in.type) {
        case REVOCATION_TYPE_SOC:
            ret = dev_upgrade_soc_process(&revocation_in);
            break;
#ifdef CFG_FEATURE_UPGRADE_CRL
        case REVOCATION_TYPE_CMS_CRL:
        case REVOCATION_TYPE_CMS_CRL_EXT:
            ret = dev_upgrade_cms_crl_process(revocation_in.dev_id,
                revocation_in.type,
                (unsigned char *)revocation_in.file_data,
                revocation_in.file_size);
            break;
#endif
        default:
            dev_upgrade_err("dev(%d) invalid type(%u).\n", revocation_in.dev_id, revocation_in.type);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }

    revocation_out.result = ret;
    if (copy_to_user((void *)upgrade_msg->out, (void *)&revocation_out, sizeof(struct upgrade_revocation_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", revocation_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }
    return ret;
#else
    /* not support for mini */
    return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
#endif
}

#if (defined CFG_SOC_PLATFORM_MDC_V51) || (defined CFG_SOC_PLATFORM_MDC_V11)
int dev_upgrade_ioctl_sync_local_component(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;
    struct upgrade_ufs_local_component_start_in *fw_start_in = NULL;
    struct upgrade_ufs_local_component_start_out fw_start_out = {0};

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_ufs_local_component_start_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_ufs_local_component_start_out));

    fw_start_in =
        (struct upgrade_ufs_local_component_start_in *)vzalloc(sizeof(struct upgrade_ufs_local_component_start_in));
    if (fw_start_in == NULL) {
        dev_upgrade_err("fw_start_in calloc fail\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    if (copy_from_user(
        (void *)fw_start_in, (void *)upgrade_msg->in, sizeof(struct upgrade_ufs_local_component_start_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        DEV_UPGRADE_VFREE(fw_start_in);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    ret = dev_upgrade_mdc_check_sync_start_in_para(fw_start_in);
    if (ret != 0) {
        DEV_UPGRADE_VFREE(fw_start_in);
        dev_upgrade_err("dev_upgrade_mdc_check_sync_start_in_para failed[%d]\n", ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(fw_start_in->dev_id);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", fw_start_in->dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
        goto out;
    }

    mutex_lock(&upgrade_ctrl->lock);

    ret = dev_upgrade_local_components_fill(fw_start_in);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_local_components_fill fail, ret: %x\n", upgrade_ctrl->dev_id, ret);
        mutex_unlock(&upgrade_ctrl->lock);
        goto out;
    }

    ret = 0;
    mutex_unlock(&upgrade_ctrl->lock);

out:
    fw_start_out.result = ret;
    if (copy_to_user(
        (void *)upgrade_msg->out, (void *)&fw_start_out, sizeof(struct upgrade_ufs_local_component_start_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", fw_start_in->dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }
    DEV_UPGRADE_VFREE(fw_start_in);
    return ret;
}

STATIC int dev_upgrade_ioctl_check(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    unsigned int check_result = IMAGE_CHK_OK;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;
    struct upgrade_check_image_in check_image_in = {0};
    struct upgrade_check_image_out check_image_out = {0};

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_check_image_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_check_image_out));

    if (copy_from_user((void *)&check_image_in, (void *)upgrade_msg->in, sizeof(struct upgrade_check_image_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    dev_upgrade_info("dev_upgrade_check_proc start.\n");
    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(check_image_in.dev_id);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", check_image_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
        goto out;
    }

    mutex_lock(&upgrade_ctrl->lock);
    ret = dev_upgrade_check_proc(&check_image_in);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_check_proc fail, ret: %d\n", check_image_in.dev_id, ret);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
        mutex_unlock(&upgrade_ctrl->lock);
        goto out;
    }
    dev_upgrade_info("dev(%d) dev_upgrade_check_proc success.\n", check_image_in.dev_id);
    check_result = dev_upgrade_ufs_check_status_get(check_image_in.dev_id);

    mutex_unlock(&upgrade_ctrl->lock);
out:
    check_image_out.result = ret;
    check_image_out.check_result = check_result;
    if (copy_to_user((void *)upgrade_msg->out, (void *)&check_image_out, sizeof(struct upgrade_check_image_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", check_image_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }
    return ret;
}

STATIC int dev_upgrade_ioctl_state_flag_get(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;
    unsigned int state_flag = IMAGE_CHK_OK;
    struct upgrade_get_state_flag_in state_flag_in = {0};
    struct upgrade_get_state_flag_out state_flag_out = {0};

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_get_state_flag_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_get_state_flag_out));

    if (copy_from_user((void *)&state_flag_in, (void *)upgrade_msg->in, sizeof(struct upgrade_get_state_flag_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(0);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", 0);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }

    ret = dev_upgrade_get_disk_img_update_flag(upgrade_ctrl, &state_flag);
    if (ret != OK) {
        dev_upgrade_warn("dev_upgrade_get_disk_img_update_flag fail, ret: %d\n", ret);
    }

    state_flag_out.result = ret;
    state_flag_out.state_flag = state_flag;
    if (copy_to_user((void *)upgrade_msg->out, (void *)&state_flag_out, sizeof(struct upgrade_get_state_flag_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", state_flag_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }

    return ret;
}
#endif

STATIC int dev_upgrade_dev_info_in_check(struct upgrade_dev_info_in *dev_info_in)
{
    if (dev_info_in->dev_id >= DEVICE_NUM_MAX || dev_info_in->dev_id < 0) {
        dev_upgrade_err("dev_id(%d) is invalid\n", dev_info_in->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }

    if (dev_info_in->buf == NULL) {
        dev_upgrade_err("dev_id(%d) outBuff is NULL\n", dev_info_in->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (dev_info_in->size > U16_MAX) {
        dev_upgrade_err("dev_id(%d) input size is %u.\n", dev_info_in->dev_id, dev_info_in->size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }
    return 0;
}

STATIC int dev_upgrade_ioctl_get_dev_info(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    struct upgrade_dev_info_in dev_info_in = {0};
    struct upgrade_dev_info_out dev_info_out = {0};

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_dev_info_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_dev_info_out));

    if (copy_from_user((void *)&dev_info_in, (void *)upgrade_msg->in, sizeof(struct upgrade_dev_info_in))) {
        dev_upgrade_err("get device info copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    ret = dev_upgrade_dev_info_in_check(&dev_info_in);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_dev_info_in_check fail, cmd = 0x%x, ret: %d\n", dev_info_in.cmd, ret);
        return ret;
    }

    ret = dev_upgrade_get_device_info(dev_info_in.dev_id, dev_info_in.cmd, dev_info_in.buf, &dev_info_in.size);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_get_dev_info fail, cmd = 0x%x, ret: %d\n", dev_info_in.cmd, ret);
    }

    dev_info_out.result = ret;
    dev_info_out.size = dev_info_in.size;

#ifdef CFG_SOC_PLATFORM_MDC_V51
    /* return not supported when tee return 0xFFFF000A (only for lbist) */
    if (ret == TEE_ERROR_NOT_SUPPORTED) {
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }
#endif

    if (copy_to_user((void *)upgrade_msg->out, (void *)&dev_info_out, sizeof(struct upgrade_dev_info_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", dev_info_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }

    return ret;
}

#ifndef DEV_UPGRADE_UT
STATIC int dev_upgrade_ioctl_set_dev_info(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    struct upgrade_dev_info_in dev_info_in = {0};
    struct upgrade_dev_info_out dev_info_out = {0};

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_dev_info_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_dev_info_out));

    if (copy_from_user((void *)&dev_info_in, (void *)upgrade_msg->in, sizeof(struct upgrade_dev_info_in))) {
        dev_upgrade_err("set device info copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    ret = dev_upgrade_dev_info_in_check(&dev_info_in);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_dev_info_in_check fail, cmd = 0x%x, ret: %d\n", dev_info_in.cmd, ret);
        return ret;
    }
    ret = dev_upgrade_set_device_info(dev_info_in.dev_id, dev_info_in.cmd, dev_info_in.buf, dev_info_in.size);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_set_dev_info fail, cmd = 0x%x, ret: %d\n", dev_info_in.cmd, ret);
    }

    dev_info_out.result = ret;
    dev_info_out.size = dev_info_in.size;

    if (copy_to_user((void *)upgrade_msg->out, (void *)&dev_info_out, sizeof(struct upgrade_dev_info_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", dev_info_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }

    return ret;
}
#endif

int dev_upgrade_ioctl_verify_img(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    struct upgrade_verify_img_in verify_img_in = {0};
    struct upgrade_verify_img_out verify_img_out = {0};
    char *tmp_path = NULL;

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_verify_img_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_verify_img_out));

    if (copy_from_user((void *)&verify_img_in, (void *)upgrade_msg->in, sizeof(struct upgrade_verify_img_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    if (verify_img_in.dev_id >= DEVICE_NUM_MAX || verify_img_in.dev_id < 0) {
        dev_upgrade_err("dev_id(%d) is invalid\n", verify_img_in.dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }

    if (verify_img_in.path == NULL) {
        dev_upgrade_err("dev_id(%d) verify image path is NULL\n", verify_img_in.dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if ((verify_img_in.size >= PATH_MAX) || (verify_img_in.size == 0)) {
        dev_upgrade_err("dev_id(%d) input path length[%u] out of range.\n", verify_img_in.dev_id, verify_img_in.size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    tmp_path = (unsigned char *)kzalloc(verify_img_in.size + 1, GFP_KERNEL | __GFP_ACCOUNT);
    if (tmp_path == NULL) {
        dev_upgrade_err("tmp_path kzalloc fail, size=%u.\n", verify_img_in.size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (copy_from_user((void *)tmp_path, (void *)verify_img_in.path, verify_img_in.size)) {
        dev_upgrade_err("copy_from_user failed\n");
        DEV_UPGRADE_KFREE(tmp_path);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    tmp_path[verify_img_in.size] = '\0';
    ret = dev_upgrade_verify_image(verify_img_in.dev_id, tmp_path, verify_img_in.mode);
    if (ret != OK) {
        dev_upgrade_err("dev_id(%d) verify image(%s) fail, ret:%d\n", verify_img_in.dev_id, tmp_path, ret);
    }

    verify_img_out.result = ret;
    if (copy_to_user((void *)upgrade_msg->out, (void *)&verify_img_out, sizeof(struct upgrade_verify_img_out))) {
        dev_upgrade_err("dev_id(%d) copy_to_user failed\n", verify_img_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }

    DEV_UPGRADE_KFREE(tmp_path);
    return ret;
}

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_set_recovery_flag(unsigned int dev_id)
{
    dev_upgrade_err("sec_set_recovery_flag error, esl don't support tee");
    return 0;
}
#endif

STATIC int dev_upgrade_ioctl_set_rcvr_flag(struct upgrade_ioctl_msg *upgrade_msg)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    int ret;
    int dev_id = upgrade_msg->dev_id;

    ret = sec_set_recovery_flag((unsigned int)dev_id);
    if (ret != OK) {
        dev_upgrade_err("dev_id(%d) sec_set_recovery_flag failed. (ret=%d)\n", dev_id, ret);
    } else {
        dev_upgrade_info("set recovery flag success!\n");
    }

    return ret;
#else
    return 0;
#endif
}

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_get_recovery_flag(unsigned int dev_id, int *flag)
{
    dev_upgrade_err("sec_get_recovery_flag error, esl don't support tee");
    return 0;
}
#endif

STATIC int dev_upgrade_ioctl_get_rcvr_flag(struct upgrade_ioctl_msg *upgrade_msg)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    int ret;
    int dev_id = upgrade_msg->dev_id;
    struct upgrade_dev_info_out dev_info_out = {0};
    int flag;

    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_dev_info_out));

    ret = sec_get_recovery_flag((unsigned int)dev_id, &flag);
    if (ret != OK) {
        dev_upgrade_err("dev_id(%d) sec_get_recovery_flag failed. (ret=%d)\n", dev_id, ret);
        return ret;
    }

    if (flag == RECOVERY_FLAG_VALUE) {
        dev_info_out.result = RECOVERY_FLAG_ENABLE;
    } else {
        dev_info_out.result = RECOVERY_FLAG_DISABLE;
    }
    dev_info_out.size = sizeof(int);

    if (copy_to_user((void *)upgrade_msg->out, (void *)&dev_info_out, sizeof(struct upgrade_dev_info_out))) {
        dev_upgrade_err("dev_id(%d) copy_to_user failed\n", dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    } else {
        dev_upgrade_info("get recovery flag success! flag = %d, size = %d\n", dev_info_out.result, dev_info_out.size);
    }

    return ret;
#else
    return 0;
#endif
}

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_clear_recovery_flag(unsigned int dev_id)
{
    dev_upgrade_err("sec_clear_recovery_flag error, esl don't support tee");
    return 0;
}
#endif

STATIC int dev_upgrade_ioctl_clean_rcvr_flag(struct upgrade_ioctl_msg *upgrade_msg)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    int ret;
    int dev_id = upgrade_msg->dev_id;

    ret = sec_clear_recovery_flag((unsigned int)dev_id);
    if (ret != OK) {
        dev_upgrade_err("dev_id(%d) sec_clear_recovery_flag failed. (ret=%d)\n", dev_id, ret);
    } else {
        dev_upgrade_info("clean recovery flag success!\n");
    }
    return ret;
#else
    return 0;
#endif
}

#ifdef CFG_SOC_PLATFORM_MDC_V51

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_reset_recovery_boot_count(unsigned int dev_id)
{
    dev_upgrade_err("sec_reset_recovery_boot_count error, esl don't support tee");
    return 0;
}
#endif

STATIC int dev_upgrade_reset_rcvr_boot_cnt(int dev_id)
{
    int ret;
    ret = sec_reset_recovery_boot_count((unsigned int)dev_id);
    if (ret != OK) {
        dev_upgrade_err("dev_id(%d) sec_reset_recovery_boot_count failed, ret = %d\n", dev_id, ret);
        return ret;
    }
    dev_upgrade_info("reset recovery boot count success.\n");
    return 0;
}

STATIC int dev_upgrade_reset_serv_boot_cnt(int dev_id)
{
    int ret;
    unsigned int val;
    unsigned int start_cnt = 0;

    ret = sec_read_ufs_resetcnt((unsigned int)dev_id, &start_cnt);
    if (ret != OK) {
        dev_upgrade_err("dev_id(%d) sec_read_ufs_resetcnt failed, ret = %d.\n", dev_id, ret);
        return ret;
    }
    val = ((start_cnt % UFS_START_CNT_MASK) < UFS_MASTER_START_MAX) ? RESTART_CNT_VAL : BACKUP_RESTART_CNT_VAL;
    ret = sec_write_ufs_resetcnt(dev_id, val);
    if (ret != OK) {
        dev_upgrade_err("dev_id(%d) sec_write_ufs_resetcnt(%d) failed, ret = %d.\n", dev_id, val, ret);
        return ret;
    }
    dev_upgrade_info("reset sevice boot cnt from (%d) to (%d) success.\n", start_cnt, val);
    return 0;
}


#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int clear_flash_resetcnt(unsigned int dev_id)
{
    dev_upgrade_err("clear_flash_resetcnt error, esl don't support tee");
    return 0;
}
#endif

STATIC int dev_upgrade_reset_flash_boot_cnt(int dev_id)
{
    int ret;
    ret = clear_flash_resetcnt((unsigned int)dev_id);
    if (ret != OK) {
        dev_upgrade_err("dev_id(%d) clear_flash_resetcnt failed, ret = %d.\n", dev_id, ret);
        return ret;
    }
    dev_upgrade_info("reset flash boot count success.\n");
    return 0;
}
#endif

STATIC int dev_upgrade_ioctl_reset_rcvr_boot_cnt(struct upgrade_ioctl_msg *upgrade_msg)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    int ret;
    int dev_id = upgrade_msg->dev_id;

    /* reset startup count */
    ret = dev_upgrade_reset_serv_boot_cnt(dev_id);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_reset_serv_boot_cnt failed, ret = %d\n", ret);
        return ret;
    }

    /* reset flash boot count */
    ret = dev_upgrade_reset_flash_boot_cnt(dev_id);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_reset_flash_boot_cnt failed, ret = %d\n", ret);
        return ret;
    }

    /* reset recovery boot count */
    ret = dev_upgrade_reset_rcvr_boot_cnt(dev_id);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_reset_rcvr_boot_cnt failed, ret = %d\n", ret);
        return ret;
    }

    return 0;
#else
    return 0;
#endif
}

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_set_recovery_status(unsigned int dev_id, unsigned int op, unsigned int part)
{
    dev_upgrade_err("sec_set_recovery_status error, esl don't support tee");
    return 0;
}
#endif

STATIC int dev_upgrade_ioctl_set_rcvr_status(struct upgrade_ioctl_msg *upgrade_msg)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    int ret;
    struct upgrade_dev_info_in dev_info_in = {0};
    struct upgrade_recovery_status_in recovery_status_in = {0};

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_dev_info_in));

    if (copy_from_user((void *)&dev_info_in, (void *)upgrade_msg->in, sizeof(struct upgrade_dev_info_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    if (dev_info_in.dev_id >= DEVICE_NUM_MAX || dev_info_in.dev_id < 0) {
        dev_upgrade_err("dev_id(%d) is invalid\n", dev_info_in.dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }

    if (dev_info_in.size != sizeof(struct upgrade_recovery_status_in)) {
        dev_upgrade_err("buf size(%u) is invalid\n", dev_info_in.size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    if (copy_from_user((void *)&recovery_status_in, (void *)dev_info_in.buf, dev_info_in.size)) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    ret = sec_set_recovery_status(dev_info_in.dev_id, recovery_status_in.op, recovery_status_in.part);
    if (ret != OK) {
        dev_upgrade_err("dev_id(%d) sec_set_recovery_status failed\n", dev_info_in.dev_id);
    } else {
        dev_upgrade_info("set recovery status success!\n");
    }

    return ret;
#else
    return 0;
#endif
}
#if ((!defined CFG_SOC_PLATFORM_MINIV2) && (!defined CFG_SOC_PLATFORM_MDC_V51) && (!defined CFG_SOC_PLATFORM_MINIV3))
STATIC int dev_upgrade_ioctl_get_check_info(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    IMG_CHECK_INFO info;
    struct upgrade_get_check_info_in ci_in = {0};
    struct upgrade_get_check_info_out ci_out = {0};

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_get_check_info_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_get_check_info_out));
    if (copy_from_user((void *)&ci_in, (void *)upgrade_msg->in, sizeof(struct upgrade_get_check_info_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }
    if (ci_in.dev_id >= DEVICE_NUM_MAX || ci_in.dev_id < 0) {
        dev_upgrade_err("dev_id invalid.(devid=%d)\n", ci_in.dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }
    if (ci_in.outBuff == NULL) {
        dev_upgrade_err("out buf is NULL.(devid=%d)\n", ci_in.dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (ci_in.buffLen < sizeof(IMG_CHECK_INFO)) {
        dev_upgrade_err("in buf len not enough.(devid=%d;buflen=%u)\n", ci_in.dev_id, ci_in.buffLen);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    /* get version from flash */
    ret = dev_upgrade_get_check_info(ci_in.dev_id, &info);
    if (ret != 0) {
        dev_upgrade_err("get check info fail.(devid=%d)\n", ci_in.dev_id);
        goto out;
    }

    ret = copy_to_user(ci_in.outBuff, (void *)&info, sizeof(info));
    if (ret != 0) {
        dev_upgrade_err("copy to user fail.(devid=%d;ret=%d)\n", ci_in.dev_id, ret);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }
out:
    ci_out.result = ret;
    if (copy_to_user((void *)upgrade_msg->out, (void *)&ci_out, sizeof(struct upgrade_get_check_info_out))) {
        dev_upgrade_err("copy to user fail.(devid=%d)\n", ci_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }

    return ret;
}
#endif

upgrade_cmd_map_t g_upgrade_cmd_func_map[] = {
    {UPGRADE_READ_SCHEDULE_CTL, NOT_NEED_PERMISSION_CHECK, dev_upgrade_ioctl_get_schedule},
    {UPGRADE_FIRMWARE_START, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_firmware_start},
    {UPGRADE_GET_FIREWARE_VER, NOT_NEED_PERMISSION_CHECK, dev_upgrade_ioctl_get_version},
    {UPGRADE_GET_COMPONENT_LIST, NOT_NEED_PERMISSION_CHECK, dev_upgrade_ioctl_get_component_list},
    {UPGRADE_SYNC_IMAGE_COPY, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_sync_image_copy},
    {UPGRADE_FIRMWARE_STOP, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_stop},
    {UPGRADE_CLR_LOCALCFG, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_clr_localcfg},
    {UPGRADE_SEC_REVOCATION, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_sec_revocation_process},
#ifdef CFG_SOC_PLATFORM_MDC_V51
    {UPGRADE_SYNC_LOCAL_COMP, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_sync_local_component},
    {UPGRADE_IMAGE_CHECK, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_check},
    {UPGRADE_GET_STATE_FLAG, NOT_NEED_PERMISSION_CHECK, dev_upgrade_ioctl_state_flag_get},
#endif
    {UPGRADE_GET_DEV_INFO, NOT_NEED_PERMISSION_CHECK, dev_upgrade_ioctl_get_dev_info},
    {UPGRADE_VERIFY_IMG, NOT_NEED_PERMISSION_CHECK, dev_upgrade_ioctl_verify_img},
    {UPGRADE_SET_RCVR_FLAG, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_set_rcvr_flag},
    {UPGRADE_GET_RCVR_FLAG, NOT_NEED_PERMISSION_CHECK, dev_upgrade_ioctl_get_rcvr_flag},
    {UPGRADE_CLEAN_RCVR_FLAG, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_clean_rcvr_flag},
    {UPGRADE_RESET_RCVR_BOOT_CNT, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_reset_rcvr_boot_cnt},
    {UPGRADE_SET_RCVR_STATUS, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_set_rcvr_status},
#ifndef DEV_UPGRADE_UT
    {UPGRADE_SET_DEV_INFO, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_set_dev_info},
#endif
#if ((!defined CFG_SOC_PLATFORM_MINIV2) && (!defined CFG_SOC_PLATFORM_MDC_V51) && (!defined CFG_SOC_PLATFORM_MINIV3))
    {UPGRADE_GET_CHECK_INFO, NOT_NEED_PERMISSION_CHECK, dev_upgrade_ioctl_get_check_info},
#endif
};

STATIC int dev_upgrade_ioctl_distribute(unsigned int cmd, struct upgrade_ioctl_msg *upgrade_msg)
{
    int cmd_num;
    int i;

    cmd_num = sizeof(g_upgrade_cmd_func_map) / sizeof(upgrade_cmd_map_t);
    for (i = 0; i < cmd_num; i++) {
        if (cmd == g_upgrade_cmd_func_map[i].cmd) {
            if (g_upgrade_cmd_func_map[i].permission_check_flag == NEED_PERMISSION_CHECK) {
                CHECK_PROCESS_PERMISSION();
            }
            break;
        }
    }

    if (i >= cmd_num) {
        dev_upgrade_err("invalid cmd: %u\n", cmd);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }

    return (g_upgrade_cmd_func_map[i].upgrade_cmd_handler)(upgrade_msg);
}

STATIC long dev_upgrade_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct upgrade_ioctl_msg upgrade_msg = { 0 };
    void *u_arg = (void *)(uintptr_t)arg;
    long ret;

    DRV_CHECK_RETV((file != NULL), (-EINVAL));
    DRV_CHECK_RETV((arg != 0), (-EINVAL));

    if (copy_from_user((void *)&upgrade_msg, u_arg, sizeof(struct upgrade_ioctl_msg))) {
        dev_upgrade_err("copy_from_user failed\n");
        return -EINVAL;
    }

    DRV_CHECK_RETV((upgrade_msg.in != NULL), (-EINVAL));
    DRV_CHECK_RETV((upgrade_msg.out != NULL), (-EINVAL));
    DRV_CHECK_RETV((upgrade_msg.dev_id < DEVICE_NUM_MAX && upgrade_msg.dev_id >= 0), (-EINVAL));

#ifndef CFG_SOC_PLATFORM_MDC_LITE_ESL
    if (!dev_upgrade_devid_enable_check(upgrade_msg.dev_id)) {
        dev_upgrade_err("dev(%d) is disable\n", upgrade_msg.dev_id);
        return -EINVAL;
    }
#endif

    ret = (long)dev_upgrade_ioctl_distribute(cmd, &upgrade_msg);
    if (ret > 0) {
        /* if return value is positive, user mode ioctl func will return 0 */
        ret = -ret;
    }

    if (ret != 0) {
        dev_upgrade_err("dev(%d) ioctl failed, ret: %ld\n", upgrade_msg.dev_id, ret);
        return ret;
    }
    return ret;
}

STATIC int dev_upgrade_open(struct inode *pnode, struct file *pfile)
{
    DRV_CHECK_RETV((pnode != NULL), -EINVAL);
    DRV_CHECK_RETV((pfile != NULL), -EINVAL);

    if (g_infra_upgrade_dev != NULL) {
        pfile->private_data = g_infra_upgrade_dev;
    }

    return OK;
}

STATIC int dev_upgrade_release(struct inode *pnode, struct file *pfile)
{
    DRV_CHECK_RETV((pnode != NULL), -EINVAL);
    DRV_CHECK_RETV((pfile != NULL), -EINVAL);
    return OK;
}

STATIC void dev_upgrade_cleanup_dev(CDEV_ST *pdev)
{
    dev_t dev_no;

    if ((pdev == NULL) || (pdev->dev_class == NULL)) {
        dev_upgrade_err("dev_upgrade_cleanup_dev input para is NULL, pdev: %pK\r\n", pdev);
        return;
    }

    dev_no = MKDEV(pdev->dev_major, pdev->dev_minor);
    cdev_del(&pdev->cdev);
    device_destroy(pdev->dev_class, dev_no);
    class_destroy(pdev->dev_class);
    unregister_chrdev_region(dev_no, UPGRADE_DEV_MAX_COUNT);

    return;
}

STATIC int dev_upgrade_init_dev(CDEV_ST *pdev, struct file_operations *pfoprs)
{
    dev_t dev_no;
    int rc;
    struct device *dev = NULL;
    struct class *dev_class = NULL;

    if ((pdev == NULL) || (pfoprs == NULL)) {
        dev_upgrade_err("comm_init_dev input para is NULL, pdev: %pK, pfoprs: %pK\r\n", pdev, pfoprs);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    pdev->dev_class = NULL;

    rc = alloc_chrdev_region(&dev_no, 0, UPGRADE_DEV_MAX_COUNT, DEV_UPGRADE_NAME_KER);
    if (rc < 0) {
        dev_upgrade_err("alloc_chrdev_region for %s error: %d\r\n", DEV_UPGRADE_NAME_KER, rc);
        return rc;
    }

    dev_class = class_create(THIS_MODULE, UPGRADE_DEV_CLASS);
    if (IS_ERR(dev_class)) {
        dev_upgrade_err("class_create %s error\r\n", UPGRADE_DEV_CLASS);
        rc = PTR_ERR(dev_class);
        goto exit_unregister_chrdev_region;
    }

    dev = device_create(dev_class, NULL, dev_no, NULL, DEV_UPGRADE_NAME_KER);
    if (IS_ERR(dev)) {
        dev_upgrade_err("device_create %s error\r\n", DEV_UPGRADE_NAME_KER);
        rc = PTR_ERR(dev);
        goto exit_class_destroy;
    }

    cdev_init(&pdev->cdev, pfoprs);
    pdev->cdev.owner = THIS_MODULE;
    rc = cdev_add(&pdev->cdev, dev_no, 1);
    if (rc) {
        dev_upgrade_err("cdev_add %s error, rc: %d\r\n", DEV_UPGRADE_NAME_KER, rc);
        goto exit_device_create;
    }

    pdev->dev_class = dev_class;
    pdev->dev_major = MAJOR(dev_no);
    pdev->dev_minor = MINOR(dev_no);

    return 0;

exit_device_create:
    device_destroy(dev_class, dev_no);
exit_class_destroy:
    class_destroy(dev_class);
exit_unregister_chrdev_region:
    unregister_chrdev_region(dev_no, UPGRADE_DEV_MAX_COUNT);

    return rc;
}

STATIC void dev_upgrade_mutex_destroy(void)
{
    int i;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(i);
        if (upgrade_ctrl != NULL) {
            if (dev_upgrade_devid_enable_check(upgrade_ctrl->dev_id)) {
                mutex_destroy(&upgrade_ctrl->lock);
            }
        }
    }
}

STATIC int dev_upgrade_init_drv(void)
{
    int ret;
    unsigned int i;
    unsigned int match_nums = 0;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;
#ifdef CFG_SOC_PLATFORM_MDC_V51
    ret = dev_upgrade_ufs_component_core_info_init();
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_ufs_component_core_info_init fail, ret:%x", ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }
#endif

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(i);
        if (upgrade_ctrl == NULL) {
#ifdef CFG_SOC_PLATFORM_MDC_V51
            dev_upgrade_ufs_component_core_info_uninit();
#endif
            dev_upgrade_err("get ctrl handle fail, dev(%u)\n", i);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
        }

        ret = memset_s((void *)upgrade_ctrl, sizeof(dev_upgrade_core_ctrl), 0, sizeof(dev_upgrade_core_ctrl));
        if (ret != 0) {
#ifdef CFG_SOC_PLATFORM_MDC_V51
            dev_upgrade_ufs_component_core_info_uninit();
#endif
            dev_upgrade_err("memset_s fail: %d\n", ret);
            return ret;
        }

        ret = dev_upgrade_component_info_match(i, &match_nums);
        if (ret != 0) {
            dev_upgrade_warn("dev_upgrade_component_info_match fail, ret:%x", (unsigned int)ret);
            continue;
        }

        dev_upgrade_info("dev%u scanning out %u components\n", i, match_nums);
        if (match_nums == 0) {
            dev_upgrade_devid_enable(i, DRV_DISABLE);
            upgrade_ctrl->dev_id = DEVICE_ID_INVALID;
        } else {
            dev_upgrade_devid_enable(i, DRV_ENABLE);
            upgrade_ctrl->dev_id = i;
            mutex_init(&upgrade_ctrl->lock);
        }
    }

    return 0;
}

#ifdef DEV_UPGRADE_ST
STATIC int dev_upgrade_module_init(void)
#else
STATIC int __init dev_upgrade_module_init(void)
#endif
{
    int ret;

    g_infra_upgrade_dev = (CDEV_ST *)kzalloc(sizeof(CDEV_ST), GFP_KERNEL | __GFP_ACCOUNT);
    if (g_infra_upgrade_dev == NULL) {
        dev_upgrade_err(" kmalloc failed!");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    /* create character devices */
    ret = dev_upgrade_init_dev(g_infra_upgrade_dev, &g_infra_upgrade_fops);
    if (ret != 0) {
        dev_upgrade_err("call dev_upgrade_init_dev failed! ret: %d", ret);
        goto free_dev;
    }

    ret = dev_upgrade_init_drv();
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_init_drv fail, ret:%x", (unsigned int)ret);
        goto uninit_drv;
    }

#ifdef CFG_SOC_PLATFORM_MDC_V51
    ret = dev_upgrade_mdc_init();
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_mdc_init fail, ret:%x", ret);
        goto uninit_drv;
    }
#endif

#ifdef CFG_SOC_PLATFORM_CLOUD
    ret = register_black_box();
    if (ret != 0) {
        dev_upgrade_warn("register_black_box fail, ret:%x", (unsigned int)ret);
        goto uninit_drv;
    }
    ret = init_get_nvcnt();
    if (ret != 0) {
        dev_upgrade_err("init get nvcnt fail, ret:%d", ret);
        goto uninit_drv;
    }
#endif

    return OK;

uninit_drv:
#ifdef CFG_SOC_PLATFORM_MDC_V51
    dev_upgrade_ufs_component_core_info_uninit();
    dev_upgrade_mdc_exit();
#endif
    dev_upgrade_mutex_destroy();
    dev_upgrade_cleanup_dev(g_infra_upgrade_dev);
free_dev:
    DEV_UPGRADE_KFREE(g_infra_upgrade_dev);

    return ret;
}

STATIC void __exit dev_upgrade_module_exit(void)
{
    if (g_infra_upgrade_dev != NULL) {
        dev_upgrade_cleanup_dev(g_infra_upgrade_dev);
        DEV_UPGRADE_KFREE(g_infra_upgrade_dev);
    }

    dev_upgrade_mutex_destroy();
#ifdef CFG_SOC_PLATFORM_CLOUD
    free_black_box();
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
    dev_upgrade_mdc_exit();
    dev_upgrade_ufs_component_core_info_uninit();
#endif
    dev_upgrade_info("upgrade_clean ok!");

    return;
}

module_init(dev_upgrade_module_init);
module_exit(dev_upgrade_module_exit);
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("dev firmware upgrade driver");
