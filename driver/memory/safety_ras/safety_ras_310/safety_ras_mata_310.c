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
 * Create: 2023-05-17
 */
#include <linux/slab.h>
#include <linux/notifier.h>
#include "soc_res.h"
#include "safety_ras_mata_310.h"
#include "safety_ras_mata_config_310.h"
#include "safety_ras_init_310.h"
#include "fpdc_ras_receiver.h"
#include "drv_ras_common.h"

bool g_mata_ras_ce_type = false;    // 进行中断风暴解抑制时判断故障类型

STATIC struct drv_memory_devices g_mata_devs;

struct drv_memory_devices *mata_get_devices(void)
{
	return &g_mata_devs;
}

uint32_t mata_dev_num(uint32_t dev_num)
{
	return (dev_num < (uint32_t)MATA_NODE_ID_MAX) ? (dev_num) : ((uint32_t)MATA_NODE_ID_MAX);
}

/*
 * @brief: get base addr of RAS_INT_COALESCE module, it will be used to unmask mata ras
 * @in[work]: pointer of work_struct
 * @in[io_base_ret]: base addr of RAS_INT_COALESCE
 * @return: 0:OK -1:fail
 */
STATIC int32_t get_unmask_mata_ras_io_base(struct work_struct *work, uint64_t *io_base_ret)
{
#ifndef DRV_MEM_GTEST
	struct soc_reg_base_info io_base;
	struct res_inst_info res_inst;
	int32_t ret;
	struct memory_dev *node;
	uint32_t devid;

	if (g_mata_ras_ce_type) {
		node = container_of(work, struct memory_dev, unmask_ce_irq_work.work);
	} else {
		node = container_of(work, struct memory_dev, unmask_ue_irq_work.work);
	}
	devid = node->die_id;

	soc_resmng_inst_pack(&res_inst, devid, TS_SUBSYS, 0);
	ret = soc_resmng_get_reg_base(&res_inst, "ARM_RAS_REG", &io_base);
	if (ret != 0) {
		memory_drv_err("Failed to get arm ras reg base. (ret=%d; devid=%u)\n", ret, devid);
		return -1;
	}

	*io_base_ret = io_base.io_base;
	return 0;
#endif
}

/*
 * @brief: unmask mata ce ras
 * @in[work]: pointer of work_struct
 * @return: NA
 */
STATIC void mata_unmask_ras_ce_irq(struct work_struct *work)
{
#ifndef DRV_MEM_GTEST
	uint64_t io_base;
	uint32_t int_type = INT_TYP0;
	int32_t ret = get_unmask_mata_ras_io_base(work, &io_base);
	if (ret != 0) {
		memory_drv_err("get_unmask_mata_ras_io_base fail\n");
		return;
	}

	(void)ras_int_unmask_reg_write(io_base + (uint64_t)INT_TYP0_ENA_OFFSET, MATA_RAS_IRQ_MASK, MATA_RAS_IRQ_MASK);
	memory_drv_info("unmask mata ras irq, type: %u\n", int_type);
#endif
}

/*
 * @brief: unmask mata ue ras
 * @in[work]: pointer of work_struct
 * @return: NA
 */
STATIC void mata_unmask_ras_ue_irq(struct work_struct *work)
{
#ifndef DRV_MEM_GTEST
	uint64_t io_base;
	uint32_t int_type = INT_TYP1;
	int32_t ret = get_unmask_mata_ras_io_base(work, &io_base);
	if (ret != 0) {
		memory_drv_err("get_unmask_mata_ras_io_base fail\n");
		return;
	}

	(void)ras_int_unmask_reg_write(io_base + (uint64_t)INT_TYP1_ENA_OFFSET, MATA_RAS_IRQ_MASK, MATA_RAS_IRQ_MASK);
	memory_drv_info("unmask mata ras irq, type: %u\n", int_type);
#endif
}

/*
 * @brief: init node and sensor info
 * @return: 0-success, others-fail
 */
