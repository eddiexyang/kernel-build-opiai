/*
 * Device driver for regulators in HISI PMIC IC
 *
 * Copyright (c) 2019 HiSilicon Technologies Co., Ltd.
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
#include <linux/err.h>

#define MAX_LDO_INDEX             20
#define RET_SUCC                  0
#define LDO_ENABLE_MASK           0x02
#define LDO_ENABLE_OFFSET         0x01
#define VOLTAGE_INIT_VALUE        0
#define SECOND_PMU_PRE            0x02
#define SECOND_PMU_BACK           0x0B
#define MAIN_PMU_DIE_ID_BASE      0x288
#define MAIN_PMU_DIE_ID_LEN       17
#define MAIN_PMU_DIE_ID_INDEX_MAX (MAIN_PMU_DIE_ID_LEN - 1)
#define SECOND_PMU_DIE_ID_BASE    0xF6
#define SECOND_PMU_DIE_ID_LEN     8
#define PMU_DIE_ID_MASK           0xFF

#define MAIN_PMU_BUCK_2           2
#define MAIN_PMU_BUCK_8           8
#define SECOND_PMU_BUCK_0         0
#define SECOND_PMU_BUCK_3         3

/*
 * hisi pmic buck information structure. it can be used to
 * caculate the buck voltage.
 *
 * buck_voltage = base_mv + Value(buck_volt_addr) * step_mv
 *
 * buck_volt_addr : the register address which control
 *					the buck voltage
 * buck_enable_addr : buck enable register address
 * buck_enabled_value : enabled mask value
 * base_mv : the buck base voltage
 * step_mv : the voltage which every step represent
 * max_step : max step count
 */
struct pmic_buck_info {
	u32 buck_volt_addr;
	u32 buck_enable_addr;
	u32 buck_enabled_value;
	u32 base_mv;
	u32 step_mv;
	u32 max_step;
};

const struct pmic_buck_info main_buck_info[9] = {
	{0x41, 0x08, 0x02, 500, 12, 64},
	{0x43, 0x09, 0x02, 1000, 10, 32},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x46, 0x0B, 0x02, 1700, 15, 32},
	{0x48, 0x0C, 0x02, 500, 12, 64},
	{0x49, 0x0D, 0x02, 500, 12, 64},
	{0x4A, 0x0E, 0x02, 500, 12, 64},
	{0x4B, 0x0F, 0x02, 500, 10, 64},
	{0x4C, 0x10, 0x02, 1700, 15, 32},
};

const struct pmic_buck_info second_buck_info[4] = {
	{0xAF, 0xAB, 0x01, 300, 5, 191},
	{0x0, 0x0, 0x0, 0, 0, 0},
	{0x0, 0x0, 0x0, 0, 0, 0},
	{0xB3, 0xAD, 0x01, 300, 5, 191},
};

static const u32 g_ldo0_volt_value_table[8] = {
	550, 600, 650, 700, 750, 775, 800, 850
};

static const u32 g_ldo5_volt_value_table[16] = {
	1600, 1620, 1640, 1660, 1680, 1700, 1720, 1800,
	1820, 1880, 1900, 1920, 1940, 1960, 1980, 2000
};

static const u32 g_ldo8_volt_value_table[16] = {
	1500, 1550, 1600, 1650, 1700, 1725, 1750, 1775,
	1800, 1825, 1850, 1875, 1900, 1925, 1950, 2000
};

static const u32 g_ldo15_volt_value_table[8] = {
	1800, 1850, 2400, 2600, 2700, 2850, 2950, 3000
};

static const u32 g_ldo18_volt_value_table[16] = {
	1500, 1550, 1600, 1650, 1700, 1725, 1750, 1775,
	1800, 1825, 1850, 1875, 1900, 1925, 1950, 2000
};

/*
 * hisi pmic LDO information structure. it can be used to
 * inquire the LDO voltage.
 *
 * reg : the register address can control the LDO voltage
 * mask : reg value mask value
 * length : the below table length
 * table : which be used to inquire the voltage
 */
struct ldo_info {
	u32 reg;
	u32 enable;
	u8  mask;
	u8  length;
	const u32* table;
};

const struct ldo_info ldo0_info = {
	.reg = 0x4E,
	.enable = 0x11,
	.mask = 0x7,
	.length = 8,
	.table = g_ldo0_volt_value_table
};

const struct ldo_info ldo5_info = {
	.reg = 0x53,
	.enable = 0x18,
	.mask = 0xf,
	.length = 16,
	.table = g_ldo5_volt_value_table
};

const struct ldo_info ldo8_info = {
	.reg = 0x56,
	.enable = 0x1B,
	.mask = 0xf,
	.length = 16,
	.table = g_ldo8_volt_value_table
};

