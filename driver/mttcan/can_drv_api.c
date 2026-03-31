/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#include "can_drv_api.h"

#include <linux/securec.h>

#include "can_drv_log.h"
#include "can_drv_mttcan.h"
#include "can_drv_core.h"

#ifdef STATIC_SKIP
    #define STATIC
#else
    #define STATIC static
#endif

#define VALID_CAN_NUN           13
#define MAX_CAN_NAME_LEN        15
#define VALID_DATA_SIZE_TYPES   8
#define CAN_NAME_PREFIX         "can"
#define CAN_NAME_PREFIX_LEN     3
#define CAN_ID_MAX_LEN          2
#define SID_FILTER_HIGH32_SHIFT 32
#define DECIMAL_CARRY_NUM       10
#define CAN_ID_LOW_OFFSET       1
#define XID_AND_MASK_RES_MASK   0x1FFFFFFF

#define TDCO_MIN_VALUE          0
#define TDCO_MAX_VALUE          127
#define TDCF_MIN_VALUE          0
#define TDCF_MAX_VALUE          127
#define DBRP_VALUE_1            1
#define DBRP_VALUE_2            2

#define BUSOFF_CONFIG_SIZE  (sizeof(struct busoff_config_param))
#define RAM_CONFIG_STRU_SIZE (sizeof(struct can_config_stru))
#define TDC_CONFIG_SIZE (sizeof(struct can_tdc_cfg_stru))

STATIC inline void mttcan_echo_cfg(const char *cfg_name, u32 new_cfg, u32 old_cfg)
{
    mttcan_info("%s is changed from %u to %u.\n", cfg_name, old_cfg, new_cfg);
}

union sid_filter_union {
    u32 val;
    struct {
        u32 sfid2 : 11;
        u32 res : 4;
        u32 ssync : 1;
        u32 sfid1 : 11;
        u32 sfec : 3;
        u32 sft : 2;
    };
};

union xid_filter_union {
    struct {
        u32 f1;
        u32 f0;
    };
    struct {
        u32 efid2 : 29;
        u32 esync : 1;
        u32 eft : 2;
        u32 efid1 : 29;
        u32 efec : 3;
    };
};

union global_filter_union {
    u32 val;
    struct {
        u32 rrfe : 1;
        u32 rrfs : 1;
        u32 anfe : 2;
        u32 anfs : 2;
        u32 res : 26;
    };
};

const int valid_can_ids[VALID_CAN_NUN] = {
    0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 14};
const int data_size_map[VALID_DATA_SIZE_TYPES] = {
    8, 12, 16, 20, 24, 32, 48, 64};

STATIC int get_id_from_name(const unsigned char *name, unsigned int namelen)
{
    int i;
    int ret;
    int can_id = -1;
    int can_id_len;

    if (unlikely(name == NULL || namelen <= CAN_NAME_PREFIX_LEN)) {
        mttcan_err("get_id_from_name, name or namelen id invalid.\n");
        return -1;
    }

    if (memcmp(name, CAN_NAME_PREFIX, CAN_NAME_PREFIX_LEN) != 0) {
        mttcan_err("get_id_from_name, name prefix is invalid.\n");
        return -1;
    }

    can_id_len = namelen - CAN_NAME_PREFIX_LEN;

    if (can_id_len > CAN_ID_MAX_LEN) {
        mttcan_err("get_id_from_name, id len is invalid.\n");
        return -1;
    }

    ret = kstrtou32((char*)(name + CAN_NAME_PREFIX_LEN), DECIMAL_CARRY_NUM, (u32*)&can_id);
    if (ret) {
        mttcan_err("get_id_from_name, kstrtou32 fail.\n");
        return -1;
    }

    for (i = 0; i < VALID_CAN_NUN; i++) {
        if (can_id == valid_can_ids[i]) {
            return can_id;
        }
    }

    mttcan_err("get_id_from_name, id is invalid.\n");
    return -1;
}

STATIC u32 get_real_data_size(u32 fake_size)
{
    return data_size_map[fake_size];
}

STATIC inline u32 get_sid_filter_val(const struct sid_filter *sid_cfg)
{
    union sid_filter_union filter;
    filter.sfid2 = sid_cfg->sfid2;
    filter.res = 0;
    filter.ssync = sid_cfg->ssync;
    filter.sfid1 = sid_cfg->sfid1;
    filter.sfec = sid_cfg->sfec;
    filter.sft = sid_cfg->sft;
    return filter.val;
}

STATIC inline void set_sid_filter_val(struct sid_filter *sid_cfg, const u32 val)
{
    union sid_filter_union filter;
    filter.val = val;
    sid_cfg->res = filter.res;
    sid_cfg->sfec = filter.sfec;
    sid_cfg->sfid1 = filter.sfid1;
    sid_cfg->sfid2 = filter.sfid2;
    sid_cfg->sft = filter.sft;
    sid_cfg->ssync = filter.ssync;
}

STATIC inline void get_xid_filter_val(const struct xid_filter *xid_cfg, u32 *xid_f0, u32 *xid_f1)
{
    union xid_filter_union filter;
    filter.efid2 = xid_cfg->efid2;
    filter.esync = xid_cfg->esync;
    filter.eft = xid_cfg->eft;
    filter.efid1 = xid_cfg->efid1;
    filter.efec = xid_cfg->efec;
    *xid_f0 = filter.f0;
    *xid_f1 = filter.f1;
}

STATIC inline void set_xid_filter_val(struct xid_filter *xid_cfg, u32 xid_f0, u32 xid_f1)
{
    union xid_filter_union filter;
    filter.f0 = xid_f0;
    filter.f1 = xid_f1;
    xid_cfg->efec = filter.efec;
    xid_cfg->efid1 = filter.efid1;
    xid_cfg->efid2 = filter.efid2;
    xid_cfg->eft = filter.eft;
    xid_cfg->esync = filter.esync;
}

STATIC inline u32 get_global_filter_val(const struct global_filter global_filter_cfg)
{
    union global_filter_union filter;
    filter.anfe = global_filter_cfg.anfe;
    filter.anfs = global_filter_cfg.anfs;
    filter.rrfe = global_filter_cfg.rrfe;
    filter.rrfs = global_filter_cfg.rrfs;
    filter.res = 0;
    return filter.val;
}

STATIC inline void set_global_filter_val(struct global_filter *global_filter_cfg, u32 val)
{
    union global_filter_union filter;
    filter.val = val;
    global_filter_cfg->anfe = filter.anfe;
    global_filter_cfg->anfs = filter.anfs;
    global_filter_cfg->res = filter.res;
    global_filter_cfg->rrfe = filter.rrfe;
    global_filter_cfg->rrfs = filter.rrfs;
}

STATIC void get_rx_fifo0_cfg(const struct mttcan_priv *priv, struct can_config_stru *can_cfg)
{
    u32 fifo_cfg = mttcan_read_reg(&priv->addr, REG_RXF0C);
    can_cfg->mode_rxf0 = (CAN_RX_FIFO_MODE)((fifo_cfg & RXF0C_F0OM_BIT) >> RXF0C_F0OM_SHIFT);
    can_cfg->watermark_rxf0 = (fifo_cfg & RXF0C_F0WM_MASK) >> RXF0C_F0WM_SHIFT;
    can_cfg->element_num_rxf0 = (fifo_cfg & RXF0C_F0S_MASK) >> RXF0C_F0S_SHIFT;
}

