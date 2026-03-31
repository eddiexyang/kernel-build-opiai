/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: hsm efuse api function
 * Author: huawei
 * Create: 2022-02-23
 */
#ifndef HSM_EFUSE_H
#define HSM_EFUSE_H

#ifndef __KERNEL__
#include <stdint.h>
#else
#include <linux/types.h>
#endif

/* hsm efuse ioctl cmd define */
#define HSM_EFUSE_MAGIC                 0xB0
#define EFUSE_IOCTL_NR_OPERATION        0x1
#define EFUSE_IOCTL_NR_BURN             0x2
#define EFUSE_IOCTL_NR_RAW_RELOAD       0x3
#define EFUSE_IOCTL_NR_SWITCH           0x4
#define EFUSE_IOCTL_NR_KEY              0x5
#define EFUSE_IOCTL_STRUCT_SIZE         0x20

#define EFUSE_IOCTL_CMD_OPERATION \
    _IOC(_IOC_READ, HSM_EFUSE_MAGIC, EFUSE_IOCTL_NR_OPERATION, EFUSE_IOCTL_STRUCT_SIZE)
#define EFUSE_IOCTL_CMD_BURN \
    _IOC(_IOC_WRITE, HSM_EFUSE_MAGIC, EFUSE_IOCTL_NR_BURN, EFUSE_IOCTL_STRUCT_SIZE)
#define EFUSE_IOCTL_CMD_RELOAD \
    _IOC(_IOC_WRITE, HSM_EFUSE_MAGIC, EFUSE_IOCTL_NR_RAW_RELOAD, EFUSE_IOCTL_STRUCT_SIZE)
#define EFUSE_IOCTL_CMD_LC_SWITCH \
    _IOC(_IOC_WRITE, HSM_EFUSE_MAGIC, EFUSE_IOCTL_NR_SWITCH, EFUSE_IOCTL_STRUCT_SIZE)
#define EFUSE_IOCTL_CMD_CHECK_KEY \
    _IOC(_IOC_READ, HSM_EFUSE_MAGIC, EFUSE_IOCTL_NR_KEY, EFUSE_IOCTL_STRUCT_SIZE)

typedef enum {
    EFUSE_CHECK_OP,
    EFUSE_WRITE_OP,
    EFUSE_READ_OP,
} efuse_op_flag;

typedef enum {
    LC_CM = 0,
    LC_DM = 1,
    LC_SD1 = 2,
    LC_SD2 = 3,
} LIFECYCLE_INFO;

/*
 * eFuse Structures
 */
typedef struct {
    uint32_t efuse_block_num;
    uint32_t start_bit;
    uint32_t dest_size;
    uint8_t *input;
    uint32_t len;
    uint32_t write_check_flag;
    uint32_t dev_id;
    LIFECYCLE_INFO lifecycle;
} EFUSE_INFO;

/*
 * @ingroup 硬件安全
 * @brief write/check/read efuse (can not write and check in one time)
 *
 * @description
 * efuse读&写&校验接口
 *
 * @param [in] efuse_info  类型 #EFUSE_INFO. EUFSE烧录信息
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion 装备使用，烧录字段需要与硬件安全组件对齐
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int kernel_operate_efuse(EFUSE_INFO efuse_info);

/*
 * @ingroup 硬件安全
 * @brief burn efuse(must use after write efuse)
 *
 * @description
 * efuse烧录接口
 *
 * @param [in] efuse_block_num  类型 #uint32_t. eFuse块信息 取值范围为[0,1]
 * @param [in] dev_id  类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景才有效)
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion 装备使用，烧录字段需要与硬件安全组件对齐
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int kernel_burn_efuse(uint32_t efuse_block_num, uint32_t dev_id);

#endif