const struct ldo_info ldo15_info = {
	.reg = 0x5C,
	.enable = 0x21,
	.mask = 0x7,
	.length = 8,
	.table = g_ldo15_volt_value_table
};

const struct ldo_info ldo18_info = {
	.reg = 0x5F,
	.enable = 0x24,
	.mask = 0xf,
	.length = 16,
	.table = g_ldo18_volt_value_table
};

static const struct ldo_info *ldo_table[MAX_LDO_INDEX] = {
	&ldo0_info, NULL, NULL, NULL, NULL,
	&ldo5_info, NULL, NULL, &ldo8_info, NULL,
	NULL, NULL, NULL, NULL, NULL,
	&ldo15_info, NULL, NULL, &ldo18_info, NULL
};

/*
 * Function : get main pmu per channel LDO voltage
 * Input    : channel    LDO id
 * Output   : volt_mv    voltage
 * return   : 0 represent success; negative value represent fail
 */
s32 get_main_pmu_ldo_volt(u32 channel, u32 *volt_mv)
{
	const struct ldo_info *ldo = NULL;
	u32 enable_pre = 0;
	u32 enable_back = 0;
	u32 value = 0;

	if (volt_mv == NULL)
		return -EFAULT;

	/* clear the voltage value */
	*volt_mv = VOLTAGE_INIT_VALUE;

	if (channel >= MAX_LDO_INDEX || ldo_table[channel] == NULL)
		return -EFAULT;

	ldo = ldo_table[channel];

	enable_pre = hisi_pmic_read_reg(ldo->enable);
	/*
	 * there is another register can control LDO0 which is
	 * beside the defined register which offset is one.
	 */
	if (channel == 0)
		enable_back = hisi_pmic_read_reg(ldo->enable + LDO_ENABLE_OFFSET);

	if ((enable_pre == UINT_MAX) || (enable_back == UINT_MAX))
		return -EINVAL;

	/*
	 * bit1 is the enable status. if the both control register are not enabeld,
	 * the output value is zero even though you have configure the voltage.
	 */
	if (channel == 0) {
		if (((enable_pre & LDO_ENABLE_MASK) == 0) &&
			((enable_back & LDO_ENABLE_MASK) == 0))
			return RET_SUCC;
	} else {
		if ((enable_pre & LDO_ENABLE_MASK) == 0)
			return RET_SUCC;
	}

	value = hisi_pmic_read_reg(ldo->reg);
	if (value == UINT_MAX)
		return -EINVAL;

	/* the mask can guarantee the value of array not overflow */
	*volt_mv = ldo->table[value & ldo->mask];
	return RET_SUCC;
}
EXPORT_SYMBOL(get_main_pmu_ldo_volt);

/*
 * Function : get main pmu per channel buck voltage
 * Input    : channel    buck id
 * Output   : volt_mv    voltage
 * return   : 0 represent success; negative value represent fail
 */
s32 get_main_pmu_buck_volt(u32 channel, u32 *volt_mv)
{
	u32 buck_volt_addr = 0;
	u32 buck_enable_addr = 0;
	u32 reg_value = 0;

	if (volt_mv == NULL)
		return -EFAULT;

	/* clear the voltage value */
	*volt_mv = VOLTAGE_INIT_VALUE;

	/* from the schematic, the channel is between 0 to 8, except buck2 */
	if ((channel == MAIN_PMU_BUCK_2) || (channel > MAIN_PMU_BUCK_8))
		return -EFAULT;

	buck_enable_addr = main_buck_info[channel].buck_enable_addr;
	reg_value = hisi_pmic_read_reg(buck_enable_addr);
	/*
	 * if buck not enabled,the real buck voltage should be zero, even though
	 * we have configured the voltage register.
	 * so we return success directly.
	 */
	if ((reg_value & main_buck_info[channel].buck_enabled_value) == 0) {
		return RET_SUCC;
	}
	else if (reg_value == UINT_MAX) {
		pr_err("buck%u enable register value is 0x%x, buck_enabled_value is 0x%x\n",
				channel, reg_value, main_buck_info[channel].buck_enabled_value);
		return -EINVAL;
	}

	buck_volt_addr = main_buck_info[channel].buck_volt_addr;
	reg_value = hisi_pmic_read_reg(buck_volt_addr);
	if (reg_value >= main_buck_info[channel].max_step) {
		pr_err("buck%u voltage register value is 0x%x\n", channel, reg_value);
		return -EINVAL;
	}

	/*
	 * the voltage value = base voltage value + step_leng * step_count.
	 * the max_step can guarantee volt_mv not overflow.
	 */
	*volt_mv = main_buck_info[channel].base_mv +
				main_buck_info[channel].step_mv * reg_value;

	return RET_SUCC;
}
EXPORT_SYMBOL(get_main_pmu_buck_volt);

