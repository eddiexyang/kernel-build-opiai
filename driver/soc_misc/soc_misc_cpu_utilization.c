/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-11-15
*/

#include <linux/vmalloc.h>
#include <linux/fcntl.h>

#include "soc_misc_init.h"
#include "dms_timer.h"
#include "dms_template.h"
#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "soc_misc_feature.h"
#include "soc_misc_info.h"
#include "ascend_platform.h"

#define PREFIX_LF_SIZE 1
#define STAT_FILE_START_LEN 1024
#define STAT_FILE_MAX_LEN (1024 * 64)

typedef struct {
    unsigned int cpu_id;
    unsigned long long usr_cnt;
    unsigned long long nice_cnt;
    unsigned long long sys_cnt;
    unsigned long long idle_cnt;
    unsigned long long iowait_cnt;
    unsigned long long irq_cnt;
    unsigned long long soft_irq_cnt;
    unsigned long long steal_cnt;
    unsigned long long guest_cnt;
    unsigned long long guest_nice_cnt;
} cpu_stat_t;

#define CPU_STAT_SCANF_CNT 11
#define CPU_ERRLOG_TIME_INTERVAL 120
#define CPU_ERRLOG_CNT 5
#define DEVDRV_MAX_DAVINCI_NUM  64

typedef struct {
    unsigned long long cpu_total_stat;
    unsigned long long cpu_idle_stat;
    unsigned long long cpu_error_stat_cnt;
} cpu_stat_sum_t;

#define CPU_UTILIZATION_RATE_PERCENT 100

#define DMS_TIMER_INVALID 0xffff
#define CPU_UTILIZATION_CYCLE_MS 1000

STATIC cpu_stat_sum_t *g_last_cpu_cnt;
STATIC unsigned int *g_cpu_utilization;
STATIC unsigned int g_cpu_num;
STATIC u32 g_cpu_utilization_timer = DMS_TIMER_INVALID;
bool g_in_suspend = 0;

