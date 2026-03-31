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

#ifndef _CAN_DRV_API_H
#define _CAN_DRV_API_H

#include <linux/types.h>

/* error code */
#define RET_OK                          0
#define RET_ERR_IOCRL_FAILED            (-3)

#define SID_FILTER_MAX_NUM  128
#define XID_FILTER_MAX_NUM  64

#define BUSOFF_RESTART_TIME_MAX_MS  (1000 * 60)
#define BUSOFF_QUICK_TIMES_MAX      100
#define BO_REPORT_THRESHLD_MAX      100

/* Configuration sid Options */
#define FILTER_RANG_FILTER 0

/* global filter configuration recv options */
#define GLOBAL_FILTER_RECV_RXF0 0
#define GLOBAL_FILTER_RECV_RXF1 1

typedef enum {
    CAN_BUS_STATE_ACTIVER,
    CAN_BUS_STATE_ERR_WARNING,
    CAN_BUS_STATE_ERR_PASSIVE,
    CAN_BUS_STATE_ERR_BUSOFF,
    CAN_BUS_STATE_DOWN,
} CAN_BUS_STATE;

typedef enum {
    CAN_RX_FIFO_BLOCKING_MODE,
    CAN_RX_FIFO_OVERWRITE_MODE,
} CAN_RX_FIFO_MODE;

typedef enum {
    CAN_TX_FIFO_OPERATION,
    CAN_TX_QUEUE_OPERATION,
} CAN_TX_FIFO_QUEUE_MODE;

typedef enum {
    CAN_SUB_CMD_SID_FILTER = 0,
    CAN_SUB_CMD_XID_FILTER,
    CAN_SUB_CMD_RAM,
    CAN_SUB_CMD_BUSOFF,
    CAN_SUB_CMD_TDC,
    CAN_SUB_CMD_MAX,
} CAN_SUB_CMD;

struct sid_filter {
    /**
     * Standard Filter Type:
     * 0 - Range filter from SFID1 to SFID2
     * 1 - Dual ID filter for SFID1 or SFID2
     * 2 - Classic filter: SFID1 = filter, SFID2 = mask
     * 3 - Filter element disabled
     */
    unsigned int sft : 2;
    /**
     * Standard Filter Element Configuration:
     * 0 - Disable filter element
     * 1 - Store in Rx FIFO 0 if filter matches
     * 2 - Store in Rx FIFO 1 if filter matches
     * 3 - Reject ID if filter matches, not intended to be used with Sync messages
     * 4 - Set priority if filter matches, not intended to be used with Sync messages, no storage
     * 5 - Set priority and store in FIFO 0 if filter matches
     * 6 - Set priority and store in FIFO 1 if filter matches
     * 7 - Store into Rx Buffer or as debug message, configuration of SFT[1:0] ignored
     */
    unsigned int sfec : 3;
    /* Standard Filter ID 1 */
    unsigned int sfid1 : 11;
    /**
     * Standard Sync Message
     * 0 - Timestamping for the matching Sync message disabled
     * 1 - Timestamping for the matching Sync message enabled
     */
    unsigned int ssync : 1;
    /* reserved */
    unsigned int res : 4;
    /* Standard Filter ID 2 */
    unsigned int sfid2 : 11;
};

struct xid_filter {
    /**
     * Extended Filter Element Configuration
     * 0 - Disable filter element
     * 1 - Store in Rx FIFO 0 if filter matches
     * 2 - Store in Rx FIFO 1 if filter matches
     * 3 - Reject ID if filter matches, not intended to be used with Sync messages
     * 4 - Set priority if filter matches, not intended to be used with Sync messages, no storage
     * 5 - Set priority and store in FIFO 0 if filter matches
     * 6 - Set priority and store in FIFO 1 if filter matches
     * 7 - Store into Rx Buffer or as debug message, configuration of EFT[1:0] ignored
     */
    unsigned int efec : 3;
    /* Extended Filter ID 1 */
    unsigned int efid1 : 29;
    /**
     * Extended Filter Type
     * 0 - Range filter from SFID1 to SFID2
     * 1 - Dual ID filter for SFID1 or SFID2
     * 2 - Classic filter: SFID1 = filter, SFID2 = mask
     * 3 - Range filter from EFID1 to EFID2 (EFID2 ≥ EFID1), XIDAM mask not applied
     */
    unsigned int eft : 2;
    /**
     * Extended Sync Message
     * 0 - Timestamping for the matching Sync message disabled
     * 1 - Timestamping for the matching Sync message enabled
     */
    unsigned int esync : 1;
    /* Extended Filter ID 2 */
    unsigned int efid2 : 29;
};

