/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: hsm flash api function
 * Author: huawei
 * Create: 2022-02-23
 */
#ifndef HSM_NORFLASH_H
#define HSM_NORFLASH_H

#ifndef __KERNEL__
#include <stdint.h>
#else
#include <linux/types.h>
#include <linux/cdev.h>
#endif

#define MAX_MTD_NAME_SIZE   32
#define FLASH_MTD_NUM_MAX   32

/* hsm flash ioctl cmd define */
#define HSM_FLASH_MAGIC                 0xA0
#define FLASH_IOCTL_NR_READ             0x1
#define FLASH_IOCTL_NR_WRITE            0x2
#define FLASH_IOCTL_NR_RAW_WRITE        0x3
#define FLASH_IOCTL_NR_ERASE            0x4
#define FLASH_IOCTL_NR_INFO             0x5
#define FLASH_IOCTL_STRUCT_SIZE         0x20

#define FLASH_IOCTL_CMD_READ \
    _IOC(_IOC_READ, HSM_FLASH_MAGIC, FLASH_IOCTL_NR_READ, FLASH_IOCTL_STRUCT_SIZE)
#define FLASH_IOCTL_CMD_WRITE \
    _IOC(_IOC_WRITE, HSM_FLASH_MAGIC, FLASH_IOCTL_NR_WRITE, FLASH_IOCTL_STRUCT_SIZE)
#define FLASH_IOCTL_CMD_RAW_WRITE \
    _IOC(_IOC_WRITE, HSM_FLASH_MAGIC, FLASH_IOCTL_NR_RAW_WRITE, FLASH_IOCTL_STRUCT_SIZE)
#define FLASH_IOCTL_CMD_ERASE \
    _IOC(_IOC_WRITE, HSM_FLASH_MAGIC, FLASH_IOCTL_NR_ERASE, FLASH_IOCTL_STRUCT_SIZE)
#define FLASH_IOCTL_CMD_GET_MTD_INFO \
    _IOC(_IOC_READ, HSM_FLASH_MAGIC, FLASH_IOCTL_NR_INFO, FLASH_IOCTL_STRUCT_SIZE)

/* flash info structures */
typedef struct {
    uint32_t mtd_size;
    int8_t mtd_name[MAX_MTD_NAME_SIZE];
    uint32_t flash_addr;
} sec_flash_mtd_info_s;

typedef struct {
    uint32_t dev_id;
    uint32_t flash_addr;
    uint8_t *buf;
    uint32_t buf_len;
} sec_flash_s;

typedef struct {
    uint32_t read_count;
    sec_flash_mtd_info_s flash_info;
} sec_flash_info_s;

/* flash area planning which matches table in dbox  */
enum SEC_FLASH_PARTITION {
    flashboot                         = 0,
    hboot1a                           = 1,
    hlink                             = 2,
    hboot1a_bak                       = 3,
    hlink_bak                         = 4,
    hboot1b                           = 5,
    hboot1b_bak                       = 6,
    hboot2                            = 7,
    hboot2_bak                        = 8,
    ddr_img                           = 9,
    ddr_img_bak                       = 10,
    hsm_img                           = 11,
    hsm_img_bak                       = 12,
    lp_img                            = 13,
    lp_img_bak                        = 14,
    safety_img                        = 15,
    safety_img_bak                    = 16,
    syscfg_img                        = 17,
    syscfg_img_bak                    = 18,
    equip_tee                         = 19,
    user_config                       = 20,
    img_upgrade_flag                  = 21,
    img_upgrade_flag_bak              = 22,
    mac_addr                          = 23,
    domain_indicate                   = 24,
    equip_test_area                   = 25,
    bist                              = 26,
    bist_bak                          = 27,
    reserved_area1                    = 28,
    reserved_area2                    = 29,
    reserved_area3                    = 30,
    max_part_num                      = 31,
    hlink32                           = 32,
    hlink32_bak                       = 33,
    hlink60                           = 34,
    hlink60_bak                       = 35,
    tee_sec_storage                   = 36,
    reserved_area                     = 37,
    function_Info                     = 38,
    product_info                      = 39,
    atf                               = 40,
    atf_bak                           = 41,
    usercfg_img                       = 42,
    usercfg_img_bak                   = 43,
    product_use                       = 44,
    equip_use                         = 45,
};

enum dsmi_recovery_ops_type {
    RECOVERY_UPGRADE = 0,
    RECOVERY_SYNC = 1
};

enum dsmi_recovery_area_type {
    RECOVERY_PARTITION_MASTER = 0,
    RECOVERY_PARTITION_SLAVE = 1
};

typedef enum {
    ISP_DISPATCH_DAW,
    AO_SC_SDS0_POWER_CTRL,
    AO_SC_SDS1_POWER_CTRL,
    TS_DISPATCH_DAW,
    L3T_SYS_BANDWIDTH,
    SC_MBIST_CPUI_ENBALE,
} SEC_IO_REGS_ID_TYPE;

