/*
 * Device driver for regulators in HISI PMIC IC
 *
 * Copyright (c) 2011 Hisilicon.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef DRV_UT
#include <hisi_pmic.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/irq.h>

#define RET_SUCC                 0
#define RET_FAIL                (-1)

/* LDO2\9\16 control register address */
#define LDO2_ONOFF1            (0x014)
#define LDO2_VSET              (0x050)
#define LDO9_ONOFF_ECO         (0x01C)
#define LDO9_VSET              (0x057)
#define LDO16_ONOFF_ECO        (0x022)
#define LDO16_VSET             (0x05D)
#define PMU_IRQ_MASK_0         (0x202)
#define PMU_IRQ_MASK_1         (0x203)
#define PMU_IRQ0               (0x212)
#define PMU_IRQ1               (0x213)
#define PMU_HRST_REG0          (0x24D)
#define PMU_IRQ_MARK           (0x061)

/* HKADC properties */
#define HKADC_CHANNEL_MAX       15
#define HKADC_VALID_VALUE      (0x0FFF)
#define HKADC_VREF_1V8          1800
#define HKADC_ACCURACY         (0xFFF)

/* HKADC register address */
#define HKADC_CTRL_ADDR        (0x2AE)
#define HKADC_START_ADDR       (0x2AF)
#define HKADC_DATA1_ADDR       (0x2B1)
#define HKADC_DATA0_ADDR       (0x2B2)
#define HKADC_VALUE_HIGH       (0x0FF0)
#define HKADC_VALUE_LOW        (0x000F)

#define HISI_MASK_FIELD        (0xFF)

#define REG_SCBAKDATA4_OFFSET  (0x3420)
#define REG_CHIPID_OFFSET      (0xfffc)
#define SC_EXT_INT_MASK        (0x3800)
#define INT_PMU0_MASK_BIT       12
#define PMU0_INT_N_OFFSET      (0x8)
#define IOMG_010_BIT            0

#define TSENSOR_SUSPEND_FLAG   (0x5A5A5A5A)

#ifndef BIT
#define BIT(x) (0x1U << (x))
#endif

/*
 * LDO2 set voltage table
 * 000:1725 reg value = 0,represent voltage = 1725mv
 * 001:1750 reg value = 1,represent voltage = 1750mv
 * 010:1775 reg value = 2,represent voltage = 1775mv
 * 011:1800 reg value = 3,represent voltage = 1800mv
 * 100:1825 reg value = 4,represent voltage = 1825mv
 * 101:1850 reg value = 5,represent voltage = 1850mv
 * 110:1875 reg value = 6,represent voltage = 1875mv
 * 111:1900 reg value = 7,represent voltage = 1900mv
 * unit:mv
 */
static const u32 g_ldo2_volt_value_table[8] = { 1725, 1750, 1775, 1800, 1825, 1850, 1875, 1900 };

/*
 * LDO9 set voltage table
 * 000:1750 reg value = 0,represent voltage = 1750mv
 * 001:1800 reg value = 1,represent voltage = 1800mv
 * 010:1825 reg value = 2,represent voltage = 1825mv
 * 011:2800 reg value = 3,represent voltage = 2800mv
 * 100:2850 reg value = 4,represent voltage = 2850mv
 * 101:2950 reg value = 5,represent voltage = 2950mv
 * 110:3000 reg value = 6,represent voltage = 3000mv
 * 111:3300 reg value = 7,represent voltage = 3300mv
 * unit:mv
 */
static const u32 g_ldo9_volt_value_table[8] = { 1750, 1800, 1825, 2800, 2850, 2950, 3000, 3300 };

/*
 * LDO16 set voltage table
 * 000:1800 reg value = 0,represent voltage = 1800mv
 * 001:1850 reg value = 1,represent voltage = 1850mv
 * 010:2400 reg value = 2,represent voltage = 2400mv
 * 011:2600 reg value = 3,represent voltage = 2600mv
 * 100:2700 reg value = 4,represent voltage = 2700mv
 * 101:2850 reg value = 5,represent voltage = 2850mv
 * 110:2950 reg value = 6,represent voltage = 2950mv
 * 111:3000 reg value = 7,represent voltage = 3000mv
 * unit:mv
 */
