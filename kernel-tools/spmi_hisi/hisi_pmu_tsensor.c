/*
 * Device driver for NTC in HISI PMIC IC
 *
 * Copyright (C) 2012 Hisilicon, Inc.
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
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/math64.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/spinlock.h>

#define RET_SUCC                   0
#define RET_FAIL                  (-1)
/* ntc in pmu adc channel is 3\4\5\6,start channel is 3 */
#define PMU_TSENSOR_START_CHANNEL  3
#define PMU_TSENSOR_MAX_CHANNEL    6

#define NTC_DEV_NOT_INITIAL        0
#define NTC_DEV_HAVE_INITIAL       1
#define TEMPERATURE_COEFF          1000
#define MICRO_VOLT                 1000
#define BISECTION_DIVIDE           2

static DEFINE_SPINLOCK(ntc_lock);

struct ntc_compensation {
	s32 temp_c;         /* ntc  temperature, unit is Celsius */
	u32 ohm;            /* ntc  resistance, unit is ohm */
};

struct ntc_device_id {
	s8 name[PLATFORM_NAME_SIZE];
	/* ntc temperature&resistance compensation table */
	const struct ntc_compensation *table;
	/* ntc temperature&resistance compensation table len */
	u32 table_len;
};

/*
 * [pullup_uV]
 *     |    |
 *    [TH]  $ (pullup_ohm)
 *     |    |
 *     +----+-----------------------[read_uV]
 *     |
 *     $ (pulldown_ohm)
 *     |
 *    --- (ground)
 */
struct ntc_device {
	u32 pullup_uv; /* ntc pullup voltage, unit is uv */
	u32 pullup_ohm; /* ntc pullup resistance, unit is ohm */
	u32 pulldown_ohm; /* ntc pulldown resistance, unit is ohm */
	/* ntc temperature&resistance compensation table */
	const struct ntc_compensation *comp;
	u32 n_comp; /* ntc temperature&resistance compensation table len */
	const s8 *name; /* ntc sensor name */
	/* ntc sensor device initial flag, default value = NTC_DEV_NOT_INITIAL */
	u32 initial_flag;
};

/*
 * A compensation table should be sorted by the values of .ohm
 * in descending order.
 * The following compensation tables are from the specification of Murata NTC
 * Thermistors Datasheet
 */
