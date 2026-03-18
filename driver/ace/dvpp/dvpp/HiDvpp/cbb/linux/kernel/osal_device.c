#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include "himedia.h"
#include <linux/fs.h>
#ifndef AOS_LLVM_BUILD
#include <linux/poll.h>
#include <linux/kmod.h>
#endif // #ifndef AOS_LLVM_BUILD
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm_types.h>
#include <linux/mm.h>
#include <linux/of_device.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#include <linux/uaccess.h>
#include "osal_ioctl.h"
#endif // #ifdef AOS_LLVM_BUILD
#include "hi_osal.h"
#include "securec.h"

#define HI_ERR_SYS_ERR 0xA0028015

#define GET_FILE(file)            \
    do {                          \
        if (add_file_refcnt(file) < 0) { \
            return -1;            \
        }                         \
    } while (0)

#define PUT_FILE(file)            \
    do {                          \
        if (sub_file_refcnt(file) < 0) { \
            return -1;            \
        }                         \
    } while (0)

typedef struct osal_coat_dev {
    struct osal_dev osal_dev;
    struct himedia_device himedia_dev;
} osal_coat_dev_t;

spinlock_t f_lock;

void osal_device_init(void)
{
    spin_lock_init(&f_lock);
}

static int32_t add_file_refcnt(const struct file *file)
{
    struct osal_private_data *pdata = NULL;

    spin_lock(&f_lock);
    pdata = file->private_data;
    if (pdata == NULL) {
        spin_unlock(&f_lock);
        return -1;
    }

    pdata->f_ref_cnt++;
    spin_unlock(&f_lock);

    return 0;
}

static int32_t sub_file_refcnt(const struct file *file)
{
    struct osal_private_data *pdata = NULL;

    spin_lock(&f_lock);
    pdata = file->private_data;
    if (pdata == NULL) {
        spin_unlock(&f_lock);
        return -1;
    }

    pdata->f_ref_cnt--;
    spin_unlock(&f_lock);

    return 0;
}

static hi_s32 osal_open(struct inode *inode, struct file *file)
{
    int32_t ret = 0;
    const struct himedia_device *himedia = NULL;
    osal_coat_dev_t *coat_dev = NULL;
    struct osal_private_data *pdata = NULL;
    HI_UNUSED(inode);

    himedia = (struct himedia_device *)file->private_data;
    if (himedia == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "get himedia device error!\n");
        return -1;
    }
    coat_dev = osal_container_of(himedia, struct osal_coat_dev, himedia_dev);
    pdata = (struct osal_private_data *)osal_kmalloc(sizeof(struct osal_private_data), osal_gfp_kernel);
    if (pdata == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }

#ifdef DRVAL_DEBUG
    HI_TRACE_OSAL(HI_DBG_DEBUG, "file->private_data=%pK!\n", pdata);
#endif // #ifdef DRVAL_DEBUG

    ret = memset_s(pdata, sizeof(struct osal_private_data), 0, sizeof(struct osal_private_data));
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_WARN, "%s - memset_s fail!\n", __FUNCTION__);
    }
    pdata->pid = current->pid;
    pdata->tgid = current->tgid;

    file->private_data = pdata;
    pdata->dev = &(coat_dev->osal_dev);

    if (!try_module_get(coat_dev->osal_dev.fops->module)) {
        osal_kfree(pdata);
        file->private_data = NULL;
        HI_TRACE_OSAL(HI_DBG_ERR, "module not alive!\n");
        return -1;
    }

    if (coat_dev->osal_dev.fops->open != NULL) {
        ret = coat_dev->osal_dev.fops->open((void *)&(pdata->data));
        if (ret != 0) {
            module_put(coat_dev->osal_dev.fops->module);
            osal_kfree(pdata);
            file->private_data = NULL;
        }
        return ret;
    }
    return 0;
}

static ssize_t osal_read(struct file *file, hi_char __user *buf, size_t size, loff_t *offset)
{
    struct osal_private_data *pdata = file->private_data;
    int32_t ret = 0;

    GET_FILE(file);

    if (pdata->dev->fops->read != NULL) {
        ret = pdata->dev->fops->read(buf, (int32_t)size, (hi_s64*)offset, (void *)&(pdata->data));
    }

    PUT_FILE(file);
    return ret;
}

