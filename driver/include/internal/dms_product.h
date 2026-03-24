/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description:
 * Author: Huawei
 * Create: 2022-3-24
 * File Name     : dms_product.c
 * Version       : Initial product
 * Author        :
 * Created       : 2022-3-24
 * Last Modified :
 * Description   :

 *  History       :
 * 1.Date        : 2022-3-24
 *    Author      :
 *    Modification: Created file
 */
#ifndef __DMS_PRODUCT_H__
#define __DMS_PRODUCT_H__

#define DMS_PRODUCT_CMD_NAME "DMS_PRODUCT_CMD_NAME"
#define DMS_SUBCMD_GET_PCIE_ID_INFO_ALL 0

#include "devdrv_interface.h"

#ifdef CONFIG_LLT
static inline void printf_stub(char *format, ...) {
}
#define dms_err          printf_stub
#endif
struct dmanage_pcie_id_info_all {
    unsigned int venderid;    /* 厂商id */
    unsigned int subvenderid; /* 厂商子id */
    unsigned int deviceid;    /* 设备id */
    unsigned int subdeviceid; /* 设备子id */
    int domain;               /* pcie域 */
    unsigned int bus;         /* 总线号 */
    unsigned int device;      /* 设备物理号 */
    unsigned int fn;          /* 设备功能号 */
    unsigned int davinci_id;  /* device id */
    unsigned char reserve[28];
};

int dms_product_init(void);
void dms_product_exit(void);
int devdrv_get_pcie_id_all(void *feature, char *in, u32 in_len, char *out, u32 out_len);
extern int devdrv_get_pcie_id_info(u32 devid, struct devdrv_pcie_id_info *pcie_id_info);
extern int devdrv_manager_container_logical_id_to_physical_id(u32 logical_dev_id, u32 *physical_dev_id, u32 *vfid);

#endif