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

#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/time.h>

#include "devdrv_common.h"
#include "devdrv_manager_common.h"
#include "devdrv_manager_ddr.h"
#include "devdrv_pm.h"
#include "devdrv_platform_resource.h"

#define FAILED (-1)
STATIC ddr_hw_info_t g_ddr_hw_info = { DDR_MAX_CHAN_NUM, 0xff,
#ifdef CFG_SOC_PLATFORM_CLOUD
{ DDRC_CHAN0_REG_BASE, DDRC_CHAN1_REG_BASE }
#else
#ifdef CFG_SOC_PLATFORM_MINIV2
    {   DDRC_CHAN0_REG_BASE, DDRC_CHAN1_REG_BASE,
        DDRC_CHAN2_REG_BASE, DDRC_CHAN3_REG_BASE,
        DDRC_CHAN4_REG_BASE, DDRC_CHAN5_REG_BASE,
        DDRC_CHAN6_REG_BASE, DDRC_CHAN7_REG_BASE,
        DDRC_CHAN8_REG_BASE, DDRC_CHAN9_REG_BASE,
        DDRC_CHAN10_REG_BASE, DDRC_CHAN11_REG_BASE,
        DDRC_CHAN12_REG_BASE, DDRC_CHAN13_REG_BASE,
        DDRC_CHAN14_REG_BASE, DDRC_CHAN15_REG_BASE,
        DDRC_CHAN16_REG_BASE, DDRC_CHAN17_REG_BASE,
        DDRC_CHAN18_REG_BASE, DDRC_CHAN19_REG_BASE,
        DDRC_CHAN20_REG_BASE, DDRC_CHAN21_REG_BASE,
        DDRC_CHAN22_REG_BASE, DDRC_CHAN23_REG_BASE,
    }
#else
    {   DDRC_CHAN0_REG_BASE, DDRC_CHAN1_REG_BASE,
        DDRC_CHAN2_REG_BASE, DDRC_CHAN3_REG_BASE,
        DDRC_CHAN4_REG_BASE, DDRC_CHAN5_REG_BASE,
        DDRC_CHAN6_REG_BASE, DDRC_CHAN7_REG_BASE
    }
#endif
#endif
};

STATIC ddr_prof_cfg_t g_ddr_prof_cfg[MAX_CHIP_NUM] = {0};
STATIC ddr_flux_info_t g_ddr_flux_info_bk[MAX_CHIP_NUM] = {0};
STATIC struct timespec64 g_time_stamp_start[MAX_CHIP_NUM] = {0};
STATIC struct mutex g_ddrc_reg_lock[MAX_CHIP_NUM];
STATIC struct mutex g_ddrc_init_lock[MAX_CHIP_NUM];
STATIC void __iomem *g_map_regs[MAX_CHIP_NUM][DDR_MAX_CHAN_NUM] = {NULL};

/*
 * ddrc register map
 */
int devdrv_ddrc_regs_map(void)
{
    unsigned int i;
    unsigned int j;

    for (i = 0; i < MAX_CHIP_NUM; i++) {
        /* initialize reg lock */
        mutex_init(&g_ddrc_reg_lock[i]);
    }

    for (i = 0; i < MAX_CHIP_NUM; i++) {
        /* initialize init lock */
        mutex_init(&g_ddrc_init_lock[i]);
    }

    for (i = 0; i < MAX_CHIP_NUM; i++) {
        for (j = 0; j < DDR_MAX_CHAN_NUM; j++) {
            g_map_regs[i][j] = ioremap(g_ddr_hw_info.reg_base[j] + i * DDRC_CHIP_REG_BASE_OFFSET, DDRC_MAP_SIZE);
            if (g_map_regs[i][j] == NULL) {
                devdrv_drv_err("ioremap failed.\n");
                devdrv_ddrc_regs_unmap();
                return -ENOMEM;
            }
        }
    }

    return 0;
}