static ssize_t osal_write(struct file *file, const hi_char __user *buf, size_t size, loff_t *offset)
{
    struct osal_private_data *pdata = file->private_data;
    int32_t ret = -1;

    GET_FILE(file);
    if (pdata->dev->fops->write != NULL) {
        ret = pdata->dev->fops->write(buf, (int32_t)size, (hi_s64*)offset, (void *)&(pdata->data));
    }
    PUT_FILE(file);
    return ret;
}

static loff_t osal_llseek(struct file *file, loff_t offset, hi_s32 whence)
{
    struct osal_private_data *const pdata = file->private_data;
    int32_t ret = 0;

    GET_FILE(file);
#ifdef DRVAL_DEBUG
    HI_TRACE_OSAL(HI_DBG_DEBUG, "file->private_data=%pK!\n", pdata);
#endif  // #ifdef DRVAL_DEBUG

    if ((pdata != NULL) && (pdata->dev != NULL) && (pdata->dev->fops != NULL) && (pdata->dev->fops->llseek != NULL)) {
        if (whence == SEEK_SET) {
            ret = pdata->dev->fops->llseek(offset, OSAL_SEEK_SET, (void *)&(pdata->data));
        } else if (whence == SEEK_CUR) {
            ret = pdata->dev->fops->llseek(offset, OSAL_SEEK_CUR, (void *)&(pdata->data));
        } else if (whence == SEEK_END) {
            ret = pdata->dev->fops->llseek(offset, OSAL_SEEK_END, (void *)&(pdata->data));
        } else {
            HI_TRACE_OSAL(HI_DBG_ERR, "err whence!\n");
        }
    }
    PUT_FILE(file);
    return (loff_t)ret;
}

static hi_s32 osal_release(struct inode *inode, struct file *file)
{
    int32_t ret = 0;
    struct osal_private_data *pdata = NULL;

    HI_UNUSED(inode);
    if (file == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "file is null!\n");
        return -1;
    }

    pdata = file->private_data;

    GET_FILE(file);

#ifdef DRVAL_DEBUG
    HI_TRACE_OSAL(HI_DBG_DEBUG, "file->private_data=%pK!\n", pdata);
#endif  // #ifdef DRVAL_DEBUG

    if (pdata == NULL) {
        PUT_FILE(file);
        HI_TRACE_OSAL(HI_DBG_ERR, "pdata is null!\n");
        return -1;
    }

    if (pdata->dev == NULL) {
        PUT_FILE(file);
        HI_TRACE_OSAL(HI_DBG_ERR, "pdata->dev is null!\n");
        return -1;
    }

    if (pdata->dev->fops == NULL) {
        PUT_FILE(file);
        HI_TRACE_OSAL(HI_DBG_ERR, "pdata->dev->fopss is null!\n");
        return -1;
    }

    if (pdata->tgid != current->tgid) {
        HI_TRACE_OSAL(HI_DBG_ERR, "pdata->dev:%s open-tgid=%d(%d) != current-tgid=%d(%d), release maybe abnormal!\n",
            pdata->dev->name, pdata->tgid, pdata->pid, current->tgid, current->pid);
    }

    if (pdata->dev->fops->release != NULL) {
        ret = pdata->dev->fops->release((void *)&(pdata->data));
    } else {
        if (pdata->dev->fops->release_ext != NULL) {
            ret = pdata->dev->fops->release_ext(pdata);
        }
    }

    if (ret != 0) {
        PUT_FILE(file);
        HI_TRACE_OSAL(HI_DBG_ERR, "release failed!\n");
        return ret;
    }

    module_put(pdata->dev->fops->module);

    PUT_FILE(file);
    spin_lock(&f_lock);
    if (pdata->f_ref_cnt != 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "release failed!\n");
        spin_unlock(&f_lock);
        return -1;
    }
    osal_kfree(file->private_data);
    file->private_data = NULL;
    spin_unlock(&f_lock);

    return 0;
}

