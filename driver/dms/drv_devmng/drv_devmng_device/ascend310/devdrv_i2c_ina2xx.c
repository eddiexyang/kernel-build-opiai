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


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/jiffies.h>
#include <linux/of.h>
#include <linux/delay.h>


#include "devdrv_i2c_ina2xx.h"
#include "devdrv_common.h"

STATIC const struct ina2xx_config ina2xx_config[] = {
    [INA226] = {
        .config_default     = INA226_CONFIG_DEFAULT,
        .calibration_factor = 5120000,
        .registers          = INA226_REGISTERS,
        .shunt_div          = 400,
        .bus_voltage_shift  = 0,
        .bus_voltage_lsb    = 1250,
        .power_lsb          = 25000,
    },
};

STATIC struct ina2xx_data *g_i2c_data = NULL;
/*
 * Available averaging rates for ina226. The indices correspond with
 * the bit values expected by the chip (according to the ina226 datasheet,
 * table 3 AVG bit settings
 */
STATIC const int ina226_avg_tab[] = { 1, 4, 16, 64, 128, 256, 512, 1024 };

STATIC int ina226_reg_to_interval(u16 config)
{
    int avg = ina226_avg_tab[INA226_READ_AVG(config)];

    /*
     * Multiply the total conversion time by the number of averages.
     * Return the result in milliseconds.
     */
    return DIV_ROUND_CLOSEST((avg * INA226_TOTAL_CONV_TIME_DEFAULT), 1000);
}

STATIC void ina226_set_update_interval(struct ina2xx_data *data)
{
    int ms;

    ms = ina226_reg_to_interval(data->curr_config);
    data->update_interval = msecs_to_jiffies(ms);
}

/*
 * Initialize the configuration and calibration registers.
 */
STATIC int ina2xx_init(struct ina2xx_data *data)
{
    struct i2c_client *client = data->client;
    int ret;
    u16 val = DIV_ROUND_CLOSEST(data->config->calibration_factor, data->rshunt);

    /* device configuration */
    ret = i2c_smbus_write_word_swapped(client, INA2XX_CONFIG, data->curr_config);
    if (ret < 0) {
        devdrv_drv_err("init INA2XX_CONFIG fail %d\n", ret);
        return ret;
    }

    ret = i2c_smbus_write_word_swapped(client, INA226_MASK_ENABLE, INA226_MASK_ENABLE_DEFAULT);
    if (ret < 0) {
        devdrv_drv_err("init INA226_MASK_ENABLE fail %d\n", ret);
        return ret;
    }

    ret = i2c_smbus_write_word_swapped(client, INA226_ALERT_LIMIT, INA226_ALERT_LIMIT_DEFAULT);
    if (ret < 0) {
        devdrv_drv_err("init INA226_ALERT_LIMIT fail %d\n", ret);
        return ret;
    }
    /*
     * Set current LSB to 1mA, shunt is in uOhms
     * (equation 13 in datasheet).
     */

    ret = i2c_smbus_write_word_swapped(client, INA2XX_CALIBRATION, val);

    if (ret < 0) {
        devdrv_drv_err("init INA2XX_CALIBRATION fail %d\n", ret);
        return ret;
    }

    return 0;
}

STATIC int ina2xx_do_update(u8 reg)
{
    struct ina2xx_data *data = g_i2c_data;
    struct i2c_client *client = data->client;
    int rv, retry;

    devdrv_drv_debug("Starting ina2xx update\n");
    if (reg >= INA2XX_MAX_REGISTERS) {
        return -EINVAL;
    }
    for (retry = 5; retry; retry--) {
        /* Read registers */
        rv = i2c_smbus_read_word_swapped(client, reg);
        if (rv < 0) {
            devdrv_drv_err("read reg fail (%d)\n", rv);
            return rv;
        }
        data->regs[reg] = rv;

        rv = i2c_smbus_read_word_swapped(client, INA2XX_CALIBRATION);
        if (rv < 0) {
            devdrv_drv_err("read INA2XX_CALIBRATION reg fail (%d) \n", rv);
            return rv;
        }
        data->regs[INA2XX_CALIBRATION] = rv;

        if (data->regs[INA2XX_CALIBRATION] == 0) {
            devdrv_drv_warn("chip not calibrated, reinitializing\n");
            rv = ina2xx_init(data);
            if (rv < 0) {
                devdrv_drv_err("init device fail (%d)\n", rv);
                return rv;
            }
            /*
             * Ensure that the current registers and power.
             * Updated and try again.
             */
            msleep(INA2XX_MAX_DELAY);
            continue;
        }
        data->last_updated = jiffies;
        data->valid = 1;
        return 0;
    }

    /*
     * At this point, the chip returns 0 in the calibration register.
     * Even though the write operation is successful.
     * There's no way.
     */
    devdrv_drv_err("unable to reinitialize the chip\n");
    return -ENODEV;
}