/*
 * ddrc register unmap
 */
void devdrv_ddrc_regs_unmap(void)
{
    unsigned int i;
    unsigned int j;

    for (i = 0; i < MAX_CHIP_NUM; i++) {
        mutex_destroy(&g_ddrc_reg_lock[i]);
        mutex_destroy(&g_ddrc_init_lock[i]);
        for (j = 0; j < DDR_MAX_CHAN_NUM; j++) {
            if (g_map_regs[i][j] != NULL) {
                (void)iounmap(g_map_regs[i][j]);
                g_map_regs[i][j] = NULL;
            }
        }
    }
}

/*
 * register operation, read or write
 */
STATIC int devdrv_ddrc_reg_op(unsigned char op_type, const void __iomem *vir_addr, unsigned long reg_offset,
                              unsigned int *val)
{
    if (vir_addr == NULL) {
        devdrv_drv_err("vir_addr error.\n");
        *val = 0;
        return -EINVAL;
    }

    if (op_type == DDRC_REG_WR) {
        writel(*val, (void __iomem *)(uintptr_t)((long)(uintptr_t)vir_addr + reg_offset));
    } else {
        *val = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)vir_addr + reg_offset));
    }

    return 0;
}

/*
 * calculte flux difference between two flux value
 */
STATIC unsigned int devdrv_ddr_get_flux_diff(unsigned int start, unsigned int end)
{
    unsigned int flux_diff;

    flux_diff = (unsigned int)((unsigned long long)end + ((unsigned long long)UINT_TYPE_MAX + 1) - start) % \
        ((unsigned long long)UINT_TYPE_MAX + 1);

    return flux_diff;
}

/*
 * calculte time interval between start and end time.
 */
STATIC unsigned long devdrv_ddr_get_time_interval(struct timespec64 start, struct timespec64 end)
{
    unsigned long time_use;

    time_use = (end.tv_sec - start.tv_sec) * US_PER_SECOND  + (end.tv_nsec - start.tv_nsec) / NSEC_PER_USEC;
    return time_use;
}

/*
 * Enable ddrc flux statistic
 */
STATIC int devdrv_ddr_clear_flux_reg(const void __iomem *reg_base)
{
    int ret;
    unsigned int i = 0;
    unsigned int op_val = 0;
    unsigned long reg_offset[2] = { DDRC_HISI_FLUX_RD_OFFSET, DDRC_HISI_FLUX_WR_OFFSET };

    /* Write flux register to 0 */
    op_val = 0;
    for (i = 0; i < 2; i++) {
        ret = devdrv_ddrc_reg_op(DDRC_REG_WR, reg_base, reg_offset[i], &op_val);
        if (ret < 0) {
            devdrv_drv_err("devdrv_ddrc_reg_op failed ret = %d.\n", ret);
            return ret;
        }
    }

    return 0;
}

/*
 * Enable ddrc flux statistic
 */
STATIC int devdrv_ddr_set_master_id(unsigned int id, const void __iomem *reg_base)
{
    int ret;
    unsigned int op_val = 0;

    if (id != DDRC_MASTER_ID_ALL) {
        /* Set master id */
        op_val = DDRC_CFG_STAID_SET | (id << DDRC_CFG_STAID_OFFSET);
        ret = devdrv_ddrc_reg_op(DDRC_REG_WR, reg_base, DDRC_CFG_STAID, &op_val);
        if (ret < 0) {
            devdrv_drv_err("devdrv_ddrc_reg_op failed ret = %d.\n", ret);
            return ret;
        }

        /* Set master id mask */
        op_val = DDRC_CFG_STAIDMSK_SET;
        ret = devdrv_ddrc_reg_op(DDRC_REG_WR, reg_base, DDRC_CFG_STAIDMSK, &op_val);
        if (ret < 0) {
            devdrv_drv_err("devdrv_ddrc_reg_op failed ret = %d.\n", ret);
            return ret;
        }
    }

    return 0;
}

