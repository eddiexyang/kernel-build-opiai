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
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/securec.h>
#include <linux/suspend.h>
#include <linux/notifier.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/gfp.h>
#include "dev_upgrade_public.h"
#include <linux/pm_wakeup.h>

#include "dev_upgrade_core.h"
#include "dev_upgrade_adapt.h"
#include "drv_whitelist.h"

static struct timer_list upgrade_ext_wdt_timer;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
struct wakeup_source *g_upgrade_source = NULL;
#else
struct wakeup_source g_upgrade_source = { 0 };
#endif

const char *upgrade_wakelock_name = "hisi_upgrade_wakelock";
static unsigned int g_upgrade_work_count = 0;

STATIC int dev_upgrade_open(struct inode *pnode, struct file *pfile);
STATIC int dev_upgrade_release(struct inode *pnode, struct file *pfile);
STATIC long dev_upgrade_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
STATIC int dev_upgrade_pre_check_nve(struct upgrade_component *component);
STATIC int dev_upgrade_update_nve(struct upgrade_component *component);
STATIC int dev_upgrade_update_single_area(struct upgrade_component *component);
STATIC int dev_upgrade_update_single_area_stub(struct upgrade_component *component);
STATIC int dev_upgrade_pre_check_common(struct upgrade_component *component);
STATIC int dev_upgrade_update_double_area(struct upgrade_component *component);

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

CDEV_ST *g_infra_upgrade_dev = NULL; /* upgrade控制器字符设备属性指针 */

/* 文件操作结构体 */
struct file_operations g_infra_upgrade_fops = {
    .owner = THIS_MODULE,
    .open = dev_upgrade_open,
    .release = dev_upgrade_release,
    .unlocked_ioctl = dev_upgrade_ioctl,
};

/* dev_id掩码，1代表使能,0代表不使能，默认0~3都使能 */
#define DEV_ID_MASK 0xf
unsigned int g_dev_id_mask = 0;
dev_upgrade_core_ctrl g_upgrade_core_ctrl[DEVICE_NUM_MAX] = {{0}};

#define UPGRADE_DEV_CLASS "upgrade_dev_class"
#define DEV_UPGRADE_NAME_KER "upgrade"
#define UPGRADE_DEV_MAX_COUNT 1
#define NEED_PERMISSION_CHECK 1
#define NOT_NEED_PERMISSION_CHECK 0

#define CHECK_PARA_SIZE_RETURN(size, para_size) do {                                                   \
        if ((size) != (para_size)) {                           \
            dev_upgrade_err("prarameter_size is error\n"); \
            return (-EINVAL);                                \
        }                                                  \
    } while (0)

#if (!defined CFG_SOC_PLATFORM_MDC_V51)
#define ANOTHER_AREA_GET(a) (((a) == FLASH_AREA_0) ? (FLASH_AREA_1) : (FLASH_AREA_0))

struct upgrade_component g_upgrade_component_list[] = {
    {
        0, DSMI_COMPONENT_TYPE_NVE, SINGLE_AREA, 0, FLASH_AREA_0, 1, 0, NULL, dev_upgrade_pre_check_nve,
        dev_upgrade_update_nve
    },
    {
        0, DSMI_COMPONENT_TYPE_XLOADER, DOUBLE_AREA, 0, FLASH_AREA_0, 1, 0, NULL, dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area
    },
    {
        0, DSMI_COMPONENT_TYPE_M3FW, SINGLE_AREA, 0, FLASH_AREA_0, 1, 0, NULL, dev_upgrade_pre_check_common,
        dev_upgrade_update_single_area_stub
    },
    {
        0, DSMI_COMPONENT_TYPE_UEFI, SINGLE_AREA, 0, FLASH_AREA_0, 1, 0, NULL, dev_upgrade_pre_check_common,
        dev_upgrade_update_single_area
    },
    {
        0, DSMI_COMPONENT_TYPE_TEE, SINGLE_AREA, 0, FLASH_AREA_0, 1, 0, NULL, dev_upgrade_pre_check_common,
        dev_upgrade_update_single_area_stub
    },
    {
        0, DSMI_COMPONENT_TYPE_DTB, SINGLE_AREA, 0, FLASH_AREA_0, 1, 0, NULL, dev_upgrade_pre_check_common,
        dev_upgrade_update_single_area_stub
    },
    {
        0, DSMI_COMPONENT_TYPE_KERNEL, SINGLE_AREA, 0, FLASH_AREA_0, 1, 0, NULL, dev_upgrade_pre_check_common,
        dev_upgrade_update_single_area_stub
    },
    {
        0, DSMI_COMPONENT_TYPE_ROOTFS, SINGLE_AREA, 0, FLASH_AREA_0, 1, 0, NULL, dev_upgrade_pre_check_common,
        dev_upgrade_update_single_area_stub
    },
    {
        0, DSMI_COMPONENT_TYPE_BOOTROM, SINGLE_AREA, 0, FLASH_AREA_0, 1, 0, NULL, dev_upgrade_pre_check_common,
        dev_upgrade_update_single_area
    },
    {
        0, DSMI_COMPONENT_TYPE_IMU, DOUBLE_AREA, 0, FLASH_AREA_1, 1, 0, NULL, dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area
    },
    {
        0, DSMI_COMPONENT_TYPE_IMP, DOUBLE_AREA, 0, FLASH_AREA_0, 1, 0, NULL, dev_upgrade_pre_check_common,
        dev_upgrade_update_double_area
    },
    {
        0, DSMI_COMPONENT_TYPE_MAX, 0, 0, FLASH_AREA_0, 0, 0, NULL, NULL, NULL
    }
};

