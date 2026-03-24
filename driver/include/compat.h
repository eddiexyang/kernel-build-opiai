/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Ascend Driver Kernel Compatibility Layer
 *
 * Provides compatibility macros/wrappers for porting drivers originally
 * written for Linux 4.19/5.10 to Linux 6.18.
 *
 * Include this header at the top of source files that use deprecated APIs.
 * This file will be incrementally expanded as more modules are migrated.
 */
#ifndef __ASCEND_COMPAT_H__
#define __ASCEND_COMPAT_H__

#include <linux/version.h>

/* ========================================================================
 * 1. Memory mapping
 * ======================================================================== */

/* ioremap_nocache() removed in 5.6, use ioremap() which is always uncached */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#ifndef ioremap_nocache
#define ioremap_nocache ioremap
#endif
#endif

/* ========================================================================
 * 2. Access checking
 * ======================================================================== */

/*
 * access_ok() lost the 'type' parameter in 5.0
 * Old: access_ok(VERIFY_WRITE, addr, size)
 * New: access_ok(addr, size)
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#define compat_access_ok(type, addr, size) access_ok((addr), (size))
#else
#define compat_access_ok(type, addr, size) access_ok((type), (addr), (size))
#endif

/* ========================================================================
 * 3. Timer API
 * ======================================================================== */

/*
 * setup_timer() removed in 4.15+, replaced by timer_setup()
 * Old: setup_timer(&timer, callback, data)  -- callback(unsigned long)
 * New: timer_setup(&timer, callback, flags) -- callback(struct timer_list *)
 *
 * Migration requires changing callback signature. compat.h cannot auto-wrap
 * this — each module must be manually converted. This note is a reminder.
 */

/* ========================================================================
 * 4. Proc filesystem
 * ======================================================================== */

/*
 * proc_create() uses struct proc_ops instead of struct file_operations
 * since Linux 5.6. Provide a compat wrapper.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#include <linux/proc_fs.h>

/* PDE_DATA renamed to pde_data in 5.17 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
#ifndef PDE_DATA
#define PDE_DATA(inode) pde_data(inode)
#endif
#endif

#endif /* >= 5.6 */

/* ========================================================================
 * 5. DMA / PCI
 * ======================================================================== */

/* pci_set_dma_mask / pci_set_consistent_dma_mask removed in 5.18 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
#include <linux/dma-mapping.h>
#ifndef pci_set_dma_mask
#define pci_set_dma_mask(pdev, mask) dma_set_mask(&(pdev)->dev, (mask))
#endif
#ifndef pci_set_consistent_dma_mask
#define pci_set_consistent_dma_mask(pdev, mask) dma_set_coherent_mask(&(pdev)->dev, (mask))
#endif
#endif

/* pci_enable_msix() removed in 4.12, use pci_alloc_irq_vectors() */
/* Manual migration required per module */

/* ========================================================================
 * 6. Signal / scheduler
 * ======================================================================== */

/* No major compat issues, but note:
 * - wait_event_interruptible() behavior unchanged
 * - signal_pending() unchanged
 */

/* ========================================================================
 * 7. IDR / IDA
 * ======================================================================== */

/* idr_remove() return type changed to void in 4.19+ (already void in our baseline) */

/* ========================================================================
 * 8. Sysfs / Kobject
 * ======================================================================== */

/* bus_find_device_by_name() unchanged */
/* class_create() lost the owner parameter in 6.4 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
#define compat_class_create(owner, name) class_create(name)
/* Auto-redirect 2-arg class_create(THIS_MODULE, name) to 1-arg */
#define _compat_class_create_1(name)          class_create(name)
#define _compat_class_create_2(owner, name)   class_create(name)
#define _compat_cc_sel(_1, _2, NAME, ...)     NAME
#undef class_create
#define class_create(...) _compat_cc_sel(__VA_ARGS__, _compat_class_create_2, _compat_class_create_1)(__VA_ARGS__)
#else
#define compat_class_create(owner, name) class_create(owner, name)
#endif

/* ========================================================================
 * 9. Semaphore
 * ======================================================================== */

/*
 * DEFINE_SEMAPHORE() gained count parameter in 6.4
 * Old: DEFINE_SEMAPHORE(name)        -- count=1
 * New: DEFINE_SEMAPHORE(name, count)
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
#define COMPAT_DEFINE_SEMAPHORE(name) DEFINE_SEMAPHORE(name, 1)
#else
#define COMPAT_DEFINE_SEMAPHORE(name) DEFINE_SEMAPHORE(name)
#endif

/* ========================================================================
 * 10. Miscellaneous
 * ======================================================================== */

