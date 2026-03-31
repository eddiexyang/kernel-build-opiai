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
#include <linux/sort.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/kallsyms.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include "drvfault_user_common.h"
#include "dev_upgrade_public.h"
#include "dev_upgrade_core.h"
#include "dev_upgrade_adapt.h"
#include "devdrv_dfm.h"
#include "dev_upgrade_sec.h"
#include "dev_upgrade_mdccore.h"
#include "dev_upgrade_ufs.h"
#include "dev_upgrade_ssd.h"
#include "dev_upgrade_crl.h"
#include "devdrv_manager_comm.h"

#define CHECK_PARA_SIZE_RETURN(size, para_size)            \
    do {                                                   \
        if ((size) != (para_size)) {                       \
            dev_upgrade_err("prarameter_size is error\n"); \
            return (-EINVAL);                              \
        }                                                  \
    } while (0)

#define ANOTHER_AREA_GET(a) (((a) == FLASH_AREA_0) ? (FLASH_AREA_1) : (FLASH_AREA_0))

#define UFS_COMPONENT_NUM 7

upgrade_ufs_component_info *g_upgrade_ufs_local_component_core_info[UFS_COMPONENT_NUM] = {NULL};

const char *g_component_name[UFS_COMPONENT_NUM] = {
    "RAWData", "RO_SysDrv", "RO_ADSApp", "RO_ComIsolator", "RO_Cluster", "RO_Customized", "Recovery"};

const DSMI_COMPONENT_TYPE g_component_type[UFS_COMPONENT_NUM] = {DSMI_COMPONENT_TYPE_RAWDATA,
    DSMI_COMPONENT_TYPE_SYSDRV,
    DSMI_COMPONENT_TYPE_ADSAPP,
    DSMI_COMPONENT_TYPE_COMISOLATOR,
    DSMI_COMPONENT_TYPE_CLUSTER,
    DSMI_COMPONENT_TYPE_CUSTOMIZED,
    DSMI_COMPONENT_TYPE_RECOVERY};

#define UPGRADE_LUN_NUM_MAX 5

#define UPGRADE_SYS_FILE_SIZE_MAX 13 /* including '\0', 12 bytes is big enough */

#define SIMPLE_STRTOL_BASE10 10

#define UPGRADE_SYS_SDA_SIZE "/sys/class/block/sda/size"
#define UPGRADE_SYS_SDB_SIZE "/sys/class/block/sdb/size"
#define UPGRADE_SYS_SDC_SIZE "/sys/class/block/sdc/size"
#define UPGRADE_SYS_SDD_SIZE "/sys/class/block/sdd/size"
#define UPGRADE_SYS_SDE_SIZE "/sys/class/block/sde/size"

#define UPGRADE_SYS_SSD_SIZE "/sys/class/block/nvme0n1/size"
#define UPGRADE_SYS_EMMC_SIZE "/sys/class/block/mmcblk0/size"

char *g_ufs_blk_size_name[UPGRADE_LUN_NUM_MAX] = {
    UPGRADE_SYS_SDA_SIZE, UPGRADE_SYS_SDB_SIZE, UPGRADE_SYS_SDC_SIZE, UPGRADE_SYS_SDD_SIZE, UPGRADE_SYS_SDE_SIZE};

char *g_ssd_blk_size_name[UPGRADE_LUN_NUM_MAX] = {UPGRADE_SYS_SSD_SIZE, 0};

#ifdef CFG_SOC_PLATFORM_MDC_V11
char *g_emmc_blk_size_name[UPGRADE_LUN_NUM_MAX] = {UPGRADE_SYS_EMMC_SIZE, 0};
#endif

int g_pxe_or_normal_scenes = NORMAL_UPGRADE_SCENES;

upgrade_thread_trans_t g_upgrade_thread_trans[DEVICE_NUM_MAX] = {0};

#ifdef CFG_SOC_PLATFORM_MDC_V11
disk_ops_t g_disk_ops = {dev_upgrade_emmc_write, dev_upgrade_emmc_read, dev_upgrade_emmc_write, dev_upgrade_emmc_read};
#else
disk_ops_t g_disk_ops = {dev_upgrade_ufs_write, dev_upgrade_ufs_read, dev_upgrade_ufs_write, dev_upgrade_ufs_read};
#endif
upgrade_ver_and_state_t *g_upgrade_ver_and_state[DEVICE_NUM_MAX] = {0};
dev_upgrade_crl_info_t *g_upgrade_crl_info[DEVICE_NUM_MAX] = {NULL};

STATIC int dev_upgrade_set_media_read_write_cb(void);
STATIC int dev_upgrade_sync_mdc_system_type(void);

long long dev_upgrade_get_lun_size(int op_flag, u32 lun)
{
    int ret;
    int len;
    loff_t pos = 0;
    struct file *filp = NULL;
    long long lun_size = -1;
    unsigned int check_media_type = UPGRADE_UFS_MEDIA;
    char buf[UPGRADE_SYS_FILE_SIZE_MAX + 1] = {0};
    char *file_name = g_ufs_blk_size_name[lun];

    ret = dev_upgrade_get_media(QUERY_FROM_BOOTSTRAP, &check_media_type);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_get_media fail, op_type=%d, ret=%d.\n", op_flag, ret);
        return -EINVAL;
    }

    if (check_media_type == UPGRADE_SSD_MEDIA) {
        file_name = g_ssd_blk_size_name[0]; /* ssd only have 1 phy partition */
    }

#ifdef CFG_SOC_PLATFORM_MDC_V11
    if (check_media_type == UPGRADE_EMMC_MEDIA) {
        file_name = g_emmc_blk_size_name[0]; /* emmc only have 1 phy partition */
    }
#endif

    filp = filp_open(file_name, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(filp)) {
        dev_upgrade_err("unable to open file: %s (%ld)\n", file_name, PTR_ERR(filp));
        return -EINVAL;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
    len = kernel_read(filp, buf, UPGRADE_SYS_FILE_SIZE_MAX, &pos);
#else
    len = kernel_read(filp, &pos, buf, UPGRADE_SYS_FILE_SIZE_MAX);
#endif
    if ((len <= 0) || (len >= UPGRADE_SYS_FILE_SIZE_MAX)) { /* len including '\0' */
        dev_upgrade_err("read fail, len %d\n", len);
        goto FILE_CLOSE;
    }

    buf[len] = '\0';
    lun_size = (long long)simple_strtol(buf, NULL, SIMPLE_STRTOL_BASE10) * UPGRADE_SYS_FILE_BLOCK_SIZE;

    dev_upgrade_info("%s %d %lld\n", file_name, len, lun_size);

FILE_CLOSE:
    filp_close(filp, NULL);
    filp = NULL;
    return lun_size;
}

int dev_upgrade_get_lun_addr(upgrade_sub_file *sub_file, struct dev_upgrade_lun_info *lun_info)
{
    u32 i;
    mdc_package_t *mdc_pkg = NULL;
    struct dev_upgrade_lun_info *lun_info_proc = NULL;

    if (sub_file->sub_file_count > MAX_MDC_PKT_NUM) {
        dev_upgrade_err("sub file count illegal, count=%d\n", sub_file->sub_file_count);
        return -EINVAL;
    }

    for (i = 0; i < sub_file->sub_file_count; i++) {
        mdc_pkg = sub_file->sub_file_info[i];
        if (IS_ERR_OR_NULL(mdc_pkg)) {
            dev_upgrade_err("sub file is null, index=%d\n", i);
            return -EINVAL;
        }

        if (mdc_pkg->storage.lun >= UPGRADE_LUN_NUM_MAX) {
            dev_upgrade_err("sub file LUN is illegal, LUN=%d\n", mdc_pkg->storage.lun);
            return -EINVAL;
        }

        dev_upgrade_info("file name=%.*s, lun=%u, offset=%llu, len=%llu\n",
            MAX_NAME_LEN,
            mdc_pkg->name,
            mdc_pkg->storage.lun,
            mdc_pkg->storage.start,
            mdc_pkg->storage.len);

        lun_info_proc = lun_info + mdc_pkg->storage.lun;
        lun_info_proc->node_info[lun_info_proc->file_cnt].node_offset = mdc_pkg->storage.start;
        lun_info_proc->node_info[lun_info_proc->file_cnt].node_len = mdc_pkg->storage.len;
        lun_info_proc->file_cnt++;
    }

    return 0;
}

static int dev_upgrade_cmp(const void *a, const void *b)
{
    const struct dev_upgrade_lun_node *l = (const struct dev_upgrade_lun_node *)a;
    const struct dev_upgrade_lun_node *r = (const struct dev_upgrade_lun_node *)b;

    if (l->node_offset > r->node_offset) {
        return 1;
    } else if (l->node_offset < r->node_offset) {
        return -1;
    } else {
        return 0;
    }
}

static void dev_upgrade_swap(void *a, void *b, int size)
{
    struct dev_upgrade_lun_node *l = (struct dev_upgrade_lun_node *)a;
    struct dev_upgrade_lun_node *r = (struct dev_upgrade_lun_node *)b;
    struct dev_upgrade_lun_node tmp;

    (void)size;
    tmp = *l;
    *l = *r;
    *r = tmp;

    return;
}

int dev_upgrade_chk_lun_size(struct dev_upgrade_lun_info *lun_info_proc)
{
    u32 i;
    u64 offset, len;
    u64 end_prev = 0;
    u64 end_curr;

    sort((void *)lun_info_proc->node_info,
        lun_info_proc->file_cnt,
        sizeof(struct dev_upgrade_lun_node),
        dev_upgrade_cmp,
        dev_upgrade_swap);

    for (i = 0; i < lun_info_proc->file_cnt; i++) {
        offset = lun_info_proc->node_info[i].node_offset;
        len = lun_info_proc->node_info[i].node_len;
        end_curr = offset + len;

        if (end_curr > lun_info_proc->lun_len) {
            dev_upgrade_err("file %u out of limit, end_curr %llu limit %llu\n", i, end_curr, lun_info_proc->lun_len);
            return -ENOMEM;
        }
        if (end_prev > offset) {
            dev_upgrade_err("file %u range is mix, end prev %llu start %llu len %llu\n", i, end_prev, offset, len);
            return -ENOMEM;
        }

        end_prev = end_curr;
    }

    return 0;
}

STATIC int dev_upgrade_ufs_update_file_size_check(upgrade_sub_file *sub_file)
{
    int ret;
    unsigned int i;
    char *file_path_tmp = NULL;
    mdc_package_t *mdc_pkg = NULL;
    unsigned int file_size = 0;

    for (i = 0; i < sub_file->sub_file_count; i++) {
        mdc_pkg = sub_file->sub_file_info[i];
        if (mdc_pkg == NULL) {
            dev_upgrade_err("sub file is null, index=%d\n", i);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
        }

        file_path_tmp = mdc_pkg->file_name;
        if (file_path_tmp == NULL) {
            dev_upgrade_err("sub file path is null, index=%d\n", i);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
        }

        ret = dev_upgrade_get_fs_file_size(file_path_tmp, &file_size);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_get_fs_file_size fail, ret=%d.\n", ret);
            return ret;
        }

        /* file size out of range */
        if (file_size > mdc_pkg->storage.len) {
            dev_upgrade_err(
                "file: %s size[0x%x] is out of range[0x%llx].\n", file_path_tmp, file_size, mdc_pkg->storage.len);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
        }
    }

    return OK;
}

int dev_upgrade_component_ufs_para_check(upgrade_sub_file *sub_file, int op_flag)
{
    struct dev_upgrade_lun_info *lun_info = NULL;
    struct dev_upgrade_lun_info *lun_info_proc = NULL;
    long long lun_len;
    u32 i;
    int ret = 0;

    lun_info = kzalloc(sizeof(struct dev_upgrade_lun_info) * UPGRADE_LUN_NUM_MAX, GFP_KERNEL | __GFP_ACCOUNT);
    if (IS_ERR_OR_NULL(lun_info)) {
        dev_upgrade_err("lun info alloc fail\n");
        return -EINVAL;
    }

    for (i = 0; i < UPGRADE_LUN_NUM_MAX; i++) {
        lun_info_proc = lun_info + i;
        lun_info_proc->lun = i;
        lun_info_proc->file_cnt = 0;
        lun_len = dev_upgrade_get_lun_size(op_flag, i);
        if (lun_len <= 0) {
            dev_upgrade_err("lun len get fail, %lld\n", lun_len);
            ret = -EINVAL;
            goto free_lun;
        }
        lun_info_proc->lun_len = (u64)lun_len;
    }

    ret = dev_upgrade_get_lun_addr(sub_file, lun_info);
    if (ret != 0) {
        dev_upgrade_err("get lun addr fail, ret %d", ret);
        ret = -EINVAL;
        goto free_lun;
    }

    for (i = 0; i < UPGRADE_LUN_NUM_MAX; i++) {
        lun_info_proc = lun_info + i;
        if (lun_info_proc->file_cnt == 0) {
            continue;
        }
        ret = dev_upgrade_chk_lun_size(lun_info_proc);
        if (ret != 0) {
            dev_upgrade_err("lun %u len %llu file num %u check fail, ret %d\n",
                i,
                lun_info_proc->lun_len,
                lun_info_proc->file_cnt,
                ret);
            ret = -EINVAL;
            goto free_lun;
        }
        dev_upgrade_info(
            "lun %u len %llu file num %u check success\n", i, lun_info_proc->lun_len, lun_info_proc->file_cnt);
    }

free_lun:
    DEV_UPGRADE_KFREE(lun_info);
    return ret;
}

