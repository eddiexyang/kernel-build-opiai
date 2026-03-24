#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/version.h>

#include "hi_osal.h"
#include "securec.h"

#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif // #ifdef AOS_LLVM_BUILD
static struct osal_list_head osal_proc_list;
static osal_proc_entry_t *proc_entry = NULL;

static hi_s32 osal_seq_show(struct seq_file *s, void *p)
{
    hi_s32 ret = 0;
    const osal_proc_entry_t *oldsentry = s->private;
    osal_proc_entry_t sentry;

    HI_UNUSED(p);

    if (oldsentry == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    ret = memset_s(&sentry, sizeof(osal_proc_entry_t), 0, sizeof(osal_proc_entry_t));
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_WARN, "%s - memset_s fail!\n", __FUNCTION__);
    }

    /* only these two parameters are used */
    sentry.seqfile = s;
    sentry.private_elem = oldsentry->private_elem;
    oldsentry->read(&sentry);
    return 0;
}

#ifdef DVPP_VCAST_UTST
static ssize_t osal_procwrite(struct file *file, const char *buf, size_t count, loff_t *ppos)
#else // #ifdef DVPP_VCAST_UTST
static ssize_t osal_procwrite(struct file *file, const hi_char __user *buf, size_t count, loff_t *ppos)
#endif // #ifdef DVPP_VCAST_UTST
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
    osal_proc_entry_t *sentry = ((struct seq_file *)(file->private_data))->private;
    return sentry->write(sentry, (hi_char *)buf, (hi_s32)count, (hi_s64 *)ppos);
#else
    osal_proc_entry_t *item = PDE_DATA(file_inode(file));
    if ((item != NULL) && (item->write != NULL)) {
        return item->write(item, buf, (hi_s32)count, (hi_s64 *)ppos);
    }

    return -ENOSYS;
#endif
}

static hi_s32 osal_procopen(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
    osal_proc_entry_t *sentry = PDE(inode)->data;
#else
    osal_proc_entry_t *sentry = PDE_DATA(inode);
#endif
    if ((sentry != NULL) && (sentry->open != NULL)) {
        sentry->open(sentry);
    }
    return single_open(file, osal_seq_show, sentry);
}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || defined AOS_LLVM_BUILD
static struct file_operations osal_proc_ops = {
    .owner = THIS_MODULE,
    .open = osal_procopen,
    .read = seq_read,
    .write = osal_procwrite,
#ifndef AOS_LLVM_BUILD
    .llseek = seq_lseek,
#endif // #ifndef AOS_LLVM_BUILD
    .release = single_release
};
#else
static const struct proc_ops osal_proc_ops = {
    .proc_open = osal_procopen,
    .proc_read = seq_read,
    .proc_write = osal_procwrite,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
static void osal_create_proc_version_branch(
    const hi_char *name, const osal_proc_entry_t *parent, osal_proc_entry_t **sentry)
{
    hi_s32 ret = 0;
    struct proc_dir_entry *entry = NULL;

    if (parent == NULL) {
        HI_TRACE_OSAL(HI_DBG_INFO, "parent is NULL!\n");
        entry = create_proc_entry(name, 0, NULL);
    } else {
        HI_TRACE_OSAL(HI_DBG_INFO, "parent is not NULL! parent=%pxK\n", parent->proc_dir_entry);
        entry = create_proc_entry(name, 0, parent->proc_dir_entry);
    }
    if (entry == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "create_proc_entry failed!\n");
        return;
    }
    *sentry = osal_kmalloc(sizeof(struct osal_proc_dir_entry), osal_gfp_kernel);
    if ((*sentry) == NULL) {
        if (parent != NULL) {
            remove_proc_entry(name, parent->proc_dir_entry);
        } else {
            remove_proc_entry(name, NULL);
        }
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc failed!\n");
        return;
    }

    ret = memset_s(*sentry, sizeof(struct osal_proc_dir_entry), 0, sizeof(struct osal_proc_dir_entry));
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_WARN, "%s line:%d - memset_s failed!\n", __FUNCTION__, __LINE__);
    }

    ret = strncpy_s((*sentry)->name, sizeof((*sentry)->name), name, sizeof((*sentry)->name) - 1);
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_WARN, "%s line:%d - strncpy_s failed!\n", __FUNCTION__, __LINE__);
    }
    (*sentry)->proc_dir_entry = entry;
    (*sentry)->open = NULL;
    entry->proc_fops = &osal_proc_ops;
    entry->data = *sentry;
}
#else // #if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
static void osal_create_proc_version_branch(
    const hi_char *name, const osal_proc_entry_t *parent, osal_proc_entry_t **sentry)
{
    hi_s32 ret = 0;
    struct proc_dir_entry *entry = NULL;

    *sentry = osal_kmalloc(sizeof(struct osal_proc_dir_entry), osal_gfp_kernel);
    if ((*sentry) == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc failed!\n");
        return;
    }

    ret = memset_s(*sentry, sizeof(struct osal_proc_dir_entry), 0, sizeof(struct osal_proc_dir_entry));
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_WARN, "memset_s failed!\n");
    }

    ret = strncpy_s((*sentry)->name, sizeof((*sentry)->name), name, sizeof((*sentry)->name) - 1U);
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_WARN, "strncpy_s failed!\n");
    }

    if (parent == NULL) {
        entry = proc_create_data(name, 0, NULL, &osal_proc_ops, *sentry);
    } else {
        entry = proc_create_data(name, 0, parent->proc_dir_entry, &osal_proc_ops, *sentry);
    }
    if (entry == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "create_proc_entry failed!\n");
        osal_kfree(*sentry);
        *sentry = NULL;
        return;
    }
    (*sentry)->proc_dir_entry = entry;
    (*sentry)->open = NULL;
}
#endif // #if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)

