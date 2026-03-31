#include <linux/module.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/version.h>
#ifndef AOS_LLVM_BUILD
#include <linux/kmod.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#include <linux/device/bus.h>
#endif
#endif
#include "base.h"
#include "securec.h"

/*****************************************************************************/
/**** himedia bus  ****/
/*****************************************************************************/
static void himedia_bus_release(struct device *dev)
{
    HI_UNUSED(dev);
    return;
}

static struct device himedia_bus = {
#if defined(AOS_LLVM_BUILD)
    .name = "himedia",
#else
    .init_name = "himedia",
#endif
    .release = himedia_bus_release
};


/* bus match & uevent */
static hi_s32 himedia_match(struct device *dev, struct device_driver *drv)
{
    const struct himedia_device *pdev = to_himedia_device(dev);
    return (strncmp(pdev->devfs_name, drv->name, sizeof(pdev->devfs_name)) == 0);
}

static hi_s32 himedia_uevent(struct device *dev, struct kobj_uevent_env *env)
{
#ifndef AOS_LLVM_BUILD
    const struct himedia_device *pdev = to_himedia_device(dev);
    (void)add_uevent_var(env, "MODALIAS=himedia:%s", pdev->devfs_name);
#endif
    return 0;
}

/*****************************************************************************/
// pm methods
static hi_s32 himedia_pm_prepare(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_prepare == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_prepare(pdev);
}

static void himedia_pm_complete(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_complete == NULL)) {
        return;
    }

    pdrv->ops->pm_complete(pdev);
}

static hi_s32 himedia_pm_suspend(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_suspend == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_suspend(pdev);
}

static hi_s32 himedia_pm_resume(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_resume == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_resume(pdev);
}

static hi_s32 himedia_pm_freeze(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_freeze == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_freeze(pdev);
}

static hi_s32 himedia_pm_thaw(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_thaw == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_thaw(pdev);
}

static hi_s32 himedia_pm_poweroff(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_poweroff == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_poweroff(pdev);
}

static hi_s32 himedia_pm_restore(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_restore == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_restore(pdev);
}
static hi_s32 himedia_pm_suspend_noirq(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_suspend_noirq == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_suspend_noirq(pdev);
}

static hi_s32 himedia_pm_resume_noirq(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_resume_noirq == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_resume_noirq(pdev);
}

static hi_s32 himedia_pm_freeze_noirq(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_freeze_noirq == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_freeze_noirq(pdev);
}

static hi_s32 himedia_pm_thaw_noirq(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_thaw_noirq == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_thaw_noirq(pdev);
}

static hi_s32 himedia_pm_poweroff_noirq(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_poweroff_noirq == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_poweroff_noirq(pdev);
}

static hi_s32 himedia_pm_restore_noirq(struct device *dev)
{
    struct himedia_device *const pdev = to_himedia_device(dev);
    const struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_restore_noirq == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_restore_noirq(pdev);
}

static struct dev_pm_ops himedia_bus_pm_ops = {
    .prepare = himedia_pm_prepare,
    .complete = himedia_pm_complete,

    // with irq
    .suspend = himedia_pm_suspend,
    .resume = himedia_pm_resume,
#ifndef AOS_LLVM_BUILD
    .freeze = himedia_pm_freeze,
    .thaw = himedia_pm_thaw,
    .poweroff = himedia_pm_poweroff,
    .restore = himedia_pm_restore,

    // with noirq
    .suspend_noirq = himedia_pm_suspend_noirq,
    .resume_noirq = himedia_pm_resume_noirq,
    .freeze_noirq = himedia_pm_freeze_noirq,
    .thaw_noirq = himedia_pm_thaw_noirq,
    .poweroff_noirq = himedia_pm_poweroff_noirq,
    .restore_noirq = himedia_pm_restore_noirq,
#endif // #ifndef AOS_LLVM_BUILD TODO
};

static struct bus_type himedia_bus_type = {
    .name = "himedia",
    .match = himedia_match,
#ifndef AOS_LLVM_BUILD
    .uevent = himedia_uevent,
#endif // #ifndef AOS_LLVM_BUILD TODO
    .pm = &himedia_bus_pm_ops,
};

hi_s32 himedia_bus_init(void)
{
    hi_s32 ret;
    ret = device_register(&himedia_bus);
    if (ret != 0) {
        return ret;
    }

    ret = bus_register(&himedia_bus_type);
    if (ret != 0) {
        goto error;
    }

    return 0;
error:

    device_unregister(&himedia_bus);
    return ret;
}

void himedia_bus_exit(void)
{
    bus_unregister(&himedia_bus_type);
    device_unregister(&himedia_bus);
}

static void himedia_device_release(struct device *dev)
{
    HI_UNUSED(dev);
    return;
}

hi_s32 himedia_device_register(struct himedia_device *pdev)
{
#ifndef AOS_LLVM_BUILD
    (void)dev_set_name(&pdev->device, "%s", pdev->devfs_name);

    pdev->device.devt = MKDEV(HIMEDIA_DEVICE_MAJOR, pdev->minor);
    pdev->device.release = himedia_device_release;
    pdev->device.bus = &himedia_bus_type;

    return device_register(&pdev->device);
#else
    return 0;
#endif
}

void himedia_device_unregister(struct himedia_device *pdev)
{
    device_unregister(&pdev->device);
}

struct himedia_driver *himedia_driver_register(const hi_char *name,
                                               struct module *owner, struct himedia_ops *ops)
{
    hi_s32 ret = 0;
    struct himedia_driver *pdrv = NULL;

    if ((name == NULL) || (owner == NULL)) {
        return ERR_PTR(-EINVAL);
    }

    pdrv = kzalloc(sizeof(struct himedia_driver) + strnlen(name, HIMIDIA_MAX_DEV_NAME_LEN), GFP_KERNEL);
    if (pdrv == NULL) {
        return ERR_PTR(-ENOMEM);
    }

    /* init driver object */
    ret = strncpy_s(pdrv->name, strnlen(name, HIMIDIA_MAX_DEV_NAME_LEN) + 1UL,
                    name, strnlen(name, HIMIDIA_MAX_DEV_NAME_LEN));
    if (ret != 0) {
        kfree(pdrv);
        return ERR_PTR(-ENOMEM);
    }

    pdrv->ops = ops;

    pdrv->driver.name = pdrv->name;
#ifndef AOS_LLVM_BUILD
    pdrv->driver.owner = owner;
#endif // #ifndef AOS_LLVM_BUILD TODO
    pdrv->driver.bus = &himedia_bus_type;

    ret = driver_register(&pdrv->driver);
    if (ret != 0) {
        kfree(pdrv);
        return ERR_PTR(ret);
    }

    return pdrv;
}

void himedia_driver_unregister(struct himedia_driver *pdrv)
{
    if (pdrv != NULL) {
        driver_unregister(&pdrv->driver);
        kfree(pdrv);
    }
}

// end!
/*****************************************************************************/

