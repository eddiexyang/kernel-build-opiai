/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_pa_offset
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_PA_OFFSET_H__
#define __REG_PA_OFFSET_H__

/* PPE_PA Base address of Module's Register */
#define PPE_PA_BASE                       (0x0)

/******************************************************************************/
/*                      xxx PPE_PA Registers' Definitions                     */
/******************************************************************************/

#define PPE_PA_PA_ET_IPV4_ENABLE_REG          (PPE_PA_BASE + 0x100)  /* 软件指定IPV4报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_IPV4_REG                 (PPE_PA_BASE + 0x104)  /* 软件指定IPV4报文判断值的配置寄存器。 */
#define PPE_PA_PA_ET_IPV6_ENABLE_REG          (PPE_PA_BASE + 0x108)  /* 软件指定IPV6报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_IPV6_REG                 (PPE_PA_BASE + 0x10C)  /* 软件指定IPV6报文判断值的配置寄存器。 */
#define PPE_PA_PA_ET_ARP_ENABLE_REG           (PPE_PA_BASE + 0x110)  /* 软件指定自定义ARP头报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_ARP_REG                  (PPE_PA_BASE + 0x114)  /* 软件指定自定义ARP头报文判断值的配置寄存器。 */
#define PPE_PA_PA_ET_RARP_ENABLE_REG          (PPE_PA_BASE + 0x118)  /* 软件指定RARP报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_RARP_REG                 (PPE_PA_BASE + 0x11C)  /* 软件指定RARP报文判断值的配置寄存器。 */
#define PPE_PA_PA_ET_DEF4_ENABLE_REG          (PPE_PA_BASE + 0x120)  /* 软件指定自定义报文4解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_DEF4_REG                 (PPE_PA_BASE + 0x124)  /* 软件指定自定义报文4判断值的配置寄存器。 */
#define PPE_PA_PA_ET_DEF5_ENABLE_REG          (PPE_PA_BASE + 0x128)  /* 软件指定自定义报文5解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_DEF5_REG                 (PPE_PA_BASE + 0x12C)  /* 软件指定自定义报文5判断值的配置寄存器。 */
#define PPE_PA_PA_ET_LLDP_ENABLE_REG          (PPE_PA_BASE + 0x130)  /* 软件指定LLDP报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_LLDP_REG                 (PPE_PA_BASE + 0x134)  /* 软件指定LLDP报文判断值的配置寄存器。 */
#define PPE_PA_PA_ET_BPDU_ENABLE_REG          (PPE_PA_BASE + 0x138)  /* 软件指定BPDU报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_MAC_PAUSE_ENABLE_REG     (PPE_PA_BASE + 0x13C)  /* 软件指定mac pause报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_MAC_PAUSE_REG            (PPE_PA_BASE + 0x140)  /* 软件指定MAC PAUSE报文判断值的配置寄存器。 */
#define PPE_PA_PA_ET_PFC_PAUSE_ENABLE_REG     (PPE_PA_BASE + 0x144)  /* 软件指定Q922解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_PFC_PAUSE_REG            (PPE_PA_BASE + 0x148)  /* 软件指定Q922判断值的配置寄存器。 */
#define PPE_PA_PA_ET_MAC_1588_ENABLE_REG      (PPE_PA_BASE + 0x14C)  /* 软件指定MAC 1588报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_MAC_1588_REG             (PPE_PA_BASE + 0x150)  /* 软件指定MAC 1588报文判断值的配置寄存器。 */
#define PPE_PA_PA_ET_8021X_ENABLE_REG         (PPE_PA_BASE + 0x154)  /* 软件指定8021x报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_8021X_REG                (PPE_PA_BASE + 0x158)  /* 软件指定8021x报文判断值的配置寄存器。 */
#define PPE_PA_PA_ET_CNM_ENABLE_REG           (PPE_PA_BASE + 0x15C)  /* CNM报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_CNM_REG                  (PPE_PA_BASE + 0x160)  /* CNM报文判断值的配置寄存器。 */
#define PPE_PA_PA_ET_Y1731_ENABLE_REG         (PPE_PA_BASE + 0x164)  /* 软件指定Y1731报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_Y1731_REG                (PPE_PA_BASE + 0x168)  /* 软件指定Y1731报文判断值的配置寄存器。 */
#define PPE_PA_PA_ET_8023AH_ENABLE_REG        (PPE_PA_BASE + 0x16C)  /* 软件指定8023ah报文14解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_ET_8023AH_REG               (PPE_PA_BASE + 0x170)  /* 软件指定8023ah报文14判断值的配置寄存器。 */
#define PPE_PA_PA_IP_UDP_ENABLE_REG           (PPE_PA_BASE + 0x200)  /* 软件指定UDP报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IP_UDP_REG                  (PPE_PA_BASE + 0x204)  /* 软件指定UDP报文判断值的配置寄存器。 */
#define PPE_PA_PA_IP_TCP_ENABLE_REG           (PPE_PA_BASE + 0x208)  /* 软件指定TCP报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IP_TCP_REG                  (PPE_PA_BASE + 0x20C)  /* 软件指定TCP报文判断值的配置寄存器。 */
#define PPE_PA_PA_IP_GRE_ENABLE_REG           (PPE_PA_BASE + 0x210)  /* 软件指定GRE报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IP_GRE_REG                  (PPE_PA_BASE + 0x214)  /* 软件指定GRE报文判断值的配置寄存器。 */
#define PPE_PA_PA_IP_SCTP_ENABLE_REG          (PPE_PA_BASE + 0x218)  /* 软件指定SCTP报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IP_SCTP_REG                 (PPE_PA_BASE + 0x21C)  /* 软件指定SCTP报文判断值的配置寄存器。 */
#define PPE_PA_PA_IP_IGMP_ENABLE_REG          (PPE_PA_BASE + 0x220)  /* 软件指定IGMP报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IP_IGMP_REG                 (PPE_PA_BASE + 0x224)  /* 软件指定IGMP报文判断值的配置寄存器。 */
#define PPE_PA_PA_IPV4_ICMP_ENABLE_REG        (PPE_PA_BASE + 0x228)  /* 软件指定IPV4 ICMP报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IPV4_ICMP_REG               (PPE_PA_BASE + 0x22C)  /* 软件指定IPV4 ICMP报文判断值的配置寄存器。 */
#define PPE_PA_PA_IP_ESP_ENABLE_REG           (PPE_PA_BASE + 0x230)  /* 软件指定ESP报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IP_ESP_REG                  (PPE_PA_BASE + 0x234)  /* 软件指定ESP报文判断值的配置寄存器。 */
#define PPE_PA_PA_IP_AH_ENABLE_REG            (PPE_PA_BASE + 0x238)  /* 软件指定AH报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IP_AH_REG                   (PPE_PA_BASE + 0x23C)  /* 软件指定AH报文判断值的配置寄存器。 */
#define PPE_PA_PA_IPV6_ICMP_ENABLE_REG        (PPE_PA_BASE + 0x240)  /* 软件指定IPV6 ICMP报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IPV6_ICMP_REG               (PPE_PA_BASE + 0x244)  /* 软件指定IPV6 ICMP报文判断值的配置寄存器。 */
#define PPE_PA_PA_IP_DEF9_ENABLE_REG          (PPE_PA_BASE + 0x248)  /* 软件指定自定义报文9解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IP_DEF9_REG                 (PPE_PA_BASE + 0x24C)  /* 软件指定自定义报文9判断值的配置寄存器。 */
#define PPE_PA_PA_IP_DEF10_ENABLE_REG         (PPE_PA_BASE + 0x250)  /* 软件指定自定义报文10解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IP_DEF10_REG                (PPE_PA_BASE + 0x254)  /* 软件指定自定义报文10判断值的配置寄存器。 */
#define PPE_PA_PA_IP_DEF11_ENABLE_REG         (PPE_PA_BASE + 0x258)  /* 软件指定自定义报文11解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IP_DEF11_REG                (PPE_PA_BASE + 0x25C)  /* 软件指定自定义报文11判断值的配置寄存器。 */
#define PPE_PA_PA_IP_DEF12_ENABLE_REG         (PPE_PA_BASE + 0x260)  /* 软件指定自定义报文12解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IP_DEF12_REG                (PPE_PA_BASE + 0x264)  /* 软件指定自定义报文12判断值的配置寄存器。 */
#define PPE_PA_PA_IP_DEF13_ENABLE_REG         (PPE_PA_BASE + 0x268)  /* 软件指定自定义报文12解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IP_DEF13_REG                (PPE_PA_BASE + 0x26C)  /* 软件指定自定义报文13判断值的配置寄存器。 */
#define PPE_PA_PA_IP_DEF14_ENABLE_REG         (PPE_PA_BASE + 0x270)  /* 软件指定自定义报文14解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_IP_DEF14_REG                (PPE_PA_BASE + 0x274)  /* 软件指定自定义报文14判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_DHBT_ENABLE_REG       (PPE_PA_BASE + 0x300)  /* 软件指定DHCP/BOOTP报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_DHBT_REG              (PPE_PA_BASE + 0x304)  /* 软件指定DHCP/BOOTP报文判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_IPPM_ENABLE_REG       (PPE_PA_BASE + 0x308)  /* 软件指定IPPM报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_IPPM_REG              (PPE_PA_BASE + 0x30C)  /* 软件指定IPPM报文判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_UDP_1588_ENABLE_REG   (PPE_PA_BASE + 0x310)  /* 软件指定UDP封装的1588报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_UDP_1588_REG          (PPE_PA_BASE + 0x314)  /* 软件指定UDP封装的1588报文判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_GTPU_ENABLE_REG       (PPE_PA_BASE + 0x318)  /* 软件指定GTPU报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_GTPU_REG              (PPE_PA_BASE + 0x31C)  /* 软件指定GTPU报文判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_HTTP_ENABLE_REG       (PPE_PA_BASE + 0x320)  /* 软件指定HTTP报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_HTTP_REG              (PPE_PA_BASE + 0x324)  /* 软件指定HTTP报文判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_HTTPS_ENABLE_REG      (PPE_PA_BASE + 0x328)  /* 软件指定HTTPS报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_HTTPS_REG             (PPE_PA_BASE + 0x32C)  /* 软件指定HTTPS报文判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_FTP_ENABLE_REG        (PPE_PA_BASE + 0x330)  /* 软件指定FTP报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_FTP_REG               (PPE_PA_BASE + 0x334)  /* 软件指定BFD报文判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_BFD_ENABLE_REG        (PPE_PA_BASE + 0x338)  /* 软件指定BFD报文解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_BFD_REG               (PPE_PA_BASE + 0x33C)  /* 软件指定BFD报文判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_ROCE_ENABLE_REG       (PPE_PA_BASE + 0x340)  /* 软件指定ROCE报文报文8解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_ROCE_REG              (PPE_PA_BASE + 0x344)  /* 软件指定ROCE报文报文8判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_DEF9_ENABLE_REG       (PPE_PA_BASE + 0x348)  /* 软件指定自定义报文9解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_DEF9_REG              (PPE_PA_BASE + 0x34C)  /* 软件指定自定义报文9判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_DEF10_ENABLE_REG      (PPE_PA_BASE + 0x350)  /* 软件指定自定义报文10解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_DEF10_REG             (PPE_PA_BASE + 0x354)  /* 软件指定自定义报文10判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_DEF11_ENABLE_REG      (PPE_PA_BASE + 0x358)  /* 软件指定自定义报文11解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_DEF11_REG             (PPE_PA_BASE + 0x35C)  /* 软件指定自定义报文11判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_DEF12_ENABLE_REG      (PPE_PA_BASE + 0x360)  /* 软件指定自定义报文12解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_DEF12_REG             (PPE_PA_BASE + 0x364)  /* 软件指定自定义报文12判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_DEF13_ENABLE_REG      (PPE_PA_BASE + 0x368)  /* 软件指定自定义报文13解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_DEF13_REG             (PPE_PA_BASE + 0x36C)  /* 软件指定自定义报文13判断值的配置寄存器。 */
#define PPE_PA_PA_DPORT_DEF14_ENABLE_REG      (PPE_PA_BASE + 0x370)  /* 软件指定自定义报文14解析使能及解析模板的配置寄存器。 */
#define PPE_PA_PA_DPORT_DEF14_REG             (PPE_PA_BASE + 0x374)  /* 软件指定自定义报文14判断值的配置寄存器。 */
#define PPE_PA_PA_ET_BPDU_0_REG               (PPE_PA_BASE + 0x440)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_1_REG               (PPE_PA_BASE + 0x444)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_2_REG               (PPE_PA_BASE + 0x448)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_3_REG               (PPE_PA_BASE + 0x44C)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_4_REG               (PPE_PA_BASE + 0x450)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_5_REG               (PPE_PA_BASE + 0x454)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_6_REG               (PPE_PA_BASE + 0x458)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_7_REG               (PPE_PA_BASE + 0x45C)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_8_REG               (PPE_PA_BASE + 0x460)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_9_REG               (PPE_PA_BASE + 0x464)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_10_REG              (PPE_PA_BASE + 0x468)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_11_REG              (PPE_PA_BASE + 0x46C)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_12_REG              (PPE_PA_BASE + 0x470)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_13_REG              (PPE_PA_BASE + 0x474)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_14_REG              (PPE_PA_BASE + 0x478)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_ET_BPDU_15_REG              (PPE_PA_BASE + 0x47C)  /* 软件指定自定义报文6判断值的配置寄存器。(BPDU) */
#define PPE_PA_PA_BPDU_ADDR_MATCH_0_REG       (PPE_PA_BASE + 0x480)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_1_REG       (PPE_PA_BASE + 0x484)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_2_REG       (PPE_PA_BASE + 0x488)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_3_REG       (PPE_PA_BASE + 0x48C)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_4_REG       (PPE_PA_BASE + 0x490)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_5_REG       (PPE_PA_BASE + 0x494)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_6_REG       (PPE_PA_BASE + 0x498)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_7_REG       (PPE_PA_BASE + 0x49C)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_8_REG       (PPE_PA_BASE + 0x4A0)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_9_REG       (PPE_PA_BASE + 0x4A4)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_10_REG      (PPE_PA_BASE + 0x4A8)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_11_REG      (PPE_PA_BASE + 0x4AC)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_12_REG      (PPE_PA_BASE + 0x4B0)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_13_REG      (PPE_PA_BASE + 0x4B4)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_14_REG      (PPE_PA_BASE + 0x4B8)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_15_REG      (PPE_PA_BASE + 0x4BC)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_0_REG  (PPE_PA_BASE + 0x4C0)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_1_REG  (PPE_PA_BASE + 0x4C4)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_2_REG  (PPE_PA_BASE + 0x4C8)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_3_REG  (PPE_PA_BASE + 0x4CC)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_4_REG  (PPE_PA_BASE + 0x4D0)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_5_REG  (PPE_PA_BASE + 0x4D4)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_6_REG  (PPE_PA_BASE + 0x4D8)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_7_REG  (PPE_PA_BASE + 0x4DC)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_8_REG  (PPE_PA_BASE + 0x4E0)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_9_REG  (PPE_PA_BASE + 0x4E4)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_10_REG (PPE_PA_BASE + 0x4E8)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_11_REG (PPE_PA_BASE + 0x4EC)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_12_REG (PPE_PA_BASE + 0x4F0)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_13_REG (PPE_PA_BASE + 0x4F4)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_14_REG (PPE_PA_BASE + 0x4F8)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_BPDU_ADDR_MATCH_MASK_15_REG (PPE_PA_BASE + 0x4FC)  /* 用于解析BPDU报文的与DMAC低24bit匹配的地址匹配掩码配置寄存器 */
#define PPE_PA_PA_L2_FLEX_OFFSET_0_REG        (PPE_PA_BASE + 0x630)  /* PA L2 Flexible Offset配置寄存器 */
#define PPE_PA_PA_L2_FLEX_OFFSET_1_REG        (PPE_PA_BASE + 0x640)  /* PA L2 Flexible Offset配置寄存器 */
#define PPE_PA_PA_L2_FLEX_OFFSET_2_REG        (PPE_PA_BASE + 0x650)  /* PA L2 Flexible Offset配置寄存器 */
#define PPE_PA_PA_L2_FLEX_OFFSET_3_REG        (PPE_PA_BASE + 0x660)  /* PA L2 Flexible Offset配置寄存器 */
#define PPE_PA_PA_L3_FLEX_OFFSET_0_REG        (PPE_PA_BASE + 0x634)  /* PA L3 Flexible Offset配置寄存器 */
#define PPE_PA_PA_L3_FLEX_OFFSET_1_REG        (PPE_PA_BASE + 0x644)  /* PA L3 Flexible Offset配置寄存器 */
#define PPE_PA_PA_L3_FLEX_OFFSET_2_REG        (PPE_PA_BASE + 0x654)  /* PA L3 Flexible Offset配置寄存器 */
#define PPE_PA_PA_L3_FLEX_OFFSET_3_REG        (PPE_PA_BASE + 0x664)  /* PA L3 Flexible Offset配置寄存器 */
#define PPE_PA_PA_L4_FLEX_OFFSET_0_REG        (PPE_PA_BASE + 0x638)  /* PA L4 Flexible Offset配置寄存器 */
#define PPE_PA_PA_L4_FLEX_OFFSET_1_REG        (PPE_PA_BASE + 0x648)  /* PA L4 Flexible Offset配置寄存器 */
#define PPE_PA_PA_L4_FLEX_OFFSET_2_REG        (PPE_PA_BASE + 0x658)  /* PA L4 Flexible Offset配置寄存器 */
#define PPE_PA_PA_L4_FLEX_OFFSET_3_REG        (PPE_PA_BASE + 0x668)  /* PA L4 Flexible Offset配置寄存器 */
#define PPE_PA_PA_UDF_DEF_VALUE_0_REG         (PPE_PA_BASE + 0x63C)  /* PA L2/L3/L4 自定义报文提取默认值配置寄存器 */
#define PPE_PA_PA_UDF_DEF_VALUE_1_REG         (PPE_PA_BASE + 0x64C)  /* PA L2/L3/L4 自定义报文提取默认值配置寄存器 */
#define PPE_PA_PA_UDF_DEF_VALUE_2_REG         (PPE_PA_BASE + 0x65C)  /* PA L2/L3/L4 自定义报文提取默认值配置寄存器 */
#define PPE_PA_PA_UDF_DEF_VALUE_3_REG         (PPE_PA_BASE + 0x66C)  /* PA L2/L3/L4 自定义报文提取默认值配置寄存器 */
#define PPE_PA_PA_ANALY_CONFIG_0_REG          (PPE_PA_BASE + 0x3500) /* PA解析配置寄存器 */
#define PPE_PA_PA_ANALY_CONFIG_1_REG          (PPE_PA_BASE + 0x3504) /* PA解析配置寄存器 */
#define PPE_PA_PA_ANALY_CONFIG_2_REG          (PPE_PA_BASE + 0x3508) /* PA解析配置寄存器 */
#define PPE_PA_PA_ANALY_CONFIG_3_REG          (PPE_PA_BASE + 0x350C) /* PA解析配置寄存器 */
#define PPE_PA_PA_PORT_TYPE_0_REG             (PPE_PA_BASE + 0x3600) /* 端口类型配置寄存器 */
#define PPE_PA_PA_PORT_TYPE_1_REG             (PPE_PA_BASE + 0x3604) /* 端口类型配置寄存器 */
#define PPE_PA_PA_PORT_TYPE_2_REG             (PPE_PA_BASE + 0x3608) /* 端口类型配置寄存器 */
#define PPE_PA_PA_PORT_TYPE_3_REG             (PPE_PA_BASE + 0x360C) /* 端口类型配置寄存器 */
#define PPE_PA_PA_PRO_LEN_0_REG               (PPE_PA_BASE + 0x3700) /* PA PRO_LEN配置寄存器 */
#define PPE_PA_PA_PRO_LEN_1_REG               (PPE_PA_BASE + 0x3704) /* PA PRO_LEN配置寄存器 */
#define PPE_PA_PA_PRO_LEN_2_REG               (PPE_PA_BASE + 0x3708) /* PA PRO_LEN配置寄存器 */
#define PPE_PA_PA_PRO_LEN_3_REG               (PPE_PA_BASE + 0x370C) /* PA PRO_LEN配置寄存器 */
#define PPE_PA_PA_XSNAP_ENABLE_0_REG          (PPE_PA_BASE + 0x3800) /* PA SNAP报文DSAP/SANP匹配使能寄存器 */
#define PPE_PA_PA_XSNAP_ENABLE_1_REG          (PPE_PA_BASE + 0x3804) /* PA SNAP报文DSAP/SANP匹配使能寄存器 */
#define PPE_PA_PA_XSNAP_ENABLE_2_REG          (PPE_PA_BASE + 0x3808) /* PA SNAP报文DSAP/SANP匹配使能寄存器 */
#define PPE_PA_PA_XSNAP_ENABLE_3_REG          (PPE_PA_BASE + 0x380C) /* PA SNAP报文DSAP/SANP匹配使能寄存器 */
#define PPE_PA_PA_CRC_CHK_ENABLE_0_REG        (PPE_PA_BASE + 0x3B00) /* PA CRC check使能配置寄存器 */
#define PPE_PA_PA_CRC_CHK_ENABLE_1_REG        (PPE_PA_BASE + 0x3B04) /* PA CRC check使能配置寄存器 */
#define PPE_PA_PA_CRC_CHK_ENABLE_2_REG        (PPE_PA_BASE + 0x3B08) /* PA CRC check使能配置寄存器 */
#define PPE_PA_PA_CRC_CHK_ENABLE_3_REG        (PPE_PA_BASE + 0x3B0C) /* PA CRC check使能配置寄存器 */
#define PPE_PA_PA_PTP_SYN_CTRL_0_REG          (PPE_PA_BASE + 0x3C00) /* 1588控制寄存器 */
#define PPE_PA_PA_PTP_SYN_CTRL_1_REG          (PPE_PA_BASE + 0x3C04) /* 1588控制寄存器 */
#define PPE_PA_PA_PTP_SYN_CTRL_2_REG          (PPE_PA_BASE + 0x3C08) /* 1588控制寄存器 */
#define PPE_PA_PA_PTP_SYN_CTRL_3_REG          (PPE_PA_BASE + 0x3C0C) /* 1588控制寄存器 */
#define PPE_PA_PA_ET_CN_TAG_REG               (PPE_PA_BASE + 0x3E04) /* PA_ET_CFG_CN_TAG为软件指定CN_TAG扩展域判断标志的配置寄存器。 */
#define PPE_PA_PA_CNT_CLR_CE_REG              (PPE_PA_BASE + 0x3E08) /* CNT_CYC类型控制寄存器 */
#define PPE_PA_PA_CLK_GATING_CTRL_REG         (PPE_PA_BASE + 0x3F00) /* 时钟自动门控使能寄存器。 */
#define PPE_PA_PA_VLAN_TAG0_REG               (PPE_PA_BASE + 0x4000) /* VLAN Tag配置寄存器0 */
#define PPE_PA_PA_VLAN_TAG1_REG               (PPE_PA_BASE + 0x4004) /* VLAN Tag配置寄存器1 */
#define PPE_PA_PA_VLAN_SELECT_0_REG           (PPE_PA_BASE + 0x4100) /* VLAN选择配置寄存器 */
#define PPE_PA_PA_VLAN_SELECT_1_REG           (PPE_PA_BASE + 0x4104) /* VLAN选择配置寄存器 */
#define PPE_PA_PA_VLAN_SELECT_2_REG           (PPE_PA_BASE + 0x4108) /* VLAN选择配置寄存器 */
#define PPE_PA_PA_VLAN_SELECT_3_REG           (PPE_PA_BASE + 0x410C) /* VLAN选择配置寄存器 */
#define PPE_PA_PA_PRI_MAP_TC_0_REG            (PPE_PA_BASE + 0x4300) /* PA Priority映射TC配置寄存器 */
#define PPE_PA_PA_PRI_MAP_TC_1_REG            (PPE_PA_BASE + 0x4330) /* PA Priority映射TC配置寄存器 */
#define PPE_PA_PA_PRI_MAP_TC_2_REG            (PPE_PA_BASE + 0x4360) /* PA Priority映射TC配置寄存器 */
#define PPE_PA_PA_PRI_MAP_TC_3_REG            (PPE_PA_BASE + 0x4390) /* PA Priority映射TC配置寄存器 */
#define PPE_PA_PA_EXP_MAP_TC_0_REG            (PPE_PA_BASE + 0x4304) /* MPLS报文配置寄存器 */
#define PPE_PA_PA_EXP_MAP_TC_1_REG            (PPE_PA_BASE + 0x4334) /* MPLS报文配置寄存器 */
#define PPE_PA_PA_EXP_MAP_TC_2_REG            (PPE_PA_BASE + 0x4364) /* MPLS报文配置寄存器 */
#define PPE_PA_PA_EXP_MAP_TC_3_REG            (PPE_PA_BASE + 0x4394) /* MPLS报文配置寄存器 */
#define PPE_PA_PA_DSCP_MAP_TC_0_0_REG         (PPE_PA_BASE + 0x4308) /* PA DSCP映射TC配置寄存器0 */
#define PPE_PA_PA_DSCP_MAP_TC_0_1_REG         (PPE_PA_BASE + 0x4338) /* PA DSCP映射TC配置寄存器0 */
#define PPE_PA_PA_DSCP_MAP_TC_0_2_REG         (PPE_PA_BASE + 0x4368) /* PA DSCP映射TC配置寄存器0 */
#define PPE_PA_PA_DSCP_MAP_TC_0_3_REG         (PPE_PA_BASE + 0x4398) /* PA DSCP映射TC配置寄存器0 */
#define PPE_PA_PA_DSCP_MAP_TC_1_0_REG         (PPE_PA_BASE + 0x430C) /* PA DSCP映射TC配置寄存器1 */
#define PPE_PA_PA_DSCP_MAP_TC_1_1_REG         (PPE_PA_BASE + 0x433C) /* PA DSCP映射TC配置寄存器1 */
#define PPE_PA_PA_DSCP_MAP_TC_1_2_REG         (PPE_PA_BASE + 0x436C) /* PA DSCP映射TC配置寄存器1 */
#define PPE_PA_PA_DSCP_MAP_TC_1_3_REG         (PPE_PA_BASE + 0x439C) /* PA DSCP映射TC配置寄存器1 */
#define PPE_PA_PA_DSCP_MAP_TC_2_0_REG         (PPE_PA_BASE + 0x4310) /* PA DSCP映射TC配置寄存器2 */
#define PPE_PA_PA_DSCP_MAP_TC_2_1_REG         (PPE_PA_BASE + 0x4340) /* PA DSCP映射TC配置寄存器2 */
#define PPE_PA_PA_DSCP_MAP_TC_2_2_REG         (PPE_PA_BASE + 0x4370) /* PA DSCP映射TC配置寄存器2 */
#define PPE_PA_PA_DSCP_MAP_TC_2_3_REG         (PPE_PA_BASE + 0x43A0) /* PA DSCP映射TC配置寄存器2 */
#define PPE_PA_PA_DSCP_MAP_TC_3_0_REG         (PPE_PA_BASE + 0x4314) /* PA DSCP映射TC配置寄存器3 */
#define PPE_PA_PA_DSCP_MAP_TC_3_1_REG         (PPE_PA_BASE + 0x4344) /* PA DSCP映射TC配置寄存器3 */
#define PPE_PA_PA_DSCP_MAP_TC_3_2_REG         (PPE_PA_BASE + 0x4374) /* PA DSCP映射TC配置寄存器3 */
#define PPE_PA_PA_DSCP_MAP_TC_3_3_REG         (PPE_PA_BASE + 0x43A4) /* PA DSCP映射TC配置寄存器3 */
#define PPE_PA_PA_DSCP_MAP_TC_4_0_REG         (PPE_PA_BASE + 0x4318) /* PA DSCP映射TC配置寄存器4 */
#define PPE_PA_PA_DSCP_MAP_TC_4_1_REG         (PPE_PA_BASE + 0x4348) /* PA DSCP映射TC配置寄存器4 */
#define PPE_PA_PA_DSCP_MAP_TC_4_2_REG         (PPE_PA_BASE + 0x4378) /* PA DSCP映射TC配置寄存器4 */
#define PPE_PA_PA_DSCP_MAP_TC_4_3_REG         (PPE_PA_BASE + 0x43A8) /* PA DSCP映射TC配置寄存器4 */
#define PPE_PA_PA_DSCP_MAP_TC_5_0_REG         (PPE_PA_BASE + 0x431C) /* PA DSCP映射TC配置寄存器5 */
#define PPE_PA_PA_DSCP_MAP_TC_5_1_REG         (PPE_PA_BASE + 0x434C) /* PA DSCP映射TC配置寄存器5 */
#define PPE_PA_PA_DSCP_MAP_TC_5_2_REG         (PPE_PA_BASE + 0x437C) /* PA DSCP映射TC配置寄存器5 */
#define PPE_PA_PA_DSCP_MAP_TC_5_3_REG         (PPE_PA_BASE + 0x43AC) /* PA DSCP映射TC配置寄存器5 */
#define PPE_PA_PA_DSCP_MAP_TC_6_0_REG         (PPE_PA_BASE + 0x4320) /* PA DSCP映射TC配置寄存器6 */
#define PPE_PA_PA_DSCP_MAP_TC_6_1_REG         (PPE_PA_BASE + 0x4350) /* PA DSCP映射TC配置寄存器6 */
#define PPE_PA_PA_DSCP_MAP_TC_6_2_REG         (PPE_PA_BASE + 0x4380) /* PA DSCP映射TC配置寄存器6 */
#define PPE_PA_PA_DSCP_MAP_TC_6_3_REG         (PPE_PA_BASE + 0x43B0) /* PA DSCP映射TC配置寄存器6 */
#define PPE_PA_PA_DSCP_MAP_TC_7_0_REG         (PPE_PA_BASE + 0x4324) /* PA DSCP映射TC配置寄存器7 */
#define PPE_PA_PA_DSCP_MAP_TC_7_1_REG         (PPE_PA_BASE + 0x4354) /* PA DSCP映射TC配置寄存器7 */
#define PPE_PA_PA_DSCP_MAP_TC_7_2_REG         (PPE_PA_BASE + 0x4384) /* PA DSCP映射TC配置寄存器7 */
#define PPE_PA_PA_DSCP_MAP_TC_7_3_REG         (PPE_PA_BASE + 0x43B4) /* PA DSCP映射TC配置寄存器7 */
#define PPE_PA_PA_TC_SEL_0_REG                (PPE_PA_BASE + 0x4328) /* PA TC 生成配置寄存器。 */
#define PPE_PA_PA_TC_SEL_1_REG                (PPE_PA_BASE + 0x4358) /* PA TC 生成配置寄存器。 */
#define PPE_PA_PA_TC_SEL_2_REG                (PPE_PA_BASE + 0x4388) /* PA TC 生成配置寄存器。 */
#define PPE_PA_PA_TC_SEL_3_REG                (PPE_PA_BASE + 0x43B8) /* PA TC 生成配置寄存器。 */
#define PPE_PA_PA_RW_RSV0_REG                 (PPE_PA_BASE + 0x7F00) /* 保留寄存器 */
#define PPE_PA_PA_RW_RSV1_REG                 (PPE_PA_BASE + 0x7F04) /* 保留寄存器 */
#define PPE_PA_PA_RW_RSV2_REG                 (PPE_PA_BASE + 0x7F08) /* 保留寄存器 */
#define PPE_PA_PA_RW_RSV3_REG                 (PPE_PA_BASE + 0x7F0C) /* 保留寄存器 */

