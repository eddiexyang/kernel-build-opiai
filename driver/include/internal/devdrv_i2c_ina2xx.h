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


#ifndef __I2C_INA2XX_H__
#define __I2C_INA2XX_H__
#include <linux/types.h>
#include <linux/mutex.h>

/* common register definitions */
#define INA2XX_CONFIG 0x00
#define INA2XX_SHUNT_VOLTAGE 0x01 /* readonly */
#define INA2XX_BUS_VOLTAGE 0x02   /* readonly */
#define INA2XX_POWER 0x03         /* readonly */
#define INA2XX_CURRENT 0x04       /* readonly */
#define INA2XX_CALIBRATION 0x05

/* INA226 register definitions */
#define INA226_MASK_ENABLE 0x06
#define INA226_ALERT_LIMIT 0x07
#define INA226_DIE_ID 0xFF

/* register count */
#define INA219_REGISTERS 6
#define INA226_REGISTERS 8

#define INA2XX_MAX_REGISTERS 8

/* settings - depend on use case */
#define INA219_CONFIG_DEFAULT ((u16)0x399F) /* PGA:8 */
#define INA226_CONFIG_DEFAULT ((u16)0x4007) /* averages:1 */

/* worst case is 68.10 ms (~14.6Hz, ina219) */
#define INA2XX_CONVERSION_RATE 15
#define INA2XX_MAX_DELAY 69 /* worst case delay in ms */

#define INA2XX_RSHUNT_DEFAULT 13333

/* bit mask for reading the averaging setting in the configuration register */
#define INA226_AVG_RD_MASK 0x0E00

#define INA226_READ_AVG(reg) (((reg)&INA226_AVG_RD_MASK) >> 9)
#define INA226_SHIFT_AVG(val) ((val) << 9)

/* common attrs, ina226 attrs and NULL */
#define INA2XX_MAX_ATTRIBUTE_GROUPS 3

/*
 * Both bus voltage and shunt voltage conversion times for ina226 are set
 * to 0b0100 on POR, which translates to 2200 microseconds in total.
 */
#define INA226_TOTAL_CONV_TIME_DEFAULT 2200

#define INA226_MASK_ENABLE_DEFAULT ((u16)0x8000)
#define INA226_ALERT_LIMIT_DEFAULT ((u16)0x3415)

enum ina2xx_ids {
    INA226
};

struct ina2xx_config {
    u16 config_default;
    int calibration_factor;
    int registers;
    int shunt_div;
    int bus_voltage_shift;
    int bus_voltage_lsb; /* uV */
    int power_lsb;       /* uW */
};

struct ina2xx_data {
    struct i2c_client *client;
    const struct ina2xx_config *config;

    long rshunt;
    u16 curr_config;

    struct mutex update_lock;
    bool valid;
    unsigned long last_updated;
    int update_interval; /* in jiffies */

    u16 regs[INA2XX_MAX_REGISTERS];
};

int ina2xx_get_value(u8 reg, int *result);
void ina2xx_register_drv(void);
void ina2xx_unregister_drv(void);

#endif