STATIC void upgrade_wakelock_activate(void)
{
    g_upgrade_work_count++;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    if (g_upgrade_source != NULL) {
        if (g_upgrade_source->active) {
            return;
        }
        __pm_stay_awake(g_upgrade_source);
    }
#else
    if (g_upgrade_source.active) {
        return;
    }
    __pm_stay_awake(&g_upgrade_source);
#endif
}


void dev_upgrade_devid_enable(int dev_id, unsigned int enable)
{
    if (enable) {
        g_dev_id_mask |= (((unsigned int)1 << (unsigned int)dev_id) & DEV_ID_MASK);
    } else {
        g_dev_id_mask &= ~(((unsigned int)1 << (unsigned int)dev_id) & DEV_ID_MASK);
    }

    return;
}

STATIC int dev_upgrade_check_call_process(void)
{
    int ret;
    const char *wl_process_name = CHECK_PROCESS_DMP;

    /* check process name and related bin/so excutable section sha256 */
    ret = whitelist_process_handler(&wl_process_name, 1);
    if (ret) {
        dev_upgrade_err("Permission denied! ret = %d.\n", ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_PERMISSION_DENIED);
    }

    return 0;
}

#define CHECK_PROCESS_PERMISSION()                   \
    do {                                             \
        if (dev_upgrade_check_call_process() != 0) {                         \
            dev_upgrade_err("Permission denied!\n");                         \
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_PERMISSION_DENIED); \
        }                                                                    \
    } while (0)


STATIC int dev_upgrade_devid_enable_check(int dev_id)
{
    return (g_dev_id_mask & ((unsigned int)1 << (unsigned int)dev_id)) ? TRUE : FALSE;
}

STATIC dev_upgrade_core_ctrl *dev_upgrade_get_upgrade_core_ctrl(int dev_id)
{
    if ((dev_id >= DEVICE_NUM_MAX) || (dev_id < 0)) {
        dev_upgrade_err("dev(%d) invalid\n", dev_id);
        return NULL;
    }

    return &g_upgrade_core_ctrl[dev_id];
}

#if (!defined CFG_SOC_PLATFORM_MDC_V51) && (!defined CFG_SOC_PLATFORM_MINIV2)
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

    if (length > FLASH_SIZE_MAX || length <= IMAGE_CODE_OFFSET) {
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

int dev_upgrade_schedule_update(int dev_id, unsigned int component_type,
    unsigned int total_size, unsigned int finish_size)
{
    unsigned int i;
    unsigned char schedule_calc;
    unsigned int curr_comp_finish_size;
    struct upgrade_component *component = NULL;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;

    dev_upgrade_info("devid=%d, component type=%u, total size=%u, finished size=%u\n",
        dev_id, component_type, total_size, finish_size);

    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(dev_id);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }

    dev_upgrade_info("upgrade_ctrl->total_size=%u, upgrade_ctrl->finish_size=%u\n",
                     upgrade_ctrl->total_size, upgrade_ctrl->finish_size);
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
    dev_upgrade_info("component->finish_areas=%u, component->weight=%u\n",
                     component->finish_areas, component->weight);

    curr_comp_finish_size = (component->finish_areas * total_size + component->weight * finish_size);

    DRV_CHECK_RETV(upgrade_ctrl->total_size != 0, ERRNO_INVALID_LENGTH);
    schedule_calc = (unsigned char)(((curr_comp_finish_size + upgrade_ctrl->finish_size) *
        UPGRADE_SCHEDULE_PERCENTAGE_100) / upgrade_ctrl->total_size);

    if (schedule_calc >= UPGRADE_SCHEDULE_PERCENTAGE_100) {
        dev_upgrade_warn("dev_id %d dev_upgrade_calc_schedule warn, schedule_calc : %u\n", dev_id, schedule_calc);
        schedule_calc = UPGRADE_SCHEDULE_PERCENTAGE_100 - 1;
    }
    dev_upgrade_info("curr_comp_finish_size=%u, upgrade schedule=%u\n",
                     curr_comp_finish_size, (unsigned int)schedule_calc);
    upgrade_ctrl->schedule = schedule_calc;

    return 0;
}

STATIC int dev_upgrade_update_nve(struct upgrade_component *component)
{
    int ret;
    upg_comm_para comm_para = { 0 };

    DRV_CHECK_RETV(component != NULL, -1);

    comm_para.dev_id = component->dev_id;
    comm_para.type = component->component_type;
    comm_para.file_content = component->file_content;
    comm_para.file_size = component->file_size;

    ret = dev_upgrade_nve_update(&comm_para, component->file_content);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_nve_update fail.\n");
        return ret;
    }

    /* 更新进度 */
    (void)dev_upgrade_schedule_update(component->dev_id, component->component_type,
                                      component->file_size, component->file_size);

    dev_upgrade_event("dev_upgrade_nve_update write flash success, size = %u\n", component->file_size);

    return ret;
}