int dev_upgrade_get_cmdline(unsigned int *data, int *pxe_or_normal_scenes)
{
    char tmp[UPGRADE_BUFFER_SIZE + 1] = {0};
    struct file *fp = NULL;
    char *p = NULL;
    loff_t pos = 0;
    int len;

    *pxe_or_normal_scenes = PXE_UPGRADE_SCENES;
    fp = filp_open(CMDLINE_FILE_PATH, O_RDONLY, UPGRADE_DEVICE_AUTHORITY);
    if (IS_ERR_OR_NULL((void const *)fp)) {
        dev_upgrade_err("file %s not exist.\n", CMDLINE_FILE_PATH);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
    len = kernel_read(fp, &tmp, UPGRADE_BUFFER_SIZE, &pos);
#else
    len = kernel_read(fp, &pos, &tmp, UPGRADE_BUFFER_SIZE);
#endif
    if ((len <= 0) || (len >= UPGRADE_BUFFER_SIZE)) { /* len including '\0' */
        dev_upgrade_err("read fail, len %d\n", len);
        filp_close(fp, NULL);
        fp = NULL;
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    tmp[len] = '\0';

    if ((p = strstr(tmp, CMDLINE_PREFIX)) == NULL) {
        dev_upgrade_info("first install get CMDLINE_FILE success.\n");
        filp_close(fp, NULL);
        fp = NULL;
        *data = UPGRADE_RUNIMGLOCATION_DEFAULT;
        return OK;
    }

    p += strlen(CMDLINE_PREFIX);
    *data = (unsigned int)simple_strtol(p, NULL, UPGRADE_CMDLINE_HEX_MODE);
    filp_close(fp, NULL);
    fp = NULL;
    *pxe_or_normal_scenes = NORMAL_UPGRADE_SCENES;
    return OK;
}

int dev_upgrade_update_cmdline_state(void)
{
    int ret;
    unsigned int cmdline = 0;
    int pxe_or_normal_scenes = NORMAL_UPGRADE_SCENES;

    ret = dev_upgrade_get_cmdline(&cmdline, &pxe_or_normal_scenes);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_get_cmdline fail, ret: %d\n", ret);
        return ret;
    }

    g_pxe_or_normal_scenes = pxe_or_normal_scenes;

    return OK;
}

int dev_upgrade_chk_if_pxe_scenes(void)
{
    if (g_pxe_or_normal_scenes == PXE_UPGRADE_SCENES) {
        return OK;
    }

    return -EINVAL;
}

void dev_upgrade_update_cur_flag(int dev_id, DSMI_COMPONENT_TYPE component_type, unsigned int *cur_boot_area,
    unsigned int *actual_boot_area, int op_flag)
{
    if (dev_upgrade_chk_if_pxe_scenes() == OK) {
        if (op_flag == DISK_OP_TYPE_SYNC) {
            *cur_boot_area = UFS_MASTER_AREA;
            *actual_boot_area = UFS_MASTER_AREA;
        } else {
            *cur_boot_area = UFS_BACKUP_AREA;
            *actual_boot_area = UFS_BACKUP_AREA;
        }
        dev_upgrade_info("dev(%d) component(%u) pxe_scenes op_flag(%u) update cur_flag(%u) success.\n",
            dev_id,
            component_type,
            op_flag,
            *cur_boot_area);
    }

    return;
}

int dev_upgrade_boot_area_chk(int dev_id, DSMI_COMPONENT_TYPE component_type, int op_flag)
{
    int ret;
    unsigned int cmdline = 0;
    unsigned int actual_boot_area = UFS_MASTER_AREA;
    upgrade_ufs_component_info *local_component = NULL;

    /* firstly check ufs boot area, then check ufs write para */
    local_component = dev_upgrade_ufs_get_local_component(component_type);
    if (local_component == NULL) {
        dev_upgrade_err("dev(%d) component not found, component: %u\n", dev_id, component_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    ret = sec_flash_read_cmdline(dev_id, &cmdline);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) sec_flash_read_cmdline fail, ret=%d.\n", dev_id, ret);
        return ret;
    }

    dev_upgrade_info("dev(%d) cmdline val is 0x%x.\n", dev_id, cmdline);
    switch (component_type) {
        case DSMI_COMPONENT_TYPE_RAWDATA:
        case DSMI_COMPONENT_TYPE_SYSDRV:
        case DSMI_COMPONENT_TYPE_ADSAPP:
        case DSMI_COMPONENT_TYPE_COMISOLATOR:
        case DSMI_COMPONENT_TYPE_CLUSTER:
        case DSMI_COMPONENT_TYPE_CUSTOMIZED:
            if ((cmdline & UFS_RAWDATA_CUR_AREA_MASK) == UFS_RAWDATA_CUR_AREA_MASK) {
                actual_boot_area = UFS_BACKUP_AREA;
            }
            dev_upgrade_update_cur_flag(
                dev_id, component_type, &(local_component->cur_flag), &actual_boot_area, op_flag);

            break;
        case DSMI_COMPONENT_TYPE_RECOVERY:
            if (op_flag == DISK_OP_TYPE_SYNC) {
                local_component->cur_flag = UFS_MASTER_AREA;
                actual_boot_area = UFS_MASTER_AREA;
            } else {
                actual_boot_area = UFS_BACKUP_AREA;
            }
            break;
        default:
            break;
    }

    if (local_component->cur_flag != actual_boot_area) {
        dev_upgrade_err("dev(%d) boot area is wrong, stop to write ufs, cur_flag=%u, actual boot area is %u.\n",
            dev_id,
            local_component->cur_flag,
            actual_boot_area);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    return OK;
}

int dev_upgrade_component_para_chk(
    int dev_id, DSMI_COMPONENT_TYPE component_type, upgrade_sub_file *sub_file, int op_flag)
{
    int ret;

    ret = dev_upgrade_boot_area_chk(dev_id, component_type, op_flag);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) component_type(%d) dev_upgrade_boot_area_chk err, op_flag(%u).\n",
            dev_id,
            component_type,
            op_flag);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    ret = dev_upgrade_component_ufs_para_check(sub_file, op_flag);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) component_type(%d) dev_upgrade_component_ufs_para_check err, op_flag(%u) ret: %d.\n",
            dev_id,
            component_type,
            op_flag,
            ret);
        return ret;
    }

    if (op_flag == DISK_OP_TYPE_UPDATE) {
        ret = dev_upgrade_ufs_update_file_size_check(sub_file);
        if (ret != 0) {
            dev_upgrade_err("dev(%d) component_type(%d) dev_upgrade_ufs_update_file_size_check fail, ret=%d.\n",
                dev_id,
                component_type,
                ret);
            return ret;
        }
    }

    return ret;
}

int dev_upgrade_ufs_component_core_info_init(void)
{
    unsigned int i;
    int ret;
    for (i = 0; i < UFS_COMPONENT_NUM; i++) {
        g_upgrade_ufs_local_component_core_info[i] =
            (upgrade_ufs_component_info *)vzalloc(sizeof(upgrade_ufs_component_info));
        if (g_upgrade_ufs_local_component_core_info[i] == NULL) {
            dev_upgrade_err(
                "g_upgrade_ufs_local_component_core_info vzalloc fail, component_type = %d.\n", g_component_type[i]);
            dev_upgrade_ufs_component_core_info_uninit();
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
        }

        g_upgrade_ufs_local_component_core_info[i]->component_type = g_component_type[i];
        ret = memcpy_s(g_upgrade_ufs_local_component_core_info[i]->component_name,
            MAX_NAME_LEN,
            g_component_name[i],
            strlen(g_component_name[i]) + 1);
        if (ret != 0) {
            dev_upgrade_err(
                "g_upgrade_ufs_local_component_name memcpy_s fail, component_type = %d.\n", g_component_type[i]);
            dev_upgrade_ufs_component_core_info_uninit();
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
        }
    }

    return 0;
}

void dev_upgrade_ufs_component_core_info_uninit(void)
{
    int i;

    dev_upgrade_local_components_release();

    for (i = 0; i < UFS_COMPONENT_NUM; i++) {
        if (g_upgrade_ufs_local_component_core_info[i] != NULL) {
            DEV_UPGRADE_VFREE(g_upgrade_ufs_local_component_core_info[i]);
            g_upgrade_ufs_local_component_core_info[i] = NULL;
        }
    }
}

upgrade_ufs_component_info *dev_upgrade_ufs_get_local_component(unsigned int component_type)
{
    upgrade_ufs_component_info *component_info = NULL;
    unsigned int idx;

    for (idx = 0; idx < UFS_COMPONENT_NUM; idx++) {
        component_info = g_upgrade_ufs_local_component_core_info[idx];
        if (component_type == component_info->component_type) {
            return component_info;
        }
    }
    return NULL;
}

STATIC bool dev_upgrade_check_mdc_package_length_valid(mdc_package_t *pkt)
{
    int name_len = strnlen(pkt->name, MAX_NAME_LEN);
    int file_name_len = strnlen(pkt->file_name, SUB_FILE_MAX_PATH_LEN);
    if (name_len >= MAX_NAME_LEN || file_name_len >= SUB_FILE_MAX_PATH_LEN) {
        return false;
    }

    return true;
}

