/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description:
 * Author: Huawei
 * Create: 2023-10-10
 * File Name     : dms_product_host.h
 * Version       : Initial product
 * Author        :
 * Created       : 2023-10-10
 * Last Modified :
 * Description   :

 *  History       :
 * 1.Date        : 2023-10-10
 *    Author      :
 *    Modification: Created file
 */
#ifndef __DMS_PRODUCT_HOST_H__
#define __DMS_PRODUCT_HOST_H__

#include "devdrv_interface.h"

#define DMS_SUBCMD_GET_WORK_MODE 1

int devdrv_get_work_mode(void *feature, char *in, u32 in_len, char *out, u32 out_len);
#ifdef CFG_SOC_PLATFORM_CLOUD
extern int devdrv_manager_get_amp_smp_mode(u32 *amp_or_smp);
#endif

#endif