static const u32 g_ldo16_volt_value_table[8] = { 1800, 1850, 2400, 2600, 2700, 2850, 2950, 3000 };

/* pmic description info */
static struct hisi_pmic *g_pmic = NULL;
/* system control regisiter base address */
static void __iomem *g_sysctrl_base = NULL;

/* lists compatible device info */
static const struct of_device_id of_hisi_pmic_match_tbl[] = {
	{
		.compatible = "hisilicon-hisi-pmic-spmi",
	},
	{/* end */}
};

/*
 * Function    : get_volt_value_index
 * Input       : u32 *volt_table   set voltage table array
 *               u32 volt_value    voltage to be set
 * Output      : set voltage index
 *               return -1 represent fail
 * Description : get voltage index from the set voltage table
 */
static s32 get_volt_value_index(const u32 *volt_table, const u32 table_len, const u32 volt_value)
{
	u32 index = 0;

	if ((volt_table == NULL) || (table_len == 0))
		return RET_FAIL;

	/* foreach voltage set table(table len is 8), find proper index */
	for (index = 0; index < table_len; index++) {
		if (volt_table[index] == volt_value)
			return (s32)index;
	}

	return RET_FAIL;
}

/*
 * Function    : hisi_pmic_read_reg
 * Input       : s32 reg           regisiter to be read
 * Output      : regisiter value
 *               return -1 represent fail
 * Description : read regisiter value from pmic base on spmi bus
 *               The PMIC register is only 8-bit.
 *               Hisilicon SoC use hardware to map PMIC register into SoC mapping.
 *               At here, we are accessing SoC register with 32-bit.
 */
u32 hisi_pmic_read_reg(const s32 reg)
{
	u32 ret = 0;
	u8 read_value = 0;
	struct spmi_device *pdev = NULL;

	if ((g_pmic == NULL) || (g_pmic->dev == NULL))
		return UINT_MAX;

	pdev = to_spmi_device(g_pmic->dev);
	if ((pdev == NULL) || (pdev->ctrl == NULL))
		return UINT_MAX;

	ret = spmi_ext_register_readl(pdev->ctrl, pdev->sid, reg, (u8 *)&read_value, 1);
	if (ret)
		return UINT_MAX;

	return (u32)read_value;
}
EXPORT_SYMBOL(hisi_pmic_read_reg);

/*
 * Function    : hisi_second_pmic_read_reg
 * Input       : s32 sid       second pmu id
 *		   : s32 reg           regisiter to be read
 * Output      : regisiter value
 *               return < 0 represent fail
 * Description : read regisiter value from pmic base on spmi bus
 *               The PMIC register is only 8-bit.
 *               Hisilicon SoC use hardware to map PMIC register into SoC mapping.
 *               At here, we are accessing SoC register with 32-bit.
 */
u32 hisi_second_pmic_read_reg(const s32 sid, const s32 reg)
{
	u32 ret = 0;
	u8 read_value = 0;
	struct spmi_device *pdev = NULL;

	if ((g_pmic == NULL) || (g_pmic->dev == NULL))
		return UINT_MAX;

	pdev = to_spmi_device(g_pmic->dev);
	if ((pdev == NULL) || (pdev->ctrl == NULL))
		return UINT_MAX;

	ret = spmi_ext_register_readl(pdev->ctrl, sid, reg, (u8 *)&read_value, 1);
	if (ret)
		return UINT_MAX;

	return (u32)read_value;
}
EXPORT_SYMBOL(hisi_second_pmic_read_reg);

/*
 * Function    : hisi_pmic_write_reg
 * Input       : s32 reg           regisiter to be write
 *               u32 val           write value
 * Output      : return write result
 *               return -1 represent fail
 * Description : write value to pmic regisiter base on spmi bus
 */
