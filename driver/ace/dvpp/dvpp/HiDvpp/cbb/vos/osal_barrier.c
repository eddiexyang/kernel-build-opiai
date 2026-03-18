#include "hi_osal.h"

#ifndef DVPP_VCAST_UTST
#define vos_osal_isb(option) __asm__ __volatile__ ("isb " #option : : : "memory")
#define vos_osal_dsb(option) __asm__ __volatile__ ("dsb " #option : : : "memory")
#define vos_osal_dmb(option) __asm__ __volatile__ ("dmb " #option : : : "memory")

#ifndef barrier
#define barrier() __asm__ __volatile__("": : :"memory")
#endif
#else
#define vos_osal_isb(option) return
#define vos_osal_dsb(option) return
#define vos_osal_dmb(option) return
#define barrier() return
#endif
void osal_mb(void)
{
    barrier();
}

void osal_rmb(void)
{
    barrier();
}

void osal_wmb(void)
{
    barrier();
}

void osal_isb(void)
{
    vos_osal_isb();
}

void osal_dsb(void)
{
    vos_osal_dsb();
}

void osal_dmb(void)
{
    vos_osal_dmb();
}
