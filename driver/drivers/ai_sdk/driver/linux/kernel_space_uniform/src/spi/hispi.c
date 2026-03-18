/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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
 * Description: SPI file function description
 * Author: huawei
 * Create: 2021-02-07
 */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/acpi.h>
#include <linux/property.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#include <linux/reset-controller.h>
#include <linux/reset.h>
#else
#include "../subctrl/include.linux/reset-controller.h"
#include "../subctrl/include.linux/reset.h"
#endif
#include "hispi.h"
#include "spi_misc.h"
#include "kdrv_spi_api.h"
#include <linux/securec.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline u32 spi_rev32(u32 w)
{
    u32 i;
    u32 result = 0;

    for (i = 0; i < WIDTH_32_BITS; i++) {
        result <<= 1;
        if ((w & 1) != 0) {
            result |= 1;
        }

        w >>= 1;
    }

    return (result);
}

static inline u16 spi_rev16(u16 w)
{
    u32 i;
    u16 result = 0;

    for (i = 0; i < WIDTH_16_BITS; i++) {
        result <<= 1;
        if ((w & 1) != 0) {
            result |= 1;
        }

        w >>= 1;
    }

    return (result);
}

/* Function Description: bit reverse order u8 */
static inline u8 spi_rev8(u8 c)
{
    u32 i;
    u8 result = 0;

    for (i = 0; i < WIDTH_8_BITS; i++) {
        result <<= 1;
        if ((c & 1) != 0) {
            result |= 1;
        }

        c >>= 1;
    }

    return (result);
}

static int hispi_do_rx(struct hisi_spi *hispi, struct hisi_trans_info *info)
{
    u32 count = min_t(u32, info->rx_cnt, hispi->fifo_depth);
    u32 data;

    while (hispi->soc_ops->rx_not_empty(&hispi->reg) && count--) {
        data = hispi->soc_ops->read_rx(&hispi->reg);
        if (info->rx_buf != NULL) {
            switch (info->bits_per_word) {
                case WIDTH_8_BITS:
                    data = (info->is_lsb_first) ? spi_rev8((u8)data) : data;
                    *(u8 *)(info->rx_buf) = data;
                    break;
                case WIDTH_16_BITS:
                    data = (info->is_lsb_first) ? spi_rev16((u16)data) : data;
                    *(u16 *)(info->rx_buf) = data;
                    break;
                case WIDTH_32_BITS:
                default:
                    data = (info->is_lsb_first) ? spi_rev32((u32)data) : data;
                    *(u32 *)(info->rx_buf) = data;
                    break;
            }
            info->rx_buf += ((u32)info->bits_per_word / WIDTH_8_BITS);
        }
        info->rx_cnt--;
    }
    return 0;
}

static int hispi_do_tx(struct hisi_spi *hispi, struct hisi_trans_info *info)
{
    u32 count = min_t(u32, info->tx_cnt, hispi->fifo_depth);
    u32 data = 0;

    while (hispi->soc_ops->tx_not_full(&hispi->reg) && count--) {
        if (info->tx_buf != NULL) {
            switch (info->bits_per_word) {
                case WIDTH_8_BITS:
                    data = *(u8 *)(info->tx_buf);
                    data = (info->is_lsb_first) ? spi_rev8((u8)data) : data;
                    break;
                case WIDTH_16_BITS:
                    data = *(u16 *)(info->tx_buf);
                    data = (info->is_lsb_first) ? spi_rev16((u16)data) : data;
                    break;
                case WIDTH_32_BITS:
                default:
                    data = *(u32 *)(info->tx_buf);
                    data = (info->is_lsb_first) ? spi_rev32((u32)data) : data;
                    break;
            }
            info->tx_buf += ((u32)info->bits_per_word / WIDTH_8_BITS);
        }
        info->tx_cnt--;
        hispi->soc_ops->write_tx(&hispi->reg, data);
    }
    return 0;
}