s32 hisi_pmic_write_reg(const s32 reg, const u32 val)
{
	s32 ret = 0;
	struct spmi_device *pdev = NULL;

	if ((g_pmic == NULL) || (g_pmic->dev == NULL))
		return RET_FAIL;

	pdev = to_spmi_device(g_pmic->dev);
	if ((pdev == NULL) || (pdev->ctrl == NULL))
		return RET_FAIL;

	ret = spmi_ext_register_writel(pdev->ctrl, pdev->sid, reg, (u8 *)&val, 1);
	if (ret)
		return RET_FAIL;

	return RET_SUCC;
}
EXPORT_SYMBOL(hisi_pmic_write_reg);

/*
 * Function    : pmu_ldo2_enable
 * Input       : void
 * Output      : return ldo2 enable result
 *               return -1 represent fail
 * Description : enable pmu ldo2 module
 *               ldo2 mainly used to provide efuse power
 */
s32 pmu_ldo2_enable(void)
{
	s32 ret = 0;
	u32 reg_val = 0;

	reg_val = hisi_pmic_read_reg(LDO2_ONOFF1);
	if (reg_val == UINT_MAX)
		return RET_FAIL;

	/* bit 1 as ldo ONOFF register status, bit 1 == 0x2, ldo enabled */
	if ((reg_val & BIT(1)) == 0x2) {
		/* if ldo 2 is enabled, do nothing */
	} else {
		/* set ldo2 to a fixed voltage(1.8v). */
		ret = pmu_ldo2_set_voltage(1800);
		if (ret < 0)
			return RET_FAIL;

		/* enable ldo2. */
		reg_val |= BIT(0);
		ret = hisi_pmic_write_reg(LDO2_ONOFF1, reg_val);
		if (ret < 0) {
			pr_err("pmic ldo2 enable failed!\n");
			return RET_FAIL;
		}
	}

	pr_info("pmic ldo2 enable voltage:0x%x.buck status:0x%x.\n",
			hisi_pmic_read_reg(LDO2_VSET),
			hisi_pmic_read_reg(LDO2_ONOFF1));
	return RET_SUCC;
}
EXPORT_SYMBOL(pmu_ldo2_enable);

/*
 * Function    : pmu_ldo2_disable
 * Input       : void
 * Output      : return ldo2 disable result
 *               return -1 represent fail
 * Description : disable pmu ldo2 module
 */
s32 pmu_ldo2_disable(void)
{
	s32 ret = 0;
	u32 reg_val = 0;

	reg_val = hisi_pmic_read_reg(LDO2_ONOFF1);
	if (reg_val == UINT_MAX)
		return RET_FAIL;

	/* bit 1 as ldo ONOFF register status, bit 1 == 0, ldo disabled */
	if ((reg_val & BIT(1)) == 0) {
		/* if ldo 2 is disabled, do nothing */
	} else {
		/* disable ldo2. */
		reg_val &= ~(BIT(0));
		ret = hisi_pmic_write_reg(LDO2_ONOFF1, reg_val);
		if (ret < 0) {
			pr_err("pmic ldo2 disable failed!\n");
			return RET_FAIL;
		}
	}

	pr_info("pmic ldo2 disable voltage:0x%x.buck status:0x%x.\n",
			hisi_pmic_read_reg(LDO2_VSET),
			hisi_pmic_read_reg(LDO2_ONOFF1));
	return RET_SUCC;
}
EXPORT_SYMBOL(pmu_ldo2_disable);

/*
 * Function    : pmu_ldo2_set_voltage
 * Input       : u32 volt_value       set voltage value
 * Output      : return set ldo2 volt result
 *               return -1 represent fail
 * Description : set pmu ldo2 voltage
 *               set value must be found in the g_ldo2_volt_value_table
 */