STATIC void get_rx_fifo1_cfg(const struct mttcan_priv *priv, struct can_config_stru *can_cfg)
{
    u32 fifo_cfg = mttcan_read_reg(&priv->addr, REG_RXF1C);
    can_cfg->mode_rxf1 = (CAN_RX_FIFO_MODE)((fifo_cfg & RXF1C_F0OM_BIT) >> RXF1C_F0OM_SHIFT);
    can_cfg->watermark_rxf1 = (fifo_cfg & RXF1C_F1WM_MASK) >> RXF1C_F1WM_SHIFT;
    can_cfg->element_num_rxf1 = (fifo_cfg & RXF1C_F1S_MASK) >> RXF1C_F1S_SHIFT;
}

STATIC void get_rx_buffer_fifo_element_size(const struct mttcan_priv *priv, struct can_config_stru *can_cfg)
{
    u32 dsize_rxb;
    u32 dsize_fifo0;
    u32 dsize_fifo1;
    u32 element_size = mttcan_read_reg(&priv->addr, REG_RXESC);
    dsize_rxb = (element_size & RXESC_RBDS_MASK) >> RXESC_RBDS_SHIFT;
    dsize_fifo1 = (element_size & RXESC_F1DS_MASK) >> RXESC_F1DS_SHIFT;
    dsize_fifo0 = (element_size & RXESC_F0DS_MASK) >> RXESC_F0DS_SHIFT;
    can_cfg->dsize_rxb = get_real_data_size(dsize_rxb);
    can_cfg->dsize_fifo1 = get_real_data_size(dsize_fifo1);
    can_cfg->dsize_fifo0 = get_real_data_size(dsize_fifo0);
}

STATIC void get_tx_buffer_cfg(const struct mttcan_priv *priv, struct can_config_stru *can_cfg)
{
    u32 tx_buf_cfg = mttcan_read_reg(&priv->addr, REG_TXBC);
    can_cfg->mode_txfq = (CAN_TX_FIFO_QUEUE_MODE)((tx_buf_cfg & TXBC_TFQM_BIT) >> TXBC_TFQM_SHIFT);
    can_cfg->tx_elmt_num_fifo_queue = (tx_buf_cfg & TXBC_TFQS_MASK) >> TXBC_TFQS_SHIFT;
    can_cfg->tx_elmt_num_dedicated_buf = (tx_buf_cfg & TXBC_NDTB_MASK) >> TXBC_NDTB_SHIFT;
    can_cfg->element_num_txb = can_cfg->tx_elmt_num_fifo_queue + can_cfg->tx_elmt_num_dedicated_buf;
}

STATIC void get_tx_event_fifo_cfg(const struct mttcan_priv *priv, struct can_config_stru *can_cfg)
{
    u32 tx_event_cfg = mttcan_read_reg(&priv->addr, REG_TXEFC);
    can_cfg->watermark_txef = (tx_event_cfg & TXEFC_EFWM_MASK) >> TXEFC_EFWM_SHIFT;
    can_cfg->element_num_txef = (tx_event_cfg & TXEFC_EFS_MASK) >> TXEFC_EFS_SHIFT;
}

STATIC void get_tx_buffer_element_size(const struct mttcan_priv *priv, struct can_config_stru *can_cfg)
{
    u32 dsize_txb;
    u32 tx_element_size = mttcan_read_reg(&priv->addr, REG_TXESC);
    dsize_txb = (tx_element_size & TXESC_TBDS_MASK) >> TXESC_TBDS_SHIFT;
    can_cfg->dsize_txb = get_real_data_size(dsize_txb);
}

STATIC void get_sid_filter_cfg(const struct mttcan_priv *priv, struct can_config_stru *can_cfg)
{
    u32 filter_cfg;

    filter_cfg = mttcan_read_reg(&priv->addr, REG_SIDFC);
    can_cfg->element_num_sidf = (filter_cfg & SIDFC_LSS_MASK) >> SIDFC_LSS_SHIFT;
}

STATIC void get_xid_filter_cfg(const struct mttcan_priv *priv, struct can_config_stru *can_cfg)
{
    u32 filter_cfg;

    filter_cfg = mttcan_read_reg(&priv->addr, REG_XIDFC);
    can_cfg->element_num_xidf = (filter_cfg & XIDFC_LES_MASK) >> XIDFC_LES_SHIFT;
}

STATIC void get_global_filter_cfg(const struct mttcan_priv *priv, struct can_config_stru *can_cfg)
{
    u32 val = mttcan_read_reg(&priv->addr, REG_GFC);
    set_global_filter_val(&can_cfg->global_filter, val);
}

STATIC void get_xid_and_mask(const struct mttcan_priv *priv, struct can_config_stru *can_cfg)
{
    can_cfg->xid_and_mask = mttcan_read_reg(&priv->addr, REG_XIDAM);
}

int get_can_config(unsigned char *name, unsigned int namelen, struct can_config_stru *can_cfg)
{
    int can_id;
    struct net_device *ndev = NULL;
    struct mttcan_priv *priv = NULL;

    can_id = get_id_from_name(name, namelen);
    if (unlikely(can_id < 0)) {
        mttcan_err("get can%d config failed, canid is invalid.\n", can_id);
        return -EINVAL;
    }

    if (unlikely(can_cfg == NULL)) {
        mttcan_err("get can%d config failed, param is invalid.\n", can_id);
        return -EINVAL;
    }

    ndev = get_net_devices(can_id);
    if (unlikely(ndev == NULL)) {
        mttcan_err("get can%d config failed, not probe.\n", can_id);
        return -ENODEV;
    }

    priv = netdev_priv(ndev);
    get_rx_fifo0_cfg(priv, can_cfg);
    get_rx_fifo1_cfg(priv, can_cfg);
    get_rx_buffer_fifo_element_size(priv, can_cfg);
    get_tx_buffer_cfg(priv, can_cfg);
    get_tx_buffer_element_size(priv, can_cfg);
    get_tx_event_fifo_cfg(priv, can_cfg);
    get_sid_filter_cfg(priv, can_cfg);
    get_xid_filter_cfg(priv, can_cfg);
    get_global_filter_cfg(priv, can_cfg);
    get_xid_and_mask(priv, can_cfg);
    can_cfg->echo_skb_max = priv->cfg.echo_skb_max;
    can_cfg->poll_weight = priv->cfg.poll_weight;
    can_cfg->ts_cnt_prescaler = priv->cfg.ts_cnt_prescaler;
    can_cfg->element_num_rxb = priv->cfg.element_num[ELMT_RXB];
    can_cfg->element_num_tmc = priv->cfg.element_num[ELMT_TMC];

    return RET_OK;
}
EXPORT_SYMBOL(get_can_config);

STATIC void get_can_bus_status(const struct net_device *ndev, struct can_status_stru *can_stat)
{
    u32 psr;
    struct mttcan_priv *priv = netdev_priv(ndev);

    if (!netif_running(ndev)) {
        can_stat->bus_state = CAN_BUS_STATE_DOWN;
        return;
    }

    psr = mttcan_read_reg(&priv->addr, REG_PSR);
    if (psr & PSR_BO_BIT) {
        can_stat->bus_state = CAN_BUS_STATE_ERR_BUSOFF;
    } else if (psr & PSR_EP_BIT) {
        can_stat->bus_state = CAN_BUS_STATE_ERR_PASSIVE;
    } else if (psr & PSR_EW_BIT) {
        can_stat->bus_state = CAN_BUS_STATE_ERR_WARNING;
    } else {
        can_stat->bus_state = CAN_BUS_STATE_ACTIVER;
    }
}

