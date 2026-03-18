#include "hi_osal.h"

// 小核上暂无模块使用相关接口，暂时空实现
void *osal_klib_fopen(const hi_char *name, hi_s32 flags, hi_u16 mode)
{
    return NULL;
}

void osal_klib_fclose(void *filp)
{
    return;
}

hi_s32 osal_klib_fwrite(const hi_char *buf, hi_ulong len, void *filp)
{
    return 0;
}

hi_s32 osal_klib_fread(hi_char *buf, hi_u32 len, void *filp)
{
    return 0;
}