int soc_misc_calc_aicpu_utilization(unsigned int dev_id, unsigned int *utilization)
{
    struct soc_misc_info_st *soc_info = NULL;
    u32 phys_id = 0, vfid = 0;
    int ret;
    unsigned int total_rate = 0, i;
    unsigned int aicpu_num = 0, aicpu_index = 0, count = 0;

    if (utilization == NULL) {
        soc_misc_drv_err("utilization is NULL. (dev_id=%u)\n", dev_id);
        return EINVAL;
    }

    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        soc_misc_drv_err("Invalid dev_id. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    ret = dms_trans_and_check_id(dev_id, &phys_id, &vfid);
    if (ret != 0) {
        soc_misc_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u, ret=%d)\n", dev_id, ret);
        return ret;
    }

    soc_info = soc_misc_get_soc_info(phys_id);
    if (soc_info == NULL) {
        soc_misc_drv_err("Soc info is invalid. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    aicpu_num = (soc_info->cpu_info).aicpu_num;
    aicpu_index = CORE_NUM_PER_CHIP * dev_id + (soc_info->cpu_info).ccpu_num + (soc_info->cpu_info).dcpu_num;

    if (aicpu_num == 0) {
        *utilization = 0;
        return 0;
    }

    for (i = 0; i < aicpu_num; i++) {
        if (g_cpu_utilization[aicpu_index + i] == 0xEE) { /* 0xEE: demaged core */
            count++;
        } else if (g_cpu_utilization[aicpu_index + i] == 0xEF) { /* 0xEF: confilict with profiling */
            *utilization = 0xEF;
            return 0;
        } else {
            total_rate += g_cpu_utilization[aicpu_index + i];
        }
    }

    if (aicpu_num == count) {
        *utilization = 0;
    } else {
        *utilization = total_rate / (aicpu_num - count);
    }

    return 0;
}
EXPORT_SYMBOL(soc_misc_calc_aicpu_utilization);

int soc_misc_get_cpu_utilization(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct dms_get_cpu_utilization_in *cpu_info = NULL;
    int ret;

    if ((in == NULL) || (out == NULL) || (in_len < sizeof(struct dms_get_cpu_utilization_in))) {
        soc_misc_drv_err("Para check failed. (in=%d; out=%d; in_len=%u)\n", in != NULL, out != NULL, in_len);
        return -EINVAL;
    }

    cpu_info = (struct dms_get_cpu_utilization_in *)in;
    if (cpu_info->index + cpu_info->num > g_cpu_num) {
        soc_misc_drv_err("CPU num exceed. (cpu index=%u; cpu num=%u; g_cpu_num=%u)\n",
            cpu_info->index, cpu_info->num, g_cpu_num);
        return -EINVAL;
    }
    ret = memcpy_s(out, out_len, g_cpu_utilization + cpu_info->index, cpu_info->num * sizeof(unsigned int));
    if (ret != 0) {
        soc_misc_drv_err("memcpy failed. (ret=%d; out_len=%u; src len=%lu)\n",
            ret, out_len, cpu_info->num * sizeof(unsigned int));
        return -EINVAL;
    }
    return 0;
}

#define DMS_SOC_CPU_UTILIZATION "DMS_SOC_MISC_CPU_UTILIZATION"
BEGIN_DMS_MODULE_DECLARATION(DMS_SOC_CPU_UTILIZATION)
BEGIN_FEATURE_COMMAND()
ADD_FEATURE_COMMAND(DMS_SOC_CPU_UTILIZATION, DMS_MAIN_CMD_SOC, DMS_SUBCMD_GET_CPU_UTILIZATION, NULL, NULL,
                    DMS_SUPPORT_ALL, soc_misc_get_cpu_utilization)
END_FEATURE_COMMAND()
END_MODULE_DECLARATION()

STATIC char *soc_read_stat_file(void)
{
    static size_t buf_len = STAT_FILE_START_LEN;
    ssize_t read_len;
    struct file *fp = NULL;
    char *buf = NULL;
    loff_t pos = 0;

    fp = filp_open("/proc/stat", O_RDONLY, 0);
    if (IS_ERR_OR_NULL(fp)) {
        soc_misc_drv_err("Unable to open file. (errno=%ld)\n", PTR_ERR(fp));
        return NULL;
    }

    for (;;) {
        buf = vmalloc(buf_len + PREFIX_LF_SIZE + 1);
        if (buf == NULL) {
            soc_misc_drv_err("malloc fail. (size=%lu)\n", buf_len);
            (void)filp_close(fp, NULL);
            return NULL;
        }
        buf[0] = '\n';
        pos = 0;
        read_len = kernel_read(fp, buf + PREFIX_LF_SIZE, buf_len, &pos);
        if (read_len <= 0) {
            soc_misc_drv_err("Kernel read fail. (read len=%ld)\n", read_len);
            (void)filp_close(fp, NULL);
            vfree(buf);
            return NULL;
        }
        if (read_len < buf_len) {
            break;
        }
        if (buf_len >= STAT_FILE_MAX_LEN) {
            soc_misc_drv_err("Kernel read truncated, file len exceed. (buf_len=%lu)\n", buf_len);
            (void)filp_close(fp, NULL);
            vfree(buf);
            return NULL;
        }
        buf_len *= 2;
        soc_misc_drv_info("Kernel read truncated, buf expand. (buf_len=%lu)\n", buf_len);
        vfree(buf);
        buf = NULL;
    }
    (void)filp_close(fp, NULL);
    buf[PREFIX_LF_SIZE + read_len] = '\0';
    return buf;
}

STATIC int soc_parse_cpu_stat(const char *file, cpu_stat_sum_t *cal_stat, unsigned int num)
{
    const char *cpux_stat = NULL;
    cpu_stat_t cpu_stat = {0};
    unsigned int cpu_cnt = 0;
    static unsigned int log_cnt = 0;
    int ret;

    for (cpux_stat = file; ; cpux_stat++) {
        cpux_stat = strstr(cpux_stat, "\ncpu");
        if (cpux_stat == NULL) {
            break;
        }
        ret = sscanf_s(cpux_stat, "\ncpu%u %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n",
            &cpu_stat.cpu_id, &cpu_stat.usr_cnt, &cpu_stat.nice_cnt, &cpu_stat.sys_cnt,
            &cpu_stat.idle_cnt, &cpu_stat.iowait_cnt, &cpu_stat.irq_cnt, &cpu_stat.soft_irq_cnt,
            &cpu_stat.steal_cnt, &cpu_stat.guest_cnt, &cpu_stat.guest_nice_cnt);
        if (ret != CPU_STAT_SCANF_CNT) {
            continue;
        }
        if (cpu_stat.cpu_id >= num) {
            soc_misc_drv_err("CPU id exceed. (cpu_id=%u).\n", cpu_stat.cpu_id);
            return -ENXIO;
        }
        cal_stat[cpu_stat.cpu_id].cpu_total_stat = cpu_stat.cpu_id + cpu_stat.usr_cnt +
            cpu_stat.nice_cnt + cpu_stat.sys_cnt + cpu_stat.idle_cnt + cpu_stat.iowait_cnt +
            cpu_stat.irq_cnt + cpu_stat.soft_irq_cnt + cpu_stat.steal_cnt;
        cal_stat[cpu_stat.cpu_id].cpu_idle_stat = cpu_stat.idle_cnt + cpu_stat.iowait_cnt;
        cpu_cnt++;
    }
    if (cpu_cnt != num) {
        if (g_in_suspend) {
            soc_misc_drv_debug("Cpu in suspend stat. (cpu_cnt=%u; num=%u)\n", cpu_cnt, num);
            return -EBADF;
        }

        if ((log_cnt & CPU_ERRLOG_TIME_INTERVAL) == 0) {
            soc_misc_drv_err("Parse cpu stat fail. (cpu_cnt=%u; num=%u)\n", cpu_cnt, num);
        }
        log_cnt++;
        return -EBADF;
    }
    return 0;
}

STATIC void soc_cal_cpu_utilization(cpu_stat_sum_t *cur, cpu_stat_sum_t *last, unsigned int num)
{
    unsigned int i;
    long long total_inc, idle_inc;
    bool reset_suspend_stat = 0;

    for (i = 0; i < num; i++) {
        if (cur[i].cpu_total_stat == 0) {
            soc_misc_drv_err("Current cpu total 0 invalid. (i=%u)\n", i);
            continue;
        }
        total_inc = cur[i].cpu_total_stat - last[i].cpu_total_stat;
        idle_inc = cur[i].cpu_idle_stat - last[i].cpu_idle_stat;
        if ((total_inc <= 0) || (idle_inc < 0) || (total_inc < idle_inc)) {
            if (g_in_suspend) {
                reset_suspend_stat = 1;
                continue;
            }
            if (last[i].cpu_error_stat_cnt >= CPU_ERRLOG_CNT) {
                soc_misc_drv_err("CPU cnt increase invalid. (total_inc=%lld; idle_inc=%lld)\n", total_inc, idle_inc);
                last[i].cpu_error_stat_cnt = 0;
            } else {
                soc_misc_drv_info("CPU cnt decrease or stop. (total_inc=%lld; idle_inc=%lld)\n", total_inc, idle_inc);
            }
            last[i].cpu_error_stat_cnt++;
            cur[i].cpu_error_stat_cnt = last[i].cpu_error_stat_cnt;
            continue;
        }
        last[i].cpu_error_stat_cnt = 0;
        cur[i].cpu_error_stat_cnt = last[i].cpu_error_stat_cnt;
        g_cpu_utilization[i] = (unsigned int)(((total_inc - idle_inc) * CPU_UTILIZATION_RATE_PERCENT) / total_inc);
    }

    if (reset_suspend_stat) {
        g_in_suspend = 0;
    }
}

STATIC int soc_misc_record_cpu_stat(u64 user_data)
{
    char *stat_file = NULL;
    cpu_stat_sum_t *cur_cpu_cnt = NULL;
    int ret;

    cur_cpu_cnt = vzalloc(sizeof(cpu_stat_sum_t) * g_cpu_num);
    if (cur_cpu_cnt == NULL) {
        soc_misc_drv_err("malloc cpu cnt failed. (g_cpu_num=%u)\n", g_cpu_num);
        return -ENOMEM;
    }

    stat_file = soc_read_stat_file();
    if (stat_file == NULL) {
        vfree(cur_cpu_cnt);
        return -ENOENT;
    }
    ret = soc_parse_cpu_stat(stat_file, cur_cpu_cnt, g_cpu_num);
    if (ret != 0) {
        vfree(stat_file);
        vfree(cur_cpu_cnt);
        return ret;
    }
    soc_cal_cpu_utilization(cur_cpu_cnt, g_last_cpu_cnt, g_cpu_num);
    (void)memcpy_s(g_last_cpu_cnt, sizeof(cpu_stat_sum_t) * g_cpu_num, cur_cpu_cnt, sizeof(cpu_stat_sum_t) * g_cpu_num);

    vfree(stat_file);
    vfree(cur_cpu_cnt);
    return 0;
}

void soc_misc_suspend_notify(void)
{
    g_in_suspend = 1;
}
EXPORT_SYMBOL(soc_misc_suspend_notify);

int soc_misc_cpu_utilization_init(void)
{
    int ret;
    struct dms_timer_task task_property = {0};

    g_cpu_num = num_online_cpus();
    if (g_cpu_num == 0) {
        soc_misc_drv_err("Get cpu online num failed.\n");
        return -ENXIO;
    }

    g_last_cpu_cnt = vzalloc(sizeof(cpu_stat_sum_t) * g_cpu_num);
    if (g_last_cpu_cnt == NULL) {
        soc_misc_drv_err("malloc cpu cnt failed. (g_cpu_num=%u)\n", g_cpu_num);
        return -ENOMEM;
    }
    g_cpu_utilization = vzalloc(sizeof(unsigned int) * g_cpu_num);
    if (g_cpu_utilization == NULL) {
        vfree(g_last_cpu_cnt);
        g_last_cpu_cnt = NULL;
        soc_misc_drv_err("malloc utilization failed. (g_cpu_num=%u)\n", g_cpu_num);
        return -ENOMEM;
    }

    task_property.expire_ms = CPU_UTILIZATION_CYCLE_MS;
    task_property.count_ms = 0;
    task_property.user_data = 0;
    task_property.handler_mode = COMMON_WORK;
    task_property.exec_task = soc_misc_record_cpu_stat;
    ret = dms_timer_task_register(&task_property, &g_cpu_utilization_timer);
    if (ret != 0) {
        vfree(g_last_cpu_cnt);
        g_last_cpu_cnt = NULL;
        vfree(g_cpu_utilization);
        g_cpu_utilization = NULL;
        soc_misc_drv_err("Dms timer task register failed. (ret=%d)\n", ret);
        return ret;
    }
    CALL_INIT_MODULE(DMS_SOC_CPU_UTILIZATION);
    soc_misc_drv_info("Init success. (online_cpu_num=%u)\n", g_cpu_num);
    return 0;
}

void soc_misc_cpu_utilization_exit(void)
{
    int ret;

    CALL_EXIT_MODULE(DMS_SOC_CPU_UTILIZATION);

    ret = dms_timer_task_unregister(g_cpu_utilization_timer);
    if (ret) {
        soc_misc_drv_err("Dms timer task unregister failed. (ret=%d)\n", ret);
    }

    vfree(g_last_cpu_cnt);
    g_last_cpu_cnt = NULL;

    vfree(g_cpu_utilization);
    g_cpu_utilization = NULL;
    soc_misc_drv_info("SOC drv exit done.\n");
}

