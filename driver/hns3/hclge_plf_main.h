/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: hclge_plf_main.h
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __HCLGE_PLF_MAIN_H
#define __HCLGE_PLF_MAIN_H

#include <linux/fs.h>
#include <linux/types.h>
#include <linux/phy.h>
#include <linux/if_vlan.h>
#include <linux/kfifo.h>
#include <linux/device.h>
#include <linux/interrupt.h>

#include "platform_define.h"
#include "hclge_plf_rss.h"
#include "kcompat.h"
#include "hnae3.h"
#include "hclge_plf_flow_director.h"
#include "fpdc.h"

#define HNSPLF_NAME "hnsplf"
#define HCLGE_PLF_DRIVER_NAME "hclge_plf"

#define PLATFORM_EEPROM_DEV_NAME "/dev/eeprom_m24256-1"
#define PLATFORM_EEPROM_OFFSET 0xd00
#define PLATFORM_EEPROM_MAC_OFFSET 0x5
#define PLATFORM_EEPROM_MAC_SIZE 0x10

#define PLATFORM_MDIO_BASE_ADDR 0xa2070000
#define PLATFORM_MIDO_REG_SIZE 128

#define PLATFORM_DEVICE_MEM_RES_ID 1
#define PLATFORM_DEVICE_MEM_RES_IOSUB_ID 2
#define PLATFORM_DEVICE_MEM_RES_MDIO_ID 3
#define PLATFORM_DEVICE_MEM_RES_GPIO_ID 4
#define PLATFORM_DEVICE_MEM_RES_PHY_GPIO_ID 5
#define PLATFORM_DEVICE_MEM_RES_IOMUX_ID 6
#define PLATFORM_DEVICE_MEM_RES_SCHE_ID 7

#define PLATFORM_TQP_NUM 16

#define PLATFORM_TQP_INT_NUM 16 /* TQP interrupt */
#define PLATFORM_ABN_INT_NUM 1  /* ABN interrupt */
#define PLATFORM_MAC_INT_NUM 1  /* MAC interrupt */
#define PLATFORM_RTC_INT_NUM 1  /* RTC_PPS interrupt */

#define TC_MAX_NUM        0x10

#define MAX_DEV_ID 64 /* for future */

#define HCLGE_PLF_RESET_MAX_FAIL_CNT   5
#define HCLGE_PLF_RESET_SYNC_TIME      100

#define HCLGE_PLF_REAL_CLK      0
#define HCLGE_PLF_VIRTUAL_CLK   1

#define CFG_WAIT_LOOP_NUM 100
#define CFG_WAIT_INTERVAL 1
#define STATE_WAIT_LOOP_NUM 10000
#define RCB_RESET_WAIT_LOOP_NUM 100
#define STATE_WAIT_INTERVAL 1

#define SUPPORT_AUTONEG_ON 1
#define SUPPORT_AUTONEG_OFF 0
#define VLAN_ID_MASK 0xfff0

#define RAS_INT_START 0
#define PTP_INT_START 0
#define ABN_INT_START 0 /* Refer to dts_file */
#define MAC_INT_START (ABN_INT_START + PLATFORM_ABN_INT_NUM)
#define RTC_INT_START (ABN_INT_START + PLATFORM_ABN_INT_NUM + PLATFORM_MAC_INT_NUM)

#define PLATFORM_MISC_INT_NUM (PLATFORM_ABN_INT_NUM + PLATFORM_MAC_INT_NUM + PLATFORM_RTC_INT_NUM)

#define PLATFORM_BUF_SIZE_UNIT 256U

#define DEF_PLATFORM_PKT_BUF_SIZE 0x2048
#define DEF_PLATFORM_TX_BUF 0x288 /* Refer to IMP FPGA CFG */

#define MAX_PLATFORM_NON_TSO_BD_NUM 18
#define MAX_PLATFORM_INT_QL_MAX 1000
#define MAX_PLATFORM_QSET_NUM 4

#define MAX_PLATFORM_TM_RATE 2500UL
#define MAX_PLATFORM_INT_GL 8160UL
#define MAX_PLATFORM_PKT_LEN_ETH 9728

#define MAX_PLATFORM_TC_NUM 8
#define DEF_PLATFORM_MIN_RING_SIZE 0x8
#define DEF_PLATFORM_MAX_RING_SIZE (0x8000 - DEF_PLATFORM_MIN_RING_SIZE)
#define DEF_PLATFORM_TQP_DESC_NUM 0x400
#define DEF_PLATFORM_PHY_ADDR 0x1A
#define DEF_PLATFORM_RX_BUF_LEN 0x800
#define DEF_PLATFORM_MAC_ADDR_LOW 0x44332211
#define DEF_PLATFORM_MAC_ADDR_HIGH 0x6655
#define DEF_PLATFORM_SPEED_1G 0
#define DEF_PLATFORM_NUMA_NODE_MAP 0xff
#define DEF_PLATFORM_SPEED_ABILITY 0x3f
#define DEF_PLATFORM_SPEED_ABILITY_EXT 0x0
#define DEF_PLATFORM_VLAN_MODE 0x0

#define PLATFORM_MAX_TC_NUM 16

#define HCLGE_PLF_SUPPORT_1G_BIT		BIT(0)
#define HCLGE_PLF_SUPPORT_10G_BIT		BIT(1)
#define HCLGE_PLF_SUPPORT_25G_BIT		BIT(2)
#define HCLGE_PLF_SUPPORT_50G_BIT		BIT(3)
#define HCLGE_PLF_SUPPORT_100G_BIT		BIT(4)
/* to be compatible with exsit board */
#define HCLGE_PLF_SUPPORT_40G_BIT		BIT(5)
#define HCLGE_PLF_SUPPORT_100M_BIT		BIT(6)
#define HCLGE_PLF_SUPPORT_10M_BIT		BIT(7)
#define HCLGE_PLF_SUPPORT_200G_BIT		BIT(8)
#define HCLGE_PLF_SUPPORT_400G_BIT		BIT(9)
#define HCLGE_PLF_SUPPORT_GE \
    (HCLGE_PLF_SUPPORT_1G_BIT | HCLGE_PLF_SUPPORT_100M_BIT | HCLGE_PLF_SUPPORT_10M_BIT)

#define PLATFORM_TQP_REG_OFFSET 0x10000
#define PLATFORM_TQP_REG_SIZE 0x200

