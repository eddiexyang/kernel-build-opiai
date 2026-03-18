/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-02-25
 */
#ifndef PROF_UNIT_TEST

#include <linux/securec.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/timekeeping.h>

#include "kernel_version_adapt.h"
#include "chip_config.h"
#include "uda.h"
#include "devdrv_interface.h"
#include "drv_profile.h"

#include "prof_drv_dev.h"
#include "prof_mem_sample.h"

#define BYTES_PER_KB 1024ul

#define PROC_MEMINFO_PATH "/proc/meminfo"

#define NID_MEMINFO_PATH_LEN 64
#define NID_MEMINFO_PATH     "/sys/devices/system/node/node%d/meminfo"
#define NID_MEMINFO_MAX_LEN  2048

#define PROF_DEV_CGROUP_LIMIT_PATH  "/sys/fs/cgroup/memory/usermemory/memory.limit_in_bytes"
#define PROF_VDEV_CGROUP_LIMIT_PATH "/sys/fs/cgroup/memory/usermemory/dev%d/vf%d/memory.limit_in_bytes"
#define PROF_VDEV_CGROUP_USAGE_PATH "/sys/fs/cgroup/memory/usermemory/dev%d/vf%d/memory.usage_in_bytes"

#define PROF_MEMNFO_TMP_LEN 128
#define PROF_DEVICE_AUTHORITY 0440

typedef int (*get_dev_meminfo)(u32, u64 *, u64 *);
typedef int (*get_vdev_meminfo)(u32, u32, u64 *, u64 *);

static struct timespec64 g_start_timestamp[PROF_MEM_SAMPLE_MAX_DEV_NUM];

static get_vdev_meminfo g_prof_get_vdev_meminfo_func = NULL;

static void prof_dev_mem_sample_func_init(void)
{
    if (g_prof_get_vdev_meminfo_func == NULL) {
        g_prof_get_vdev_meminfo_func = (get_vdev_meminfo)(uintptr_t)
            __kallsyms_lookup_name("devmm_get_hbm_meminfo_by_vdev");
    }
}

static int prof_get_vdev_hbm_meminfo(u32 devid, u32 vfid, u64 *total_size, u64 *free_size)
{
    *total_size = 0;
    *free_size = 0;
    if (g_prof_get_vdev_meminfo_func != NULL) {
        return g_prof_get_vdev_meminfo_func(devid, vfid, total_size, free_size);
    }

    return 0;
}

static ssize_t prof_kernel_read(struct file *fp, char *dst_addr, size_t fsize, loff_t *pos)
{
    return kernel_read(fp, dst_addr, fsize, pos);
}

static int prof_get_data_from_buffer(const char *buffer, const char *key, size_t len, u64 *data)
{
    char *pos = NULL;
    u64 tmp;
    int ret;

    pos = strstr(buffer, key);
    if (pos == NULL) {
        prof_err("Can not find key from nid info. (key=%s)\n", key);
        return -EINVAL;
    }

    ret = sscanf_s(pos + len + 1, " %llu", &tmp);
    if (ret <= 0) {
        prof_info("Sscanf_s value not find. (key=%s)\n", key);
        tmp = 0;
    }
    *data = tmp;

    return 0;
}

static int create_node_name(char *node_name, size_t memnfo_path_len, u32 node_index)
{
    if (num_online_nodes() > 1) {
        return snprintf_s(node_name, memnfo_path_len, memnfo_path_len - 1, NID_MEMINFO_PATH, node_index);
    }

    return snprintf_s(node_name, memnfo_path_len, memnfo_path_len - 1, "%s", PROC_MEMINFO_PATH);
}