/*
 * @ingroup 硬件安全
 * @brief flash operation in flash addr : read/erase/write(integrated erase)/raw_write(without erase)
 *
 * @description
 * FLASH读&写（集成擦除）&擦除&空写（不集成擦除） 按flash具体地址操作
 *
 * @param [in] dev_id       类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景才有效)
 * @param [in] flash_offset 类型 #uint32_t. flash地址偏移 业务可用flash范围为[0x9C0000, 0xE40000], [0xE60000, 0x1000000]
 * @param [in/out] buf      类型 #uint8_t *. 写入/读出内容
 * @param [in] buf_len      类型 #uint32_t. 数据长度
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion 装备与业务场景共用（装备可使用全部flash区域）
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_flash_erase(uint32_t dev_id, uint32_t flash_offset, uint32_t buf_len);
int sec_flash_write(uint32_t dev_id, uint32_t flash_offset, const uint8_t *buf, uint32_t buf_len);
int sec_flash_read(uint32_t dev_id, uint32_t flash_offset, uint8_t *buf, uint32_t buf_len);
int sec_flash_raw_write(uint32_t dev_id, uint32_t flash_offset, const uint8_t *buf, uint32_t buf_len);

/*
 * @ingroup 硬件安全
 * @brief flash operation in area : read, write(integrated erase)
 *
 * @description
 * MDC专用 FLASH读&写（集成擦除） 按flash区域操作
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景才有效)
 * @param [in] area     类型 #uint32_t. flash区域，与SEC_FLASH_PARTITION对应，业务版本可用区域范围为[19, 20], [23, 30]
 * @param [in/out] buf  类型 #uint8_t *. 写入/读出内容
 * @param [in] buf_len  类型 #uint32_t. 数据长度
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion 装备与业务场景共用（仅限mdc使用）
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int mdc_flash_read(uint32_t dev_id, uint32_t area, uint8_t *buf, uint32_t buf_len);
int mdc_flash_write(uint32_t dev_id, uint32_t area, uint8_t *buf, uint32_t buf_len);

/*
 * @ingroup 硬件安全
 * @brief read the numbers of flash area
 *
 * @description
 * 读当前存在的flash区域数量，与SEC_FLASH_PARTITION匹配
 *
 * @param [out] nums    类型 #uint32_t *. 读出区域数量 值范围[0,31]
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion 装备与业务场景共用（仅限dsmi使用）
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_flash_read_mtd_nums(uint32_t *nums);

/*
 * @ingroup 硬件安全
 * @brief :read the flash addr, size, name of area,from 0~read_count
 *
 * @description
 * 根据区域值（readcount)读取0~当前区域对应的所有flash地址，区域大小和名称
 *
 * @param [in] read_count           类型 #uint32_t. 区域号
 * @param [out] sec_flash_mtd_info  类型 #sec_flash_mtd_info_s. 该区域对应的flash地址 大小及名称
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion 装备与业务场景共用（仅限mdc使用）
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_flash_read_mtd_info(uint32_t read_count, sec_flash_mtd_info_s *sec_flash_mtd_info);

/*
 * @ingroup 硬件安全
 * @brief :read the flash addr, size, name of area chosen(read count)
 *
 * @description
 * 根据区域值（readcount)读取当前区域对应的flash地址，区域大小和名称
 *
 * @param [in] read_count           类型 #uint32_t. 区域号
 * @param [out] sec_flash_mtd_info  类型 #sec_flash_mtd_info_s. 该区域对应的flash地址 大小及名称
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion mdc装备专用
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int32_t mdc_flash_read_area_info(sec_flash_info_s *sec_flash_info);
/*
 * @ingroup 硬件安全
 * @brief : the operation of recovery flag in norflash
 *
 * @description
 * flash中recovery标记的操作：设置&获取&清除recovery标记
 *
 * @param [in] dev_id   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景才有效)
 * @param [out] flag    类型 #int *. 获取到的标记值，flag值为0x464F5243强制进入recovery模式，否则看计数
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion 装备与业务场景共用（仅限mdc使用）
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_set_recovery_flag(uint32_t dev_id);
int sec_get_recovery_flag(uint32_t dev_id, int *flag);
int sec_clear_recovery_flag(uint32_t dev_id);

/*
 * @ingroup 硬件安全
 * @brief : the operation of upgrade flag in norflash
 *
 * @description
 * flash中升级标记的操作：升级标记是否置位&置位主区/备区升级标记
 *
 * @param [in] dev_id                   类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景才有效)
 * @param [in] ops                      类型 #dsmi_recovery_ops_type. 设置升级标记或不设置
 * @param [in] part                     类型 #dsmi_recovery_area_type. 设置升级标记位主区还是没去
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion 装备与业务场景共用（仅限mdc使用）
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int sec_set_recovery_status(uint32_t dev_id, enum dsmi_recovery_ops_type ops, enum dsmi_recovery_area_type part);

/*
 * @ingroup 硬件安全
 * @brief sec io operation : write/read
 *
 * @description
 * sec io 读/写 按寄存器具体地址操作
 *
 * @param [in] dev_id       类型 #uint32_t. 多P信息，取值范围为[0,1](1:DC多P场景才有效)
 * @param [in] type         类型 #SEC_IO_REGS_ID_TYPE. 寄存器类别
 * @param [in/out] val      类型 #uint32_t *. 写入/读出内容
 *
 * @retval #OK,  成功
 * @retval #其它值, 失败
 * @suggestion 装备与业务场景共用（仅限mdc使用）
 * @depend
 * @li 该接口声明所在的头文件
 * @since
 */
int32_t sec_io_write32(uint32_t dev_id, SEC_IO_REGS_ID_TYPE type, uint32_t val);
int32_t sec_io_read32(uint32_t dev_id, SEC_IO_REGS_ID_TYPE type, uint32_t *val);

#endif