static const struct ntc_compensation ncp_XXxh103[] = { \
	{ .temp_c = -40, .ohm = 195652 },
	{ .temp_c = -39, .ohm = 184917 },
	{ .temp_c = -38, .ohm = 174845 },
	{ .temp_c = -37, .ohm = 165391 },
	{ .temp_c = -36, .ohm = 156513 },
	{ .temp_c = -35, .ohm = 148171 },
	{ .temp_c = -34, .ohm = 140330 },
	{ .temp_c = -33, .ohm = 132958 },
	{ .temp_c = -32, .ohm = 126022 },
	{ .temp_c = -31, .ohm = 119494 },
	{ .temp_c = -30, .ohm = 113347 },
	{ .temp_c = -29, .ohm = 107565 },
	{ .temp_c = -28, .ohm = 102116 },
	{ .temp_c = -27, .ohm = 96978 },
	{ .temp_c = -26, .ohm = 92132 },
	{ .temp_c = -25, .ohm = 87559 },
	{ .temp_c = -24, .ohm = 83242 },
	{ .temp_c = -23, .ohm = 79166 },
	{ .temp_c = -22, .ohm = 75316 },
	{ .temp_c = -21, .ohm = 71677 },
	{ .temp_c = -20, .ohm = 68237 },
	{ .temp_c = -19, .ohm = 64991 },
	{ .temp_c = -18, .ohm = 61919 },
	{ .temp_c = -17, .ohm = 59011 },
	{ .temp_c = -16, .ohm = 56258 },
	{ .temp_c = -15, .ohm = 53650 },
	{ .temp_c = -14, .ohm = 51178 },
	{ .temp_c = -13, .ohm = 48835 },
	{ .temp_c = -12, .ohm = 46613 },
	{ .temp_c = -11, .ohm = 44506 },
	{ .temp_c = -10, .ohm = 42506 },
	{ .temp_c = -9,  .ohm = 40600 },
	{ .temp_c = -8,  .ohm = 38791 },
	{ .temp_c = -7,  .ohm = 37073 },
	{ .temp_c = -6,  .ohm = 35442 },
	{ .temp_c = -5,  .ohm = 33892 },
	{ .temp_c = -4,  .ohm = 32420 },
	{ .temp_c = -3,  .ohm = 31020 },
	{ .temp_c = -2,  .ohm = 29689 },
	{ .temp_c = -1,  .ohm = 28423 },
	{ .temp_c = 0,   .ohm = 27219 },
	{ .temp_c = 1,   .ohm = 26076 },
	{ .temp_c = 2,   .ohm = 24988 },
	{ .temp_c = 3,   .ohm = 23951 },
	{ .temp_c = 4,   .ohm = 22963 },
	{ .temp_c = 5,   .ohm = 22021 },
	{ .temp_c = 6,   .ohm = 21123 },
	{ .temp_c = 7,   .ohm = 20267 },
	{ .temp_c = 8,   .ohm = 19450 },
	{ .temp_c = 9,   .ohm = 18670 },
	{ .temp_c = 10,  .ohm = 17926 },
	{ .temp_c = 11,  .ohm = 17214 },
	{ .temp_c = 12,  .ohm = 16534 },
	{ .temp_c = 13,  .ohm = 15886 },
	{ .temp_c = 14,  .ohm = 15266 },
	{ .temp_c = 15,  .ohm = 14674 },
	{ .temp_c = 16,  .ohm = 14108 },
	{ .temp_c = 17,  .ohm = 13566 },
	{ .temp_c = 18,  .ohm = 13049 },
	{ .temp_c = 19,  .ohm = 12554 },
	{ .temp_c = 20,  .ohm = 12081 },
	{ .temp_c = 21,  .ohm = 11628 },
	{ .temp_c = 22,  .ohm = 11195 },
	{ .temp_c = 23,  .ohm = 10780 },
	{ .temp_c = 24,  .ohm = 10382 },
	{ .temp_c = 25,  .ohm = 10000 },
	{ .temp_c = 26,  .ohm = 9634 },
	{ .temp_c = 27,  .ohm = 9284 },
	{ .temp_c = 28,  .ohm = 8947 },
	{ .temp_c = 29,  .ohm = 8624 },
	{ .temp_c = 30,  .ohm = 8315 },
	{ .temp_c = 31,  .ohm = 8018 },
	{ .temp_c = 32,  .ohm = 7734 },
	{ .temp_c = 33,  .ohm = 7461 },
	{ .temp_c = 34,  .ohm = 7199 },
	{ .temp_c = 35,  .ohm = 6948 },
	{ .temp_c = 36,  .ohm = 6707 },
	{ .temp_c = 37,  .ohm = 6475 },
	{ .temp_c = 38,  .ohm = 6253 },
	{ .temp_c = 39,  .ohm = 6039 },
	{ .temp_c = 40,  .ohm = 5834 },
	{ .temp_c = 41,  .ohm = 5636 },
	{ .temp_c = 42,  .ohm = 5445 },
	{ .temp_c = 43,  .ohm = 5262 },
	{ .temp_c = 44,  .ohm = 5086 },
	{ .temp_c = 45,  .ohm = 4917 },
	{ .temp_c = 46,  .ohm = 4754 },
	{ .temp_c = 47,  .ohm = 4597 },
	{ .temp_c = 48,  .ohm = 4446 },
	{ .temp_c = 49,  .ohm = 4301 },
	{ .temp_c = 50,  .ohm = 4161 },
	{ .temp_c = 51,  .ohm = 4026 },
	{ .temp_c = 52,  .ohm = 3896 },
	{ .temp_c = 53,  .ohm = 3771 },
	{ .temp_c = 54,  .ohm = 3651 },
	{ .temp_c = 55,  .ohm = 3535 },
	{ .temp_c = 56,  .ohm = 3423 },
	{ .temp_c = 57,  .ohm = 3315 },
	{ .temp_c = 58,  .ohm = 3211 },
	{ .temp_c = 59,  .ohm = 3111 },
	{ .temp_c = 60,  .ohm = 3014 },
	{ .temp_c = 61,  .ohm = 2922 },
	{ .temp_c = 62,  .ohm = 2834 },
	{ .temp_c = 63,  .ohm = 2748 },
	{ .temp_c = 64,  .ohm = 2666 },
	{ .temp_c = 65,  .ohm = 2586 },
	{ .temp_c = 66,  .ohm = 2509 },
	{ .temp_c = 67,  .ohm = 2435 },
	{ .temp_c = 68,  .ohm = 2364 },
	{ .temp_c = 69,  .ohm = 2294 },
	{ .temp_c = 70,  .ohm = 2228 },
	{ .temp_c = 71,  .ohm = 2163 },
	{ .temp_c = 72,  .ohm = 2100 },
	{ .temp_c = 73,  .ohm = 2040 },
	{ .temp_c = 74,  .ohm = 1981 },
	{ .temp_c = 75,  .ohm = 1925 },
	{ .temp_c = 76,  .ohm = 1870 },
	{ .temp_c = 77,  .ohm = 1817 },
	{ .temp_c = 78,  .ohm = 1766 },
	{ .temp_c = 79,  .ohm = 1716 },
	{ .temp_c = 80,  .ohm = 1669 },
	{ .temp_c = 81,  .ohm = 1622 },
	{ .temp_c = 82,  .ohm = 1578 },
	{ .temp_c = 83,  .ohm = 1535 },
	{ .temp_c = 84,  .ohm = 1493 },
	{ .temp_c = 85,  .ohm = 1452 },
	{ .temp_c = 86,  .ohm = 1413 },
	{ .temp_c = 87,  .ohm = 1375 },
	{ .temp_c = 88,  .ohm = 1338 },
	{ .temp_c = 89,  .ohm = 1303 },
	{ .temp_c = 90,  .ohm = 1268 },
	{ .temp_c = 91,  .ohm = 1234 },
	{ .temp_c = 92,  .ohm = 1202 },
	{ .temp_c = 93,  .ohm = 1170 },
	{ .temp_c = 94,  .ohm = 1139 },
	{ .temp_c = 95,  .ohm = 1110 },
	{ .temp_c = 96,  .ohm = 1081 },
	{ .temp_c = 97,  .ohm = 1053 },
	{ .temp_c = 98,  .ohm = 1026 },
	{ .temp_c = 99,  .ohm = 999 },
	{ .temp_c = 100, .ohm = 974 },
	{ .temp_c = 101, .ohm = 949 },
	{ .temp_c = 102, .ohm = 925 },
	{ .temp_c = 103, .ohm = 902 },
	{ .temp_c = 104, .ohm = 880 },
	{ .temp_c = 105, .ohm = 858 },
	{ .temp_c = 106, .ohm = 837 },
	{ .temp_c = 107, .ohm = 816 },
	{ .temp_c = 108, .ohm = 796 },
	{ .temp_c = 109, .ohm = 777 },
	{ .temp_c = 110, .ohm = 758 },
	{ .temp_c = 111, .ohm = 740 },
	{ .temp_c = 112, .ohm = 722 },
	{ .temp_c = 113, .ohm = 705 },
	{ .temp_c = 114, .ohm = 688 },
	{ .temp_c = 115, .ohm = 672 },
	{ .temp_c = 116, .ohm = 656 },
	{ .temp_c = 117, .ohm = 640 },
	{ .temp_c = 118, .ohm = 625 },
	{ .temp_c = 119, .ohm = 611 },
	{ .temp_c = 120, .ohm = 596 },
	{ .temp_c = 121, .ohm = 583 },
	{ .temp_c = 122, .ohm = 569 },
	{ .temp_c = 123, .ohm = 556 },
	{ .temp_c = 124, .ohm = 544 },
	{ .temp_c = 125, .ohm = 531 },
};

