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
* Description:
* Author: huawei
* Create: 2022-02-02
*/

#ifndef SOC_MISC_SAFETY_CONF_H
#define SOC_MISC_SAFETY_CONF_H

struct dfm_struct;
struct safety_irq_info;
struct ras_irq_info;

struct soc_misc_safety_cfg {
    u32 dms_node_num;
    u32 irq_cfg_num;
    struct dfm_struct *dfm;
    union {
        struct safety_irq_info *safety_irq_cfgs;
        struct ras_irq_info *ras_irq_cfgs;
    };
    int(*register_dms_node)(u32 dev_id);
    int(*post_init_process)(u32 dev_id);
    int(*post_uninit_process)(u32 dev_id);
};

/*
 * soc_misc_get_safety_cfg_array - get the safety fault configurations of all the modules
 * @num: the number of the configurations
 *
 * Safety fault configuration parameters of each module in the SOC_MISC module.
 * Based on these parameters, DMS nodes and interrupt information can be registered.
 */
struct soc_misc_safety_cfg **soc_misc_get_safety_cfg_array(u32 *num);

#endif
