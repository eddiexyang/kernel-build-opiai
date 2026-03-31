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
 * Description: SPI file function description.
 * Author: huawei
 * Create: 2021-01-21
 */
#include <linux/types.h>
#include <linux/printk.h>
#include "kdrv_spi_api.h"
#include "hispi.h"

#define CS_HIGH_SHIFT 2
#define LSB_SHIFT 3
#define LOOP_SHIFT 5

struct device_find_data {
    uint32_t spi_no;
    uint32_t cs;
    struct spi_device *spi;
};

static int find_spi_device_by_spi_no_and_cs(struct device *dev, void *data)
{
    struct spi_device *spi = to_spi_device(dev);
    struct device_find_data *find_data = (struct device_find_data *)data;
    struct spi_controller *master = spi->controller;

    if (((uint32_t)(int32_t)master->bus_num == find_data->spi_no) && (spi->chip_select == find_data->cs)) {
        find_data->spi = spi;
        return 1;
    }
    return 0;
}

static struct spi_device *hisi_spi_get_device_by_spi_no_and_cs(uint32_t spi_no, uint32_t cs)
{
    int ret;
    struct device_find_data spi_search_info = { 0 };
    spi_search_info.spi_no = spi_no;
    spi_search_info.cs = cs;

    ret = bus_for_each_dev(&spi_bus_type, NULL, &spi_search_info, find_spi_device_by_spi_no_and_cs);
    if (ret == 0) {
        pr_err("can't find the spi dev: spi_no = %u, cs = %u\n", spi_no, cs);
        return NULL;
    }
    return spi_search_info.spi;
}

int kdrv_spi_set_device_config(u32 spi_no, u32 cs, struct kdrv_spi_device_config *config)
{
    struct spi_device *spi = NULL;
    u32 save_speed;
    u8 bits_per_word;
    u16 save_mode;

    if (config == NULL) {
        pr_err("config ptr is null\n");
        return -1;
    }
    spi = hisi_spi_get_device_by_spi_no_and_cs(spi_no, cs);
    if (spi == NULL) {
        pr_err("get spi fail: spi_no = %u, cs = %u\n", spi_no, cs);
        return -ENODEV;
    }

    save_mode = spi->mode;
    save_speed = spi->max_speed_hz;
    bits_per_word = spi->bits_per_word;

    spi->mode = config->mode | (config->cs_high << CS_HIGH_SHIFT) | (config->lsb << LSB_SHIFT) |
        (config->is_loop << LOOP_SHIFT);
    spi->max_speed_hz = config->speed;
    spi->bits_per_word = config->bits_per_word;

    if (spi_setup(spi) != 0) {
        spi->mode = save_mode;
        spi->max_speed_hz = save_speed;
        spi->bits_per_word = bits_per_word;
        pr_err("[spi_kern_setup], dev spi_no=0x%x, cs=0x%x, setup failed\n", spi_no, cs);
        return -1;
    }
    return 0;
}
EXPORT_SYMBOL(kdrv_spi_set_device_config);

static int spi_send_message(struct spi_device *spi, const void *tx_buf, void *rx_buf, u32 len)
{
    int ret;
    struct spi_message m;
    struct spi_transfer spi_xfer = { 0 };

    spi_bus_lock(spi->controller);

    spi_xfer.tx_buf = tx_buf;
    spi_xfer.rx_buf = rx_buf;
    spi_xfer.len = len;

    spi_message_init(&m);
    spi_message_add_tail(&spi_xfer, &m);
    ret = spi_sync_locked(spi, &m);
    spi_bus_unlock(spi->controller);

    return ret;
}

int kdrv_spi_start_trans(u32 spi_no, u32 cs, const void *tx_buf, void *rx_buf, u32 len)
{
    struct spi_device *spi = NULL;
    int ret;

    if (tx_buf == NULL && rx_buf == NULL) {
        pr_err("[spi trans], para is invalid \n");
        return -1;
    }

    spi = hisi_spi_get_device_by_spi_no_and_cs(spi_no, cs);
    if (spi == NULL) {
        pr_err("[spi start trans] get spi fail: spi_no = %u, cs = %u\n", spi_no, cs);
        return -ENODEV;
    }
    ret = spi_send_message(spi, tx_buf, rx_buf, len);
    return ret;
}
EXPORT_SYMBOL(kdrv_spi_start_trans);

int kdrv_spi_get_master_current_config(u32 spi_no, struct kdrv_spi_master_config *config)
{
    struct hisi_spi *hispi = NULL;

    if (config == NULL) {
        pr_err("[spi get master config], config is null ptr\n");
        return -1;
    }
    hispi = hisi_spi_get_by_spi_no(spi_no);
    if (hispi == NULL) {
        pr_err("[spi get master config], get hispi failed\n");
        return -ENODEV;
    }
    if (hispi->soc_ops->get_current_config == NULL) {
        pr_err("get_current_config ops is null\n");
        return -1;
    }
    hispi->soc_ops->get_current_config(&hispi->reg, hispi->clk_freq, config);

    return 0;
}
EXPORT_SYMBOL(kdrv_spi_get_master_current_config);

