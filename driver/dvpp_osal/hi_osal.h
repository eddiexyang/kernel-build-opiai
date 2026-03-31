/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-7-23
 */
#ifndef HI_OSAL__
#define HI_OSAL__

#define HI_OSAL_VERSION    "1.0"

#include "osal_list.h"
#include "hi_debug.h"

#define osal_gfp_kernel    0
#define osal_gfp_atomic    1
#define MAX_MEM_INFO 5000U

typedef enum {
    MEM_OP_VMALLOC = 0,
    MEM_OP_KMALLOC,
    MEM_OP_SP_MALLOC,
    MEM_OP_VFREE,
    MEM_OP_KFREE,
    MEM_OP_SP_FREE,
} mem_operation_type;

typedef struct {
    hi_char *func;
    hi_void *kva;
    hi_void *uva;
    hi_u32 size;
    mem_operation_type type;
} mem_record_info;

extern void osal_device_init(void);

void *osal_vmalloc_(hi_ulong size, const char *function);
void osal_vfree_(const void *addr, const char *function);
void *osal_kmalloc_(hi_ulong size, hi_u32 osal_gfp_flag, const char *function);
void osal_kfree_(const void *addr, const char *function);
hi_void *osal_vmalloc_node(hi_ulong size, hi_s32 node_id);
hi_void *osal_vmalloc_hugepage_node(hi_ulong size, hi_s32 node_id);

#define osal_vmalloc(size) osal_vmalloc_((size), (__FUNCTION__))
#define osal_vfree(addr) osal_vfree_((addr), (__FUNCTION__))
#define osal_kmalloc(size, flag) osal_kmalloc_((size), (flag), (__FUNCTION__))
#define osal_kfree(addr) osal_kfree_((addr), (__FUNCTION__))

hi_ulong osal_get_free_pages(hi_u32 osal_gfp_flag, hi_u32 order);
hi_ulong osal_get_free_pages_node(hi_s32 nid, hi_u32 osal_gfp_flag, hi_u32 order);
hi_void osal_free_pages(hi_ulong addr, hi_u32 order);
hi_u32 osal_get_order(hi_ulong size);

// mmz相关接口
hi_s32 osal_mmz_init(void);
hi_s32 osal_mmz_exit(void);
hi_u64 osal_mmz_malloc(const hi_char *mmz_name, const hi_char *buf_name, hi_ulong size);
hi_void osal_mmz_free(hi_u64 phy_addr, hi_void *vir_addr);
hi_s32 osal_mmz_is_valid(const hi_char *mmz_name, hi_u64 phy_addr);

hi_bool osal_is_sharepool_addr(hi_ulong addr);
#ifndef DVPP_HLP
void* osal_vmalloc_to_page(void *addr);
hi_u32 osal_get_page_size(void *pg);
hi_phys_addr_t osal_page_to_phys(hi_void *pg);
#endif // DVPP_HLP
extern hi_void *osal_find_vma(hi_void *mm, hi_ulong addr);
extern hi_void osal_down_read_mmap_sem(hi_void *mm);
extern hi_void osal_up_read_mmap_sem(hi_void *mm);

extern mem_record_info g_mem_record_info[MAX_MEM_INFO];
extern hi_u32 g_mem_record_idx;

extern void *osal_symbol_get(const char *symbol);
extern void osal_symbol_put(const char *symbol);
// atomic api
typedef struct {
    void *atomic;
} osal_atomic_t;

#define OSAL_ATOMIC_INIT(i)  { (i) }
#define osal_atomic64_set(v, i)  atomic64_set(v, i)
#define osal_atomic64_add(v, i)  atomic64_add(v, i)

hi_s32 osal_atomic_init(osal_atomic_t *atomic);
void osal_atomic_destroy(osal_atomic_t *atomic);
void osal_atomic_destory(osal_atomic_t *atomic);
hi_s32 osal_atomic_read(const osal_atomic_t *atomic);
void osal_atomic_set(const osal_atomic_t *atomic, hi_s32 i);
hi_s32 osal_atomic_inc_return(const osal_atomic_t *atomic);
hi_s32 osal_atomic_dec_return(const osal_atomic_t *atomic);

// semaphore api
#define EINTR              4
typedef struct osal_semaphore {
    void *sem;
} osal_semaphore_t;

hi_s32 osal_sema_init(osal_semaphore_t *sem, hi_s32 val);
hi_s32 osal_down(const osal_semaphore_t *sem);
hi_s32 osal_down_timeout(osal_semaphore_t *sem, long msecs);
hi_s32 osal_down_interruptible(const osal_semaphore_t *sem);
hi_s32 osal_down_trylock(const osal_semaphore_t *sem);
void osal_up(const osal_semaphore_t *sem);
// notice:must be called when kmod exit, other wise will lead to memory leak;
void osal_sema_destroy(osal_semaphore_t *sem);
void osal_sema_destory(osal_semaphore_t *sem);