/* Module Reg Base */
#define TOP_REG_BASE 0x410000
#define FUNC_COM_REG_BASE 0x0
#define RX_PA_REG_BASE 0x4d0000
#define TX_PA_REG_BASE 0x4e0000
#define RCB_COM_REG_BASE 0x440000
#define RCB_TQP_REG_BASE 0x10000
#define TPU_REG_BASE 0x430000
#define RPU_REG_BASE 0x4f0000
#define SSU_REG_BASE 0x420000
#define PPP_REG_BASE 0x470000

/* FUNC_COM */
#define TQP_INT_CTRL_REG(ring_id)                 (FUNC_COM_REG_BASE + 0x0 + ((ring_id) << 2))

/* RCB_COM */
#define RCB_RING_PORT_MAP(Ring_num) (0x02018 + 0x40 * Ring_num)
#define RCB_MEM_INIT_START 0x0c
#define RCB_MEM_INIT_DONE 0x10

#define RCB_TX_Q_INTCTL_REG(ring_id)           (RCB_COM_REG_BASE + 0x2000 + ((ring_id) << 6))
#define RCB_RX_Q_INTCTL_REG(ring_id)           (RCB_COM_REG_BASE + 0x2004 + ((ring_id) << 6))

#define ETS_QUEUE_EN 0x1
#define ETS_QUEUE_FLUSH 0x1
#define ETS_QUEUE_FLUSH_DONE 0x1

#define RCB_RING_PORT_MAP_REG(ring_id)           (RCB_COM_REG_BASE + 0x2018 + ((ring_id) << 6))
#define ETS_QUEUE_EN_CFG_REG(ring_id)            (RCB_COM_REG_BASE + 0x1A80 + ((ring_id) << 2))
#define ETS_QUEUE_FLUSH_REQ_REG(ring_id)         (RCB_COM_REG_BASE + 0x1DC0 + ((ring_id) << 2))
#define ETS_QUEUE_FLUSH_DONE_REG(ring_id)        (RCB_COM_REG_BASE + 0x1E00 + ((ring_id) << 2))
#define RCB_RING_TO_BE_RST_REG(ring_id)          (RCB_COM_REG_BASE + 0x02010 + ((ring_id) << 6))
#define RCB_RING_COULD_BE_RST_REG(ring_id)       (RCB_COM_REG_BASE + 0x02014 + ((ring_id) << 6))

#define RCB_USER_STRMID(ring_id)           (RCB_COM_REG_BASE + 0xa04 + ((ring_id) << 4))

#define FSM_DFX_ST0_ADDR                         (RCB_COM_REG_BASE + 0x01000)
#define FIFO_DFX_ST4_ADDR(ring_id)               (RCB_COM_REG_BASE + 0x01020 + ((ring_id) << 2))
#define FIFO_DFX_ST5_ADDR(ring_id)               (RCB_COM_REG_BASE + 0x01080 + ((ring_id) << 2))

/* RCB_TQP */
#define RCB_RING_CFG_EN(Ring_num) (0x02018 + 0x40 * Ring_num)

#define RCB_CFG_EN			                     0x1
#define RCB_CFG_EN_REG(ring_id)                 (RCB_TQP_REG_BASE + 0x0090 + ((ring_id) << 9))
#define RCB_CFG_RX_RING_HEAD_REG(ring_id)       (RCB_TQP_REG_BASE + 0x001c + ((ring_id) << 9))

/* RX PA */
#define PA_ANALY_CONFIG(port)                   (RX_PA_REG_BASE + 0x3500 + ((port) << 2))
#define PA_VLAN_SELECT_RX_REG(port)             (RX_PA_REG_BASE + 0x4100 + ((port) << 2))
#define PA_L23_PRO_CHECK_ENABLE(port)           (RX_PA_REG_BASE + 0xA000 + ((port) << 5))
#define PA_L4_PRO_CHECK_ENABLE(port)            (RX_PA_REG_BASE + 0xA004 + ((port) << 5))
#define  PA_L2_FLEX_OFFSET_CFG_ADDR(port)       (RX_PA_REG_BASE + 0x630 + ((port) << 4U))
#define  PA_L3_FLEX_OFFSET_CFG_ADDR(port)       (RX_PA_REG_BASE + 0x634 + ((port) << 4U))
#define  PA_L4_FLEX_OFFSET_CFG_ADDR(port)       (RX_PA_REG_BASE + 0x638 + ((port) << 4U))
#define  PA_UDF_DEF_VALUE_CFG_ADDR(port)        (RX_PA_REG_BASE + 0x63C + ((port) << 4U))

/* PPP */
#define PPP_PPP_TCAM_MODE_REG                   (PPP_REG_BASE + 0x321C)
#define PPP_PPP_FD_KEY_TEMPL_DATA_0_REG         (PPP_REG_BASE + 0x3900)
#define PPP_PPP_FD_KEY_TEMPL_DATA_1_REG         (PPP_REG_BASE + 0x3904)
#define PPP_PPP_FD_KEY_TEMPL_DATA_2_REG         (PPP_REG_BASE + 0x3908)
#define PPP_PPP_FD_KEY_TEMPL_DATA_3_REG         (PPP_REG_BASE + 0x390C)
#define PPP_PPP_FD_KEY_TEMPL_CFG_REG            (PPP_REG_BASE + 0x3910)
#define PPP_PPP_FD_KEY_SEL_TDATA_0_REG          (PPP_REG_BASE + 0x3A00)
#define PPP_PPP_FD_KEY_SEL_TDATA_1_REG          (PPP_REG_BASE + 0x3A04)
#define PPP_PPP_FD_KEY_SEL_MDATA_REG            (PPP_REG_BASE + 0x3A14)
#define PPP_PPP_FD_KEY_SEL_CFG_REG              (PPP_REG_BASE + 0x3A18)
#define PPP_PPP_FD_KEY_DATA_ADDR(index)         (PPP_REG_BASE + 0x3B00 + ((index) << 2U))
#define PPP_PPP_FD_KEY_CFG_REG                  (PPP_REG_BASE + 0x3B34)
#define PPP_EGR_PORT_ATTR_REG                   (PPP_REG_BASE + 0x4a00)
#define PPP_PPP_EGR_PORT_ATTR_CFG               (PPP_REG_BASE + 0x4a04)
#define PPP_PPP_TNL_MISC_CFG_REG(tnl_num)       (PPP_REG_BASE + 0x5a00 + ((tnl_num) << 2))


/* AXI_MST_OOO */
#define AM_CTRL_GLOBAL_REG                      (TOP_REG_BASE + 0x2000 + 0x0)
#define AM_CURR_PORT_STS_REG                    (TOP_REG_BASE + 0x2000 + 0x100)

#define AM_CFG_PORT_RD_EN_REG                   (TOP_REG_BASE + 0x2000 + 0x18)
#define AM_CFG_PORT_WR_EN_REG                   (TOP_REG_BASE + 0x2000 + 0x1C)