int kdrv_spi_get_device_config(u32 spi_no, u32 cs, struct kdrv_spi_device_config *config)
{
    struct spi_device *spi = NULL;

    if (config == NULL) {
        pr_err("[spi get device config], config is null ptr");
        return -1;
    }
    spi = hisi_spi_get_device_by_spi_no_and_cs(spi_no, cs);
    if (spi == NULL) {
        pr_err("[spi get device config], get spi fail: spi_no = %u, cs = %u\n", spi_no, cs);
        return -ENODEV;
    }
    config->speed = spi->max_speed_hz;
    config->mode = spi->mode & 0x3;
    config->cs_high = (spi->mode >> CS_HIGH_SHIFT) & 0x1;
    config->lsb = (spi->mode >> LSB_SHIFT) & 0x1;
    config->is_loop = (spi->mode >> LOOP_SHIFT) & 0x1;
    config->bits_per_word = spi->bits_per_word;

    return 0;
}
EXPORT_SYMBOL(kdrv_spi_get_device_config);

int kdrv_spi_get_trans_status(u32 spi_no, struct kdrv_spi_trans_status *status)
{
    struct hisi_spi *hispi = NULL;
    if (status == NULL) {
        pr_err("[spi get status], para is null ptr\n");
        return -1;
    }

    hispi = hisi_spi_get_by_spi_no(spi_no);
    if (hispi == NULL) {
        pr_err("[spi get master status], get hispi failed\n");
        return -ENODEV;
    }
    if (hispi->soc_ops->get_trans_status == NULL) {
        pr_err("get_trans_status ops is null\n");
        return -1;
    }
    hispi->soc_ops->get_trans_status(&hispi->reg, status);
    return 0;
}
EXPORT_SYMBOL(kdrv_spi_get_trans_status);

int kdrv_spi_get_trans_irq(u32 spi_no, struct kdrv_spi_trans_irq *status)
{
    struct hisi_spi *hispi = NULL;
    if (status == NULL) {
        pr_err("[spi get trans irq], para is null ptr\n");
        return -1;
    }

    hispi = hisi_spi_get_by_spi_no(spi_no);
    if (hispi == NULL) {
        pr_err("[spi get trans irq], get hispi failed\n");
        return -ENODEV;
    }
    if (hispi->soc_ops->get_trans_irq == NULL) {
        pr_err("get_trans_irq ops is null\n");
        return -1;
    }
    hispi->soc_ops->get_trans_irq(&hispi->reg, status);
    return 0;
}
EXPORT_SYMBOL(kdrv_spi_get_trans_irq);

int kdrv_spi_get_err_irq(u32 spi_no, struct kdrv_spi_err_irq *status)
{
    struct hisi_spi *hispi = NULL;
    if (status == NULL) {
        pr_err("[spi get err irq], para is null ptr\n");
        return -1;
    }

    hispi = hisi_spi_get_by_spi_no(spi_no);
    if (hispi == NULL) {
        pr_err("[spi get err irq], get hispi failed\n");
        return -ENODEV;
    }
    if (hispi->soc_ops->get_err_irq == NULL) {
        pr_err("get_err_irq ops is null\n");
        return -1;
    }
    hispi->soc_ops->get_err_irq(&hispi->reg, status);
    return 0;
}
EXPORT_SYMBOL(kdrv_spi_get_err_irq);

int kdrv_spi_get_fifo_info(u32 spi_no, struct kdrv_spi_fifo *fifo)
{
    struct hisi_spi *hispi = NULL;
    if (fifo == NULL) {
        pr_err("[kdrv_spi_get_fifo_info], para is null ptr\n");
        return -1;
    }

    hispi = hisi_spi_get_by_spi_no(spi_no);
    if (hispi == NULL) {
        pr_err("[kdrv_spi_get_fifo_info], get hispi failed\n");
        return -ENODEV;
    }
    if (hispi->soc_ops->get_fifo_info == NULL) {
        pr_err("get_fifo_info ops is null\n");
        return -1;
    }
    hispi->soc_ops->get_fifo_info(&hispi->reg, fifo);
    return 0;
}
EXPORT_SYMBOL(kdrv_spi_get_fifo_info);

int kdrv_spi_reset(u32 spi_no)
{
    struct hisi_spi *hispi = NULL;
    int ret;
    hispi = hisi_spi_get_by_spi_no(spi_no);
    if (hispi == NULL) {
        pr_err("[spi reset], get hispi failed");
        return -ENODEV;
    }
    ret = hisi_spi_reset(hispi);
    if (ret != 0) {
        pr_err("[spi reset], reset spi:0x%x failed", spi_no);
        return ret;
    }
    return ret;
}
EXPORT_SYMBOL(kdrv_spi_reset);
