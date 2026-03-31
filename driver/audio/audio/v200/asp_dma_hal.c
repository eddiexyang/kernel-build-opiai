/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/hwspinlock.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include "ot_type.h"
#include "ai_drv.h"
#include "ao_drv.h"
#include "mm_ext.h"
#include "asp_dma.h"
#include "asp_dma_hal.h"
#include <linux/kernel.h>
#include <linux/time.h>
#include <securec.h>
#define DMA_LLI_ENABLE 0x2
#define DMA_CHANNEL_MAX 16
#define PCM_PORTS_NUM 1
#define PCM_PORTS_MAX 4

#define AI_DMA_LLI_NUM 2
#define AO_DMA_LLI_NUM 2

enum {
    PCM_PORTS_I2S1_UL, // 0 -> 14
    PCM_PORTS_I2S1_DL,
    PCM_PORTS_I2S2_UL,
    PCM_PORTS_I2S2_DL,
    PCM_PORTS_ANA_UL,
    PCM_PORTS_ANA_DL,
    PCM_PORT_HDMI0_DL,
    PCM_PORT_HDMI1_DL,
    PCM_PORTS_BUTT
};

enum hi3xxx_asp_dmac_status {
    STATUS_DMAC_STOP = 0,
    STATUS_DMAC_RUNNING
};
struct pcm_ports {
    unsigned int ports_tbl_idx; /* device ports table index */
    unsigned int ports_cnt; /* ports number */
};

struct dma_config {
    td_u64 port;        /* port addr */
    td_u32 config;      /* dma config */
    td_u32 channel;     /* dma channel */
};

struct hi3xxx_asp_dmac_runtime_data {
    spinlock_t lock;
    struct mutex mutex;
    enum hi3xxx_asp_dmac_status status;
    unsigned int dma_addr;
    unsigned int buf_size;  // ao ai申请的dma总长度
    unsigned int dma_size;  // 一次dma中断传输的长度
    unsigned int dma_buf_next[PCM_PORTS_NUM];
    unsigned int irq_cnt;
    struct pcm_ports ports;
    struct dma_config dma_cfg[PCM_PORTS_NUM];
    struct dma_lli_cfg *pdma_lli_cfg[PCM_PORTS_NUM];
    td_u32 lli_dma_addr[PCM_PORTS_NUM];
    unsigned int (*pcm_port_tbl)[PCM_PORTS_MAX];
    struct dma_config *dma_cfg_tbl;
};

struct hi3xxx_asp_dmac_runtime_data *g_prtd_play[OT_AO_DEV_MAX_NUM];
struct hi3xxx_asp_dmac_runtime_data *g_prtd_capture[OT_AI_DEV_MAX_NUM];

static unsigned int pcm_port_tbl_6405[PCM_PORTS_BUTT][PCM_PORTS_MAX] = {
    {14},                  /* PCM_DEVICE_VOICE_UL_PORT_NUM */
    {7},                   /* PCM_DEVICE_VOICE_DL_PORT_NUM */
    {15},                   /* PCM_DEVICE_AUDIO_UL_PORT_NUM */
    {6},                    /* PCM_DEVICE_AUDIO_DL_PORT_NUM */
    {0},
    {1},
    {8},
    {9},
};
static struct dma_config dma_cfg_i2s_6405[DMA_CHANNEL_MAX] = {
    {0x4030c2000, 0x43333127,  2},   /* SPA_UPLINK_1_2  --- 15 */
    {0x4030c2000, 0x83333157,  3},   /* SPA_UPLINK_3_4  --- 14 */
    {0x403152308, 0x433220d7,  7},   /* SPA_UPLINK_5_6  --- 13 */
    {0x40315230C, 0x433220c7,  9},   /* SPA_UPLINK_7_8  --- 12 */
    {0x403152310, 0x433220b7,  4},   /* SPA1_UPLINK     --- 11 */
    {0x403152314, 0x433220a7,  5},   /* SPA2_UPLINK     --- 10 */
    {0x403152318, 0x87732037,  7},   /* CODEC_3RD_DLINK --- 3  */
    {0x40315231c, 0x87722097,  1},   /* dev 0 play */
    {0x4030c0000, 0x83333137,  4},   /* hdmi0      --- 0  */
    {0x4030c1000, 0x83333147,  5},   /* hdmi1    --- 8  */
    {0x403152328, 0x47732077, 10},   /* VOICE_UPLINK    --- 7  */
    {0x40315232C, 0x43322067, 11},   /* MIC3_4_UPLINK   --- 6  */
    {0x403152330, 0x43322057, 12},   /* MIC5_6_UPLINK   --- 5  */
    {0x403152334, 0x43322047, 13},   /* MIC7_8_UPLINK   --- 4  */
    {0x403152304, 0x477220e7,  0},   /* dev 0 cap */
    {0x403152338, 0x47722017,  6},   /* MDM_DLINK       --- 2  */
};

