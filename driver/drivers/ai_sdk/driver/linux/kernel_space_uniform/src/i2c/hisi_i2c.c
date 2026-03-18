/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Description: i2c source files of logical layer Source Files
 * Author: huawei
 * Create: 2022-04-14
 */

#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/version.h>
#include <linux/securec.h>
#include <linux/acpi.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/reset-controller.h>
#include <linux/reset.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/err.h>
#include "hisi_i2c.h"
#include "kdrv_i2c_api.h"
#include "i2c_misc.h"

static int hisi_i2c_reset(struct device *dev, u32 bus_id)
{
    int ret;
    bool enabled;

    /* 1. Disable the clock. */
    ret = hisi_subctrl_i2c_clkgate_disable(dev, bus_id);
    if (ret != 0) {
        dev_err(dev, "hisi_subctrl_i2c_clkgate_disable failed, ret=%d\n", ret);
        return ret;
    }
    /* 2. Perform reset. */
    ret = hisi_subctrl_i2c_reset_assert(dev, bus_id);
    if (ret != 0) {
        dev_err(dev, "hisi_subctrl_i2c_reset_assert failed, ret=%d\n", ret);
        return ret;
    }
    /* 3. Turn on the clock. */
    ret = hisi_subctrl_i2c_clkgate_enable(dev, bus_id);
    if (ret != 0) {
        dev_err(dev, "hisi_subctrl_i2c_clkgate_enable failed, ret=%d\n", ret);
        return ret;
    }
    /* 4. Wait for 100 μs. */
    udelay(100);
    /* 5. Disable the clock. */
    ret = hisi_subctrl_i2c_clkgate_disable(dev, bus_id);
    if (ret != 0) {
        dev_err(dev, "hisi_subctrl_i2c_clkgate_disable failed, ret=%d\n", ret);
        return ret;
    }
    /* 6. Deassert reset. */
    ret = hisi_subctrl_i2c_reset_deassert(dev, bus_id);
    if (ret != 0) {
        dev_err(dev, "hisi_subctrl_i2c_reset_deassert failed, ret=%d\n", ret);
        return ret;
    }
    /* 7. Obtain the reset deassertion status. */
    ret = hisi_subctrl_i2c_get_reset_status(dev, bus_id);
    if (ret != 0) {
        dev_err(dev, "i2c is not deassert status, ret=%d\n", ret);
        return ret;
    }
    /* 8. Turn on the clock */
    ret = hisi_subctrl_i2c_clkgate_enable(dev, bus_id);
    if (ret != 0) {
        dev_err(dev, "hisi_subctrl_i2c_clkgate_enable failed, ret=%d\n", ret);
        return ret;
    }
    enabled = hisi_subctrl_i2c_clkgate_is_enabled(dev, bus_id);
    if (!enabled) {
        dev_err(dev, "hisi_i2c_clkgate_is_ not enabled\n");
        return -1;
    }
    return 0;
}

static void hisi_i2c_clear_trans_info(struct hisi_i2c_controller *hi2c)
{
    hi2c->trans_info.msg_num = 0;
    hi2c->trans_info.xfer_err = 0;
    hi2c->trans_info.msg_tx_idx = 0;
    hi2c->trans_info.msg_rx_idx = 0;
    hi2c->trans_info.buf_tx_idx = 0;
    hi2c->trans_info.buf_rx_idx = 0;
    hi2c->trans_info.msg_err = 0;
}

static void hisi_i2c_clear_trans_statis_info(struct hisi_i2c_controller *hi2c)
{
    hi2c->statis_info.rx_cnt = 0;
    hi2c->statis_info.tx_cnt = 0;
    hi2c->statis_info.err_times = 0;
    hi2c->statis_info.recovery_times = 0;
}

static void hisi_i2c_enable_xfer_int(struct hisi_i2c_controller *hi2c)
{
    hi2c->soc_ops->enable_rx_afull_int(&hi2c->reg, 1);
    hi2c->soc_ops->enable_trans_cplt_int(&hi2c->reg, 1);
    hi2c->soc_ops->enable_err_int(&hi2c->reg, 1);
    hi2c->soc_ops->enable_tx_aempty_int(&hi2c->reg, 1);
}

static void hisi_i2c_disable_xfer_int(struct hisi_i2c_controller *hi2c)
{
    hi2c->soc_ops->enable_tx_aempty_int(&hi2c->reg, 0);
    hi2c->soc_ops->enable_rx_afull_int(&hi2c->reg, 0);
    hi2c->soc_ops->enable_trans_cplt_int(&hi2c->reg, 0);
    hi2c->soc_ops->enable_err_int(&hi2c->reg, 0);
}