int get_can_status(const unsigned char *name, unsigned int namelen, struct can_status_stru *can_stat)
{
    int can_id;
    struct net_device *ndev = NULL;

    can_id = get_id_from_name(name, namelen);
    if (unlikely(can_id == -1)) {
        mttcan_err("get can%d status failed, canid is invalid.\n", can_id);
        return -EINVAL;
    }

    if (unlikely(can_stat == NULL)) {
        mttcan_err("get can%d status failed, param is invalid.\n", can_id);
        return -EINVAL;
    }

    ndev = get_net_devices(can_id);
    if (unlikely(ndev == NULL)) {
        mttcan_err("get can%d status failed, not probe.\n", can_id);
        return -ENODEV;
    }

    get_can_bus_status(ndev, can_stat);
    get_err_counter(ndev, &can_stat->rx_err_counter, &can_stat->tx_err_counter);

    return RET_OK;
}
EXPORT_SYMBOL(get_can_status);

STATIC void mttcan_echo_can_cfg(struct can_config_stru *can_config, struct mttcan_config *cfg)
{
    mttcan_echo_cfg("ELMT_SIDF", cfg->element_num[ELMT_SIDF], can_config->element_num_sidf);
    mttcan_echo_cfg("ELMT_XIDF", cfg->element_num[ELMT_XIDF], can_config->element_num_xidf);
    mttcan_echo_cfg("ELMT_RXF0", cfg->element_num[ELMT_RXF0], can_config->element_num_rxf0);
    mttcan_echo_cfg("ELMT_RXF1", cfg->element_num[ELMT_RXF1], can_config->element_num_rxf1);
    mttcan_echo_cfg("ELMT_RXB", cfg->element_num[ELMT_RXB], can_config->element_num_rxb);
    mttcan_echo_cfg("ELMT_TXEF", cfg->element_num[ELMT_TXEF], can_config->element_num_txef);
    mttcan_echo_cfg("ELMT_TXB", cfg->element_num[ELMT_TXB], can_config->element_num_txb);
    mttcan_echo_cfg("ELMT_TMC", cfg->element_num[ELMT_TMC], can_config->element_num_tmc);
    mttcan_echo_cfg("TX_ELMT_BUF", cfg->tx_element_num[TX_ELMT_BUF], can_config->tx_elmt_num_dedicated_buf);
    mttcan_echo_cfg("TX_ELMT_FQ", cfg->tx_element_num[TX_ELMT_FQ], can_config->tx_elmt_num_fifo_queue);
    mttcan_echo_cfg("WATERMARK_RXF0", cfg->watermark[WATERMARK_RXF0], can_config->watermark_rxf0);
    mttcan_echo_cfg("WATERMARK_RXF1", cfg->watermark[WATERMARK_RXF1], can_config->watermark_rxf1);
    mttcan_echo_cfg("WATERMARK_TXEF", cfg->watermark[WATERMARK_TXEF], can_config->watermark_txef);
    mttcan_echo_cfg("DSIZE_RXF0", cfg->dsize[DSIZE_RXF0], can_config->dsize_fifo0);
    mttcan_echo_cfg("DSIZE_RXF1", cfg->dsize[DSIZE_RXF1], can_config->dsize_fifo1);
    mttcan_echo_cfg("DSIZE_RXB", cfg->dsize[DSIZE_RXB], can_config->dsize_rxb);
    mttcan_echo_cfg("DSIZE_TXB", cfg->dsize[DSIZE_TXB], can_config->dsize_txb);
    mttcan_echo_cfg("MODE_RXF0", cfg->mode[MODE_RXF0], can_config->mode_rxf0);
    mttcan_echo_cfg("MODE_RXF1", cfg->mode[MODE_RXF1], can_config->mode_rxf1);
    mttcan_echo_cfg("MODE_TXFQ", cfg->mode[MODE_TXFQ], can_config->mode_txfq);
    mttcan_echo_cfg("XIDAM", cfg->extend_id_and_mask, can_config->xid_and_mask);
    mttcan_echo_cfg("ECHO_SKB_MAX", cfg->echo_skb_max, can_config->echo_skb_max);
    mttcan_echo_cfg("POLL_WEIGHT", cfg->poll_weight, can_config->poll_weight);
    mttcan_echo_cfg("PRESCALER", cfg->ts_cnt_prescaler, can_config->ts_cnt_prescaler);
}

STATIC int mttcan_check_rx_cfg(struct mttcan_config *cfg, const struct can_config_stru *can_stru)
{
    cfg->element_num[ELMT_RXF0] = can_stru->element_num_rxf0;
    if (can_stru->element_num_rxf0 > ELMT_NUM_MAX_RXF0) {
        mttcan_err("invalid element_num_rxf0:is %u \n", can_stru->element_num_rxf0);
        return -1;
    }

    cfg->element_num[ELMT_RXF1] = can_stru->element_num_rxf1;
    if (can_stru->element_num_rxf1 > ELMT_NUM_MAX_RXF1) {
        mttcan_err("invalid element_num_rxf1:is %u \n", can_stru->element_num_rxf1);
        return -1;
    }

    cfg->element_num[ELMT_RXB] = can_stru->element_num_rxb;
    if (can_stru->element_num_rxb > ELMT_NUM_MAX_RXB) {
        mttcan_err("invalid element_num_rxb:is %u \n", can_stru->element_num_rxb);
        return -1;
    }

    cfg->watermark[WATERMARK_RXF0] = can_stru->watermark_rxf0;
    if (can_stru->watermark_rxf0 > can_stru->element_num_rxf0) {
        mttcan_err("invalid watermark_rxf0:is %u \n", can_stru->watermark_rxf0);
        return -1;
    }

    cfg->watermark[WATERMARK_RXF1] = can_stru->watermark_rxf1;
    if (can_stru->watermark_rxf1 > can_stru->element_num_rxf1) {
        mttcan_err("invalid watermark_rxf1:is %u \n", can_stru->watermark_rxf1);
        return -1;
    }
    return 0;
}

STATIC int mttcan_check_mode_and_dsize_cfg(struct mttcan_config *cfg, const struct can_config_stru *can_stru)
{
    cfg->mode[MODE_RXF0] = (CAN_RX_FIFO_MODE)can_stru->mode_rxf0;
    if ((can_stru->mode_rxf0 != 0) && (can_stru->mode_rxf0 != 1)) {
        mttcan_err("invalid mode_rxf0 :is %u.\n", can_stru->mode_rxf0);
        return -1;
    }

    cfg->mode[MODE_RXF1] = (CAN_RX_FIFO_MODE)can_stru->mode_rxf1;
    if ((can_stru->mode_rxf1 != 0) && (can_stru->mode_rxf1 != 1)) {
        mttcan_err("invalid mode_rxf1 :is %u.\n", can_stru->mode_rxf1);
        return -1;
    }

    cfg->mode[MODE_TXFQ] = (CAN_TX_FIFO_QUEUE_MODE)can_stru->mode_txfq;
    if ((can_stru->mode_txfq != 0) && (can_stru->mode_txfq != 1)) {
        mttcan_err("invalid mode_txfq :is %u.\n", can_stru->mode_txfq);
        return -1;
    }

    cfg->dsize[DSIZE_RXF0] = can_stru->dsize_fifo0;
    if (!mttcan_is_data_size_valid(can_stru->dsize_fifo0)) {
        mttcan_err("invalid dsize_fifo0:is %u \n", can_stru->dsize_fifo0);
        return -1;
    }

    cfg->dsize[DSIZE_RXF1] = can_stru->dsize_fifo1;
    if (!mttcan_is_data_size_valid(can_stru->dsize_fifo1)) {
        mttcan_err("invalid dsize_fifo1:is %u \n", can_stru->dsize_fifo1);
        return -1;
    }

    cfg->dsize[DSIZE_RXB] = can_stru->dsize_rxb;
    if (!mttcan_is_data_size_valid(can_stru->dsize_rxb)) {
        mttcan_err("invalid dsize_rxb:is %u \n", can_stru->dsize_rxb);
        return -1;
    }

    cfg->dsize[DSIZE_TXB] = can_stru->dsize_txb;
    if (!mttcan_is_data_size_valid(can_stru->dsize_txb)) {
        mttcan_err("invalid dsize_txb:is %u \n", can_stru->dsize_txb);
        return -1;
    }
    return 0;
}

