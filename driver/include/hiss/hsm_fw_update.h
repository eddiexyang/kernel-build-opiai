/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 * Description: hsm api function
 * Author: huawei
 * Create: 2020-10-12
 */
#ifndef HSM_FW_UPDATE_H_
#define HSM_FW_UPDATE_H_

#ifndef __KERNEL__
#include <stdint.h>
#else
#include <linux/types.h>
#include <linux/cdev.h>
#endif

/**
 * img_verify_info_s
 * @description
 * 外部获取的flash信息具体内容
 */
typedef struct {
    uint64_t flash_id;
    uint16_t device_id;
    uint16_t vendor;
    uint32_t state;
    uint64_t size;
    uint32_t sector_count;
    uint16_t manufactuer_id;
} sec_flash_info_t;

/**
 * img_verify_info_s
 * @description
 * 待校验的镜像信息
 */
typedef struct {
    uint32_t img_id;
    uint8_t *image_buf;
    uint32_t img_len;
} img_verify_info_s;

/**
 * sec_ufsck_s
 * @description
 * ufs ck密钥参数信息
 */
typedef struct {
    uint32_t salt_flag; // 0：使用hardcoded密钥 1：使用给定密钥
    uint32_t it_cnt; // 派生次数
    uint32_t pbkdf2_alg; // 派生算法
} sec_ufsck_s;

/* 升级动作指导：
 * verify(可执行多次)--->update（与verify匹配）--->update_finish--->reset and sync(重启后同步)---> rim_update(若有需求)
 */