/*
 * Function : get second pmu (6422) per channel buck voltage
 * Input    : device_id  second pmu id
 * Input    : channel    buck id
 * Output   : volt_mv    voltage
 * return   : 0 represent success; negative value represent fail
 */
s32 get_second_pmu_buck_volt(u32 device_id, u32 channel, u32 *volt_mv)
{
	u32 buck_volt_addr = 0;
	u32 buck_enable_addr = 0;
	u32 reg_value = 0;

	if (volt_mv == NULL)
		return -EFAULT;

	/* clear the voltage value */
	*volt_mv = VOLTAGE_INIT_VALUE;

	/* there has two salve PMU, which id are 0x02 and 0x0B respectively */
	if (((device_id != SECOND_PMU_PRE) && (device_id != SECOND_PMU_BACK)) ||
		((channel != SECOND_PMU_BUCK_0) && (channel != SECOND_PMU_BUCK_3)))
		return -EFAULT;

	buck_enable_addr = second_buck_info[channel].buck_enable_addr;
	reg_value = hisi_second_pmic_read_reg(device_id, buck_enable_addr);
	/*
	 * if buck not enabled,the real buck voltage should be zero, even though
	 * we have configured the voltage register.
	 * so we return success directly.
	 */
	if ((reg_value & second_buck_info[channel].buck_enabled_value) == 0) {
		return RET_SUCC;
	}
	else if (reg_value == UINT_MAX) {
		pr_err("pmu_id %u, buck%u enable value is 0x%x, buck_enabled_value is 0x%x\n",
				device_id, channel, reg_value,
				second_buck_info[channel].buck_enabled_value);
		return -EINVAL;
	}

	buck_volt_addr = second_buck_info[channel].buck_volt_addr;
	reg_value = hisi_second_pmic_read_reg(device_id, buck_volt_addr);
	if (reg_value >= second_buck_info[channel].max_step) {
		pr_err("pmu_id %u, buck%u voltage register value is 0x%x\n",
				device_id, channel, reg_value);
		return -EINVAL;
	}

	/* the voltage value = base voltage value + step_leng * step_count */
	*volt_mv = second_buck_info[channel].base_mv +
				second_buck_info[channel].step_mv * reg_value;

	return RET_SUCC;
}
EXPORT_SYMBOL(get_second_pmu_buck_volt);

/*
 * Function : get main pmu DIE id
 * Input    : len      length of die_id buffer, at least be 17
 * Output   : die_id   main pmu DIE id buffer
 * return   : 0 represent success; negative value represent fail
 */
s32 get_main_pmu_die_id(u8 *die_id, u8 len)
{
	u8 i = 0;
	u32 result = 0;

	if ((die_id == NULL) || (len < MAIN_PMU_DIE_ID_LEN))
		return -EFAULT;

	for (i = 0; i < MAIN_PMU_DIE_ID_LEN; i++) {
		result = hisi_pmic_read_reg(MAIN_PMU_DIE_ID_BASE + i);
		if (result == UINT_MAX)
			return -EINVAL;
		else
			die_id[MAIN_PMU_DIE_ID_INDEX_MAX - i] = result & PMU_DIE_ID_MASK; /* reverse order */
	}

	return RET_SUCC;
}
EXPORT_SYMBOL(get_main_pmu_die_id);

/*
 * Function : get second pmu DIE id
 * Input    : device_id  second pmu id
 * Input    : len        length of die_id buffer, at least be 8
 * Output   : die_id     second pmu DIE id buffer
 * return   : 0 represent success; negative value represent fail
 */
s32 get_second_pmu_die_id(u32 device_id, u8 *die_id, u8 len)
{
	u8 i = 0;
	u32 result = 0;

	/* there has two salve PMU, which id are 0x02 and 0x0B respectively */
	if (((device_id != SECOND_PMU_PRE) && (device_id != SECOND_PMU_BACK)) ||
		(die_id == NULL) || (len < SECOND_PMU_DIE_ID_LEN))
		return -EFAULT;

	for (i = 0; i < SECOND_PMU_DIE_ID_LEN; i++) {
		result = hisi_second_pmic_read_reg(device_id, SECOND_PMU_DIE_ID_BASE + i);
		if (result == UINT_MAX)
			return -EINVAL;
		else
			die_id[i] = result & PMU_DIE_ID_MASK;
	}

	return RET_SUCC;
}
EXPORT_SYMBOL(get_second_pmu_die_id);

#else
int get_second_pmu_die_id(void)
{
	return 0;
}
#endif
