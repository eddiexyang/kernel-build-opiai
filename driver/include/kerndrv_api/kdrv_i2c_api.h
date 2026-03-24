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
 * Description: External APIs of the I2C module
 * Author: huawei
 * Create: 2022-05-9
 */

#ifndef KDRV_I2C_API_H
#define KDRV_I2C_API_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_FMEA
/**
 * @brief i2c fmea inspection entry
 * @param devid                  i2c controller ID
 * @param err_info               Error information returned by the fmea
 * @param alm_flg                Indicates the alarm type returned by the FMEA.
 * @param buf                    Start address of the description string returned by the FMEA.
 * @param size                   Indicates the size of the buf.
 * @return int                   Return value. 0: success Others: failure
 */
int kdrv_i2c_fmea_entry(u32 devid, u64 *err_info, u32 *alm_flg, char *buf, u32 size);
#endif
/**
 * @brief Obtains the I2C transmission error status.
 * @param[in] bus_id                 i2c bus id
 * @param[out] status                Pointer to the transmission error status.
 * @return int                       Return value. 0: success Others: failure
 */
union kdrv_i2c_trans_err_status {
    struct {
        u32 user_abrt : 1; // master detects that the transmission is bort, only syns ip support
        u32 arb_lost : 1; // master loses the bus control right, only syns ip support
        u32 arb_master_dis : 1; // user attempts to use a master component that is not enabled. Only SYNS IP supports.
        u32 rd_10b_no_restart : 1; // Restart func disabled, master attempts to initiate a read operation in 10bit mode.
        u32 sbyte_no_restart : 1; // Restart function disabled, master attempts to initiate the start operation.
        u32 hs_no_restart : 1; // Restart function disabled, master attempts to transmit data in high-speed mode.
        u32 sbyte_ackdet : 1; // The master initiates the start command and receives a response.
        u32 hs_ackdet : 1; // The master high-speed mode is responded.
        u32 gcall_read : 1; // master sends a generic request, user initiates a read operation after the request.
        u32 gcall_noack : 1;  // The master sends a general request, but there is no response from the slave on the bus.
        u32 txdata_noack : 1; // addr sent from master to slave is responded, but the sent data is not responded.
        u32 addr2_10b_noack : 1; // aster works in 10bit addr mode, the second send addr not responded by the slave.
        u32 addr1_10b_noack : 1; // aster works in 10bit addr mode, the first send addr not responded by the slave.
        u32 addr_7b_noack : 1; // master works in 7bit addr mode, the transmitted address is not responded by any slave.
        u32 rsv : 18;  // reserved
    } bits;
    u32 err;
};

/**
 * @brief To query the I2C FIFO error status
 * @param[in] bus_id             i2c bus id
 * @param[out] status            Pointer to the FIFO error status structure
 * @return int                   Return value. 0: success Others: failure
 */
union kdrv_i2c_fifo_err_status {
    struct {
        u32 tx_fifo_werror : 1; // tx fifo write error
        u32 tx_fifo_rerror : 1; // tx fifo read error
        u32 rx_fifo_werror : 1; // tx fifo write error
        u32 rx_fifo_rerror : 1; // tx fifo read error
        u32 rsv : 28;    // reserved
    } bits;
    u32 err;
};

struct kdrv_i2c_err_status {
    u32 slv_addr;   //  device addr
    u32 addr_width;  // Address width: 7 bits and 10 bits
    union kdrv_i2c_fifo_err_status fifo_err;
    union kdrv_i2c_trans_err_status trans_err;
};

/**
 * @brief Obtains the I2C error status.
 * @param[in] bus_id              i2c bus id
 * @param[in] status              Pointer to the error status structure.
 * @return int                    Return value. 0: success Others: failure
 */
int kdrv_i2c_get_err_status(u32 bus_id, struct kdrv_i2c_err_status *status);

/**
 * @brief I2C rate mode
 */
enum kdrv_i2c_speed_mode {
    KDRV_I2C_STD_SPEED_MODE,  // Standard rate mode
    KDRV_I2C_FAST_SPEED_MODE,
    KDRV_I2C_HIGH_SPEED_MODE
};

struct kdrv_i2c_timing_cfg {
    enum kdrv_i2c_speed_mode speed_mode;
    u32 scl_hcnt; // Number of SCL high-level cycles in standard mode
    u32 scl_lcnt; // Number of SCL low-level cycles in standard mode
    u32 sda_hold_cnt; // sda_hold number of cycles
    u32 spk_len; // Filtering burr length
};
/**
 * @brief Obtains the I2C timing configuration.
 * @param[in] bus_id             I2C controller ID
 * @param[out] cfg               Pointer to the timing configuration structure
 * @return int                   Return value. 0: success Others: failure
 */
int kdrv_get_i2c_timing_cfg(u32 bus_id, struct kdrv_i2c_timing_cfg *cfg);

struct kdrv_i2c_trans_statistic_info {
    int tx_cnt;
    int rx_cnt;
    int err_times;
    int recovery_times;
};
/**
 * @brief Obtains I2C statistics.
 * @param[in] bus_id              I2C controller ID
 * @param[out] info               Pointer to the transmission statistics structure
 * @return int                    Return value. 0: success Others: failure
 */
int kdrv_get_i2c_trans_statistic_info(u32 bus_id, struct kdrv_i2c_trans_statistic_info *info);
#ifdef __cplusplus
}
#endif
#endif