struct upgrade_component *dev_component_list_search(unsigned int component_type)
{
    unsigned int i = 0;
    struct upgrade_component *component_list = &g_upgrade_component_list[0];
    bool rc_flag = devdrv_is_pci_rc_mode();
    unsigned int count = sizeof(g_upgrade_component_list) / sizeof(struct upgrade_component);

    for (i = 0; i < count; i++) {
        if (component_type == component_list->component_type) {
            if (component_type == DSMI_COMPONENT_TYPE_UEFI && rc_flag) {
                component_list->total_areas = DOUBLE_AREA;
                component_list->update = dev_upgrade_update_double_area;
            }
            return component_list;
        }
        component_list++;
    }

    return NULL;
}

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

STATIC int dev_upgrade_pre_check_common(struct upgrade_component *component)
{
    if (component == NULL) {
        dev_upgrade_err("para is null.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (dev_upgrade_sec_verification(component->dev_id, component->component_type,
                                     component->file_content, component->file_size) != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_sec_verification fail.\n", component->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_HASH_CHECK);
    }

    return OK;
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
        dev_upgrade_err("dev(%d) get ver from flash fail, ret:%x\n", comm_para_tmp.dev_id, ret);
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
        dev_upgrade_err("dev(%d) get ver from mem fail, ret:%x\n", comm_para_tmp.dev_id, ret);
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
        dev_upgrade_err("version check fail, ret: %x\n", ret);
        return ret;
    }

    if (is_same_version == CHECK_NO) {
        *is_same = CHECK_NO;
        return 0;
    }

    ret = dev_upgrade_content_check(comm_para, phase, &is_same_content);
    if (ret != 0) {
        dev_upgrade_err("content check fail, ret: %x\n", ret);
        return ret;
    }

    if (is_same_content == CHECK_YES) {
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
        dev_upgrade_err("dev(%d) update fail, component%u area : %u\n",
            comm_para->dev_id, comm_para->type, comm_para->area);
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
        dev_upgrade_err("dev(%d) flash image check fail, ret: %x.\n", comm_para->dev_id, ret);
        return ret;
    }

    DEV_UPGRADE_VFREE(read_buff);

    return OK;
}

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
    upg_comm_para comm_para = { 0 };

    if (component->total_areas != SINGLE_AREA) {
        dev_upgrade_err("dev(%u) type(%u) total_areas error.\n",
            component->dev_id, component->component_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    comm_para.dev_id = component->dev_id;
    comm_para.type = component->component_type;
    comm_para.area = FLASH_AREA_0;
    comm_para.file_content = component->file_content;
    comm_para.file_size = component->file_size;

    ret = dev_upgrade_ver_and_content_check(&comm_para, PHASE_UPGRADE, &is_same);
    if (ret != 0) {
            dev_upgrade_err("dev(%d) ver and content check fail, ret:%x\n", comm_para.dev_id, ret);
        return ret;
    }

        if (is_same == CHECK_YES) {
            dev_upgrade_warn("dev(%d) the version and content are the same.\n", comm_para.dev_id);
        return 0;
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

STATIC int dev_upgrade_double_image_check(upg_comm_para *comm_para, unsigned int *img_state)
{
    int ret;
    unsigned int index;
    unsigned int check_res = CHECK_YES;
    upg_comm_para comm_para_tmp = *comm_para;

    /* comm_para->area default is main */
    for (index = 0; index < DEV_FLASH_DOUBLE_AREAS; index++) {
        ret = dev_upgrade_image_integrity_check(&comm_para_tmp, &check_res);
        if (ret != 0) {
            dev_upgrade_warn("dev(%d) dev_upgrade_image_integrity_check fail, ret:%x\n", comm_para->dev_id, ret);
        }

        /* bit0 -> main area, bit1 -> standby area */
        *img_state |= (check_res == CHECK_NO) ? (1U << index) : 0;

        /* comm_para->area switch to standby */
        comm_para_tmp.area = ANOTHER_AREA_GET(comm_para->area);
    }

    dev_upgrade_info("dev(%d) component(%u) image state: %u\n", comm_para->dev_id, comm_para->type, *img_state);

    return 0;
}

STATIC int dev_upgrade_update_image(struct upgrade_component *component,
    upg_comm_para *comm_para, unsigned int upgrade_flag)
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
                component->dev_id, component->component_type);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
        }
    } else if (upgrade_flag == UPGRADE_MASTER) {
        component->finish_areas = 0;
        component->weight = DEV_FLASH_DOUBLE_AREAS;
        ret = dev_upgrade_write_flash_and_check(&comm_para_tmp);
        if (ret != 0) {
            dev_upgrade_err("Failed to upgrade the component and would stop upgrading. (dev=%d; component=%u)\n",
                component->dev_id, component->component_type);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
        }

        component->finish_areas = component->total_areas;
        dev_upgrade_info("The component was upgraded successfully. (dev=%d; component=%u; area=%u)\n",
            component->dev_id, component->component_type, component->main_area);
    }

    /* clear start fail count for selecting boot from main area after reboot */
    ret = dev_upgrade_component_boot_area_op(component->dev_id,
        component->component_type, DEV_CLEAR_BOOT_COUNT, &op_area);
    if (ret != 0) {
        dev_upgrade_err("Failed to clear boot-count. (ret=%x)\n", ret);
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
    upg_comm_para comm_para = { 0 };

    if (component->total_areas != DOUBLE_AREA) {
        dev_upgrade_err("dev(%u) type(%u) total_areas error.\n",
            component->dev_id, component->component_type);
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
        dev_upgrade_err("dev_upgrade_double_image_check fail, ret:%x\n", ret);
        return ret;
    }

    if (img_state == IMG_STAT_DOUBLE_OK) {
        ret = dev_upgrade_ver_and_content_check(&comm_para, PHASE_UPGRADE, &is_same);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_ver_and_content_check fail, ret:%x\n", ret);
            return ret;
        }
        upgrade_flag = (is_same == CHECK_YES) ? UPGRADE_NONE : UPGRADE_MASTER;
    } else if (img_state == IMG_STAT_M_OK_S_BAD) {
        /* sync the main area to the standby area first, and then upgrade the main area */
        ret = dev_upgrade_component_image_copy(comm_para.dev_id, comm_para.type, standby_area, comm_para.area);
        if (ret != 0) {
            dev_upgrade_err("dev(%d) component(%u) copy from area(%u) to area(%u) fail, ret: %d\n",
                comm_para.dev_id, comm_para.type, comm_para.area, standby_area, ret);
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
        dev_upgrade_err("dev(%d) dev_upgrade_update_image fail, ret:%x\n", component->dev_id, ret);
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
#else
    unsigned int bios_type = DSMI_COMPONENT_TYPE_XLOADER;
#endif

    if (upgrade_ctrl->clr_localcfg && (component->component_type == bios_type)) {
        dev_upgrade_clear_localcfg(upgrade_ctrl->dev_id);
        upgrade_ctrl->clr_localcfg = 0;
    }
}
#endif

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
        ret = ERROR;
    }

    dev_upgrade_core_resource_release(upgrade_ctrl);

    return ret;
}

