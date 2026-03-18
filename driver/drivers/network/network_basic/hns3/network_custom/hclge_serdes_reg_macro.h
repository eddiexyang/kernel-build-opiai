/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Create: 2023-2-7
 */
#ifndef HCLGE_SERDES_REG_MACRO_H
#define HCLGE_SERDES_REG_MACRO_H

#include <linux/types.h>
#include "network_custom.h"

#define H60_BASE_ADDR           0x704000000
#define CHIP_OFFSET             0x80000000000
#define DIE_OFFSET              0x10000000000
#define H60_MACRO_OFFSET        0x40000
#define H60_SINGLE_MACRO_SIZE   0x40000
#define H60_REG_M0_M1_SIZE      0x80000
#define DSAPI_OFFSET            0x1c
#define DS_OFFSET               0x400
#define CS_BASE                 0
#define SS_BASE                 0x0080
#define CP_BASE                 0x0100
#define DSCOM_ANA_BASE          0x4000
#define DSCOM_DIG_BASE          0x4040
#define DSRX_DIG_BASE           0x4280
#define DSRX_ANA_BASE           0x4200
#define DSTX_ANA_BASE           0x4080
#define DSTX_DIG_BASE           0x4100

#define HCCS_BASE_ADDR          0x200000000000
#define HCCS_CHIP_OFFSET        0x20000000000

#define H60_HILINK0_BASE_IOREMAP_ADDR(chip_id, die_id, connect_type) \
    (connect_type == HCCS_CONNECT_TYPE ? \
     (((chip_id) * HCCS_CHIP_OFFSET) + HCCS_BASE_ADDR + H60_BASE_ADDR + \
      ((die_id) * DIE_OFFSET)) : \
     (((chip_id) * CHIP_OFFSET) + H60_BASE_ADDR + ((die_id) * DIE_OFFSET)))

#define H60_HILINK1_BASE_IOREMAP_ADDR(chip_id, die_id, connect_type) \
    (connect_type == HCCS_CONNECT_TYPE ? \
     (((chip_id) * HCCS_CHIP_OFFSET) + HCCS_BASE_ADDR + H60_BASE_ADDR + \
      ((die_id) * DIE_OFFSET) + H60_SINGLE_MACRO_SIZE) : \
     (((chip_id) * CHIP_OFFSET) + H60_BASE_ADDR + ((die_id) * DIE_OFFSET) + H60_SINGLE_MACRO_SIZE))

#define DSAPI_REG_OFFSET(chip_info) \
    ((chip_info->connect_type) == HCCS_CONNECT_TYPE ? \
     (((chip_info->chip_id) * HCCS_CHIP_OFFSET) + ((chip_info->macro_id) * H60_MACRO_OFFSET) + \
       ((chip_info->csds_id) * DSAPI_OFFSET) + ((chip_info->die_id) * DIE_OFFSET)) : \
     (((chip_info->chip_id) * CHIP_OFFSET) + ((chip_info->macro_id) * H60_MACRO_OFFSET) + \
       ((chip_info->csds_id) * DSAPI_OFFSET) + ((chip_info->die_id) * DIE_OFFSET)))

#define DS_REG_OFFSET(chip_info) \
    ((chip_info->connect_type) == HCCS_CONNECT_TYPE ? \
     (((chip_info->chip_id) * HCCS_CHIP_OFFSET) + ((chip_info->macro_id) * H60_MACRO_OFFSET) + \
      ((chip_info->csds_id) * DS_OFFSET) + ((chip_info->die_id) * DIE_OFFSET)) : \
     (((chip_info->chip_id) * CHIP_OFFSET) + ((chip_info->macro_id) * H60_MACRO_OFFSET) + \
      ((chip_info->csds_id) * DS_OFFSET) + ((chip_info->die_id) * DIE_OFFSET)))

#define CS_REG_OFFSET(chip_info) \
    ((chip_info->connect_type) == HCCS_CONNECT_TYPE ? \
     (((chip_info->chip_id) * HCCS_CHIP_OFFSET) + ((chip_info->macro_id) * H60_MACRO_OFFSET) + \
      ((chip_info->die_id) * DIE_OFFSET)) : \
     (((chip_info->chip_id) * CHIP_OFFSET) + ((chip_info->macro_id) * H60_MACRO_OFFSET) + \
      ((chip_info->die_id) * DIE_OFFSET)))

#define CS_CSR1r            (CS_BASE + 0x2)         /* reg  default(0x00002123) */
#define CS_CSR2r            (CS_BASE + 0x4)         /* reg  default(0x00004000) */
#define CS_CSR3r            (CS_BASE + 0x6)         /* reg  default(0x00002102) */
#define CS_CSR4r            (CS_BASE + 0x8)         /* reg  default(0x00004000) */
#define CS_CSR5r            (CS_BASE + 0xA)         /* reg  default(0x00000005) */
#define CS_CSR6r            (CS_BASE + 0xC)         /* reg  default(0x00000000) */
#define CS_CSR10r           (CS_BASE + 0x14)        /* reg  default(0x0000007c) */
#define CS_CSR11r           (CS_BASE + 0x16)        /* reg  default(0x0000a400) */
#define CS_CSR14r           (CS_BASE + 0x1C)        /* reg  default(0x00000403) */
#define CS_CSR15r           (CS_BASE + 0x1E)        /* reg  default(0x00000040) */
#define CS_CSR19r           (CS_BASE + 0x26)        /* reg  default(0x000008f0) */
#define CS_CSR20r           (CS_BASE + 0x28)        /* reg  default(0x0000a400) */
#define CS_CSR23r           (CS_BASE + 0x2E)        /* reg  default(0x00000003) */
#define CS_CSR24r           (CS_BASE + 0x30)        /* reg  default(0x00000040) */
#define CS_CSR35r           (CS_BASE + 0x46)        /* reg  default(0x00000000) */
#define SS_CSR11r           (SS_BASE + 0x16)        /* reg  default(0x00000991) */
#define SS_CSR32r           (SS_BASE + 0x40)        /* reg  default(0x00000000) */
#define SS_CSR34r           (SS_BASE + 0x44)        /* reg  default(0x00000003) */
#define SS_CSR37r           (SS_BASE + 0x4A)        /* reg  default(0x00000003) */
#define SS_CSR39r           (SS_BASE + 0x4E)        /* reg  default(0x0000006f) */
#define SS_CSR52r           (SS_BASE + 0x68)        /* reg  default(0x00000000) */
#define SS_CSR53r           (SS_BASE + 0x6A)        /* reg  default(0x00001184) */
#define SS_CSR54r           (SS_BASE + 0x6C)        /* reg  default(0x00000400) */
#define SS_CSR55r           (SS_BASE + 0x6E)        /* reg  default(0x00000000) */
#define SS_CSR56r           (SS_BASE + 0x70)        /* reg  default(0x00004000) */
#define SS_CSR57r           (SS_BASE + 0x72)        /* reg  default(0x00000000) */
#define SS_CSR58r           (SS_BASE + 0x74)        /* reg  default(0x00001184) */
#define SS_CSR59r           (SS_BASE + 0x76)        /* reg  default(0x00000400) */
#define SS_CSR60r           (SS_BASE + 0x78)        /* reg  default(0x00000000) */
#define SS_CSR61r           (SS_BASE + 0x7A)        /* reg  default(0x00004000) */
#define CP_CSR12r           (CP_BASE + 0x18)        /* reg  default(0x00000006) */

