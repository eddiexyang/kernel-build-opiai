/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#ifndef PKICMS_MODULE_H
#define PKICMS_MODULE_H

#define ROOTFS_PATH_MASTER "/dev/sde1"
#define ROOTFS_PATH_BACKUP "/dev/sde2"

#define DEVNODE_MODE 0600
#define DEV_NAME "pkicms"
#define DEV_CLASS "pkicms_class"
#define DEV_COUNT 1

#define PKICMS_CMD_MAGIC            'P'
#define PKICMS_CMD_GET_EFUSE_FLAG   _IOR(PKICMS_CMD_MAGIC, 0, int)  /* get efuse flage */
#define PKICMS_CMD_GET_EFUSE_NVCNT  _IOR(PKICMS_CMD_MAGIC, 1, int)  /* get efuse L2NVCNT */
#define PKICMS_CMD_MAX_NR           1


typedef struct pkicms_dev_st {
    struct cdev cdev;
    struct class *dev_class;
    unsigned int dev_major;
    unsigned int dev_minor;
} cdev_st;

#endif