/*
 * @ingroup 硬件安全
 * @brief Verify the validity of the image to be upgraded.
 *
 * @description
 * 用于校验待升级镜像的合法性
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [in] img_num  类型 #uint32_t. 待校验镜像个数，取值范围为[1,9]
 * @param [in] img_info 类型 #img_verify_info_s *. 待校验镜像信息列表
 * @param [in] pss_cfg  类型 #uint32_t. 是否使用pss填充模式，1表示使用，0表示不使用
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion (for dsmi & equip)
 * Must be called before calling sec_img_update.
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_img_verify(uint32_t dev_id, uint32_t img_num, img_verify_info_s *img_info, uint32_t pss_cfg);

/*
 * @ingroup 硬件安全
 * @brief Upgrade the specified image.
 *
 * @description
 * 升级指定镜像
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [in] img_index  类型 #uint32_t. 要升级的镜像索引，取值范围为[0,16]
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion (for dsmi & equip)
 * 1、Must be called after calling sec_img_update.
 * 2、Must be called before calling sec_update_finish.
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_img_update(uint32_t dev_id, uint32_t img_index);

/*
 * @ingroup 硬件安全
 * @brief Finish the whole update process.
 *
 * @description
 * 升级后处理动作
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion (for dsmi & equip)
 * 1、升级后处理中会搬运hboot1_a及hilink镜像，若搬运失败，则升级流程失败；
 * 2、后处理动作中
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_update_finish(uint32_t dev_id);

/*
 * @ingroup 硬件安全
 * @brief Synchronize images to bak area and update efuse nvcnt value.
 *
 * @description
 * 完成镜像同步且更新efuse中的nvcnt值
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion (for dsmi & equip)
 * 1、
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_img_sync_and_efuse_update(uint32_t dev_id);

/*
 * @ingroup 硬件安全
 * @brief Update efuse rim value.
 *
 * @description
 * 更新efuse中的密钥吊销列表
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [in] rim      类型 #uint8_t *. 待吊销的密钥吊销列表信息
 * @param [in] rim_len  类型 #uint32_t. 待吊销的密钥吊销列表信息长度
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion (for dsmi & equip)
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_rim_update(uint32_t dev_id, uint8_t *rim, uint32_t rim_len);

/*
* function : clear cnt base action: clear flash_cnt->clear ufs_cnt->clear recovery_cnt
* usage scenario : equip and common(only for dsmi), the action is executed by system_service after all boot ok
* return : TEEC_SUCCESS or OTHERS
*/
/*
 * @ingroup 硬件安全
 * @brief Clear flash boot count
 *
 * @description
 * 清除Flash启动计数
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion (for dsmi & equip)
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int clear_flash_resetcnt(uint32_t dev_id);

/*
 * @ingroup 硬件安全
 * @brief Get ufs boot count
 *
 * @description
 * 读取ufs启动计数
 *
 * @param [in] dev_id       类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [out] out_value   类型 #uint32_t *. usf启动计数值
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_read_ufs_resetcnt(uint32_t dev_id, uint32_t *out_value);

/*
 * @ingroup 硬件安全
 * @brief Update ufs boot count
 *
 * @description
 * 更新ufs启动计数
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [in] value    类型 #uint32_t *. usf启动计数新值
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_write_ufs_resetcnt(uint32_t dev_id, uint32_t value);

/*
 * @ingroup 硬件安全
 * @brief Clear recovery boot count
 *
 * @description
 * 清零recovery区启动计数
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_reset_recovery_boot_count(uint32_t dev_id);

/*
 * @ingroup 硬件安全
 * @brief Read the firmware and driver boot area which is recorded in L3SRAM
 *
 * @description
 * 获取启动分区信息
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [out] out_value   类型 #uint32_t *. 获取到的启动分区信息
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion(for dsmi & equip)
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_flash_read_cmdline(uint32_t dev_id, uint32_t *out_value);

/*
 * @ingroup 硬件安全
 * @brief Get the firmware version of current area, based on cmdline
 *
 * @description
 * 获取指定区（主区/备区）的指定镜像的版本号
 *
 * @param [in] dev_id        类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [in] img_id        类型 #uint32_t. 要查询的镜像id
 * @param [out] version_str  类型 #uint8_t *. 获取到的镜像的版本号
 * @param [out] max_len      类型 #uint32_t. 版本号buffer长度
 * @param [in] area_check    类型 #uint32_t. 查询哪个区，0表示主区，1表示备区
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion(for dsmi & equip)
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_flash_get_version(uint32_t dev_id, uint32_t img_id, uint8_t *boot_version,
    uint32_t max_len, uint32_t area_check);

/*
 * @ingroup 硬件安全
 * @brief Get efuse nvcnt value
 *
 * @description
 * 获取efuse中的nvcnt
 *
 * @param [in] dev_id 类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [out] buff  类型 #uint32_t *. 存储结果的buffer
 * @param [in] size   类型 #uint32_t. 入参长度
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int soc_get_nvcnt(uint32_t dev_id, uint32_t *buf, uint32_t size);

/*
 * @ingroup 硬件安全
 * @brief Get the number of flash.
 *
 * @description
 * 获取flash个数
 *
 * @param [in] dev_id       类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [out] pflash_cnt  类型 #uint32_t *. 获取到的flash个数
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_flash_get_count(uint32_t dev_id, uint32_t *pflash_cnt);

/*
 * @ingroup 硬件安全
 * @brief Get flash details.
 *
 * @description
 * 获取flash信息
 *
 * @param [in] dev_id       类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [in] flash_idx    类型 #uint32_t. 要查询的flash索引
 * @param [out] flash_info  类型 #sec_flash_info_t *. 获取到的flash信息
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_flash_get_info(uint32_t dev_id, uint32_t flash_idx, sec_flash_info_t *flash_info);

/*
 * @ingroup 硬件安全
 * @brief ufs ck key derive.
 *
 * @description
 * 获取flash信息
 *
 * @param [in] dev_id       类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [in] key_num    类型 #uint32_t. 派生密钥数量
 * @param [in] pbkdf2_info  类型 #sec_ufsck_s *. 获取到的flash信息
 * @param [in] derive_info_size  类型 #uint32_t. pbkdf2_info长度
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_ufsck_key_cfg(uint32_t dev_id, uint32_t key_num, sec_ufsck_s *pbkdf2_info, uint32_t derive_info_size);

/*
 * @ingroup 硬件安全

 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [in] nsforbid_val  类型 #uint32_t *. 获取到nsforbid的信息
 * @retval 0,  非安全环境启动
 * @retval #其它值, 安全环境启动
 * @suggestion (for dsmi & equip)
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int32_t sec_read_nsforbid(uint32_t dev_id, uint32_t *nsforbid_val);

/*
 * @ingroup 硬件安全
 * @description
 * 获取主备区所有镜像SOC头校验结果
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [in] area_check    类型 #uint32_t. 查询哪个区，0表示主区，1表示备区

 * @suggestion (for dsmi & equip)
 * @param [out] verify_result   类型 #uint32_t *. 获取到主备区所有镜像SOC头校验结果
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion(for dsmi & equip)
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int32_t sec_get_fw_verify_result(uint32_t dev_id, uint32_t partition_type, uint32_t *result);

/*
 * @ingroup 硬件安全
 * @description
 * 创建密钥的参数信息
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [in] key_index    类型 #uint32_t. 对应0~7系统逻辑分区编号
 * @param [out] op_state  类型 #uint32_t *. 获取返回码
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion(for dsmi & equip)
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int32_t sec_part_ufsck_info_create(uint32_t dev_id, uint32_t key_index, uint32_t *op_state);

/*
 * @ingroup 硬件安全
 * @description
 * 删除密钥参数信息
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [in] key_index    类型 #uint32_t. 对应0~7系统逻辑分区编号
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion(for dsmi & equip)
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int32_t sec_part_ufsck_info_delete(uint32_t dev_id, uint32_t key_index);

/*
 * @ingroup 硬件安全
 * @description
 * 单个密钥派生并配置给UFS控制器
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 * @param [in] key_index   类型 #uint32_t. 对应0~7系统逻辑分区编号
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion(for dsmi & equip)
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int32_t sec_part_ufsck_validate(uint32_t dev_id, uint32_t key_index);

/*
 * @ingroup 硬件安全
 * @description
 * 所有密钥派生并配置给UFS控制器
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景有效)
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion(for dsmi & equip)
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int32_t sec_part_ufsck_validate_all(uint32_t dev_id, uint32_t flag, uint8_t *buf, uint32_t buf_len);

#endif