/*
 * Enable ddrc flux statistic
 */
STATIC int devdrv_ddr_flux_stat_enable(unsigned int device_id, const ddr_hw_info_t *info)
{
    int ret;
    unsigned int i;
    unsigned int op_val;
    void __iomem *reg_base = NULL;

    if (info == NULL) {
        devdrv_drv_err("devid %d invalid para.\n", device_id);
        return -EINVAL;
    }

    mutex_lock(&g_ddrc_reg_lock[device_id]);
    for (i = 0; i < DDR_MAX_CHAN_NUM; i++) {
        if (info->channel_bitmap & (1 << i)) {
            reg_base = g_map_regs[device_id][i];
            /* disable flux statistic */
            op_val = 0;
            ret = devdrv_ddrc_reg_op(DDRC_REG_WR, reg_base, DDRC_CTRL_PERF_OFFSET, &op_val);
            if (ret < 0) {
                mutex_unlock(&g_ddrc_reg_lock[device_id]);
                devdrv_drv_err("devid %d devdrv_ddrc_reg_op failed ret = %d.\n", device_id, ret);
                return ret;
            }

            ret = devdrv_ddr_set_master_id(g_ddr_prof_cfg[device_id].master_id, reg_base);
            if (ret < 0) {
                mutex_unlock(&g_ddrc_reg_lock[device_id]);
                devdrv_drv_err("devid %d devdrv_ddr_clear_flux_reg failed ret = %d.\n", device_id, ret);
                return ret;
            }

            ret = devdrv_ddr_clear_flux_reg(reg_base);
            if (ret < 0) {
                mutex_unlock(&g_ddrc_reg_lock[device_id]);
                devdrv_drv_err("devid %d devdrv_ddr_clear_flux_reg failed ret = %d.\n", device_id, ret);
                return ret;
            }

            /* Enable flux statistic */
            op_val = DDRC_PERF_EN_SET;
            ret = devdrv_ddrc_reg_op(DDRC_REG_WR, reg_base, DDRC_CTRL_PERF_OFFSET, &op_val);
            if (ret < 0) {
                mutex_unlock(&g_ddrc_reg_lock[device_id]);
                devdrv_drv_err("devid %d devdrv_ddrc_reg_op failed ret = %d.\n", device_id, ret);
                return ret;
            }
        }
    }

    mutex_unlock(&g_ddrc_reg_lock[device_id]);

    return 0;
}

/*
 * Initialize ddrc flux statistic
 */
EXPORT_SYMBOL(devdrv_ddr_stat_init);
int devdrv_ddr_stat_init(u32 device_id, u32 chan_num, u32 chan_bitmap)
{
    int ret;
    STATIC unsigned int init_flg[MAX_CHIP_NUM] = {0};

    if ((chan_num > DDR_MAX_CHAN_NUM) || (device_id >= MAX_CHIP_NUM)) {
        devdrv_drv_err("invalid para chan_num is %d, device_id=%u.\n", chan_num, device_id);
        return -EINVAL;
    }

    mutex_lock(&g_ddrc_init_lock[device_id]);
    if (init_flg[device_id] == 0) {
        g_ddr_hw_info.channel_num = chan_num;
        g_ddr_hw_info.channel_bitmap = chan_bitmap;
        g_ddr_prof_cfg[device_id].master_id = DDRC_MASTER_ID_ALL;
        ret = devdrv_ddr_flux_stat_enable(device_id, &g_ddr_hw_info);
        if (ret < 0) {
            mutex_unlock(&g_ddrc_init_lock[device_id]);
            devdrv_drv_err("devid %d devdrv_ddr_flux_stat_enable failed ret = %d.\n", device_id, ret);
            return ret;
        }

        init_flg[device_id] = 1;
    }
    mutex_unlock(&g_ddrc_init_lock[device_id]);

    return 0;
}