static int prof_get_nid_meminfo(int nid, u64 *total_size, u64 *free_size)
{
    char path[NID_MEMINFO_PATH_LEN] = {0};
    struct file *fp = NULL;
    u64 hpg_free_num, total_size_kb, normal_free_size_kb;
    loff_t pos = 0;
    char *buffer = NULL;
    int ret;

    ret = create_node_name(path, NID_MEMINFO_PATH_LEN, nid);
    if (ret < 0) {
        prof_err("Sprintf_s fail. (nid=%d)\n", nid);
        return -EINVAL;
    }

    fp = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        prof_err("File not exist. (file=\"%s\")\n", path);
        return -ENOENT;
    }

    buffer = (char *)kzalloc(NID_MEMINFO_MAX_LEN, GFP_KERNEL | __GFP_ACCOUNT);
    if (buffer == NULL) {
        prof_err("Kzalloc fail.\n");
        ret = -ENOMEM;
        goto out_close;
    }

    ret = prof_kernel_read(fp, buffer, NID_MEMINFO_MAX_LEN - 1, &pos);
    if (ret < 0) {
        prof_err("Kernel_read fail. (file=\"%s\")\n", path);
        ret = -EINVAL;
        goto out_kfree;
    }

    ret = prof_get_data_from_buffer(buffer, "MemTotal", strlen("MemTotal"), &total_size_kb);
    if (ret != 0) {
        prof_err("Get nid MemTotal fail. (file=\"%s\")\n", path);
        goto out_kfree;
    }
    ret = prof_get_data_from_buffer(buffer, "MemFree", strlen("MemFree"), &normal_free_size_kb);
    if (ret != 0) {
        prof_err("Get nid MemFree fail. (file=\"%s\")\n", path);
        goto out_kfree;
    }
    ret = prof_get_data_from_buffer(buffer, "HugePages_Free", strlen("HugePages_Free"), &hpg_free_num);
    if (ret != 0) {
        prof_err("Get nid HugePages_Free fail. (file=\"%s\")\n", path);
        goto out_kfree;
    }

    *total_size = total_size_kb * BYTES_PER_KB;
    *free_size = normal_free_size_kb * BYTES_PER_KB  + (hpg_free_num * HPAGE_SIZE);

    ret = 0;

out_kfree:
    kfree(buffer);
out_close:
    filp_close(fp, NULL);
    return ret;
}

static int prof_get_nids_meminfo(int nids[], int nid_num, u64 *total_size, u64 *free_size)
{
    u64 tmp_total_size, tmp_free_size;
    int ret, i;

    for (i = 0; i < nid_num; i++) {
        ret = prof_get_nid_meminfo(nids[i], &tmp_total_size, &tmp_free_size);
        if (ret != 0) {
            return ret;
        }

        *total_size += tmp_total_size;
        *free_size += tmp_free_size;
    }
    return 0;
}

static int _prof_get_dev_meminfo(u32 devid, int mem_type, u64 *total_size, u64 *free_size)
{
    int nids[DBL_NUMA_ID_MAX_NUM] = {0};
    int nid_num, ret;

#ifdef CFG_FEATURE_NO_SUPPORT_NODE_DIVIDE
    struct uda_mia_dev_para mia_para;
    if (!uda_is_phy_dev(devid)) {
        ret = uda_udevid_to_mia_devid(devid, &mia_para);
        if (ret != 0) {
            prof_err("Failed to get mia devid. (devid=%u)\n", devid);
            return ret;
        }
        devid = mia_para.phy_devid;
    }
#endif

    *total_size = 0;
    *free_size = 0;

    nid_num = dbl_nid_get_nid_num(devid, mem_type, DBL_SUB_MEMTYPE_ALL);
    if (nid_num <= 0) {
        return -ENOMEM;
    }

    ret = dbl_nid_get_nid(devid, mem_type, DBL_SUB_MEMTYPE_ALL, nids, nid_num);
    if (ret <= 0) {
        return ret;
    }

    return prof_get_nids_meminfo(nids, nid_num, total_size, free_size);
}

static int prof_get_dev_hbm_meminfo(u32 devid, u64 *total_size, u64 *free_size)
{
    return _prof_get_dev_meminfo(devid, DBL_MEMTYPE_HBM, total_size, free_size);
}

static int prof_get_dev_ddr_meminfo(u32 devid, u64 *total_size, u64 *free_size)
{
    return _prof_get_dev_meminfo(devid, DBL_MEMTYPE_DDR, total_size, free_size);
}

