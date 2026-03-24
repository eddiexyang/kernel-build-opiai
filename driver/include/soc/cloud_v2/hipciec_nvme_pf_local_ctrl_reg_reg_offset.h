// ****************************************************************************** 
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co. Ltd.
// File name     :  hipciec_nvme_pf_local_ctrl_reg_reg_offset.h
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1.0
// Date          :  2017/10/24
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V4.2 
// History       :  xxx 2018/07/13 11:22:07 Create file
// ******************************************************************************

#ifndef __HIPCIEC_NVME_PF_LOCAL_CTRL_REG_REG_OFFSET_H__
#define __HIPCIEC_NVME_PF_LOCAL_CTRL_REG_REG_OFFSET_H__

/* HIPCIEC_NVME_PF_LOCAL_CTRL_REG Base address of Module's Register */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE                       (0x10000)


/******************************************************************************/
/*                      HiPCIECTRL40V200 HIPCIEC_NVME_PF_LOCAL_CTRL_REG Registers' Definitions                            */
/******************************************************************************/

#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_NVME_CTRL_CAP_LOW_REG  (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x0)    /* NVMe Controller Capabilities Low */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_NVME_CTRL_CAP_HIGH_REG (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x4)    /* NVMe Controller Capabilities High */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_NVME_CTRL_VS_REG       (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x8)    /* NVMe Controller Version */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_NVME_CTRL_INTMS_REG    (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0xC)    /* NVMe Controller Interrupt Mask */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_NVME_CTRL_INTMC_REG    (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x10)   /* NVMe Controller Interrupt Clear */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_NVME_CTRL_CONFIG_REG   (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x14)   /* NVMe Controller Configuration */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_NVME_CTRL_STATUS_REG   (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x1C)   /* NVMe Controller Status */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_NVME_SUBSYS_RST_REG    (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x20)   /* NVMe subsystem Reset */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_AQA_REG                (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x24)   /* NVMe Admin Queue Attributes */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_ASQB_LOW_REG           (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x28)   /* NVMe Admin Submission Queue Base Addrss low */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_ASQB_HIGH_REG          (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x2C)   /* NVMe Admin Submission Queue Base Addrss high */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_ACQB_LOW_REG           (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x30)   /* NVMe Admin Completion Queue Base Address low */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_ACQB_HIGH_REG          (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x34)   /* NVMe Admin Completion Queue Base Address high */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_CMBLOC_REG             (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x38)   /* NVMe Controller Memory Buffer Location */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_CMBSZ_REG              (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x3C)   /* NVMe Controller Memory Buffer Size */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_SQTDBL_0_REG           (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x1000) /* SQ Ring tail doorbell */
#define HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_CQHDBL_0_REG           (HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_BASE + 0x1004) /* CQ Ring head doorbell */

#endif // __HIPCIEC_NVME_PF_LOCAL_CTRL_REG_REG_OFFSET_H__