/*
 * Get ddrc flux statistic information
 */
STATIC int devdrv_ddr_get_stat_info(unsigned int device_id, ddr_flux_info_t *flux_info)
{
    int ret;
    unsigned int i = 0;
    unsigned int j;
    unsigned long reg_offset[DDRC_INFO_ITEM_NUM] = {
        DDRC_HISI_FLUX_RD_OFFSET,
        DDRC_HISI_FLUX_WR_OFFSET,
        DDRC_HISI_FLUXID_WR_OFFSET,
        DDRC_HISI_FLUXID_RD_OFFSET
    };
    unsigned int *val[DDRC_INFO_ITEM_NUM] = {NULL};
    struct timespec64 tmp_timestamp = {0};

    val[i++] = flux_info->flux_rd;
    val[i++] = flux_info->flux_wr;
    val[i++] = flux_info->id_flux_wr;
    val[i++] = flux_info->id_flux_rd;

    ktime_get_raw_ts64(&tmp_timestamp);
    for (i = 0; i < DDR_MAX_CHAN_NUM; i++) {
        /* channel bitmap is zero, set default value */
        if ((g_ddr_hw_info.channel_bitmap & (1 << i)) == 0) {
            flux_info->time_stamp[i].tv_sec = 0;
            flux_info->time_stamp[i].tv_nsec = 0;
            flux_info->flux_rd[i] = 0;
            flux_info->flux_wr[i] = 0;
            flux_info->id_flux_rd[i] = 0;
            flux_info->id_flux_wr[i] = 0;
            continue;
        }

        flux_info->time_stamp[i] = tmp_timestamp;
        for (j = 0; j < DDRC_INFO_ITEM_NUM; j++) {
            ret = devdrv_ddrc_reg_op(DDRC_REG_RD, g_map_regs[device_id][i], reg_offset[j], &val[j][i]);
            if (ret < 0) {
                devdrv_drv_err("devid %d devdrv_ddrc_reg_op failed ret = %d.\n", device_id, ret);
                return ret;
            }
        }
    }

    return 0;
}

/*
 * Update profiling buff index
 */
STATIC ddr_prof_data_t *devdrv_ddr_update_buff_index(ddr_prof_data_t *buf, int *data_len, int max_len)
{
    ddr_prof_data_t *prof_data_pt = NULL;

    *data_len += sizeof(ddr_prof_data_t);

    if (*data_len < max_len) {
        prof_data_pt = (ddr_prof_data_t *)(uintptr_t)((unsigned long)(uintptr_t)buf + *data_len);
    } else {
        devdrv_drv_err("buffer is overflow.\n");
        return NULL;
    }

    return prof_data_pt;
}

/*
 * check user configure data
 */
STATIC int devdrv_ddr_check_usrdata(const ddr_prof_cfg_t *data)
{
    ddr_prof_cfg_t *prof_cfg_pt = (ddr_prof_cfg_t *)data;

    /* check event num */
    if ((prof_cfg_pt->event_num == 0) || (prof_cfg_pt->event_num > MAX_EVENT_NUM)) {
        devdrv_drv_err("cfg event_num error event_num %d.\n", prof_cfg_pt->event_num);
        return FAILED;
    }

    /* check event value */
    if (((prof_cfg_pt->event_num == 1) && (prof_cfg_pt->event[0] >= MAX_EVENT_NUM)) ||
        ((prof_cfg_pt->event_num == MAX_EVENT_NUM) &&
        ((prof_cfg_pt->event[0] >= MAX_EVENT_NUM) || (prof_cfg_pt->event[1] >= MAX_EVENT_NUM)))) {
        devdrv_drv_err("event value error.\n");
        return FAILED;
    }

    /* check master id */
    if ((prof_cfg_pt->master_id != DDRC_MASTER_ID_ALL) && (prof_cfg_pt->master_id >= DDR_MAX_CHAN_NUM)) {
        devdrv_drv_err("master id value %d error.\n", prof_cfg_pt->master_id);
        return FAILED;
    }

    return 0;
}