// mutex api
typedef struct osal_mutex {
    void *mutex;
} osal_mutex_t;
extern hi_s32 osal_mutex_init(osal_mutex_t *mutex);
extern hi_s32 osal_mutex_lock(const osal_mutex_t *mutex);
extern hi_s32 osal_mutex_lock_interruptible(const osal_mutex_t *mutex);
extern hi_s32 osal_mutex_trylock(const osal_mutex_t *mutex);
extern void osal_mutex_unlock(const osal_mutex_t *mutex);
// notice:must be called when kmod exit, other wise will lead to memory leak;
extern void osal_mutex_destroy(osal_mutex_t *mutex);
extern void osal_mutex_destory(osal_mutex_t *mutex);

// spin lock api
typedef struct osal_spinlock {
    void *lock;
} osal_spinlock_t;

hi_s32 osal_spin_lock_init(osal_spinlock_t *lock);
void osal_spin_lock(const osal_spinlock_t *lock);
hi_s32 osal_spin_trylock(const osal_spinlock_t *lock);
void osal_spin_unlock(const osal_spinlock_t *lock);
void osal_spin_lock_irqsave(const osal_spinlock_t *lock, hi_ulong *flags);
void osal_spin_unlock_irqrestore(const osal_spinlock_t *lock, const hi_ulong *flags);
// notice:must be called when kmod exit, other wise will lead to memory leak;
void osal_spin_lock_destroy(osal_spinlock_t *lock);
void osal_spin_lock_destory(osal_spinlock_t *lock);

// wait api
typedef int (*osal_wait_cond_func_t)(const void *param);

typedef struct osal_wait {
    void *wait;
} osal_wait_t;
#define ERESTARTSYS        512

hi_ulong osal_msecs_to_jiffies(const hi_u32 m);
hi_s32 osal_wait_init(osal_wait_t *wait);
hi_s32 osal_wait_uninterruptible(const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param);
hi_s32 osal_wait_timeout_interruptible(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s32 ms);
hi_s32 osal_wait_timeout_interruptible_exclusive(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s32 ms);
hi_s32 osal_wait_timeout_uninterruptible(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s32 ms);
hi_s64 osal_wait_event_hrtimeout_interruptible(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s64 timeout);
hi_s64 osal_wait_event_hrtimeout_interruptible_exclusive(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s64 timeout);
void osal_wakeup(const osal_wait_t *wait);
void osal_wakeup_one(const osal_wait_t *wait);
void osal_wakeup_nr(const osal_wait_t *wait, int nr);
void osal_wait_destroy(osal_wait_t *wait);
void osal_wait_destory(osal_wait_t *wait);
void osal_hrtime_nsleep(hi_u64 ns);
void osal_init_completion(void *x);
void osal_wait_for_completion(void *x);
void osal_complete(void *x);

#define osal_wait_event_interruptible(wait, func, param)                       \
    ({                                                                         \
        hi_s32 ret__ = 0;                                                      \
                                                                               \
        do {                                                                   \
            if (((func)((param))) != 0) {                                        \
                ret__ = 0;                                                     \
                break;                                                         \
            }                                                                  \
            ret__ = osal_wait_timeout_interruptible((wait), (func), (param), 100); \
        } while (ret__ >= 0);                                                  \
        ret__;                                                                 \
    })

#define osal_wait_event_interruptible_exclusive(wait, func, param)                       \
    ({                                                                         \
        hi_s32 ret__ = 0;                                                      \
                                                                               \
        do {                                                                   \
            if (((func)((param))) != 0) {                                        \
                ret__ = 0;                                                     \
                break;                                                         \
            }                                                                  \
            ret__ = osal_wait_timeout_interruptible_exclusive((wait), (func), (param), 100); \
        } while (ret__ >= 0);                                                  \
        ret__;                                                                 \
    })

#define osal_wait_event_uninterruptible(wait, func, param)          \
    ({                                                              \
        hi_s32 ret__ = 0;                                              \
                                                                    \
        do {                                                        \
            if (((func)((param))) != 0) {                             \
                ret__ = 0;                                          \
                break;                                              \
            }                                                       \
            ret__ = osal_wait_uninterruptible((wait), (func), (param)); \
        } while (ret__ >= 0);                                       \
        ret__;                                                      \
    })