td_s32 asp_dmac_trigger(ot_audio_dev audio_dev_id, td_bool is_ao)
{
    int ret = 0;
    struct hi3xxx_asp_dmac_runtime_data *prtd = NULL;
    unsigned int ports_cnt;
    unsigned int port_index;

    if (is_ao == TD_TRUE) {
        prtd = g_prtd_play[audio_dev_id];
    } else {
        prtd = g_prtd_capture[audio_dev_id];
    }
    ports_cnt = prtd->ports.ports_cnt;

    aio_info_trace("asp_dmac_trigger entry, ports_cnt = %u\n", ports_cnt);

    prtd->status = STATUS_DMAC_RUNNING;
    for (port_index = 0; port_index < ports_cnt; port_index++) {
        ret = asp_dma_start(0, prtd->dma_cfg[port_index].channel, prtd->pdma_lli_cfg[port_index]);
        if (ret < 0) {
            aio_err_trace("[%s:%d] dma channel %u start failed\n", __func__,
                          __LINE__, prtd->dma_cfg[port_index].channel);
            return ret;
        }
    }
    return ret;
}
EXPORT_SYMBOL(asp_dmac_trigger);

td_s32 asp_dmac_stop(ot_audio_dev audio_dev_id, td_bool is_ao)
{
    struct hi3xxx_asp_dmac_runtime_data *prtd = NULL;
    unsigned int ports_cnt;
    unsigned int port_index;
    unsigned int lli_index;
    unsigned int dma_lli_num;

    if (is_ao == TD_TRUE) {
        prtd = g_prtd_play[audio_dev_id];
    } else {
        prtd = g_prtd_capture[audio_dev_id];
    }

    dma_lli_num = prtd->buf_size / prtd->dma_size;
    ports_cnt = prtd->ports.ports_cnt;

    spin_lock(&prtd->lock);

    prtd->status = STATUS_DMAC_RUNNING;
    spin_unlock(&prtd->lock);

    for (port_index = 0; port_index < ports_cnt; port_index++) {
        asp_dma_stop(0, prtd->dma_cfg[port_index].channel);
    }
    spin_lock(&prtd->lock);

    prtd->irq_cnt = 0;
    prtd->status = STATUS_DMAC_STOP;
    spin_unlock(&prtd->lock);
    mutex_destroy(&prtd->mutex);
    kfree(prtd);
    return 0;
}
EXPORT_SYMBOL(asp_dmac_stop);

static void asp_dma_set_lli_node(struct hi3xxx_asp_dmac_runtime_data *prtd,
                                 unsigned int port_index, unsigned int lli_index, td_bool is_ao)
{
    unsigned int ports_cnt = prtd->ports.ports_cnt;
    unsigned int dma_size = prtd->dma_size;
    unsigned int dma_buf_num = 0;

    aio_info_trace("set dma buf[%u/%u] dma_size: %u\n", prtd->dma_buf_next[port_index], dma_buf_num, dma_size);

    if (is_ao == TD_TRUE) {
        // 现在偏移1个dma_size
        dma_buf_num = AO_DMA_LLI_NUM;
        prtd->pdma_lli_cfg[port_index][lli_index].src_addr = prtd->dma_addr + prtd->dma_buf_next[port_index] * dma_size;
    } else {
        dma_buf_num = AI_DMA_LLI_NUM;
        prtd->pdma_lli_cfg[port_index][lli_index].des_addr = prtd->dma_addr + prtd->dma_buf_next[port_index] * dma_size;
    }

    prtd->dma_buf_next[port_index] = (prtd->dma_buf_next[port_index] + ports_cnt) % dma_buf_num;
}