STATIC int dev_upgrade_update_proc(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    return dev_upgrade_unsec_update_proc(upgrade_ctrl);
}

#if (!defined CFG_SOC_PLATFORM_MDC_V51) && (!defined CFG_SOC_PLATFORM_MINIV2)
STATIC void dev_upgrade_sync_info_print(upg_comm_para *comm_para)
{
    int ret;
    unsigned char src_ver[COMM_VERSION_LENGTH] = { 0 };
    unsigned char dest_ver[COMM_VERSION_LENGTH] = { 0 };
    unsigned int *pver_src = (unsigned int *)&src_ver;
    unsigned int *pver_dest = (unsigned int *)&dest_ver;
    upg_comm_para comm_para_tmp = *comm_para;

    ret = dev_upgrade_component_version_get(&comm_para_tmp, 0, src_ver, COMM_VERSION_LENGTH);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) get ver from flash fail, ret:%x\n", comm_para_tmp.dev_id, ret);
        return ;
    }

    dev_upgrade_info("[M.F.R.B] dev%u component%u area%u version is %x.%x.%x.%x\n",
        comm_para_tmp.dev_id, comm_para_tmp.type, comm_para_tmp.area, pver_src[INDEX_0],
        pver_src[INDEX_1], pver_src[INDEX_2], pver_src[INDEX_3]);

    /* msg->area switch to another */
    comm_para_tmp.area = ANOTHER_AREA_GET(comm_para->area);
    ret = dev_upgrade_component_version_get(&comm_para_tmp, 0, dest_ver, COMM_VERSION_LENGTH);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) get ver from flash fail, ret:%x\n", comm_para_tmp.dev_id, ret);
        return ;
    }

    dev_upgrade_info("[M.F.R.B] dev%u component%u area%u version is %x.%x.%x.%x\n",
        comm_para_tmp.dev_id, comm_para_tmp.type, comm_para_tmp.area, pver_dest[INDEX_0],
        pver_dest[INDEX_1], pver_dest[INDEX_2], pver_dest[INDEX_3]);
}
#endif

