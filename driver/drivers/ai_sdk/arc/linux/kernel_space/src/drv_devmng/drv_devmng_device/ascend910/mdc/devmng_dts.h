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

#ifndef __DEVDRV_GPIO_H
#define __DEVDRV_GPIO_H

/* MDC 610 防水板    BoardID 59
GPIO98/LCD_TE0/PROBE_D2      GPIO98     GPIO98    对接显示芯片，作为GPIO透传
GPIO274/PMU_DVPP_PG          GPIO274    GPIO274   对接显示芯片，作为GPIO透传
PWM4/GPIO188	             PWM4       GPIO188   对接显示芯片，作为GPIO透传
PWM5/GPIO189	             PWM5       GPIO189   对接显示芯片，作为GPIO透传
*/
/* MDC Pro 610 C1板  BoardID 2
GPIO192/UART5_RX/PROBE_F2    GPIO192    GPIO      不用串口，连接至MCU的供电电源35584的SS1管脚
GPIO199/UART7_CTS/PROBE_F13  GPIO199    GPIO      连接到CPLD，接收PPS的输入
GPIO98/LCD_TE0/PROBE_D2      GPIO98     GPIO      连接到CXD4953的GPIO2，串阻NC
GPIO274/PMU_DVPP_PG          GPIO274    GPIO      连接到CXD4953的GPIO3，串阻NC
GPIO45/SPI5_CLK/PROBE_A52    GPIO45     GPIO      作为INT使用
GPIO46/SPI5_CSN0/PROBE_A53   GPIO46     GPIO      作为INT使用
GPIO47/SPI5_MOSI/PROBE_A54   GPIO47     GPIO      作为INT使用
GPIO48/SPI5_MISO/PROBE_A55   GPIO48     GPIO      作为INT使用
PWM4/GPIO188	             PWM4       GPIO      连接到CXD4953的GPIO0，串阻NC
PWM5/GPIO189	             PWM5       GPIO      连接到CXD4953的GPIO1，串阻NC
*/
#define BOARD_ID_000 (0)    /* hisi,boardid = <0 0 0 0> */
#define BOARD_ID_001 (1)    /* hisi,boardid = <0 0 0 1> */
#define BOARD_ID_002 (2)    /* hisi,boardid = <0 0 0 2> */
#define BOARD_ID_059 (59)   /* hisi,boardid = <0 0 5 9> */
#define BOARD_ID_100 (100)  /* hisi,boardid = <0 1 0 0> */
#define BOARD_ID_900 (900)  /* hisi,boardid = <0 9 0 0> */
#define BOARD_ID_905 (905)  /* hisi,boardid = <0 9 0 5> */
#define BOARD_ID_909 (909)  /* hisi,boardid = <0 9 0 9> */
#define BOARD_ID_915 (915)  /* hisi,boardid = <0 9 1 5> */
#define BOARD_ID_919 (919)  /* hisi,boardid = <0 9 1 9> */
#define BOARD_ID_920 (920)  /* hisi,boardid = <0 9 2 0> */
#define BOARD_ID_925 (925)  /* hisi,boardid = <0 9 2 5> */
#define BOARD_ID_929 (929)  /* hisi,boardid = <0 9 2 9> */
#define BOARD_ID_940 (940)  /* hisi,boardid = <0 9 4 0> */
#define BOARD_ID_949 (949)  /* hisi,boardid = <0 9 4 9> */
#define BOARD_ID_999 (999)  /* hisi,boardid = <0 9 9 9> */

#define BOARDID_UNINITED (-1)
#define BOARDID_INIT_FAIL (-2)

#define BOARDID_LENGTH 4
#define DEVDRV_MDC_GPIO_SLOT_ID 338
#define DEVDRV_GPIO_NAME "gpio-read"

#define DEVDRV_PLAT_TYPE_REG_OFFSET 0XFFFC
#define CHIP_TYPE_REG_VALUE_MASK 0xF

#define CMD_LINE_BUFFER_SIZE              1024
#define CMDLINE_FILE_PATH                 "/proc/cmdline"
#define CMD_TYPE_STRING                   "ChipType="
#define CHIP_TYPE_DECIMAL                 10

typedef enum val_is_actstd_mode {
    VAL_IS_ACTSTD_MODE_NO = 0,
    VAL_IS_ACTSTD_MODE_YES
} VAL_IS_ACTSTD_MODE;


typedef enum val_actstd_state {
    VAL_ACTSTD_STATE_ACT = 0,
    VAL_ACTSTD_STATE_STD
} VAL_ACTSTD_STATE;

typedef enum center_key_list {
    KEY_IS_ACTSTD = 0,
    KEY_IS_LOADBALANC = 1,
    KEY_TOTAL_CHIPNUM = 2,
    KEY_PHY_LOG_MAP = 3,
    KEY_DOMAIN_MODE = 4,
    // Default is KEY_CUR_ACTSTD when KEY_IS_ACTSTD = VAL_IS_ACTSTD_MODE_NO 
    // Or default is KEY_CUR_ACTSTD when KEY_IS_ACTSTD = VAL_IS_ACTSTD_MODE_YES
    KEY_CUR_ACTSTD = 5,
    KEY_CUR_PHYPOS = 6,
    KEY_BOARD_ID = 7,
    KEY_CHIP_TYPE = 8,
    KEY_MAX
} CENTER_KEY_LIST;

void devdrv_soc_workmode_init(void);
void devdrv_gpio_iomux_init(struct device *dev);
#endif /*  __DEVDRV_GPIO_H  */
