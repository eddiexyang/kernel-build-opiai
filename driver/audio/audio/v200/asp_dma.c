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
#include <linux/pm_wakeup.h>
#include <linux/interrupt.h>
#include <asp_dma.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <securec.h>
#include "hi_osal.h"
#include "mkp_aio.h"
#include "audio_fault.h"
#include "aiao_hal_comm.h"

#define ASP_DMAC_READ_DELAY 250

struct dma_callback {
    callback_t callback;
    td_bool para;
};

struct asp_dma_priv {
    struct device *dev;
    td_u32 dmac_num;
    td_u32 irq;
    spinlock_t lock;
    struct resource *res;
    struct hwspinlock *hwlock;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    struct wakeup_source *wake_lock;
#else
    struct wakeup_source wake_lock;
#endif
    void __iomem *asp_dma_reg_base_addr;
    struct dma_callback callback[ASP_DMA_MAX_CHANNEL_NUM];
};

static struct asp_dma_priv *g_asp_dma_priv[2] = { NULL };

td_u32 _dmac_reg_read(td_u32 dmac_num, td_u32 reg)
{
    struct asp_dma_priv *priv = g_asp_dma_priv[dmac_num];
    td_u32 ret;

    if (priv == NULL) {
        aio_err_trace("%s: priv is null\n", __FUNCTION__);
        return EINVAL;
    }

    ret = readl(priv->asp_dma_reg_base_addr + reg);
    return ret;
}

static void _dmac_reg_write(td_u32 dmac_num, td_u32 reg, td_u32 value)
{
    struct asp_dma_priv *priv = g_asp_dma_priv[dmac_num];

    if (priv == NULL) {
        aio_err_trace("%s: priv is null\n", __FUNCTION__);
        return;
    }

    writel(value, priv->asp_dma_reg_base_addr + reg);
}

static void _dmac_reg_set_bit(td_u32 dmac_num, td_u32 reg, td_u32 offset)
{
    struct asp_dma_priv *priv = g_asp_dma_priv[dmac_num];
    td_u32 value;
    unsigned long flag_sft = 0;

    if (priv == NULL) {
        aio_err_trace("%s: priv is null\n", __FUNCTION__);
        return;
    }

    spin_lock_irqsave(&priv->lock, flag_sft);

    value = readl(priv->asp_dma_reg_base_addr + reg);
    value |= (1U << offset);
    writel(value, priv->asp_dma_reg_base_addr + reg);

    spin_unlock_irqrestore(&priv->lock, flag_sft);
}

static void _dmac_reg_clr_bit(td_u32 dmac_num, td_u32 reg, td_u32 offset)
{
    struct asp_dma_priv *priv = g_asp_dma_priv[dmac_num];
    td_u32 value;
    unsigned long flag_sft = 0;

    if (priv == NULL) {
        aio_err_trace("%s: priv is null\n", __FUNCTION__);
        return;
    }

    spin_lock_irqsave(&priv->lock, flag_sft);

    value = readl(priv->asp_dma_reg_base_addr + reg);
    value &= ~(1U << offset);
    writel(value, priv->asp_dma_reg_base_addr + reg);

    spin_unlock_irqrestore(&priv->lock, flag_sft);
}

static void dmac_dump(td_u32 dmac_num, td_u32 dma_channel)
{
    if (dma_channel >= ASP_DMA_MAX_CHANNEL_NUM) {
        aio_err_trace("dma channel err:%u\n", dma_channel);
        return;
    }

    aio_info_trace("a count:0x%x\n", _dmac_reg_read(dmac_num, ASP_DMA_CX_CNT0(dma_channel)));
    aio_info_trace("src addr:0x%x\n", _dmac_reg_read(dmac_num, ASP_DMA_CX_SRC_ADDR(dma_channel)));
    aio_info_trace("des addr:0x%x\n", _dmac_reg_read(dmac_num, ASP_DMA_CX_DES_ADDR(dma_channel)));
    aio_info_trace("lli:0x%x\n", _dmac_reg_read(dmac_num, ASP_DMA_CX_LLI(dma_channel)));
    aio_info_trace("config:0x%x\n", _dmac_reg_read(dmac_num, ASP_DMA_CX_CONFIG(dma_channel)));

    aio_info_trace("c count:0x%x\n", _dmac_reg_read(dmac_num, ASP_DMA_CX_CNT1(dma_channel)));
    aio_info_trace("b index:0x%x\n", _dmac_reg_read(dmac_num, ASP_DMA_CX_BINDX(dma_channel)));
    aio_info_trace("c index:0x%x\n", _dmac_reg_read(dmac_num, ASP_DMA_CX_CINDX(dma_channel)));
}

