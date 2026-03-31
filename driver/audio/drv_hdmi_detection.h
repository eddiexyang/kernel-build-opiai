/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef DRV_HDMI_DETECTION_H
#define DRV_HDMI_DETECTION_H

#include "ot_type.h"
#include "drv_hdmi_ioctl.h"

#define hdmi_dfx_info(fmt, args...) \
    osal_printk("HDMI:INFO:[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##args)
#define hdmi_dfx_warn(fmt, args...) \
    osal_printk("HDMI:WARN:[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##args)
#define hdmi_dfx_err(fmt, args...) \
    osal_printk("HDMI:ERROR:[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##args)

#define HDMI_TX_REG_BASE_ADDRESS 0x4001A0000
#define HDMI_TX_1_REG_BASE_ADDRESS 0x4001C0000
#define HDMI_GEN_SUB_PKT1_H_OFFSET 0x8B8
#define HDMI_FDET_HORT_RES_OFFSET 0x8C0
#define HDMI_FDET_VERT_RES_OFFSET 0x8C4


/* define the union hdmi_fdet_hort_res */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int hsync_active_cnt : 13; /* [12..0]  */
        unsigned int hsync_total_cnt : 13;  /* [25..13]  */
        unsigned int reserved_1 : 6;        /* [31..26]  */
    } bits;

    /* define an unsigned member */
    unsigned int u32;
} hdmi_fdet_hort_res;


/* define the union hdmi_fdet_vert_res */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int vsync_active_cnt : 13; /* [12..0]  */
        unsigned int vsync_total_cnt : 13;  /* [25..13]  */
        unsigned int reserved_1 : 6;        /* [31..26]  */
    } bits;

    /* define an unsigned member */
    unsigned int u32;
} hdmi_fdet_vert_res;

/* define the union hdmi_gen_sub_pkt1_h */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int gen_sub_pkt1_pb4 : 8; /* [7..0]  */
        unsigned int gen_sub_pkt1_pb5 : 8; /* [15..8]  */
        unsigned int gen_sub_pkt1_pb6 : 8; /* [23..16]  */
        unsigned int reserved_1 : 8;       /* [31..24]  */
    } bits;

    /* define an unsigned member */
    unsigned int u32;
} hdmi_gen_sub_pkt1_h;

td_s32 hdmi_set_detection(drv_hdmi_detection *hdmi_detection);

#endif /* DRV_HDMI_DETECTION_H */