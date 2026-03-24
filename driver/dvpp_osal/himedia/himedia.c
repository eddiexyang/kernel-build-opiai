/*
 * linux/drivers/char/himedia.c
 *
 * HISILICON driver base frameforwk
 * Changed for hisilicon-media devices by Liu Jiandong 10-Dec-2007
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#ifndef AOS_LLVM_BUILD
#include <linux/kmod.h>
#endif
#ifdef AOS_LLVM_BUILD
#include <linux/cdev.h>
#endif
#include "base.h"
#include "hi_osal.h"

static OSAL_LIST_HEAD(himedia_list);
static DEFINE_MUTEX(himedia_sem);

/*
 * Assigned numbers, used for dynamic minors
 */
#define DYNAMIC_MINORS 64 /* like dynamic majors */
static hi_uchar himedia_minors[DYNAMIC_MINORS / 8];

static hi_s32 himedia_open(struct inode *inode, struct file *file)
{
    /* aoscore不支持设备注册，整体himedia框架改用register_driver功能替代 */
#ifndef AOS_LLVM_BUILD
    hi_u32 minor_dev_num = iminor(inode);
    struct himedia_device *c = NULL;
    hi_s32 err = -ENODEV;
    const struct file_operations *old_fops = NULL;
    const struct file_operations *new_fops = NULL;

    mutex_lock(&himedia_sem);

    osal_list_for_each_entry(c, &himedia_list, list) {
        if (c->minor == minor_dev_num) {
            new_fops = fops_get(c->fops);
            break;
        }
    }

    if (new_fops == NULL) {
        mutex_unlock(&himedia_sem);
        request_module("char-major-%d-%d", HIMEDIA_DEVICE_MAJOR, minor_dev_num);
        mutex_lock(&himedia_sem);

        osal_list_for_each_entry(c, &himedia_list, list) {
            if (c->minor == minor_dev_num) {
                new_fops = fops_get(c->fops);
                break;
            }
        }

        if (new_fops == NULL) {
            goto fail;
        }
    }

    err = 0;

    old_fops = file->f_op;
    file->f_op = new_fops;
    if (file->f_op->open != NULL) {
        file->private_data = c;
        err = file->f_op->open(inode, file);
        if (err != 0) {
            fops_put(file->f_op);
            file->private_data = NULL;
            file->f_op = fops_get(old_fops);
        }
    }

    fops_put(old_fops);
fail:
    mutex_unlock(&himedia_sem);
    return err;
#else
    return 0;
#endif // #ifndef AOS_LLVM_BUILD TODO
}

static struct file_operations himedia_fops = {
    .owner = THIS_MODULE,
    .open = himedia_open,
};

/**
 *    himedia_register - register a himedia device
 *    @himedia: device structure
 *
 *    Register a himedia device with the kernel. If the minor
 *    number is set to %HIMEDIA_DYNAMIC_MINOR a minor number is assigned
 *    and placed in the minor field of the structure. For other cases
 *    the minor number requested is used.
 *
 *    The structure passed is linked into the kernel and may not be
 *    destroyed until it has been unregistered.
 *
 *    A zero is returned on success and a negative errno code for
 *    failure.
 */

hi_s32 himedia_register(struct himedia_device *himedia)
{
    const struct himedia_device *ptmp = NULL;
    struct himedia_driver *pdrv = NULL;

    hi_s32 err = 0;

    mutex_lock(&himedia_sem);

    /* check if registered */
    osal_list_for_each_entry(ptmp, &himedia_list, list) {
        if (ptmp->minor == himedia->minor) {
            mutex_unlock(&himedia_sem);
            return -EBUSY;
        }
    }

    /* check minor */
    if (himedia->minor == HIMEDIA_DYNAMIC_MINOR) {
        hi_s32 i = DYNAMIC_MINORS;

        while (--i >= 0) {
            if ((himedia_minors[(hi_u32)i >> 3U] & (1U << ((hi_u32)i & 7U))) == 0) {
                break;
            }
        }
        if (i < 0) {
            mutex_unlock(&himedia_sem);
            return -EBUSY;
        }

        himedia->minor = i;
    }

    if (himedia->minor < DYNAMIC_MINORS) {
        himedia_minors[himedia->minor >> 3U] |= (1U << (himedia->minor & 7U));
    }

    /* device register */
    err = himedia_device_register(himedia);
    if (err < 0) {
        himedia_minors[himedia->minor >> 3U] &= ~(1U << (himedia->minor & 7U));
        goto out;
    }

    /* driver register */
    pdrv = himedia_driver_register(himedia->devfs_name, himedia->owner, himedia->drvops);
    if (IS_ERR(pdrv)) {
        himedia_device_unregister(himedia);

        himedia_minors[himedia->minor >> 3U] &= ~(1U << (himedia->minor & 7U));

        err = PTR_ERR(pdrv);
        goto out;
    }

    himedia->driver = pdrv;

    /*
     * Add it to the front, so that later devices can "override"
     * earlier defaults
     */
    osal_list_add(&himedia->list, &himedia_list);

out:
    mutex_unlock(&himedia_sem);
    return err;
}
EXPORT_SYMBOL(himedia_register);

/**
 *    himedia_unregister - unregister a himedia device
 *    @himedia: device to unregister
 *
 *    Unregister a himedia device that was previously
 *    successfully registered with himedia_register(). Success
 *    is indicated by a zero return, a negative errno code
 *    indicates an error.
 */

hi_s32 himedia_unregister(struct himedia_device *himedia)
{
    const struct himedia_device *ptmp = NULL;
    const struct himedia_device *tmp = NULL;

    if (osal_list_empty(&himedia->list) == HI_TRUE) {
        return -EINVAL;
    }

    mutex_lock(&himedia_sem);

    osal_list_for_each_entry_safe(ptmp, tmp, &himedia_list, list) {
        /* if found, unregister device & driver */
        if (ptmp->minor == himedia->minor) {
            osal_list_del(&himedia->list);

            himedia_driver_unregister(himedia->driver);

            himedia->driver = NULL;

            himedia_device_unregister(himedia);

            himedia_minors[himedia->minor >> 3U] &= ~(1U << (himedia->minor & 7U));

            break;
        }
    }

    mutex_unlock(&himedia_sem);

    return 0;
}
EXPORT_SYMBOL(himedia_unregister);

hi_s32 himedia_init(void)
{
    hi_s32 ret;
    // 1
#ifndef AOS_LLVM_BUILD
    ret = himedia_bus_init();
    if (ret != HI_SUCCESS) {
        goto err0;
    }
#endif

    // 2
    ret = -EIO;
#ifndef AOS_LLVM_BUILD
    if (register_chrdev(HIMEDIA_DEVICE_MAJOR, "himedia", &himedia_fops) != HI_SUCCESS) {
        goto err1;
    }
#endif
    HI_TRACE_OSAL(HI_DBG_INFO, "Module himedia: init ok\n");

    return 0;

    // 3
err1:
    himedia_bus_exit();
err0:
    return ret;
}

void himedia_exit(void)
{
    // 0
    if (!(osal_list_empty(&himedia_list) == HI_TRUE)) {
        HI_TRACE_OSAL(HI_DBG_WARN, "!!! Module himedia: sub module in list\n");
    }
#ifndef AOS_LLVM_BUILD
    unregister_chrdev(HIMEDIA_DEVICE_MAJOR, "himedia");
#endif
    himedia_bus_exit();

    HI_TRACE_OSAL(HI_DBG_INFO, "Module himedia: exit ok\n");
}