static osal_proc_entry_t *osal_create_proc(const hi_char *name, const osal_proc_entry_t *parent)
{
    osal_proc_entry_t *sentry = NULL;
    osal_create_proc_version_branch(name, parent, &sentry);
    osal_list_add_tail(&(sentry->node), &osal_proc_list);
    return sentry;
}

static void osal_remove_proc(const hi_char *name, const osal_proc_entry_t *parent)
{
    struct osal_proc_dir_entry *sproc = NULL;

    if (name == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    if (parent != NULL) {
        remove_proc_entry(name, parent->proc_dir_entry);
    } else {
        remove_proc_entry(name, NULL);
    }
    osal_list_for_each_entry(sproc, (&osal_proc_list), node) {
        if (osal_strncmp(sproc->name, name, sizeof(sproc->name)) == 0) {
            osal_list_del(&(sproc->node));
            break;
        }
    }
    if (sproc != NULL) {
        osal_kfree(sproc);
    }
}

osal_proc_entry_t *osal_create_proc_entry(const hi_char *name, osal_proc_entry_t *parent)
{
    parent = proc_entry;

    return osal_create_proc(name, parent);
}
EXPORT_SYMBOL(osal_create_proc_entry);

void osal_remove_proc_entry(const hi_char *name, const osal_proc_entry_t *parent)
{
    parent = proc_entry;
    osal_remove_proc(name, parent);
    return;
}
EXPORT_SYMBOL(osal_remove_proc_entry);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
static void osal_proc_mkdir_version1(
    const hi_char *name, const osal_proc_entry_t *parent, struct osal_proc_dir_entry **sproc)
{
    hi_s32 ret = 0;
    struct proc_dir_entry *proc = NULL;

    if (parent != NULL) {
        proc = proc_mkdir(name, parent->proc_dir_entry);
        HI_TRACE_OSAL(HI_DBG_INFO, "parent is not NULL!\n");
    } else {
        proc = proc_mkdir(name, NULL);
        HI_TRACE_OSAL(HI_DBG_INFO, "parent is NULL! proc=%pxK \n", proc);
    }

    if (proc == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "proc_mkdir failed!\n");
        return;
    }

    *sproc = osal_kmalloc(sizeof(struct osal_proc_dir_entry), osal_gfp_kernel);
    if ((*sproc) == NULL) {
        proc_remove(proc);
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc failed!\n");
        return;
    }

    ret = memset_s(*sproc, sizeof(struct osal_proc_dir_entry), 0, sizeof(struct osal_proc_dir_entry));
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_WARN, "%s - memset_s failed!\n", __FUNCTION__);
    }

    ret = strncpy_s((*sproc)->name, sizeof((*sproc)->name), name, sizeof((*sproc)->name) - 1);
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_WARN, "%s - strncpy_s failed!\n", __FUNCTION__);
    }

    (*sproc)->proc_dir_entry = proc;
    proc->data = *sproc;
}
#else // #if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
static void osal_proc_mkdir_version_branch(
    const hi_char *name, const osal_proc_entry_t *parent, struct osal_proc_dir_entry **sproc)
{
    hi_s32 ret = 0;
    struct proc_dir_entry *proc = NULL;

    *sproc = osal_kmalloc(sizeof(struct osal_proc_dir_entry), osal_gfp_kernel);
    if ((*sproc) == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc failed!\n");
        return;
    }

    ret = memset_s(*sproc, sizeof(struct osal_proc_dir_entry), 0, sizeof(struct osal_proc_dir_entry));
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_WARN, "memset_s failed!\n");
    }

    ret = strncpy_s((*sproc)->name, sizeof((*sproc)->name), name, sizeof((*sproc)->name) - 1U);
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_WARN, "strncpy_s failed!\n");
    }

    if (parent != NULL) {
        proc = proc_mkdir_data(name, 0, parent->proc_dir_entry, *sproc);
    } else {
        proc = proc_mkdir_data(name, 0, NULL, *sproc);
    }

    if (proc == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "proc_mkdir failed!\n");
        osal_kfree(*sproc);
        *sproc = NULL;
        return;
    }

    (*sproc)->proc_dir_entry = proc;
}
#endif // #if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)