static hi_slong osal_unlocked_ioctl_private(struct file *file, hi_ulong cmd, hi_ulong arg)
{
    int32_t ret = -1;
    struct osal_private_data *pdata = file->private_data;
    uint8_t *kbuf = NULL;

#ifdef DRVAL_DEBUG
    HI_TRACE_OSAL(HI_DBG_DEBUG, "file->private_data=%pK!\n", pdata);
#endif  // #ifdef DRVAL_DEBUG

    if (((_IOC_SIZE(cmd) == 0) && (_IOC_DIR(cmd) != _IOC_NONE))) {
        return HI_ERR_SYS_ERR;
    }

    if ((_IOC_DIR(cmd) != _IOC_NONE) && (((uint8_t *)(uintptr_t)arg) == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "Input param err,it is null!\n");
        return HI_ERR_SYS_ERR;
    }

    if (_IOC_DIR(cmd) == _IOC_NONE) {
        if (pdata->dev->fops->unlocked_ioctl == NULL) {
            return HI_ERR_SYS_ERR;
        } else {
            ret = pdata->dev->fops->unlocked_ioctl(cmd, arg, (void *)&(pdata->data));
        }
    } else if (_IOC_DIR(cmd) == _IOC_WRITE) {
        kbuf = (uint8_t *)osal_kmalloc(_IOC_SIZE(cmd), osal_gfp_atomic);
        if (kbuf == NULL) {
            HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc failed!\n");
            return HI_ERR_SYS_ERR;
        }

        if (copy_from_user(kbuf, (uint8_t *)(uintptr_t)arg, _IOC_SIZE(cmd)) != HI_SUCCESS) {
            osal_kfree(kbuf);
            return HI_ERR_SYS_ERR;
        }

        if (pdata->dev->fops->unlocked_ioctl == NULL) {
            osal_kfree(kbuf);
            return HI_ERR_SYS_ERR;
        } else {
            ret = pdata->dev->fops->unlocked_ioctl (cmd, (hi_ulong)(uintptr_t)kbuf, (void *)&(pdata->data));
        }
    } else if (_IOC_DIR(cmd) == _IOC_READ) {
        kbuf = (uint8_t*)osal_kmalloc(_IOC_SIZE(cmd), osal_gfp_atomic);
        if (kbuf == NULL) {
            HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc failed!\n");
            return HI_ERR_SYS_ERR;
        }
        (void)memset_s(kbuf, _IOC_SIZE(cmd), 0, _IOC_SIZE(cmd));
        if (pdata->dev->fops->unlocked_ioctl == NULL) {
            osal_kfree(kbuf);
            return HI_ERR_SYS_ERR;
        } else {
            ret = pdata->dev->fops->unlocked_ioctl (cmd, (hi_ulong)(uintptr_t)kbuf, (void *)&(pdata->data));
            if (ret == 0) {
                if (copy_to_user((uint8_t *)(uintptr_t)arg, kbuf, _IOC_SIZE(cmd)) != HI_SUCCESS) {
                    osal_kfree(kbuf);
                    return HI_ERR_SYS_ERR;
                }
            }
        }
    } else if (_IOC_DIR(cmd) == (_IOC_READ + _IOC_WRITE)) {
        kbuf = (uint8_t*)osal_kmalloc(_IOC_SIZE(cmd), osal_gfp_atomic);
        if (kbuf == NULL) {
            HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc failed!\n");
            return HI_ERR_SYS_ERR;
        }
        (void)memset_s(kbuf, _IOC_SIZE(cmd), 0, _IOC_SIZE(cmd));

        if (copy_from_user(kbuf, (uint8_t *)(uintptr_t)arg, _IOC_SIZE(cmd)) != HI_SUCCESS) {
            osal_kfree(kbuf);
            return HI_ERR_SYS_ERR;
        }

        if (pdata->dev->fops->unlocked_ioctl == NULL) {
            osal_kfree(kbuf);
            return HI_ERR_SYS_ERR;
        } else {
            ret = pdata->dev->fops->unlocked_ioctl (cmd, (hi_ulong)(uintptr_t)kbuf, (void *)&(pdata->data));
            if (ret == 0) {
                if (copy_to_user((uint8_t *)(uintptr_t)arg, kbuf, _IOC_SIZE(cmd)) != HI_SUCCESS) {
                    osal_kfree(kbuf);
                    return HI_ERR_SYS_ERR;
                }
            }
        }
    }

    if (kbuf != NULL) {
        osal_kfree(kbuf);
    }

    return ret;
}