STATIC int mttcan_check_tx_cfg(struct mttcan_config *cfg, const struct can_config_stru *can_stru)
{
    cfg->element_num[ELMT_TXB] = can_stru->element_num_txb;
    if (can_stru->element_num_txb > ELMT_NUM_MAX_TXB) {
        mttcan_err("invalid element_num_txb:is %u \n", can_stru->element_num_txb);
        return -1;
    }

    cfg->tx_element_num[TX_ELMT_BUF] = can_stru->tx_elmt_num_dedicated_buf;
    cfg->tx_element_num[TX_ELMT_FQ] = can_stru->tx_elmt_num_fifo_queue;
    if (can_stru->tx_elmt_num_dedicated_buf + can_stru->tx_elmt_num_fifo_queue > can_stru->element_num_txb) {
        mttcan_err("invalid tx_dedicated_buf = %u, tx_fifo_queue = %u, larger than element_num_txb - %u\n",
            can_stru->tx_elmt_num_dedicated_buf, can_stru->tx_elmt_num_fifo_queue, can_stru->element_num_txb);
        return -1;
    }

    cfg->element_num[ELMT_TXEF] = can_stru->element_num_txef;
    if (can_stru->element_num_txef > ELMT_NUM_MAX_TXEF) {
        mttcan_err("invalid element_num_txef:is %u \n", can_stru->element_num_txef);
        return -1;
    }

    cfg->element_num[ELMT_TMC] = can_stru->element_num_tmc;
    if (can_stru->element_num_tmc > ELMT_NUM_MAX_TRIG_MEM) {
        mttcan_err("invalid element_num_tmc:is %u \n", can_stru->element_num_tmc);
        return -1;
    }

    cfg->watermark[WATERMARK_TXEF] = can_stru->watermark_txef;
    if (can_stru->watermark_txef > can_stru->element_num_txef) {
        mttcan_err("invalid watermark_txef:is %u \n", can_stru->watermark_txef);
        return -1;
    }
    return 0;
}

STATIC int mttcan_check_others_cfg(struct mttcan_config *cfg,
    const struct can_config_stru *can_stru, unsigned char tdc_flag)
{
    cfg->extend_id_and_mask = can_stru->xid_and_mask & XID_AND_MASK_RES_MASK;
    cfg->echo_skb_max = can_stru->echo_skb_max;
    cfg->poll_weight = can_stru->poll_weight;

    /*  When TDC = '1', the Data Bit Rate Prescaler range is limited to 0,1 */
    if ((tdc_flag == TDC_FLAG_AUTOADAPTER) || (tdc_flag == TDC_FLAG_ENABLE)) {
        if ((can_stru->ts_cnt_prescaler != DBRP_VALUE_1) && (can_stru->ts_cnt_prescaler != DBRP_VALUE_2)) {
            mttcan_err("when TDC enable, invalid ts_cnt_prescaler:is %u \n", can_stru->ts_cnt_prescaler);
            return -1;
        }
    }

    cfg->ts_cnt_prescaler = can_stru->ts_cnt_prescaler;
    if ((can_stru->ts_cnt_prescaler < 1) || (can_stru->ts_cnt_prescaler > TIMEBASE_PRE_MAX)) {
        mttcan_err("invalid ts_cnt_prescaler:is %u \n", can_stru->ts_cnt_prescaler);
        return -1;
    }
    return 0;
}

STATIC inline int mttcan_check_sid_cfg(struct mttcan_config *cfg, const struct can_config_stru *can_stru)
{
    cfg->element_num[ELMT_SIDF] = can_stru->element_num_sidf;
    if (can_stru->element_num_sidf > ELMT_NUM_MAX_SIDF) {
        mttcan_err("invalid element_num_sidf:is %u \n", can_stru->element_num_sidf);
        return -1;
    }
    return 0;
}

STATIC inline int mttcan_check_xid_cfg(struct mttcan_config *cfg, const struct can_config_stru *can_stru)
{
    cfg->element_num[ELMT_XIDF] = can_stru->element_num_xidf;
    if (can_stru->element_num_xidf > ELMT_NUM_MAX_XIDF) {
        mttcan_err("invalid element_num_xidf:is %u \n", can_stru->element_num_xidf);
        return -1;
    }
    return 0;
}

STATIC int mttcan_check_global_cfg(struct mttcan_config *cfg, const struct can_config_stru *can_stru)
{
    cfg->global_filter_cfg = get_global_filter_val(can_stru->global_filter);

    if ((can_stru->global_filter.anfe == GLOBAL_FILTER_RECV_RXF0) && (can_stru->element_num_rxf0 == 0)) {
        mttcan_err("invalid, received to fifl0, but the configured size of fifo0 is 0.\n");
        return -1;
    }

    if ((can_stru->global_filter.anfe == GLOBAL_FILTER_RECV_RXF1) && (can_stru->element_num_rxf1 == 0)) {
        mttcan_err("invalid, received to fifl1, but the configured size of fifo1 is 0.\n");
        return -1;
    }

    if ((can_stru->global_filter.anfs == GLOBAL_FILTER_RECV_RXF0) && (can_stru->element_num_rxf0 == 0)) {
        mttcan_err("invalid, received to fifl0, but the configured size of fifo0 is 0.\n");
        return -1;
    }

    if ((can_stru->global_filter.anfs == GLOBAL_FILTER_RECV_RXF1) && (can_stru->element_num_rxf1 == 0)) {
        mttcan_err("invalid, received to fifl1, but the configured size of fifo1 is 0.\n");
        return -1;
    }
    return 0;
}

STATIC int mttcan_check_ram_param(struct mttcan_config *cfg,
    const struct can_config_stru *can_stru, unsigned char tdc_flag)
{
    int ret;

    ret = mttcan_check_rx_cfg(cfg, can_stru);
    if (ret < 0) {
        mttcan_err("mttcan_check_rx_cfg fail \n");
        return ret;
    }

    ret = mttcan_check_mode_and_dsize_cfg(cfg, can_stru);
    if (ret < 0) {
        mttcan_err("mttcan_check_mode_and_dsize_cfg fail \n");
        return ret;
    }

    ret = mttcan_check_tx_cfg(cfg, can_stru);
    if (ret < 0) {
        mttcan_err("mttcan_check_tx_cfg fail \n");
        return ret;
    }

    ret = mttcan_check_others_cfg(cfg, can_stru, tdc_flag);
    if (ret < 0) {
        mttcan_err("mttcan_check_others_cfg fail \n");
        return ret;
    }

    ret = mttcan_check_sid_cfg(cfg, can_stru);
    if (ret < 0) {
        mttcan_err("mttcan_check_sid_cfg fail \n");
        return ret;
    }

    ret = mttcan_check_xid_cfg(cfg, can_stru);
    if (ret < 0) {
        mttcan_err("mttcan_check_xid_cfg fail \n");
        return ret;
    }

    ret = mttcan_check_global_cfg(cfg, can_stru);
    if (ret < 0) {
        mttcan_err("mttcan_check_global_cfg fail \n");
        return ret;
    }
    return 0;
}

