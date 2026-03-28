/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: Common data types of mipi
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-08
 */

#ifndef MIPI_RX_TYPE
#define MIPI_RX_TYPE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef NULL
    #define NULL    0L
#endif

#ifndef FALSE
    #define FALSE   0
#endif

#ifndef TRUE
    #define TRUE    1
#endif

#ifndef TD_SUCCESS
#define TD_SUCCESS  0
#endif

#ifndef  TD_FAILURE
#define TD_FAILURE  (-1)
#endif

#define mipi_rx_emerg_trace(fmt, ...) \
    HI_EMERG_TRACE(HI_ID_VI, "[mipi-rx]:%s [line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define mipi_rx_alert_trace(fmt, ...) \
    HI_ALERT_TRACE(HI_ID_VI, "[mipi-rx]:%s [line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define mipi_rx_crit_trace(fmt, ...) \
    HI_CRIT_TRACE(HI_ID_VI, "[mipi-rx]:%s [line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define mipi_rx_err_trace(fmt, ...) \
    HI_ERR_TRACE(HI_ID_VI, "[mipi-rx]:%s [line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define mipi_rx_warn_trace(fmt, ...) \
    HI_WARN_TRACE(HI_ID_VI, "[mipi-rx]:%s [line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define mipi_rx_notice_trace(fmt, ...) \
    HI_NOTICE_TRACE(HI_ID_VI, "[mipi-rx]:%s [line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define mipi_rx_info_trace(fmt, ...) \
    HI_INFO_TRACE(HI_ID_VI, "[mipi-rx]:%s [line]:%d :" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define mipi_rx_debug_trace(fmt, ...) \
    HI_DEBUG_TRACE(HI_ID_VI, "[mipi-rx]:%s [line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define hi_mipi_rx_unused(x) ((void)(x))
#define mipi_rx_unused(x) ((void)(x))
#define max2(x, y)       ((x) > (y) ? (x) : (y))
#define min2(x, y)       ((x) < (y) ? (x) : (y))
#define mipi_align_up(x, a)           ((((x) + ((a) - 1U)) / (a)) * (a))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MIPI_RX_TYPE */