/*
 * Set ddr profiling data buffer, for the user.
 */
STATIC int devdrv_ddr_prof_set_buff(unsigned int device_id, void *buf, int len, ddr_flux_info_t *info)
{
    ddr_prof_data_t *prof_data_pt = NULL;
    int data_len = 0;
    unsigned int i;
    unsigned int j;

    prof_data_pt = (ddr_prof_data_t *)buf;
    for (i = 0; i < g_ddr_prof_cfg[device_id].event_num; i++) {
        for (j = 0; j < DDR_MAX_CHAN_NUM; j++) {
            if ((g_ddr_hw_info.channel_bitmap & (1 << j)) == 0) {
                continue;
            }

            /* set write event data */
            if (g_ddr_prof_cfg[device_id].event[i] == DDR_PROF_EVENT_WR) {
                prof_data_pt->event = DDR_PROF_EVENT_WR;
                prof_data_pt->count = info->flux_wr[j];
            } else {
                prof_data_pt->event = DDR_PROF_EVENT_RD;
                prof_data_pt->count = info->flux_rd[j];
            }

            prof_data_pt->timestamp = (u32)devdrv_ddr_get_time_interval(g_time_stamp_start[device_id], 
                                                                        info->time_stamp[j]);
            prof_data_pt->ddr_id = j;
            prof_data_pt->master_id = DDRC_MASTER_ID_ALL;
            prof_data_pt = devdrv_ddr_update_buff_index((ddr_prof_data_t *)buf, &data_len, len);
            if (prof_data_pt == NULL) {
                devdrv_drv_err("devid %d buffer is overflow.\n", device_id);
                return len;
            }

            /* if master id is set, set id event */
            if (g_ddr_prof_cfg[device_id].master_id == DDRC_MASTER_ID_ALL) {
                continue;
            }

            if (g_ddr_prof_cfg[device_id].event[i] == DDR_PROF_EVENT_WR) {
                prof_data_pt->event = DDR_PROF_EVENT_WR;
                prof_data_pt->count = info->flux_wr[j];
            } else {
                prof_data_pt->event = DDR_PROF_EVENT_RD;
                prof_data_pt->count = info->flux_rd[j];
            }

            prof_data_pt->timestamp = (u32)devdrv_ddr_get_time_interval(g_time_stamp_start[device_id],
                                                                        info->time_stamp[j]);
            prof_data_pt->ddr_id = j;
            prof_data_pt->master_id = g_ddr_prof_cfg[device_id].master_id;

            prof_data_pt = devdrv_ddr_update_buff_index(buf, &data_len, len);
            if (prof_data_pt == NULL) {
                devdrv_drv_err("devid %d buffer is overflow.\n", device_id);
                return len;
            }
        }
    }

    return data_len;
}

STATIC unsigned long devdrv_ddr_get_max_bandwidth(unsigned int channel_num, unsigned int freq)
{
    unsigned long max_bandwidth = 0;

#ifdef CFG_SOC_PLATFORM_MINIV2
    max_bandwidth = (unsigned long)freq * DDRC_FREQ_DOUBLE * DDRC_DQ_NUM / 8; /* 8 bps per byte/s */
#else
    max_bandwidth = (unsigned long)channel_num * DDRC_BANDWIDTH_PER_CHAN;
#endif

    return max_bandwidth;
}

/*
 * DDR profiling sample func, callback for the profiling driver.
 */