/* IO_SUBCTRL */
#define SC_NIC_INTF0_RESET_REQ_REG              (0x450)
#define SC_NIC_INTF1_RESET_REQ_REG              (0x458)
#define SC_NIC_INTF0_ICG_EN_REG                 (0x398)
#define SC_NIC_INTF0_ICG_DIS_REG                (0x39C)
#define SC_NIC_INTF1_ICG_EN_REG                 (0x3A0)
#define SC_NIC_INTF1_ICG_DIS_REG                (0x3A4)

/* TPU */
#define TPU_MEM_INIT_START 0x2800
#define TPU_MEM_INIT_DONE 0x2804

#define TPU_USER_STRMID(ring_id)           (TPU_REG_BASE + 0x7004 + ((ring_id) << 5))

/* RPU */
#define RPU_MEM_INIT_START 0x2800
#define RPU_MEM_INIT_DONE 0x2804

/* SSU */
#define SSU_MEM_INIT_START 0x400
#define SSU_MEM_INIT_DONE 0x404
#define SSU_RPU_PORT_INIT_ENABLE 0x490
#define SSU_TX_OQ_SCHEDULE_EN 0x4A4
#define SSU_L2ERR_DROP_EN 0x4AC
#define SSU_EG_MAC_SCH_ROUND_NUM 0xB80

/* PPP */
#define PPP_MEM_INIT_START 0x3000
#define PPP_MEM_INIT_DONE 0x3004

/* IO subctrl */
#define IO_SUBCTRL_SC_MDIO_ICG_EN 0x320
#define IO_SUBCTRL_SC_MDIO_ICG_DIS 0x324
#define IO_SUBCTRL_SC_NIC_INTF0_ICG_EN 0x398
#define IO_SUBCTRL_SC_NIC_INTF0_ICG_DIS 0x39c
#define IO_SUBCTRL_SC_NIC_INTF1_ICG_EN 0x3A0
#define IO_SUBCTRL_SC_NIC_INTF1_ICG_DIS 0x3A4
#define IO_SUBCTRL_SC_RGMII_ICG_EN 0x3A8
#define IO_SUBCTRL_SC_RGMII_ICG_DIS 0x3AC
#define IO_SUBCTRL_SC_MDIO_RESET_REQ 0x418
#define IO_SUBCTRL_SC_MDIO_RESET_DREQ 0x41C
#define IO_SUBCTRL_SC_NIC_INTF0_RESET_REQ 0x450
#define IO_SUBCTRL_SC_NIC_INTF0_RESET_DREQ 0x454
#define IO_SUBCTRL_SC_NIC_INTF1_RESET_REQ 0x458
#define IO_SUBCTRL_SC_NIC_INTF1_RESET_DREQ 0x45C
#define IO_SUBCTRL_SC_RGMII_RESET_REQ 0x480
#define IO_SUBCTRL_SC_RGMII_RESET_DREQ 0x484
#define IO_SUBCTRL_SC_NIC_CLK_SEL 0x3700
#define IO_SUBCTRL_SC_NIC_TX_MODE 0x3704

/* IOMUX */
#define IOMUX_RGMII_PHY_START 0x0
#define IOMUX_RGMII_PHY_END 0x74
#define IOMUX_PHY_RST_0 0x30
#define IOMUX_PHY_RST_1 0x64

static inline void hclge_plf_isb(void)
{
#ifndef DEFINE_HNS_LLT
    asm volatile("isb" : : : "memory");
#endif
}

static inline void hclge_plf_dsb(void)
{
#ifndef DEFINE_HNS_LLT
    asm volatile("dsb sy" : : : "memory");
#endif
}

enum { PORT_MT_NOUSE = 0, PORT_MT_FIBER = 1, PORT_MT_PHY = 2, PORT_MT_BACKPLANE = 3 };

static inline const char *hns3_platform_name(const struct platform_device *pdev)
{
    return dev_name(&pdev->dev);
}

#define HCLGE_PLF_DEFAULT_DV 0xA000 /* 40k byte */

#define HCLGE_PLF_INVALID_VPORT 0xffff

#define HCLGE_PLF_VECTOR_REG_BASE 0x0

#define HCLGE_PLF_VECTOR_REG_OFFSET 0x4

#define HCLGE_PLF_MAX_QSET_NUM 1024

#define HCLGE_PLF_MAC_MIN_FRAME 64
#define HCLGE_PLF_MAC_MAX_FRAME 9728

enum HCLGE_DEV_STATE {
    HCLGE_STATE_REINITING,
    HCLGE_STATE_DOWN,
    HCLGE_STATE_DISABLED,
    HCLGE_STATE_REMOVING,
    HCLGE_STATE_NIC_REGISTERED,
    HCLGE_STATE_ROCE_REGISTERED,
    HCLGE_STATE_ROH_REGISTERED,
    HCLGE_STATE_SERVICE_INITED,
    HCLGE_STATE_RST_SERVICE_SCHED,
    HCLGE_STATE_RST_HANDLING,
    HCLGE_STATE_MBX_SERVICE_SCHED,
    HCLGE_STATE_MBX_HANDLING,
    HCLGE_STATE_STATISTICS_UPDATING,
    HCLGE_STATE_CMD_DISABLE,
    HCLGE_STATE_LINK_UPDATING,
    HCLGE_STATE_HW_QB_ENABLE,
    HCLGE_STATE_RST_FAIL,
    HCLGE_STATE_PTP_EN,
    HCLGE_STATE_PTP_TX_HANDLING,
    HCLGE_STATE_FD_TBL_CHANGED,
    HCLGE_STATE_FD_CLEAR_ALL,
    HCLGE_STATE_FD_USER_DEF_CHANGED,
    HCLGE_STATE_ERR_HANDLING_SCHED,
    HCLGE_STATE_FD_USE_RSS_UPDATING,
    HCLGE_STATE_INITING,
    HCLGE_STATE_QOS_CFG_CHANGED,
    HCLGE_STATE_MAX
};
#define HCLGE_MAC_DEFAULT_FRAME (ETH_HLEN + ETH_FCS_LEN + 2 * VLAN_HLEN + ETH_DATA_LEN)

