/* Stub for Huawei-custom linux/aos/cpu_domain_info.h
 * Not available in upstream 6.18 kernel */
#ifndef _LINUX_AOS_CPU_DOMAIN_INFO_H
#define _LINUX_AOS_CPU_DOMAIN_INFO_H

#include <linux/errno.h>

struct cpu_domain_info {
    unsigned int ctrlcpu_bitmap;
    unsigned int aicpu_bitmap;
    unsigned int domain_num;
};

static inline int get_cpudomain_info(struct cpu_domain_info *info)
{
    if (info) {
        info->ctrlcpu_bitmap = 0;
        info->aicpu_bitmap = 0;
        info->domain_num = 1;
    }
    return -ENODEV;
}
static inline int get_cpu_domain_id(int cpu) { return 0; }
static inline int get_cpu_domain_num(void) { return 1; }

#endif