/* Order matters, ntc_match references the entries by index */
static struct ntc_device_id g_ntc_thermistor_id[] = {
	{ "murata,ncp15xh103", ncp_XXxh103, ARRAY_SIZE(ncp_XXxh103) },
	{ },
};

struct ntc_device g_ntc_dev = {
	.pullup_uv = 0,
	.pullup_ohm = 0,
	.pulldown_ohm = 0,
	.comp = NULL,
	.n_comp = 0,
	.name = NULL,
	.initial_flag = NTC_DEV_NOT_INITIAL,
};

/*
 * Function    : get_device_tree_data
 * Input       : struct ntc_device *dev     ntc device point
 * Output      : success return 0, other return -ENODEV
 * Description : get ntc device info from dts
 */
static s32 get_device_tree_data(struct ntc_device *dev)
{
	struct device_node *node = NULL;
	u32 tsensor_id = 0;
	s32 len = 0;
	s32 ret = 0;

	if (dev == NULL)
		return -ENODEV;

	node = of_find_compatible_node(NULL, NULL, "hisilicon,pmuadc");
	if (node == NULL)
		return -ENODEV;

	ret = of_property_read_string(node, "tsensor-name",
		(const char **)(&(dev->name)));
	if (ret || (dev->name == NULL))
		return -ENODEV;

	/* match device table , find proper compensation */
	for (tsensor_id = 0; tsensor_id < ARRAY_SIZE(g_ntc_thermistor_id); tsensor_id++) {
		len = (strlen(g_ntc_thermistor_id[tsensor_id].name) >= strlen(dev->name))
				? (strlen(g_ntc_thermistor_id[tsensor_id].name))
				: (strlen(dev->name));
		if (strncmp(g_ntc_thermistor_id[tsensor_id].name, dev->name, len) == 0) {
			dev->comp = g_ntc_thermistor_id[tsensor_id].table;
			dev->n_comp = g_ntc_thermistor_id[tsensor_id].table_len;
			break;
		}
	}

	/* not support this type ntc */
	if (dev->comp == NULL)
		return -ENODEV;

	ret = of_property_read_u32(node, "pullup-uv", &dev->pullup_uv);
	if (ret) {
		of_node_put(node);
		return -ENODEV;
	}

	ret = of_property_read_u32(node, "pullup-ohm", &dev->pullup_ohm);
	if (ret) {
		of_node_put(node);
		return -ENODEV;
	}

	ret = of_property_read_u32(node, "pulldown-ohm", &dev->pulldown_ohm);
	if (ret) {
		of_node_put(node);
		return -ENODEV;
	}

	of_node_put(node);
	return RET_SUCC;
}