#define osal_wait_event_timeout_interruptible(wait, func, param, timeout)        \
    ({                                                                           \
        hi_s32 ret__ = (hi_s32)(timeout);                                        \
                                                                                 \
        if ((((func)((param))) != 0) && ((timeout) == 0)) {                        \
            ret__ = 1;                                                           \
        }                                                                        \
                                                                                 \
        do {                                                                     \
            if (((func)((param))) != 0) {                                            \
                ret__ = (hi_s32)osal_msecs_to_jiffies((hi_u32)ret__);            \
                break;                                                           \
            }                                                                    \
            ret__ = osal_wait_timeout_interruptible((wait), (func), (param), ret__); \
        } while ((ret__ != 0) && (ret__ != -ERESTARTSYS));                       \
        ret__;                                                                   \
    })

#define osal_wait_event_timeout_uninterruptible(wait, func, param, timeout)        \
    ({                                                                             \
        hi_s32 ret__ = (hi_s32)(timeout);                                          \
                                                                                   \
        if ((((func)((param))) != 0) && ((timeout) == 0)) {                          \
            ret__ = 1;                                                             \
        }                                                                          \
                                                                                   \
        do {                                                                       \
            if (((func)((param))) != 0) {                                            \
                ret__ = (hi_s32)osal_msecs_to_jiffies((hi_u32)ret__);              \
                break;                                                             \
            }                                                                      \
            ret__ = osal_wait_timeout_uninterruptible((wait), (func), (param), ret__); \
        } while ((ret__ != 0) && (ret__ != -ERESTARTSYS));                         \
        ret__;                                                                     \
    })

// workqueue api
typedef struct osal_work_struct {
    void *work;
    void (*func)(struct osal_work_struct *work);
} osal_work_struct_t;
typedef void (*osal_work_func_t)(struct osal_work_struct *work);
extern int osal_init_work(struct osal_work_struct *work, osal_work_func_t func);
#define OSAL_INIT_WORK(_work, _func)      \
    do {                                  \
        osal_init_work((_work), (_func)); \
    } while (0)

extern int osal_schedule_work(struct osal_work_struct *work);
extern void osal_destroy_work(struct osal_work_struct *work);

// shedule
extern void osal_yield(void);

// interrupt api
enum osal_irqreturn {
    OSAL_IRQ_NONE = (0 << 0),
    OSAL_IRQ_HANDLED = (1 << 0),
    OSAL_IRQ_WAKE_THREAD = (1 << 1),
};

typedef int (*osal_irq_handler_t)(int, void *);
extern hi_s32 osal_request_irq(hi_u32 irq, osal_irq_handler_t handler, osal_irq_handler_t thread_fn,
                               const hi_char *name, void *dev);
extern hi_s32 osal_request_irq_ex(hi_u32 irq, osal_irq_handler_t handler, osal_irq_handler_t thread_fn,
                                  const hi_char *name, void *dev);
extern void osal_free_irq(hi_u32 irq, void *dev);
extern hi_s32 osal_in_interrupt(void);
hi_u32 osal_irq_of_parse_and_map(void *dev, hi_s32 idx);
hi_s32 osal_irq_force_affinity(hi_u32 irq, const void *cpumask);
hi_s32 osal_irq_set_affinity_hint(hi_u32 irq, const void *cpumask);
const void *osal_get_cpu_mask(hi_u32 cpu);

// task api
typedef struct osal_task {
    void *task_struct;
} osal_task_t;

// task priority
#define osal_priority_min  1U
#define osal_priority_normal 5U
#define osal_priority_max 10U

typedef int (*threadfn_t)(void *data);
osal_task_t *osal_kthread_create(threadfn_t thread, void *data, const hi_char *name);
osal_task_t *osal_kthread_create_ex(threadfn_t thread, void *data, const hi_char *name, hi_u32 priority);
void osal_kthread_destroy(osal_task_t *task, hi_u32 stop_flag);
void osal_kthread_destory(osal_task_t *task, hi_u32 stop_flag);
osal_task_t *osal_kthread_create_and_bind(threadfn_t thread, void *data, const hi_char *name, hi_u32 cpu);
osal_task_t *osal_kthread_create_and_bind_cpu_bit_map(
    threadfn_t thread, void *data, const hi_char *name, const void *mask);
void osal_kthread_bind_cpu_bitmap(void *task_struct, const void *mask);

