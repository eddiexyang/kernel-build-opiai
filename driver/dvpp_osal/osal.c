#include <linux/module.h>
#include <linux/kernel.h>
#include "hi_osal.h"
#include "himedia.h"
#include "osal_timer.h"

extern void himedia_exit(void);

static hi_s32 __init osal_init(void)
{
    hi_s32 ret;
    hi_u64 begin;
    hi_u64 end;

    begin = osal_asm_get_clock();
    osal_timer_get_timestamp_type();
    osal_device_init();
    osal_proc_init();
    (void)himedia_init();
    ret = osal_bootdot_sym_init();
    if (ret != HI_SUCCESS) {
        HI_TRACE_OSAL(HI_DBG_WARN, "current version can not support bootdot.\n");
    }
    end = osal_asm_get_clock();
    HI_TRACE_OSAL(HI_DBG_INFO, "hi_osal init success!cost time %lluns\n", end - begin);
    return 0;
}

static void __exit osal_exit(void)
{
    osal_bootdot_sym_deinit();
    himedia_exit();
    osal_proc_exit();
    HI_TRACE_OSAL(HI_DBG_INFO, "hi_osal v1.0 exit!\n");
}

module_init(osal_init);
module_exit(osal_exit);

MODULE_AUTHOR("Hisilicon");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
