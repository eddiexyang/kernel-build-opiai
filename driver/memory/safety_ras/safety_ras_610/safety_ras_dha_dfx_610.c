/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-10-27
 */
#include <linux/io.h>
#include <linux/of.h>
#include <linux/platform_device.h>

#include "memory_log.h"
#include "safety_ras_dha_610.h"

STATIC void __iomem *g_dha_virtaddr[RAS_DDRA_DHA_NUM] = {NULL, NULL};

/**
 * @brief ddra_dha_ras_init: dha ras使能
 * 使能前清除clear寄存器和status寄存器
 */
void ddra_dha_ras_dfx(void)
{
	uint32_t dha_idx;
	uint32_t regl_val;
	uint32_t regh_val;

	for (dha_idx = 0; dha_idx < RAS_DDRA_DHA_NUM; dha_idx++) {
		if (g_dha_virtaddr[dha_idx] != NULL) {
			regl_val = readl(g_dha_virtaddr[dha_idx] + DHA_ERR_MISC0L_STATUS_REG_OFFSET);
			regh_val = readl(g_dha_virtaddr[dha_idx] + DHA_ERR_MISC0H_STATUS_REG_OFFSET);
			memory_drv_info("ddra dha ras status0L:0x%x, status0H:0x%x\n", regl_val, regh_val);
			regl_val = readl(g_dha_virtaddr[dha_idx] + DHA_ERR_MISC1L_STATUS_REG_OFFSET);
			regh_val = readl(g_dha_virtaddr[dha_idx] + DHA_ERR_MISC1H_STATUS_REG_OFFSET);
			memory_drv_info("ddra dha ras status1L:0x%x, status1H:0x%x\n", regl_val, regh_val);
			regl_val = readl(g_dha_virtaddr[dha_idx] + DHA_ERR_ADDRL_REG_OFFSET);
			regh_val = readl(g_dha_virtaddr[dha_idx] + DHA_ERR_ADDRH_REG_OFFSET);
			memory_drv_info("ddra dha ras err addrL:0x%x, addrH:0x%x\n", regl_val, regh_val);
			writel(0x0U, g_dha_virtaddr[dha_idx] + DHA_ERR_MISC0L_STATUS_REG_OFFSET);
			writel(0x0U, g_dha_virtaddr[dha_idx] + DHA_ERR_MISC0H_STATUS_REG_OFFSET);
			writel(0x0U, g_dha_virtaddr[dha_idx] + DHA_ERR_MISC1L_STATUS_REG_OFFSET);
			writel(0x0U, g_dha_virtaddr[dha_idx] + DHA_ERR_MISC1H_STATUS_REG_OFFSET);
		}
	}

	return;
}

STATIC void ddra_dha_ras_init(void)
{
	uint32_t dha_idx;
	for (dha_idx = 0; dha_idx < RAS_DDRA_DHA_NUM; dha_idx++) {
		writel(0xffffffffU, g_dha_virtaddr[dha_idx] + DHA_ERR_STATUSL_REG_OFFSET);
		writel(0x0U, g_dha_virtaddr[dha_idx] + DHA_ERR_MISC1L_STATUS_REG_OFFSET);
		writel(0x0U, g_dha_virtaddr[dha_idx] + DHA_ERR_MISC1H_STATUS_REG_OFFSET);
		writel(0x19U, g_dha_virtaddr[dha_idx] + DHA_ERR_CTRLL_REG_OFFSET);
	}
	memory_drv_info("ddr_dha_ras_init success!\n");

	return;
}

STATIC void dha_ras_remove(struct platform_device *pdev)
{
	uint32_t dha_idx;
	(void)pdev;
	for (dha_idx = 0; dha_idx < RAS_DDRA_DHA_NUM; dha_idx++) {
		if (g_dha_virtaddr[dha_idx] != NULL) {
			writel(0x1U, g_dha_virtaddr[dha_idx] + DHA_ERR_CTRLL_REG_OFFSET);
			iounmap(g_dha_virtaddr[dha_idx]);
			g_dha_virtaddr[dha_idx] = NULL;
		}
	}

	return 0;
}

STATIC int dha_ras_probe(struct platform_device *pdev)
{
	uint32_t dha_idx;
	uint64_t mod_base;
	int32_t ret;
	for (dha_idx = 0; dha_idx < RAS_DDRA_DHA_NUM; dha_idx++) {
		mod_base = (uint32_t)(BS9SX1A_AO_CFGBUS_RING_BASE + ((CFGBUS_ID_DHA0 + dha_idx) << 16U));
		g_dha_virtaddr[dha_idx] = ioremap(mod_base, DDR_DHA_RAS_BASE_ADDR_LEN);
		if (g_dha_virtaddr[dha_idx] == NULL) {
			memory_drv_err("ddr dha ras ioremap fail\n");
			ret = -ENOMEM;
			goto out_release_source;
		}
	}
	ddra_dha_ras_init();
	return 0;
out_release_source:
	(void)dha_ras_remove(pdev);
	return ret;
}

STATIC int dha_ras_suspend(struct platform_device *pdev, pm_message_t state)
{
	uint32_t dha_idx;
	uint32_t reg_val;
	(void)pdev;
	(void)state;
	for (dha_idx = 0; dha_idx < RAS_DDRA_DHA_NUM; dha_idx++) {
		writel(0xffffffffU, g_dha_virtaddr[dha_idx] + DHA_RAS_MASK_MISC1H_REG_OFFSET);
		writel(0xffffffffU, g_dha_virtaddr[dha_idx] + DHA_RAS_MASK_MISC0L_REG_OFFSET);
		writel(0x1U, g_dha_virtaddr[dha_idx] + DHA_ERR_CTRLL_REG_OFFSET);
		reg_val = readl(g_dha_virtaddr[dha_idx] + DHA_ERR_MISC1L_STATUS_REG_OFFSET);
		if (reg_val != 0) {
			memory_drv_err("In ddr suspend, dha ras statusL:0x%x\n", reg_val);
		}
		reg_val = readl(g_dha_virtaddr[dha_idx] + DHA_ERR_MISC1H_STATUS_REG_OFFSET);
		if (reg_val != 0) {
			memory_drv_err("In ddr suspend, dha ras statusH:0x%x\n", reg_val);
		}
	}
	memory_drv_info("dha_ras_suspend ok!\n");
	return 0;
}

STATIC int dha_ras_resume(struct platform_device *pdev)
{
	uint32_t dha_idx;
	(void)pdev;
	ddra_dha_ras_init();
	for (dha_idx = 0; dha_idx < RAS_DDRA_DHA_NUM; dha_idx++) {
		writel(0, g_dha_virtaddr[dha_idx] + DHA_RAS_MASK_MISC1H_REG_OFFSET);
		writel(0, g_dha_virtaddr[dha_idx] + DHA_RAS_MASK_MISC0L_REG_OFFSET);
	}
	return 0;
}

STATIC const struct of_device_id dha_ras_match_table[] = {
	{ .compatible = "fhi_dha"},
	{ }
};

struct platform_driver g_dha_ras_driver = {
	.probe = dha_ras_probe,
	.remove = dha_ras_remove,
	.suspend = dha_ras_suspend,
	.resume = dha_ras_resume,
	.driver = {
		.name = "dha_ras",
		.of_match_table = dha_ras_match_table,
	}
};