STATIC int mttcan_check_ram_size(const struct can_config_stru *can_stru)
{
    u32 offset_addr = 0;

    offset_addr += can_stru->element_num_sidf * ELMT_SIZE_SIDF;
    offset_addr += can_stru->element_num_xidf * ELMT_SIZE_XIDF;
    offset_addr += can_stru->element_num_rxf0 * ELMT_SIZE_RXB_MAX;
    offset_addr += can_stru->element_num_rxf1 * ELMT_SIZE_RXB_MAX;
    offset_addr += can_stru->element_num_rxb * ELMT_SIZE_RXB_MAX;
    offset_addr += can_stru->element_num_txef * ELMT_SIZE_TXEF;
    offset_addr += can_stru->element_num_txb * ELMT_SIZE_TXB_MAX;
    offset_addr += can_stru->element_num_tmc * ELMT_SIZE_TRIG_MEM;
    if (offset_addr >= BYTES_OF_MSG_RAM_MAX) {
        mttcan_err("Incorrect size configuration for message ram! size is %u >= %d\n",
            offset_addr, BYTES_OF_MSG_RAM_MAX);
        return -1;
    }
    return 0;
}

STATIC int mttcan_check_can_ram_config(struct mttcan_config *cfg,
    struct can_config_stru *can_stru, unsigned char tdc_flag)
{
    int ret;

    ret = mttcan_check_ram_param(cfg, can_stru, tdc_flag);
    if (ret < 0) {
        mttcan_err("ram config param have err param\n");
        return -EINVAL;
    }

    ret = mttcan_check_ram_size(can_stru);
    if (ret < 0) {
        mttcan_err("ram config size have err param\n");
        return -EINVAL;
    }
    return 0;
}

STATIC int mttcan_config_take_effect(struct net_device *ndev, struct mttcan_config *cfg)
{
    int ret;
    struct mttcan_priv *priv = netdev_priv(ndev);

    ret = memcpy_s(&priv->cfg, sizeof(struct mttcan_config), cfg, sizeof(struct mttcan_config));
    if (ret != 0) {
        mttcan_err("memory copy failed!\n");
        return RET_ERR_IOCRL_FAILED;
    }

    ret = mttcan_set_mram_parameter(priv, cfg);
    if (ret < 0) {
        mttcan_err("mttcan_config_message_ram failed.\n");
        return -ENODEV;
    }

    /* dedicated tx buffer mask */
    if (priv->cfg.tx_element_num[TX_ELMT_BUF] > 0) {
        priv->dedicated_txb_mask = GENMASK(cfg->tx_element_num[TX_ELMT_BUF] - 1, 0);
    } else {
        priv->dedicated_txb_mask = 0;
    }

    ret = mttcan_init(ndev);
    if (ret < 0) {
        mttcan_err("%s: mttcan_init failed!\n", ndev->name);
        return RET_ERR_IOCRL_FAILED;
    }
    return 0;
}

STATIC void mttcan_cfg_clear_filter(struct mttcan_config *cfg)
{
    int i;
    int max_xid_filter_num = ELMT_NUM_MAX_XIDF << 1;

    for (i = 0; i < ELMT_NUM_MAX_SIDF; i++) {
        cfg->sid_filter[i] = 0;
    }

    for (i = 0; i < max_xid_filter_num; i++) {
        cfg->xid_filter[i] = 0;
    }
}

STATIC struct mttcan_config* mttcan_copy_mttcan_cfg(struct mttcan_priv *priv)
{
    int ret;
    struct mttcan_config *cfg = NULL;

    cfg = kmalloc(sizeof(struct mttcan_config), GFP_KERNEL_ACCOUNT);
    if (cfg == NULL) {
        mttcan_err("kmalloc fail\n");
        return NULL;
    }

    ret = memcpy_s(cfg, sizeof(struct mttcan_config), &priv->cfg, sizeof(struct mttcan_config));
    if (ret != 0) {
        mttcan_err("ram set copy cfg failed!\n");
        kfree(cfg);
        return NULL;
    }
    return cfg;
}

STATIC int mttcan_set_can_ram(struct net_device *ndev, void *data, unsigned int input_size)
{
    int ret;
    struct mttcan_config *cfg = NULL;
    struct mttcan_priv *priv = netdev_priv(ndev);

    /* if canx is down ,do not set config */
    if (netif_running(ndev)) {
        mttcan_err("this net_device is running, can't to set ram\n");
        return -EBUSY;
    }

    if (input_size != RAM_CONFIG_STRU_SIZE) {
        mttcan_err("set ram stru failed, input_size = %u, is not an RAM size %lu.\n",
            input_size, RAM_CONFIG_STRU_SIZE);
        return -EINVAL;
    }

    cfg = mttcan_copy_mttcan_cfg(priv);
    if (cfg == NULL) {
        return RET_ERR_IOCRL_FAILED;
    }

    mttcan_cfg_clear_filter(cfg);

    ret = mttcan_check_can_ram_config(cfg, (struct can_config_stru *)data, priv->tdc_flag);
    if (ret < 0) {
        mttcan_err("mttcan_check_can_ram_config failed!\n");
        goto exit;
    }

    mttcan_echo_can_cfg((struct can_config_stru *)data, &priv->cfg);

    ret = mttcan_config_take_effect(ndev, cfg);
    if (ret < 0) {
        mttcan_err("ram set take effect failed!\n");
        goto exit;
    }

    kfree(cfg);

    mttcan_info("config can ram paramters successfully.\n");
    return 0;
exit:
    kfree(cfg);
    return ret;
}

STATIC int mttcan_check_filter_index(struct mttcan_priv *priv, unsigned int fec)
{
    if ((fec == FEC_STORE_RXF0) || (fec == FEC_SP_STORE_RXF0)) {
        if (priv->cfg.element_num[ELMT_RXF0] == 0) {
            mttcan_err("set recv in rxf0 but rxf0 no space\n");
            return -1;
        }
    }

    if ((fec == FEC_STORE_RXF1) || (fec == FEC_SP_STORE_RXF1)) {
        if (priv->cfg.element_num[ELMT_RXF1] == 0) {
            mttcan_err("set recv in rxf1 but rxf1 no space\n");
            return -1;
        }
    }

    if (fec == FEC_STORE_RXB_DEBUG) {
        if (priv->cfg.element_num[ELMT_RXB] == 0) {
            mttcan_err("set recv in buff but buff no space \n");
            return -1;
        }
    }
    return 0;
}

STATIC int mttcan_check_sid_filter_cfg(struct mttcan_priv *priv, struct sid_filter *filter)
{
    int ret;
    u32 offset;
    u32 filter_val = get_sid_filter_val(filter);

    /* Check whether the space for storing data is available */
    ret = mttcan_check_filter_index(priv, filter->sfec);
    if (ret != 0) {
        mttcan_err("set sid filter failed, filter = 0x%08x\n", filter_val);
        return -1;
    }

    /* RXB must be greater than offset. */
    if (filter->sfec == FEC_STORE_RXB_DEBUG) {
        offset = filter_val & SIDF_SFID2_OFFSET;
        if (offset >= priv->cfg.element_num[ELMT_RXB]) {
            mttcan_err("set sid filter 0x%08x failed, because the offset = %u is bigger than"
                " the rxb = %u.\n", filter_val, offset, priv->cfg.element_num[ELMT_RXB]);
            return -1;
        }
    } else {
        /* sid Range filtering configuration must sfid1 <= sfid2 */
        if (filter->sft == FILTER_RANG_FILTER) {
            if (filter->sfid1 > filter->sfid2) {
                mttcan_err("set sid filter failed, range filter should id1 <= id2, \
                    but now id1 = %u, id2 = %u\n", filter->sfid1, filter->sfid2);
                return -1;
            }
        }
    }

    return 0;
}