static irqreturn_t hisi_spi_irq(int irq, void *dev_id)
{
    struct hisi_spi *hispi = NULL;
    struct spi_master *master = dev_id;
    struct kdrv_spi_trans_irq status = {0};

    hispi = spi_master_get_devdata(master);
    if (master->cur_msg == NULL) {
        dev_err(hispi->dev, "cur_msg is null\n");
        hispi->soc_ops->disable(&hispi->reg);
        hispi->soc_ops->disable_int(&hispi->reg);
        return IRQ_HANDLED;
    }
    hispi->soc_ops->get_trans_irq(&hispi->reg, &status);

    if (status.rx_overflow == true) {
        dev_err(hispi->dev, "interrupt : rx fifo overlow\n");
        master->cur_msg->status = -EIO;
        hispi->soc_ops->flush_rx_fifo(&hispi->reg);
        hispi->soc_ops->clear_rxoi(&hispi->reg);
        goto finalize_transfer;
    }

    if (status.rx_irq == true) {
        hispi_do_rx(hispi, &hispi->info);
    }
    // transmission is complete only after the rx reception is complete.
    if (hispi->info.rx_cnt == 0) {
        goto finalize_transfer;
    }

    if (status.tx_irq == true) {
        hispi_do_tx(hispi, &hispi->info);
    }

    return IRQ_HANDLED;

finalize_transfer:
    hispi->soc_ops->disable(&hispi->reg);
    hispi->soc_ops->disable_int(&hispi->reg);
    // The frame completion value is set to 1, and the waiting ends.
    spi_finalize_current_transfer(master);
    return IRQ_HANDLED;
}

static void spi_save_load_trans_info(struct hisi_trans_info *dst, struct hisi_trans_info *src)
{
    dst->tx_buf = src->tx_buf;
    dst->rx_buf = src->rx_buf;
    dst->bits_per_word = src->bits_per_word;
    dst->tx_cnt = src->tx_cnt;
    dst->rx_cnt = src->rx_cnt;
    dst->max_speed_hz = src->max_speed_hz;
    dst->chip_select = src->chip_select;
    dst->mode = src->mode;
    dst->is_lsb_first = src->is_lsb_first;
}

static int hisi_transfer_one(struct spi_master *master, struct spi_device *spi, struct spi_transfer *transfer)
{
    struct hisi_spi *hispi = NULL;
    struct hisi_trans_info tmp;
    int ret;

    hispi = spi_master_get_devdata(master);
    spi_save_load_trans_info(&tmp, &hispi->info);
    hispi->info.tx_buf = transfer->tx_buf;
    hispi->info.rx_buf = transfer->rx_buf;
    hispi->info.bits_per_word = transfer->bits_per_word;
    hispi->info.tx_cnt = transfer->len / (transfer->bits_per_word / WIDTH_8_BITS);
    hispi->info.rx_cnt = hispi->info.tx_cnt;
    hispi->info.max_speed_hz = transfer->speed_hz;

    hispi->info.chip_select = spi->chip_select;
    hispi->info.mode = spi->mode;
    hispi->info.is_lsb_first = ((spi->mode & SPI_LSB_FIRST) != 0);

    dev_dbg(&spi->dev, "spi speed=%u, cs=%u, mode=0x%x, bpw=%u, len=%u\n",
        transfer->speed_hz, spi->chip_select, spi->mode, transfer->bits_per_word, transfer->len);
    ret = hispi->soc_ops->setup(&hispi->reg, &hispi->info, hispi->clk_freq, hispi->rx_fifo_level);
    if (ret != 0) {
        dev_err(&spi->dev, "setup fail , ret = %d\n", ret);
        spi_save_load_trans_info(&hispi->info, &tmp);
        return ret;
    }

    hispi->soc_ops->flush_rx_fifo(&hispi->reg);
    /* Ensure that the previous data is updated before the interrupt is enabled */
    smp_mb();
    hispi->soc_ops->enable_int(&hispi->reg);
    hispi->soc_ops->enable(&hispi->reg);
    // returned > 0, the framework enters the wait completion state,
    // and the waiting interrupt sets the completion value to 1.
    return 1;
}