#define DSCOM_DIG_CSR2r     (DSCOM_DIG_BASE + 0x4)   /* reg  default(0x00000000) */
#define DSCOM_ANA_CSR15r    (DSCOM_ANA_BASE + 0x1E)  /* reg  default(0x00000000) */

#define DSRX_DIG_CSR0r      (DSRX_DIG_BASE + 0x0)   /* reg  default(0x00000304) */
#define DSRX_DIG_CSR1r      (DSRX_DIG_BASE + 0x2)   /* reg  default(0x00000304) */
#define DSRX_DIG_CSR2r      (DSRX_DIG_BASE + 0x4)   /* reg  default(0x00000304) */
#define DSRX_DIG_CSR4r      (DSRX_DIG_BASE + 0x8)   /* reg  default(0x00000304) */
#define DSRX_DIG_CSR5r      (DSRX_DIG_BASE + 0xA)   /* reg  default(0x00000304) */
#define DSRX_DIG_CSR6r      (DSRX_DIG_BASE + 0xC)   /* reg  default(0x00000304) */
#define DSRX_DIG_CSR7r      (DSRX_DIG_BASE + 0xE)   /* reg  default(0x00000304) */
#define DSRX_DIG_CSR8r      (DSRX_DIG_BASE + 0x10)  /* reg  default(0x00000304) */
#define DSRX_DIG_CSR9r      (DSRX_DIG_BASE + 0x12)  /* reg  default(0x00000000) */
#define DSRX_DIG_CSR10r     (DSRX_DIG_BASE + 0x14)  /* reg  default(0x00000000) */
#define DSRX_DIG_CSR13r     (DSRX_DIG_BASE + 0x1A)  /* reg  default(0x00000000) */
#define DSRX_DIG_CSR22r     (DSRX_DIG_BASE + 0x2C)  /* reg  default(0x00000000) */
#define DSRX_DIG_CSR25r     (DSRX_DIG_BASE + 0x32)  /* reg  default(0x00002104) */
#define DSRX_DIG_CSR26r     (DSRX_DIG_BASE + 0x34)  /* reg  default(0x00000064) */
#define DSRX_DIG_CSR39r     (DSRX_DIG_BASE + 0x4E)  /* reg  default(0x00000000) */
#define DSRX_DIG_CSR40r     (DSRX_DIG_BASE + 0x50)  /* reg  default(0x00000000) */
#define DSRX_DIG_CSR41r     (DSRX_DIG_BASE + 0x52)  /* reg  default(0x00000000) */
#define DSRX_DIG_CSR42r     (DSRX_DIG_BASE + 0x54)  /* reg  default(0x000003e8) */
#define DSRX_DIG_CSR113r    (DSRX_DIG_BASE + 0xE2)  /* reg  default(0x00000000) */
#define DSRX_DIG_CSR114r    (DSRX_DIG_BASE + 0xE4)  /* reg  default(0x00000000) */
#define DSRX_DIG_CSR115r    (DSRX_DIG_BASE + 0xE6)  /* reg  default(0x000003e8) */
#define DSRX_DIG_CSR119r    (DSRX_DIG_BASE + 0xEE)  /* reg  default(0x000003e8) */
#define DSRX_DIG_CSR120r    (DSRX_DIG_BASE + 0xF0)  /* reg  default(0x000003e8) */
#define DSRX_DIG_CSR121r    (DSRX_DIG_BASE + 0xF2)  /* reg  default(0x000003e8) */

#define DSRX_ANA_CSR4r      (DSRX_ANA_BASE + 0x8)   /* reg  default(0x00002200) */
#define DSRX_ANA_CSR5r      (DSRX_ANA_BASE + 0xA)   /* reg  default(0x00002200) */
#define DSRX_ANA_CSR6r      (DSRX_ANA_BASE + 0xC)   /* reg  default(0x00002200) */
#define DSRX_ANA_CSR7r      (DSRX_ANA_BASE + 0xE)   /* reg  default(0x00002200) */
#define DSRX_ANA_CSR8r      (DSRX_ANA_BASE + 0x10)  /* reg  default(0x00002200) */
#define DSRX_ANA_CSR9r      (DSRX_ANA_BASE + 0x12)  /* reg  default(0x00002200) */
#define DSRX_ANA_CSR10r     (DSRX_ANA_BASE + 0x14)  /* reg  default(0x00002200) */
#define DSRX_ANA_CSR11r     (DSRX_ANA_BASE + 0x16)  /* reg  default(0x00002200) */
#define DSRX_ANA_CSR12r     (DSRX_ANA_BASE + 0x18)  /* reg  default(0x00002200) */
#define DSRX_ANA_CSR13r     (DSRX_ANA_BASE + 0x1A)  /* reg  default(0x00002200) */
#define DSRX_ANA_CSR17r     (DSRX_ANA_BASE + 0x22)  /* reg  default(0x00002200) */
#define DSRX_ANA_CSR24r     (DSRX_ANA_BASE + 0x30)  /* reg  default(0x00002200) */
#define DSRX_ANA_CSR27r     (DSRX_ANA_BASE + 0x36)  /* reg  default(0x00000102) */
#define DSRX_ANA_CSR28r     (DSRX_ANA_BASE + 0x38)  /* reg  default(0x00000102) */
#define DSRX_ANA_CSR31r     (DSRX_ANA_BASE + 0x3E)  /* reg  default(0x00000102) */
#define DSRX_ANA_CSR39r     (DSRX_ANA_BASE + 0x4E)  /* reg  default(0x00000102) */
#define DSRX_ANA_CSR120r    (DSRX_ANA_BASE + 0xF0)  /* reg  default(0x00000102) */
#define DSRX_ANA_CSR121r    (DSRX_ANA_BASE + 0xF2)  /* reg  default(0x00000102) */