STATIC int dev_upgrade_sub_file_fill(struct upgrade_component *component, upgrade_sub_file *sub_pkt_src)
{
    unsigned int idx;
    unsigned int cnt = sub_pkt_src->sub_file_count;
    mdc_package_t *src_pkt = NULL;
    mdc_package_t *dst_pkt = NULL;

    if (cnt > MAX_MDC_PKT_NUM) {
        dev_upgrade_err(
            "dev(%d) component(0x%x) sub_file(%d) over limit!\n", component->dev_id, component->component_type, cnt);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }
    component->sub_pkt.sub_file_count = cnt;

    for (idx = 0; idx < cnt; idx++) {
        src_pkt = sub_pkt_src->sub_file_info[idx];
        dst_pkt = (mdc_package_t *)(void *)vzalloc(sizeof(mdc_package_t));
        if (dst_pkt == NULL) {
            dev_upgrade_err("dev(%d) component(0x%x) sub_file(%d) vzalloc failed!\n",
                component->dev_id,
                component->component_type,
                idx);
            break;
        }

        if (copy_from_user((void *)dst_pkt, (void *)src_pkt, sizeof(mdc_package_t))) {
            dev_upgrade_err("dev(%d) component(0x%x) sub_file(%d) copy_from_user failed\n",
                component->dev_id,
                component->component_type,
                idx);
            DEV_UPGRADE_VFREE(dst_pkt);
            break;
        }

        if (!dev_upgrade_check_mdc_package_length_valid(dst_pkt)) {
            dev_upgrade_err("dev(%d) component(0x%x) sub_file(%d) name length is invalid\n",
                component->dev_id,
                component->component_type,
                idx);
            DEV_UPGRADE_VFREE(dst_pkt);
            break;
        }

        component->sub_pkt.sub_file_info[idx] = dst_pkt;
        component->file_size += dst_pkt->storage.len;
    }

    if (idx < cnt) {
        for (idx = 0; idx < cnt; idx++) {
            dst_pkt = NULL;
            dst_pkt = component->sub_pkt.sub_file_info[idx];
            if (dst_pkt != NULL) {
                DEV_UPGRADE_VFREE(dst_pkt);
            }
        }
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    return OK;
}

STATIC int dev_upgrade_single_file_copy(upgrade_sub_file *dst, const upgrade_sub_file *src, unsigned int idx)
{
    mdc_package_t *src_pkt = NULL;
    mdc_package_t *dst_pkt = NULL;

    src_pkt = src->sub_file_info[idx];
    if (src_pkt == NULL) {
        dev_upgrade_err("sub_file(%d) src_pkt NULL!\n", idx);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    dst_pkt = (mdc_package_t *)(void *)vzalloc(sizeof(mdc_package_t));
    if (dst_pkt == NULL) {
        dev_upgrade_err("sub_file(%d) vzalloc failed!\n", idx);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    if (copy_from_user((void *)dst_pkt, (void *)src_pkt, sizeof(mdc_package_t))) {
        dev_upgrade_err("sub_file(%d) copy_from_user failed\n", idx);
        DEV_UPGRADE_VFREE(dst_pkt);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_TO_USER);
    }
    dst->sub_file_info[idx] = dst_pkt;

    return OK;
}

STATIC void dev_upgrade_single_area_release(upgrade_sub_file *sub_file)
{
    unsigned int idx;

    for (idx = 0; idx < sub_file->sub_file_count; idx++) {
        if (sub_file->sub_file_info[idx] != NULL) {
            DEV_UPGRADE_VFREE(sub_file->sub_file_info[idx]);
        }
    }

    sub_file->sub_file_count = 0;
}

STATIC int dev_upgrade_single_area_copy(upgrade_sub_file *dst, const upgrade_sub_file *src)
{
    unsigned int ret;
    unsigned int idx;
    unsigned int cnt = src->sub_file_count;

    if (cnt > MAX_MDC_PKT_NUM) {
        dev_upgrade_err("sub_file cnt(%d) over limit!\n", cnt);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    dst->sub_file_count = cnt;

    for (idx = 0; idx < cnt; idx++) {
        ret = dev_upgrade_single_file_copy(dst, src, idx);
        if (ret != OK) {
            dev_upgrade_err("sub_file(%d) dev_upgrade_single_file_copy failed!\n", idx);
            break;
        }
    }

    if (idx < cnt) {
        dev_upgrade_single_area_release(dst);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_COPY_FROM_USER);
    }

    return OK;
}

STATIC int dev_upgrade_sub_file_copy(upgrade_ufs_component_info *dst, const upgrade_ufs_single_area_info *src)
{
    unsigned int ret;

    dst->valid = src->valid;
    dst->area_flag = src->area_flag;
    dst->cur_flag = src->cur_flag;

    if (src->master_flag == UFS_MASTER_AREA) {
        dev_upgrade_single_area_release(&dst->sub_pkt_m);
        ret = dev_upgrade_single_area_copy(&dst->sub_pkt_m, &src->sub_pkt);
    } else {
        dev_upgrade_single_area_release(&dst->sub_pkt_b);
        ret = dev_upgrade_single_area_copy(&dst->sub_pkt_b, &src->sub_pkt);
    }

    if (ret != OK) {
        dev_upgrade_err("area(%d) dev_upgrade_single_area_copy failed!\n", src->master_flag);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    return OK;
}

STATIC void dev_upgrade_components_free_all(dev_upgrade_core_ctrl *upgrade_ctrl, unsigned int len)
{
    unsigned int sub_num;
    unsigned int i, j;

    for (i = 0; i < len; i++) {
        if (upgrade_ctrl->component_list[i] == NULL) {
            continue;
        }
        if (upgrade_ctrl->component_list[i]->file_content != NULL) {
            DEV_UPGRADE_VFREE(upgrade_ctrl->component_list[i]->file_content);
        }

        sub_num = upgrade_ctrl->component_list[i]->sub_pkt.sub_file_count;
        for (j = 0; j < sub_num; j++) {
            if (upgrade_ctrl->component_list[i]->sub_pkt.sub_file_info[j] != NULL) {
                DEV_UPGRADE_VFREE(upgrade_ctrl->component_list[i]->sub_pkt.sub_file_info[j]);
            }
        }

        DEV_UPGRADE_VFREE(upgrade_ctrl->component_list[i]);
    }
}

void dev_upgrade_local_components_release(void)
{
    unsigned int i;
    upgrade_ufs_component_info *local_component = NULL;

    for (i = DSMI_COMPONENT_TYPE_RAWDATA; i < DSMI_COMPONENT_TYPE_MAX; i++) {
        if (i > DSMI_COMPONENT_TYPE_CUSTOMIZED && i != DSMI_COMPONENT_TYPE_RECOVERY) {
            continue;
        }

        local_component = dev_upgrade_ufs_get_local_component(i);
        if (local_component == NULL) {
            dev_upgrade_err("find component %u fail.\n", i);
            continue;
        }

        local_component->valid = 0;
        dev_upgrade_single_area_release(&local_component->sub_pkt_m);
        dev_upgrade_single_area_release(&local_component->sub_pkt_b);
    }
}

STATIC int dev_upgrade_mdc_check_component_size(int dev_id, unsigned int component_type, unsigned int file_size)
{
    int ret;
    unsigned int alloc_size = 0;

    ret = dev_upgrade_component_space_get(dev_id, component_type, &alloc_size);
    if (ret != OK) {
        dev_upgrade_err("dev(%u) dev_upgrade_component_space_get fail, ret=%d\n", dev_id, ret);
        return ret;
    }

    if (file_size == 0 || file_size > alloc_size) {
        dev_upgrade_err("dev(%d) component size[0x%x] is invalid. type=%u\n", dev_id, file_size, component_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    return OK;
}

STATIC int dev_upgrade_mdc_check_sub_file(upgrade_sub_file *sub_file)
{
    int file_index;

    if (sub_file->sub_file_count > MAX_MDC_PKT_NUM) {
        dev_upgrade_err("sub_file_count[%u] is invalid.\n", sub_file->sub_file_count);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    for (file_index = 0; file_index < sub_file->sub_file_count; file_index++) {
        if (sub_file->sub_file_info[file_index] == NULL) {
            dev_upgrade_err("sub_file[%u] is null.\n", file_index);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
        }
    }

    return OK;
}

int dev_upgrade_mdc_check_update_start_in_para(struct upgrade_start_in *start_in)
{
    unsigned int i, ret;
    unsigned int dev_id = (unsigned int)start_in->dev_id;

    if (dev_id >= DEVICE_NUM_MAX) {
        dev_upgrade_err("dev(%u) invalid\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if ((start_in->count == 0) || (start_in->count > DSMI_COMPONENT_TYPE_MAX)) {
        dev_upgrade_err("dev(%d) component count[%u] invalid\n", start_in->dev_id, start_in->count);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if ((start_in->system_type != SYSTEM_TYPE_MULTI) && (start_in->system_type != SYSTEM_TYPE_SINGLE)) {
        dev_upgrade_err("dev(%d) system type[%u] invalid.\n", start_in->dev_id, start_in->system_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    for (i = 0; i < start_in->count; i++) {
        if (start_in->file_list[i].component_type >= DSMI_COMPONENT_TYPE_MAX) {
            dev_upgrade_err("dev(%d) file_list[%u] component type[%u] invalid\n",
                start_in->dev_id,
                i,
                start_in->file_list[i].component_type);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
        }

        /* upgrade flash image, use file content is pass from user space */
        if (dev_upgrade_ufs_valid_chk(start_in->file_list[i].component_type) != OK) {
            if (start_in->file_list[i].file_content == NULL) {
                dev_upgrade_err("dev(%d) file_list[%u] file content is null.\n", start_in->dev_id, i);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
            }

            ret = dev_upgrade_mdc_check_component_size(
                dev_id, start_in->file_list[i].component_type, start_in->file_list[i].file_size);
            if (ret != OK) {
                dev_upgrade_err("dev(%d) component size[0x%x] is invalid. type=%u\n",
                    start_in->dev_id,
                    start_in->file_list[i].file_size,
                    start_in->file_list[i].component_type);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
            }

            continue;
        }

        ret = dev_upgrade_mdc_check_sub_file(&start_in->file_list[i].sub_pkt);
        if (ret != OK) {
            dev_upgrade_err("dev(%d) dev_upgrade_mdc_check_sub_file fail, ret=%d.\n", start_in->dev_id, ret);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
        }
    }

    return OK;
}

int dev_upgrade_mdc_check_update_proc_para(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int ret;
    unsigned int i;
    unsigned int dev_id = (unsigned int)upgrade_ctrl->dev_id;
    upgrade_ufs_component_info *local_component = NULL;

    if ((dev_id >= DEVICE_NUM_MAX) || (upgrade_ctrl->total_component == 0) ||
        (upgrade_ctrl->total_component > DSMI_COMPONENT_TYPE_MAX)) {
        dev_upgrade_err("dev(%u) or component_count[%u] invalid\n", dev_id, upgrade_ctrl->total_component);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if ((upgrade_ctrl->system_type != SYSTEM_TYPE_MULTI) && (upgrade_ctrl->system_type != SYSTEM_TYPE_SINGLE)) {
        dev_upgrade_err("dev(%d) system type[%u] invalid.\n", upgrade_ctrl->dev_id, upgrade_ctrl->system_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    for (i = 0; i < upgrade_ctrl->total_component; i++) {
        if (upgrade_ctrl->component_list[i] == NULL) {
            dev_upgrade_err("dev(%d) component list is null.\n", upgrade_ctrl->dev_id);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
        }

        if (upgrade_ctrl->component_list[i]->component_type >= DSMI_COMPONENT_TYPE_MAX) {
            dev_upgrade_err("dev(%d) component_list[%u] component type[%u] invalid\n",
                upgrade_ctrl->dev_id,
                i,
                upgrade_ctrl->component_list[i]->component_type);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
        }

        /* Recovery image is fixed to upgrade the primary area. During synchronization,
         * data is synchronized from the primary area to the secondary area.
         */
        if (upgrade_ctrl->component_list[i]->component_type == DSMI_COMPONENT_TYPE_RECOVERY) {
            local_component = dev_upgrade_ufs_get_local_component(DSMI_COMPONENT_TYPE_RECOVERY);
            if (local_component == NULL) {
                dev_upgrade_err("dev(%d) component_list[%u] local_component invalid\n", upgrade_ctrl->dev_id, i);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
            }
            local_component->cur_flag = UFS_BACKUP_AREA;
        }

        /* upgrade flash image, use file content is pass from user space */
        if (dev_upgrade_ufs_valid_chk(upgrade_ctrl->component_list[i]->component_type) != OK) {
            if (upgrade_ctrl->component_list[i]->file_content == NULL) {
                dev_upgrade_err("dev(%d) component_list[%u] file content is null.\n", upgrade_ctrl->dev_id, i);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
            }

            ret = dev_upgrade_mdc_check_component_size(upgrade_ctrl->dev_id,
                upgrade_ctrl->component_list[i]->component_type,
                upgrade_ctrl->component_list[i]->file_size);
            if (ret != OK) {
                dev_upgrade_err("dev(%d) component size is invalid. type=%u\n",
                    upgrade_ctrl->dev_id,
                    upgrade_ctrl->component_list[i]->component_type);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
            }
            continue;
        }

        ret = dev_upgrade_mdc_check_sub_file(&upgrade_ctrl->component_list[i]->sub_pkt);
        if (ret != OK) {
            dev_upgrade_err("dev(%d) dev_upgrade_mdc_check_sub_file fail, ret=%d.\n", upgrade_ctrl->dev_id, ret);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
        }
    }

    return OK;
}

int dev_upgrade_mdc_check_sync_start_in_para(struct upgrade_ufs_local_component_start_in *start_in)
{
    unsigned int ret, i;

    if ((start_in->dev_id >= DEVICE_NUM_MAX) || (start_in->dev_id < 0)) {
        dev_upgrade_err("dev(%d) invalid\n", start_in->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if ((start_in->component_cnt == 0) || (start_in->component_cnt > DSMI_COMPONENT_TYPE_MAX)) {
        dev_upgrade_err("dev(%d) component count[%u] invalid\n", start_in->dev_id, start_in->component_cnt);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    for (i = 0; i < start_in->component_cnt; i++) {
        if (start_in->component_info[i].component_type >= DSMI_COMPONENT_TYPE_MAX) {
            dev_upgrade_err("dev(%d) file_list[%u] component type[%u] invalid\n",
                start_in->dev_id,
                i,
                start_in->component_info[i].component_type);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
        }

        ret = dev_upgrade_mdc_check_sub_file(&start_in->component_info[i].sub_pkt);
        if (ret != OK) {
            dev_upgrade_err("dev(%d) dev_upgrade_mdc_check_sub_file fail, ret=%d.\n", start_in->dev_id, ret);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
        }
    }

    return OK;
}

int dev_upgrade_mdc_check_sync_proc_para(upgrade_ufs_component_info *local_component)
{
    int ret;

    if ((local_component->system_type != SYSTEM_TYPE_MULTI) && (local_component->system_type != SYSTEM_TYPE_SINGLE)) {
        dev_upgrade_err("system type[%u] invalid.\n", local_component->system_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if (local_component->component_type >= DSMI_COMPONENT_TYPE_MAX) {
        dev_upgrade_err("component type[%u] invalid\n", local_component->component_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    ret = dev_upgrade_mdc_check_sub_file(&local_component->sub_pkt_m);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_mdc_check_sub_file fail, ret=%d.\n", ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    ret = dev_upgrade_mdc_check_sub_file(&local_component->sub_pkt_b);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_mdc_check_sub_file fail, ret=%d.\n", ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    return OK;
}

int dev_upgrade_components_fill(dev_upgrade_core_ctrl *upgrade_ctrl, struct upgrade_start_in *start_in)
{
    int ret;
    unsigned int i, type;
    struct upgrade_component *component = NULL;

    ret = dev_upgrade_mdc_check_update_start_in_para(start_in);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_mdc_check_upgrade_data fail, ret=%d.\n", upgrade_ctrl->dev_id, ret);
        return ret;
    }

    upgrade_ctrl->total_size = 0;
    upgrade_ctrl->system_type = start_in->system_type;

    for (i = 0; i < start_in->count; i++) {
        type = start_in->file_list[i].component_type;

        component = (struct upgrade_component *)vzalloc(sizeof(struct upgrade_component));
        if (component == NULL) {
            dev_upgrade_err("dev(%d) malloc fail\n", upgrade_ctrl->dev_id);
            break;
        }

        if (dev_upgrade_ufs_valid_chk(type) == OK) {
            component->file_size = 0;
            if (dev_upgrade_sub_file_fill(component, &(start_in->file_list[i].sub_pkt)) != OK) {
                DEV_UPGRADE_VFREE(component);
                break;
            }
        } else {
            component->file_size = start_in->file_list[i].file_size;
            component->file_content = (unsigned char *)vzalloc(component->file_size);
            if (component->file_content == NULL) {
                dev_upgrade_err("dev(%d) vzalloc failed!\n", upgrade_ctrl->dev_id);
                DEV_UPGRADE_VFREE(component);
                break;
            }

            if (copy_from_user((void *)component->file_content,
                (void *)start_in->file_list[i].file_content,
                component->file_size)) {
                dev_upgrade_err("dev(%d) file_content copy_from_user failed\n", upgrade_ctrl->dev_id);
                DEV_UPGRADE_VFREE(component->file_content);
                DEV_UPGRADE_VFREE(component);
                break;
            }
        }

        upgrade_ctrl->component_list[i] = component;
        component->dev_id = upgrade_ctrl->dev_id;
        component->component_type = type;
        component->weight = 1;
        component->total_areas = 1;
        upgrade_ctrl->total_size += (component->file_size * component->total_areas);
    }

    if (i != start_in->count) {
        dev_upgrade_components_free_all(upgrade_ctrl, i);
        dev_upgrade_err("dev(%d) rollback %u component.\n", upgrade_ctrl->dev_id, i);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    return 0;
}

int dev_upgrade_local_components_fill(const struct upgrade_ufs_local_component_start_in *start_in)
{
    unsigned int i, type;
    upgrade_ufs_component_info *local_component = NULL;

    for (i = 0; i < start_in->component_cnt; i++) {
        type = start_in->component_info[i].component_type;

        local_component = dev_upgrade_ufs_get_local_component(type);
        if (local_component == NULL) {
            dev_upgrade_err("dev(%d) find component fail, component: %u\n", start_in->dev_id, type);
            break;
        }

        if (dev_upgrade_sub_file_copy(local_component, &start_in->component_info[i]) != OK) {
            dev_upgrade_err("dev(%d) dev_upgrade_sub_file_copy fail, component: %u\n", start_in->dev_id, type);
            break;
        }
    }

    if (i != start_in->component_cnt) {
        dev_upgrade_local_components_release();
        dev_upgrade_err("dev(%d) rollback %u component.\n", start_in->dev_id, i);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    return 0;
}

void dev_upgrade_core_resource_release(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    unsigned int idx;

    for (idx = 0; idx < upgrade_ctrl->total_component; idx++) {
        if (upgrade_ctrl->component_list[idx] != NULL) {
            if (upgrade_ctrl->component_list[idx]->file_content != NULL) {
                DEV_UPGRADE_VFREE(upgrade_ctrl->component_list[idx]->file_content);
            }
            dev_upgrade_single_area_release(&upgrade_ctrl->component_list[idx]->sub_pkt);
            DEV_UPGRADE_VFREE(upgrade_ctrl->component_list[idx]);
        }
    }

    upgrade_ctrl->clr_localcfg = 0;
    upgrade_ctrl->total_component = 0;
    upgrade_ctrl->stop_flag = 0;

    return;
}

void dev_upgrade_fault_report(int dev_id, enum dev_upgrade_fault_state fault_state, enum dev_upgrade_error_no error_no,
    enum dev_upgrade_fault_id fault_id, enum dev_upgrade_fault_level level)
{
    int ret = 0;
    struct drvfault_msg_info msg_info = {0};

    dev_upgrade_info("upgrade fault report start.\n");

    msg_info.event_code.bit_s.error_num = error_no;
    msg_info.event_code.bit_s.id = fault_id;
    msg_info.event_code.bit_s.reserve = 0;
    msg_info.event_code.bit_s.event_type = EVENT_TYPE_SOFTWARE;
    msg_info.event_code.bit_s.level = level;
    msg_info.event_code.bit_s.direc = fault_state;

    dev_upgrade_info("upgrade event_code val 0x%x.\n", msg_info.event_code.val);

#ifndef CFG_SOC_PLATFORM_MDC_V11
    ret = drvfault_msg_send_report(dev_id, (void *)&msg_info);
#endif
    if (ret != 0) {
        dev_upgrade_err("drvfault_msg_send_report failed, ret = %d\n", ret);
    }

    dev_upgrade_info("upgrade fault report end.\n");
    return;
}

void dev_upgrade_check_fault_resume(int dev_id)
{
    unsigned int check_result;

    check_result = dev_upgrade_ufs_check_status_get(dev_id);
    if (check_result == IMAGE_CHK_OK) {
        return;
    }

    dev_upgrade_fault_report(dev_id,
        UPGRADE_FAULT_STATE_RESUME,
        UPGRADE_ERROR_NO_START_FAULT_EVENT,
        UPGRADE_FAULT_ID_UFS_CHK_ERR,
        UPGRADE_FAULT_LEVLE_MAJOR);

    dev_upgrade_info("dev(%d) event info(0x%x) resume success.\n", dev_id, UPGRADE_ERROR_NO_START_FAULT_EVENT);
    dev_upgrade_info("fault_id(0x%x) fault_state(%d) fault_level(%d).\n",
        UPGRADE_FAULT_ID_UFS_CHK_ERR,
        UPGRADE_FAULT_STATE_RESUME,
        UPGRADE_FAULT_LEVLE_MAJOR);
}

void dev_upgrade_check_fault_report(int dev_id)
{
    unsigned int check_result;

    check_result = dev_upgrade_ufs_check_status_get(dev_id);
    if (check_result == IMAGE_CHK_OK) {
        return;
    }

    dev_upgrade_fault_report(dev_id,
        UPGRADE_FAULT_STATE_OCCUR,
        UPGRADE_ERROR_NO_START_FAULT_EVENT,
        UPGRADE_FAULT_ID_UFS_CHK_ERR,
        UPGRADE_FAULT_LEVLE_MAJOR);

    dev_upgrade_info("dev(%d) event info(0x%x) report success.\n", dev_id, UPGRADE_ERROR_NO_START_FAULT_EVENT);
    dev_upgrade_info("fault_id(0x%x) fault_state(%d) fault_level(%d).\n",
        UPGRADE_FAULT_ID_UFS_CHK_ERR,
        UPGRADE_FAULT_STATE_OCCUR,
        UPGRADE_FAULT_LEVLE_MAJOR);
}

int dev_upgrade_sync_single_file(unsigned int cur_flag, mdc_package_t *sub_file_info)
{
    int ret;
    char *read_buff = NULL;
    int read_len;
    unsigned long long offset = 0;
    unsigned int src_lun, dst_lun;
    unsigned long long src_addr, dst_addr, len;

    read_buff = (char *)vmalloc(UFS_SYNC_SIZE + 1);
    if (read_buff == NULL) {
        dev_upgrade_err("alloc memory failed.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    src_addr = (cur_flag == UFS_MASTER_AREA) ? sub_file_info->storage.start : sub_file_info->storage.back;
    dst_addr = (cur_flag == UFS_MASTER_AREA) ? sub_file_info->storage.back : sub_file_info->storage.start;
    src_lun = (cur_flag == UFS_MASTER_AREA) ? sub_file_info->storage.lun : sub_file_info->storage.back_lun;
    dst_lun = (cur_flag == UFS_MASTER_AREA) ? sub_file_info->storage.back_lun : sub_file_info->storage.lun;
    len = sub_file_info->storage.len;
    dev_upgrade_info("Sync from src to dst. (src_lun=%u; src_addr=%llu; dst_lun=%u; dst_addr=%llu; size=%llu)\n",
        src_lun,
        src_addr,
        dst_lun,
        dst_addr,
        len);

    while (offset <= len) {
        ret = memset_s(read_buff, UFS_SYNC_SIZE, 0, UFS_SYNC_SIZE);
        if (ret != 0) {
            dev_upgrade_err("memset_s abnormal. (filename=%.*s)", MAX_NAME_LEN, sub_file_info->name);
            goto out;
        }

        read_len = ((offset + UFS_SYNC_SIZE) <= len) ? UFS_SYNC_SIZE : (len - offset);
        if (read_len == 0) {
            dev_upgrade_event("Read to the end of the file. (filename=%.*s)", MAX_NAME_LEN, sub_file_info->name);
            ret = OK;
            goto out;
        }

        ret = dev_upgrade_disk_read(QUERY_FROM_CMDLINE, (unsigned char)src_lun, src_addr + offset, read_buff, read_len);
        if (ret != OK) {
            dev_upgrade_err(
                "Failed to read. (lun=%d; base=0x%lld; offset=0x%lld; ret=%d)\n", src_lun, src_addr, offset, ret);
            goto out;
        }

        ret =
            dev_upgrade_disk_write(QUERY_FROM_CMDLINE, (unsigned char)dst_lun, dst_addr + offset, read_buff, read_len);
        if (ret != OK) {
            dev_upgrade_err(
                "Failed to write. (lun=%d; base=0x%lld; offset=0x%lld; ret=%d)\n", dst_lun, dst_addr, offset, ret);
            goto out;
        }

        offset += read_len;
    }

out:
    DEV_UPGRADE_VFREE(read_buff);
    return ret;
}

int dev_upgrade_sync_single_component(upgrade_ufs_component_info *local_component)
{
    int ret;
    unsigned int sub_file_idx;
    upgrade_sub_file *sub_pkt = NULL;

    sub_pkt = &local_component->sub_pkt_m;
    if (sub_pkt->sub_file_count > MAX_MDC_PKT_NUM) {
        dev_upgrade_err("component(0x%x) sub_file_count(%d) out of range(%d).\n",
            local_component->component_type,
            sub_pkt->sub_file_count,
            MAX_MDC_PKT_NUM);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    for (sub_file_idx = 0; sub_file_idx < sub_pkt->sub_file_count; sub_file_idx++) {
        if (sub_pkt->sub_file_info[sub_file_idx] == NULL) {
            continue;
        }
        ret = dev_upgrade_sync_single_file(local_component->cur_flag, sub_pkt->sub_file_info[sub_file_idx]);
        if (ret != OK) {
            dev_upgrade_err("component(0x%x) sub_file_idx(0x%x) sync fail, ret: %d.\n",
                local_component->component_type,
                sub_file_idx,
                ret);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
        }
    }

    return OK;
}

STATIC int dev_upgrade_sync_ufs_para_check(int dev_id)
{
    int ret;
    int i;
    upgrade_ufs_component_info *local_component = NULL;

    for (i = 0; i < DSMI_COMPONENT_TYPE_MAX; i++) {
        if (dev_upgrade_ufs_valid_chk(i) == OK) {
            local_component = dev_upgrade_ufs_get_local_component(i);
            if (local_component == NULL) {
                dev_upgrade_warn("dev(%d) can't find component, component: %u\n", dev_id, i);
                continue;
            }

            if ((local_component->valid == UFS_INVALID_AREA) || (local_component->area_flag == UFS_SINGLE_AREA)) {
                continue;
            }

            /* In the recovery scenario, the master area is upgraded during installation or upgrade.
             * During synchronization, the master area is synchronized to the slave area.
             */
            if (local_component->component_type == DSMI_COMPONENT_TYPE_RECOVERY) {
                dev_upgrade_info("recovery component no need to sync with ufs images\n");
                continue;
            }

            ret = dev_upgrade_mdc_check_sync_proc_para(local_component);
            if (ret != 0) {
                dev_upgrade_err("dev_upgrade_mdc_check_sync_proc_para fail, ret = %d, index = %u\n", ret, i);
                return ret;
            }

            ret = dev_upgrade_component_para_chk(
                dev_id, (DSMI_COMPONENT_TYPE)i, &(local_component->sub_pkt_m), DISK_OP_TYPE_SYNC);
            if (ret != OK) {
                dev_upgrade_err("update para chk fail, ret = %d, index = %u\n", ret, i);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
            }
        }
    }

    return OK;
}

int dev_upgrade_sync_ufs_image(int dev_id)
{
    int i;
    int ret;
    upgrade_ufs_component_info *local_component = NULL;

    /* check para before write data to ufs */
    ret = dev_upgrade_sync_ufs_para_check(dev_id);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_sync_ufs_para_check fail, ret=%d.\n", ret);
        return ret;
    }

    for (i = 0; i < DSMI_COMPONENT_TYPE_MAX; i++) {
        if (dev_upgrade_ufs_valid_chk(i) == OK) {
            local_component = dev_upgrade_ufs_get_local_component(i);
            if (local_component == NULL) {
                dev_upgrade_warn("dev(%d) can't find component, component: %u\n", dev_id, i);
                continue;
            }

            /* Recovery image are upgraded and synchronized separately */
            if (local_component->component_type == DSMI_COMPONENT_TYPE_RECOVERY) {
                dev_upgrade_info("dev(%d) recovery component is no need to sync\n", dev_id);
                continue;
            }

            if ((local_component->valid == UFS_INVALID_AREA) || (local_component->area_flag == UFS_SINGLE_AREA)) {
                continue;
            }

            ret = dev_upgrade_sync_single_component(local_component);
            if (ret != OK) {
                dev_upgrade_err("component(0x%x) dev_upgrade_sync_single_component fail, ret: %d\n", i, ret);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
            }
        }
    }

    dev_upgrade_check_fault_resume(dev_id);
    dev_upgrade_ufs_check_status_set(dev_id, IMAGE_CHK_OK);

    ret = dev_upgrade_sync_mdc_system_type();
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_sync_mdc_system_type fail, ret=%d.\n", ret);
        return ret;
    }

    /* update ufs upgrade condition, sync end status */
    ret = dev_upgrade_set_mdc_flag(
        QUERY_FROM_CMDLINE, UFS_UPGRADE_FLAG_LUN, UFS_FLAG_BASE_OFFSET, UFS_COND_FLAG_OFFSET, UFS_SYNC_END_FLAG);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_set_mdc_flag fail, ret: %d\n", ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    dev_upgrade_info("dev_upgrade_sync_ufs_image success.\n");

    return OK;
}

STATIC bool dev_upgrade_is_in_recovery_system(void);
int dev_upgrade_ufs_single_component_check(int dev_id, upgrade_ufs_component_info *local_component)
{
    int ret;
    unsigned int start_area;
    unsigned int start_cnt = 0;

    ret = sec_read_ufs_resetcnt((unsigned int)dev_id, &start_cnt);
    if (ret != OK) {
        dev_upgrade_err("dev(%d) sec_read_ufs_resetcnt fail, ret: %d\n", dev_id, ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    start_area = ((start_cnt % UFS_START_CNT_MASK) < UFS_MASTER_START_MAX) ? UFS_MASTER_AREA : UFS_BACKUP_AREA;

    if (dev_upgrade_is_in_recovery_system()) {
        // cur_flag的值来自于用户态解析cmdline的最小4位，而进入recovery后，cmdline不再由emmc计数，而是由recovery计数决定。
        // 而start_area始终由emmc计数决定，这样就会产生不一致的现象，打印1427行的错误日志。因此针对recovery的情况，直接返回OK。
        local_component->area_check = IMAGE_CHK_OK;
        return OK;
    }

    if (local_component->cur_flag != start_area) {
        dev_upgrade_err("dev(%d) component(0x%x) ufs image check err, cur_area(%d) start_area(%d).\n",
            dev_id,
            local_component->component_type,
            local_component->cur_flag,
            start_area);
        local_component->area_check = IMAGE_CHK_ERR;
        return OK;
    }

    dev_upgrade_info("dev(%d) component(0x%x) ufs image check ok, cur_area(%d) start_area(%d).\n",
        dev_id,
        local_component->component_type,
        local_component->cur_flag,
        start_area);
    local_component->area_check = IMAGE_CHK_OK;

    return OK;
}

int dev_upgrade_ufs_image_check(int dev_id)
{
    int i;
    int ret;
    upgrade_ufs_component_info *local_component = NULL;

    for (i = 0; i < DSMI_COMPONENT_TYPE_MAX; i++) {
        if (dev_upgrade_ufs_valid_chk(i) == OK) {
            local_component = dev_upgrade_ufs_get_local_component(i);
            if (local_component == NULL) {
                dev_upgrade_warn("dev(%d) find component fail, component: %u\n", dev_id, i);
                continue;
            }

            if ((local_component->valid == UFS_INVALID_AREA) || (local_component->area_flag == UFS_SINGLE_AREA)) {
                continue;
            }

            if (local_component->component_type == DSMI_COMPONENT_TYPE_RECOVERY) {
                dev_upgrade_info("recovery component is beed upgrade in master area and sync in slave area.\n");
                continue;
            }

            ret = dev_upgrade_ufs_single_component_check(dev_id, local_component);
            if (ret != OK) {
                dev_upgrade_err("dev(%d) ufs_single_component_check fail, ret: %d\n", dev_id, ret);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
            }
        }
    }

    dev_upgrade_info("dev_upgrade_ufs_image_check success.\n");

    return OK;
}

unsigned int dev_upgrade_ufs_check_status_get(int dev_id)
{
    int idx;
    unsigned int check_result = IMAGE_CHK_OK;
    upgrade_ufs_component_info *local_component = NULL;

    for (idx = 0; idx < DSMI_COMPONENT_TYPE_MAX; idx++) {
        if (dev_upgrade_ufs_valid_chk(idx) == OK) {
            local_component = dev_upgrade_ufs_get_local_component(idx);
            if (local_component == NULL) {
                dev_upgrade_warn("dev(%d) find component fail, component: %u\n", dev_id, idx);
                continue;
            }

            if (local_component->valid == UFS_INVALID_AREA) {
                continue;
            }

            check_result |= local_component->area_check;
        }
    }

    return check_result;
}

void dev_upgrade_ufs_check_status_set(int dev_id, unsigned int status)
{
    int idx;
    upgrade_ufs_component_info *local_component = NULL;

    for (idx = 0; idx < DSMI_COMPONENT_TYPE_MAX; idx++) {
        if (dev_upgrade_ufs_valid_chk(idx) == OK) {
            local_component = dev_upgrade_ufs_get_local_component(idx);
            if (local_component == NULL) {
                dev_upgrade_warn("dev(%d) find component fail, component: %u\n", dev_id, idx);
                continue;
            }

            if (local_component->valid == UFS_INVALID_AREA) {
                continue;
            }

            local_component->area_check = status;
        }
    }

    return;
}

int dev_upgrade_check_proc(struct upgrade_check_image_in *check_image_in)
{
    int ret;

    dev_upgrade_debug("dev(%d) dev_upgrade_check_proc start, flash_en(%d) ufs_en(%d).\n",
        check_image_in->dev_id,
        check_image_in->flash_chk,
        check_image_in->ufs_chk);

    if (check_image_in->flash_chk == IMAGE_CHK_ENABLE) {
        ret = sec_img_sync_and_efuse_update(check_image_in->dev_id);
        if (ret != 0) {
            dev_upgrade_err("dev(%d) sec_img_sync_and_efuse_update fail.\n", check_image_in->dev_id);
            return ret;
        }
    }

    if (check_image_in->ufs_chk == IMAGE_CHK_ENABLE) {
        ret = dev_upgrade_ufs_image_check(check_image_in->dev_id);
        if (ret != 0) {
            dev_upgrade_err("dev(%d) dev_upgrade_ufs_image_check fail.\n", check_image_in->dev_id);
            return ret;
        }
        dev_upgrade_check_fault_report(check_image_in->dev_id);
    }

    return OK;
}

int dev_upgrade_set_disk_update_flag(int mode, unsigned int lu_idx, unsigned int offset, const char *buff)
{
    unsigned int ret;

    ret = dev_upgrade_disk_write(mode, (unsigned char)lu_idx, offset, buff, UFS_BLOCK_SIZE);
    if (ret != OK) {
        dev_upgrade_err("lu_idx(0x%x) offset(0x%x) dev_upgrade_disk_write fail, ret: %d\n", lu_idx, offset, ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    ret = dev_upgrade_disk_write(
        mode, (unsigned char)lu_idx, offset + UFS_FLAG_BACKUP_OFFSET, (const char *)buff, UFS_BLOCK_SIZE);
    if (ret != OK) {
        dev_upgrade_err("lu_idx(0x%x) backup offset(0x%x) dev_upgrade_disk_write fail, ret: %d\n",
            lu_idx,
            offset + UFS_FLAG_BACKUP_OFFSET,
            ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    return OK;
}

int dev_upgrade_get_mdc_flag(
    int mode, unsigned int lun, unsigned int lun_base_addr, unsigned int offset, unsigned int *val)
{
    unsigned int ret;
    char *buff = NULL;

    buff = (char *)(void *)vzalloc(UFS_BLOCK_SIZE);
    if (buff == NULL) {
        dev_upgrade_err("dev_upgrade_get_mdc_flag malloc fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    ret = dev_upgrade_disk_read(mode, lun, lun_base_addr, buff, UFS_BLOCK_SIZE);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_disk_read LU(%d) lun_base_addr(0x%x) fail, ret: %d.\n", lun, lun_base_addr, ret);
        DEV_UPGRADE_VFREE(buff);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    *val = *(unsigned int *)(buff + offset);

    dev_upgrade_info("Get LU(%d) addr(0x%x) flag(0x%x) success.\n", lun, lun_base_addr + offset, *val);
    DEV_UPGRADE_VFREE(buff);

    return OK;
}

int dev_upgrade_set_mdc_flag(
    int mode, unsigned int lun, unsigned int lun_base_addr, unsigned int offset, unsigned int val)
{
    unsigned int ret;
    char *buff = NULL;

    buff = (char *)(void *)vzalloc(UFS_BLOCK_SIZE);
    if (buff == NULL) {
        dev_upgrade_err("dev_upgrade_mdc_update_flag_set malloc fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    ret = dev_upgrade_disk_read(mode, lun, lun_base_addr, buff, UFS_BLOCK_SIZE);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_disk_read LU(%d) lun_base_addr(0x%x) fail, ret: %d.\n", lun, lun_base_addr, ret);
        DEV_UPGRADE_VFREE(buff);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    *(unsigned int *)(buff + offset) = val;

    ret = dev_upgrade_set_disk_update_flag(mode, lun, lun_base_addr, (const char *)buff);
    if (ret != OK) {
        dev_upgrade_err("set LU(%d) addr(0x%x) flag(0x%x) fail, ret: 0x%x.\n", lun, lun_base_addr + offset, val, ret);
        DEV_UPGRADE_VFREE(buff);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    dev_upgrade_info("set LU(%d) addr(0x%x) flag(0x%x) success.\n", lun, lun_base_addr + offset, val);
    DEV_UPGRADE_VFREE(buff);
    return OK;
}

int dev_upgrade_mdc_update_flag_set(upgrade_ufs_component_info *local_component)
{
    unsigned int ret;
    unsigned int val;
    unsigned int area_flag;

    area_flag = (local_component->cur_flag == UFS_BACKUP_AREA) ? UFS_MASTER_NEW_FLAG : UFS_BACKUP_NEW_FLAG;

    /* update ufs upgrade condition, upgrade or sync status */
    ret = dev_upgrade_set_mdc_flag(
        QUERY_FROM_BOOTSTRAP, UFS_UPGRADE_FLAG_LUN, UFS_FLAG_BASE_OFFSET, UFS_COND_FLAG_OFFSET, UFS_UPGRADE_END_FLAG);
    if (ret != OK) {
        dev_upgrade_err("set LU(%d) addr(0x%x) cond_flag(0x%x) fail, ret: %d\n",
            UFS_UPGRADE_FLAG_LUN,
            UFS_FLAG_BASE_OFFSET + UFS_COND_FLAG_OFFSET,
            UFS_UPGRADE_END_FLAG,
            ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    /* update ufs upgrade area flag, update main or backup area */
    ret = dev_upgrade_set_mdc_flag(
        QUERY_FROM_BOOTSTRAP, UFS_UPGRADE_FLAG_LUN, UFS_FLAG_BASE_OFFSET, UFS_AREA_FLAG_OFFSET, area_flag);
    if (ret != OK) {
        dev_upgrade_err("set LU(%d) addr(0x%x) area_flag(0x%x) fail, ret: %d\n",
            UFS_UPGRADE_FLAG_LUN,
            UFS_FLAG_BASE_OFFSET + UFS_AREA_FLAG_OFFSET,
            area_flag,
            ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    /* update ufs upgrade status flag, waiting restart or normal */
    ret = dev_upgrade_set_mdc_flag(
        QUERY_FROM_BOOTSTRAP, UFS_UPGRADE_FLAG_LUN, UFS_FLAG_BASE_OFFSET, UFS_STATE_FLAG_OFFSET, UFS_WAIT_RST_FLAG);
    if (ret != OK) {
        dev_upgrade_err("set LU(%d) addr(0x%x) state_flag(0x%x) fail, ret: %d\n",
            UFS_UPGRADE_FLAG_LUN,
            UFS_FLAG_BASE_OFFSET + UFS_STATE_FLAG_OFFSET,
            UFS_WAIT_RST_FLAG,
            ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    /* update ufs upgrade reset cnt */
    val = (area_flag == UFS_BACKUP_NEW_FLAG) ? BACKUP_RESTART_CNT_VAL : RESTART_CNT_VAL;
    ret = sec_write_ufs_resetcnt(0, val);
    if (ret != OK) {
        dev_upgrade_err("sec_write_ufs_resetcnt(%u) fail, ret: %d\n", val, ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    dev_upgrade_info("component(0x%x) cur_area(%d) upgrade other area success.\n",
        local_component->component_type,
        local_component->cur_flag);
    dev_upgrade_info("key_flag(0x%x), end_flag(0x%x), restart cnt(%d),state_flag(0x%x).\n",
        area_flag,
        UFS_UPGRADE_END_FLAG,
        val,
        UFS_WAIT_RST_FLAG);
    return OK;
}

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
int sec_flash_write(unsigned int dev_id, unsigned int flash_offset, const unsigned char *buf, unsigned int buf_len)
{
    dev_upgrade_err("sec_flash_write error, esl don't support tee");
    return 0;
}
#endif

#ifndef CFG_SOC_PLATFORM_MDC_V11
STATIC int dev_upgrade_write_flash(int dev_id, unsigned int offset, const unsigned char *write_buf, unsigned int size)
{
    int ret;
    unsigned char *read_buf = NULL;

    if (size != FLASH_BLOCK_SIZE) {
        dev_upgrade_err("dev_id[%d] size[0x%x] is not valid.\n", dev_id, size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    read_buf = (unsigned char *)kzalloc(FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ACCOUNT);
    if (read_buf == NULL) {
        dev_upgrade_err("kzalloc for write data fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    ret = sec_flash_write(0, offset, (const unsigned char *)write_buf, FLASH_BLOCK_SIZE);
    if (ret != 0) {
        dev_upgrade_err("sec_flash_write fail, ret=%d.\n", ret);
        goto free_read_buf_exit;
    }

    ret = sec_flash_read(0, offset, (unsigned char *)read_buf, FLASH_BLOCK_SIZE);
    if (ret != 0) {
        dev_upgrade_err("sec_flash_read fail, ret=%d.\n", ret);
        goto free_read_buf_exit;
    }

    if (memcmp(write_buf, read_buf, FLASH_BLOCK_SIZE) != 0) {
        dev_upgrade_warn("read buf data is not equal to write buf data.\n");
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
    }

free_read_buf_exit:
    DEV_UPGRADE_KFREE(read_buf);
    return ret;
}
#endif

STATIC int dev_upgrade_write_system_type(unsigned int offset, unsigned int val)
{
#ifdef CFG_SOC_PLATFORM_MDC_V11
    return 0;
#else
    int ret;
    int cycle_time;
    unsigned int addr[2] = {MASTER_SYS_TYPE_BASE, BACKUP_SYS_TYPE_BASE}; /* 2 array for master and backup area */
    unsigned char *write_buf = NULL;

    write_buf = (unsigned char *)kzalloc(FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ACCOUNT);
    if (write_buf == NULL) {
        dev_upgrade_err("kzalloc for write data fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    /* write flash master and backup area */
    for (cycle_time = 0; cycle_time < 2; cycle_time++) { /* cycle 2 times for master and backup area */
        ret = sec_flash_read(0, addr[cycle_time], (unsigned char *)write_buf, FLASH_BLOCK_SIZE);
        if (ret != 0) {
            dev_upgrade_err("sec_flash_read fail, ret=%d.\n", ret);
            goto free_write_buf_exit;
        }

        if (*(unsigned int *)(write_buf + offset) != val) {
            *(unsigned int *)(write_buf + offset) = val;
            ret = dev_upgrade_write_flash(0, addr[cycle_time], write_buf, FLASH_BLOCK_SIZE);
            if (ret != 0) {
                dev_upgrade_err("dev_upgrade_write_flash fail, ret=%d.\n", ret);
                goto free_write_buf_exit;
            }
        }
    }

free_write_buf_exit:
    DEV_UPGRADE_KFREE(write_buf);
    return ret;
#endif
}

STATIC int dev_upgrade_set_mdc_system_type(unsigned int system_type)
{
    int ret;
    unsigned int write_offset;
    unsigned int write_tmp;
    upgrade_ufs_component_info *local_component = NULL;

    local_component = dev_upgrade_ufs_get_local_component(DSMI_COMPONENT_TYPE_RAWDATA);
    if (local_component == NULL) {
        dev_upgrade_err("find rawdata component fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (local_component->component_type == DSMI_COMPONENT_TYPE_RECOVERY) {
        dev_upgrade_info("recovery component needn't to set system type.\n");
        return OK;
    }

    /* get flash addr to write */
    if (local_component->cur_flag == UFS_MASTER_AREA) {
        write_offset = UFS_BACKUP_SYS_TYPE_OFFSET;
    } else {
        write_offset = UFS_MASTER_SYS_TYPE_OFFSET;
    }

    /* get write value */
    if (system_type == SYSTEM_TYPE_MULTI) {
        write_tmp = SYS_TYPE_FLAG_MULTI;
    } else if (system_type == SYSTEM_TYPE_SINGLE) {
        write_tmp = SYS_TYPE_FLAG_SINGLE;
    } else {
        dev_upgrade_err("system type[%u] invalid.\n", system_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    dev_upgrade_info("dev_upgrade_set_mdc_system_type[%u], val=0x%x.\n", system_type, write_tmp);
    ret = dev_upgrade_write_system_type(write_offset, write_tmp);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_write_system_type fail, ret=%d.\n", ret);
    }

    return ret;
}

STATIC int dev_upgrade_sync_get_current_sys_type(unsigned int offset, unsigned int *system_type)
{
#ifdef CFG_SOC_PLATFORM_MDC_V11
    *system_type = SYSTEM_TYPE_SINGLE;
    return 0;
#else
    int ret;
    unsigned int sync_val, master_flag, backup_flag;
    unsigned char *read_buf = NULL;

    read_buf = (unsigned char *)kzalloc(FLASH_BLOCK_SIZE, GFP_KERNEL | __GFP_ACCOUNT);
    if (read_buf == NULL) {
        dev_upgrade_err("kzalloc for write data fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    ret = sec_flash_read(0, MASTER_SYS_TYPE_BASE, (unsigned char *)read_buf, FLASH_BLOCK_SIZE);
    if (ret != 0) {
        dev_upgrade_err("sec_flash_read fail, ret=%d.\n", ret);
        goto free_read_buf_exit;
    }

    master_flag = *(unsigned int *)(read_buf + offset);
    ret = sec_flash_read(0, BACKUP_SYS_TYPE_BASE, (unsigned char *)read_buf, FLASH_BLOCK_SIZE);
    if (ret != 0) {
        dev_upgrade_err("sec_flash_read fail, ret=%d.\n", ret);
        goto free_read_buf_exit;
    }

    backup_flag = *(unsigned int *)(read_buf + offset);

    /*
     * 1. master flag is valid, use master flag to sync
     * 2. backup flag is valid, use backup flag to sync
     * 3. neither master nor backup flag is valid, sync with single os flag
     */
    if ((master_flag == SYS_TYPE_FLAG_MULTI) || (master_flag == SYS_TYPE_FLAG_SINGLE)) {
        sync_val = master_flag;
    } else if ((backup_flag == SYS_TYPE_FLAG_MULTI) || (backup_flag == SYS_TYPE_FLAG_SINGLE)) {
        sync_val = backup_flag;
    } else {
        dev_upgrade_warn("master flag[0x%x] and backup flag[0x%x] are all invalid.\n", master_flag, backup_flag);
        sync_val = SYS_TYPE_FLAG_SINGLE; /* can't read valid flag, set single os flag */
        master_flag = SYS_TYPE_FLAG_SINGLE;
    }

    if (sync_val == SYS_TYPE_FLAG_MULTI) {
        *system_type = SYSTEM_TYPE_MULTI;
    } else {
        *system_type = SYSTEM_TYPE_SINGLE;
    }

    /* either master flag or backup flag is invalid, sync it with the valid flag */
    if (master_flag != backup_flag) {
        dev_upgrade_warn("system type flag master[0x%x] and backup[0x%x] are different.\n", master_flag, backup_flag);
        ret = dev_upgrade_write_system_type(offset, sync_val);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_write_system_type fail, ret=%d.\n", ret);
        }
    }

free_read_buf_exit:
    DEV_UPGRADE_KFREE(read_buf);
    return ret;
#endif
}

STATIC int dev_upgrade_sync_mdc_system_type(void)
{
    int ret;
    unsigned int read_offset;
    unsigned int system_type = 0;
    upgrade_ufs_component_info *local_component = NULL;

    local_component = dev_upgrade_ufs_get_local_component(DSMI_COMPONENT_TYPE_RAWDATA);
    if (local_component == NULL) {
        dev_upgrade_err("find rawdata component fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (local_component->cur_flag == UFS_MASTER_AREA) {
        read_offset = UFS_MASTER_SYS_TYPE_OFFSET;
    } else {
        read_offset = UFS_BACKUP_SYS_TYPE_OFFSET;
    }

    ret = dev_upgrade_sync_get_current_sys_type(read_offset, &system_type);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_sync_get_current_sys_type fail, ret=%d.\n", ret);
        return ret;
    }

    if (system_type != SYSTEM_TYPE_INVALID) {
        ret = dev_upgrade_set_mdc_system_type(system_type);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_set_mdc_system_type fail, ret=%d.\n", ret);
        }
    }

    return ret;
}

int dev_upgrade_mdc_flag_update(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    unsigned int i;
    unsigned int ret;
    unsigned int type;
    unsigned int total_cnt;
    upgrade_ufs_component_info *local_component = NULL;

    total_cnt = upgrade_ctrl->total_component;

    for (i = 0; i < total_cnt; i++) {
        type = upgrade_ctrl->component_list[i]->component_type;
        if (type == DSMI_COMPONENT_TYPE_RECOVERY) {
            dev_upgrade_info("recovery image isn't need to set  upgrade flag. component: %u\n", type);
            continue;
        }

        if (dev_upgrade_ufs_valid_chk(type) == OK) {
            local_component = dev_upgrade_ufs_get_local_component(type);
            if (local_component == NULL) {
                dev_upgrade_err("dev(%d) find component fail, component: %u\n", upgrade_ctrl->dev_id, type);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
            }

            if (local_component->area_flag == UFS_SINGLE_AREA) {
                continue;
            }

            ret = dev_upgrade_mdc_update_flag_set(local_component);
            if (ret != OK) {
                dev_upgrade_err("component(0x%x) dev_upgrade_mdc_update_flag_set fail, ret: %d\n", type, ret);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
            }

            ret = dev_upgrade_set_mdc_system_type(upgrade_ctrl->system_type);
            if (ret != 0) {
                dev_upgrade_err("dev_upgrade_set_mdc_system_type fail, ret=%d.\n", ret);
                return ret;
            }
            break;
        } else {
            ret = dev_upgrade_set_mdc_flag(QUERY_FROM_BOOTSTRAP,
                UFS_UPGRADE_FLAG_LUN,
                UFS_FLAG_BASE_OFFSET,
                UFS_STATE_FLAG_OFFSET,
                UFS_WAIT_RST_FLAG);
            if (ret != OK) {
                dev_upgrade_err("set LU(%d) addr(0x%x) state_flag(0x%x) fail, ret: %d\n",
                    UFS_UPGRADE_FLAG_LUN,
                    UFS_FLAG_BASE_OFFSET + UFS_STATE_FLAG_OFFSET,
                    UFS_WAIT_RST_FLAG,
                    ret);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
            }
            break;
        }
    }

    return OK;
}

STATIC void dev_upgrade_set_ver_and_state_flag(unsigned int dev_id, int is_valid)
{
    g_upgrade_ver_and_state[dev_id]->is_valid = is_valid;
}

STATIC int dev_upgrade_get_ver_and_state_flag(unsigned int dev_id)
{
    return g_upgrade_ver_and_state[dev_id]->is_valid;
}

STATIC int dev_upgrade_update_ver_and_state(unsigned int dev_id)
{
    int ret;
    int i;

    for (i = DSMI_COMPONENT_TYPE_RAWDATA; i < DSMI_COMPONENT_TYPE_MAX; i++) {
        if (i > DSMI_COMPONENT_TYPE_SYSDRV && i != DSMI_COMPONENT_TYPE_RECOVERY) {
            continue;
        }

        ret = dev_upgrade_get_single_image_version(
            dev_id, UFS_MASTER_AREA, i, g_upgrade_ver_and_state[dev_id]->main_zone_ver[i], COMM_VERSION_LENGTH);
        if (ret != OK) {
            dev_upgrade_err("dev[%u] dev_upgrade_get_single_image_version failed, ret=%d.\n", dev_id, ret);
            dev_upgrade_set_ver_and_state_flag(dev_id, VER_AND_STATE_INVALID);
            return ret;
        }

        ret = dev_upgrade_get_single_image_version(
            dev_id, UFS_BACKUP_AREA, i, g_upgrade_ver_and_state[dev_id]->backup_zone_ver[i], COMM_VERSION_LENGTH);
        if (ret != OK) {
            dev_upgrade_err("dev[%u] dev_upgrade_get_single_image_version failed, ret=%d.\n", dev_id, ret);
            dev_upgrade_set_ver_and_state_flag(dev_id, VER_AND_STATE_INVALID);
            return ret;
        }
    }

    ret = dev_upgrade_get_mdc_flag(QUERY_FROM_CMDLINE,
        UFS_UPGRADE_FLAG_LUN,
        UFS_FLAG_BASE_OFFSET,
        UFS_STATE_FLAG_OFFSET,
        &g_upgrade_ver_and_state[dev_id]->state);
    if (ret != OK) {
        dev_upgrade_err("Get LU(%d) addr(0x%x) state_flag(0x%x) invalid, ret: %d\n",
            UFS_UPGRADE_FLAG_LUN,
            UFS_FLAG_BASE_OFFSET + UFS_STATE_FLAG_OFFSET,
            g_upgrade_ver_and_state[dev_id]->state,
            ret);
        dev_upgrade_set_ver_and_state_flag(dev_id, VER_AND_STATE_INVALID);
        return ret;
    }

    dev_upgrade_set_ver_and_state_flag(dev_id, VER_AND_STATE_VALID);

    return OK;
}

/*
 * Prototype    : dev_upgrade_get_version_and_state_init
 * Description  : get component versin and upgrade state info and store to global structure.
 * Params       : None
 * Return Value : 0 success, others for fail
 */
STATIC int dev_upgrade_version_and_state_init(void)
{
    int ret;
    int i;

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        g_upgrade_ver_and_state[i] = (upgrade_ver_and_state_t *)vzalloc(sizeof(upgrade_ver_and_state_t));
        if (g_upgrade_ver_and_state[i] == NULL) {
            dev_upgrade_err("alloc upgrade version and state struct fail, i=%d.\n", i);
            ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
            goto free_ver_mem;
        }

        dev_upgrade_set_ver_and_state_flag(i, VER_AND_STATE_INVALID);
    }

    return OK;

free_ver_mem:
    dev_upgrade_mdc_exit();
    return ret;
}

STATIC int dev_upgrade_mdc_upgrade_proc(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int ret;

    if (dev_upgrade_get_ver_and_state_flag(upgrade_ctrl->dev_id) == VER_AND_STATE_INVALID) {
        ret = dev_upgrade_update_ver_and_state(upgrade_ctrl->dev_id);
        if (ret != OK) {
            dev_upgrade_warn("dev(%u) dev_upgrade_update_ver_and_state invalid, ret=%d.\n", upgrade_ctrl->dev_id, ret);
        }
    }

    ret = dev_upgrade_sec_upgrade_proc(upgrade_ctrl);
    if (ret != OK) {
        dev_upgrade_err("dev(%u) dev_upgrade_sec_upgrade_proc fail, ret=%d.\n", upgrade_ctrl->dev_id, ret);
        return ret;
    }

    ret = dev_upgrade_update_ver_and_state(upgrade_ctrl->dev_id);
    if (ret != OK) {
        dev_upgrade_warn("dev(%u) dev_upgrade_update_ver_and_state fail, ret=%d.\n", upgrade_ctrl->dev_id, ret);
    }

    return OK;
}

STATIC int dev_upgrade_mdc_sync_proc(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int ret;

    if (dev_upgrade_get_ver_and_state_flag(upgrade_ctrl->dev_id) == VER_AND_STATE_INVALID) {
        ret = dev_upgrade_update_ver_and_state(upgrade_ctrl->dev_id);
        if (ret != OK) {
            dev_upgrade_warn("dev(%u) dev_upgrade_update_ver_and_state invalid, ret=%d.\n", upgrade_ctrl->dev_id, ret);
        }
    }

    ret = dev_upgrade_sync_proc(upgrade_ctrl);
    if (ret != OK) {
        dev_upgrade_err("dev(%u) dev_upgrade_sync_proc fail, ret=%d.\n", upgrade_ctrl->dev_id, ret);
        return ret;
    }

    ret = dev_upgrade_update_ver_and_state(upgrade_ctrl->dev_id);
    if (ret != OK) {
        dev_upgrade_warn("dev(%u) dev_upgrade_update_ver_and_state fail, ret=%d.\n", upgrade_ctrl->dev_id, ret);
    }

    return OK;
}

STATIC int dev_upgrade_mdc_firmware_sync_proc(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int ret;

    ret = dev_upgrade_firmware_sync_proc(upgrade_ctrl);
    if (ret != OK) {
        dev_upgrade_err("dev(%u) dev_upgrade_sync_proc fail, ret=%d.\n", upgrade_ctrl->dev_id, ret);
        return ret;
    }
    dev_upgrade_info("dev_upgrade_firmware_sync_proc success\n");

    return OK;
}

STATIC int dev_upgrade_get_recovery_boot_area(int dev_id, int *actual_boot_area)
{
    int ret;
    unsigned int cmdline = 0;

    ret = sec_flash_read_cmdline(dev_id, &cmdline);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) sec_flash_read_cmdline fail, ret=%d.\n", dev_id, ret);
        return ret;
    }

    dev_upgrade_info("dev(%d) sec_flash_read_cmdline: %x.\n", dev_id, cmdline);
    if ((cmdline & UFS_RAWDATA_CUR_AREA_MASK) == UFS_RAWDATA_CUR_AREA_MASK) {
        *actual_boot_area = UFS_BACKUP_AREA;
    }
    dev_upgrade_info("dev(%d) boot area: %d\n", dev_id, *actual_boot_area);

    return 0;
}

STATIC bool dev_upgrade_is_in_recovery_system(void)
{
    int err;
    struct kstat statbuff;
    const char *recovery_init_file = "/var/recovery_flag";

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    struct file *srcFilp = NULL;
    srcFilp = filp_open(recovery_init_file, O_RDONLY, 0);
    if (IS_ERR(srcFilp)) {
        dev_upgrade_info("unable to open file: %s, errno = %ld.\n", recovery_init_file, PTR_ERR(srcFilp));
        return false;
    }
    err = vfs_getattr(&srcFilp->f_path, &statbuff, STATX_BASIC_STATS, AT_NO_AUTOMOUNT);
    (void)filp_close(srcFilp, NULL);
#else
    mm_segment_t old_fs;
    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    statbuff.size = 0;
    err = vfs_stat(recovery_init_file, &statbuff);
    set_fs(old_fs);
#endif

    if (err < 0) {
        dev_upgrade_info("unable to get file %s status. %d\n", recovery_init_file, err);
        return false;
    }

    dev_upgrade_info("get file %s status success. %d\n", recovery_init_file, err);
    return true;
}

STATIC int dev_upgrade_mdc_sync_recovery_proc(dev_upgrade_core_ctrl *upgrade_ctrl)
{
#if (defined CFG_SOC_PLATFORM_MDC_V51)
    int ret;
    unsigned int actual_boot_area = UFS_MASTER_AREA;
    upgrade_ufs_component_info *local_component = NULL;

    upgrade_ctrl->schedule = SYNC_PROP_START;
    local_component = dev_upgrade_ufs_get_local_component(DSMI_COMPONENT_TYPE_RECOVERY);
    if (local_component == NULL) {
        dev_upgrade_warn(
            "dev(%d) can't find component, component: %u\n", upgrade_ctrl->dev_id, DSMI_COMPONENT_TYPE_RECOVERY);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    /* If the standby area of the recovery system is being started, the active
     * area may be damaged and cannot be started. In this case, do not synchronize
     * the active and standby areas
     */
    if (dev_upgrade_is_in_recovery_system()) {
        ret = dev_upgrade_get_recovery_boot_area(upgrade_ctrl->dev_id, &actual_boot_area);
        if (ret != 0) {
            dev_upgrade_err("dev(%d) get recovery boot area fail, ret=%d.\n", upgrade_ctrl->dev_id, ret);
            return ret;
        }

        if (actual_boot_area == UFS_BACKUP_AREA) {
            dev_upgrade_err("dev(%d) can't sync in backup area\n", upgrade_ctrl->dev_id);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_PERMISSION_DENIED);
        }
    }

    /* Recovery component from master to slave */
    local_component->cur_flag = UFS_MASTER_AREA;
    ret = dev_upgrade_sync_single_component(local_component);
    if (ret != OK) {
        dev_upgrade_err("recovery component dev_upgrade_sync_single_component fail, ret: %d\n", ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }
    upgrade_ctrl->schedule = SYNC_PROP_FINISHED;
    dev_upgrade_info("sync recovery component successfully.\n");
#endif
    return OK;
}

STATIC int dev_upgrade_update_crl(dev_upgrade_crl_info_t *crl_info)
{
    int ret;

    if (crl_info == NULL) {
        dev_upgrade_err("crl_info NULL.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (crl_info->crl_data == NULL) {
        dev_upgrade_err("crl_data NULL.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (crl_info->size > CRL_FILE_SIZE_MAX) {
        dev_upgrade_err("crl size(0x%x) over limit(0x%x).\n", crl_info->size, CRL_FILE_SIZE_MAX);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    ret = dev_upgrade_update_crl_process(crl_info->crl_data, crl_info->size);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_update_crl_process fail, ret=%d, size=%u.\n", ret, crl_info->size);
        return ret;
    }

    return OK;
}

STATIC int dev_upgrade_thread_func(void *upgrade_data)
{
    int ret;
    upgrade_thread_trans_t *upgrade_thread_trans = (upgrade_thread_trans_t *)upgrade_data;

    while (!kthread_should_stop()) {
        dev_upgrade_info("upgrade thread wait event start.\n");
        ret = wait_event_interruptible(upgrade_thread_trans->wait_queue,
            (atomic_read(&upgrade_thread_trans->cmd) != UPGRADE_THREAD_CMD_DEFAULT) || kthread_should_stop());
        if (ret) {
            dev_upgrade_warn("wait event interrupt, dev_id:%u, cmd=%d ret:%d.\n",
                upgrade_thread_trans->dev_id,
                atomic_read(&upgrade_thread_trans->cmd),
                ret);
            continue;
        }

        dev_upgrade_info("upgrade thread recv event, cmd=%d.\n", atomic_read(&upgrade_thread_trans->cmd));
        switch (atomic_read(&upgrade_thread_trans->cmd)) {
            case UPGRADE_THREAD_CMD_UPDATE:
                ret = dev_upgrade_mdc_upgrade_proc(upgrade_thread_trans->upgrade_ctrl);
                break;
            case UPGRADE_THREAD_CMD_SYNC:
                ret = dev_upgrade_mdc_sync_proc(upgrade_thread_trans->upgrade_ctrl);
                break;
            case UPGRADE_THREAD_CMD_SYNC_RECOVERY:
                ret = dev_upgrade_mdc_sync_recovery_proc(upgrade_thread_trans->upgrade_ctrl);
                break;
            case UPGRADE_THREAD_CMD_GET_VER_AND_STATE:
                ret = dev_upgrade_update_ver_and_state(upgrade_thread_trans->dev_id);
                break;
            case UPGRADE_THREAD_CMD_CRL_UPDATE:
                ret = dev_upgrade_update_crl(upgrade_thread_trans->crl_info);
                break;
            case UPGRADE_THREAD_CMD_SYNC_FIRMWARE:
                ret = dev_upgrade_mdc_firmware_sync_proc(upgrade_thread_trans->upgrade_ctrl);
                break;
            default:
                dev_upgrade_warn(
                    "upgrade thread cmd[%d] invalid, ret=%d.\n", atomic_read(&upgrade_thread_trans->cmd), ret);
                atomic_set(&upgrade_thread_trans->cmd, UPGRADE_THREAD_CMD_DEFAULT);
                continue;
        }

        if (ret != OK) {
            dev_upgrade_warn("upgrade thread cmd[%d] fail, ret=%d.\n", atomic_read(&upgrade_thread_trans->cmd), ret);
            atomic_set(&upgrade_thread_trans->cmd, UPGRADE_THREAD_CMD_DEFAULT);
            atomic_set(&upgrade_thread_trans->result, UPGRADE_THREAD_RESULT_FAIL);
            continue;
        }
        dev_upgrade_info("cmd(%d) process success.\n", atomic_read(&upgrade_thread_trans->cmd));
        atomic_set(&upgrade_thread_trans->cmd, UPGRADE_THREAD_CMD_DEFAULT);

        atomic_set(&upgrade_thread_trans->result, UPGRADE_THREAD_RESULT_SUCCESS);
        dev_upgrade_info("result(%d).\n", atomic_read(&upgrade_thread_trans->result));
    }

    dev_upgrade_info("upgrade thread run end.\n");
    return OK;
}

int dev_upgrade_mdc_update_crl_proc(int dev_id, unsigned char *src_crl_data, unsigned int src_size)
{
    int ret = OK;
    int timeout = UPGRADE_CRL_UPDATE_TIMEOUT;

    dev_upgrade_crl_info_t *crl_info = g_upgrade_crl_info[dev_id];

    if (crl_info == NULL) {
        dev_upgrade_err("dev[%u] crl_info NULL, src_size=0x%x.\n", dev_id, src_size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (crl_info->crl_data == NULL) {
        dev_upgrade_err("dev[%u] crl_data NULL, src_size=0x%x.\n", dev_id, src_size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    ret = memcpy_s(crl_info->crl_data, CRL_FILE_SIZE_MAX, src_crl_data, src_size);
    if (ret != EOK) {
        dev_upgrade_err("dev[%u] copy crl_data fail, ret=%d src_size=0x%x.\n", dev_id, ret, src_size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }
    crl_info->size = src_size;

    g_upgrade_thread_trans[dev_id].dev_id = dev_id;
    g_upgrade_thread_trans[dev_id].crl_info = crl_info;

    atomic_set(&g_upgrade_thread_trans[dev_id].cmd, UPGRADE_THREAD_CMD_CRL_UPDATE);
    wake_up_interruptible(&g_upgrade_thread_trans[dev_id].wait_queue);

    /* wait for upgrade result */
    while (atomic_read(&g_upgrade_thread_trans[dev_id].result) == UPGRADE_THREAD_RESULT_DEFAULT) {
        msleep(UPGRADE_CRL_UPDATE_DELAY);
        timeout--;
        if (timeout == 0) {
            dev_upgrade_err("mdc upgrade update crl process timeout.\n");
        }
    }

    if (atomic_read(&g_upgrade_thread_trans[dev_id].result) != UPGRADE_THREAD_RESULT_SUCCESS) {
        dev_upgrade_err("dev(%d) mdc upgrade update crl process fail.\n", dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_IOCTL);
    }

    atomic_set(&g_upgrade_thread_trans[dev_id].result, UPGRADE_THREAD_RESULT_DEFAULT);
    return ret;
}

int dev_upgrade_mdc_update_sync_proc(int op_type, dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int ret = OK;
    int timeout = UPGRADE_SYNC_PROC_TIMEOUT;

    g_upgrade_thread_trans[upgrade_ctrl->dev_id].dev_id = upgrade_ctrl->dev_id;
    g_upgrade_thread_trans[upgrade_ctrl->dev_id].upgrade_ctrl = upgrade_ctrl;
    if (op_type == DISK_OP_TYPE_UPDATE) {
        atomic_set(&g_upgrade_thread_trans[upgrade_ctrl->dev_id].cmd, UPGRADE_THREAD_CMD_UPDATE);
    } else if (op_type == DISK_OP_TYPE_SYNC_RECOVERY) {
        atomic_set(&g_upgrade_thread_trans[upgrade_ctrl->dev_id].cmd, UPGRADE_THREAD_CMD_SYNC_RECOVERY);
    } else if (op_type == DISK_OP_TYPE_SYNC_FIRMWARE) {
        atomic_set(&g_upgrade_thread_trans[upgrade_ctrl->dev_id].cmd, UPGRADE_THREAD_CMD_SYNC_FIRMWARE);
    } else {
        atomic_set(&g_upgrade_thread_trans[upgrade_ctrl->dev_id].cmd, UPGRADE_THREAD_CMD_SYNC);
    }
    wake_up_interruptible(&g_upgrade_thread_trans[upgrade_ctrl->dev_id].wait_queue);
    dev_upgrade_info("start:dev(%d) op_type(%d) result(%d).\n",
        upgrade_ctrl->dev_id,
        op_type,
        atomic_read(&g_upgrade_thread_trans[upgrade_ctrl->dev_id].result));

    /* wait for upgrade result */
    while (atomic_read(&g_upgrade_thread_trans[upgrade_ctrl->dev_id].result) == UPGRADE_THREAD_RESULT_DEFAULT) {
        msleep(UPGRADE_SYNC_PROC_DELAY);
        timeout--;
        if (timeout == 0) {
            dev_upgrade_err("mdc upgrade update sync process timeout.\n");
        }
    }

    if (atomic_read(&g_upgrade_thread_trans[upgrade_ctrl->dev_id].result) != UPGRADE_THREAD_RESULT_SUCCESS) {
        dev_upgrade_err("dev(%d) mdc upgrade update sync process fail.\n", upgrade_ctrl->dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_IOCTL);
    }

    atomic_set(&g_upgrade_thread_trans[upgrade_ctrl->dev_id].result, UPGRADE_THREAD_RESULT_DEFAULT);
    dev_upgrade_info("end:dev(%d) op_type(%d) result(%d).\n",
        upgrade_ctrl->dev_id,
        op_type,
        atomic_read(&g_upgrade_thread_trans[upgrade_ctrl->dev_id].result));
    return ret;
}

void dev_upgrade_mdc_update_crl_uninit(void)
{
    int i;
    dev_upgrade_crl_info_t *crl_info = NULL;

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        crl_info = g_upgrade_crl_info[i];
        if (crl_info == NULL) {
            continue;
        }

        DEV_UPGRADE_VFREE(crl_info->crl_data);
        DEV_UPGRADE_VFREE(crl_info);
    }
}

int dev_upgrade_mdc_update_crl_init(void)
{
    int i, ret;
    dev_upgrade_crl_info_t *crl_info = NULL;

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        crl_info = (dev_upgrade_crl_info_t *)vzalloc(sizeof(dev_upgrade_crl_info_t));
        if (crl_info == NULL) {
            dev_upgrade_err("dev(%d) vzalloc crl_info fail.\n", i);
            ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
            goto crl_exit;
        }

        crl_info->crl_data = (unsigned char *)vzalloc(CRL_FILE_SIZE_MAX);
        if (crl_info->crl_data == NULL) {
            dev_upgrade_err("dev(%d) vzalloc crl_data fail.\n", i);
            ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
            DEV_UPGRADE_VFREE(crl_info);
            crl_info = NULL;
            goto crl_exit;
        }
        g_upgrade_crl_info[i] = crl_info;
    }

    return OK;

crl_exit:
    dev_upgrade_mdc_update_crl_uninit();

    return ret;
}

/*
 * Prototype    : dev_upgrade_update_and_sync_init
 * Description  : init update and sync proc thread
 * Params       : None
 * Return Value : 0 success, others for fail
 */
int dev_upgrade_update_and_sync_init(void)
{
    int i;

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        init_waitqueue_head(&g_upgrade_thread_trans[i].wait_queue);
        atomic_set(&g_upgrade_thread_trans[i].cmd, UPGRADE_THREAD_CMD_DEFAULT);
        atomic_set(&g_upgrade_thread_trans[i].result, UPGRADE_THREAD_RESULT_DEFAULT);
        g_upgrade_thread_trans[i].thread =
            kthread_run(dev_upgrade_thread_func, &g_upgrade_thread_trans[i], "kupgrade_%d", i);
        if (IS_ERR(g_upgrade_thread_trans[i].thread)) {
            dev_upgrade_err("creat upgrade thread fail, ret=%ld.\n", PTR_ERR(g_upgrade_thread_trans[i].thread));
            return PTR_ERR(g_upgrade_thread_trans[i].thread);
        }
    }

    return OK;
}

/*
 * Prototype    : dev_upgrade_mdc_init
 * Description  : init for mdc upgrade.
 * Params       : None
 * Return Value : 0 success, others for fail
 */
int dev_upgrade_mdc_init(void)
{
    int ret;

    ret = dev_upgrade_set_media_read_write_cb();
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_set_media_read_write_cb fail, ret=%x", ret);
        return ret;
    }

    ret = dev_upgrade_version_and_state_init();
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_get_components_and_state_init fail, ret=%x", ret);
        return ret;
    }

    ret = dev_upgrade_mdc_update_crl_init();
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_mdc_update_crl_init fail, ret=%x", ret);
        goto mdc_exit;
    }

    ret = dev_upgrade_mdc_register_program_package_init();
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_mdc_register_program_package_init fail, ret=%x", ret);
        goto mdc_exit;
    }

    ret = dev_upgrade_update_and_sync_init();
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_update_and_sync_init fail, ret=%x", ret);
        goto mdc_exit;
    }

    ret = dev_upgrade_update_cmdline_state();
    if (ret != OK) {
        dev_upgrade_warn("dev_upgrade_update_cmdline_state fail, ret=%x", ret);
    }

    dev_upgrade_img_id_init();

    return OK;

mdc_exit:
    dev_upgrade_mdc_exit();
    return ret;
}

void dev_upgrade_mdc_exit(void)
{
    int i;

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        if (!IS_ERR_OR_NULL(g_upgrade_thread_trans[i].thread)) {
            kthread_stop(g_upgrade_thread_trans[i].thread);
            g_upgrade_thread_trans[i].thread = NULL;
        }

        if (g_upgrade_ver_and_state[i] != NULL) {
            vfree(g_upgrade_ver_and_state[i]);
            g_upgrade_ver_and_state[i] = NULL;
        }
    }

    dev_upgrade_mdc_update_crl_uninit();
    dev_upgrade_img_id_uninit();
    dev_upgrade_mdc_register_program_package_uninit();
}

STATIC int dev_upgrade_get_cur_media(unsigned int *stat)
{
    int ret;
    unsigned int cmdline = 0;
    int pxe_or_normal_scenes = NORMAL_UPGRADE_SCENES;

    ret = dev_upgrade_get_cmdline(&cmdline, &pxe_or_normal_scenes);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_get_cmdline fail, ret: %d\n", ret);
        return ret;
    }

    *stat = UPGRADE_UFS_MEDIA;
    if (pxe_or_normal_scenes == NORMAL_UPGRADE_SCENES) {
        if ((cmdline & UPGRADE_CUR_MEDIA_MASK) == UPGRADE_CUR_MEDIA_MASK) {
            *stat = UPGRADE_SSD_MEDIA;
        }
    }

    return 0;
}

int dev_upgrade_get_media(int mode, unsigned int *stat)
{
    int ret = OK;

    if (mode == QUERY_FROM_CMDLINE) {
        ret = dev_upgrade_get_cur_media(stat);
    } else if (mode == QUERY_FROM_BOOTSTRAP) {
#ifdef CFG_SOC_PLATFORM_MDC_V11
        // 先打桩直接赋值
        *stat = UPGRADE_EMMC_MEDIA;
#else
        ret = devdrv_manager_get_bootstrap(stat);
        if (ret != 0) {
            dev_upgrade_err("get_bootstrap_func fail, ret=%d.\n", ret);
            return ret;
        }
#endif
    } else {
        dev_upgrade_err("mode(%d) err.\n", mode);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if (ret != OK) {
        dev_upgrade_err("get cur media fail, mode(%d) ret: %d\n", mode, ret);
        return ret;
    }

    return OK;
}

int dev_upgrade_set_media_read_write_cb(void)
{
    int ret;
    unsigned int stat = UPGRADE_UFS_MEDIA;

    ret = dev_upgrade_get_media(QUERY_FROM_BOOTSTRAP, &stat);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_get_media fail, ret=%d.\n", ret);
        return ret;
    }

    if (stat == UPGRADE_SSD_MEDIA) {
        g_disk_ops.cur_boot_disk_write = dev_upgrade_ssd_write;
        g_disk_ops.cur_boot_disk_read = dev_upgrade_ssd_read;
        g_disk_ops.bootstrap_disk_write = dev_upgrade_ssd_write;
        g_disk_ops.bootstrap_disk_read = dev_upgrade_ssd_read;
        dev_upgrade_info("current and bootstrap storage media is ssd.\n");
#ifdef CFG_SOC_PLATFORM_MDC_V11
    } else if (stat == UPGRADE_EMMC_MEDIA) {
        g_disk_ops.cur_boot_disk_write = dev_upgrade_emmc_write;
        g_disk_ops.cur_boot_disk_read = dev_upgrade_emmc_read;
        g_disk_ops.bootstrap_disk_write = dev_upgrade_emmc_write;
        g_disk_ops.bootstrap_disk_read = dev_upgrade_emmc_read;
        dev_upgrade_info("current and bootstrap storage media is emmc.\n");
#endif
    } else {
        dev_upgrade_info("current and bootstrap storage media is ufs.\n");
    }

    return OK;
}

STATIC void dev_upgrade_abnormal_data_print(const char *write_buff, const char *read_buff, const u32 len)
{
    u32 i;
    u32 count = 100;

    for (i = 0; i < len; i++) {
        if ((write_buff[i] != read_buff[i]) && (count != 0)) {
            count--;
            dev_upgrade_err("[%d] write_buff:0x%02x; read_buff:0x%02x.\n", i, write_buff[i], read_buff[i]);
        }
    }
}

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
void ufs_dfx_data_check(const u8 *addr, uint32_t size)
{
    dev_upgrade_err("ufs_dfx_data_check error, esl don't support ufs");
    return;
}
#endif

int dev_upgrade_disk_write(int mode, const u8 lun, const u64 offset, const char *buff, const u32 len)
{
    int ret;
    char *read_buff = NULL;
    disk_write_handle disk_write = g_disk_ops.cur_boot_disk_write;
    disk_read_handle disk_read = g_disk_ops.cur_boot_disk_read;

    if (buff == NULL) {
        dev_upgrade_err("mode(%d) input buff is null\n", mode);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if (len > MAX_BUFFER_SIZE) {
        dev_upgrade_err("mode(%d) len to write can't be larger than %dKB\n", mode, MAX_BUFFER_SIZE);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if (mode == QUERY_FROM_BOOTSTRAP) {
        disk_write = g_disk_ops.bootstrap_disk_write;
        disk_read = g_disk_ops.bootstrap_disk_read;
    }

    read_buff = (char *)vzalloc(MAX_BUFFER_SIZE);
    if (read_buff == NULL) {
        dev_upgrade_err("mode(%d) alloc read buff fail.\n", mode);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    ret = disk_write(lun, offset, buff, len);
    if (ret != 0) {
        dev_upgrade_err("mode(%d) disk write failed, lu=%u, offset=0x%llx\n", mode, lun, offset);
        goto _free_buff;
    }

    /* after finish write data, read data and compare if data has write to disk */
    ret = disk_read(lun, offset, read_buff, len);
    if (ret != 0) {
        dev_upgrade_err("mode(%d) disk read failed, lu=%u, offset=0x%llx\n", mode, lun, offset);
        goto _free_buff;
    }

    if (memcmp(buff, read_buff, len) != 0) {
        dev_upgrade_err("mode(%d) write verify failed, lu=%u, offset=0x%llx\n", mode, lun, offset);
#ifndef CFG_SOC_PLATFORM_MDC_V11
        ufs_dfx_data_check(buff, len);
#endif
        dev_upgrade_abnormal_data_print(buff, read_buff, len);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
        goto _free_buff;
    }

    yield();

_free_buff:
    DEV_UPGRADE_VFREE(read_buff);
    return ret;
}

int dev_upgrade_disk_read(int mode, const u8 lun, const u64 offset, char *buff, const u32 len)
{
    int ret;
    disk_read_handle disk_read = g_disk_ops.cur_boot_disk_read;

    if (len > MAX_BUFFER_SIZE) {
        dev_upgrade_err("mode(%d) length to read can't be larger than %dKB\n", mode, MAX_BUFFER_SIZE);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }
    if (buff == NULL) {
        dev_upgrade_err("mode(%d) buffer can't be NULL\n", mode);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if (mode == QUERY_FROM_BOOTSTRAP) {
        disk_read = g_disk_ops.bootstrap_disk_read;
    }

    ret = disk_read(lun, offset, buff, len);
    if (ret != 0) {
        dev_upgrade_err("mode(%d) disk read failed, lu=%u, offset=0x%llx\n", mode, lun, offset);
        return ret;
    }

    return ret;
}

STATIC int dev_upgrade_get_ver_and_state_info(unsigned int dev_id)
{
    int ret = OK;
    int timeout = GET_VER_AND_STATE_PROC_TIMEOUT;

    g_upgrade_thread_trans[dev_id].upgrade_ctrl = NULL;
    g_upgrade_thread_trans[dev_id].dev_id = dev_id;
    atomic_set(&g_upgrade_thread_trans[dev_id].cmd, UPGRADE_THREAD_CMD_GET_VER_AND_STATE);
    wake_up_interruptible(&g_upgrade_thread_trans[dev_id].wait_queue);

    /* wait for upgrade result */
    while (atomic_read(&g_upgrade_thread_trans[dev_id].result) == UPGRADE_THREAD_RESULT_DEFAULT) {
        msleep(GET_VER_AND_STATE_PROC_DELAY);
        timeout--;
        if (timeout == 0) {
            dev_upgrade_err("dev_upgrade_get_ver_and_state_info timeout.\n");
        }
    }

    if (atomic_read(&g_upgrade_thread_trans[dev_id].result) != UPGRADE_THREAD_RESULT_SUCCESS) {
        dev_upgrade_err("dev(%d) dev_upgrade_get_ver_and_state_info info fail.\n", dev_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_IOCTL);
    }

    atomic_set(&g_upgrade_thread_trans[dev_id].result, UPGRADE_THREAD_RESULT_DEFAULT);
    return ret;
}

int dev_upgrade_get_disk_img_version(dev_upgrade_core_ctrl *upgrade_ctrl, unsigned int component_type,
    unsigned int cur_flag, unsigned char *version_str, unsigned int max_len)
{
    int ret;
    unsigned char *ver_buf = NULL;
    unsigned int cmdline = 0;
    int pxe_or_normal_scenes = NORMAL_UPGRADE_SCENES;
    unsigned int dev_id = upgrade_ctrl->dev_id;

    ret = dev_upgrade_get_cmdline(&cmdline, &pxe_or_normal_scenes);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_get_cmdline fail, ret: %d\n", ret);
        return ret;
    }

    if ((dev_upgrade_get_ver_and_state_flag(dev_id) == VER_AND_STATE_INVALID) &&
        (pxe_or_normal_scenes == NORMAL_UPGRADE_SCENES)) {
        if (mutex_trylock(&upgrade_ctrl->lock)) {
            ret = dev_upgrade_get_ver_and_state_info(dev_id);
            if (ret != OK) {
                dev_upgrade_err("dev(%u) dev_upgrade_get_ver_and_state_info fail, ret=%d.\n", dev_id, ret);
                mutex_unlock(&upgrade_ctrl->lock);
                return ret;
            }

            mutex_unlock(&upgrade_ctrl->lock);
        }
    }

    if (cur_flag == UFS_MASTER_AREA) {
        ver_buf = g_upgrade_ver_and_state[dev_id]->main_zone_ver[component_type];
    } else {
        ver_buf = g_upgrade_ver_and_state[dev_id]->backup_zone_ver[component_type];
    }

    ret = memcpy_s(version_str, max_len, ver_buf, COMM_VERSION_LENGTH);
    if (ret != EOK) {
        dev_upgrade_err("dev[%u] copy version data fail, ret=%d.\n", dev_id, ret);
        return ret;
    }

    return ret;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
int dev_upgrade_refresh_register_program_package_version(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int ret;
    unsigned int cmdline = 0;
    int pxe_or_normal_scenes = NORMAL_UPGRADE_SCENES;
    unsigned int dev_id = upgrade_ctrl->dev_id;

    ret = dev_upgrade_get_cmdline(&cmdline, &pxe_or_normal_scenes);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_get_cmdline fail, ret: %d\n", ret);
        return ret;
    }

    if (pxe_or_normal_scenes == NORMAL_UPGRADE_SCENES) {
        ret = dev_upgrade_get_ver_and_state_info(dev_id);
        if (ret != OK) {
            dev_upgrade_err("dev(%u) dev_upgrade_get_ver_and_state_info fail, ret=%d.\n", dev_id, ret);
            return ret;
        }
    }
    return ret;
}
#endif

int dev_upgrade_get_disk_img_update_flag(dev_upgrade_core_ctrl *upgrade_ctrl, unsigned int *update_flag)
{
    int ret;
    unsigned int cmdline = 0;
    int pxe_or_normal_scenes = NORMAL_UPGRADE_SCENES;
    unsigned int dev_id = upgrade_ctrl->dev_id;

    ret = dev_upgrade_get_cmdline(&cmdline, &pxe_or_normal_scenes);
    if (ret != OK) {
        dev_upgrade_err("dev_upgrade_get_cmdline fail, ret: %d\n", ret);
        return ret;
    }

    if (dev_upgrade_get_ver_and_state_flag(dev_id) == VER_AND_STATE_INVALID) {
        if (pxe_or_normal_scenes == PXE_UPGRADE_SCENES) {
            *update_flag = UFS_NORMAL_FLAG;
            return OK;
        }

        if (mutex_trylock(&upgrade_ctrl->lock)) {
            ret = dev_upgrade_get_ver_and_state_info(dev_id);
            if (ret != OK) {
                dev_upgrade_err("dev(%u) dev_upgrade_get_ver_and_state_info fail, ret=%d.\n", dev_id, ret);
                mutex_unlock(&upgrade_ctrl->lock);
                return ret;
            }

            mutex_unlock(&upgrade_ctrl->lock);
        }
    }

    *update_flag = g_upgrade_ver_and_state[dev_id]->state;
    return OK;
}
