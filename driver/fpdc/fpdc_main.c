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

#include <linux/module.h>

#include "fpdc_define.h"
#include "receiver/fpdc_ras_receiver.h"


STATIC int __init fpdc_receiver_init(void)
{
    return fpdc_ras_receiver_init();
}

STATIC void __exit fpdc_receiver_exit(void) {
    fpdc_ras_receiver_exit();
}


module_init(fpdc_receiver_init);
module_exit(fpdc_receiver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI driver");


