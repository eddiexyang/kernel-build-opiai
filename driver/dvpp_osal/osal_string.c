#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/version.h>
#include "securec.h"
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif // #ifdef AOS_LLVM_BUILD

#ifndef AOS_LLVM_BUILD
hi_ulong osal_strlcpy(hi_char *dest, const hi_char *src, hi_ulong size)
{
    return strlcpy(dest, src, size);
}
EXPORT_SYMBOL(osal_strlcpy);

hi_ulong osal_strlcat(hi_char *dest, const hi_char *src, hi_ulong count)
{
    return strlcat(dest, src, count);
}
EXPORT_SYMBOL(osal_strlcat);
#endif // #ifndef AOS_LLVM_BUILD

hi_s32 osal_strcmp(const hi_char *cs, const hi_char *ct)
{
    return strcmp(cs, ct);
}
EXPORT_SYMBOL(osal_strcmp);
hi_char *osal_strsep(hi_char **stringp, const hi_char *delim)
{
    return strsep(stringp, delim);  // 调用系统接口
}
EXPORT_SYMBOL(osal_strsep);

hi_s32 osal_kstrtouint(const hi_char *s, hi_u32 base, hi_u32 *res)
{
    return kstrtouint(s, base, res); // 调用系统接口
}
EXPORT_SYMBOL(osal_kstrtouint);

hi_s32 osal_strncmp(const hi_char *cs, const hi_char *ct, hi_ulong count)
{
    return strncmp(cs, ct, count);
}
EXPORT_SYMBOL(osal_strncmp);


#ifndef AOS_LLVM_BUILD
hi_s32 osal_strcasecmp(const hi_char *s1, const hi_char *s2)
{
    return strcasecmp(s1, s2);
}
EXPORT_SYMBOL(osal_strcasecmp);
hi_s32 osal_strncasecmp(const hi_char *s1, const hi_char *s2, hi_ulong n)
{
    return strncasecmp(s1, s2, n);
}
EXPORT_SYMBOL(osal_strncasecmp);
#endif // #ifndef AOS_LLVM_BUILD
const hi_char *osal_strchr(const hi_char *s, hi_s32 c)
{
    return strchr(s, c);
}
#ifndef AOS_LLVM_BUILD
EXPORT_SYMBOL(osal_strchr);
hi_char *osal_strnchr(const hi_char *s, hi_ulong count, hi_s32 c)
{
    return strnchr(s, count, c);
}
EXPORT_SYMBOL(osal_strnchr);
const hi_char *osal_strrchr(const hi_char *s, hi_s32 c)
{
    return strrchr(s, c);
}
EXPORT_SYMBOL(osal_strrchr);
hi_char *osal_strnstr(const hi_char *s1, const hi_char *s2, hi_ulong len)
{
    return strnstr(s1, s2, len);
}
EXPORT_SYMBOL(osal_strnstr);
#endif // #ifndef AOS_LLVM_BUILD
hi_ulong osal_strlen(const hi_char *s)
{
    return strlen(s);
}
EXPORT_SYMBOL(osal_strlen);
hi_ulong osal_strnlen(const hi_char *s, hi_ulong count)
{
    return strnlen(s, count);
}
EXPORT_SYMBOL(osal_strnlen);
#ifndef AOS_LLVM_BUILD
const hi_char *osal_strpbrk(const hi_char *cs, const hi_char *ct)
{
    return strpbrk(cs, ct);
}
EXPORT_SYMBOL(osal_strpbrk);
#endif // #ifndef AOS_LLVM_BUILD
hi_ulong osal_strspn(const hi_char *s, const hi_char *accept)
{
    return strspn(s, accept);
}
EXPORT_SYMBOL(osal_strspn);
hi_ulong osal_strcspn(const hi_char *s, const hi_char *reject)
{
    return strcspn(s, reject);
}
EXPORT_SYMBOL(osal_strcspn);
#ifndef AOS_LLVM_BUILD
void *osal_memscan(void *addr, hi_s32 c, hi_ulong size)
{
    return memscan(addr, c, size);
}
EXPORT_SYMBOL(osal_memscan);
#endif // #ifndef AOS_LLVM_BUILD
hi_s32 osal_memcmp(const void *cs, const void *ct, hi_ulong count)
{
    return memcmp(cs, ct, count);
}
EXPORT_SYMBOL(osal_memcmp);
void *osal_memchr(const void *s, hi_s32 c, hi_ulong n)
{
    return memchr(s, c, n);
}
EXPORT_SYMBOL(osal_memchr);
#ifndef AOS_LLVM_BUILD
void *osal_memchr_inv(const void *start, hi_s32 c, hi_ulong bytes)
{
    return memchr_inv(start, c, bytes);
}
EXPORT_SYMBOL(osal_memchr_inv);
#endif // #ifndef AOS_LLVM_BUILD
hi_u64 osal_strtoull(const hi_char *cp, hi_char **endp, hi_u32 base)
{
#ifndef AOS_LLVM_BUILD
    return simple_strtoull(cp, endp, base);
#else
    hi_u64 res;
    (hi_void)kstrtoull(cp, base, &res);
    return res;
#endif
}
EXPORT_SYMBOL(osal_strtoull);
#ifndef AOS_LLVM_BUILD
hi_u32 osal_strtoul(const hi_char *cp, hi_char **endp, hi_u32 base)
{
    return simple_strtoul(cp, endp, base);
}
EXPORT_SYMBOL(osal_strtoul);
#endif // #ifndef AOS_LLVM_BUILD
hi_s32 osal_strtol(const hi_char *cp, hi_char **endp, hi_u32 base)
{
#ifndef AOS_LLVM_BUILD
    return simple_strtol(cp, endp, base);
#else
    hi_slong res;
    (hi_void)kstrtol(cp, base, &res);
    return res;
#endif
}
EXPORT_SYMBOL(osal_strtol);
#ifndef AOS_LLVM_BUILD
hi_s64 osal_strtoll(const hi_char *cp, hi_char **endp, hi_u32 base)
{
    return simple_strtoll(cp, endp, base);
}
EXPORT_SYMBOL(osal_strtoll);
#endif // #ifndef AOS_LLVM_BUILD