// string api
extern hi_ulong osal_strlcpy(hi_char *dest, const hi_char *src, hi_ulong size);
extern hi_ulong osal_strlcat(hi_char *dest, const hi_char *src, hi_ulong count);
extern hi_s32 osal_strcmp(const hi_char *cs, const hi_char *ct);
extern hi_s32 osal_strncmp(const hi_char *cs, const hi_char *ct, hi_ulong count);
extern hi_s32 osal_strcasecmp(const hi_char *s1, const hi_char *s2);
extern hi_s32 osal_strncasecmp(const hi_char *s1, const hi_char *s2, hi_ulong n);
extern const hi_char *osal_strchr(const hi_char *s, hi_s32 c);
extern hi_char *osal_strnchr(const hi_char *s, hi_ulong count, hi_s32 c);
extern const hi_char *osal_strrchr(const hi_char *s, hi_s32 c);
extern hi_char *osal_strnstr(const hi_char *s1, const hi_char *s2, hi_ulong len);
extern hi_ulong osal_strlen(const hi_char *s);
extern hi_ulong osal_strnlen(const hi_char *s, hi_ulong count);
extern const hi_char *osal_strpbrk(const hi_char *cs, const hi_char *ct);
extern hi_ulong osal_strspn(const hi_char *s, const hi_char *accept);
extern hi_ulong osal_strcspn(const hi_char *s, const hi_char *reject);
extern void *osal_memscan(void *addr, hi_s32 c, hi_ulong size);
extern hi_s32 osal_memcmp(const void *cs, const void *ct, hi_ulong count);
extern void *osal_memchr(const void *s, hi_s32 c, hi_ulong n);
extern void *osal_memchr_inv(const void *start, hi_s32 c, hi_ulong bytes);
extern hi_char *osal_strsep(hi_char **stringp, const hi_char *delim);
extern hi_s32 osal_kstrtouint(const hi_char *s, hi_u32 base, hi_u32 *res);

extern hi_u64 osal_strtoull(const hi_char *cp, hi_char **endp, hi_u32 base);
extern hi_u32 osal_strtoul(const hi_char *cp, hi_char **endp, hi_u32 base);
extern hi_s32 osal_strtol(const hi_char *cp, hi_char **endp, hi_u32 base);
extern hi_s64 osal_strtoll(const hi_char *cp, hi_char **endp, hi_u32 base);

// addr translate
void *osal_ioremap(hi_ulong phys_addr, hi_ulong size);
void *osal_ioremap_nocache(hi_ulong phys_addr, hi_ulong size);
void *osal_ioremap_wc(hi_ulong phys_addr, hi_ulong size);
void osal_iounmap(void *addr);
hi_ulong osal_copy_from_user(void *to, const void *from, hi_ulong n);
hi_ulong osal_copy_to_user(void *to, const void *from, hi_ulong n);

#define OSAL_VERIFY_READ   0
#define OSAL_VERIFY_WRITE  1
hi_s32 osal_access_ok(hi_s32 type, const void *addr, hi_ulong size);

hi_u32 osal_reg_rd(hi_ulong vir_addr);
void osal_reg_wr(hi_ulong vir_addr, hi_u32 val);
#define DVPP_REG_RD(base_addr, reg) osal_reg_rd(((hi_ulong)(uintptr_t)(base_addr)) + ((hi_ulong)(reg)))
#define DVPP_REG_WR(base_addr, reg, val) osal_reg_wr(((hi_ulong)(uintptr_t)(base_addr)) + ((hi_ulong)(reg)), (val))

#define osal_readl(x) (*((volatile hi_u32 *)(x)))
#define osal_writel(v, x) (*((volatile hi_u32 *)(x)) = (v))

// cache api
extern void osal_cpuc_flush_dcache_area(const void *addr, hi_s32 size);
extern void osal_flush_dcache_area(void *kvirt, hi_ulong phys_addr, hi_ulong length);
extern void osal_invalid_dcache_area(void *kvirt, hi_ulong phys_addr, hi_ulong length);

// math
extern hi_u64 osal_div_u64(hi_u64 dividend, hi_u32 divisor);
extern hi_s64 osal_div_s64(hi_s64 dividend, hi_s32 divisor);
extern hi_u64 osal_div64_u64(hi_u64 dividend, hi_u64 divisor);
extern hi_s64 osal_div64_s64(hi_s64 dividend, hi_s64 divisor);
extern hi_u64 osal_div_u64_rem(hi_u64 dividend, hi_u32 divisor);
extern hi_s64 osal_div_s64_rem(hi_s64 dividend, hi_s32 divisor);
extern hi_u64 osal_div64_u64_rem(hi_u64 dividend, hi_u64 divisor);
extern hi_u32 osal_random(void);

#define osal_max(x, y) ({             \
        __typeof__(x) max1 = (x);     \
        __typeof__(y) max2 = (y);     \
        (void) (&max1 == &max2);      \
        max1 > max2 ? max1 : max2;    \
})