STATIC int dev_upgrade_component_sync_check(upg_comm_para *comm_para, unsigned int *need_sync)
{
    int ret;
    unsigned int is_same = CHECK_NO;

    *need_sync = CHECK_NO;

    ret = dev_upgrade_ver_and_content_check(comm_para, PHASE_SYNC, &is_same);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_ver_and_content_check fail, ret:%x\n", ret);
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
    upg_comm_para comm_para = { 0 };

    ret = dev_upgrade_component_boot_area_op(dev_id, component_type, DEV_GET_CURR_BOOT_AREA, &from_area);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) get curr boot area fail, boot_area: %u, component type: %u, ret: %d.\n",
                        dev_id, from_area, component_type, ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    to_area = ANOTHER_AREA_GET(from_area);

    comm_para.dev_id = dev_id;
    comm_para.type = component_type;
    comm_para.area = from_area;
    /* before sync, verify the signature of the current area */
    ret = dev_upgrade_image_integrity_check(&comm_para, &check_res);
    if ((ret != 0) || (check_res != CHECK_YES)) {
        dev_upgrade_err("dev(%d) component(%u) arae%u image check fail, need to report alarm, ret: %d\n",
                        dev_id, component_type, from_area, ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_HASH_CHECK);
    }

    ret = dev_upgrade_component_sync_check(&comm_para, &need_sync);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_component_sync_check fail, ret:%x\n", dev_id, ret);
        return ret;
    }
    if (need_sync) {
        ret = dev_upgrade_component_image_copy(dev_id, component_type, to_area, from_area);
        if (ret != 0) {
            dev_upgrade_err("dev(%d) component(%u) sync from area%u to area%u fail.\n",
                dev_id, component_type, from_area, to_area);
            return ret;
        } else {
            dev_upgrade_info("dev(%d) component(%u) sync from area%u to area%u success.\n",
                             dev_id, component_type, from_area, to_area);
        }
    }

    ret = dev_upgrade_component_boot_area_op(dev_id, component_type, DEV_CLEAR_BOOT_COUNT, NULL);
    if (ret != 0) {
        dev_upgrade_warn("dev(%d) clear boot count fail, ret:%x\n", dev_id, ret);
        return ret;
    }

    return OK;
}


STATIC int dev_upgrade_sync_proc(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int ret;

    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("upgrade_ctrl is NULL\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    /* mini EP need sync xloader only */
    ret = dev_upgrade_sync_image(upgrade_ctrl->dev_id, DSMI_COMPONENT_TYPE_XLOADER);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_sync_firmware fail\n");
        return ret;
    }

    if (devdrv_is_pci_rc_mode()) {
        ret = dev_upgrade_sync_image(upgrade_ctrl->dev_id, DSMI_COMPONENT_TYPE_UEFI);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_sync_uefi fail\n");
            return ret;
        }
    }

    return 0;
}

