// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "svm: " fmt

#include <asm/esr.h>
#include <linux/mmu_context.h>

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/iommu.h>
#include <linux/miscdevice.h>
#include <linux/mman.h>
#include <linux/mmu_notifier.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/ptrace.h>
#include <linux/pid.h>
#include <linux/kprobes.h>
#include <linux/security.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/hugetlb.h>
#include <linux/sched/mm.h>
#include <linux/msi.h>
#include <linux/acpi.h>
#ifndef CONFIG_AOS_FEATURES
#include <linux/ascend_smmu.h>
#endif
#include <linux/share_pool.h>
#include <linux/securec.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

#ifdef DEV_SVM_UT
#define STATIC
#define INLINE
#define VM_PA32BIT  0x400000000
#else
#define STATIC static
#define INLINE inline
#endif

#ifndef VM_PA32BIT
#define VM_PA32BIT 0x400000000ULL
#endif

#ifndef CONFIG_EXTEND_HUGEPAGE_MAPPING
STATIC INLINE void *vmap_hugepage(struct page **pages, unsigned int count,
		unsigned long flags, pgprot_t prot)
{
	return vmap(pages, count, flags, prot);
}
#endif

#define SVM_DEVICE_NAME "svm"
#define ASID_SHIFT		48

#define SVM_IOCTL_PROCESS_BIND		0xffff
#define SVM_IOCTL_GET_PHYS		0xfff9
#define SVM_IOCTL_SET_RC		0xfffc
#define SVM_IOCTL_LOAD_FLAG		0xfffa
#define SVM_IOCTL_PIN_MEMORY		0xfff7
#define SVM_IOCTL_UNPIN_MEMORY		0xfff5
#define SVM_IOCTL_REMAP_PROC		0xfff4

#define SVM_REMAP_MEM_LEN_MAX		(16 * 1024 * 1024)

#define SVM_IOCTL_RELEASE_PHYS32	0xfff3
#define MMAP_PHY32_MAX (16 * 1024 * 1024)

#define SVM_IOCTL_SP_ALLOC		0xfff2
#define SVM_IOCTL_SP_FREE		0xfff1
#define SPG_DEFAULT_ID			0
#define CORE_SID		0

bool ts_shmem_need_init = false;

STATIC int probe_index;
STATIC DECLARE_RWSEM(svm_sem);
STATIC DECLARE_RWSEM(svm_register_sem);
STATIC struct rb_root svm_process_root = RB_ROOT;
STATIC struct mutex svm_process_mutex;
STATIC DEFINE_MUTEX(svm_symbol_mutex);

typedef unsigned long (*svm_kallsyms_lookup_name_t)(const char *name);
typedef struct mm_struct *(*svm_mm_access_t)(struct task_struct *task,
	unsigned int mode);

STATIC svm_kallsyms_lookup_name_t svm_kallsyms_lookup_name;
STATIC svm_mm_access_t svm_mm_access;