static void hisi_i2c_start_xfer(struct hisi_i2c_controller *hi2c)
{
    bool is_10bit_addr = false;
    struct i2c_msg *msg = hi2c->trans_info.msgs;

    if (msg->flags & I2C_M_TEN) {
        is_10bit_addr = true;
    }

    hi2c->soc_ops->xfer_init(&hi2c->reg, msg->addr, is_10bit_addr);
    hisi_i2c_enable_xfer_int(hi2c);
}

static void hisi_i2c_xfer_msg(struct hisi_i2c_controller *hi2c)
{
    int max_write = hi2c->soc_ops->get_tx_fifo_limit(&hi2c->reg);
    bool last_msg;
    struct i2c_msg *cur_msg;
    bool need_restart = false;
    bool need_stop = false;
    u8 data;
    u16 wr;

    while (hi2c->trans_info.msg_tx_idx < hi2c->trans_info.msg_num) {
        cur_msg = hi2c->trans_info.msgs + hi2c->trans_info.msg_tx_idx;
        last_msg = (hi2c->trans_info.msg_tx_idx == hi2c->trans_info.msg_num - 1);
        if (cur_msg->buf == NULL) {
            hi2c->trans_info.msg_err = -EINVAL;
            dev_err(hi2c->dev, "msg[%d]->buf is null\n", hi2c->trans_info.msg_tx_idx);
            break;
        }
        if (hi2c->trans_info.msg_tx_idx && !hi2c->trans_info.buf_tx_idx) {
        // The second message starts. The first frame needs to be restarted.
            need_restart = true;
        }

        while ((hi2c->trans_info.buf_tx_idx < cur_msg->len) && (max_write > 0)) {
            if (hi2c->trans_info.buf_tx_idx == cur_msg->len - 1 && last_msg) {
            // The last message. The last frame needs to be stopped.
                need_stop = true;
            }
            data = cur_msg->buf[hi2c->trans_info.buf_tx_idx];
            wr = cur_msg->flags & I2C_M_RD;
            hi2c->soc_ops->write_tx_data(&hi2c->reg, data, wr, need_stop, need_restart);
            need_restart = false;
            need_stop = false;
            hi2c->trans_info.buf_tx_idx++;
            hi2c->statis_info.tx_cnt++;
            max_write--;
        }

        /* Each message is sent. Updates idx */
        if (hi2c->trans_info.buf_tx_idx == cur_msg->len) {
            hi2c->trans_info.buf_tx_idx = 0;
            hi2c->trans_info.msg_tx_idx++;
        }

        if ((hi2c->soc_ops->tx_fifo_full(&hi2c->reg) == true) || max_write == 0) {
        // fifo full, Wait for the next interruption to continue sending.
            break;
        }
    }
    if (hi2c->trans_info.msg_tx_idx == hi2c->trans_info.msg_num) { // It's all over. forbide tx, empty irq.
        hi2c->soc_ops->enable_tx_aempty_int(&hi2c->reg, 0);
    }
}

static int hisi_i2c_read_rx_fifo(struct hisi_i2c_controller *hi2c)
{
    struct i2c_msg *cur_msg;

    while (hi2c->trans_info.msg_rx_idx < hi2c->trans_info.msg_num) {
        cur_msg = hi2c->trans_info.msgs + hi2c->trans_info.msg_rx_idx;

        if (cur_msg->buf == NULL) {
            hi2c->trans_info.msg_err = -EINVAL;
            dev_err(hi2c->dev, "msg[%d]->buf is null\n", hi2c->trans_info.msg_rx_idx);
            break;
        }
        if (!(cur_msg->flags & I2C_M_RD)) { // write
            hi2c->trans_info.msg_rx_idx++;
            continue;
        }

        while ((hi2c->soc_ops->rx_fifo_empty(&hi2c->reg) == false) && (hi2c->trans_info.buf_rx_idx < cur_msg->len)) {
            cur_msg->buf[hi2c->trans_info.buf_rx_idx++] = hi2c->soc_ops->read_rx_data(&hi2c->reg);
            hi2c->statis_info.rx_cnt++;
        }

        if (hi2c->trans_info.buf_rx_idx == cur_msg->len) {
            hi2c->trans_info.buf_rx_idx = 0;
            hi2c->trans_info.msg_rx_idx++;
        }

        if (hi2c->soc_ops->rx_fifo_empty(&hi2c->reg) == true) {
        // fifo empty, Wait for the next interruption to continue receiving.
            break;
        }
    }

    if (hi2c->trans_info.msg_rx_idx == hi2c->trans_info.msg_num) {
        hi2c->soc_ops->enable_rx_afull_int(&hi2c->reg, 0);
    }

    return 0;
}