s32 pmu_ldo2_set_voltage(const u32 volt_value)
{
	s32 ret = 0;
	s32 volt_index = 0;
	u32 reg_val = 0;

	volt_index = get_volt_value_index(g_ldo2_volt_value_table, ARRAY_SIZE(g_ldo2_volt_value_table), volt_value);
	if (volt_index < 0)
		return RET_FAIL;

	reg_val = hisi_pmic_read_reg(LDO2_VSET);
	if (reg_val == UINT_MAX)
		return RET_FAIL;

	/* The low 3 bits of voltage register are the desired voltage value */
	if ((reg_val & 0x7) == volt_index) {
		/* if setting value = current voltage, do nothing */
	} else {
		/* set ldo2 voltage. */
		ret = hisi_pmic_write_reg(LDO2_VSET, volt_index);
		if (ret < 0) {
			pr_err("pmic ldo2 set volt failed!\n");
			return RET_FAIL;
		}
	}

	return RET_SUCC;
}
EXPORT_SYMBOL(pmu_ldo2_set_voltage);

/*
 * Function    : pmu_ldo16_enable
 * Input       : void
 * Output      : return ldo16 enable result
 *               return -1 represent fail
 * Description : enable pmu ldo16 module
 *               ldo16 mainly used to provide SD power
 */
s32 pmu_ldo16_enable(void)
{
	s32 ret = 0;
	u32 reg_val = 0;

	reg_val = hisi_pmic_read_reg(LDO16_ONOFF_ECO);
	if (reg_val == UINT_MAX)
		return RET_FAIL;

	/* bit 1 as ldo ONOFF register status, bit 1 == 0x2, ldo enabled */
	if ((reg_val & BIT(1)) == 0x2) {
		/* if ldo 16 is enabled, do nothing */
	} else {
		/* enable ldo16. */
		reg_val |= BIT(0);
		ret = hisi_pmic_write_reg(LDO16_ONOFF_ECO, reg_val);
		if (ret < 0) {
			pr_err("pmic ldo16 enable failed!\n");
			return RET_FAIL;
		}
	}

	pr_info("pmic ldo16 enable voltage:0x%x.buck status:0x%x.\n",
			hisi_pmic_read_reg(LDO16_VSET),
			hisi_pmic_read_reg(LDO16_ONOFF_ECO));
	return RET_SUCC;
}
EXPORT_SYMBOL(pmu_ldo16_enable);

s32 pmu_ldo16_disable(void)
{
	s32 ret = 0;
	u32 reg_val = 0;

	reg_val = hisi_pmic_read_reg(LDO16_ONOFF_ECO);
	if (reg_val == UINT_MAX)
		return RET_FAIL;

	/* bit 1 as ldo ONOFF register status, bit 1 == 0, ldo disabled */
	if ((reg_val & BIT(1)) == 0) {
		/* if ldo 16 is disabled, do nothing */
	} else {
		/* disable ldo16. */
		reg_val &= ~(BIT(0));
		ret = hisi_pmic_write_reg(LDO16_ONOFF_ECO, reg_val);
		if (ret < 0) {
			pr_err("pmic ldo16 disable failed!\n");
			return RET_FAIL;
		}
	}

	pr_info("pmic ldo16 disable voltage:0x%x.buck status:0x%x.\n",
			hisi_pmic_read_reg(LDO16_VSET),
			hisi_pmic_read_reg(LDO16_ONOFF_ECO));
	return RET_SUCC;
}
EXPORT_SYMBOL(pmu_ldo16_disable);

s32 pmu_ldo16_set_voltage(const u32 volt_value)
{
	s32 ret = 0;
	u32 reg_val = 0;
	s32 volt_index = 0;

	volt_index = get_volt_value_index(g_ldo16_volt_value_table, ARRAY_SIZE(g_ldo16_volt_value_table), volt_value);
	if (volt_index < 0)
		return RET_FAIL;

	reg_val = hisi_pmic_read_reg(LDO16_VSET);
	if (reg_val == UINT_MAX)
		return RET_FAIL;

	/* The low 3 bits of voltage register are the desired voltage value */
	if ((reg_val & 0x7) == volt_index) {
		/* if setting value = current voltage, do nothing */
	} else {
		/* set ldo16 voltage. */
		ret = hisi_pmic_write_reg(LDO16_VSET, volt_index);
		if (ret < 0) {
			pr_err("pmic ldo16 set volt failed!\n");
			return RET_FAIL;
		}
	}

	pr_info("pmic ldo16 set voltage:0x%x.buck status:0x%x.\n",
			hisi_pmic_read_reg(LDO16_VSET),
			hisi_pmic_read_reg(LDO16_ONOFF_ECO));
	return RET_SUCC;
}
EXPORT_SYMBOL(pmu_ldo16_set_voltage);

