/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-1-13
*/
#ifndef __TSDRV_FAULT_PLATFORM_H__
#define __TSDRV_FAULT_PLATFORM_H__

#define RAS_L2BUF_SRAM_MULTI_BIT_ECC_UER 0x0106         // EventID=0x80CD8008
#define RAS_L2BUF_SRAM_SINGLE_BIT_ECC_CE 0x0206         // EventID=0x80CD8006
#define RAS_L2BUF_CFG_0  0x0301 /* UEO */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_1  0x0401 /* UEU */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_2  0x0501 /* UEU */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_3  0x0601 /* UEU */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_4  0x0701 /* UEU */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_5  0x0801 /* UEO */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_6  0x120D /* UER */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_7  0x0A01 /* UER */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_8  0x0B01 /* UEO */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_9  0x0C01 /* UEO */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_10 0x0D01 /* UEO */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_11 0x0E01 /* UEO */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_12 0x0F01 /* UEO */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_13 0x1001 /* UEO */               // EventID=0x80CD8003
#define RAS_L2BUF_CFG_14 0x1101 /* UEO */               // EventID=0x80CD8003

#define RAS_DISP_CFG_ERR      0x0F0E    /* UER */       // EventID=0x813B8004
#define RAS_DISP_INPUT_ERR_WD 0x1515    /* UEO */       // EventID=0x813B8002
#define RAS_DISP_INPUT_ERR_RSP_0 0x0712 /* UER */       // EventID=0x813B8002
#define RAS_DISP_INPUT_ERR_RSP_1 0x0812 /* UER */       // EventID=0x813B8002
#define RAS_DISP_INPUT_ERR_RSP_2 0x0912 /* UER */       // EventID=0x813B8002
#define RAS_DISP_INPUT_ERR_RSP_3 0x0A12 /* UER */       // EventID=0x813B8002
#define RAS_DISP_INPUT_ERR_RSP_4 0x0B12 /* UER */       // EventID=0x813B8002
#define RAS_DISP_INPUT_ERR_RSP_5 0x0C12 /* UER */       // EventID=0x813B8002
#define RAS_DISP_INPUT_ERR_RSP_6 0x0D12 /* UER */       // EventID=0x813B8002
#define RAS_DISP_INPUT_ERR_RSP_7 0x0E12 /* UER */       // EventID=0x813B8002
#define RAS_DISP_INPUT_ERR_RSP_8 0x0F12 /* UER */       // EventID=0x813B8002
#define RAS_DISP_DATRAM_MULTI_BIT_ECC  0x0102 /* UEU */ // EventID=0x813B8008
#define RAS_DISP_CMDRAM_MULTI_BIT_ECC  0x0202 /* UEU */ // EventID=0x813B8008
#define RAS_DISP_NCDIR_MULTI_BIT_ECC   0x0302 /* UEU */ // EventID=0x813B8008
#define RAS_DISP_DATRAM_SINGLE_BIT_ECC 0x0401 /* CE */  // EventID=0x813B8006
#define RAS_DISP_CMDRAM_SINGLE_BIT_ECC 0x0501 /* CE */  // EventID=0x813B8006
#define RAS_DISP_NCDIR_SINGLE_BIT_ECC  0x0601 /* CE */  // EventID=0x813B8006


#define AIC_DISP_NUM 1U
#define L2BUFF_NUM   2U
#define TSCPU_NUM    1U
#endif