/* pci_free_irq_vectors() available since 4.10 - safe to use */

/* get_user_pages() signature changes:
 * 4.9:  get_user_pages(tsk, mm, start, nr, write, force, pages, vmas)
 * 4.10: get_user_pages(start, nr, gup_flags, pages, vmas)
 * 5.9:  get_user_pages(start, nr, gup_flags, pages)  -- vmas removed for non-locked
 * 6.5:  get_user_pages(start, nr, gup_flags, pages)   -- further simplification
 * Each module using this must be adapted individually.
 */

/* strlcpy() deprecated in 6.8, use strscpy() */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
#ifndef strlcpy
#define strlcpy(dst, src, size) strscpy((dst), (src), (size))
#endif
#endif

/* profile_event_register/unregister removed in 5.x
 * PROFILE_TASK_EXIT profiling hooks no longer available */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#ifndef PROFILE_TASK_EXIT
#define PROFILE_TASK_EXIT 0
#endif
static inline int profile_event_register(int t, struct notifier_block *n) { return 0; }
static inline int profile_event_unregister(int t, struct notifier_block *n) { return 0; }
#endif

/* __get_free_pages / free_pages unchanged */

/* of_device_is_available() unchanged */

/* platform_driver_register / platform_device_register unchanged */

/* pci_iomap / pci_iounmap unchanged */

/* ========================================================================
 * 11. Firmware
 * ======================================================================== */

/* request_firmware() / release_firmware() unchanged */

/* ========================================================================
 * 12. GPIO
 * ======================================================================== */

/* In 6.x, struct gpio_chip definition moved to <linux/gpio/driver.h>
 * Include it so that modules using <linux/gpio.h> with struct gpio_chip still work */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
#include <linux/gpio/driver.h>
#endif

/* gpio_request/gpio_free deprecated in favor of gpiod_* API since 3.x
 * but still functional through 6.x via <linux/gpio.h>.
 * Full migration to gpiod_* is recommended but not required.
 */

/* irq_domain_add_simple() renamed to irq_domain_create_simple() in 6.6+
 * and takes fwnode_handle* instead of device_node* */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
#include <linux/irqdomain.h>
#include <linux/of.h>
#define irq_domain_add_simple(node, size, first, ops, host_data) \
    irq_domain_create_simple(of_fwnode_handle(node), size, first, ops, host_data)
#endif

/* i2c_driver.probe lost the id parameter in 6.3+ */
/* i2c_driver.remove returns void instead of int in 6.1+ */
/* These must be fixed per-module in source code */

/* cyclecounter.read callback gained const in 6.x */
/* Must be fixed per-module in source code */

/* Include vendor compat layer (nth_page, etc.) */
#include <linux/opiai_vendor_compat.h>

/* del_timer_sync renamed to timer_delete_sync in 6.4+ */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
#define del_timer_sync timer_delete_sync
#endif


/* follow_pfn() removed in 6.x; stub it (returns -EINVAL) */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
static inline int follow_pfn(struct vm_area_struct *vma, unsigned long address, unsigned long *pfn) { return -EINVAL; }
#endif

/* hrtimer_init renamed to hrtimer_setup in 6.x series */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 12, 0)
#define hrtimer_init(timer, clock, mode) hrtimer_setup(timer, NULL, clock, mode)
#endif
/* Huawei custom hugepage API stub */
#ifndef HUGETLB_ALLOC_BUDDY
#define HUGETLB_ALLOC_BUDDY 0
static inline struct page *hugetlb_alloc_hugepage(int nid, int flag) { return NULL; }
#endif

/* Huawei custom mmap API stubs */

#ifndef __do_mmap_mm

static inline unsigned long __do_mmap_mm(struct mm_struct *mm, struct file *file, unsigned long addr,

    unsigned long len, unsigned long prot, unsigned long flags,

    unsigned long pgoff, unsigned long *populate, struct list_head *uf) { return -ENOTSUPP; }

#endif

/* IOMMU device feature API removed in kernel 6.x */

#ifndef IOMMU_DEV_FEAT_IOPF

#define IOMMU_DEV_FEAT_IOPF 0

#define IOMMU_DEV_FEAT_SVA 1

static inline int iommu_dev_enable_feature(struct device *dev, int feat) { return 0; }

static inline int iommu_dev_disable_feature(struct device *dev, int feat) { return 0; }

#endif

#endif /* __ASCEND_COMPAT_H__ */