#define DSTX_ANA_CSR1r      (DSTX_ANA_BASE + 0x2)   /* reg  default(0x00000666) */
#define DSTX_ANA_CSR2r      (DSTX_ANA_BASE + 0x4)   /* reg  default(0x00000040) */
#define DSTX_ANA_CSR3r      (DSTX_ANA_BASE + 0x6)
#define DSTX_ANA_CSR4r      (DSTX_ANA_BASE + 0x8)   /* reg  default(0x00000040) */
#define DSTX_ANA_CSR7r      (DSTX_ANA_BASE + 0xE)
#define DSTX_ANA_CSR8r      (DSTX_ANA_BASE + 0x10)
#define DSTX_ANA_CSR14r     (DSTX_ANA_BASE + 0x1C)  /* reg  default(0x00000040) */
#define DSTX_ANA_CSR15r     (DSTX_ANA_BASE + 0x1E)  /* reg  default(0x00000040) */
#define DSTX_ANA_CSR6r      (DSTX_ANA_BASE + 0xC)   /* reg  default(0x00000000) */
#define DSTX_ANA_CSR9r      (DSTX_ANA_BASE + 0x12)  /* reg  default(0x00000000) */
#define DSTX_ANA_CSR10r     (DSTX_ANA_BASE + 0x14)  /* reg  default(0x00000000) */
#define DSTX_ANA_CSR11r     (DSTX_ANA_BASE + 0x16)  /* reg  default(0x00000000) */
#define DSTX_ANA_CSR17r     (DSTX_ANA_BASE + 0x22)  /* reg  default(0x00000000) */
#define DSTX_DIG_CSR1r      (DSTX_DIG_BASE + 0x2)   /* reg  default(0x00000800) */
#define DSTX_DIG_CSR2r      (DSTX_DIG_BASE + 0x4)   /* reg  default(0x000000a1) */
#define DSTX_DIG_CSR4r      (DSTX_DIG_BASE + 0x8)   /* reg  default(0x00000800) */
#define DSTX_DIG_CSR11r     (DSTX_DIG_BASE + 0x16)  /* reg  default(0x00000800) */
#define DSTX_DIG_CSR12r     (DSTX_DIG_BASE + 0x18)  /* reg  default(0x00000800) */
#define DSAPI_CSR0r         0x3ff06
#define DSAPI_CSR1r         0x3ff08
#define DSAPI_CSR2r         0x3ff0a
#define DSAPI_CSR3r         0x3ff0c
#define DSAPI_CSR4r         0x3ff0e
#define DSAPI_CSR5r         0x3ff10
#define DSAPI_CSR6r         0x3ff12
#define DSAPI_CSR7r         0x3ff14
#define DSAPI_CSR8r         0x3ff16
#define DSAPI_CSR9r         0x3ff18
#define DSAPI_CSR10r        0x3ff1a
#define DSAPI_CSR11r        0x3ff1c
#define DSAPI_CSR12r        0x3ff1e
#define DSAPI_CSR13r        0x3ff20
#define CSAPI_CSR0r         0x3ffe6
#define CSAPI_CSR1r         0x3ffe8
#define CSAPI_CSR2r         0x3ffea
#define CSAPI_CSR3r         0x3ffec
#define CSAPI_CSR4r         0x3ffee
#define CSAPI_CSR5r         0x3fff0
#define CSAPI_CSR6r         0x3fff2
#define CSAPI_CSR7r         0x3fff4
#define CSAPI_CSR8r         0x3fff6
#define CSAPI_CSR9r         0x3fff8
#define CSAPI_CSR10r        0x3fffa
#define CSAPI_CSR11r        0x3fffc

/* Register and Filed used to read or write different registers in different scenarios */
typedef struct tagSERDES_REG_FIELD {
    u32 reg_id;
    u32 field_id;
} SERDES_REG_FIELD_S;