osal_proc_entry_t *osal_proc_mkdir(const hi_char *name, const osal_proc_entry_t *parent)
{
    struct osal_proc_dir_entry *sproc = NULL;
    osal_proc_mkdir_version_branch(name, parent, &sproc);
    if (sproc != NULL) {
        osal_list_add_tail(&(sproc->node), &osal_proc_list);
    }
    return sproc;
}
EXPORT_SYMBOL(osal_proc_mkdir);

static void osal_remove_proc_root(const hi_char *name, const osal_proc_entry_t *parent)
{
    struct osal_proc_dir_entry *sproc = NULL;
    if (name == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    if (parent != NULL) {
        remove_proc_entry(name, parent->proc_dir_entry);
    } else {
        remove_proc_entry(name, NULL);
    }
    osal_list_for_each_entry(sproc, &osal_proc_list, node) {
        if (osal_strncmp(sproc->name, name, sizeof(sproc->name)) == 0) {
            osal_list_del(&(sproc->node));
            break;
        }
    }
    if (sproc != NULL) {
        osal_kfree(sproc);
    }
}

void osal_seq_printf(const osal_proc_entry_t *entry, const hi_char *fmt, ...)
{
    struct seq_file *s = NULL;
    if (entry == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    s = (struct seq_file *)(entry->seqfile);
    va_list args;

    va_start((args), (fmt));
    seq_vprintf(s, fmt, args);
    va_end(args);
}
EXPORT_SYMBOL(osal_seq_printf);

void osal_proc_init(void)
{
    OSAL_INIT_LIST_HEAD(&osal_proc_list);
    proc_entry = osal_proc_mkdir("umap", NULL);
    if (proc_entry == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "test init, proc mkdir error!\n");
    }
}
void osal_proc_exit(void)
{
    osal_remove_proc_root("umap", NULL);
}