int32_t mata_dms_node_init(void)
{
	int32_t ret;
	uint32_t i, j;

	ret = devdrv_get_devnum(&g_mata_devs.dev_num);
	if (ret != 0) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return ret;
	}
	memory_drv_info("dev num: %u\n", g_mata_devs.dev_num);

	for (i = 0; i < mata_dev_num(g_mata_devs.dev_num); i++) {
		g_mata_devs.m_dev[i][MATA_SENSOR_DDRC].die_id = i;
		g_mata_devs.m_dev[i][MATA_SENSOR_DDRC].dev_node = &g_mata_dev_node_table[i][MATA_SENSOR_DDRC];
		g_mata_devs.m_dev[i][MATA_SENSOR_DDRC].sensor_obj_num =
			(uint32_t)sizeof(g_mata_sensor_table[i]) / (uint32_t)sizeof(struct dms_sensor_object_cfg);
		g_mata_devs.m_dev[i][MATA_SENSOR_DDRC].sensor_obj_table = g_mata_sensor_table[i];
		for (j = 0; j < g_mata_devs.m_dev[i][MATA_SENSOR_DDRC].sensor_obj_num; j++) {
			mutex_init(&g_mata_devs.m_dev[i][MATA_SENSOR_DDRC].sensor_event_queue[j].mutex);
			INIT_LIST_HEAD(&g_mata_devs.m_dev[i][MATA_SENSOR_DDRC].sensor_event_queue[j].error_list.list);
			INIT_DELAYED_WORK(&g_mata_devs.m_dev[i][MATA_SENSOR_DDRC].unmask_ce_irq_work, mata_unmask_ras_ce_irq);
			INIT_DELAYED_WORK(&g_mata_devs.m_dev[i][MATA_SENSOR_DDRC].unmask_ue_irq_work, mata_unmask_ras_ue_irq);
		}
	}

	return 0;
}

int32_t mata_dms_node_destroy(void)
{
	uint32_t i, j;

	for (i = 0; i < mata_dev_num(g_mata_devs.dev_num); i++) {
		for (j = 0; j < g_mata_devs.m_dev[i][MATA_SENSOR_DDRC].sensor_obj_num; j++)
			mutex_destroy(&g_mata_devs.m_dev[i][MATA_SENSOR_DDRC].sensor_event_queue[j].mutex);
	}

	return 0;
}

int32_t mata_dev_node_register(void)
{
	int32_t i;
	uint32_t j, sensor_id;
	uint32_t dev_id;
	uint32_t successed_sensor_num;
	struct memory_dev *m_dev = NULL;
	struct drv_memory_devices *mata_devs = mata_get_devices();
	int32_t ret;

	for (dev_id = 0; dev_id < mata_dev_num(mata_devs->dev_num); dev_id++) {
		for (sensor_id = 0; sensor_id < (uint32_t)MATA_SENSOR_ID_MAX; sensor_id++) {
			m_dev = &mata_devs->m_dev[dev_id][sensor_id];	/* get each device each sensor dev_node */
			ret = memory_register_one_node(m_dev);
			if (ret != 0) {
				memory_drv_err("register one node failed. (dev_id=%d; s_id=%d; ret=%d)\n", dev_id, sensor_id, ret);
				goto out;
			}
		}
	}

	return 0;
out:
	for (i = (int32_t)dev_id; i >= 0; i--) {
		successed_sensor_num = ((i == (int32_t)dev_id) ? sensor_id : (uint32_t)MATA_SENSOR_ID_MAX);
		for (j = 0; j < successed_sensor_num; j++)
			memory_unregister_one_node(&mata_devs->m_dev[i][j]);
	}
	return ret;
}

int32_t mata_dev_node_uregister(void)
{
	int32_t j;
	uint32_t i, k;
	struct drv_memory_devices *mata_devs = mata_get_devices();

	for (i = 0; i < mata_dev_num(mata_devs->dev_num); i++) {
		for (j = 0; j < (int32_t)MATA_SENSOR_ID_MAX; j++) {
			(void)dms_sensor_node_unregister(mata_devs->m_dev[i][j].dev_node);
			(void)dms_unregister_dev_node(mata_devs->m_dev[i][j].dev_node);
			for (k = 0; k < mata_devs->m_dev[i][j].sensor_obj_num; k++)
				memory_fault_event_free(&mata_devs->m_dev[i][j].sensor_event_queue[k]);
		}
	}

	return 0;
}

STATIC void memory_kfree(struct memory_error_list **ptr)
{
	if (*ptr != NULL) {
		kfree(*ptr);
		*ptr = NULL;
	}
}

/*
 * @brief: parse data from fpdc and fill event data
 * @in[data]: data from fpdc
 * @out[event]: store result of parsing data from fpdc
 * @return: 0-success, others-fail
 */