/* cs reg */
#define H60_PLL0REFCLKSELf                     (CS_CSR1r),          (0x00008000) /* field,bit[15:15],RW,default=0x0 */
#define H60_PLL0VCODIVNAf                      (CS_CSR1r),          (0x00006000) /* field,bit[14:13],RW,default=0x1 */
#define H60_PLL0VCODIVKAf                      (CS_CSR1r),          (0x00001800) /* field,bit[12:11],RW,default=0x0 */
#define H60_PLL0FBDIVPf                        (CS_CSR1r),          (0x000007e0) /* field,bit[10:5],RW,default=0x09 */
#define H60_PLL0FBDIVSf                        (CS_CSR1r),          (0x0000001f) /* field,bit[4:0],RW,default=0x3 */
#define H60_PLL0CORECLKDIVRSTNf                (CS_CSR2r),          (0x00008000) /* field,bit[15:15],RW,default=0x0 */
#define H60_PLL0CORECLKDIVDOUBLEf              (CS_CSR2r),          (0x00004000) /* field,bit[14:14],RW,default=0x1 */
#define H60_PLL0MCLKDIVf                       (CS_CSR2r),          (0x00000c00) /* field,bit[11:10],RW,default=0x0 */
#define H60_PLL0MCLK2DIGDIV1PWRDNBf            (CS_CSR2r),          (0x00000200) /* field,bit[9:9],RW,default=0x0 */
#define H60_PLL0HSCLKDISTSPAREf                (CS_CSR2r),          (0x00000180) /* field,bit[8:7],RW,default=0x0 */
#define H60_PLL0REFCLKDIVf                     (CS_CSR2r),          (0x00000060) /* field,bit[6:5],RW,default=0x0 */
#define H60_PLL0MCLKDIV40Bf                    (CS_CSR2r),          (0x0000000c) /* field,bit[3:2],RW,default=0x0 */
#define H60_PLL0MCLKDIV32Bf                    (CS_CSR2r),          (0x00000003) /* field,bit[1:0],RW,default=0x0 */
#define H60_PLL1REFCLKSELf                     (CS_CSR3r),          (0x00008000) /* field,bit[15:15],RW,default=0x0 */
#define H60_PLL1VCODIVNAf                      (CS_CSR3r),          (0x00006000) /* field,bit[14:13],RW,default=0x1 */
#define H60_PLL1VCODIVKAf                      (CS_CSR3r),          (0x00001800) /* field,bit[12:11],RW,default=0x0 */
#define H60_PLL1FBDIVPf                        (CS_CSR3r),          (0x000007e0) /* field,bit[10:5],RW,default=0x08 */
#define H60_PLL1FBDIVSf                        (CS_CSR3r),          (0x0000001f) /* field,bit[4:0],RW,default=0x2 */
#define H60_PLL1CORECLKDIVRSTNf                (CS_CSR4r),          (0x00008000) /* field,bit[15:15],RW,default=0x0 */
#define H60_PLL1CORECLKDIVDOUBLEf              (CS_CSR4r),          (0x00004000) /* field,bit[14:14],RW,default=0x1 */
#define H60_PLL1MCLKDIVf                       (CS_CSR4r),          (0x00000c00) /* field,bit[11:10],RW,default=0x0 */
#define H60_PLL1MCLK2DIGDIV1PWRDNBf            (CS_CSR4r),          (0x00000200) /* field,bit[9:9],RW,default=0x0 */
#define H60_PLL1HSCLKDISTSPAREf                (CS_CSR4r),          (0x00000180) /* field,bit[8:7],RW,default=0x0 */
#define H60_PLL1REFCLKDIVf                     (CS_CSR4r),          (0x00000060) /* field,bit[6:5],RW,default=0x0 */
#define H60_PLL1MCLKDIV40Bf                    (CS_CSR4r),          (0x0000000c) /* field,bit[3:2],RW,default=0x0 */
#define H60_PLL1MCLKDIV32Bf                    (CS_CSR4r),          (0x00000003) /* field,bit[1:0],RW,default=0x0 */
#define H60_PLL0OUTOFLOCKf                     (CS_CSR5r),          (0x00000004) /* field,bit[2:2],RO,default=0x1 */
#define H60_PLL1OUTOFLOCKf                     (CS_CSR5r),          (0x00000001) /* field,bit[0:0],RO,default=0x1 */
#define H60_CS_SPARE0f                         (CS_CSR6r),          (0x0000ffff) /* field,bit[15:0],RW,default=0x0 */
#define H60_PLL0CPINTCRNTSELf                  (CS_CSR10r),         (0x000001c0) /* field,bit[8:6],RW,default=0x1 */
#define H60_PLL0CPPROPCRNTSELf                 (CS_CSR10r),         (0x0000003c) /* field,bit[5:2],RW,default=0xF */
#define H60_PLL0DLFCINTSELf                    (CS_CSR11r),         (0x0000f800) /* field,bit[15:11],RW,default=0x14 */
#define H60_PLL1CPINTCRNTSELf                  (CS_CSR19r),         (0x00003800) /* field,bit[13:11],RW,default=0x1 */
#define H60_PLL1CPPROPCRNTSELf                 (CS_CSR19r),         (0x000000f0) /* field,bit[7:4],RW,default=0xF */
#define H60_PLL1DLFCINTSELf                    (CS_CSR20r),         (0x0000f800) /* field,bit[15:11],RW,default=0x14 */
#define H60_PLL0VREGPLLBLEEDBf                 (CS_CSR14r),         (0x00000002) /* field,bit[1:1],RW,default=0x1 */
#define H60_PLL0VREGDLFBLEEDBf                 (CS_CSR14r),         (0x00000001) /* field,bit[0:0],RW,default=0x1 */
#define H60_PLL0VREGVCODIVBLEEDBf              (CS_CSR15r),         (0x00000040) /* field,bit[6:6],RW,default=0x1 */
#define H60_PLL1VREGPLLBLEEDBf                 (CS_CSR23r),         (0x00000002) /* field,bit[1:1],RW,default=0x1 */
#define H60_PLL1VREGDLFBLEEDBf                 (CS_CSR23r),         (0x00000001) /* field,bit[0:0],RW,default=0x1 */
#define H60_PLL1VREGVCODIVBLEEDBf              (CS_CSR24r),         (0x00000040) /* field,bit[6:6],RW,default=0x1 */
#define H60_CS_SPARE1f                         (CS_CSR35r),         (0x0000ffff) /* field,bit[15:0],RW,default=0x0 */
#define H60_CORECLKSELf                        (SS_CSR11r),         (0x00001000) /* field,bit[12:12],RW,default=0x0 */
#define H60_PLL0CORECLKDIVSELf                 (SS_CSR11r),         (0x00000f00) /* field,bit[11:8],RW,default=0x9 */
#define H60_PLL1CORECLKDIVSELf                 (SS_CSR11r),         (0x000000f0) /* field,bit[7:4],RW,default=0x9 */
#define H60_CORECLK2DIGDIVf                    (SS_CSR11r),         (0x00000001) /* field,bit[0:0],RW,default=0x1 */
#define H60_CS_CORECLK_SELEXT_FROMPINf         (SS_CSR32r),         (0x00001000) /* field,bit[12:12],RO,default=0x0 */
#define H60_PLL0HSCLKDIVf                      (SS_CSR34r),         (0x0000000f) /* field,bit[3:0],RWW,default=0x3 */
#define H60_PLL1HSCLKDIVf                      (SS_CSR37r),         (0x0000000f) /* field,bit[3:0],RWW,default=0x3 */
#define H60_CSPWRSEQDONEf                      (SS_CSR39r),         (0x00000040) /* field,bit[6:6],RO,default=0x1 */
#define H60_PLL1PWRDBTRIGGERf                  (SS_CSR39r),         (0x00000008) /* field,bit[3:3],RW,default=0x1 */
#define H60_PLL0PWRDBTRIGGERf                  (SS_CSR39r),         (0x00000004) /* field,bit[2:2],RW,default=0x1 */
#define H60_PLL0_FRACN_PI_DIVMAIN_PWRDNBf      (SS_CSR52r),         (0x00008000) /* field,bit[15:15],RW,default=0x0 */
#define H60_PLL0_FRACN_PI_PWRDNBf              (SS_CSR52r),         (0x00004000) /* field,bit[14:14],RW,default=0x0 */
#define H60_PLL0_FRACN_PI_QDIV_PWRDNBf         (SS_CSR52r),         (0x00002000) /* field,bit[13:13],RW,default=0x0 */
#define H60_PLL0_FRACN_PI_DIV1_PWRDNBf         (SS_CSR52r),         (0x00000800) /* field,bit[11:11],RW,default=0x0 */
#define H60_PLL0_FRACN_PI_MUXCTRLf             (SS_CSR53r),         (0x00000020) /* field,bit[5:5],RW,default=0x0 */
#define H60_PLL0_FRACN_PI_DIVCTRLf             (SS_CSR53r),         (0x0000001c) /* field,bit[4:2],RW,default=0x1 */
#define H60_PLL0_FRACN_PI_SDM_ENf              (SS_CSR54r),         (0x00000800) /* field,bit[11:11],RW,default=0x0 */
#define H60_PLL0_FRACN_PI_SDM_RST_Nf           (SS_CSR54r),         (0x00000400) /* field,bit[10:10],RW,default=0x1 */
#define H60_PLL0_FRACN_PI_FCWLf                (SS_CSR55r),         (0x0000ffff) /* field,bit[15:0],RWW,default=0x0 */
#define H60_PLL0_FRACN_PI_FCW_UPDATE_ENf       (SS_CSR56r),         (0x00004000) /* field,bit[14:14],RW,default=0x1 */
#define H60_PLL0_FRACN_PI_FCW_VALIDf           (SS_CSR56r),         (0x00002000) /* field,bit[13:13],RWW,default=0x0 */
#define H60_PLL0_FRACN_PI_FCWHf                (SS_CSR56r),         (0x00001fff) /* field,bit[12:0],RWW,default=0x0 */
#define H60_PLL1_FRACN_PI_DIVMAIN_PWRDNBf      (SS_CSR57r),         (0x00008000) /* field,bit[15:15],RW,default=0x0 */
#define H60_PLL1_FRACN_PI_PWRDNBf              (SS_CSR57r),         (0x00004000) /* field,bit[14:14],RW,default=0x0 */
#define H60_PLL1_FRACN_PI_QDIV_PWRDNBf         (SS_CSR57r),         (0x00002000) /* field,bit[13:13],RW,default=0x0 */
#define H60_PLL1_FRACN_PI_DIV1_PWRDNBf         (SS_CSR57r),         (0x00000800) /* field,bit[11:11],RW,default=0x0 */
#define H60_PLL1_FRACN_PI_MUXCTRLf             (SS_CSR58r),         (0x00000020) /* field,bit[5:5],RW,default=0x0 */
#define H60_PLL1_FRACN_PI_DIVCTRLf             (SS_CSR58r),         (0x0000001c) /* field,bit[4:2],RW,default=0x1 */
#define H60_PLL1_FRACN_PI_SDM_ENf              (SS_CSR59r),         (0x00000800) /* field,bit[11:11],RW,default=0x0 */
#define H60_PLL1_FRACN_PI_SDM_RST_Nf           (SS_CSR59r),         (0x00000400) /* field,bit[10:10],RW,default=0x1 */
#define H60_PLL1_FRACN_PI_FCWLf                (SS_CSR60r),         (0x0000ffff) /* field,bit[15:0],RWW,default=0x0 */
#define H60_PLL1_FRACN_PI_FCW_UPDATE_ENf       (SS_CSR61r),         (0x00004000) /* field,bit[14:14],RW,default=0x1 */
#define H60_PLL1_FRACN_PI_FCW_VALIDf           (SS_CSR61r),         (0x00002000) /* field,bit[13:13],RWW,default=0x0 */
#define H60_PLL1_FRACN_PI_FCWHf                (SS_CSR61r),         (0x00001fff) /* field,bit[12:0],RWW,default=0x0 */
#define H60_MCURSTBf                           (CP_CSR12r),         (0x00000001) /* field,bit[0:0],RW,default=0x0 */