/*
 * Function    : div64_u64_safe
 * Input       : dividend and divisor
 * Output      : return value = dividend / divisor
 * Description : unsigned 64bit divide with 64bit divisor
 */
static inline u64 div64_u64_safe(const u64 dividend, const u64 divisor)
{
	if (divisor == 0 && dividend == 0)
		return RET_SUCC;

	if (divisor == 0)
		return UINT_MAX;

	return div64_u64(dividend, divisor);
}

/*
 * Function    : get_ohm_of_thermistor
 * Input       : voltage,unit is mv
 * Output      : resistance, unit is ohm
 * Description : convert voltage to resistance
 */
static s32 get_ohm_of_thermistor(const struct ntc_device *dev, u32 mv)
{
	u64 pmv = 0;
	u64 n = 0;
	u64 puo = 0;

	/* convert pullup_uv to mv */
	pmv = dev->pullup_uv / MICRO_VOLT;
	puo = dev->pullup_ohm;

	/* if adc mv == 0, the ohm = 0 */
	if (mv == 0)
		return mv;

	/* use NTC_CONNECTED_GROUND, adc mv will not lager than pullupmv */
	if (mv >= pmv) {
		pr_err("adc volt(%u mv) >= pullup voltage(%llu)\n\r", mv, pmv);
		return INT_MAX;
	}

	/*
	*  adc_uv = out_ohm * pullup_uv /(pullup_ohm + out_ohm)
	*  out_ohm = (adc_uv * pullup_ohm) / (pullup_uv  - adc_uv )
	*/
	n = div64_u64_safe(puo * mv, pmv - mv);
	if (n > INT_MAX)
		n = INT_MAX;

	return (s32)n;
}

/*
 * Function    : lookup_comp
 * Input       : resistance value,unit is ohm
 * Output      : ohm index
 * Description : lookup temperature vs resistance table, get proper ohm index
 */
static void lookup_comp(const struct ntc_device *dev, u32 ohm,
						s32  *i_low, s32 *i_high)
{
	s32 comp_start = 0;
	s32 comp_mid = 0;
	s32 comp_end = 0;

	if ( (dev == NULL) ||(dev->comp == NULL) ||
		(i_low == NULL) || (i_high == NULL))
		return;

	/*
	* Handle special cases: Resistance is higher than or equal to resistance
	* in first table entry, or resistance is lower or equal to resistance
	* in last table entry.
	* In these cases, return i_low == i_high, either pointing to
	* the beginning or to the end of the table depending on the condition.
	*/
	if (ohm >= dev->comp[0].ohm) {
		*i_high = 0;
		*i_low = 0;
		return;
	}
	if (ohm <= dev->comp[dev->n_comp - 1].ohm) {
		*i_high = dev->n_comp - 1;
		*i_low = dev->n_comp - 1;
		return;
	}

	/* Do a binary search on compensation table */
	comp_end = dev->n_comp;
	comp_start = 0;
	while (comp_start < comp_end) {
		comp_mid = comp_start + (comp_end - comp_start) / BISECTION_DIVIDE;
		/*
		* comp_start <= comp_mid < comp_end
		* data->comp[comp_start].ohm > ohm >= data->comp[comp_end].ohm
		*
		* We could check for "ohm == data->comp[comp_mid].ohm" here,
		* but that is a quite unlikely condition, and we would have
		* to check again after updating comp_start. Check it at the comp_end
		* instead for simplicity.
		*/
		if (ohm >= dev->comp[comp_mid].ohm) {
			comp_end = comp_mid;
		} else {
			comp_start = comp_mid + 1;
			/*
			* ohm >= data->comp[start].ohm might be true here, since
			* we set start to mid + 1. In that case, we are done.
			* We could keep going, but the condition is quite likely
			* to occur, so it is worth checking for it.
			*/
			if (ohm >= dev->comp[comp_start].ohm)
				comp_end = comp_start;
		}
		/*
		* data->comp[comp_start].ohm >= ohm >= data->comp[comp_end].ohm
		* comp_start <= comp_end
		*/
	}
	/*
	* ohm >= data->comp[comp_end].ohm
	* comp_start == comp_end
	*/
	*i_low = comp_end;
	if (ohm == dev->comp[comp_end].ohm)
		*i_high = comp_end;
	else
		*i_high = comp_end - 1;

}