/*
 * Function    : pmu_ldo9_enable
 * Input       : void
 * Output      : return ldo9 enable result
 *               return -1 represent fail
 * Description : enable pmu ldo9 module
 *               ldo9 mainly used to provide SDIO power
 */
s32 pmu_ldo9_enable(void)
{
	s32 ret = 0;
	u32 reg_val = 0;

	reg_val = hisi_pmic_read_reg(LDO9_ONOFF_ECO);
	if (reg_val == UINT_MAX)
		return RET_FAIL;

	/* bit 1 as ldo ONOFF register status, bit 1 == 0x2, ldo enabled */
	if ((reg_val & BIT(1)) == 0x2) {
		/* if ldo 9 is enabled, do nothing */
	} else {
		/* enable ldo9. */
		reg_val |= BIT(0);
		ret = hisi_pmic_write_reg(LDO9_ONOFF_ECO, reg_val);
		if (ret < 0) {
			pr_err("pmic ldo9 enable failed!\n");
			return RET_FAIL;
		}
	}
	pr_info("pmic ldo9 enable voltage:0x%x.buck status:0x%x.\n",
			hisi_pmic_read_reg(LDO9_VSET),
			hisi_pmic_read_reg(LDO9_ONOFF_ECO));
	return RET_SUCC;
}
EXPORT_SYMBOL(pmu_ldo9_enable);

s32 pmu_ldo9_disable(void)
{
	s32 ret = 0;
	u32 reg_val = 0;

	reg_val = hisi_pmic_read_reg(LDO9_ONOFF_ECO);
	if (reg_val == UINT_MAX)
		return RET_FAIL;

	/* bit 1 as ldo ONOFF register status, bit 1 == 0, ldo disabled */
	if ((reg_val & BIT(1)) == 0) {
		/* if ldo 9 is disabled, do nothing */
	} else {
		/* disable ldo9. */
		reg_val &= ~(BIT(0));
		ret = hisi_pmic_write_reg(LDO9_ONOFF_ECO, reg_val);
		if (ret < 0) {
			pr_err("pmic ldo9 disable failed!\n");
			return RET_FAIL;
		}
	}
	pr_info("pmic ldo9 disable voltage:0x%x.buck status:0x%x.\n",
			hisi_pmic_read_reg(LDO9_VSET),
			hisi_pmic_read_reg(LDO9_ONOFF_ECO));
	return RET_SUCC;
}
EXPORT_SYMBOL(pmu_ldo9_disable);

s32 pmu_ldo9_set_voltage(const u32 volt_value)
{
	s32 ret = 0;
	u32 reg_val = 0;
	s32 volt_index = 0;

	volt_index = get_volt_value_index(g_ldo9_volt_value_table, ARRAY_SIZE(g_ldo9_volt_value_table), volt_value);
	if (volt_index < 0)
		return RET_FAIL;

	reg_val = hisi_pmic_read_reg(LDO9_VSET);
	if (reg_val == UINT_MAX)
		return RET_FAIL;

	/* The low 3 bits of voltage register are the desired voltage value */
	if ((reg_val & 0x7) == volt_index) {
		/* if setting value = current voltage, do nothing */
	} else {
		/* set ldo9 voltage. */
		ret = hisi_pmic_write_reg(LDO9_VSET, volt_index);
		if (ret < 0) {
			pr_err("pmic ldo9 set volt failed!\n");
			return RET_FAIL;
		}
	}

	pr_info("pmic ldo9 set voltage:0x%x.buck status:0x%x.\n",
			hisi_pmic_read_reg(LDO9_VSET),
			hisi_pmic_read_reg(LDO9_ONOFF_ECO));
	return RET_SUCC;
}
EXPORT_SYMBOL(pmu_ldo9_set_voltage);

/*
 * Function    : hisi_pmic_get_adc
 * Input       : u32 channel        get adc's channel
 * Output      : return adc value
 *               return -1 represent fail
 * Description : get adc value from pmu adc
 */
