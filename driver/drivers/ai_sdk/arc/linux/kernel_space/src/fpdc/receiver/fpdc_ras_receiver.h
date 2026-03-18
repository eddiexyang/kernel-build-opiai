/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* Description:
* Author: huawei
* Create: 2019-10-15
*/

#ifndef FPDC_RECEIVER_FPDC_RAS_RECEIVER_H
#define FPDC_RECEIVER_FPDC_RAS_RECEIVER_H


#define CPER_SEC_HISI_OEM_2 \
    GUID_INIT(0x45534EA6, 0xCE23, 0x4115, 0x85, 0x35, 0xE0, 0x7A, \
        0xB3, 0xAE, 0xF9, 0x1D)
#define CPER_SEC_HISI_PCIE_LOCAL \
    GUID_INIT(0xb2889fc9, 0xe7d7, 0x4f9d, 0xa8, 0x67, 0xaf, 0x42, \
        0xe9, 0x8b, 0xe7, 0x72)
#define CPER_SEC_HISI_COMMON \
    GUID_INIT(0xC8B328A8, 0x9917, 0x4AF6, 0x9A, 0x13, 0x2E, 0x08, \
        0xAB, 0x2E, 0x75, 0x86)

/* module id of section type oem2 */
#define RAS_SMMU_MODULE     0
#define RAS_HHA_MODULE      1
#define RAS_PA_MODULE       2
#define RAS_HLLC_MODULE     3
#define RAS_L2BUFF_MODULE   4
#define RAS_MEMORY_MODULE   5
#define RAS_INVALID_MODULE  0xff

/* module id of section type ts */
#define RAS_TS_MODULE       0

/* Indicates module where the ECC error occurs */
#ifndef ECC_MODULE_DDRC0
#  define ECC_MODULE_DDRC0                (0x0)
#  define ECC_MODULE_DDRC1                (0x1)
#  define ECC_MODULE_HBM_MIN              (0x2)
#  define ECC_MODULE_HBM_MAX              (0x41)
#endif

#define HISI_PCIE_LOCAL_VALID_SUB_MODULE_ID BIT(4)
#define HISI_PCIE_LOCAL_VALID_ERROR_TYPE    BIT(7)
#define CPER_SEC_PCIE_LOCAL_RESV_LEN        2
enum {
    PCIE_LOCAL_MODULE_AP = 0,
    PCIE_LOCAL_MODULE_TL = 1,
    PCIE_LOCAL_MODULE_MAC = 2,
    PCIE_LOCAL_MODULE_DL = 3,
    PCIE_LOCAL_MODULE_SDI = 4,
};

/* OEM RAS */
struct sec_oem_error {
    unsigned int  valid_fields;
    unsigned char version;
    unsigned char soc_id;
    unsigned char socket_id;
    unsigned char nimbus_id;
    unsigned char module_id;
    unsigned char submodule_id;
    unsigned char error_severity;
    unsigned char reserve;
    unsigned int  err_fr_l;
    unsigned int  err_fr_h;
    unsigned int  err_ctrl_l;
    unsigned int  err_ctrl_h;
    unsigned int  err_status_l;
    unsigned int  err_status_h;
    unsigned int  err_addr_l;
    unsigned int  err_addr_h;
    unsigned int  err_misc0_l;
    unsigned int  err_misc0_h;
    unsigned int  err_misc1_l;
    unsigned int  err_misc1_h;
};

struct cper_sec_pcie_local {
    u64 valid_fields;
    u8 version;
    u8 soc_id;
    u8 socket_id;
    u8 nimbus_id;
    u8 submodule_id;
    u8 core_id;
    u8 port_id;
    u8 err_severity;
    u16 err_type;
    u8 reserve[CPER_SEC_PCIE_LOCAL_RESV_LEN];
    u32 err_misc_0;
    u32 err_misc_1;
    u32 err_misc_2;
    u32 err_misc_3;
    u32 err_misc_4;
    u32 err_misc_5;
    u32 err_misc_6;
    u32 err_misc_7;
    u32 err_misc_8;
    u32 err_misc_9;
    u32 err_misc_10;
    u32 err_misc_11;
    u32 err_misc_12;
    u32 err_misc_13;
    u32 err_misc_14;
    u32 err_misc_15;
    u32 err_misc_16;
    u32 err_misc_17;
    u32 err_misc_18;
    u32 err_misc_19;
    u32 err_misc_20;
    u32 err_misc_21;
    u32 err_misc_22;
    u32 err_misc_23;
    u32 err_misc_24;
    u32 err_misc_25;
    u32 err_misc_26;
    u32 err_misc_27;
    u32 err_misc_28;
    u32 err_misc_29;
    u32 err_misc_30;
    u32 err_misc_31;
    u32 err_misc_32;
};

#define HISI_OEM_VALID_SOC_ID		BIT(0)
#define HISI_OEM_VALID_SOCKET_ID	BIT(1)
#define HISI_OEM_VALID_NIMBUS_ID	BIT(2)
#define HISI_OEM_VALID_MODULE_ID	BIT(3)
#define HISI_OEM_VALID_SUB_MODULE_ID	BIT(4)
#define HISI_OEM_VALID_ERR_SEVERITY	BIT(5)

#define CPER_ARM_MPIDR_SOCKETID_SHIFT    21
#define CPER_ARM_MPIDR_SOCKETID_MASK    GENMASK(1, 0)

extern int fpdc_ras_receiver_init(void);
extern void fpdc_ras_receiver_exit(void);

#endif