enum HCLGE_MAC_SPEED {
    HCLGE_MAC_SPEED_UNKNOWN = 0,   /* unknown */
    HCLGE_MAC_SPEED_10M = 10,      /* 10 Mbps */
    HCLGE_MAC_SPEED_100M = 100,    /* 100 Mbps */
    HCLGE_MAC_SPEED_1G = 1000,     /* 1000 Mbps   = 1 Gbps */
    HCLGE_MAC_SPEED_2_5G = 2500,   /* 2500 Mbps   = 2.5 Gbps */
    HCLGE_MAC_SPEED_10G = 10000,   /* 10000 Mbps  = 10 Gbps */
    HCLGE_MAC_SPEED_25G = 25000,   /* 25000 Mbps  = 25 Gbps */
    HCLGE_MAC_SPEED_40G = 40000,   /* 40000 Mbps  = 40 Gbps */
    HCLGE_MAC_SPEED_50G = 50000,   /* 50000 Mbps  = 50 Gbps */
    HCLGE_MAC_SPEED_100G = 100000, /* 100000 Mbps = 100 Gbps */
    HCLGE_MAC_SPEED_200G = 200000, /* 200000 Mbps = 200 Gbps */
    HCLGE_MAC_SPEED_400G = 400000  /* 400000 Mbps = 400 Gbps */
};

/* mac speed type defined in firmware command */
enum HCLGE_FIRMWARE_MAC_SPEED {
    HCLGE_FW_MAC_SPEED_1G,
    HCLGE_FW_MAC_SPEED_10G,
    HCLGE_FW_MAC_SPEED_25G,
    HCLGE_FW_MAC_SPEED_40G,
    HCLGE_FW_MAC_SPEED_50G,
    HCLGE_FW_MAC_SPEED_100G,
    HCLGE_FW_MAC_SPEED_10M,
    HCLGE_FW_MAC_SPEED_100M,
    HCLGE_FW_MAC_SPEED_200G,
    HCLGE_FW_MAC_SPEED_400G,
    HCLGE_FW_MAC_SPEED_2_5G
};

enum HCLGE_MAC_DUPLEX { HCLGE_MAC_HALF, HCLGE_MAC_FULL };

#define HCLGE_LINK_STATUS_MS 10

struct reset_tqp_dfx_reg {
    char *reg_name;
    u32 reg_addr;
};

#define QUERY_SFP_SPEED 0
#define QUERY_ACTIVE_SPEED 1


#define SFP_GPIO_DEFAULT_INDEX (0xffffffff)

#define ETH_ALEN 6
struct hclge_plf_mac {
    u8 mac_id;
    u8 phy_addr;
    u8 flag;
    u8 media_type; /* port media type, e.g. fibre/copper/backplane */
    u8 mac_addr[ETH_ALEN];
    u8 autoneg;
    u8 autoneg_last;
    u8 duplex;
    u8 duplex_last;
    u8 support_autoneg;
    u8 speed_type; /* 0: sfp speed, 1: active speed */
    u8 mac_type;   /* 0: Ethernet, 1: ROH */
    u32 speed;
    u32 max_speed;
    u32 speed_ability; /* speed ability supported by current media */
    u32 module_type;   /* sub media type, e.g. kr/cr/sr/lr */
    u32 fec_mode;      /* active fec mode */
    u32 user_fec_mode;
    u32 fec_ability;
    int link; /* store the link status of mac & phy (if phy exists) */
    struct phy_device *phydev;
    struct mii_bus *mdio_bus;
    phy_interface_t phy_if;
    __ETHTOOL_DECLARE_LINK_MODE_MASK(supported);
    __ETHTOOL_DECLARE_LINK_MODE_MASK(advertising);
    void __iomem *mdio_base; /* mdio reg base addr */
    u32 mac_mode;            /* 0: SGMII 1: RGMII */
    void *mdio_reg;
    void __iomem *gpio_base; /* gpio reg base addr */
    u32 gpio_index;
    u32 sfp_present;
    u32 ds_index;
    void __iomem *phy_gpio_base; /* phy_gpio reg base addr */
    u32 phy_gpio_index;
    atomic_t int_clear_flag; /* when first link up, clear interrupt after initialization */
};

struct hclge_plf_hw {
    void __iomem *io_base;
    void __iomem *iosub_base;
    void __iomem *mem_base;
    void __iomem *iomux_base;
    void __iomem *sche_base;
    struct hclge_plf_mac mac;
    int num_vec;
};

/* TQP stats */
struct hlcge_plf_tqp_stats {
    /* query_tqp_tx_queue_statistics ,opcode id:  0x0B03 */
    u64 rcb_tx_ring_pktnum_rcd; /* 32bit */
    /* query_tqp_rx_queue_statistics ,opcode id:  0x0B13 */
    u64 rcb_rx_ring_pktnum_rcd; /* 32bit */
};

struct hclge_plf_tqp {
    /* copy of device pointer from pci_dev,
     * used when perform DMA mapping
     */
    struct device *dev;
    struct hnae3_queue q;
    struct hlcge_plf_tqp_stats tqp_stats;
    u16 index; /* Global index in a NIC controller */

    bool alloced;
};

enum hclge_plf_fc_mode {
    HCLGE_FC_NONE,
    HCLGE_FC_RX_PAUSE,
    HCLGE_FC_TX_PAUSE,
    HCLGE_FC_FULL,
    HCLGE_FC_PFC,
    HCLGE_FC_DEFAULT
};

#define HCLGE_FILTER_TYPE_VF 0
#define HCLGE_FILTER_TYPE_PORT 1
#define HCLGE_FILTER_FE_EGRESS_V1_B BIT(0)
#define HCLGE_FILTER_FE_NIC_INGRESS_B BIT(0)
#define HCLGE_FILTER_FE_NIC_EGRESS_B BIT(1)
#define HCLGE_FILTER_FE_ROCE_INGRESS_B BIT(2)
#define HCLGE_FILTER_FE_ROCE_EGRESS_B BIT(3)
#define HCLGE_FILTER_FE_EGRESS (HCLGE_FILTER_FE_NIC_EGRESS_B | HCLGE_FILTER_FE_ROCE_EGRESS_B)
#define HCLGE_FILTER_FE_INGRESS (HCLGE_FILTER_FE_NIC_INGRESS_B | HCLGE_FILTER_FE_ROCE_INGRESS_B)

enum hclge_vlan_mode_sel {
    HCLGE_VLAN_DEFAULT_MODE,
    HCLGE_VLAN_DYNAMIC_MODE
};

enum hclge_link_fail_code {
    HCLGE_LF_NORMAL,
    HCLGE_LF_REF_CLOCK_LOST,
    HCLGE_LF_XSFP_TX_DISABLE,
    HCLGE_LF_XSFP_ABSENT
};

#define HCLGE_LINK_STATUS_DOWN 0
#define HCLGE_LINK_STATUS_UP 1

