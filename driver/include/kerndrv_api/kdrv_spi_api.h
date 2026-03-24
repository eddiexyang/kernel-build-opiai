/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Description: SPI External Interfaces
 * Author: huawei
 * Create: 2022-10-25
 */

#ifndef KDRV_SPI_API_H
#define KDRV_SPI_API_H

#ifdef __cplusplus
extern "C" {
#endif

enum kdrv_spi_mode {
    KDRV_SPI_MODE_0,
    KDRV_SPI_MODE_1,
    KDRV_SPI_MODE_2,
    KDRV_SPI_MODE_3,
};

enum kdrv_spi_signific_bit {
    KDRV_SPI_MSB_FIRST,
    KDRV_SPI_LSB_FIRST,
};

enum kdrv_spi_cs_active {
    KDRV_SPI_CS_LOW,
    KDRV_SPI_CS_HIGH,
};

/**
 * @brief SPI device configuration information structure
 */
struct kdrv_spi_device_config {
    u32 speed;  /* baud rate, clock rate */
    enum kdrv_spi_mode mode;
    enum kdrv_spi_signific_bit lsb;
    enum kdrv_spi_cs_active cs_high;
    u32 bits_per_word;  /* Transfer bit width */
    bool is_loop;
};

/**
 * @brief SPI controller configuration information structure
 */
struct kdrv_spi_master_config {
    u32 cs; /* CS currently used by the controller */
    struct kdrv_spi_device_config cfg;
};

struct kdrv_spi_trans_status {
    u32 busy;
    u32 rx_full;
    u32 rx_not_empty;
    u32 tx_not_full;
    u32 tx_empty;
};

struct kdrv_spi_trans_irq {
    u32 tx_irq;
    u32 rx_irq;
    u32 rx_overflow;
    u32 rx_timeout;
};

struct kdrv_spi_err_irq {
    u32 mstis; /* Multi-host contention bus error */
    u32 rf_ecc_1bit_err; /* Receive FIFO ECC 1bit err irq */
    u32 rf_ecc_multi_err; /* Receive FIFO ECC Multi-bit error */
    u32 tf_ecc_1bit_err; /* Send FIFO ECC 1bit error */
    u32 tf_ecc_multi_err; /* Send FIFO ECC Multi-bit error  */
};

struct kdrv_spi_fifo {
    u32 intr_lvl_tx; /* TX FIFO request interrupt threshold */
    u32 intr_lvl_rx; /* RX FIFO request interrupt threshold */
};

/**
 * @brief Sets chip selection, mode, and rate of the SPI device.Interface not invoked, the default configuration: DTS.
 * @param[in] spi_no                 SPI controller ID. For example, 0 to 3 indicate SPI0 to SPI3.
 * @param[in] cs                     Chip selection corresponding to the SPI device
 * @param[in] config                 Configuration information structure
 * @return int                       Return value. 0: success Others: failure
 */
int kdrv_spi_set_device_config(u32 spi_no, u32 cs, struct kdrv_spi_device_config *config);
/**
 * @brief 启动spi传输
 * @param[in] spi_no                 Spi Controller ID, 0 to 3 indicate SPI0 to SPI3.
 * @param[in] cs                     Chip selection corresponding to the SPI device
 * @param[in] tx_buf                 Address for sending data
 * @param[out] rx_buf                Address for receiving data
 * @param[in] len                    Length of the transmitted data (received data len = transmitted data len)
 * @return int                       Return value. 0: success Others: failure
 */
int kdrv_spi_start_trans(u32 spi_no, u32 cs, const void *tx_buf, void *rx_buf, u32 len);
/**
 * @brief To query the SPI controller configuration information
 * @param[in] spi_no                 Spi Controller ID, 0 to 3 indicate SPI0 to SPI3.
 * @param[out] config                Current configuration information of the SPI controller
 * @return int                       Return value. 0: success Others: failure
 */
int kdrv_spi_get_master_current_config(u32 spi_no, struct kdrv_spi_master_config *config);
/**
 * @brief Querying SPI Device Configurations
 * @param[in] spi_no                 Spi Controller ID, 0 to 3 indicate SPI0 to SPI3.
 * @param[in] cs                     SPI device chip selection
 * @param[out] config                Configuration information of the SPI device
 * @return int                       Return value. 0: success Others: failure
 */
int kdrv_spi_get_device_config(u32 spi_no, u32 cs, struct kdrv_spi_device_config *config);
/**
 * @brief Querying the SPI Controller Status
 * @param[in] spi_no                 Spi Controller ID
 * @param[out] status                status register
 * @return int                       Return value. 0: success Others: failure
 */
int kdrv_spi_get_trans_status(u32 spi_no, struct kdrv_spi_trans_status *status);
/**
 * @brief To query the transmission interrupt status of the SPI controller
 * @param[in] spi_no                 Spi Controller ID
 * @param[out] status                Transmission Interrupt Status
 * @return int                       Return value. 0: success Others: failure
 */
int kdrv_spi_get_trans_irq(u32 spi_no, struct kdrv_spi_trans_irq *status);
/**
 * @brief To query the SPI controller error interrupt status
 * @param[in] spi_no                 Spi Controller ID
 * @param[out] status                Error interrupt status
 * @return int                       Return value. 0: success Others: failure
 */
int kdrv_spi_get_err_irq(u32 spi_no, struct kdrv_spi_err_irq *status);
/**
 * @brief Obtains the current waterline information of the controller.
 * @param[in] spi_no                 Spi Controller ID
 * @param[out] fifo                  Controller waterline information structure
 * @return int                       Return value. 0: success Others: failure
 */
int kdrv_spi_get_fifo_info(u32 spi_no, struct kdrv_spi_fifo *fifo);
/**
 * @brief Resetting the SPI Controller
 * @param[in] spi_no                 Spi Controller ID
 * @return int                       Return value. 0: success Others: failure
 */
int kdrv_spi_reset(u32 spi_no);
/**
 * @brief spi fmea Inspection Entry
 * @param[in] devid                  Spi Controller ID
 * @param[out] err_info              Error information returned by the fmea
 * @param[out] alm_flg               Indicates the alarm type returned by the FMEA.
 * @param[out] buf                   Start address of the description string returned by the FMEA.
 * @param[in] size                   buf size
 * @return int                       Return value. 0: success Others: failure
 */
#ifdef CONFIG_FMEA
int kdrv_spi_fmea_entry(u32 devid, u64 *err_info, u32 *alm_flg, char *buf, u32 size);
#endif
#ifdef __cplusplus
}
#endif
#endif