STATIC struct upgrade_component *dev_upgrade_component_find(int dev_id,
    unsigned int type, unsigned int file_size)
{
    int ret;
    struct upgrade_component *component = NULL;

    ret = dev_upgrade_component_space_check(dev_id, type, file_size);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) component(%u) check space fail, ret: %x\n", dev_id, type, ret);
        return NULL;
    }

    component = dev_component_list_search(type);
    if ((file_size > UPGRADE_FILE_SIZE_MAX) || (component == NULL)) {
        dev_upgrade_err("dev(%d) component(%u) find component fail or size(%d) error\n",
                        dev_id, type, file_size);
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

STATIC int dev_upgrade_ioctl_firmware_start(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;
    struct upgrade_start_in fw_start_in = { 0 };
    struct upgrade_start_out fw_start_out = { 0 };

    CHECK_PARA_SIZE_RETURN(upgrade_msg->in_size, sizeof(struct upgrade_start_in));
    CHECK_PARA_SIZE_RETURN(upgrade_msg->out_size, sizeof(struct upgrade_start_out));

    if (copy_from_user((void *)&fw_start_in, (void *)upgrade_msg->in, sizeof(struct upgrade_start_in))) {
        dev_upgrade_err("copy_from_user failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }
    DRV_CHECK_RETV((fw_start_in.count <= DSMI_COMPONENT_TYPE_MAX), ERRNO_INVALID_PARAMS);

    upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(fw_start_in.dev_id);
    if (upgrade_ctrl == NULL) {
        dev_upgrade_err("get ctrl handle fail, dev(%d)\n", fw_start_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
        goto out;
    }

    mutex_lock(&upgrade_ctrl->lock);
    upgrade_wakelock_activate();
    upgrade_ctrl->total_size = 0;
    upgrade_ctrl->finish_size = 0;
    upgrade_ctrl->schedule = 0;
    upgrade_ctrl->total_component = fw_start_in.count;
    dev_upgrade_info("upgrade start, dev(%d), count: %u\n", upgrade_ctrl->dev_id, upgrade_ctrl->total_component);

    ret = dev_upgrade_components_fill(upgrade_ctrl, &fw_start_in);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_components_fill fail, ret: %x\n", upgrade_ctrl->dev_id, ret);
        mutex_unlock(&upgrade_ctrl->lock);
        goto out;
    }

    ret = dev_upgrade_update_proc(upgrade_ctrl);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_update_proc fail, ret: %x\n", upgrade_ctrl->dev_id, ret);
        mutex_unlock(&upgrade_ctrl->lock);
        goto out;
    }

    ret = 0;
    mutex_unlock(&upgrade_ctrl->lock);

out:
    fw_start_out.result = ret;
    if (copy_to_user((void *)upgrade_msg->out, (void *)&fw_start_out, sizeof(struct upgrade_start_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", fw_start_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }
    return ret;
}

STATIC int dev_upgrade_version_get(int dev_id, unsigned int type, unsigned int area,
    unsigned char *buf, unsigned int len)
{
    int ret;
    upg_comm_para comm_para = { 0 };
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

STATIC int dev_upgrade_ioctl_get_version(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    unsigned char ver_buf[COMM_VERSION_LENGTH] = { 0 };
    struct upgrade_get_version_in ver_in = { 0 };
    struct upgrade_get_version_out ver_out = { 0 };
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

    /* get version from flash */
    ret = dev_upgrade_version_get(ver_in.dev_id, ver_in.component_type, cur_flag,
                                  ver_buf, COMM_VERSION_LENGTH);
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
    if (copy_to_user((void *)upgrade_msg->out, (void *)&ver_out, sizeof(struct upgrade_get_version_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", ver_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }
    return ret;
}

STATIC int dev_upgrade_ioctl_get_schedule(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret = 0;
    struct upgrade_read_schedule_in schedule_in = { 0 };
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

    if (copy_to_user((void *)upgrade_msg->out, (void *)&upgrade_ctrl->schedule,
                     sizeof(struct upgrade_read_schedule_out))) {
        dev_upgrade_err("dev(%d) copy_to_user failed\n", schedule_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }

    return ret;
}

STATIC int dev_upgrade_ioctl_get_component_list(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    unsigned int bitmap = 0;
    struct upgrade_get_component_list_in list_in = { 0 };

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
    struct upgrade_sync_image_in sync_image_in = { 0 };
    struct upgrade_sync_image_out sync_image_out = { 0 };

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
    upgrade_wakelock_activate();
    ret = dev_upgrade_sync_proc(upgrade_ctrl);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_sync_proc fail, ret: %d\n", sync_image_in.dev_id, ret);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
        mutex_unlock(&upgrade_ctrl->lock);
        goto out;
    }

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
    struct upgrade_stop_in stop_in = { 0 };
    struct upgrade_stop_out stop_out = { 0 };
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
    struct upgrade_clr_localcfg_in clr_localcfg_in = { 0 };
    struct upgrade_clr_localcfg_out clr_localcfg_out = { 0 };
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

int dev_upgrade_ioctl_verify_img(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    struct upgrade_verify_img_in verify_img_in = { 0 };
    struct upgrade_verify_img_out verify_img_out = { 0 };
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

    if ((verify_img_in.size > PATH_MAX) || (verify_img_in.size == 0)) {
        dev_upgrade_err("dev_id(%d) input path length[%d] out of range.\n",
            verify_img_in.dev_id, verify_img_in.size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    tmp_path = (unsigned char *)kzalloc(verify_img_in.size + 1, GFP_KERNEL | __GFP_ACCOUNT);
    if (tmp_path == NULL) {
        dev_upgrade_err("tmp_path kzalloc fail, size=%d.\n", verify_img_in.size);
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
        dev_upgrade_err("dev_id(%d) verify image(%s) fail, ret:%d\n", verify_img_in.dev_id,
            tmp_path, ret);
    }

    verify_img_out.result = ret;
    if (copy_to_user((void *)upgrade_msg->out, (void *)&verify_img_out, sizeof(struct upgrade_verify_img_out))) {
        dev_upgrade_err("dev_id(%d) copy_to_user failed\n", verify_img_in.dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }

    DEV_UPGRADE_KFREE(tmp_path);
    return ret;
}
STATIC int dev_upgrade_ioctl_get_check_info(struct upgrade_ioctl_msg *upgrade_msg)
{
    int ret;
    IMG_CHECK_INFO info;
    struct upgrade_get_check_info_in ci_in = { 0 };
    struct upgrade_get_check_info_out ci_out = { 0 };

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

upgrade_cmd_map_t g_upgrade_cmd_func_map[] = {
    { UPGRADE_READ_SCHEDULE_CTL, NOT_NEED_PERMISSION_CHECK, dev_upgrade_ioctl_get_schedule },
    { UPGRADE_FIRMWARE_START, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_firmware_start },
    { UPGRADE_GET_FIREWARE_VER, NOT_NEED_PERMISSION_CHECK, dev_upgrade_ioctl_get_version },
    { UPGRADE_GET_COMPONENT_LIST, NOT_NEED_PERMISSION_CHECK, dev_upgrade_ioctl_get_component_list },
    { UPGRADE_SYNC_IMAGE_COPY, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_sync_image_copy },
    { UPGRADE_FIRMWARE_STOP, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_stop },
    { UPGRADE_CLR_LOCALCFG, NEED_PERMISSION_CHECK, dev_upgrade_ioctl_clr_localcfg },
    { UPGRADE_VERIFY_IMG, NOT_NEED_PERMISSION_CHECK, dev_upgrade_ioctl_verify_img },
    { UPGRADE_GET_CHECK_INFO, NOT_NEED_PERMISSION_CHECK, dev_upgrade_ioctl_get_check_info },
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

    if (!dev_upgrade_devid_enable_check(upgrade_msg.dev_id)) {
        dev_upgrade_err("dev(%d) is disable\n", upgrade_msg.dev_id);
        return -EINVAL;
    }

    ret = (long)dev_upgrade_ioctl_distribute(cmd, &upgrade_msg);
    if (ret > 0) {
        /* if return value is positive, user mode ioctl func will return 0 */
        ret = -ret;
    }

    if (ret != 0) {
        dev_upgrade_err("dev(%d) ioctl failed, ret: %ld\n", upgrade_msg.dev_id, ret);
        return -EINVAL;
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

STATIC void dev_upgrade_release_runing(int max_index)
{
    int j;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;
    DRV_CHECK_RET(max_index <= DEVICE_NUM_MAX);
    for (j = 0; j < max_index; j++) {
        if (dev_upgrade_devid_enable_check(j)) {
            upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(j);
            if (upgrade_ctrl == NULL) {
                continue;
            }
            mutex_unlock(&upgrade_ctrl->lock);
        }
    }
}

STATIC int dev_upgrade_check_running(void)
{
    int i = 0;
    int ret = 0;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;
    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        if (dev_upgrade_devid_enable_check(i)) {
            upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(i);
            if (upgrade_ctrl == NULL) {
                continue;
            }
            ret = mutex_trylock(&upgrade_ctrl->lock);
            if (!ret) {
                return -1;
            }
            mutex_unlock(&upgrade_ctrl->lock);
        }
    }

    return 0;
}

#define UPGRADE_WAKELOCK_TIMEOUT 3
#define UPGRADE_WAKELOCK_CHECK_TIME 1000


#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
STATIC void upgrade_wdt_handler(unsigned long time)
#else
STATIC void upgrade_wdt_handler(struct timer_list *t)
#endif
{
    static int count = 0;
    static unsigned int curr_upgrade_work_count = 0;
    unsigned long timeout;
    int ret;
    ret = dev_upgrade_check_running();
    if (ret != 0) {
        count = 0;
    } else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
        if (g_upgrade_source != NULL && g_upgrade_source->active) {
            if (curr_upgrade_work_count == g_upgrade_work_count) {
                count++;
                dev_upgrade_info("upgrade idle, wait 3s count:%d\n", count);
            } else {
                dev_upgrade_info("upgrade works during 3s, recount it:%d\n", count);
                count = 0;
            }

            if (count == UPGRADE_WAKELOCK_TIMEOUT) {
                dev_upgrade_info("upgrade has not worked for 3s, release wakeup source:%s\n",
                    g_upgrade_source->name);
                __pm_relax(g_upgrade_source);
                count = 0;
            }
        }
#else
        if (g_upgrade_source.active) {
            if (curr_upgrade_work_count == g_upgrade_work_count) {
                count++;
                dev_upgrade_info("upgrade idle, wait 3s count:%d\n", count);
            } else {
                dev_upgrade_info("upgrade works during 3s, recount it:%d\n", count);
                count = 0;
            }

            if (count == UPGRADE_WAKELOCK_TIMEOUT) {
                dev_upgrade_info("upgrade has not worked for 3s, release wakeup source:%s\n", g_upgrade_source.name);
                __pm_relax(&g_upgrade_source);
                count = 0;
            }
        }
#endif
    }

    timeout = UPGRADE_WAKELOCK_CHECK_TIME;

    mod_timer(&upgrade_ext_wdt_timer, jiffies + msecs_to_jiffies(timeout));

    curr_upgrade_work_count = g_upgrade_work_count;
    return;
}


STATIC void upgrade_wdt_init(unsigned char ntime)
{
    /* ** Initialize the timer structure ** */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
    init_timer(&upgrade_ext_wdt_timer);                   // 初始化内核定时器
    upgrade_ext_wdt_timer.function = upgrade_wdt_handler; // 定时器函数，这里每隔一秒执行一次
#else
    timer_setup(&upgrade_ext_wdt_timer, upgrade_wdt_handler, 0);
#endif
    upgrade_ext_wdt_timer.expires = jiffies + HZ * ntime; // HZ : 100
    add_timer(&upgrade_ext_wdt_timer);
    /* ** Initialisation ends ** */
    return;
}

STATIC void upgrade_wdt_exit(void)
{
    del_timer_sync(&upgrade_ext_wdt_timer);
}

STATIC int dev_upgrade_check_running_and_lock(void)
{
    int i = 0;
    int ret = TRUE;
    dev_upgrade_core_ctrl *upgrade_ctrl = NULL;
    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        if (dev_upgrade_devid_enable_check(i)) {
            upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(i);
            if (upgrade_ctrl == NULL) {
                continue;
            }
            ret = mutex_trylock(&upgrade_ctrl->lock);
            if (!ret) {
                dev_upgrade_warn("upgrade is running and is not allowed to sleep.\n");
                dev_upgrade_release_runing(i);
                return -1;
            }
        }
    }
    return 0;
}

STATIC int sleep_upgradedrv_pm_notify(struct notifier_block *nb, unsigned long mode, void *_unused)
{
    int ret = 0;
    switch (mode) {
        // design this function.
        case PM_SUSPEND_PREPARE:
            ret = dev_upgrade_check_running_and_lock();
            if (ret != 0) {
                dev_upgrade_warn("upgrade is running,so exit suspend.\n");
                return NOTIFY_BAD;
            }
            dev_upgrade_info("upgrade is prepare suspend.\n");
            break;
        case PM_POST_SUSPEND:
            dev_upgrade_release_runing(DEVICE_NUM_MAX);
            dev_upgrade_info("upgrade is post suspend.\n");
            break;
        default:
            break;
    }
    return NOTIFY_OK;
}

static struct notifier_block sleep_upgradedrv_pm_nb = {
    .notifier_call = sleep_upgradedrv_pm_notify,
};

STATIC int hisi_d_upgarde_driver_probe(struct platform_device *pltdev)
{
    return 0;
}

STATIC int hisi_d_upgarde_driver_remove(struct platform_device *pltdev)
{
    return 0;
}

STATIC int hisi_d_upgarde_driver_suspend(struct platform_device *pltdev, pm_message_t state)
{
    dev_upgrade_info("dev_upgrade  suspend\n");
    return 0;
}

STATIC int hisi_d_upgarde_driver_resume(struct platform_device *pltdev)
{
    dev_upgrade_info("dev_upgrade resume\n");
    return 0;
}
/*lint -e485 */
static struct platform_driver hisi_d_upgarde_pltdrv = {
    .probe = hisi_d_upgarde_driver_probe,
    .remove = hisi_d_upgarde_driver_remove,
    .suspend = hisi_d_upgarde_driver_suspend,
    .resume = hisi_d_upgarde_driver_resume,
    .driver.name = "hisi_d_upgrade",
    .driver.owner = THIS_MODULE,
    .driver.bus = &platform_bus_type,
}; /*lint +e485 */

STATIC void dev_upgrade_cleanup_dev(CDEV_ST *pdev)
{
    dev_t dev_no;

    if ((pdev == NULL) || (pdev->dev_class == NULL)) {
        dev_upgrade_err("dev_upgrade_cleanup_dev input para is NULL, pdev: %pK\r\n", pdev);
        return;
    }

    dev_no = MKDEV(pdev->dev_major, pdev->dev_minor);
    platform_driver_unregister(&hisi_d_upgarde_pltdrv);
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

    rc = platform_driver_register(&hisi_d_upgarde_pltdrv);
    if (rc < 0) {
        dev_upgrade_err("register dev_upgrade driver failed!");
        goto exit_cdev_add;
    }

    return 0;
exit_cdev_add:
    cdev_del(&pdev->cdev);
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

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        upgrade_ctrl = dev_upgrade_get_upgrade_core_ctrl(i);
        if (upgrade_ctrl == NULL) {
            dev_upgrade_err("get ctrl handle fail, dev(%d)\n", i);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
        }

        ret = memset_s((void *)upgrade_ctrl, sizeof(dev_upgrade_core_ctrl), 0, sizeof(dev_upgrade_core_ctrl));
        if (ret != 0) {
            dev_upgrade_err("memset_s fail: %d\n", ret);
            return ret;
        }

        ret = dev_upgrade_component_info_match(i, &match_nums);
        if (ret != 0) {
            dev_upgrade_warn("dev_upgrade_component_info_match fail, ret:%x", ret);
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

STATIC int __init dev_upgrade_module_init(void)
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
        dev_upgrade_err("dev_upgrade_init_drv fail, ret:%x", ret);
        goto unint_drv;
    }

    ret = register_pm_notifier(&sleep_upgradedrv_pm_nb);
    if (ret != 0) {
        dev_upgrade_err("register_pm_notifier failed:%d\n", ret);
        ret = -ENOENT;
        goto unint_drv;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    g_upgrade_source = wakeup_source_create(upgrade_wakelock_name);
    if (g_upgrade_source == NULL)
    {
        ret = -ENOMEM;
        goto unint_drv;
    }
    wakeup_source_add(g_upgrade_source);;
#else
    wakeup_source_init(&g_upgrade_source, upgrade_wakelock_name);
#endif
    upgrade_wdt_init(1);

    return OK;

unint_drv:
    dev_upgrade_mutex_destroy();
    dev_upgrade_cleanup_dev(g_infra_upgrade_dev);
free_dev:
    DEV_UPGRADE_KFREE(g_infra_upgrade_dev);
    return ret;
}

STATIC void __exit dev_upgrade_module_exit(void)
{

    upgrade_wdt_exit();

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    if (g_upgrade_source != NULL)
    {
        wakeup_source_remove(g_upgrade_source);
        wakeup_source_destroy(g_upgrade_source);
        g_upgrade_source = NULL;
    }
#else
    wakeup_source_trash(&g_upgrade_source);
#endif

    (void)unregister_pm_notifier(&sleep_upgradedrv_pm_nb);

    /* 字符设备清除 */
    if (g_infra_upgrade_dev != NULL) {
        dev_upgrade_cleanup_dev(g_infra_upgrade_dev);
        DEV_UPGRADE_KFREE(g_infra_upgrade_dev);
    }

    dev_upgrade_mutex_destroy();

    dev_upgrade_info("upgrade_clean ok!");

    return;
}

module_init(dev_upgrade_module_init);
module_exit(dev_upgrade_module_exit);
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("dev firmware upgrade driver");