#define HCLGE_MAX_TC_NUM 8
#define HCLGE_PG_NUM 4
#define HCLGE_SCH_MODE_SP 0
#define HCLGE_SCH_MODE_DWRR 1
#define HCLGE_SCH_MODE_CBS 2
struct hclge_plf_pg_info {
    u8 pg_id;
    u8 pg_sch_mode; /* 0: sp; 1: dwrr */
    u8 tc_bit_map;
    u32 bw_limit;
    u8 tc_dwrr[HNAE3_MAX_TC];
};

struct hclge_plf_tc_info {
    u8 tc_id;
    u8 tc_sch_mode; /* 0: sp; 1: dwrr */
    u8 pgid;
    u8 tc_dwrr;
    u32 bw_limit;
};

struct hclge_plf_cfg {
    u8 tc_num;
    u16 tqp_desc_num;
    u16 rx_buf_len;
    u16 vf_rss_size_max;
    u16 pf_rss_size_max;
    u8 phy_addr;
    u8 media_type;
    u8 mac_addr[ETH_ALEN];
    u8 default_speed;
    u32 numa_node_map;
    u32 tx_spare_buf_size;
    u16 speed_ability;
    u8 vlan_mode_sel;
    u16 umv_space;
};

struct hclge_plf_tm_info {
    u8 num_tc;
    u8 num_pg; /* It must be 1 if vNET-Base schd */
    u8 pg_dwrr[HCLGE_PG_NUM];
    u8 prio_tc[HNAE3_MAX_USER_PRIO];
    struct hclge_plf_pg_info pg_info[HCLGE_PG_NUM];
    struct hclge_plf_tc_info tc_info[HNAE3_MAX_TC];
    enum hclge_plf_fc_mode fc_mode;
    u8 hw_pfc_map; /* Allow for packet drop or not on this TC */
    u8 pfc_en;     /* PFC enabled or not for user priority */
};

/* max number of mac statistics on each version */
#define HCLGE_MAC_STATS_MAX_NUM_V1 84
#define HCLGE_MAC_STATS_MAX_NUM_V2 105

struct hclge_comm_stats_str {
    char desc[ETH_GSTRING_LEN];
    u32 stats_num;
    unsigned long offset;
};

/* mac stats ,opcode id: 0x0032 */
struct hclge_plf_mac_stats {
    u64 mac_tx_mac_pause_num;
    u64 mac_rx_mac_pause_num;
    u64 rsv0;
    u64 mac_tx_pfc_pri0_pkt_num;
    u64 mac_tx_pfc_pri1_pkt_num;
    u64 mac_tx_pfc_pri2_pkt_num;
    u64 mac_tx_pfc_pri3_pkt_num;
    u64 mac_tx_pfc_pri4_pkt_num;
    u64 mac_tx_pfc_pri5_pkt_num;
    u64 mac_tx_pfc_pri6_pkt_num;
    u64 mac_tx_pfc_pri7_pkt_num;
    u64 mac_rx_pfc_pri0_pkt_num;
    u64 mac_rx_pfc_pri1_pkt_num;
    u64 mac_rx_pfc_pri2_pkt_num;
    u64 mac_rx_pfc_pri3_pkt_num;
    u64 mac_rx_pfc_pri4_pkt_num;
    u64 mac_rx_pfc_pri5_pkt_num;
    u64 mac_rx_pfc_pri6_pkt_num;
    u64 mac_rx_pfc_pri7_pkt_num;
    u64 mac_tx_total_pkt_num;
    u64 mac_tx_total_oct_num;
    u64 mac_tx_good_pkt_num;
    u64 mac_tx_bad_pkt_num;
    u64 mac_tx_good_oct_num;
    u64 mac_tx_bad_oct_num;
    u64 mac_tx_uni_pkt_num;
    u64 mac_tx_multi_pkt_num;
    u64 mac_tx_broad_pkt_num;
    u64 mac_tx_undersize_pkt_num;
    u64 mac_tx_oversize_pkt_num;
    u64 mac_tx_64_oct_pkt_num;
    u64 mac_tx_65_127_oct_pkt_num;
    u64 mac_tx_128_255_oct_pkt_num;
    u64 mac_tx_256_511_oct_pkt_num;
    u64 mac_tx_512_1023_oct_pkt_num;
    u64 mac_tx_1024_1518_oct_pkt_num;
    u64 mac_tx_1519_2047_oct_pkt_num;
    u64 mac_tx_2048_4095_oct_pkt_num;
    u64 mac_tx_4096_8191_oct_pkt_num;
    u64 rsv1;
    u64 mac_tx_8192_9216_oct_pkt_num;
    u64 mac_tx_9217_12287_oct_pkt_num;
    u64 mac_tx_12288_16383_oct_pkt_num;
    u64 mac_tx_1519_max_good_oct_pkt_num;
    u64 mac_tx_1519_max_bad_oct_pkt_num;

    u64 mac_rx_total_pkt_num;
    u64 mac_rx_total_oct_num;
    u64 mac_rx_good_pkt_num;
    u64 mac_rx_bad_pkt_num;
    u64 mac_rx_good_oct_num;
    u64 mac_rx_bad_oct_num;
    u64 mac_rx_uni_pkt_num;
    u64 mac_rx_multi_pkt_num;
    u64 mac_rx_broad_pkt_num;
    u64 mac_rx_undersize_pkt_num;
    u64 mac_rx_oversize_pkt_num;
    u64 mac_rx_64_oct_pkt_num;
    u64 mac_rx_65_127_oct_pkt_num;
    u64 mac_rx_128_255_oct_pkt_num;
    u64 mac_rx_256_511_oct_pkt_num;
    u64 mac_rx_512_1023_oct_pkt_num;
    u64 mac_rx_1024_1518_oct_pkt_num;
    u64 mac_rx_1519_2047_oct_pkt_num;
    u64 mac_rx_2048_4095_oct_pkt_num;
    u64 mac_rx_4096_8191_oct_pkt_num;
    u64 rsv2;
    u64 mac_rx_8192_9216_oct_pkt_num;
    u64 mac_rx_9217_12287_oct_pkt_num;
    u64 mac_rx_12288_16383_oct_pkt_num;
    u64 mac_rx_1519_max_good_oct_pkt_num;
    u64 mac_rx_1519_max_bad_oct_pkt_num;

    u64 mac_tx_fragment_pkt_num;
    u64 mac_tx_undermin_pkt_num;
    u64 mac_tx_jabber_pkt_num;
    u64 mac_tx_err_all_pkt_num;
    u64 mac_tx_from_app_good_pkt_num;
    u64 mac_tx_from_app_bad_pkt_num;
    u64 mac_rx_fragment_pkt_num;
    u64 mac_rx_undermin_pkt_num;
    u64 mac_rx_jabber_pkt_num;
    u64 mac_rx_fcs_err_pkt_num;
    u64 mac_rx_send_app_good_pkt_num;
    u64 mac_rx_send_app_bad_pkt_num;
    u64 mac_tx_pfc_pause_pkt_num;
    u64 mac_rx_pfc_pause_pkt_num;
    u64 mac_tx_ctrl_pkt_num;
    u64 mac_rx_ctrl_pkt_num;