#ifndef DRV_MEM_GTEST
STATIC int32_t mata_ras_data_parse(const struct notify_data *data, struct memory_fault_event *event)
{
	hisi_common_error_info *pdata_common = NULL;
	uint32_t error_code;
	uint16_t event_type;

	if (guid_equal(data->section_type, &CPER_SEC_HISI_COMMON)) {
		pdata_common = (hisi_common_error_info *)data->origin_data;
		error_code = pdata_common->register_array[4U] & MEMORY_MASK_16BIT;	/* err_status: 0~15 bit */
		if (get_mata_event_type_by_error_code(error_code, &event_type) != 0) {
			memory_drv_warn("Invalid error_code.\n");
			return -EINVAL;
		}
		event->error_code = error_code;
		event->event_type = event_type;
		event->device_id = (uint8_t)data->chip_id;
		event->sensor_id = MATA_SENSOR_DDRC;
		memory_drv_event("[MATA RAS DETAIL INFO] dump start ==========\n");
		memory_drv_event("ERR_FRL:     0x%x\n", pdata_common->register_array[0U]);
		memory_drv_event("ERR_STATUSL: 0x%x\n", pdata_common->register_array[4U]);
		memory_drv_event("ERR_ADDRL:   0x%x\n", pdata_common->register_array[6U]);
		memory_drv_event("ERR_ADDRH:   0x%x\n", pdata_common->register_array[7U]);
		memory_drv_event("ERR_MISC1L:  0x%x\n", pdata_common->register_array[10U]);
		memory_drv_event("ERR_MISC1H:  0x%x\n", pdata_common->register_array[11U]);
		memory_drv_event("[MATA RAS DETAIL INFO] dump end   ==========\n");
	} else {
		memory_drv_warn("Invalid section_type.\n");
		return -EINVAL;
	}

	if (mata_parse_fault_table(event) == NULL) {
		memory_drv_warn("Event not in mata ras table, (device_id=%u; sensor_id=%u; event_type=0x%x)\n",
			event->device_id, event->sensor_id, event->event_type);
		return -EINVAL;
	}

	return 0;
}
#endif

/*
 * @brief: find the corresponding sensor and add event
 * @in[event]: event
 * @return: 0-success, others-fail
 */
STATIC int32_t mata_ras_event_handler(struct memory_fault_event event)
{
	int32_t ret;
	struct memory_dev *node = NULL;
	struct memory_event *event_queue = NULL;
	struct memory_error_list *error_new = NULL;
	struct drv_memory_devices *mata_devs = mata_get_devices();

	node = &mata_devs->m_dev[event.device_id][event.sensor_id];
	event_queue = &node->sensor_event_queue[MATA_SENSOR_DDRC]; /* get event queue list head */

	error_new = (struct memory_error_list *)kzalloc(sizeof(struct memory_error_list), GFP_KERNEL | __GFP_ACCOUNT);
	if (error_new == NULL) {
		memory_drv_err("New_event kzalloc failed.\n");
		return -ENOMEM;
	}

	ret = memcpy_s(&error_new->error, sizeof(struct memory_fault_event), &event, sizeof(struct memory_fault_event));
	if (ret != 0) {
		memory_kfree(&error_new);
		memory_drv_err("Memcpy_s failed. (ret=%d; die_id=%u; sensor_id=%u; event_type=0x%x)\n",
			ret, event.device_id, event.sensor_id, event.event_type);
		return ret;
	}

	memory_drv_event("[MATA RAS EVENT INFO] sensor_type: 0x%x, event_type: 0x%x, error_code: 0x%x\n",
		event.sensor_type, event.event_type, event.error_code);
	unmask_mata_irq_process(event.device_id, event.sensor_id, event.error_code);

	ret = mata_add_fault_event(error_new, event_queue);
	if (ret != 0) {
		memory_kfree(&error_new);
		memory_drv_info("Fault event already exist. (die_id=%u; sensor_id=%u; event_type=0x%x)\n",
			event.device_id, event.sensor_id, event.event_type);
		return ret;
	}

	return 0;
}

#ifndef DRV_MEM_GTEST
void mata_ras_event_callback(const struct notify_data *pdata)
{
	struct memory_fault_event event = {0};
	struct drv_memory_devices *mata_devs = mata_get_devices();

	if ((pdata == NULL) || (mata_devs == NULL) || (pdata->chip_id >= mata_dev_num(mata_devs->dev_num))) {
		memory_drv_err("Invalid para. (data=%d; mata_devs=%d)\n", (pdata == NULL), (mata_devs == NULL));
		return;
	}

	if (mata_ras_data_parse(pdata, &event) == 0)
		(void)mata_ras_event_handler(event);
}
#endif

int32_t mata_fpdc_notifier_unregister(void)
{
	int32_t ret;
#ifndef DRV_MEM_GTEST
	ret = fpdc_unregister_fault_notifier(DMS_DEV_TYPE_DDRA);
	if (ret != 0)
		memory_drv_warn("MEMDrv unregister mata fpdc fault notifier failed, (ret=%d)\n", ret);
#endif
	return 0;
}

int32_t mata_fpdc_notifier_register(void)
{
	int32_t ret;
#ifndef DRV_MEM_GTEST
	ret = fpdc_register_fault_notifier(DMS_DEV_TYPE_DDRA, mata_ras_event_callback);
	if (ret != 0) {
		memory_drv_err("MEMDrv register mata fpdc fault notifier failed, (ret=%d)\n", ret);
		goto ERR_EXIT;
	}
#endif
	return 0;
ERR_EXIT:
	(void)mata_fpdc_notifier_unregister();
	return ret;
}