static hi_slong osal_unlocked_ioctl(struct file *file, hi_u32 cmd, hi_ulong arg)
{
    hi_slong ret = 0;

    GET_FILE(file);

    ret = osal_unlocked_ioctl_private(file, cmd, arg);
    PUT_FILE(file);

    return ret;
}

#if defined CONFIG_COMPAT || defined AOS_LLVM_BUILD
static hi_slong osal_compat_ioctl_private(struct file *file, hi_u32 cmd, hi_ulong arg)
{
    hi_s32 ret = -1;
    struct osal_private_data *pdata = file->private_data;
    uint8_t *kbuf = NULL;

#ifdef DRVAL_DEBUG
    HI_TRACE_OSAL(HI_DBG_DEBUG, "file->private_data=%pK!\n", pdata);
#endif  // #ifdef DRVAL_DEBUG

    if (((_IOC_SIZE(cmd) == 0) && (_IOC_DIR(cmd) != _IOC_NONE))) {
        return -1;
    }

    if ((_IOC_DIR(cmd) != _IOC_NONE) && (((uint8_t *)(uintptr_t)arg) == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "Input param err,it is null!\n");
        return -1;
    }

    if (_IOC_DIR(cmd) == _IOC_NONE) {
        if (pdata->dev->fops->compat_ioctl == NULL) {
            return -1;
        } else {
            ret = pdata->dev->fops->compat_ioctl(cmd, arg, (void *)&(pdata->data));
        }
    } else if (_IOC_DIR(cmd) == _IOC_WRITE) {
        kbuf = (uint8_t *)vmalloc(_IOC_SIZE(cmd));
        if (kbuf == NULL) {
            HI_TRACE_OSAL(HI_DBG_ERR, "vmalloc failed!\n");
            return -1;
        }

        if (copy_from_user(kbuf, (uint8_t *)(uintptr_t)arg, _IOC_SIZE(cmd)) != HI_SUCCESS) {
            vfree(kbuf);
            return -1;
        }

        if (pdata->dev->fops->compat_ioctl == NULL) {
            vfree(kbuf);
            return -1;
        } else {
            ret = pdata->dev->fops->compat_ioctl (cmd, (hi_ulong)(uintptr_t)kbuf, (void *)&(pdata->data));
        }
    } else if (_IOC_DIR(cmd) == _IOC_READ) {
        kbuf = vmalloc(_IOC_SIZE(cmd));
        if (kbuf == NULL) {
            HI_TRACE_OSAL(HI_DBG_ERR, "vmalloc failed!\n");
            return -1;
        }
        (void)memset_s(kbuf, _IOC_SIZE(cmd), 0, _IOC_SIZE(cmd));
        if (pdata->dev->fops->compat_ioctl == NULL) {
            vfree(kbuf);
            return -1;
        } else {
            ret = pdata->dev->fops->compat_ioctl (cmd, (hi_ulong)(uintptr_t)kbuf, (void *)&(pdata->data));
            if (ret == 0) {
                if (copy_to_user((uint8_t *)(uintptr_t)arg, kbuf, _IOC_SIZE(cmd)) != HI_SUCCESS) {
                    vfree(kbuf);
                    return -1;
                }
            }
        }
    } else if (_IOC_DIR(cmd) == (_IOC_READ + _IOC_WRITE)) {
        kbuf = vmalloc(_IOC_SIZE(cmd));
        if (kbuf == NULL) {
            HI_TRACE_OSAL(HI_DBG_ERR, "vmalloc failed!\n");
            return -1;
        }
        (void)memset_s(kbuf, _IOC_SIZE(cmd), 0, _IOC_SIZE(cmd));
        if (copy_from_user(kbuf, (uint8_t *)(uintptr_t)arg, _IOC_SIZE(cmd)) != HI_SUCCESS) {
            vfree(kbuf);
            return -1;
        }

        if (pdata->dev->fops->compat_ioctl == NULL) {
            vfree(kbuf);
            return -1;
        } else {
            ret = pdata->dev->fops->compat_ioctl (cmd, (hi_ulong)(uintptr_t)kbuf, (void *)&(pdata->data));
            if (ret == 0) {
                if (copy_to_user((uint8_t *)(uintptr_t)arg, kbuf, _IOC_SIZE(cmd)) != HI_SUCCESS) {
                    vfree(kbuf);
                    return -1;
                }
            }
        }
    } else {
        HI_TRACE_OSAL(HI_DBG_ERR, "err cmd direction.\n");
        return -1;
    }

    if (kbuf != NULL) {
        vfree(kbuf);
    }

    return ret;
}