static int hisi_i2c_master_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    struct hisi_i2c_controller *hi2c = i2c_get_adapdata(adap);
    int ret = 0;
    struct i2c_int_status_info int_status = {0};

    reinit_completion(&hi2c->trans_info.cmd_complete);
    hi2c->soc_ops->get_int_status_info(&hi2c->reg, &int_status);
    hi2c->soc_ops->clear_all_int(&hi2c->reg);

    hisi_i2c_clear_trans_statis_info(hi2c);
    hi2c->trans_info.msg_num = num;
    hi2c->trans_info.msgs = msgs;

    hisi_i2c_start_xfer(hi2c); // Start transfer, Enables related interrupts.

    if (!wait_for_completion_timeout(&hi2c->trans_info.cmd_complete, adap->timeout)) {
        hisi_i2c_disable_xfer_int(hi2c);
        i2c_recover_bus(&hi2c->adapter);
        dev_err(hi2c->dev, "i2c bus transfer timeout\n");
        ret = -ETIMEDOUT;
    }

    if (hi2c->trans_info.msg_err != 0) {
        ret = hi2c->trans_info.msg_err;
    }
    if (hi2c->trans_info.xfer_err != 0) {
        hi2c->soc_ops->handle_errors(hi2c->dev, &hi2c->reg, hi2c->trans_info.xfer_err, hi2c->trans_info.err_source);
        ret = -EIO;
    }
    hisi_i2c_clear_trans_info(hi2c);

    return ret ? ret : num;
}