static int hisi_spi_get_dts_acpi(struct platform_device *pdev, struct hisi_spi *hispi)
{
    struct device *dev = &pdev->dev;
    struct resource *res = NULL;
    void *base = NULL;
    int ret, irq;

    ret = device_property_read_u32(dev, "num-cs", &hispi->num_chipselect);
    if (ret != 0) {
        dev_err(&pdev->dev, "can't get num-cs, ret=%d\n", ret);
        return ret;
    }

    ret = device_property_read_u32(dev, "bus-num", &hispi->bus_num);
    if (ret != 0) {
        dev_err(&pdev->dev, "can't get bus-num, ret=%d\n", ret);
        return ret;
    }

    ret = device_property_read_u32(dev, "clk-freq", &hispi->clk_freq);
    if (ret != 0) {
        dev_err(&pdev->dev, "can't get clk-freq, use default value\n");
        hispi->clk_freq = CLK_FRE_DEFAULT;
    }
    if (hispi->clk_freq < SPEED_HZ_MAX) {
        dev_err(&pdev->dev, "clk-freq from dts is invalid\n");
        return -EINVAL;
    }

    ret = device_property_read_u32(dev, "rx-fifo-level", &hispi->rx_fifo_level);
    if (ret != 0) {
        dev_info(&pdev->dev, "can't get rx-fifo-level, use default value\n");
        hispi->rx_fifo_level = HISPI_RX_4;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(base)) {
        ret = PTR_ERR(base);
        return ret;
    }
    irq = platform_get_irq(pdev, 0);
    if (irq < 0) {
        dev_err(&pdev->dev, "can't get irq, ret=%d\n", irq);
        return irq;
    }
    hispi->reg.io_base = base;
    hispi->reg.io_size = res->end - res->start + 1;
    hispi->irq = irq;
    return 0;
}

static inline void hisi_spi_construct_master(struct spi_master *master,
    struct hisi_spi *hispi, struct platform_device *pdev)
{
    master->num_chipselect = hispi->num_chipselect;
    master->bus_num = hispi->bus_num;
    master->transfer = NULL;
    master->transfer_one = hisi_transfer_one;
    master->dev.of_node = pdev->dev.of_node;
    master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_LSB_FIRST | SPI_LOOP;
    master->bits_per_word_mask = SPI_BPW_RANGE_MASK(WIDTH_8_BITS, WIDTH_32_BITS);
    master->setup = NULL;
}

static int hisi_spi_check_ops(const struct hisi_spi_ops *ops)
{
    if (ops == NULL || ops->disable == NULL || ops->disable_int == NULL ||
        ops->enable == NULL || ops->enable_int == NULL ||
        ops->get_trans_irq == NULL || ops->read_rx == NULL ||
        ops->setup == NULL || ops->write_tx == NULL || ops->get_fifo_depth == NULL) {
            pr_err("check ops failed\n");
            return -1;
        }
    return 0;
}

static int hisi_spi_reset_before_init(struct hisi_spi *hispi)
{
    int ret;
    struct device *dev = hispi->dev;
#ifdef CONFIG_SPI_UDRV
    ret = spi_crg_init(hispi->bus_num);
    if (ret != 0) {
        return ret;
    }
#endif
    ret = hisi_spi_get_clk_rst_info(hispi, dev);
    if (ret != 0) {
        return ret;
    }
    ret = hisi_spi_reset(hispi);
    if (ret != 0) {
        return ret;
    }
    return 0;
}

static int hisi_spi_probe(struct platform_device *pdev)
{
    struct spi_master *master = NULL;
    struct hisi_spi *hispi = NULL;
    int ret;

    master = spi_alloc_master(&pdev->dev, sizeof(struct hisi_spi));
    if (master == NULL) {
        return -ENOMEM;
    }
    hispi = spi_master_get_devdata(master);
    hispi->master = master;
    hispi->dev = &pdev->dev;
    hispi->soc_ops = spi_get_ops();
    hispi->info.max_speed_hz = SPEED_HZ_MAX;
    hispi->info.bits_per_word = WIDTH_8_BITS;
    hispi->info.chip_select = 0;
    ret = hisi_spi_check_ops(hispi->soc_ops);
    if (ret != 0) {
        goto out_master_put;
    }
    hispi->fifo_depth = hispi->soc_ops->get_fifo_depth();
    ret = hisi_spi_get_dts_acpi(pdev, hispi);
    if (ret != 0) {
        dev_err(&pdev->dev, "get dts or acpi info failed, ret=%d\n", ret);
        goto out_master_put;
    }
    hisi_spi_construct_master(master, hispi, pdev);
    platform_set_drvdata(pdev, master);
    ret = hisi_spi_reset_before_init(hispi);
    if (ret != 0) {
        goto out_master_put;
    }

    ret = devm_request_irq(&pdev->dev, hispi->irq, hisi_spi_irq, 0, dev_name(&pdev->dev), master);
    if (ret < 0) {
        dev_err(&pdev->dev, "failed to get IRQ=%d, ret=%d\n", hispi->irq, ret);
        goto out_master_put;
    }

    ret = spi_register_master(master);
    if (ret != 0) {
        dev_err(&pdev->dev, "register spi master failed, ret=%d\n", ret);
        goto out_master_put;
    }
    return 0;

out_master_put:
	spi_master_put(master);
    return ret;
}

