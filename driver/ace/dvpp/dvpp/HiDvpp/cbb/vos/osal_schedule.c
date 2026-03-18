#include "hi_osal.h"
#include "Os.h"

void osal_yield(void)
{
    StatusType ret = E_NOT_OK;

    /*
     * 调用Schedule接口时触发调度情况：
     *   a、如果已有更高优先级task处于ready状态，则立即触发调度执行更高优先级task；
     *   b、如果有同等优先级或更低优先级task处于ready状态，则继续执行当前task。
     */
    ret = Schedule();
    if (ret != E_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, "Schedule fail, ret=%d\n", ret);
    }
}