static hi_slong osal_compat_ioctl(struct file *file, hi_u32 cmd, hi_ulong arg)
{
    hi_slong ret = 0;

    GET_FILE(file);

    ret = osal_compat_ioctl_private(file, cmd, arg);
    PUT_FILE(file);

    return ret;
}

#endif

#ifndef AOS_LLVM_BUILD
static hi_u32 osal_poll(struct file *file, struct poll_table_struct *table)
{
    struct osal_private_data *pdata = file->private_data;
    struct osal_poll t;
    hi_u32 ret = 0;

    if (add_file_refcnt(file) < 0) {
        return OSAL_POLLERR;
    }

#ifdef DRVAL_DEBUG
    HI_TRACE_OSAL(HI_DBG_ERR, "table=%pK, file=%pK!\n", table, file);
#endif  // #ifdef DRVAL_DEBUG
    t.poll_table = table;
    t.data = file;
    if (pdata->dev->fops->poll != NULL) {
        ret = pdata->dev->fops->poll(&t, (void *)&(pdata->data));
    }

    if (sub_file_refcnt(file) < 0) {
        return OSAL_POLLERR;
    }

    return ret;
}

static hi_s32 osal_mmap(struct file *file, struct vm_area_struct *vm)
{
    struct osal_vm osal_vm;
    struct osal_private_data *pdata = file->private_data;
    osal_vm.vm = vm;
    if (vm == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "vm is null!\n");
        return -1;
    }
    if (pdata == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "pdata is null!\n");
        return -1;
    }
    if (pdata->dev->fops->mmap != NULL) {
        return pdata->dev->fops->mmap(&osal_vm, vm->vm_start, vm->vm_end, vm->vm_pgoff, (void *)&(pdata->data));
    }
    return 0;
}

static struct file_operations s_osal_fops = {
    .owner = THIS_MODULE,
    .open = osal_open,
    .read = osal_read,
    .write = osal_write,
    .llseek = osal_llseek,
    .unlocked_ioctl = osal_unlocked_ioctl,
    .release = osal_release,
    .poll = osal_poll,
    .mmap = osal_mmap,
#ifdef CONFIG_COMPAT
    .compat_ioctl = osal_compat_ioctl,
#endif
};
#endif // #ifndef AOS_LLVM_BUILD

static hi_s32 osal_aos_open(struct inode *inode, struct file *file)
{
    osal_dev_t *osal_dev = NULL;
    struct osal_private_data *pdata = NULL;
    hi_s32 ret = 0;

    osal_dev = (osal_dev_t *)inode->i_private;
    if (osal_dev == NULL || osal_dev->fops == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "get osal_dev device of fops error!\n");
        return -1;
    }
    pdata = (struct osal_private_data *)kmalloc(sizeof(struct osal_private_data), GFP_KERNEL);
    if (pdata == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }

    ret = memset_s(pdata, sizeof(struct osal_private_data), 0, sizeof(struct osal_private_data));
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_WARN, "%s - memset_s fail!\n", __FUNCTION__);
    }

    file->private_data = pdata;
    pdata->dev = osal_dev;
    if (osal_dev->fops->open != NULL) {
        return osal_dev->fops->open((void *)&(pdata->data));
    }
    return 0;
}

static struct file_operations aos_osal_fops = {
    .owner = THIS_MODULE,
    .open = osal_aos_open,
    .read = osal_read,
    .write = osal_write,
    .release = osal_release,
#ifdef AOS_LLVM_BUILD
    .ioctl = osal_compat_ioctl,
#endif // #ifndef AOS_LLVM_BUILD
};

static hi_s32 osal_pm_prepare(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_prepare != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_prepare(&(coat_dev->osal_dev));
    }
    return 0;
}

