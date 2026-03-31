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
 * Create: 2022-08-13
 */
#ifndef EMU_ST
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/pci.h>
#include "devdrv_interface.h"
#include "host_config.h"

#define PCI_VENDOR_ID_HUAWEI 0x19e5

static const struct pci_device_id g_dp_proc_mng_driver_tbl[] = {{ PCI_VDEVICE(HUAWEI, 0xd100), 0 },
                                                                { PCI_VDEVICE(HUAWEI, 0xd105), 0 },
                                                                { PCI_VDEVICE(HUAWEI, 0xa126), 0 },
                                                                { PCI_VDEVICE(HUAWEI, 0xd801), 0 },
                                                                { PCI_VDEVICE(HUAWEI, 0xd500), 0 },
                                                                { PCI_VDEVICE(HUAWEI, 0xd501), 0 },
                                                                { PCI_VDEVICE(HUAWEI, 0xd802), 0 },
                                                                { PCI_VDEVICE(HUAWEI, 0xd803), 0 },
                                                                { PCI_VDEVICE(HUAWEI, 0xd804), 0 },
                                                                { DEVDRV_DIVERSITY_PCIE_VENDOR_ID, 0xd500,
                                                                  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
                                                                {}};
MODULE_DEVICE_TABLE(pci, g_dp_proc_mng_driver_tbl);

#else
void dp_proc_mng_pci_dev_tbl_test(void)
{
}
#endif