static s32 hisi_pmic_get_adc(const u32 channel)
{
	s32 ret = 0;
	u32 rd_data0 = 0;
	u32 rd_data1 = 0;

	/* ADC config register */
	ret = hisi_pmic_write_reg(HKADC_CTRL_ADDR, (u8)(channel));
	if (ret < 0)
		return RET_FAIL;

	if (channel == 0 || channel == 1 || channel == 2)
		udelay(503);
	else
		udelay(8);

	/* start converting */
	ret = hisi_pmic_write_reg(HKADC_START_ADDR, 1);
	if (ret < 0)
		return RET_FAIL;

	udelay(15);

	/* get data */
	rd_data0 = hisi_pmic_read_reg(HKADC_DATA0_ADDR);
	rd_data1 = hisi_pmic_read_reg(HKADC_DATA1_ADDR);
	if ((rd_data0 == UINT_MAX) || (rd_data1 == UINT_MAX))
		return RET_FAIL;

	/* close hkadc */
	ret = hisi_pmic_write_reg(HKADC_CTRL_ADDR, 0x80);
	if (ret < 0)
		return RET_FAIL;

	ret = (s32)(((rd_data1 << 4) & HKADC_VALUE_HIGH) |
		((rd_data0 >> 4) & HKADC_VALUE_LOW));
	pr_info("hisi_adc_get_value: data=%d\n", ret);

	return ret;
}

/*
 * Function    : hisi_adc_get_value
 * Input       : u32 channel        get adc's channel
 * Output      : return adc voltage, unit is mv
 *               return -1 represent fail
 * Description : get pmu adc voltage base on spmi bus
 *                   convert register value to voltage
 */
s32 hisi_adc_get_value(const u32 channel)
{
	s32 ret = 0;

	if (channel > HKADC_CHANNEL_MAX)
		return RET_FAIL;

	ret = hisi_pmic_get_adc(channel);
	if (ret < 0)
		return ret;

	/* convert ADC value to millivolts */
	ret = ((((u32)ret & HKADC_VALID_VALUE) * HKADC_VREF_1V8) / HKADC_ACCURACY);
	return ret;
}
EXPORT_SYMBOL(hisi_adc_get_value);

/*
 * Function    : hisi_irq_handler
 * Input       : u32 irq
 *               void *data
 * Output      : return IRQ_HANDLED
 * Description : pmu abnormal irq handler
 *               clear IRQ pending, mask abnormal event
 */
static irqreturn_t hisi_irq_handler(s32 irq, void *data)
{
	u32 pending = 0;
	s32 reg_count = 0;
	s32 ret = 0;
	static s32 irq_count = 0;

	/* clear IRQ pending, include two regisiter */
	for (reg_count = 0; reg_count < 2; reg_count++) {
		pending = hisi_pmic_read_reg((reg_count + PMU_IRQ0));
		pending &= HISI_MASK_FIELD;
		if (pending != 0) {
			ret = hisi_pmic_write_reg((reg_count + PMU_IRQ0), pending);
			if (ret != RET_SUCC)
				return IRQ_NONE;

			/* irq print up to MAX(10 times) to prevent brushing */
			if (irq_count <= 10) {
				pr_warn("pending[%d]=0x%x\n\r", reg_count, pending);
			} else {
				/* mask all interrupts from PMU */
				ret = hisi_pmic_write_reg(PMU_IRQ_MASK_0, 0xFF);
				if (ret != RET_SUCC)
					return IRQ_NONE;

				ret = hisi_pmic_write_reg(PMU_IRQ_MASK_1, 0xFF);
				if (ret != RET_SUCC)
					return IRQ_NONE;
			}
		}
	}
	irq_count++;

	return IRQ_HANDLED;
}

/*
 * Function    : get ASIC chip id
 * Input       : void
 * Output      : return IRQ_HANDLED
 * Description : pmu abnormal irq handler
 *               clear IRQ pending, mask abnormal event
 */