    /* duration of pfc */
    u64 mac_tx_pfc_pri0_xoff_time;
    u64 mac_tx_pfc_pri1_xoff_time;
    u64 mac_tx_pfc_pri2_xoff_time;
    u64 mac_tx_pfc_pri3_xoff_time;
    u64 mac_tx_pfc_pri4_xoff_time;
    u64 mac_tx_pfc_pri5_xoff_time;
    u64 mac_tx_pfc_pri6_xoff_time;
    u64 mac_tx_pfc_pri7_xoff_time;
    u64 mac_rx_pfc_pri0_xoff_time;
    u64 mac_rx_pfc_pri1_xoff_time;
    u64 mac_rx_pfc_pri2_xoff_time;
    u64 mac_rx_pfc_pri3_xoff_time;
    u64 mac_rx_pfc_pri4_xoff_time;
    u64 mac_rx_pfc_pri5_xoff_time;
    u64 mac_rx_pfc_pri6_xoff_time;
    u64 mac_rx_pfc_pri7_xoff_time;

    /* duration of pause */
    u64 mac_tx_pause_xoff_time;
    u64 mac_rx_pause_xoff_time;
};

#define HCLGE_STATS_TIMER_INTERVAL 300UL

static inline void hclge_write_reg(void __iomem *base, u32 reg, u32 value)
{
    writel(value, base + reg);
}

#define hclge_write_dev(a, reg, value) hclge_write_reg((a)->io_base, reg, value)
#define hclge_read_dev(a, reg) hclge_read_reg((a)->io_base, reg)

static inline u32 hclge_read_reg(u8 __iomem *base, u32 reg)
{
    u8 __iomem *reg_addr = READ_ONCE(base);

    return readl(reg_addr + reg);
}

enum HCLGE_MAC_NODE_STATE { HCLGE_MAC_TO_ADD, HCLGE_MAC_TO_DEL, HCLGE_MAC_ACTIVE };

struct hclge_vport_mac_addr_cfg {
    struct list_head node;
    enum HCLGE_MAC_NODE_STATE state;
    u8 mac_addr[ETH_ALEN];
};

enum HCLGE_MAC_ADDR_TYPE { HCLGE_MAC_ADDR_UC, HCLGE_MAC_ADDR_MC };

enum HCLGE_IMP_ERR_TYPE {
    HCLGE_IMP_RD_POISON,
    HCLGE_IMP_CMDQ_ERROR
};

struct hclge_vport_vlan_cfg {
    struct list_head node;
    int hd_tbl_status;
    u16 vlan_id;
};

struct hclge_plf_rst_stats {
    u32 reset_done_cnt;    /* the number of reset has completed */
    u32 hw_reset_done_cnt; /* the number of HW reset has completed */
    u32 pf_rst_cnt;        /* the number of PF reset */
    u32 flr_rst_cnt;       /* the number of FLR */
    u32 core_rst_cnt;      /* the number of CORE reset */
    u32 global_rst_cnt;    /* the number of GLOBAL */
    u32 imp_rst_cnt;       /* the number of IMP reset */
    u32 reset_cnt;         /* the number of reset */
    u32 reset_fail_cnt;    /* the number of reset fail */
};

/* time and register status when mac tunnel interruption occur */
struct hclge_mac_tnl_stats {
    u64 time;
    u32 status;
};

#define HCLGE_RESET_INTERVAL (12 * HZ)
#define HCLGE_WAIT_RESET_DONE 100

#pragma pack(1)
struct hclge_vf_vlan_cfg {
    u8 mbx_cmd;
    u8 subcode;
    u8 is_kill;
    u16 vlan;
    u16 proto;
};

#pragma pack()

struct hclge_plf_misc_vector {
    u8 __iomem *addr;
    int vector_irq;
    char name[HNAE3_INT_NAME_LEN];
};

#define HCLGE_MAC_TNL_LOG_SIZE 8
#define HCLGE_PLF_VPORT_NUM 256

#define USER_BC_MC_ENTRY_NUM 9 /* 8 user + 1 multicast = 9 */

#define VLAN_TABLE_NUM 5

struct vlan_table_cfg {
    u32 loc;
    u16 vlan_id;
};

#define HCLGE_PLF_CRC_FLAG 8
#define HCLGE_PLF_CRC_CL 0x01
#define HCLGE_PLF_CRC_CH 0xA0
#define HCLGE_PLF_CRC16 0xFF
#define HCLGE_PLF_CRC_CODE 0x80
#define HCLGE_PLF_CRC_DATA_LEN 9

#define HCLGE_PLF_KEPP_ADDR_LEN 5
struct hclge_plf_mac_info {
    u16 crc_value;
    u8 data_length;
    u8 mac_id;
    u8 mac_type;
    u8 mac_addr[ETH_ALEN];
    u8 keep_addr[HCLGE_PLF_KEPP_ADDR_LEN];
};

struct hclge_plf_dev {
    struct platform_device *pdev;
    struct hnae3_ae_dev *ae_dev;
    struct hclge_plf_hw hw;
    struct hclge_plf_misc_vector misc_vector[PLATFORM_MISC_INT_NUM];
    struct hclge_plf_mac_stats mac_stats;
    unsigned long state;
    unsigned long flr_state;
    unsigned long last_reset_time;

    enum hnae3_reset_type reset_type;
    enum hnae3_reset_type reset_level;
    unsigned long default_reset_request;
    unsigned long reset_request; /* reset has been requested */
    unsigned long reset_pending; /* client rst is pending to be served */
    struct hclge_plf_rst_stats rst_stats;
    struct semaphore reset_sem; /* protect reset process */
    u32 reset_fail_cnt;
    u32 fw_version;
    u16 num_tqps;    /* Num task queue pairs of this PF */
    u16 tqp_offset;

    u16 base_tqp_pid;      /* Base task tqp physical id of this PF */
    u16 alloc_rss_size;    /* Allocated RSS task queue */
    u16 vf_rss_size_max;   /* HW defined VF max RSS task queue */
    u16 pf_rss_size_max;   /* HW defined PF max RSS task queue */
    u32 tx_spare_buf_size; /* HW defined TX spare buffer size */

