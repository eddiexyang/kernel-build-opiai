/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
  文 件 名   : soc_asp_codec_interface.h
  版 本 号   : 初稿
  作    者   : Excel2Code
  生成日期   : 2019-11-11 17:52:05
  最近修改   :
  功能描述   : 接口头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年11月11日
    作    者   : asp
    修改内容   : 从《V100 SOC寄存器手册_ASP_CODEC.xml》自动生成

******************************************************************************/

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#ifndef __SOC_ASP_CODEC_INTERFACE_H__
#define __SOC_ASP_CODEC_INTERFACE_H__

#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif


/*****************************************************************************
  2 宏定义
*****************************************************************************/

/****************************************************************************
                     (1/1) SOC CODEC
 ****************************************************************************/
#ifndef __SOC_H_FOR_ASM__


/* 寄存器说明：CODEC版本寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_VERSION_UNION */
#define SOC_ASP_CODEC_VERSION_ADDR(base)                      ((base) + (0x0000UL))

/* 寄存器说明：CODEC子模块时钟门控寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_CLK_EN0_UNION */
#define SOC_ASP_CODEC_CODEC_CLK_EN0_ADDR(base)                ((base) + (0x0004UL))

/* 寄存器说明：CODEC子模块时钟门控寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_CLK_EN1_UNION */
#define SOC_ASP_CODEC_CODEC_CLK_EN1_ADDR(base)                ((base) + (0x0008UL))

/* 寄存器说明：CODEC子系统门控使能寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_CLK_EN2_UNION */
#define SOC_ASP_CODEC_CODEC_CLK_EN2_ADDR(base)                ((base) + (0x000CUL))

/* 寄存器说明：CODEC的复位寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_SW_RST_N_UNION */
#define SOC_ASP_CODEC_CODEC_SW_RST_N_ADDR(base)               ((base) + (0x0010UL))

/* 寄存器说明：I2S1_PCM接口控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S1_CTRL_UNION */
#define SOC_ASP_CODEC_I2S1_CTRL_ADDR(base)                    ((base) + (0x0014UL))

/* 寄存器说明：I2S1_TDM接口控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_I2S1_TDM_CTRL0_UNION */
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_ADDR(base)               ((base) + (0x0018UL))

/* 寄存器说明：I2S1_TDM接口控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_I2S1_TDM_CTRL1_UNION */
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_ADDR(base)               ((base) + (0x001CUL))

/* 寄存器说明：I2S2_PCM接口控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S2_PCM_CTRL_UNION */
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_ADDR(base)                ((base) + (0x0020UL))

/* 寄存器说明：I2S3_PCM接口控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S3_PCM_CTRL_UNION */
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_ADDR(base)                ((base) + (0x0024UL))

/* 寄存器说明：I2S4_PCM接口控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S4_CTRL_UNION */
#define SOC_ASP_CODEC_I2S4_CTRL_ADDR(base)                    ((base) + (0x0028UL))

/* 寄存器说明：I2S4_TDM接口控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_I2S4_TDM_CTRL0_UNION */
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_ADDR(base)               ((base) + (0x002CUL))

/* 寄存器说明：I2S4_TDM接口控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_I2S4_TDM_CTRL1_UNION */
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_ADDR(base)               ((base) + (0x0030UL))

/* 寄存器说明：PGA/MIXER阈值控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_THRE_CTRL0_UNION */
#define SOC_ASP_CODEC_PGA_THRE_CTRL0_ADDR(base)               ((base) + (0x0034UL))

/* 寄存器说明：PGA/MIXER阈值控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_THRE_CTRL1_UNION */
#define SOC_ASP_CODEC_PGA_THRE_CTRL1_ADDR(base)               ((base) + (0x0038UL))

/* 寄存器说明：PGA/MIXER阈值控制寄存器2
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_THRE_CTRL2_UNION */
#define SOC_ASP_CODEC_PGA_THRE_CTRL2_ADDR(base)               ((base) + (0x003CUL))

/* 寄存器说明：PGA GAINOFFSET配置寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_ADDR(base)         ((base) + (0x0040UL))

/* 寄存器说明：PGA GAINOFFSET配置寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_ADDR(base)         ((base) + (0x0044UL))

/* 寄存器说明：PGA GAINOFFSET配置寄存器2
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_ADDR(base)         ((base) + (0x0048UL))

/* 寄存器说明：PGA GAINOFFSET配置寄存器3
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL3_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL3_ADDR(base)         ((base) + (0x004CUL))

/* 寄存器说明：CODEC3_L_DN PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_ADDR(base)         ((base) + (0x0050UL))

/* 寄存器说明：CODEC3_R_DN PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_ADDR(base)         ((base) + (0x0054UL))

/* 寄存器说明：AUDIO下行通路左声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_ADDR(base)          ((base) + (0x0058UL))

/* 寄存器说明：AUDIO下行通路右声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_ADDR(base)          ((base) + (0x005CUL))

/* 寄存器说明：SIDETONE通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SIDETONE_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_ADDR(base)            ((base) + (0x0060UL))

/* 寄存器说明：AUDIO上行通路左声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_ADDR(base)          ((base) + (0x0064UL))

/* 寄存器说明：AUDIO上行通路右声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_ADDR(base)          ((base) + (0x0068UL))

/* 寄存器说明：VOICE上行通路左声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_ADDR(base)          ((base) + (0x006CUL))

/* 寄存器说明：VOICE上行通路右声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_ADDR(base)          ((base) + (0x0070UL))

/* 寄存器说明：MIC3上行PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0074UL))

/* 寄存器说明：MIC4上行PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0078UL))

/* 寄存器说明：MDM_5G上行通路左声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_ADDR(base)         ((base) + (0x007CUL))

/* 寄存器说明：MDM_5G上行通路右声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_ADDR(base)         ((base) + (0x0080UL))

/* 寄存器说明：I2S2 RX L PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_ADDR(base)           ((base) + (0x0084UL))

/* 寄存器说明：I2S2 RX R PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_ADDR(base)           ((base) + (0x0088UL))

/* 寄存器说明：0p5 PGA控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_UNION */
#define SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_ADDR(base)         ((base) + (0x008CUL))

/* 寄存器说明：0p5 PGA控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_UNION */
#define SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_ADDR(base)         ((base) + (0x0090UL))

/* 寄存器说明：ADC1上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0094UL))

/* 寄存器说明：ADC2上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0098UL))

/* 寄存器说明：ADC3上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_ADDR(base)             ((base) + (0x009CUL))

/* 寄存器说明：ADC4上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_ADDR(base)             ((base) + (0x00A0UL))

/* 寄存器说明：ADC5上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_ADDR(base)             ((base) + (0x00A4UL))

/* 寄存器说明：SRCUP控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SRCUP_CTRL_UNION */
#define SOC_ASP_CODEC_SRCUP_CTRL_ADDR(base)                   ((base) + (0x00A8UL))

/* 寄存器说明：SRCDN控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_SRCDN_CTRL0_UNION */
#define SOC_ASP_CODEC_SRCDN_CTRL0_ADDR(base)                  ((base) + (0x00ACUL))

/* 寄存器说明：SRCDN控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_SRCDN_CTRL1_UNION */
#define SOC_ASP_CODEC_SRCDN_CTRL1_ADDR(base)                  ((base) + (0x00B0UL))

/* 寄存器说明：DACL_MIXER4控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_DACL_MIXER4_CTRL0_UNION */
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_ADDR(base)            ((base) + (0x00B4UL))

/* 寄存器说明：DACL_MIXER4控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_DACL_MIXER4_CTRL1_UNION */
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL1_ADDR(base)            ((base) + (0x00B8UL))

/* 寄存器说明：DACR_MIXER4控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_DACR_MIXER4_CTRL0_UNION */
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_ADDR(base)            ((base) + (0x00BCUL))

/* 寄存器说明：DACR_MIXER4控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_DACR_MIXER4_CTRL1_UNION */
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL1_ADDR(base)            ((base) + (0x00C0UL))

/* 寄存器说明：I2S2MIXER2控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_UNION */
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_ADDR(base)          ((base) + (0x00C4UL))

/* 寄存器说明：CODEC的使能控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_CLK_EN3_UNION */
#define SOC_ASP_CODEC_CODEC_CLK_EN3_ADDR(base)                ((base) + (0x00C8UL))

/* 寄存器说明：CODEC的使能控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_CLK_EN4_UNION */
#define SOC_ASP_CODEC_CODEC_CLK_EN4_ADDR(base)                ((base) + (0x00CCUL))

/* 寄存器说明：ADC_FILTER1控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC_FILTER_CTRL0_UNION */
#define SOC_ASP_CODEC_ADC_FILTER_CTRL0_ADDR(base)             ((base) + (0x00D0UL))

/* 寄存器说明：ADC_FILTER控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC_FILTER_CTRL_UNION */
#define SOC_ASP_CODEC_ADC_FILTER_CTRL_ADDR(base)              ((base) + (0x00D4UL))

/* 寄存器说明：DMIC_CTRL控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_DMIC_CTRL_UNION */
#define SOC_ASP_CODEC_DMIC_CTRL_ADDR(base)                    ((base) + (0x00D8UL))

/* 寄存器说明：DMIC_DIV分频控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_DMIC_DIV_UNION */
#define SOC_ASP_CODEC_DMIC_DIV_ADDR(base)                     ((base) + (0x00DCUL))

/* 寄存器说明：SPA反馈通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x00E0UL))

/* 寄存器说明：SPA反馈通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x00E4UL))

/* 寄存器说明：CODEC3下行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_ADDR(base)         ((base) + (0x00E8UL))

/* 寄存器说明：AUDIO下行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_ADDR(base)          ((base) + (0x00ECUL))

/* 寄存器说明：ULTR下行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ADDR(base)           ((base) + (0x00F0UL))

/* 寄存器说明：SPA反馈通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x00F4UL))

/* 寄存器说明：AUDIO上行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x00F8UL))

/* 寄存器说明：VOICE上行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x00FCUL))

/* 寄存器说明：MIC34上行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x0100UL))

/* 寄存器说明：5GMDM下行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_ADDR(base)         ((base) + (0x0104UL))

/* 寄存器说明：5GMDM上行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_ADDR(base)         ((base) + (0x0108UL))

/* 寄存器说明：SPA反馈通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x010CUL))

/* 寄存器说明：SIF_CTRL控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SIF_CTRL_UNION */
#define SOC_ASP_CODEC_SIF_CTRL_ADDR(base)                     ((base) + (0x0110UL))

/* 寄存器说明：CODEC通道内模块状态查询寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_STATE_UNION */
#define SOC_ASP_CODEC_CODEC_STATE_ADDR(base)                  ((base) + (0x0114UL))

/* 寄存器说明：CODEC内部模块采样率控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL0_UNION */
#define SOC_ASP_CODEC_FS_CTRL0_ADDR(base)                     ((base) + (0x0118UL))

/* 寄存器说明：CODEC内部模块采样率控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL1_UNION */
#define SOC_ASP_CODEC_FS_CTRL1_ADDR(base)                     ((base) + (0x011CUL))

/* 寄存器说明：CODEC内部模块采样率控制寄存器2
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL2_UNION */
#define SOC_ASP_CODEC_FS_CTRL2_ADDR(base)                     ((base) + (0x0120UL))

/* 寄存器说明：CODEC内部模块采样率控制寄存器3
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL3_UNION */
#define SOC_ASP_CODEC_FS_CTRL3_ADDR(base)                     ((base) + (0x0124UL))

/* 寄存器说明：CODEC内部模块采样率控制寄存器4
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL4_UNION */
#define SOC_ASP_CODEC_FS_CTRL4_ADDR(base)                     ((base) + (0x0128UL))

/* 寄存器说明：CODEC内部模块采样率控制寄存器5
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL5_UNION */
#define SOC_ASP_CODEC_FS_CTRL5_ADDR(base)                     ((base) + (0x012CUL))

/* 寄存器说明：CODEC内部模块采样率控制寄存器6
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL6_UNION */
#define SOC_ASP_CODEC_FS_CTRL6_ADDR(base)                     ((base) + (0x0130UL))

/* 寄存器说明：CODEC内部模块采样率控制寄存器7
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL7_UNION */
#define SOC_ASP_CODEC_FS_CTRL7_ADDR(base)                     ((base) + (0x0134UL))

/* 寄存器说明：CODEC内部模块输入数据选择寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_DIN_MUX0_UNION */
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_ADDR(base)               ((base) + (0x0138UL))

/* 寄存器说明：CODEC内部模块输入数据选择寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_DIN_MUX1_UNION */
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_ADDR(base)               ((base) + (0x013CUL))

/* 寄存器说明：ADC1通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC1_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC1_DC_OFFSET_ADDR(base)         ((base) + (0x0140UL))

/* 寄存器说明：ADC2通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC2_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC2_DC_OFFSET_ADDR(base)         ((base) + (0x0144UL))

/* 寄存器说明：ADC3通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC3_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC3_DC_OFFSET_ADDR(base)         ((base) + (0x0148UL))

/* 寄存器说明：ADC4通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC4_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC4_DC_OFFSET_ADDR(base)         ((base) + (0x0150UL))

/* 寄存器说明：ADC5通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC5_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC5_DC_OFFSET_ADDR(base)         ((base) + (0x0154UL))

/* 寄存器说明：单口memory的控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MEM_CTRL_S_UNION */
#define SOC_ASP_CODEC_MEM_CTRL_S_ADDR(base)                   ((base) + (0x0158UL))

/* 寄存器说明：双口memory的控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MEM_CTRL_1W2R_UNION */
#define SOC_ASP_CODEC_MEM_CTRL_1W2R_ADDR(base)                ((base) + (0x015CUL))

/* 寄存器说明：HPF滤波器的bypass控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC_CTRL_HPF_UNION */
#define SOC_ASP_CODEC_ADC_CTRL_HPF_ADDR(base)                 ((base) + (0x0160UL))

/* 寄存器说明：PGA GAINOFFSET配置寄存器4
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_ADDR(base)         ((base) + (0x0164UL))

/* 寄存器说明：MIC8上行PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0168UL))

/* 寄存器说明：MIC7上行PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_ADDR(base)             ((base) + (0x016CUL))

/* 寄存器说明：MIC6上行PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0170UL))

/* 寄存器说明：MIC5上行PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0174UL))

/* 寄存器说明：ADC6上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0178UL))

/* 寄存器说明：ADC7上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_ADDR(base)             ((base) + (0x017CUL))

/* 寄存器说明：ADC6上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0180UL))

/* 寄存器说明：MIC56上行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x0184UL))

/* 寄存器说明：MIC78上行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x0188UL))

/* 寄存器说明：CODEC内部模块采样率控制寄存器8
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL8_UNION */
#define SOC_ASP_CODEC_FS_CTRL8_ADDR(base)                     ((base) + (0x018CUL))

/* 寄存器说明：CODEC内部模块采样率控制寄存器9
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL9_UNION */
#define SOC_ASP_CODEC_FS_CTRL9_ADDR(base)                     ((base) + (0x0190UL))

/* 寄存器说明：ADC6通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC6_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC6_DC_OFFSET_ADDR(base)         ((base) + (0x0194UL))

/* 寄存器说明：ADC7通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC7_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC7_DC_OFFSET_ADDR(base)         ((base) + (0x0198UL))

/* 寄存器说明：ADC8通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC8_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC8_DC_OFFSET_ADDR(base)         ((base) + (0x019CUL))

/* 寄存器说明：PGA GAINOFFSET配置寄存器2
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL5_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL5_ADDR(base)         ((base) + (0x01A0UL))

/* 寄存器说明：MDM_5G下行通路左声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_ADDR(base)         ((base) + (0x01A4UL))

/* 寄存器说明：MDM_5G下行通路右声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_ADDR(base)         ((base) + (0x01A8UL))

/* 寄存器说明：SPA反馈通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_ADDR(base)         ((base) + (0x01ACUL))

/* 寄存器说明：DACL通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_DACL_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_DACL_DC_OFFSET_ADDR(base)         ((base) + (0x01B0UL))

/* 寄存器说明：SPA反馈通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_ADDR(base)         ((base) + (0x01B4UL))

/* 寄存器说明：DACL通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_DACR_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_DACR_DC_OFFSET_ADDR(base)         ((base) + (0x01B8UL))

/* 寄存器说明：spa的adc控制信号
   位域定义UNION结构:  SOC_ASP_CODEC_SPAIV1_ADC_FILTER_UNION */
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_ADDR(base)            ((base) + (0x01BCUL))

/* 寄存器说明：classh的控制信号
   位域定义UNION结构:  SOC_ASP_CODEC_CLASSH_L_CTRL1_UNION */
#define SOC_ASP_CODEC_CLASSH_L_CTRL1_ADDR(base)               ((base) + (0x01C0UL))

/* 寄存器说明：DAC_FILTER控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_DAC_FILTER_CTRL_UNION */
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_ADDR(base)              ((base) + (0x01C4UL))

/* 寄存器说明：classh的控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CLASSH_L_CTRL2_UNION */
#define SOC_ASP_CODEC_CLASSH_L_CTRL2_ADDR(base)               ((base) + (0x01C8UL))

/* 寄存器说明：spa的adc控制信号
   位域定义UNION结构:  SOC_ASP_CODEC_SPAIV2_ADC_FILTER_UNION */
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_ADDR(base)            ((base) + (0x01CCUL))

/* 寄存器说明：classh的控制信号
   位域定义UNION结构:  SOC_ASP_CODEC_CLASSH_R_CTRL1_UNION */
#define SOC_ASP_CODEC_CLASSH_R_CTRL1_ADDR(base)               ((base) + (0x01D0UL))

/* 寄存器说明：classh的控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CLASSH_R_CTRL2_UNION */
#define SOC_ASP_CODEC_CLASSH_R_CTRL2_ADDR(base)               ((base) + (0x01D4UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG0_3_UNION */
#define SOC_ASP_CODEC_LUT_L_REG0_3_ADDR(base)                 ((base) + (0x01D8UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG0_3_UNION */
#define SOC_ASP_CODEC_LUT_R_REG0_3_ADDR(base)                 ((base) + (0x01DCUL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG4_7_UNION */
#define SOC_ASP_CODEC_LUT_L_REG4_7_ADDR(base)                 ((base) + (0x01E0UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG4_7_UNION */
#define SOC_ASP_CODEC_LUT_R_REG4_7_ADDR(base)                 ((base) + (0x01E4UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG8_11_UNION */
#define SOC_ASP_CODEC_LUT_L_REG8_11_ADDR(base)                ((base) + (0x01E8UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG8_11_UNION */
#define SOC_ASP_CODEC_LUT_R_REG8_11_ADDR(base)                ((base) + (0x01ECUL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG12_15_UNION */
#define SOC_ASP_CODEC_LUT_L_REG12_15_ADDR(base)               ((base) + (0x01F0UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG12_15_UNION */
#define SOC_ASP_CODEC_LUT_R_REG12_15_ADDR(base)               ((base) + (0x01F4UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG16_19_UNION */
#define SOC_ASP_CODEC_LUT_L_REG16_19_ADDR(base)               ((base) + (0x01F8UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG16_19_UNION */
#define SOC_ASP_CODEC_LUT_R_REG16_19_ADDR(base)               ((base) + (0x01FCUL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG20_23_UNION */
#define SOC_ASP_CODEC_LUT_L_REG20_23_ADDR(base)               ((base) + (0x0200UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG20_23_UNION */
#define SOC_ASP_CODEC_LUT_R_REG20_23_ADDR(base)               ((base) + (0x0204UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG24_27_UNION */
#define SOC_ASP_CODEC_LUT_L_REG24_27_ADDR(base)               ((base) + (0x0208UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG24_27_UNION */
#define SOC_ASP_CODEC_LUT_R_REG24_27_ADDR(base)               ((base) + (0x020CUL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG28_31_UNION */
#define SOC_ASP_CODEC_LUT_L_REG28_31_ADDR(base)               ((base) + (0x0210UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG28_31_UNION */
#define SOC_ASP_CODEC_LUT_R_REG28_31_ADDR(base)               ((base) + (0x0214UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG32_35_UNION */
#define SOC_ASP_CODEC_LUT_L_REG32_35_ADDR(base)               ((base) + (0x0218UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG32_35_UNION */
#define SOC_ASP_CODEC_LUT_R_REG32_35_ADDR(base)               ((base) + (0x021CUL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG36_39_UNION */
#define SOC_ASP_CODEC_LUT_L_REG36_39_ADDR(base)               ((base) + (0x0220UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG36_39_UNION */
#define SOC_ASP_CODEC_LUT_R_REG36_39_ADDR(base)               ((base) + (0x0224UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG40_43_UNION */
#define SOC_ASP_CODEC_LUT_L_REG40_43_ADDR(base)               ((base) + (0x0228UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG40_43_UNION */
#define SOC_ASP_CODEC_LUT_R_REG40_43_ADDR(base)               ((base) + (0x022CUL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG44_47_UNION */
#define SOC_ASP_CODEC_LUT_L_REG44_47_ADDR(base)               ((base) + (0x0230UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG44_47_UNION */
#define SOC_ASP_CODEC_LUT_R_REG44_47_ADDR(base)               ((base) + (0x0234UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG48_51_UNION */
#define SOC_ASP_CODEC_LUT_L_REG48_51_ADDR(base)               ((base) + (0x0238UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG48_51_UNION */
#define SOC_ASP_CODEC_LUT_R_REG48_51_ADDR(base)               ((base) + (0x023CUL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG52_55_UNION */
#define SOC_ASP_CODEC_LUT_L_REG52_55_ADDR(base)               ((base) + (0x0240UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG52_55_UNION */
#define SOC_ASP_CODEC_LUT_R_REG52_55_ADDR(base)               ((base) + (0x0244UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG56_59_UNION */
#define SOC_ASP_CODEC_LUT_L_REG56_59_ADDR(base)               ((base) + (0x0248UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG56_59_UNION */
#define SOC_ASP_CODEC_LUT_R_REG56_59_ADDR(base)               ((base) + (0x024CUL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG60_63_UNION */
#define SOC_ASP_CODEC_LUT_L_REG60_63_ADDR(base)               ((base) + (0x0250UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG60_63_UNION */
#define SOC_ASP_CODEC_LUT_R_REG60_63_ADDR(base)               ((base) + (0x0254UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG64_67_UNION */
#define SOC_ASP_CODEC_LUT_L_REG64_67_ADDR(base)               ((base) + (0x0258UL))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG64_67_UNION */
#define SOC_ASP_CODEC_LUT_R_REG64_67_ADDR(base)               ((base) + (0x025CUL))

/* 寄存器说明：ADC_SPAI1通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC_SPI1_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC_SPI1_DC_OFFSET_ADDR(base)     ((base) + (0x0260UL))

/* 寄存器说明：ADC_SPAI2通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC_SPI2_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC_SPI2_DC_OFFSET_ADDR(base)     ((base) + (0x0264UL))

/* 寄存器说明：ADC_SPAV1通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC_SPA1_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC_SPA1_DC_OFFSET_ADDR(base)     ((base) + (0x0268UL))

/* 寄存器说明：ADC_SPAV2通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC_SPA2_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC_SPA2_DC_OFFSET_ADDR(base)     ((base) + (0x026CUL))

/* 寄存器说明：I2S1_TDM接口控制寄存器3
   位域定义UNION结构:  SOC_ASP_CODEC_I2S1_TDM_CTRL2_UNION */
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_ADDR(base)               ((base) + (0x0270UL))

/* 寄存器说明：I2S1_TDM接口控制寄存器2
   位域定义UNION结构:  SOC_ASP_CODEC_I2S1_TDM_CTRL3_UNION */
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_ADDR(base)               ((base) + (0x0274UL))

/* 寄存器说明：I2S1_TDM接口控制寄存器4
   位域定义UNION结构:  SOC_ASP_CODEC_I2S1_TDM_CTRL4_UNION */
#define SOC_ASP_CODEC_I2S1_TDM_CTRL4_ADDR(base)               ((base) + (0x0278UL))

/* 寄存器说明：PGA GAINOFFSET配置寄存器6
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL6_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL6_ADDR(base)         ((base) + (0x027CUL))

/* 寄存器说明：ULTRA_L_DN PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ADDR(base)          ((base) + (0x0280UL))

/* 寄存器说明：ULTRA_R_DN PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ADDR(base)          ((base) + (0x0284UL))

/* 寄存器说明：SRCUP控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_DAC_SRCUP_CTRL_UNION */
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_ADDR(base)               ((base) + (0x0288UL))

/* 寄存器说明：6阶cic选择滤波器
   位域定义UNION结构:  SOC_ASP_CODEC_CIC_6STAGE_SEL_UNION */
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_ADDR(base)               ((base) + (0x028CUL))


#else


/* 寄存器说明：CODEC版本寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_VERSION_UNION */
#define SOC_ASP_CODEC_VERSION_ADDR(base)                      ((base) + (0x0000))

/* 寄存器说明：CODEC子模块时钟门控寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_CLK_EN0_UNION */
#define SOC_ASP_CODEC_CODEC_CLK_EN0_ADDR(base)                ((base) + (0x0004))

/* 寄存器说明：CODEC子模块时钟门控寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_CLK_EN1_UNION */
#define SOC_ASP_CODEC_CODEC_CLK_EN1_ADDR(base)                ((base) + (0x0008))

/* 寄存器说明：CODEC子系统门控使能寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_CLK_EN2_UNION */
#define SOC_ASP_CODEC_CODEC_CLK_EN2_ADDR(base)                ((base) + (0x000C))

/* 寄存器说明：CODEC的复位寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_SW_RST_N_UNION */
#define SOC_ASP_CODEC_CODEC_SW_RST_N_ADDR(base)               ((base) + (0x0010))

/* 寄存器说明：I2S1_PCM接口控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S1_CTRL_UNION */
#define SOC_ASP_CODEC_I2S1_CTRL_ADDR(base)                    ((base) + (0x0014))

/* 寄存器说明：I2S1_TDM接口控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_I2S1_TDM_CTRL0_UNION */
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_ADDR(base)               ((base) + (0x0018))

/* 寄存器说明：I2S1_TDM接口控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_I2S1_TDM_CTRL1_UNION */
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_ADDR(base)               ((base) + (0x001C))

/* 寄存器说明：I2S2_PCM接口控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S2_PCM_CTRL_UNION */
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_ADDR(base)                ((base) + (0x0020))

/* 寄存器说明：I2S3_PCM接口控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S3_PCM_CTRL_UNION */
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_ADDR(base)                ((base) + (0x0024))

/* 寄存器说明：I2S4_PCM接口控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S4_CTRL_UNION */
#define SOC_ASP_CODEC_I2S4_CTRL_ADDR(base)                    ((base) + (0x0028))

/* 寄存器说明：I2S4_TDM接口控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_I2S4_TDM_CTRL0_UNION */
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_ADDR(base)               ((base) + (0x002C))

/* 寄存器说明：I2S4_TDM接口控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_I2S4_TDM_CTRL1_UNION */
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_ADDR(base)               ((base) + (0x0030))

/* 寄存器说明：PGA/MIXER阈值控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_THRE_CTRL0_UNION */
#define SOC_ASP_CODEC_PGA_THRE_CTRL0_ADDR(base)               ((base) + (0x0034))

/* 寄存器说明：PGA/MIXER阈值控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_THRE_CTRL1_UNION */
#define SOC_ASP_CODEC_PGA_THRE_CTRL1_ADDR(base)               ((base) + (0x0038))

/* 寄存器说明：PGA/MIXER阈值控制寄存器2
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_THRE_CTRL2_UNION */
#define SOC_ASP_CODEC_PGA_THRE_CTRL2_ADDR(base)               ((base) + (0x003C))

/* 寄存器说明：PGA GAINOFFSET配置寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_ADDR(base)         ((base) + (0x0040))

/* 寄存器说明：PGA GAINOFFSET配置寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_ADDR(base)         ((base) + (0x0044))

/* 寄存器说明：PGA GAINOFFSET配置寄存器2
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_ADDR(base)         ((base) + (0x0048))

/* 寄存器说明：PGA GAINOFFSET配置寄存器3
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL3_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL3_ADDR(base)         ((base) + (0x004C))

/* 寄存器说明：CODEC3_L_DN PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_ADDR(base)         ((base) + (0x0050))

/* 寄存器说明：CODEC3_R_DN PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_ADDR(base)         ((base) + (0x0054))

/* 寄存器说明：AUDIO下行通路左声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_ADDR(base)          ((base) + (0x0058))

/* 寄存器说明：AUDIO下行通路右声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_ADDR(base)          ((base) + (0x005C))

/* 寄存器说明：SIDETONE通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SIDETONE_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_ADDR(base)            ((base) + (0x0060))

/* 寄存器说明：AUDIO上行通路左声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_ADDR(base)          ((base) + (0x0064))

/* 寄存器说明：AUDIO上行通路右声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_ADDR(base)          ((base) + (0x0068))

/* 寄存器说明：VOICE上行通路左声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_ADDR(base)          ((base) + (0x006C))

/* 寄存器说明：VOICE上行通路右声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_ADDR(base)          ((base) + (0x0070))

/* 寄存器说明：MIC3上行PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0074))

/* 寄存器说明：MIC4上行PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0078))

/* 寄存器说明：MDM_5G上行通路左声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_ADDR(base)         ((base) + (0x007C))

/* 寄存器说明：MDM_5G上行通路右声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_ADDR(base)         ((base) + (0x0080))

/* 寄存器说明：I2S2 RX L PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_ADDR(base)           ((base) + (0x0084))

/* 寄存器说明：I2S2 RX R PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_ADDR(base)           ((base) + (0x0088))

/* 寄存器说明：0p5 PGA控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_UNION */
#define SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_ADDR(base)         ((base) + (0x008C))

/* 寄存器说明：0p5 PGA控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_UNION */
#define SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_ADDR(base)         ((base) + (0x0090))

/* 寄存器说明：ADC1上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0094))

/* 寄存器说明：ADC2上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0098))

/* 寄存器说明：ADC3上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_ADDR(base)             ((base) + (0x009C))

/* 寄存器说明：ADC4上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_ADDR(base)             ((base) + (0x00A0))

/* 寄存器说明：ADC5上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_ADDR(base)             ((base) + (0x00A4))

/* 寄存器说明：SRCUP控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SRCUP_CTRL_UNION */
#define SOC_ASP_CODEC_SRCUP_CTRL_ADDR(base)                   ((base) + (0x00A8))

/* 寄存器说明：SRCDN控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_SRCDN_CTRL0_UNION */
#define SOC_ASP_CODEC_SRCDN_CTRL0_ADDR(base)                  ((base) + (0x00AC))

/* 寄存器说明：SRCDN控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_SRCDN_CTRL1_UNION */
#define SOC_ASP_CODEC_SRCDN_CTRL1_ADDR(base)                  ((base) + (0x00B0))

/* 寄存器说明：DACL_MIXER4控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_DACL_MIXER4_CTRL0_UNION */
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_ADDR(base)            ((base) + (0x00B4))

/* 寄存器说明：DACL_MIXER4控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_DACL_MIXER4_CTRL1_UNION */
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL1_ADDR(base)            ((base) + (0x00B8))

/* 寄存器说明：DACR_MIXER4控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_DACR_MIXER4_CTRL0_UNION */
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_ADDR(base)            ((base) + (0x00BC))

/* 寄存器说明：DACR_MIXER4控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_DACR_MIXER4_CTRL1_UNION */
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL1_ADDR(base)            ((base) + (0x00C0))

/* 寄存器说明：I2S2MIXER2控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_UNION */
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_ADDR(base)          ((base) + (0x00C4))

/* 寄存器说明：CODEC的使能控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_CLK_EN3_UNION */
#define SOC_ASP_CODEC_CODEC_CLK_EN3_ADDR(base)                ((base) + (0x00C8))

/* 寄存器说明：CODEC的使能控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_CLK_EN4_UNION */
#define SOC_ASP_CODEC_CODEC_CLK_EN4_ADDR(base)                ((base) + (0x00CC))

/* 寄存器说明：ADC_FILTER1控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC_FILTER_CTRL0_UNION */
#define SOC_ASP_CODEC_ADC_FILTER_CTRL0_ADDR(base)             ((base) + (0x00D0))

/* 寄存器说明：ADC_FILTER控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC_FILTER_CTRL_UNION */
#define SOC_ASP_CODEC_ADC_FILTER_CTRL_ADDR(base)              ((base) + (0x00D4))

/* 寄存器说明：DMIC_CTRL控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_DMIC_CTRL_UNION */
#define SOC_ASP_CODEC_DMIC_CTRL_ADDR(base)                    ((base) + (0x00D8))

/* 寄存器说明：DMIC_DIV分频控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_DMIC_DIV_UNION */
#define SOC_ASP_CODEC_DMIC_DIV_ADDR(base)                     ((base) + (0x00DC))

/* 寄存器说明：SPA反馈通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x00E0))

/* 寄存器说明：SPA反馈通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x00E4))

/* 寄存器说明：CODEC3下行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_ADDR(base)         ((base) + (0x00E8))

/* 寄存器说明：AUDIO下行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_ADDR(base)          ((base) + (0x00EC))

/* 寄存器说明：ULTR下行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ADDR(base)           ((base) + (0x00F0))

/* 寄存器说明：SPA反馈通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x00F4))

/* 寄存器说明：AUDIO上行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x00F8))

/* 寄存器说明：VOICE上行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x00FC))

/* 寄存器说明：MIC34上行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x0100))

/* 寄存器说明：5GMDM下行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_ADDR(base)         ((base) + (0x0104))

/* 寄存器说明：5GMDM上行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_ADDR(base)         ((base) + (0x0108))

/* 寄存器说明：SPA反馈通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x010C))

/* 寄存器说明：SIF_CTRL控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SIF_CTRL_UNION */
#define SOC_ASP_CODEC_SIF_CTRL_ADDR(base)                     ((base) + (0x0110))

/* 寄存器说明：CODEC通道内模块状态查询寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_STATE_UNION */
#define SOC_ASP_CODEC_CODEC_STATE_ADDR(base)                  ((base) + (0x0114))

/* 寄存器说明：CODEC内部模块采样率控制寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL0_UNION */
#define SOC_ASP_CODEC_FS_CTRL0_ADDR(base)                     ((base) + (0x0118))

/* 寄存器说明：CODEC内部模块采样率控制寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL1_UNION */
#define SOC_ASP_CODEC_FS_CTRL1_ADDR(base)                     ((base) + (0x011C))

/* 寄存器说明：CODEC内部模块采样率控制寄存器2
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL2_UNION */
#define SOC_ASP_CODEC_FS_CTRL2_ADDR(base)                     ((base) + (0x0120))

/* 寄存器说明：CODEC内部模块采样率控制寄存器3
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL3_UNION */
#define SOC_ASP_CODEC_FS_CTRL3_ADDR(base)                     ((base) + (0x0124))

/* 寄存器说明：CODEC内部模块采样率控制寄存器4
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL4_UNION */
#define SOC_ASP_CODEC_FS_CTRL4_ADDR(base)                     ((base) + (0x0128))

/* 寄存器说明：CODEC内部模块采样率控制寄存器5
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL5_UNION */
#define SOC_ASP_CODEC_FS_CTRL5_ADDR(base)                     ((base) + (0x012C))

/* 寄存器说明：CODEC内部模块采样率控制寄存器6
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL6_UNION */
#define SOC_ASP_CODEC_FS_CTRL6_ADDR(base)                     ((base) + (0x0130))

/* 寄存器说明：CODEC内部模块采样率控制寄存器7
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL7_UNION */
#define SOC_ASP_CODEC_FS_CTRL7_ADDR(base)                     ((base) + (0x0134))

/* 寄存器说明：CODEC内部模块输入数据选择寄存器0
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_DIN_MUX0_UNION */
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_ADDR(base)               ((base) + (0x0138))

/* 寄存器说明：CODEC内部模块输入数据选择寄存器1
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_DIN_MUX1_UNION */
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_ADDR(base)               ((base) + (0x013C))

/* 寄存器说明：ADC1通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC1_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC1_DC_OFFSET_ADDR(base)         ((base) + (0x0140))

/* 寄存器说明：ADC2通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC2_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC2_DC_OFFSET_ADDR(base)         ((base) + (0x0144))

/* 寄存器说明：ADC3通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC3_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC3_DC_OFFSET_ADDR(base)         ((base) + (0x0148))

/* 寄存器说明：ADC4通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC4_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC4_DC_OFFSET_ADDR(base)         ((base) + (0x0150))

/* 寄存器说明：ADC5通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC5_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC5_DC_OFFSET_ADDR(base)         ((base) + (0x0154))

/* 寄存器说明：单口memory的控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MEM_CTRL_S_UNION */
#define SOC_ASP_CODEC_MEM_CTRL_S_ADDR(base)                   ((base) + (0x0158))

/* 寄存器说明：双口memory的控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MEM_CTRL_1W2R_UNION */
#define SOC_ASP_CODEC_MEM_CTRL_1W2R_ADDR(base)                ((base) + (0x015C))

/* 寄存器说明：HPF滤波器的bypass控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC_CTRL_HPF_UNION */
#define SOC_ASP_CODEC_ADC_CTRL_HPF_ADDR(base)                 ((base) + (0x0160))

/* 寄存器说明：PGA GAINOFFSET配置寄存器4
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_ADDR(base)         ((base) + (0x0164))

/* 寄存器说明：MIC8上行PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0168))

/* 寄存器说明：MIC7上行PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_ADDR(base)             ((base) + (0x016C))

/* 寄存器说明：MIC6上行PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0170))

/* 寄存器说明：MIC5上行PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0174))

/* 寄存器说明：ADC6上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0178))

/* 寄存器说明：ADC7上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_ADDR(base)             ((base) + (0x017C))

/* 寄存器说明：ADC6上行通路PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_ADDR(base)             ((base) + (0x0180))

/* 寄存器说明：MIC56上行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x0184))

/* 寄存器说明：MIC78上行通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_ADDR(base)          ((base) + (0x0188))

/* 寄存器说明：CODEC内部模块采样率控制寄存器8
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL8_UNION */
#define SOC_ASP_CODEC_FS_CTRL8_ADDR(base)                     ((base) + (0x018C))

/* 寄存器说明：CODEC内部模块采样率控制寄存器9
   位域定义UNION结构:  SOC_ASP_CODEC_FS_CTRL9_UNION */
#define SOC_ASP_CODEC_FS_CTRL9_ADDR(base)                     ((base) + (0x0190))

/* 寄存器说明：ADC6通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC6_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC6_DC_OFFSET_ADDR(base)         ((base) + (0x0194))

/* 寄存器说明：ADC7通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC7_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC7_DC_OFFSET_ADDR(base)         ((base) + (0x0198))

/* 寄存器说明：ADC8通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC8_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC8_DC_OFFSET_ADDR(base)         ((base) + (0x019C))

/* 寄存器说明：PGA GAINOFFSET配置寄存器2
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL5_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL5_ADDR(base)         ((base) + (0x01A0))

/* 寄存器说明：MDM_5G下行通路左声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_ADDR(base)         ((base) + (0x01A4))

/* 寄存器说明：MDM_5G下行通路右声道PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_ADDR(base)         ((base) + (0x01A8))

/* 寄存器说明：SPA反馈通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_ADDR(base)         ((base) + (0x01AC))

/* 寄存器说明：DACL通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_DACL_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_DACL_DC_OFFSET_ADDR(base)         ((base) + (0x01B0))

/* 寄存器说明：SPA反馈通路AFIFO控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_UNION */
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_ADDR(base)         ((base) + (0x01B4))

/* 寄存器说明：DACL通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_DACR_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_DACR_DC_OFFSET_ADDR(base)         ((base) + (0x01B8))

/* 寄存器说明：spa的adc控制信号
   位域定义UNION结构:  SOC_ASP_CODEC_SPAIV1_ADC_FILTER_UNION */
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_ADDR(base)            ((base) + (0x01BC))

/* 寄存器说明：classh的控制信号
   位域定义UNION结构:  SOC_ASP_CODEC_CLASSH_L_CTRL1_UNION */
#define SOC_ASP_CODEC_CLASSH_L_CTRL1_ADDR(base)               ((base) + (0x01C0))

/* 寄存器说明：DAC_FILTER控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_DAC_FILTER_CTRL_UNION */
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_ADDR(base)              ((base) + (0x01C4))

/* 寄存器说明：classh的控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CLASSH_L_CTRL2_UNION */
#define SOC_ASP_CODEC_CLASSH_L_CTRL2_ADDR(base)               ((base) + (0x01C8))

/* 寄存器说明：spa的adc控制信号
   位域定义UNION结构:  SOC_ASP_CODEC_SPAIV2_ADC_FILTER_UNION */
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_ADDR(base)            ((base) + (0x01CC))

/* 寄存器说明：classh的控制信号
   位域定义UNION结构:  SOC_ASP_CODEC_CLASSH_R_CTRL1_UNION */
#define SOC_ASP_CODEC_CLASSH_R_CTRL1_ADDR(base)               ((base) + (0x01D0))

/* 寄存器说明：classh的控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CLASSH_R_CTRL2_UNION */
#define SOC_ASP_CODEC_CLASSH_R_CTRL2_ADDR(base)               ((base) + (0x01D4))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG0_3_UNION */
#define SOC_ASP_CODEC_LUT_L_REG0_3_ADDR(base)                 ((base) + (0x01D8))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG0_3_UNION */
#define SOC_ASP_CODEC_LUT_R_REG0_3_ADDR(base)                 ((base) + (0x01DC))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG4_7_UNION */
#define SOC_ASP_CODEC_LUT_L_REG4_7_ADDR(base)                 ((base) + (0x01E0))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG4_7_UNION */
#define SOC_ASP_CODEC_LUT_R_REG4_7_ADDR(base)                 ((base) + (0x01E4))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG8_11_UNION */
#define SOC_ASP_CODEC_LUT_L_REG8_11_ADDR(base)                ((base) + (0x01E8))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG8_11_UNION */
#define SOC_ASP_CODEC_LUT_R_REG8_11_ADDR(base)                ((base) + (0x01EC))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG12_15_UNION */
#define SOC_ASP_CODEC_LUT_L_REG12_15_ADDR(base)               ((base) + (0x01F0))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG12_15_UNION */
#define SOC_ASP_CODEC_LUT_R_REG12_15_ADDR(base)               ((base) + (0x01F4))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG16_19_UNION */
#define SOC_ASP_CODEC_LUT_L_REG16_19_ADDR(base)               ((base) + (0x01F8))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG16_19_UNION */
#define SOC_ASP_CODEC_LUT_R_REG16_19_ADDR(base)               ((base) + (0x01FC))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG20_23_UNION */
#define SOC_ASP_CODEC_LUT_L_REG20_23_ADDR(base)               ((base) + (0x0200))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG20_23_UNION */
#define SOC_ASP_CODEC_LUT_R_REG20_23_ADDR(base)               ((base) + (0x0204))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG24_27_UNION */
#define SOC_ASP_CODEC_LUT_L_REG24_27_ADDR(base)               ((base) + (0x0208))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG24_27_UNION */
#define SOC_ASP_CODEC_LUT_R_REG24_27_ADDR(base)               ((base) + (0x020C))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG28_31_UNION */
#define SOC_ASP_CODEC_LUT_L_REG28_31_ADDR(base)               ((base) + (0x0210))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG28_31_UNION */
#define SOC_ASP_CODEC_LUT_R_REG28_31_ADDR(base)               ((base) + (0x0214))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG32_35_UNION */
#define SOC_ASP_CODEC_LUT_L_REG32_35_ADDR(base)               ((base) + (0x0218))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG32_35_UNION */
#define SOC_ASP_CODEC_LUT_R_REG32_35_ADDR(base)               ((base) + (0x021C))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG36_39_UNION */
#define SOC_ASP_CODEC_LUT_L_REG36_39_ADDR(base)               ((base) + (0x0220))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG36_39_UNION */
#define SOC_ASP_CODEC_LUT_R_REG36_39_ADDR(base)               ((base) + (0x0224))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG40_43_UNION */
#define SOC_ASP_CODEC_LUT_L_REG40_43_ADDR(base)               ((base) + (0x0228))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG40_43_UNION */
#define SOC_ASP_CODEC_LUT_R_REG40_43_ADDR(base)               ((base) + (0x022C))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG44_47_UNION */
#define SOC_ASP_CODEC_LUT_L_REG44_47_ADDR(base)               ((base) + (0x0230))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG44_47_UNION */
#define SOC_ASP_CODEC_LUT_R_REG44_47_ADDR(base)               ((base) + (0x0234))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG48_51_UNION */
#define SOC_ASP_CODEC_LUT_L_REG48_51_ADDR(base)               ((base) + (0x0238))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG48_51_UNION */
#define SOC_ASP_CODEC_LUT_R_REG48_51_ADDR(base)               ((base) + (0x023C))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG52_55_UNION */
#define SOC_ASP_CODEC_LUT_L_REG52_55_ADDR(base)               ((base) + (0x0240))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG52_55_UNION */
#define SOC_ASP_CODEC_LUT_R_REG52_55_ADDR(base)               ((base) + (0x0244))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG56_59_UNION */
#define SOC_ASP_CODEC_LUT_L_REG56_59_ADDR(base)               ((base) + (0x0248))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG56_59_UNION */
#define SOC_ASP_CODEC_LUT_R_REG56_59_ADDR(base)               ((base) + (0x024C))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG60_63_UNION */
#define SOC_ASP_CODEC_LUT_L_REG60_63_ADDR(base)               ((base) + (0x0250))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG60_63_UNION */
#define SOC_ASP_CODEC_LUT_R_REG60_63_ADDR(base)               ((base) + (0x0254))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_L_REG64_67_UNION */
#define SOC_ASP_CODEC_LUT_L_REG64_67_ADDR(base)               ((base) + (0x0258))

/* 寄存器说明：LUT的查找表寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_LUT_R_REG64_67_UNION */
#define SOC_ASP_CODEC_LUT_R_REG64_67_ADDR(base)               ((base) + (0x025C))

/* 寄存器说明：ADC_SPAI1通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC_SPI1_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC_SPI1_DC_OFFSET_ADDR(base)     ((base) + (0x0260))

/* 寄存器说明：ADC_SPAI2通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC_SPI2_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC_SPI2_DC_OFFSET_ADDR(base)     ((base) + (0x0264))

/* 寄存器说明：ADC_SPAV1通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC_SPA1_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC_SPA1_DC_OFFSET_ADDR(base)     ((base) + (0x0268))

/* 寄存器说明：ADC_SPAV2通路DC_OFFSET配置寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_CODEC_ADC_SPA2_DC_OFFSET_UNION */
#define SOC_ASP_CODEC_CODEC_ADC_SPA2_DC_OFFSET_ADDR(base)     ((base) + (0x026C))

/* 寄存器说明：I2S1_TDM接口控制寄存器3
   位域定义UNION结构:  SOC_ASP_CODEC_I2S1_TDM_CTRL2_UNION */
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_ADDR(base)               ((base) + (0x0270))

/* 寄存器说明：I2S1_TDM接口控制寄存器2
   位域定义UNION结构:  SOC_ASP_CODEC_I2S1_TDM_CTRL3_UNION */
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_ADDR(base)               ((base) + (0x0274))

/* 寄存器说明：I2S1_TDM接口控制寄存器4
   位域定义UNION结构:  SOC_ASP_CODEC_I2S1_TDM_CTRL4_UNION */
#define SOC_ASP_CODEC_I2S1_TDM_CTRL4_ADDR(base)               ((base) + (0x0278))

/* 寄存器说明：PGA GAINOFFSET配置寄存器6
   位域定义UNION结构:  SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL6_UNION */
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL6_ADDR(base)         ((base) + (0x027C))

/* 寄存器说明：ULTRA_L_DN PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ADDR(base)          ((base) + (0x0280))

/* 寄存器说明：ULTRA_R_DN PGA控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_UNION */
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ADDR(base)          ((base) + (0x0284))

/* 寄存器说明：SRCUP控制寄存器
   位域定义UNION结构:  SOC_ASP_CODEC_DAC_SRCUP_CTRL_UNION */
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_ADDR(base)               ((base) + (0x0288))

/* 寄存器说明：6阶cic选择滤波器
   位域定义UNION结构:  SOC_ASP_CODEC_CIC_6STAGE_SEL_UNION */
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_ADDR(base)               ((base) + (0x028C))


#endif


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 消息头定义
*****************************************************************************/


/*****************************************************************************
  5 消息定义
*****************************************************************************/


/*****************************************************************************
  6 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  7 UNION定义
*****************************************************************************/

/****************************************************************************
                     (1/1) SOC CODEC
 ****************************************************************************/
/*****************************************************************************
 结构名    : SOC_ASP_CODEC_VERSION_UNION
 结构说明  : VERSION 寄存器结构定义。地址偏移量:0x0000，宽度:32
 寄存器说明: CODEC版本寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  version : 32; /* bit[0-31]: Codec版本寄存器 */
    } reg;
} SOC_ASP_CODEC_VERSION_UNION;
#endif
#define SOC_ASP_CODEC_VERSION_version_START  (0)
#define SOC_ASP_CODEC_VERSION_version_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_CLK_EN0_UNION
 结构说明  : CODEC_CLK_EN0 寄存器结构定义。地址偏移量:0x0004，初值:0x00000000，宽度:32
 寄存器说明: CODEC子模块时钟门控寄存器0
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc1_clken              : 1;  /* bit[0] : adc1的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  adc2_clken              : 1;  /* bit[1] : adc2的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  adc3_clken              : 1;  /* bit[2] : adc3的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  adc4_clken              : 1;  /* bit[3] : adc4的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  codec3_l_dn_afifo_clken : 1;  /* bit[4] : codec3下行通路左声道FIFO时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  codec3_r_dn_afifo_clken : 1;  /* bit[5] : codec3下行通路右声道FIFO时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  audio_l_dn_afifo_clken  : 1;  /* bit[6] : audio下行通路左声道FIFO时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  audio_r_dn_afifo_clken  : 1;  /* bit[7] : audio下行通路右声道FIFO时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  ultr_l_dn_afifo_clken   : 1;  /* bit[8] : ultr下行通路左声道FIFO时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  ultr_r_dn_afifo_clken   : 1;  /* bit[9] : ultr下行通路右声道FIFO时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  reserved                : 1;  /* bit[10]: reserved */
        unsigned int  audio_l_up_afifo_clken  : 1;  /* bit[11]: audio上行通路左声道FIFO时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  audio_r_up_afifo_clken  : 1;  /* bit[12]: audio上行通路右声道FIFO时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  voice_l_up_afifo_clken  : 1;  /* bit[13]: voice上行通路左声道FIFO时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  voice_r_up_afifo_clken  : 1;  /* bit[14]: voice上行通路右声道FIFO时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  mic3_up_afifo_clken     : 1;  /* bit[15]: mic3上行FIFO的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  mic4_up_afifo_clken     : 1;  /* bit[16]: mic4上行FIFO的时钟控制
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  mdm_5g_l_up_afifo_clken : 1;  /* bit[17]: mdm_5g_l_up上行FIFO的时钟控制
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  mdm_5g_r_up_afifo_clken : 1;  /* bit[18]: mdm_5g_r_up上行FIFO的时钟控制
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  mdm_5g_l_dn_afifo_clken : 1;  /* bit[19]: mdm_5g_l_dn下行FIFO的时钟控制
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  mdm_5g_r_dn_afifo_clken : 1;  /* bit[20]: mdm_5g_r_dn下行FIFO的时钟控制
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  ultra_dn_l_pga_clken    : 1;  /* bit[21]: ultra_dn_l PGA的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  ultra_dn_r_pga_clken    : 1;  /* bit[22]: ultra_dn_r PGA的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  codec3_dn_l_pga_clken   : 1;  /* bit[23]: codec3_dn_l PGA的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  codec3_dn_r_pga_clken   : 1;  /* bit[24]: codec3_dn_r PGA的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  audio_dn_l_pga_clken    : 1;  /* bit[25]: audio_dn_l PGA的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  audio_dn_r_pga_clken    : 1;  /* bit[26]: audio_dn_r PGA的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  audio_up_l_pga_clken    : 1;  /* bit[27]: audio_up_l PGA的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  audio_up_r_pga_clken    : 1;  /* bit[28]: audio_up_r PGA的时钟控制。。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  voice_up_l_pga_clken    : 1;  /* bit[29]: voice_up_l PGA的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  voice_up_r_pga_clken    : 1;  /* bit[30]: voice_up_r PGA的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
        unsigned int  mic3_up_pga_clken       : 1;  /* bit[31]: mic3_up PGA的时钟控制。
                                                                0：关闭时钟；
                                                                1：打开时钟。 */
    } reg;
} SOC_ASP_CODEC_CODEC_CLK_EN0_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_CLK_EN0_adc1_clken_START               (0)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_adc1_clken_END                 (0)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_adc2_clken_START               (1)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_adc2_clken_END                 (1)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_adc3_clken_START               (2)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_adc3_clken_END                 (2)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_adc4_clken_START               (3)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_adc4_clken_END                 (3)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_codec3_l_dn_afifo_clken_START  (4)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_codec3_l_dn_afifo_clken_END    (4)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_codec3_r_dn_afifo_clken_START  (5)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_codec3_r_dn_afifo_clken_END    (5)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_l_dn_afifo_clken_START   (6)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_l_dn_afifo_clken_END     (6)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_r_dn_afifo_clken_START   (7)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_r_dn_afifo_clken_END     (7)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_ultr_l_dn_afifo_clken_START    (8)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_ultr_l_dn_afifo_clken_END      (8)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_ultr_r_dn_afifo_clken_START    (9)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_ultr_r_dn_afifo_clken_END      (9)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_l_up_afifo_clken_START   (11)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_l_up_afifo_clken_END     (11)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_r_up_afifo_clken_START   (12)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_r_up_afifo_clken_END     (12)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_voice_l_up_afifo_clken_START   (13)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_voice_l_up_afifo_clken_END     (13)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_voice_r_up_afifo_clken_START   (14)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_voice_r_up_afifo_clken_END     (14)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mic3_up_afifo_clken_START      (15)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mic3_up_afifo_clken_END        (15)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mic4_up_afifo_clken_START      (16)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mic4_up_afifo_clken_END        (16)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mdm_5g_l_up_afifo_clken_START  (17)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mdm_5g_l_up_afifo_clken_END    (17)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mdm_5g_r_up_afifo_clken_START  (18)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mdm_5g_r_up_afifo_clken_END    (18)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mdm_5g_l_dn_afifo_clken_START  (19)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mdm_5g_l_dn_afifo_clken_END    (19)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mdm_5g_r_dn_afifo_clken_START  (20)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mdm_5g_r_dn_afifo_clken_END    (20)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_ultra_dn_l_pga_clken_START     (21)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_ultra_dn_l_pga_clken_END       (21)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_ultra_dn_r_pga_clken_START     (22)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_ultra_dn_r_pga_clken_END       (22)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_codec3_dn_l_pga_clken_START    (23)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_codec3_dn_l_pga_clken_END      (23)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_codec3_dn_r_pga_clken_START    (24)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_codec3_dn_r_pga_clken_END      (24)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_dn_l_pga_clken_START     (25)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_dn_l_pga_clken_END       (25)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_dn_r_pga_clken_START     (26)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_dn_r_pga_clken_END       (26)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_up_l_pga_clken_START     (27)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_up_l_pga_clken_END       (27)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_up_r_pga_clken_START     (28)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_audio_up_r_pga_clken_END       (28)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_voice_up_l_pga_clken_START     (29)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_voice_up_l_pga_clken_END       (29)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_voice_up_r_pga_clken_START     (30)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_voice_up_r_pga_clken_END       (30)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mic3_up_pga_clken_START        (31)
#define SOC_ASP_CODEC_CODEC_CLK_EN0_mic3_up_pga_clken_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_CLK_EN1_UNION
 结构说明  : CODEC_CLK_EN1 寄存器结构定义。地址偏移量:0x0008，初值:0x00000000，宽度:32
 寄存器说明: CODEC子模块时钟门控寄存器1
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  mic4_up_pga_clken       : 1;  /* bit[0]    : mic4上行通路PGA时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  mdm_5g_l_up_pga_clken   : 1;  /* bit[1]    : mdm_5g_l 上行通路PGA时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  mdm_5g_r_up_pga_clken   : 1;  /* bit[2]    : mdm_5g_r 上行通路PGA时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  i2s2_rx_l_pga_clken     : 1;  /* bit[3]    : i2s2 上行左声道通路PGA时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  i2s2_rx_r_pga_clken     : 1;  /* bit[4]    : i2s2 上行右声道通路PGA时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  sidetone_pga_clken      : 1;  /* bit[5]    : sidetone通路PGA时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  adc1_0p5_pga_clken      : 1;  /* bit[6]    : adc1通路0.5 PGA时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  adc2_0p5_pga_clken      : 1;  /* bit[7]    : adc2通路0.5 PGA时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  adc3_0p5_pga_clken      : 1;  /* bit[8]    : adc3通路0.5 PGA时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  adc4_0p5_pga_clken      : 1;  /* bit[9]    : adc4通路0.5 PGA时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  adc5_0p5_pga_clken      : 1;  /* bit[10]   : adc5通路0.5 PGA时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  reserved_0              : 2;  /* bit[11-12]: reserved */
        unsigned int  codec3_dn_l_srcup_clken : 1;  /* bit[13]   : codec3的左声道下行srcup时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  codec3_dn_r_srcup_clken : 1;  /* bit[14]   : codec3的右声道下行srcup时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  audio_dn_l_srcup_clken  : 1;  /* bit[15]   : audio的右声道下行srcup时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  audio_dn_r_srcup_clken  : 1;  /* bit[16]   : audio的右声道下行srcup时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  i2s2_rx_l_srcup_clken   : 1;  /* bit[17]   : i2s2的左声道上行srcup时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  i2s2_rx_r_srcup_clken   : 1;  /* bit[18]   : i2s2的右声道上行srcup时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  dacl_mixer4_srcup_clken : 1;  /* bit[19]   : dacl_mixer4的左声道下行srcup时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  dacr_mixer4_srcup_clken : 1;  /* bit[20]   : dacl_mixer4的左声道下行srcup时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  audio_up_l_srcdn_clken  : 1;  /* bit[21]   : audio_up_l的srcdn时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  audio_up_r_srcdn_clken  : 1;  /* bit[22]   : audio_up_r的srcdn时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  voice_up_l_srcdn_clken  : 1;  /* bit[23]   : voice_up_l的srcdn时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  voice_up_r_srcdn_clken  : 1;  /* bit[24]   : voice_up_r的srcdn时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  mic3_up_srcdn_clken     : 1;  /* bit[25]   : mic3的上行srcdn时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  mic4_up_srcdn_clken     : 1;  /* bit[26]   : mic4的上行srcdn时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  mdm_5g_up_l_srcdn_clken : 1;  /* bit[27]   : 5G mdm的上行左声道srcdn时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  mdm_5g_up_r_srcdn_clken : 1;  /* bit[28]   : 5G mdm的上行右声道srcdn时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  i2s2_rx_l_srcdn_clken   : 1;  /* bit[29]   : i2s2的rx_l srcdn时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  i2s2_rx_r_srcdn_clken   : 1;  /* bit[30]   : i2s2的rx_r srcdn时钟控制。
                                                                   0：关闭时钟；
                                                                   1：打开时钟。 */
        unsigned int  reserved_1              : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_CLK_EN1_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mic4_up_pga_clken_START        (0)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mic4_up_pga_clken_END          (0)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mdm_5g_l_up_pga_clken_START    (1)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mdm_5g_l_up_pga_clken_END      (1)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mdm_5g_r_up_pga_clken_START    (2)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mdm_5g_r_up_pga_clken_END      (2)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_i2s2_rx_l_pga_clken_START      (3)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_i2s2_rx_l_pga_clken_END        (3)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_i2s2_rx_r_pga_clken_START      (4)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_i2s2_rx_r_pga_clken_END        (4)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_sidetone_pga_clken_START       (5)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_sidetone_pga_clken_END         (5)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_adc1_0p5_pga_clken_START       (6)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_adc1_0p5_pga_clken_END         (6)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_adc2_0p5_pga_clken_START       (7)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_adc2_0p5_pga_clken_END         (7)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_adc3_0p5_pga_clken_START       (8)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_adc3_0p5_pga_clken_END         (8)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_adc4_0p5_pga_clken_START       (9)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_adc4_0p5_pga_clken_END         (9)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_adc5_0p5_pga_clken_START       (10)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_adc5_0p5_pga_clken_END         (10)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_codec3_dn_l_srcup_clken_START  (13)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_codec3_dn_l_srcup_clken_END    (13)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_codec3_dn_r_srcup_clken_START  (14)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_codec3_dn_r_srcup_clken_END    (14)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_audio_dn_l_srcup_clken_START   (15)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_audio_dn_l_srcup_clken_END     (15)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_audio_dn_r_srcup_clken_START   (16)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_audio_dn_r_srcup_clken_END     (16)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_i2s2_rx_l_srcup_clken_START    (17)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_i2s2_rx_l_srcup_clken_END      (17)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_i2s2_rx_r_srcup_clken_START    (18)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_i2s2_rx_r_srcup_clken_END      (18)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_dacl_mixer4_srcup_clken_START  (19)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_dacl_mixer4_srcup_clken_END    (19)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_dacr_mixer4_srcup_clken_START  (20)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_dacr_mixer4_srcup_clken_END    (20)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_audio_up_l_srcdn_clken_START   (21)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_audio_up_l_srcdn_clken_END     (21)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_audio_up_r_srcdn_clken_START   (22)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_audio_up_r_srcdn_clken_END     (22)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_voice_up_l_srcdn_clken_START   (23)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_voice_up_l_srcdn_clken_END     (23)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_voice_up_r_srcdn_clken_START   (24)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_voice_up_r_srcdn_clken_END     (24)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mic3_up_srcdn_clken_START      (25)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mic3_up_srcdn_clken_END        (25)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mic4_up_srcdn_clken_START      (26)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mic4_up_srcdn_clken_END        (26)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mdm_5g_up_l_srcdn_clken_START  (27)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mdm_5g_up_l_srcdn_clken_END    (27)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mdm_5g_up_r_srcdn_clken_START  (28)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_mdm_5g_up_r_srcdn_clken_END    (28)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_i2s2_rx_l_srcdn_clken_START    (29)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_i2s2_rx_l_srcdn_clken_END      (29)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_i2s2_rx_r_srcdn_clken_START    (30)
#define SOC_ASP_CODEC_CODEC_CLK_EN1_i2s2_rx_r_srcdn_clken_END      (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_CLK_EN2_UNION
 结构说明  : CODEC_CLK_EN2 寄存器结构定义。地址偏移量:0x000C，初值:0x00000000，宽度:32
 寄存器说明: CODEC子系统门控使能寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  i2s2_tx_l_srcdn_clken : 1;  /* bit[0] : i2s2的tx_l通路srcdn时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  i2s2_tx_r_srcdn_clken : 1;  /* bit[1] : i2s2的tx_r通路srcdn时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  spa_5_up_afifo_clken  : 1;  /* bit[2] : SPA上行通路5声道FIFO时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  spa_6_up_afifo_clken  : 1;  /* bit[3] : SPA上行通路6声道FIFO时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  spa_7_up_afifo_clken  : 1;  /* bit[4] : SPA上行通路7声道FIFO时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  spa_8_up_afifo_clken  : 1;  /* bit[5] : SPA上行通路8声道FIFO时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  i2s1_tdm_clken        : 1;  /* bit[6] : i2s1的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  i2s2_pcm_clken        : 1;  /* bit[7] : i2s2的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  i2s3_pcm_clken        : 1;  /* bit[8] : i2s3的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  i2s4_tdm_clken        : 1;  /* bit[9] : i2s4的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  dacl_mixer4_clken     : 1;  /* bit[10]: dacl_mixer4的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  dacr_mixer4_clken     : 1;  /* bit[11]: dacr_mixer4的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  mdm_5g_r_dn_pga_clken : 1;  /* bit[12]: mdm_5g_dn_r PGA时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  mdm_5g_l_dn_pga_clken : 1;  /* bit[13]: mdm_5g_dn_l PGA时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  i2s2_tx_mixer2_clken  : 1;  /* bit[14]: i2s2_tx_mixer2的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  mic6_up_srcdn_clken   : 1;  /* bit[15]: mic6_up上行srcup时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  mic5_up_srcdn_clken   : 1;  /* bit[16]: mic5_up上行srcup时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  spa_1_up_afifo_clken  : 1;  /* bit[17]: SPA上行通路1声道FIFO时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  spa_2_up_afifo_clken  : 1;  /* bit[18]: SPA上行通路2声道FIFO时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  spa_3_up_afifo_clken  : 1;  /* bit[19]: SPA上行通路3声道FIFO时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  spa_4_up_afifo_clken  : 1;  /* bit[20]: SPA上行通路4声道FIFO时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  adc5_clken            : 1;  /* bit[21]: adc5的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  adc1_cic_d16_clken    : 1;  /* bit[22]: adc1_cic_d16的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  adc2_cic_d16_clken    : 1;  /* bit[23]: adc2_cic_d16的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  adc3_cic_d16_clken    : 1;  /* bit[24]: adc3_cic_d16的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  adc4_cic_d16_clken    : 1;  /* bit[25]: adc4_cic_d16的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  adc5_cic_d16_clken    : 1;  /* bit[26]: adc5_cic_d16的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  dmic1_clken           : 1;  /* bit[27]: dmic1的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  dmic2_clken           : 1;  /* bit[28]: dmic2的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  sif_49152k_clken      : 1;  /* bit[29]: sif_49152k的时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  mic8_up_srcdn_clken   : 1;  /* bit[30]: mic8_up上行srcup时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
        unsigned int  mic7_up_srcdn_clken   : 1;  /* bit[31]: mic7_up上行srcup时钟控制。
                                                              0：关闭时钟；
                                                              1：打开时钟。 */
    } reg;
} SOC_ASP_CODEC_CODEC_CLK_EN2_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s2_tx_l_srcdn_clken_START  (0)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s2_tx_l_srcdn_clken_END    (0)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s2_tx_r_srcdn_clken_START  (1)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s2_tx_r_srcdn_clken_END    (1)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_5_up_afifo_clken_START   (2)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_5_up_afifo_clken_END     (2)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_6_up_afifo_clken_START   (3)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_6_up_afifo_clken_END     (3)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_7_up_afifo_clken_START   (4)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_7_up_afifo_clken_END     (4)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_8_up_afifo_clken_START   (5)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_8_up_afifo_clken_END     (5)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s1_tdm_clken_START         (6)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s1_tdm_clken_END           (6)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s2_pcm_clken_START         (7)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s2_pcm_clken_END           (7)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s3_pcm_clken_START         (8)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s3_pcm_clken_END           (8)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s4_tdm_clken_START         (9)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s4_tdm_clken_END           (9)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_dacl_mixer4_clken_START      (10)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_dacl_mixer4_clken_END        (10)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_dacr_mixer4_clken_START      (11)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_dacr_mixer4_clken_END        (11)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_mdm_5g_r_dn_pga_clken_START  (12)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_mdm_5g_r_dn_pga_clken_END    (12)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_mdm_5g_l_dn_pga_clken_START  (13)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_mdm_5g_l_dn_pga_clken_END    (13)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s2_tx_mixer2_clken_START   (14)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_i2s2_tx_mixer2_clken_END     (14)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_mic6_up_srcdn_clken_START    (15)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_mic6_up_srcdn_clken_END      (15)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_mic5_up_srcdn_clken_START    (16)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_mic5_up_srcdn_clken_END      (16)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_1_up_afifo_clken_START   (17)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_1_up_afifo_clken_END     (17)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_2_up_afifo_clken_START   (18)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_2_up_afifo_clken_END     (18)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_3_up_afifo_clken_START   (19)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_3_up_afifo_clken_END     (19)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_4_up_afifo_clken_START   (20)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_spa_4_up_afifo_clken_END     (20)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_adc5_clken_START             (21)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_adc5_clken_END               (21)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_adc1_cic_d16_clken_START     (22)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_adc1_cic_d16_clken_END       (22)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_adc2_cic_d16_clken_START     (23)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_adc2_cic_d16_clken_END       (23)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_adc3_cic_d16_clken_START     (24)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_adc3_cic_d16_clken_END       (24)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_adc4_cic_d16_clken_START     (25)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_adc4_cic_d16_clken_END       (25)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_adc5_cic_d16_clken_START     (26)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_adc5_cic_d16_clken_END       (26)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_dmic1_clken_START            (27)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_dmic1_clken_END              (27)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_dmic2_clken_START            (28)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_dmic2_clken_END              (28)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_sif_49152k_clken_START       (29)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_sif_49152k_clken_END         (29)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_mic8_up_srcdn_clken_START    (30)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_mic8_up_srcdn_clken_END      (30)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_mic7_up_srcdn_clken_START    (31)
#define SOC_ASP_CODEC_CODEC_CLK_EN2_mic7_up_srcdn_clken_END      (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_SW_RST_N_UNION
 结构说明  : CODEC_SW_RST_N 寄存器结构定义。地址偏移量:0x0010，初值:0x00000001，宽度:32
 寄存器说明: CODEC的复位寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  codec_sw_rst_n            : 1;  /* bit[0]   : codec的软复位，低有效 */
        unsigned int  reserved_0                : 1;  /* bit[1]   : reserved */
        unsigned int  rst_4mic_voice_access_irq : 1;  /* bit[2]   : 上行4mic语音通路一键复位请求，高有效； */
        unsigned int  rst_4mic_audio_access_irq : 1;  /* bit[3]   : 上行4mic录音通路一键复位请求，高有效； */
        unsigned int  slv_mode_441              : 1;  /* bit[4]   : 指示i2s3在slv mode时，可以支持44.1k系列，高有效； */
        unsigned int  rst_4mic_dn_access_irq    : 1;  /* bit[5]   : 下行4mic通路一键复位请求，高有效； */
        unsigned int  rst_8mic_dn_access_irq    : 1;  /* bit[6]   : 下行8mic通路一键复位请求，高有效； */
        unsigned int  rst_8mic_up_access_irq    : 1;  /* bit[7]   : 上行8mic通路一键复位请求，高有效； */
        unsigned int  reserved_1                : 24; /* bit[8-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_SW_RST_N_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_SW_RST_N_codec_sw_rst_n_START             (0)
#define SOC_ASP_CODEC_CODEC_SW_RST_N_codec_sw_rst_n_END               (0)
#define SOC_ASP_CODEC_CODEC_SW_RST_N_rst_4mic_voice_access_irq_START  (2)
#define SOC_ASP_CODEC_CODEC_SW_RST_N_rst_4mic_voice_access_irq_END    (2)
#define SOC_ASP_CODEC_CODEC_SW_RST_N_rst_4mic_audio_access_irq_START  (3)
#define SOC_ASP_CODEC_CODEC_SW_RST_N_rst_4mic_audio_access_irq_END    (3)
#define SOC_ASP_CODEC_CODEC_SW_RST_N_slv_mode_441_START               (4)
#define SOC_ASP_CODEC_CODEC_SW_RST_N_slv_mode_441_END                 (4)
#define SOC_ASP_CODEC_CODEC_SW_RST_N_rst_4mic_dn_access_irq_START     (5)
#define SOC_ASP_CODEC_CODEC_SW_RST_N_rst_4mic_dn_access_irq_END       (5)
#define SOC_ASP_CODEC_CODEC_SW_RST_N_rst_8mic_dn_access_irq_START     (6)
#define SOC_ASP_CODEC_CODEC_SW_RST_N_rst_8mic_dn_access_irq_END       (6)
#define SOC_ASP_CODEC_CODEC_SW_RST_N_rst_8mic_up_access_irq_START     (7)
#define SOC_ASP_CODEC_CODEC_SW_RST_N_rst_8mic_up_access_irq_END       (7)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S1_CTRL_UNION
 结构说明  : I2S1_CTRL 寄存器结构定义。地址偏移量:0x0014，初值:0x30006000，宽度:32
 寄存器说明: I2S1_PCM接口控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                 : 13; /* bit[0-12] : reserved */
        unsigned int  fs_i2s1                  : 3;  /* bit[13-15]: I2S1接口采样率配置:
                                                                    3'b000：保留;
                                                                    3'b001：保留；
                                                                    3'b010：保留；
                                                                    3'b011：48k；
                                                                    3'b100：保留；
                                                                    3'b101：96k；
                                                                    3'b110：192k;
                                                                    其它：保留
                                                                    注：TDM模式只支持最大48k */
        unsigned int  i2s1_if_tx_ena           : 1;  /* bit[16]   : I2S1接口TX方向使能:
                                                                    1'b0: 关闭
                                                                    1'b1: 使能 */
        unsigned int  i2s1_if_rx_ena           : 1;  /* bit[17]   : I2S1接口RX方向使能:
                                                                    1'b0: 关闭
                                                                    1'b1: 使能 */
        unsigned int  i2s1_mst_slv             : 1;  /* bit[18]   : I2S1接口Master/Slave模式选择:
                                                                    1'b0: Mater模式
                                                                    1'b1: Slave模式 */
        unsigned int  i2s1_direct_loop         : 2;  /* bit[19-20]: I2S1 I2S接口环回模式选择.
                                                                    2'b00: 正常工作模式；
                                                                    2'b01: Sdin->Sdout.
                                                                    2'b10: RX_DATA[31:0] -> TX_IN[31:0]
                                                                    2'b11: Sdout -> Sdin */
        unsigned int  i2s1_func_mode           : 3;  /* bit[21-23]: I2S1接口模式选择:
                                                                    3'b000: I2S
                                                                    3'b010: PCM STD
                                                                    3'b011: PCM USER Defined
                                                                    3'b100: Left Justified
                                                                    3'b101: Right Justified
                                                                    其他: 保留 */
        unsigned int  i2s1_frame_mode          : 1;  /* bit[24]   : I2S1接口I2S模式下帧格式配置:
                                                                    1'b0: I2S模式下64bit frame，PCM模式下32bit frame
                                                                    1'b1: I2S模式下32bit frame，PCM模式下16bit frame */
        unsigned int  i2s1_lrclk_mode          : 1;  /* bit[25]   : I2S1接口帧同步信号左右声道选择:
                                                                    1'b0: low -> left channel
                                                                    1'b1: low -> right channel */
        unsigned int  i2s1_chnnl_mode          : 1;  /* bit[26]   : I2S1接口声道配置:
                                                                    1'b0: 左声道在前
                                                                    1'b1: 右声道在前 */
        unsigned int  i2s1_codec_data_format   : 1;  /* bit[27]   : I2S1接口Codec ADC/DAC数据格式选择.
                                                                    0: 表示二进制补码；
                                                                    1: 表示二进制码（binary offset）. */
        unsigned int  i2s1_codec_io_wordlength : 2;  /* bit[28-29]: I2S1接口位数控制.
                                                                    2'b00: 16bit；
                                                                    2'b01: 20bit；
                                                                    2'b10: 24bit；
                                                                    2'b11: 32bit. */
        unsigned int  i2s1_tx_clk_sel          : 1;  /* bit[30]   : I2S1接口TX方向时钟选择:
                                                                    1'b0: BCLK上升沿打出数据
                                                                    1'b1: BCLK下降沿打出数据 */
        unsigned int  i2s1_rx_clk_sel          : 1;  /* bit[31]   : I2S1接口RX方向时钟选择:
                                                                    1'b0: BCLK上升沿接收数据
                                                                    1'b1: BCLK下降沿接收数据 */
    } reg;
} SOC_ASP_CODEC_I2S1_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_I2S1_CTRL_fs_i2s1_START                   (13)
#define SOC_ASP_CODEC_I2S1_CTRL_fs_i2s1_END                     (15)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_if_tx_ena_START            (16)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_if_tx_ena_END              (16)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_if_rx_ena_START            (17)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_if_rx_ena_END              (17)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_mst_slv_START              (18)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_mst_slv_END                (18)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_direct_loop_START          (19)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_direct_loop_END            (20)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_func_mode_START            (21)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_func_mode_END              (23)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_frame_mode_START           (24)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_frame_mode_END             (24)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_lrclk_mode_START           (25)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_lrclk_mode_END             (25)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_chnnl_mode_START           (26)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_chnnl_mode_END             (26)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_codec_data_format_START    (27)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_codec_data_format_END      (27)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_codec_io_wordlength_START  (28)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_codec_io_wordlength_END    (29)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_tx_clk_sel_START           (30)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_tx_clk_sel_END             (30)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_rx_clk_sel_START           (31)
#define SOC_ASP_CODEC_I2S1_CTRL_i2s1_rx_clk_sel_END             (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S1_TDM_CTRL0_UNION
 结构说明  : I2S1_TDM_CTRL0 寄存器结构定义。地址偏移量:0x0018，初值:0x13411B00，宽度:32
 寄存器说明: I2S1_TDM接口控制寄存器0
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  s1_fifo_clr           : 1;  /* bit[0]    : S1路清fifo使能
                                                                 1'b0:不使能
                                                                 1'b1:使能 */
        unsigned int  s1_mem_icg_bp_en      : 1;  /* bit[1]    : i2s1_mem_icg enable:
                                                                 1'b1：enable;
                                                                 1'b0：disable; */
        unsigned int  s1_tdm_frame_mode     : 3;  /* bit[2-4]  : S1 TDM帧格式选择配置寄存器
                                                                 3’b000：pulse左对齐模式；
                                                                 3’b001：pulse shift模式；
                                                                 3’b010：one-chan左对齐模式；
                                                                 3’b011：one-chan shift模式；
                                                                 3’b100：half frame左对齐模式；
                                                                 3’b101：half frame shift模式；
                                                                 others：Reserved */
        unsigned int  s1_tdm_direct_loop    : 2;  /* bit[5-6]  : S1 TDM接口环回模式选择.
                                                                 2'b00: 正常工作模式；
                                                                 2'b01: Sdin->Sdout.
                                                                 2'b10: RX_DATA[31:0] -> TX_IN[31:0]
                                                                 2'b11: Sdout -> Sdin */
        unsigned int  s1_tdm_if_en          : 1;  /* bit[7]    : S1 TDM接口使能
                                                                 1'b0:不使能
                                                                 1'b1:使能 */
        unsigned int  s1_tdm_tx_ch3_sel     : 2;  /* bit[8-9]  : S1 TDM播放通路ch3声道选择，可以配置为0~3 */
        unsigned int  s1_tdm_tx_ch2_sel     : 2;  /* bit[10-11]: S1 TDM播放通路ch2声道选择，可以配置为0~3 */
        unsigned int  s1_tdm_tx_ch1_sel     : 2;  /* bit[12-13]: S1 TDM播放通路ch1声道选择，可以配置为0~3 */
        unsigned int  s1_tdm_tx_ch0_sel     : 2;  /* bit[14-15]: S1 TDM播放通路ch0声道选择，可以配置为0~3 */
        unsigned int  s1_tdm_rx_slot_sel_v0 : 3;  /* bit[16-18]: S1 TDM IV反馈通路V0 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  s1_tdm_rx_slot_sel_i0 : 3;  /* bit[19-21]: S1 TDM IV反馈通路I0 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  s1_tdm_tx_clk_sel     : 1;  /* bit[22]   : S1 TDM 接口TX方向时钟选择:
                                                                 1'b0: BCLK上升沿打出数据
                                                                 1'b1: BCLK下降沿打出数据 */
        unsigned int  s1_tdm_rx_clk_sel     : 1;  /* bit[23]   : S1 TDM 接口RX方向时钟选择:
                                                                 1'b0: BCLK上升沿接收数据
                                                                 1'b1: BCLK下降沿接收数据 */
        unsigned int  s1_tdm_rx_slot_sel_v1 : 3;  /* bit[24-26]: S1 TDM IV反馈通路V1 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  s1_tdm_rx_slot_sel_i1 : 3;  /* bit[27-29]: S1 TDM IV反馈通路I1 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  reserved              : 2;  /* bit[30-31]: reserved */
    } reg;
} SOC_ASP_CODEC_I2S1_TDM_CTRL0_UNION;
#endif
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_fifo_clr_START            (0)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_fifo_clr_END              (0)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_mem_icg_bp_en_START       (1)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_mem_icg_bp_en_END         (1)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_frame_mode_START      (2)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_frame_mode_END        (4)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_direct_loop_START     (5)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_direct_loop_END       (6)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_if_en_START           (7)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_if_en_END             (7)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_tx_ch3_sel_START      (8)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_tx_ch3_sel_END        (9)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_tx_ch2_sel_START      (10)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_tx_ch2_sel_END        (11)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_tx_ch1_sel_START      (12)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_tx_ch1_sel_END        (13)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_tx_ch0_sel_START      (14)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_tx_ch0_sel_END        (15)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_rx_slot_sel_v0_START  (16)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_rx_slot_sel_v0_END    (18)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_rx_slot_sel_i0_START  (19)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_rx_slot_sel_i0_END    (21)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_tx_clk_sel_START      (22)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_tx_clk_sel_END        (22)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_rx_clk_sel_START      (23)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_rx_clk_sel_END        (23)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_rx_slot_sel_v1_START  (24)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_rx_slot_sel_v1_END    (26)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_rx_slot_sel_i1_START  (27)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL0_s1_tdm_rx_slot_sel_i1_END    (29)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S1_TDM_CTRL1_UNION
 结构说明  : I2S1_TDM_CTRL1 寄存器结构定义。地址偏移量:0x001C，初值:0x00003725，宽度:32
 寄存器说明: I2S1_TDM接口控制寄存器1
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  s1_tdm_rx_slot_sel_v2 : 3;  /* bit[0-2]  : S1 TDM IV反馈通路V2 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  s1_tdm_rx_slot_sel_i2 : 3;  /* bit[3-5]  : S1 TDM IV反馈通路I2 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  reserved_0            : 2;  /* bit[6-7]  : reserved */
        unsigned int  s1_tdm_rx_slot_sel_v3 : 3;  /* bit[8-10] : S1 TDM IV反馈通路V3 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  s1_tdm_rx_slot_sel_i3 : 3;  /* bit[11-13]: S1 TDM IV反馈通路I3 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  reserved_1            : 2;  /* bit[14-15]: reserved */
        unsigned int  s1_tdm_mst_slv        : 1;  /* bit[16]   : S1 TDM Master/Slave选择
                                                                 1'b0: Master模式
                                                                 1'b1: Slave模式
                                                                 (只能做主) */
        unsigned int  s1_i2s_tdm_mode       : 1;  /* bit[17]   : s1路i2s/tdm模式选择
                                                                 0：i2s；
                                                                 1：tdm； */
        unsigned int  s1_tdm_err_stat       : 2;  /* bit[18-19]: S1 tdm配置错误状态查询
                                                                 bit[0]：rx_slot_sel_*有彼此配置相同；
                                                                 bit[1]：tx_ch*_sel有彼此配置相同； */
        unsigned int  s3_fifo_clr           : 1;  /* bit[20]   : S3路清fifo使能
                                                                 1'b0:不使能
                                                                 1'b1:使能 */
        unsigned int  s2_fifo_clr           : 1;  /* bit[21]   : S2路清fifo使能
                                                                 1'b0:不使能
                                                                 1'b1:使能 */
        unsigned int  reserved_2            : 10; /* bit[22-31]: reserved */
    } reg;
} SOC_ASP_CODEC_I2S1_TDM_CTRL1_UNION;
#endif
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_tdm_rx_slot_sel_v2_START  (0)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_tdm_rx_slot_sel_v2_END    (2)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_tdm_rx_slot_sel_i2_START  (3)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_tdm_rx_slot_sel_i2_END    (5)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_tdm_rx_slot_sel_v3_START  (8)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_tdm_rx_slot_sel_v3_END    (10)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_tdm_rx_slot_sel_i3_START  (11)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_tdm_rx_slot_sel_i3_END    (13)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_tdm_mst_slv_START         (16)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_tdm_mst_slv_END           (16)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_i2s_tdm_mode_START        (17)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_i2s_tdm_mode_END          (17)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_tdm_err_stat_START        (18)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s1_tdm_err_stat_END          (19)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s3_fifo_clr_START            (20)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s3_fifo_clr_END              (20)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s2_fifo_clr_START            (21)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL1_s2_fifo_clr_END              (21)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S2_PCM_CTRL_UNION
 结构说明  : I2S2_PCM_CTRL 寄存器结构定义。地址偏移量:0x0020，初值:0x20006000，宽度:32
 寄存器说明: I2S2_PCM接口控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                 : 13; /* bit[0-12] : reserved */
        unsigned int  fs_i2s2                  : 3;  /* bit[13-15]: I2S2接口采样率配置:
                                                                    3'b000：8k;
                                                                    3'b001：16k；
                                                                    3'b010：32k；
                                                                    3'b011：48k；
                                                                    3'b100：保留；
                                                                    3'b101：96k；
                                                                    3'b110：192k；
                                                                    其它：保留 */
        unsigned int  i2s2_if_tx_ena           : 1;  /* bit[16]   : I2S2接口TX方向使能:
                                                                    1'b1: 使能
                                                                    1'b0: 关闭 */
        unsigned int  i2s2_if_rx_ena           : 1;  /* bit[17]   : I2S2接口RX方向使能:
                                                                    1'b1: 使能
                                                                    1'b0: 关闭 */
        unsigned int  i2s2_mst_slv             : 1;  /* bit[18]   : I2S2接口Master/Slave模式选择:
                                                                    1'b0: Mater模式
                                                                    1'b1: Slave模式 */
        unsigned int  i2s2_direct_loop         : 2;  /* bit[19-20]: I2S2 I2S接口环回模式选择.
                                                                    2'b00: 正常工作模式；
                                                                    2'b01: Sdin->Sdout.
                                                                    2'b10: RX_DATA[31:0] -> TX_IN[31:0]
                                                                    2'b11: Sdout -> Sdin */
        unsigned int  i2s2_func_mode           : 3;  /* bit[21-23]: I2S2接口模式选择:
                                                                    3'b000: I2S
                                                                    3'b010: PCM STD
                                                                    3'b011: PCM USER Defined
                                                                    3'b100: Left Justified
                                                                    3'b101: Right Justified
                                                                    其他: 保留 */
        unsigned int  i2s2_frame_mode          : 1;  /* bit[24]   : I2S2接口I2S模式下帧格式配置:
                                                                    1'b0: I2S模式下64bit frame，PCM模式下32bit frame
                                                                    1'b1: I2S模式下32bit frame，PCM模式下16bit frame */
        unsigned int  i2s2_lrclk_mode          : 1;  /* bit[25]   : I2S2接口帧同步信号左右声道选择:
                                                                    1'b0: low -> left channel
                                                                    1'b1: low -> right channel */
        unsigned int  i2s2_chnnl_mode          : 1;  /* bit[26]   : I2S2接口声道配置:
                                                                    1'b0: 左声道在前
                                                                    1'b1: 右声道在前 */
        unsigned int  i2s2_codec_data_format   : 1;  /* bit[27]   : I2S2接口Codec ADC/DAC数据格式选择.
                                                                    0: 表示二进制补码；
                                                                    1: 表示二进制码（binary offset）. */
        unsigned int  i2s2_codec_io_wordlength : 2;  /* bit[28-29]: I2S2接口位数控制.
                                                                    2'b00: 16bit；
                                                                    2'b01: 20bit；
                                                                    2'b10: 24bit；
                                                                    2'b11: 32bit. */
        unsigned int  i2s2_tx_clk_sel          : 1;  /* bit[30]   : I2S2接口TX方向时钟选择:
                                                                    1'b0: BCLK上升沿打出数据
                                                                    1'b1: BCLK下降沿打出数据 */
        unsigned int  i2s2_rx_clk_sel          : 1;  /* bit[31]   : I2S2接口RX方向时钟选择:
                                                                    1'b0: BCLK上升沿接收数据
                                                                    1'b1: BCLK下降沿接收数据 */
    } reg;
} SOC_ASP_CODEC_I2S2_PCM_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_fs_i2s2_START                   (13)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_fs_i2s2_END                     (15)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_if_tx_ena_START            (16)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_if_tx_ena_END              (16)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_if_rx_ena_START            (17)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_if_rx_ena_END              (17)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_mst_slv_START              (18)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_mst_slv_END                (18)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_direct_loop_START          (19)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_direct_loop_END            (20)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_func_mode_START            (21)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_func_mode_END              (23)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_frame_mode_START           (24)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_frame_mode_END             (24)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_lrclk_mode_START           (25)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_lrclk_mode_END             (25)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_chnnl_mode_START           (26)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_chnnl_mode_END             (26)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_codec_data_format_START    (27)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_codec_data_format_END      (27)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_codec_io_wordlength_START  (28)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_codec_io_wordlength_END    (29)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_tx_clk_sel_START           (30)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_tx_clk_sel_END             (30)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_rx_clk_sel_START           (31)
#define SOC_ASP_CODEC_I2S2_PCM_CTRL_i2s2_rx_clk_sel_END             (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S3_PCM_CTRL_UNION
 结构说明  : I2S3_PCM_CTRL 寄存器结构定义。地址偏移量:0x0024，初值:0x2000C000，宽度:32
 寄存器说明: I2S3_PCM接口控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                 : 13; /* bit[0-12] : reserved */
        unsigned int  fs_i2s3                  : 3;  /* bit[13-15]: I2S3采样率控制：（44.1k~176.4k仅在slv_mode_441==1'b1时生效）
                                                                    3'b000：16k;
                                                                    3'b001：32k；
                                                                    3'b010：保留；
                                                                    3'b011：96k/88.2k；
                                                                    3'b100：48k/44.1k；
                                                                    3'b101：192k/176.4k；
                                                                    3'b110：384k/352.8k；
                                                                    3'b111 : 8k ;
                                                                    其它：48k； */
        unsigned int  i2s3_if_tx_ena           : 1;  /* bit[16]   : I2S3接口TX方向使能:
                                                                    1'b1: 使能
                                                                    1'b0: 关闭 */
        unsigned int  i2s3_if_rx_ena           : 1;  /* bit[17]   : I2S3接口RX方向使能:
                                                                    1'b1: 使能
                                                                    1'b0: 关闭 */
        unsigned int  i2s3_mst_slv             : 1;  /* bit[18]   : I2S3接口Master/Slave模式选择:
                                                                    1'b0: Mater模式
                                                                    1'b1: Slave模式 */
        unsigned int  i2s3_direct_loop         : 2;  /* bit[19-20]: I2S3 I2S接口环回模式选择.
                                                                    2'b00: 正常工作模式；
                                                                    2'b01: Sdin->Sdout.
                                                                    2'b10: RX_DATA[31:0] -> TX_IN[31:0]
                                                                    2'b11: Sdout -> Sdin */
        unsigned int  i2s3_func_mode           : 3;  /* bit[21-23]: I2S3接口模式选择:
                                                                    3'b000: I2S
                                                                    3'b010: PCM STD
                                                                    3'b011: PCM USER Defined
                                                                    3'b100: Left Justified
                                                                    3'b101: Right Justified
                                                                    其他: 保留 */
        unsigned int  i2s3_frame_mode          : 1;  /* bit[24]   : I2S3接口I2S模式下帧格式配置:
                                                                    1'b0: I2S模式下64bit frame，PCM模式下32bit frame
                                                                    1'b1: I2S模式下32bit frame，PCM模式下16bit frame */
        unsigned int  i2s3_lrclk_mode          : 1;  /* bit[25]   : I2S3接口帧同步信号左右声道选择:
                                                                    1'b0: low -> left channel
                                                                    1'b1: low -> right channel */
        unsigned int  i2s3_chnnl_mode          : 1;  /* bit[26]   : I2S3接口声道配置:
                                                                    1'b0: 左声道在前
                                                                    1'b1: 右声道在前 */
        unsigned int  i2s3_codec_data_format   : 1;  /* bit[27]   : I2S3接口Codec ADC/DAC数据格式选择.
                                                                    0: 表示二进制补码；
                                                                    1: 表示二进制码（binary offset）. */
        unsigned int  i2s3_codec_io_wordlength : 2;  /* bit[28-29]: I2S3接口位数控制.
                                                                    2'b00: 16bit；
                                                                    2'b01: 20bit；
                                                                    2'b10: 24bit；
                                                                    2'b11: 32bit. */
        unsigned int  i2s3_tx_clk_sel          : 1;  /* bit[30]   : I2S3接口TX方向时钟选择:
                                                                    1'b0: BCLK上升沿打出数据
                                                                    1'b1: BCLK下降沿打出数据 */
        unsigned int  i2s3_rx_clk_sel          : 1;  /* bit[31]   : I2S3接口RX方向时钟选择:
                                                                    1'b0: BCLK上升沿接收数据
                                                                    1'b1: BCLK下降沿接收数据 */
    } reg;
} SOC_ASP_CODEC_I2S3_PCM_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_fs_i2s3_START                   (13)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_fs_i2s3_END                     (15)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_if_tx_ena_START            (16)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_if_tx_ena_END              (16)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_if_rx_ena_START            (17)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_if_rx_ena_END              (17)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_mst_slv_START              (18)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_mst_slv_END                (18)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_direct_loop_START          (19)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_direct_loop_END            (20)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_func_mode_START            (21)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_func_mode_END              (23)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_frame_mode_START           (24)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_frame_mode_END             (24)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_lrclk_mode_START           (25)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_lrclk_mode_END             (25)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_chnnl_mode_START           (26)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_chnnl_mode_END             (26)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_codec_data_format_START    (27)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_codec_data_format_END      (27)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_codec_io_wordlength_START  (28)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_codec_io_wordlength_END    (29)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_tx_clk_sel_START           (30)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_tx_clk_sel_END             (30)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_rx_clk_sel_START           (31)
#define SOC_ASP_CODEC_I2S3_PCM_CTRL_i2s3_rx_clk_sel_END             (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S4_CTRL_UNION
 结构说明  : I2S4_CTRL 寄存器结构定义。地址偏移量:0x0028，初值:0x3000A000，宽度:32
 寄存器说明: I2S4_PCM接口控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                 : 13; /* bit[0-12] : reserved */
        unsigned int  fs_i2s4                  : 3;  /* bit[13-15]: I2S4接口采样率配置:
                                                                    I2S2接口采样率配置:
                                                                    3'b000：保留;
                                                                    3'b001：保留；
                                                                    3'b010：保留；
                                                                    3'b011：保留；
                                                                    3'b100：保留；
                                                                    3'b101：96k；
                                                                    3'b110：保留；
                                                                    其它：保留
                                                                    注：TDM模式只支持最大48k */
        unsigned int  i2s4_if_tx_ena           : 1;  /* bit[16]   : I2S4接口TX方向使能:
                                                                    1'b1: 使能
                                                                    1'b0: 关闭 */
        unsigned int  i2s4_if_rx_ena           : 1;  /* bit[17]   : I2S4接口RX方向使能:
                                                                    1'b1: 使能
                                                                    1'b0: 关闭 */
        unsigned int  i2s4_mst_slv             : 1;  /* bit[18]   : I2S4接口Master/Slave模式选择:
                                                                    1'b0: Mater模式
                                                                    1'b1: Slave模式 */
        unsigned int  i2s4_direct_loop         : 2;  /* bit[19-20]: I2S4 I2S接口环回模式选择.
                                                                    2'b00: 正常工作模式；
                                                                    2'b01: Sdin->Sdout.
                                                                    2'b10: RX_DATA[31:0] -> TX_IN[31:0]
                                                                    2'b11: Sdout -> Sdin */
        unsigned int  i2s4_func_mode           : 3;  /* bit[21-23]: I2S4接口模式选择:
                                                                    3'b000: I2S
                                                                    3'b010: PCM STD
                                                                    3'b011: PCM USER Defined
                                                                    3'b100: Left Justified
                                                                    3'b101: Right Justified
                                                                    其他: 保留 */
        unsigned int  i2s4_frame_mode          : 1;  /* bit[24]   : I2S4接口I2S模式下帧格式配置:
                                                                    1'b0: I2S模式下64bit frame，PCM模式下32bit frame
                                                                    1'b1: I2S模式下32bit frame，PCM模式下16bit frame */
        unsigned int  i2s4_lrclk_mode          : 1;  /* bit[25]   : I2S4接口帧同步信号左右声道选择:
                                                                    1'b0: low -> left channel
                                                                    1'b1: low -> right channel */
        unsigned int  i2s4_chnnl_mode          : 1;  /* bit[26]   : I2S4接口声道配置:
                                                                    1'b0: 左声道在前
                                                                    1'b1: 右声道在前 */
        unsigned int  i2s4_codec_data_format   : 1;  /* bit[27]   : I2S4接口Codec ADC/DAC数据格式选择.
                                                                    0: 表示二进制补码；
                                                                    1: 表示二进制码（binary offset）. */
        unsigned int  i2s4_codec_io_wordlength : 2;  /* bit[28-29]: I2S4接口位数控制.
                                                                    2'b00: 16bit；
                                                                    2'b01: 20bit；
                                                                    2'b10: 24bit；
                                                                    2'b11: 32bit. */
        unsigned int  i2s4_tx_clk_sel          : 1;  /* bit[30]   : I2S4接口TX方向时钟选择:
                                                                    1'b0: BCLK上升沿打出数据
                                                                    1'b1: BCLK下降沿打出数据 */
        unsigned int  i2s4_rx_clk_sel          : 1;  /* bit[31]   : I2S4接口RX方向时钟选择:
                                                                    1'b0: BCLK上升沿接收数据
                                                                    1'b1: BCLK下降沿接收数据 */
    } reg;
} SOC_ASP_CODEC_I2S4_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_I2S4_CTRL_fs_i2s4_START                   (13)
#define SOC_ASP_CODEC_I2S4_CTRL_fs_i2s4_END                     (15)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_if_tx_ena_START            (16)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_if_tx_ena_END              (16)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_if_rx_ena_START            (17)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_if_rx_ena_END              (17)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_mst_slv_START              (18)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_mst_slv_END                (18)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_direct_loop_START          (19)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_direct_loop_END            (20)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_func_mode_START            (21)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_func_mode_END              (23)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_frame_mode_START           (24)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_frame_mode_END             (24)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_lrclk_mode_START           (25)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_lrclk_mode_END             (25)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_chnnl_mode_START           (26)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_chnnl_mode_END             (26)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_codec_data_format_START    (27)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_codec_data_format_END      (27)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_codec_io_wordlength_START  (28)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_codec_io_wordlength_END    (29)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_tx_clk_sel_START           (30)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_tx_clk_sel_END             (30)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_rx_clk_sel_START           (31)
#define SOC_ASP_CODEC_I2S4_CTRL_i2s4_rx_clk_sel_END             (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S4_TDM_CTRL0_UNION
 结构说明  : I2S4_TDM_CTRL0 寄存器结构定义。地址偏移量:0x002C，初值:0x13411B00，宽度:32
 寄存器说明: I2S4_TDM接口控制寄存器0
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  s4_fifo_clr           : 1;  /* bit[0]    : S4路清fifo使能
                                                                 1'b0:不使能
                                                                 1'b1:使能 */
        unsigned int  s4_mem_icg_bp_en      : 1;  /* bit[1]    : i2s4_mem_icg enable:
                                                                 1'b1：enable;
                                                                 1'b0：disable; */
        unsigned int  s4_tdm_frame_mode     : 3;  /* bit[2-4]  : S4 TDM帧格式选择配置寄存器
                                                                 3’b000：pulse左对齐模式；
                                                                 3’b001：pulse shift模式；
                                                                 3’b010：one-chan左对齐模式；
                                                                 3’b011：one-chan shift模式；
                                                                 3’b100：half frame左对齐模式；
                                                                 3’b101：half frame shift模式；
                                                                 others：Reserved */
        unsigned int  s4_tdm_direct_loop    : 2;  /* bit[5-6]  : S4 TDM接口环回模式选择.
                                                                 2'b00: 正常工作模式；
                                                                 2'b01: Sdin->Sdout.
                                                                 2'b10: RX_DATA[31:0] -> TX_IN[31:0]
                                                                 2'b11: Sdout -> Sdin */
        unsigned int  s4_tdm_if_en          : 1;  /* bit[7]    : S4 TDM接口使能
                                                                 1'b0:不使能
                                                                 1'b1:使能 */
        unsigned int  s4_tdm_tx_ch3_sel     : 2;  /* bit[8-9]  : S4 TDM播放通路ch3声道选择，可以配置为0~3 */
        unsigned int  s4_tdm_tx_ch2_sel     : 2;  /* bit[10-11]: S4 TDM播放通路ch2声道选择，可以配置为0~3 */
        unsigned int  s4_tdm_tx_ch1_sel     : 2;  /* bit[12-13]: S4 TDM播放通路ch1声道选择，可以配置为0~3 */
        unsigned int  s4_tdm_tx_ch0_sel     : 2;  /* bit[14-15]: S4 TDM播放通路ch0声道选择，可以配置为0~3 */
        unsigned int  s4_tdm_rx_slot_sel_v0 : 3;  /* bit[16-18]: S4 TDM IV反馈通路V0 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  s4_tdm_rx_slot_sel_i0 : 3;  /* bit[19-21]: S4 TDM IV反馈通路I0 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  s4_tdm_tx_clk_sel     : 1;  /* bit[22]   : S4 TDM 接口TX方向时钟选择:
                                                                 1'b0: BCLK上升沿打出数据
                                                                 1'b1: BCLK下降沿打出数据 */
        unsigned int  s4_tdm_rx_clk_sel     : 1;  /* bit[23]   : S4 TDM 接口RX方向时钟选择:
                                                                 1'b0: BCLK上升沿接收数据
                                                                 1'b1: BCLK下降沿接收数据 */
        unsigned int  s4_tdm_rx_slot_sel_v1 : 3;  /* bit[24-26]: S4 TDM IV反馈通路V1 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  s4_tdm_rx_slot_sel_i1 : 3;  /* bit[27-29]: S4 TDM IV反馈通路I1 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  reserved              : 2;  /* bit[30-31]: reserved */
    } reg;
} SOC_ASP_CODEC_I2S4_TDM_CTRL0_UNION;
#endif
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_fifo_clr_START            (0)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_fifo_clr_END              (0)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_mem_icg_bp_en_START       (1)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_mem_icg_bp_en_END         (1)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_frame_mode_START      (2)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_frame_mode_END        (4)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_direct_loop_START     (5)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_direct_loop_END       (6)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_if_en_START           (7)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_if_en_END             (7)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_tx_ch3_sel_START      (8)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_tx_ch3_sel_END        (9)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_tx_ch2_sel_START      (10)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_tx_ch2_sel_END        (11)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_tx_ch1_sel_START      (12)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_tx_ch1_sel_END        (13)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_tx_ch0_sel_START      (14)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_tx_ch0_sel_END        (15)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_rx_slot_sel_v0_START  (16)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_rx_slot_sel_v0_END    (18)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_rx_slot_sel_i0_START  (19)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_rx_slot_sel_i0_END    (21)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_tx_clk_sel_START      (22)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_tx_clk_sel_END        (22)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_rx_clk_sel_START      (23)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_rx_clk_sel_END        (23)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_rx_slot_sel_v1_START  (24)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_rx_slot_sel_v1_END    (26)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_rx_slot_sel_i1_START  (27)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL0_s4_tdm_rx_slot_sel_i1_END    (29)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S4_TDM_CTRL1_UNION
 结构说明  : I2S4_TDM_CTRL1 寄存器结构定义。地址偏移量:0x0030，初值:0x00003725，宽度:32
 寄存器说明: I2S4_TDM接口控制寄存器1
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  s4_tdm_rx_slot_sel_v2 : 3;  /* bit[0-2]  : S4 TDM IV反馈通路V2 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  s4_tdm_rx_slot_sel_i2 : 3;  /* bit[3-5]  : S4 TDM IV反馈通路I2 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  reserved_0            : 2;  /* bit[6-7]  : reserved */
        unsigned int  s4_tdm_rx_slot_sel_v3 : 3;  /* bit[8-10] : S4 TDM IV反馈通路V3 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  s4_tdm_rx_slot_sel_i3 : 3;  /* bit[11-13]: S4 TDM IV反馈通路I3 slot选择，可以配置为0~7
                                                                 3'b000: slot0 （[127:112]bit）
                                                                 3'b001: slot1 （[111:96]bit）
                                                                 3'b010: slot2 （[95:80]bit）
                                                                 3'b011: slot3 （[79:64]bit）
                                                                 3'b100: slot4 （[63:48]bit）
                                                                 3'b101: slot5 （[47:32]bit）
                                                                 3'b110: slot6 （[31:16]bit）
                                                                 3'b111: slot7 （[15:0]bit） */
        unsigned int  reserved_1            : 2;  /* bit[14-15]: reserved */
        unsigned int  s4_tdm_mst_slv        : 1;  /* bit[16]   : S4 TDM Master/Slave选择
                                                                 1'b0: Master模式
                                                                 1'b1: Slave模式
                                                                 (只能做主) */
        unsigned int  s4_i2s_tdm_mode       : 1;  /* bit[17]   : s4路i2s/tdm模式选择
                                                                 0：i2s；
                                                                 1：tdm； */
        unsigned int  s4_tdm_err_stat       : 2;  /* bit[18-19]: S4 tdm配置错误状态查询
                                                                 bit[0]：rx_slot_sel_*有彼此配置相同；
                                                                 bit[1]：tx_ch*_sel有彼此配置相同； */
        unsigned int  reserved_2            : 12; /* bit[20-31]: reserved */
    } reg;
} SOC_ASP_CODEC_I2S4_TDM_CTRL1_UNION;
#endif
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_tdm_rx_slot_sel_v2_START  (0)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_tdm_rx_slot_sel_v2_END    (2)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_tdm_rx_slot_sel_i2_START  (3)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_tdm_rx_slot_sel_i2_END    (5)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_tdm_rx_slot_sel_v3_START  (8)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_tdm_rx_slot_sel_v3_END    (10)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_tdm_rx_slot_sel_i3_START  (11)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_tdm_rx_slot_sel_i3_END    (13)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_tdm_mst_slv_START         (16)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_tdm_mst_slv_END           (16)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_i2s_tdm_mode_START        (17)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_i2s_tdm_mode_END          (17)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_tdm_err_stat_START        (18)
#define SOC_ASP_CODEC_I2S4_TDM_CTRL1_s4_tdm_err_stat_END          (19)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_PGA_THRE_CTRL0_UNION
 结构说明  : PGA_THRE_CTRL0 寄存器结构定义。地址偏移量:0x0034，初值:0x00000000，宽度:32
 寄存器说明: PGA/MIXER阈值控制寄存器0
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  pga_mixer_thre_ctrl0 : 32; /* bit[0-31]: PGA/MIXER阈值信号[31:0]，用于过零检测。
                                                               每个例化的PGA/MIXER模块有四种24bit的PGA阈值可配 */
    } reg;
} SOC_ASP_CODEC_PGA_THRE_CTRL0_UNION;
#endif
#define SOC_ASP_CODEC_PGA_THRE_CTRL0_pga_mixer_thre_ctrl0_START  (0)
#define SOC_ASP_CODEC_PGA_THRE_CTRL0_pga_mixer_thre_ctrl0_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_PGA_THRE_CTRL1_UNION
 结构说明  : PGA_THRE_CTRL1 寄存器结构定义。地址偏移量:0x0038，初值:0x00000000，宽度:32
 寄存器说明: PGA/MIXER阈值控制寄存器1
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  pga_mixer_thre_ctrl1 : 32; /* bit[0-31]: PGA/MIXER阈值信号[63:32]，用于过零检测。
                                                               每个例化的PGA/MIXER模块有四种24bit的PGA阈值可配 */
    } reg;
} SOC_ASP_CODEC_PGA_THRE_CTRL1_UNION;
#endif
#define SOC_ASP_CODEC_PGA_THRE_CTRL1_pga_mixer_thre_ctrl1_START  (0)
#define SOC_ASP_CODEC_PGA_THRE_CTRL1_pga_mixer_thre_ctrl1_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_PGA_THRE_CTRL2_UNION
 结构说明  : PGA_THRE_CTRL2 寄存器结构定义。地址偏移量:0x003C，初值:0x00000000，宽度:32
 寄存器说明: PGA/MIXER阈值控制寄存器2
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  pga_mixer_thre_ctrl2 : 32; /* bit[0-31]: PGA/MIXER阈值信号[95:64]，用于过零检测。
                                                               每个例化的PGA/MIXER模块有四种24bit的PGA阈值可配 */
    } reg;
} SOC_ASP_CODEC_PGA_THRE_CTRL2_UNION;
#endif
#define SOC_ASP_CODEC_PGA_THRE_CTRL2_pga_mixer_thre_ctrl2_START  (0)
#define SOC_ASP_CODEC_PGA_THRE_CTRL2_pga_mixer_thre_ctrl2_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_UNION
 结构说明  : PGA_GAINOFFSET_CTRL0 寄存器结构定义。地址偏移量:0x0040，初值:0x00000000，宽度:32
 寄存器说明: PGA GAINOFFSET配置寄存器0
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  codec3_dn_l_pga_gainoffset : 8;  /* bit[0-7]  : 增益偏置，gainoffset有效配置范围是0~255
                                                                      gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                      为粒度= max(2^(fadeInTime -12),1)，
                                                                      以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  codec3_dn_r_pga_gainoffset : 8;  /* bit[8-15] : 增益偏置，gainoffset有效配置范围是0~255
                                                                      gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                      为粒度= max(2^(fadeInTime -12),1)，
                                                                      以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  audio_dn_l_pga_gainoffset  : 8;  /* bit[16-23]: 增益偏置，gainoffset有效配置范围是0~255
                                                                      gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                      为粒度= max(2^(fadeInTime -12),1)，
                                                                      以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  audio_dn_r_pga_gainoffset  : 8;  /* bit[24-31]: 增益偏置，gainoffset有效配置范围是0~255
                                                                      gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                      为粒度= max(2^(fadeInTime -12),1)，
                                                                      以fadeInTime=16为例，gainOffset可取0,16,32……; */
    } reg;
} SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_UNION;
#endif
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_codec3_dn_l_pga_gainoffset_START  (0)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_codec3_dn_l_pga_gainoffset_END    (7)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_codec3_dn_r_pga_gainoffset_START  (8)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_codec3_dn_r_pga_gainoffset_END    (15)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_audio_dn_l_pga_gainoffset_START   (16)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_audio_dn_l_pga_gainoffset_END     (23)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_audio_dn_r_pga_gainoffset_START   (24)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL0_audio_dn_r_pga_gainoffset_END     (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_UNION
 结构说明  : PGA_GAINOFFSET_CTRL1 寄存器结构定义。地址偏移量:0x0044，初值:0x00000000，宽度:32
 寄存器说明: PGA GAINOFFSET配置寄存器1
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  audio_up_l_pga_gainoffset : 8;  /* bit[0-7]  : 增益偏置，gainoffset有效配置范围是0~255
                                                                     gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                     为粒度= max(2^(fadeInTime -12),1)，
                                                                     以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  audio_up_r_pga_gainoffset : 8;  /* bit[8-15] : 增益偏置，gainoffset有效配置范围是0~255
                                                                     gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                     为粒度= max(2^(fadeInTime -12),1)，
                                                                     以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  voice_up_l_pga_gainoffset : 8;  /* bit[16-23]: 增益偏置，gainoffset有效配置范围是0~255
                                                                     gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                     为粒度= max(2^(fadeInTime -12),1)，
                                                                     以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  voice_up_r_pga_gainoffset : 8;  /* bit[24-31]: 增益偏置，gainoffset有效配置范围是0~255
                                                                     gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                     为粒度= max(2^(fadeInTime -12),1)，
                                                                     以fadeInTime=16为例，gainOffset可取0,16,32……; */
    } reg;
} SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_UNION;
#endif
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_audio_up_l_pga_gainoffset_START  (0)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_audio_up_l_pga_gainoffset_END    (7)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_audio_up_r_pga_gainoffset_START  (8)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_audio_up_r_pga_gainoffset_END    (15)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_voice_up_l_pga_gainoffset_START  (16)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_voice_up_l_pga_gainoffset_END    (23)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_voice_up_r_pga_gainoffset_START  (24)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL1_voice_up_r_pga_gainoffset_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_UNION
 结构说明  : PGA_GAINOFFSET_CTRL2 寄存器结构定义。地址偏移量:0x0048，初值:0x00000000，宽度:32
 寄存器说明: PGA GAINOFFSET配置寄存器2
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  mic3_up_pga_gainoffset     : 8;  /* bit[0-7]  : 增益偏置，gainoffset有效配置范围是0~255
                                                                      gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                      为粒度= max(2^(fadeInTime -12),1)，
                                                                      以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  mic4_up_pga_gainoffset     : 8;  /* bit[8-15] : 增益偏置，gainoffset有效配置范围是0~255
                                                                      gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                      为粒度= max(2^(fadeInTime -12),1)，
                                                                      以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  mdm_5g_up_l_pga_gainoffset : 8;  /* bit[16-23]: 增益偏置，gainoffset有效配置范围是0~255
                                                                      gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                      为粒度= max(2^(fadeInTime -12),1)，
                                                                      以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  mdm_5g_up_r_pga_gainoffset : 8;  /* bit[24-31]: 增益偏置，gainoffset有效配置范围是0~255
                                                                      gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                      为粒度= max(2^(fadeInTime -12),1)，
                                                                      以fadeInTime=16为例，gainOffset可取0,16,32……; */
    } reg;
} SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_UNION;
#endif
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_mic3_up_pga_gainoffset_START      (0)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_mic3_up_pga_gainoffset_END        (7)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_mic4_up_pga_gainoffset_START      (8)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_mic4_up_pga_gainoffset_END        (15)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_mdm_5g_up_l_pga_gainoffset_START  (16)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_mdm_5g_up_l_pga_gainoffset_END    (23)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_mdm_5g_up_r_pga_gainoffset_START  (24)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL2_mdm_5g_up_r_pga_gainoffset_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL3_UNION
 结构说明  : PGA_GAINOFFSET_CTRL3 寄存器结构定义。地址偏移量:0x004C，初值:0x00000000，宽度:32
 寄存器说明: PGA GAINOFFSET配置寄存器3
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  i2s2_rx_l_pga_gainoffset : 8;  /* bit[0-7]  : 增益偏置，gainoffset有效配置范围是0~255
                                                                    gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                    为粒度= max(2^(fadeInTime -12),1)，
                                                                    以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  i2s2_rx_r_pga_gainoffset : 8;  /* bit[8-15] : 增益偏置，gainoffset有效配置范围是0~255
                                                                    gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                    为粒度= max(2^(fadeInTime -12),1)，
                                                                    以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  sidetone_pga_gainoffset  : 8;  /* bit[16-23]: 增益偏置，gainoffset有效配置范围是0~255
                                                                    gain_offset的调整粒度与fadeInTime相关，映射关系
                                                                    为粒度= max(2^(fadeInTime -12),1)，
                                                                    以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  reserved                 : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL3_UNION;
#endif
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL3_i2s2_rx_l_pga_gainoffset_START  (0)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL3_i2s2_rx_l_pga_gainoffset_END    (7)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL3_i2s2_rx_r_pga_gainoffset_START  (8)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL3_i2s2_rx_r_pga_gainoffset_END    (15)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL3_sidetone_pga_gainoffset_START   (16)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL3_sidetone_pga_gainoffset_END     (23)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_UNION
 结构说明  : CODEC3_L_DN_PGA_CTRL 寄存器结构定义。地址偏移量:0x0050，初值:0x00000050，宽度:32
 寄存器说明: CODEC3_L_DN PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                   : 1;  /* bit[0]    : reserved */
        unsigned int  codec3_l_dn_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                      1'b0: 选择对数淡出淡出
                                                                      1'b1: 选择线性淡入淡出 */
        unsigned int  codec3_l_dn_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  codec3_l_dn_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用
                                                                      其中2bit的id选择四种场景的阈值
                                                                      0：选择pga_thre_ctrl[23:0]
                                                                      1：选择pga_thre_ctrl[47:24]
                                                                      2：选择pga_thre_ctrl[71:48]
                                                                      3：选择pga_thre_ctrl[95:72] */
        unsigned int  codec3_l_dn_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值
                                                                      的小信号放过去，低表示低于阈值的信号钳制到0
                                                                      1：不对噪音信号做任何处理
                                                                      0：将低于阈值的噪音信号过滤成0 */
        unsigned int  codec3_l_dn_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                      1'b0: 不bypass
                                                                      1'b1: bypass */
        unsigned int  codec3_l_dn_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                      采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  codec3_l_dn_pga_fade_in    : 5;  /* bit[16-20]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                      采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  codec3_l_dn_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                      [7]：淡入淡出功能使能
                                                                      1：淡入淡出功能使能
                                                                      0：淡入淡出功能不使能
                                                                      [6]: 抗削波功能使能
                                                                      1：抗削波功能使能
                                                                      0：抗削波功能不使能
                                                                      [5]: 小信号功能使能
                                                                      1：小信号功能使能
                                                                      0：小信号功能不使能 */
        unsigned int  codec3_l_dn_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                      例如：60dB配置成0x3C；
                                                                       0dB配置为0x00；
                                                                       -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_bypass_START      (10)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_bypass_END        (10)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_cfg_START         (21)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_cfg_END           (23)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_gain_START        (24)
#define SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_codec3_l_dn_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_UNION
 结构说明  : CODEC3_R_DN_PGA_CTRL 寄存器结构定义。地址偏移量:0x0054，初值:0x00000050，宽度:32
 寄存器说明: CODEC3_R_DN PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                   : 1;  /* bit[0]    : reserved */
        unsigned int  codec3_r_dn_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                      1'b0: 选择对数淡出淡出
                                                                      1'b1: 选择线性淡入淡出 */
        unsigned int  codec3_r_dn_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  codec3_r_dn_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用
                                                                      其中2bit的id选择四种场景的阈值
                                                                      0：选择pga_thre_ctrl[23:0]
                                                                      1：选择pga_thre_ctrl[47:24]
                                                                      2：选择pga_thre_ctrl[71:48]
                                                                      3：选择pga_thre_ctrl[95:72] */
        unsigned int  codec3_r_dn_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值
                                                                      的小信号放过去，低表示低于阈值的信号钳制到0
                                                                      1：不对噪音信号做任何处理
                                                                      0：将低于阈值的噪音信号过滤成0 */
        unsigned int  codec3_r_dn_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                      1'b0: 不bypass
                                                                      1'b1: bypass */
        unsigned int  codec3_r_dn_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                      采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  codec3_r_dn_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                      采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  codec3_r_dn_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                      [7]：淡入淡出功能使能
                                                                      1：淡入淡出功能使能
                                                                      0：淡入淡出功能不使能
                                                                      [6]: 抗削波功能使能
                                                                      1：抗削波功能使能
                                                                      0：抗削波功能不使能
                                                                      [5]: 小信号功能使能
                                                                      1：小信号功能使能
                                                                      0：小信号功能不使能 */
        unsigned int  codec3_r_dn_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                      例如：60dB配置成0x3C；
                                                                       0dB配置为0x00；
                                                                       -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_bypass_START      (10)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_bypass_END        (10)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_cfg_START         (21)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_cfg_END           (23)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_gain_START        (24)
#define SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_codec3_r_dn_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_UNION
 结构说明  : AUDIO_L_DN_PGA_CTRL 寄存器结构定义。地址偏移量:0x0058，初值:0x00000050，宽度:32
 寄存器说明: AUDIO下行通路左声道PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                  : 1;  /* bit[0]    : reserved */
        unsigned int  audio_l_dn_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                     1'b0: 选择对数淡出淡出
                                                                     1'b1: 选择线性淡入淡出 */
        unsigned int  audio_l_dn_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  audio_l_dn_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用
                                                                     其中2bit的id选择四种场景的阈值
                                                                     0：选择pga_thre_ctrl[23:0]
                                                                     1：选择pga_thre_ctrl[47:24]
                                                                     2：选择pga_thre_ctrl[71:48]
                                                                     3：选择pga_thre_ctrl[95:72] */
        unsigned int  audio_l_dn_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的
                                                                     小信号放过去，低表示低于阈值的信号钳制到0
                                                                     1：不对噪音信号做任何处理
                                                                     0：将低于阈值的噪音信号过滤成0 */
        unsigned int  audio_l_dn_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                     1'b0: 不bypass
                                                                     1'b1: bypass */
        unsigned int  audio_l_dn_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  audio_l_dn_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  audio_l_dn_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                     [7]：淡入淡出功能使能
                                                                     1：淡入淡出功能使能
                                                                     0：淡入淡出功能不使能
                                                                     [6]: 抗削波功能使能
                                                                     1：抗削波功能使能
                                                                     0：抗削波功能不使能
                                                                     [5]: 小信号功能使能
                                                                     1：小信号功能使能
                                                                     0：小信号功能不使能 */
        unsigned int  audio_l_dn_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                     例如：60dB配置成0x3C；
                                                                      0dB配置为0x00；
                                                                      -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_bypass_START      (10)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_bypass_END        (10)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_cfg_START         (21)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_cfg_END           (23)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_gain_START        (24)
#define SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_audio_l_dn_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_UNION
 结构说明  : AUDIO_R_DN_PGA_CTRL 寄存器结构定义。地址偏移量:0x005C，初值:0x00000050，宽度:32
 寄存器说明: AUDIO下行通路右声道PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                  : 1;  /* bit[0]    : reserved */
        unsigned int  audio_r_dn_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                     1'b0: 选择对数淡出淡出
                                                                     1'b1: 选择线性淡入淡出 */
        unsigned int  audio_r_dn_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  audio_r_dn_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用
                                                                     其中2bit的id选择四种场景的阈值
                                                                     0：选择pga_thre_ctrl[23:0]
                                                                     1：选择pga_thre_ctrl[47:24]
                                                                     2：选择pga_thre_ctrl[71:48]
                                                                     3：选择pga_thre_ctrl[95:72] */
        unsigned int  audio_r_dn_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的
                                                                     小信号放过去，低表示低于阈值的信号钳制到0
                                                                     1：不对噪音信号做任何处理
                                                                     0：将低于阈值的噪音信号过滤成0 */
        unsigned int  audio_r_dn_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                     1'b0: 不bypass
                                                                     1'b1: bypass */
        unsigned int  audio_r_dn_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  audio_r_dn_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  audio_r_dn_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                     [7]：淡入淡出功能使能
                                                                     1：淡入淡出功能使能
                                                                     0：淡入淡出功能不使能
                                                                     [6]: 抗削波功能使能
                                                                     1：抗削波功能使能
                                                                     0：抗削波功能不使能
                                                                     [5]: 小信号功能使能
                                                                     1：小信号功能使能
                                                                     0：小信号功能不使能 */
        unsigned int  audio_r_dn_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                     例如：60dB配置成0x3C；
                                                                      0dB配置为0x00；
                                                                      -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_bypass_START      (10)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_bypass_END        (10)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_cfg_START         (21)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_cfg_END           (23)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_gain_START        (24)
#define SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_audio_r_dn_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_SIDETONE_PGA_CTRL_UNION
 结构说明  : SIDETONE_PGA_CTRL 寄存器结构定义。地址偏移量:0x0060，初值:0x00000050，宽度:32
 寄存器说明: SIDETONE通路PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                : 1;  /* bit[0]    : reserved */
        unsigned int  sidetone_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                   1'b0: 选择对数淡出淡出
                                                                   1'b1: 选择线性淡入淡出 */
        unsigned int  sidetone_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  sidetone_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用
                                                                   其中2bit的id选择四种场景的阈值
                                                                   0：选择pga_thre_ctrl[23:0]
                                                                   1：选择pga_thre_ctrl[47:24]
                                                                   2：选择pga_thre_ctrl[71:48]
                                                                   3：选择pga_thre_ctrl[95:72] */
        unsigned int  sidetone_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值
                                                                   的小信号放过去，低表示低于阈值的信号钳制到0
                                                                   1：不对噪音信号做任何处理
                                                                   0：将低于阈值的噪音信号过滤成0 */
        unsigned int  sidetone_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                   1'b0: 不bypass
                                                                   1'b1: bypass */
        unsigned int  sidetone_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                   采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  sidetone_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                   采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  sidetone_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                   [7]：淡入淡出功能使能
                                                                   1：淡入淡出功能使能
                                                                   0：淡入淡出功能不使能
                                                                   [6]: 抗削波功能使能
                                                                   1：抗削波功能使能
                                                                   0：抗削波功能不使能
                                                                   [5]: 小信号功能使能
                                                                   1：小信号功能使能
                                                                   0：小信号功能不使能 */
        unsigned int  sidetone_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                   例如：60dB配置成0x3C；
                                                                    0dB配置为0x00；
                                                                    -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_SIDETONE_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_bypass_START      (10)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_bypass_END        (10)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_cfg_START         (21)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_cfg_END           (23)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_gain_START        (24)
#define SOC_ASP_CODEC_SIDETONE_PGA_CTRL_sidetone_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_UNION
 结构说明  : AUDIO_L_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x0064，初值:0x00000050，宽度:32
 寄存器说明: AUDIO上行通路左声道PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                  : 1;  /* bit[0]    : reserved */
        unsigned int  audio_l_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                     1'b0: 选择对数淡出淡出
                                                                     1'b1: 选择线性淡入淡出 */
        unsigned int  audio_l_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  audio_l_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用
                                                                     其中2bit的id选择四种场景的阈值
                                                                     0：选择pga_thre_ctrl[23:0]
                                                                     1：选择pga_thre_ctrl[47:24]
                                                                     2：选择pga_thre_ctrl[71:48]
                                                                     3：选择pga_thre_ctrl[95:72] */
        unsigned int  audio_l_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的
                                                                     小信号放过去，低表示低于阈值的信号钳制到0
                                                                     1：不对噪音信号做任何处理
                                                                     0：将低于阈值的噪音信号过滤成0 */
        unsigned int  audio_l_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                     1'b0: 不bypass
                                                                     1'b1: bypass */
        unsigned int  audio_l_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  audio_l_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  audio_l_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                     [7]：淡入淡出功能使能
                                                                     1：淡入淡出功能使能
                                                                     0：淡入淡出功能不使能
                                                                     [6]: 抗削波功能使能
                                                                     1：抗削波功能使能
                                                                     0：抗削波功能不使能
                                                                     [5]: 小信号功能使能
                                                                     1：小信号功能使能
                                                                     0：小信号功能不使能 */
        unsigned int  audio_l_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                     例如：60dB配置成0x3C；
                                                                      0dB配置为0x00；
                                                                      -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_audio_l_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_UNION
 结构说明  : AUDIO_R_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x0068，初值:0x00000050，宽度:32
 寄存器说明: AUDIO上行通路右声道PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                  : 1;  /* bit[0]    : reserved */
        unsigned int  audio_r_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                     1'b0: 选择对数淡出淡出
                                                                     1'b1: 选择线性淡入淡出 */
        unsigned int  audio_r_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  audio_r_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用
                                                                     其中2bit的id选择四种场景的阈值
                                                                     0：选择pga_thre_ctrl[23:0]
                                                                     1：选择pga_thre_ctrl[47:24]
                                                                     2：选择pga_thre_ctrl[71:48]
                                                                     3：选择pga_thre_ctrl[95:72] */
        unsigned int  audio_r_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的
                                                                     小信号放过去，低表示低于阈值的信号钳制到0
                                                                     1：不对噪音信号做任何处理
                                                                     0：将低于阈值的噪音信号过滤成0 */
        unsigned int  audio_r_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                     1'b0: 不bypass
                                                                     1'b1: bypass */
        unsigned int  audio_r_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  audio_r_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  audio_r_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                     [7]：淡入淡出功能使能
                                                                     1：淡入淡出功能使能
                                                                     0：淡入淡出功能不使能
                                                                     [6]: 抗削波功能使能
                                                                     1：抗削波功能使能
                                                                     0：抗削波功能不使能
                                                                     [5]: 小信号功能使能
                                                                     1：小信号功能使能
                                                                     0：小信号功能不使能 */
        unsigned int  audio_r_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                     例如：60dB配置成0x3C；
                                                                      0dB配置为0x00；
                                                                      -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_audio_r_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_UNION
 结构说明  : VOICE_L_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x006C，初值:0x00000050，宽度:32
 寄存器说明: VOICE上行通路左声道PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                  : 1;  /* bit[0]    : reserved */
        unsigned int  voice_l_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                     1'b0: 选择对数淡出淡出
                                                                     1'b1: 选择线性淡入淡出 */
        unsigned int  voice_l_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  voice_l_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用
                                                                     其中2bit的id选择四种场景的阈值
                                                                     0：选择pga_thre_ctrl[23:0]
                                                                     1：选择pga_thre_ctrl[47:24]
                                                                     2：选择pga_thre_ctrl[71:48]
                                                                     3：选择pga_thre_ctrl[95:72] */
        unsigned int  voice_l_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的
                                                                     小信号放过去，低表示低于阈值的信号钳制到0
                                                                     1：不对噪音信号做任何处理
                                                                     0：将低于阈值的噪音信号过滤成0 */
        unsigned int  voice_l_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                     1'b0: 不bypass
                                                                     1'b1: bypass */
        unsigned int  voice_l_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  voice_l_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  voice_l_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                     [7]：淡入淡出功能使能
                                                                     1：淡入淡出功能使能
                                                                     0：淡入淡出功能不使能
                                                                     [6]: 抗削波功能使能
                                                                     1：抗削波功能使能
                                                                     0：抗削波功能不使能
                                                                     [5]: 小信号功能使能
                                                                     1：小信号功能使能
                                                                     0：小信号功能不使能 */
        unsigned int  voice_l_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                     例如：60dB配置成0x3C；
                                                                      0dB配置为0x00；
                                                                      -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_VOICE_L_UP_PGA_CTRL_voice_l_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_UNION
 结构说明  : VOICE_R_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x0070，初值:0x00000050，宽度:32
 寄存器说明: VOICE上行通路右声道PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                  : 1;  /* bit[0]    : reserved */
        unsigned int  voice_r_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                     1'b0: 选择对数淡出淡出
                                                                     1'b1: 选择线性淡入淡出 */
        unsigned int  voice_r_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  voice_r_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使
                                                                     用其中2bit的id选择四种场景的阈值
                                                                     0：选择pga_thre_ctrl[23:0]
                                                                     1：选择pga_thre_ctrl[47:24]
                                                                     2：选择pga_thre_ctrl[71:48]
                                                                     3：选择pga_thre_ctrl[95:72] */
        unsigned int  voice_r_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的
                                                                     小信号放过去，低表示低于阈值的信号钳制到0
                                                                     1：不对噪音信号做任何处理
                                                                     0：将低于阈值的噪音信号过滤成0 */
        unsigned int  voice_r_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                     1'b0: 不bypass
                                                                     1'b1: bypass */
        unsigned int  voice_r_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  voice_r_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  voice_r_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                     [7]：淡入淡出功能使能
                                                                     1：淡入淡出功能使能
                                                                     0：淡入淡出功能不使能
                                                                     [6]: 抗削波功能使能
                                                                     1：抗削波功能使能
                                                                     0：抗削波功能不使能
                                                                     [5]: 小信号功能使能
                                                                     1：小信号功能使能
                                                                     0：小信号功能不使能 */
        unsigned int  voice_r_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                     例如：60dB配置成0x3C；
                                                                      0dB配置为0x00；
                                                                      -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_VOICE_R_UP_PGA_CTRL_voice_r_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_UNION
 结构说明  : MIC3_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x0074，初值:0x00000050，宽度:32
 寄存器说明: MIC3上行PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  mic3_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  mic3_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  mic3_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用
                                                                  其中2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  mic3_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的
                                                                  小信号放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  mic3_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  mic3_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                  采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mic3_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                  采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mic3_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  mic3_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                  例如：60dB配置成0x3C；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_MIC3_UP_PGA_CTRL_mic3_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_UNION
 结构说明  : MIC4_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x0078，初值:0x00000050，宽度:32
 寄存器说明: MIC4上行PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  mic4_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  mic4_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  mic4_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中
                                                                  2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  mic4_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小
                                                                  信号放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  mic4_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  mic4_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                  采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mic4_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                  采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mic4_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  mic4_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                  例如：60dB配置成0x3C；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_MIC4_UP_PGA_CTRL_mic4_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_UNION
 结构说明  : MDM_5G_L_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x007C，初值:0x00000050，宽度:32
 寄存器说明: MDM_5G上行通路左声道PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                   : 1;  /* bit[0]    : reserved */
        unsigned int  mdm_5g_l_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                      1'b0: 选择对数淡出淡出
                                                                      1'b1: 选择线性淡入淡出 */
        unsigned int  mdm_5g_l_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  mdm_5g_l_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga
                                                                      通过使用其中2bit的id选择四种场景的阈值
                                                                      0：选择pga_thre_ctrl[23:0]
                                                                      1：选择pga_thre_ctrl[47:24]
                                                                      2：选择pga_thre_ctrl[71:48]
                                                                      3：选择pga_thre_ctrl[95:72] */
        unsigned int  mdm_5g_l_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的
                                                                      小信号放过去，低表示低于阈值的信号钳制到0
                                                                      1：不对噪音信号做任何处理
                                                                      0：将低于阈值的噪音信号过滤成0 */
        unsigned int  mdm_5g_l_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                      1'b0: 不bypass
                                                                      1'b1: bypass */
        unsigned int  mdm_5g_l_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                      采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mdm_5g_l_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                      采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mdm_5g_l_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                      [7]：淡入淡出功能使能
                                                                      1：淡入淡出功能使能
                                                                      0：淡入淡出功能不使能
                                                                      [6]: 抗削波功能使能
                                                                      1：抗削波功能使能
                                                                      0：抗削波功能不使能
                                                                      [5]: 小信号功能使能
                                                                      1：小信号功能使能
                                                                      0：小信号功能不使能 */
        unsigned int  mdm_5g_l_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                      例如：60dB配置成0x3C；
                                                                       0dB配置为0x00；
                                                                       -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_mdm_5g_l_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_UNION
 结构说明  : MDM_5G_R_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x0080，初值:0x00000050，宽度:32
 寄存器说明: MDM_5G上行通路右声道PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                   : 1;  /* bit[0]    : reserved */
        unsigned int  mdm_5g_r_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                      1'b0: 选择对数淡出淡出
                                                                      1'b1: 选择线性淡入淡出 */
        unsigned int  mdm_5g_r_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  mdm_5g_r_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用
                                                                      其中2bit的id选择四种场景的阈值
                                                                      0：选择pga_thre_ctrl[23:0]
                                                                      1：选择pga_thre_ctrl[47:24]
                                                                      2：选择pga_thre_ctrl[71:48]
                                                                      3：选择pga_thre_ctrl[95:72] */
        unsigned int  mdm_5g_r_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的
                                                                      小信号放过去，低表示低于阈值的信号钳制到0
                                                                      1：不对噪音信号做任何处理
                                                                      0：将低于阈值的噪音信号过滤成0 */
        unsigned int  mdm_5g_r_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                      1'b0: 不bypass
                                                                      1'b1: bypass */
        unsigned int  mdm_5g_r_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                      采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mdm_5g_r_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                      采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mdm_5g_r_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                      [7]：淡入淡出功能使能
                                                                      1：淡入淡出功能使能
                                                                      0：淡入淡出功能不使能
                                                                      [6]: 抗削波功能使能
                                                                      1：抗削波功能使能
                                                                      0：抗削波功能不使能
                                                                      [5]: 小信号功能使能
                                                                      1：小信号功能使能
                                                                      0：小信号功能不使能 */
        unsigned int  mdm_5g_r_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                      例如：60dB配置成0x3C；
                                                                       0dB配置为0x00；
                                                                       -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_mdm_5g_r_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_UNION
 结构说明  : I2S2_L_RX_PGA_CTRL 寄存器结构定义。地址偏移量:0x0084，初值:0x00000050，宽度:32
 寄存器说明: I2S2 RX L PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                 : 1;  /* bit[0]    : reserved */
        unsigned int  i2s2_l_rx_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                    1'b0: 选择对数淡出淡出
                                                                    1'b1: 选择线性淡入淡出 */
        unsigned int  i2s2_l_rx_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  i2s2_l_rx_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用
                                                                  其中2bit的id选择四种场景的阈值
                                                                    0：选择pga_thre_ctrl[23:0]
                                                                    1：选择pga_thre_ctrl[47:24]
                                                                    2：选择pga_thre_ctrl[71:48]
                                                                    3：选择pga_thre_ctrl[95:72] */
        unsigned int  i2s2_l_rx_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小
                                                                    信号放过去，低表示低于阈值的信号钳制到0
                                                                    1：不对噪音信号做任何处理
                                                                    0：将低于阈值的噪音信号过滤成0 */
        unsigned int  i2s2_l_rx_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                    1'b0: 不bypass
                                                                    1'b1: bypass */
        unsigned int  i2s2_l_rx_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                    采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  i2s2_l_rx_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                    采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  i2s2_l_rx_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                    [7]：淡入淡出功能使能
                                                                    1：淡入淡出功能使能
                                                                    0：淡入淡出功能不使能
                                                                    [6]: 抗削波功能使能
                                                                    1：抗削波功能使能
                                                                    0：抗削波功能不使能
                                                                    [5]: 小信号功能使能
                                                                    1：小信号功能使能
                                                                    0：小信号功能不使能 */
        unsigned int  i2s2_l_rx_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                    例如：60dB配置成0x3C；
                                                                     0dB配置为0x00；
                                                                     -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_bypass_START      (10)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_bypass_END        (10)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_cfg_START         (21)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_cfg_END           (23)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_gain_START        (24)
#define SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_i2s2_l_rx_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_UNION
 结构说明  : I2S2_R_RX_PGA_CTRL 寄存器结构定义。地址偏移量:0x0088，初值:0x00000050，宽度:32
 寄存器说明: I2S2 RX R PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                 : 1;  /* bit[0]    : reserved */
        unsigned int  i2s2_r_rx_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                    1'b0: 选择对数淡出淡出
                                                                    1'b1: 选择线性淡入淡出 */
        unsigned int  i2s2_r_rx_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  i2s2_r_rx_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用
                                                                    其中2bit的id选择四种场景的阈值
                                                                    0：选择pga_thre_ctrl[23:0]
                                                                    1：选择pga_thre_ctrl[47:24]
                                                                    2：选择pga_thre_ctrl[71:48]
                                                                    3：选择pga_thre_ctrl[95:72] */
        unsigned int  i2s2_r_rx_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小
                                                                    信号放过去，低表示低于阈值的信号钳制到0
                                                                    1：不对噪音信号做任何处理
                                                                    0：将低于阈值的噪音信号过滤成0 */
        unsigned int  i2s2_r_rx_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                    1'b0: 不bypass
                                                                    1'b1: bypass */
        unsigned int  i2s2_r_rx_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                    采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  i2s2_r_rx_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                    采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  i2s2_r_rx_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                    [7]：淡入淡出功能使能
                                                                    1：淡入淡出功能使能
                                                                    0：淡入淡出功能不使能
                                                                    [6]: 抗削波功能使能
                                                                    1：抗削波功能使能
                                                                    0：抗削波功能不使能
                                                                    [5]: 小信号功能使能
                                                                    1：小信号功能使能
                                                                    0：小信号功能不使能 */
        unsigned int  i2s2_r_rx_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                    例如：60dB配置成0x3C；
                                                                     0dB配置为0x00；
                                                                     -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_bypass_START      (10)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_bypass_END        (10)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_cfg_START         (21)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_cfg_END           (23)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_gain_START        (24)
#define SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_i2s2_r_rx_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_UNION
 结构说明  : ADC0_PGA_GAIN_OFFSET 寄存器结构定义。地址偏移量:0x008C，初值:0x00000000，宽度:32
 寄存器说明: 0p5 PGA控制寄存器0
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc1_pga_gain_offset : 8;  /* bit[0-7]  : adc1_pga增益偏置
                                                                对于step=1dB的PGA，gainoffset有效配置范围是0~255，
                                                               对于step=0.5dB的PGA，gainoffset有效配置范围是0~127； */
        unsigned int  adc2_pga_gain_offset : 8;  /* bit[8-15] : adc2_pga增益偏置
                                                                对于step=1dB的PGA，gainoffset有效配置范围是0~255，
                                                                对于step=0.5dB的PGA，gainoffset有效配置范围是0~127； */
        unsigned int  adc3_pga_gain_offset : 8;  /* bit[16-23]: adc3_pga增益偏置
                                                                对于step=1dB的PGA，gainoffset有效配置范围是0~255，
                                                                对于step=0.5dB的PGA，gainoffset有效配置范围是0~127； */
        unsigned int  adc4_pga_gain_offset : 8;  /* bit[24-31]: adc4_pga增益偏置
                                                                对于step=1dB的PGA，gainoffset有效配置范围是0~255，
                                                                对于step=0.5dB的PGA，gainoffset有效配置范围是0~127； */
    } reg;
} SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_adc1_pga_gain_offset_START  (0)
#define SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_adc1_pga_gain_offset_END    (7)
#define SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_adc2_pga_gain_offset_START  (8)
#define SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_adc2_pga_gain_offset_END    (15)
#define SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_adc3_pga_gain_offset_START  (16)
#define SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_adc3_pga_gain_offset_END    (23)
#define SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_adc4_pga_gain_offset_START  (24)
#define SOC_ASP_CODEC_ADC0_PGA_GAIN_OFFSET_adc4_pga_gain_offset_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_UNION
 结构说明  : ADC1_PGA_GAIN_OFFSET 寄存器结构定义。地址偏移量:0x0090，初值:0x00000000，宽度:32
 寄存器说明: 0p5 PGA控制寄存器1
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc5_pga_gain_offset : 8;  /* bit[0-7]  : adc5_pga增益偏置
                                                                对于step=1dB的PGA，gainoffset有效配置范围是0~255，
                                                                对于step=0.5dB的PGA，gainoffset有效配置范围是0~127； */
        unsigned int  adc6_pga_gain_offset : 8;  /* bit[8-15] : adc6_pga增益偏置
                                                                对于step=1dB的PGA，gainoffset有效配置范围是0~255，
                                                                对于step=0.5dB的PGA，gainoffset有效配置范围是0~127； */
        unsigned int  adc7_pga_gain_offset : 8;  /* bit[16-23]: adc7_pga增益偏置
                                                                对于step=1dB的PGA，gainoffset有效配置范围是0~255，
                                                                对于step=0.5dB的PGA，gainoffset有效配置范围是0~127； */
        unsigned int  adc8_pga_gain_offset : 8;  /* bit[24-31]: adc8_pga增益偏置
                                                                对于step=1dB的PGA，gainoffset有效配置范围是0~255，
                                                                对于step=0.5dB的PGA，gainoffset有效配置范围是0~127； */
    } reg;
} SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_adc5_pga_gain_offset_START  (0)
#define SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_adc5_pga_gain_offset_END    (7)
#define SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_adc6_pga_gain_offset_START  (8)
#define SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_adc6_pga_gain_offset_END    (15)
#define SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_adc7_pga_gain_offset_START  (16)
#define SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_adc7_pga_gain_offset_END    (23)
#define SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_adc8_pga_gain_offset_START  (24)
#define SOC_ASP_CODEC_ADC1_PGA_GAIN_OFFSET_adc8_pga_gain_offset_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_UNION
 结构说明  : ADC1_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x0094，初值:0x00000050，宽度:32
 寄存器说明: ADC1上行通路PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  adc1_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  adc1_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  adc1_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中
                                                                  2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  adc1_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的
                                                                  小信号放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  adc1_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  adc1_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、采样率相关，
                                                                  详细对应关系请参考用户手册。 */
        unsigned int  adc1_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                  采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc1_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  adc1_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~3dB（bit7为符号位）
                                                                  例如：3dB配置成0x3；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_ADC1_UP_PGA_CTRL_adc1_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_UNION
 结构说明  : ADC2_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x0098，初值:0x00000050，宽度:32
 寄存器说明: ADC2上行通路PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  adc2_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  adc2_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  adc2_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中
                                                                  2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  adc2_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小信号
                                                                  放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  adc2_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  adc2_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                  采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc2_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                  采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc2_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  adc2_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~3dB（bit7为符号位）
                                                                  例如：3dB配置成0x3；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_ADC2_UP_PGA_CTRL_adc2_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_UNION
 结构说明  : ADC3_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x009C，初值:0x00000050，宽度:32
 寄存器说明: ADC3上行通路PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  adc3_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  adc3_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  adc3_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中
                                                                  2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  adc3_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小
                                                                  信号放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  adc3_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  adc3_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                  采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc3_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                  采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc3_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  adc3_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~3dB（bit7为符号位）
                                                                  例如：3dB配置成0x3；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_ADC3_UP_PGA_CTRL_adc3_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_UNION
 结构说明  : ADC4_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x00A0，初值:0x00000050，宽度:32
 寄存器说明: ADC4上行通路PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  adc4_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  adc4_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  adc4_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中
                                                                  2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  adc4_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小
                                                                  信号放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  adc4_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  adc4_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                  采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc4_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                  采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc4_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  adc4_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~3dB（bit7为符号位）
                                                                  例如：3dB配置成0x3；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_ADC4_UP_PGA_CTRL_adc4_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_UNION
 结构说明  : ADC5_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x00A4，初值:0x00000050，宽度:32
 寄存器说明: ADC5上行通路PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  adc5_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  adc5_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  adc5_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中
                                                                  2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  adc5_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小信号
                                                                  放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  adc5_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  adc5_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                  采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc5_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                  采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc5_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  adc5_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~3dB（bit7为符号位）
                                                                  例如：3dB配置成0x3；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_ADC5_UP_PGA_CTRL_adc5_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_SRCUP_CTRL_UNION
 结构说明  : SRCUP_CTRL 寄存器结构定义。地址偏移量:0x00A8，初值:0x00000000，宽度:32
 寄存器说明: SRCUP控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  codec3_l_dn_srcup_src_mode  : 3;  /* bit[0-2]  : SRCUP升采样倍数配置:
                                                                       3'b111: 保留;
                                                                       3'b101: 6倍升采样；
                                                                       3'b100: 2倍升采样；
                                                                       3'b001: 3倍升采样;
                                                                       3'b010: 1.5倍升采样.
                                                                       others: bypass. */
        unsigned int  codec3_l_dn_srcup_fifo_clr  : 1;  /* bit[3]    : FIFO Clear Signal, high active.
                                                                       0：不清空FIFO数据
                                                                       1：清空FIFO数据 */
        unsigned int  codec3_r_dn_srcup_src_mode  : 3;  /* bit[4-6]  : SRCUP升采样倍数配置:
                                                                       3'b111: 保留；
                                                                       3'b101: 6倍升采样；
                                                                       3'b100: 2倍升采样；
                                                                       3'b001: 3倍升采样;
                                                                       3'b010: 1.5倍升采样.
                                                                       others: bypass. */
        unsigned int  codec3_r_dn_srcup_fifo_clr  : 1;  /* bit[7]    : FIFO Clear Signal, high active.
                                                                       0：不清空FIFO数据
                                                                       1：清空FIFO数据 */
        unsigned int  audio_l_dn_srcup_src_mode   : 3;  /* bit[8-10] : SRCUP升采样倍数配置:
                                                                       3'b111: 保留；
                                                                       3'b101: 6倍升采样；
                                                                       3'b100: 2倍升采样；
                                                                       3'b001: 3倍升采样;
                                                                       3'b010: 1.5倍升采样.
                                                                       others: bypass. */
        unsigned int  audio_l_dn_srcup_fifo_clr   : 1;  /* bit[11]   : FIFO Clear Signal, high active.
                                                                       0：不清空FIFO数据
                                                                       1：清空FIFO数据 */
        unsigned int  audio_r_dn_srcup_src_mode   : 3;  /* bit[12-14]: SRCUP升采样倍数配置:
                                                                       3'b111: 保留；
                                                                       3'b101: 6倍升采样；
                                                                       3'b100: 保留；
                                                                       3'b001: 3倍升采样;
                                                                       3'b010: 1.5倍升采样.
                                                                       others: bypass. */
        unsigned int  audio_r_dn_srcup_fifo_clr   : 1;  /* bit[15]   : FIFO Clear Signal, high active.
                                                                       0：不清空FIFO数据
                                                                       1：清空FIFO数据 */
        unsigned int  i2s2_rx_l_srcup_src_mode    : 3;  /* bit[16-18]: SRCUP升采样倍数配置:
                                                                       3'b111: 保留；
                                                                       3'b101: 6倍升采样；
                                                                       3'b100: 保留；
                                                                       3'b001: 3倍升采样;
                                                                       3'b010: 1.5倍升采样.
                                                                       others: bypass. */
        unsigned int  i2s2_rx_l_up_srcup_fifo_clr : 1;  /* bit[19]   : FIFO Clear Signal, high active.
                                                                       0：不清空FIFO数据
                                                                       1：清空FIFO数据 */
        unsigned int  i2s2_rx_r_srcup_src_mode    : 3;  /* bit[20-22]: SRCUP升采样倍数配置:
                                                                       3'b111: 12倍升采样；
                                                                       3'b101: 6倍升采样；
                                                                       3'b100: 2倍升采样；
                                                                       3'b001: 3倍升采样;
                                                                       3'b010: 保留；
                                                                       others: bypass. */
        unsigned int  i2s2_rx_r_up_srcup_fifo_clr : 1;  /* bit[23]   : FIFO Clear Signal, high active.
                                                                       0：不清空FIFO数据
                                                                       1：清空FIFO数据 */
        unsigned int  dacl_mixer4_srcup_src_mode  : 3;  /* bit[24-26]: SRCUP升采样倍数配置:
                                                                       3'b111: 12倍升采样；
                                                                       3'b101: 6倍升采样；
                                                                       3'b100: 2倍升采样；
                                                                       3'b001: 3倍升采样;
                                                                       3'b010: 保留.
                                                                       others: bypass. */
        unsigned int  dacl_mixer4_srcup_fifo_clr  : 1;  /* bit[27]   : FIFO Clear Signal, high active.
                                                                       0：不清空FIFO数据
                                                                       1：清空FIFO数据 */
        unsigned int  dacr_mixer4_srcup_src_mode  : 3;  /* bit[28-30]: SRCUP升采样倍数配置:
                                                                       3'b111: 保留;
                                                                       3'b101: 6倍升采样；
                                                                       3'b100: 2倍升采样；
                                                                       3'b001: 3倍升采样;
                                                                       3'b010: 1.5倍升采样.
                                                                       others: bypass. */
        unsigned int  dacr_mixer4_srcup_fifo_clr  : 1;  /* bit[31]   : FIFO Clear Signal, high active.
                                                                       0：不清空FIFO数据
                                                                       1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_SRCUP_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_SRCUP_CTRL_codec3_l_dn_srcup_src_mode_START   (0)
#define SOC_ASP_CODEC_SRCUP_CTRL_codec3_l_dn_srcup_src_mode_END     (2)
#define SOC_ASP_CODEC_SRCUP_CTRL_codec3_l_dn_srcup_fifo_clr_START   (3)
#define SOC_ASP_CODEC_SRCUP_CTRL_codec3_l_dn_srcup_fifo_clr_END     (3)
#define SOC_ASP_CODEC_SRCUP_CTRL_codec3_r_dn_srcup_src_mode_START   (4)
#define SOC_ASP_CODEC_SRCUP_CTRL_codec3_r_dn_srcup_src_mode_END     (6)
#define SOC_ASP_CODEC_SRCUP_CTRL_codec3_r_dn_srcup_fifo_clr_START   (7)
#define SOC_ASP_CODEC_SRCUP_CTRL_codec3_r_dn_srcup_fifo_clr_END     (7)
#define SOC_ASP_CODEC_SRCUP_CTRL_audio_l_dn_srcup_src_mode_START    (8)
#define SOC_ASP_CODEC_SRCUP_CTRL_audio_l_dn_srcup_src_mode_END      (10)
#define SOC_ASP_CODEC_SRCUP_CTRL_audio_l_dn_srcup_fifo_clr_START    (11)
#define SOC_ASP_CODEC_SRCUP_CTRL_audio_l_dn_srcup_fifo_clr_END      (11)
#define SOC_ASP_CODEC_SRCUP_CTRL_audio_r_dn_srcup_src_mode_START    (12)
#define SOC_ASP_CODEC_SRCUP_CTRL_audio_r_dn_srcup_src_mode_END      (14)
#define SOC_ASP_CODEC_SRCUP_CTRL_audio_r_dn_srcup_fifo_clr_START    (15)
#define SOC_ASP_CODEC_SRCUP_CTRL_audio_r_dn_srcup_fifo_clr_END      (15)
#define SOC_ASP_CODEC_SRCUP_CTRL_i2s2_rx_l_srcup_src_mode_START     (16)
#define SOC_ASP_CODEC_SRCUP_CTRL_i2s2_rx_l_srcup_src_mode_END       (18)
#define SOC_ASP_CODEC_SRCUP_CTRL_i2s2_rx_l_up_srcup_fifo_clr_START  (19)
#define SOC_ASP_CODEC_SRCUP_CTRL_i2s2_rx_l_up_srcup_fifo_clr_END    (19)
#define SOC_ASP_CODEC_SRCUP_CTRL_i2s2_rx_r_srcup_src_mode_START     (20)
#define SOC_ASP_CODEC_SRCUP_CTRL_i2s2_rx_r_srcup_src_mode_END       (22)
#define SOC_ASP_CODEC_SRCUP_CTRL_i2s2_rx_r_up_srcup_fifo_clr_START  (23)
#define SOC_ASP_CODEC_SRCUP_CTRL_i2s2_rx_r_up_srcup_fifo_clr_END    (23)
#define SOC_ASP_CODEC_SRCUP_CTRL_dacl_mixer4_srcup_src_mode_START   (24)
#define SOC_ASP_CODEC_SRCUP_CTRL_dacl_mixer4_srcup_src_mode_END     (26)
#define SOC_ASP_CODEC_SRCUP_CTRL_dacl_mixer4_srcup_fifo_clr_START   (27)
#define SOC_ASP_CODEC_SRCUP_CTRL_dacl_mixer4_srcup_fifo_clr_END     (27)
#define SOC_ASP_CODEC_SRCUP_CTRL_dacr_mixer4_srcup_src_mode_START   (28)
#define SOC_ASP_CODEC_SRCUP_CTRL_dacr_mixer4_srcup_src_mode_END     (30)
#define SOC_ASP_CODEC_SRCUP_CTRL_dacr_mixer4_srcup_fifo_clr_START   (31)
#define SOC_ASP_CODEC_SRCUP_CTRL_dacr_mixer4_srcup_fifo_clr_END     (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_SRCDN_CTRL0_UNION
 结构说明  : SRCDN_CTRL0 寄存器结构定义。地址偏移量:0x00AC，初值:0x00000033，宽度:32
 寄存器说明: SRCDN控制寄存器0
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  audio_l_up_srcdn_src_mode  : 3;  /* bit[0-2]  : SRC降采样抽取倍数配置
                                                                      3'b000: 3倍抽取；
                                                                      3'b001: 保留；
                                                                      3'b010: 6倍抽取；
                                                                      3'b011: 2倍抽取;
                                                                      3'b100: 1.5倍抽取;
                                                                      others: 保留. */
        unsigned int  audio_l_up_srcdn_fifo_clr  : 1;  /* bit[3]    : FIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
        unsigned int  audio_r_up_srcdn_src_mode  : 3;  /* bit[4-6]  : SRC降采样抽取倍数配置
                                                                      3'b000: 3倍抽取；
                                                                      3'b001: 保留；
                                                                      3'b010: 6倍抽取；
                                                                      3'b011: 2倍抽取;
                                                                      3'b100: 1.5倍抽取;
                                                                      others: 保留. */
        unsigned int  audio_r_up_srcdn_fifo_clr  : 1;  /* bit[7]    : FIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
        unsigned int  voice_l_up_srcdn_src_mode  : 3;  /* bit[8-10] : SRC降采样抽取倍数配置
                                                                      3'b000: 3倍抽取；
                                                                      3'b001: 保留；
                                                                      3'b010: 6倍抽取；
                                                                      3'b011: 2倍抽取;
                                                                      3'b100: 1.5倍抽取;
                                                                      others: 保留. */
        unsigned int  voice_l_up_srcdn_fifo_clr  : 1;  /* bit[11]   : FIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
        unsigned int  voice_r_up_srcdn_src_mode  : 3;  /* bit[12-14]: SRC降采样抽取倍数配置
                                                                      3'b000: 3倍抽取；
                                                                      3'b001: 保留；
                                                                      3'b010: 6倍抽取；
                                                                      3'b011: 2倍抽取;
                                                                      3'b100: 1.5倍抽取;
                                                                      others: 保留. */
        unsigned int  voice_r_up_srcdn_fifo_clr  : 1;  /* bit[15]   : FIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
        unsigned int  mic3_up_srcdn_src_mode     : 3;  /* bit[16-18]: SRC降采样抽取倍数配置
                                                                      3'b000: 3倍抽取；
                                                                      3'b001: 保留；
                                                                      3'b010: 6倍抽取；
                                                                      3'b011: 2倍抽取;
                                                                      3'b100: 1.5倍抽取;
                                                                      others: 保留. */
        unsigned int  mic3_up_srcdn_fifo_clr     : 1;  /* bit[19]   : FIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
        unsigned int  mic4_up_srcdn_src_mode     : 3;  /* bit[20-22]: SRC降采样抽取倍数配置
                                                                      3'b000: 3倍抽取；
                                                                      3'b001: 保留；
                                                                      3'b010: 6倍抽取；
                                                                      3'b011: 2倍抽取;
                                                                      3'b100: 1.5倍抽取;
                                                                      others: 保留. */
        unsigned int  mic4_up_srcdn_fifo_clr     : 1;  /* bit[23]   : FIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
        unsigned int  mdm_5g_l_up_srcdn_src_mode : 3;  /* bit[24-26]: SRC降采样抽取倍数配置
                                                                      3'b000: 3倍抽取；
                                                                      3'b001: 保留；
                                                                      3'b010: 6倍抽取；
                                                                      3'b011: 2倍抽取;
                                                                      3'b100: 1.5倍抽取;
                                                                      others: 保留. */
        unsigned int  mdm_5g_l_up_srcdn_fifo_clr : 1;  /* bit[27]   : FIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
        unsigned int  mdm_5g_r_up_srcdn_src_mode : 3;  /* bit[28-30]: SRC降采样抽取倍数配置
                                                                      3'b000: 3倍抽取；
                                                                      3'b001: 保留；
                                                                      3'b010: 6倍抽取；
                                                                      3'b011: 2倍抽取;
                                                                      3'b100: 1.5倍抽取;
                                                                      others: 保留. */
        unsigned int  mdm_5g_r_up_srcdn_fifo_clr : 1;  /* bit[31]   : FIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_SRCDN_CTRL0_UNION;
#endif
#define SOC_ASP_CODEC_SRCDN_CTRL0_audio_l_up_srcdn_src_mode_START   (0)
#define SOC_ASP_CODEC_SRCDN_CTRL0_audio_l_up_srcdn_src_mode_END     (2)
#define SOC_ASP_CODEC_SRCDN_CTRL0_audio_l_up_srcdn_fifo_clr_START   (3)
#define SOC_ASP_CODEC_SRCDN_CTRL0_audio_l_up_srcdn_fifo_clr_END     (3)
#define SOC_ASP_CODEC_SRCDN_CTRL0_audio_r_up_srcdn_src_mode_START   (4)
#define SOC_ASP_CODEC_SRCDN_CTRL0_audio_r_up_srcdn_src_mode_END     (6)
#define SOC_ASP_CODEC_SRCDN_CTRL0_audio_r_up_srcdn_fifo_clr_START   (7)
#define SOC_ASP_CODEC_SRCDN_CTRL0_audio_r_up_srcdn_fifo_clr_END     (7)
#define SOC_ASP_CODEC_SRCDN_CTRL0_voice_l_up_srcdn_src_mode_START   (8)
#define SOC_ASP_CODEC_SRCDN_CTRL0_voice_l_up_srcdn_src_mode_END     (10)
#define SOC_ASP_CODEC_SRCDN_CTRL0_voice_l_up_srcdn_fifo_clr_START   (11)
#define SOC_ASP_CODEC_SRCDN_CTRL0_voice_l_up_srcdn_fifo_clr_END     (11)
#define SOC_ASP_CODEC_SRCDN_CTRL0_voice_r_up_srcdn_src_mode_START   (12)
#define SOC_ASP_CODEC_SRCDN_CTRL0_voice_r_up_srcdn_src_mode_END     (14)
#define SOC_ASP_CODEC_SRCDN_CTRL0_voice_r_up_srcdn_fifo_clr_START   (15)
#define SOC_ASP_CODEC_SRCDN_CTRL0_voice_r_up_srcdn_fifo_clr_END     (15)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mic3_up_srcdn_src_mode_START      (16)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mic3_up_srcdn_src_mode_END        (18)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mic3_up_srcdn_fifo_clr_START      (19)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mic3_up_srcdn_fifo_clr_END        (19)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mic4_up_srcdn_src_mode_START      (20)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mic4_up_srcdn_src_mode_END        (22)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mic4_up_srcdn_fifo_clr_START      (23)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mic4_up_srcdn_fifo_clr_END        (23)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mdm_5g_l_up_srcdn_src_mode_START  (24)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mdm_5g_l_up_srcdn_src_mode_END    (26)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mdm_5g_l_up_srcdn_fifo_clr_START  (27)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mdm_5g_l_up_srcdn_fifo_clr_END    (27)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mdm_5g_r_up_srcdn_src_mode_START  (28)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mdm_5g_r_up_srcdn_src_mode_END    (30)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mdm_5g_r_up_srcdn_fifo_clr_START  (31)
#define SOC_ASP_CODEC_SRCDN_CTRL0_mdm_5g_r_up_srcdn_fifo_clr_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_SRCDN_CTRL1_UNION
 结构说明  : SRCDN_CTRL1 寄存器结构定义。地址偏移量:0x00B0，初值:0x00003344，宽度:32
 寄存器说明: SRCDN控制寄存器1
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  i2s2_rx_l_srcdn_src_mode : 3;  /* bit[0-2]  : SRC降采样抽取倍数配置
                                                                    3'b000: 3倍抽取；
                                                                    3'b001: 保留；
                                                                    3'b010: 6倍抽取；
                                                                    3'b011: 2倍抽取;
                                                                    3'b100: 1.5倍抽取;
                                                                    others: 保留. */
        unsigned int  i2s2_rx_l_srcdn_fifo_clr : 1;  /* bit[3]    : FIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
        unsigned int  i2s2_rx_r_srcdn_src_mode : 3;  /* bit[4-6]  : SRC降采样抽取倍数配置
                                                                    3'b000: 3倍抽取；
                                                                    3'b001: 保留；
                                                                    3'b010: 6倍抽取；
                                                                    3'b011: 2倍抽取;
                                                                    3'b100: 1.5倍抽取;
                                                                    others: 保留. */
        unsigned int  i2s2_rx_r_srcdn_fifo_clr : 1;  /* bit[7]    : FIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
        unsigned int  i2s2_tx_l_srcdn_src_mode : 3;  /* bit[8-10] : SRC降采样抽取倍数配置
                                                                    3'b000: 3倍抽取；
                                                                    3'b001: 保留；
                                                                    3'b010: 6倍抽取；
                                                                    3'b011: 2倍抽取;
                                                                    3'b100: 1.5倍抽取;
                                                                    others: 保留. */
        unsigned int  i2s2_tx_l_srcdn_fifo_clr : 1;  /* bit[11]   : FIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
        unsigned int  i2s2_tx_r_srcdn_src_mode : 3;  /* bit[12-14]: SRC降采样抽取倍数配置
                                                                    3'b000: 3倍抽取；
                                                                    3'b001: 保留；
                                                                    3'b010: 6倍抽取；
                                                                    3'b011: 2倍抽取;
                                                                    3'b100: 1.5倍抽取;
                                                                    others: 保留. */
        unsigned int  i2s2_tx_r_srcdn_fifo_clr : 1;  /* bit[15]   : FIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
        unsigned int  mic5_up_srcdn_src_mode   : 3;  /* bit[16-18]: SRC降采样抽取倍数配置
                                                                    3'b000: 3倍抽取；
                                                                    3'b001: 保留；
                                                                    3'b010: 6倍抽取；
                                                                    3'b011: 2倍抽取;
                                                                    3'b100: 1.5倍抽取;
                                                                    others: 保留. */
        unsigned int  mic5_up_srcdn_fifo_clr   : 1;  /* bit[19]   : FIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
        unsigned int  mic6_up_srcdn_src_mode   : 3;  /* bit[20-22]: SRC降采样抽取倍数配置
                                                                    3'b000: 3倍抽取；
                                                                    3'b001: 保留；
                                                                    3'b010: 6倍抽取；
                                                                    3'b011: 2倍抽取;
                                                                    3'b100: 1.5倍抽取;
                                                                    others: 保留. */
        unsigned int  mic6_up_srcdn_fifo_clr   : 1;  /* bit[23]   : FIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
        unsigned int  mic7_up_srcdn_src_mode   : 3;  /* bit[24-26]: SRC降采样抽取倍数配置
                                                                    3'b000: 3倍抽取；
                                                                    3'b001: 保留；
                                                                    3'b010: 6倍抽取；
                                                                    3'b011: 2倍抽取;
                                                                    3'b100: 1.5倍抽取;
                                                                    others: 保留. */
        unsigned int  mic7_up_srcdn_fifo_clr   : 1;  /* bit[27]   : FIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
        unsigned int  mic8_up_srcdn_src_mode   : 3;  /* bit[28-30]: SRC降采样抽取倍数配置
                                                                    3'b000: 3倍抽取；
                                                                    3'b001: 保留；
                                                                    3'b010: 6倍抽取；
                                                                    3'b011: 2倍抽取;
                                                                    3'b100: 1.5倍抽取;
                                                                    others: 保留. */
        unsigned int  mic8_up_srcdn_fifo_clr   : 1;  /* bit[31]   : FIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_SRCDN_CTRL1_UNION;
#endif
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_rx_l_srcdn_src_mode_START  (0)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_rx_l_srcdn_src_mode_END    (2)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_rx_l_srcdn_fifo_clr_START  (3)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_rx_l_srcdn_fifo_clr_END    (3)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_rx_r_srcdn_src_mode_START  (4)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_rx_r_srcdn_src_mode_END    (6)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_rx_r_srcdn_fifo_clr_START  (7)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_rx_r_srcdn_fifo_clr_END    (7)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_tx_l_srcdn_src_mode_START  (8)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_tx_l_srcdn_src_mode_END    (10)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_tx_l_srcdn_fifo_clr_START  (11)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_tx_l_srcdn_fifo_clr_END    (11)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_tx_r_srcdn_src_mode_START  (12)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_tx_r_srcdn_src_mode_END    (14)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_tx_r_srcdn_fifo_clr_START  (15)
#define SOC_ASP_CODEC_SRCDN_CTRL1_i2s2_tx_r_srcdn_fifo_clr_END    (15)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic5_up_srcdn_src_mode_START    (16)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic5_up_srcdn_src_mode_END      (18)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic5_up_srcdn_fifo_clr_START    (19)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic5_up_srcdn_fifo_clr_END      (19)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic6_up_srcdn_src_mode_START    (20)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic6_up_srcdn_src_mode_END      (22)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic6_up_srcdn_fifo_clr_START    (23)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic6_up_srcdn_fifo_clr_END      (23)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic7_up_srcdn_src_mode_START    (24)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic7_up_srcdn_src_mode_END      (26)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic7_up_srcdn_fifo_clr_START    (27)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic7_up_srcdn_fifo_clr_END      (27)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic8_up_srcdn_src_mode_START    (28)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic8_up_srcdn_src_mode_END      (30)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic8_up_srcdn_fifo_clr_START    (31)
#define SOC_ASP_CODEC_SRCDN_CTRL1_mic8_up_srcdn_fifo_clr_END      (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_DACL_MIXER4_CTRL0_UNION
 结构说明  : DACL_MIXER4_CTRL0 寄存器结构定义。地址偏移量:0x00B4，初值:0x00B00000，宽度:32
 寄存器说明: DACL_MIXER4控制寄存器0
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved             : 12; /* bit[0-11] : reserved */
        unsigned int  dacl_mixer4_gain1    : 2;  /* bit[12-13]: dacl_mixer4_gain1增益控制
                                                                配置范围: 0~3
                                                                0: -12 dB
                                                                1: -6 dB
                                                                2:- 2 dB
                                                                3: 0 dB  */
        unsigned int  dacl_mixer4_gain2    : 2;  /* bit[14-15]: dacl_mixer4_gain2增益控制
                                                                配置范围: 0~3
                                                                0: -12 dB
                                                                1: -6 dB
                                                                2:- 2 dB
                                                                3: 0 dB  */
        unsigned int  dacl_mixer4_gain3    : 2;  /* bit[16-17]: dacl_mixer4_gain3增益控制
                                                                配置范围: 0~3
                                                                0: -12 dB
                                                                1: -6 dB
                                                                2:- 2 dB
                                                                3: 0 dB  */
        unsigned int  dacl_mixer4_gain4    : 2;  /* bit[18-19]: dacl_mixer4_gain4增益控制
                                                                配置范围: 0~3
                                                                0: -12 dB
                                                                1: -6 dB
                                                                2:- 2 dB
                                                                3: 0 dB  */
        unsigned int  dacl_mixer4_in1_mute : 1;  /* bit[20]   : dacl_mixer4_in1控制
                                                                1：mute通道1的输入数据
                                                                0：不mute通道1的输入数据，正常通过 */
        unsigned int  dacl_mixer4_in2_mute : 1;  /* bit[21]   : dacl_mixer4_in2控制
                                                                1：mute通道2的输入数据
                                                                0：不mute通道2的输入数据，正常通过 */
        unsigned int  dacl_mixer4_in3_mute : 1;  /* bit[22]   : dacl_mixer4_in3控制
                                                                1：mute通道3的输入数据
                                                                0：不mute通道3的输入数据，正常通过 */
        unsigned int  dacl_mixer4_in4_mute : 1;  /* bit[23]   : dacl_mixer4_in4控制
                                                                1：mute通道4的输入数据
                                                                0：不mute通道4的输入数据，正常通过 */
        unsigned int  dacl_mixer4_in1_id   : 2;  /* bit[24-25]: DACL_MIXER4 in1阈值选择信号，用于过零检测。其中每个pga
                                                                通过使用其中2bit的id选择四种场景的阈值
                                                                0：选择pga_thre_ctrl[23:0]
                                                                1：选择pga_thre_ctrl[47:24]
                                                                2：选择pga_thre_ctrl[71:48]
                                                                3：选择pga_thre_ctrl[95:72] */
        unsigned int  dacl_mixer4_in2_id   : 2;  /* bit[26-27]: DACL_MIXER4 in2阈值选择信号，用于过零检测。其中每个pga
                                                                通过使用其中2bit的id选择四种场景的阈值
                                                                0：选择pga_thre_ctrl[23:0]
                                                                1：选择pga_thre_ctrl[47:24]
                                                                2：选择pga_thre_ctrl[71:48]
                                                                3：选择pga_thre_ctrl[95:72] */
        unsigned int  dacl_mixer4_in3_id   : 2;  /* bit[28-29]: DACL_MIXER4 in3阈值选择信号，用于过零检测。其中每个pga
                                                                通过使用其中2bit的id选择四种场景的阈值
                                                                0：选择pga_thre_ctrl[23:0]
                                                                1：选择pga_thre_ctrl[47:24]
                                                                2：选择pga_thre_ctrl[71:48]
                                                                3：选择pga_thre_ctrl[95:72] */
        unsigned int  dacl_mixer4_in4_id   : 2;  /* bit[30-31]: DACL_MIXER4 in4阈值选择信号，用于过零检测。其中每个pga
                                                                通过使用其中2bit的id选择四种场景的阈值
                                                                0：选择pga_thre_ctrl[23:0]
                                                                1：选择pga_thre_ctrl[47:24]
                                                                2：选择pga_thre_ctrl[71:48]
                                                                3：选择pga_thre_ctrl[95:72] */
    } reg;
} SOC_ASP_CODEC_DACL_MIXER4_CTRL0_UNION;
#endif
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_gain1_START     (12)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_gain1_END       (13)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_gain2_START     (14)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_gain2_END       (15)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_gain3_START     (16)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_gain3_END       (17)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_gain4_START     (18)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_gain4_END       (19)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in1_mute_START  (20)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in1_mute_END    (20)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in2_mute_START  (21)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in2_mute_END    (21)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in3_mute_START  (22)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in3_mute_END    (22)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in4_mute_START  (23)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in4_mute_END    (23)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in1_id_START    (24)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in1_id_END      (25)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in2_id_START    (26)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in2_id_END      (27)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in3_id_START    (28)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in3_id_END      (29)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in4_id_START    (30)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL0_dacl_mixer4_in4_id_END      (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_DACL_MIXER4_CTRL1_UNION
 结构说明  : DACL_MIXER4_CTRL1 寄存器结构定义。地址偏移量:0x00B8，初值:0x00140000，宽度:32
 寄存器说明: DACL_MIXER4控制寄存器1
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved             : 16; /* bit[0-15] : reserved */
        unsigned int  dacl_mixer4_zero_num : 5;  /* bit[16-20]: dacl_mixer4过零检测数目，默认值为5'd20 */
        unsigned int  dacl_mixer4_fade_out : 5;  /* bit[21-25]: dacl_mixer4淡出时间配置，配置范围: 9~20（64ms~600ms），
                                                                通道内所有MIXER淡入淡出时间配置与采样率相关，
                                                                具体约束参见用户手册 */
        unsigned int  dacl_mixer4_fade_in  : 5;  /* bit[26-30]: DACL_MIXER4淡入时间配置，配置范围: 9~20（64ms~600ms），
                                                                通道内所有MIXER淡入淡出时间配置与采样率相关，
                                                                具体约束参见用户手册 */
        unsigned int  dacl_mixer4_fade_en  : 1;  /* bit[31]   : dacl_mixer4淡入淡出功能使能，高有效
                                                                1：淡入淡出功能使能
                                                                0：淡入淡出功能不使能 */
    } reg;
} SOC_ASP_CODEC_DACL_MIXER4_CTRL1_UNION;
#endif
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL1_dacl_mixer4_zero_num_START  (16)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL1_dacl_mixer4_zero_num_END    (20)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL1_dacl_mixer4_fade_out_START  (21)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL1_dacl_mixer4_fade_out_END    (25)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL1_dacl_mixer4_fade_in_START   (26)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL1_dacl_mixer4_fade_in_END     (30)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL1_dacl_mixer4_fade_en_START   (31)
#define SOC_ASP_CODEC_DACL_MIXER4_CTRL1_dacl_mixer4_fade_en_END     (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_DACR_MIXER4_CTRL0_UNION
 结构说明  : DACR_MIXER4_CTRL0 寄存器结构定义。地址偏移量:0x00BC，初值:0x00B00000，宽度:32
 寄存器说明: DACR_MIXER4控制寄存器0
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved             : 12; /* bit[0-11] : reserved */
        unsigned int  dacr_mixer4_gain1    : 2;  /* bit[12-13]: dacr_mixer4_gain1增益控制
                                                                配置范围: 0~3
                                                                0: -12 dB
                                                                1: -6 dB
                                                                2:- 2 dB
                                                                3: 0 dB  */
        unsigned int  dacr_mixer4_gain2    : 2;  /* bit[14-15]: dacr_mixer4_gain2增益控制
                                                                配置范围: 0~3
                                                                0: -12 dB
                                                                1: -6 dB
                                                                2:- 2 dB
                                                                3: 0 dB  */
        unsigned int  dacr_mixer4_gain3    : 2;  /* bit[16-17]: dacr_mixer4_gain3增益控制
                                                                配置范围: 0~3
                                                                0: -12 dB
                                                                1: -6 dB
                                                                2:- 2 dB
                                                                3: 0 dB  */
        unsigned int  dacr_mixer4_gain4    : 2;  /* bit[18-19]: dacr_mixer4_gain4增益控制
                                                                配置范围: 0~3
                                                                0: -12 dB
                                                                1: -6 dB
                                                                2:- 2 dB
                                                                3: 0 dB  */
        unsigned int  dacr_mixer4_in1_mute : 1;  /* bit[20]   : dacr_mixer4_in1控制
                                                                1：mute通道1的输入数据
                                                                0：不mute通道1的输入数据，正常通过 */
        unsigned int  dacr_mixer4_in2_mute : 1;  /* bit[21]   : dacr_mixer4_in2控制
                                                                1：mute通道2的输入数据
                                                                0：不mute通道2的输入数据，正常通过 */
        unsigned int  dacr_mixer4_in3_mute : 1;  /* bit[22]   : dacr_mixer4_in3控制
                                                                1：mute通道3的输入数据
                                                                0：不mute通道3的输入数据，正常通过 */
        unsigned int  dacr_mixer4_in4_mute : 1;  /* bit[23]   : dacr_mixer4_in4控制
                                                                1：mute通道4的输入数据
                                                                0：不mute通道4的输入数据，正常通过 */
        unsigned int  dacr_mixer4_in1_id   : 2;  /* bit[24-25]: DACR_MIXER4 in1阈值选择信号，用于过零检测。其中每个pga
                                                                通过使用其中2bit的id选择四种场景的阈值
                                                                0：选择pga_thre_ctrl[23:0]
                                                                1：选择pga_thre_ctrl[47:24]
                                                                2：选择pga_thre_ctrl[71:48]
                                                                3：选择pga_thre_ctrl[95:72] */
        unsigned int  dacr_mixer4_in2_id   : 2;  /* bit[26-27]: DACR_MIXER4 in2阈值选择信号，用于过零检测。其中每个pga
                                                                通过使用其中2bit的id选择四种场景的阈值
                                                                0：选择pga_thre_ctrl[23:0]
                                                                1：选择pga_thre_ctrl[47:24]
                                                                2：选择pga_thre_ctrl[71:48]
                                                                3：选择pga_thre_ctrl[95:72] */
        unsigned int  dacr_mixer4_in3_id   : 2;  /* bit[28-29]: DACR_MIXER4 in3阈值选择信号，用于过零检测。其中每个pga
                                                                通过使用其中2bit的id选择四种场景的阈值
                                                                0：选择pga_thre_ctrl[23:0]
                                                                1：选择pga_thre_ctrl[47:24]
                                                                2：选择pga_thre_ctrl[71:48]
                                                                3：选择pga_thre_ctrl[95:72] */
        unsigned int  dacr_mixer4_in4_id   : 2;  /* bit[30-31]: DACR_MIXER4 in4阈值选择信号，用于过零检测。其中每个pga
                                                                通过使用其中2bit的id选择四种场景的阈值
                                                                0：选择pga_thre_ctrl[23:0]
                                                                1：选择pga_thre_ctrl[47:24]
                                                                2：选择pga_thre_ctrl[71:48]
                                                                3：选择pga_thre_ctrl[95:72] */
    } reg;
} SOC_ASP_CODEC_DACR_MIXER4_CTRL0_UNION;
#endif
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_gain1_START     (12)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_gain1_END       (13)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_gain2_START     (14)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_gain2_END       (15)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_gain3_START     (16)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_gain3_END       (17)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_gain4_START     (18)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_gain4_END       (19)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in1_mute_START  (20)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in1_mute_END    (20)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in2_mute_START  (21)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in2_mute_END    (21)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in3_mute_START  (22)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in3_mute_END    (22)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in4_mute_START  (23)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in4_mute_END    (23)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in1_id_START    (24)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in1_id_END      (25)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in2_id_START    (26)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in2_id_END      (27)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in3_id_START    (28)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in3_id_END      (29)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in4_id_START    (30)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL0_dacr_mixer4_in4_id_END      (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_DACR_MIXER4_CTRL1_UNION
 结构说明  : DACR_MIXER4_CTRL1 寄存器结构定义。地址偏移量:0x00C0，初值:0x00140000，宽度:32
 寄存器说明: DACR_MIXER4控制寄存器1
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved             : 16; /* bit[0-15] : reserved */
        unsigned int  dacr_mixer4_zero_num : 5;  /* bit[16-20]: dacr_mixer4过零检测数目，默认值为5'd20 */
        unsigned int  dacr_mixer4_fade_out : 5;  /* bit[21-25]: dacr_mixer4淡出时间配置，配置范围: 9~20（64ms~600ms），
                                                                通道内所有MIXER淡入淡出时间配置与采样率相关，
                                                                具体约束参见用户手册 */
        unsigned int  dacr_mixer4_fade_in  : 5;  /* bit[26-30]: DACR_MIXER4淡入时间配置，配置范围: 9~20（64ms~600ms），
                                                                通道内所有MIXER淡入淡出时间配置与采样率相关，
                                                                具体约束参见用户手册 */
        unsigned int  dacr_mixer4_fade_en  : 1;  /* bit[31]   : dacr_mixer4淡入淡出功能使能，高有效
                                                                1：淡入淡出功能使能
                                                                0：淡入淡出功能不使能 */
    } reg;
} SOC_ASP_CODEC_DACR_MIXER4_CTRL1_UNION;
#endif
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL1_dacr_mixer4_zero_num_START  (16)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL1_dacr_mixer4_zero_num_END    (20)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL1_dacr_mixer4_fade_out_START  (21)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL1_dacr_mixer4_fade_out_END    (25)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL1_dacr_mixer4_fade_in_START   (26)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL1_dacr_mixer4_fade_in_END     (30)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL1_dacr_mixer4_fade_en_START   (31)
#define SOC_ASP_CODEC_DACR_MIXER4_CTRL1_dacr_mixer4_fade_en_END     (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_UNION
 结构说明  : I2S2_TX_MIXER2_CTRL 寄存器结构定义。地址偏移量:0x00C4，初值:0x0C000500，宽度:32
 寄存器说明: I2S2MIXER2控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                : 6;  /* bit[0-5]  : reserved */
        unsigned int  i2s2_tx_mixer2_zero_num : 5;  /* bit[6-10] : i2s2_tx_mixer2过零检测数目，默认值为5'd20 */
        unsigned int  i2s2_tx_mixer2_fade_out : 5;  /* bit[11-15]: i2s2_tx_mixer2淡出时间配置，配置范围: 9~20（64ms~600ms）
                                                                   通道内所有MIXER淡入淡出时间配置与采样率相关，
                                                                   具体约束参见用户手册 */
        unsigned int  i2s2_tx_mixer2_fade_in  : 5;  /* bit[16-20]: I2S2_TX_MIXER2淡入时间配置，配置范围: 9~20（64ms~600ms）
                                                                   通道内所有MIXER淡入淡出时间配置与采样率相关，
                                                                   具体约束参见用户手册 */
        unsigned int  i2s2_tx_mixer2_fade_en  : 1;  /* bit[21]   : i2s2_tx_mixer2淡入淡出功能使能，高有效
                                                                   1：淡入淡出功能使能
                                                                   0：淡入淡出功能不使能 */
        unsigned int  i2s2_tx_mixer2_gain1    : 2;  /* bit[22-23]: i2s2_tx_mixer2_gain1增益控制
                                                                   配置范围: 0~3
                                                                   0: -12 dB
                                                                   1: -6 dB
                                                                   2:- 2 dB
                                                                   3: 0 dB  */
        unsigned int  i2s2_tx_mixer2_gain2    : 2;  /* bit[24-25]: i2s2_tx_mixer2_gain2增益控制
                                                                   配置范围: 0~3
                                                                   0: -12 dB
                                                                   1: -6 dB
                                                                   2:- 2 dB
                                                                   3: 0 dB  */
        unsigned int  i2s2_tx_mixer2_in1_mute : 1;  /* bit[26]   : i2s2_tx_mixer2_in1控制
                                                                   1：mute通道1的输入数据
                                                                   0：不mute通道1的输入数据，正常通过 */
        unsigned int  i2s2_tx_mixer2_in2_mute : 1;  /* bit[27]   : i2s2_tx_mixer2_in2控制
                                                                   1：mute通道2的输入数据
                                                                   0：不mute通道2的输入数据，正常通过 */
        unsigned int  i2s2_tx_mixer2_in1_id   : 2;  /* bit[28-29]: I2S2_TX_MIXER2 in1阈值选择信号，用于过零检测。其中每个pga
                                                                   通过使用其中2bit的id选择四种场景的阈值
                                                                   0：选择pga_thre_ctrl[23:0]
                                                                   1：选择pga_thre_ctrl[47:24]
                                                                   2：选择pga_thre_ctrl[71:48]
                                                                   3：选择pga_thre_ctrl[95:73] */
        unsigned int  i2s2_tx_mixer2_in2_id   : 2;  /* bit[30-31]: I2S2_TX_MIXER2 in2阈值选择信号，用于过零检测。其中每个pga
                                                                   通过使用其中2bit的id选择四种场景的阈值
                                                                   0：选择pga_thre_ctrl[23:0]
                                                                   1：选择pga_thre_ctrl[47:24]
                                                                   2：选择pga_thre_ctrl[71:48]
                                                                   3：选择pga_thre_ctrl[95:73] */
    } reg;
} SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_zero_num_START  (6)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_zero_num_END    (10)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_fade_out_START  (11)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_fade_out_END    (15)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_fade_in_START   (16)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_fade_in_END     (20)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_fade_en_START   (21)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_fade_en_END     (21)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_gain1_START     (22)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_gain1_END       (23)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_gain2_START     (24)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_gain2_END       (25)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_in1_mute_START  (26)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_in1_mute_END    (26)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_in2_mute_START  (27)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_in2_mute_END    (27)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_in1_id_START    (28)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_in1_id_END      (29)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_in2_id_START    (30)
#define SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_i2s2_tx_mixer2_in2_id_END      (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_CLK_EN3_UNION
 结构说明  : CODEC_CLK_EN3 寄存器结构定义。地址偏移量:0x00C8，初值:0x00000000，宽度:32
 寄存器说明: CODEC的使能控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc6_0p5_pga_clken  : 1;  /* bit[0]    : adc6通路0.5 PGA时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  adc7_0p5_pga_clken  : 1;  /* bit[1]    : adc7通路0.5 PGA时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  adc8_0p5_pga_clken  : 1;  /* bit[2]    : adc8通路0.5 PGA时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  dmic3_clken         : 1;  /* bit[3]    : dmic3的时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  dmic4_clken         : 1;  /* bit[4]    : dmic4的时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  adc6_cic_d16_clken  : 1;  /* bit[5]    : adc6_cic_d16的时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  adc7_cic_d16_clken  : 1;  /* bit[6]    : adc7_cic_d16的时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  adc8_cic_d16_clken  : 1;  /* bit[7]    : adc8_cic_d16的时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  adc6_clken          : 1;  /* bit[8]    : adc6的时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  adc7_clken          : 1;  /* bit[9]    : adc7的时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  adc8_clken          : 1;  /* bit[10]   : adc8的时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  mic5_up_afifo_clken : 1;  /* bit[11]   : mic5上行FIFO的时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  mic6_up_afifo_clken : 1;  /* bit[12]   : mic6上行FIFO的时钟控制
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  mic7_up_afifo_clken : 1;  /* bit[13]   : mic7上行FIFO的时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  mic8_up_afifo_clken : 1;  /* bit[14]   : mic8上行FIFO的时钟控制
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  sif_mst_r_clken     : 1;  /* bit[15]   : SIF_MST_R接口时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  dacr_srcup_clken    : 1;  /* bit[16]   : DACR_SRCUP时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  dacl_srcup_clken    : 1;  /* bit[17]   : DACL_SRCUP时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  dacr_filter_clken   : 1;  /* bit[18]   : DACR_FILTER时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  dacl_filter_clken   : 1;  /* bit[19]   : DACL_FILTER时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  dacr_up16_clken     : 1;  /* bit[20]   : DACR_UP16时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  dacl_up16_clken     : 1;  /* bit[21]   : DACL_UP16时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  sdm_r_clken         : 1;  /* bit[22]   : SDM_R时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  sdm_l_clken         : 1;  /* bit[23]   : SDM_R时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  sif_mst_l_clken     : 1;  /* bit[24]   : SIF_MST_L接口时钟控制。
                                                               0：关闭时钟；
                                                               1：打开时钟。 */
        unsigned int  reserved            : 7;  /* bit[25-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_CLK_EN3_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc6_0p5_pga_clken_START   (0)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc6_0p5_pga_clken_END     (0)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc7_0p5_pga_clken_START   (1)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc7_0p5_pga_clken_END     (1)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc8_0p5_pga_clken_START   (2)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc8_0p5_pga_clken_END     (2)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dmic3_clken_START          (3)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dmic3_clken_END            (3)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dmic4_clken_START          (4)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dmic4_clken_END            (4)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc6_cic_d16_clken_START   (5)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc6_cic_d16_clken_END     (5)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc7_cic_d16_clken_START   (6)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc7_cic_d16_clken_END     (6)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc8_cic_d16_clken_START   (7)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc8_cic_d16_clken_END     (7)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc6_clken_START           (8)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc6_clken_END             (8)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc7_clken_START           (9)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc7_clken_END             (9)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc8_clken_START           (10)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_adc8_clken_END             (10)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_mic5_up_afifo_clken_START  (11)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_mic5_up_afifo_clken_END    (11)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_mic6_up_afifo_clken_START  (12)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_mic6_up_afifo_clken_END    (12)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_mic7_up_afifo_clken_START  (13)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_mic7_up_afifo_clken_END    (13)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_mic8_up_afifo_clken_START  (14)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_mic8_up_afifo_clken_END    (14)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_sif_mst_r_clken_START      (15)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_sif_mst_r_clken_END        (15)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dacr_srcup_clken_START     (16)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dacr_srcup_clken_END       (16)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dacl_srcup_clken_START     (17)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dacl_srcup_clken_END       (17)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dacr_filter_clken_START    (18)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dacr_filter_clken_END      (18)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dacl_filter_clken_START    (19)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dacl_filter_clken_END      (19)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dacr_up16_clken_START      (20)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dacr_up16_clken_END        (20)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dacl_up16_clken_START      (21)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_dacl_up16_clken_END        (21)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_sdm_r_clken_START          (22)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_sdm_r_clken_END            (22)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_sdm_l_clken_START          (23)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_sdm_l_clken_END            (23)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_sif_mst_l_clken_START      (24)
#define SOC_ASP_CODEC_CODEC_CLK_EN3_sif_mst_l_clken_END        (24)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_CLK_EN4_UNION
 结构说明  : CODEC_CLK_EN4 寄存器结构定义。地址偏移量:0x00CC，初值:0x00000000，宽度:32
 寄存器说明: CODEC的使能控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  mic5_up_pga_clken     : 1;  /* bit[0]    : mic5_up PGA的时钟控制。
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  mic6_up_pga_clken     : 1;  /* bit[1]    : mic6_up PGA的时钟控制。
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  mic7_up_pga_clken     : 1;  /* bit[2]    : mic7_up PGA的时钟控制。
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  mic8_up_pga_clken     : 1;  /* bit[3]    : mic8_up PGA的时钟控制。
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  classh_r_clken        : 1;  /* bit[4]    : classh_top模块的时钟控制
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  spa_i2_adc_clken      : 1;  /* bit[5]    : spi_adc的超声上行时钟控制
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  spa_v2_adc_clken      : 1;  /* bit[6]    : spv_adc的超声上行时钟控制
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  classh_l_clken        : 1;  /* bit[7]    : classh_top模块的时钟控制
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  spa_i1_adc_clken      : 1;  /* bit[8]    : spi_adc的超声上行时钟控制
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  spa_v1_adc_clken      : 1;  /* bit[9]    : spv_adc的超声上行时钟控制
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  spa_v2_up_afifo_clken : 1;  /* bit[10]   : spa_v上行通路FIFO时钟控制。
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  spa_i2_up_afifo_clken : 1;  /* bit[11]   : spa_i上行通路FIFO时钟控制。
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  spa_v1_up_afifo_clken : 1;  /* bit[12]   : spa_v上行通路FIFO时钟控制。
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  spa_i1_up_afifo_clken : 1;  /* bit[13]   : spa_i上行通路FIFO时钟控制。
                                                                 0：关闭时钟；
                                                                 1：打开时钟。 */
        unsigned int  sif2_spa_adc_en       : 1;  /* bit[14]   : SIF_SPA_ADC方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif2_spa_dac_en       : 1;  /* bit[15]   : SIF_SPA_DAC方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif2_adc_ultr_en      : 1;  /* bit[16]   : SIF_ADC_ULTR方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif2_adcr_en          : 1;  /* bit[17]   : SIF_ADC_R方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif2_adcl_en          : 1;  /* bit[18]   : SIF_ADC_L方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif2_dacr_en          : 1;  /* bit[19]   : SIF_DAC_R方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif2_dacl_en          : 1;  /* bit[20]   : SIF_DAC_L方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif2_adc_mic3_en      : 1;  /* bit[21]   : SIF_ADC_MIC3方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif1_spa_adc_en       : 1;  /* bit[22]   : SIF_SPA_ADC方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif1_spa_dac_en       : 1;  /* bit[23]   : SIF_SPA_DAC方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif1_adc_ultr_en      : 1;  /* bit[24]   : SIF_ADC_ULTR方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif1_adcr_en          : 1;  /* bit[25]   : SIF_ADC_R方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif1_adcl_en          : 1;  /* bit[26]   : SIF_ADC_L方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif1_dacr_en          : 1;  /* bit[27]   : SIF_DAC_R方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif1_dacl_en          : 1;  /* bit[28]   : SIF_DAC_L方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  sif1_adc_mic3_en      : 1;  /* bit[29]   : SIF_ADC_MIC3方向使能：
                                                                 1：向串行接口发送数据
                                                                 0：不发送数据 */
        unsigned int  reserved              : 2;  /* bit[30-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_CLK_EN4_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_CLK_EN4_mic5_up_pga_clken_START      (0)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_mic5_up_pga_clken_END        (0)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_mic6_up_pga_clken_START      (1)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_mic6_up_pga_clken_END        (1)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_mic7_up_pga_clken_START      (2)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_mic7_up_pga_clken_END        (2)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_mic8_up_pga_clken_START      (3)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_mic8_up_pga_clken_END        (3)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_classh_r_clken_START         (4)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_classh_r_clken_END           (4)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_i2_adc_clken_START       (5)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_i2_adc_clken_END         (5)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_v2_adc_clken_START       (6)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_v2_adc_clken_END         (6)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_classh_l_clken_START         (7)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_classh_l_clken_END           (7)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_i1_adc_clken_START       (8)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_i1_adc_clken_END         (8)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_v1_adc_clken_START       (9)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_v1_adc_clken_END         (9)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_v2_up_afifo_clken_START  (10)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_v2_up_afifo_clken_END    (10)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_i2_up_afifo_clken_START  (11)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_i2_up_afifo_clken_END    (11)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_v1_up_afifo_clken_START  (12)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_v1_up_afifo_clken_END    (12)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_i1_up_afifo_clken_START  (13)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_spa_i1_up_afifo_clken_END    (13)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_spa_adc_en_START        (14)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_spa_adc_en_END          (14)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_spa_dac_en_START        (15)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_spa_dac_en_END          (15)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_adc_ultr_en_START       (16)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_adc_ultr_en_END         (16)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_adcr_en_START           (17)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_adcr_en_END             (17)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_adcl_en_START           (18)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_adcl_en_END             (18)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_dacr_en_START           (19)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_dacr_en_END             (19)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_dacl_en_START           (20)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_dacl_en_END             (20)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_adc_mic3_en_START       (21)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif2_adc_mic3_en_END         (21)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_spa_adc_en_START        (22)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_spa_adc_en_END          (22)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_spa_dac_en_START        (23)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_spa_dac_en_END          (23)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_adc_ultr_en_START       (24)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_adc_ultr_en_END         (24)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_adcr_en_START           (25)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_adcr_en_END             (25)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_adcl_en_START           (26)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_adcl_en_END             (26)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_dacr_en_START           (27)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_dacr_en_END             (27)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_dacl_en_START           (28)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_dacl_en_END             (28)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_adc_mic3_en_START       (29)
#define SOC_ASP_CODEC_CODEC_CLK_EN4_sif1_adc_mic3_en_END         (29)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ADC_FILTER_CTRL0_UNION
 结构说明  : ADC_FILTER_CTRL0 寄存器结构定义。地址偏移量:0x00D0，初值:0x82008200，宽度:32
 寄存器说明: ADC_FILTER1控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0    : 4;  /* bit[0-3]  : reserved */
        unsigned int  adc8_cic_gain : 6;  /* bit[4-9]  : adc8_cic_gain增益配置 */
        unsigned int  adc7_cic_gain : 6;  /* bit[10-15]: adc7_cic_gain增益配置 */
        unsigned int  reserved_1    : 4;  /* bit[16-19]: reserved */
        unsigned int  adc6_cic_gain : 6;  /* bit[20-25]: adc6_cic_gain增益配置 */
        unsigned int  adc5_cic_gain : 6;  /* bit[26-31]: adc5_cic_gain增益配置 */
    } reg;
} SOC_ASP_CODEC_ADC_FILTER_CTRL0_UNION;
#endif
#define SOC_ASP_CODEC_ADC_FILTER_CTRL0_adc8_cic_gain_START  (4)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL0_adc8_cic_gain_END    (9)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL0_adc7_cic_gain_START  (10)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL0_adc7_cic_gain_END    (15)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL0_adc6_cic_gain_START  (20)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL0_adc6_cic_gain_END    (25)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL0_adc5_cic_gain_START  (26)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL0_adc5_cic_gain_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ADC_FILTER_CTRL_UNION
 结构说明  : ADC_FILTER_CTRL 寄存器结构定义。地址偏移量:0x00D4，初值:0x82008200，宽度:32
 寄存器说明: ADC_FILTER控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0    : 4;  /* bit[0-3]  : reserved */
        unsigned int  adc4_cic_gain : 6;  /* bit[4-9]  : adc4_cic_gain增益配置 */
        unsigned int  adc3_cic_gain : 6;  /* bit[10-15]: adc3_cic_gain增益配置 */
        unsigned int  reserved_1    : 4;  /* bit[16-19]: reserved */
        unsigned int  adc2_cic_gain : 6;  /* bit[20-25]: adc2_cic_gain增益配置 */
        unsigned int  adc1_cic_gain : 6;  /* bit[26-31]: adc1_cic_gain增益配置 */
    } reg;
} SOC_ASP_CODEC_ADC_FILTER_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_ADC_FILTER_CTRL_adc4_cic_gain_START  (4)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL_adc4_cic_gain_END    (9)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL_adc3_cic_gain_START  (10)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL_adc3_cic_gain_END    (15)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL_adc2_cic_gain_START  (20)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL_adc2_cic_gain_END    (25)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL_adc1_cic_gain_START  (26)
#define SOC_ASP_CODEC_ADC_FILTER_CTRL_adc1_cic_gain_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_DMIC_CTRL_UNION
 结构说明  : DMIC_CTRL 寄存器结构定义。地址偏移量:0x00D8，初值:0x00000000，宽度:32
 寄存器说明: DMIC_CTRL控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  dmic1_sw_dmic_mode : 1;  /* bit[0]   : DMIC声道配置。
                                                             0: 不交换左右声道；
                                                             1: 交换左右声道 */
        unsigned int  dmic1_reverse      : 1;  /* bit[1]   : DMIC输入编码配置。
                                                             0: 0 -> +4, 1 -> -4；
                                                             1: 0 -> -4, 1 -> +4 */
        unsigned int  dmic2_sw_dmic_mode : 1;  /* bit[2]   : DMIC声道配置。
                                                             0: 不交换左右声道；
                                                             1: 交换左右声道 */
        unsigned int  dmic2_reverse      : 1;  /* bit[3]   : DMIC输入编码配置。
                                                             0: 0 -> +4, 1 -> -4；
                                                             1: 0 -> -4, 1 -> +4 */
        unsigned int  dmic3_sw_dmic_mode : 1;  /* bit[4]   : DMIC声道配置。
                                                             0: 不交换左右声道；
                                                             1: 交换左右声道 */
        unsigned int  dmic3_reverse      : 1;  /* bit[5]   : DMIC输入编码配置。
                                                             0: 0 -> +4, 1 -> -4；
                                                             1: 0 -> -4, 1 -> +4 */
        unsigned int  dmic4_sw_dmic_mode : 1;  /* bit[6]   : DMIC声道配置。
                                                             0: 不交换左右声道；
                                                             1: 交换左右声道 */
        unsigned int  dmic4_reverse      : 1;  /* bit[7]   : DMIC输入编码配置。
                                                             0: 0 -> +4, 1 -> -4；
                                                             1: 0 -> -4, 1 -> +4 */
        unsigned int  reserved           : 24; /* bit[8-31]: reserved */
    } reg;
} SOC_ASP_CODEC_DMIC_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_DMIC_CTRL_dmic1_sw_dmic_mode_START  (0)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic1_sw_dmic_mode_END    (0)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic1_reverse_START       (1)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic1_reverse_END         (1)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic2_sw_dmic_mode_START  (2)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic2_sw_dmic_mode_END    (2)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic2_reverse_START       (3)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic2_reverse_END         (3)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic3_sw_dmic_mode_START  (4)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic3_sw_dmic_mode_END    (4)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic3_reverse_START       (5)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic3_reverse_END         (5)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic4_sw_dmic_mode_START  (6)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic4_sw_dmic_mode_END    (6)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic4_reverse_START       (7)
#define SOC_ASP_CODEC_DMIC_CTRL_dmic4_reverse_END         (7)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_DMIC_DIV_UNION
 结构说明  : DMIC_DIV 寄存器结构定义。地址偏移量:0x00DC，初值:0x00001111，宽度:32
 寄存器说明: DMIC_DIV分频控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  fs_dmic1 : 4;  /* bit[0-3]  : DMIC时钟配置：
                                                    0000：关闭时钟；
                                                    0001：2分频；3.072MHz
                                                    0010：3分频；2.048MHz
                                                    0011：4分频；1.536MHz
                                                    0111：8分频；768KHz
                                                    1011：12分频；512KHz
                                                    Other:关闭时钟。 */
        unsigned int  fs_dmic2 : 4;  /* bit[4-7]  : DMIC时钟配置：
                                                    0000：关闭时钟；
                                                    0001：2分频；3.072MHz
                                                    0010：3分频；2.048MHz
                                                    0011：4分频；1.536MHz
                                                    0111：8分频；768KHz
                                                    1011：12分频；512KHz
                                                    Other:关闭时钟。 */
        unsigned int  fs_dmic3 : 4;  /* bit[8-11] : DMIC时钟配置：
                                                    0000：关闭时钟；
                                                    0001：2分频；3.072MHz
                                                    0010：3分频；2.048MHz
                                                    0011：4分频；1.536MHz
                                                    0111：8分频；768KHz
                                                    1011：12分频；512KHz
                                                    Other:关闭时钟。 */
        unsigned int  fs_dmic4 : 4;  /* bit[12-15]: DMIC时钟配置：
                                                    0000：关闭时钟；
                                                    0001：2分频；3.072MHz
                                                    0010：3分频；2.048MHz
                                                    0011：4分频；1.536MHz
                                                    0111：8分频；768KHz
                                                    1011：12分频；512KHz
                                                    Other:关闭时钟。 */
        unsigned int  reserved : 16; /* bit[16-31]: reserved */
    } reg;
} SOC_ASP_CODEC_DMIC_DIV_UNION;
#endif
#define SOC_ASP_CODEC_DMIC_DIV_fs_dmic1_START  (0)
#define SOC_ASP_CODEC_DMIC_DIV_fs_dmic1_END    (3)
#define SOC_ASP_CODEC_DMIC_DIV_fs_dmic2_START  (4)
#define SOC_ASP_CODEC_DMIC_DIV_fs_dmic2_END    (7)
#define SOC_ASP_CODEC_DMIC_DIV_fs_dmic3_START  (8)
#define SOC_ASP_CODEC_DMIC_DIV_fs_dmic3_END    (11)
#define SOC_ASP_CODEC_DMIC_DIV_fs_dmic4_START  (12)
#define SOC_ASP_CODEC_DMIC_DIV_fs_dmic4_END    (15)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_UNION
 结构说明  : SPA12_UP_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x00E0，初值:0x7A007A00，宽度:32
 寄存器说明: SPA反馈通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0              : 5;  /* bit[0-4]  : reserved */
        unsigned int  spa_1_up_fifo_aempty_th : 5;  /* bit[5-9]  : SPA上行通路通路一声道AFIFO将空阈值。 */
        unsigned int  spa_1_up_fifo_afull_th  : 5;  /* bit[10-14]: SPA上行通路通路一声道AFIFO将满阈值。 */
        unsigned int  spa_1_up_fifo_clr       : 1;  /* bit[15]   : SPA上行通路通路一声道AFIFO Clear Signal, high active.
                                                                   0：不清空FIFO数据
                                                                   1：清空FIFO数据 */
        unsigned int  reserved_1              : 5;  /* bit[16-20]: reserved */
        unsigned int  spa_2_up_fifo_aempty_th : 5;  /* bit[21-25]: SPA上行通路通路二声道AFIFO将空阈值。 */
        unsigned int  spa_2_up_fifo_afull_th  : 5;  /* bit[26-30]: SPA上行通路通路二声道AFIFO将满阈值。 */
        unsigned int  spa_2_up_fifo_clr       : 1;  /* bit[31]   : SPA上行通路通路二声道AFIFO Clear Signal, high active.
                                                                   0：不清空FIFO数据
                                                                   1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_spa_1_up_fifo_aempty_th_START  (5)
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_spa_1_up_fifo_aempty_th_END    (9)
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_spa_1_up_fifo_afull_th_START   (10)
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_spa_1_up_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_spa_1_up_fifo_clr_START        (15)
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_spa_1_up_fifo_clr_END          (15)
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_spa_2_up_fifo_aempty_th_START  (21)
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_spa_2_up_fifo_aempty_th_END    (25)
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_spa_2_up_fifo_afull_th_START   (26)
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_spa_2_up_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_spa_2_up_fifo_clr_START        (31)
#define SOC_ASP_CODEC_SPA12_UP_AFIFO_CTRL_spa_2_up_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_UNION
 结构说明  : SPA34_UP_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x00E4，初值:0x7A007A00，宽度:32
 寄存器说明: SPA反馈通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0              : 5;  /* bit[0-4]  : reserved */
        unsigned int  spa_3_up_fifo_aempty_th : 5;  /* bit[5-9]  : SPA上行通路通路三声道AFIFO将空阈值。 */
        unsigned int  spa_3_up_fifo_afull_th  : 5;  /* bit[10-14]: SPA上行通路通路三声道AFIFO将满阈值。 */
        unsigned int  spa_3_up_fifo_clr       : 1;  /* bit[15]   : SPA上行通路通路三声道AFIFO Clear Signal, high active.
                                                                   0：不清空FIFO数据
                                                                   1：清空FIFO数据 */
        unsigned int  reserved_1              : 5;  /* bit[16-20]: reserved */
        unsigned int  spa_4_up_fifo_aempty_th : 5;  /* bit[21-25]: SPA上行通路通路四声道AFIFO将空阈值。 */
        unsigned int  spa_4_up_fifo_afull_th  : 5;  /* bit[26-30]: SPA上行通路通路四声道AFIFO将满阈值。 */
        unsigned int  spa_4_up_fifo_clr       : 1;  /* bit[31]   : SPA上行通路通路四声道AFIFO Clear Signal, high active.
                                                                   0：不清空FIFO数据
                                                                   1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_spa_3_up_fifo_aempty_th_START  (5)
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_spa_3_up_fifo_aempty_th_END    (9)
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_spa_3_up_fifo_afull_th_START   (10)
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_spa_3_up_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_spa_3_up_fifo_clr_START        (15)
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_spa_3_up_fifo_clr_END          (15)
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_spa_4_up_fifo_aempty_th_START  (21)
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_spa_4_up_fifo_aempty_th_END    (25)
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_spa_4_up_fifo_afull_th_START   (26)
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_spa_4_up_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_spa_4_up_fifo_clr_START        (31)
#define SOC_ASP_CODEC_SPA34_UP_AFIFO_CTRL_spa_4_up_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_UNION
 结构说明  : CODEC3_DN_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x00E8，初值:0x7C107C10，宽度:32
 寄存器说明: CODEC3下行通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  codec3_r_dn_fifo_aempty_th : 7;  /* bit[0-6]  : codec3下行通路通路右声道AFIFO将空阈值。 */
        unsigned int  reserved_0                 : 1;  /* bit[7]    : reserved */
        unsigned int  codec3_r_dn_fifo_afull_th  : 7;  /* bit[8-14] : codec3下行通路通路右声道AFIFO将满阈值。 */
        unsigned int  codec3_r_dn_fifo_clr       : 1;  /* bit[15]   : codec3下行通路通路右声道AFIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
        unsigned int  codec3_l_dn_fifo_aempty_th : 7;  /* bit[16-22]: reserved */
        unsigned int  reserved_1                 : 1;  /* bit[23]   : codec3下行通路通路左声道AFIFO将空阈值。 */
        unsigned int  codec3_l_dn_fifo_afull_th  : 7;  /* bit[24-30]: codec3下行通路通路左声道AFIFO将满阈值。 */
        unsigned int  codec3_l_dn_fifo_clr       : 1;  /* bit[31]   : codec3下行通路通路左声道AFIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_codec3_r_dn_fifo_aempty_th_START  (0)
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_codec3_r_dn_fifo_aempty_th_END    (6)
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_codec3_r_dn_fifo_afull_th_START   (8)
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_codec3_r_dn_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_codec3_r_dn_fifo_clr_START        (15)
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_codec3_r_dn_fifo_clr_END          (15)
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_codec3_l_dn_fifo_aempty_th_START  (16)
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_codec3_l_dn_fifo_aempty_th_END    (22)
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_codec3_l_dn_fifo_afull_th_START   (24)
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_codec3_l_dn_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_codec3_l_dn_fifo_clr_START        (31)
#define SOC_ASP_CODEC_CODEC3_DN_AFIFO_CTRL_codec3_l_dn_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_UNION
 结构说明  : AUDIO_DN_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x00EC，初值:0x7C107C10，宽度:32
 寄存器说明: AUDIO下行通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  audio_r_dn_fifo_aempty_th : 7;  /* bit[0-6]  : Audio下行通路通路右声道AFIFO将空阈值。 */
        unsigned int  reserved_0                : 1;  /* bit[7]    : reserved */
        unsigned int  audio_r_dn_fifo_afull_th  : 7;  /* bit[8-14] : Audio下行通路通路右声道AFIFO将满阈值。 */
        unsigned int  audio_r_dn_fifo_clr       : 1;  /* bit[15]   : Audio下行通路通路右声道AFIFO Clear Signal, high active.
                                                                     0：不清空FIFO数据
                                                                     1：清空FIFO数据 */
        unsigned int  audio_l_dn_fifo_aempty_th : 7;  /* bit[16-22]: Audio下行通路通路左声道AFIFO将空阈值。 */
        unsigned int  reserved_1                : 1;  /* bit[23]   : reserved */
        unsigned int  audio_l_dn_fifo_afull_th  : 7;  /* bit[24-30]: Audio下行通路通路左声道AFIFO将满阈值。 */
        unsigned int  audio_l_dn_fifo_clr       : 1;  /* bit[31]   : Audio下行通路通路左声道AFIFO Clear Signal, high active.
                                                                     0：不清空FIFO数据
                                                                     1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_audio_r_dn_fifo_aempty_th_START  (0)
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_audio_r_dn_fifo_aempty_th_END    (6)
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_audio_r_dn_fifo_afull_th_START   (8)
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_audio_r_dn_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_audio_r_dn_fifo_clr_START        (15)
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_audio_r_dn_fifo_clr_END          (15)
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_audio_l_dn_fifo_aempty_th_START  (16)
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_audio_l_dn_fifo_aempty_th_END    (22)
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_audio_l_dn_fifo_afull_th_START   (24)
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_audio_l_dn_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_audio_l_dn_fifo_clr_START        (31)
#define SOC_ASP_CODEC_AUDIO_DN_AFIFO_CTRL_audio_l_dn_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_UNION
 结构说明  : ULTR_DN_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x00F0，初值:0x7C107C10，宽度:32
 寄存器说明: ULTR下行通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  ultr_r_dn_fifo_aempty_th : 7;  /* bit[0-6]  : ultr下行通路通路右声道AFIFO将空阈值。 */
        unsigned int  reserved_0               : 1;  /* bit[7]    : reserved */
        unsigned int  ultr_r_dn_fifo_afull_th  : 7;  /* bit[8-14] : ultr下行通路通路右声道AFIFO将满阈值。 */
        unsigned int  ultr_r_dn_fifo_clr       : 1;  /* bit[15]   : ultr下行通路通路右声道AFIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
        unsigned int  ultr_l_dn_fifo_aempty_th : 7;  /* bit[16-22]: reserved */
        unsigned int  reserved_1               : 1;  /* bit[23]   : ultr下行通路通路左声道AFIFO将空阈值。 */
        unsigned int  ultr_l_dn_fifo_afull_th  : 7;  /* bit[24-30]: ultr下行通路通路左声道AFIFO将满阈值。 */
        unsigned int  ultr_l_dn_fifo_clr       : 1;  /* bit[31]   : ultr下行通路通路左声道AFIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ultr_r_dn_fifo_aempty_th_START  (0)
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ultr_r_dn_fifo_aempty_th_END    (6)
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ultr_r_dn_fifo_afull_th_START   (8)
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ultr_r_dn_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ultr_r_dn_fifo_clr_START        (15)
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ultr_r_dn_fifo_clr_END          (15)
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ultr_l_dn_fifo_aempty_th_START  (16)
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ultr_l_dn_fifo_aempty_th_END    (22)
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ultr_l_dn_fifo_afull_th_START   (24)
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ultr_l_dn_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ultr_l_dn_fifo_clr_START        (31)
#define SOC_ASP_CODEC_ULTR_DN_AFIFO_CTRL_ultr_l_dn_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_UNION
 结构说明  : SPA56_UP_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x00F4，初值:0x7A007A00，宽度:32
 寄存器说明: SPA反馈通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0              : 5;  /* bit[0-4]  : reserved */
        unsigned int  spa_5_up_fifo_aempty_th : 5;  /* bit[5-9]  : SPA上行通路通路三声道AFIFO将空阈值。 */
        unsigned int  spa_5_up_fifo_afull_th  : 5;  /* bit[10-14]: SPA上行通路通路三声道AFIFO将满阈值。 */
        unsigned int  spa_5_up_fifo_clr       : 1;  /* bit[15]   : SPA上行通路通路三声道AFIFO Clear Signal, high active.
                                                                   0：不清空FIFO数据
                                                                   1：清空FIFO数据 */
        unsigned int  reserved_1              : 5;  /* bit[16-20]: reserved */
        unsigned int  spa_6_up_fifo_aempty_th : 5;  /* bit[21-25]: SPA上行通路通路四声道AFIFO将空阈值。 */
        unsigned int  spa_6_up_fifo_afull_th  : 5;  /* bit[26-30]: SPA上行通路通路四声道AFIFO将满阈值。 */
        unsigned int  spa_6_up_fifo_clr       : 1;  /* bit[31]   : SPA上行通路通路四声道AFIFO Clear Signal, high active.
                                                                   0：不清空FIFO数据
                                                                   1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_spa_5_up_fifo_aempty_th_START  (5)
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_spa_5_up_fifo_aempty_th_END    (9)
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_spa_5_up_fifo_afull_th_START   (10)
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_spa_5_up_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_spa_5_up_fifo_clr_START        (15)
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_spa_5_up_fifo_clr_END          (15)
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_spa_6_up_fifo_aempty_th_START  (21)
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_spa_6_up_fifo_aempty_th_END    (25)
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_spa_6_up_fifo_afull_th_START   (26)
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_spa_6_up_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_spa_6_up_fifo_clr_START        (31)
#define SOC_ASP_CODEC_SPA56_UP_AFIFO_CTRL_spa_6_up_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_UNION
 结构说明  : AUDIO_UP_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x00F8，初值:0x7A007A00，宽度:32
 寄存器说明: AUDIO上行通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0                : 5;  /* bit[0-4]  : reserved */
        unsigned int  audio_r_up_fifo_aempty_th : 5;  /* bit[5-9]  : Audio上行通路通路右声道AFIFO将空阈值。 */
        unsigned int  audio_r_up_fifo_afull_th  : 5;  /* bit[10-14]: Audio上行通路通路右声道AFIFO将满阈值。 */
        unsigned int  audio_r_up_fifo_clr       : 1;  /* bit[15]   : Audio上行通路通路右声道AFIFO Clear Signal, high active.
                                                                     0：不清空FIFO数据
                                                                     1：清空FIFO数据 */
        unsigned int  reserved_1                : 5;  /* bit[16-20]: reserved */
        unsigned int  audio_l_up_fifo_aempty_th : 5;  /* bit[21-25]: Audio上行通路通路左声道AFIFO将空阈值。 */
        unsigned int  audio_l_up_fifo_afull_th  : 5;  /* bit[26-30]: Audio上行通路通路左声道AFIFO将满阈值。 */
        unsigned int  audio_l_up_fifo_clr       : 1;  /* bit[31]   : Audio上行通路通路左声道AFIFO Clear Signal, high active.
                                                                     0：不清空FIFO数据
                                                                     1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_audio_r_up_fifo_aempty_th_START  (5)
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_audio_r_up_fifo_aempty_th_END    (9)
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_audio_r_up_fifo_afull_th_START   (10)
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_audio_r_up_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_audio_r_up_fifo_clr_START        (15)
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_audio_r_up_fifo_clr_END          (15)
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_audio_l_up_fifo_aempty_th_START  (21)
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_audio_l_up_fifo_aempty_th_END    (25)
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_audio_l_up_fifo_afull_th_START   (26)
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_audio_l_up_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_audio_l_up_fifo_clr_START        (31)
#define SOC_ASP_CODEC_AUDIO_UP_AFIFO_CTRL_audio_l_up_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_UNION
 结构说明  : VOICE_UP_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x00FC，初值:0x7A007A00，宽度:32
 寄存器说明: VOICE上行通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0                : 5;  /* bit[0-4]  : reserved */
        unsigned int  voice_r_up_fifo_aempty_th : 5;  /* bit[5-9]  : Voice上行通路通路右声道AFIFO将空阈值。 */
        unsigned int  voice_r_up_fifo_afull_th  : 5;  /* bit[10-14]: Voice上行通路通路右声道AFIFO将满阈值。 */
        unsigned int  voice_r_up_fifo_clr       : 1;  /* bit[15]   : Voice上行通路通路右声道AFIFO Clear Signal, high active.
                                                                     0：不清空FIFO数据
                                                                     1：清空FIFO数据 */
        unsigned int  reserved_1                : 5;  /* bit[16-20]: reserved */
        unsigned int  voice_l_up_fifo_aempty_th : 5;  /* bit[21-25]: Voice上行通路通路左声道AFIFO将空阈值。 */
        unsigned int  voice_l_up_fifo_afull_th  : 5;  /* bit[26-30]: Voice上行通路通路左声道AFIFO将满阈值。 */
        unsigned int  voice_l_up_fifo_clr       : 1;  /* bit[31]   : Voice上行通路通路左声道AFIFO Clear Signal, high active.
                                                                     0：不清空FIFO数据
                                                                     1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_voice_r_up_fifo_aempty_th_START  (5)
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_voice_r_up_fifo_aempty_th_END    (9)
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_voice_r_up_fifo_afull_th_START   (10)
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_voice_r_up_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_voice_r_up_fifo_clr_START        (15)
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_voice_r_up_fifo_clr_END          (15)
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_voice_l_up_fifo_aempty_th_START  (21)
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_voice_l_up_fifo_aempty_th_END    (25)
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_voice_l_up_fifo_afull_th_START   (26)
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_voice_l_up_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_voice_l_up_fifo_clr_START        (31)
#define SOC_ASP_CODEC_VOICE_UP_AFIFO_CTRL_voice_l_up_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_UNION
 结构说明  : MIC34_UP_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x0100，初值:0x7A007A00，宽度:32
 寄存器说明: MIC34上行通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0             : 5;  /* bit[0-4]  : reserved */
        unsigned int  mic3_up_fifo_aempty_th : 5;  /* bit[5-9]  : MIC3上行通路通路右声道AFIFO将空阈值。 */
        unsigned int  mic3_up_fifo_afull_th  : 5;  /* bit[10-14]: MIC3上行通路通路右声道AFIFO将满阈值。 */
        unsigned int  mic3_up_fifo_clr       : 1;  /* bit[15]   : MIC3上行通路通路右声道AFIFO Clear Signal, high active.
                                                                  0：不清空FIFO数据
                                                                  1：清空FIFO数据 */
        unsigned int  reserved_1             : 5;  /* bit[16-20]: reserved */
        unsigned int  mic4_up_fifo_aempty_th : 5;  /* bit[21-25]: MIC4上行通路通路左声道AFIFO将空阈值。 */
        unsigned int  mic4_up_fifo_afull_th  : 5;  /* bit[26-30]: MIC4上行通路通路左声道AFIFO将满阈值。 */
        unsigned int  mic4_up_fifo_clr       : 1;  /* bit[31]   : MIC4上行通路通路左声道AFIFO Clear Signal, high active.
                                                                  0：不清空FIFO数据
                                                                  1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_mic3_up_fifo_aempty_th_START  (5)
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_mic3_up_fifo_aempty_th_END    (9)
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_mic3_up_fifo_afull_th_START   (10)
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_mic3_up_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_mic3_up_fifo_clr_START        (15)
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_mic3_up_fifo_clr_END          (15)
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_mic4_up_fifo_aempty_th_START  (21)
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_mic4_up_fifo_aempty_th_END    (25)
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_mic4_up_fifo_afull_th_START   (26)
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_mic4_up_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_mic4_up_fifo_clr_START        (31)
#define SOC_ASP_CODEC_MIC34_UP_AFIFO_CTRL_mic4_up_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_UNION
 结构说明  : MDM_5G_DN_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x0104，初值:0x7A007A00，宽度:32
 寄存器说明: 5GMDM下行通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0                 : 5;  /* bit[0-4]  : reserved */
        unsigned int  mdm_5g_r_dn_fifo_aempty_th : 5;  /* bit[5-9]  : MDM_5G下行右声道通路通路AFIFO将空阈值。 */
        unsigned int  mdm_5g_r_dn_fifo_afull_th  : 5;  /* bit[10-14]: MDM_5G下行右声道通路通路AFIFO将满阈值。 */
        unsigned int  mdm_5g_r_dn_fifo_clr       : 1;  /* bit[15]   : MDM_5G下行右声道通路通路AFIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
        unsigned int  reserved_1                 : 5;  /* bit[16-20]: reserved */
        unsigned int  mdm_5g_l_dn_fifo_aempty_th : 5;  /* bit[21-25]: MDM_5G下行左声道通路通路AFIFO将空阈值。 */
        unsigned int  mdm_5g_l_dn_fifo_afull_th  : 5;  /* bit[26-30]: MDM_5G下行左声道通路通路AFIFO将满阈值。 */
        unsigned int  mdm_5g_l_dn_fifo_clr       : 1;  /* bit[31]   : MDM_5G下行左声道通路通路AFIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_mdm_5g_r_dn_fifo_aempty_th_START  (5)
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_mdm_5g_r_dn_fifo_aempty_th_END    (9)
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_mdm_5g_r_dn_fifo_afull_th_START   (10)
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_mdm_5g_r_dn_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_mdm_5g_r_dn_fifo_clr_START        (15)
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_mdm_5g_r_dn_fifo_clr_END          (15)
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_mdm_5g_l_dn_fifo_aempty_th_START  (21)
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_mdm_5g_l_dn_fifo_aempty_th_END    (25)
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_mdm_5g_l_dn_fifo_afull_th_START   (26)
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_mdm_5g_l_dn_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_mdm_5g_l_dn_fifo_clr_START        (31)
#define SOC_ASP_CODEC_MDM_5G_DN_AFIFO_CTRL_mdm_5g_l_dn_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_UNION
 结构说明  : MDM_5G_UP_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x0108，初值:0x7A007A00，宽度:32
 寄存器说明: 5GMDM上行通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0                 : 5;  /* bit[0-4]  : reserved */
        unsigned int  mdm_5g_r_up_fifo_aempty_th : 5;  /* bit[5-9]  : MDM_5G上行通路通路右声道AFIFO将空阈值。 */
        unsigned int  mdm_5g_r_up_fifo_afull_th  : 5;  /* bit[10-14]: MDM_5G上行通路通路右声道AFIFO将满阈值。 */
        unsigned int  mdm_5g_r_up_fifo_clr       : 1;  /* bit[15]   : MDM_5G上行通路通路右声道AFIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
        unsigned int  reserved_1                 : 5;  /* bit[16-20]: reserved */
        unsigned int  mdm_5g_l_up_fifo_aempty_th : 5;  /* bit[21-25]: MDM_5G上行通路通路左声道AFIFO将空阈值。 */
        unsigned int  mdm_5g_l_up_fifo_afull_th  : 5;  /* bit[26-30]: MDM_5G上行通路通路左声道AFIFO将满阈值。 */
        unsigned int  mdm_5g_l_up_fifo_clr       : 1;  /* bit[31]   : MDM_5G上行通路通路左声道AFIFO Clear Signal, high active.
                                                                      0：不清空FIFO数据
                                                                      1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_mdm_5g_r_up_fifo_aempty_th_START  (5)
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_mdm_5g_r_up_fifo_aempty_th_END    (9)
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_mdm_5g_r_up_fifo_afull_th_START   (10)
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_mdm_5g_r_up_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_mdm_5g_r_up_fifo_clr_START        (15)
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_mdm_5g_r_up_fifo_clr_END          (15)
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_mdm_5g_l_up_fifo_aempty_th_START  (21)
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_mdm_5g_l_up_fifo_aempty_th_END    (25)
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_mdm_5g_l_up_fifo_afull_th_START   (26)
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_mdm_5g_l_up_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_mdm_5g_l_up_fifo_clr_START        (31)
#define SOC_ASP_CODEC_MDM_5G_UP_AFIFO_CTRL_mdm_5g_l_up_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_UNION
 结构说明  : SPA78_UP_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x010C，初值:0x7A007A00，宽度:32
 寄存器说明: SPA反馈通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0              : 5;  /* bit[0-4]  : reserved */
        unsigned int  spa_7_up_fifo_aempty_th : 5;  /* bit[5-9]  : SPA上行通路通路一声道AFIFO将空阈值。 */
        unsigned int  spa_7_up_fifo_afull_th  : 5;  /* bit[10-14]: SPA上行通路通路一声道AFIFO将满阈值。 */
        unsigned int  spa_7_up_fifo_clr       : 1;  /* bit[15]   : SPA上行通路通路一声道AFIFO Clear Signal, high active.
                                                                   0：不清空FIFO数据
                                                                   1：清空FIFO数据 */
        unsigned int  reserved_1              : 5;  /* bit[16-20]: reserved */
        unsigned int  spa_8_up_fifo_aempty_th : 5;  /* bit[21-25]: SPA上行通路通路二声道AFIFO将空阈值。 */
        unsigned int  spa_8_up_fifo_afull_th  : 5;  /* bit[26-30]: SPA上行通路通路二声道AFIFO将满阈值。 */
        unsigned int  spa_8_up_fifo_clr       : 1;  /* bit[31]   : SPA上行通路通路二声道AFIFO Clear Signal, high active.
                                                                   0：不清空FIFO数据
                                                                   1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_spa_7_up_fifo_aempty_th_START  (5)
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_spa_7_up_fifo_aempty_th_END    (9)
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_spa_7_up_fifo_afull_th_START   (10)
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_spa_7_up_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_spa_7_up_fifo_clr_START        (15)
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_spa_7_up_fifo_clr_END          (15)
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_spa_8_up_fifo_aempty_th_START  (21)
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_spa_8_up_fifo_aempty_th_END    (25)
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_spa_8_up_fifo_afull_th_START   (26)
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_spa_8_up_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_spa_8_up_fifo_clr_START        (31)
#define SOC_ASP_CODEC_SPA78_UP_AFIFO_CTRL_spa_8_up_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_SIF_CTRL_UNION
 结构说明  : SIF_CTRL 寄存器结构定义。地址偏移量:0x0110，初值:0x00000000，宽度:32
 寄存器说明: SIF_CTRL控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  sif_mst_dac_edge_sel      : 1;  /* bit[0]    : sif_dac_clk的正反沿选择：
                                                                     0：正沿；
                                                                     1：反沿。 */
        unsigned int  sif_mst_adc_edge_sel      : 1;  /* bit[1]    : sif_adc_clk的正反沿选择：
                                                                     0：正沿；
                                                                     1：反沿。 */
        unsigned int  sif_s2p_loop              : 2;  /* bit[2-3]  : sif的环回配置：
                                                                     2'b00：正常工作；
                                                                     2'b01：下行并行的输入环回到并行的输出0~4；
                                                                     2'b10：并行的输入环回到并行的输出，5~9；
                                                                     其它：保留 */
        unsigned int  sif_p2s_loop              : 2;  /* bit[4-5]  : sif的环回配置：
                                                                     2'b00：正常工作；
                                                                     2'b01：上行并行的输出环回到下行并行的输入0~4；
                                                                     2'b10：上行并行的输出环回到下行并行的输入5~9；
                                                                     其它：保留 */
        unsigned int  sif_spa2_mst_dac_edge_sel : 1;  /* bit[6]    : sif_spa2_dac_clk的正反沿选择：
                                                                     0：正沿；
                                                                     1：反沿。 */
        unsigned int  sif_spa2_mst_adc_edge_sel : 1;  /* bit[7]    : sif_spa2_adc_clk的正反沿选择：
                                                                     0：正沿；
                                                                     1：反沿。 */
        unsigned int  sif_spa1_mst_dac_edge_sel : 1;  /* bit[8]    : sif_spa1_dac_clk的正反沿选择：
                                                                     0：正沿；
                                                                     1：反沿。 */
        unsigned int  sif_spa1_mst_adc_edge_sel : 1;  /* bit[9]    : sif_spa1_adc_clk的正反沿选择：
                                                                     0：正沿；
                                                                     1：反沿。 */
        unsigned int  reserved                  : 22; /* bit[10-31]: reserved */
    } reg;
} SOC_ASP_CODEC_SIF_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_SIF_CTRL_sif_mst_dac_edge_sel_START       (0)
#define SOC_ASP_CODEC_SIF_CTRL_sif_mst_dac_edge_sel_END         (0)
#define SOC_ASP_CODEC_SIF_CTRL_sif_mst_adc_edge_sel_START       (1)
#define SOC_ASP_CODEC_SIF_CTRL_sif_mst_adc_edge_sel_END         (1)
#define SOC_ASP_CODEC_SIF_CTRL_sif_s2p_loop_START               (2)
#define SOC_ASP_CODEC_SIF_CTRL_sif_s2p_loop_END                 (3)
#define SOC_ASP_CODEC_SIF_CTRL_sif_p2s_loop_START               (4)
#define SOC_ASP_CODEC_SIF_CTRL_sif_p2s_loop_END                 (5)
#define SOC_ASP_CODEC_SIF_CTRL_sif_spa2_mst_dac_edge_sel_START  (6)
#define SOC_ASP_CODEC_SIF_CTRL_sif_spa2_mst_dac_edge_sel_END    (6)
#define SOC_ASP_CODEC_SIF_CTRL_sif_spa2_mst_adc_edge_sel_START  (7)
#define SOC_ASP_CODEC_SIF_CTRL_sif_spa2_mst_adc_edge_sel_END    (7)
#define SOC_ASP_CODEC_SIF_CTRL_sif_spa1_mst_dac_edge_sel_START  (8)
#define SOC_ASP_CODEC_SIF_CTRL_sif_spa1_mst_dac_edge_sel_END    (8)
#define SOC_ASP_CODEC_SIF_CTRL_sif_spa1_mst_adc_edge_sel_START  (9)
#define SOC_ASP_CODEC_SIF_CTRL_sif_spa1_mst_adc_edge_sel_END    (9)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_STATE_UNION
 结构说明  : CODEC_STATE 寄存器结构定义。地址偏移量:0x0114，初值:0x00000000，宽度:32
 寄存器说明: CODEC通道内模块状态查询寄存器0
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  codec3_l_dn_srcup_rdy : 1;  /* bit[0]    : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  codec3_r_dn_srcup_rdy : 1;  /* bit[1]    : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  audio_l_dn_srcup_rdy  : 1;  /* bit[2]    : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  audio_r_dn_srcup_rdy  : 1;  /* bit[3]    : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  i2s2_rx_l_srcup_rdy   : 1;  /* bit[4]    : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  i2s2_rx_r_srcup_rdy   : 1;  /* bit[5]    : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  dac_l_mixer_srcup_rdy : 1;  /* bit[6]    : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  dac_r_mixer_srcup_rdy : 1;  /* bit[7]    : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  audio_l_up_srcdn_rdy  : 1;  /* bit[8]    : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  audio_r_up_srcdn_rdy  : 1;  /* bit[9]    : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  voice_l_up_srcdn_rdy  : 1;  /* bit[10]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  voice_r_up_srcdn_rdy  : 1;  /* bit[11]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  mic3_up_srcdn_rdy     : 1;  /* bit[12]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  mic4_up_srcdn_rdy     : 1;  /* bit[13]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  mdm_5g_l_up_srcdn_rdy : 1;  /* bit[14]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  mdm_5g_r_up_srcdn_rdy : 1;  /* bit[15]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  i2s2_rx_l_srcdn_rdy   : 1;  /* bit[16]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  i2s2_rx_r_srcdn_rdy   : 1;  /* bit[17]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  i2s2_tx_l_srcdn_rdy   : 1;  /* bit[18]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  i2s2_tx_r_srcdn_rdy   : 1;  /* bit[19]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  mic5_up_srcdn_rdy     : 1;  /* bit[20]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  mic6_up_srcdn_rdy     : 1;  /* bit[21]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  mic7_up_srcdn_rdy     : 1;  /* bit[22]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  mic8_up_srcdn_rdy     : 1;  /* bit[23]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  sif_dac_l_srcup_rdy   : 1;  /* bit[24]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  sif_dac_r_srcup_rdy   : 1;  /* bit[25]   : SRC Ready Signal, high active
                                                                 0：not ready
                                                                 1：ready */
        unsigned int  reserved              : 6;  /* bit[26-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_STATE_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_STATE_codec3_l_dn_srcup_rdy_START  (0)
#define SOC_ASP_CODEC_CODEC_STATE_codec3_l_dn_srcup_rdy_END    (0)
#define SOC_ASP_CODEC_CODEC_STATE_codec3_r_dn_srcup_rdy_START  (1)
#define SOC_ASP_CODEC_CODEC_STATE_codec3_r_dn_srcup_rdy_END    (1)
#define SOC_ASP_CODEC_CODEC_STATE_audio_l_dn_srcup_rdy_START   (2)
#define SOC_ASP_CODEC_CODEC_STATE_audio_l_dn_srcup_rdy_END     (2)
#define SOC_ASP_CODEC_CODEC_STATE_audio_r_dn_srcup_rdy_START   (3)
#define SOC_ASP_CODEC_CODEC_STATE_audio_r_dn_srcup_rdy_END     (3)
#define SOC_ASP_CODEC_CODEC_STATE_i2s2_rx_l_srcup_rdy_START    (4)
#define SOC_ASP_CODEC_CODEC_STATE_i2s2_rx_l_srcup_rdy_END      (4)
#define SOC_ASP_CODEC_CODEC_STATE_i2s2_rx_r_srcup_rdy_START    (5)
#define SOC_ASP_CODEC_CODEC_STATE_i2s2_rx_r_srcup_rdy_END      (5)
#define SOC_ASP_CODEC_CODEC_STATE_dac_l_mixer_srcup_rdy_START  (6)
#define SOC_ASP_CODEC_CODEC_STATE_dac_l_mixer_srcup_rdy_END    (6)
#define SOC_ASP_CODEC_CODEC_STATE_dac_r_mixer_srcup_rdy_START  (7)
#define SOC_ASP_CODEC_CODEC_STATE_dac_r_mixer_srcup_rdy_END    (7)
#define SOC_ASP_CODEC_CODEC_STATE_audio_l_up_srcdn_rdy_START   (8)
#define SOC_ASP_CODEC_CODEC_STATE_audio_l_up_srcdn_rdy_END     (8)
#define SOC_ASP_CODEC_CODEC_STATE_audio_r_up_srcdn_rdy_START   (9)
#define SOC_ASP_CODEC_CODEC_STATE_audio_r_up_srcdn_rdy_END     (9)
#define SOC_ASP_CODEC_CODEC_STATE_voice_l_up_srcdn_rdy_START   (10)
#define SOC_ASP_CODEC_CODEC_STATE_voice_l_up_srcdn_rdy_END     (10)
#define SOC_ASP_CODEC_CODEC_STATE_voice_r_up_srcdn_rdy_START   (11)
#define SOC_ASP_CODEC_CODEC_STATE_voice_r_up_srcdn_rdy_END     (11)
#define SOC_ASP_CODEC_CODEC_STATE_mic3_up_srcdn_rdy_START      (12)
#define SOC_ASP_CODEC_CODEC_STATE_mic3_up_srcdn_rdy_END        (12)
#define SOC_ASP_CODEC_CODEC_STATE_mic4_up_srcdn_rdy_START      (13)
#define SOC_ASP_CODEC_CODEC_STATE_mic4_up_srcdn_rdy_END        (13)
#define SOC_ASP_CODEC_CODEC_STATE_mdm_5g_l_up_srcdn_rdy_START  (14)
#define SOC_ASP_CODEC_CODEC_STATE_mdm_5g_l_up_srcdn_rdy_END    (14)
#define SOC_ASP_CODEC_CODEC_STATE_mdm_5g_r_up_srcdn_rdy_START  (15)
#define SOC_ASP_CODEC_CODEC_STATE_mdm_5g_r_up_srcdn_rdy_END    (15)
#define SOC_ASP_CODEC_CODEC_STATE_i2s2_rx_l_srcdn_rdy_START    (16)
#define SOC_ASP_CODEC_CODEC_STATE_i2s2_rx_l_srcdn_rdy_END      (16)
#define SOC_ASP_CODEC_CODEC_STATE_i2s2_rx_r_srcdn_rdy_START    (17)
#define SOC_ASP_CODEC_CODEC_STATE_i2s2_rx_r_srcdn_rdy_END      (17)
#define SOC_ASP_CODEC_CODEC_STATE_i2s2_tx_l_srcdn_rdy_START    (18)
#define SOC_ASP_CODEC_CODEC_STATE_i2s2_tx_l_srcdn_rdy_END      (18)
#define SOC_ASP_CODEC_CODEC_STATE_i2s2_tx_r_srcdn_rdy_START    (19)
#define SOC_ASP_CODEC_CODEC_STATE_i2s2_tx_r_srcdn_rdy_END      (19)
#define SOC_ASP_CODEC_CODEC_STATE_mic5_up_srcdn_rdy_START      (20)
#define SOC_ASP_CODEC_CODEC_STATE_mic5_up_srcdn_rdy_END        (20)
#define SOC_ASP_CODEC_CODEC_STATE_mic6_up_srcdn_rdy_START      (21)
#define SOC_ASP_CODEC_CODEC_STATE_mic6_up_srcdn_rdy_END        (21)
#define SOC_ASP_CODEC_CODEC_STATE_mic7_up_srcdn_rdy_START      (22)
#define SOC_ASP_CODEC_CODEC_STATE_mic7_up_srcdn_rdy_END        (22)
#define SOC_ASP_CODEC_CODEC_STATE_mic8_up_srcdn_rdy_START      (23)
#define SOC_ASP_CODEC_CODEC_STATE_mic8_up_srcdn_rdy_END        (23)
#define SOC_ASP_CODEC_CODEC_STATE_sif_dac_l_srcup_rdy_START    (24)
#define SOC_ASP_CODEC_CODEC_STATE_sif_dac_l_srcup_rdy_END      (24)
#define SOC_ASP_CODEC_CODEC_STATE_sif_dac_r_srcup_rdy_START    (25)
#define SOC_ASP_CODEC_CODEC_STATE_sif_dac_r_srcup_rdy_END      (25)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_FS_CTRL0_UNION
 结构说明  : FS_CTRL0 寄存器结构定义。地址偏移量:0x0118，初值:0x0003F6DB，宽度:32
 寄存器说明: CODEC内部模块采样率控制寄存器0
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  fs_spa_1_up_afifo    : 3;  /* bit[0-2]  : spa上行通路1声道输入端采样率控制：
                                                                3'b000：保留；
                                                                3'b001：保留；
                                                                3'b010：保留；
                                                                3'b011：48k；
                                                                3'b100 : 96k ;
                                                                其它 ： 保留 ； */
        unsigned int  fs_spa_2_up_afifo    : 3;  /* bit[3-5]  : spa上行通路2声道输入端采样率控制：
                                                                3'b000：保留；
                                                                3'b001：保留；
                                                                3'b010：保留；
                                                                3'b011：48k；
                                                                3'b100 : 96k ;
                                                                其它 ： 保留 ； */
        unsigned int  fs_spa_3_up_afifo    : 3;  /* bit[6-8]  : spa上行通路3声道输入端采样率控制：
                                                                3'b000：保留；
                                                                3'b001：保留；
                                                                3'b010：保留；
                                                                3'b011：48k；
                                                                3'b100 : 96k ;
                                                                其它 ： 保留 ； */
        unsigned int  fs_spa_4_up_afifo    : 3;  /* bit[9-11] : spa上行通路4声道输入端采样率控制：
                                                                3'b000：保留；
                                                                3'b001：保留；
                                                                3'b010：保留；
                                                                3'b011：48k；
                                                                3'b100 : 96k ;
                                                                其它 ： 保留 ； */
        unsigned int  fs_codec3_l_dn_afifo : 3;  /* bit[12-14]: fs_codec3上行通路左声道FIFO的采样率控制：（44.1k~176.4k仅在
                                                                slv_mode_441==1'b1时生效）
                                                                3'b000：8k;
                                                                3'b001：16k；
                                                                3'b010：32k；
                                                                3'b011：48k/44.1k；
                                                                3'b100：保留；
                                                                3'b101：96k/88.2k；
                                                                3'b110：192k/176.4k；
                                                                3'b111 : 384k/352.8k ;
                                                                其它：48k； */
        unsigned int  fs_codec3_r_dn_afifo : 3;  /* bit[15-17]: fs_codec3上行通路左声道FIFO的采样率控制：（44.1k~176.4k仅在
                                                                slv_mode_441==1'b1时生效）
                                                                3'b000：8k;
                                                                3'b001：16k；
                                                                3'b010：32k；
                                                                3'b011：48k/44.1k；
                                                                3'b100：保留；
                                                                3'b101：96k/88.2k；
                                                                3'b110：192k/176.4k；
                                                                3'b111 : 384k/352.8k ;
                                                                其它：48k； */
        unsigned int  fs_audio_l_dn_afifo  : 2;  /* bit[18-19]: audio_dnlink上行通路左声道FIFO的采样率控制：
                                                                2'b00：48k；
                                                                2'b01：96k；
                                                                2'b10：192k；
                                                                其它：保留； */
        unsigned int  fs_audio_r_dn_afifo  : 2;  /* bit[20-21]: audio_dnlink上行通路右声道FIFO的采样率控制：
                                                                2'b00：48k；
                                                                2'b01：96k；
                                                                2'b10：192k；
                                                                其它：保留； */
        unsigned int  reserved             : 10; /* bit[22-31]: reserved */
    } reg;
} SOC_ASP_CODEC_FS_CTRL0_UNION;
#endif
#define SOC_ASP_CODEC_FS_CTRL0_fs_spa_1_up_afifo_START     (0)
#define SOC_ASP_CODEC_FS_CTRL0_fs_spa_1_up_afifo_END       (2)
#define SOC_ASP_CODEC_FS_CTRL0_fs_spa_2_up_afifo_START     (3)
#define SOC_ASP_CODEC_FS_CTRL0_fs_spa_2_up_afifo_END       (5)
#define SOC_ASP_CODEC_FS_CTRL0_fs_spa_3_up_afifo_START     (6)
#define SOC_ASP_CODEC_FS_CTRL0_fs_spa_3_up_afifo_END       (8)
#define SOC_ASP_CODEC_FS_CTRL0_fs_spa_4_up_afifo_START     (9)
#define SOC_ASP_CODEC_FS_CTRL0_fs_spa_4_up_afifo_END       (11)
#define SOC_ASP_CODEC_FS_CTRL0_fs_codec3_l_dn_afifo_START  (12)
#define SOC_ASP_CODEC_FS_CTRL0_fs_codec3_l_dn_afifo_END    (14)
#define SOC_ASP_CODEC_FS_CTRL0_fs_codec3_r_dn_afifo_START  (15)
#define SOC_ASP_CODEC_FS_CTRL0_fs_codec3_r_dn_afifo_END    (17)
#define SOC_ASP_CODEC_FS_CTRL0_fs_audio_l_dn_afifo_START   (18)
#define SOC_ASP_CODEC_FS_CTRL0_fs_audio_l_dn_afifo_END     (19)
#define SOC_ASP_CODEC_FS_CTRL0_fs_audio_r_dn_afifo_START   (20)
#define SOC_ASP_CODEC_FS_CTRL0_fs_audio_r_dn_afifo_END     (21)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_FS_CTRL1_UNION
 结构说明  : FS_CTRL1 寄存器结构定义。地址偏移量:0x011C，初值:0x00B64492，宽度:32
 寄存器说明: CODEC内部模块采样率控制寄存器1
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  fs_voice_l_up_afifo  : 3;  /* bit[0-2]  : voice上行通路左声道输入端采样率控制：
                                                                3'b000：8k；
                                                                3'b001：16k；
                                                                3'b010：32k；
                                                                3'b011：48k；
                                                                3'b100 : 96k ;
                                                                其它 ： 保留 ； */
        unsigned int  fs_voice_r_up_afifo  : 3;  /* bit[3-5]  : voice上行通路右声道输入端采样率控制：
                                                                3'b000：8k；
                                                                3'b001：16k；
                                                                3'b010：32k；
                                                                3'b011：48k；
                                                                3'b100 : 96k ;
                                                                其它 ： 保留 ； */
        unsigned int  fs_mic3_up_afifo     : 3;  /* bit[6-8]  : mic3上行通路输入端采样率控制：
                                                                3'b000：8k；
                                                                3'b001：16k；
                                                                3'b010：32k；
                                                                3'b011：48k；
                                                                3'b100 : 96k ;
                                                                其它 ： 保留 ； */
        unsigned int  fs_mic4_up_afifo     : 3;  /* bit[9-11] : mic4上行通路输入端采样率控制：
                                                                3'b000：8k；
                                                                3'b001：16k；
                                                                3'b010：32k；
                                                                3'b011：48k；
                                                                3'b100 : 96k ;
                                                                其它 ： 保留 ； */
        unsigned int  fs_mdm_5g_l_dn_afifo : 3;  /* bit[12-14]: 5G MDM下行通路左声道输入端采样率控制：
                                                                3'b000：保留；
                                                                3'b001：保留；
                                                                3'b010：保留；
                                                                3'b011：48k；
                                                                3'b100 : 96k ;
                                                                其它 ： 保留 ； */
        unsigned int  fs_mdm_5g_r_dn_afifo : 3;  /* bit[15-17]: 5G MDM下行通路右声道输入端采样率控制：
                                                                3'b000：保留；
                                                                3'b001：保留；
                                                                3'b010：保留；
                                                                3'b011：48k；
                                                                3'b100 : 96k ;
                                                                其它 ： 保留 ； */
        unsigned int  fs_mdm_5g_l_up_afifo : 3;  /* bit[18-20]: fs_5g_mdm上行通路左声道FIFO的采样率控制：（44.1k~176.4k仅在
                                                                slv_mode_441==1'b1时生效）
                                                                3'b000：8k;
                                                                3'b001：16k；
                                                                3'b010：32k；
                                                                3'b011：48k/44.1k；
                                                                3'b100：保留；
                                                                3'b101：96k/88.2k；
                                                                3'b110：192k/176.4k；
                                                                3'b111 : 384k/352.8k ;
                                                                其它：48k； */
        unsigned int  fs_mdm_5g_r_up_afifo : 3;  /* bit[21-23]: fs_5g_mdm上行通路左声道FIFO的采样率控制：（44.1k~176.4k仅在
                                                                slv_mode_441==1'b1时生效）
                                                                3'b000：8k;
                                                                3'b001：16k；
                                                                3'b010：32k；
                                                                3'b011：48k/44.1k；
                                                                3'b100：保留；
                                                                3'b101：96k/88.2k；
                                                                3'b110：192k/176.4k；
                                                                3'b111 : 384k/352.8k ;
                                                                其它：48k； */
        unsigned int  reserved             : 4;  /* bit[24-27]: reserved */
        unsigned int  fs_audio_l_uplink    : 2;  /* bit[28-29]: audio_uplink上行通路左声道FIFO的采样率控制：
                                                                2'b00：48k；
                                                                2'b01：96k；
                                                                2'b10：192k；
                                                                其它：保留； */
        unsigned int  fs_audio_r_uplink    : 2;  /* bit[30-31]: audio_uplink上行通路右声道FIFO的采样率控制：
                                                                2'b00：48k；
                                                                2'b01：96k；
                                                                2'b10：192k；
                                                                其它：保留； */
    } reg;
} SOC_ASP_CODEC_FS_CTRL1_UNION;
#endif
#define SOC_ASP_CODEC_FS_CTRL1_fs_voice_l_up_afifo_START   (0)
#define SOC_ASP_CODEC_FS_CTRL1_fs_voice_l_up_afifo_END     (2)
#define SOC_ASP_CODEC_FS_CTRL1_fs_voice_r_up_afifo_START   (3)
#define SOC_ASP_CODEC_FS_CTRL1_fs_voice_r_up_afifo_END     (5)
#define SOC_ASP_CODEC_FS_CTRL1_fs_mic3_up_afifo_START      (6)
#define SOC_ASP_CODEC_FS_CTRL1_fs_mic3_up_afifo_END        (8)
#define SOC_ASP_CODEC_FS_CTRL1_fs_mic4_up_afifo_START      (9)
#define SOC_ASP_CODEC_FS_CTRL1_fs_mic4_up_afifo_END        (11)
#define SOC_ASP_CODEC_FS_CTRL1_fs_mdm_5g_l_dn_afifo_START  (12)
#define SOC_ASP_CODEC_FS_CTRL1_fs_mdm_5g_l_dn_afifo_END    (14)
#define SOC_ASP_CODEC_FS_CTRL1_fs_mdm_5g_r_dn_afifo_START  (15)
#define SOC_ASP_CODEC_FS_CTRL1_fs_mdm_5g_r_dn_afifo_END    (17)
#define SOC_ASP_CODEC_FS_CTRL1_fs_mdm_5g_l_up_afifo_START  (18)
#define SOC_ASP_CODEC_FS_CTRL1_fs_mdm_5g_l_up_afifo_END    (20)
#define SOC_ASP_CODEC_FS_CTRL1_fs_mdm_5g_r_up_afifo_START  (21)
#define SOC_ASP_CODEC_FS_CTRL1_fs_mdm_5g_r_up_afifo_END    (23)
#define SOC_ASP_CODEC_FS_CTRL1_fs_audio_l_uplink_START     (28)
#define SOC_ASP_CODEC_FS_CTRL1_fs_audio_l_uplink_END       (29)
#define SOC_ASP_CODEC_FS_CTRL1_fs_audio_r_uplink_START     (30)
#define SOC_ASP_CODEC_FS_CTRL1_fs_audio_r_uplink_END       (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_FS_CTRL2_UNION
 结构说明  : FS_CTRL2 寄存器结构定义。地址偏移量:0x0120，初值:0xB524801B，宽度:32
 寄存器说明: CODEC内部模块采样率控制寄存器2
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  fs_codec3_l_dn_pga : 3;  /* bit[0-2]  : fs_codec3上行通路左声道PGA的采样率控制：（44.1k~176.4k仅在
                                                              slv_mode_441==1'b1时生效）
                                                              3'b000：8k;
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k/44.1k；
                                                              3'b100：保留；
                                                              3'b101：96k/88.2k；
                                                              3'b110：192k/176.4k；
                                                              3'b111 : 384k/352.8k ;
                                                              其它：48k； */
        unsigned int  fs_codec3_r_dn_pga : 3;  /* bit[3-5]  : fs_codec3上行通路右声道PGA的采样率控制：（44.1k~176.4k仅在
                                                              slv_mode_441==1'b1时生效）
                                                              3'b000：8k;
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k/44.1k；
                                                              3'b100：保留；
                                                              3'b101：96k/88.2k；
                                                              3'b110：192k/176.4k；
                                                              3'b111 : 384k/352.8k ;
                                                              其它：48k； */
        unsigned int  fs_audio_l_dn_pga  : 2;  /* bit[6-7]  : audio_dnlink下行通路左声道PGA的采样率控制：
                                                              2'b00：48k；
                                                              2'b01：96k；
                                                              2'b10：192k；
                                                              其它：保留； */
        unsigned int  fs_audio_r_dn_pga  : 2;  /* bit[8-9]  : audio_dnlink下行通路右声道PGA的采样率控制：
                                                              2'b00：48k；
                                                              2'b01：96k；
                                                              2'b10：192k；
                                                              其它：保留； */
        unsigned int  fs_audio_l_up_pga  : 2;  /* bit[10-11]: audio_uplink上行通路左声道PGA的采样率控制：
                                                              2'b00：48k；
                                                              2'b01：96k；
                                                              2'b10：192k；
                                                              其它：保留； */
        unsigned int  fs_audio_r_up_pga  : 2;  /* bit[12-13]: audio_uplink上行通路右声道PGA的采样率控制：
                                                              2'b00：48k；
                                                              2'b01：96k；
                                                              2'b10：192k；
                                                              其它：保留； */
        unsigned int  fs_voice_l_up_pga  : 3;  /* bit[14-16]: voice上行通路左声道PGA输入端采样率控制：
                                                              3'b000：8k；
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k；
                                                              3'b100 : 96k ;
                                                              其它 ： 保留 ； */
        unsigned int  fs_voice_r_up_pga  : 3;  /* bit[17-19]: voice上行通路右声道PGA输入端采样率控制：
                                                              3'b000：8k；
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k；
                                                              3'b100 : 96k ;
                                                              其它 ： 保留 ； */
        unsigned int  fs_mic3_up_pga     : 3;  /* bit[20-22]: MIC3上行通路声道PGA输入端采样率控制：
                                                              3'b000：8k；
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k；
                                                              3'b100 : 96k ;
                                                              其它 ： 保留 ； */
        unsigned int  fs_mic4_up_pga     : 3;  /* bit[23-25]: MIC4上行通路声道PGA输入端采样率控制：
                                                              3'b000：8k；
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k；
                                                              3'b100 : 96k ;
                                                              其它 ： 保留 ； */
        unsigned int  fs_mdm_5g_l_up_pga : 3;  /* bit[26-28]: fs_mdm_5g上行通路左声道PGA的采样率控制：
                                                              3'b000：8k;
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k；
                                                              3'b100：保留；
                                                              3'b101：96k；
                                                              3'b110：192k；
                                                              3'b111 : 384k ;
                                                              其它：48k； */
        unsigned int  fs_mdm_5g_r_up_pga : 3;  /* bit[29-31]: fs_mdm_5g上行通路右声道PGA的采样率控制：
                                                              3'b000：8k;
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k；
                                                              3'b100：保留；
                                                              3'b101：96k；
                                                              3'b110：192k；
                                                              3'b111 : 384k ;
                                                              其它：48k； */
    } reg;
} SOC_ASP_CODEC_FS_CTRL2_UNION;
#endif
#define SOC_ASP_CODEC_FS_CTRL2_fs_codec3_l_dn_pga_START  (0)
#define SOC_ASP_CODEC_FS_CTRL2_fs_codec3_l_dn_pga_END    (2)
#define SOC_ASP_CODEC_FS_CTRL2_fs_codec3_r_dn_pga_START  (3)
#define SOC_ASP_CODEC_FS_CTRL2_fs_codec3_r_dn_pga_END    (5)
#define SOC_ASP_CODEC_FS_CTRL2_fs_audio_l_dn_pga_START   (6)
#define SOC_ASP_CODEC_FS_CTRL2_fs_audio_l_dn_pga_END     (7)
#define SOC_ASP_CODEC_FS_CTRL2_fs_audio_r_dn_pga_START   (8)
#define SOC_ASP_CODEC_FS_CTRL2_fs_audio_r_dn_pga_END     (9)
#define SOC_ASP_CODEC_FS_CTRL2_fs_audio_l_up_pga_START   (10)
#define SOC_ASP_CODEC_FS_CTRL2_fs_audio_l_up_pga_END     (11)
#define SOC_ASP_CODEC_FS_CTRL2_fs_audio_r_up_pga_START   (12)
#define SOC_ASP_CODEC_FS_CTRL2_fs_audio_r_up_pga_END     (13)
#define SOC_ASP_CODEC_FS_CTRL2_fs_voice_l_up_pga_START   (14)
#define SOC_ASP_CODEC_FS_CTRL2_fs_voice_l_up_pga_END     (16)
#define SOC_ASP_CODEC_FS_CTRL2_fs_voice_r_up_pga_START   (17)
#define SOC_ASP_CODEC_FS_CTRL2_fs_voice_r_up_pga_END     (19)
#define SOC_ASP_CODEC_FS_CTRL2_fs_mic3_up_pga_START      (20)
#define SOC_ASP_CODEC_FS_CTRL2_fs_mic3_up_pga_END        (22)
#define SOC_ASP_CODEC_FS_CTRL2_fs_mic4_up_pga_START      (23)
#define SOC_ASP_CODEC_FS_CTRL2_fs_mic4_up_pga_END        (25)
#define SOC_ASP_CODEC_FS_CTRL2_fs_mdm_5g_l_up_pga_START  (26)
#define SOC_ASP_CODEC_FS_CTRL2_fs_mdm_5g_l_up_pga_END    (28)
#define SOC_ASP_CODEC_FS_CTRL2_fs_mdm_5g_r_up_pga_START  (29)
#define SOC_ASP_CODEC_FS_CTRL2_fs_mdm_5g_r_up_pga_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_FS_CTRL3_UNION
 结构说明  : FS_CTRL3 寄存器结构定义。地址偏移量:0x0124，初值:0x9001B6DB，宽度:32
 寄存器说明: CODEC内部模块采样率控制寄存器3
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  fs_i2s2_rx_l_pga          : 3;  /* bit[0-2]  : I2S2上行通路左声道PGA输入端采样率控制：
                                                                     3'b000：8k；
                                                                     3'b001：16k；
                                                                     3'b010：32k；
                                                                     3'b011：48k；
                                                                     3'b100 : 96k ;
                                                                     3'b101：192k；
                                                                     其它 ： 保留 ； */
        unsigned int  fs_i2s2_rx_r_pga          : 3;  /* bit[3-5]  : I2S2上行通路右声道PGA输入端采样率控制：
                                                                     3'b000：8k；
                                                                     3'b001：16k；
                                                                     3'b010：32k；
                                                                     3'b011：48k；
                                                                     3'b100 : 96k ;
                                                                     3'b101：192k；
                                                                     其它 ： 保留 ； */
        unsigned int  fs_codec3_l_dn_srcup_din  : 3;  /* bit[6-8]  : fs_codec3上行通路左声道SRCUP输入的采样率控制：
                                                                     3'b000：8k;
                                                                     3'b001：16k；
                                                                     3'b010：32k；
                                                                     3'b011：48k；
                                                                     3'b100：保留；
                                                                     3'b101：96k；
                                                                     3'b110：保留；
                                                                     3'b111 : 保留 ;
                                                                     其它：48k； */
        unsigned int  fs_codec3_l_dn_srcup_dout : 3;  /* bit[9-11] : fs_codec3上行通路左声道SRCUP输出的采样率控制：
                                                                     3'b000：8k;
                                                                     3'b001：16k；
                                                                     3'b010：32k；
                                                                     3'b011：48k；
                                                                     3'b100：保留；
                                                                     3'b101：96k；
                                                                     3'b110：保留；
                                                                     3'b111 : 保留 ;
                                                                     其它：48k； */
        unsigned int  fs_codec3_r_dn_srcup_din  : 3;  /* bit[12-14]: fs_codec3上行通路左声道SRCUP输入的采样率控制：
                                                                     3'b000：8k;
                                                                     3'b001：16k；
                                                                     3'b010：32k；
                                                                     3'b011：48k；
                                                                     3'b100：保留；
                                                                     3'b101：96k；
                                                                     3'b110：保留；
                                                                     3'b111 : 保留 ;
                                                                     其它：48k； */
        unsigned int  fs_codec3_r_dn_srcup_dout : 3;  /* bit[15-17]: fs_codec3上行通路左声道SRCUP输出的采样率控制：
                                                                     3'b000：8k;
                                                                     3'b001：16k；
                                                                     3'b010：32k；
                                                                     3'b011：48k；
                                                                     3'b100：保留；
                                                                     3'b101：96k；
                                                                     3'b110：保留；
                                                                     3'b111 : 保留 ;
                                                                     其它：48k； */
        unsigned int  fs_audio_l_dn_srcup_din   : 2;  /* bit[18-19]: audio_dnlink下行通路左声道SRCUP输入的采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     2'b10：保留；
                                                                     其它：保留； */
        unsigned int  fs_audio_l_dn_srcup_dout  : 2;  /* bit[20-21]: audio_dnlink下行通路左声道SRCUP输出的采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     2'b10：保留；
                                                                     其它：保留； */
        unsigned int  fs_audio_r_dn_srcup_din   : 2;  /* bit[22-23]: audio_dnlink下行通路右声道SRCUP输入的采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     2'b10：保留；
                                                                     其它：保留； */
        unsigned int  fs_audio_r_dn_srcup_dout  : 2;  /* bit[24-25]: audio_dnlink下行通路右声道SRCUP输出的采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     2'b10：保留；
                                                                     其它：保留； */
        unsigned int  fs_i2s2_rx_l_srcup_din    : 3;  /* bit[26-28]: i2s2上行通路左声道SRCUP输入端采样率控制：
                                                                     3'b000：8k；
                                                                     3'b001：16k；
                                                                     3'b010：32k；
                                                                     3'b011：48k；
                                                                     3'b100 : 96k ;
                                                                     3'b101：保留;
                                                                     其它 ： 保留 ； */
        unsigned int  fs_i2s2_rx_l_srcup_dout   : 3;  /* bit[29-31]: i2s2上行通路左声道SRCUP输出端采样率控制：
                                                                     3'b000：保留；
                                                                     3'b001：保留；
                                                                     3'b010：保留；
                                                                     3'b011：保留；
                                                                     3'b100 : 96k ;
                                                                     3'b101：保留;
                                                                     其它 ： 保留 ； */
    } reg;
} SOC_ASP_CODEC_FS_CTRL3_UNION;
#endif
#define SOC_ASP_CODEC_FS_CTRL3_fs_i2s2_rx_l_pga_START           (0)
#define SOC_ASP_CODEC_FS_CTRL3_fs_i2s2_rx_l_pga_END             (2)
#define SOC_ASP_CODEC_FS_CTRL3_fs_i2s2_rx_r_pga_START           (3)
#define SOC_ASP_CODEC_FS_CTRL3_fs_i2s2_rx_r_pga_END             (5)
#define SOC_ASP_CODEC_FS_CTRL3_fs_codec3_l_dn_srcup_din_START   (6)
#define SOC_ASP_CODEC_FS_CTRL3_fs_codec3_l_dn_srcup_din_END     (8)
#define SOC_ASP_CODEC_FS_CTRL3_fs_codec3_l_dn_srcup_dout_START  (9)
#define SOC_ASP_CODEC_FS_CTRL3_fs_codec3_l_dn_srcup_dout_END    (11)
#define SOC_ASP_CODEC_FS_CTRL3_fs_codec3_r_dn_srcup_din_START   (12)
#define SOC_ASP_CODEC_FS_CTRL3_fs_codec3_r_dn_srcup_din_END     (14)
#define SOC_ASP_CODEC_FS_CTRL3_fs_codec3_r_dn_srcup_dout_START  (15)
#define SOC_ASP_CODEC_FS_CTRL3_fs_codec3_r_dn_srcup_dout_END    (17)
#define SOC_ASP_CODEC_FS_CTRL3_fs_audio_l_dn_srcup_din_START    (18)
#define SOC_ASP_CODEC_FS_CTRL3_fs_audio_l_dn_srcup_din_END      (19)
#define SOC_ASP_CODEC_FS_CTRL3_fs_audio_l_dn_srcup_dout_START   (20)
#define SOC_ASP_CODEC_FS_CTRL3_fs_audio_l_dn_srcup_dout_END     (21)
#define SOC_ASP_CODEC_FS_CTRL3_fs_audio_r_dn_srcup_din_START    (22)
#define SOC_ASP_CODEC_FS_CTRL3_fs_audio_r_dn_srcup_din_END      (23)
#define SOC_ASP_CODEC_FS_CTRL3_fs_audio_r_dn_srcup_dout_START   (24)
#define SOC_ASP_CODEC_FS_CTRL3_fs_audio_r_dn_srcup_dout_END     (25)
#define SOC_ASP_CODEC_FS_CTRL3_fs_i2s2_rx_l_srcup_din_START     (26)
#define SOC_ASP_CODEC_FS_CTRL3_fs_i2s2_rx_l_srcup_din_END       (28)
#define SOC_ASP_CODEC_FS_CTRL3_fs_i2s2_rx_l_srcup_dout_START    (29)
#define SOC_ASP_CODEC_FS_CTRL3_fs_i2s2_rx_l_srcup_dout_END      (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_FS_CTRL4_UNION
 结构说明  : FS_CTRL4 寄存器结构定义。地址偏移量:0x0128，初值:0x4A444023，宽度:32
 寄存器说明: CODEC内部模块采样率控制寄存器4
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  fs_i2s2_rx_r_srcup_din    : 3;  /* bit[0-2]  : i2s2上行通路右声道SRCUP输入端采样率控制：
                                                                     3'b000：8k；
                                                                     3'b001：16k；
                                                                     3'b010：32k；
                                                                     3'b011：48k；
                                                                     3'b100 : 96k ;
                                                                     3'b101：保留;
                                                                     其它 ： 保留 ； */
        unsigned int  fs_i2s2_rx_r_srcup_dout   : 3;  /* bit[3-5]  : i2s2上行通路右声道SRCUP输出端采样率控制：
                                                                     3'b000：保留；
                                                                     3'b001：保留；
                                                                     3'b010：保留；
                                                                     3'b011：保留；
                                                                     3'b100 : 96k ;
                                                                     3'b101：保留;
                                                                     其它 ： 保留 ； */
        unsigned int  fs_dacl_mixer4_srcup_din  : 2;  /* bit[6-7]  : DACL_MIXER4通路左声道SRCUP输入端采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     2'b10：192k；
                                                                     其它 ： 保留 ； */
        unsigned int  fs_dacl_mixer4_srcup_dout : 2;  /* bit[8-9]  : DACL_MIXER4通路左声道SRCUP输出端采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     2'b10 : 192k ;
                                                                     其它 ： 保留 ； */
        unsigned int  fs_dacr_mixer4_srcup_din  : 2;  /* bit[10-11]: DACR_MIXER4通路右声道SRCUP输入端采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     2'b10：192k；
                                                                     其它 ： 保留 ； */
        unsigned int  fs_dacr_mixer4_srcup_dout : 2;  /* bit[12-13]: DACR_MIXER4通路右声道SRCUP输出端采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     2'b10 : 192k ;
                                                                     其它 ： 保留 ； */
        unsigned int  fs_audio_up_l_srcdn_din   : 2;  /* bit[14-15]: audio上行通路左声道SRCDN输入端采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     2'b10：保留；
                                                                     其它 ： 保留 ； */
        unsigned int  fs_audio_up_l_srcdn_dout  : 2;  /* bit[16-17]: audio上行通路左声道SRCDN输出端采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     2'b10：保留；
                                                                     其它 ： 保留 ； */
        unsigned int  fs_audio_up_r_srcdn_din   : 2;  /* bit[18-19]: audio上行通路右声道SRCDN输入端采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     2'b10：保留；
                                                                     其它 ： 保留 ； */
        unsigned int  fs_audio_up_r_srcdn_dout  : 2;  /* bit[20-21]: audio上行通路右声道SRCDN输出端采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     2'b10：保留；
                                                                     其它 ： 保留 ； */
        unsigned int  fs_voice_up_l_srcdn_din   : 2;  /* bit[22-23]: voice上行通路左声道SRCDN输入端采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     其它 ： 保留 ； */
        unsigned int  fs_voice_up_l_srcdn_dout  : 3;  /* bit[24-26]: voice上行通路左声道SRCDN输出端采样率控制：
                                                                     3'b000：8k；
                                                                     3'b001：16k；
                                                                     3'b010 : 32k ;
                                                                     3'b011 : 48k ;
                                                                     3'b100 : 96k ;
                                                                     其它 ： 保留 ； */
        unsigned int  fs_voice_up_r_srcdn_din   : 2;  /* bit[27-28]: voice上行通路右声道SRCDN输入端采样率控制：
                                                                     2'b00：48k；
                                                                     2'b01：96k；
                                                                     其它 ： 保留 ； */
        unsigned int  fs_voice_up_r_srcdn_dout  : 3;  /* bit[29-31]: voice上行通路右声道SRCDN输出端采样率控制：
                                                                     3'b000：8k；
                                                                     3'b001：16k；
                                                                     3'b010 : 32k ;
                                                                     3'b011 : 48k ;
                                                                     3'b100 : 96k ;
                                                                     其它 ： 保留 ； */
    } reg;
} SOC_ASP_CODEC_FS_CTRL4_UNION;
#endif
#define SOC_ASP_CODEC_FS_CTRL4_fs_i2s2_rx_r_srcup_din_START     (0)
#define SOC_ASP_CODEC_FS_CTRL4_fs_i2s2_rx_r_srcup_din_END       (2)
#define SOC_ASP_CODEC_FS_CTRL4_fs_i2s2_rx_r_srcup_dout_START    (3)
#define SOC_ASP_CODEC_FS_CTRL4_fs_i2s2_rx_r_srcup_dout_END      (5)
#define SOC_ASP_CODEC_FS_CTRL4_fs_dacl_mixer4_srcup_din_START   (6)
#define SOC_ASP_CODEC_FS_CTRL4_fs_dacl_mixer4_srcup_din_END     (7)
#define SOC_ASP_CODEC_FS_CTRL4_fs_dacl_mixer4_srcup_dout_START  (8)
#define SOC_ASP_CODEC_FS_CTRL4_fs_dacl_mixer4_srcup_dout_END    (9)
#define SOC_ASP_CODEC_FS_CTRL4_fs_dacr_mixer4_srcup_din_START   (10)
#define SOC_ASP_CODEC_FS_CTRL4_fs_dacr_mixer4_srcup_din_END     (11)
#define SOC_ASP_CODEC_FS_CTRL4_fs_dacr_mixer4_srcup_dout_START  (12)
#define SOC_ASP_CODEC_FS_CTRL4_fs_dacr_mixer4_srcup_dout_END    (13)
#define SOC_ASP_CODEC_FS_CTRL4_fs_audio_up_l_srcdn_din_START    (14)
#define SOC_ASP_CODEC_FS_CTRL4_fs_audio_up_l_srcdn_din_END      (15)
#define SOC_ASP_CODEC_FS_CTRL4_fs_audio_up_l_srcdn_dout_START   (16)
#define SOC_ASP_CODEC_FS_CTRL4_fs_audio_up_l_srcdn_dout_END     (17)
#define SOC_ASP_CODEC_FS_CTRL4_fs_audio_up_r_srcdn_din_START    (18)
#define SOC_ASP_CODEC_FS_CTRL4_fs_audio_up_r_srcdn_din_END      (19)
#define SOC_ASP_CODEC_FS_CTRL4_fs_audio_up_r_srcdn_dout_START   (20)
#define SOC_ASP_CODEC_FS_CTRL4_fs_audio_up_r_srcdn_dout_END     (21)
#define SOC_ASP_CODEC_FS_CTRL4_fs_voice_up_l_srcdn_din_START    (22)
#define SOC_ASP_CODEC_FS_CTRL4_fs_voice_up_l_srcdn_din_END      (23)
#define SOC_ASP_CODEC_FS_CTRL4_fs_voice_up_l_srcdn_dout_START   (24)
#define SOC_ASP_CODEC_FS_CTRL4_fs_voice_up_l_srcdn_dout_END     (26)
#define SOC_ASP_CODEC_FS_CTRL4_fs_voice_up_r_srcdn_din_START    (27)
#define SOC_ASP_CODEC_FS_CTRL4_fs_voice_up_r_srcdn_din_END      (28)
#define SOC_ASP_CODEC_FS_CTRL4_fs_voice_up_r_srcdn_dout_START   (29)
#define SOC_ASP_CODEC_FS_CTRL4_fs_voice_up_r_srcdn_dout_END     (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_FS_CTRL5_UNION
 结构说明  : FS_CTRL5 寄存器结构定义。地址偏移量:0x012C，初值:0x1BB6D514，宽度:32
 寄存器说明: CODEC内部模块采样率控制寄存器5
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  fs_mic3_srcdn_din       : 3;  /* bit[0-2]  : MIC3上行通路声道SRCDN输入端采样率控制：
                                                                   3'b000：8k；
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100 : 96k ;
                                                                   其它 ： 保留 ； */
        unsigned int  fs_mic3_srcdn_dout      : 3;  /* bit[3-5]  : MIC3上行通路右声道SRCDN输出端采样率控制：
                                                                   3'b000：8k；
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100 : 96k ;
                                                                   其它 ： 保留 ； */
        unsigned int  fs_mic4_srcdn_din       : 3;  /* bit[6-8]  : MIC4上行通路声道SRCDN输入端采样率控制：
                                                                   3'b000：8k；
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100 : 96k ;
                                                                   其它 ： 保留 ； */
        unsigned int  fs_mic4_srcdn_dout      : 3;  /* bit[9-11] : MIC4上行通路右声道SRCDN输出端采样率控制：
                                                                   3'b000：8k；
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100 : 96k ;
                                                                   其它 ： 保留 ； */
        unsigned int  fs_mdm_5g_l_srcdn_din   : 3;  /* bit[12-14]: fs_mdm_5g上行通路左声道SRCDN的输入采样率控制：
                                                                   3'b000：8k;
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100：保留；
                                                                   3'b101：96k；
                                                                   3'b110：保留；
                                                                   3'b111 : 保留 ;
                                                                   其它：48k； */
        unsigned int  fs_mdm_5g_l_srcdn_dout  : 3;  /* bit[15-17]: fs_mdm_5g上行通路左声道SRCDN的输出采样率控制：（44.1k~176.4k仅在
                                                                   slv_mode_441==1'b1时生效）
                                                                   3'b000：8k;
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100：保留；
                                                                   3'b101：96k；
                                                                   3'b110：保留；
                                                                   3'b111 : 保留 ;
                                                                   其它：48k； */
        unsigned int  fs_mdm_5g_r_srcdn_din   : 3;  /* bit[18-20]: fs_mdm_5g上行通路右声道SRCDN的输入采样率控制：（44.1k~176.4k仅在
                                                                  slv_mode_441==1'b1时生效）
                                                                   3'b000：8k;
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100：保留；
                                                                   3'b101：96k；
                                                                   3'b110：保留；
                                                                   3'b111 : 保留 ;
                                                                   其它：48k； */
        unsigned int  fs_mdm_5g_r_srcdn_dout  : 3;  /* bit[21-23]: fs_mdm_5g上行通路右声道SRCDN的输出采样率控制：（44.1k~176.4k仅在
                                                                   slv_mode_441==1'b1时生效）
                                                                   3'b000：8k;
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100：保留；
                                                                   3'b101：96k；
                                                                   3'b110：保留；
                                                                   3'b111 : 保留;
                                                                   其它：48k； */
        unsigned int  fs_i2s2_rx_l_srcdn_din  : 3;  /* bit[24-26]: I2S2上行通路左声道SRCDN输入端采样率控制：
                                                                   3'b000：8k；
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100 : 96k ;
                                                                   3'b101：保留;
                                                                   其它 ： 保留 ； */
        unsigned int  fs_i2s2_rx_l_srcdn_dout : 3;  /* bit[27-29]: I2S2上行通路左声道SRCDN输出端采样率控制：
                                                                   3'b000：8k；
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100 : 96k ;
                                                                   3'b101：保留;
                                                                   其它 ： 保留 ； */
        unsigned int  reserved                : 2;  /* bit[30-31]: reserved */
    } reg;
} SOC_ASP_CODEC_FS_CTRL5_UNION;
#endif
#define SOC_ASP_CODEC_FS_CTRL5_fs_mic3_srcdn_din_START        (0)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mic3_srcdn_din_END          (2)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mic3_srcdn_dout_START       (3)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mic3_srcdn_dout_END         (5)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mic4_srcdn_din_START        (6)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mic4_srcdn_din_END          (8)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mic4_srcdn_dout_START       (9)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mic4_srcdn_dout_END         (11)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mdm_5g_l_srcdn_din_START    (12)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mdm_5g_l_srcdn_din_END      (14)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mdm_5g_l_srcdn_dout_START   (15)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mdm_5g_l_srcdn_dout_END     (17)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mdm_5g_r_srcdn_din_START    (18)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mdm_5g_r_srcdn_din_END      (20)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mdm_5g_r_srcdn_dout_START   (21)
#define SOC_ASP_CODEC_FS_CTRL5_fs_mdm_5g_r_srcdn_dout_END     (23)
#define SOC_ASP_CODEC_FS_CTRL5_fs_i2s2_rx_l_srcdn_din_START   (24)
#define SOC_ASP_CODEC_FS_CTRL5_fs_i2s2_rx_l_srcdn_din_END     (26)
#define SOC_ASP_CODEC_FS_CTRL5_fs_i2s2_rx_l_srcdn_dout_START  (27)
#define SOC_ASP_CODEC_FS_CTRL5_fs_i2s2_rx_l_srcdn_dout_END    (29)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_FS_CTRL6_UNION
 结构说明  : FS_CTRL6 寄存器结构定义。地址偏移量:0x0130，初值:0x0012631B，宽度:32
 寄存器说明: CODEC内部模块采样率控制寄存器6
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  fs_i2s2_rx_r_srcdn_din  : 3;  /* bit[0-2]  : I2S2上行通路右声道SRCDN输入端采样率控制：
                                                                   3'b000：8k；
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100 : 96k ;
                                                                   3'b101：保留；
                                                                   其它 ： 保留 ； */
        unsigned int  fs_i2s2_rx_r_srcdn_dout : 3;  /* bit[3-5]  : I2S2上行通路右声道SRCDN输出端采样率控制：
                                                                   3'b000：8k；
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100 : 96k ;
                                                                   3'b101：保留;
                                                                   其它 ： 保留 ； */
        unsigned int  fs_i2s2_tx_l_srcdn_din  : 2;  /* bit[6-7]  : I2S2下行通路左声道SRCDN输入端采样率控制：
                                                                   2'b00：48k；
                                                                   2'b01：96k；
                                                                   2'b10 : 保留 ;
                                                                   其它 ： 保留 ； */
        unsigned int  fs_i2s2_tx_l_srcdn_dout : 3;  /* bit[8-10] : I2S2下行通路左声道SRCDN输出端采样率控制：
                                                                   3'b000：8k；
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100 : 96k ;
                                                                   3'b101 : 保留 ;
                                                                   其它 ： 保留 ； */
        unsigned int  fs_i2s2_tx_r_srcdn_din  : 2;  /* bit[11-12]: I2S2下行通路右声道SRCDN输入端采样率控制：
                                                                   2'b00：48k；
                                                                   2'b01：96k；
                                                                   2'b10 : 保留 ;
                                                                   其它 ： 保留 ； */
        unsigned int  fs_i2s2_tx_r_srcdn_dout : 3;  /* bit[13-15]: I2S2下行通路右声道SRCDN输出端采样率控制：
                                                                   3'b000：8k；
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100 : 96k ;
                                                                   3'b101 : 保留 ;
                                                                   其它 ： 保留 ； */
        unsigned int  fs_mic5_srcdn_dout      : 3;  /* bit[16-18]: MIC5上行通路右声道SRCDN输出端采样率控制：
                                                                   3'b000：8k；
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100 : 96k ;
                                                                   其它 ： 保留 ； */
        unsigned int  fs_mic6_srcdn_dout      : 3;  /* bit[19-21]: MIC6上行通路右声道SRCDN输出端采样率控制：
                                                                   3'b000：8k；
                                                                   3'b001：16k；
                                                                   3'b010：32k；
                                                                   3'b011：48k；
                                                                   3'b100 : 96k ;
                                                                   其它 ： 保留 ； */
        unsigned int  reserved_0              : 2;  /* bit[22-23]: reserved */
        unsigned int  fs_dacl_mixer4          : 2;  /* bit[24-25]: DACL_MIXER4左声道采样率控制：
                                                                   2'b00：48k；
                                                                   2'b01：96k；
                                                                   2'b10：192k；
                                                                   其它 : 保留 ; */
        unsigned int  fs_dacr_mixer4          : 2;  /* bit[26-27]: DACL_MIXER4右声道采样率控制：
                                                                   2'b00：48k；
                                                                   2'b01：96k；
                                                                   2'b10：192k；
                                                                   其它 : 保留 ; */
        unsigned int  reserved_1              : 4;  /* bit[28-31]: reserved */
    } reg;
} SOC_ASP_CODEC_FS_CTRL6_UNION;
#endif
#define SOC_ASP_CODEC_FS_CTRL6_fs_i2s2_rx_r_srcdn_din_START   (0)
#define SOC_ASP_CODEC_FS_CTRL6_fs_i2s2_rx_r_srcdn_din_END     (2)
#define SOC_ASP_CODEC_FS_CTRL6_fs_i2s2_rx_r_srcdn_dout_START  (3)
#define SOC_ASP_CODEC_FS_CTRL6_fs_i2s2_rx_r_srcdn_dout_END    (5)
#define SOC_ASP_CODEC_FS_CTRL6_fs_i2s2_tx_l_srcdn_din_START   (6)
#define SOC_ASP_CODEC_FS_CTRL6_fs_i2s2_tx_l_srcdn_din_END     (7)
#define SOC_ASP_CODEC_FS_CTRL6_fs_i2s2_tx_l_srcdn_dout_START  (8)
#define SOC_ASP_CODEC_FS_CTRL6_fs_i2s2_tx_l_srcdn_dout_END    (10)
#define SOC_ASP_CODEC_FS_CTRL6_fs_i2s2_tx_r_srcdn_din_START   (11)
#define SOC_ASP_CODEC_FS_CTRL6_fs_i2s2_tx_r_srcdn_din_END     (12)
#define SOC_ASP_CODEC_FS_CTRL6_fs_i2s2_tx_r_srcdn_dout_START  (13)
#define SOC_ASP_CODEC_FS_CTRL6_fs_i2s2_tx_r_srcdn_dout_END    (15)
#define SOC_ASP_CODEC_FS_CTRL6_fs_mic5_srcdn_dout_START       (16)
#define SOC_ASP_CODEC_FS_CTRL6_fs_mic5_srcdn_dout_END         (18)
#define SOC_ASP_CODEC_FS_CTRL6_fs_mic6_srcdn_dout_START       (19)
#define SOC_ASP_CODEC_FS_CTRL6_fs_mic6_srcdn_dout_END         (21)
#define SOC_ASP_CODEC_FS_CTRL6_fs_dacl_mixer4_START           (24)
#define SOC_ASP_CODEC_FS_CTRL6_fs_dacl_mixer4_END             (25)
#define SOC_ASP_CODEC_FS_CTRL6_fs_dacr_mixer4_START           (26)
#define SOC_ASP_CODEC_FS_CTRL6_fs_dacr_mixer4_END             (27)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_FS_CTRL7_UNION
 结构说明  : FS_CTRL7 寄存器结构定义。地址偏移量:0x0134，初值:0x0000007B，宽度:32
 寄存器说明: CODEC内部模块采样率控制寄存器7
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  fs_i2s2_tx_mixer2 : 3;  /* bit[0-2] : I2S2上行通路右声道SRCDN输入端采样率控制：
                                                            3'b000：保留；
                                                            3'b001：保留；
                                                            3'b010：保留；
                                                            3'b011：48k；
                                                            3'b100 : 96k ;
                                                            3'b101 : 保留;
                                                            其它 ： 保留 ； */
        unsigned int  fs_s1_tdm         : 2;  /* bit[3-4] : TDM采样率选择:
                                                            2'b00：保留；
                                                            2'b01：保留；
                                                            2'b10：保留；
                                                            2'b11：48k； */
        unsigned int  fs_s4_tdm         : 2;  /* bit[5-6] : TDM采样率选择:
                                                            2'b00：8k；
                                                            2'b01：16k；
                                                            2'b10：32k；
                                                            2'b11：48k； */
        unsigned int  reserved          : 25; /* bit[7-31]: reserved */
    } reg;
} SOC_ASP_CODEC_FS_CTRL7_UNION;
#endif
#define SOC_ASP_CODEC_FS_CTRL7_fs_i2s2_tx_mixer2_START  (0)
#define SOC_ASP_CODEC_FS_CTRL7_fs_i2s2_tx_mixer2_END    (2)
#define SOC_ASP_CODEC_FS_CTRL7_fs_s1_tdm_START          (3)
#define SOC_ASP_CODEC_FS_CTRL7_fs_s1_tdm_END            (4)
#define SOC_ASP_CODEC_FS_CTRL7_fs_s4_tdm_START          (5)
#define SOC_ASP_CODEC_FS_CTRL7_fs_s4_tdm_END            (6)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_DIN_MUX0_UNION
 结构说明  : CODEC_DIN_MUX0 寄存器结构定义。地址偏移量:0x0138，初值:0x04208F0A，宽度:32
 寄存器说明: CODEC内部模块输入数据选择寄存器0
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  i2s1_tx_l_sel          : 2;  /* bit[0-1]  : I2S1的左声道数据源选择：
                                                                  2'b00 : 来源于AU_L；

                                                                  2'b10 : 来源于mixer4_src_l;
                                                                  其它 ： 保留； */
        unsigned int  i2s1_tx_r_sel          : 2;  /* bit[2-3]  : I2S1的右声道数据源选择：
                                                                  2'b00 : 来源于AU_R；

                                                                  2'b10 : 来源于mixer4_src_r;
                                                                  其它 ： 保留； */
        unsigned int  i2s2_tx_r_sel          : 1;  /* bit[4]    : I2S2的右声道数据源选择：
                                                                  1'b0 : 来源于i2s2_tx_l的btmatch输出信号；
                                                                  1'b1 : 来源于i2s2_tx_r的btmatch输出信号； */
        unsigned int  sidetone_pga_din_sel   : 3;  /* bit[5-7]  : sidetone_pga_din的数据源选择：
                                                                  3'b000 : mic1_adc
                                                                  3'b001 : mic2_adc
                                                                  3'b010 : mic3_adc
                                                                  3'b011 : mic4_adc 3'b100 : mic5_adc
                                                                  3'b101 : mic6_adc
                                                                  3'b110 : mic7_adc
                                                                  3'b111 : mic8_adc */
        unsigned int  adc1_din_sel           : 1;  /* bit[8]    : ADC1的数据源选择：
                                                                  1'b0 :从sif传过来的数据；
                                                                  1'b1：从adc1_cic传过来的数据； */
        unsigned int  adc2_din_sel           : 1;  /* bit[9]    : ADC2的数据源选择：
                                                                  1'b0 :从sif传过来的数据；
                                                                  1'b1：从adc2_cic传过来的数据； */
        unsigned int  adc3_din_sel           : 1;  /* bit[10]   : ADC3的数据源选择：
                                                                  1'b0 :从sif传过来的数据；
                                                                  1'b1：从adc3_cic传过来的数据； */
        unsigned int  adc4_din_sel           : 1;  /* bit[11]   : ADC4的数据源选择：
                                                                  1'b0 :从sif传过来的数据；
                                                                  1'b1：从adc4_cic传过来的数据； */
        unsigned int  au_up_l_din_sel        : 3;  /* bit[12-14]: au_up_l的数据源选择：
                                                                  3'b000 : mic1_adc;
                                                                  3'b001：mic2_adc;
                                                                  3'b010：i2s2_rx_l;
                                                                  3'b011：mixer4_l;
                                                                  3'b100：audio_dlink_l；
                                                                  其它： 保留 */
        unsigned int  au_up_r_din_sel        : 3;  /* bit[15-17]: au_up_r的数据源选择：
                                                                  3'b000 : mic1_adc;
                                                                  3'b001：mic2_adc;
                                                                  3'b010：i2s2_rx_r;
                                                                  3'b011：mixer4_r;
                                                                  3'b100：audio_dlink_r；
                                                                  其它： 保留 */
        unsigned int  vo_up_l_din_sel        : 3;  /* bit[18-20]: vo_up_l的数据源选择：
                                                                  3'b000 : mic1_adc;
                                                                  3'b001：mic2_adc;
                                                                  3'b010：i2s2_rx_l;
                                                                  3'b011：mixer4_l;
                                                                  3'b100：codec3_dlink_l；
                                                                  其它： 保留 */
        unsigned int  vo_up_r_din_sel        : 3;  /* bit[21-23]: vo_up_r的数据源选择：
                                                                  3'b000 : mic1_adc;
                                                                  3'b001：mic2_adc;
                                                                  3'b010：i2s2_rx_r;
                                                                  3'b011：mixer4_r;
                                                                  3'b100：codec3_dlink_r；
                                                                  其它： 保留 */
        unsigned int  mic3_din_sel           : 2;  /* bit[24-25]: mic3的数据源选择：
                                                                  2'b00 : mic3_adc;
                                                                  2'b01 : mic4_adc;
                                                                  2'b10 : i2s3_rx_l ;
                                                                  其它 ： 保留； */
        unsigned int  mic4_din_sel           : 2;  /* bit[26-27]: mic4的数据源选择：
                                                                  2'b00 : mic3_adc;
                                                                  2'b01 : mic4_adc;
                                                                  2'b10 : i2s3_rx_r ;
                                                                  其它 ： 保留； */
        unsigned int  sif_dacr_srcup_din_sel : 1;  /* bit[28]   : dacl_srcup的输入数据源选择：
                                                                  0：数据来自于codec3_dn_r_pga_output[25:0]；
                                                                  1：数据来自audio_dn_r_pga_dout[25:0]; */
        unsigned int  sif_dacl_srcup_din_sel : 1;  /* bit[29]   : dacl_srcup的输入数据源选择：
                                                                  0：数据来自于codec3_dn_l_pga_output[25:0]；
                                                                  1：数据来自audio_dn_l_pga_dout[25:0]; */
        unsigned int  reserved               : 2;  /* bit[30-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_DIN_MUX0_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_i2s1_tx_l_sel_START           (0)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_i2s1_tx_l_sel_END             (1)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_i2s1_tx_r_sel_START           (2)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_i2s1_tx_r_sel_END             (3)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_i2s2_tx_r_sel_START           (4)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_i2s2_tx_r_sel_END             (4)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_sidetone_pga_din_sel_START    (5)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_sidetone_pga_din_sel_END      (7)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_adc1_din_sel_START            (8)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_adc1_din_sel_END              (8)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_adc2_din_sel_START            (9)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_adc2_din_sel_END              (9)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_adc3_din_sel_START            (10)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_adc3_din_sel_END              (10)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_adc4_din_sel_START            (11)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_adc4_din_sel_END              (11)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_au_up_l_din_sel_START         (12)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_au_up_l_din_sel_END           (14)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_au_up_r_din_sel_START         (15)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_au_up_r_din_sel_END           (17)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_vo_up_l_din_sel_START         (18)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_vo_up_l_din_sel_END           (20)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_vo_up_r_din_sel_START         (21)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_vo_up_r_din_sel_END           (23)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_mic3_din_sel_START            (24)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_mic3_din_sel_END              (25)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_mic4_din_sel_START            (26)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_mic4_din_sel_END              (27)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_sif_dacr_srcup_din_sel_START  (28)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_sif_dacr_srcup_din_sel_END    (28)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_sif_dacl_srcup_din_sel_START  (29)
#define SOC_ASP_CODEC_CODEC_DIN_MUX0_sif_dacl_srcup_din_sel_END    (29)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_DIN_MUX1_UNION
 结构说明  : CODEC_DIN_MUX1 寄存器结构定义。地址偏移量:0x013C，初值:0x0014C000，宽度:32
 寄存器说明: CODEC内部模块输入数据选择寄存器1
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0               : 2;  /* bit[0-1]  : reserved */
        unsigned int  mdm_5g_up_l_din_sel      : 4;  /* bit[2-5]  : MDM_5G_UP_L的数据源选择：
                                                                    4'b0000 : mixer4_l ;
                                                                    4'b0001 : codec3_dlink_l ;
                                                                    4'b0011 : ultr_dlink_l ;
                                                                    4'b0100 : i2s3_rx_l ;
                                                                    其它 ： 保留； */
        unsigned int  mdm_5g_up_r_din_sel      : 4;  /* bit[6-9]  : MDM_5G_UP_R的数据源选择：
                                                                    4'b0000 : mixer4_r ;
                                                                    4'b0001 : codec3_dlink_r ;
                                                                    4'b0011 : ultr_dlink_r ;
                                                                    4'b0100 : i2s3_rx_r ;
                                                                    其它 ： 保留； */
        unsigned int  bm_i2s1_tdm_tx_l_din_sel : 1;  /* bit[10]   : bm_i2s1_tdm_tx_l_din的数据源选择：
                                                                    1'b0 : audio_l_dn_srcup_dout;
                                                                    1'b1 : dacl_mixer4_srcup_dout; */
        unsigned int  bm_i2s1_tdm_tx_r_din_sel : 1;  /* bit[11]   : bm_i2s1_tdm_tx_r_din的数据源选择：
                                                                    1'b0 : audio_r_dn_srcup_dout;
                                                                    1'b1 : dacr_mixer4_srcup_dout; */
        unsigned int  reserved_1               : 2;  /* bit[12-13]: reserved */
        unsigned int  mdm_5g_l_afifo_din_sel   : 1;  /* bit[14]   : mdm_5g_l_afifo_din的数据源选择：
                                                                    1'b0：从bm_mdm_5g_l_up来；
                                                                    1'b1：从i2s4_rx_l来； */
        unsigned int  mdm_5g_r_afifo_din_sel   : 1;  /* bit[15]   : mdm_5g_r_afifo_din的数据源选择：
                                                                    1'b0：从bm_mdm_5g_r_up来；
                                                                    1'b1：从i2s4_rx_r来； */
        unsigned int  reserved_2               : 1;  /* bit[16]   : reserved */
        unsigned int  bt_tx_srcdn_din_sel      : 1;  /* bit[17]   : bt_srcdn_din_sel：
                                                                    1'b0：来自mixer2的混音数据；
                                                                    1'b1：来自mixer4_l; */
        unsigned int  spai2_adc_din_sel        : 1;  /* bit[18]   : spai2_adc_din_sel
                                                                    1'b0：spa_codec_adcv;
                                                                    1'b1：spa_codec_adci; */
        unsigned int  spav2_adc_din_sel        : 1;  /* bit[19]   : spav2_adc_din_sel
                                                                    1'b0：spa_codec_adcv;
                                                                    1'b1：spa_codec_adci; */
        unsigned int  spai1_adc_din_sel        : 1;  /* bit[20]   : spai1_adc_din_sel
                                                                    1'b0：spa_codec_adcv;
                                                                    1'b1：spa_codec_adci; */
        unsigned int  spav1_adc_din_sel        : 1;  /* bit[21]   : spav1_adc_din_sel
                                                                    1'b0：spa_codec_adcv;
                                                                    1'b1：spa_codec_adci; */
        unsigned int  reserved_3               : 10; /* bit[22-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_DIN_MUX1_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_mdm_5g_up_l_din_sel_START       (2)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_mdm_5g_up_l_din_sel_END         (5)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_mdm_5g_up_r_din_sel_START       (6)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_mdm_5g_up_r_din_sel_END         (9)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_bm_i2s1_tdm_tx_l_din_sel_START  (10)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_bm_i2s1_tdm_tx_l_din_sel_END    (10)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_bm_i2s1_tdm_tx_r_din_sel_START  (11)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_bm_i2s1_tdm_tx_r_din_sel_END    (11)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_mdm_5g_l_afifo_din_sel_START    (14)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_mdm_5g_l_afifo_din_sel_END      (14)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_mdm_5g_r_afifo_din_sel_START    (15)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_mdm_5g_r_afifo_din_sel_END      (15)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_bt_tx_srcdn_din_sel_START       (17)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_bt_tx_srcdn_din_sel_END         (17)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_spai2_adc_din_sel_START         (18)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_spai2_adc_din_sel_END           (18)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_spav2_adc_din_sel_START         (19)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_spav2_adc_din_sel_END           (19)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_spai1_adc_din_sel_START         (20)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_spai1_adc_din_sel_END           (20)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_spav1_adc_din_sel_START         (21)
#define SOC_ASP_CODEC_CODEC_DIN_MUX1_spav1_adc_din_sel_END           (21)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_ADC1_DC_OFFSET_UNION
 结构说明  : CODEC_ADC1_DC_OFFSET 寄存器结构定义。地址偏移量:0x0140，初值:0x00000000，宽度:32
 寄存器说明: ADC1通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc1_dc_offset : 24; /* bit[0-23] : adc1_hpf dc_offset配置 */
        unsigned int  reserved       : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_ADC1_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_ADC1_DC_OFFSET_adc1_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_ADC1_DC_OFFSET_adc1_dc_offset_END    (23)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_ADC2_DC_OFFSET_UNION
 结构说明  : CODEC_ADC2_DC_OFFSET 寄存器结构定义。地址偏移量:0x0144，初值:0x00000000，宽度:32
 寄存器说明: ADC2通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc2_dc_offset : 24; /* bit[0-23] : adc2_hpf dc_offset配置 */
        unsigned int  reserved       : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_ADC2_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_ADC2_DC_OFFSET_adc2_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_ADC2_DC_OFFSET_adc2_dc_offset_END    (23)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_ADC3_DC_OFFSET_UNION
 结构说明  : CODEC_ADC3_DC_OFFSET 寄存器结构定义。地址偏移量:0x0148，初值:0x00000000，宽度:32
 寄存器说明: ADC3通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc3_dc_offset : 24; /* bit[0-23] : adc3_hpf dc_offset配置 */
        unsigned int  reserved       : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_ADC3_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_ADC3_DC_OFFSET_adc3_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_ADC3_DC_OFFSET_adc3_dc_offset_END    (23)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_ADC4_DC_OFFSET_UNION
 结构说明  : CODEC_ADC4_DC_OFFSET 寄存器结构定义。地址偏移量:0x0150，初值:0x00000000，宽度:32
 寄存器说明: ADC4通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc4_dc_offset : 24; /* bit[0-23] : adc4_hpf dc_offset配置 */
        unsigned int  reserved       : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_ADC4_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_ADC4_DC_OFFSET_adc4_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_ADC4_DC_OFFSET_adc4_dc_offset_END    (23)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_ADC5_DC_OFFSET_UNION
 结构说明  : CODEC_ADC5_DC_OFFSET 寄存器结构定义。地址偏移量:0x0154，初值:0x00000000，宽度:32
 寄存器说明: ADC5通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc5_dc_offset : 24; /* bit[0-23] : adc5_hpf dc_offset配置 */
        unsigned int  reserved       : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_ADC5_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_ADC5_DC_OFFSET_adc5_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_ADC5_DC_OFFSET_adc5_dc_offset_END    (23)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MEM_CTRL_S_UNION
 结构说明  : MEM_CTRL_S 寄存器结构定义。地址偏移量:0x0158，初值:0x000002C0，宽度:32
 寄存器说明: 单口memory的控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  mem_ctrl_s : 26; /* bit[0-25] : 单口memory的控制信号 */
        unsigned int  reserved   : 6;  /* bit[26-31]: reserved */
    } reg;
} SOC_ASP_CODEC_MEM_CTRL_S_UNION;
#endif
#define SOC_ASP_CODEC_MEM_CTRL_S_mem_ctrl_s_START  (0)
#define SOC_ASP_CODEC_MEM_CTRL_S_mem_ctrl_s_END    (25)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MEM_CTRL_1W2R_UNION
 结构说明  : MEM_CTRL_1W2R 寄存器结构定义。地址偏移量:0x015C，初值:0x00000A40，宽度:32
 寄存器说明: 双口memory的控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  mem_ctrl_1w2r : 16; /* bit[0-15] : 双口memory的控制信号 */
        unsigned int  reserved      : 16; /* bit[16-31]: reserved */
    } reg;
} SOC_ASP_CODEC_MEM_CTRL_1W2R_UNION;
#endif
#define SOC_ASP_CODEC_MEM_CTRL_1W2R_mem_ctrl_1w2r_START  (0)
#define SOC_ASP_CODEC_MEM_CTRL_1W2R_mem_ctrl_1w2r_END    (15)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ADC_CTRL_HPF_UNION
 结构说明  : ADC_CTRL_HPF 寄存器结构定义。地址偏移量:0x0160，初值:0x00000000，宽度:32
 寄存器说明: HPF滤波器的bypass控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc1_hpf_bypass_en       : 1;  /* bit[0]    : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  adc2_hpf_bypass_en       : 1;  /* bit[1]    : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  adc3_hpf_bypass_en       : 1;  /* bit[2]    : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  adc4_hpf_bypass_en       : 1;  /* bit[3]    : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  adc5_hpf_bypass_en       : 1;  /* bit[4]    : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  adc6_hpf_bypass_en       : 1;  /* bit[5]    : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  adc7_hpf_bypass_en       : 1;  /* bit[6]    : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  adc8_hpf_bypass_en       : 1;  /* bit[7]    : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  dacr_up16_hbf3_bypass_en : 1;  /* bit[8]    : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  dacr_up16_hbf2_bypass_en : 1;  /* bit[9]    : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  dacr_up16_hbf1_bypass_en : 1;  /* bit[10]   : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  dacr_up16_hbf0_bypass_en : 1;  /* bit[11]   : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  dacl_up16_hbf3_bypass_en : 1;  /* bit[12]   : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  dacl_up16_hbf2_bypass_en : 1;  /* bit[13]   : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  dacl_up16_hbf1_bypass_en : 1;  /* bit[14]   : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  dacl_up16_hbf0_bypass_en : 1;  /* bit[15]   : hpf的bypass控制寄存器
                                                                    1'b0：不bypass；
                                                                    1'b1：bypass; */
        unsigned int  reserved                 : 16; /* bit[16-31]: reserved */
    } reg;
} SOC_ASP_CODEC_ADC_CTRL_HPF_UNION;
#endif
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc1_hpf_bypass_en_START        (0)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc1_hpf_bypass_en_END          (0)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc2_hpf_bypass_en_START        (1)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc2_hpf_bypass_en_END          (1)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc3_hpf_bypass_en_START        (2)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc3_hpf_bypass_en_END          (2)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc4_hpf_bypass_en_START        (3)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc4_hpf_bypass_en_END          (3)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc5_hpf_bypass_en_START        (4)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc5_hpf_bypass_en_END          (4)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc6_hpf_bypass_en_START        (5)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc6_hpf_bypass_en_END          (5)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc7_hpf_bypass_en_START        (6)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc7_hpf_bypass_en_END          (6)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc8_hpf_bypass_en_START        (7)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_adc8_hpf_bypass_en_END          (7)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacr_up16_hbf3_bypass_en_START  (8)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacr_up16_hbf3_bypass_en_END    (8)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacr_up16_hbf2_bypass_en_START  (9)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacr_up16_hbf2_bypass_en_END    (9)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacr_up16_hbf1_bypass_en_START  (10)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacr_up16_hbf1_bypass_en_END    (10)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacr_up16_hbf0_bypass_en_START  (11)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacr_up16_hbf0_bypass_en_END    (11)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacl_up16_hbf3_bypass_en_START  (12)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacl_up16_hbf3_bypass_en_END    (12)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacl_up16_hbf2_bypass_en_START  (13)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacl_up16_hbf2_bypass_en_END    (13)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacl_up16_hbf1_bypass_en_START  (14)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacl_up16_hbf1_bypass_en_END    (14)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacl_up16_hbf0_bypass_en_START  (15)
#define SOC_ASP_CODEC_ADC_CTRL_HPF_dacl_up16_hbf0_bypass_en_END    (15)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_UNION
 结构说明  : PGA_GAINOFFSET_CTRL4 寄存器结构定义。地址偏移量:0x0164，初值:0x00000000，宽度:32
 寄存器说明: PGA GAINOFFSET配置寄存器4
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  mic5_up_pga_gainoffset : 8;  /* bit[0-7]  : 增益偏置，gainoffset有效配置范围是0~255
                                                                  gain_offset的调整粒度与fadeInTime相关，映射关系为粒度=
                                                                  max(2^(fadeInTime -12),1)，
                                                                  以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  mic6_up_pga_gainoffset : 8;  /* bit[8-15] : 增益偏置，gainoffset有效配置范围是0~255
                                                                  gain_offset的调整粒度与fadeInTime相关，映射关系为粒度=
                                                                  max(2^(fadeInTime -12),1)，
                                                                  以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  mic7_up_pga_gainoffset : 8;  /* bit[16-23]: 增益偏置，gainoffset有效配置范围是0~255
                                                                  gain_offset的调整粒度与fadeInTime相关，映射关系为粒度=
                                                                  max(2^(fadeInTime -12),1)，
                                                                  以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  mic8_up_pga_gainoffset : 8;  /* bit[24-31]: 增益偏置，gainoffset有效配置范围是0~255
                                                                  gain_offset的调整粒度与fadeInTime相关，映射关系为粒度=
                                                                  max(2^(fadeInTime -12),1)，
                                                                  以fadeInTime=16为例，gainOffset可取0,16,32……; */
    } reg;
} SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_UNION;
#endif
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_mic5_up_pga_gainoffset_START  (0)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_mic5_up_pga_gainoffset_END    (7)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_mic6_up_pga_gainoffset_START  (8)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_mic6_up_pga_gainoffset_END    (15)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_mic7_up_pga_gainoffset_START  (16)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_mic7_up_pga_gainoffset_END    (23)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_mic8_up_pga_gainoffset_START  (24)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL4_mic8_up_pga_gainoffset_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_UNION
 结构说明  : MIC8_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x0168，初值:0x00000050，宽度:32
 寄存器说明: MIC8上行PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  mic8_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  mic8_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  mic8_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  mic8_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小信号放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  mic8_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  mic8_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mic8_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mic8_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  mic8_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                  例如：60dB配置成0x3C；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_MIC8_UP_PGA_CTRL_mic8_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_UNION
 结构说明  : MIC7_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x016C，初值:0x00000050，宽度:32
 寄存器说明: MIC7上行PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  mic7_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  mic7_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  mic7_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  mic7_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小信号放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  mic7_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  mic7_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mic7_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mic7_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  mic7_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                  例如：60dB配置成0x3C；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_MIC7_UP_PGA_CTRL_mic7_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_UNION
 结构说明  : MIC6_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x0170，初值:0x00000050，宽度:32
 寄存器说明: MIC6上行PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  mic6_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  mic6_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  mic6_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  mic6_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小信号放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  mic6_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  mic6_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mic6_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mic6_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  mic6_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                  例如：60dB配置成0x3C；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_MIC6_UP_PGA_CTRL_mic6_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_UNION
 结构说明  : MIC5_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x0174，初值:0x00000050，宽度:32
 寄存器说明: MIC5上行PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  mic5_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  mic5_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  mic5_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  mic5_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小信号放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  mic5_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  mic5_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mic5_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mic5_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  mic5_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                  例如：60dB配置成0x3C；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_MIC5_UP_PGA_CTRL_mic5_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_UNION
 结构说明  : ADC8_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x0178，初值:0x00000050，宽度:32
 寄存器说明: ADC6上行通路PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  adc8_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  adc8_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  adc8_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  adc8_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小信号放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  adc8_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  adc8_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc8_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc8_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  adc8_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~3dB（bit7为符号位）
                                                                  例如：3dB配置成0x3；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_ADC8_UP_PGA_CTRL_adc8_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_UNION
 结构说明  : ADC7_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x017C，初值:0x00000050，宽度:32
 寄存器说明: ADC7上行通路PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  adc7_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  adc7_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  adc7_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  adc7_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小信号放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  adc7_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  adc7_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc7_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc7_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  adc7_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~3dB（bit7为符号位）
                                                                  例如：3dB配置成0x3；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_ADC7_UP_PGA_CTRL_adc7_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_UNION
 结构说明  : ADC6_UP_PGA_CTRL 寄存器结构定义。地址偏移量:0x0180，初值:0x00000050，宽度:32
 寄存器说明: ADC6上行通路PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved               : 1;  /* bit[0]    : reserved */
        unsigned int  adc6_up_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                  1'b0: 选择对数淡出淡出
                                                                  1'b1: 选择线性淡入淡出 */
        unsigned int  adc6_up_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  adc6_up_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中2bit的id选择四种场景的阈值
                                                                  0：选择pga_thre_ctrl[23:0]
                                                                  1：选择pga_thre_ctrl[47:24]
                                                                  2：选择pga_thre_ctrl[71:48]
                                                                  3：选择pga_thre_ctrl[95:72] */
        unsigned int  adc6_up_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小信号放过去，低表示低于阈值的信号钳制到0
                                                                  1：不对噪音信号做任何处理
                                                                  0：将低于阈值的噪音信号过滤成0 */
        unsigned int  adc6_up_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                  1'b0: 不bypass
                                                                  1'b1: bypass */
        unsigned int  adc6_up_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc6_up_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  adc6_up_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                  [7]：淡入淡出功能使能
                                                                  1：淡入淡出功能使能
                                                                  0：淡入淡出功能不使能
                                                                  [6]: 抗削波功能使能
                                                                  1：抗削波功能使能
                                                                  0：抗削波功能不使能
                                                                  [5]: 小信号功能使能
                                                                  1：小信号功能使能
                                                                  0：小信号功能不使能 */
        unsigned int  adc6_up_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~3dB（bit7为符号位）
                                                                  例如：3dB配置成0x3；
                                                                   0dB配置为0x00；
                                                                   -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_bypass_START      (10)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_bypass_END        (10)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_cfg_START         (21)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_cfg_END           (23)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_gain_START        (24)
#define SOC_ASP_CODEC_ADC6_UP_PGA_CTRL_adc6_up_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_UNION
 结构说明  : MIC56_UP_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x0184，初值:0x7A007A00，宽度:32
 寄存器说明: MIC56上行通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0             : 5;  /* bit[0-4]  : reserved */
        unsigned int  mic5_up_fifo_aempty_th : 5;  /* bit[5-9]  : MIC5上行通路通路右声道AFIFO将空阈值。 */
        unsigned int  mic5_up_fifo_afull_th  : 5;  /* bit[10-14]: MIC5上行通路通路右声道AFIFO将满阈值。 */
        unsigned int  mic5_up_fifo_clr       : 1;  /* bit[15]   : MIC5上行通路通路右声道AFIFO Clear Signal, high active.
                                                                  0：不清空FIFO数据
                                                                  1：清空FIFO数据 */
        unsigned int  reserved_1             : 5;  /* bit[16-20]: reserved */
        unsigned int  mic6_up_fifo_aempty_th : 5;  /* bit[21-25]: MIC6上行通路通路左声道AFIFO将空阈值。 */
        unsigned int  mic6_up_fifo_afull_th  : 5;  /* bit[26-30]: MIC6上行通路通路左声道AFIFO将满阈值。 */
        unsigned int  mic6_up_fifo_clr       : 1;  /* bit[31]   : MIC6上行通路通路左声道AFIFO Clear Signal, high active.
                                                                  0：不清空FIFO数据
                                                                  1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_mic5_up_fifo_aempty_th_START  (5)
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_mic5_up_fifo_aempty_th_END    (9)
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_mic5_up_fifo_afull_th_START   (10)
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_mic5_up_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_mic5_up_fifo_clr_START        (15)
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_mic5_up_fifo_clr_END          (15)
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_mic6_up_fifo_aempty_th_START  (21)
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_mic6_up_fifo_aempty_th_END    (25)
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_mic6_up_fifo_afull_th_START   (26)
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_mic6_up_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_mic6_up_fifo_clr_START        (31)
#define SOC_ASP_CODEC_MIC56_UP_AFIFO_CTRL_mic6_up_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_UNION
 结构说明  : MIC78_UP_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x0188，初值:0x7A007A00，宽度:32
 寄存器说明: MIC78上行通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0             : 5;  /* bit[0-4]  : reserved */
        unsigned int  mic7_up_fifo_aempty_th : 5;  /* bit[5-9]  : MIC7上行通路通路右声道AFIFO将空阈值。 */
        unsigned int  mic7_up_fifo_afull_th  : 5;  /* bit[10-14]: MIC7上行通路通路右声道AFIFO将满阈值。 */
        unsigned int  mic7_up_fifo_clr       : 1;  /* bit[15]   : MIC7上行通路通路右声道AFIFO Clear Signal, high active.
                                                                  0：不清空FIFO数据
                                                                  1：清空FIFO数据 */
        unsigned int  reserved_1             : 5;  /* bit[16-20]: reserved */
        unsigned int  mic8_up_fifo_aempty_th : 5;  /* bit[21-25]: MIC8上行通路通路左声道AFIFO将空阈值。 */
        unsigned int  mic8_up_fifo_afull_th  : 5;  /* bit[26-30]: MIC8上行通路通路左声道AFIFO将满阈值。 */
        unsigned int  mic8_up_fifo_clr       : 1;  /* bit[31]   : MIC8上行通路通路左声道AFIFO Clear Signal, high active.
                                                                  0：不清空FIFO数据
                                                                  1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_mic7_up_fifo_aempty_th_START  (5)
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_mic7_up_fifo_aempty_th_END    (9)
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_mic7_up_fifo_afull_th_START   (10)
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_mic7_up_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_mic7_up_fifo_clr_START        (15)
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_mic7_up_fifo_clr_END          (15)
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_mic8_up_fifo_aempty_th_START  (21)
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_mic8_up_fifo_aempty_th_END    (25)
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_mic8_up_fifo_afull_th_START   (26)
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_mic8_up_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_mic8_up_fifo_clr_START        (31)
#define SOC_ASP_CODEC_MIC78_UP_AFIFO_CTRL_mic8_up_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_FS_CTRL8_UNION
 结构说明  : FS_CTRL8 寄存器结构定义。地址偏移量:0x018C，初值:0x00492000，宽度:32
 寄存器说明: CODEC内部模块采样率控制寄存器8
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0       : 12; /* bit[0-11] : reserved */
        unsigned int  fs_mic5_up_afifo : 3;  /* bit[12-14]: mic5上行通路输入端采样率控制：
                                                            3'b000：8k；
                                                            3'b001：16k；
                                                            3'b010：32k；
                                                            3'b011：48k；
                                                            3'b100 : 96k ;
                                                            其它 ： 保留 ； */
        unsigned int  fs_mic6_up_afifo : 3;  /* bit[15-17]: mic6上行通路输入端采样率控制：
                                                            3'b000：8k；
                                                            3'b001：16k；
                                                            3'b010：32k；
                                                            3'b011：48k；
                                                            3'b100 : 96k ;
                                                            其它 ： 保留 ； */
        unsigned int  fs_mic7_up_afifo : 3;  /* bit[18-20]: mic7上行通路输入端采样率控制：
                                                            3'b000：8k；
                                                            3'b001：16k；
                                                            3'b010：32k；
                                                            3'b011：48k；
                                                            3'b100 : 96k ;
                                                            其它 ： 保留 ； */
        unsigned int  fs_mic8_up_afifo : 3;  /* bit[21-23]: mic8上行通路输入端采样率控制：
                                                            3'b000：8k；
                                                            3'b001：16k；
                                                            3'b010：32k；
                                                            3'b011：48k；
                                                            3'b100 : 96k ;
                                                            其它 ： 保留 ； */
        unsigned int  reserved_1       : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_FS_CTRL8_UNION;
#endif
#define SOC_ASP_CODEC_FS_CTRL8_fs_mic5_up_afifo_START  (12)
#define SOC_ASP_CODEC_FS_CTRL8_fs_mic5_up_afifo_END    (14)
#define SOC_ASP_CODEC_FS_CTRL8_fs_mic6_up_afifo_START  (15)
#define SOC_ASP_CODEC_FS_CTRL8_fs_mic6_up_afifo_END    (17)
#define SOC_ASP_CODEC_FS_CTRL8_fs_mic7_up_afifo_START  (18)
#define SOC_ASP_CODEC_FS_CTRL8_fs_mic7_up_afifo_END    (20)
#define SOC_ASP_CODEC_FS_CTRL8_fs_mic8_up_afifo_START  (21)
#define SOC_ASP_CODEC_FS_CTRL8_fs_mic8_up_afifo_END    (23)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_FS_CTRL9_UNION
 结构说明  : FS_CTRL9 寄存器结构定义。地址偏移量:0x0190，初值:0x00B52492，宽度:32
 寄存器说明: CODEC内部模块采样率控制寄存器9
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  fs_mic5_up_pga     : 3;  /* bit[0-2]  : MIC5上行通路声道PGA输入端采样率控制：
                                                              3'b000：8k；
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k；
                                                              3'b100 : 96k ;
                                                              其它 ： 保留 ； */
        unsigned int  fs_mic6_up_pga     : 3;  /* bit[3-5]  : MIC6上行通路声道PGA输入端采样率控制：
                                                              3'b000：8k；
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k；
                                                              3'b100 : 96k ;
                                                              其它 ： 保留 ； */
        unsigned int  fs_mic7_up_pga     : 3;  /* bit[6-8]  : MIC7上行通路声道PGA输入端采样率控制：
                                                              3'b000：8k；
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k；
                                                              3'b100 : 96k ;
                                                              其它 ： 保留 ； */
        unsigned int  fs_mic8_up_pga     : 3;  /* bit[9-11] : MIC8上行通路声道PGA输入端采样率控制：
                                                              3'b000：8k；
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k；
                                                              3'b100 : 96k ;
                                                              其它 ： 保留 ； */
        unsigned int  fs_mic7_srcdn_dout : 3;  /* bit[12-14]: MIC8上行通路右声道SRCDN输出端采样率控制：
                                                              3'b000：8k；
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k；
                                                              3'b100 : 96k ;
                                                              其它 ： 保留 ； */
        unsigned int  fs_mic8_srcdn_dout : 3;  /* bit[15-17]: MIC8上行通路右声道SRCDN输出端采样率控制：
                                                              3'b000：8k；
                                                              3'b001：16k；
                                                              3'b010：32k；
                                                              3'b011：48k；
                                                              3'b100 : 96k ;
                                                              其它 ： 保留 ； */
        unsigned int  fs_mdm_5g_l_dn_pga : 3;  /* bit[18-20]: fs_mdm_5g下行通路左声道PGA的采样率控制：
                                                              3'b000：48k；
                                                              3'b001：96k；
                                                              其它：保留； */
        unsigned int  fs_mdm_5g_r_dn_pga : 3;  /* bit[21-23]: fs_mdm_5g下行通路右声道PGA的采样率控制：
                                                              3'b000：48k；
                                                              3'b001：96k；
                                                              其它：保留； */
        unsigned int  reserved           : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_FS_CTRL9_UNION;
#endif
#define SOC_ASP_CODEC_FS_CTRL9_fs_mic5_up_pga_START      (0)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mic5_up_pga_END        (2)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mic6_up_pga_START      (3)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mic6_up_pga_END        (5)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mic7_up_pga_START      (6)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mic7_up_pga_END        (8)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mic8_up_pga_START      (9)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mic8_up_pga_END        (11)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mic7_srcdn_dout_START  (12)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mic7_srcdn_dout_END    (14)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mic8_srcdn_dout_START  (15)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mic8_srcdn_dout_END    (17)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mdm_5g_l_dn_pga_START  (18)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mdm_5g_l_dn_pga_END    (20)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mdm_5g_r_dn_pga_START  (21)
#define SOC_ASP_CODEC_FS_CTRL9_fs_mdm_5g_r_dn_pga_END    (23)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_ADC6_DC_OFFSET_UNION
 结构说明  : CODEC_ADC6_DC_OFFSET 寄存器结构定义。地址偏移量:0x0194，初值:0x00000000，宽度:32
 寄存器说明: ADC6通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc6_dc_offset : 24; /* bit[0-23] : adc6_hpf dc_offset配置 */
        unsigned int  reserved       : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_ADC6_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_ADC6_DC_OFFSET_adc6_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_ADC6_DC_OFFSET_adc6_dc_offset_END    (23)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_ADC7_DC_OFFSET_UNION
 结构说明  : CODEC_ADC7_DC_OFFSET 寄存器结构定义。地址偏移量:0x0198，初值:0x00000000，宽度:32
 寄存器说明: ADC7通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc7_dc_offset : 24; /* bit[0-23] : adc7_hpf dc_offset配置 */
        unsigned int  reserved       : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_ADC7_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_ADC7_DC_OFFSET_adc7_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_ADC7_DC_OFFSET_adc7_dc_offset_END    (23)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_ADC8_DC_OFFSET_UNION
 结构说明  : CODEC_ADC8_DC_OFFSET 寄存器结构定义。地址偏移量:0x019C，初值:0x00000000，宽度:32
 寄存器说明: ADC8通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc8_dc_offset : 24; /* bit[0-23] : adc8_hpf dc_offset配置 */
        unsigned int  reserved       : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_ADC8_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_ADC8_DC_OFFSET_adc8_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_ADC8_DC_OFFSET_adc8_dc_offset_END    (23)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL5_UNION
 结构说明  : PGA_GAINOFFSET_CTRL5 寄存器结构定义。地址偏移量:0x01A0，初值:0x00000000，宽度:32
 寄存器说明: PGA GAINOFFSET配置寄存器2
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                   : 16; /* bit[0-15] : reserved */
        unsigned int  mdm_5g_dn_l_pga_gainoffset : 8;  /* bit[16-23]: 增益偏置，gainoffset有效配置范围是0~255
                                                                      gain_offset的调整粒度与fadeInTime相关，
                                                                      映射关系为粒度= max(2^(fadeInTime -12),1)，
                                                                      以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  mdm_5g_dn_r_pga_gainoffset : 8;  /* bit[24-31]: 增益偏置，gainoffset有效配置范围是0~255
                                                                      gain_offset的调整粒度与fadeInTime相关，
                                                                      映射关系为粒度= max(2^(fadeInTime -12),1)，
                                                                      以fadeInTime=16为例，gainOffset可取0,16,32……; */
    } reg;
} SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL5_UNION;
#endif
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL5_mdm_5g_dn_l_pga_gainoffset_START  (16)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL5_mdm_5g_dn_l_pga_gainoffset_END    (23)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL5_mdm_5g_dn_r_pga_gainoffset_START  (24)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL5_mdm_5g_dn_r_pga_gainoffset_END    (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_UNION
 结构说明  : MDM_5G_L_DN_PGA_CTRL 寄存器结构定义。地址偏移量:0x01A4，初值:0x00000050，宽度:32
 寄存器说明: MDM_5G下行通路左声道PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                   : 1;  /* bit[0]    : reserved */
        unsigned int  mdm_5g_l_dn_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                      1'b0: 选择对数淡出淡出
                                                                      1'b1: 选择线性淡入淡出 */
        unsigned int  mdm_5g_l_dn_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  mdm_5g_l_dn_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中
                                                                      2bit的id选择四种场景的阈值
                                                                      0：选择pga_thre_ctrl[23:0]
                                                                      1：选择pga_thre_ctrl[47:24]
                                                                      2：选择pga_thre_ctrl[71:48]
                                                                      3：选择pga_thre_ctrl[95:72] */
        unsigned int  mdm_5g_l_dn_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的
                                                                      小信号放过去，低表示低于阈值的信号钳制到0
                                                                      1：不对噪音信号做任何处理
                                                                      0：将低于阈值的噪音信号过滤成0 */
        unsigned int  mdm_5g_l_dn_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                      1'b0: 不bypass
                                                                      1'b1: bypass */
        unsigned int  mdm_5g_l_dn_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                      采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mdm_5g_l_dn_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                      采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mdm_5g_l_dn_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                      [7]：淡入淡出功能使能
                                                                      1：淡入淡出功能使能
                                                                      0：淡入淡出功能不使能
                                                                      [6]: 抗削波功能使能
                                                                      1：抗削波功能使能
                                                                      0：抗削波功能不使能
                                                                      [5]: 小信号功能使能
                                                                      1：小信号功能使能
                                                                      0：小信号功能不使能 */
        unsigned int  mdm_5g_l_dn_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                      例如：60dB配置成0x3C；
                                                                       0dB配置为0x00；
                                                                       -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_bypass_START      (10)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_bypass_END        (10)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_cfg_START         (21)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_cfg_END           (23)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_gain_START        (24)
#define SOC_ASP_CODEC_MDM_5G_L_DN_PGA_CTRL_mdm_5g_l_dn_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_UNION
 结构说明  : MDM_5G_R_DN_PGA_CTRL 寄存器结构定义。地址偏移量:0x01A8，初值:0x00000050，宽度:32
 寄存器说明: MDM_5G下行通路右声道PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                   : 1;  /* bit[0]    : reserved */
        unsigned int  mdm_5g_r_dn_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                      1'b0: 选择对数淡出淡出
                                                                      1'b1: 选择线性淡入淡出 */
        unsigned int  mdm_5g_r_dn_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  mdm_5g_r_dn_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用其中2bit的id选择四种场景的阈值
                                                                      0：选择pga_thre_ctrl[23:0]
                                                                      1：选择pga_thre_ctrl[47:24]
                                                                      2：选择pga_thre_ctrl[71:48]
                                                                      3：选择pga_thre_ctrl[95:72] */
        unsigned int  mdm_5g_r_dn_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的小信号放过去，低表示低于阈值的信号钳制到0
                                                                      1：不对噪音信号做任何处理
                                                                      0：将低于阈值的噪音信号过滤成0 */
        unsigned int  mdm_5g_r_dn_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                      1'b0: 不bypass
                                                                      1'b1: bypass */
        unsigned int  mdm_5g_r_dn_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mdm_5g_r_dn_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  mdm_5g_r_dn_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                      [7]：淡入淡出功能使能
                                                                      1：淡入淡出功能使能
                                                                      0：淡入淡出功能不使能
                                                                      [6]: 抗削波功能使能
                                                                      1：抗削波功能使能
                                                                      0：抗削波功能不使能
                                                                      [5]: 小信号功能使能
                                                                      1：小信号功能使能
                                                                      0：小信号功能不使能 */
        unsigned int  mdm_5g_r_dn_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                      例如：60dB配置成0x3C；
                                                                       0dB配置为0x00；
                                                                       -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_bypass_START      (10)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_bypass_END        (10)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_cfg_START         (21)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_cfg_END           (23)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_gain_START        (24)
#define SOC_ASP_CODEC_MDM_5G_R_DN_PGA_CTRL_mdm_5g_r_dn_pga_gain_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_UNION
 结构说明  : SPAIV1_UP_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x01AC，初值:0x7A007A00，宽度:32
 寄存器说明: SPA反馈通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0               : 5;  /* bit[0-4]  : reserved */
        unsigned int  spa_v1_up_fifo_aempty_th : 5;  /* bit[5-9]  : SPA 上行V通路通路一声道AFIFO将空阈值。 */
        unsigned int  spa_v1_up_fifo_afull_th  : 5;  /* bit[10-14]: SPA 上行V通路通路一声道AFIFO将满阈值。 */
        unsigned int  spa_v1_up_fifo_clr       : 1;  /* bit[15]   : SPA上行V通路一声道AFIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
        unsigned int  reserved_1               : 5;  /* bit[16-20]: reserved */
        unsigned int  spa_i1_up_fifo_aempty_th : 5;  /* bit[21-25]: SPA上行I通路二声道AFIFO将空阈值。 */
        unsigned int  spa_i1_up_fifo_afull_th  : 5;  /* bit[26-30]: SPA上行I通路二声道AFIFO将满阈值。 */
        unsigned int  spa_i1_up_fifo_clr       : 1;  /* bit[31]   : SPA上行I通路二声道AFIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_spa_v1_up_fifo_aempty_th_START  (5)
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_spa_v1_up_fifo_aempty_th_END    (9)
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_spa_v1_up_fifo_afull_th_START   (10)
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_spa_v1_up_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_spa_v1_up_fifo_clr_START        (15)
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_spa_v1_up_fifo_clr_END          (15)
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_spa_i1_up_fifo_aempty_th_START  (21)
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_spa_i1_up_fifo_aempty_th_END    (25)
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_spa_i1_up_fifo_afull_th_START   (26)
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_spa_i1_up_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_spa_i1_up_fifo_clr_START        (31)
#define SOC_ASP_CODEC_SPAIV1_UP_AFIFO_CTRL_spa_i1_up_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_DACL_DC_OFFSET_UNION
 结构说明  : CODEC_DACL_DC_OFFSET 寄存器结构定义。地址偏移量:0x01B0，初值:0x00000000，宽度:32
 寄存器说明: DACL通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  dacl_dc_offset : 24; /* bit[0-23] : dacl_sdm_bitmatch dc_offset配置 */
        unsigned int  reserved       : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_DACL_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_DACL_DC_OFFSET_dacl_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_DACL_DC_OFFSET_dacl_dc_offset_END    (23)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_UNION
 结构说明  : SPAIV2_UP_AFIFO_CTRL 寄存器结构定义。地址偏移量:0x01B4，初值:0x7A007A00，宽度:32
 寄存器说明: SPA反馈通路AFIFO控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0               : 5;  /* bit[0-4]  : reserved */
        unsigned int  spa_v2_up_fifo_aempty_th : 5;  /* bit[5-9]  : SPA 上行V通路通路一声道AFIFO将空阈值。 */
        unsigned int  spa_v2_up_fifo_afull_th  : 5;  /* bit[10-14]: SPA 上行V通路通路一声道AFIFO将满阈值。 */
        unsigned int  spa_v2_up_fifo_clr       : 1;  /* bit[15]   : SPA上行V通路一声道AFIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
        unsigned int  reserved_1               : 5;  /* bit[16-20]: reserved */
        unsigned int  spa_i2_up_fifo_aempty_th : 5;  /* bit[21-25]: SPA上行I通路二声道AFIFO将空阈值。 */
        unsigned int  spa_i2_up_fifo_afull_th  : 5;  /* bit[26-30]: SPA上行I通路二声道AFIFO将满阈值。 */
        unsigned int  spa_i2_up_fifo_clr       : 1;  /* bit[31]   : SPA上行I通路二声道AFIFO Clear Signal, high active.
                                                                    0：不清空FIFO数据
                                                                    1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_spa_v2_up_fifo_aempty_th_START  (5)
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_spa_v2_up_fifo_aempty_th_END    (9)
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_spa_v2_up_fifo_afull_th_START   (10)
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_spa_v2_up_fifo_afull_th_END     (14)
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_spa_v2_up_fifo_clr_START        (15)
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_spa_v2_up_fifo_clr_END          (15)
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_spa_i2_up_fifo_aempty_th_START  (21)
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_spa_i2_up_fifo_aempty_th_END    (25)
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_spa_i2_up_fifo_afull_th_START   (26)
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_spa_i2_up_fifo_afull_th_END     (30)
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_spa_i2_up_fifo_clr_START        (31)
#define SOC_ASP_CODEC_SPAIV2_UP_AFIFO_CTRL_spa_i2_up_fifo_clr_END          (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_DACR_DC_OFFSET_UNION
 结构说明  : CODEC_DACR_DC_OFFSET 寄存器结构定义。地址偏移量:0x01B8，初值:0x00000000，宽度:32
 寄存器说明: DACL通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  dacr_dc_offset : 24; /* bit[0-23] : dacr_sdm_bitmatch dc_offset配置 */
        unsigned int  reserved       : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_DACR_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_DACR_DC_OFFSET_dacr_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_DACR_DC_OFFSET_dacr_dc_offset_END    (23)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_SPAIV1_ADC_FILTER_UNION
 结构说明  : SPAIV1_ADC_FILTER 寄存器结构定义。地址偏移量:0x01BC，初值:0xB82E0000，宽度:32
 寄存器说明: spa的adc控制信号
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                  : 12; /* bit[0-11] : reserved */
        unsigned int  adc_spai1_hpf_bypass_en   : 1;  /* bit[12]   : SPAI模块hpf滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spai1_hbfvd_bypass_en : 1;  /* bit[13]   : SPAI模块hbfvd滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spai1_hbf2d_bypass_en : 1;  /* bit[14]   : SPAI模块hbf2d滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spai1_compd_bypass_en : 1;  /* bit[15]   : SPAI模块compd滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spai1_cic_gain        : 6;  /* bit[16-21]: adc_spai_cic_gain增益配置 */
        unsigned int  adc_spav1_hpf_bypass_en   : 1;  /* bit[22]   : SPAV模块hpf滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spav1_hbfvd_bypass_en : 1;  /* bit[23]   : SPAV模块hbfvd滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spav1_hbf2d_bypass_en : 1;  /* bit[24]   : SPAV模块hbf2d滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spav1_compd_bypass_en : 1;  /* bit[25]   : SPAV模块compd滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spav1_cic_gain        : 6;  /* bit[26-31]: adc_spav_cic_gain增益配置 */
    } reg;
} SOC_ASP_CODEC_SPAIV1_ADC_FILTER_UNION;
#endif
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spai1_hpf_bypass_en_START    (12)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spai1_hpf_bypass_en_END      (12)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spai1_hbfvd_bypass_en_START  (13)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spai1_hbfvd_bypass_en_END    (13)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spai1_hbf2d_bypass_en_START  (14)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spai1_hbf2d_bypass_en_END    (14)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spai1_compd_bypass_en_START  (15)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spai1_compd_bypass_en_END    (15)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spai1_cic_gain_START         (16)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spai1_cic_gain_END           (21)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spav1_hpf_bypass_en_START    (22)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spav1_hpf_bypass_en_END      (22)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spav1_hbfvd_bypass_en_START  (23)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spav1_hbfvd_bypass_en_END    (23)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spav1_hbf2d_bypass_en_START  (24)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spav1_hbf2d_bypass_en_END    (24)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spav1_compd_bypass_en_START  (25)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spav1_compd_bypass_en_END    (25)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spav1_cic_gain_START         (26)
#define SOC_ASP_CODEC_SPAIV1_ADC_FILTER_adc_spav1_cic_gain_END           (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CLASSH_L_CTRL1_UNION
 结构说明  : CLASSH_L_CTRL1 寄存器结构定义。地址偏移量:0x01C0，初值:0x00000001，宽度:32
 寄存器说明: classh的控制信号
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  dn_l_cfg            : 4;  /* bit[0-3]  : 延时的拍数(默认64拍)：
                                                               4’b0000：延时48拍；
                                                               4‘b0001：延时64拍；
                                                               4’b0010：延时80拍；
                                                               4‘b0011：延时96拍；
                                                               4’b0100：延时112拍；
                                                               4‘b0101：延时128拍；
                                                               4’b0110：延时144拍；
                                                               4‘b0111：延时160拍；
                                                               4’b1000：延时176拍；
                                                               4‘b1001：延时192拍；
                                                               其它：保留 */
        unsigned int  fall_delay_l_en     : 1;  /* bit[4]    : fall_delay功能的使能
                                                               1'b0：不使能；
                                                               1'b1：使能； */
        unsigned int  reserved_0          : 3;  /* bit[5-7]  : reserved */
        unsigned int  do_l_cfg            : 4;  /* bit[8-11] : do_delay的延时配置：
                                                               4’b000：延迟0拍；
                                                               4’b001：延迟4拍；
                                                               4‘b010：延迟8拍；
                                                               4’b011：延迟16拍；
                                                               4‘b100：延迟24拍；
                                                               4’b101：延时32拍；
                                                               4’b110：延时40拍；
                                                               4‘b111：延时48拍；
                                                               4‘b1000：延时56拍；
                                                               4’b1001：延时64拍；
                                                               其它：保留； */
        unsigned int  reserved_1          : 4;  /* bit[12-15]: reserved */
        unsigned int  fall_delay_df_l_cfg : 15; /* bit[16-30]: fall_delay功能的延时配置寄存器 */
        unsigned int  classh_l_bypass     : 1;  /* bit[31]   : classh的bypass
                                                               1'b0：不bypass；
                                                               1'b1：bypass； */
    } reg;
} SOC_ASP_CODEC_CLASSH_L_CTRL1_UNION;
#endif
#define SOC_ASP_CODEC_CLASSH_L_CTRL1_dn_l_cfg_START             (0)
#define SOC_ASP_CODEC_CLASSH_L_CTRL1_dn_l_cfg_END               (3)
#define SOC_ASP_CODEC_CLASSH_L_CTRL1_fall_delay_l_en_START      (4)
#define SOC_ASP_CODEC_CLASSH_L_CTRL1_fall_delay_l_en_END        (4)
#define SOC_ASP_CODEC_CLASSH_L_CTRL1_do_l_cfg_START             (8)
#define SOC_ASP_CODEC_CLASSH_L_CTRL1_do_l_cfg_END               (11)
#define SOC_ASP_CODEC_CLASSH_L_CTRL1_fall_delay_df_l_cfg_START  (16)
#define SOC_ASP_CODEC_CLASSH_L_CTRL1_fall_delay_df_l_cfg_END    (30)
#define SOC_ASP_CODEC_CLASSH_L_CTRL1_classh_l_bypass_START      (31)
#define SOC_ASP_CODEC_CLASSH_L_CTRL1_classh_l_bypass_END        (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_DAC_FILTER_CTRL_UNION
 结构说明  : DAC_FILTER_CTRL 寄存器结构定义。地址偏移量:0x01C4，初值:0x00000000，宽度:32
 寄存器说明: DAC_FILTER控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  sdm_r_calt_vld       : 1;  /* bit[0]   : SDM_R功能使能。
                                                               0：不使能；
                                                               1：使能。 */
        unsigned int  sdm_r_dither         : 1;  /* bit[1]   : SDM_R dither使能控制。
                                                               0：不使能；
                                                               1：使能。 */
        unsigned int  dacr_fir2d_bypass_en : 1;  /* bit[2]   : DACR滤波器通道FIR2D bypass配置:
                                                               1：bypass;
                                                               0：work. */
        unsigned int  dacr_fir2c_bypass_en : 1;  /* bit[3]   : DACR滤波器通道FIR2C bypass配置:
                                                               1：bypass;
                                                               0：work. */
        unsigned int  sdm_l_calt_vld       : 1;  /* bit[4]   : SDM_L功能使能。
                                                               0：不使能；
                                                               1：使能。 */
        unsigned int  sdm_l_dither         : 1;  /* bit[5]   : SDM_L dither使能控制。
                                                               0：不使能；
                                                               1：使能。 */
        unsigned int  dacl_fir2d_bypass_en : 1;  /* bit[6]   : DACL滤波器通道FIR2D bypass配置:
                                                               1：bypass;
                                                               0：work. */
        unsigned int  dacl_fir2c_bypass_en : 1;  /* bit[7]   : DACL滤波器通道FIR2C bypass配置:
                                                               1：bypass;
                                                               0：work. */
        unsigned int  reserved             : 24; /* bit[8-31]: reserved */
    } reg;
} SOC_ASP_CODEC_DAC_FILTER_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_sdm_r_calt_vld_START        (0)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_sdm_r_calt_vld_END          (0)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_sdm_r_dither_START          (1)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_sdm_r_dither_END            (1)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_dacr_fir2d_bypass_en_START  (2)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_dacr_fir2d_bypass_en_END    (2)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_dacr_fir2c_bypass_en_START  (3)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_dacr_fir2c_bypass_en_END    (3)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_sdm_l_calt_vld_START        (4)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_sdm_l_calt_vld_END          (4)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_sdm_l_dither_START          (5)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_sdm_l_dither_END            (5)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_dacl_fir2d_bypass_en_START  (6)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_dacl_fir2d_bypass_en_END    (6)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_dacl_fir2c_bypass_en_START  (7)
#define SOC_ASP_CODEC_DAC_FILTER_CTRL_dacl_fir2c_bypass_en_END    (7)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CLASSH_L_CTRL2_UNION
 结构说明  : CLASSH_L_CTRL2 寄存器结构定义。地址偏移量:0x01C8，初值:0x00000000，宽度:32
 寄存器说明: classh的控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  classh_l_pga_gain : 9;  /* bit[0-8]  : pga的增益配置 */
        unsigned int  classh_l_dym_en   : 1;  /* bit[9]    : ClassH动态模式使能：
                                                             0：ClassH静态模式，根据系统配置的音量标示，数字通路增益与
                                                             模拟通路增益计算需要的BUCK1/CP1输出电压；(信号可能的最大值)
                                                             1：ClassH动态模式，根据SRC16输入数字信号幅度与模拟通路增益
                                                             计算需要的BUCK1/CP1输出电压（信号过去未来一段时间实际最大值）； */
        unsigned int  classh_l_en       : 1;  /* bit[10]   : classh/classab切换：
                                                             0：classab模式，BUCK电压由reg_vctrl1控制；
                                                             1：classh模式，BUCK电压由信号和通路增益确定； */
        unsigned int  reserved_0        : 5;  /* bit[11-15]: reserved */
        unsigned int  reg_l_vctrl1      : 8;  /* bit[16-23]: 寄存器配置的调压控制信号 */
        unsigned int  fall_l_step       : 6;  /* bit[24-29]: 调压步进信号 */
        unsigned int  reserved_1        : 2;  /* bit[30-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CLASSH_L_CTRL2_UNION;
#endif
#define SOC_ASP_CODEC_CLASSH_L_CTRL2_classh_l_pga_gain_START  (0)
#define SOC_ASP_CODEC_CLASSH_L_CTRL2_classh_l_pga_gain_END    (8)
#define SOC_ASP_CODEC_CLASSH_L_CTRL2_classh_l_dym_en_START    (9)
#define SOC_ASP_CODEC_CLASSH_L_CTRL2_classh_l_dym_en_END      (9)
#define SOC_ASP_CODEC_CLASSH_L_CTRL2_classh_l_en_START        (10)
#define SOC_ASP_CODEC_CLASSH_L_CTRL2_classh_l_en_END          (10)
#define SOC_ASP_CODEC_CLASSH_L_CTRL2_reg_l_vctrl1_START       (16)
#define SOC_ASP_CODEC_CLASSH_L_CTRL2_reg_l_vctrl1_END         (23)
#define SOC_ASP_CODEC_CLASSH_L_CTRL2_fall_l_step_START        (24)
#define SOC_ASP_CODEC_CLASSH_L_CTRL2_fall_l_step_END          (29)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_SPAIV2_ADC_FILTER_UNION
 结构说明  : SPAIV2_ADC_FILTER 寄存器结构定义。地址偏移量:0x01CC，初值:0xB82E0000，宽度:32
 寄存器说明: spa的adc控制信号
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                  : 12; /* bit[0-11] : reserved */
        unsigned int  adc_spai2_hpf_bypass_en   : 1;  /* bit[12]   : SPAI模块hpf滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spai2_hbfvd_bypass_en : 1;  /* bit[13]   : SPAI模块hbfvd滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spai2_hbf2d_bypass_en : 1;  /* bit[14]   : SPAI模块hbf2d滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spai2_compd_bypass_en : 1;  /* bit[15]   : SPAI模块compd滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spai2_cic_gain        : 6;  /* bit[16-21]: adc_spai_cic_gain增益配置 */
        unsigned int  adc_spav2_hpf_bypass_en   : 1;  /* bit[22]   : SPAV模块hpf滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spav2_hbfvd_bypass_en : 1;  /* bit[23]   : SPAV模块hbfvd滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spav2_hbf2d_bypass_en : 1;  /* bit[24]   : SPAV模块hbf2d滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spav2_compd_bypass_en : 1;  /* bit[25]   : SPAV模块compd滤波器 bypass配置:
                                                                     1：bypass;
                                                                     0：work. */
        unsigned int  adc_spav2_cic_gain        : 6;  /* bit[26-31]: adc_spav_cic_gain增益配置 */
    } reg;
} SOC_ASP_CODEC_SPAIV2_ADC_FILTER_UNION;
#endif
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spai2_hpf_bypass_en_START    (12)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spai2_hpf_bypass_en_END      (12)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spai2_hbfvd_bypass_en_START  (13)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spai2_hbfvd_bypass_en_END    (13)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spai2_hbf2d_bypass_en_START  (14)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spai2_hbf2d_bypass_en_END    (14)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spai2_compd_bypass_en_START  (15)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spai2_compd_bypass_en_END    (15)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spai2_cic_gain_START         (16)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spai2_cic_gain_END           (21)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spav2_hpf_bypass_en_START    (22)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spav2_hpf_bypass_en_END      (22)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spav2_hbfvd_bypass_en_START  (23)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spav2_hbfvd_bypass_en_END    (23)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spav2_hbf2d_bypass_en_START  (24)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spav2_hbf2d_bypass_en_END    (24)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spav2_compd_bypass_en_START  (25)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spav2_compd_bypass_en_END    (25)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spav2_cic_gain_START         (26)
#define SOC_ASP_CODEC_SPAIV2_ADC_FILTER_adc_spav2_cic_gain_END           (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CLASSH_R_CTRL1_UNION
 结构说明  : CLASSH_R_CTRL1 寄存器结构定义。地址偏移量:0x01D0，初值:0x00000001，宽度:32
 寄存器说明: classh的控制信号
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  dn_r_cfg            : 4;  /* bit[0-3]  : 延时的拍数(默认64拍)：
                                                               4’b0000：延时48拍；
                                                               4‘b0001：延时64拍；
                                                               4’b0010：延时80拍；
                                                               4‘b0011：延时96拍；
                                                               4’b0100：延时112拍；
                                                               4‘b0101：延时128拍；
                                                               4’b0110：延时144拍；
                                                               4‘b0111：延时160拍；
                                                               4’b1000：延时176拍；
                                                               4‘b1001：延时192拍；
                                                               其它：保留 */
        unsigned int  fall_delay_r_en     : 1;  /* bit[4]    : fall_delay功能的使能
                                                               1'b0：不使能；
                                                               1'b1：使能； */
        unsigned int  reserved_0          : 3;  /* bit[5-7]  : reserved */
        unsigned int  do_r_cfg            : 4;  /* bit[8-11] : do_delay的延时配置：
                                                               4’b000：延迟0拍；
                                                               4’b001：延迟4拍；
                                                               4‘b010：延迟8拍；
                                                               4’b011：延迟16拍；
                                                               4‘b100：延迟24拍；
                                                               4’b101：延时32拍；
                                                               4’b110：延时40拍；
                                                               4‘b111：延时48拍；
                                                               4‘b1000：延时56拍；
                                                               4’b1001：延时64拍；
                                                               其它：保留； */
        unsigned int  reserved_1          : 4;  /* bit[12-15]: reserved */
        unsigned int  fall_delay_df_r_cfg : 15; /* bit[16-30]: fall_delay功能的延时配置寄存器 */
        unsigned int  classh_r_bypass     : 1;  /* bit[31]   : classh的bypass
                                                               1'b0：不bypass；
                                                               1'b1：bypass； */
    } reg;
} SOC_ASP_CODEC_CLASSH_R_CTRL1_UNION;
#endif
#define SOC_ASP_CODEC_CLASSH_R_CTRL1_dn_r_cfg_START             (0)
#define SOC_ASP_CODEC_CLASSH_R_CTRL1_dn_r_cfg_END               (3)
#define SOC_ASP_CODEC_CLASSH_R_CTRL1_fall_delay_r_en_START      (4)
#define SOC_ASP_CODEC_CLASSH_R_CTRL1_fall_delay_r_en_END        (4)
#define SOC_ASP_CODEC_CLASSH_R_CTRL1_do_r_cfg_START             (8)
#define SOC_ASP_CODEC_CLASSH_R_CTRL1_do_r_cfg_END               (11)
#define SOC_ASP_CODEC_CLASSH_R_CTRL1_fall_delay_df_r_cfg_START  (16)
#define SOC_ASP_CODEC_CLASSH_R_CTRL1_fall_delay_df_r_cfg_END    (30)
#define SOC_ASP_CODEC_CLASSH_R_CTRL1_classh_r_bypass_START      (31)
#define SOC_ASP_CODEC_CLASSH_R_CTRL1_classh_r_bypass_END        (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CLASSH_R_CTRL2_UNION
 结构说明  : CLASSH_R_CTRL2 寄存器结构定义。地址偏移量:0x01D4，初值:0x00000000，宽度:32
 寄存器说明: classh的控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  classh_r_pga_gain : 9;  /* bit[0-8]  : pga的增益配置 */
        unsigned int  classh_r_dym_en   : 1;  /* bit[9]    : ClassH动态模式使能：
                                                             0：ClassH静态模式，根据系统配置的音量标示，数字通路增益与模拟
                                                             通路增益计算需要的BUCK1/CP1输出电压；(信号可能的最大值)
                                                             1：ClassH动态模式，根据SRC16输入数字信号幅度与模拟通路增益
                                                             计算需要的BUCK1/CP1输出电压（信号过去未来一段时间实际最大值）； */
        unsigned int  classh_r_en       : 1;  /* bit[10]   : classh/classab切换：
                                                             0：classab模式，BUCK电压由reg_vctrl1控制；
                                                             1：classh模式，BUCK电压由信号和通路增益确定； */
        unsigned int  reserved_0        : 5;  /* bit[11-15]: reserved */
        unsigned int  reg_r_vctrl1      : 8;  /* bit[16-23]: 寄存器配置的调压控制信号 */
        unsigned int  fall_r_step       : 6;  /* bit[24-29]: 调压步进信号 */
        unsigned int  reserved_1        : 2;  /* bit[30-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CLASSH_R_CTRL2_UNION;
#endif
#define SOC_ASP_CODEC_CLASSH_R_CTRL2_classh_r_pga_gain_START  (0)
#define SOC_ASP_CODEC_CLASSH_R_CTRL2_classh_r_pga_gain_END    (8)
#define SOC_ASP_CODEC_CLASSH_R_CTRL2_classh_r_dym_en_START    (9)
#define SOC_ASP_CODEC_CLASSH_R_CTRL2_classh_r_dym_en_END      (9)
#define SOC_ASP_CODEC_CLASSH_R_CTRL2_classh_r_en_START        (10)
#define SOC_ASP_CODEC_CLASSH_R_CTRL2_classh_r_en_END          (10)
#define SOC_ASP_CODEC_CLASSH_R_CTRL2_reg_r_vctrl1_START       (16)
#define SOC_ASP_CODEC_CLASSH_R_CTRL2_reg_r_vctrl1_END         (23)
#define SOC_ASP_CODEC_CLASSH_R_CTRL2_fall_r_step_START        (24)
#define SOC_ASP_CODEC_CLASSH_R_CTRL2_fall_r_step_END          (29)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG0_3_UNION
 结构说明  : LUT_L_REG0_3 寄存器结构定义。地址偏移量:0x01D8，初值:0x17171717，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg0 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0 : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg1 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1 : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg2 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2 : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg3 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3 : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG0_3_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG0_3_lut_l_reg0_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG0_3_lut_l_reg0_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG0_3_lut_l_reg1_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG0_3_lut_l_reg1_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG0_3_lut_l_reg2_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG0_3_lut_l_reg2_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG0_3_lut_l_reg3_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG0_3_lut_l_reg3_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG0_3_UNION
 结构说明  : LUT_R_REG0_3 寄存器结构定义。地址偏移量:0x01DC，初值:0x17171717，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg0 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0 : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg1 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1 : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg2 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2 : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg3 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3 : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG0_3_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG0_3_lut_r_reg0_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG0_3_lut_r_reg0_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG0_3_lut_r_reg1_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG0_3_lut_r_reg1_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG0_3_lut_r_reg2_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG0_3_lut_r_reg2_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG0_3_lut_r_reg3_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG0_3_lut_r_reg3_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG4_7_UNION
 结构说明  : LUT_L_REG4_7 寄存器结构定义。地址偏移量:0x01E0，初值:0x17171717，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg4 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0 : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg5 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1 : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg6 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2 : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg7 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3 : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG4_7_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG4_7_lut_l_reg4_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG4_7_lut_l_reg4_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG4_7_lut_l_reg5_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG4_7_lut_l_reg5_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG4_7_lut_l_reg6_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG4_7_lut_l_reg6_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG4_7_lut_l_reg7_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG4_7_lut_l_reg7_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG4_7_UNION
 结构说明  : LUT_R_REG4_7 寄存器结构定义。地址偏移量:0x01E4，初值:0x17171717，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg4 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0 : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg5 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1 : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg6 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2 : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg7 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3 : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG4_7_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG4_7_lut_r_reg4_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG4_7_lut_r_reg4_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG4_7_lut_r_reg5_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG4_7_lut_r_reg5_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG4_7_lut_r_reg6_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG4_7_lut_r_reg6_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG4_7_lut_r_reg7_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG4_7_lut_r_reg7_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG8_11_UNION
 结构说明  : LUT_L_REG8_11 寄存器结构定义。地址偏移量:0x01E8，初值:0x17171717，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg8  : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg9  : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg10 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg11 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG8_11_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG8_11_lut_l_reg8_START   (0)
#define SOC_ASP_CODEC_LUT_L_REG8_11_lut_l_reg8_END     (6)
#define SOC_ASP_CODEC_LUT_L_REG8_11_lut_l_reg9_START   (8)
#define SOC_ASP_CODEC_LUT_L_REG8_11_lut_l_reg9_END     (14)
#define SOC_ASP_CODEC_LUT_L_REG8_11_lut_l_reg10_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG8_11_lut_l_reg10_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG8_11_lut_l_reg11_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG8_11_lut_l_reg11_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG8_11_UNION
 结构说明  : LUT_R_REG8_11 寄存器结构定义。地址偏移量:0x01EC，初值:0x17171717，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg8  : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg9  : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg10 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg11 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG8_11_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG8_11_lut_r_reg8_START   (0)
#define SOC_ASP_CODEC_LUT_R_REG8_11_lut_r_reg8_END     (6)
#define SOC_ASP_CODEC_LUT_R_REG8_11_lut_r_reg9_START   (8)
#define SOC_ASP_CODEC_LUT_R_REG8_11_lut_r_reg9_END     (14)
#define SOC_ASP_CODEC_LUT_R_REG8_11_lut_r_reg10_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG8_11_lut_r_reg10_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG8_11_lut_r_reg11_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG8_11_lut_r_reg11_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG12_15_UNION
 结构说明  : LUT_L_REG12_15 寄存器结构定义。地址偏移量:0x01F0，初值:0x17171717，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg12 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg13 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg14 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg15 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG12_15_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG12_15_lut_l_reg12_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG12_15_lut_l_reg12_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG12_15_lut_l_reg13_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG12_15_lut_l_reg13_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG12_15_lut_l_reg14_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG12_15_lut_l_reg14_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG12_15_lut_l_reg15_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG12_15_lut_l_reg15_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG12_15_UNION
 结构说明  : LUT_R_REG12_15 寄存器结构定义。地址偏移量:0x01F4，初值:0x17171717，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg12 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg13 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg14 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg15 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG12_15_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG12_15_lut_r_reg12_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG12_15_lut_r_reg12_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG12_15_lut_r_reg13_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG12_15_lut_r_reg13_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG12_15_lut_r_reg14_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG12_15_lut_r_reg14_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG12_15_lut_r_reg15_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG12_15_lut_r_reg15_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG16_19_UNION
 结构说明  : LUT_L_REG16_19 寄存器结构定义。地址偏移量:0x01F8，初值:0x59585757，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg16 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg17 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg18 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg19 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG16_19_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG16_19_lut_l_reg16_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG16_19_lut_l_reg16_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG16_19_lut_l_reg17_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG16_19_lut_l_reg17_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG16_19_lut_l_reg18_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG16_19_lut_l_reg18_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG16_19_lut_l_reg19_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG16_19_lut_l_reg19_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG16_19_UNION
 结构说明  : LUT_R_REG16_19 寄存器结构定义。地址偏移量:0x01FC，初值:0x59585757，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg16 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg17 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg18 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg19 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG16_19_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG16_19_lut_r_reg16_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG16_19_lut_r_reg16_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG16_19_lut_r_reg17_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG16_19_lut_r_reg17_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG16_19_lut_r_reg18_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG16_19_lut_r_reg18_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG16_19_lut_r_reg19_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG16_19_lut_r_reg19_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG20_23_UNION
 结构说明  : LUT_L_REG20_23 寄存器结构定义。地址偏移量:0x0200，初值:0x5C5B5A5A，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg20 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg21 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg22 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg23 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG20_23_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG20_23_lut_l_reg20_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG20_23_lut_l_reg20_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG20_23_lut_l_reg21_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG20_23_lut_l_reg21_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG20_23_lut_l_reg22_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG20_23_lut_l_reg22_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG20_23_lut_l_reg23_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG20_23_lut_l_reg23_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG20_23_UNION
 结构说明  : LUT_R_REG20_23 寄存器结构定义。地址偏移量:0x0204，初值:0x5C5B5A5A，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg20 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg21 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg22 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg23 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG20_23_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG20_23_lut_r_reg20_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG20_23_lut_r_reg20_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG20_23_lut_r_reg21_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG20_23_lut_r_reg21_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG20_23_lut_r_reg22_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG20_23_lut_r_reg22_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG20_23_lut_r_reg23_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG20_23_lut_r_reg23_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG24_27_UNION
 结构说明  : LUT_L_REG24_27 寄存器结构定义。地址偏移量:0x0208，初值:0x5F5E5D5D，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg24 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg25 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg26 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg27 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG24_27_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG24_27_lut_l_reg24_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG24_27_lut_l_reg24_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG24_27_lut_l_reg25_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG24_27_lut_l_reg25_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG24_27_lut_l_reg26_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG24_27_lut_l_reg26_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG24_27_lut_l_reg27_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG24_27_lut_l_reg27_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG24_27_UNION
 结构说明  : LUT_R_REG24_27 寄存器结构定义。地址偏移量:0x020C，初值:0x5F5E5D5D，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg24 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg25 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg26 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg27 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG24_27_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG24_27_lut_r_reg24_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG24_27_lut_r_reg24_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG24_27_lut_r_reg25_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG24_27_lut_r_reg25_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG24_27_lut_r_reg26_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG24_27_lut_r_reg26_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG24_27_lut_r_reg27_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG24_27_lut_r_reg27_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG28_31_UNION
 结构说明  : LUT_L_REG28_31 寄存器结构定义。地址偏移量:0x0210，初值:0x62616060，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg28 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg29 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg30 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg31 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG28_31_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG28_31_lut_l_reg28_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG28_31_lut_l_reg28_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG28_31_lut_l_reg29_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG28_31_lut_l_reg29_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG28_31_lut_l_reg30_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG28_31_lut_l_reg30_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG28_31_lut_l_reg31_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG28_31_lut_l_reg31_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG28_31_UNION
 结构说明  : LUT_R_REG28_31 寄存器结构定义。地址偏移量:0x0214，初值:0x62616060，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg28 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg29 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg30 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg31 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG28_31_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG28_31_lut_r_reg28_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG28_31_lut_r_reg28_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG28_31_lut_r_reg29_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG28_31_lut_r_reg29_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG28_31_lut_r_reg30_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG28_31_lut_r_reg30_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG28_31_lut_r_reg31_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG28_31_lut_r_reg31_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG32_35_UNION
 结构说明  : LUT_L_REG32_35 寄存器结构定义。地址偏移量:0x0218，初值:0x65646363，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg32 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg33 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg34 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg35 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG32_35_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG32_35_lut_l_reg32_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG32_35_lut_l_reg32_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG32_35_lut_l_reg33_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG32_35_lut_l_reg33_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG32_35_lut_l_reg34_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG32_35_lut_l_reg34_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG32_35_lut_l_reg35_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG32_35_lut_l_reg35_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG32_35_UNION
 结构说明  : LUT_R_REG32_35 寄存器结构定义。地址偏移量:0x021C，初值:0x65646363，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg32 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg33 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg34 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg35 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG32_35_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG32_35_lut_r_reg32_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG32_35_lut_r_reg32_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG32_35_lut_r_reg33_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG32_35_lut_r_reg33_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG32_35_lut_r_reg34_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG32_35_lut_r_reg34_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG32_35_lut_r_reg35_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG32_35_lut_r_reg35_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG36_39_UNION
 结构说明  : LUT_L_REG36_39 寄存器结构定义。地址偏移量:0x0220，初值:0x68676666，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg36 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg37 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg38 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg39 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG36_39_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG36_39_lut_l_reg36_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG36_39_lut_l_reg36_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG36_39_lut_l_reg37_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG36_39_lut_l_reg37_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG36_39_lut_l_reg38_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG36_39_lut_l_reg38_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG36_39_lut_l_reg39_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG36_39_lut_l_reg39_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG36_39_UNION
 结构说明  : LUT_R_REG36_39 寄存器结构定义。地址偏移量:0x0224，初值:0x68676666，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg36 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg37 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg38 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg39 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG36_39_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG36_39_lut_r_reg36_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG36_39_lut_r_reg36_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG36_39_lut_r_reg37_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG36_39_lut_r_reg37_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG36_39_lut_r_reg38_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG36_39_lut_r_reg38_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG36_39_lut_r_reg39_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG36_39_lut_r_reg39_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG40_43_UNION
 结构说明  : LUT_L_REG40_43 寄存器结构定义。地址偏移量:0x0228，初值:0x6B6A6969，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg40 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg41 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg42 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg43 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG40_43_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG40_43_lut_l_reg40_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG40_43_lut_l_reg40_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG40_43_lut_l_reg41_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG40_43_lut_l_reg41_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG40_43_lut_l_reg42_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG40_43_lut_l_reg42_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG40_43_lut_l_reg43_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG40_43_lut_l_reg43_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG40_43_UNION
 结构说明  : LUT_R_REG40_43 寄存器结构定义。地址偏移量:0x022C，初值:0x6B6A6969，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg40 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg41 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg42 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg43 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG40_43_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG40_43_lut_r_reg40_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG40_43_lut_r_reg40_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG40_43_lut_r_reg41_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG40_43_lut_r_reg41_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG40_43_lut_r_reg42_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG40_43_lut_r_reg42_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG40_43_lut_r_reg43_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG40_43_lut_r_reg43_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG44_47_UNION
 结构说明  : LUT_L_REG44_47 寄存器结构定义。地址偏移量:0x0230，初值:0x6E6D6C6B，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg44 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg45 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg46 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg47 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG44_47_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG44_47_lut_l_reg44_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG44_47_lut_l_reg44_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG44_47_lut_l_reg45_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG44_47_lut_l_reg45_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG44_47_lut_l_reg46_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG44_47_lut_l_reg46_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG44_47_lut_l_reg47_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG44_47_lut_l_reg47_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG44_47_UNION
 结构说明  : LUT_R_REG44_47 寄存器结构定义。地址偏移量:0x0234，初值:0x6E6D6C6B，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg44 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg45 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg46 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg47 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG44_47_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG44_47_lut_r_reg44_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG44_47_lut_r_reg44_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG44_47_lut_r_reg45_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG44_47_lut_r_reg45_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG44_47_lut_r_reg46_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG44_47_lut_r_reg46_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG44_47_lut_r_reg47_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG44_47_lut_r_reg47_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG48_51_UNION
 结构说明  : LUT_L_REG48_51 寄存器结构定义。地址偏移量:0x0238，初值:0x71706F6E，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg48 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg49 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg50 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg51 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG48_51_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG48_51_lut_l_reg48_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG48_51_lut_l_reg48_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG48_51_lut_l_reg49_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG48_51_lut_l_reg49_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG48_51_lut_l_reg50_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG48_51_lut_l_reg50_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG48_51_lut_l_reg51_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG48_51_lut_l_reg51_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG48_51_UNION
 结构说明  : LUT_R_REG48_51 寄存器结构定义。地址偏移量:0x023C，初值:0x71706F6E，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg48 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg49 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg50 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg51 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG48_51_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG48_51_lut_r_reg48_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG48_51_lut_r_reg48_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG48_51_lut_r_reg49_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG48_51_lut_r_reg49_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG48_51_lut_r_reg50_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG48_51_lut_r_reg50_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG48_51_lut_r_reg51_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG48_51_lut_r_reg51_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG52_55_UNION
 结构说明  : LUT_L_REG52_55 寄存器结构定义。地址偏移量:0x0240，初值:0x74737271，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg52 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg53 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg54 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg55 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG52_55_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG52_55_lut_l_reg52_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG52_55_lut_l_reg52_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG52_55_lut_l_reg53_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG52_55_lut_l_reg53_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG52_55_lut_l_reg54_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG52_55_lut_l_reg54_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG52_55_lut_l_reg55_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG52_55_lut_l_reg55_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG52_55_UNION
 结构说明  : LUT_R_REG52_55 寄存器结构定义。地址偏移量:0x0244，初值:0x74737271，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg52 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg53 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg54 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg55 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG52_55_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG52_55_lut_r_reg52_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG52_55_lut_r_reg52_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG52_55_lut_r_reg53_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG52_55_lut_r_reg53_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG52_55_lut_r_reg54_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG52_55_lut_r_reg54_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG52_55_lut_r_reg55_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG52_55_lut_r_reg55_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG56_59_UNION
 结构说明  : LUT_L_REG56_59 寄存器结构定义。地址偏移量:0x0248，初值:0x77767575，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg56 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg57 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg58 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg59 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG56_59_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG56_59_lut_l_reg56_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG56_59_lut_l_reg56_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG56_59_lut_l_reg57_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG56_59_lut_l_reg57_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG56_59_lut_l_reg58_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG56_59_lut_l_reg58_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG56_59_lut_l_reg59_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG56_59_lut_l_reg59_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG56_59_UNION
 结构说明  : LUT_R_REG56_59 寄存器结构定义。地址偏移量:0x024C，初值:0x77767575，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg56 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg57 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg58 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg59 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG56_59_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG56_59_lut_r_reg56_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG56_59_lut_r_reg56_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG56_59_lut_r_reg57_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG56_59_lut_r_reg57_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG56_59_lut_r_reg58_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG56_59_lut_r_reg58_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG56_59_lut_r_reg59_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG56_59_lut_r_reg59_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG60_63_UNION
 结构说明  : LUT_L_REG60_63 寄存器结构定义。地址偏移量:0x0250，初值:0x7A797878，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg60 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg61 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg62 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg63 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG60_63_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG60_63_lut_l_reg60_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG60_63_lut_l_reg60_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG60_63_lut_l_reg61_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG60_63_lut_l_reg61_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG60_63_lut_l_reg62_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG60_63_lut_l_reg62_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG60_63_lut_l_reg63_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG60_63_lut_l_reg63_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG60_63_UNION
 结构说明  : LUT_R_REG60_63 寄存器结构定义。地址偏移量:0x0254，初值:0x7A797878，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg60 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg61 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg62 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg63 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG60_63_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG60_63_lut_r_reg60_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG60_63_lut_r_reg60_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG60_63_lut_r_reg61_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG60_63_lut_r_reg61_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG60_63_lut_r_reg62_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG60_63_lut_r_reg62_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG60_63_lut_r_reg63_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG60_63_lut_r_reg63_END    (30)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_L_REG64_67_UNION
 结构说明  : LUT_L_REG64_67 寄存器结构定义。地址偏移量:0x0258，初值:0x7D7C7B7A，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_l_reg64 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_l_reg65 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_l_reg66 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_l_reg67 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_L_REG64_67_UNION;
#endif
#define SOC_ASP_CODEC_LUT_L_REG64_67_lut_l_reg64_START  (0)
#define SOC_ASP_CODEC_LUT_L_REG64_67_lut_l_reg64_END    (6)
#define SOC_ASP_CODEC_LUT_L_REG64_67_lut_l_reg65_START  (8)
#define SOC_ASP_CODEC_LUT_L_REG64_67_lut_l_reg65_END    (14)
#define SOC_ASP_CODEC_LUT_L_REG64_67_lut_l_reg66_START  (16)
#define SOC_ASP_CODEC_LUT_L_REG64_67_lut_l_reg66_END    (22)
#define SOC_ASP_CODEC_LUT_L_REG64_67_lut_l_reg67_START  (24)
#define SOC_ASP_CODEC_LUT_L_REG64_67_lut_l_reg67_END    (30)

/*****************************************************************************
 结构名    : SOC_ASP_CODEC_LUT_R_REG64_67_UNION
 结构说明  : LUT_R_REG64_67 寄存器结构定义。地址偏移量:0x025C，初值:0x7D7C7B7A，宽度:32
 寄存器说明: LUT的查找表寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  lut_r_reg64 : 7;  /* bit[0-6]  : 查找表配置寄存器 */
        unsigned int  reserved_0  : 1;  /* bit[7]    : reserved */
        unsigned int  lut_r_reg65 : 7;  /* bit[8-14] : 查找表配置寄存器 */
        unsigned int  reserved_1  : 1;  /* bit[15]   : reserved */
        unsigned int  lut_r_reg66 : 7;  /* bit[16-22]: 查找表配置寄存器 */
        unsigned int  reserved_2  : 1;  /* bit[23]   : reserved */
        unsigned int  lut_r_reg67 : 7;  /* bit[24-30]: 查找表配置寄存器 */
        unsigned int  reserved_3  : 1;  /* bit[31]   : reserved */
    } reg;
} SOC_ASP_CODEC_LUT_R_REG64_67_UNION;
#endif
#define SOC_ASP_CODEC_LUT_R_REG64_67_lut_r_reg64_START  (0)
#define SOC_ASP_CODEC_LUT_R_REG64_67_lut_r_reg64_END    (6)
#define SOC_ASP_CODEC_LUT_R_REG64_67_lut_r_reg65_START  (8)
#define SOC_ASP_CODEC_LUT_R_REG64_67_lut_r_reg65_END    (14)
#define SOC_ASP_CODEC_LUT_R_REG64_67_lut_r_reg66_START  (16)
#define SOC_ASP_CODEC_LUT_R_REG64_67_lut_r_reg66_END    (22)
#define SOC_ASP_CODEC_LUT_R_REG64_67_lut_r_reg67_START  (24)
#define SOC_ASP_CODEC_LUT_R_REG64_67_lut_r_reg67_END    (30)

/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_ADC_SPI1_DC_OFFSET_UNION
 结构说明  : CODEC_ADC_SPI1_DC_OFFSET 寄存器结构定义。地址偏移量:0x0260，初值:0x00000000，宽度:32
 寄存器说明: ADC_SPAI1通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc_spai1_dc_offset : 24; /* bit[0-23] : adc_spai_hpf dc_offset配置 */
        unsigned int  reserved            : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_ADC_SPI1_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_ADC_SPI1_DC_OFFSET_adc_spai1_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_ADC_SPI1_DC_OFFSET_adc_spai1_dc_offset_END    (23)

/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_ADC_SPI2_DC_OFFSET_UNION
 结构说明  : CODEC_ADC_SPI2_DC_OFFSET 寄存器结构定义。地址偏移量:0x0264，初值:0x00000000，宽度:32
 寄存器说明: ADC_SPAI2通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc_spai2_dc_offset : 24; /* bit[0-23] : adc_spai_hpf dc_offset配置 */
        unsigned int  reserved            : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_ADC_SPI2_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_ADC_SPI2_DC_OFFSET_adc_spai2_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_ADC_SPI2_DC_OFFSET_adc_spai2_dc_offset_END    (23)

/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_ADC_SPA1_DC_OFFSET_UNION
 结构说明  : CODEC_ADC_SPA1_DC_OFFSET 寄存器结构定义。地址偏移量:0x0268，初值:0x00000000，宽度:32
 寄存器说明: ADC_SPAV1通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc_spav1_dc_offset : 24; /* bit[0-23] : adcr_spav_hpf dc_offset配置 */
        unsigned int  reserved            : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_ADC_SPA1_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_ADC_SPA1_DC_OFFSET_adc_spav1_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_ADC_SPA1_DC_OFFSET_adc_spav1_dc_offset_END    (23)

/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CODEC_ADC_SPA2_DC_OFFSET_UNION
 结构说明  : CODEC_ADC_SPA2_DC_OFFSET 寄存器结构定义。地址偏移量:0x026C，初值:0x00000000，宽度:32
 寄存器说明: ADC_SPAV2通路DC_OFFSET配置寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc_spav2_dc_offset : 24; /* bit[0-23] : adcr_spav_hpf dc_offset配置 */
        unsigned int  reserved            : 8;  /* bit[24-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CODEC_ADC_SPA2_DC_OFFSET_UNION;
#endif
#define SOC_ASP_CODEC_CODEC_ADC_SPA2_DC_OFFSET_adc_spav2_dc_offset_START  (0)
#define SOC_ASP_CODEC_CODEC_ADC_SPA2_DC_OFFSET_adc_spav2_dc_offset_END    (23)

/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S1_TDM_CTRL2_UNION
 结构说明  : I2S1_TDM_CTRL2 寄存器结构定义。地址偏移量:0x0270，初值:0x003C0000，宽度:32
 寄存器说明: I2S1_TDM接口控制寄存器3
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0               : 4;  /* bit[0-3]  : reserved */
        unsigned int  s1_tdm_fifo_clr          : 1;  /* bit[4]    : S1路清fifo使能
                                                                    1'b0:不使能
                                                                    1'b1:使能 */
        unsigned int  s1_tdm_mem_icg_bp_en     : 1;  /* bit[5]    : i2s1_mem_icg enable:
                                                                    1'b1：enable;
                                                                    1'b0：disable; */
        unsigned int  s1_tdm_256_frame_mode    : 2;  /* bit[6-7]  : S1 TDM帧格式选择配置寄存器
                                                                    0：pulse左对齐模式；
                                                                    1：pulse shift模式； */
        unsigned int  s1_tdm_256_direct_loop   : 2;  /* bit[8-9]  : S1 TDM接口环回模式选择.
                                                                    2'b00: 正常工作模式；
                                                                    2'b01: Sdin->Sdout.
                                                                    2'b10: RX_DATA -> TX_IN
                                                                    2'b11: Sdout -> Sdin */
        unsigned int  reserved_1               : 8;  /* bit[10-17]: reserved */
        unsigned int  s1_tdm_rx_slot_sel_v3_32 : 1;  /* bit[18]   : S1 TDM IV反馈通路32bit一个slot时，v3的slot选择
                                                                    0: slot16 （[63:32]bit）
                                                                    1: slot17 （[31:0]bit） */
        unsigned int  s1_tdm_rx_slot_sel_v2_32 : 1;  /* bit[19]   : S1 TDM IV反馈通路32bit一个slot时，v2的slot选择
                                                                    0: slot4 （[127:96]bit）
                                                                    1: slot5 （[95:64]bit） */
        unsigned int  s1_tdm_rx_slot_sel_v1_32 : 1;  /* bit[20]   : S1 TDM IV反馈通路32bit一个slot时，v1的slot选择
                                                                    0: slot2 （[191:160]bit）
                                                                    1: slot3 （[159:128]bit） */
        unsigned int  s1_tdm_rx_slot_sel_v0_32 : 1;  /* bit[21]   : S1 TDM IV反馈通路32bit一个slot时，v0的slot选择
                                                                    0: slot0 （[255:224]bit）
                                                                    1: slot1 （[223:192]bit） */
        unsigned int  s1_tdm_rx_slot_sel_i3_32 : 1;  /* bit[22]   : S1 TDM IV反馈通路16bit一个slot时，i3的slot选择
                                                                    0: slot16 （[63:32]bit）
                                                                    1: slot17 （[31:0]bit） */
        unsigned int  s1_tdm_rx_slot_sel_i2_32 : 1;  /* bit[23]   : S1 TDM IV反馈通路32bit一个slot时，i2的slot选择
                                                                    0: slot4 （[127:96]bit）
                                                                    1: slot5 （[95:64]bit） */
        unsigned int  s1_tdm_rx_slot_sel_i1_32 : 1;  /* bit[24]   : S1 TDM IV反馈通路32bit一个slot时，i1的slot选择
                                                                    0: slot2 （[191:160]bit）
                                                                    1: slot3 （[159:128]bit） */
        unsigned int  s1_tdm_rx_slot_sel_i0_32 : 1;  /* bit[25]   : S1 TDM IV反馈通路32bit一个slot时，i0的slot选择
                                                                    0: slot0 （[255:224]bit）
                                                                    1: slot1 （[223:192]bit） */
        unsigned int  i2s1_tdm_slot_sel        : 1;  /* bit[26]   : I2S1_tdm接口反馈数据32bit/16bit选择
                                                                    1'b0: 选择16bit一个slot
                                                                    1'b1: 选择32bit一个slot */
        unsigned int  s1_tdm_256_mst_slv       : 1;  /* bit[27]   : S1 TDM Master/Slave选择
                                                                    1'b0: Master模式
                                                                    1'b1: Slave模式
                                                                    (只能做主) */
        unsigned int  s1_tdm_256_if_en         : 1;  /* bit[28]   : S1 TDM接口使能
                                                                    1'b0:不使能
                                                                    1'b1:使能 */
        unsigned int  reserved_2               : 2;  /* bit[29-30]: reserved */
        unsigned int  i2s1_tdm256_sel          : 1;  /* bit[31]   : TDM128和TDM256选择信号
                                                                    1'b0: 选择新加的TDM256模式
                                                                    1'b1: 选择原来的TDM128模式 */
    } reg;
} SOC_ASP_CODEC_I2S1_TDM_CTRL2_UNION;
#endif
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_fifo_clr_START           (4)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_fifo_clr_END             (4)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_mem_icg_bp_en_START      (5)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_mem_icg_bp_en_END        (5)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_256_frame_mode_START     (6)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_256_frame_mode_END       (7)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_256_direct_loop_START    (8)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_256_direct_loop_END      (9)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_v3_32_START  (18)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_v3_32_END    (18)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_v2_32_START  (19)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_v2_32_END    (19)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_v1_32_START  (20)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_v1_32_END    (20)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_v0_32_START  (21)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_v0_32_END    (21)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_i3_32_START  (22)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_i3_32_END    (22)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_i2_32_START  (23)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_i2_32_END    (23)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_i1_32_START  (24)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_i1_32_END    (24)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_i0_32_START  (25)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_rx_slot_sel_i0_32_END    (25)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_i2s1_tdm_slot_sel_START         (26)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_i2s1_tdm_slot_sel_END           (26)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_256_mst_slv_START        (27)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_256_mst_slv_END          (27)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_256_if_en_START          (28)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_s1_tdm_256_if_en_END            (28)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_i2s1_tdm256_sel_START           (31)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL2_i2s1_tdm256_sel_END             (31)

/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S1_TDM_CTRL3_UNION
 结构说明  : I2S1_TDM_CTRL3 寄存器结构定义。地址偏移量:0x0274，初值:0x0055AAFF，宽度:32
 寄存器说明: I2S1_TDM接口控制寄存器2
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  s1_tdm_rx_slot_sel_vb3_16 : 2;  /* bit[0-1]  : S1 TDM IV反馈通路16bit一个slot时，vb3的slot选择
                                                                     2'b00: slot12 （[63:48]bit）
                                                                     2'b01: slot13 （[47:32]bit）
                                                                     2'b10: slot14 （[31:16]bit）
                                                                     2'b11: slot15 （[15:0]bit） */
        unsigned int  s1_tdm_rx_slot_sel_vb2_16 : 2;  /* bit[2-3]  : S1 TDM IV反馈通路16bit一个slot时，vb2的slot选择
                                                                     2'b00: slot8 （[127:112]bit）
                                                                     2'b01: slot9 （[111:96]bit）
                                                                     2'b10: slot10 （[95:80]bit）
                                                                     2'b11: slot11 （[79:64]bit） */
        unsigned int  s1_tdm_rx_slot_sel_vb1_16 : 2;  /* bit[4-5]  : S1 TDM IV反馈通路16bit一个slot时，vb1的slot选择
                                                                     2'b00: slot4 （[191:176]bit）
                                                                     2'b01: slot5 （[175:160]bit）
                                                                     2'b10: slot6 （[159:144]bit）
                                                                     2'b11: slot7 （[143:128]bit） */
        unsigned int  s1_tdm_rx_slot_sel_vb0_16 : 2;  /* bit[6-7]  : S1 TDM IV反馈通路16bit一个slot时，vb0的slot选择
                                                                     2'b00: slot0 （[255:240]bit）
                                                                     2'b01: slot1 （[239:224]bit）
                                                                     2'b10: slot2 （[223:208]bit）
                                                                     2'b11: slot3 （[207:192]bit） */
        unsigned int  s1_tdm_rx_slot_sel_d3_16  : 2;  /* bit[8-9]  : S1 TDM IV反馈通路16bit一个slot时，d3的slot选择
                                                                     2'b00: slot12 （[63:48]bit）
                                                                     2'b01: slot13 （[47:32]bit）
                                                                     2'b10: slot14 （[31:16]bit）
                                                                     2'b11: slot15 （[15:0]bit） */
        unsigned int  s1_tdm_rx_slot_sel_d2_16  : 2;  /* bit[10-11]: S1 TDM IV反馈通路16bit一个slot时，d2的slot选择
                                                                     2'b00: slot8 （[127:112]bit）
                                                                     2'b01: slot9 （[111:96]bit）
                                                                     2'b10: slot10 （[95:80]bit）
                                                                     2'b11: slot11 （[79:64]bit） */
        unsigned int  s1_tdm_rx_slot_sel_d1_16  : 2;  /* bit[12-13]: S1 TDM IV反馈通路16bit一个slot时，d1的slot选择
                                                                     2'b00: slot4 （[191:176]bit）
                                                                     2'b01: slot5 （[175:160]bit）
                                                                     2'b10: slot6 （[159:144]bit）
                                                                     2'b11: slot7 （[143:128]bit） */
        unsigned int  s1_tdm_rx_slot_sel_d0_16  : 2;  /* bit[14-15]: S1 TDM IV反馈通路16bit一个slot时，d0的slot选择
                                                                     2'b00: slot0 （[255:240]bit）
                                                                     2'b01: slot1 （[239:224]bit）
                                                                     2'b10: slot2 （[223:208]bit）
                                                                     2'b11: slot3 （[207:192]bit） */
        unsigned int  s1_tdm_rx_slot_sel_v3_16  : 2;  /* bit[16-17]: S1 TDM IV反馈通路16bit一个slot时，v3的slot选择
                                                                     2'b00: slot12 （[63:48]bit）
                                                                     2'b01: slot13 （[47:32]bit）
                                                                     2'b10: slot14 （[31:16]bit）
                                                                     2'b11: slot15 （[15:0]bit） */
        unsigned int  s1_tdm_rx_slot_sel_v2_16  : 2;  /* bit[18-19]: S1 TDM IV反馈通路16bit一个slot时，v2的slot选择
                                                                     2'b00: slot8 （[127:112]bit）
                                                                     2'b01: slot9 （[111:96]bit）
                                                                     2'b10: slot10 （[95:80]bit）
                                                                     2'b11: slot11 （[79:64]bit） */
        unsigned int  s1_tdm_rx_slot_sel_v1_16  : 2;  /* bit[20-21]: S1 TDM IV反馈通路16bit一个slot时，v1的slot选择
                                                                     2'b00: slot4 （[191:176]bit）
                                                                     2'b01: slot5 （[175:160]bit）
                                                                     2'b10: slot6 （[159:144]bit）
                                                                     2'b11: slot7 （[143:128]bit） */
        unsigned int  s1_tdm_rx_slot_sel_v0_16  : 2;  /* bit[22-23]: S1 TDM IV反馈通路16bit一个slot时，v0的slot选择
                                                                     2'b00: slot0 （[255:240]bit）
                                                                     2'b01: slot1 （[239:224]bit）
                                                                     2'b10: slot2 （[223:208]bit）
                                                                     2'b11: slot3 （[207:192]bit） */
        unsigned int  s1_tdm_rx_slot_sel_i3_16  : 2;  /* bit[24-25]: S1 TDM IV反馈通路16bit一个slot时，i3的slot选择
                                                                     2'b00: slot12 （[63:48]bit）
                                                                     2'b01: slot13 （[47:32]bit）
                                                                     2'b10: slot14 （[31:16]bit）
                                                                     2'b11: slot15 （[15:0]bit） */
        unsigned int  s1_tdm_rx_slot_sel_i2_16  : 2;  /* bit[26-27]: S1 TDM IV反馈通路16bit一个slot时，i2的slot选择
                                                                     2'b00: slot8 （[127:112]bit）
                                                                     2'b01: slot9 （[111:96]bit）
                                                                     2'b10: slot10 （[95:80]bit）
                                                                     2'b11: slot11 （[79:64]bit） */
        unsigned int  s1_tdm_rx_slot_sel_i1_16  : 2;  /* bit[28-29]: S1 TDM IV反馈通路16bit一个slot时，i1的slot选择
                                                                     2'b00: slot4 （[191:176]bit）
                                                                     2'b01: slot5 （[175:160]bit）
                                                                     2'b10: slot6 （[159:144]bit）
                                                                     2'b11: slot7 （[143:128]bit） */
        unsigned int  s1_tdm_rx_slot_sel_i0_16  : 2;  /* bit[30-31]: S1 TDM IV反馈通路16bit一个slot时，i0的slot选择
                                                                     2'b00: slot0 （[255:240]bit）
                                                                     2'b01: slot1 （[239:224]bit）
                                                                     2'b10: slot2 （[223:208]bit）
                                                                     2'b11: slot3 （[207:192]bit） */
    } reg;
} SOC_ASP_CODEC_I2S1_TDM_CTRL3_UNION;
#endif
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_vb3_16_START  (0)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_vb3_16_END    (1)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_vb2_16_START  (2)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_vb2_16_END    (3)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_vb1_16_START  (4)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_vb1_16_END    (5)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_vb0_16_START  (6)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_vb0_16_END    (7)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_d3_16_START   (8)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_d3_16_END     (9)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_d2_16_START   (10)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_d2_16_END     (11)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_d1_16_START   (12)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_d1_16_END     (13)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_d0_16_START   (14)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_d0_16_END     (15)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_v3_16_START   (16)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_v3_16_END     (17)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_v2_16_START   (18)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_v2_16_END     (19)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_v1_16_START   (20)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_v1_16_END     (21)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_v0_16_START   (22)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_v0_16_END     (23)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_i3_16_START   (24)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_i3_16_END     (25)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_i2_16_START   (26)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_i2_16_END     (27)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_i1_16_START   (28)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_i1_16_END     (29)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_i0_16_START   (30)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL3_s1_tdm_rx_slot_sel_i0_16_END     (31)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_I2S1_TDM_CTRL4_UNION
 结构说明  : I2S1_TDM_CTRL4 寄存器结构定义。地址偏移量:0x0278，初值:0x00000000，宽度:32
 寄存器说明: I2S1_TDM接口控制寄存器4
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved_0          : 17; /* bit[0-16] : reserved */
        unsigned int  s1_tdm_256_err_stat : 5;  /* bit[17-21]: 保留 */
        unsigned int  reserved_1          : 10; /* bit[22-31]: reserved */
    } reg;
} SOC_ASP_CODEC_I2S1_TDM_CTRL4_UNION;
#endif
#define SOC_ASP_CODEC_I2S1_TDM_CTRL4_s1_tdm_256_err_stat_START  (17)
#define SOC_ASP_CODEC_I2S1_TDM_CTRL4_s1_tdm_256_err_stat_END    (21)


/*****************************************************************************
 结构名    : SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL6_UNION
 结构说明  : PGA_GAINOFFSET_CTRL6 寄存器结构定义。地址偏移量:0x027C，初值:0x00000000，宽度:32
 寄存器说明: PGA GAINOFFSET配置寄存器6
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  ultra_dn_l_pga_gainoffset : 8;  /* bit[0-7]  : 增益偏置，gainoffset有效配置范围是0~255
                                                                     gain_offset的调整粒度与fadeInTime相关，
                                                                     映射关系为粒度= max(2^(fadeInTime -12),1)，
                                                                     以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  ultra_dn_r_pga_gainoffset : 8;  /* bit[8-15] : 增益偏置，gainoffset有效配置范围是0~255
                                                                     gain_offset的调整粒度与fadeInTime相关，
                                                                     映射关系为粒度= max(2^(fadeInTime -12),1)，
                                                                     以fadeInTime=16为例，gainOffset可取0,16,32……; */
        unsigned int  reserved                  : 16; /* bit[16-31]: reserved */
    } reg;
} SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL6_UNION;
#endif
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL6_ultra_dn_l_pga_gainoffset_START  (0)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL6_ultra_dn_l_pga_gainoffset_END    (7)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL6_ultra_dn_r_pga_gainoffset_START  (8)
#define SOC_ASP_CODEC_PGA_GAINOFFSET_CTRL6_ultra_dn_r_pga_gainoffset_END    (15)

/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_UNION
 结构说明  : ULTRA_L_DN_PGA_CTRL 寄存器结构定义。地址偏移量:0x0280，初值:0x00000050，宽度:32
 寄存器说明: ULTRA_L_DN PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                  : 1;  /* bit[0]    : reserved */
        unsigned int  ultra_l_dn_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                     1'b0: 选择对数淡出淡出
                                                                     1'b1: 选择线性淡入淡出 */
        unsigned int  ultra_l_dn_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  ultra_l_dn_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga通过使用
                                                                     其中2bit的id选择四种场景的阈值
                                                                     0：选择pga_thre_ctrl[23:0]
                                                                     1：选择pga_thre_ctrl[47:24]
                                                                     2：选择pga_thre_ctrl[71:48]
                                                                     3：选择pga_thre_ctrl[95:72] */
        unsigned int  ultra_l_dn_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值
                                                                     的小信号放过去，低表示低于阈值的信号钳制到0
                                                                     1：不对噪音信号做任何处理
                                                                     0：将低于阈值的噪音信号过滤成0 */
        unsigned int  ultra_l_dn_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                     1'b0: 不bypass
                                                                     1'b1: bypass */
        unsigned int  ultra_l_dn_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  ultra_l_dn_pga_fade_in    : 5;  /* bit[16-20]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  ultra_l_dn_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                     [7]：淡入淡出功能使能
                                                                     1：淡入淡出功能使能
                                                                     0：淡入淡出功能不使能
                                                                     [6]: 抗削波功能使能
                                                                     1：抗削波功能使能
                                                                     0：抗削波功能不使能
                                                                     [5]: 小信号功能使能
                                                                     1：小信号功能使能
                                                                     0：小信号功能不使能 */
        unsigned int  ultra_l_dn_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                     例如：60dB配置成0x3C；
                                                                      0dB配置为0x00；
                                                                      -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_bypass_START      (10)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_bypass_END        (10)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_cfg_START         (21)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_cfg_END           (23)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_gain_START        (24)
#define SOC_ASP_CODEC_ULTRA_L_DN_PGA_CTRL_ultra_l_dn_pga_gain_END          (31)

/*****************************************************************************
 结构名    : SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_UNION
 结构说明  : ULTRA_R_DN_PGA_CTRL 寄存器结构定义。地址偏移量:0x0284，初值:0x00000050，宽度:32
 寄存器说明: ULTRA_R_DN PGA控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved                  : 1;  /* bit[0]    : reserved */
        unsigned int  ultra_r_dn_pga_linear_sel : 1;  /* bit[1]    : PGA线性选择控制
                                                                     1'b0: 选择对数淡出淡出
                                                                     1'b1: 选择线性淡入淡出 */
        unsigned int  ultra_r_dn_pga_zero_num   : 5;  /* bit[2-6]  : PGA过零检测数目，默认值为5'd20 */
        unsigned int  ultra_r_dn_pga_thre_id    : 2;  /* bit[7-8]  : PGA阈值选择信号，用于过零检测。其中每个pga
                                                                     通过使用其中2bit的id选择四种场景的阈值
                                                                     0：选择pga_thre_ctrl[23:0]
                                                                     1：选择pga_thre_ctrl[47:24]
                                                                     2：选择pga_thre_ctrl[71:48]
                                                                     3：选择pga_thre_ctrl[95:72] */
        unsigned int  ultra_r_dn_pga_noise_en   : 1;  /* bit[9]    : PGA噪音使能，高电平代表将噪音使能，即将抵于阈值的
                                                                     小信号放过去，低表示低于阈值的信号钳制到0
                                                                     1：不对噪音信号做任何处理
                                                                     0：将低于阈值的噪音信号过滤成0 */
        unsigned int  ultra_r_dn_pga_bypass     : 1;  /* bit[10]   : PGA bypass使能开关
                                                                     1'b0: 不bypass
                                                                     1'b1: bypass */
        unsigned int  ultra_r_dn_pga_fade_out   : 5;  /* bit[11-15]: PGA淡出时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  ultra_r_dn_pga_fade_in    : 5;  /* bit[16-20]: PGA淡入时间等级设置，实际淡入时间与等级设置、
                                                                     采样率相关，详细对应关系请参考用户手册。 */
        unsigned int  ultra_r_dn_pga_cfg        : 3;  /* bit[21-23]: PGA三种功能使能开关，淡入淡出，抗削波，小信号
                                                                     [7]：淡入淡出功能使能
                                                                     1：淡入淡出功能使能
                                                                     0：淡入淡出功能不使能
                                                                     [6]: 抗削波功能使能
                                                                     1：抗削波功能使能
                                                                     0：抗削波功能不使能
                                                                     [5]: 小信号功能使能
                                                                     1：小信号功能使能
                                                                     0：小信号功能不使能 */
        unsigned int  ultra_r_dn_pga_gain       : 8;  /* bit[24-31]: PGA增益配置，配置范围: -120~60dB（bit7为符号位）
                                                                     例如：60dB配置成0x3C；
                                                                      0dB配置为0x00；
                                                                      -120dB配置为0x88； */
    } reg;
} SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_linear_sel_START  (1)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_linear_sel_END    (1)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_zero_num_START    (2)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_zero_num_END      (6)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_thre_id_START     (7)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_thre_id_END       (8)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_noise_en_START    (9)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_noise_en_END      (9)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_bypass_START      (10)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_bypass_END        (10)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_fade_out_START    (11)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_fade_out_END      (15)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_fade_in_START     (16)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_fade_in_END       (20)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_cfg_START         (21)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_cfg_END           (23)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_gain_START        (24)
#define SOC_ASP_CODEC_ULTRA_R_DN_PGA_CTRL_ultra_r_dn_pga_gain_END          (31)

/*****************************************************************************
 结构名    : SOC_ASP_CODEC_DAC_SRCUP_CTRL_UNION
 结构说明  : DAC_SRCUP_CTRL 寄存器结构定义。地址偏移量:0x0288，初值:0x00000000，宽度:32
 寄存器说明: SRCUP控制寄存器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  reserved            : 16; /* bit[0-15] : reserved */
        unsigned int  sif2_s2p_loop       : 2;  /* bit[16-17]: sif的环回配置：
                                                               2'b00：正常工作；
                                                               2'b01：下行并行的输入环回到并行的输出0~4；
                                                               2'b10：并行的输入环回到并行的输出，5~9；
                                                               其它：保留 */
        unsigned int  sif2_p2s_loop       : 2;  /* bit[18-19]: sif的环回配置：
                                                               2'b00：正常工作；
                                                               2'b01：上行并行的输出环回到下行并行的输入0~4；
                                                               2'b10：上行并行的输出环回到下行并行的输入5~9；
                                                               其它：保留 */
        unsigned int  sif1_s2p_loop       : 2;  /* bit[20-21]: sif的环回配置：
                                                               2'b00：正常工作；
                                                               2'b01：下行并行的输入环回到并行的输出0~4；
                                                               2'b10：并行的输入环回到并行的输出，5~9；
                                                               其它：保留 */
        unsigned int  sif1_p2s_loop       : 2;  /* bit[22-23]: sif的环回配置：
                                                               2'b00：正常工作；
                                                               2'b01：上行并行的输出环回到下行并行的输入0~4；
                                                               2'b10：上行并行的输出环回到下行并行的输入5~9；
                                                               其它：保留 */
        unsigned int  dacl_srcup_src_mode : 3;  /* bit[24-26]: SRCUP升采样倍数配置:
                                                               3'b111: 12倍升采样；
                                                               3'b101: 6倍升采样；
                                                               3'b100: 2倍升采样；
                                                               3'b001: 3倍升采样;
                                                               3'b010: 保留.
                                                               others: bypass. */
        unsigned int  dacl_srcup_fifo_clr : 1;  /* bit[27]   : FIFO Clear Signal, high active.
                                                               0：不清空FIFO数据
                                                               1：清空FIFO数据 */
        unsigned int  dacr_srcup_src_mode : 3;  /* bit[28-30]: SRCUP升采样倍数配置:
                                                               3'b111: 保留;
                                                               3'b101: 6倍升采样；
                                                               3'b100: 2倍升采样；
                                                               3'b001: 3倍升采样;
                                                               3'b010: 1.5倍升采样.
                                                               others: bypass. */
        unsigned int  dacr_srcup_fifo_clr : 1;  /* bit[31]   : FIFO Clear Signal, high active.
                                                               0：不清空FIFO数据
                                                               1：清空FIFO数据 */
    } reg;
} SOC_ASP_CODEC_DAC_SRCUP_CTRL_UNION;
#endif
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_sif2_s2p_loop_START        (16)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_sif2_s2p_loop_END          (17)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_sif2_p2s_loop_START        (18)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_sif2_p2s_loop_END          (19)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_sif1_s2p_loop_START        (20)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_sif1_s2p_loop_END          (21)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_sif1_p2s_loop_START        (22)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_sif1_p2s_loop_END          (23)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_dacl_srcup_src_mode_START  (24)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_dacl_srcup_src_mode_END    (26)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_dacl_srcup_fifo_clr_START  (27)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_dacl_srcup_fifo_clr_END    (27)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_dacr_srcup_src_mode_START  (28)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_dacr_srcup_src_mode_END    (30)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_dacr_srcup_fifo_clr_START  (31)
#define SOC_ASP_CODEC_DAC_SRCUP_CTRL_dacr_srcup_fifo_clr_END    (31)

/*****************************************************************************
 结构名    : SOC_ASP_CODEC_CIC_6STAGE_SEL_UNION
 结构说明  : CIC_6STAGE_SEL 寄存器结构定义。地址偏移量:0x028C，初值:0x00000000，宽度:32
 寄存器说明: 6阶cic选择滤波器
*****************************************************************************/
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int      value;
    struct {
        unsigned int  adc1_cic_6sel : 1;  /* bit[0]   : 6阶cic滤波器的选择：
                                                        1'b0：选择6阶cic滤波器；
                                                        1'b1：选择4阶cic滤波器； */
        unsigned int  adc2_cic_6sel : 1;  /* bit[1]   : 保留 */
        unsigned int  adc3_cic_6sel : 1;  /* bit[2]   : 保留 */
        unsigned int  adc4_cic_6sel : 1;  /* bit[3]   : 保留 */
        unsigned int  adc5_cic_6sel : 1;  /* bit[4]   : 保留 */
        unsigned int  adc6_cic_6sel : 1;  /* bit[5]   : 保留 */
        unsigned int  adc7_cic_6sel : 1;  /* bit[6]   : 保留 */
        unsigned int  adc8_cic_6sel : 1;  /* bit[7]   : 保留 */
        unsigned int  reserved      : 24; /* bit[8-31]: reserved */
    } reg;
} SOC_ASP_CODEC_CIC_6STAGE_SEL_UNION;
#endif
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc1_cic_6sel_START  (0)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc1_cic_6sel_END    (0)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc2_cic_6sel_START  (1)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc2_cic_6sel_END    (1)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc3_cic_6sel_START  (2)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc3_cic_6sel_END    (2)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc4_cic_6sel_START  (3)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc4_cic_6sel_END    (3)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc5_cic_6sel_START  (4)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc5_cic_6sel_END    (4)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc6_cic_6sel_START  (5)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc6_cic_6sel_END    (5)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc7_cic_6sel_START  (6)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc7_cic_6sel_END    (6)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc8_cic_6sel_START  (7)
#define SOC_ASP_CODEC_CIC_6STAGE_SEL_adc8_cic_6sel_END    (7)


#define SOC_ASP_CODEC_ANAI2S_CTRL_intf1_i2s_codec_io_wordlength_START (13)
#define SOC_ASP_CODEC_ANAI2S_CTRL_intf1_i2s_codec_io_wordlength_END (14)

#define SOC_ASP_CODEC_HDMI0_CTRL_intf2_i2s_codec_io_wordlength_START (13)
#define SOC_ASP_CODEC_HDMI0_CTRL_intf2_i2s_codec_io_wordlength_END (14)

#define SOC_ASP_CODEC_HDMI1_CTRL_intf3_i2s_codec_io_wordlength_START (13)
#define SOC_ASP_CODEC_HDMI1_CTRL_intf3_i2s_codec_io_wordlength_END (14)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif /* end of soc_asp_codec_interface.h */