EXPORT_SYMBOL(devdrv_ddr_prof_sample);
int devdrv_ddr_prof_sample(struct prof_peri_para para)
{
    int ret;
    ddr_flux_info_t cur_flux_info = {0};
    ddr_flux_info_t tx_flux_info = {0};
    ddr_flux_info_t *bk_info_pt = NULL;
    unsigned int device_id = para.device_id;
    void *buf = para.buff;
    int len = para.buff_len;
    unsigned int i;
    int buf_data_len;

    if ((buf == NULL) || (device_id >= MAX_CHIP_NUM)) {
        devdrv_drv_err("invalid para buf is %pK device_id=%u.\n", buf, device_id);
        return FAILED;
    }

    /* 1. get ddr flux statistic info */
    ret = devdrv_ddr_get_stat_info(device_id, &cur_flux_info);
    if (ret < 0) {
        devdrv_drv_err("devid %d devdrv_ddr_get_stat_info error ret = %d.\n", device_id, ret);
        return ret;
    }

    /* 2. calculate flux difference and record it */
    for (i = 0; i < DDR_MAX_CHAN_NUM; i++) {
        tx_flux_info.flux_rd[i] = devdrv_ddr_get_flux_diff(g_ddr_flux_info_bk[device_id].flux_rd[i],
                                                           cur_flux_info.flux_rd[i]);
        tx_flux_info.flux_wr[i] = devdrv_ddr_get_flux_diff(g_ddr_flux_info_bk[device_id].flux_wr[i],
                                                           cur_flux_info.flux_wr[i]);
        tx_flux_info.id_flux_rd[i] = devdrv_ddr_get_flux_diff(g_ddr_flux_info_bk[device_id].id_flux_rd[i],
                                                              cur_flux_info.id_flux_rd[i]);
        tx_flux_info.id_flux_wr[i] = devdrv_ddr_get_flux_diff(g_ddr_flux_info_bk[device_id].id_flux_wr[i],
                                                              cur_flux_info.id_flux_wr[i]);
        tx_flux_info.time_stamp[i] = cur_flux_info.time_stamp[i];
    }

    /* 3. backup current flux info */
    bk_info_pt = &(g_ddr_flux_info_bk[device_id]);
    ret = memcpy_s(bk_info_pt, sizeof(ddr_flux_info_t), &cur_flux_info, sizeof(ddr_flux_info_t));
    if (ret != 0) {
        devdrv_drv_err("devid %d memcpy_s failed: %d.\n", device_id, ret);
        return ret;
    }

    /* 4. set data buff */
    buf_data_len = devdrv_ddr_prof_set_buff(device_id, buf, len, &tx_flux_info);

    return buf_data_len;
}

/*
 * Get DDR profiling cfg data, callback for the profiling driver.
 */