static bool error_interrupt_handle(td_u32 int_type, td_u32 dma_channel)
{
    td_u32 count = 0;

    switch (int_type) {
        case ASP_DMA_INT_TYPE_ERR1:
            aio_err_trace("[%s:%d] dmac channel %u interrupt config error happend\n",
                          __func__, __LINE__, dma_channel);
            count++;
            break;
        case ASP_DMA_INT_TYPE_ERR2:
            aio_err_trace("[%s:%d] dmac channel %u interrupt transit error happend\n",
                          __func__, __LINE__, dma_channel);
            count++;
            break;
        case ASP_DMA_INT_TYPE_ERR3:
            aio_err_trace("[%s:%d] dmac channel %u interrupt read lli error happend\n",
                          __func__, __LINE__, dma_channel);
            count++;
            break;
        case ASP_DMA_INT_TYPE_TC1:
            aio_err_trace("[%s:%d] dmac channel %u transit finished\n", __func__, __LINE__, dma_channel);
            break;
        /* dma lli transit finish interrupt */
        case ASP_DMA_INT_TYPE_TC2:
            break;
        default:
            aio_info_trace("[%s:%d] dmac interrupt error type[%u]\n", __func__, __LINE__, int_type);
            count++;
            break;
    }

    if (unlikely(count > 0))
        return TD_TRUE;

    return TD_FALSE;
}

static td_s32 _asp_dmac_irq_handler(td_s32 irq, void *data)
{
    td_u32 err1;
    td_u32 err2;
    td_u32 err3;
    td_u32 tc1;
    td_u32 tc2;
    td_u32 int_mask;
    td_u32 int_state;
    td_u32 int_type;
    td_u32 i;
    td_u32 dmac_num = 0;
    struct asp_dma_priv *priv = g_asp_dma_priv[dmac_num];

    /* if have interupts */
    int_state = _dmac_reg_read(dmac_num, ASP_DMA_INT_STAT_AP);
    if (0 == int_state)
        return (td_s32)IRQ_HANDLED;

    /* read interupt states */
    err1 = _dmac_reg_read(dmac_num, ASP_DMA_INT_ERR1_AP);
    err2 = _dmac_reg_read(dmac_num, ASP_DMA_INT_ERR2_AP);
    err3 = _dmac_reg_read(dmac_num, ASP_DMA_INT_ERR3_AP);
    tc1  = _dmac_reg_read(dmac_num, ASP_DMA_INT_TC1_AP);
    tc2  = _dmac_reg_read(dmac_num, ASP_DMA_INT_TC2_AP);

    /* clr interupt states */
    _dmac_reg_write(dmac_num, ASP_DMA_INT_TC1_RAW, int_state);
    _dmac_reg_write(dmac_num, ASP_DMA_INT_TC2_RAW, int_state);
    _dmac_reg_write(dmac_num, ASP_DMA_INT_ERR1_RAW, int_state);
    _dmac_reg_write(dmac_num, ASP_DMA_INT_ERR2_RAW, int_state);
    _dmac_reg_write(dmac_num, ASP_DMA_INT_ERR3_RAW, int_state);

    for (i = 0; i < ASP_DMA_MAX_CHANNEL_NUM; i++) {
        int_mask = 0x1U << i;
        if (int_state & int_mask) {
            if (priv->callback[i].callback) {
                if (err1 & int_mask)
                    int_type = ASP_DMA_INT_TYPE_ERR1;
                else if (err2 & int_mask)
                    int_type = ASP_DMA_INT_TYPE_ERR2;
                else if (err3 & int_mask)
                    int_type = ASP_DMA_INT_TYPE_ERR3;
                else if (tc1 & int_mask)
                    int_type = ASP_DMA_INT_TYPE_TC1;
                else if (tc2 & int_mask)
                    int_type = ASP_DMA_INT_TYPE_TC2;
                else
                    int_type = ASP_DMA_INT_TYPE_BUTT;
                if ((td_bool)error_interrupt_handle(int_type, i) == TD_TRUE) {
                    audio_set_err(int_type);
                    break;
                } else {
                    priv->callback[i].callback(int_state);
                }
            }
        }
    }
    return (td_s32)IRQ_HANDLED;
}