static int prof_get_cgroup_meminfo_by_path(const char *cgroup_path, u64 *size)
{
    char file_string[PROF_MEMNFO_TMP_LEN] = {0};
    struct file *fp = NULL;
    loff_t pos = 0;
    int ret;

    fp = filp_open(cgroup_path, O_RDONLY, PROF_DEVICE_AUTHORITY);
    if (IS_ERR((void const *)fp)) {
        prof_err("Open failed. (file=%s; errno=%ld)\n", cgroup_path, PTR_ERR((void const *)fp));
        return -EINVAL;
    }

    if (prof_kernel_read(fp, file_string, PROF_MEMNFO_TMP_LEN - 1, &pos) < 0) {
        prof_err("Filestring not right. (file=%s; pos=%lld)\n", cgroup_path, pos);
        (void)filp_close(fp, NULL);
        return -EINVAL;
    }

    ret = sscanf_s(file_string, "%llu", size);
    if (ret != 1) {
        prof_err("Sscanf not right. (file=%s; ret=%d)\n", cgroup_path, ret);
        (void)filp_close(fp, NULL);
        return -EINVAL;
    }

    (void)filp_close(fp, NULL);
    return 0;
}

static int prof_get_vdev_cgroup_meminfo(u32 devid, u32 vfid, u64 *total_size, u64 *free_size)
{
    char limit_size_path[PROF_MEMNFO_TMP_LEN] = {0};
    char usage_size_path[PROF_MEMNFO_TMP_LEN] = {0};
    u64 usage_size;
    int ret;

    ret = snprintf_s(limit_size_path, PROF_MEMNFO_TMP_LEN, PROF_MEMNFO_TMP_LEN - 1,
        PROF_VDEV_CGROUP_LIMIT_PATH, devid, vfid);
    if (ret < 0) {
        return ret;
    }

    ret = snprintf_s(usage_size_path, PROF_MEMNFO_TMP_LEN, PROF_MEMNFO_TMP_LEN - 1,
        PROF_VDEV_CGROUP_USAGE_PATH, devid, vfid);
    if (ret < 0) {
        return ret;
    }

    ret = prof_get_cgroup_meminfo_by_path(limit_size_path, total_size);
    if (ret != 0) {
        return ret;
    }

    ret = prof_get_cgroup_meminfo_by_path(usage_size_path, &usage_size);
    if (ret != 0) {
        return ret;
    }

    *free_size = *total_size - usage_size;
    return 0;
}

static get_dev_meminfo g_get_dev_meminfo_func[HISI_CHIP_NUM][DBL_MEMTYPE_NUM] = {
    [HISI_MINI_V1] = {
        [DBL_MEMTYPE_DDR] = prof_get_dev_ddr_meminfo,
        [DBL_MEMTYPE_HBM] = NULL
    },
    [HISI_MINI_V2] = {
        [DBL_MEMTYPE_DDR] = prof_get_dev_ddr_meminfo,
        [DBL_MEMTYPE_HBM] = NULL
    },
    [HISI_MINI_V3] = {
        [DBL_MEMTYPE_DDR] = prof_get_dev_ddr_meminfo,
        [DBL_MEMTYPE_HBM] = NULL
    },
    [HISI_CLOUD_V1] = {
        [DBL_MEMTYPE_DDR] = prof_get_dev_ddr_meminfo,
        [DBL_MEMTYPE_HBM] = prof_get_dev_hbm_meminfo
    },
    [HISI_CLOUD_V2] = {
        [DBL_MEMTYPE_DDR] = NULL,
        [DBL_MEMTYPE_HBM] = prof_get_dev_hbm_meminfo
    }
};

static get_vdev_meminfo g_get_vdev_meminfo_func[HISI_CHIP_NUM][DBL_MEMTYPE_NUM] = {
    [HISI_MINI_V2] = {
        [DBL_MEMTYPE_DDR] = prof_get_vdev_cgroup_meminfo,
        [DBL_MEMTYPE_HBM] = NULL
    },
    [HISI_CLOUD_V1] = {
        [DBL_MEMTYPE_DDR] = prof_get_vdev_cgroup_meminfo,
        [DBL_MEMTYPE_HBM] = prof_get_vdev_hbm_meminfo
    }
};