/* PPE_CFG_PRO_CHECK_EN是协议检查的使能，每个域段对应每种错误的检查使能。（此部分为L2和L3）注：正常情况不能关闭。 */
#define PPE_PA_PA_L23_PRO_CHECK_ENABLE_0_REG  (PPE_PA_BASE + 0xA000)
#define PPE_PA_PA_L23_PRO_CHECK_ENABLE_1_REG  (PPE_PA_BASE + 0xA020)
#define PPE_PA_PA_L23_PRO_CHECK_ENABLE_2_REG  (PPE_PA_BASE + 0xA040)
#define PPE_PA_PA_L23_PRO_CHECK_ENABLE_3_REG  (PPE_PA_BASE + 0xA060)

/*
 PPE_CFG_PRO_CHECK_EN是协议检查的使能，每个域段对应每种错误的检查使能。
 （当MF报文的地址映射窗错误时，必定会产生l4_err）注：正常情况不能关闭。
 */
#define PPE_PA_PA_L4_PRO_CHECK_ENABLE_0_REG   (PPE_PA_BASE + 0xA004)
#define PPE_PA_PA_L4_PRO_CHECK_ENABLE_1_REG   (PPE_PA_BASE + 0xA024)
#define PPE_PA_PA_L4_PRO_CHECK_ENABLE_2_REG   (PPE_PA_BASE + 0xA044)
#define PPE_PA_PA_L4_PRO_CHECK_ENABLE_3_REG   (PPE_PA_BASE + 0xA064)