/**
 * asp dma clk has the same life cycle as asp subsys clk.
 * ensure asp subsys clk enabled, when called this func.
 * asp subsys clk enabled in slimbus & codec controller.
 */
td_s32 asp_dma_config(td_u32 dmac_num, td_u32 dma_channel, struct dma_lli_cfg *lli_cfg, td_bool para)
{
    struct asp_dma_priv *priv = g_asp_dma_priv[dmac_num];
    td_u32 channel_mask = (0x1U << dma_channel);

    if (!priv) {
        aio_err_trace("asp_dma_config priv is null\n");
        return -EINVAL;
    }

    if (dma_channel >= ASP_DMA_MAX_CHANNEL_NUM) {
        dev_err(priv->dev, "asp_dma_config dma channel err:%u\n", dma_channel);
        return -EINVAL;
    }

    if (!lli_cfg) {
        dev_err(priv->dev, "asp_dma_config lli cfg is null.\n");
        return -EINVAL;
    }

    /* disable dma channel */
    _dmac_reg_clr_bit(dmac_num, ASP_DMA_CX_CONFIG(dma_channel), 0);
    _dmac_reg_write(dmac_num, ASP_DMA_CX_CNT0(dma_channel), lli_cfg->a_count);

    /* c count */
    _dmac_reg_write(dmac_num, ASP_DMA_CX_CNT1(dma_channel), 0);
    _dmac_reg_write(dmac_num, ASP_DMA_CX_BINDX(dma_channel), 0);
    _dmac_reg_write(dmac_num, ASP_DMA_CX_CINDX(dma_channel), 0);

    /* set dma src/des addr */
    _dmac_reg_write(dmac_num, ASP_DMA_CX_SRC_ADDR(dma_channel), lli_cfg->src_addr);
    _dmac_reg_write(dmac_num, ASP_DMA_CX_DES_ADDR(dma_channel), lli_cfg->des_addr);
    aio_info_trace("asp_dma_config lli_cfg->src_addr = %#x, lli_cfg->des_addr = %#x",
        lli_cfg->src_addr, lli_cfg->des_addr);
    aio_info_trace("asp_dma_config src_reg = %#x, des_reg = %#x",
        (td_u32)ASP_DMA_CX_SRC_ADDR(dma_channel), (td_u32)ASP_DMA_CX_DES_ADDR(dma_channel));
    aio_info_trace("asp_dma_config src_reg_val = %#x, des_reg_val = %#x",
        _dmac_reg_read(0U, ASP_DMA_CX_SRC_ADDR(dma_channel)),
        _dmac_reg_read(0U, ASP_DMA_CX_DES_ADDR(dma_channel)));

    /* set dma lli config */
    _dmac_reg_write(dmac_num, ASP_DMA_CX_LLI(dma_channel), lli_cfg->lli);

    aio_info_trace("asp_dma_config next lli addr = %u\n", lli_cfg->lli);

    /* clr irq status of dma channel */
    _dmac_reg_write(dmac_num, ASP_DMA_INT_TC1_RAW, channel_mask);
    _dmac_reg_write(dmac_num, ASP_DMA_INT_TC2_RAW, channel_mask);
    _dmac_reg_write(dmac_num, ASP_DMA_INT_ERR1_RAW, channel_mask);
    _dmac_reg_write(dmac_num, ASP_DMA_INT_ERR2_RAW, channel_mask);
    _dmac_reg_write(dmac_num, ASP_DMA_INT_ERR3_RAW, channel_mask);

    priv->callback[dma_channel].callback = aio_interrupt_route;
    priv->callback[dma_channel].para = para;

    /* release irq mask */
    _dmac_reg_set_bit(dmac_num, ASP_DMA_INT_ERR1_MASK_AP, dma_channel);
    _dmac_reg_set_bit(dmac_num, ASP_DMA_INT_ERR2_MASK_AP, dma_channel);
    _dmac_reg_set_bit(dmac_num, ASP_DMA_INT_ERR3_MASK_AP, dma_channel);
    _dmac_reg_set_bit(dmac_num, ASP_DMA_INT_TC1_MASK_AP, dma_channel);
    _dmac_reg_set_bit(dmac_num, ASP_DMA_INT_TC2_MASK_AP, dma_channel);
    aio_info_trace("dma config succ.\n");

    return 0;
}
EXPORT_SYMBOL(asp_dma_config);

