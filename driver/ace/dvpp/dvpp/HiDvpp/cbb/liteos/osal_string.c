/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal string source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"
#include "limits.h"
#include "securec.h"
#include "string.h"


hi_ulong osal_strlcpy(hi_char *dest, const hi_char *src, hi_ulong size)
{
    if (strncpy_s(dest, size, src, size - 1) != EOK) {
        return 0;
    }
    return strlen(src);
}

int osal_strcmp(const char *cs, const char *ct)
{
    return strcmp(cs, ct);
}

int osal_strncmp(const char *cs, const char *ct, hi_ulong count)
{
    return strncmp(cs, ct, count);
}

hi_ulong osal_strlen(const char *s)
{
    return strlen(s);
}

hi_ulong osal_strnlen(const char *s, hi_ulong count)
{
    return strnlen(s, count);
}

int osal_memcmp(const void *cs, const void *ct, hi_ulong count)
{
    return memcmp(cs, ct, count);
}
hi_s32 osal_strtol(const char *cp, char **endp, unsigned int base)
{
    return strtol(cp, endp, base);
}