static u32 hisi_i2c_functionality(struct i2c_adapter *adap)
{
    return I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm hisi_i2c_algo = {
    .master_xfer    = hisi_i2c_master_xfer,
    .functionality    = hisi_i2c_functionality,
};

static irqreturn_t hisi_i2c_irq_handle(int irq, void *data)
{
    struct hisi_i2c_controller *hi2c = (struct hisi_i2c_controller *)data;
    struct i2c_int_status_info int_status = {0};

    hi2c->soc_ops->get_int_status_info(&hi2c->reg, &int_status);
    if (int_status.is_err_irq) {
        hi2c->statis_info.err_times++;
        hi2c->trans_info.xfer_err = int_status.xfer_err;
        hi2c->trans_info.err_source = int_status.err_source;
        hi2c->soc_ops->clear_err_int(&hi2c->reg);
        goto out;
    }

    /* tx empty */
    if (int_status.is_tx_aempty_irq) {
        hisi_i2c_xfer_msg(hi2c);
        hi2c->soc_ops->clear_tx_aempty_int(&hi2c->reg);
    }

    /* rx full or transfer completed */
    if (int_status.is_rx_afull_irq || int_status.is_trans_cplt_irq) {
        hisi_i2c_read_rx_fifo(hi2c);
        hi2c->soc_ops->clear_rx_afull_int(&hi2c->reg);
    }
out:
    mb();
    // successfully compltion or msg err.
    // skipping err INT_ERR because on error cases, it would be two interrupts: INT_ERR first then TRANS_CPLT.
    if (int_status.is_trans_cplt_irq || (hi2c->trans_info.msg_err != 0)) {
        hisi_i2c_disable_xfer_int(hi2c);
        hi2c->soc_ops->clear_trans_cplt_int(&hi2c->reg);
        complete(&hi2c->trans_info.cmd_complete);
    }

    return IRQ_HANDLED;
}

static int hisi_i2c_subctrl_recovery(struct i2c_adapter *adap)
{
    struct hisi_i2c_controller *hi2c = i2c_get_adapdata(adap);
    int ret;

    ret = hisi_subctrl_i2c_recovery(hi2c->dev, hi2c->bus_id);
    if (ret != 0) {
        dev_err(hi2c->dev, "%s hisi_subctrl_i2c_recovery fail\n", __FUNCTION__);
        return ret;
    }
    ret = hisi_i2c_reset(hi2c->dev, hi2c->bus_id);
    if (ret != 0) {
        dev_err(hi2c->dev, "%s hisi_i2c_reset fail\n", __FUNCTION__);
        return ret;
    }
    hi2c->soc_ops->config_bus(&hi2c->reg, hi2c->timings.t.bus_freq_hz, &hi2c->timings.cfg);
    hi2c->statis_info.recovery_times++;
    return 0;
}

static void hisi_i2c_prepare_recovery(struct i2c_adapter *adap)
{
    struct hisi_i2c_controller *hi2c = i2c_get_adapdata(adap);

    dev_info(hi2c->dev, "%s pin switch to gpio\n", __FUNCTION__);
    pinctrl_select_state(hi2c->pinctrl, hi2c->pinctrl_pins_gpio);
}

static void hisi_i2c_unprepare_recovery(struct i2c_adapter *adap)
{
    struct hisi_i2c_controller *hi2c = i2c_get_adapdata(adap);
    int ret;

    dev_info(hi2c->dev, "%s pin switch to i2c fuc\n", __FUNCTION__);
    pinctrl_select_state(hi2c->pinctrl, hi2c->pinctrl_pins_default);
    ret = hisi_i2c_reset(hi2c->dev, hi2c->bus_id);
    if (ret != 0) {
        dev_err(hi2c->dev, "%s hisi_i2c_reset fail\n", __FUNCTION__);
        return;
    }
    hi2c->soc_ops->config_bus(&hi2c->reg, hi2c->timings.t.bus_freq_hz, &hi2c->timings.cfg);
    hi2c->statis_info.recovery_times++;
}

static int hisi_i2c_init_recovery_info(struct hisi_i2c_controller *hi2c)
{
    struct i2c_bus_recovery_info *info = &hi2c->rinfo;
    struct device *dev = hi2c->dev;
    // The subctrl mode is used to prevent suspension.
    if (device_property_read_bool(dev, "recovery_by_subctrl")) {
        info->recover_bus = hisi_i2c_subctrl_recovery;
        hi2c->adapter.bus_recovery_info = info;
        return 0;
    }
    // Use the gpio mode to prevent suspension.
    hi2c->pinctrl = devm_pinctrl_get(dev);
    if (!hi2c->pinctrl || IS_ERR(hi2c->pinctrl)) {
        dev_info(dev, "can't get pinctrl, bus recovery not supported\n");
        return -EINVAL;
    }
    hi2c->pinctrl_pins_default = pinctrl_lookup_state(hi2c->pinctrl, PINCTRL_STATE_DEFAULT);
    hi2c->pinctrl_pins_gpio = pinctrl_lookup_state(hi2c->pinctrl, "gpio");
    if (IS_ERR(hi2c->pinctrl_pins_default) || IS_ERR(hi2c->pinctrl_pins_gpio)) {
        dev_err(dev, " get pinctrl_pins_default or pinctrl_pins_gpio fail!\n");
        return -EINVAL;
    }

    info->sda_gpiod = devm_gpiod_get(dev, "sda", GPIOD_IN);
    info->scl_gpiod = devm_gpiod_get(dev, "scl", GPIOD_OUT_HIGH_OPEN_DRAIN);

    if (IS_ERR(info->scl_gpiod) || IS_ERR(info->sda_gpiod)) {
        dev_err(dev, "get scl_gpiod or sda_gpiod fail\n");
        return -EINVAL;
    }
    info->recover_bus = i2c_generic_scl_recovery;
    info->prepare_recovery = hisi_i2c_prepare_recovery;
    info->unprepare_recovery = hisi_i2c_unprepare_recovery;
    hi2c->adapter.bus_recovery_info = info;
    return 0;
}

static int hisi_i2c_set_pinctrl_default(struct hisi_i2c_controller *hi2c)
{
    int ret;
    struct device *dev = hi2c->dev;

    hi2c->pinctrl = devm_pinctrl_get(dev);
    if (IS_ERR(hi2c->pinctrl)) {
        dev_info(dev, "current not support pinctrl\n");
        hi2c->pinctrl = NULL;
        return 0;
    }
    hi2c->pinctrl_pins_default = pinctrl_lookup_state(hi2c->pinctrl, PINCTRL_STATE_DEFAULT);
    if (IS_ERR(hi2c->pinctrl_pins_default)) {
        dev_info(dev, "current not support pinctrl_pins_default\n");
        devm_pinctrl_put(hi2c->pinctrl);
        hi2c->pinctrl_pins_default = NULL;
        return 0;
    }
    ret = pinctrl_select_state(hi2c->pinctrl, hi2c->pinctrl_pins_default);
    if (ret != 0) {
        dev_err(dev, "pinctrl select default state fail\n");
        devm_pinctrl_put(hi2c->pinctrl);
        hi2c->pinctrl_pins_default = NULL;
    }

    return ret;
}

static int hisi_i2c_set_pinctrl_idle(struct hisi_i2c_controller *hi2c)
{
    int ret;
    struct device *dev = hi2c->dev;

    hi2c->pinctrl = devm_pinctrl_get(dev);
    if (IS_ERR(hi2c->pinctrl)) {
        dev_info(dev, "current not support pinctrl\n");
        hi2c->pinctrl = NULL;
        return 0;
    }
    hi2c->pinctrl_pins_idle = pinctrl_lookup_state(hi2c->pinctrl, PINCTRL_STATE_IDLE);
    if (IS_ERR(hi2c->pinctrl_pins_idle)) {
        dev_info(dev, "current not support pinctrl_pins_idle\n");
        devm_pinctrl_put(hi2c->pinctrl);
        hi2c->pinctrl_pins_idle = NULL;
        return 0;
    }
    ret = pinctrl_select_state(hi2c->pinctrl, hi2c->pinctrl_pins_idle);
    if (ret != 0) {
        dev_err(dev, "pinctrl select idle state fail\n");
        devm_pinctrl_put(hi2c->pinctrl);
        hi2c->pinctrl_pins_idle = NULL;
    }

    return ret;
}

static int hisi_i2c_parse_dts_acpi_para(struct platform_device *pdev, struct hisi_i2c_controller *hi2c)
{
    int ret;
    struct resource *res;
    u32 clk_rate_hz;
    struct device *dev = hi2c->dev;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    hi2c->reg.io_base = devm_ioremap_resource(dev, res);
    if (IS_ERR(hi2c->reg.io_base)) {
        dev_err(dev, "failed to get reg io_base\n");
        return PTR_ERR(hi2c->reg.io_base);
    }
    hi2c->reg.io_size = res->end - res->start + 1;
    hi2c->irq = platform_get_irq(pdev, 0);
    if (hi2c->irq < 0) {
        dev_err(dev, "failed to get irq\n");
        return hi2c->irq;
    }
    /* Obtains the I2C working clock. */
    ret = device_property_read_u32(dev, "clk_rate", &clk_rate_hz);
    if (ret != 0) {
        dev_err(dev, "failed to get clock frequency, ret = %d\n", ret);
        return ret;
    }
    ret = device_property_read_u32(dev, "bus_id", &hi2c->bus_id);
    if (ret != 0) {
        dev_err(dev, "failed to get i2c bus_id, ret = %d\n", ret);
        return ret;
    }
    hi2c->timings.cfg.clk_rate_khz = DIV_ROUND_UP_ULL(clk_rate_hz, HZ_PER_KHZ);
    i2c_parse_fw_timings(dev, &hi2c->timings.t, true);

#ifdef CONFIG_DEBUG_UDRV
    ret = hisi_i2c_subctrl_init(hi2c->bus_id, dev);
    if (ret != 0) {
        return ret;
    }
#endif
    ret = hisi_i2c_get_clk_rst_info(hi2c);
    if (ret != 0) {
        return ret;
    }
    // Initialize. Mounting anti-suspension information.
    ret = hisi_i2c_init_recovery_info(hi2c);
    if (ret != 0) {
        dev_err(dev, "hisi_i2c_init_recovery_info fail\n");
        return ret;
    }
    return 0;
}

static void hisi_i2c_calcu_cfg_cnt(struct i2c_timings *t, struct i2c_timings_cfg *cfg)
{
    u32 total_cnt;
    u32 t_scl_hcnt, t_scl_lcnt, scl_fall_cnt, scl_rise_cnt;
    u32 divide;
    u32 divisor;

    switch (t->bus_freq_hz) {
        case I2C_MAX_FAST_MODE_FREQ:
            divide = FAST_SPEED_MODE_DIVIDE;
            divisor = FAST_SPEED_MODE_DIVISOR;
            break;
        case I2C_MAX_HIGH_SPEED_MODE_FREQ:
            divide = HIGH_SPEED_MODE_DIVIDE;
            divisor = HIGH_SPEED_MODE_DIVISOR;
            break;
        case I2C_MAX_STD_MODE_FREQ:
        default:
            t->bus_freq_hz = I2C_MAX_STD_MODE_FREQ;
            divide = STD_SPEED_MODE_DIVIDE;
            divisor = STD_SPEED_MODE_DIVISOR;
            break;
    }
    /* Total SCL clock cycles per speed period */
    total_cnt = DIV_ROUND_UP_ULL(cfg->clk_rate_khz * HZ_PER_KHZ, t->bus_freq_hz);
    /* Total HIGH level SCL clock cycles including edges */
    t_scl_hcnt = DIV_ROUND_UP_ULL(total_cnt * divide, divisor);
    /* Total LOW level SCL clock cycles including edges */
    t_scl_lcnt = total_cnt - t_scl_hcnt;
    /* Fall edge SCL clock cycles */
    scl_fall_cnt = DIV_ROUND_UP_ULL(t->scl_fall_ns * cfg->clk_rate_khz, NSEC_PER_MSEC);
    /* Rise edge SCL clock cycles */
    scl_rise_cnt = DIV_ROUND_UP_ULL(t->scl_rise_ns * cfg->clk_rate_khz, NSEC_PER_MSEC);

    /* Configure the SDA hold time required when sending */
    cfg->sda_hold_cnt = DIV_ROUND_UP_ULL(t->sda_hold_ns * cfg->clk_rate_khz, NSEC_PER_MSEC);

    /* In fast mode, deglitch length */
    cfg->spk_len = DIV_ROUND_UP_ULL(t->digital_filter_width_ns * cfg->clk_rate_khz, NSEC_PER_MSEC);

    /* Calculated HIGH and LOW periods of SCL clock */
    cfg->scl_hcnt = t_scl_hcnt - cfg->spk_len - 7 - scl_fall_cnt; // Formulas hcnt-spk_len-7-scl_fall_cnt
    cfg->scl_lcnt = t_scl_lcnt - 1 - scl_rise_cnt;
}

static int hisi_i2c_fill_adapter(struct i2c_adapter *adapter, struct hisi_i2c_controller *hi2c)
{
    int ret;

    adapter->owner = THIS_MODULE;
    adapter->algo = &hisi_i2c_algo;
    adapter->dev.parent = hi2c->dev;
    adapter->dev.of_node = hi2c->dev->of_node;
    adapter->nr = hi2c->bus_id;
    ret = snprintf_s(adapter->name, sizeof(adapter->name), sizeof(adapter->name) - 1,
                     "HiSilicon I2C Controller %s", dev_name(hi2c->dev));
    if (ret == -1) {
        dev_err(hi2c->dev, "snprintf_s failed\n");
        return ret;
    }
    return 0;
}

#ifdef CONFIG_FMEA
static struct fmea_ops i2c_fmea_ops;
static int i2c_fmea_init(struct hisi_i2c_controller *hi2c)
{
    struct fmea_dev_info dev_info;
    struct fmea_iomm_info iomm_info;
    const struct fmea_item_ruler_data *ruler_data;
    const struct ndrv_fmea_item_info *item_table;
    int nums = 0;
    int ret;
    dev_info.dev = hi2c->dev;
    dev_info.name = FMEA_MODULE_NAME(i2c);
    dev_info.pdev_id = hi2c->bus_id;
    (void)fmea_init_default_ops(&i2c_fmea_ops);

    ruler_data = i2c_get_fmea_ruler_data(&nums);
    if (ruler_data == NULL || nums == 0) {
        dev_err(hi2c->dev, "spi_get_fmea_io_region_node fail\n");
        return -EINVAL;
    }
    iomm_info.vaddr = hi2c->reg.io_base;
    iomm_info.io_size = hi2c->reg.io_size;
    iomm_info.ops = &i2c_fmea_ops;
    item_table = i2c_get_fmea_table();
    if (item_table == NULL) {
        dev_err(hi2c->dev, "i2c_get_fmea_table fail\n");
        return -EINVAL;
    }
    dev_err(dev_info.dev, "i2c_dev_init_fmea %s\n", dev_info.name);

    ret = kdrv_fmea_iomm_unit_init(&hi2c->obj, &dev_info, &iomm_info, ruler_data, item_table);
    if (ret != 0) {
        dev_err(hi2c->dev, "kdrv_fmea_iomm_unit_init fail\n");
        return -EINVAL;
    }
    return 0;
}

static void i2c_fmea_deinit(struct hisi_i2c_controller *hi2c)
{
    kdrv_fmea_iomm_unit_deinit(&hi2c->obj);
}

int kdrv_i2c_fmea_entry(u32 devid, u64 *err_info, u32 *alm_flg, char *buf, u32 size)
{
    int ret;
    struct hisi_i2c_controller *hi2c = NULL;
    struct fmea_iomm_unit* i2c_fmobj = NULL;
    hi2c = hisi_i2c_get_controller_by_id(devid);
    if (hi2c == NULL) {
        pr_err("[kdrv_i2c_fmea_entry], get hi2c failed\n");
        return -EINVAL;
    }

    i2c_fmobj =  &hi2c->obj;
    ret = kdrv_fmea_entry(i2c_fmobj, err_info, alm_flg, buf, size);
    return ret;
}
EXPORT_SYMBOL(kdrv_i2c_fmea_entry);
#endif

static int hisi_i2c_check_ops(const struct hisi_i2c_ops *ops)
{
    if (ops->config_bus == NULL || ops->enable_tx_aempty_int == NULL || ops->enable_rx_afull_int == NULL ||
        ops->enable_trans_cplt_int == NULL || ops->disable_all_int == NULL || ops->get_int_status_info == NULL ||
        ops->clear_all_int == NULL || ops->clear_tx_aempty_int == NULL || ops->clear_rx_afull_int == NULL ||
        ops->clear_trans_cplt_int == NULL || ops->xfer_init == NULL || ops->write_tx_data == NULL ||
        ops->read_rx_data == NULL || ops->tx_fifo_full == NULL || ops->rx_fifo_empty == NULL ||
        ops->get_tx_fifo_limit == NULL || ops->clear_err_int == NULL || ops->handle_errors == NULL) {
        pr_err("hisi_i2c_check_ops failed\n");
        return -EINVAL;
    }
    return 0;
}

static int hisi_i2c_probe(struct platform_device *pdev)
{
    struct hisi_i2c_controller *hi2c;
    struct device *dev = &pdev->dev;
    struct i2c_adapter *adapter;
    int ret;

    dev_info(dev, "hisi_i2c_probe start\n");
    hi2c = devm_kzalloc(dev, sizeof(*hi2c), GFP_KERNEL);
    if (!hi2c) {
        return -ENOMEM;
    }
    init_completion(&hi2c->trans_info.cmd_complete);
    platform_set_drvdata(pdev, hi2c);
    hi2c->dev = dev;
    hi2c->soc_ops = hisi_i2c_get_ops();
    if (hisi_i2c_check_ops(hi2c->soc_ops)) {
        platform_set_drvdata(pdev, NULL);
        return -EINVAL;
    }

    if (hisi_i2c_parse_dts_acpi_para(pdev, hi2c)) {
        platform_set_drvdata(pdev, NULL);
        return -EINVAL;
    }

    // Reset/Deassert reset controller. Ensure IP address is in reset deassertion state. Clock gating is on.
    if (hisi_i2c_reset(dev, hi2c->bus_id)) {
        platform_set_drvdata(pdev, NULL);
        return -EINVAL;
    }
    hi2c->soc_ops->disable_all_int(&hi2c->reg);
    hi2c->soc_ops->clear_all_int(&hi2c->reg);
    ret = devm_request_irq(dev, hi2c->irq, hisi_i2c_irq_handle, 0, "hisi-i2c", hi2c);
    if (ret != 0) {
        platform_set_drvdata(pdev, NULL);
        dev_err(dev, "failed to request irq handler, ret = %d\n", ret);
        return ret;
    }

#ifdef CONFIG_FMEA
    if (i2c_fmea_init(hi2c)) {
        return -EINVAL;
    }
#endif
    hisi_i2c_calcu_cfg_cnt(&hi2c->timings.t, &hi2c->timings.cfg);
    hi2c->soc_ops->config_bus(&hi2c->reg, hi2c->timings.t.bus_freq_hz, &hi2c->timings.cfg);
    adapter = &hi2c->adapter;
    if (hisi_i2c_fill_adapter(adapter, hi2c)) {
        platform_set_drvdata(pdev, NULL);
        return -EINVAL;
    }
    i2c_set_adapdata(adapter, hi2c);
    ret = i2c_add_numbered_adapter(adapter);
    if (ret != 0) {
        platform_set_drvdata(pdev, NULL);
        dev_err(dev, "failed to i2c add adapter, ret = %d\n", ret);
        return ret;
    }
    dev_info(dev, "hisi_i2c_probe end\n");
    return 0;
}

static int hisi_i2c_remove(struct platform_device *pdev)
{
    struct hisi_i2c_controller *hisi_i2c = platform_get_drvdata(pdev);

    dev_info(&pdev->dev, "%s\n", __FUNCTION__);
    i2c_del_adapter(&hisi_i2c->adapter);
#ifdef CONFIG_FMEA
    i2c_fmea_deinit(hisi_i2c);
#endif
    return 0;
}

static const struct of_device_id hisi_i2c_ids[] = {
    {
        .compatible = "hisilicon,udrv_i2c",
    },
    {}
};
MODULE_DEVICE_TABLE(of, hisi_i2c_ids);

static const struct acpi_device_id hisi_i2c_acpi_ids[] = {
    { "HISI03D1", 0, 0, 0},
    { }
};
MODULE_DEVICE_TABLE(acpi, hisi_i2c_acpi_ids);

static int hisi_i2c_suspend(struct device *dev)
{
    int ret;
    struct hisi_i2c_controller *hi2c = dev_get_drvdata(dev);

    dev_info(dev, "enter %s\n", __FUNCTION__);

    /* Turn off the clock */
    ret = hisi_subctrl_i2c_clkgate_disable(dev, hi2c->bus_id);
    if (ret != 0) {
        dev_err(dev, "hisi_subctrl_i2c_clkgate_disable failed, ret=%d\n", ret);
        return ret;
    }
    /* resetting */
    ret = hisi_subctrl_i2c_reset_assert(dev, hi2c->bus_id);
    if (ret != 0) {
        dev_err(dev, "%s i2c reset failed failed, ret=%d\n", __FUNCTION__, ret);
        return ret;
    }
    /* Set i2c pin to gpio and pulldown */
    ret = hisi_i2c_set_pinctrl_idle(hi2c);
    if (ret != 0) {
        dev_err(dev, "%s failed to select pinctrl state, ret=%d\n", __FUNCTION__, ret);
        return ret;
    }
    return 0;
}

static int hisi_i2c_resume(struct device *dev)
{
    int ret;
    struct hisi_i2c_controller *hi2c = dev_get_drvdata(dev);

    dev_info(dev, "enter %s\n", __FUNCTION__);
    /* Turn off the clock */
    ret = hisi_subctrl_i2c_clkgate_disable(dev, hi2c->bus_id);
    if (ret != 0) {
        dev_err(dev, "hisi_subctrl_i2c_clkgate_disable failed, ret=%d\n", ret);
        return ret;
    }
    /* Deassert reset */
    ret = hisi_subctrl_i2c_reset_deassert(dev, hi2c->bus_id);
    if (ret != 0) {
        dev_err(dev, "%s i2c dereset failed, ret=%d\n", __FUNCTION__, ret);
        return ret;
    }
    /* Obtains the reset deassertion status. */
    ret = hisi_subctrl_i2c_get_reset_status(dev, hi2c->bus_id);
    if (ret != 0) {
        dev_err(dev, "i2c is not deassert status, ret=%d\n", ret);
        return ret;
    }
    /* Turn on the clock */
    ret = hisi_subctrl_i2c_clkgate_enable(dev, hi2c->bus_id);
    if (ret != 0) {
        dev_err(dev, "%s i2c icg enable failed, ret=%d\n", __FUNCTION__, ret);
        return ret;
    }
    hi2c->soc_ops->config_bus(&hi2c->reg, hi2c->timings.t.bus_freq_hz, &hi2c->timings.cfg);
    /* Set i2c pin and pullup */
    ret = hisi_i2c_set_pinctrl_default(hi2c);
    if (ret != 0) {
        dev_err(dev, "%s failed to select pinctrl state, ret=%d\n", __FUNCTION__, ret);
        return ret;
    }
    return 0;
}

int kdrv_i2c_get_err_status(u32 bus_id, struct kdrv_i2c_err_status *status)
{
    struct hisi_i2c_controller *hi2c;

    hi2c = hisi_i2c_get_controller_by_id(bus_id);
    if (hi2c == NULL) {
        pr_err("hisi_i2c_get_controller_by_id fail! bus_id = %u\n", bus_id);
        return -1;
    }
    if (status == NULL) {
        return -EINVAL;
    }
    if (hi2c->soc_ops->get_err_status != NULL) {
        hi2c->soc_ops->get_err_status(&hi2c->reg, status);
    }
    return 0;
}
EXPORT_SYMBOL(kdrv_i2c_get_err_status);

int kdrv_get_i2c_timing_cfg(u32 bus_id, struct kdrv_i2c_timing_cfg *cfg)
{
    struct hisi_i2c_controller *hi2c;

    hi2c = hisi_i2c_get_controller_by_id(bus_id);
    if (hi2c == NULL) {
        pr_err("hisi_i2c_get_controller_by_id fail! bus_id = %u\n", bus_id);
        return -EINVAL;
    }
    if (cfg == NULL) {
        return -EINVAL;
    }
    if (hi2c->soc_ops->get_timing_cfg != NULL) {
        hi2c->soc_ops->get_timing_cfg(&hi2c->reg, cfg);
    }
    return 0;
}
EXPORT_SYMBOL(kdrv_get_i2c_timing_cfg);

int kdrv_get_i2c_trans_statistic_info(u32 bus_id, struct kdrv_i2c_trans_statistic_info *info)
{
    struct hisi_i2c_controller *hi2c;

    hi2c = hisi_i2c_get_controller_by_id(bus_id);
    if (hi2c == NULL) {
        pr_err("hisi_i2c_get_controller_by_id fail! bus_id = %u\n", bus_id);
        return -EINVAL;
    }
    if (info == NULL) {
        return -EINVAL;
    }
    info = &hi2c->statis_info;
    return 0;
}
EXPORT_SYMBOL(kdrv_get_i2c_trans_statistic_info);

static const struct dev_pm_ops hisi_i2c_pm = {
    .suspend        = hisi_i2c_suspend,
    .resume         = hisi_i2c_resume,
};

static struct platform_driver hisi_i2c_driver = {
    .probe        = hisi_i2c_probe,
    .remove		= hisi_i2c_remove,
    .driver        = {
        .name    = "hisi-i2c",
        .pm = &hisi_i2c_pm,
        .acpi_match_table = ACPI_PTR(hisi_i2c_acpi_ids),
        .of_match_table = of_match_ptr(hisi_i2c_ids),
    },
};
module_platform_driver(hisi_i2c_driver);

static int find_hi2c_by_dev_handle(struct device *dev, const void *data)
{
    u32 bus_id = *(u32 *)data;
    struct hisi_i2c_controller *hi2c = (struct hisi_i2c_controller *)dev_get_drvdata(dev);

    if (hi2c->bus_id == bus_id) {
        return 1; // Found exit traversal dev
    }
    return 0;
}
struct hisi_i2c_controller *hisi_i2c_get_controller_by_id(u32 bus_id)
{
    struct device *dev = NULL;

    dev = driver_find_device(&hisi_i2c_driver.driver, NULL, (void *)&bus_id, find_hi2c_by_dev_handle);
    if (dev == NULL) {
        pr_err("driver_find_device fail: bus_id=%u,\n", bus_id);
        return NULL;
    }

    return (struct hisi_i2c_controller *)dev_get_drvdata(dev);
}

MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("I2C Controller Driver");
MODULE_LICENSE("GPL");