/*
 * Function    : get_temp_mc
 * Input       : resistance value
 * Output      : temperature
 * Description : lookup temperature vs resistance table, convert resistance to temperature
 */
static s32 get_temp_mc(const struct ntc_device *dev, const u32 ohm)
{
	s32 low = 0;
	s32 high = 0;
	s32 temp = 0;

	if ((dev == NULL) || ( dev->comp == NULL))
		return -ENODEV;

	lookup_comp(dev, ohm, &low, &high);
	if (low == high) {
		/* Unable to use linear approximation */
		temp = dev->comp[low].temp_c * TEMPERATURE_COEFF;
	} else {
		/* calc temperature by linear interpolation
		* (T_high - T_low) * 1000 / (O_high - O_low) = (T_cur - T_low) * 1000 / (O_cur - O_low)
		* (T_cur - T_low) * 1000 = (T_high - T_low) * 1000 * (O_cur - O_low) / (O_high - O_low)
		* T_cur * 1000 = T_low * 1000 + (T_high - T_low) * 1000 * (O_cur - O_low) / (O_high - O_low)
		*
		* by the way, we get the dev->comp[x].ohm value with dichotomy lookup table which is ordered,
		* the difference value always be not zero.
		*/
		temp = dev->comp[low].temp_c * TEMPERATURE_COEFF +
			((dev->comp[high].temp_c - dev->comp[low].temp_c) *
			TEMPERATURE_COEFF * ((int)ohm - (int)dev->comp[low].ohm)) /
			((int)dev->comp[high].ohm - (int)dev->comp[low].ohm);
	}
	return temp;
}

/*
 * Function    : ntc_read_temp
 * Input       : ADC channel, ntc in pmu adc channel is 3\4\5\6
 * Output      : output temperature = actual temperature * 1000
 * Description : get ntc sensor temperature from pmu
 */
s32 ntc_read_temp(const s32 channel, s32 *temp)
{
	s32 ohm = 0;
	s32 ret = 0;
	s32 mv = 0;
	s32 pmu_channel = channel;

	if ((pmu_channel < PMU_TSENSOR_START_CHANNEL) ||
		(pmu_channel > PMU_TSENSOR_MAX_CHANNEL) || (temp == NULL)) {
		pr_err("not support %d channel\n\r", pmu_channel);
		return RET_FAIL;
	}

	spin_lock(&ntc_lock);
	/* if ntc device not initial, must to get ntc info from dts */
	if (g_ntc_dev.initial_flag == NTC_DEV_NOT_INITIAL) {
		ret = get_device_tree_data(&g_ntc_dev);
		if (ret < 0) {
			pr_err("from dts get_device_tree_data fail\n\r");
			spin_unlock(&ntc_lock);
			return RET_FAIL;
		}
		g_ntc_dev.initial_flag = NTC_DEV_HAVE_INITIAL;
	}

	/* get voltage from pmu base on spmi */
	mv = hisi_adc_get_value(pmu_channel);
	if (mv < 0) {
		pr_err("get channel %d adc value fail\n\r", pmu_channel);
		spin_unlock(&ntc_lock);
		return RET_FAIL;
	}

	/* convert voltage to ohm */
	ohm = get_ohm_of_thermistor(&g_ntc_dev, mv);
	if (ohm == INT_MAX) {
		pr_err("get ohm fail\n");
		spin_unlock(&ntc_lock);
		return RET_FAIL;
	}

	*temp = get_temp_mc(&g_ntc_dev, ohm);
	spin_unlock(&ntc_lock);

	return RET_SUCC;
}
EXPORT_SYMBOL(ntc_read_temp);

MODULE_DESCRIPTION("HISI PMIC tsensor helpers");
MODULE_LICENSE("GPL v2");

#else
int ntc_read_temp(void)
{
	return 0;
}
#endif