STATIC int ina2xx_update_device(u8 reg)
{
    struct ina2xx_data *data = g_i2c_data;

    unsigned long after;
    int rv;

    mutex_lock(&data->update_lock);

    after = data->last_updated + data->update_interval;
    if (time_after(jiffies, after) || !data->valid) {
        rv = ina2xx_do_update(reg);
        if (rv < 0) {
            mutex_unlock(&data->update_lock);
            devdrv_drv_err("unable to update the data\n");
            return -1;
        }
    }

    mutex_unlock(&data->update_lock);

    return 0;
}

int ina2xx_get_value(u8 reg, int *result)
{
    int val;
    struct ina2xx_data *data = g_i2c_data;

    if (data == NULL) {
        return -ENODEV;
    }
    if (ina2xx_update_device(reg) != 0) {
        return -EIO;
    }
    switch (reg) {
        case INA2XX_SHUNT_VOLTAGE:
            /* signed register */
            val = DIV_ROUND_CLOSEST((s16)data->regs[reg], data->config->shunt_div);
            break;
        case INA2XX_BUS_VOLTAGE:
            val = (data->regs[reg] >> (unsigned int)data->config->bus_voltage_shift) * data->config->bus_voltage_lsb;
            val = DIV_ROUND_CLOSEST(val, 1000);
            break;
        case INA2XX_POWER:
            val = DIV_ROUND_CLOSEST(((int)data->regs[reg] * data->config->power_lsb), 1000);
            break;
        case INA2XX_CURRENT:
            /* signed register, LSB=1mA (selected), in mA */
            val = (s16)data->regs[reg];
            break;
        default:
            /* programmer goofed */
            devdrv_drv_err("Invaild reg\n");
            val = 0;
            return -EINVAL;
    }
    *result = val;
    return 0;
}

STATIC int ina2xx_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct i2c_adapter *adapter = client->adapter;
    int ret;

    if (g_i2c_data != NULL) {
        devdrv_drv_err("Only can be probed once g_i2c_data=%pK.\n", g_i2c_data);
        return -EINVAL;
    }

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_WORD_DATA)) {
        devdrv_drv_err("i2c_check_functionality fail\n");
        return -ENODEV;
    }

    g_i2c_data = kzalloc(sizeof(struct ina2xx_data), GFP_KERNEL | __GFP_ACCOUNT);
    if (g_i2c_data == NULL) {
        devdrv_drv_err("kzalloc fail\n");
        return -ENOMEM;
    }
    mutex_init(&g_i2c_data->update_lock);

    g_i2c_data->rshunt = INA2XX_RSHUNT_DEFAULT;


    /* set the device type */
    g_i2c_data->config = &ina2xx_config[INA226];
    g_i2c_data->curr_config = g_i2c_data->config->config_default;
    g_i2c_data->client = client;

    /*
     * Ina226 has a variable update_interval. For ina219 we
     * use a constant value.
     */
    ina226_set_update_interval(g_i2c_data);

    ret = ina2xx_init(g_i2c_data);
    if (ret < 0) {
        devdrv_drv_err("error configuring the device: %d\n", ret);
        kfree(g_i2c_data);
        g_i2c_data = NULL;
        return -ENODEV;
    }


    devdrv_drv_info("power monitor %s (Rshunt = %li uOhm)\n", id->name, g_i2c_data->rshunt);
    return 0;
}

STATIC int ina2xx_remove(struct i2c_client *client)
{
    if (g_i2c_data != NULL) {
        kfree(g_i2c_data);
        g_i2c_data = NULL;
    }
    return 0;
}

STATIC const struct i2c_device_id ina2xx_id[] = {
    { "ti,ina226", INA226 },
    { "ti,ina230", INA226 },
    { "ti,ina231", INA226 },
    { }
};
MODULE_DEVICE_TABLE(i2c, ina2xx_id);

STATIC const struct of_device_id ina2xx_of_match[] = {
    {.compatible = "ti,ina231", },
    {},
};
MODULE_DEVICE_TABLE(of, ina2xx_of_match);


STATIC struct i2c_driver ina2xx_driver = {
    .driver = {
    .name = "ina2xx",
    .of_match_table = ina2xx_of_match,
    },
    .probe = ina2xx_probe,
    .remove = ina2xx_remove,
    .id_table = ina2xx_id,
};

void ina2xx_register_drv(void)
{
    int ret;
    ret = i2c_add_driver(&ina2xx_driver);
    if (ret != 0) {
        devdrv_drv_err("error register the device: %d\n", ret);
    }
}

void ina2xx_unregister_drv(void)
{
    i2c_del_driver(&ina2xx_driver);
}