STATIC int svm_init_runtime_symbols(void)
{
	int err = 0;
	struct kprobe kp = {
		.symbol_name = "kallsyms_lookup_name",
	};

	if (svm_mm_access != NULL)
		return 0;

	mutex_lock(&svm_symbol_mutex);
	if (svm_mm_access != NULL)
		goto out;

	if (svm_kallsyms_lookup_name == NULL) {
		err = register_kprobe(&kp);
		if (err)
			goto out;
		svm_kallsyms_lookup_name = (svm_kallsyms_lookup_name_t)kp.addr;
		unregister_kprobe(&kp);
	}

	svm_mm_access = (svm_mm_access_t)svm_kallsyms_lookup_name("mm_access");
	if (svm_mm_access == NULL)
		err = -ENOENT;

out:
	mutex_unlock(&svm_symbol_mutex);
	return err;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
struct mm_struct *iommu_sva_find(int pasid);
#endif
void svm_process_exit_clean_slots(int pid);
int svm_register_pci_device(struct device *dev, u32 key);
int svm_unregister_pci_device(struct device *dev, u32 key);
struct iommu_domain *svm_find_domain_by_name(char *smmu_name);

STATIC INLINE int svm_request_dm_for_dev(struct device *dev)
{
#ifdef CONFIG_AOS_FEATURES
	dev_err(dev, "smmu bypass not supported\n");
	return -EINVAL;
#else
	struct iommu_domain *domain = iommu_get_domain_for_dev(dev);

	if (domain && domain->type == IOMMU_DOMAIN_IDENTITY)
		return 0;

	dev_err(dev, "direct-mapping request requires source kernel iommu dm-domain support\n");
	return -EOPNOTSUPP;
#endif
}

STATIC INLINE struct iommu_domain *svm_domain_alloc(struct device *dev)
{
	return iommu_paging_domain_alloc(dev);
}

struct dev_slot_ops {
	int				(*prepare)(struct device *dev);
	void				(*clean)(struct device *dev);
	int				(*bind)(struct device *dev, void *data);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
	int				(*set_mpam)(struct device *dev, void *data);
	int				(*get_mpam)(struct device *dev, void *data);
#endif
};

struct dev_slot {
	struct device			 *dev;
	u32				 key; /* globally unique */
	const struct dev_slot_ops	 *ops;
	struct list_head		 entry;
};

struct process_sva {
	struct iommu_sva *sva;
#ifdef CONFIG_ASCEND_SVSP
	struct iommu_sva *svsp_sva;
#endif
	struct list_head entry;
};

struct svm_device {
	unsigned long long	id;
	struct miscdevice	miscdev;
	struct device		*dev;
	phys_addr_t l2buff;
	unsigned long		l2size;
	struct list_head	entry;
	/* All modifications happen during module loading and unloading */
	struct list_head	child_list;
};

STATIC LIST_HEAD(sdev_list);

struct svm_bind_process {
	pid_t			vpid;
	u64			ttbr;
	u64			tcr;
	int			pasid;
	u32			flags;
};

/*
 *svm_process is released in svm_notifier_release() when mm refcnt
 *goes down zero. We should access svm_process only in the context
 *where mm_struct is valid, which means we should always get mm
 *refcnt first.
 */
struct svm_process {
	struct pid		 *tgid;
	struct mm_struct	 *mm;
	unsigned long		 asid;
	struct rb_node		 rb_node;
	struct mmu_notifier	 notifier;
	/* For postponed release  */
	struct rcu_head		 rcu;
	int			 pasid;
	struct mutex		 mutex;
	struct rb_root		 sdma_list;
	struct svm_device	 *sdev;
	struct list_head	 sva_list;
#ifdef CONFIG_ASCEND_SVSP
	struct mm_struct	 *svsp;
#endif
};

struct svm_sdma {
	struct rb_node node;
	unsigned long addr;
	int nr_pages;
	struct page **pages;
	atomic64_t ref;
};

struct svm_proc_mem {
	u32 dev_id;
	u32 len;
	u64 pid;
	u64 vaddr;
	u64 buf;
};

struct spalloc {
	unsigned long addr;
	unsigned long size;
	unsigned long flag;
};

struct addr_trans_args {
	unsigned long vptr;
	unsigned long *pptr;
	unsigned int device_id;
};

STATIC struct bus_type svm_bus_type = {
	.name		= "svm_bus",
};

STATIC char *svm_cmd_to_string(unsigned int cmd)
{
	switch (cmd) {
	case SVM_IOCTL_PROCESS_BIND:
		return "bind";
	case SVM_IOCTL_GET_PHYS:
		return "get phys";
	case SVM_IOCTL_SET_RC:
		return "set rc";
	case SVM_IOCTL_PIN_MEMORY:
		return "pin memory";
	case SVM_IOCTL_UNPIN_MEMORY:
		return "unpin memory";
	case SVM_IOCTL_REMAP_PROC:
		return "remap proc";
	case SVM_IOCTL_LOAD_FLAG:
		return "load flag";
	case SVM_IOCTL_RELEASE_PHYS32:
		return "release phys";
	default:
		return "unsupported";
	}

	return NULL;
}

/*
 * image word of slot
 * SVM_IMAGE_WORD_INIT: initial value, indicating that the slot is not used.
 * SVM_IMAGE_WORD_VALID: valid data is filled in the slot
 * SVM_IMAGE_WORD_DONE: the DMA operation is complete when the TS uses this address,
                        so, this slot can be freed.
 */
#define SVM_IMAGE_WORD_INIT	0x0
#define SVM_IMAGE_WORD_VALID	0xaa55aa55
#define SVM_IMAGE_WORD_DONE	0x55ff55ff

/*
 * The length of this structure must be 64 bytes, which is the agreement with the TS.
 * And the data type and sequence cannot be changed, because the TS core reads data
 * based on the data type and sequence.
 * image_word: slot status. For details, see SVM_IMAGE_WORD_xxx
 * pid: pid of process which ioctl svm device to get physical addr, it is used for
        verification by TS.
 * data_type: used to determine the data type by TS. Currently, data type must be
              SVM_VA2PA_TYPE_DMA.
 * char data[48]: for the data type SVM_VA2PA_TYPE_DMA, the DMA address is stored.
 */
struct svm_va2pa_slot {
	int image_word;
	int resv;
	int pid;
	int data_type;
	union {
		char user_defined_data[48];
		struct {
			unsigned long phys;
			unsigned long len;
			char reserved[32];
		};
	};
};

/* Note: the following definitions are for ascend rc scenario.
 *
 * TS driver will allocate 15 huge pages after it is setup ready, and put their
 * physic addresses in the addr[] of a ts_share_mem struct.
 *
 * Bios reserves a 1k-size block to hold this ts_share_mem struct, which
 * locates at (end - 9k).
 *
 * */
#define TASKID_SHARE_MEM_SIZE		1024
#define TASKID_SHARE_MEM_BLOCK_NUM	16
#define TASKID_SHARE_MEM_BLOCK_SIZE	(2 * 1024UL * 1024UL)
#define TASKID_SHARE_MEM_MAGIC		0xABCD
#define TS_MEM_REG	"ts dynamic"
#define TS_MEM_OFFSET 0x2400 // ts shmem is at (end - 9k)
struct ts_share_mem {
	u32 magic;
	u32 count;
	u64 addr[TASKID_SHARE_MEM_BLOCK_NUM];
};

struct svm_va2pa_trunk {
	struct svm_va2pa_slot *slots;
	int slot_total;
	int slot_used;
	unsigned long *bitmap;
	struct mutex mutex;
	phys_addr_t base;
	unsigned long size;
	int last_done_slot;
	int bitmap_cursor;

	int count; /* ts hugepage count */
	struct page *ts_share_mem_hugepages[TASKID_SHARE_MEM_BLOCK_NUM]; /* ts hugepages */
};

#define SVM_VA2PA_TRUNK_SIZE_MAX	0x3200000
#define SVM_VA2PA_MEMORY_ALIGN		64
#define SVM_VA2PA_SLOT_SIZE		sizeof(struct svm_va2pa_slot)
#define SVM_VA2PA_TYPE_DMA		0x1
#define SVM_MEM_REG			"va2pa trunk"
#define SVM_VA2PA_CLEAN_BATCH_NUM	0x80
#define SVM_VA2PA_TRUNK_COUNT_MAX	0x8

STATIC struct svm_va2pa_trunk va2pa_trunk[SVM_VA2PA_TRUNK_COUNT_MAX];

#define SVM_SMMU_NAME_LEN_MAX 16U

STATIC struct device_node *svm_find_mem_reg_node(struct device *dev, const char *compat)
{
	int index = 0;
	struct device_node *tmp = NULL;
	struct device_node *np = dev->of_node;

	for (; ; index++) {
		tmp = of_parse_phandle(np, "memory-region", index);
		if (!tmp)
			break;

		if (of_device_is_compatible(tmp, compat))
			return tmp;

		of_node_put(tmp);
	}

	return NULL;
}

STATIC int svm_parse_trunk_memory(struct device *dev)
{
	int err, count;
	struct resource r;
	struct device_node *trunk = NULL;

	/* va2pa trunk dynamically set by ts driver */
	trunk = svm_find_mem_reg_node(dev, TS_MEM_REG);
	if (trunk) {
		ts_shmem_need_init = true;
		goto parse_res;
	}

	/* va2pa trunk reserved by bios */
	trunk = svm_find_mem_reg_node(dev, SVM_MEM_REG);
	if (!trunk) {
		dev_err(dev, "Didn't find reserved memory\n");
		return -EINVAL;
	}

parse_res:
	for (count = 0; count < SVM_VA2PA_TRUNK_COUNT_MAX; count++) {
		err = of_address_to_resource(trunk, count, &r);
		if (err)
			break;

		va2pa_trunk[count].base = r.start;
		va2pa_trunk[count].size = resource_size(&r);
	}

	if (!count) {
		dev_err(dev, "Couldn't address to resource for reserved memory\n");
		return -ENODEV;
	}

	return 0;
}

STATIC int __svm_setup_trunk(struct device *dev, struct svm_va2pa_trunk *trunk)
{
	int slot_total;
	unsigned long *bitmap = NULL;
	struct svm_va2pa_slot *slot = NULL;
	phys_addr_t base = trunk->base;
	unsigned long size = trunk->size;
	int count = trunk->count;

	if (!IS_ALIGNED(base, SVM_VA2PA_MEMORY_ALIGN)) {
		dev_err(dev, "Didn't aligned to %u\n", SVM_VA2PA_MEMORY_ALIGN);
		return -EINVAL;
	}

	if (!ts_shmem_need_init &&
			((size == 0) || (size > SVM_VA2PA_TRUNK_SIZE_MAX))) {
		dev_err(dev, "Size of reserved memory is not right\n");
		return -EINVAL;
	}

	slot_total = size / SVM_VA2PA_SLOT_SIZE;
	if (slot_total < BITS_PER_LONG)
		return -EINVAL;

	bitmap = kvcalloc(ALIGN(slot_total, BITS_PER_LONG) / BITS_PER_LONG,
		sizeof(unsigned long), GFP_KERNEL);
	if (!bitmap) {
		dev_err(dev, "alloc memory failed\n");
		return -ENOMEM;
	}

	if (ts_shmem_need_init)
		slot = vmap_hugepage(trunk->ts_share_mem_hugepages,
				count, VM_MAP, __pgprot(PROT_NORMAL_NC));
	else
		slot = ioremap(base, size);

	if (!slot) {
		kvfree(bitmap);
		dev_err(dev, "Ioremap trunk failed\n");
		return -ENXIO;
	}

	trunk->slots = slot;
	trunk->slot_used = 0;
	trunk->slot_total = slot_total;
	trunk->bitmap = bitmap;
	trunk->last_done_slot = -1;
	trunk->bitmap_cursor = 0;
	mutex_init(&trunk->mutex);

	return 0;
}

STATIC int svm_setup_trunk(struct device *dev)
{
	int err = 0;
	int count;

	for (count = 0; count < SVM_VA2PA_TRUNK_COUNT_MAX; count++) {
		if (!va2pa_trunk[count].base)
			break;

		err = __svm_setup_trunk(dev, &va2pa_trunk[count]);
		if (err)
			break;
	}

	return err;
}

STATIC void svm_remove_trunk(struct device *dev)
{
	int count;

	for (count = 0; count < SVM_VA2PA_TRUNK_COUNT_MAX; count++) {
		if (!va2pa_trunk[count].base)
			break;

		iounmap(va2pa_trunk[count].slots);
		kvfree(va2pa_trunk[count].bitmap);
		va2pa_trunk[count].slots = NULL;
		va2pa_trunk[count].bitmap = NULL;
	}
}

STATIC void svm_set_slot_valid(struct svm_va2pa_trunk *trunk, unsigned long index,
		unsigned long phys, unsigned long len)
{
	struct svm_va2pa_slot *slot = &trunk->slots[index];

	slot->phys = phys;
	slot->len = len;
	slot->image_word = SVM_IMAGE_WORD_VALID;
	slot->pid = current->tgid;
	slot->data_type = SVM_VA2PA_TYPE_DMA;
	__bitmap_set(trunk->bitmap, index, 1);
	trunk->slot_used++;
}

STATIC void svm_set_slot_init(struct svm_va2pa_trunk *trunk, unsigned long index)
{
	struct svm_va2pa_slot *slot = &trunk->slots[index];

	slot->image_word = SVM_IMAGE_WORD_INIT;
	__bitmap_clear(trunk->bitmap, index, 1);
	trunk->slot_used--;
}

STATIC void svm_clean_one_slot(struct svm_va2pa_trunk *trunk, long index)
{
	phys_addr_t addr = (phys_addr_t)trunk->slots[index].phys;
	put_page(pfn_to_page(PHYS_PFN(addr)));
	svm_set_slot_init(trunk, index);
}

STATIC void svm_clean_done_slots(struct svm_va2pa_trunk *trunk)
{
	int used = trunk->slot_used;
	int count = 0;
	long temp = trunk->last_done_slot;
	unsigned long *bitmap = trunk->bitmap;

	for (; count < used && count < SVM_VA2PA_CLEAN_BATCH_NUM;) {
		temp = find_next_bit(bitmap, trunk->slot_total, temp + 1);
		if (temp == trunk->slot_total) {
			temp = -1;
			continue;
		}

		count++;
		if (trunk->slots[temp].image_word != SVM_IMAGE_WORD_DONE)
			continue;

		svm_clean_one_slot(trunk, temp);
	}
	trunk->last_done_slot = temp;
}

STATIC void __svm_procss_exit_clean_slots(struct svm_va2pa_trunk *trunk, int pid)
{
	unsigned long *bitmap;
	int used, i;
	int temp = -1;
	int count = 0;
	struct task_struct *tsk = NULL;

	if (!trunk->slot_used)
		return;

	mutex_lock(&trunk->mutex);

	tsk = get_pid_task(find_vpid(pid), PIDTYPE_PID);
	if (tsk) {
		if (!(tsk->flags & PF_EXITING)) {
			put_task_struct(tsk);
			mutex_unlock(&trunk->mutex);
			return;
		}
		put_task_struct(tsk);
	}

	used = trunk->slot_used;
	bitmap = trunk->bitmap;
	for (i = 0; i < used ; i++) {
		temp = find_next_bit(bitmap, trunk->slot_total, temp + 1);
		if (temp == trunk->slot_total)
			break;

		if (trunk->slots[temp].pid != pid)
			continue;

		count++;
		svm_clean_one_slot(trunk, temp);
	}
	mutex_unlock(&trunk->mutex);

	if (count)
		pr_info("svm process clean slot %d\n", count);
}

/*
 * The TS driver invokes this interface when the TS has used
 * the resources related to this process.
 */
void svm_process_exit_clean_slots(int pid)
{
	int i;

	for (i = 0; i < SVM_VA2PA_TRUNK_COUNT_MAX; i++) {
		if (!va2pa_trunk[i].base)
			break;

		__svm_procss_exit_clean_slots(&va2pa_trunk[i], pid);
	}
}
EXPORT_SYMBOL_GPL(svm_process_exit_clean_slots);

STATIC int svm_find_slot_init(struct svm_va2pa_trunk *trunk, unsigned long *index)
{
	int temp;
	unsigned long *bitmap = trunk->bitmap;

	temp = find_next_zero_bit(bitmap, trunk->slot_total, trunk->bitmap_cursor);
	if (temp == trunk->slot_total) {
		temp = find_first_zero_bit(bitmap, trunk->slot_total);
		if (temp == trunk->slot_total) {
			trunk->bitmap_cursor = 0;
			return -ENOSPC;
		}
	}

	trunk->bitmap_cursor = temp;
	*index = temp;
	return 0;
}

#ifndef DEV_SVM_UT
STATIC int svm_parse_trunk_memory_dynamic(struct device *dev)
{
	int i;
	struct ts_share_mem *idx_addr = NULL;
	u64 block_pa;
	struct page *hugepage;
	u64 shm_addr = va2pa_trunk[0].base + va2pa_trunk[0].size - TS_MEM_OFFSET;

	idx_addr = ioremap(shm_addr, TASKID_SHARE_MEM_SIZE);
	if (idx_addr == NULL) {
		dev_err(dev, "ioremap ts addr block failed!\n");
		return -ENXIO;
	}

	if (idx_addr->magic != TASKID_SHARE_MEM_MAGIC) {
		dev_err(dev, "ts addr magic check failed: %x\n", idx_addr->magic);
		iounmap(idx_addr);
		return -ENXIO;
	}

	if (idx_addr->count > TASKID_SHARE_MEM_BLOCK_NUM) {
		dev_err(dev, "ts addr count check failed: %d\n", idx_addr->count);
		iounmap(idx_addr);
		return -ENXIO;
	}

	for (i = 0; i < idx_addr->count; i++) {
		block_pa = idx_addr->addr[i];
		if (!block_pa) {
			dev_err(dev, "addr %d is NULL", i);
			iounmap(idx_addr);
			return -ENXIO;
		}
		hugepage = pfn_to_page(block_pa >> PAGE_SHIFT);
		va2pa_trunk[0].ts_share_mem_hugepages[i] = hugepage;
	}

	va2pa_trunk[0].count = idx_addr->count;
	va2pa_trunk[0].size = va2pa_trunk[0].count * TASKID_SHARE_MEM_BLOCK_SIZE;
	va2pa_trunk[0].base = shm_addr;

	iounmap(idx_addr);
	return 0;
}
#else
STATIC int svm_parse_trunk_memory_dynamic(struct device *dev)
{
	return 0;
}
#endif

STATIC int svm_va2pa_trunk_init_dynamic(struct device *dev)
{
	int err;

	err = svm_parse_trunk_memory_dynamic(dev);
	if (err)
		return err;

	return svm_setup_trunk(dev);
}

STATIC int svm_va2pa_trunk_init(struct device *dev)
{
	int err;

	err = svm_parse_trunk_memory(dev);
	if (err)
		return err;

	/* If va2pa trunk is dynamically set by ts driver, time order is:
	 * svm driver ready -> ts driver ready -> ts allocate hugepages -> svm setup va2pa trunk
	 * So do not setup trunk in svm_probe stage. */
	return ts_shmem_need_init ? 0 : svm_setup_trunk(dev);
}

STATIC struct svm_process *find_svm_process(unsigned long asid)
{
	struct rb_node *node = svm_process_root.rb_node;

	while (node) {
		struct svm_process *process = NULL;

		process = rb_entry(node, struct svm_process, rb_node);
		if (asid < process->asid)
			node = node->rb_left;
		else if (asid > process->asid)
			node = node->rb_right;
		else
			return process;
	}

	return NULL;
}

STATIC void insert_svm_process(struct svm_process *process)
{
	struct rb_node **p = &svm_process_root.rb_node;
	struct rb_node *parent = NULL;

	while (*p) {
		struct svm_process *tmp_process = NULL;

		parent = *p;
		tmp_process = rb_entry(parent, struct svm_process, rb_node);
		if (process->asid < tmp_process->asid) {
			p = &(*p)->rb_left;
		} else if (process->asid > tmp_process->asid) {
			p = &(*p)->rb_right;
		} else {
			WARN_ON_ONCE("asid already in the tree");
			return;
		}
	}

	rb_link_node(&process->rb_node, parent, p);
	rb_insert_color(&process->rb_node, &svm_process_root);
}

STATIC void delete_svm_process(struct svm_process *process)
{
	rb_erase(&process->rb_node, &svm_process_root);
	RB_CLEAR_NODE(&process->rb_node);
}

STATIC struct svm_device *file_to_sdev(struct file *file)
{
	return container_of(file->private_data,
			struct svm_device, miscdev);
}

STATIC int svm_open(struct inode *inode, struct file *file)
{
	return 0;
}


/* common funciton */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
STATIC int svm_enable_sva(struct device *dev)
{
	int err;

	pr_debug("enable sva for device %s.\n", dev_name(dev));

	err = iommu_dev_enable_feature(dev, IOMMU_DEV_FEAT_IOPF);
	if (err) {
		dev_err(dev, "failed to enable iopf feature, errno: %d.\n", err);
		goto out;
	}

	err = iommu_dev_enable_feature(dev, IOMMU_DEV_FEAT_SVA);
	if (err) {
		dev_err(dev, "failed to enable sva feature, errno: %d.\n", err);
		goto iopf_disable;
	}

#ifdef CONFIG_ASCEND_SVSP
	err = iommu_dev_enable_feature(dev, IOMMU_DEV_FEAT_SVSP);
	if (err) {
		dev_err(dev, "failed to enable svsp feature, errno: %d.\n", err);
		goto sva_disable;
	}
#endif

	return 0;

#ifdef CONFIG_ASCEND_SVSP
sva_disable:
	iommu_dev_disable_feature(dev, IOMMU_DEV_FEAT_SVA);
#endif

iopf_disable:
	iommu_dev_disable_feature(dev, IOMMU_DEV_FEAT_IOPF);

out:
	return err;
}

STATIC void svm_disable_sva(struct device *dev)
{
#ifdef CONFIG_ASCEND_SVSP
	iommu_dev_disable_feature(dev, IOMMU_DEV_FEAT_SVSP);
#endif
	iommu_dev_disable_feature(dev, IOMMU_DEV_FEAT_SVA);
	iommu_dev_disable_feature(dev, IOMMU_DEV_FEAT_IOPF);

	pr_debug("disable sva for device %s.\n", dev_name(dev));
}
#else /* kernel 4.19 */
STATIC int svm_enable_sva(struct device *dev)
{
	int err;
	unsigned long features = 0;

	pr_debug("enable sva for device %s.\n", dev_name(dev));

	features = IOMMU_SVA_FEAT_IOPF;
#ifdef CONFIG_ASCEND_SVSP
	features |= IOMMU_SVA_FEAT_SVSP;
#endif
	err = iommu_sva_device_init(dev, features, UINT_MAX, 0);
	if (err)
		dev_err(dev, "failed to enable sva feature, errno: %d.\n", err);

	return err;
}

STATIC void svm_disable_sva(struct device *dev)
{
	iommu_sva_device_shutdown(dev);

	pr_debug("disable sva for device %s.\n", dev_name(dev));
}
#endif

/* dev_slot function */
STATIC struct dev_slot *svm_dev_slot_find(u32 key)
{
	struct svm_device *sdev;
	struct dev_slot *dev_slot;

	list_for_each_entry(sdev, &sdev_list, entry)
		list_for_each_entry(dev_slot, &sdev->child_list, entry)
			if (dev_slot->key == key)
				return dev_slot;

	return NULL;
}

STATIC int svm_dev_slot_ops_prepare(struct dev_slot *dev_slot)
{
	if (dev_slot && dev_slot->ops && dev_slot->ops->prepare)
		return dev_slot->ops->prepare(dev_slot->dev);

	return 0;
}

STATIC void svm_dev_slot_ops_clean(struct dev_slot *dev_slot)
{
	if (dev_slot && dev_slot->ops && dev_slot->ops->clean)
		dev_slot->ops->clean(dev_slot->dev);
}

STATIC int svm_dev_slot_ops_bind(struct dev_slot *dev_slot, void *data)
{
	if (dev_slot && dev_slot->ops && dev_slot->ops->bind)
		return dev_slot->ops->bind(dev_slot->dev, data);

	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
STATIC int svm_dev_slot_ops_set_mpam(struct dev_slot *dev_slot, void *data)
{
	if (dev_slot && dev_slot->ops && dev_slot->ops->bind)
		return dev_slot->ops->bind(dev_slot->dev, data);

	return 0;
}

STATIC int svm_dev_slot_ops_get_mpam(struct dev_slot *dev_slot, void *data)
{
	if (dev_slot && dev_slot->ops && dev_slot->ops->bind)
		return dev_slot->ops->bind(dev_slot->dev, data);

	return 0;
}
#endif

STATIC inline int __svm_register_device_locked(struct dev_slot *dev_slot,
		 struct dev_slot_ops *ops, struct device *dev)
{
	int err;
	struct dev_slot tmp_slot;

	if (dev_slot->dev) {
		pr_err("register %s failed. the slot has been registered by %s.\n",
				dev_name(dev), dev_name(dev_slot->dev));
		return -EBUSY;
	}

	get_device(dev);
	tmp_slot.ops = ops;
	tmp_slot.dev = dev;
	err = svm_dev_slot_ops_prepare(&tmp_slot);
	if (err) {
		pr_err("register %s failed. prepare device failed.\n", dev_name(dev));
		put_device(dev);
		return err;
	}
	dev_slot->ops = ops;
	dev_slot->dev = dev;

	pr_debug("register %s with %u success.\n", dev_name(dev), dev_slot->key);

	return 0;
}

STATIC int __svm_register_device(struct dev_slot *dev_slot,
		struct dev_slot_ops *ops, struct device *dev)
{
	int err;

	down_write(&svm_register_sem);
	err = __svm_register_device_locked(dev_slot, ops, dev);
	up_write(&svm_register_sem);

	return err;
}

STATIC int svm_register_device(struct device *dev, struct dev_slot_ops *ops, u32 key)
{
	struct dev_slot *dev_slot;

	dev_slot = svm_dev_slot_find(key);
	if (!dev_slot) {
		pr_err("register %s failed. Invalid key(%d).\n", dev_name(dev), key);
		return -EINVAL;
	}

	return __svm_register_device(dev_slot, ops, dev);
}

STATIC inline int __svm_unregister_device_locked(struct dev_slot *dev_slot, struct device *dev)
{
	struct dev_slot tmp_slot;

	if (dev_slot->dev != dev) {
		pr_err("unregister %s failed. key not matched.\n", dev_name(dev));
		return -EINVAL;
	}

	tmp_slot.ops = dev_slot->ops;
	tmp_slot.dev = dev_slot->dev;
	dev_slot->ops = NULL;
	dev_slot->dev = NULL;
	svm_dev_slot_ops_clean(&tmp_slot);
	put_device(dev); /* get_device in __svm_register_device_locked */

	pr_debug("unregister %s with %u success.\n", dev_name(dev), dev_slot->key);

	return 0;
}

STATIC int __svm_unregister_device(struct dev_slot *dev_slot, struct device *dev)
{
	int err;

	down_write(&svm_register_sem);
	err = __svm_unregister_device_locked(dev_slot, dev);
	up_write(&svm_register_sem);

	return err;
}

/* make sure all svas are unbind */
STATIC int svm_unregister_device(struct device *dev, u32 key)
{
	struct dev_slot *dev_slot;

	dev_slot = svm_dev_slot_find(key);
	if (!dev_slot) {
		pr_err("unregister %s failed. invalid key[%d].\n", dev_name(dev),  key);
		return -EINVAL;
	}

	return __svm_unregister_device(dev_slot, dev);
}

STATIC struct dev_slot *svm_add_dev_slot(struct svm_device *sdev)
{
	struct dev_slot *dev_slot;

	dev_slot = kzalloc(sizeof(*dev_slot), GFP_KERNEL);
	if (!dev_slot)
		return NULL;

	list_add(&dev_slot->entry, &sdev->child_list);

	return dev_slot;
}

STATIC void svm_remove_dev_slot(struct dev_slot *dev_slot)
{
	if (dev_slot->dev)
		svm_unregister_device(dev_slot->dev, dev_slot->key);

	list_del(&dev_slot->entry);
	kfree(dev_slot);
}

STATIC void svm_remove_dev_slots(struct svm_device *sdev)
{
	struct dev_slot *pos = NULL;
	struct dev_slot *next = NULL;

	list_for_each_entry_safe(pos, next, &sdev->child_list, entry) {
		svm_remove_dev_slot(pos);
	}
}

STATIC struct svm_sdma *svm_find_sdma(struct svm_process *process,
				unsigned long addr, int nr_pages)
{
	struct rb_node *node = process->sdma_list.rb_node;

	while (node) {
		struct svm_sdma *sdma = NULL;

		sdma = rb_entry(node, struct svm_sdma, node);
		if (addr < sdma->addr)
			node = node->rb_left;
		else if (addr > sdma->addr)
			node = node->rb_right;
		else if (nr_pages < sdma->nr_pages)
			node = node->rb_left;
		else if (nr_pages > sdma->nr_pages)
			node = node->rb_right;
		else
			return sdma;
	}

	return NULL;
}

STATIC int svm_insert_sdma(struct svm_process *process, struct svm_sdma *sdma)
{
	struct rb_node **p = &process->sdma_list.rb_node;
	struct rb_node *parent = NULL;

	while (*p) {
		struct svm_sdma *tmp_sdma = NULL;

		parent = *p;
		tmp_sdma = rb_entry(parent, struct svm_sdma, node);
		if (sdma->addr < tmp_sdma->addr) {
			p = &(*p)->rb_left;
		} else if (sdma->addr > tmp_sdma->addr) {
			p = &(*p)->rb_right;
		} else if (sdma->nr_pages < tmp_sdma->nr_pages) {
			p = &(*p)->rb_left;
		} else if (sdma->nr_pages > tmp_sdma->nr_pages) {
			p = &(*p)->rb_right;
		} else {
			/*
			 * add reference count and return -EBUSY
			 * to free former alloced one.
			 */
			atomic64_inc(&tmp_sdma->ref);
			return -EBUSY;
		}
	}

	rb_link_node(&sdma->node, parent, p);
	rb_insert_color(&sdma->node, &process->sdma_list);

	return 0;
}

STATIC void svm_remove_sdma(struct svm_process *process,
			    struct svm_sdma *sdma, bool try_rm)
{
	int null_count = 0;

	if (try_rm && (!atomic64_dec_and_test(&sdma->ref)))
		return;

	rb_erase(&sdma->node, &process->sdma_list);
	RB_CLEAR_NODE(&sdma->node);

	while (sdma->nr_pages--) {
		if (sdma->pages[sdma->nr_pages] == NULL) {
			pr_err("null pointer, nr_pages:%d.\n", sdma->nr_pages);
			null_count++;
			continue;
		}

		put_page(sdma->pages[sdma->nr_pages]);
	}

	if (null_count)
		dump_stack();

	kvfree(sdma->pages);
	kfree(sdma);
}

STATIC int svm_pin_pages(unsigned long addr, int nr_pages,
			 struct page **pages)
{
	int err;

	err = get_user_pages_fast(addr, nr_pages, 1, pages);
	if (err > 0 && err < nr_pages) {
		while (err--)
			put_page(pages[err]);
		err = -EFAULT;
	} else if (err == 0) {
		err = -EFAULT;
	}

	return err;
}

STATIC int svm_add_sdma(struct svm_process *process,
			unsigned long addr, unsigned long size)
{
	int err;
	struct svm_sdma *sdma = NULL;

	sdma = kzalloc(sizeof(struct svm_sdma), GFP_KERNEL);
	if (sdma == NULL)
		return -ENOMEM;

	atomic64_set(&sdma->ref, 1);
	sdma->addr = addr & PAGE_MASK;
	sdma->nr_pages = (PAGE_ALIGN(size + addr) >> PAGE_SHIFT) -
			 (sdma->addr >> PAGE_SHIFT);
	sdma->pages = kvcalloc(sdma->nr_pages, sizeof(char *), GFP_KERNEL);
	if (sdma->pages == NULL) {
		err = -ENOMEM;
		goto err_free_sdma;
	}

	/*
	 * If always pin the same addr with the same nr_pages, pin pages
	 * maybe should move after insert sdma with mutex lock.
	 */
	err = svm_pin_pages(sdma->addr, sdma->nr_pages, sdma->pages);
	if (err < 0) {
		pr_err("%s: failed to pin pages addr 0x%pK, size 0x%lx\n",
		       __func__, (void *)addr, size);
		goto err_free_pages;
	}

	err = svm_insert_sdma(process, sdma);
	if (err < 0) {
		err = 0;
		pr_debug("%s: sdma already exist!\n", __func__);
		goto err_unpin_pages;
	}

	return err;

err_unpin_pages:
	while (sdma->nr_pages--)
		put_page(sdma->pages[sdma->nr_pages]);
err_free_pages:
	kvfree(sdma->pages);
err_free_sdma:
	kfree(sdma);

	return err;
}

STATIC int svm_pin_memory(unsigned long __user *arg)
{
	int err;
	struct svm_process *process = NULL;
	unsigned long addr, size, asid;

	if (arg == NULL)
		return -EINVAL;

	if (get_user(addr, arg))
		return -EFAULT;

	if (get_user(size, arg + 1))
		return -EFAULT;

	if ((addr + size <= addr) || (size >= (u64)UINT_MAX) || (addr == 0))
		return -EINVAL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	asid = arm64_mm_context_get(current->mm);
#else
	asid = mm_context_get(current->mm);
#endif

	if (!asid)
		return -ENOSPC;

	mutex_lock(&svm_process_mutex);
	process = find_svm_process(asid);
	if (process == NULL) {
		mutex_unlock(&svm_process_mutex);
		err = -ESRCH;
		goto out;
	}
	mutex_unlock(&svm_process_mutex);

	mutex_lock(&process->mutex);
	err = svm_add_sdma(process, addr, size);
	mutex_unlock(&process->mutex);

out:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	arm64_mm_context_put(current->mm);
#else
	mm_context_put(current->mm);
#endif

	return err;
}

STATIC int svm_unpin_memory(unsigned long __user *arg)
{
	int err = 0, nr_pages;
	struct svm_sdma *sdma = NULL;
	unsigned long addr, size, asid;
	struct svm_process *process = NULL;

	if (arg == NULL)
		return -EINVAL;

	if (get_user(addr, arg))
		return -EFAULT;

	if (get_user(size, arg + 1))
		return -EFAULT;

	if (ULONG_MAX - addr < size)
		return -EINVAL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	asid = arm64_mm_context_get(current->mm);
#else
	asid = mm_context_get(current->mm);
#endif
	if (!asid)
		return -ENOSPC;

	nr_pages = (PAGE_ALIGN(size + addr) >> PAGE_SHIFT) -
		   ((addr & PAGE_MASK) >> PAGE_SHIFT);
	addr &= PAGE_MASK;

	mutex_lock(&svm_process_mutex);
	process = find_svm_process(asid);
	if (process == NULL) {
		mutex_unlock(&svm_process_mutex);
		err = -ESRCH;
		goto out;
	}
	mutex_unlock(&svm_process_mutex);

	mutex_lock(&process->mutex);
	sdma = svm_find_sdma(process, addr, nr_pages);
	if (sdma == NULL) {
		mutex_unlock(&process->mutex);
		err = -ESRCH;
		goto out;
	}

	svm_remove_sdma(process, sdma, true);
	mutex_unlock(&process->mutex);

out:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	arm64_mm_context_put(current->mm);
#else
	mm_context_put(current->mm);
#endif

	return err;
}

STATIC void svm_unpin_all(struct svm_process *process)
{
	struct rb_node *node = NULL;

	while ((node = rb_first(&process->sdma_list)))
		svm_remove_sdma(process,
				rb_entry(node, struct svm_sdma, node),
				false);
}

#ifdef CONFIG_ASCEND_SVSP
STATIC int svm_svsp_bind_core(struct device *dev, void *data,
		struct process_sva *proc_sva);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
unsigned int svm_svsp_extract_ssid_bits(void *master);
#endif
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
STATIC int svm_device_bind(struct device *dev, void *data)
{
	int err = 0;
	struct svm_process *process = data;
	struct process_sva *proc_sva;
	struct iommu_sva *sva;

	pr_debug("bind device %s.\n", dev_name(dev));

	proc_sva = kzalloc(sizeof(*proc_sva), GFP_KERNEL);
	if (!proc_sva) {
		return -ENOMEM;
	}

	sva = iommu_sva_bind_device(dev, current->mm);
	if (IS_ERR_OR_NULL(sva)) {
		pr_err("failed to bind device, err %ld\n", PTR_ERR(sva));
		err = PTR_ERR(sva);
		goto free_proc;
	}
	process->pasid = iommu_sva_get_pasid(sva);

#ifdef CONFIG_ASCEND_SVSP
	err = svm_svsp_bind_core(dev, data, proc_sva);
	if (err) {
		iommu_sva_unbind_device(sva);
		pr_err("svsp bind device failed, err %d\n", err);
		goto free_proc;
	}
#endif

	proc_sva->sva = sva;
	list_add(&proc_sva->entry, &process->sva_list);
	return 0;

free_proc:
	kfree(proc_sva);
	return err;
}
#else
STATIC int svm_device_bind(struct device *dev, void *data)
{
	struct svm_process *process = data;
	int err;

	err = iommu_sva_bind_device(dev, current->mm,
			 &process->pasid, IOMMU_SVA_FEAT_IOPF, NULL);
	if (err) {
		pr_err("failed to get the pasid\n");
		return err;
	}

#ifdef CONFIG_ASCEND_SVSP
	err = svm_svsp_bind_core(dev, data, NULL);
	if (err) {
		iommu_sva_unbind_device(dev, process->pasid);
		pr_err("svsp bind core failed\n");
		return err;
	}
#endif

	return 0;
}
#endif

STATIC void svm_bind_dev_slot(struct dev_slot *dev_slot, struct svm_process *process)
{
	down_read(&svm_register_sem);
	svm_dev_slot_ops_bind(dev_slot, process);
	up_read(&svm_register_sem);
}

STATIC void svm_bind_dev_slots(struct svm_process *process)
{
	struct dev_slot *pos = NULL;

	list_for_each_entry(pos, &process->sdev->child_list, entry) {
		svm_bind_dev_slot(pos, process);
	}
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
STATIC void svm_unbind_cores(struct svm_process *process)
{
	struct process_sva *proc_sva, *tmp;
	list_for_each_entry_safe(proc_sva, tmp, &process->sva_list, entry) {
		list_del(&proc_sva->entry);
		iommu_sva_unbind_device(proc_sva->sva);
#ifdef CONFIG_ASCEND_SVSP
		if (proc_sva->svsp_sva)
			iommu_sva_unbind_device(proc_sva->svsp_sva);
#endif
		kfree(proc_sva);
	}
}
#else
STATIC void svm_process_free(struct rcu_head *rcu)
{
	struct svm_process *process = NULL;

	process = container_of(rcu, struct svm_process, rcu);
	svm_unpin_all(process);
	mm_context_put(process->mm);
	kfree(process);
}
#endif

STATIC void svm_process_release(struct svm_process *process)
{
	delete_svm_process(process);
	put_pid(process->tgid);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	svm_unbind_cores(process);
	mmu_notifier_put(&process->notifier);
#else
	/*
	 * If we're being released from process exit, the notifier callback
	 * ->release has already been called. Otherwise we don't need to go
	 * through there, the process isn't attached to anything anymore. Hence
	 * no_release.
	 */
	mmu_notifier_unregister_no_release(&process->notifier, process->mm);

	/*
	 * We can't free the structure here, because ->release might be
	 * attempting to grab it concurrently. And in the other case, if the
	 * structure is being released from within ->release, then
	 * __mmu_notifier_release expects to still have a valid mn when
	 * returning. So free the structure when it's safe, after the RCU grace
	 * period elapsed.
	 */
	mmu_notifier_call_srcu(&process->rcu, svm_process_free);
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
STATIC void svm_free_notifier(struct mmu_notifier *subscription)
{
	struct svm_process *process = NULL;

	process = container_of(subscription, struct svm_process, notifier);
	svm_unpin_all(process);
	arm64_mm_context_put(process->mm);
	kfree(process);
}
#endif

STATIC void svm_notifier_release(struct mmu_notifier *mn,
					struct mm_struct *mm)
{
	struct svm_process *process = NULL;

	process = container_of(mn, struct svm_process, notifier);

	/*
	 * No need to call svm_unbind_cores(), as iommu-sva will do the
	 * unbind in its mm_notifier callback.
	 */

	mutex_lock(&svm_process_mutex);
	svm_process_release(process);
	mutex_unlock(&svm_process_mutex);
}

STATIC struct mmu_notifier_ops svm_process_mmu_notifier = {
	.release	= svm_notifier_release,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	.free_notifier = svm_free_notifier,
#endif
};

STATIC struct svm_process *
svm_process_alloc(struct svm_device *sdev, struct pid *tgid,
		struct mm_struct *mm, unsigned long asid)
{
	struct svm_process *process = kzalloc(sizeof(*process), GFP_ATOMIC);

	if (!process)
		return ERR_PTR(-ENOMEM);

	process->sdev = sdev;
	process->tgid = tgid;
	process->mm = mm;
	process->asid = asid;
	process->sdma_list = RB_ROOT; //lint !e64
	mutex_init(&process->mutex);
	INIT_LIST_HEAD(&process->sva_list);
	process->notifier.ops = &svm_process_mmu_notifier;

	return process;
}

STATIC struct task_struct *svm_get_task_by_vpid(pid_t vpid)
{
	struct task_struct *task;
	struct mm_struct *mm;

	int err;

	task = get_pid_task(find_vpid(vpid), PIDTYPE_PID);
	if (task == NULL)
		return ERR_PTR(-ESRCH);

	err = svm_init_runtime_symbols();
	if (err) {
		put_task_struct(task);
		return ERR_PTR(err);
	}

	/* check the permission */
	mm = svm_mm_access(task, PTRACE_MODE_ATTACH_REALCREDS);
	if (IS_ERR_OR_NULL(mm)) {
		pr_err("cannot access mm\n");
		put_task_struct(task);
		return ERR_PTR(-ESRCH);
	}
	mmput(mm);

	return task;
}

STATIC int __svm_process_bind(struct svm_device *sdev)
{
	int err;
	unsigned long asid;
	struct pid *tgid = NULL;
	struct svm_process *process = NULL;
	struct mm_struct *mm = current->mm;

	tgid = get_task_pid(current, PIDTYPE_TGID);
	if (tgid == NULL)
		return -EINVAL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	asid = arm64_mm_context_get(mm);
#else
	asid = mm_context_get(mm);
#endif
	if (!asid) {
		err = -ENOSPC;
		goto err_put_pid;
	}

	/* If a svm_process already exists, use it */
	mutex_lock(&svm_process_mutex);
	process = find_svm_process(asid);
	if (process == NULL) {
		process = svm_process_alloc(sdev, tgid, mm, asid);
		if (IS_ERR(process)) {
			err = PTR_ERR(process);
			mutex_unlock(&svm_process_mutex);
			goto err_put_mm_context;
		}
		err = mmu_notifier_register(&process->notifier, mm);
		if (err) {
			mutex_unlock(&svm_process_mutex);
			goto err_free_svm_process;
		}

		insert_svm_process(process);

		svm_bind_dev_slots(process);

		mutex_unlock(&svm_process_mutex);
	} else {
		mutex_unlock(&svm_process_mutex);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
		arm64_mm_context_put(mm);
#else
		mm_context_put(mm);
#endif
		put_pid(tgid);
	}

	return 0;

err_free_svm_process:
	kfree(process);
err_put_mm_context:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	arm64_mm_context_put(mm);
#else
	mm_context_put(mm);
#endif
err_put_pid:
	put_pid(tgid);

	return err;
}

STATIC int svm_process_bind(struct svm_device *sdev, void __user *arg)
{
	int err = -EINVAL;
	struct svm_bind_process params;

	err = copy_from_user(&params, arg, sizeof(params));
	if (err) {
		dev_err(sdev->dev, "fail to copy params %d\n", err);
		return -EFAULT;
	}

	if (params.flags)
		return -EINVAL;

	err = __svm_process_bind(sdev);
	if (err) {
		dev_err(sdev->dev, "failed to bind task %d\n", err);
		return err;
	}

	err = copy_to_user((void __user *)arg, &params,
			sizeof(params));
	if (err) {
		dev_err(sdev->dev, "failed to copy to user!\n");
		return -EFAULT;
	}

	return err;
}

static struct dev_slot_ops pci_device_ops = {
	.bind = svm_device_bind,
};

/* make sure sva is enabled before calling this function */
int svm_register_pci_device(struct device *dev, u32 key)
{
	if (!dev) {
		pr_err("register NULL device.\n");
		return -EINVAL;
	}

	return svm_register_device(dev, &pci_device_ops, key);
}
EXPORT_SYMBOL(svm_register_pci_device);

/* make sure device is unbind. */
int svm_unregister_pci_device(struct device *dev, u32 key)
{
	if (!dev) {
		pr_err("unregister NULL device.\n");
		return -EINVAL;
	}

	return svm_unregister_device(dev, key);
}
EXPORT_SYMBOL(svm_unregister_pci_device);

/* core device */
struct core_device {
	struct device		dev;
	struct iommu_group	*group;
	struct iommu_domain	*domain;
	u8			smmu_bypass;
	u8			enable_ttb1;
	char			smmu_name[SVM_SMMU_NAME_LEN_MAX];
};

STATIC INLINE struct core_device *to_core_device(struct device *dev)
{
	return container_of(dev, struct core_device, dev);
}

STATIC void cdev_device_release(struct device *dev)
{
	struct core_device *cdev = to_core_device(dev);

	kfree(cdev);
}

STATIC int svm_core_device_prepare(struct device *dev)
{
	struct core_device *cdev = to_core_device(dev);

	if (!cdev->smmu_bypass)
		return svm_enable_sva(dev);

	return 0;
}

STATIC void svm_core_device_clean(struct device *dev)
{
	struct core_device *cdev = to_core_device(dev);

	if (!cdev->smmu_bypass) {
		svm_disable_sva(dev);
		iommu_detach_group(cdev->domain, cdev->group);
		iommu_domain_free(cdev->domain);
		iommu_group_put(cdev->group);
	}

	device_unregister(&cdev->dev);
}

STATIC int svm_core_device_bind(struct device *dev, void *data)
{
	struct core_device *cdev = to_core_device(dev);

	if (cdev->smmu_bypass)
		return 0;

	return svm_device_bind(dev, data);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
STATIC int svm_set_core_mpam(struct device *dev, void *data);
STATIC int svm_get_core_mpam(struct device *dev, void *data);
#endif

static struct dev_slot_ops core_device_ops = {
	.prepare = svm_core_device_prepare,
	.clean = svm_core_device_clean,
	.bind = svm_core_device_bind,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
	.set_mpam = svm_set_core_mpam,
	.get_mpam = svm_get_core_mpam,
#endif
};

STATIC int __svm_register_core_device(struct dev_slot *dev_slot, struct device *dev)
{
	return __svm_register_device(dev_slot, &core_device_ops, dev);
}

#ifdef CONFIG_ACPI
STATIC int svm_acpi_new_core(struct svm_device *sdev,
		struct acpi_device *children, int id,
		struct device **pdev)
{
	int err;
	struct core_device *cdev = NULL;
	char *name = NULL;
	enum dev_dma_attr attr;
	const union acpi_object *obj = NULL;

	name = devm_kasprintf(sdev->dev, GFP_KERNEL, "svm_child_dev%d", id);
	if (name == NULL)
		return -ENOMEM;

	cdev = kzalloc(sizeof(*cdev), GFP_KERNEL);
	if (cdev == NULL)
		return -ENOMEM;

	cdev->dev.fwnode = &children->fwnode;
	cdev->dev.parent = sdev->dev;
	cdev->dev.bus = &svm_bus_type;
	cdev->dev.release = cdev_device_release;
	cdev->smmu_bypass = 0;
	dev_set_name(&cdev->dev, "%s", name);

	err = device_register(&cdev->dev);
	if (err) {
		dev_info(&cdev->dev, "core_device register failed\n");
		put_device(&cdev->dev);
		return err;
	}

	attr = device_get_dma_attr(&cdev->dev);
	if (attr != DEV_DMA_NOT_SUPPORTED) {
		err = acpi_dma_configure(&cdev->dev, attr);
		if (err) {
			dev_dbg(&cdev->dev, "acpi_dma_configure failed\n");
			goto dev_err;
		}
	}

	err = acpi_dev_get_property(children, "hisi,smmu-bypass",
			DEV_PROP_U8, &obj);
	if (err)
		dev_info(&children->dev, "read smmu bypass failed\n");

	if (obj)
		cdev->smmu_bypass = (u8)obj->integer.value;

	if (cdev->smmu_bypass) {
		err = svm_request_dm_for_dev(&cdev->dev);
		if (err)
			goto dev_err;
		return 0;
	}

	cdev->group = iommu_group_get(&cdev->dev);
	if (IS_ERR_OR_NULL(cdev->group)) {
		dev_err(&cdev->dev, "smmu is not right configured\n");
		err = -ENXIO;
		goto dev_err;
	}

	cdev->domain = svm_domain_alloc(&cdev->dev);
	if (cdev->domain == NULL) {
		dev_info(&cdev->dev, "failed to alloc domain\n");
		err = -ENOMEM;
		goto group_put;
	}

	err = iommu_attach_group(cdev->domain, cdev->group);
	if (err) {
		dev_err(&cdev->dev, "failed group to domain\n");
		goto domain_free;
	}

	*pdev = &cdev->dev;

	return 0;

domain_free:
	iommu_domain_free(cdev->domain);
group_put:
	iommu_group_put(cdev->group);
dev_err:
	device_unregister(&cdev->dev);
	return err;
}

STATIC int svm_acpi_init_core(struct svm_device *sdev)
{
	int err = 0;
	struct dev_slot *dev_slot;
	struct device *dev;
	struct acpi_device *adev = ACPI_COMPANION(sdev->dev);
	struct fwnode_handle *child = NULL;
	struct acpi_device *cdev = NULL;
	int id = 0;

	fwnode_for_each_available_child_node(acpi_fwnode_handle(adev), child) {
		cdev = to_acpi_device_node(child);
		if (!cdev)
			continue;

		dev_slot = svm_add_dev_slot(sdev);
		if (!dev_slot) {
			err = -ENOMEM;
			break;
		}

		err = fwnode_property_read_u32(&cdev->fwnode, "key", &dev_slot->key);
		if (err) { /* no key means this is ai core */
			dev = NULL;
			err = svm_acpi_new_core(sdev, cdev, id++, &dev);
			if (err)
				break;

			err = __svm_register_core_device(dev_slot, dev);
			if (err) {
				device_unregister(dev);
				break;
			}
		}
	}

	if (err)
		svm_remove_dev_slots(sdev);

	return err;
}

STATIC int svm_acpi_parse_trunk_memory(struct acpi_device *adev)
{
	int i, err;
	char acpi_name[200];
	const union acpi_object *propvalue;
	for (i = 0; i < SVM_VA2PA_TRUNK_COUNT_MAX; i++) {
		err = snprintf_s(acpi_name, sizeof(acpi_name), sizeof(acpi_name) - 1,
				"memory-region-va2pa-reg-base%d", i);
		if (err < 0)
			return -EINVAL;

		err = acpi_dev_get_property(adev, acpi_name, ACPI_TYPE_INTEGER, &propvalue);
		if (err) {
			break;
		}

		va2pa_trunk[i].base = propvalue->integer.value;

		err = snprintf_s(acpi_name, sizeof(acpi_name), sizeof(acpi_name) - 1,
				"memory-region-va2pa-reg-length%d", i);
		if (err < 0)
			return -EINVAL;

		err = acpi_dev_get_property(adev, acpi_name, ACPI_TYPE_INTEGER, &propvalue);
		if (err) {
			dev_err(&adev->dev, "read memory-region size failed, "
								  "memory type: %x\n", propvalue->type);
			return -EINVAL;
		}
		va2pa_trunk[i].size = propvalue->integer.value;
	}
	if (i == 0) {
		dev_err(&adev->dev, "unrecognize va2pa memory\n");
		return -EINVAL;
	}
	return 0;
}

STATIC int svm_acpi_va2pa_trunk_init(struct svm_device *sdev)
{
	struct device *dev = sdev->dev;
	struct acpi_device *adev = ACPI_COMPANION(sdev->dev);

	if (svm_acpi_parse_trunk_memory(adev)) {
		dev_err(&adev->dev, "parse trunk memory failed\n");
		return -EINVAL;
	}
	svm_setup_trunk(dev);
	dev_info(&adev->dev, "va2pa init success\n");
	return 0;
}
#else
STATIC int svm_acpi_init_core(struct svm_device *sdev) { return 0; }
STATIC int svm_acpi_va2pa_trunk_init(struct svm_device *sdev) { return 0; }
#endif

STATIC int svm_of_new_core(struct svm_device *sdev, struct device_node *np,
		struct device **pdev)
{
	int err;
	struct resource res;
	struct core_device *cdev = NULL;
	char *name = NULL;
	// the enumeration DOMAIN_ATTR_TTB1 is not defined on the helper.
#ifndef HELPER_KERNAL
	char *smmu_name = NULL;
#endif

	name = devm_kasprintf(sdev->dev, GFP_KERNEL, "svm%llu_%s",
			sdev->id, np->name);
	if (name == NULL)
		return -ENOMEM;

	cdev = kzalloc(sizeof(*cdev), GFP_KERNEL);
	if (cdev == NULL)
		return -ENOMEM;

	cdev->dev.of_node = np;
	cdev->dev.parent = sdev->dev;
	cdev->dev.bus = &svm_bus_type;
	cdev->dev.release = cdev_device_release;
	cdev->smmu_bypass = 0;
	dev_set_name(&cdev->dev, "%s", name);

	err = device_register(&cdev->dev);
	if (err) {
		dev_info(&cdev->dev, "core_device register failed\n");
		put_device(&cdev->dev);
		return err;
	}

	err = of_dma_configure(&cdev->dev, np, true);
	if (err) {
		dev_dbg(&cdev->dev, "of_dma_configure failed\n");
		goto dev_err;
	}

	err = of_address_to_resource(np, 0, &res);
	if (err) {
		dev_info(&cdev->dev, "no reg, FW should install the sid\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
		err = 0;
#endif
	} else {
		/* If the reg specified, install sid for the core */
		void __iomem *core_base = NULL;
#ifdef DEV_SVM_UT
		int sid = 1;
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
		int sid = cdev->dev.iommu->fwspec->ids[0];
#else
		int sid = cdev->dev.iommu_fwspec->ids[0];
#endif
#endif

		core_base = ioremap(res.start, resource_size(&res));
		if (core_base == NULL) {
			dev_err(&cdev->dev, "ioremap failed\n");
			err = -ENOMEM;
			goto dev_err;
		}

		writel_relaxed(sid, core_base + CORE_SID);
		iounmap(core_base);
	}

	cdev->smmu_bypass = of_property_read_bool(np, "hisi,smmu_bypass");

	/* If core device is smmu bypass, request direct map. */
	if (cdev->smmu_bypass) {
		err = svm_request_dm_for_dev(&cdev->dev);
		if (err)
			goto dev_err;

		*pdev = &cdev->dev;
		return 0;
	}

	cdev->group = iommu_group_get(&cdev->dev);
	if (IS_ERR_OR_NULL(cdev->group)) {
		dev_err(&cdev->dev, "smmu is not right configured\n");
		err = -ENXIO;
		goto dev_err;
	}

	cdev->domain = svm_domain_alloc(&cdev->dev);
	if (cdev->domain == NULL) {
		dev_info(&cdev->dev, "failed to alloc domain\n");
		err = -ENOMEM;
		goto group_put;
	}
	// the enumeration DOMAIN_ATTR_TTB1 is not defined on the helper.
#ifndef HELPER_KERNAL
	cdev->enable_ttb1 = of_property_read_bool(np, "enable-ttb1");
	if (cdev->enable_ttb1 == 1) {
#if defined(DOMAIN_ATTR_TTB1) && defined(DOMAIN_ATTR_GEOMETRY)
		int flag = 1;
		u64 iova_range[2] = {0};
		struct iommu_domain_geometry geometry;

		err = iommu_domain_set_attr(cdev->domain, DOMAIN_ATTR_TTB1, &flag);
		if (err != 0) {
			dev_err(&cdev->dev, "iommu_domain_set_attr DOMAIN_ATTR_TTB1 failed ret=%d", err);
			goto domain_free;
		}

		err = of_property_read_u64_array(np, "iova-range", iova_range, 2U);
		if (err != 0) {
			dev_err(&cdev->dev, "read iova-range failed ret=%d", err);
			goto domain_free;
		}
		geometry.aperture_start = iova_range[0];
		geometry.aperture_end = iova_range[0] + iova_range[1] - 1;
		geometry.force_aperture = true;
		err = iommu_domain_set_attr(cdev->domain, DOMAIN_ATTR_GEOMETRY, &geometry);
		if (err != 0) {
			dev_err(&cdev->dev, "iommu_domain_set_attr DOMAIN_ATTR_GEOMETRY failed ret=%d", err);
			goto domain_free;
		}
		pr_info("property read enable_ttb1\n");
#else
		dev_err(&cdev->dev, "enable-ttb1 requires source kernel smmu_ttb1 support\n");
		err = -EOPNOTSUPP;
		goto domain_free;
#endif
	}
	err = of_property_read_string(np, "smmu-name", (const char **)&smmu_name);
	if (err == 0) {
		err = strncpy_s(cdev->smmu_name, sizeof(cdev->smmu_name), smmu_name, strlen(smmu_name));
		if (err != 0) {
			dev_err(&cdev->dev, "strncpy_s smmu-name failed ret=%d", err);
			goto domain_free;
		}
	}
#endif

	err = iommu_attach_group(cdev->domain, cdev->group);
	if (err) {
		dev_err(&cdev->dev, "failed group to domain\n");
		goto domain_free;
	}

	*pdev = &cdev->dev;

	return 0;

domain_free:
	iommu_domain_free(cdev->domain);
group_put:
	iommu_group_put(cdev->group);
dev_err:
	device_unregister(&cdev->dev);
	return err;
}

STATIC int svm_dt_init_core(struct svm_device *sdev, struct device_node *np)
{
	int err = 0;
	struct dev_slot *dev_slot;
	struct device *dev;
	struct device_node *child = NULL;

	for_each_available_child_of_node(np, child) {
		dev_slot = svm_add_dev_slot(sdev);
		if (!dev_slot) {
			err = -ENOMEM;
			break;
		}

		err = fwnode_property_read_u32(of_fwnode_handle(child), "key", &dev_slot->key);
		if (err) { /* no key */
			dev = NULL;
			err = svm_of_new_core(sdev, child, &dev);
			if (err)
				break;

			err = __svm_register_core_device(dev_slot, dev);
			if (err) {
				device_unregister(dev);
				break;
			}
		}
	}

	if (err)
		svm_remove_dev_slots(sdev);

	return err;
}

STATIC pte_t *svm_get_pte(struct vm_area_struct *vma,
			  pud_t *pud,
			  unsigned long addr,
			  unsigned long *page_size,
			  unsigned long *offset)
{
	pte_t *pte = NULL;
	unsigned long size = 0;

	if (is_vm_hugetlb_page(vma)) {
		if (pud_present(*pud)) {
			if (pud_leaf(*pud)) {
				pte = (pte_t *)pud;
				*offset = addr & (PUD_SIZE - 1);
				size = PUD_SIZE;
			} else {
				pte = (pte_t *)pmd_offset(pud, addr);
				*offset = addr & (PMD_SIZE - 1);
				size = PMD_SIZE;
			}
		} else {
			pr_err("%s:hugetlb but pud not present\n", __func__);
		}
	} else {
		pmd_t *pmd = pmd_offset(pud, addr);

		if (pmd_none(*pmd))
			return NULL;

		if (pmd_trans_huge(*pmd)) {
			pte = (pte_t *)pmd;
			*offset = addr & (PMD_SIZE - 1);
			size = PMD_SIZE;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
		} else if (pmd_trans_unstable(pmd)) {
			pr_warn("%s: thp unstable\n", __func__);
#endif
		} else {
			pte = pte_offset_kernel(pmd, addr);
			*offset = addr & (PAGE_SIZE - 1);
			size = PAGE_SIZE;
		}
	}

	if (page_size)
		*page_size = size;

	return pte;
}

/* Must be called with mmap_sem held */
STATIC pte_t *svm_walk_pt(unsigned long addr, unsigned long *page_size,
			  unsigned long *offset)
{
	pgd_t *pgd = NULL;
	pud_t *pud = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	p4d_t *p4d = NULL;
#endif
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma = NULL;

	vma = find_vma(mm, addr);
	if (!vma)
		return NULL;

	pgd = pgd_offset(mm, addr);
	if (pgd_none(*pgd))
		return NULL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	p4d = p4d_offset(pgd, addr);
	if (p4d_none(*p4d))
		return NULL;

	pud = pud_offset(p4d, addr);
#else
	pud = pud_offset(pgd, addr);
#endif
	if (pud_none(*pud))
		return NULL;

	return svm_get_pte(vma, pud, addr, page_size, offset);
}

STATIC int svm_get_phys(unsigned long __user *arg)
{
	int err;
	pte_t *ptep = NULL;
	pte_t pte;
	unsigned long index = 0;
	struct page *page;
	struct addr_trans_args args;
	unsigned long addr, phys, offset;
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma = NULL;
	unsigned long len;
	unsigned int trunk_id;
	struct svm_va2pa_trunk *trunk;

	if (copy_from_user(&args, (void __user *)arg, sizeof(args)))
		return -EFAULT;

	addr = args.vptr;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	down_read(&mm->mmap_lock);
#else
	down_read(&mm->mmap_sem);
#endif
	ptep = svm_walk_pt(addr, NULL, &offset);
	if (!ptep) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
		up_read(&mm->mmap_lock);
#else
		up_read(&mm->mmap_sem);
#endif
		return -EINVAL;
	}

	pte = READ_ONCE(*ptep);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	if (!pte_present(pte) || !(pfn_in_present_section(pte_pfn(pte)))) {
		up_read(&mm->mmap_lock);
#else
	if (!pte_present(pte) || !(pfn_present(pte_pfn(pte)))) {
		up_read(&mm->mmap_sem);
#endif
		return -EINVAL;
	}

	page = pte_page(pte);
	get_page(page);

	phys = PFN_PHYS(pte_pfn(pte)) + offset;

	/* fix ts problem, which need the len to check out memory */
	len = 0;
	vma = find_vma(mm, addr);
	if (vma)
		len = vma->vm_end - addr;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	up_read(&mm->mmap_lock);
#else
	up_read(&mm->mmap_sem);
#endif

	trunk_id = args.device_id;
	if (trunk_id >= SVM_VA2PA_TRUNK_COUNT_MAX)
		return -EINVAL;
	trunk = &va2pa_trunk[trunk_id];
	mutex_lock(&trunk->mutex);
	svm_clean_done_slots(trunk);
	if (trunk->slot_used == trunk->slot_total) {
		err = -ENOSPC;
		goto err_mutex_unlock;
	}

	err = svm_find_slot_init(trunk, &index);
	if (err)
		goto err_mutex_unlock;

	svm_set_slot_valid(trunk, index, phys, len);

	err = put_user(index * SVM_VA2PA_SLOT_SIZE, (unsigned long __user *)args.pptr);
	if (err)
		goto err_slot_init;

	mutex_unlock(&trunk->mutex);
	return 0;

err_slot_init:
	svm_set_slot_init(trunk, index);
err_mutex_unlock:
	mutex_unlock(&trunk->mutex);
	put_page(page);
	return err;
}

// the enumeration DOMAIN_ATTR_TTB1 is not defined on the helper.
#ifndef HELPER_KERNAL
struct iommu_domain *svm_find_domain_by_name(char *smmu_name)
{
	struct dev_slot *dev_slot = NULL;
	struct svm_device *sdev = NULL;

	list_for_each_entry(sdev, &sdev_list, entry) {
		list_for_each_entry(dev_slot, &sdev->child_list, entry) {
			if ((dev_slot->dev != NULL) && (dev_slot->dev->bus == &svm_bus_type)) {
				struct core_device *cdev = to_core_device(dev_slot->dev);
				if (strncmp(smmu_name, cdev->smmu_name, SVM_SMMU_NAME_LEN_MAX) == 0) {
					return cdev->domain;
				}
			}
		}
	}
	return NULL;
}
EXPORT_SYMBOL_GPL(svm_find_domain_by_name);
#endif

int svm_get_pasid(pid_t vpid, int dev_id __maybe_unused)
{
	int pasid;
	unsigned long asid;
	struct task_struct *task = NULL;
	struct mm_struct *mm = NULL;
	struct svm_process *process = NULL;

	task = svm_get_task_by_vpid(vpid);
	if (IS_ERR(task))
		return PTR_ERR(task);

	mm = get_task_mm(task);
	if (mm == NULL) {
		pasid = -EINVAL;
		goto put_task;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	asid = arm64_mm_context_get(mm);
#else
	asid = mm_context_get(mm);
#endif

	if (!asid) {
		pasid = -ENOSPC;
		goto put_mm;
	}

	mutex_lock(&svm_process_mutex);
	process = find_svm_process(asid);
	mutex_unlock(&svm_process_mutex);
	if (process)
		pasid = process->pasid;
	else
		pasid = -ESRCH;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	arm64_mm_context_put(mm);
#else
	mm_context_put(mm);
#endif
put_mm:
	mmput(mm);
put_task:
	put_task_struct(task);

	return pasid;
}
EXPORT_SYMBOL_GPL(svm_get_pasid);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
struct svm_mpam {
#define SVM_GET_DEV_MPAM	(1 << 0)
#define SVM_SET_DEV_MPAM	(1 << 1)
#define SVM_GET_USER_MPAM_EN	(1 << 2)
#define SVM_SET_USER_MPAM_EN	(1 << 3)
	int flags;
	int pasid;
	int partid;
	int pmg;
	int s1mpam;
	int user_mpam_en;
};

STATIC int svm_get_core_mpam(struct device *dev, void *data)
{
	int err = 0;
	struct svm_mpam *mpam = data;

	if (!dev)
		return 0;

	if (mpam->flags & SVM_GET_DEV_MPAM) {
		err = arm_smmu_get_dev_mpam(dev, mpam->pasid, &mpam->partid,
				&mpam->pmg, &mpam->s1mpam);
		if (err) {
			dev_err(dev, "get mpam failed, %d\n", err);
			return err;
		}
	}

	if (mpam->flags & SVM_GET_USER_MPAM_EN) {
		err = arm_smmu_get_dev_user_mpam_en(dev, &mpam->user_mpam_en);
		if (err) {
			dev_err(dev, "set user_mpam_en failed, %d\n", err);
			return err;
		}
	}

	return err;
}

int __svm_get_mpam(struct svm_mpam *mpam)
{
	int err = 0;
	struct dev_slot *dev_slot = NULL;
	struct svm_device *sdev = NULL;

	list_for_each_entry(sdev, &sdev_list, entry) {
		list_for_each_entry(dev_slot, &sdev->child_list, entry) {
			err = svm_dev_slot_ops_get_mpam(dev_slot, mpam);
			if (err)
				return err;
		}
	}

	return 0;
}

STATIC int svm_set_core_mpam(struct device *dev, void *data)
{
	int err = 0;
	struct svm_mpam *mpam = data;

	if (!dev)
		return 0;

	if (mpam->flags & SVM_SET_DEV_MPAM) {
		err = arm_smmu_set_dev_mpam(dev, mpam->pasid, mpam->partid,
				mpam->pmg, mpam->s1mpam);
		if (err) {
			dev_err(dev, "set mpam failed, %d\n", err);
			return err;
		}
	}

	if (mpam->flags & SVM_SET_USER_MPAM_EN) {
		err = arm_smmu_set_dev_user_mpam_en(dev, mpam->user_mpam_en);
		if (err) {
			dev_err(dev, "set user_mpam_en failed, %d\n", err);
			return err;
		}
	}

	return 0;
}

STATIC int __svm_set_mpam(struct svm_mpam *mpam)
{
	int err = 0;
	struct dev_slot *dev_slot = NULL;
	struct svm_device *sdev = NULL;

	list_for_each_entry(sdev, &sdev_list, entry) {
		list_for_each_entry(dev_slot, &sdev->child_list, entry) {
			err = svm_dev_slot_ops_set_mpam(dev_slot, mpam);
			if (err)
				return err;
		}
	}

	return 0;
}

/**
 * svm_set_mpam() - set mpam configuration of all core device in smmu
 * @pasid: substream id
 * @partid: mpam partition id
 * @pmg: mpam pmg
 * @s1mpam: 0 for ste mpam, 1 for cd mpam
 */
int svm_set_mpam(int pasid, int partid, int pmg, int s1mpam)
{
	int err;
	struct svm_mpam mpam, old_mpam;

	old_mpam.flags = SVM_GET_DEV_MPAM;
	old_mpam.pasid = pasid;
	err = __svm_get_mpam(&old_mpam);
	if (err)
		return err;

	mpam.flags = SVM_SET_DEV_MPAM;
	mpam.pasid = pasid;
	mpam.partid = partid;
	mpam.pmg = pmg;
	mpam.s1mpam = s1mpam;
	err = __svm_set_mpam(&mpam);
	if (err)
		goto rollback;

	return 0;

rollback:
	__svm_set_mpam(&old_mpam);
	return err;
}
EXPORT_SYMBOL_GPL(svm_set_mpam);

/**
 * svm_get_mpam() - get smmu mpam configuration of core device
 * @pasid: substream id
 * @partid: pointer to partid
 * @pmg: pointer to pmg
 * @s1mpam: pointer to s1mpam
 */
int svm_get_mpam(int pasid, int *partid, int *pmg, int *s1mpam)
{
	int err = 0;
	struct svm_mpam mpam;

	if (!partid || !pmg || !s1mpam)
		return -EINVAL;

	mpam.flags = SVM_GET_DEV_MPAM,
	mpam.pasid = pasid,
	err = __svm_get_mpam(&mpam);
	if (err)
		return err;

	*partid = mpam.partid;
	*pmg = mpam.pmg;
	*s1mpam = mpam.s1mpam;

	return 0;
}
EXPORT_SYMBOL_GPL(svm_get_mpam);

/**
 * svm_set_user_mpam_en() - set user_mpam_en
 * @user_mpam_en: 0 for smmu mpam, 1 for user mpam
 */
int svm_set_user_mpam_en(int user_mpam_en)
{
	int err;
	struct svm_mpam mpam, old_mpam;

	old_mpam.flags = SVM_GET_USER_MPAM_EN;
	err = __svm_get_mpam(&old_mpam);

	mpam.flags = SVM_SET_USER_MPAM_EN,
	mpam.user_mpam_en = user_mpam_en,
	err = __svm_set_mpam(&mpam);
	if (err)
		goto rollback;

	return 0;

rollback:
	__svm_set_mpam(&mpam);
	return err;
}
EXPORT_SYMBOL_GPL(svm_set_user_mpam_en);

/**
 * svm_set_user_mpam_en() - set user_mpam_en
 * @user_mpam_en: pointer to user_mpam_en
 */
int svm_get_user_mpam_en(int *user_mpam_en)
{
	int err;
	struct svm_mpam mpam;

	mpam.flags = SVM_GET_USER_MPAM_EN;
	err = __svm_get_mpam(&mpam);
	if (err)
		return err;

	*user_mpam_en = mpam.user_mpam_en;
	return 0;
}
EXPORT_SYMBOL_GPL(svm_get_user_mpam_en);
#endif //4.19version

STATIC int svm_proc_load_flag(int __user *arg)
{
	STATIC atomic_t l2buf_load_flag = ATOMIC_INIT(0);
	int flag;

	if (arg == NULL)
		return -EINVAL;

	if (0 == (atomic_cmpxchg(&l2buf_load_flag, 0, 1)))
		flag = 0;
	else
		flag = 1;

	return put_user(flag, arg);
}

/* the caller held mm semaphore already */
STATIC void svm_vma_close(struct vm_area_struct *vma)
{
	struct page *page = vma->vm_private_data;

	put_page(page);
}

STATIC struct vm_operations_struct svm_vma_ops = {
	.close = svm_vma_close,
};

STATIC int svm_mmap(struct file *file, struct vm_area_struct *vma)
{
	int err;
	/* vma->vm_pgoff transfer the nid */
	int nid = (int)vma->vm_pgoff;
	struct svm_device *sdev = file_to_sdev(file);

	if (vma->vm_flags & VM_PA32BIT) {
		unsigned long vm_size = vma->vm_end - vma->vm_start;
		struct page *page = NULL;

		if ((vma->vm_end < vma->vm_start) || (vm_size > MMAP_PHY32_MAX))
			return -EINVAL;

		if (nid == 0) {
			page = alloc_pages(GFP_KERNEL | GFP_DMA32 | __GFP_COMP,
					get_order(vm_size));
		} else {
			if (nid >= MAX_NUMNODES || nid < 0 || !node_online(nid)) {
				dev_err(sdev->dev, "node id: %d invalid\n", nid);
				return -ENODEV;
			}
			page = alloc_pages_node(nid,
					GFP_HIGHUSER_MOVABLE | __GFP_THISNODE | __GFP_COMP,
					get_order(vm_size));
		}

		if (!page) {
			dev_err(sdev->dev, "fail to alloc page on node %d\n", nid);
			return -ENOMEM;
		}

		err = remap_pfn_range(vma,
				vma->vm_start,
				page_to_pfn(page),
				vm_size, vma->vm_page_prot);
		if (err) {
			dev_err(sdev->dev,
				"fail to remap 0x%pK err=%d\n",
				(void *)vma->vm_start, err);
			put_page(page);
		}

		vm_flags_set(vma, VM_DONTCOPY);
		vma->vm_private_data = page;
		vma->vm_ops = &svm_vma_ops;
	} else {
		/* Apply for a space twice the size and align it to l2size
		 * to ensure that the entire l2buf is mapped within the range.
		 */
		if ((vma->vm_end - vma->vm_start) !=  2 * sdev->l2size)
			return -EINVAL;

#ifndef DEV_SVM_UT
		vma->vm_page_prot = __pgprot((~PTE_SHARED) &
				    vma->vm_page_prot.pgprot);

		err = remap_pfn_range(vma,
				round_up(vma->vm_start, sdev->l2size),
				sdev->l2buff >> PAGE_SHIFT,
				sdev->l2size,
				__pgprot(vma->vm_page_prot.pgprot | PTE_DIRTY));
		if (err)
			dev_err(sdev->dev,
				"fail to remap 0x%pK err=%d\n",
				(void *)vma->vm_start, err);
#endif
	}

	return err;
}

STATIC int svm_release_phys32(unsigned long __user *arg)
{
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma = NULL;
	unsigned long addr;
	unsigned int len = 0;

	if (arg == NULL)
		return -EINVAL;

	if (get_user(addr, arg))
		return -EFAULT;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	down_read(&mm->mmap_lock);
#else
	down_read(&mm->mmap_sem);
#endif

	vma = find_vma(mm, addr);
	if (!vma || addr != vma->vm_start) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
		up_read(&mm->mmap_lock);
#else
		up_read(&mm->mmap_sem);
#endif
		return -EFAULT;
	}

	len = vma->vm_end - vma->vm_start;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	up_read(&mm->mmap_lock);
#else
	up_read(&mm->mmap_sem);
#endif

	return vm_munmap(addr, len);
}

STATIC int svm_sp_alloc_mem(unsigned long __user *arg)
{
	struct spalloc spallocinfo;
	void *addr;
	int ret;

	if (arg == NULL) {
		pr_err("arg is invalid value.\n");
		return -EFAULT;
	}

	ret = copy_from_user(&spallocinfo, (void __user *)arg, sizeof(spallocinfo));
	if (ret) {
		pr_err("failed to copy args from user space.\n");
		return -EFAULT;
	}

	addr = mg_sp_alloc(spallocinfo.size, spallocinfo.flag, SPG_DEFAULT_ID);
	if (IS_ERR_VALUE(addr)) {
		pr_err("svm: sp alloc failed with %ld\n", PTR_ERR(addr));
		return -EFAULT;
	}

	spallocinfo.addr = (uintptr_t)addr;
	if (copy_to_user((void __user *)arg, &spallocinfo, sizeof(struct spalloc))) {
#ifdef HELPER
		sp_free(spallocinfo.addr);
#else
		mg_sp_free(spallocinfo.addr, SPG_DEFAULT_ID);
#endif
		return -EFAULT;
	}

	return 0;
}

STATIC int svm_sp_free_mem(unsigned long __user *arg)
{
	int ret;
	struct spalloc spallocinfo;

	if (arg == NULL) {
		pr_err("arg ivalue.\n");
		return -EFAULT;
	}

	ret = copy_from_user(&spallocinfo, (void __user *)arg, sizeof(spallocinfo));
	if (ret) {
		pr_err("failed to copy args from user space.\n");
		return -EFAULT;
	}

	ret = mg_is_sharepool_addr(spallocinfo.addr);
	if (!ret) {
		pr_err("svm: sp free failed because the addr is not from sp.\n");
		return -EINVAL;
	}
#ifdef HELPER
	ret = sp_free(spallocinfo.addr);
#else
	ret = mg_sp_free(spallocinfo.addr, SPG_DEFAULT_ID);
#endif
	if (ret != 0) {
		pr_err("svm: sp free failed with %d.\n", ret);
		return -EFAULT;
	}

	return 0;
}

#ifdef CONFIG_ASCEND_SVSP
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
STATIC unsigned int __set_pasid_highest_bit(unsigned int pasid,
				struct device *dev)
{
	unsigned int ssid_bits = svm_svsp_extract_ssid_bits(dev_iommu_priv_get(dev));
	return pasid + (1 << (ssid_bits - 1));
}
#endif

STATIC struct mm_struct *svm_svsp_create(void)
{
	struct mm_struct *mm = NULL;

	mm = mm_alloc();
	if (!mm) {
		pr_err("svsp create failed\n");
		return NULL;
	}

	mm->get_unmapped_area = current->mm->get_unmapped_area;
	current->mm->svsp_mm = mm;

	return mm;
}

STATIC int svm_svsp_bind_core(struct device *dev, void *data,
		struct process_sva *proc_sva)
{
	int ret = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	struct iommu_sva *sva;
#else
	int pasid;
#endif
	struct mm_struct *mm = NULL;
	struct svm_process *process = data;

	mm = process->svsp;
	if (!mm) {
		mm = svm_svsp_create();
		if (!mm)
			return -ENOMEM;
		process->svsp = mm;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	mm->pasid = __set_pasid_highest_bit(process->pasid, dev);
	sva = iommu_sva_bind_device(dev, mm, NULL);
	if (IS_ERR_OR_NULL(sva)) {
		pr_err("svsp bind device failed, pasid %x\n", process->pasid);
		return PTR_ERR(sva);
	}
	proc_sva->svsp_sva = sva;
#else
	pasid = process->pasid;
	ret = iommu_sva_bind_device(dev, mm, &pasid,
			IOMMU_SVA_FEAT_IOPF | IOMMU_SVA_FEAT_SVSP, NULL);
	if (ret)
		pr_err("svsp bind device failed, pasid %x\n", process->pasid);
#endif

	return ret;
}

STATIC struct mm_struct *svm_svsp_of_pasid(int pasid)
{
	int err = 0;
	unsigned long asid;
	struct mm_struct *sva;
	struct mm_struct *svsp;
	struct svm_process *process = NULL;

	if (current->mm) {
		pr_err("Not allowed to be called by non-kenerl thread\n");
		return ERR_PTR(-EPERM);
	}

	sva = iommu_sva_find(pasid);
	if (!sva) {
		pr_err("Cannot find shared virtual space, pasid %x\n", pasid);
		return ERR_PTR(-EINVAL);
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	asid = arm64_mm_context_get(sva);
#else
	asid = mm_context_get(sva);
#endif

	if (!asid) {
		pr_err("get sva failed, pasid %x\n", pasid);
		err = -ENOSPC;
		goto err_put_mm;
	}

	mutex_lock(&svm_process_mutex);
	process = find_svm_process(asid);
	if (!process) {
		pr_err("find_svm_process failed, pasid %x, asid %lu\n", pasid, asid);
		err = -ENOENT;
		mutex_unlock(&svm_process_mutex);
		goto err_put_context;
	}

	svsp = process->svsp;
	if (!svsp) {
		pr_err("process has no svsp, pasid %x, asid %lu\n", pasid, asid);
		err = -ESRCH;
		mutex_unlock(&svm_process_mutex);
		goto err_put_context;
	}
	mutex_unlock(&svm_process_mutex);

err_put_context:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	arm64_mm_context_put(sva);
#else
	mm_context_put(sva);
#endif
err_put_mm:
	mmput(sva);

	return err ? ERR_PTR(err) : svsp;
}

unsigned long svm_svsp_mmap(unsigned long len, int pasid)
{
	unsigned long addr;
	struct mm_struct *svsp;
	unsigned long populate;

	svsp = svm_svsp_of_pasid(pasid);
	if (IS_ERR_VALUE(svsp)) {
		pr_err("svsp mmap failed, pasid %x\n", pasid);
		return -EINVAL;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	if (down_write_killable(&svsp->mmap_lock))
		return -EINTR;

	current->mm = svsp;
	addr = __do_mmap_mm(svsp, NULL, 0, len, PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS | MAP_SVSP, 0, 0, &populate, NULL);
	current->mm = NULL;
	up_write(&svsp->mmap_lock);
#else
	addr = do_vm_mmap(svsp, 0, len, PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS | MAP_SVSP, 0);
#endif
	return addr;
}
EXPORT_SYMBOL(svm_svsp_mmap);

void svm_svsp_munmap(unsigned long start, unsigned long len, int pasid)
{
	int ret;
	struct mm_struct *svsp;

	svsp = svm_svsp_of_pasid(pasid);
	if (IS_ERR_VALUE(svsp)) {
		pr_err("svsp munmap failed, pasid %x\n", pasid);
		return;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	if (down_write_killable(&svsp->mmap_lock)) {
		pr_err("svsp down_write mmap_lock failed, pasid %x\n", pasid);
		return;
	}

	ret = __do_munmap(svsp, start, len, NULL, false);
#else
	ret = do_vm_munmap(svsp, start, len);
#endif
	if (ret)
		pr_err("svsp munmap failed, start %lx, len %lx, pasid %x\n",
				start, len, pasid);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	up_write(&svsp->mmap_lock);
#endif
	return;
}
EXPORT_SYMBOL(svm_svsp_munmap);

int svm_svsp_remap_range(unsigned long va, unsigned long pfn,
		size_t size, int pasid, pgprot_t prot)
{
	int ret;
	struct vm_area_struct *vma;
	struct mm_struct *mm;

	mm = svm_svsp_of_pasid(pasid);
	if (IS_ERR_VALUE(mm)) {
		pr_err("svsp map, get svsp of dev failed, pasid %x\n", pasid);
		return -EINVAL;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	down_write(&mm->mmap_lock);
#else
	down_write(&mm->mmap_sem);
#endif

	vma = find_vma(mm, va);
	if (vma == NULL || vma->vm_start > va) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
		up_write(&mm->mmap_lock);
#else
		up_write(&mm->mmap_sem);
#endif
		pr_err("svm_svsp_map_range find_vma failed!");
		return -EINVAL;
	}

	ret = remap_pfn_range(vma, va, pfn, size, prot);
	if (ret) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
		up_write(&mm->mmap_lock);
#else
		up_write(&mm->mmap_sem);
#endif
		pr_err("svsp, remap range failed %d\n", ret);
		return ret;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	up_write(&mm->mmap_lock);
#else
	up_write(&mm->mmap_sem);
#endif

	return 0;
}
EXPORT_SYMBOL(svm_svsp_remap_range);

struct mm_struct *svm_svsp_of_mm(struct mm_struct *mm)
{
	if (!mm || !mm->svsp_mm)
		return NULL;

	return mm->svsp_mm;
}
EXPORT_SYMBOL(svm_svsp_of_mm);
#endif

/*svm ioctl will include some case for HI1980 and HI1910*/
STATIC long svm_ioctl(struct file *file, unsigned int cmd,
			 unsigned long arg)
{
	int err = -EINVAL;
	struct svm_device *sdev = file_to_sdev(file);

	down_write(&svm_sem);
	if (unlikely(ts_shmem_need_init)) {
		err = svm_va2pa_trunk_init_dynamic(sdev->dev);
		ts_shmem_need_init = false;
	}
	up_write(&svm_sem);

	if (!arg)
		return -EINVAL;

	switch (cmd) {
	case SVM_IOCTL_PROCESS_BIND:
		err = svm_process_bind(sdev, (void __user *)arg);
		break;
	case SVM_IOCTL_GET_PHYS:
		err = svm_get_phys((unsigned long __user *)arg);
		break;
	case SVM_IOCTL_PIN_MEMORY:
		err = svm_pin_memory((unsigned long __user *)arg);
		break;
	case SVM_IOCTL_UNPIN_MEMORY:
		err = svm_unpin_memory((unsigned long __user *)arg);
		break;
	case SVM_IOCTL_LOAD_FLAG:
		err = svm_proc_load_flag((int __user *)arg);
		break;
	case SVM_IOCTL_RELEASE_PHYS32:
		err = svm_release_phys32((unsigned long __user *)arg);
		break;
	case SVM_IOCTL_SP_ALLOC:
		err = svm_sp_alloc_mem((unsigned long __user *)arg);
		break;
	case SVM_IOCTL_SP_FREE:
		err = svm_sp_free_mem((unsigned long __user *)arg);
		break;
	default:
		err = -EINVAL;
	}

	if (err)
		dev_err(sdev->dev, "%s: %s failed err = %d\n", __func__,
				svm_cmd_to_string(cmd), err);

	return err;
}

STATIC const struct file_operations svm_fops = {
	.owner			= THIS_MODULE,
	.open			= svm_open,
	.mmap			= svm_mmap,
	.unlocked_ioctl		= svm_ioctl,
};

STATIC int svm_dt_setup_l2buff(struct svm_device *sdev, struct device_node *np)
{
	struct device_node *l2buff = of_parse_phandle(np, "memory-region", 0);

	if (l2buff) {
		struct resource r;
		int err = of_address_to_resource(l2buff, 0, &r);

		if (err || !is_power_of_2(resource_size(&r))) {
			of_node_put(l2buff);
			return err;
		}

		sdev->l2buff = r.start;
		sdev->l2size = resource_size(&r);
	}

	of_node_put(l2buff);
	return 0;
}

/*svm device probe this is init the svm device*/
STATIC int svm_device_probe(struct platform_device *pdev)
{
	int err = -1;
	struct device *dev = &pdev->dev;
	struct svm_device *sdev = NULL;
	struct device_node *np = dev->of_node;
	int alias_id;

	if (acpi_disabled && np == NULL)
		return -ENODEV;

	if (!dev->bus) {
		dev_dbg(dev, "this dev bus is NULL\n");
		return -EPROBE_DEFER;
	}

	if (!device_iommu_mapped(dev)) {
		dev_dbg(dev, "defer probe svm device\n");
		return -EPROBE_DEFER;
	}

	sdev = devm_kzalloc(dev, sizeof(*sdev), GFP_KERNEL);
	if (sdev == NULL)
		return -ENOMEM;

	if (!acpi_disabled) {
		err = device_property_read_u64(dev, "svmid", &sdev->id);
		if (err) {
			dev_err(dev, "failed to get this svm device id\n");
			return err;
		}
	} else {
		alias_id = of_alias_get_id(np, "svm");
		if (alias_id < 0)
			sdev->id = probe_index;
		else
			sdev->id = alias_id;
	}

	sdev->dev = dev;
	sdev->miscdev.minor = MISC_DYNAMIC_MINOR;
	sdev->miscdev.fops = &svm_fops;
	sdev->miscdev.name = devm_kasprintf(dev, GFP_KERNEL,
			SVM_DEVICE_NAME"%llu", sdev->id);
	if (sdev->miscdev.name == NULL)
		return -ENOMEM;

	INIT_LIST_HEAD(&sdev->child_list);
	dev_set_drvdata(dev, sdev);
	err = misc_register(&sdev->miscdev);
	if (err) {
		dev_err(dev, "Unable to register misc device\n");
		return err;
	}

	err = memset_s(va2pa_trunk,  sizeof(va2pa_trunk), 0, sizeof(va2pa_trunk));
	if (err)
		goto err_unregister_misc;

	if (!acpi_disabled) {
		if (svm_acpi_va2pa_trunk_init(sdev)) {
			dev_warn(dev, "failed to init va2pa trunk\n");
		}

		err = svm_acpi_init_core(sdev);
		if (err) {
			dev_err(dev, "failed to init acpi cores\n");
			goto err_unregister_misc;
		}
	} else {
		/*
		 * Get the l2buff phys address and size, if it do not exist
		 * just warn and continue, and runtime can not use L2BUFF.
		 */
		err = svm_dt_setup_l2buff(sdev, np);
		if (err)
			dev_warn(dev, "Cannot get l2buff\n");

		if (svm_va2pa_trunk_init(dev))
			dev_warn(dev, "failed to init va2pa trunk\n");

		err = svm_dt_init_core(sdev, np);
		if (err) {
			dev_err(dev, "failed to init dt cores\n");
			goto err_remove_trunk;
		}

		probe_index++;
	}

	mutex_init(&svm_process_mutex);
	list_add(&sdev->entry, &sdev_list);

	return 0;

err_remove_trunk:
	svm_remove_trunk(dev);

err_unregister_misc:
	misc_deregister(&sdev->miscdev);
	if (acpi_disabled)
		svm_remove_trunk(dev);

	return err;
}

STATIC void svm_device_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct svm_device *sdev = dev_get_drvdata(dev);

	svm_remove_dev_slots(sdev);
	misc_deregister(&sdev->miscdev);
	list_del(&sdev->entry);
}

#ifndef CONFIG_AOS_FEATURES
STATIC void svm_device_shutdown(struct platform_device *pdev)
{
	svm_device_remove(pdev);
}
#endif
STATIC const struct acpi_device_id svm_acpi_match[] = {
	{"HSVM1980", 0},
	{"HSVM0310", 0},
	{}
};
MODULE_DEVICE_TABLE(acpi, svm_acpi_match);

STATIC const struct of_device_id svm_of_match[] = {
	{ .compatible = "hisilicon,svm" },
	{ }
};
MODULE_DEVICE_TABLE(of, svm_of_match);

/*svm acpi probe and remove*/
STATIC struct platform_driver svm_driver = {
	.probe	=	svm_device_probe,
	.remove	=	svm_device_remove,
#ifndef CONFIG_AOS_FEATURES
	.shutdown =	svm_device_shutdown,
#endif
	.driver	=	{
		.name = SVM_DEVICE_NAME,
		.acpi_match_table = ACPI_PTR(svm_acpi_match),
		.of_match_table = svm_of_match,
	},
};

STATIC int svm_init(void)
{
	int err;

	err = bus_register(&svm_bus_type);
	if (err)
		return err;

	err = platform_driver_register(&svm_driver);
	if (err) {
		bus_unregister(&svm_bus_type);
		return err;
	}

	return 0;
}

STATIC void svm_exit(void)
{
	platform_driver_unregister(&svm_driver);
	bus_unregister(&svm_bus_type);
}

module_init(svm_init);
module_exit(svm_exit);

MODULE_DESCRIPTION("Hisilicon SVM driver");
MODULE_LICENSE("GPL v2");