/* ds reg */
#define H60_DS_TERMCALf                        (DSCOM_DIG_CSR2r),   (0x0000000f) /* field,bit[3:0],RW,default=0x5 */
#define H60_DSCLK_BUS_SELRXf                   (DSCOM_ANA_CSR15r),  (0x00000200) /* field,bit[9:9],RW,default=0x0 */
#define H60_DSCLK_BUS_SELTXf                   (DSCOM_ANA_CSR15r),  (0x00000100) /* field,bit[8:8],RW,default=0x0 */

#define H60_RXPWRDBf                           (DSRX_DIG_CSR0r),    (0x00000010) /* field,bit[4:4],RW,default=0x1 */
#define H60_RXGRAYDECf                         (DSRX_DIG_CSR2r),    (0x00002000) /* field,bit[13:13],RW,default=0x0 */
#define H60_RXBITORDERf                        (DSRX_DIG_CSR2r),    (0x00001000) /* field,bit[12:12],RW,default=0x0 */
#define H60_RXPOLARITYf                        (DSRX_DIG_CSR2r),    (0x00000800) /* field,bit[11:11],RW,default=0x0 */
#define H60_TXRXPARLPBKENf                     (DSRX_DIG_CSR2r),    (0x00000400) /* field,bit[10:10],RW,default=0x0 */
#define H60_MODCONFIGf                         (DSRX_DIG_CSR2r),    (0x00000100) /* field,bit[8:8],RW,default=0x1 */
#define H60_DFE_ACTIVEf                        (DSRX_DIG_CSR4r),    (0x00008000) /* field,bit[15:15],RW,default=0x0 */
#define H60_CDR_C0_CFf                         (DSRX_DIG_CSR5r),    (0x00003f80) /* field,bit[13:7],RWW,default=0x00 */
#define H60_CDR_C1_CFf                         (DSRX_DIG_CSR5r),    (0x0000007f) /* field,bit[6:0],RWW,default=0x00 */
#define H60_CDR_C3_CFf                         (DSRX_DIG_CSR6r),    (0x00003f80) /* field,bit[13:7],RWW,default=0x00 */
#define H60_CDR_C4_CFf                         (DSRX_DIG_CSR6r),    (0x0000007f) /* field,bit[6:0],RWW,default=0x00 */
#define H60_CDRTHRESHOLDCFf                    (DSRX_DIG_CSR7r),    (0x00000fff) /* field,bit[11:0],RWW,default=0x000 */
#define H60_CDR_COEF_FREEZEf                   (DSRX_DIG_CSR8r),    (0x00003c00) /* field,bit[13:10],RW,default=0x0 */
#define H60_RXRATEMODEf                        (DSRX_DIG_CSR9r),    (0x00000003) /* field,bit[1:0],RW,default=0x0 */
#define H60_CDRCOEFCONFIGf                     (DSRX_DIG_CSR10r),   (0x00002000) /* field,bit[13:13],RW,default=0x1 */
#define H60_DPHECFf                            (DSRX_DIG_CSR13r),   (0x00000fff) /* field,bit[11:0],RWW,default=0x000 */
#define H60_COEF_CONFIGf                       (DSRX_DIG_CSR22r),   (0x00007800) /* field,bit[14:11],RW,default=0xF */
#define H60_MU_SELf                            (DSRX_DIG_CSR22r),   (0x00000007) /* field,bit[2:0],RW,default=0x1 */
#define H60_LOS_LMS_THRESHf                    (DSRX_DIG_CSR25r),   (0x00007c00) /* field,bit[14:10],RW,default=0x08 */
#define H60_RND_MAX_THRESHf                    (DSRX_DIG_CSR26r),   (0x000003ff) /* field,bit[9:0],RW,default=0x064 */
#define H60_CDRPROPFREEZEf                     (DSRX_DIG_CSR39r),   (0x00000008) /* field,bit[3:3],RW,default=0x0 */
#define H60_CDRFREEZECFGf                      (DSRX_DIG_CSR39r),   (0x00000010) /* field,bit[4:4],RW,default=0x0 */
#define H60_CDR2NDLPCFGf                       (DSRX_DIG_CSR39r),   (0x00000100) /* field,bit[8:8],RW,default=0x0 */
#define H60_RXCAL_GOSC_ENf                     (DSRX_DIG_CSR41r),   (0x00000100) /* field,bit[8:8],RW,default=0x0 */
#define H60_RXCAL_GOSC_MODEf                   (DSRX_DIG_CSR41r),   (0x00000070) /* field,bit[6:4],RW,default=0x0 */
#define H60_RXCAL_NUM_CLK_CYCLESf              (DSRX_DIG_CSR42r),   (0x0000ffff) /* field,bit[15:0],RW,default=0x3e8 */
#define H60_RX_PRBS_MOD4_BYPASSf               (DSRX_DIG_CSR113r),  (0x00000080) /* field,bit[7:7],RW,default=0x0 */
#define H60_RX_PRBS_GRAY_BYPASSf               (DSRX_DIG_CSR113r),  (0x00000040) /* field,bit[6:6],RW,default=0x0 */
#define H60_RX_PRBS_CHK_ENf                    (DSRX_DIG_CSR113r),  (0x00000020) /* field,bit[5:5],RW,default=0x0 */
#define H60_RX_PRBS_CHK_MODEf                  (DSRX_DIG_CSR113r),  (0x00000010) /* field,bit[4:4],RW,default=0x0 */
#define H60_RX_PRBS_CHK_PATSELf                (DSRX_DIG_CSR113r),  (0x0000000f) /* field,bit[3:0],RW,default=0x0 */
#define H60_DSRX_DIG_CSR114_ALLf               (DSRX_DIG_CSR114r),  (0xffffffff) /* reg,default=0x00000000 */
#define H60_CDR_LI_INTEGRATOR_OUTf             (DSRX_DIG_CSR115r),  (0x00003fff) /* field,bit[13:0],RW,default=0x03E8 */
#define H60_INTEGRATOR_OUTf                    (DSRX_DIG_CSR119r),  (0x00003fff) /* field,bit[13:0],RO,default=0x0000 */
#define H60_JITAMPf                            (DSRX_DIG_CSR120r),  (0x000001fc) /* field,bit[8:2],RW,default=0x00 */
#define H60_PPMGENf                            (DSRX_DIG_CSR120r),  (0x00003e00) /* field,bit[13:9],RW,default=0x00 */
#define H60_JITPPMCTRLf                        (DSRX_DIG_CSR120r),  (0x00000001) /* field,bit[0:0],RW,default=0x0 */
#define H60_JITTER_GEN_ENf                     (DSRX_DIG_CSR120r),  (0x00000002) /* field,bit[1:1],RW,default=0x0 */
#define H60_JITSTEPf                           (DSRX_DIG_CSR121r),  (0x00000fff) /* field,bit[11:0],RW,default=0x000 */