static int prof_get_dev_meminfo(u32 devid, u32 vfid, int mem_type, u64 *total_size, u64 *free_size)
{
    u32 chip_type;

    *total_size = 0;
    *free_size = 0;

    chip_type = uda_get_chip_type(devid);
    if (chip_type >= HISI_CHIP_NUM) {
        return -ENODEV;
    }

    if (vfid == 0) {
        if (g_get_dev_meminfo_func[chip_type][mem_type] != NULL) {
            return g_get_dev_meminfo_func[chip_type][mem_type](devid, total_size, free_size);
        }
    } else {
        if (g_get_vdev_meminfo_func[chip_type][mem_type] != NULL) {
            return g_get_vdev_meminfo_func[chip_type][mem_type](devid, vfid, total_size, free_size);
        }
    }
    return 0;
}

static int prof_dev_mem_sample_para_check(struct prof_peri_para *para)
{
    if (para->buff == NULL) {
        prof_err("Buff is NULL.\n");
        return -EINVAL;
    }

    if (para->buff_len < sizeof(struct prof_mem_sample_data)) {
        prof_err("Buff_len is invalid. (buff_len=%u)\n", para->buff_len);
        return -EINVAL;
    }

    if (para->device_id >= PROF_MEM_SAMPLE_MAX_DEV_NUM) {
        prof_err("Devid is out of range. (devid=%u)\n", para->device_id);
        return -EINVAL;
    }

    if (para->vfid >= PROF_MEM_SAMPLE_MAX_VF_NUM) {
        prof_err("Vfid is out of range. (devid=%u)\n", para->vfid);
        return -EINVAL;
    }
    return 0;
}

static u64 prof_get_dev_mem_sample_timestamp(u32 devid)
{
    struct timespec64 timestamp;

    ktime_get_raw_ts64(&timestamp);
    return prof_get_time_interval(&g_start_timestamp[devid], &timestamp);
}

int prof_sample_dev_mem_used_info(struct prof_peri_para *para)
{
    struct prof_mem_sample_data *data = (struct prof_mem_sample_data *)para->buff;
    u64 total_size, free_size;
    int ret;

    ret = prof_dev_mem_sample_para_check(para);
    if (ret != 0) {
        return ret;
    }

    ret = prof_get_dev_meminfo(para->device_id, para->vfid, DBL_MEMTYPE_DDR, &total_size, &free_size);
    if (ret != 0) {
        prof_err("Get dev ddr meminfo failed. (devid=%u; vfid=%u; ret=%d)", para->device_id, para->vfid, ret);
        return ret;
    }
    data->ddr_used_size = total_size - free_size;

    ret = prof_get_dev_meminfo(para->device_id, para->vfid, DBL_MEMTYPE_HBM, &total_size, &free_size);
    if (ret != 0) {
        prof_err("Get dev hbm meminfo failed. (devid=%u; vfid=%u; ret=%d)", para->device_id, para->vfid, ret);
        return ret;
    }
    data->hbm_used_size = total_size - free_size;
    data->timestamp = (u32)prof_get_dev_mem_sample_timestamp(para->device_id);
    data->event = PROF_MEM_SAMPLE_DEV_MODE;

    return sizeof(struct prof_mem_sample_data);
}
EXPORT_SYMBOL(prof_sample_dev_mem_used_info);

int prof_dev_mem_sample_init(struct prof_peri_para *para)
{
    if (para->device_id >= PROF_MEM_SAMPLE_MAX_DEV_NUM) {
        prof_err("Devid is out of range. (devid=%u)\n", para->device_id);
        return -EINVAL;
    }

    prof_dev_mem_sample_func_init();
    ktime_get_raw_ts64(&g_start_timestamp[para->device_id]);
    prof_debug("Dev mem sample init. (dev_id=%u)\n", para->device_id);
    return 0;
}
EXPORT_SYMBOL(prof_dev_mem_sample_init);
#else
void prof_mem_sample_by_dev_ut_test(void)
{
}
#endif
