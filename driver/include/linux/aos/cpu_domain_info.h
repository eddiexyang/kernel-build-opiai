/* SPDX-License-Identifier: GPL-2.0 */
/*
 * cpu_domain_info.h - Huawei AOS CPU domain interface
 *
 * This header is NOT part of upstream Linux. It is defined by Huawei's
 * customized kernel (EulerOS/openEuler with AOS patches). Since the
 * original header is not available in the open-source Ascend310B package,
 * this definition is derived from the exact field usage across all driver
 * source files that reference struct cpu_domain_info:
 *
 *   - drv_cpu_type.c (bbox, pcie, davinci_intf, dms, dms_smf, logdrv,
 *     prof, event_sched, ipcdrv, pkicms): cpu_domain.ctrlcpu_bitmap
 *   - davinci_intf_info.c: cpu_domain.ctrlcpu_num, datacpu_num, aicpu_num
 *
 * get_cpudomain_info() is expected to be provided by the AOS kernel;
 * all callers use __attribute__((weak)) so the driver compiles and loads
 * even when the function is absent at runtime.
 */
#ifndef _LINUX_AOS_CPU_DOMAIN_INFO_H
#define _LINUX_AOS_CPU_DOMAIN_INFO_H

struct cpu_domain_info {
	unsigned long ctrlcpu_bitmap;
	unsigned int ctrlcpu_num;
	unsigned int datacpu_num;
	unsigned int aicpu_num;
};

int get_cpudomain_info(struct cpu_domain_info *info);

#endif /* _LINUX_AOS_CPU_DOMAIN_INFO_H */