#define H60_RX_TERM_FLOAT_ENf                  (DSRX_ANA_CSR4r),    (0x00000100) /* field,bit[8:8],RW,default=0x1 */
#define H60_RX_CTLE_PASSATTNf                  (DSRX_ANA_CSR5r),    (0x00000f00) /* field,bit[11:8],RW,default=0x0 */
#define H60_RX_CTLE_PASSTUNEf                  (DSRX_ANA_CSR5r),    (0x000000fe) /* field,bit[7:1],RW,default=0x00 */
#define H60_RX_CTLE_PASSCMBGf                  (DSRX_ANA_CSR6r),    (0x00000380) /* field,bit[9:7],RW,default=0x0 */
#define H60_RX_CTLE_PASSRMBGf                  (DSRX_ANA_CSR6r),    (0x0000007f) /* field,bit[6:0],RW,default=0x00 */
#define H60_RX_CTLE_VREG_BLEEDBf               (DSRX_ANA_CSR7r),    (0x00000002) /* field,bit[1:1],RW,default=0x1 */
#define H60_RX_CTLE_SQUELCHf                   (DSRX_ANA_CSR8r),    (0x000003c0) /* field,bit[9:6],RW,default=0x0 */
#define H60_RX_CTLE_ST1_AGCf                   (DSRX_ANA_CSR8r),    (0x0000000f) /* field,bit[3:0],RW,default=0x7 */
#define H60_RX_CTLE_ST1_RDGENf                 (DSRX_ANA_CSR10r),   (0x00000380) /* field,bit[9:7],RW,default=0x3 */
#define H60_RX_CTLE_ST1_RSQUELCHf              (DSRX_ANA_CSR10r),   (0x00001c00) /* field,bit[12:10],RW,default=0x0 */
#define H60_RX_CTLE_ST1_COARSEBSTf             (DSRX_ANA_CSR11r),   (0x0000001c) /* field,bit[4:2],RW,default=0x2 */
#define H60_RX_CTLE_ST1_FINEBSTf               (DSRX_ANA_CSR11r),   (0x000000e0) /* field,bit[7:5],RW,default=0x3 */
#define H60_RX_CTLE_ST1_HFZEROf                (DSRX_ANA_CSR11r),   (0x00000003) /* field,bit[1:0],RW,default=0x1 */
#define H60_RX_LPBK_ENf                        (DSRX_ANA_CSR11r),   (0x00000700) /* field,bit[10:8],RW,default=0x0 */
#define H60_RX_CTLE_ST1_CDGENf                 (DSRX_ANA_CSR12r),   (0x00007800) /* field,bit[14:11],RW,default=0x7 */
#define H60_RX_CTLE_VCMSELf                    (DSRX_ANA_CSR12r),   (0x00000700) /* field,bit[10:8],RW,default=0x3 */
#define H60_RX_CTLE_UNPEAKf                    (DSRX_ANA_CSR13r),   (0x00007800) /* field,bit[14:11],RW,default=0x0 */
#define H60_RX_SAR_DAC_COUNT_RSTBf             (DSRX_ANA_CSR24r),   (0x00002000) /* field,bit[13:13],RW,default=0x1 */
#define H60_RX_SAR_DAC_REG_SELf                (DSRX_ANA_CSR24r),   (0x00001000) /* field,bit[12:12],RW,default=0x0 */
#define H60_RX_SAR_DAC_WRENf                   (DSRX_ANA_CSR24r),   (0x00000800) /* field,bit[11:11],RW,default=0x0 */
#define H60_RX_SAR_SIZE_REG_SELf               (DSRX_ANA_CSR24r),   (0x00000400) /* field,bit[10:10],RW,default=0x0 */
#define H60_RX_SAR_BK0_TRKCLK_DELAYf           (DSRX_ANA_CSR27r),   (0x00003e00) /* field,bit[13:9],RW,default=0x0 */
#define H60_RX_SAR_BK90_TRKCLK_DELAYf          (DSRX_ANA_CSR27r),   (0x0000007c) /* field,bit[6:2],RW,default=0x0 */
#define H60_RX_SAR_BK180_TRKCLK_DELAYf         (DSRX_ANA_CSR28r),   (0x00003e00) /* field,bit[13:9],RW,default=0x0 */
#define H60_RX_SAR_BK270_TRKCLK_DELAYf         (DSRX_ANA_CSR28r),   (0x0000007c) /* field,bit[6:2],RW,default=0x0 */
#define H60_RX_REFGEN_RANGEf                   (DSRX_ANA_CSR31r),   (0x00000030) /* field,bit[5:4],RW,default=0x0 */

#define H60_DSTX_ANA_CSR1_ALLf                 (DSTX_ANA_CSR1r),    (0xffffffff) /* reg,default=0x00000000 */
#define H60_TX_DRV_ZCTRL_TAP_PWRDNBf           (DSTX_ANA_CSR1r),    (0x00001000) /* field,bit[12:12],RW,default=0x0 */
#define H60_TX_DRV_TAP_PWRDNBf                 (DSTX_ANA_CSR1r),    (0x00001ffe) /* field,bit[12:1],RW,default=0x666 */
#define H60_TX_BITSLIPf                        (DSTX_ANA_CSR2r),    (0x00000020) /* field,bit[5:5],RW,default=0x0 */
#define H60_TX_DRVREG_LEVELf                   (DSTX_ANA_CSR3r),    (0x000007e0) /* field,bit[10:5],RW,default=0x1F */
#define H60_TX_DRV_AMPCTRLf                    (DSTX_ANA_CSR4r),    (0x0000fc00) /* field,bit[15:10],RWW,default=0x3F */
#define H60_TX_PREDRVREG_LEVELf                (DSTX_ANA_CSR4r),    (0x000003f0) /* field,bit[9:4],RW,default=0x1F */
#define H60_TX_DRV_TAPDEL_SELf                 (DSTX_ANA_CSR6r),    (0x00000fff) /* field,bit[11:0],RW,default=0x000 */
#define H60_TX_DRV_UP_OCB_31TO16f              (DSTX_ANA_CSR7r),    (0x0000ffff) /* field,bit[15:0],RW,default=0xffff */
#define H60_TX_DRV_DN_OC_31TO16f               (DSTX_ANA_CSR8r),    (0x0000ffff) /* field,bit[15:0],RW,default=0x0 */
#define H60_TXCP_DCD_ADJf                      (DSTX_ANA_CSR9r),    (0x00001e00) /* field,bit[12:9],RW,default=0x0 */
#define H60_TXCP_DCD_ADJ_SIGNf                 (DSTX_ANA_CSR9r),    (0x00000100) /* field,bit[8:8],RW,default=0x0 */
#define H60_TX_DRV_UP_OCB_35TO32f              (DSTX_ANA_CSR9r),    (0x000000f0) /* field,bit[7:4],RWW,default=0x0 */
#define H60_TX_DRV_DN_OC_35TO32f               (DSTX_ANA_CSR9r),    (0x0000000f) /* field,bit[3:0],RWW,default=0xF */
#define H60_TX_DRV_UP_OCBf                     (DSTX_ANA_CSR10r),   (0x0000ffff) /* field,bit[15:0],RWW,default=0x0 */
#define H60_TX_DRV_DN_OCf                      (DSTX_ANA_CSR11r),   (0x0000ffff) /* field,bit[15:0],RW,default=0x0 */
#define H60_TX_DRV_HSWING_DUALMODE_PWRDNBf     (DSTX_ANA_CSR14r),   (0x00010000) /* field,bit[16:16],RW,default=0x0 */
#define H60_TX_DRV_HSWING_ENf                  (DSTX_ANA_CSR15r),   (0x00000040) /* field,bit[6:6],RWW,default=0x0 */
#define H60_TX_LPBK_SELPf                      (DSTX_ANA_CSR17r),   (0x0000c000) /* field,bit[15:14],RW,default=0x0 */
#define H60_TX_LPBK_SELMf                      (DSTX_ANA_CSR17r),   (0x00003000) /* field,bit[13:12],RW,default=0x0 */