static int hisi_spi_remove(struct platform_device *pdev)
{
    struct spi_master *master = platform_get_drvdata(pdev);

    spi_unregister_master(master);

    return 0;
}

int hisi_spi_reset(struct hisi_spi *hispi)
{
    int ret;

    /* 1.Turn off the clock */
    ret = hisi_subctrl_spi_clkgate_disable(hispi->dev, hispi->bus_num);
    if (ret != 0) {
        dev_err(hispi->dev, "hisi_subctrl_spi_clkgate_disable failed, ret=%d\n", ret);
        return ret;
    }
    /* 2.Reset */
    ret = hisi_subctrl_spi_reset_assert(hispi->dev, hispi->bus_num);
    if (ret != 0) {
        dev_err(hispi->dev, "soft reset failed, ret=%d\n", ret);
        return ret;
    }
    /* 3.Turn on the clock */
    ret = hisi_subctrl_spi_clkgate_enable(hispi->dev, hispi->bus_num);
    if (ret != 0) {
        dev_err(hispi->dev, "hisi_subctrl_spi_clkgate_enable failed, ret=%d\n", ret);
        return ret;
    }
    /* 4.Wait 100us */
    udelay(100);
    /* 5.Turn off the clock */
    ret = hisi_subctrl_spi_clkgate_disable(hispi->dev, hispi->bus_num);
    if (ret != 0) {
        dev_err(hispi->dev, "hisi_subctrl_spi_clkgate_disable failed, ret=%d\n", ret);
        return ret;
    }
    /* 6.Deassert reset */
    ret = hisi_subctrl_spi_reset_deassert(hispi->dev, hispi->bus_num);
    if (ret != 0) {
        dev_err(hispi->dev, "soft dereset failed, ret=%d\n", ret);
        return ret;
    }
    /* 7.Obtains the reset deassertion status. */
    ret = hisi_subctrl_spi_get_reset_status(hispi->dev, hispi->bus_num);
    if (ret != 0) {
        dev_err(hispi->dev, "dev is not deassert status, ret=%d\n", ret);
        return ret;
    }
    /* 8.Turn on the clock */
    ret = hisi_subctrl_spi_clkgate_enable(hispi->dev, hispi->bus_num);
    if (ret != 0) {
        dev_err(hispi->dev, "hisi_subctrl_spi_clkgate_enable failed, ret=%d\n", ret);
        return ret;
    }
    dev_info(hispi->dev, "spi reset success\n");

    return 0;
}

static int hisi_spi_suspend(struct device *dev)
{
    struct spi_master *master = NULL;
    struct hisi_spi *hispi = NULL;
    int ret;

    dev_info(dev, "spi suspend enter\n");
    master = dev_get_drvdata(dev);
    hispi = spi_master_get_devdata(master);

    ret = spi_master_suspend(master);
    if (ret != 0) {
        dev_err(dev, "spi master suspend failed, ret=%d\n", ret);
        return ret;
    }

    /* Turn off the clock */
    ret = hisi_subctrl_spi_clkgate_disable(hispi->dev, hispi->bus_num);
    if (ret != 0) {
        dev_err(dev, "hisi_subctrl_spi_clkgate_disable failed, ret=%d\n", ret);
        return ret;
    }
    /* Soft reset */
    ret = hisi_subctrl_spi_reset_assert(hispi->dev, hispi->bus_num);
    if (ret != 0) {
        dev_err(dev, "soft reset failed, ret=%d\n", ret);
        return ret;
    }

    return 0;
}