static void asp_dma_lli_cfg(struct hi3xxx_asp_dmac_runtime_data *prtd, unsigned int port_index, td_bool is_ao)
{
    unsigned int lli_index;
    unsigned int next_addr = 0x0;
    unsigned int config = 0x0;
    unsigned int dma_lli_num;
    td_u32 tx_dma_addr = 0x0;
    td_u32 rx_dma_addr = 0x0;
    unsigned int dma_size = prtd->dma_size;
    unsigned int dma_lli_size;

    config = prtd->dma_cfg[port_index].config;
    if (is_ao == TD_TRUE) {
        tx_dma_addr = prtd->dma_cfg[port_index].port;
        dma_lli_num = AO_DMA_LLI_NUM;
    } else {
        rx_dma_addr = prtd->dma_cfg[port_index].port;
        dma_lli_num = AI_DMA_LLI_NUM;
    }

    dma_lli_size = dma_lli_num * (td_u32)sizeof(struct dma_lli_cfg);

    aio_info_trace("asp_dma_lli_cfg in 1111, dma_lli_num = %u", dma_lli_num);
    // 为什么录音和播音都设置两个链表
    (void)memset_s(prtd->pdma_lli_cfg[port_index], dma_lli_size, 0, dma_lli_size);
    for (lli_index = 0; lli_index < dma_lli_num; lli_index++) {
        next_addr = (unsigned int)(prtd->lli_dma_addr[port_index] + sizeof(struct dma_lli_cfg) * (lli_index + 1));
        prtd->pdma_lli_cfg[port_index][lli_index].lli       = next_addr | DMA_LLI_ENABLE; // 这个是以为链表的下一个
        prtd->pdma_lli_cfg[port_index][lli_index].config    = config;
        prtd->pdma_lli_cfg[port_index][lli_index].des_addr  = tx_dma_addr;   // fifo的地址
        prtd->pdma_lli_cfg[port_index][lli_index].src_addr  = rx_dma_addr;
        prtd->pdma_lli_cfg[port_index][lli_index].a_count   = dma_size; // trans len
        /* reset the src or dest addr of one dma list */
        asp_dma_set_lli_node(prtd, port_index, lli_index, is_ao);
    }

    prtd->pdma_lli_cfg[port_index][dma_lli_num - 1].lli = prtd->lli_dma_addr[port_index] | (td_u32)DMA_LLI_ENABLE;

    for (lli_index = 0; lli_index < dma_lli_num; lli_index++) {
        aio_info_trace("port[%u] node[%u] lli = 0x%x, count = %u, src_addr = 0x%x, des_addr = 0x%x, config = 0x%x\n",
                       port_index, lli_index, prtd->pdma_lli_cfg[port_index][lli_index].lli,
                       prtd->pdma_lli_cfg[port_index][lli_index].a_count,
                       prtd->pdma_lli_cfg[port_index][lli_index].src_addr,
                       prtd->pdma_lli_cfg[port_index][lli_index].des_addr,
                       prtd->pdma_lli_cfg[port_index][lli_index].config);
    }
}

static void hi3xxx_asp_dmac_get_dma_cfg(struct hi3xxx_asp_dmac_runtime_data *prtd, unsigned int port_index)
{
    struct dma_config *portDmaCfg = &(prtd->dma_cfg[port_index]);  // 把prtd->dma_cfg[port_index]地址赋给portDmaCfg
    unsigned int portNum = prtd->pcm_port_tbl[prtd->ports.ports_tbl_idx][port_index];

    portDmaCfg->port = prtd->dma_cfg_tbl[portNum].port;
    portDmaCfg->config = prtd->dma_cfg_tbl[portNum].config;
    portDmaCfg->channel = prtd->dma_cfg_tbl[portNum].channel;
    aio_info_trace("[%s:%d] port = 0x%llx, config = 0x%x, channel = %u\n", __func__, __LINE__,
        prtd->dma_cfg[port_index].port, prtd->dma_cfg[port_index].config, prtd->dma_cfg[port_index].channel);
}