struct global_filter {
    /* reserved */
    unsigned int res : 26;
    /**
     * Accept Non-matching Frames Standard
     * 0 - Accept in Rx FIFO 0
     * 1 - Accept in Rx FIFO 1
     * 2 - Reject
     * 3 - Reject
     */
    unsigned int anfs : 2;
    /**
     * Accept Non-matching Frames Standard
     * 0 - Accept in Rx FIFO 0
     * 1 - Accept in Rx FIFO 1
     * 2 - Reject
     * 3 - Reject
     */
    unsigned int anfe : 2;
    /**
     * Reject Remote Frames Standard
     * 0 - Filter remote frames with 11-bit standard IDs
     * 1 - Reject all remote frames with 11-bit standard IDs
     */
    unsigned int rrfs : 1;
    /**
     * Reject Remote Frames Extended
     * 0 - Filter remote frames with 29-bit extended IDs
     * 1 - Reject all remote frames with 29-bit extended IDs
     */
    unsigned int rrfe : 1;
};

struct can_config_stru {
    unsigned int element_num_rxf0; /* Rx FIFO 0 quantity */
    unsigned int element_num_rxf1; /* Rx FIFO 1 quantity */
    unsigned int element_num_rxb;  /* Rx Buffer quantity */
    unsigned int element_num_txef; /* Tx Event FIFO quantity */
    unsigned int element_num_txb;  /* Tx Buffer quantity */
    unsigned int element_num_tmc;  /* Trigger Memory quantity */
    unsigned int tx_elmt_num_dedicated_buf; /* Tx dedicated buf quantity */
    unsigned int tx_elmt_num_fifo_queue;    /* Tx FIFO/Queue quantity */
    unsigned int dsize_fifo0; /* Rx FIFO 0 data size */
    unsigned int dsize_fifo1; /* Rx FIFO 1 data size */
    unsigned int dsize_rxb;   /* Rx Buffer data size */
    unsigned int dsize_txb;   // Tx Buffer data size
    unsigned int watermark_rxf0;    /* Rx FIFO 0 watermark */
    unsigned int watermark_rxf1;    /* Rx FIFO 1 watermark */
    unsigned int watermark_txef;    /* Tx Event FIFO watermark */
    CAN_RX_FIFO_MODE mode_rxf0;   /* Rx FIFO 0 work mode */
    CAN_RX_FIFO_MODE mode_rxf1;   /* Rx FIFO 0 work mode */
    CAN_TX_FIFO_QUEUE_MODE mode_txfq;   /* Tx Event FIFO work mode */
    unsigned int element_num_sidf;  /* Standard ID Filter quantity */
    unsigned int element_num_xidf;  /* Extended ID Filter quantity */
    struct global_filter global_filter;                /* Global Filter */
    unsigned int xid_and_mask;      /* Extended ID AND Mask */
    unsigned int echo_skb_max;      /* Local socket buffer quantity */
    unsigned int poll_weight;       /* napi poll weight */
    unsigned int ts_cnt_prescaler;  /* timestamp counter prescaler */
};

struct can_status_stru {
    CAN_BUS_STATE bus_state;
    unsigned int rx_err_counter;
    unsigned int tx_err_counter;
    unsigned int err_passive;
};

struct can_tdc_cfg_stru {
    unsigned char tdc_flag; /**< TDC mode: 0-Auto adaptation config, 1-Disable TDC, 2-Enable TDC */
    unsigned char tdco; /**< Transmitter Delay Compensation SSP Offset, Configured only when tdc_mode=2 */
    unsigned char tdcf; /**< Transmitter Delay Compensation Filter Window Length, Configured only when tdc_mode=2 */
};


/**
 * brief：get can config
 * param：[IN] name -can name
 * param：[IN] namelen -can name length
 * param：[OUT] can_cfg - can config
 * return：0 - succ, others -error code
 */
int get_can_config(unsigned char *name, unsigned int namelen, struct can_config_stru *can_cfg);

/**
 * brief：get can status
 * param：[IN] name -can name
 * param：[IN] namelen -can name length
 * param：[OUT] can_cfg - can status
 * return：0 - succ, others -error code
 */
int get_can_status(const unsigned char *name, unsigned int namelen, struct can_status_stru *can_stat);

#define SID_FILTER_SIZE (sizeof(struct sid_filter))
#define SID_FILTER_MAX_NUM 128
#define SID_FILTER_MAX_SIZE (SID_FILTER_SIZE * SID_FILTER_MAX_NUM)
#define XID_FILTER_SIZE (sizeof(struct xid_filter))
#define XID_FILTER_MAX_NUM 64
#define XID_FILTER_MAX_SIZE (XID_FILTER_SIZE * XID_FILTER_MAX_NUM)

int mttcan_set_can_config(unsigned int can_id, CAN_SUB_CMD sub_cmd, void *data, unsigned int input_size);
int mttcan_get_can_config(unsigned int can_id, CAN_SUB_CMD sub_cmd, void *data,
    unsigned int input_size, unsigned int *out_size);

#endif