STATIC int mttcan_check_xid_filter_cfg(struct mttcan_priv *priv, struct xid_filter *filter)
{
    int ret;
    u32 offset;
    u32 filter_val_f0, filter_val_f1;

    get_xid_filter_val(filter, &filter_val_f0, &filter_val_f1);
    /* Check whether the space for storing data is available */
    ret = mttcan_check_filter_index(priv, filter->efec);
    if (ret != 0) {
        mttcan_err("set xid filter failed, filter0 = 0x%08x, filter1 = 0x%08x\n",
            filter_val_f0, filter_val_f1);
        return -1;
    }

    /* RXB must be greater than offset. */
    if (filter->efec == FEC_STORE_RXB_DEBUG) {
        offset = filter_val_f1 & EIDF_F1_EFID2_OFFSET;
        if (offset >= priv->cfg.element_num[ELMT_RXB]) {
            mttcan_err("set xid filter f0.0x%08x f1.0x%08x failed, because the offset = %u is bigger than"
                " the rxb = %u.\n", filter_val_f0, filter_val_f1, offset, priv->cfg.element_num[ELMT_RXB]);
            return -1;
        }
    } else {
        /* xid Range filtering configuration must efid1 <= efid2 */
        if (filter->eft == FILTER_RANG_FILTER) {
            if (filter->efid1 > filter->efid2) {
                mttcan_err("set xid filter failed, range filter should id1 <= id2, \
                    but now id1 = %u, id2 = %u\n", filter->efid1, filter->efid2);
                return -1;
            }
        }
    }
    return 0;
}

STATIC int mttcan_set_sid_filter(struct net_device *ndev, void *data, unsigned int input_size)
{
    u32 i;
    u32 ret;
    u32 data_word;
    u32 filter_size;
    void __iomem *element_addr = NULL;
    struct mttcan_priv *priv = netdev_priv(ndev);
    struct sid_filter *filter = (struct sid_filter *)data;

    if ((input_size == 0) || (input_size % SID_FILTER_SIZE != 0)) {
        mttcan_err("set sid filter failed, input_size = %u, "
            "is not an integer multiple of sid_filter's size %lu.\n", input_size, SID_FILTER_SIZE);
        return -EINVAL;
    }

    filter_size = input_size / SID_FILTER_SIZE;
    if (priv->elmt_param[ELMT_SIDF].num < filter_size) {
        mttcan_err("sid filter size err, configure filter size is %u,"
                   "but filter_size is %u.\n", priv->elmt_param[ELMT_SIDF].num, filter_size);
        return -EINVAL;
    }

    for (i = 0; i < filter_size; i++) {
        ret = mttcan_check_sid_filter_cfg(priv, &filter[i]);
        if (ret != 0) {
            return -EINVAL;
        }
    }

    /* set filter */
    element_addr = priv->addr.mram_base + priv->elmt_param[ELMT_SIDF].off;
    for (i = 0; i < priv->elmt_param[ELMT_SIDF].num; i++) {
        if (i < filter_size) {
            data_word = get_sid_filter_val(&filter[i]);
        } else {
            /* add 0 to the remaining configuration item. */
            data_word = 0;
        }
        if (write_reg_and_report_fault(ndev, element_addr, i * ELMT_SIZE_SIDF, data_word) != 0) {
            mttcan_err("write sid_filter failed.\n");
            return -ETIME;
        }
        priv->cfg.sid_filter[i] = data_word;
    }

    mttcan_info("config standard filter succ, sft = %u, sfec = %u, sfid1 = %u, ssync = %u, sfid2 = %u\n",
        filter->sft, filter->sfec, filter->sfid1, filter->ssync, filter->sfid2);
    return 0;
}

STATIC int mttcan_set_xid_filter(struct net_device *ndev, void *data, unsigned int input_size)
{
    u32 i;
    u32 xid_filter_f0;
    u32 xid_filter_f1;
    u32 filter_size;
    int ret;
    void __iomem *element_addr = NULL;
    struct mttcan_priv *priv = netdev_priv(ndev);
    struct xid_filter *filter = (struct xid_filter *)data;

    if ((input_size == 0) || (input_size % XID_FILTER_SIZE != 0)) {
        mttcan_err("set xid filter failed, input_size = %u, "
            "is not an integer multiple of xid_filter's size %lu.\n", input_size, XID_FILTER_SIZE);
        return -EINVAL;
    }

    filter_size = input_size / XID_FILTER_SIZE;
    if (priv->elmt_param[ELMT_XIDF].num < filter_size) {
        mttcan_err("xid filter size err, configure filter "
            "size is %u, but filter_size is %u.\n", priv->elmt_param[ELMT_XIDF].num, filter_size);
        return -EINVAL;
    }

    for (i = 0; i < filter_size; i++) {
        ret = mttcan_check_xid_filter_cfg(priv, &filter[i]);
        if (ret != 0) {
            return -EINVAL;
        }
    }

    /* set filter */
    element_addr = priv->addr.mram_base + priv->elmt_param[ELMT_XIDF].off;
    for (i = 0; i < priv->elmt_param[ELMT_XIDF].num; i++) {
        if (i < filter_size) {
            get_xid_filter_val(&filter[i], &xid_filter_f0, &xid_filter_f1);
        } else {
            /* add 0 to the remaining configuration item. */
            xid_filter_f0 = 0;
            xid_filter_f1 = 0;
        }
        if (write_reg_and_report_fault(ndev, element_addr, i * ELMT_SIZE_XIDF, xid_filter_f0) != 0) {
            mttcan_err("write xid_filter_f0 failed.\n");
            return -ETIME;
        }
        if (write_reg_and_report_fault(ndev, element_addr, i * ELMT_SIZE_XIDF + BYTES_OF_WORD, xid_filter_f1) != 0) {
            mttcan_err("write xid_filter_f1 failed.\n");
            return -ETIME;
        }
        priv->cfg.xid_filter[(u64)i << 1] = xid_filter_f0;
        priv->cfg.xid_filter[((u64)i << 1) + 1] = xid_filter_f1;
    }

    mttcan_info("config extend filter succ, efec = %u, efid1 = %u, eft = %u, esync = %u, efid2 = %u\n",
        filter->efec, filter->efid1, filter->eft, filter->esync, filter->efid2);
    return 0;
}

