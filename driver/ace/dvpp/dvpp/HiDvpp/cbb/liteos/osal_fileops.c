/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal fileops source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"

void *osal_klib_fopen(const char *filename, int flags, unsigned short mode)
{
    return NULL;
}

void osal_klib_fclose(void *filp)
{
}

int osal_klib_fwrite(const char *buf, hi_ulong len, void *filp)
{
    return -1;
}

int osal_klib_fread(char *buf, unsigned int len, void *filp)
{
    return 0;
}

