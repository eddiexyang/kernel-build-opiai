// ******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co. Ltd.
// File name     :  hipciec_nvme_global_reg_reg_offset.h
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1.0
// Date          :  2017/10/24
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V4.2
// History       :  xxx 2018/07/13 11:22:07 Create file
// ******************************************************************************

#ifndef __HIPCIEC_NVME_GLOBAL_REG_REG_OFFSET_H__
#define __HIPCIEC_NVME_GLOBAL_REG_REG_OFFSET_H__

/* HIPCIEC_NVME_GLOBAL_REG Base address of Module's Register */
#define HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_BASE                       (0x0)

/******************************************************************************/
/*                      HiPCIECTRL40V200 HIPCIEC_NVME_GLOBAL_REG Registers' Definitions                            */
/******************************************************************************/

#define HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_CQDB_INT_MASK0_REG              (HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_BASE + 0x5000)  /* cqdb_int mask0 */
#define HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_CQDB_INT_STS0_REG               (HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_BASE + 0x5100)  /* cqdb_int sts0 */
#define HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_SQDB_INT_MASK0_REG              (HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_BASE + 0x5400)  /* sqdb_int mask0 */
#define HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_SQDB_INT_STS0_REG               (HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_BASE + 0x5500)  /* sqdb_int sts0 */
#define HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_INT_REQ_REG                     (HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_BASE + 0x8408)  /* time threshlod for int req */
#define HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_NVME_VECTOR_MASK_0_REG          (HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_BASE + 0x8540)  /* nvme interrupt vector  mask */

#endif // __HIPCIEC_NVME_GLOBAL_REG_REG_OFFSET_H__