void hi3xxx_asp_dmac_prepare(ot_audio_dev audio_dev_id, td_bool is_ao)
{
    unsigned int ports_cnt = 0;
    unsigned int port_index;
    int ret;

    struct hi3xxx_asp_dmac_runtime_data *prtd = NULL;

    if (is_ao == TD_TRUE) {
        prtd = g_prtd_play[audio_dev_id];
    } else {
        prtd = g_prtd_capture[audio_dev_id];
    }

    ports_cnt = prtd->ports.ports_cnt;

    mutex_lock(&prtd->mutex); // ao和ai都会调用

    prtd->status = STATUS_DMAC_STOP;
    for (port_index = 0; port_index < ports_cnt; port_index++) {
        prtd->dma_buf_next[port_index] = port_index;
    }

    for (port_index = 0; port_index < ports_cnt; port_index++) {
        hi3xxx_asp_dmac_get_dma_cfg(prtd, port_index);
        asp_dma_lli_cfg(prtd, port_index, is_ao);

        ret = asp_dma_config(0, prtd->dma_cfg[port_index].channel, prtd->pdma_lli_cfg[port_index], is_ao);
        if (ret != 0) {
            aio_err_trace("asp dma config fail in asp dmc prepare\n");
        }
    }

    mutex_unlock(&prtd->mutex);
}
EXPORT_SYMBOL(hi3xxx_asp_dmac_prepare);

static td_u32 get_port_cnt(td_u32 chn_cnt)
{
    if (chn_cnt >= CHN_CNT_NUM_01 && chn_cnt <= CHN_CNT_NUM_04) {
        return CHN_CNT_NUM_01;
    } else if (chn_cnt >= CHN_CNT_NUM_05 && chn_cnt <= CHN_CNT_NUM_08) {
        return CHN_CNT_NUM_02;
    } else if (chn_cnt >= CHN_CNT_NUM_09 && chn_cnt <= CHN_CNT_NUM_12) {
        return CHN_CNT_NUM_03;
    } else if (chn_cnt >= CHN_CNT_NUM_13 && chn_cnt <= CHN_CNT_NUM_16) {
        return CHN_CNT_NUM_04;
    }
    return CHN_CNT_NUM_01;
}

// dma 初始化（同时支持播放和录音）
// 变量分开，链表分开
// 通过audio_dev_id和chn_cnt来决定play_tal_idex和ports_capture_tbl_idx
td_s32 hi19xx_asp_dma_open(ot_audio_dev audio_dev_id, td_bool is_ao, td_u32 chn_cnt)
{
    struct hi3xxx_asp_dmac_runtime_data *prtd = NULL;

    if (is_ao == TD_TRUE) {
        prtd = g_prtd_play[audio_dev_id];
    } else {
        prtd = g_prtd_capture[audio_dev_id];
    }

    prtd->pcm_port_tbl = pcm_port_tbl_6405;
    prtd->dma_cfg_tbl = dma_cfg_i2s_6405;

    if (is_ao == TD_TRUE) {
        // playback
        if (audio_dev_id == I2S_DIGIT_0) {
            prtd->ports.ports_tbl_idx = PCM_PORTS_I2S1_DL; // i2s1
        } else if (audio_dev_id == I2S_DIGIT_1) {
            prtd->ports.ports_tbl_idx = PCM_PORTS_I2S2_DL; // i2s2
        } else if (audio_dev_id == I2S_ANA) {
            prtd->ports.ports_tbl_idx = PCM_PORTS_ANA_DL;  // ana
        } else if (audio_dev_id == I2S_HDMI_0) {
            prtd->ports.ports_tbl_idx = PCM_PORT_HDMI0_DL; // hdmi0
        } else if (audio_dev_id == I2S_HDMI_1) {
            prtd->ports.ports_tbl_idx = PCM_PORT_HDMI1_DL; // hdmi1
        }
    } else {
        // capture
        if (audio_dev_id == I2S_DIGIT_0) {
            prtd->ports.ports_tbl_idx = PCM_PORTS_I2S1_UL; // i2s1
        } else if (audio_dev_id == I2S_DIGIT_1) {
            prtd->ports.ports_tbl_idx = PCM_PORTS_I2S2_UL; // i2s2
        } else if (audio_dev_id == I2S_ANA) {
            prtd->ports.ports_tbl_idx = PCM_PORTS_ANA_UL; // ana
        }
    }

    return 0;
}
EXPORT_SYMBOL(hi19xx_asp_dma_open);