static int hisi_spi_resume(struct device *dev)
{
    struct spi_master *master = NULL;
    struct hisi_spi *hispi = NULL;
    int ret;

    dev_info(dev, "spi resume enter\n");
    master = dev_get_drvdata(dev);
    hispi = spi_master_get_devdata(master);
    ret = hisi_spi_reset(hispi);
    if (ret != 0) {
        dev_err(dev, "spi reset fail\n");
        return ret;
    }
    ret = hispi->soc_ops->setup(&hispi->reg, &hispi->info, hispi->clk_freq, hispi->rx_fifo_level);
    if (ret != 0) {
        dev_err(dev, "setup fail , ret = %d\n", ret);
        return ret;
    }
    ret = spi_master_resume(master);
    if (ret != 0) {
        dev_err(dev, "spi master resume fail, ret = %d\n", ret);
        return ret;
    }

    return 0;
}

int kdrv_spi_suspend(u32 spi_no)
{
    struct hisi_spi *hispi = NULL;
    int ret;
    hispi = hisi_spi_get_by_spi_no(spi_no);
    if (hispi == NULL) {
        pr_err("[spi suspend], get hispi failed");
        return -ENODEV;
    }
    ret = hisi_spi_suspend(hispi->dev);
    dev_info(hispi->dev, "spi suspend ret 0x%x\n", ret);
    return ret;
}
EXPORT_SYMBOL(kdrv_spi_suspend);

int kdrv_spi_resume(u32 spi_no)
{
    struct hisi_spi *hispi = NULL;
    int ret;
    hispi = hisi_spi_get_by_spi_no(spi_no);
    if (hispi == NULL) {
        pr_err("[spi resume], get hispi failed");
        return -ENODEV;
    }
    ret = hisi_spi_resume(hispi->dev);
    dev_info(hispi->dev, "spi resume ret 0x%x\n", ret);
    return ret;
}
EXPORT_SYMBOL(kdrv_spi_resume);

static const struct dev_pm_ops hisi_spi_pm_ops = {
    .suspend    = hisi_spi_suspend,
    .resume     = hisi_spi_resume,
};

static const struct of_device_id spi_of_match[] = {
    { .compatible = "synopsis_spi",
    },
    { .compatible = "hisilicon,udrv_spi",
    },
    {},
};
MODULE_DEVICE_TABLE(of, spi_of_match);

static const struct acpi_device_id spi_acpi_match[] = {
    {"HISI03E1", 0, 0, 0},
    {}
};
MODULE_DEVICE_TABLE(acpi, spi_acpi_match);

static struct platform_driver hisi_spi_drv = {
    .driver = {
        .name = "hisi_spi_drv",
        .pm = &hisi_spi_pm_ops,
        .of_match_table = spi_of_match,
        .acpi_match_table = ACPI_PTR(spi_acpi_match),
    },
    .probe = hisi_spi_probe,
    .remove = hisi_spi_remove,
};
module_platform_driver(hisi_spi_drv);

static int find_spi_master_by_spi_no(struct device *dev, void *data)
{
    struct master_find_data *spi_search_info = (struct master_find_data *)data;
    struct spi_master *master = dev_get_drvdata(dev);
    struct hisi_spi *hispi = spi_master_get_devdata(master);

    if (hispi->bus_num == spi_search_info->spi_no) {
        spi_search_info->hispi = hispi;
        return 1; // Found exit traversal dev
    }
    return 0;
}

struct hisi_spi *hisi_spi_get_by_spi_no(uint32_t spi_no)
{
    int ret;
    struct master_find_data spi_search_info = {0};
    spi_search_info.spi_no = spi_no;

    ret = driver_for_each_device(&hisi_spi_drv.driver, NULL, &spi_search_info, find_spi_master_by_spi_no);
    if (ret == 0) {
        pr_err("find spi fail: spi_no = %u,\n", spi_no);
        return NULL;
    }

    return spi_search_info.hispi;
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("hisi_spi kernel driver");
#ifdef __cplusplus
}
#endif
