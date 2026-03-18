/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: custom efuse ree driver api header
 * Author: zengxiangshun
 * Create: 2020-04-25
 */
#ifndef CUS_EFUSE_API_H
#define CUS_EFUSE_API_H

#include <stdint.h>

#define TM_OK  0
#define TM_ERROR 0x7fffffff

#define TEE_EFUSE_FLASH_POWER_ON    (0x1)
#define TEE_EFUSE_FLASH_POWER_OFF   (0x0)

/**
* EFUSE写数据功能接口
 * @param [in] obj              类型 #uint8_t. 用于选择No Secure EFUSE(0) or Secure EFUSE (1)
 * @param [in] dest_bit         类型 #uint32_t. EFUSE中目标起始bit位,只支持按word写入，即这里的dest_bit目标起始bit位为word开始位
 * @param [in] dest_size        类型 #uint32_t. EFUSE中目标长度,目标长度默认为32，需要调用者拼装
 * @param [in] src              类型 #uint32_t *. 源数据存储地址
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 */
int tf_custom_efuse_write(uint8_t obj, uint32_t dest_bit, uint32_t dest_size, uint32_t *src);

/**
 * @description
 * EFUSE烧录功能接口
 * @param [in] obj       类型 #uint8_t. 用于选择No Secure EFUSE (0) or Secure EFUSE (1)
 * @retval #OK,  成功
 * @retval #其它值, 失败
 */
int tf_custom_burn_efuse(uint8_t obj);

 /**
 * EFUSE烧入数据是否OK的功能接口
 * @param [in] obj                类型 #uint8_t. 用于选择No Secure EFUSE (0) or Secure EFUSE(1)
 * @param [in] dest_bit           类型 #uint32_t. EFUSE中目标起始bit位
 * @param [in] dest_size          类型 #uint32_t. EFUSE中目标长度
 * @param [in] input              类型 #uint32_t *. 输入出来的数据保存地址
 * @param [out]out_flag           类型 #uint32_t *.检测数据是否符合预期，符合预期返回0x1，否则返回0x0
 * @retval #OK,  成功
 * @retval #其它值, 失败
 */
int tf_custom_efuse_data_check(uint8_t obj, uint32_t dest_bit, uint32_t dest_size,
    uint32_t *input, uint32_t *out_flag);

/**
* EFUSE烧入数据打开或关闭[正式商用此电源管脚处于关闭]
* @param [in] onoff      类型 #uint8_t. 用于打开电源(1)或者关闭电源(0)
* @retval #OK,  电源打开成功(0)
* @retval #其它值, 电源打开失败(非0值)
*/
int tf_custom_efuse_power(uint8_t onoff);

#endif