STATIC int mttcan_set_busoff_param(struct net_device *ndev, void *data, unsigned int data_size)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    struct busoff_config_param *bo_cfg = (struct busoff_config_param *)data;

    if (data_size != BUSOFF_CONFIG_SIZE) {
        mttcan_err("set busoff_config_param failed, data_size = %u, "
            "is not equal to busoff_config_param's size %lu.\n", data_size, BUSOFF_CONFIG_SIZE);
        return -EINVAL;
    }
    if (bo_cfg->busoff_quick >= bo_cfg->busoff_slow) {
        mttcan_err("set busoff_config_param failed, busoff_quick must less "
            "than busoff_slow, busoff_quick = %u, busoff_slow = %u.\n", bo_cfg->busoff_quick, bo_cfg->busoff_slow);
        return -EINVAL;
    }
    if (bo_cfg->busoff_quick <= 0 || bo_cfg->busoff_quick > BUSOFF_RESTART_TIME_MAX_MS) {
        mttcan_err("set busoff_config_param failed, "
            "invalid busoff_quick = %u.\n", bo_cfg->busoff_quick);
        return -EINVAL;
    }
    if (bo_cfg->busoff_slow <= 0 || bo_cfg->busoff_slow > BUSOFF_RESTART_TIME_MAX_MS) {
        mttcan_err("set busoff_config_param failed, "
            "invalid busoff_slow = %u.\n", bo_cfg->busoff_slow);
        return -EINVAL;
    }
    if (bo_cfg->busoff_quick_times <= 0 || bo_cfg->busoff_quick_times > BUSOFF_QUICK_TIMES_MAX) {
        mttcan_err("set busoff_config_param failed, "
            "invalid busoff_quick_times = %u.\n", bo_cfg->busoff_quick_times);
        return -EINVAL;
    }
    if (bo_cfg->busoff_report_threshold <= 0 || bo_cfg->busoff_report_threshold > BO_REPORT_THRESHLD_MAX) {
        mttcan_err("set busoff_config_param failed, "
            "invalid busoff_report_threshold = %u.\n", bo_cfg->busoff_report_threshold);
        return -EINVAL;
    }

    priv->busoff_cfg_param.busoff_quick = bo_cfg->busoff_quick;
    priv->busoff_cfg_param.busoff_slow = bo_cfg->busoff_slow;
    priv->busoff_cfg_param.busoff_quick_times = bo_cfg->busoff_quick_times;
    priv->busoff_cfg_param.busoff_report_threshold = bo_cfg->busoff_report_threshold;
    mttcan_info("config busoff param successfully.\n"
        "busoff_quick is %u.\nbusoff_slow is %u.\nbusoff_quick_times is %u\nbusoff_report_threshold is %u\n",
        priv->busoff_cfg_param.busoff_quick, priv->busoff_cfg_param.busoff_slow,
        priv->busoff_cfg_param.busoff_quick_times, priv->busoff_cfg_param.busoff_report_threshold);
    return 0;
}

STATIC int mttcan_get_sid_filter(struct net_device *ndev, void *data, unsigned int input_size, unsigned int *out_size)
{
    u32 i;
    u32 filter_cfg;
    u32 filter_num;
    u32 filter_addr;
    u32 sid_filter_val;
    void __iomem *element_addr = NULL;
    struct mttcan_priv *priv = netdev_priv(ndev);
    struct sid_filter *filter = (struct sid_filter *)data;

    if ((input_size == 0) || (input_size % SID_FILTER_SIZE != 0)) {
        mttcan_print_oplog(priv, CAN_LOG_FILTER_GET_FAIL,
            "get sid filter failed, input_size = %u, is not an integer multiple of sid_filter's size %lu.\n",
            input_size, SID_FILTER_SIZE);
        return -EINVAL;
    }

    /* read REG_SIDFC, get sid filter number and address configuration */
    filter_cfg = mttcan_read_reg(&priv->addr, REG_SIDFC);
    filter_num = (filter_cfg & SIDFC_LSS_MASK) >> SIDFC_LSS_SHIFT;
    if (filter_num != priv->elmt_param[ELMT_SIDF].num) {
        mttcan_print_oplog(priv, CAN_LOG_FILTER_GET_FAIL,
            "sid filter config error. filter num(%u) is not expect(%u).\n",
            filter_num, priv->elmt_param[ELMT_SIDF].num);
        return -EINVAL;
    }

    if (input_size < filter_num * SID_FILTER_SIZE) {
        mttcan_print_oplog(priv, CAN_LOG_FILTER_GET_FAIL,
            "input buffer has no enough memory, input_size = %u, filter_size = %lu.\n",
            input_size, filter_num * SID_FILTER_SIZE);
        return -EINVAL;
    }

    filter_addr = (filter_cfg & SIDFC_FLSSA_MASK) >> SIDFC_FLSSA_SHIFT;
    element_addr = priv->addr.mram_base + (long)filter_addr * BYTES_OF_WORD;
    for (i = 0; i < filter_num; i++) {
        sid_filter_val = readl(element_addr + (i * ELMT_SIZE_SIDF));
        /* base on u32 sid filter value to struct sid_filter */
        set_sid_filter_val(&filter[i], sid_filter_val);
    }

    *out_size = filter_num * SID_FILTER_SIZE;
    return 0;
}

STATIC int mttcan_get_xid_filter(struct net_device *ndev, void *data,
    unsigned int input_size, unsigned int* const out_size)
{
    u32 i;
    u32 filter_cfg;
    u32 filter_num;
    u32 filter_addr;
    u32 xid_value_f0, xid_value_f1;
    void __iomem *element_addr = NULL;
    struct mttcan_priv *priv = netdev_priv(ndev);
    struct xid_filter *filter = (struct xid_filter *)data;

    if ((input_size == 0) || (input_size % XID_FILTER_SIZE != 0)) {
        mttcan_print_oplog(priv, CAN_LOG_FILTER_GET_FAIL,
            "set xid filter failed, input_size = %u, is not an integer multiple of xid_filter's size %lu.\n",
            input_size, XID_FILTER_SIZE);
        return -EINVAL;
    }

    /* read REG_XIDFC, get sid filter number and address configuration */
    filter_cfg = mttcan_read_reg(&priv->addr, REG_XIDFC);
    filter_num = (filter_cfg & XIDFC_LES_MASK) >> XIDFC_LES_SHIFT;
    if (filter_num != priv->elmt_param[ELMT_XIDF].num) {
        mttcan_print_oplog(priv, CAN_LOG_FILTER_GET_FAIL,
            "xid filter config error. filter num(%u) is not expect(%u).\n",
            filter_num, priv->elmt_param[ELMT_XIDF].num);
        return -EINVAL;
    }

    if (input_size < filter_num * XID_FILTER_SIZE) {
        mttcan_print_oplog(priv, CAN_LOG_FILTER_GET_FAIL,
            "input buffer has no enough memory, input_size = %u, filter_size = %lu\n",
            input_size, filter_num * XID_FILTER_SIZE);
        return -EINVAL;
    }

    filter_addr = (filter_cfg & XIDFC_FLESA_MASK) >> XIDFC_FLESA_SHIFT;
    element_addr = priv->addr.mram_base + (long)filter_addr * BYTES_OF_WORD;
    for (i = 0; i < filter_num; i++) {
        xid_value_f0 = readl(element_addr + (i * ELMT_SIZE_XIDF));
        xid_value_f1 = readl(element_addr + (i * ELMT_SIZE_XIDF) + BYTES_OF_WORD);
        /* base on u32 xid filter value1 and value2 to get struct xid_filter */
        set_xid_filter_val(&filter[i], xid_value_f0, xid_value_f1);
    }

    *out_size = filter_num * XID_FILTER_SIZE;
    return 0;
}

STATIC int mttcan_get_can_ram(struct net_device *ndev, void *data,
    unsigned int input_size, unsigned int* const out_size)
{
    struct can_config_stru *cfg = (struct can_config_stru *)data;
    struct mttcan_priv *priv = netdev_priv(ndev);
    int ret;

    *out_size = RAM_CONFIG_STRU_SIZE;
    if (*out_size != input_size) {
        mttcan_print_oplog(priv, CAN_LOG_RAM_CFG_GET_FAIL,
            "input_size is not ram space, input_size = %u ram space size = %u\n",
            input_size, *out_size);
        return -EINVAL;
    }

    ret = get_can_config((unsigned char *)ndev->name, (unsigned int)strlen(ndev->name), cfg);
    if (ret < 0) {
        mttcan_err("get_can_config is failed.\n");
        return ret;
    }
    return 0;
}