EXPORT_SYMBOL(devdrv_ddr_prof_sample_userdata);
int devdrv_ddr_prof_sample_userdata(struct prof_peri_para para)
{
    unsigned int device_id = para.device_id;
    void *data = para.user_data;
    unsigned int data_size = para.user_data_len;
    int ret;

    if ((data == NULL) || (device_id >= MAX_CHIP_NUM) ||
        ((data_size != sizeof(ddr_prof_cfg_t)) && (data_size != (sizeof(ddr_prof_cfg_t) - sizeof(unsigned int))))) {
        devdrv_drv_err("invalid para data is %pK, device_id=%u, data_size: %u.\n", data, device_id, data_size);
        return FAILED;
    }

#if ((defined CFG_SOC_PLATFORM_CLOUD) || (defined CFG_SOC_PLATFORM_MINIV2))
    ret = devdrv_ddr_stat_init(device_id, DDR_MAX_CHAN_NUM, DDR_CHAN_BITMAP);
    if (ret < 0) {
        devdrv_drv_err("devid %d devdrv_ddr_stat_init error ret = %d.\n", device_id, ret);
        return ret;
    }
#endif

    ret = devdrv_ddr_check_usrdata((ddr_prof_cfg_t *)data);
    if (ret < 0) {
        devdrv_drv_err("devid %d profiling cfg data error ret = %d.\n", device_id, ret);
        return ret;
    }

    ret = memcpy_s(&g_ddr_prof_cfg[device_id], sizeof(ddr_prof_cfg_t), (void *)data, data_size);
    if (ret != OK) {
        devdrv_drv_err("devid %d memcpy_s return error: %d.\n", device_id, ret);
        return FAILED;
    }

    devdrv_drv_info("device_id: %d.\n", device_id);
    devdrv_drv_info("g_ddr_prof_cfg.period: %d.\n", g_ddr_prof_cfg[device_id].period);
    devdrv_drv_info("g_ddr_prof_cfg.master_id: %d.\n", g_ddr_prof_cfg[device_id].master_id);
    devdrv_drv_info("g_ddr_prof_cfg.event_num: %d.\n", g_ddr_prof_cfg[device_id].event_num);
    devdrv_drv_info("g_ddr_prof_cfg.event[0]: %d.\n", g_ddr_prof_cfg[device_id].event[0]);
    devdrv_drv_info("g_ddr_prof_cfg.event[1]: %d.\n", g_ddr_prof_cfg[device_id].event[1]);

    /* if master id is invalid,set master id and restart flux statistics */
    if (g_ddr_prof_cfg[device_id].master_id != DDRC_MASTER_ID_ALL) {
        ret = devdrv_ddr_flux_stat_enable(device_id, (const ddr_hw_info_t *)&g_ddr_hw_info);
        if (ret < 0) {
            devdrv_drv_err("devid %d devdrv_ddr_flux_stat_enable error ret = %d.\n", device_id, ret);
            return ret;
        }
    }

    /* get statistic info and fill backup struct after statistic enable */
    ret = devdrv_ddr_get_stat_info(device_id, &g_ddr_flux_info_bk[device_id]);
    if (ret < 0) {
        devdrv_drv_err("devid %d devdrv_ddr_get_stat_info error ret = %d.\n", device_id, ret);
        return ret;
    }

    g_time_stamp_start[device_id] = g_ddr_flux_info_bk[device_id].time_stamp[0];

    return 0;
}

/*
 * Get DDR bandwidth utilization rate, used by dsmi module
 */
