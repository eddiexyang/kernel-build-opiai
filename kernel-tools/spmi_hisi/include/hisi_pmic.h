/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2011-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef __HISI_PMIC_H
#define __HISI_PMIC_H

#include <linux/irqdomain.h>

struct irq_mask_info {
	int start_addr;
	int array;
};

struct irq_info {
	int start_addr;
	int array;
};

struct bit_info {
	int addr;
	int bit;
};

struct write_lock {
	int addr;
	int val;
};

struct hisi_pmic {
	struct resource *res;
	struct device *dev;
	void __iomem *regs;
	spinlock_t lock;
	struct irq_domain *domain;
	int irq;
	int gpio;
	unsigned int *irqs;
	int irqnum;
	int irqarray;
	struct irq_mask_info irq_mask_addr;
	struct irq_info irq_addr;
	int irqnum1;
	int irqarray1;
	struct irq_mask_info irq_mask_addr1;
	struct irq_info irq_addr1;
	struct write_lock normal_lock;
	struct write_lock debug_lock;
};

#if defined(CONFIG_HISI_SPMI) || defined(CONFIG_HISI_SPMI_MODULE)
u32 hisi_pmic_read_reg(const s32 reg);
s32 hisi_pmic_write_reg(const s32 reg, const u32 val);
s32 pmu_ldo2_enable(void);
s32 pmu_ldo2_disable(void);
s32 pmu_ldo2_set_voltage(const u32 volt_value);
s32 pmu_ldo16_enable(void);
s32 pmu_ldo16_disable(void);
s32 pmu_ldo16_set_voltage(const u32 volt_value);
s32 pmu_ldo9_enable(void);
s32 pmu_ldo9_disable(void);
s32 pmu_ldo9_set_voltage(const u32 volt_value);
s32 hisi_adc_get_value(const u32 channel);
s32 ntc_read_temp(const s32 channel, s32 *temp);
u32 hisi_second_pmic_read_reg(const s32 sid, const s32 reg);
s32 get_second_pmu_buck_volt(u32 device_id, u32 channel, u32 *volt_mv);
s32 get_main_pmu_buck_volt(u32 channel, u32 *volt_mv);
s32 get_main_pmu_ldo_volt(u32 channel, u32 *volt_mv);
s32 get_main_pmu_die_id(u8 *die_id, u8 len);
s32 get_second_pmu_die_id(u32 device_id, u8 *die_id, u8 len);
#else
static inline s32 hisi_pmic_read_reg(const s32 reg) { return 0; }
static inline s32 hisi_pmic_write_reg(const s32 reg, const u32 val) { return 0; }
static inline s32 pmu_ldo2_enable(void) { return 0; }
static inline s32 pmu_ldo2_disable(void) { return 0; }
static inline s32 pmu_ldo2_set_voltage(const u32 volt_value) { return 0; }
static inline s32 pmu_ldo16_enable(void) { return 0; }
static inline s32 pmu_ldo16_disable(void) { return 0; }
static inline s32 pmu_ldo16_set_voltage(const u32 volt_value) { return 0; }
static inline s32 pmu_ldo9_enable(void) { return 0; }
static inline s32 pmu_ldo9_disable(void) { return 0; }
static inline s32 pmu_ldo9_set_voltage(const u32 volt_value) { return 0; }
static inline s32 hisi_adc_get_value(const u32 channel) { return 0; }
static inline s32 ntc_read_temp(const s32 channel, s32 *temp) { return 0; }
static inline u32 hisi_second_pmic_read_reg(const s32 sid, const s32 reg) { return 0; }
static inline s32 get_second_pmu_buck_volt(u32 device_id, u32 channel, u32 *volt_mv) { return 0; }
static inline s32 get_main_pmu_buck_volt(u32 channel, u32 *volt_mv) { return 0; }
static inline s32 get_main_pmu_ldo_volt(u32 channel, u32 *volt_mv) { return 0; }
static inline s32 get_main_pmu_die_id(u8 *die_id, u8 len) { return 0; }
static inline s32 get_second_pmu_die_id(u32 device_id, u8 *die_id, u8 len) { return 0; }
#endif

#endif /* __HISI_PMIC_H */
