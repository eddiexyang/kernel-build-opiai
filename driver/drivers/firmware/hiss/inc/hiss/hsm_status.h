/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 * Description: hsm api function
 * Author: huawei
 * Create: 2020-10-12
 */
#ifndef HSM_STATUS_H
#define HSM_STATUS_H

#ifndef __KERNEL__
#include <stdint.h>
#else
#include <linux/types.h>
#endif

/* hsm heartbeat ioctl cmd define */
#define HSM_HEARTBEAT_MAGIC                 0xC0
#define HEARTBEAT_IOCTL_NR_READ             0x1
#define HEARTBEAT_IOCTL_STRUCT_SIZE         0x8

#define HEARTBEAT_IOCTL_CMD_READ \
    _IOC(_IOC_READ, HSM_HEARTBEAT_MAGIC, HEARTBEAT_IOCTL_NR_READ, HEARTBEAT_IOCTL_STRUCT_SIZE)

/**
 * PG_MODULE_TYPE
 * @description
 * PG 模块类型
 */
typedef enum {
    PG_MODULE_TYPE_CPU,
    PG_MODULE_TYPE_AIC,
    PG_MODULE_TYPE_AIV,
    PG_MODULE_TYPE_HBM,
    PG_MODULE_TYPE_DVPP,
    PG_MODULE_TYPE_GPU,
    PG_MODULE_TYPE_ALL, /* PG V2.0 */
    PG_MODULE_TYPE_MAX = PG_MODULE_TYPE_ALL,
    PG_MODULE_TYPE_INVALID,
} module_type;

/**
 * PG_DATA_TYPE
 * @description
 * PG 数据类型
 */
typedef enum {
    PG_DATA_TYPE_FREQ,
    PG_DATA_TYPE_TOTAL_NUM,
    PG_DATA_TYPE_CORE_MAP,
    PG_DATA_TYPE_MAX = PG_DATA_TYPE_CORE_MAP,
    PG_DATA_TYPE_INVALID,
} data_type;

typedef struct {
    module_type module;
    data_type data;
} pg_cmd_data;

/**
 * hsm_status_t
 * @description
 * HSM状态信息
 */
typedef enum {
    HSM_NOT_INITIALIZED,
    HSM_ERROR,
    HSM_OK
} HSM_STATUS;

typedef struct {
    HSM_STATUS hsm_status_code; /* 状态码 */
    unsigned long hsm_status_info;
} hsm_status_t;

typedef struct {
    uint32_t dev_id;
    uint32_t hsm_acc_status; /* 加速器自检状态 */
} heartbeat_info_t;

/*
 * @ingroup 硬件安全
 * @brief Obtains the running status of the HISS.
 *
 * @description
 * 获取hiss运行状态
 *
 * @param [in] dev_id       类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [out] hsm_status  类型 #hsm_status_t. hsm运行状态
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion
 *   1、失败场景，无需解析hsm_status值
 *   2、DC:DSMI don't call the interface in common scene
 *   3、MDC:SocDevMgr circle call the interface in less than 2 seconds
 *   4、该接口内置获取当前utc时间并传入相关时间数据的功能
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int hsm_get_hiss_status(uint32_t dev_id, hsm_status_t *hsm_status);

/*
 * @ingroup 硬件安全
 * @brief The system resets and instructs the HSM to disable services.
 *
 * @description
 * 系统复位，通知HSM关闭服务
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景才有效)
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion (only for dsmi)
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int hsm_notify_reset_action(uint32_t dev_id);

/*
 * @ingroup 硬件安全
 * @brief The DC equipment obtains the hardware accelerator self-test result from the HISS.
 *
 * @description
 * DC装备获取hiss中硬件加速器自测试结果
 *
 * @param [in] dev_id  类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景才有效)
 * @param [out] hsm_accelerator_status   类型 #uint32_t *. 自测试结果
 * @param [in] hsm_status_len            类型 #uint32_t. 入参长度
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion (only for equip)
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int hsm_accelerator_check(uint32_t dev_id, uint32_t *hsm_accelerator_status, uint32_t hsm_status_len);

/*
 * @ingroup 硬件安全
 * @brief Reads the core configuration information.
 *
 * @description
 * 读取核配比信息
 *
 * @param [in] dev_id     类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景才有效)
 * @param [out] cmd       类型 #pg_cmd_data. pg命令字信息
 * @param [out] out_arg   类型 #uint64_t *. 返回核配比信息
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_read_pg_info(uint32_t dev_id, pg_cmd_data cmd, uint64_t *out_arg);

/* @ingroup 硬件安全
 * @Reads PG information as blocks.
 *
 * @description
 * 兼容PGV1.0和PGV2.0, 读取PG信息
 *
 * @param [in] dev_id     类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景才有效)
 * @param [in] cmd        类型 #pg_cmd_data. pg命令字信息
 * @param [out] buf       类型 #uint8_t *. 返回PG信息
 * @param [in] buf_len    类型 #uint32_t. PG信息buf长度
 *
 * @retval #0,  成功
 * @retval #其它值, 失败
 * @suggestion
 * 1.PGV1.0：
 *   入参：cmd.module可取: 0~4; cmd.data可取: 0~2; buf_len固定8Byte
 * 2.PGV2.0：
 *   入参：cmd.module可取: 5; cmd.data未使用, 默认0; buf_len小于4K
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_read_pg_info_block(uint32_t dev_id, pg_cmd_data cmd, uint8_t *buf, uint32_t buf_len);

#endif