td_s32 hi19xx_asp_dma_init(aio_drv_dev_ctx *aio_drv_dev, td_u32 lli_offset, ot_audio_dev audio_dev_id, td_bool is_ao)
{
    td_u32 port_index;
    struct hi3xxx_asp_dmac_runtime_data *prtd = NULL;
    td_u8 *vir_addr = aio_drv_dev->cir_buf.vir_addr;
    td_u32 phy_addr = (td_u32)aio_drv_dev->cir_buf.phys_addr;
    td_u32 buf_size = aio_drv_dev->cir_buf.buf_size;
    td_u32 trans_len = aio_drv_dev->trans_len;
    td_u32 chn_cnt = aio_drv_dev->aio_attr.chn_cnt;

    if (is_ao == TD_TRUE) {
        g_prtd_play[audio_dev_id] = kzalloc(sizeof(struct hi3xxx_asp_dmac_runtime_data), GFP_KERNEL);
        if (NULL == g_prtd_play[audio_dev_id]) {
            aio_err_trace("kzalloc hi3xxx_asp_dmac_runtime_data play error!\n");
            return -ENOMEM;
        }

        prtd = g_prtd_play[audio_dev_id];

        mutex_init(&prtd->mutex);
        spin_lock_init(&prtd->lock);
        spin_lock(&prtd->lock);

        prtd->dma_addr = phy_addr;
        prtd->buf_size = buf_size;

        prtd->dma_size = trans_len;

        prtd->ports.ports_cnt = 1;
        for (port_index = 0; port_index < prtd->ports.ports_cnt; port_index++) {
            prtd->pdma_lli_cfg[port_index] = (struct dma_lli_cfg *)((vir_addr + lli_offset) +
                                (port_index * AO_DMA_LLI_NUM * ((td_u32)sizeof(struct dma_lli_cfg))));
            prtd->lli_dma_addr[port_index] =
                phy_addr + lli_offset + (port_index * AO_DMA_LLI_NUM * ((td_u32)sizeof(struct dma_lli_cfg)));
            aio_info_trace("ai lli vir addr = 0x%x, phy addr = 0x%x\n",
                (td_u32 *)((vir_addr + lli_offset) + (port_index * AO_DMA_LLI_NUM *
                ((td_u32)sizeof(struct dma_lli_cfg)))),
                (phy_addr + (port_index * AO_DMA_LLI_NUM * ((td_u32)sizeof(struct dma_lli_cfg)))));
        }
        spin_unlock(&prtd->lock);
    } else {
        g_prtd_capture[audio_dev_id] = kzalloc(sizeof(struct hi3xxx_asp_dmac_runtime_data), GFP_KERNEL);
        if (NULL == g_prtd_capture[audio_dev_id]) {
            aio_err_trace("kzalloc hi3xxx_asp_dmac_runtime_data capture error!\n");
            return -ENOMEM;
        }
        prtd = g_prtd_capture[audio_dev_id];

        mutex_init(&prtd->mutex);
        spin_lock_init(&prtd->lock);
        spin_lock(&prtd->lock);

        prtd->dma_addr = phy_addr;
        prtd->buf_size = buf_size;
        prtd->dma_size = trans_len;
        prtd->ports.ports_cnt = get_port_cnt(chn_cnt);   // dma通道数

        for (port_index = 0; port_index < prtd->ports.ports_cnt; port_index++) {
            prtd->pdma_lli_cfg[port_index] = (struct dma_lli_cfg *)((vir_addr + lli_offset) +
                                   (port_index * AI_DMA_LLI_NUM * ((td_u32)sizeof(struct dma_lli_cfg))));
            prtd->lli_dma_addr[port_index] = phy_addr + lli_offset +
                (port_index * AI_DMA_LLI_NUM * (td_u32)sizeof(struct dma_lli_cfg));
            aio_info_trace("ai lli vir addr = 0x%x, phy addr = 0x%x port_cnt = %u\n",
                (td_u32 *)((vir_addr + lli_offset) + (port_index * AO_DMA_LLI_NUM *
                ((td_u32)sizeof(struct dma_lli_cfg)))),
                (phy_addr + (port_index * AO_DMA_LLI_NUM * ((td_u32)sizeof(struct dma_lli_cfg)))),
                prtd->ports.ports_cnt);
        }

        spin_unlock(&prtd->lock);
    }
    return 0;
}
EXPORT_SYMBOL(hi19xx_asp_dma_init);

MODULE_AUTHOR("Hisilicon Technologies Co., Ltd");
MODULE_DESCRIPTION("ASP DMA HAL Driver");
MODULE_LICENSE("GPL");