td_s32 asp_dma_start(td_u32 dmac_num, td_u32 dma_channel, const struct dma_lli_cfg *lli_cfg)
{
    struct asp_dma_priv *priv = g_asp_dma_priv[dmac_num];
    td_u32 lli_reg;

    if (!priv) {
        aio_err_trace("priv is null\n");
        return -EINVAL;
    }

    if (dma_channel >= ASP_DMA_MAX_CHANNEL_NUM) {
        dev_err(priv->dev, "dma channel err:%u\n", dma_channel);
        return -EINVAL;
    }

    if (!lli_cfg) {
        dev_err(priv->dev, "lli cfg is null.\n");
        return -EINVAL;
    }

    lli_reg = _dmac_reg_read(dmac_num, ASP_DMA_CX_LLI(dma_channel));
    aio_info_trace("lli is changed, lli_reg: %#x, lli_cfg: %#x, config = %#x\n",
        lli_reg, lli_cfg->lli, lli_cfg->config);
    if (lli_reg != lli_cfg->lli) {
        dev_err(priv->dev, "lli is changed, lli_reg: %u, lli_cfg: %u\n", lli_reg, lli_cfg->lli);
        return -EINVAL;
    }

    _dmac_reg_write(dmac_num, ASP_DMA_CX_CONFIG(dma_channel), lli_cfg->config);
    dmac_dump(dmac_num, dma_channel);

    aio_info_trace("dma start succ.\n");
    return 0;
}
EXPORT_SYMBOL(asp_dma_start);

void asp_dma_stop(td_u32 dmac_num, td_u32 dma_channel)
{
    struct asp_dma_priv *priv = g_asp_dma_priv[dmac_num];
    td_u32 channel_mask = (0x1U << dma_channel);
    td_u32 i = 40; // 40 : 等待1ms

    if (!priv) {
        return;
    }

    if (dma_channel >= ASP_DMA_MAX_CHANNEL_NUM) {
        return;
    }

    /* disable dma channel */
    _dmac_reg_clr_bit(dmac_num, ASP_DMA_CX_CONFIG(dma_channel), 0);

    do {
        if (0 == (_dmac_reg_read(dmac_num, ASP_DMA_CH_STAT) & channel_mask)) {
            break;
        }
        osal_udelay(ASP_DMAC_READ_DELAY);
    } while (--i);

    _dmac_reg_clr_bit(dmac_num, ASP_DMA_INT_ERR1_MASK_AP, dma_channel);
    _dmac_reg_clr_bit(dmac_num, ASP_DMA_INT_ERR2_MASK_AP, dma_channel);
    _dmac_reg_clr_bit(dmac_num, ASP_DMA_INT_ERR3_MASK_AP, dma_channel);
    _dmac_reg_clr_bit(dmac_num, ASP_DMA_INT_TC1_MASK_AP, dma_channel);
    _dmac_reg_clr_bit(dmac_num, ASP_DMA_INT_TC2_MASK_AP, dma_channel);
}
EXPORT_SYMBOL(asp_dma_stop);

