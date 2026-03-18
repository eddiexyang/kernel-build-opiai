#include "hi_osal.h"
#include "securec.h"
#include "qn_libcutil.h"

hi_ulong osal_strlcat(hi_char *dest, const hi_char *src, hi_ulong count)
{
    return strncat_s(dest, count, src, count);
}

hi_s32 osal_strcmp(const hi_char *cs, const hi_char *ct)
{
    return qn_strcmp(cs, ct);
}

hi_s32 osal_strncmp(const hi_char *cs, const hi_char *ct, hi_ulong count)
{
    return qn_strncmp(cs, ct, count);
}

hi_ulong osal_strlen(const hi_char *s)
{
    return qn_strlen(s);
}

hi_ulong osal_strnlen(const hi_char *s, hi_ulong count)
{
    return qn_strnlen(s, count);
}

hi_s32 osal_memcmp(const void *cs, const void *ct, hi_ulong count)
{
    const unsigned char *su1 = NULL;
    const unsigned char *su2 = NULL;
    int res = 0;

    for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--) {
        if ((res = *su1 - *su2) != 0) {
            break;
        }
    }
    return res;
}