static u32 hisi_get_asic_chipid(void)
{
	struct device_node *np = NULL;
	void __iomem *cluster_base_addr = NULL;
	u32 chipid = UINT_MAX;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,lp_chipid");
	if (np == NULL)
		return UINT_MAX;

	cluster_base_addr = of_iomap(np, 0);
	if (cluster_base_addr == NULL) {
		of_node_put(np);
		return UINT_MAX;
	}
	of_node_put(np);

	chipid = readl(cluster_base_addr + REG_CHIPID_OFFSET);
	return chipid;
}

static s32 hisi_pmu_interrupt_init(struct hisi_pmic *pmic)
{
	struct device_node *np = NULL;
	void __iomem *peri_subctrl_base_addr = NULL;
	struct resource res;
	s32 ret = 0;
	u32 value = 0;

#ifdef CONFIG_IO_MUX_PMU0_INT
	void __iomem *IO_mux_base_addr = NULL;
#endif

	if (pmic == NULL)
		return -ENODEV;

	np = of_find_compatible_node(NULL, NULL, "hisilicon-hisi-pmic-spmi");
	if (np == NULL)
		return -ENODEV;

	ret = of_address_to_resource(np, 0, &res);
	if (ret)
		return -ENXIO;

	/* set ext interrput mask bit */
	peri_subctrl_base_addr = ioremap(res.start, resource_size(&res));
	if (peri_subctrl_base_addr == NULL) {
		of_node_put(np);
		return -ENODEV;
	}

	value = readl(peri_subctrl_base_addr + SC_EXT_INT_MASK);
	value &= ~(BIT(INT_PMU0_MASK_BIT));
	writel(value, peri_subctrl_base_addr + SC_EXT_INT_MASK);
	iounmap(peri_subctrl_base_addr);
	peri_subctrl_base_addr = NULL;

#ifdef CONFIG_IO_MUX_PMU0_INT
	ret = of_address_to_resource(np, 1, &res);
	if (ret)
		return -ENXIO;

	/* set GPIO5 as a pmu0 int */
	IO_mux_base_addr = ioremap(res.start, resource_size(&res));
	if (IO_mux_base_addr == NULL) {
		of_node_put(np);
		return -ENODEV;
	}

	value = readl(IO_mux_base_addr + PMU0_INT_N_OFFSET);
	value &= ~(BIT(IOMG_010_BIT));
	writel(value, IO_mux_base_addr + PMU0_INT_N_OFFSET);
	iounmap(IO_mux_base_addr);
	IO_mux_base_addr = NULL;
#endif

	/* set pmu mask bit */
	ret = hisi_pmic_write_reg(PMU_IRQ_MASK_0, 0xFE);
	if (ret != RET_SUCC)
		return -ENODEV;

	pr_info("pmu PMU_IRQ0 register value:0x%x\n", hisi_pmic_read_reg(PMU_IRQ0));
	/* clear pmic int pending */
	ret = hisi_pmic_write_reg(PMU_IRQ0, 0xFF);
	if (ret != RET_SUCC)
		return -ENODEV;

	ret = hisi_pmic_write_reg(PMU_IRQ1, 0xFF);
	if (ret != RET_SUCC)
		return -ENODEV;

	pmic->irq = irq_of_parse_and_map(np, 0);
	if (pmic->irq <= 0) {
		of_node_put(np);
		return -ENODEV;
	}
	of_node_put(np);

	return RET_SUCC;
}