#define H60_TX_PA_CLK_SELf                     (DSTX_DIG_CSR1r),    (0x00001000) /* field,bit[12:12],RW,default=0x0 */
#define H60_TX_PAM4_MODEf                      (DSTX_DIG_CSR1r),    (0x00000800) /* field,bit[11:11],RW,default=0x1 */
#define H60_TXBITORDERf                        (DSTX_DIG_CSR1r),    (0x00000200) /* field,bit[9:9],RW,default=0x0 */
#define H60_TXSYMBORDERf                       (DSTX_DIG_CSR1r),    (0x00000100) /* field,bit[8:8],RW,default=0x0 */
#define H60_TXPOLARITYf                        (DSTX_DIG_CSR1r),    (0x00000080) /* field,bit[7:7],RW,default=0x0 */
#define H60_TXRATEMODEf                        (DSTX_DIG_CSR1r),    (0x00000060) /* field,bit[6:5],RW,default=0x0 */
#define H60_TXCLKSELf                          (DSTX_DIG_CSR1r),    (0x00000010) /* field,bit[4:4],RW,default=0x0 */
#define H60_TXDATASELf                         (DSTX_DIG_CSR1r),    (0x0000000c) /* field,bit[3:2],RW,default=0x0 */
#define H60_TXALIGNMODEf                       (DSTX_DIG_CSR1r),    (0x00000003) /* field,bit[1:0],RW,default=0x0 */

#define H60_TXPOLARITYf                        (DSTX_DIG_CSR1r),    (0x00000080) /* field,bit[7:7],RW,default=0x0 */
#define H60_TXDATASELf                         (DSTX_DIG_CSR1r),    (0x0000000c) /* field,bit[3:2],RW,default=0x0 */
#define H60_TXPA_SLAVE_FLAGf                   (DSTX_DIG_CSR2r),    (0x00000080) /* field,bit[7:7],RO,default=0x1 */
#define H60_TXPA_OUTOFBANDf                    (DSTX_DIG_CSR2r),    (0x00000060) /* field,bit[6:5],RO,default=0x1 */
#define H60_TXPA_ACCDEPTHf                     (DSTX_DIG_CSR2r),    (0x0000001c) /* field,bit[4:2],RW,default=0x0 */
#define H60_TX_GRAY_BYPASSf                    (DSTX_DIG_CSR4r),    (0x00000080) /* field,bit[7:7],RW,default=0x01 */
#define H60_TX_MOD4ENC_BYPASSf                 (DSTX_DIG_CSR4r),    (0x00000020) /* field,bit[5:5],RW,default=0x01 */
#define H60_PRBSGENENf                         (DSTX_DIG_CSR4r),    (0x00000010) /* field,bit[4:4],RW,default=0x0 */
#define H60_PRBSGENPATSELf                     (DSTX_DIG_CSR4r),    (0x0000000f) /* field,bit[3:0],RW,default=0x0 */
#define H60_TXDRV_TAPMAP_ENf                   (DSTX_DIG_CSR11r),   (0x00000040) /* field,bit[6:6],RW,default=0x0 */
#define H60_TXFIRPOST_SIGNf                    (DSTX_DIG_CSR11r),   (0x00000020) /* field,bit[5:5],RWW,default=0x0 */
#define H60_TXFIRPOSTf                         (DSTX_DIG_CSR11r),   (0x0000001f) /* field,bit[4:0],RWW,default=0x00 */
#define H60_TXFIRPRE1_SIGNf                    (DSTX_DIG_CSR12r),   (0x00000020) /* field,bit[5:5],RWW,default=0x0 */
#define H60_TXFIRPRE1f                         (DSTX_DIG_CSR12r),   (0x0000001f) /* field,bit[4:0],RWW,default=0x00 */