#define PPE_PA_PA_L23_HIS_PRO_ERR_0_REG       (PPE_PA_BASE + 0xA008) /* 层2层3协议检查曾经检查到的错误。 */
#define PPE_PA_PA_L23_HIS_PRO_ERR_1_REG       (PPE_PA_BASE + 0xA028) /* 层2层3协议检查曾经检查到的错误。 */
#define PPE_PA_PA_L23_HIS_PRO_ERR_2_REG       (PPE_PA_BASE + 0xA048) /* 层2层3协议检查曾经检查到的错误。 */
#define PPE_PA_PA_L23_HIS_PRO_ERR_3_REG       (PPE_PA_BASE + 0xA068) /* 层2层3协议检查曾经检查到的错误。 */
#define PPE_PA_PA_L4_HIS_PRO_ERR_0_REG        (PPE_PA_BASE + 0xA00C) /* 层4协议检查曾经检查到的错误。 */
#define PPE_PA_PA_L4_HIS_PRO_ERR_1_REG        (PPE_PA_BASE + 0xA02C) /* 层4协议检查曾经检查到的错误。 */
#define PPE_PA_PA_L4_HIS_PRO_ERR_2_REG        (PPE_PA_BASE + 0xA04C) /* 层4协议检查曾经检查到的错误。 */
#define PPE_PA_PA_L4_HIS_PRO_ERR_3_REG        (PPE_PA_BASE + 0xA06C) /* 层4协议检查曾经检查到的错误。 */
#define PPE_PA_PA_L2E_PKT_REG                 (PPE_PA_BASE + 0xB004) /* PA接收的L2错误报文统计计数器 */
#define PPE_PA_PA_L3E_PKT_REG                 (PPE_PA_BASE + 0xB00C) /* PA接收的L3错误报文统计计数器 */
#define PPE_PA_PA_L4E_PKT_REG                 (PPE_PA_BASE + 0xB014) /* PA接收的L4错误报文统计计数器 */
#define PPE_PA_PA_DFX_PORT_REG                (PPE_PA_BASE + 0xB100) /* PA_DFX 统计计数器端口配置寄存器 */
#define PPE_PA_PA_KEY_DFX_CLR_REG             (PPE_PA_BASE + 0xB104) /* PA KEY值DFX清除配置寄存器 */
#define PPE_PA_PA_KEY_SEL_REG                 (PPE_PA_BASE + 0xB108) /* PA KEY值选择配置寄存器 */
#define PPE_PA_PA_KEY_VALUE_REG               (PPE_PA_BASE + 0xB10C) /* PA KEY值DFX寄存器 */

#endif // __REG_PA_OFFSET_H__
