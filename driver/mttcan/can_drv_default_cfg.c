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

#include "can_drv_default_cfg.h"

/* Default Configuration Element */
#define DEFAULT_ELEMENT_NUM_SIDF 1
#define DEFAULT_ELEMENT_NUM_XIDF 1
#define DEFAULT_ELEMENT_NUM_RXF0 38
#define DEFAULT_ELEMENT_NUM_RXF1 0
#define DEFAULT_ELEMENT_NUM_RXB 0
#define DEFAULT_ELEMENT_NUM_TXEF 24
#define DEFAULT_ELEMENT_NUM_TXB 10
#define DEFAULT_ELEMENT_NUM_TMC 8

/* Default Configuration Txbuff */
#define DEFAULT_TX_ELMT_NUM_DEDICATED_BUF 0
#define DEFAULT_TX_ELMT_NUM_FIFO_QUEUE 10

/* Default Configuration Watermark */
#define DEFAULT_WATERMARK_RXF0 3
#define DEFAULT_WATERMARK_RXF1 0
#define DEFAULT_WATERMARK_TXEF 3

/* Default Configuration Dsize */
#define DEFAULT_DSIZE_RXF0 64
#define DEFAULT_DSIZE_RXF1 64
#define DEFAULT_DSIZE_RXB 64
#define DEFAULT_DSIZE_TXB 64

/* Default Configuration FIFO mode */
#define DEFAULT_MODE_RXF0 0
#define DEFAULT_MODE_RXF1 0
#define DEFAULT_MODE_TXFQ 0

/* Default Configuration Filter */
#define DEFAULT_SID_FILTER_ID 0
#define DEFAULT_XID_FILTER_ID_F0 0
#define DEFAULT_XID_FILTER_ID_F1 1

/* sid filter to fif0 and id from 0x0 to 0x7ff */
#define DEFAULT_SID_FILTER 0x080087FF
/* xid filter to fif0 and id from 0x0000000 to 0x1ffffff */
#define DEFAULT_XID_FILTER_F0 0x20000000
#define DEFAULT_XID_FILTER_F1 0x3FFFFFFF

#define DEFAULT_GLOBAL_FILTER 0x0000003F

/* Default Configuration others */
#define DEFAULT_XID_AND_MASK 0x1FFFFFFF
#define DEFAULT_ECHO_SKB_MAX 32
#define DEFAULT_POLL_WEIGHT 8
#define DEFAULT_TS_CNT_PRESCALER 1

void mttcan_set_default_cfg(struct mttcan_config *cfg)
{
    cfg->element_num[ELMT_SIDF] = DEFAULT_ELEMENT_NUM_SIDF;
    cfg->element_num[ELMT_XIDF] = DEFAULT_ELEMENT_NUM_XIDF;
    cfg->element_num[ELMT_RXF0] = DEFAULT_ELEMENT_NUM_RXF0;
    cfg->element_num[ELMT_RXF1] = DEFAULT_ELEMENT_NUM_RXF1;
    cfg->element_num[ELMT_RXB] = DEFAULT_ELEMENT_NUM_RXB;
    cfg->element_num[ELMT_TXEF] = DEFAULT_ELEMENT_NUM_TXEF;
    cfg->element_num[ELMT_TXB] = DEFAULT_ELEMENT_NUM_TXB;
    cfg->element_num[ELMT_TMC] = DEFAULT_ELEMENT_NUM_TMC;
    cfg->tx_element_num[TX_ELMT_BUF] = DEFAULT_TX_ELMT_NUM_DEDICATED_BUF;
    cfg->tx_element_num[TX_ELMT_FQ] = DEFAULT_TX_ELMT_NUM_FIFO_QUEUE;
    cfg->watermark[WATERMARK_RXF0] = DEFAULT_WATERMARK_RXF0;
    cfg->watermark[WATERMARK_RXF1] = DEFAULT_WATERMARK_RXF1;
    cfg->watermark[WATERMARK_TXEF] = DEFAULT_WATERMARK_TXEF;
    cfg->dsize[DSIZE_RXF0] = DEFAULT_DSIZE_RXF0;
    cfg->dsize[DSIZE_RXF1] = DEFAULT_DSIZE_RXF1;
    cfg->dsize[DSIZE_RXB] = DEFAULT_DSIZE_RXB;
    cfg->dsize[DSIZE_TXB] = DEFAULT_DSIZE_TXB;
    cfg->mode[MODE_RXF0] = DEFAULT_MODE_RXF0;
    cfg->mode[MODE_RXF1] = DEFAULT_MODE_RXF1;
    cfg->mode[MODE_TXFQ] = DEFAULT_MODE_TXFQ;
    /* set filter */
    cfg->sid_filter[DEFAULT_SID_FILTER_ID] = DEFAULT_SID_FILTER;
    cfg->xid_filter[DEFAULT_XID_FILTER_ID_F0] = DEFAULT_XID_FILTER_F0;
    cfg->xid_filter[DEFAULT_XID_FILTER_ID_F1] = DEFAULT_XID_FILTER_F1;
    cfg->global_filter_cfg = DEFAULT_GLOBAL_FILTER;
    /* set others */
    cfg->extend_id_and_mask = DEFAULT_XID_AND_MASK;
    cfg->echo_skb_max = DEFAULT_ECHO_SKB_MAX;
    cfg->poll_weight = DEFAULT_POLL_WEIGHT;
    cfg->ts_cnt_prescaler = DEFAULT_TS_CNT_PRESCALER;
    cfg->tasklet_schedule_type = TASKLET_HI_SCHEDULE;
}