#define osal_min(x, y) ({             \
    __typeof__(x) min1 = (x);         \
    __typeof__(y) min2 = (y);         \
    (void) (&min1 == &min2);          \
    (min1 < min2) ? min1 : min2;      \
})

#define osal_abs(x) ({                \
    long ret;                         \
    if (sizeof(x) == sizeof(long)) {  \
        long __x = (x);               \
        ret = (__x < 0) ? -__x : __x; \
    } else {                          \
        int __x = (x);                \
        ret = (__x < 0) ? -__x : __x; \
    }                                 \
    ret;                              \
})

// barrier
extern void osal_mb(void);
extern void osal_rmb(void);
extern void osal_wmb(void);
extern void osal_smp_mb(void);
extern void osal_smp_rmb(void);
extern void osal_smp_wmb(void);
extern void osal_isb(void);
extern void osal_dsb(void);
extern void osal_dmb(void);

// debug
extern hi_s32 osal_printk(const hi_char *fmt, ...) __attribute__((format(printf, 1, 2)));
extern void osal_panic(const char *fmt, const hi_char *fun, hi_s32 line, const hi_char *cond);
#define OSAL_BUG() \
    do {           \
    } while (1)

#define OSAL_ASSERT(expr)                       \
    do {                                        \
        if (!(expr)) {                          \
            osal_printk("\nASSERT failed at:\n" \
                        "  >Condition: %s\n",   \
                #expr);                         \
            OSAL_BUG();                         \
        }                                       \
    } while (0)

// proc
typedef struct osal_proc_dir_entry {
    char name[50];
    void *proc_dir_entry;
    int (*open)(struct osal_proc_dir_entry *entry);
    int (*read)(const struct osal_proc_dir_entry *entry);
    int (*write)(struct osal_proc_dir_entry *entry, const char *buf, int count, long long *);
    union {
        void *private_elem;
        void *private_osal;
    };
    void *seqfile;
    struct osal_list_head node;
} osal_proc_entry_t;

typedef struct osal_seq_file {
    hi_char *base;
    hi_u64 capacity;
    hi_u64 size;
} osal_seq_file_t;

osal_proc_entry_t *osal_create_proc_entry(const hi_char *name, osal_proc_entry_t *parent);
void osal_remove_proc_entry(const hi_char *name, const osal_proc_entry_t *parent);
osal_proc_entry_t *osal_proc_mkdir(const hi_char *name, const osal_proc_entry_t *parent);
void osal_seq_printf(const osal_proc_entry_t *entry, const hi_char *fmt, ...) __attribute__((format(printf, 2, 3)));
void osal_proc_init(void);
void osal_proc_exit(void);

// device api
#ifndef _IOC_TYPECHECK
#include "osal_ioctl.h"
#endif
#define OSAL_MODULE_DEVICE_TABLE(type, name) MODULE_DEVICE_TABLE(type, name)
typedef struct osal_dev {
    char name[48];
    void *dev;
    hi_u32 minor;
    struct osal_fileops *fops;
    struct osal_pmops *osal_pmops;
} osal_dev_t;

typedef struct osal_vm {
    void *vm;
} osal_vm_t;

#define OSAL_POLLIN        0x0001U
#define OSAL_POLLPRI       0x0002U
#define OSAL_POLLOUT       0x0004U
#define OSAL_POLLERR       0x0008U
#define OSAL_POLLHUP       0x0010U
#define OSAL_POLLNVAL      0x0020U
#define OSAL_POLLRDNORM    0x0040U
#define OSAL_POLLRDBAND    0x0080U
#define OSAL_POLLWRNORM    0x0100U

typedef struct osal_poll {
    void *poll_table;
    void *data;
} osal_poll_t;

typedef struct osal_private_data {
    struct osal_dev *dev;
    void *data;
    struct osal_poll table;
    int32_t f_ref_cnt;
    hi_s32 tgid;
    hi_s32 pid;
} osal_private_data_t;

typedef struct osal_fileops {
    int (*open)(void *private_data);
    int (*read)(char *buf, int size, hi_s64 *offset, void *private_data);
    int (*write)(const char *buf, int size, hi_s64 *offset, void *private_data);
    long (*llseek)(hi_s64 offset, int whence, void *private_data);
    int (*release_ext)(osal_private_data_t *pdata);
    int (*release)(void *private_data);
    long (*unlocked_ioctl)(unsigned int cmd, unsigned long arg, void *private_data);
    unsigned int (*poll)(osal_poll_t *poll_data, void *private_data);
    int (*mmap)(osal_vm_t *vm, unsigned long start, unsigned long end, unsigned long vm_pgoff,
                void *private_data);
#if defined CONFIG_COMPAT || defined AOS_LLVM_BUILD
    long (*compat_ioctl)(unsigned int cmd, unsigned long arg, void *private_data);
#endif
    void *module;
} osal_fileops_t;

typedef struct osal_pmops {
    int (*pm_prepare)(osal_dev_t *dev);
    void (*pm_complete)(osal_dev_t *dev);
    int (*pm_suspend)(osal_dev_t *dev);
    int (*pm_resume)(osal_dev_t *dev);
    int (*pm_freeze)(const osal_dev_t *dev);
    int (*pm_thaw)(osal_dev_t *dev);
    int (*pm_poweroff)(osal_dev_t *dev);
    int (*pm_restore)(const osal_dev_t *dev);
    int (*pm_suspend_late)(osal_dev_t *dev);
    int (*pm_resume_early)(osal_dev_t *dev);
    int (*pm_freeze_late)(osal_dev_t *dev);
    int (*pm_thaw_early)(osal_dev_t *dev);
    int (*pm_poweroff_late)(osal_dev_t *dev);
    int (*pm_restore_early)(osal_dev_t *dev);
    int (*pm_suspend_noirq)(osal_dev_t *dev);
    int (*pm_resume_noirq)(osal_dev_t *dev);
    int (*pm_freeze_noirq)(osal_dev_t *dev);
    int (*pm_thaw_noirq)(osal_dev_t *dev);
    int (*pm_poweroff_noirq)(osal_dev_t *dev);
    int (*pm_restore_noirq)(osal_dev_t *dev);
} osal_pmops_t;

#define OSAL_SEEK_SET      0
#define OSAL_SEEK_CUR      1
#define OSAL_SEEK_END      2

extern osal_dev_t *osal_createdev(const char *name);
extern hi_s32 osal_destroydev(const osal_dev_t *pdev);
extern hi_s32 osal_registerdevice(osal_dev_t *pdev);
extern void osal_deregisterdevice(const osal_dev_t *pdev);
extern void osal_poll_wait(osal_poll_t *table, osal_wait_t *wait);
extern void osal_pgprot_noncached(const osal_vm_t *vm);
extern void osal_pgprot_cached(const osal_vm_t *vm);
extern void osal_pgprot_writecombine(const osal_vm_t *vm);
extern void osal_pgprot_stronglyordered(const osal_vm_t *vm);
extern hi_s32 osal_remap_pfn_range(const osal_vm_t *vm, unsigned long addr, unsigned long pfn, unsigned long size);
extern hi_s32 osal_io_remap_pfn_range(const osal_vm_t *vm, unsigned long addr, unsigned long pfn, unsigned long size);

// timer
typedef struct osal_timer {
    void *timer;
    void (*function)(unsigned long);
    unsigned long data;
    hi_u32 bind_cpu_id;
} osal_timer_t;

typedef struct osal_timeval {
    long tv_sec;
    long tv_usec;
} osal_timeval_t;

typedef struct osal_rtc_time {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
} osal_rtc_time_t;

typedef struct osal_timespec64 {
    hi_s64 tv_sec;
    long tv_nsec;
} osal_timespec64_t;

/* Return values for the timer callback function */
typedef enum hiOSAL_HRTIMER_RESTART_E {
    OSAL_HRTIMER_NORESTART, /* < The timer will not be restarted. */
    OSAL_HRTIMER_RESTART /* < The timer must be restarted. */
} OSAL_HRTIMER_RESTART_E;

/* hrtimer struct */
typedef struct osal_hrtimer {
    void *timer;
    OSAL_HRTIMER_RESTART_E (*function)(void *timer);
    unsigned long interval; /* Unit ms */
} osal_hrtimer_t;

extern int osal_hrtimer_create(osal_hrtimer_t *phrtimer);
extern int osal_hrtimer_start(osal_hrtimer_t *phrtimer);
extern int osal_hrtimer_destroy(osal_hrtimer_t *phrtimer);
extern int osal_hrtimer_destory(osal_hrtimer_t *phrtimer);

hi_ulong osal_timer_get_private_data(const void *data);
hi_s32 osal_timer_init(osal_timer_t *timer);
hi_s32 osal_set_timer(const osal_timer_t *timer, hi_ulong interval);
hi_void osal_set_timer_on_cpu(const osal_timer_t *timer, hi_ulong interval);
hi_s32 osal_del_timer(const osal_timer_t *timer);
hi_s32 osal_timer_destroy(osal_timer_t *timer);
hi_s32 osal_timer_destory(osal_timer_t *timer);
hi_s32 osal_stop_timer(const osal_timer_t *timer);
hi_ulong osal_msleep(hi_u32 msecs);
void osal_msleep_uninterrupt(hi_u32 msecs);
void osal_udelay(hi_u32 usecs);
hi_u32 osal_get_tickcount(void);
hi_u64 osal_sched_clock(void);
hi_u64 osal_asm_get_clock(void);
hi_u64 osal_asm_get_sys_cnt(void);
hi_u64 osal_get_tick_count(void);
void osal_gettimeofday(osal_timeval_t *tv);
void osal_rtc_time_to_tm(hi_ulong local_time, osal_rtc_time_t *tm);
void osal_ktime_get_raw_ts64(osal_timespec64_t *tv);
void osal_get_real_ts64(osal_timespec64_t *tv);
#define OSAL_O_ACCMODE     00000003
#define OSAL_O_RDONLY      00000000
#define OSAL_O_WRONLY      00000001
#define OSAL_O_RDWR        00000002
#define OSAL_O_CREAT       00000100

extern void *osal_klib_fopen(const hi_char *filename, hi_s32 flags, hi_u16 mode);
extern void osal_klib_fclose(void *filp);
extern hi_s32 osal_klib_fwrite(const hi_char *buf, hi_ulong len, void *filp);
extern hi_s32 osal_klib_fread(hi_char *buf, hi_u32 len, void *filp);

#include <linux/kernel.h> /* stdarg.h not available in kernel */

#ifndef OSAL_VA_LIST

#define OSAL_VA_LIST
#define osal_va_list       va_list
#define osal_va_arg(ap, T) va_arg(ap, T)
#define osal_va_end(ap) va_end(ap)
#define osal_va_start(ap, A) va_start(ap, A)

#endif /* va_arg */

#define NULL_STRING        "NULL"

extern void osal_vprintk(const char *fmt, osal_va_list args);

#ifdef CONFIG_HISI_SNAPSHOT_BOOT

#ifndef OSAL_UMH_WAIT_PROC
#define OSAL_UMH_WAIT_PROC 2 /* wait for the process to complete */
#endif

extern int osal_call_usermodehelper_force(char *path, char **argv, char **envp, int wait);
#endif
void *osal_platform_device_register_simple(const char *name, hi_s32 id, const void *res, hi_u32 num);
int osal_platform_device_register(void *pdev);
void osal_platform_device_unregister(void *pdev);
hi_s32 osal_platform_driver_register(void *drv);
void osal_platform_driver_unregister(void *drv);
void *osal_platform_get_resource_byname(void *dev, hi_u32 type, const hi_char *name);
void *osal_platform_get_resource(void *dev, hi_u32 type, hi_u32 num);
hi_s32 osal_platform_get_irq(void *dev, hi_u32 num);
hi_s32 osal_platform_get_irq_byname(void *dev, const hi_char *name);
hi_s32 osal_bus_register(void* bus);
hi_void osal_bus_unregister(void* bus);
hi_s32 osal_of_dma_configure(void *dev, void *np, hi_bool force_dma);

#define osal_module_driver(osal_driver, osal_register, osal_unregister, ...) \
    static hi_s32 __init osal_driver##_init(void)                               \
    {                                                                        \
        return osal_register(&(osal_driver));                                \
    }                                                                        \
    module_init(osal_driver##_init);                                         \
    static void __exit osal_driver##_exit(void)                              \
    {                                                                        \
        osal_unregister(&(osal_driver));                                     \
    }                                                                        \
    module_exit(osal_driver##_exit);

#define osal_module_platform_driver(platform_driver)                   \
    osal_module_driver(platform_driver, osal_platform_driver_register, \
        osal_platform_driver_unregister)

#define HI_TRACE_OSAL(level, fmt, ...) \
    do { \
        const hi_char *osal_function = __FUNCTION__; \
        hi_s32 line = __LINE__; \
        HI_TRACE(level, HI_ID_CMPI, "[osal]:%s [Line]:%d " fmt, osal_function, line, ##__VA_ARGS__); \
    } while (0)

#define HI_TRACE_MMZ(level, fmt, ...) \
    do { \
        const hi_char *function = __FUNCTION__; \
        hi_s32 line = __LINE__; \
        HI_TRACE(level, HI_ID_CMPI, "[mmz]:%s [Line]:%d " fmt, function, line, ##__VA_ARGS__); \
    } while (0)

#define OASL_OS_LINUX  0x0U
#define OASL_OS_AOSCORE 0x1U

hi_u32 osal_get_os_type(void);

#define OSAL_EXPORT_SYMBOL(sym) EXPORT_SYMBOL(sym)

#define osal_module_param(name, type, perm) module_param(name, type, perm)
#define osal_module_init(fn)  module_init(fn)
#define osal_module_exit(fn) module_exit(fn)
#define osal_module_refcount(mod) module_refcount(mod)
#define osal_module_put(mod) module_put(mod)

// iommu.h
hi_s32 osal_iommu_attach_group(void *domain, void *group);
hi_void osal_iommu_detach_group(void *domain, void *group);
hi_s32 osal_bus_set_iommu(void *bus, const void *ops);
hi_s32 osal_device_register(void *dev);
hi_void osal_device_unregister(void *dev);
hi_s32 osal_dev_set_name(void *dev, const char *name);
void osal_iommu_domain_free(void *domain);
void osal_iommu_group_put(void *group);
hi_s32 osal_iommu_map(void *domain, hi_ulong iova, hi_ulong paddr, hi_ulong size, hi_s32 prot);
hi_ulong osal_iommu_unmap(void *domain, unsigned long iova, hi_ulong size);
int osal_svm_get_pasid(pid_t vpid, int dev_id);
hi_void osal_iommu_sva_unbind_device(void *handle, hi_u32 ssid);
struct iommu_domain *osal_svm_get_dvpp_smmu_domain(hi_u32 idx);
void *osal_iommu_group_get(void *dev);
void *osal_iommu_domain_alloc(void *bus);
int osal_iommu_domain_set_attr(void *domain, hi_s32 attr, void *data);
hi_void osal_put_device(void *dev);
hi_s32 osal_iommu_sva_bind_device(void *dev, hi_s32 *pasid, hi_void** sva);
hi_s32 osal_iommu_sva_device_init(void *dev);
hi_s32 osal_iommu_sva_device_deinit(void *dev);
hi_u64 osal_iommu_iova_to_phys(struct iommu_domain *domain, hi_u64 iova);

// acpi
hi_s32 osal_acpi_dma_configure(void *dev, hi_s32 attr);

// of.h
int osal_of_property_read_u32(const void *np, const char *propname, hi_u32 *out_value);
hi_s32 osal_of_property_read_u64_array(const void *np, const char *propname, hi_u64 *out_values, size_t sz);
void *osal_of_irq_find_parent(void *child);
hi_s32 osal_of_property_count_u32_elems(const void *np, const char *propname);
hi_s32 osal_of_property_read_u32_index(const void *np, const char *propname, hi_u32 idx, hi_u32 *out_value);

// osal_property.c
hi_s32 osal_device_property_read_u64_array(void *dev, const char *propname, hi_u64 *val, size_t nval);
hi_s32 osal_device_property_read_string(void *dev, const char *propname, const char **val);
hi_s32 osal_device_property_read_u32(void *dev, const char *propname, hi_u32 *val);
hi_s32 osal_platform_get_irq(void *dev, hi_u32 num);

// notifier.c
hi_s32 osal_atomic_notifier_chain_register(void *nh, void *n);
hi_s32 osal_atomic_notifier_chain_unregister(void *nh, void *n);
hi_s32 osal_register_hisi_oom_notifier(void *nb);
hi_s32 osal_unregister_hisi_oom_notifier(void *nb);
hi_s32 osal_register_die_notifier(void *nb);
hi_s32 osal_unregister_die_notifier(void *nb);
hi_s32 osal_register_pm_notifier(void *nb);
hi_s32 osal_unregister_pm_notifier(void *nb);
hi_s32 osal_register_reboot_notifier(void *nb);
hi_s32 osal_unregister_reboot_notifier(void *nb);

// sharepool
hi_s32 osal_mg_sp_id_of_current(void);
hi_void *osal_mg_sp_make_share_k2u(hi_ulong kva, hi_ulong size,
    hi_ulong sp_flags, hi_s32 tgid, hi_s32 spg_id);
int osal_mg_sp_unshare(hi_ulong va, hi_ulong size, int spg_id);
void *osal_mg_sp_make_share_u2k(hi_ulong uva, hi_ulong size, int tgid);

void osal_set_print_alloc_mem_info(hi_bool flag);

void osal_mmput(void *mm);
void osal_mmget(void *mm);

// bootdot
hi_s32 osal_bootdot_sym_init(hi_void);
hi_void osal_bootdot_sym_deinit(hi_void);
hi_s32 osal_bootdot_init_blk(hi_u32 block_id, hi_u32 magic, hi_u32 execption_id, hi_u32 expect_status);
hi_s32 osal_bootdot_set_blk(hi_u32 block_id, hi_u32 magic, hi_u32 current_status);

#endif