void __iomem *g_asp_dma_reg_base_addr_temp;
void __iomem * get_base(void)
{
    return g_asp_dma_reg_base_addr_temp;
}
EXPORT_SYMBOL(get_base);

td_s32 asp_dma_init(struct platform_device *pdev)
{
    int ret = 0;
    struct device *dev = &pdev->dev;
    struct asp_dma_priv *priv = NULL;
    struct device_node *np = pdev->dev.of_node;
    int err;
    const char *irq_name = NULL;

    priv = devm_kzalloc(dev, sizeof(struct asp_dma_priv), GFP_KERNEL);
    if (!priv) {
        aio_err_trace("malloc failed.\n");
        return -ENOMEM;
    }

    priv->res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dma");
    if (!priv->res) {
        aio_err_trace("get resource failed.\n");
        ret = -ENOENT;
        goto err_exit;
    }

    priv->asp_dma_reg_base_addr = devm_ioremap(dev, priv->res->start, resource_size(priv->res));

    if (!priv->asp_dma_reg_base_addr) {
        aio_err_trace("asp dma reg addr ioremap failed.\n");
        ret = -ENOMEM;
        goto err_exit;
    }

    g_asp_dma_reg_base_addr_temp = priv->asp_dma_reg_base_addr;

    err = of_property_read_u32(np, "dmac", &priv->dmac_num);
    if (err) {
        aio_err_trace("get dmac node err!\n");
        goto err_exit;
    }

    err = of_property_read_string_index(np, "irq_name", 0, (const char **)&irq_name);
    if (err) {
        aio_err_trace("get irq_name node err!\n");
        goto err_exit;
    }

    priv->irq = (td_u32)osal_platform_get_irq_byname(pdev, irq_name);
    ret = osal_request_irq((unsigned int)priv->irq, _asp_dmac_irq_handler, TD_NULL, "asp_dma_irq",
                           _asp_dmac_irq_handler);
    if (ret) {
        aio_err_trace("request asp dma irq failed. %u ,%d\n", priv->irq, ret);
        ret = -ENOENT;
        goto err_exit;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    if (priv->wake_lock == NULL) {
        priv->wake_lock = wakeup_source_create("asp_dma");
    }
#else
    wakeup_source_init(&priv->wake_lock, "asp_dma");
#endif
    spin_lock_init(&priv->lock);

    priv->dev = dev;
    platform_set_drvdata(pdev, priv);
    g_asp_dma_priv[priv->dmac_num] = priv;
    aio_info_trace("asp_dma_init end.\n");

    return 0;

err_exit:
    if (priv->asp_dma_reg_base_addr) {
        devm_iounmap(dev, priv->asp_dma_reg_base_addr);
        priv->asp_dma_reg_base_addr = NULL;
    }

    devm_kfree(dev, priv);
    priv = NULL;
    aio_err_trace("asp_dma_init failed.\n");

    return ret;
}

td_s32 asp_dma_remove(const struct platform_device *pdev)
{
    struct asp_dma_priv *priv = (struct asp_dma_priv*)platform_get_drvdata(pdev);

    if (!priv)
        return 0;

    if (priv->asp_dma_reg_base_addr) {
        devm_iounmap(priv->dev, priv->asp_dma_reg_base_addr);
        priv->asp_dma_reg_base_addr = NULL;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    if (priv->wake_lock != NULL) {
        wakeup_source_destroy(priv->wake_lock);
    }
    priv->wake_lock = NULL;
#else
    wakeup_source_trash(&priv->wake_lock);
#endif
    osal_free_irq(priv->irq, _asp_dmac_irq_handler);
    devm_kfree(priv->dev, priv);
    priv = NULL;
    return 0;
}

MODULE_AUTHOR("Hisilicon Technologies Co., Ltd");
MODULE_DESCRIPTION("ASP DMA Driver");
MODULE_LICENSE("GPL");