static void osal_pm_complete(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_complete != HI_NULL)) {
        coat_dev->osal_dev.osal_pmops->pm_complete(&(coat_dev->osal_dev));
    }
}

static hi_s32 osal_pm_suspend(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_suspend != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_suspend(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_resume(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_resume != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_resume(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_freeze(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_freeze != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_freeze(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_thaw(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_thaw != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_thaw(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_poweroff(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_poweroff != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_poweroff(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_restore(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_restore != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_restore(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_suspend_late(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_suspend_late != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_suspend_late(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_resume_early(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_resume_early != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_resume_early(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_freeze_late(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_freeze_late != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_freeze_late(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_thaw_early(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_thaw_early != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_thaw_early(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_poweroff_late(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_poweroff_late != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_poweroff_late(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_restore_early(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_restore_early != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_restore_early(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_suspend_noirq(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_suspend_noirq != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_suspend_noirq(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_resume_noirq(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_resume_noirq != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_resume_noirq(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_freeze_noirq(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_freeze_noirq != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_freeze_noirq(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_thaw_noirq(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_thaw_noirq != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_thaw_noirq(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_poweroff_noirq(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_poweroff_noirq != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_poweroff_noirq(&(coat_dev->osal_dev));
    }
    return 0;
}

static hi_s32 osal_pm_restore_noirq(struct himedia_device *himedia)
{
    osal_coat_dev_t *const coat_dev = container_of(himedia, struct osal_coat_dev, himedia_dev);
    if ((coat_dev->osal_dev.osal_pmops != HI_NULL) && (coat_dev->osal_dev.osal_pmops->pm_restore_noirq != HI_NULL)) {
        return coat_dev->osal_dev.osal_pmops->pm_restore_noirq(&(coat_dev->osal_dev));
    }
    return 0;
}

static struct himedia_ops s_osal_pmops = {
    .pm_prepare = osal_pm_prepare,
    .pm_complete = osal_pm_complete,
    .pm_suspend = osal_pm_suspend,
    .pm_resume = osal_pm_resume,
    .pm_freeze = osal_pm_freeze,
    .pm_thaw = osal_pm_thaw,
    .pm_poweroff = osal_pm_poweroff,
    .pm_restore = osal_pm_restore,
    .pm_suspend_late = osal_pm_suspend_late,
    .pm_resume_early = osal_pm_resume_early,
    .pm_freeze_late = osal_pm_freeze_late,
    .pm_thaw_early = osal_pm_thaw_early,
    .pm_poweroff_late = osal_pm_poweroff_late,
    .pm_restore_early = osal_pm_restore_early,
    .pm_suspend_noirq = osal_pm_suspend_noirq,
    .pm_resume_noirq = osal_pm_resume_noirq,
    .pm_freeze_noirq = osal_pm_freeze_noirq,
    .pm_thaw_noirq = osal_pm_thaw_noirq,
    .pm_poweroff_noirq = osal_pm_poweroff_noirq,
    .pm_restore_noirq = osal_pm_restore_noirq,
};

osal_dev_t *osal_createdev(const hi_char *name)
{
    int32_t ret = 0;
    osal_coat_dev_t *pdev = NULL;
    if (name == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return NULL;
    }
    pdev = (osal_coat_dev_t *)osal_kmalloc(sizeof(osal_coat_dev_t), osal_gfp_kernel);
    if (pdev == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return NULL;
    }
    ret = memset_s(pdev, sizeof(osal_coat_dev_t), 0, sizeof(osal_coat_dev_t));
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "%s - memset_s error!\n", __FUNCTION__);
    }
#ifndef AOS_LLVM_BUILD
    ret = strncpy_s(pdev->osal_dev.name, sizeof(pdev->osal_dev.name), name, sizeof(pdev->osal_dev.name) - 1UL);
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "%s - strncpy_s error!\n", __FUNCTION__);
    }
#else
#define STRLEN 5
    ret = strncpy_s(pdev->osal_dev.name, sizeof(pdev->osal_dev.name), "/dev/", sizeof("/dev/") - 1UL);
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "%s - strncpy_s error!\n", __FUNCTION__);
    }
    ret = strncpy_s(&pdev->osal_dev.name[STRLEN], (sizeof(pdev->osal_dev.name) - STRLEN), name,
        (sizeof(pdev->osal_dev.name) - 1UL - STRLEN));
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "%s - strncpy_s error!\n", __FUNCTION__);
    }
    HI_TRACE_OSAL(HI_DBG_INFO, "pdev->osal_dev.name:%s!\n", pdev->osal_dev.name);
#endif  // #ifndef AOS_LLVM_BUILD
    pdev->osal_dev.dev = pdev;
    return &(pdev->osal_dev);
}
EXPORT_SYMBOL(osal_createdev);

hi_s32 osal_destroydev(const osal_dev_t *osal_dev)
{
    const osal_coat_dev_t *pdev = NULL;
    if (osal_dev == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    pdev = osal_dev->dev;
    if (pdev == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    osal_kfree(pdev);
    return 0;
}
EXPORT_SYMBOL(osal_destroydev);

hi_s32 osal_registerdevice(osal_dev_t *osal_dev)
{
#ifndef AOS_LLVM_BUILD
    int32_t ret = 0;
    struct himedia_device *himedia = NULL;
    if ((osal_dev == NULL) || (osal_dev->fops == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, " parameter invalid!\n");
        return -1;
    }
    himedia = &(((osal_coat_dev_t *)(osal_dev->dev))->himedia_dev);
    if (osal_dev->minor != 0) {
        himedia->minor = osal_dev->minor;
    } else {
        himedia->minor = HIMEDIA_DYNAMIC_MINOR;
    }
    himedia->owner = THIS_MODULE;
    himedia->fops = &s_osal_fops;
    himedia->drvops = &s_osal_pmops;
    ret = strncpy_s(himedia->devfs_name, sizeof(himedia->devfs_name),
                    osal_dev->name, sizeof(himedia->devfs_name) - 1UL);
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_WARN, "%s - strncpy_s fail!\n", __FUNCTION__);
    }
    return himedia_register(himedia);
#else // #ifndef AOS_LLVM_BUILD
    const mode_t mode = 0660U;
    if ((osal_dev == NULL) || (osal_dev->fops == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, " parameter invalid!\n");
        return -1;
    }
    HI_TRACE_OSAL(HI_DBG_WARN, "register_driver %s - osal_dev->name!\n", osal_dev->name);
    return register_driver(osal_dev->name, &aos_osal_fops, mode, osal_dev);
#endif // #ifndef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_registerdevice);

void osal_deregisterdevice(const osal_dev_t *pdev)
{
    if (pdev == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    (void)himedia_unregister((struct himedia_device *const)&(((osal_coat_dev_t *const)(pdev->dev))->himedia_dev));
}
EXPORT_SYMBOL(osal_deregisterdevice);

void osal_poll_wait(osal_poll_t *table, osal_wait_t *wait)
{
#ifndef AOS_LLVM_BUILD
#ifdef DRVAL_DEBUG
    HI_TRACE_OSAL(HI_DBG_DEBUG, "call poll_wait +!, table=%pK, file=%pK\n", table->poll_table, table->data);
#endif  // #ifdef DRVAL_DEBUG

    poll_wait((struct file *)table->data, (wait_queue_head_t *)(wait->wait), table->poll_table);

#ifdef DRVAL_DEBUG
    HI_TRACE_OSAL(HI_DBG_DEBUG, "call poll_wait -!\n");
#endif  // #ifdef DRVAL_DEBUG
#endif  // #ifndef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_poll_wait);

void osal_pgprot_noncached(const osal_vm_t *vm)
{
#ifndef AOS_LLVM_BUILD
    struct vm_area_struct *const v = (struct vm_area_struct *const)(vm->vm);
    v->vm_page_prot = pgprot_noncached(v->vm_page_prot);
#endif  // #ifndef AOS_LLVM_BUILD
}

EXPORT_SYMBOL(osal_pgprot_noncached);

void osal_pgprot_cached(const osal_vm_t *vm)
{
#ifndef AOS_LLVM_BUILD
    struct vm_area_struct *const v = (struct vm_area_struct *const)(vm->vm);

#ifdef CONFIG_64BIT
    v->vm_page_prot = __pgprot(pgprot_val(v->vm_page_prot) | PTE_VALID | PTE_DIRTY | PTE_AF);
#else

    v->vm_page_prot =
        __pgprot(pgprot_val(v->vm_page_prot) | L_PTE_PRESENT | L_PTE_YOUNG | L_PTE_DIRTY | L_PTE_MT_DEV_CACHED);
#endif
#endif  // #ifndef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_pgprot_cached);

void osal_pgprot_writecombine(const osal_vm_t *vm)
{
#ifndef AOS_LLVM_BUILD
    struct vm_area_struct *const v = (struct vm_area_struct *const)(vm->vm);
    v->vm_page_prot = pgprot_writecombine(v->vm_page_prot);
#endif  // #ifndef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_pgprot_writecombine);

void osal_pgprot_stronglyordered(const osal_vm_t *vm)
{
#ifndef AOS_LLVM_BUILD
    struct vm_area_struct *const v = (struct vm_area_struct *const)(vm->vm);

#ifdef CONFIG_64BIT
    v->vm_page_prot = pgprot_device(v->vm_page_prot);
#else
    v->vm_page_prot = pgprot_stronglyordered(v->vm_page_prot);
#endif
#endif  // #ifndef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_pgprot_stronglyordered);

hi_s32 osal_remap_pfn_range(const osal_vm_t *vm, hi_ulong addr, hi_ulong pfn, hi_ulong size)
{
#ifndef AOS_LLVM_BUILD
    struct vm_area_struct *const v = (struct vm_area_struct *const)(vm->vm);
    if (size == 0) {
        return -EPERM;
    }
    return remap_pfn_range(v, addr, pfn, size, v->vm_page_prot);
#else   // #ifndef AOS_LLVM_BUILD
    return 0;
#endif  // #ifndef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_remap_pfn_range);

hi_s32 osal_io_remap_pfn_range(const osal_vm_t *vm, hi_ulong addr, hi_ulong pfn, hi_ulong size)
{
#ifndef AOS_LLVM_BUILD
    struct vm_area_struct *const v = (struct vm_area_struct *const)(vm->vm);
    v->vm_flags |= (hi_ulong)VM_IO;
    if (size == 0) {
        return -EPERM;
    }
    return io_remap_pfn_range(v, addr, pfn, size, v->vm_page_prot);
#else   // #ifndef AOS_LLVM_BUILD
    return 0;
#endif  // #ifndef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_io_remap_pfn_range);

#ifdef CONFIG_HISI_SNAPSHOT_BOOT
hi_s32 osal_call_usermodehelper_force(hi_char *path, hi_char **argv, hi_char **envp, hi_s32 wait)
{
    return call_usermodehelper_force(path, argv, envp, wait);
}
EXPORT_SYMBOL(osal_call_usermodehelper_force);
#endif

hi_s32 osal_bus_register(hi_void *bus)
{
    return bus_register((struct bus_type *)bus);
}
EXPORT_SYMBOL(osal_bus_register);

hi_void osal_bus_unregister(hi_void *bus)
{
    bus_unregister((struct bus_type *)bus);
}
EXPORT_SYMBOL(osal_bus_unregister);

hi_s32 osal_of_dma_configure(void *dev, void *np, hi_bool force_dma)
{
    return of_dma_configure((struct device *)dev, (struct device_node *)np, force_dma);
}
EXPORT_SYMBOL(osal_of_dma_configure);

hi_s32 osal_device_register(void *dev)
{
    return device_register((struct device *)dev);
}
EXPORT_SYMBOL(osal_device_register);

hi_void osal_device_unregister(void *dev)
{
    device_unregister((struct device*)dev);
}
EXPORT_SYMBOL(osal_device_unregister);

hi_s32 osal_dev_set_name(void *dev, const char *name)
{
    if ((dev == NULL) || (name == NULL)) {
        return -EINVAL;
    }
    return dev_set_name((struct device*)dev, "%s", name);
}
EXPORT_SYMBOL(osal_dev_set_name);

hi_void osal_put_device(void *dev)
{
    put_device((struct device*)dev);
}
EXPORT_SYMBOL(osal_put_device);