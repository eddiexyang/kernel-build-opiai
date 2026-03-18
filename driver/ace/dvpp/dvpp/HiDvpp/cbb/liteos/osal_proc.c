/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal proc source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */
#include "hi_osal.h"
#include "securec.h"

osal_proc_entry_t *osal_create_proc_entry(const hi_char *name, osal_proc_entry_t *parent)
{
    static osal_proc_entry_t ent = {};
    return &ent;
}

void osal_remove_proc_entry(const hi_char *name, const osal_proc_entry_t *parent)
{
}

osal_proc_entry_t *osal_proc_mkdir(const hi_char *name, const osal_proc_entry_t *parent)
{
    static osal_proc_entry_t sproc = {};
    return &sproc;
}

void osal_seq_printf(const osal_proc_entry_t *entry, const hi_char *fmt, ...)
{
    va_list args;
    osal_seq_file_t *s;
    int len;

    if (entry == NULL || entry->seqfile == NULL) {
        return;
    }

    s = (osal_seq_file_t *)entry->seqfile;
    if (s->base == NULL) {
        return;
    }

    if ((s->size + 1) >= s->capacity) {
        HI_TRACE_OSAL(HI_DBG_ERR, "buffer full, capacity=%llu size=%llu\n",
                      s->capacity, s->size);
        return;
    }

    va_start(args, fmt);
    len = vsnprintf_s(s->base + s->size, (s->capacity - s->size),
                      (s->capacity - s->size - 1), fmt, args);
    va_end(args);

    if (len < 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "vsnpintf_s fail\n");
        s->base[s->size] = 0;
        return;
    }

    s->size += (hi_u32)len;
}

void osal_proc_init(void)
{
}

void osal_proc_exit(void)
{
}