    u16 fdir_pf_filter_count; /* Num of guaranteed filters for this PF */
    u16 num_alloc_vport;      /* Num vports this driver supports */
    u32 numa_node_mask;
    u16 rx_buf_len;
    u16 num_tx_desc; /* desc num of per tx queue */
    u16 num_rx_desc; /* desc num of per rx queue */
    u8 hw_tc_map;
    u8 tc_num_last_time;
    enum hclge_plf_fc_mode fc_mode_last_time;
    bool support_sfp_query;
    bool ppu_poison_ras_err;
    unsigned long imp_err_state;

#define HCLGE_FLAG_TC_BASE_SCH_MODE 1
#define HCLGE_FLAG_VNET_BASE_SCH_MODE 2
    u8 tx_sch_mode;
    u8 pfc_max;
    u8 id; // port id
    u8 tc_max;
    u8 tc_offset;

    u8 default_up;
    u8 dcbx_cap;
    struct hclge_plf_tm_info tm_info;

    u16 num_msi;
    u16 num_msi_left;
    u16 num_msi_used;
    u16 *vector_status;
    int *vector_irq;
    u16 num_nic_msi;  /* Num of nic vectors for this PF */
    u16 num_roce_msi; /* Num of roce vectors for this PF */
    u16 num_roh_msi;  /* Num of roh vectors for this PF */
    int queue_irq[PLATFORM_TQP_INT_NUM];
    int abn_irq;
    int mac_irq;
    int rtc_irq;

    u16 pending_udp_bitmap;

    u16 rx_itr_default;
    u16 tx_itr_default;

    u16 adminq_work_limit; /* Num of admin receive queue desc to process */
    unsigned long service_timer_period;
    unsigned long service_timer_previous;
    struct timer_list reset_timer;
    struct delayed_work service_task;

    bool cur_promisc;
    int num_alloc_vfs; /* Actual number of VFs allocated */

    struct hclge_plf_tqp *htqp;
    struct hclge_plf_vport *vport;

    struct dentry *hclge_dbgfs;

    struct hnae3_client *nic_client;
    struct hnae3_client *roce_client;
    struct hnae3_client *roh_client;

#define HCLGE_FLAG_MAIN BIT(0)
#define HCLGE_FLAG_DCB_CAPABLE BIT(1)
#define HCLGE_FLAG_DCB_ENABLE BIT(2)
#define HCLGE_FLAG_MQPRIO_ENABLE BIT(3)
    u32 flag;

    u32 pkt_buf_size; /* Total pf buf size for tx/rx */
    u32 tx_buf_size;  /* Tx buffer size for each TC */
    u32 dv_buf_size;  /* Dv buffer size for each TC */

    int mps; /* Max packet size */
    /* vport_lock protect resource shared by vports */
    struct mutex vport_lock;

    unsigned long vlan_table[VLAN_N_VID][BITS_TO_LONGS(HCLGE_PLF_VPORT_NUM)];
    unsigned long vf_vlan_full[BITS_TO_LONGS(HCLGE_PLF_VPORT_NUM)];

    unsigned long vport_config_block[BITS_TO_LONGS(HCLGE_PLF_VPORT_NUM)];

    struct hclge_plf_fd_cfg fd_cfg;
    struct hlist_head fd_rule_list;
    spinlock_t fd_rule_lock; /* protect fd_rule_list and fd_bmap */
    u16 hclge_fd_rule_num;
    u16 hclge_fd_user_rule_num;
    unsigned long serv_processed_cnt;
    unsigned long last_serv_processed;
    unsigned long fd_bmap[BITS_TO_LONGS(MAX_FD_FILTER_NUM)];
    enum hclge_fd_active_rule_type fd_active_type;
    u8 fd_en;

    u8 vlan_mode;

    u16 wanted_umv_size;
    /* max available unicast mac vlan space */
    u16 max_umv_size;
    /* private unicast mac vlan space, it's same for PF and its VFs */
    u16 priv_umv_size;
    /* unicast mac vlan space shared by PF and its VFs */
    u16 share_umv_size;

    DECLARE_KFIFO(mac_tnl_log, struct hclge_mac_tnl_stats, HCLGE_MAC_TNL_LOG_SIZE);

    /* affinity mask and notify for misc interrupt */
    cpumask_t affinity_mask;
    struct irq_affinity_notify affinity_notify;

    u32 rss_indir_tbl_offset;
    u32 init_flag;

    spinlock_t vlan_table_lock;
    struct vlan_table_cfg vlan_cfg_tbl[VLAN_TABLE_NUM - 1];

    bool promisc_en;
};

typedef struct hclge_plf_dev compat_hdev;

/* VPort level vlan tag configuration for TX direction */
struct hclge_tx_vtag_cfg {
    bool accept_tag1;   /* Whether accept tag1 packet from host */
    bool accept_untag1; /* Whether accept untag1 packet from host */
    bool accept_tag2;
    bool accept_untag2;
    bool insert_tag1_en; /* Whether insert inner vlan tag */
    bool insert_tag2_en; /* Whether insert outer vlan tag */
    u16 default_tag1;    /* The default inner vlan tag to insert */
    u16 default_tag2;    /* The default outer vlan tag to insert */
    bool tag_shift_mode_en;
};

/* VPort level vlan tag configuration for RX direction */
struct hclge_rx_vtag_cfg {
    bool rx_vlan_offload_en;    /* Whether enable rx vlan offload */
    bool strip_tag1_en;         /* Whether strip inner vlan tag */
    bool strip_tag2_en;         /* Whether strip outer vlan tag */
    bool vlan1_vlan_prionly;    /* Inner VLAN Tag up to descriptor Enable */
    bool vlan2_vlan_prionly;    /* Outer VLAN Tag up to descriptor Enable */
    bool strip_tag1_discard_en; /* Inner VLAN Tag discard for BD enable */
    bool strip_tag2_discard_en; /* Outer VLAN Tag discard for BD enable */
};

struct hclge_rss_tuple_cfg {
    u8 ipv4_tcp_en;
    u8 ipv4_udp_en;
    u8 ipv4_sctp_en;
    u8 ipv4_fragment_en;
    u8 ipv6_tcp_en;
    u8 ipv6_udp_en;
    u8 ipv6_sctp_en;
    u8 ipv6_fragment_en;
};

#define HCLGE_MAC_TBL_SYNC_INTERVAL 3U

enum HCLGE_VPORT_STATE {
    HCLGE_VPORT_STATE_ALIVE,
    HCLGE_VPORT_STATE_MAC_TBL_CHANGE,
    HCLGE_VPORT_STATE_PROMISC_CHANGE,
    HCLGE_VPORT_STATE_QB_CHANGE,
    HCLGE_VPORT_STATE_MAX
};

