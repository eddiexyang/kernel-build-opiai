/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/
#include <stdarg.h>
#include "hi_osal.h"
#include "mkp_ioctl.h"
#include "osal_ioctl.h"
#include "Os.h"

typedef struct trusted_func_param_ {
    unsigned int cmd;
    unsigned long arg;
    void *private_data;
    unsigned long err_code; // 用于回写错误码
} trusted_func_param;

/* 封装的系统函数 open 接口，暂存在osal_device */
int osal_open(char real_path, int flags)
{
    return 0;
}

/* 封装的系统函数 close 接口，暂存在osal_device */
int osal_close(int fieldes)
{
    return 0;
}

/* 封装的系统函数 realpath 接口，暂存在osal_device */
char *osal_realpath(const char *path, char *resolved_path)
{
    return path;
}

/* vos中不支持realpath 接口， 与 osal_realpath 配合使用 */
void osal_realpath_free(const char *path)
{
    return;
}

/* VOS中 可信OSA 的处理函数，在实际应用中，非可信OSA 通过 CallTrustedFunction 调用 */
void TRUSTED_TaskTrustedService(TrustedFunctionIndexType index, TrustedFunctionParameterRefType ref)
{
    trusted_func_param *param = (trusted_func_param*)ref;
    unsigned int cmd = param->cmd;
    unsigned int module_type = (cmd >> _IOC_TYPESHIFT) & 0xFF;

    // 解析参数，待下一轮迭代实现
    switch (module_type) {
        case IOC_TYPE_VENC:
            HI_TRACE_OSAL(HI_DBG_INFO, "call venc process!\n");
            break;
        case IOC_TYPE_VDEC:
            HI_TRACE_OSAL(HI_DBG_INFO, "call vdec process!\n");
            break;
        case IOC_TYPE_VPC:
            HI_TRACE_OSAL(HI_DBG_INFO, "call vpc process!\n");
            break;
        case IOC_TYPE_SYS:
            HI_TRACE_OSAL(HI_DBG_INFO, "call sys process!\n");
        default:
            HI_TRACE_OSAL(HI_DBG_ERR, "module %d err!\n", module_type);
            break;
    }
    return;
}

/* 封装的vos ioctl 接口，简易实现方式, 可变入参只会解析第一个变参，类型为unsigned long */
int osal_ioctl(int fd, int cmd, ...)
{
    va_list arg_ptr;
    int ret = 0;
    unsigned long arg = 0;
    trusted_func_param param = {0};

    if (_IOC_DIR(cmd) != _IOC_NONE) {
        va_start(arg_ptr, cmd);
        arg = va_arg(arg_ptr, unsigned long);
        va_end(arg_ptr);
    }

    param.cmd = cmd;
    param.arg = arg;
    ret =  CallTrustedFunction(TRUSTED_TaskTrustedService_INDEX, (TrustedFunctionParameterRefType)&param);
    if (ret != E_OK) {
        return param.err_code;
    }

    return 0;
}