static s32 hisi_pmic_probe(struct spmi_device *pdev)
{
	s32 ret = 0;
	struct hisi_pmic *pmic = NULL;
	struct device_node *np = NULL;
	struct device *dev = NULL;

	if (pdev == NULL)
		return -ENODEV;

	/*
	 * pmu driver only support asic
	 * environment list table
	 *     asic: 0x00000000
	 *     fpga: 0x0000xxxx
	 *     emu:  0x00010000
	 *     esl:  0x00020000
	 */
	if (hisi_get_asic_chipid() != 0x0) {
		pr_err("pmu driver only support asic.\n");
		return -ENODEV;
	}

	/* get sysctrl base address */
	np = of_find_compatible_node(NULL, NULL, "hisilicon,lp_sysctrl");
	if (np == NULL)
		return -ENODEV;

	/* get the system control register address */
	g_sysctrl_base = of_iomap(np, 0);
	if (g_sysctrl_base == NULL) {
		of_node_put(np);
		return -ENODEV;
	}
	of_node_put(np);

	dev= &pdev->dev;
	pmic = devm_kmalloc(dev, sizeof(struct hisi_pmic), GFP_KERNEL | __GFP_ZERO);
	if (pmic == NULL) {
		iounmap(g_sysctrl_base);
		g_sysctrl_base = NULL;
		return -ENOMEM;
	}

	/* get and enable clk request */
	spin_lock_init(&pmic->lock);
	pmic->dev = dev;
	g_pmic = pmic;

	ret = hisi_pmu_interrupt_init(pmic);
	if (ret < 0) {
		devm_kfree(&pdev->dev, pmic);
		iounmap(g_sysctrl_base);
		g_pmic = NULL;
		g_sysctrl_base = NULL;
		return ret;
	}

	ret = request_threaded_irq(pmic->irq, hisi_irq_handler, NULL,
							IRQF_TRIGGER_HIGH | IRQF_NO_SUSPEND,
							"pmic", pmic);
	if (ret < 0) {
		pr_err("could not claim pmic %d\n", ret);
		devm_kfree(&pdev->dev, pmic);
		iounmap(g_sysctrl_base);
		g_pmic = NULL;
		g_sysctrl_base = NULL;
		return -ENODEV;
	}

	return RET_SUCC;
}

static s32 hisi_pmic_remove(struct spmi_device *pdev)
{
	struct hisi_pmic *pmic = NULL;

	if (pdev == NULL)
		return -ENODEV;

	pmic = dev_get_drvdata(&pdev->dev);
	if (pmic == NULL)
		return -ENOMEM;

	(void)free_irq(pmic->irq, (void*)0);
	(void)devm_kfree(&pdev->dev, pmic);
	(void)iounmap(g_sysctrl_base);
	g_pmic = NULL;
	g_sysctrl_base = NULL;

	return RET_SUCC;
}

static s32 hisi_pmic_suspend(struct spmi_device *pdev, pm_message_t state)
{
	if (g_sysctrl_base == NULL)
		return -ENOMEM;

	/* set scbakdata4 register = !0 to stop reading AICORE tsensor */
	writel(TSENSOR_SUSPEND_FLAG, g_sysctrl_base + REG_SCBAKDATA4_OFFSET);
	pr_info("pmic suspend flag = 0x%x\n",
		readl(g_sysctrl_base + REG_SCBAKDATA4_OFFSET));

	return RET_SUCC;
}

static s32 hisi_pmic_resume(struct spmi_device *pdev)
{
	if (g_sysctrl_base == NULL)
		return -ENOMEM;

	/* clear scbakdata4 register to begin reading AICORE tsensor */
	writel(0, g_sysctrl_base + REG_SCBAKDATA4_OFFSET);
	pr_info("pmic suspend flag = 0x%x\n",
		readl(g_sysctrl_base + REG_SCBAKDATA4_OFFSET));

	return RET_SUCC;
}

static struct spmi_driver hisi_pmic_driver = {
	.driver = {
		.name = "hisi_pmic",
		.owner = THIS_MODULE,
		.of_match_table = of_hisi_pmic_match_tbl,
	},
	.probe = hisi_pmic_probe,
	.remove = hisi_pmic_remove,
	.suspend = hisi_pmic_suspend,
	.resume = hisi_pmic_resume,
};

static s32 __init hisi_pmic_init(void)
{
	return spmi_driver_register(&hisi_pmic_driver);
}

static void __exit hisi_pmic_exit(void)
{
	spmi_driver_unregister(&hisi_pmic_driver);
}

subsys_initcall_sync(hisi_pmic_init);
module_exit(hisi_pmic_exit);

MODULE_DESCRIPTION("PMIC driver");
MODULE_LICENSE("GPL v2");
#else
void hisi_pmic_init(void)
{
	return 0;
}
#endif