struct hclge_vlan_info {
    u16 vlan_proto; /* so far support 802.1Q only */
    u16 qos;
    u16 vlan_tag;
};

struct hclge_port_base_vlan_config {
    u16 state;
    bool tbl_sta;
    struct hclge_vlan_info vlan_info;
    struct hclge_vlan_info old_vlan_info;
};

struct hclge_vf_info {
    int link_state;
    u8 mac[ETH_ALEN];
    u32 spoofchk;
    u32 max_tx_rate;
    u32 trusted;
    u8 request_uc_en;
    u8 request_mc_en;
    u8 request_bc_en;
    u8 request_qb_en;
    u8 qb_en;
};

struct hclge_plf_vport {
    u16 alloc_tqps; /* Allocated Tx/Rx queues */
    /* User configured hash keys */
    u8 rss_hash_key[HCLGE_PLF_RSS_KEY_SIZE_MAX];
    /* User configured lookup table entries */
    u16 rss_indirection_tbl[HCLGE_PLF_RSS_IND_TBL_SIZE_MAX];
    int rss_algo; /* User configured hash algorithm */
    /* User configured rss tuple sets */
    struct hclge_rss_tuple_cfg rss_tuple_sets;

    u16 alloc_rss_size;

    u16 qs_offset;
    u32 bw_limit; /* VSI BW Limit (0 = disabled) */
    u8 dwrr;

    bool vf_vlan_en;
    unsigned long vlan_del_fail_bmap[BITS_TO_LONGS(VLAN_N_VID)];
    struct hclge_port_base_vlan_config port_base_vlan_cfg;
    struct hclge_tx_vtag_cfg txvlan_cfg;
    struct hclge_rx_vtag_cfg rxvlan_cfg;

    u16 used_umv_num;

    u16 vport_id;
    struct hclge_plf_dev *back; /* Back reference to associated dev */
    struct hnae3_handle nic;
    struct hnae3_handle roce;
    struct hnae3_handle roh;

    unsigned long state;
    unsigned long last_active_jiffies;
    int mps; /* Max packet size */
    struct hclge_vf_info vf_info;

    u8 overflow_promisc_flags;
    u8 last_promisc_flags;

    spinlock_t mac_list_lock;     /* protect mac address need to add/detele */
    struct list_head uc_mac_list; /* Store VF unicast table */
    struct list_head mc_mac_list; /* Store VF multicast table */
    struct list_head vlan_list;   /* Store VF vlan table */
};

int xxvge_mag_init(struct hclge_plf_dev *hdev);
int hns_mac_mdio_init(struct hclge_plf_dev *hdev);
int phy_hw_init(struct hclge_plf_dev *hdev);
int mac_get_link_status(struct hclge_plf_dev *hdev);
int mag_set_port_enable(struct hclge_plf_dev *hdev);
int mag_set_port_disable(struct hclge_plf_dev *hdev);
int hclge_plf_reset_tqp(struct hnae3_handle *handle);
struct hclge_plf_vport *hclge_plf_get_vport(struct hnae3_handle *handle);

int hclge_plf_buffer_alloc(struct hclge_plf_dev *hdev);
void hclge_plf_rss_indir_init_cfg(struct hclge_plf_dev *hdev);
int hclge_plf_rss_init_hw(struct hclge_plf_dev *hdev);
int hclge_plf_notify_client(struct hclge_plf_dev *hdev,
    enum hnae3_reset_notify_type type);
int wait_hardware_done(void __iomem *reg, u32 val_mask, u32 value, u32 interval, u32 cnt);

extern int hns3_client_init(struct hnae3_handle *handle);
extern void hns3_client_uninit(struct hnae3_handle *handle, bool reset);
extern void hns3_link_status_change(struct hnae3_handle *handle, bool linkup);
extern int hns3_reset_notify(struct hnae3_handle *handle, enum hnae3_reset_notify_type type);
extern void hns3_process_hw_error(struct hnae3_handle *handle, enum hnae3_hw_error_type type);

extern int memset_s(void *dest, size_t destMax, int c, size_t count);
extern int memcpy_s(void *dest, size_t destMax, const void *src, size_t count);
extern int strncat_s(char *strDest, size_t destMax, const char *strSrc, size_t count);
extern int strncpy_s(char *strDest, size_t destMax, const char *strSrc, size_t count);
extern int sscanf_s(const char *buffer, const char *format, ...);
extern int snprintf_s(char *strDest, size_t destMax, size_t count, const char *format, ...);
extern int vsnprintf_s(char *strDest, size_t destMax, size_t count, const char *format, va_list argList);
void hclge_plf_task_schedule(struct hclge_plf_dev *hdev, unsigned long delay_time);
int hclge_plf_dbg_read_cmd(struct hnae3_handle *handle, enum hnae3_dbg_cmd cmd, char *buf, int len);
void hclge_plf_init_vlan_config(struct hclge_plf_dev *hdev);
struct platform_device *hclge_get_platform_device(u8 port_id);
u8 hclge_get_chip_port_nums(void);
u32 hclge_get_id_map(void);
u8 hclge_plf_get_first_probe_id(void);
#ifdef CONFIG_PLATFORM_ASIC
int hclge_plf_mac_connect_phy(struct hnae3_handle *handle);
void hclge_plf_mac_disconnect_phy(struct hnae3_handle *handle);
int hclge_plf_cfg_mac_speed_dup(struct hclge_plf_dev *hdev, int speed, u8 duplex);
void hclge_plf_mac_start_phy(struct hclge_plf_dev *hdev);
void hclge_plf_mac_stop_phy(struct hclge_plf_dev *hdev);
int hclge_plf_enable_phy_loopback(struct hclge_plf_dev *hdev, struct phy_device *phydev);
int hclge_plf_disable_phy_loopback(struct hclge_plf_dev *hdev, struct phy_device *phydev);
int hclge_plf_cfg_flowctrl(struct hclge_plf_dev *hdev);
int hclge_plf_get_phy_link(struct phy_device *phydev);
enum hnae3_reset_type hclge_plf_get_reset_level(struct hnae3_ae_dev *ae_dev, unsigned long *addr);
void hclge_plf_set_def_reset_request(struct hnae3_ae_dev *ae_dev, enum hnae3_reset_type rst_type);
struct hclge_plf_dev *hclge_get_hclge_plf_dev(u8 port_id);
struct hclge_plf_dev *hclge_get_first_hclge_plf_dev(void);
void hclge_plf_rgmii_clk_cfg(struct hclge_plf_dev *hdev);

#endif
#endif // __HCLGE_PLF_MAIN_H
