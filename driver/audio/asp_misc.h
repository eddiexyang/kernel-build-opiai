/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/

#ifndef ASP_MISC_H
#define ASP_MISC_H

#include <linux/types.h>
#include <linux/of_platform.h>

int asp_misc_exit(void);
int asp_misc_init(struct platform_device *pdev);
void asp_do_resume(void);
void asp_do_suspend(void);
#endif
