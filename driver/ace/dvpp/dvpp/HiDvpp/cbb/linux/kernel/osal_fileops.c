#include "hi_osal.h"
#include <linux/fs.h>
#include <asm/uaccess.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif // #ifdef AOS_LLVM_BUILD
static struct file *klib_fopen(const hi_char *name, hi_s32 flags, hi_u16 mode)
{
    struct file *filp = filp_open(name, flags, mode);
    return (IS_ERR(filp)) ? NULL : filp;
}

static void klib_fclose(struct file *filp)
{
    if (filp != NULL) {
        (void)filp_close(filp, NULL);
    }
    return;
}

static hi_s32 klib_fwrite(const hi_char *buf, hi_ulong len, struct file *filp)
{
    if (filp == NULL) {
        return -ENOENT;
    }
#ifndef AOS_LLVM_BUILD
    return __kernel_write(filp, buf, len, &filp->f_pos);
#else
    HI_TRACE_OSAL(HI_DBG_ERR, "%s\n", buf);
    return 0;
#endif
}

static hi_s32 klib_fread(hi_char *buf, hi_u32 len, struct file *filp)
{
    hi_s32 readlen;
    if (filp == NULL) {
        return -ENOENT;
    }

    readlen = kernel_read(filp, buf, len, &filp->f_pos);
    return readlen;
}

void *osal_klib_fopen(const hi_char *name, hi_s32 flags, hi_u16 mode)
{
    return (void *)klib_fopen(name, flags, mode);
}
EXPORT_SYMBOL(osal_klib_fopen);

void osal_klib_fclose(void *filp)
{
    klib_fclose((struct file *)filp);
}
EXPORT_SYMBOL(osal_klib_fclose);

hi_s32 osal_klib_fwrite(const hi_char *buf, hi_ulong len, void *filp)
{
    return klib_fwrite(buf, len, (struct file *)filp);
}
EXPORT_SYMBOL(osal_klib_fwrite);

hi_s32 osal_klib_fread(hi_char *buf, hi_u32 len, void *filp)
{
    return klib_fread(buf, len, (struct file *)filp);
}
EXPORT_SYMBOL(osal_klib_fread);