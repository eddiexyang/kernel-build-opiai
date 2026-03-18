/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * Description: DMS qos related interface
 * Author: huawei
 * Create: 2021-12-15
 */

#ifndef __DMS_QOS_INTERFACE_H
#define __DMS_QOS_INTERFACE_H

int DmsGetQosInfo(unsigned int dev_id, unsigned int vfid, unsigned int sub_cmd, void *buf, unsigned int *size);
int DmsSetQosInfo(unsigned int dev_id, unsigned int sub_cmd, void *buf, unsigned int size);

#endif