STATIC int mttcan_get_busoff_param(struct net_device *ndev, void *data,
    unsigned int data_size, unsigned int* const out_size)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    struct busoff_config_param *bo_cfg = (struct busoff_config_param *)data;
    if (data_size != BUSOFF_CONFIG_SIZE) {
        mttcan_print_oplog(priv, CAN_LOG_BUSOFF_CFG_GET_FAIL,
            "data_size is not busoff_config_param's size, data_size = %u, busoff_config_param size = %lu\n",
            data_size, BUSOFF_CONFIG_SIZE);
        return -EINVAL;
    }
    bo_cfg->busoff_quick = priv->busoff_cfg_param.busoff_quick;
    bo_cfg->busoff_slow = priv->busoff_cfg_param.busoff_slow;
    bo_cfg->busoff_quick_times = priv->busoff_cfg_param.busoff_quick_times;
    bo_cfg->busoff_report_threshold = priv->busoff_cfg_param.busoff_report_threshold;
    *out_size = BUSOFF_CONFIG_SIZE;
    return 0;
}

STATIC int mttcan_set_tdc(struct net_device *ndev, void *data, unsigned int data_size)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    struct can_tdc_cfg_stru *tdc_cfg = (struct can_tdc_cfg_stru *)data;

    /* if canx is down ,do not set config */
    if (netif_running(ndev)) {
        mttcan_err("this net_device is running, can't to set ram\n");
        return -EBUSY;
    }

    if (data_size != TDC_CONFIG_SIZE) {
        mttcan_err("set tdc failed, data_size = %u, "
            "is not equal to can_tdc_cfg_stru's size %lu.\n", data_size, BUSOFF_CONFIG_SIZE);
        return -EINVAL;
    }

    if ((tdc_cfg->tdc_flag < TDC_FLAG_AUTOADAPTER) || (tdc_cfg->tdc_flag > TDC_FLAG_ENABLE)) {
        mttcan_err("set tdc failed, tdc flag is invalid "
            "tdc_cfg->tdc_flag = %d.\n", tdc_cfg->tdc_flag);
        return -EINVAL;
    }

    if ((tdc_cfg->tdco < TDCO_MIN_VALUE) || (tdc_cfg->tdco > TDCO_MAX_VALUE)) {
        mttcan_err("set tdc failed, tdco flag is invalid "
            "tdc_cfg->tdco = %d.\n", tdc_cfg->tdco);
        return -EINVAL;
    }

    if ((tdc_cfg->tdcf < TDCF_MIN_VALUE) || (tdc_cfg->tdcf > TDCF_MAX_VALUE)) {
        mttcan_err("set tdc failed, tdcf flag is invalid "
            "tdc_cfg->tdcf = %d.\n", tdc_cfg->tdcf);
        return -EINVAL;
    }

    priv->tdc_flag = tdc_cfg->tdc_flag;
    if (tdc_cfg->tdc_flag == TDC_FLAG_ENABLE) {
        priv->tdco = tdc_cfg->tdco;
        priv->tdcf = tdc_cfg->tdcf;
    } else {
        priv->tdco = 0;
        priv->tdcf = 0;
    }

    mttcan_info("set tdc param successfully.\n"
        "tdc_cfg->tdc_flag = %d, tdc_cfg->tdco = %d, tdc_cfg->tdcf = %d.\n",
        tdc_cfg->tdc_flag, tdc_cfg->tdco, tdc_cfg->tdcf);
    return 0;
}

STATIC int mttcan_get_tdc(struct net_device *ndev, void *data,
    unsigned int data_size, unsigned int* const out_size)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    struct can_tdc_cfg_stru *tdc_cfg = (struct can_tdc_cfg_stru *)data;

    if (data_size != TDC_CONFIG_SIZE) {
        mttcan_print_oplog(priv, CAN_LOG_TDC_CFG_GET_FAIL,
            "data_size is not can_tdc_cfg_stru's size, data_size = %u, can_tdc_cfg_stru size = %lu\n",
            data_size, TDC_CONFIG_SIZE);
        return -EINVAL;
    }

    tdc_cfg->tdc_flag = priv->tdc_flag;
    tdc_cfg->tdco = priv->tdco;
    tdc_cfg->tdcf = priv->tdcf;
    *out_size = TDC_CONFIG_SIZE;
    mttcan_print_oplog(priv, CAN_LOG_TDC_CFG_GET_SUCC,
        "get tdc cfg succ, tdc_flag = %u, tdco = %u, tdcf = %u\n",
        tdc_cfg->tdc_flag, tdc_cfg->tdco, tdc_cfg->tdcf);
    return 0;
}

int mttcan_set_can_config(unsigned int can_id, CAN_SUB_CMD sub_cmd, void *data, unsigned int input_size)
{
    struct net_device *ndev = get_net_devices(can_id);
    int ret;

    if (ndev == NULL) {
        mttcan_err("cant found can_id:%u, sub_cmd:%u\n", can_id, sub_cmd);
        return -ENODEV;
    }

    if (data == NULL) {
        mttcan_err("input data is NULL, sub_cmd:%u\n", sub_cmd);
        return -EINVAL;
    }

    switch (sub_cmd) {
        case CAN_SUB_CMD_SID_FILTER:
            ret = mttcan_set_sid_filter(ndev, data, input_size);
            break;
        case CAN_SUB_CMD_XID_FILTER:
            ret = mttcan_set_xid_filter(ndev, data, input_size);
            break;
        case CAN_SUB_CMD_RAM:
            ret = mttcan_set_can_ram(ndev, data, input_size);
            break;
        case CAN_SUB_CMD_BUSOFF:
            ret = mttcan_set_busoff_param(ndev, data, input_size);
            break;
        case CAN_SUB_CMD_TDC:
            ret = mttcan_set_tdc(ndev, data, input_size);
            break;
        default:
            mttcan_err("not support for sub_cmd:%u\n", sub_cmd);
            return -EINVAL;
    }
    return ret;
}
EXPORT_SYMBOL(mttcan_set_can_config);

int mttcan_get_can_config(unsigned int can_id, CAN_SUB_CMD sub_cmd, void *data,
    unsigned int input_size, unsigned int *out_size)
{
    struct net_device *ndev = get_net_devices(can_id);
    struct mttcan_priv *priv;
    int ret = -1;

    if (ndev == NULL) {
        mttcan_err("Can't find this can_id = %u, sub_cmd:%u\n",
            can_id, sub_cmd);
        return -ENODEV;
    }

    priv = netdev_priv(ndev);

    if (data == NULL) {
        mttcan_print_oplog(priv, CAN_LOG_CFG_GET_FAIL,
            "input data is NULL, sub_cmd:%u\n", sub_cmd);
        return -EINVAL;
    }

    if (out_size == NULL) {
        mttcan_print_oplog(priv, CAN_LOG_CFG_GET_FAIL,
            "out_size is NULL, sub_cmd:%u\n", sub_cmd);
        return -EINVAL;
    }

    switch (sub_cmd) {
        case CAN_SUB_CMD_SID_FILTER:
            ret = mttcan_get_sid_filter(ndev, data, input_size, out_size);
            break;
        case CAN_SUB_CMD_XID_FILTER:
            ret = mttcan_get_xid_filter(ndev, data, input_size, out_size);
            break;
        case CAN_SUB_CMD_RAM:
            ret = mttcan_get_can_ram(ndev, data, input_size, out_size);
            break;
        case CAN_SUB_CMD_BUSOFF:
            ret = mttcan_get_busoff_param(ndev, data, input_size, out_size);
            break;
        case CAN_SUB_CMD_TDC:
            ret = mttcan_get_tdc(ndev, data, input_size, out_size);
            break;
        default:
            mttcan_print_oplog(priv, CAN_LOG_CFG_GET_FAIL,
                "not support for sub_cmd:%u\n", sub_cmd);
            return -EINVAL;
    }
    return ret;
}
EXPORT_SYMBOL(mttcan_get_can_config);