/* ds api reg */
#define H60_DSAPI_CSR0_ALLf                    (DSAPI_CSR0r),       (0xffffffff) /* DSAPICSR0 */
#define H60_DS_DFX_EN_APIf                     (DSAPI_CSR0r),       (0x04000000) /* [26],RW,default=(0) */
#define H60_DS_DFX_MODE_APIf                   (DSAPI_CSR0r),       (0x03c00000) /* [25:22],RW,default=(0) */
#define H60_DS_CALIB_EN_APIf                   (DSAPI_CSR0r),       (0x00000030) /* [5:4],RW,default=0x0 */
#define H60_INIT_ADAPT_EN_APIf                 (DSAPI_CSR0r),       (0x0000000c) /* [3:2],RW,default=0x0 */
#define H60_CONT_ADAPT_EN_APIf                 (DSAPI_CSR0r),       (0x00000003) /* [1:0],RW,default=0x0 */
#define H60_DSAPI_CSR1_ALLf                    (DSAPI_CSR1r),       (0xffffffff) /* DSAPICSR1 */
#define H60_DSAPI_CSR2_ALLf                    (DSAPI_CSR2r),       (0xffffffff) /* DSAPICSR2 */
#define H60_CONT_NINETAPCDRBYPASS_APIf         (DSAPI_CSR2r),       (0x00008000) /* [15],RW,default=0x0 */
#define H60_CONT_MLBYPASS_APIf                 (DSAPI_CSR2r),       (0x00004000) /* [14],RW,default=0x0 */
#define H60_MIN_PRE2POST_APIf                  (DSAPI_CSR2r),       (0x00000030) /* [5:4],RW,default=0x0 */
#define H60_DSAPI_CSR3_ALLf                    (DSAPI_CSR3r),       (0xffffffff) /* DSAPICSR3 */
#define H60_CONT_TNHGAINBYPASS_APIf            (DSAPI_CSR3r),       (0x00000800) /* [11],RW,default=0x0 */
#define H60_CONT_LINKRECOVERYBYPASS_APIf       (DSAPI_CSR3r),       (0x00000080) /* [7],RW,default=0x0 */
#define H60_CONT_BIASRCVITRIM_APIf             (DSAPI_CSR3r),       (0x00000010) /* [4],RW,default=0x0 */
#define H60_CONT_CTLECMTRKBYPASS_APIf          (DSAPI_CSR3r),       (0x00000008) /* [3],RW,default=0x0 */
#define H60_CONT_CTLEOFSTBYPASS_APIf           (DSAPI_CSR3r),       (0x00000004) /* [2],RW,default=0x0 */
#define H60_CONT_SAROFSTBYPASS_APIf            (DSAPI_CSR3r),       (0x00000002) /* [1],RW,default=0x0 */
#define H60_CONT_CDRPREBYPASS_APIf             (DSAPI_CSR3r),       (0x00000001) /* [0],RW,default=0x0 */
#define H60_DSAPI_CSR4_ALLf                    (DSAPI_CSR4r),       (0xffffffff) /* DSAPICSR4 */
#define H60_DS_INCREASE_CDR_LOCK_APIf          (DSAPI_CSR4r),       (0x00004000) /* [14],RW,default=0x0 */
#define H60_CONT_ABGCBYPASS_APIf               (DSAPI_CSR4r),       (0x00002000) /* [13],RW,default=0x0 */
#define H60_DSAPI_CSR5_ALLf                    (DSAPI_CSR5r),       (0xffffffff) /* DSAPICSR5 */
#define H60_DS_STATE_APIf                      (DSAPI_CSR5r),       (0x0000f000) /* [15:12],RW,default=0x0 */
#define H60_INIT_ADAPT_STATUS_APIf             (DSAPI_CSR5r),       (0x00000300) /* [9:8],RW,default=0x0 */
#define H60_CONT_ADAPT_STATUS_APIf             (DSAPI_CSR5r),       (0x000000c0) /* [7:6],RW,default=0x0 */
#define H60_LT_TX_PROPOSAL_IND_APIf            (DSAPI_CSR5r),       (0x00000020) /* [5],RW,default=0x0 */
#define H60_LMS_LOS_DET_STATUS_APIf            (DSAPI_CSR5r),       (0x00000004) /* [2],RW,default=0x0 */
#define H60_CDR_LOS_DET_STATUS_APIf            (DSAPI_CSR5r),       (0x00000002) /* [1],RW,default=0x0 */
#define H60_DSAPI_CSR6_ALLf                    (DSAPI_CSR6r),       (0xffffffff) /* DSAPICSR6 */
#define H60_DSP_ADC_UPGRADE_TH_APIf            (DSAPI_CSR6r),       (0x0000e000)
#define H60_DSP_FFE_UPGRADE_TH_APIf            (DSAPI_CSR6r),       (0x00001c00)
#define H60_DSAPI_CSR7_ALLf                    (DSAPI_CSR7r),       (0xffffffff) /* DSAPICSR7 */
#define H60_DS_RX_INSERTION_LOSS_APIf          (DSAPI_CSR7r),       (0x000000fc) /* [7:2],RW,default=0x0 */
#define H60_DSAPI_CSR8_ALLf                    (DSAPI_CSR8r),       (0xffffffff) /* DSAPICSR8 */
#define H60_DSAPI_CSR9_ALLf                    (DSAPI_CSR9r),       (0xffffffff) /* DSAPICSR9 */
#define H60_DSAPI_CSR10_ALLf                   (DSAPI_CSR10r),      (0xffffffff) /* DSAPICSR10 */
#define H60_DSP_PD_UPGRADE_TH_APIf             (DSAPI_CSR10r),      (0x00000700)
#define H60_DSAPI_CSR11_ALLf                   (DSAPI_CSR11r),      (0xffffffff) /* DSAPICSR11 */
#define H60_DSAPI_CSR12_ALLf                   (DSAPI_CSR12r),      (0xffffffff) /* DSAPICSR12 */
#define H60_DSAPI_CSR13_ALLf                   (DSAPI_CSR13r),      (0xffffffff) /* DSAPICSR13 */
#define H60_DS_DFX_DONE_STATUS_APIf            (DSAPI_CSR13r),      (0x00000080) /* [7],RO,default=(0) */

#define H60_CSAPI_CSR0_ALLf                    (CSAPI_CSR0r),       (0xffffffff) /* CSAPICSR0 */
#define H60_CS_READTEMPBYPASS_APIf             (CSAPI_CSR0r),       (0x00000020) /* [5],RW,default=0x0 */
#define H60_CSAPI_CSR1_ALLf                    (CSAPI_CSR1r),       (0xffffffff) /* CSAPICSR1 */
#define H60_REFCLK0_FREQM100_APIf              (CSAPI_CSR1r),       (0x0000ffff) /* [15:0],RW,default=0x0 */
#define H60_CSAPI_CSR2_ALLf                    (CSAPI_CSR2r),       (0xffffffff) /* CSAPICSR2 */
#define H60_REFCLK1_FREQM100_APIf              (CSAPI_CSR2r),       (0x0000ffff) /* [15:0],RW,default=0x0 */
#define H60_CSAPI_CSR3_ALLf                    (CSAPI_CSR3r),       (0xffffffff) /* CSAPICSR3 */
#define H60_CS_CAL_MODE_APIf                   (CSAPI_CSR3r),       (0x0000f000) /* [15:12],RW,default=0x0 */
#define H60_CS_FNPLL1_EN_APIf                  (CSAPI_CSR3r),       (0x00000800) /* [11],RW,default=0x0 */
#define H60_CS_FNPLL0_EN_APIf                  (CSAPI_CSR3r),       (0x00000400) /* [10],RW,default=0x0 */
#define H60_CS_CALIB_EN_APIf                   (CSAPI_CSR3r),       (0x00000300) /* [9:8],RW,default=0x0 */
#define H60_CSAPI_CSR4_ALLf                    (CSAPI_CSR4r),       (0xffffffff) /* CSAPICSR4 */
#define H60_CS_POWER_MODE_APIf                 (CSAPI_CSR4r),       (0x0000e000) /* [15:13],RW,default=0x0 */
#define H60_CORECLK_OPT_BYPASS_APIf            (CSAPI_CSR4r),       (0x00001000) /* [12],RW,default=0x0 */
#define H60_CSAPI_CSR5_ALLf                    (CSAPI_CSR5r),       (0xffffffff) /* CSAPICSR5 */
#define H60_CSAPI_CSR6_ALLf                    (CSAPI_CSR6r),       (0xffffffff) /* CSAPICSR6 */
#define H60_CSAPI_CSR7_ALLf                    (CSAPI_CSR7r),       (0xffffffff) /* CSAPICSR7 */
#define H60_CSAPI_CSR8_ALLf                    (CSAPI_CSR8r),       (0xffffffff) /* CSAPICSR8 */
#define H60_CSAPI_CSR9_ALLf                    (CSAPI_CSR9r),       (0xffffffff) /* CSAPICSR9 */
#define H60_CS_CALIB_STATUS_APIf               (CSAPI_CSR9r),       (0x00000700) /* [10:8],RW,default=0x0 */
#define H60_CSAPI_CSR10_ALLf                   (CSAPI_CSR10r),      (0xffffffff) /* CSAPICSR10 */
#define H60_CSAPI_CSR11_ALLf                   (CSAPI_CSR11r),      (0xffffffff) /* CSAPICSR11*/

#define GET_BIT(value, n)   (((value) >> (n)) & 0x1U)
#define SET_BIT(value, n)   ((value) = ((value) & (~(0x1U << (n)))) | (0x1U << (n)))
#endif