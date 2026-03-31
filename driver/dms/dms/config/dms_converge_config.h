/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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

#ifndef __DMS_CONVERGE_CONFIG_H__
#define __DMS_CONVERGE_CONFIG_H__

/* true:on, false:off */
#ifdef CFG_SOC_PLATFORM_MDC_V51
#  define DMS_EVENT_CONVERGE_SWITCH false
#else
#  define DMS_EVENT_CONVERGE_SWITCH false
#endif

EVENT_CONVERGE_DECLARATION_BEGIN()
/* convergent diagrams of SOC ERROR declaration */
EVENT_CONVERGE_ITEM_DECLARATION(0x80818000, 0x80B78000,  /* SLLC    ERROR */
                                            0x80B78008,  /* SLLC    MBECC */
                                            0x80B38000,  /* CS      ERROR */
                                            0x80DF8000,  /* DDR     ERROR */
                                            0x80F38000,  /* DHA     ERROR */
                                            0x80F38008,  /* DHA     MBECC */
                                            0x80F38001,  /* DHA     ERROR_NF */
                                            0x80C78008,  /* TS      MBECC */
                                            0x80AD8008,  /* GIC     MBECC */
                                            0x80E18005,  /* HBM     PARITY */
                                            0x80E18000,  /* HBM     ERROR */
                                            0x80E1800A,  /* HBM     TIMEOUT_ERR */
                                            0x80F18000,  /* HHA     ERROR */
                                            0x80F18008,  /* HHA     MBECC */
                                            0x80BD8000,  /* NIC     ERROR */
                                            0x80BD8008,  /* NIC     MBECC */
                                            0x80F78000,  /* HWTS/Stars-TS ERROR */
                                            0x80E38000,  /* LPM     ERROR */
                                            0x80CD8000,  /* L2BUF   ERROR */
                                            0x80CD8008,  /* L2BUF   MBECC */
                                            0x80A38000,  /* L3D     ERROR */
                                            0x80A38008,  /* L3D     MBECC */
                                            0x80A58000,  /* L3T     ERROR */
                                            0x80A58008,  /* L3T     MBECC */
                                            0x80A78000,  /* MESH    ERROR */
                                            0x80B18000,  /* MN      ERROR */
                                            0x80B58000,  /* SIOE    ERROR */
                                            0x80A18008,  /* CPUcore MBECC */
                                            0x80B98000,  /* pcie local ERROR */
                                            0x80B98008,  /* pcie local MBECC */
                                            0x80FB8000,  /* TSCPU(A55/R52) ERROR */
                                            0x80FB8005); /* TSCPU(A55/R52) PARITY */
/* convergent diagrams of SOC ERROR_NF declaration */
EVENT_CONVERGE_ITEM_DECLARATION(0x80818001, 0x80BB8001); /* ROCE    ERROR_NF */
/* convergent diagrams of SOC IN_CFG_ERR declaration */
EVENT_CONVERGE_ITEM_DECLARATION(0x80818003, 0x80B78003,  /* SLLC    IN_CFG_ERR */
                                            0x80F38003,  /* DHA     IN_CFG_ERR */
                                            0x80D18003,  /* DVPP    IN_CFG_ERR */
                                            0x80F18003,  /* HHA     IN_CFG_ERR */
                                            0x80BD8003,  /* NIC     IN_CFG_ERR */
                                            0x80CD8003,  /* L2BUF   IN_CFG_ERR */
                                            0x80A38003,  /* L3D     IN_CFG_ERR */
                                            0x80A58003,  /* L3T     IN_CFG_ERR */
                                            0x80B18003,  /* MN      IN_CFG_ERR */
                                            0x80BB8003); /* ROCE    IN_CFG_ERR */
EVENT_CONVERGE_ITEM_INIT(0x80818000);
EVENT_CONVERGE_ITEM_INIT(0x80818001);
EVENT_CONVERGE_ITEM_INIT(0x80818003);
EVENT_CONVERGE_DECLARATION_END();
#endif