EXPORT_SYMBOL(devdrv_ddr_get_bandwidth_util_rate);
int devdrv_ddr_get_bandwidth_util_rate(unsigned int device_id, unsigned int freq, unsigned int *rate)
{
    int ret;
    ddr_flux_info_t ddr_flux_info[DDRC_BW_ITEM_NUM] = {0};
    ddr_flux_info_t *flux_info_pt = NULL;
    unsigned long total_flux = 0;
    unsigned int i;
    unsigned long total_time_stamp = 0;
    unsigned long ddr_bandwidth = 0;
    unsigned long actual_ddr_bandwidth = 0;

    if ((rate == NULL) || (device_id >= MAX_CHIP_NUM)) {
        devdrv_drv_err("invalid para data is %pK device_id=%u.\n", rate, device_id);
        return FAILED;
    }

    mutex_lock(&g_ddrc_reg_lock[device_id]);

    flux_info_pt = &ddr_flux_info[0];
    ret = devdrv_ddr_get_stat_info(device_id, flux_info_pt);
    if (ret < 0) {
        mutex_unlock(&g_ddrc_reg_lock[device_id]);
        devdrv_drv_err("devid %d devdrv_ddr_get_stat_info error ret = %d.\n", device_id, ret);
        return ret;
    }

    msleep(DDRC_FLUX_STAT_TIME);

    flux_info_pt = &ddr_flux_info[1];
    ret = devdrv_ddr_get_stat_info(device_id, flux_info_pt);
    if (ret < 0) {
        mutex_unlock(&g_ddrc_reg_lock[device_id]);
        devdrv_drv_err("devid %d devdrv_ddr_get_stat_info error ret = %d.\n", device_id, ret);
        return ret;
    }

    for (i = 0; i < DDR_MAX_CHAN_NUM; i++) {
        /* calculate total flux, including rollover case */
        total_flux += devdrv_ddr_get_flux_diff(ddr_flux_info[0].flux_rd[i], ddr_flux_info[1].flux_rd[i]);
        total_flux += devdrv_ddr_get_flux_diff(ddr_flux_info[0].flux_wr[i], ddr_flux_info[1].flux_wr[i]);

        total_time_stamp += devdrv_ddr_get_time_interval(ddr_flux_info[0].time_stamp[i],
                                                         ddr_flux_info[1].time_stamp[i]);
    }

    total_time_stamp = total_time_stamp / g_ddr_hw_info.channel_num;

    ddr_bandwidth = devdrv_ddr_get_max_bandwidth(g_ddr_hw_info.channel_num, freq);

    /* actual bandwidth formulate */
    if (total_time_stamp > 0) {
        actual_ddr_bandwidth = total_flux * DDRC_TIME_STAMP_PER_SECOND / (total_time_stamp * DDRC_FLUX_MB_DIV);
    } else {
        mutex_unlock(&g_ddrc_reg_lock[device_id]);
        devdrv_drv_err("devid %d timestamp error.\n", device_id);
        return FAILED;
    }

    if (ddr_bandwidth != 0) {
        /* ddr bandwidth rate = actual bandwidth/theory bandwidth */
        *rate = (actual_ddr_bandwidth * DDRC_STAT_PERTAGE) / ddr_bandwidth;
    } else {
        devdrv_drv_err("ddr_bandwidth (%lu) value is zero, channel_num = %d .\n", ddr_bandwidth,
                g_ddr_hw_info.channel_num);
        mutex_unlock(&g_ddrc_reg_lock[device_id]);
        return FAILED;
    }

    mutex_unlock(&g_ddrc_reg_lock[device_id]);

    return 0;
}

/*
 * get ecc info
 */
#if ((defined CFG_SOC_PLATFORM_MINI) && (!defined CFG_SOC_PLATFORM_MINIV2))
int devdrv_ddr_get_ecc_info(unsigned int device_id, unsigned int bitmap, unsigned int *ecc_result, unsigned int num)
{
    int ret;
    unsigned int i;
    unsigned int reg_val_1 = 0;
    unsigned int reg_val_2 = 0;
    unsigned int ecc_err_bit_statis[DDR_ECC_RESULT_NUM] = {0};

    if ((ecc_result == NULL) || (num < DDR_ECC_RESULT_NUM)) {
        devdrv_drv_err("devid %d , invalid param: (ecc_result == NULL)=%d or ecc result array num %d err.\n",
            device_id, (ecc_result == NULL), num);
        return FAILED;
    }

    for (i = 0; i < DDR_MAX_CHAN_NUM; i++) {
        if ((1 << i) & bitmap) {
            ret = devdrv_ddrc_reg_op(DDRC_REG_RD, g_map_regs[device_id][i], DDR_ECC_OFFSET1, &reg_val_1);
            if (ret < 0) {
                devdrv_drv_err("devid %d devdrv_ddrc_reg_op err ret = %d.\n", device_id, ret);
                return ret;
            }

            ret = devdrv_ddrc_reg_op(DDRC_REG_RD, g_map_regs[device_id][i], DDR_ECC_OFFSET2, &reg_val_2);
            if (ret < 0) {
                devdrv_drv_err("devid %d devdrv_ddrc_reg_op err ret = %d.\n", device_id, ret);
                return ret;
            }

            ecc_err_bit_statis[0] = ecc_err_bit_statis[0] + reg_val_1;
            ecc_err_bit_statis[1] = ecc_err_bit_statis[1] + reg_val_2;
        }
    }

    ecc_result[0] = ecc_err_bit_statis[0];
    ecc_result[1] = ecc_err_bit_statis[1];

    return 0;
}
#endif
