/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Description:
 * Author: huawei
 * Create: 2019-10-15
 */
#ifndef __DEVDRV_USER_CONFIG_H__
#define __DEVDRV_USER_CONFIG_H__

#define CTRL_CPU_NUM_MIN 1
#define DATA_CPU_NUM_MIN 0
#define AI_CPU_NUM_MIN 1
#define TOTAL_CPU_NUM_MAX 16

int devdrv_get_user_config_ex(unsigned int dev_id, const char *name, unsigned char *buf, unsigned int *buf_size);
int devdrv_set_user_config_ex(unsigned int dev_id, const char *name, unsigned char *buf, unsigned int buf_size);
int devdrv_clear_user_config_ex(unsigned int dev_id, const char *name);
int devdrv_get_boot_cfg(unsigned char *chip_info);
int devdrv_get_chip_type_from_user_cfg(unsigned int *chip_type);
int devdrv_user_config_common_check(unsigned int dev_id, const char *name);
#endif
