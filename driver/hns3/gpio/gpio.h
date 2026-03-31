/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: gpio driver function
 * Author: huawei
 * Create: 2022-08-15
 */

#ifndef _GPIO_H_
#define _GPIO_H_

#define GPIO_SWPORT_DR 0x0
#define GPIO_SWPORT_DDR 0x04

void gpio_present_init(void __iomem *gpio_base, unsigned int index);
unsigned int gpio_get_present(void __iomem *gpio_base, unsigned int index);
void gpio_phy_init(void __iomem *gpio_base, unsigned int index);
void gpio_set(void __iomem *gpio_base, unsigned int index, bool val);
#endif // __GPIO_H__
