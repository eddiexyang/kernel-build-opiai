/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#ifndef __TSDRV_USER_INTERFACE_H__
#define __TSDRV_USER_INTERFACE_H__

#include "tsdrv_user_common.h"

#ifdef __cplusplus
extern "C" {
#endif

drvError_t tsdrv_id_res_info_query(uint32_t devId, uint32_t tsId, struct tsdrv_id_query_para *para);
drvError_t tsdrv_alloc_ipc_event_id(uint32_t devId, uint32_t tsId, uint32_t *id);

#ifdef __cplusplus
}
#endif

#endif /* __TSDRV_USER_INTERFACE_H__ */
