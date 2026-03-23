/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: hclge plf main
 * Author: huawei
 * Create: 2021-12-28
 */

#include <linux/acpi.h>
#include <linux/device.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/if_vlan.h>
#include <linux/crash_dump.h>
#include <net/ipv6.h>
#include <net/rtnetlink.h>
#include <linux/of.h>
#include <linux/vmalloc.h>
#include <linux/kallsyms.h>
#include "platform_mac.h"
#include "hclge_plf_err.h"
#include "hclge_plf_bbox.h"
#include "hclge_plf_qos.h"
#include "hclge_plf_main.h"

#include "kcompat.h"
#include "hnae3.h"
#include "reg_rcb_com.h"
#include "reg_ppp.h"
#include "gpio.h"
#include "reg_top_offset.h"

#ifdef PLATFORM_SUPPORT_EEPROM_MAC
#include "eeprom_M24256.h"
#endif

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

STATIC struct hnae3_client platform_client;

STATIC const struct hnae3_client_ops platform_client_ops = {
    .init_instance = hns3_client_init,
    .uninit_instance = hns3_client_uninit,
    .link_status_change = hns3_link_status_change,
    .reset_notify = hns3_reset_notify,
    .process_hw_error = hns3_process_hw_error,
};

STATIC const char hns3_platform_drv_name[] = "hns3-platform";
STATIC const char hns3_platform_drv_string[] = "Hisilicon Ethernet Network Driver for hns3-platform Family";
STATIC const char hns3_platform_copyright[] = "Copyright (c) 2021 Huawei Corporation.";

STATIC struct hnae3_ae_algo ae_algo_plf;

STATIC struct workqueue_struct *hclge_plf_wq;

#define HCLGE_PLF_NAME "hclge_plf"

STATIC atomic_t g_hclge_plf_first_probe_flag;
STATIC atomic_t g_hclge_plf_first_probe_id;
STATIC u32 g_hclge_plf_id_map;
STATIC atomic_t g_hclge_plf_tqp_offset;
STATIC atomic_t g_hclge_plf_tc_offset;

STATIC const struct hclge_comm_stats_str g_mac_stats_string[] = {
    { "mac_tx_pkts_fragment", 0, 0 },
    { "mac_tx_pkts_undersize", 0, 0 },
    { "mac_tx_pkts_undermin", 0, 0 },
    { "mac_tx_pkts_64octets", 0, 0 },
    { "mac_tx_pkts_65to127octets", 0, 0 },
    { "mac_tx_pkts_128to255octets", 0, 0 },
    { "mac_tx_pkts_256to511octets", 0, 0 },
    { "mac_tx_pkts_512to1023octets", 0, 0 },
    { "mac_tx_pkts_1024to1518octets", 0, 0 },
    { "mac_tx_pkts_1519to2047octets", 0, 0 },
    { "mac_tx_pkts_2048to4095octets", 0, 0 },
    { "mac_tx_pkts_4096to8191octets", 0, 0 },
    { "mac_tx_pkts_8192to9216octets", 0, 0 },
    { "mac_tx_pkts_9217to12287octets", 0, 0 },
    { "mac_tx_pkts_12288to16383octets", 0, 0 },
    { "mac_tx_pkts_1519tomaxoctetsbad", 0, 0 },
    { "mac_tx_pkts_1519tomaxoctetsgood", 0, 0 },
    { "mac_tx_pkts_oversize", 0, 0 },
    { "mac_tx_pkts_jabber", 0, 0 },
    { "mac_tx_bad_pkts", 0, 0 },
    { "mac_tx_bad_octets", 0, 0 },
    { "mac_tx_good_pkts", 0, 0 },
    { "mac_tx_good_octets", 0, 0 },
    { "mac_tx_total_pkts", 0, 0 },
    { "mac_tx_total_octets", 0, 0 },
    { "mac_tx_unicast_pkts", 0, 0 },
    { "mac_tx_multicast_pkts", 0, 0 },
    { "mac_tx_broadcast_pkts", 0, 0 },
    { "mac_tx_pause_pkts", 0, 0 },
    { "mac_tx_pfc_pkts", 0, 0 },
    { "mac_tx_pri0_pause_pkts", 0, 0 },
    { "mac_tx_pri1_pause_pkts", 0, 0 },
    { "mac_tx_pri2_pause_pkts", 0, 0 },
    { "mac_tx_pri3_pause_pkts", 0, 0 },
    { "mac_tx_pri4_pause_pkts", 0, 0 },
    { "mac_tx_pri5_pause_pkts", 0, 0 },
    { "mac_tx_pri6_pause_pkts", 0, 0 },
    { "mac_tx_pri7_pause_pkts", 0, 0 },
    { "mac_tx_macctrl_pkts", 0, 0 },
    { "mac_tx_1731_pkts", 0, 0 },
    { "mac_tx_1588_pkts", 0, 0 },
    { "mac_tx_err_allpkts", 0, 0 },
    { "mac_tx_from_app_good_pkts", 0, 0 },
    { "mac_tx_from_app_bad_pkts", 0, 0 },
    { "mac_rx_pkts_fragment", 0, 0 },
    { "mac_rx_pkts_undersize", 0, 0 },
    { "mac_rx_pkts_undermin", 0, 0 },
    { "mac_rx_pkts_64octets", 0, 0 },
    { "mac_rx_pkts_65to127octets", 0, 0 },
    { "mac_rx_pkts_128to255octets", 0, 0 },
    { "mac_rx_pkts_256to511octets", 0, 0 },
    { "mac_rx_pkts_512to1023octets", 0, 0 },
    { "mac_rx_pkts_1024to1518octets", 0, 0 },
    { "mac_rx_pkts_1519to2047octets", 0, 0 },
    { "mac_rx_pkts_2048to4095octets", 0, 0 },
    { "mac_rx_pkts_4096to8191octets", 0, 0 },
    { "mac_rx_pkts_8192to9216octets", 0, 0 },
    { "mac_rx_pkts_9217to12287octets", 0, 0 },
    { "mac_rx_pkts_12288to16383octets", 0, 0 },
    { "mac_rx_pkts_1519tomaxoctetsbad", 0, 0 },
    { "mac_rx_pkts_1519tomaxoctetsgood", 0, 0 },
    { "mac_rx_pkts_oversize", 0, 0 },
    { "mac_rx_pkts_jabber", 0, 0 },
    { "mac_rx_bad_pkts", 0, 0 },
    { "mac_rx_bad_octets", 0, 0 },
    { "mac_rx_good_pkts", 0, 0 },
    { "mac_rx_good_octets", 0, 0 },
    { "mac_rx_total_pkts", 0, 0 },
    { "mac_rx_total_octets", 0, 0 },
    { "mac_rx_unicast_pkts", 0, 0 },
    { "mac_rx_multicast_pkts", 0, 0 },
    { "mac_rx_broadcast_pkts", 0, 0 },
    { "mac_rx_pause_pkts", 0, 0 },
    { "mac_rx_pfc_pkts", 0, 0 },
    { "mac_rx_pri0_pause_pkts", 0, 0 },
    { "mac_rx_pri1_pause_pkts", 0, 0 },
    { "mac_rx_pri2_pause_pkts", 0, 0 },
    { "mac_rx_pri3_pause_pkts", 0, 0 },
    { "mac_rx_pri4_pause_pkts", 0, 0 },
    { "mac_rx_pri5_pause_pkts", 0, 0 },
    { "mac_rx_pri6_pause_pkts", 0, 0 },
    { "mac_rx_pri7_pause_pkts", 0, 0 },
    { "mac_rx_macctrl_pkts", 0, 0 },
    { "mac_rx_1731_pkts", 0, 0 },
    { "mac_rx_symbol_err_pkts", 0, 0 },
    { "mac_rx_fcs_err_pkts", 0, 0 },
    { "mac_rx_align_err_pkts", 0, 0 },
    { "mac_rx_send_app_good_pkts", 0, 0 },
    { "mac_rx_send_app_bad_pkts", 0, 0 },
    { "mac_rx_segment_octets", 0, 0 },
    { "mac_tx_deferral_frames", 0, 0 },
    { "mac_tx_singlecollision_frames", 0, 0 },
    { "mac_tx_multiplecollision_frames", 0, 0 },
    { "mac_tx_excessivecollision_frames", 0, 0 },
    { "mac_tx_latecollision_frames", 0, 0 },
    { "mac_tx_crslost_frames", 0, 0 },
    { "mac_tx_mergeframeasserror_pkts", 0, 0 },
    { "mac_tx_mergeframeassok_pkts", 0, 0 },
    { "mac_tx_mergeframefrag_count", 0, 0 },
    { "mac_rx_mergeframeasserror_pkts", 0, 0 },
    { "mac_rx_mergeframeassok_pkts", 0, 0 },
    { "mac_rx_mergeframefrag_count", 0, 0 },
    { "mac_rx_mergeframesmderror_pkts", 0, 0 }
};

static void hclge_plf_request_update_promisc_mode(struct hnae3_handle *handle);

u8 hclge_get_chip_port_nums(void)
{
    u32 id_map = g_hclge_plf_id_map;
    u8 chip_port_nums = 0;

    while (id_map > 0) {
        chip_port_nums++;
        id_map &= (id_map - 1);
    }

    return chip_port_nums;
}

u32 hclge_get_id_map(void)
{
    return g_hclge_plf_id_map;
}

u8 hclge_plf_get_first_probe_id(void)
{
    return (u8)atomic_read(&g_hclge_plf_first_probe_id);
}

struct hclge_plf_dev *hclge_get_hclge_plf_dev(u8 port_id)
{
    struct list_head *ae_dev_list = NULL;
    struct hnae3_ae_dev *ae_dev2 = NULL;
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hclge_plf_dev *hdev = NULL;

    ae_dev_list = hnae3_get_ae_dev_list();
    list_for_each_entry_safe(ae_dev, ae_dev2, ae_dev_list, node) {
        if (ae_dev->priv == NULL) {
            pr_err("hns3: ae_dev->priv is NULL.\n");
            hdev = NULL;
            break;
        }
        hdev = ae_dev->priv;
        if (hdev->id == port_id) {
            break;
        }
        hdev = NULL;
    }
    hnae3_put_ae_dev_list();
    return hdev;
}

struct hclge_plf_dev *hclge_get_first_hclge_plf_dev(void)
{
    u8 first_probe_id;

    first_probe_id = hclge_plf_get_first_probe_id();
    return hclge_get_hclge_plf_dev(first_probe_id);
}

struct platform_device *hclge_get_platform_device(u8 port_id)
{
    struct platform_device *pdev = NULL;
    struct hclge_plf_dev *hdev = NULL;

    hdev = hclge_get_hclge_plf_dev(port_id);
    if (hdev == NULL) {
        pr_err("hns3: hclge get hclge plf dev fail.\n");
        return NULL;
    }
    pdev = hdev->pdev;
    return pdev;
}

int hclge_plf_buffer_alloc(struct hclge_plf_dev *hdev)
{
    return 0;
}

STATIC int hclge_plf_parse_speed(u8 speed_cmd, u32 *speed)
{
    switch (speed_cmd) {
        case HCLGE_FW_MAC_SPEED_10M:
            *speed = XXVGE_MAC_SPEED_10M;
            break;
        case HCLGE_FW_MAC_SPEED_100M:
            *speed = XXVGE_MAC_SPEED_100M;
            break;
        case HCLGE_FW_MAC_SPEED_1G:
            *speed = XXVGE_MAC_SPEED_1G;
            break;
        case HCLGE_FW_MAC_SPEED_2_5G:
            *speed = XXVGE_MAC_SPEED_2_5G;
            break;
        default:
            return -EINVAL;
    }

    return 0;
}

STATIC void hclge_plf_get_cfg(struct hclge_plf_dev *hdev, struct hclge_plf_cfg *hcfg)
{
#define SPEED_ABILITY_EXT_S 8

    u64 mac_addr_tmp_high;
    u64 mac_addr_tmp;
    u32 i;

    /* get the configuration */
    hcfg->media_type = PORT_MT_PHY;
    hcfg->rx_buf_len = DEF_PLATFORM_RX_BUF_LEN;

    /* gen mac address */
    mac_addr_tmp = DEF_PLATFORM_MAC_ADDR_LOW;
    mac_addr_tmp_high = DEF_PLATFORM_MAC_ADDR_HIGH;
    mac_addr_tmp |= (mac_addr_tmp_high << 32);  // mac high 16bit shift 32

    hcfg->vf_rss_size_max = 0;

    for (i = 0; i < ETH_ALEN; i++)
        hcfg->mac_addr[i] = (mac_addr_tmp >> (8 * i)) & 0xff;  // each mac size is 8bit

    hcfg->numa_node_map = DEF_PLATFORM_NUMA_NODE_MAP;

    hcfg->speed_ability = HCLGE_PLF_SUPPORT_GE;

    hcfg->vlan_mode_sel = DEF_PLATFORM_VLAN_MODE;

    hcfg->umv_space = 0;
    hcfg->pf_rss_size_max = hdev->num_tqps;
    hcfg->tx_spare_buf_size = 0;
}

static void hclge_plf_set_basic_cfg(struct hclge_plf_dev *hdev, struct hclge_plf_cfg *cfg)
{
    hdev->base_tqp_pid = 0;
    hdev->vf_rss_size_max = cfg->vf_rss_size_max;
    hdev->pf_rss_size_max = cfg->pf_rss_size_max;
    hdev->rx_buf_len = cfg->rx_buf_len;
    ether_addr_copy(hdev->hw.mac.mac_addr, cfg->mac_addr);
    hdev->hw.mac.media_type = cfg->media_type;
    hdev->tm_info.num_pg = 1;
    hdev->tm_info.hw_pfc_map = 0;
    hdev->wanted_umv_size = cfg->umv_space;
    hdev->vlan_mode = cfg->vlan_mode_sel;
    hdev->tx_spare_buf_size = cfg->tx_spare_buf_size;
}

static void hclge_plf_parse_copper_link_mode(struct hclge_plf_dev *hdev, u16 speed_ability)
{
    unsigned long *supported = hdev->hw.mac.supported;

    /* default to support all speed for GE port */
    if (!speed_ability) {
        speed_ability = HCLGE_PLF_SUPPORT_GE;
    }

    if (speed_ability & HCLGE_PLF_SUPPORT_1G_BIT) {
        linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT, supported);
    }

    if (speed_ability & HCLGE_PLF_SUPPORT_100M_BIT) {
        linkmode_set_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT, supported);
    }

    if (speed_ability & HCLGE_PLF_SUPPORT_10M_BIT) {
        linkmode_set_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT, supported);
    }

    linkmode_set_bit(ETHTOOL_LINK_MODE_Pause_BIT, supported);
    linkmode_set_bit(ETHTOOL_LINK_MODE_Asym_Pause_BIT, supported);

    if (hdev->hw.mac.support_autoneg == SUPPORT_AUTONEG_ON) {
        linkmode_set_bit(ETHTOOL_LINK_MODE_Autoneg_BIT, supported);
    }
}

static void hclge_plf_parse_link_mode(struct hclge_plf_dev *hdev, u16 speed_ability)
{
    u8 media_type = hdev->hw.mac.media_type;

    if (media_type == HNAE3_MEDIA_TYPE_COPPER)
        hclge_plf_parse_copper_link_mode(hdev, speed_ability);
}

STATIC void hclge_plf_configure(struct hclge_plf_dev *hdev)
{
    const struct cpumask *cpumask = cpu_online_mask;
    struct hclge_plf_cfg cfg;
    int node;
    u32 i;

    hclge_plf_get_cfg(hdev, &cfg);

    hclge_plf_set_basic_cfg(hdev, &cfg);

    hclge_plf_parse_link_mode(hdev, cfg.speed_ability);

    if (hnae3_dev_fd_supported(hdev)) {
        hdev->fd_en = true;
        hdev->fd_active_type = HCLGE_FD_RULE_NONE;
    }

    if ((hdev->tc_max > HNAE3_MAX_TC) || (hdev->tc_max < 1)) {
        dev_warn(&hdev->pdev->dev, "TC num = %u.\n", hdev->tc_max);
        hdev->tc_max = 1;
    }

    /* Dev does not support DCB */
    if (!hnae3_dev_dcb_supported(hdev)) {
        hdev->tc_max = 1;
        hdev->pfc_max = 0;
    } else {
        hdev->pfc_max = hdev->tc_max;
    }

    hdev->tm_info.num_tc = hdev->tc_max;

    /* Currently not support uncontiuous tc */
    for (i = 0; i < hdev->tm_info.num_tc; i++)
        hnae3_set_bit(hdev->hw_tc_map, i, 1);

    hdev->tx_sch_mode = HCLGE_FLAG_TC_BASE_SCH_MODE;

    /* Set the init affinity based on pci func number */
    node = dev_to_node(&hdev->pdev->dev);
    if (node != NUMA_NO_NODE)
        cpumask = cpumask_of_node(node);

    i = 0;
    cpumask_set_cpu(cpumask_local_spread(i, dev_to_node(&hdev->pdev->dev)), &hdev->affinity_mask);
}

STATIC int hclge_plf_alloc_tqps(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_tqp *tqp;
    u16 i;

    hdev->htqp = devm_kcalloc(&hdev->pdev->dev, hdev->num_tqps, sizeof(struct hclge_plf_tqp), GFP_KERNEL);
    if (!hdev->htqp) {
        dev_err(&hdev->pdev->dev, "alloc tqp memory failed.\n");
        return -ENOMEM;
    }

    tqp = hdev->htqp;

    for (i = 0; i < hdev->num_tqps; i++) {
        tqp->dev = &hdev->pdev->dev;
        tqp->index = i;

        tqp->q.ae_algo = &ae_algo_plf;
        tqp->q.buf_size = hdev->rx_buf_len;
        tqp->q.tx_desc_num = hdev->num_tx_desc;
        tqp->q.rx_desc_num = hdev->num_rx_desc;
        tqp->q.io_base = hdev->hw.io_base + PLATFORM_TQP_REG_OFFSET + (i + hdev->tqp_offset) * PLATFORM_TQP_REG_SIZE;

        tqp++;
    }

    return 0;
}

STATIC void hclge_plf_assign_tqp(struct hclge_plf_vport *vport, u16 num_tqps)
{
    struct hnae3_knic_private_info *kinfo = &vport->nic.kinfo;
    struct hclge_plf_dev *hdev = vport->back;
    int alloced;
    u16 i;

    for (i = 0, alloced = 0; i < hdev->num_tqps && alloced < num_tqps; i++) {
        if (!hdev->htqp[i].alloced) {
            hdev->htqp[i].q.handle = &vport->nic;
            hdev->htqp[i].q.tqp_index = alloced;
            hdev->htqp[i].q.tx_desc_num = kinfo->num_tx_desc;
            hdev->htqp[i].q.rx_desc_num = kinfo->num_rx_desc;
            kinfo->tqp[alloced] = &hdev->htqp[i].q;
            hdev->htqp[i].alloced = true;
            alloced++;
        }
    }
    vport->alloc_tqps = alloced;
    kinfo->rss_size = min_t(u16, hdev->pf_rss_size_max, vport->alloc_tqps / hdev->tm_info.num_tc);

    /* ensure one to one mapping between irq and queue at default */
    kinfo->rss_size = min_t(u16, kinfo->rss_size, (hdev->num_nic_msi - 1) / hdev->tm_info.num_tc);
}

STATIC int hclge_plf_knic_setup(struct hclge_plf_vport *vport, u16 num_tqps, u16 num_tx_desc, u16 num_rx_desc)
{
    struct hnae3_handle *nic = &vport->nic;
    struct hnae3_knic_private_info *kinfo = &nic->kinfo;
    struct hclge_plf_dev *hdev = vport->back;

    kinfo->num_tx_desc = num_tx_desc;
    kinfo->num_rx_desc = num_rx_desc;

    kinfo->rx_buf_len = hdev->rx_buf_len;
    kinfo->tx_spare_buf_size = hdev->tx_spare_buf_size;

    kinfo->tqp = devm_kcalloc(&hdev->pdev->dev, num_tqps, sizeof(struct hnae3_queue *), GFP_KERNEL);
    if (!kinfo->tqp) {
        dev_err(&hdev->pdev->dev, "alloc hnae3_queue memory failed.\n");
        return -ENOMEM;
    }

    hclge_plf_assign_tqp(vport, num_tqps);

    return 0;
}

STATIC int hclge_plf_vport_setup(struct hclge_plf_vport *vport, u16 num_tqps)
{
    struct hclge_plf_dev *hdev = vport->back;
    struct hnae3_handle *nic = &vport->nic;
    int ret;

    nic->plfdev = hdev->pdev;
    nic->ae_algo = &ae_algo_plf;
    nodes_clear(nic->numa_node_mask);
    if (hdev->numa_node_mask < MAX_NUMNODES)
        node_set(hdev->numa_node_mask, nic->numa_node_mask);
    nic->kinfo.io_base = hdev->hw.io_base;
    nic->flags |= HNAE3_SUPPORT_PLATFORM_DEV;

    ret = hclge_plf_knic_setup(vport, num_tqps, hdev->num_tx_desc, hdev->num_rx_desc);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "knic setup failed %d\n", ret);
    }

    return ret;
}

STATIC int hclge_plf_alloc_vport(struct hclge_plf_dev *hdev)
{
    struct platform_device *pdev = hdev->pdev;
    struct hclge_plf_vport *vport;
    int ret;

    vport = devm_kcalloc(&pdev->dev, 1, sizeof(struct hclge_plf_vport), GFP_KERNEL);
    if (!vport) {
        dev_err(&hdev->pdev->dev, "alloc vport mem failed.\n");
        return -ENOMEM;
    }

    hdev->vport = vport;
    hdev->num_alloc_vport = 1;

    vport->back = hdev;
    vport->vf_info.link_state = IFLA_VF_LINK_STATE_AUTO;
    vport->mps = HCLGE_MAC_DEFAULT_FRAME;
    vport->port_base_vlan_cfg.state = HNAE3_PORT_BASE_VLAN_DISABLE;
    vport->port_base_vlan_cfg.tbl_sta = true;
    vport->rxvlan_cfg.rx_vlan_offload_en = true;
    INIT_LIST_HEAD(&vport->vlan_list);
    INIT_LIST_HEAD(&vport->uc_mac_list);
    INIT_LIST_HEAD(&vport->mc_mac_list);
    spin_lock_init(&vport->mac_list_lock);

    ret = hclge_plf_vport_setup(vport, hdev->num_tqps);
    if (ret != 0) {
        dev_err(&pdev->dev, "vport setup failed for vport, %d\n", ret);
        return ret;
    }

    return 0;
}

STATIC void hclge_plf_mac_int_enable(struct hclge_plf_dev *hdev)
{
    mac_cfg_common_link_up_int(hdev, 1);
    /* check link status */
    mac_link_up_waiting(hdev);
}

STATIC void hclge_plf_state_init(struct hclge_plf_dev *hdev)
{
    set_bit(HCLGE_STATE_SERVICE_INITED, &hdev->state);
    set_bit(HCLGE_STATE_DOWN, &hdev->state);
    clear_bit(HCLGE_STATE_RST_SERVICE_SCHED, &hdev->state);
    clear_bit(HCLGE_STATE_RST_HANDLING, &hdev->state);
    clear_bit(HCLGE_STATE_RST_FAIL, &hdev->state);
    clear_bit(HCLGE_STATE_MBX_SERVICE_SCHED, &hdev->state);
    clear_bit(HCLGE_STATE_MBX_HANDLING, &hdev->state);
}

STATIC void hclge_plf_state_uninit(struct hclge_plf_dev *hdev)
{
    set_bit(HCLGE_STATE_DOWN, &hdev->state);
    set_bit(HCLGE_STATE_REMOVING, &hdev->state);

    if (hdev->reset_timer.function)
        timer_delete_sync(&hdev->reset_timer);
    if (hdev->service_task.work.func)
        cancel_delayed_work_sync(&hdev->service_task);
}

STATIC void hclge_plf_enable_vector(struct hclge_plf_misc_vector *vector, bool enable)
{
    writel(enable ? 1 : 0, vector->addr);
}

STATIC int hclge_platform_init(struct hclge_plf_dev *hdev)
{
    struct platform_device *pdev = hdev->pdev;
    struct hclge_plf_hw *hw;
    struct resource *res;
    int ret;

    res = platform_get_resource(pdev, IORESOURCE_MEM, PLATFORM_DEVICE_MEM_RES_ID);
    if (!res) {
        dev_err(&pdev->dev, "get io_base mem resource failed\n");
        return -ESRCH;
    }

    hw = &hdev->hw;
    hw->io_base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
    if (!hw->io_base) {
        dev_err(&pdev->dev, "failed to do ioremap io_base\n");
        return -EINVAL;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, PLATFORM_DEVICE_MEM_RES_IOSUB_ID);
    if (!res) {
        ret = -ESRCH;
        dev_err(&pdev->dev, "get iosub_base mem resource failed\n");
        goto err_devm_iounmap;
    }

    hw->iosub_base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
    if (!hw->iosub_base) {
        ret = -EINVAL;
        dev_err(&pdev->dev, "failed to do ioremap iosub_base\n");
        goto err_devm_iounmap;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, PLATFORM_DEVICE_MEM_RES_MDIO_ID);
    if (!res) {
        ret = -ESRCH;
        dev_err(&pdev->dev, "get mdio mem resource failed\n");
        goto err_iosub_iounmap;
    }

    hw->mac.mdio_base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
    if (!hw->mac.mdio_base) {
        ret = -EINVAL;
        dev_err(&pdev->dev, "failed to do ioremap mdio\n");
        goto err_iosub_iounmap;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, PLATFORM_DEVICE_MEM_RES_GPIO_ID);
    if (!res) {
        ret = -ESRCH;
        dev_err(&pdev->dev, "get gpio mem resource failed\n");
        goto err_mdio_iounmap;
    }

    hw->mac.gpio_base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
    if (!hw->mac.gpio_base) {
        ret = -EINVAL;
        dev_err(&pdev->dev, "failed to do ioremap gpio\n");
        goto err_mdio_iounmap;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, PLATFORM_DEVICE_MEM_RES_PHY_GPIO_ID);
    if (!res) {
        ret = -ESRCH;
        dev_err(&pdev->dev, "get phy_gpio mem resource failed\n");
        goto err_gpio_iounmap;
    }

    hw->mac.phy_gpio_base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
    if (!hw->mac.phy_gpio_base) {
        ret = -EINVAL;
        dev_err(&pdev->dev, "failed to do ioremap phy_gpio\n");
        goto err_gpio_iounmap;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, PLATFORM_DEVICE_MEM_RES_IOMUX_ID);
    if (!res) {
        ret = -ESRCH;
        dev_err(&pdev->dev, "get iomux mem resource failed\n");
        goto err_phy_gpio_iounmap;
    }

    hw->iomux_base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
    if (!hw->iomux_base) {
        ret = -EINVAL;
        dev_err(&pdev->dev, "failed to do ioremap iomux\n");
        goto err_phy_gpio_iounmap;
    }

#if (!defined(CONFIG_PLATFORM_MDC) && !defined(DEFINE_HNS_LLT))
    res = platform_get_resource(pdev, IORESOURCE_MEM, PLATFORM_DEVICE_MEM_RES_SCHE_ID);
    if (!res) {
        ret = -ESRCH;
        dev_err(&pdev->dev, "get sche mem resource failed\n");
        goto err_iomux_iounmap;
    }

    hw->sche_base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
    if (!hw->sche_base) {
        ret = -EINVAL;
        dev_err(&pdev->dev, "failed to do ioremap sche\n");
        goto err_iomux_iounmap;
    }
#endif

    ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));  // 64 bit DMA addr
    if (ret != 0) {
        ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));  // 32 bit DMA addr
        if (ret != 0) {
            dev_err(&pdev->dev, "can't set consistent DMA\n");
            goto err_sche_iounmap;
        }
        dev_warn(&pdev->dev, "set DMA mask to 32 bits\n");
    }

    return 0;

err_sche_iounmap:
    devm_iounmap(&pdev->dev, hw->sche_base);
err_iomux_iounmap:
    devm_iounmap(&pdev->dev, hw->iomux_base);
err_phy_gpio_iounmap:
    devm_iounmap(&pdev->dev, hw->mac.phy_gpio_base);
err_gpio_iounmap:
    devm_iounmap(&pdev->dev, hw->mac.gpio_base);
err_mdio_iounmap:
    devm_iounmap(&pdev->dev, hw->mac.mdio_base);
err_iosub_iounmap:
    devm_iounmap(&pdev->dev, hw->iosub_base);
err_devm_iounmap:
    devm_iounmap(&pdev->dev, hw->io_base);
    return ret;
}

STATIC void hclge_platform_uninit(struct hclge_plf_dev *hdev)
{
    struct platform_device *pdev = hdev->pdev;
    struct hclge_plf_hw *hw = &hdev->hw;

    devm_iounmap(&pdev->dev, hw->sche_base);

    devm_iounmap(&pdev->dev, hw->iomux_base);

    devm_iounmap(&pdev->dev, hw->mac.phy_gpio_base);

    devm_iounmap(&pdev->dev, hw->mac.gpio_base);

    devm_iounmap(&pdev->dev, hw->mac.mdio_base);

    devm_iounmap(&pdev->dev, hw->iosub_base);

    devm_iounmap(&pdev->dev, hw->io_base);
}

STATIC void hclge_plf_check_reg_state_and_delay(void __iomem *reg_addr, u32 delay)
{
    u32 timeout = 0;

    do {
        if (readl(reg_addr) & 0x1) {
            return;
        }
        udelay(1);
        timeout++;
    } while (timeout < delay);

    return;
}

STATIC void hclge_plf_iomux_init(struct hclge_plf_dev *hdev)
{
    u32 addr;

    for (addr = IOMUX_RGMII_PHY_START; addr <= IOMUX_RGMII_PHY_END; addr += 4) { // reg size 4 bytes
        if ((addr == IOMUX_PHY_RST_0) || (addr == IOMUX_PHY_RST_1)) { // phy gpio use default mode
            continue;
        }
        writel(0, (hdev->hw.iomux_base + addr));
    }
}

STATIC int hclge_plf_init_hw(struct hclge_plf_dev *hdev)
{
    void __iomem *reg_addr;
    u32 delay = 100;
    int time_out;

    hclge_write_dev(&hdev->hw, AM_CFG_PORT_RD_EN_REG, 0x0);
    hclge_write_dev(&hdev->hw, AM_CFG_PORT_WR_EN_REG, 0x0);

    time_out = wait_hardware_done(hdev->hw.io_base + AM_CURR_PORT_STS_REG, 0xFFFFFFFF, 0x0,
                                  STATE_WAIT_INTERVAL, STATE_WAIT_LOOP_NUM);
    if (time_out) {
        dev_err(&hdev->pdev->dev, "failed to disable AXI_MSTER_OOO!\n");
        return time_out;
    }

    /* TPU mem */
    reg_addr = hdev->hw.io_base + TPU_REG_BASE + TPU_MEM_INIT_START;
    writel(1, reg_addr);

    reg_addr = hdev->hw.io_base + TPU_REG_BASE + TPU_MEM_INIT_DONE;
    hclge_plf_check_reg_state_and_delay(reg_addr, delay);

    /* RPU mem */
    reg_addr = hdev->hw.io_base + RPU_REG_BASE + RPU_MEM_INIT_START;
    writel(1, reg_addr);

    reg_addr = hdev->hw.io_base + RPU_REG_BASE + RPU_MEM_INIT_DONE;
    hclge_plf_check_reg_state_and_delay(reg_addr, delay);

    /* RCB mem */
    reg_addr = hdev->hw.io_base + RCB_COM_REG_BASE + RCB_MEM_INIT_START;
    writel(1, reg_addr);

    reg_addr = hdev->hw.io_base + RCB_COM_REG_BASE + RCB_MEM_INIT_DONE;
    hclge_plf_check_reg_state_and_delay(reg_addr, delay);

    /* SSU mem */
    reg_addr = hdev->hw.io_base + SSU_REG_BASE + SSU_MEM_INIT_START;
    writel(1, reg_addr);

    reg_addr = hdev->hw.io_base + SSU_REG_BASE + SSU_MEM_INIT_DONE;
    hclge_plf_check_reg_state_and_delay(reg_addr, delay);

    /* PPP mem */
    reg_addr = hdev->hw.io_base + PPP_REG_BASE + PPP_MEM_INIT_START;
    writel(1, reg_addr);

    reg_addr = hdev->hw.io_base + PPP_REG_BASE + PPP_MEM_INIT_DONE;
    hclge_plf_check_reg_state_and_delay(reg_addr, delay);

    hclge_write_dev(&hdev->hw, AM_CFG_PORT_RD_EN_REG, 0xFFFF);
    hclge_write_dev(&hdev->hw, AM_CFG_PORT_WR_EN_REG, 0xFFFF);

    /* iomux */
    hclge_plf_iomux_init(hdev);

    return 0;
}

STATIC void hclge_plf_phy_gpio_init(struct hclge_plf_dev *hdev)
{
#define INVALID_GPIO_INDEX 0xff
    void __iomem *gpio_base;
    u32 gpio_index;

    gpio_base = hdev->hw.mac.phy_gpio_base;
    gpio_index = hdev->hw.mac.phy_gpio_index;

    if (gpio_index == INVALID_GPIO_INDEX) {
        return;
    }

    gpio_phy_init(gpio_base, gpio_index);

    /* reset PHY */
    gpio_set(gpio_base, gpio_index, 0);
    usleep_range(20000, 20000); // 20ms(20000 us)
    gpio_set(gpio_base, gpio_index, 1);
    usleep_range(80000, 80000); // 80ms(80000 us)
}

STATIC void hclge_plf_tpu_init(struct hclge_plf_dev *hdev)
{
    u16 i;

    for (i = 0; i < hdev->num_tqps; i++) {
        hclge_write_dev(&hdev->hw, TPU_USER_STRMID((i + hdev->tqp_offset)), hdev->id);
    }
}

STATIC void hclge_plf_ssu_init(struct hclge_plf_dev *hdev)
{
    void __iomem *reg_addr;

    /* SSU SSU_EG_MAC_SCH_ROUND_NUM */
    reg_addr = hdev->hw.io_base + SSU_REG_BASE + SSU_EG_MAC_SCH_ROUND_NUM + hdev->id * 0x4; // reg size 4 Bytes
    writel((25000 / hdev->hw.mac.speed), reg_addr); // 25000 / 1000 = 25; 25Gbps/rate

    /* SSU SSU_RPU_PORT_INIT_ENABLE */
    reg_addr = hdev->hw.io_base + SSU_REG_BASE + SSU_RPU_PORT_INIT_ENABLE;
    /* rpu_port_init_en:0xF */
    writel(0xF, reg_addr);

    /* SSU L2ERR pkt do not drop */
    reg_addr = hdev->hw.io_base + SSU_REG_BASE + SSU_L2ERR_DROP_EN;
    writel(0, reg_addr);
}

STATIC void hclge_plf_rcb_init(struct hclge_plf_dev *hdev)
{
    u16 i;

    for (i = 0; i < hdev->num_tqps; i++) {
        /* add RCB_RING_PORT_MAP tqp:hdev->id */
        hclge_write_dev(&hdev->hw, RCB_RING_PORT_MAP_REG((i + hdev->tqp_offset)), hdev->id);
        hclge_write_dev(&hdev->hw, RCB_USER_STRMID((i + hdev->tqp_offset)), hdev->id);
        hclge_write_dev(&hdev->hw, RCB_CFG_EN_REG((i + hdev->tqp_offset)), 0);
    }
}

STATIC void hclge_plf_pa_init(struct hclge_plf_dev *hdev)
{
    void __iomem *reg_addr;

    reg_addr = hdev->hw.io_base + PA_ANALY_CONFIG(hdev->id);
    writel(0xffffffff, reg_addr);

    reg_addr = hdev->hw.io_base + PA_L23_PRO_CHECK_ENABLE(hdev->id);
    writel(0xffffffff, reg_addr);

    reg_addr = hdev->hw.io_base + PA_L4_PRO_CHECK_ENABLE(hdev->id);
    writel(0xffffffff, reg_addr);
}

STATIC bool hclge_plf_port_speed_is_valid(u32 mac_mode, u32 port_speed)
{
    static u32 valid_speed[] = {HCLGE_MAC_SPEED_10M, HCLGE_MAC_SPEED_100M, HCLGE_MAC_SPEED_1G, HCLGE_MAC_SPEED_2_5G};
    static u32 valid_mode[] = {XXVGE_MAC_MODE_SGMII, XXVGE_MAC_MODE_RGMII, XXVGE_MAC_MODE_SFP_1000BASE,
        XXVGE_MAC_MODE_SFP_2500BASE};
    u32 size;
    u32 i;

    size = (u32)ARRAY_SIZE(valid_mode);
    for (i = 0; i < size; i++) {
        if (mac_mode == valid_mode[i]) {
            break;
        }
    }
    if (i == size) {
        return false;
    }

    size = (u32)ARRAY_SIZE(valid_speed);
    for (i = 0; i < size; i++) {
        if (valid_speed[i] == port_speed) {
            if ((mac_mode == XXVGE_MAC_MODE_SGMII || mac_mode == XXVGE_MAC_MODE_RGMII) &&
                (port_speed == HCLGE_MAC_SPEED_2_5G)) {
                return false;
            }

            return true;
        }
    }

    return false;
}

STATIC int hclge_plf_config_port_speed(struct hclge_plf_dev *hdev)
{
    struct platform_device *pdev = hdev->pdev;
    u32 port_speed;
    int ret;

    ret = of_property_read_u32(pdev->dev.of_node, "port_speed", &port_speed);
    if (ret != 0) {
        dev_err(&pdev->dev, "failed to get port_speed, ret=%d.\n", ret);
        return ret;
    }

    if (hclge_plf_port_speed_is_valid(hdev->hw.mac.mac_mode, port_speed)) {
        hdev->hw.mac.speed = port_speed;
        return 0;
    }

    dev_err(&pdev->dev, "phy_mode:%u, port_speed:%u is invalid.\n", hdev->hw.mac.mac_mode, port_speed);
    return -EINVAL;
}

STATIC int hclge_plf_config_tqp_num_cfg(struct hclge_plf_dev *hdev)
{
    u32 tqp_offset = atomic_read(&g_hclge_plf_tqp_offset);
    struct platform_device *pdev = hdev->pdev;
    u32 queue_num;
    int ret;

    ret = of_property_read_u32(pdev->dev.of_node, "queue_num", &queue_num);
    if (ret != 0) {
        dev_err(&pdev->dev, "read queue_num failed, ret=%d\n", ret);
        return ret;
    }

    if (tqp_offset + queue_num > PLATFORM_TQP_NUM) {
        dev_err(&pdev->dev, "invalid parameter, tqp_offset(%d) + num_tqps(%d) exceeds %u\n",
                tqp_offset, queue_num, PLATFORM_TQP_NUM);
        return -EINVAL;
    }

    hdev->tqp_offset = tqp_offset;
    hdev->num_tqps = queue_num;
    return 0;
}

STATIC int hclge_plf_config_tqp_desc_cfg(struct hclge_plf_dev *hdev)
{
    struct platform_device *pdev = hdev->pdev;
    u32 ring_size = DEF_PLATFORM_TQP_DESC_NUM;

    if (ring_size < DEF_PLATFORM_MIN_RING_SIZE || ring_size > DEF_PLATFORM_MAX_RING_SIZE) {
        dev_err(&pdev->dev, "ring_size %u invalid, should range from [%u, %u]\n",
                ring_size, DEF_PLATFORM_MIN_RING_SIZE, DEF_PLATFORM_MAX_RING_SIZE);
        return -EINVAL;
    }

    hdev->num_tx_desc = ring_size;
    hdev->num_rx_desc = ring_size;
    return 0;
}

STATIC int hclge_plf_config_tc_cfg(struct hclge_plf_dev *hdev)
{
    u32 tc_offset = atomic_read(&g_hclge_plf_tc_offset);
    struct platform_device *pdev = hdev->pdev;
    u32 tc_num;
    int ret;

    ret = of_property_read_u32(pdev->dev.of_node, "tc_num", &tc_num);
    if (ret != 0) {
        dev_err(&pdev->dev, "read tc_num failed, ret=%d\n", ret);
        return ret;
    }

    if (tc_offset + tc_num > TC_MAX_NUM) {
        dev_err(&pdev->dev, "invalid parameter, tc_offset(%d) + tc_max(%d) exceeds %u\n",
                tc_offset, tc_num, TC_MAX_NUM);
        return -EINVAL;
    }

    if (tc_num > hdev->num_tqps) {
        dev_err(&pdev->dev, "invalid parameter, tc_max(%d) > num_tqps(%d)\n", tc_num, hdev->num_tqps);
        return -EINVAL;
    }

    hdev->tc_offset = tc_offset;
    hdev->tc_max = tc_num;
    return 0;
}

STATIC int hclge_plf_get_cap(struct hclge_plf_dev *hdev)
{
    struct platform_device *pdev = hdev->pdev;
    int ret;
    u32 tmp;

    ret = of_property_read_u32(pdev->dev.of_node, "id", &tmp);
    if (ret != 0) {
        dev_err(&pdev->dev, "failed to get id, ret = %d\n", ret);
        return ret;
    }
    hdev->id = tmp;
    if (hdev->id >= MAX_PORT_NUMBER) {
        dev_err(&pdev->dev, "invalid parameter, hdev_id = %d\n", hdev->id);
        return -EINVAL;
    }

    if (atomic_read(&g_hclge_plf_first_probe_flag) == false) {
        atomic_set(&g_hclge_plf_first_probe_id, hdev->id);
    }

    ret = of_property_read_u32(pdev->dev.of_node, "gpio_index", &tmp);
    if (ret == 0) {
        hdev->hw.mac.gpio_index = tmp;
    } else {
        hdev->hw.mac.gpio_index = SFP_GPIO_DEFAULT_INDEX;
    }

    ret = of_property_read_u32(pdev->dev.of_node, "phy_gpio_index", &tmp);
    if (ret != 0) {
        dev_err(&pdev->dev, "failed to get phy_gpio_index, ret = %d\n", ret);
        return ret;
    }
    hdev->hw.mac.phy_gpio_index = tmp;

    ret = hclge_plf_config_tqp_num_cfg(hdev);
    if (ret) {
        return ret;
    }

    ret = hclge_plf_config_tqp_desc_cfg(hdev);
    if (ret) {
        return ret;
    }

    ret = hclge_plf_config_tc_cfg(hdev);
    if (ret) {
        return ret;
    }

    dev_info(&hdev->pdev->dev,
             "%s id:%d, tc_offset:%u, tc_max:%u, tqp_offset:%u, num_tqps:%u, num_tx_desc:%u.\n",
             HCLGE_PLF_DRIVER_NAME,
             hdev->id,
             hdev->tc_offset,
             hdev->tc_max,
             hdev->tqp_offset,
             hdev->num_tqps,
             hdev->num_tx_desc);

    hdev->pkt_buf_size = DEF_PLATFORM_PKT_BUF_SIZE;
    hdev->tx_buf_size = DEF_PLATFORM_TX_BUF;
    hdev->tx_buf_size = roundup(hdev->tx_buf_size, PLATFORM_BUF_SIZE_UNIT);
    hdev->dv_buf_size = HCLGE_PLF_DEFAULT_DV;
    hdev->num_nic_msi = PLATFORM_MISC_INT_NUM + hdev->num_tqps;
    hdev->num_msi = hdev->num_nic_msi;

    ret = of_property_read_u32(pdev->dev.of_node, "phy_addr", &tmp);
    if (ret != 0) {
        dev_err(&pdev->dev, "failed to get phy_addr, ret = %d\n", ret);
        return ret;
    }
    hdev->hw.mac.phy_addr = (u8)tmp;

#ifdef CONFIG_PLATFORM_ASIC
    ret = of_property_read_u32(pdev->dev.of_node, "ds_index", &tmp);
    if (ret != 0) {
        dev_err(&pdev->dev, "failed to get phy_addr, ret = %d\n", ret);
        return ret;
    }
    hdev->hw.mac.ds_index = tmp;
#endif

    ret = of_property_read_u32(pdev->dev.of_node, "phy_mode", &tmp);
    if (ret != 0) {
        dev_err(&pdev->dev, "failed to get phy_mode, ret = %d\n", ret);
        return ret;
    }
    hdev->hw.mac.mac_mode = (u8)tmp;
    ret = hclge_plf_config_port_speed(hdev);
    if (ret != 0) {
        return ret;
    }

    hdev->hw.mac.mac_id = hdev->id;
    hdev->hw.mac.duplex = 1;
#if defined(CONFIG_PLATFORM_ASIC) && defined(CONFIG_PLATFORM_SUPPORT_MAC_SGMII_AUTONEG)
    if (hdev->hw.mac.mac_mode == XXVGE_MAC_MODE_SGMII) {
        hdev->hw.mac.support_autoneg = SUPPORT_AUTONEG_ON; /* SGMII default support autoneg */
    }
#endif

    dev_info(&hdev->pdev->dev,
             "%s mac_mode:(%d), mac_speed(%d).\n",
             HCLGE_PLF_DRIVER_NAME,
             hdev->hw.mac.mac_mode,
             hdev->hw.mac.speed);

    hnae3_set_bit(hdev->ae_dev->flag, HNAE3_DEV_SUPPORT_DCB_B, 1);  // mark add

    return 0;
}

STATIC void hclge_plf_query_dev_specs(struct hclge_plf_dev *hdev)
{
#define HCLGE_MAX_NON_TSO_BD_NUM 8U
    struct hnae3_ae_dev *ae_dev = platform_get_drvdata(hdev->pdev);

    ae_dev->dev_specs.max_non_tso_bd_num = HCLGE_MAX_NON_TSO_BD_NUM;
    ae_dev->dev_specs.max_int_gl = MAX_PLATFORM_INT_GL;
    ae_dev->dev_specs.int_ql_max = MAX_PLATFORM_INT_QL_MAX;
    ae_dev->dev_specs.rss_ind_tbl_size = HCLGE_PLF_RSS_IND_TBL_SIZE;
    hdev->ae_dev->dev_specs.rss_key_size = HCLGE_PLF_RSS_KEY_SIZE;
    ae_dev->dev_specs.max_tm_rate = MAX_PLATFORM_TM_RATE;
    ae_dev->dev_specs.max_frm_size = HCLGE_PLF_MAC_MAX_FRAME;
    ae_dev->dev_specs.max_qset_num = HCLGE_PLF_MAX_QSET_NUM;

    /* support simple BD csum, next step: dts input cap */
    set_bit(HNAE3_DEV_SUPPORT_HW_TX_CSUM_B, ae_dev->caps);
    set_bit(HNAE3_DEV_SUPPORT_PAUSE_B, ae_dev->caps);
    set_bit(HNAE3_DEV_SUPPORT_FD_B, ae_dev->caps);
    ae_dev->dev_version = HNAE3_DEVICE_VERSION_V3;
}

STATIC int hclge_plf_init_msi(struct hclge_plf_dev *hdev)
{
    struct platform_device *pdev = hdev->pdev;
    u16 i, j;

    /* parse tqp irq */
    for (i = hdev->tqp_offset, j = 0; i < hdev->tqp_offset + hdev->num_tqps; i++, j++) {
        hdev->queue_irq[j] = platform_get_irq(pdev, i);
        if (hdev->queue_irq[j] < 0) {
            dev_err(&pdev->dev, "failed to get tqp irq[%d], irq = %d\n", i, hdev->queue_irq[j]);
            return hdev->queue_irq[j];
        }
    }

    /* parse abn irq */
    i = PLATFORM_TQP_INT_NUM;
    hdev->abn_irq = platform_get_irq(pdev, i++);
    if (hdev->abn_irq < 0) {
        dev_err(&pdev->dev, "failed to get abn irq, irq = %d\n", hdev->abn_irq);
        return hdev->abn_irq;
    }

    /* parse mac irq */
    hdev->mac_irq = platform_get_irq(pdev, i++);
    if (hdev->mac_irq < 0) {
        dev_err(&pdev->dev, "failed to get mac irq, irq = %d\n", hdev->mac_irq);
        return hdev->mac_irq;
    }

    /* parse rtc irq */
    hdev->rtc_irq = platform_get_irq(pdev, i++);
    if (hdev->rtc_irq < 0) {
        dev_err(&pdev->dev, "failed to get rtc irq, irq = %d\n", hdev->rtc_irq);
        return hdev->rtc_irq;
    }

    hdev->num_msi = hdev->num_tqps + PLATFORM_MISC_INT_NUM;
    hdev->num_msi_left = hdev->num_tqps + PLATFORM_MISC_INT_NUM;

    hdev->vector_status = devm_kcalloc(&pdev->dev, hdev->num_msi, sizeof(u16), GFP_KERNEL);
    if (!hdev->vector_status) {
        dev_err(&pdev->dev, "failed to alloc vector status\n");
        return -ENOMEM;
    }

    for (i = 0; i < hdev->num_msi; i++)
        hdev->vector_status[i] = HCLGE_PLF_INVALID_VPORT;

    hdev->vector_irq = devm_kcalloc(&pdev->dev, hdev->num_msi, sizeof(int), GFP_KERNEL);
    if (!hdev->vector_irq) {
        dev_err(&pdev->dev, "failed to alloc vector irq\n");
        return -ENOMEM;
    }

    return 0;
}

STATIC void hclge_plf_get_misc_vector(struct hclge_plf_dev *hdev)
{
    u16 i;

    for (i = 0; i < PLATFORM_MISC_INT_NUM; i++) {
        struct hclge_plf_misc_vector *vector;

        vector = &hdev->misc_vector[i];
        if (i < MAC_INT_START) { /* ABN */
            vector->vector_irq = hdev->abn_irq;
            hdev->vector_status[i + hdev->num_tqps] = 0;
        } else if (i < RTC_INT_START) { /* MAC */
            vector->addr = (u8 __iomem *)hdev->hw.io_base +
                HCLGE_MISC_MAC_VECTOR_REG_BASE + hdev->id * XXVGE_MAC_REG_PERIOD;
            vector->vector_irq = hdev->mac_irq;
            hdev->vector_status[i + hdev->num_tqps] = 0;
        } else { /* RTC_PPS */
            vector->vector_irq = hdev->rtc_irq;
            hdev->vector_status[i + hdev->num_tqps] = 0;
        }

        hdev->num_msi_left -= 1;
        hdev->num_msi_used += 1;
    }
}

STATIC irqreturn_t hclge_plf_abn_irq_handle(int irq, void *data)
{
    return IRQ_HANDLED;
}

STATIC irqreturn_t hclge_plf_mac_irq_handle(int irq, void *data)
{
    struct hclge_plf_dev *hdev = data;

    mac_int_handle(hdev);

    return IRQ_HANDLED;
}

STATIC irqreturn_t hclge_plf_rtc_irq_handle(int irq, void *data)
{
    return IRQ_HANDLED;
}

STATIC int hclge_plf_misc_irq_init(struct hclge_plf_dev *hdev)
{
    int ret;

    hclge_plf_get_misc_vector(hdev);

    /* ABN INT */
    (void)snprintf_s(hdev->misc_vector[0].name,
        HNAE3_INT_NAME_LEN,
        HNAE3_INT_NAME_LEN - 1,
        "%s-abn-%s",
        HNSPLF_NAME,
        hns3_platform_name(hdev->pdev));
    ret = devm_request_irq(&hdev->pdev->dev,
        hdev->abn_irq,
        hclge_plf_abn_irq_handle,
        IRQF_SHARED,
        hdev->misc_vector[0].name,
        hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "failed to request abn irq, ret = %d\n", ret);
        return ret;
    }

    /* MAC INT */
    (void)snprintf_s(hdev->misc_vector[1].name,
        HNAE3_INT_NAME_LEN,
        HNAE3_INT_NAME_LEN - 1,
        "%s-mac-%s",
        HNSPLF_NAME,
        hns3_platform_name(hdev->pdev));
    ret = devm_request_irq(&hdev->pdev->dev,
        hdev->mac_irq,
        hclge_plf_mac_irq_handle,
        IRQF_SHARED,
        hdev->misc_vector[1].name,
        hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "failed to request mac irq, ret = %d\n", ret);
        devm_free_irq(&hdev->pdev->dev, hdev->abn_irq, hdev);
        return ret;
    }

    /* RTC INT */
    (void)snprintf_s(hdev->misc_vector[RTC_INT_START].name,
        HNAE3_INT_NAME_LEN,
        HNAE3_INT_NAME_LEN - 1,
        "%s-rtc-%s",
        HNSPLF_NAME,
        hns3_platform_name(hdev->pdev));
    ret = devm_request_irq(&hdev->pdev->dev,
        hdev->rtc_irq,
        hclge_plf_rtc_irq_handle,
        IRQF_SHARED,
        hdev->misc_vector[RTC_INT_START].name,
        hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "failed to request rtc irq, ret = %d\n", ret);
        devm_free_irq(&hdev->pdev->dev, hdev->mac_irq, hdev);
        devm_free_irq(&hdev->pdev->dev, hdev->abn_irq, hdev);
        return ret;
    }

    return 0;
}

STATIC void hclge_plf_free_vector(struct hclge_plf_dev *hdev, int vector_id)
{
    if (hdev->vector_status[vector_id] == HCLGE_PLF_INVALID_VPORT) {
        dev_warn(&hdev->pdev->dev, "vector(vector_id %d) has been freed.\n", vector_id);
        return;
    }

    hdev->vector_status[vector_id] = HCLGE_PLF_INVALID_VPORT;
    hdev->num_msi_left += 1;
    hdev->num_msi_used -= 1;
}

STATIC void hclge_plf_misc_irq_uninit(struct hclge_plf_dev *hdev)
{
    devm_free_irq(&hdev->pdev->dev, hdev->rtc_irq, hdev);
    hclge_plf_free_vector(hdev, RTC_INT_START + hdev->num_tqps);

    devm_free_irq(&hdev->pdev->dev, hdev->mac_irq, hdev);
    hclge_plf_free_vector(hdev, MAC_INT_START + hdev->num_tqps);

    devm_free_irq(&hdev->pdev->dev, hdev->abn_irq, hdev);
    hclge_plf_free_vector(hdev, ABN_INT_START + hdev->num_tqps);
}

STATIC void hclge_plf_update_link_status(struct hclge_plf_dev *hdev)
{
    struct hnae3_handle *handle = &hdev->vport[0].nic;
    struct hnae3_client *client = hdev->nic_client;
    int state;
#if (defined(CONFIG_PLATFORM_ASIC) && !defined(CONFIG_PLATFORM_SUPPORT_AUTONEG))
    int phy_link;
#endif

    if (!client)
        return;

    if (test_and_set_bit(HCLGE_STATE_LINK_UPDATING, &hdev->state))
        return;

    state = mac_get_link_status(hdev);
#ifdef CONFIG_PLATFORM_ASIC
    if (hdev->hw.mac.phydev != NULL) {
#ifdef CONFIG_PLATFORM_SUPPORT_AUTONEG
        if (hdev->hw.mac.phydev->state == PHY_RUNNING) {
            state &= (hdev->hw.mac.phydev->link);
        } else {
            state = 0;
        }
#else
        phy_link = hclge_plf_get_phy_link(hdev->hw.mac.phydev);
        if (phy_link < 0) {
            clear_bit(HCLGE_STATE_LINK_UPDATING, &hdev->state);
            dev_err(&hdev->pdev->dev, "failed to get phy link, phy_link = %d\n", phy_link);
            return;
        }
        state &= phy_link;
#endif
    }
#endif

    if (state != hdev->hw.mac.link) {
        hdev->hw.mac.link = state;
        client->ops->link_status_change(handle, state != 0);
        if ((state == 1) && (atomic_read(&hdev->hw.mac.int_clear_flag) == true)) {
            /* clear interrupt */
            mac_clear_mib_warnning(hdev);
            atomic_set(&hdev->hw.mac.int_clear_flag, false);
        }
    }

    clear_bit(HCLGE_STATE_LINK_UPDATING, &hdev->state);
}

STATIC int hclge_plf_get_status(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    hclge_plf_update_link_status(hdev);

    return hdev->hw.mac.link;
}

#ifndef DEFINE_HNS_LLT
STATIC void hclge_plf_reset_task_schedule(struct hclge_plf_dev *hdev)
{
    if (!test_bit(HCLGE_STATE_REMOVING, &hdev->state) &&
        !test_and_set_bit(HCLGE_STATE_RST_SERVICE_SCHED, &hdev->state)) {
            mod_delayed_work_on(cpumask_first(&hdev->affinity_mask), hclge_plf_wq, &hdev->service_task, 0);
        }
}

void hclge_plf_task_schedule(struct hclge_plf_dev *hdev, unsigned long delay_time)
{
    if (!test_bit(HCLGE_STATE_REMOVING, &hdev->state) && !test_bit(HCLGE_STATE_RST_FAIL, &hdev->state))
        mod_delayed_work_on(cpumask_first(&hdev->affinity_mask), hclge_plf_wq, &hdev->service_task, delay_time);
}

STATIC void hclge_plf_update_sfp_present(struct hclge_plf_dev *hdev)
{
    if (hdev->hw.mac.media_type != HNAE3_MEDIA_TYPE_FIBER)
        return;

    u32 cur_present = gpio_get_present(hdev->hw.mac.gpio_base, hdev->hw.mac.gpio_index);
    u32 last_present = hdev->hw.mac.sfp_present;

    if (last_present != cur_present) {
        dev_info(&hdev->pdev->dev, "port(%d) sfp present flag change(%u -> %u)\n", hdev->id, last_present, cur_present);
    }

    hdev->hw.mac.sfp_present = cur_present;
}

STATIC void hclge_plf_periodic_service_task(struct hclge_plf_dev *hdev)
{
    unsigned long delta = round_jiffies_relative(HZ);

    if (test_bit(HCLGE_STATE_RST_FAIL, &hdev->state))
        return;

    hclge_plf_update_link_status(hdev);

    hclge_plf_update_sfp_present(hdev);
    hclge_plf_sync_fd_table(hdev);

#ifndef CONFIG_PLATFORM_MDC
    hclge_plf_update_qos_node(hdev);
#endif

    hclge_plf_task_schedule(hdev, delta);
}

STATIC void hclge_plf_reset_subtask(struct hclge_plf_dev *hdev);
STATIC void hclge_plf_reset_service_task(struct hclge_plf_dev *hdev)
{
    if (!test_and_clear_bit(HCLGE_STATE_RST_SERVICE_SCHED, &hdev->state))
        return;

    down(&hdev->reset_sem);
    set_bit(HCLGE_STATE_RST_HANDLING, &hdev->state);

    hclge_plf_reset_subtask(hdev);

    clear_bit(HCLGE_STATE_RST_HANDLING, &hdev->state);
    up(&hdev->reset_sem);
}

STATIC void hclge_plf_service_task(struct work_struct *work)
{
    struct hclge_plf_dev *hdev = container_of(work, struct hclge_plf_dev, service_task.work);

    hclge_plf_reset_service_task(hdev);
    hclge_plf_periodic_service_task(hdev);

    /* Handle error recovery, reset and mbx again in case periodical task
     * delays the handling by calling hclge_task_schedule() in
     * hclge_periodic_service_task().
     */
    hclge_plf_reset_service_task(hdev);
}
#endif

int wait_hardware_done(void __iomem *reg, u32 val_mask, u32 value, u32 interval, u32 cnt)
{
    u32 time_out = cnt;
    u32 tmp_value;

    while (time_out) {
        tmp_value = readl(reg);
        if ((tmp_value & val_mask) == value) {
            return 0;
        }
        udelay(interval);
        time_out--;
    }

    return -ETIME;
}

int hclge_plf_notify_client(struct hclge_plf_dev *hdev, enum hnae3_reset_notify_type type)
{
    struct hnae3_handle *handle = &hdev->vport[0].nic;
    struct hnae3_client *client = hdev->nic_client;
    int ret;

    if (!test_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state) || !client)
        return 0;

    if (!client->ops->reset_notify)
        return -EOPNOTSUPP;

    ret = client->ops->reset_notify(handle, type);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "failed to notify plf_nic client, type(%d), ret = %d.\n", type, ret);
    }
    return ret;
}

/* port reset */
STATIC void plf_nic_func_reset(struct hclge_plf_dev *hdev)
{
    mag_set_port_disable(hdev);
    if (!test_bit(HCLGE_STATE_INITING, &hdev->state)) {
        hclge_plf_bbox_excep_report(0, HCLGE_PLF_EXCEPID_NIC_PORT_RESET);
    }
}

STATIC void mac_set_single_mac_disable(struct hclge_plf_dev *hdev, u8 mac_id)
{
    u32 rxmac_enable_addr = XXVGE_RXMAC_ENABLE_REG + mac_id * XXVGE_MAC_REG_PERIOD;
    u32 txmac_enable_addr = XXVGE_TXMAC_ENABLE_REG + mac_id * XXVGE_MAC_REG_PERIOD;
    u_xxvge_rxmac_enable rxmac_enable;
    u_xxvge_txmac_enable txmac_enable;

    rxmac_enable.value = hclge_read_dev(&hdev->hw, rxmac_enable_addr);
    rxmac_enable.bits.rx_enable = 0;
    hclge_write_mag_dev(&hdev->hw, rxmac_enable_addr, rxmac_enable.value);

    txmac_enable.value = hclge_read_dev(&hdev->hw, txmac_enable_addr);
    txmac_enable.bits.tx_enable = 0;
    hclge_write_mag_dev(&hdev->hw, txmac_enable_addr, txmac_enable.value);
}

STATIC void mag_disable_single_igu_egu_txrx(struct hclge_plf_dev *hdev, u8 mac_id)
{
#define IGU_MAC_EN_CFG_WAIT_MS 100
    u32 igu_mac_en_cfg_addr = IGU_EGU_CFG_IGU_MAC_EN_CFG_0_REG + mac_id * 0x200;
    u_igu_mac_en_cfg igu_mac_en_cfg;
    int i = 0;

    /* disable IGU EGU txrx */
    igu_mac_en_cfg.value = hclge_read_dev(&hdev->hw, igu_mac_en_cfg_addr);
    igu_mac_en_cfg.bits.cfg_mac_rx_en = 0;
    igu_mac_en_cfg.bits.cfg_mac_tx_en = 0;
    hclge_write_dev(&hdev->hw, igu_mac_en_cfg_addr, igu_mac_en_cfg.value);

    while (i < IGU_MAC_EN_CFG_WAIT_MS) {
        igu_mac_en_cfg.value = hclge_read_dev(&hdev->hw, igu_mac_en_cfg_addr);
        if ((igu_mac_en_cfg.value & 0xa) == 0)
            break;
        msleep(1);
        i++;
    }
}

static void hclge_plf_nic_clock_cfg(struct hclge_plf_dev *hdev, bool enable)
{
    void __iomem *reg_addr;

    if (enable) {
        reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_ICG_EN;
        writel(0xFFFFFFFF, reg_addr);
        reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF1_ICG_EN;
        writel(0x1F, reg_addr);
        hclge_plf_dsb();
        udelay(1);
    } else {
        reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_ICG_DIS;
        writel(0xFFFFFFFF, reg_addr);
        reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF1_ICG_DIS;
        writel(0x1F, reg_addr);
        hclge_plf_dsb();
        udelay(1);
    }
}

static void hclge_plf_nic_rst_cfg(struct hclge_plf_dev *hdev, bool rst_en)
{
    void __iomem *reg_addr;

    if (rst_en) {
        reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_RESET_REQ;
        writel(0xFFFFFFFF, reg_addr);
        reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF1_RESET_REQ;
        writel(0xFF, reg_addr);
        hclge_plf_dsb();
        udelay(1);
    } else {
        reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_RESET_DREQ;
        writel(0xFFFFFFFF, reg_addr);
        reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF1_RESET_DREQ;
        writel(0xFF, reg_addr);
        hclge_plf_dsb();
        udelay(1);
    }
}

STATIC void global_reset_cfg(struct hclge_plf_dev *hdev)
{
    void __iomem *reg_addr;
    u32 value;

    hclge_plf_nic_rst_cfg(hdev, true);
    hclge_plf_nic_clock_cfg(hdev, false);
    hclge_plf_nic_rst_cfg(hdev, false);
    hclge_plf_nic_clock_cfg(hdev, true);

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_MDIO_RESET_REQ;
    writel(0x3, reg_addr);
    hclge_plf_dsb();
    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_MDIO_RESET_DREQ;
    writel(0x3, reg_addr);
    hclge_plf_dsb();
    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_MDIO_ICG_EN;
    writel(0x3, reg_addr);
    hclge_plf_dsb();

    if (hdev->init_flag == 0) {
        reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_ICG_DIS;
        value = (0x3 << 16) | (0x3 << 20); // rx_sds 23:20, tx_sds 19:16
        writel(value, reg_addr);
        hclge_plf_dsb();

        reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_CLK_SEL;
        value = 0; // tx_clk 7:4, rx_clk 3:0
        writel(value, reg_addr);
        hclge_plf_dsb();
        udelay(1000); // delay 1000us

        reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_ICG_EN;
        value = (0x3 << 16) | (0x3 << 20); // rx_sds 23:20, tx_sds 19:16
        writel(value, reg_addr);
    }
}

STATIC int hclge_plf_global_reset(struct hclge_plf_dev *hdev)
{
    u32 mac_pause_pfc_ctrl_addr;
    int time_out;
    int ring_id;
    u8 mac_id;
    int ret;

    if (hdev->init_flag == 1) {
        for (mac_id = 0; mac_id < MAX_PORT_NUMBER; mac_id++) {
            if (hnae3_get_bit(g_hclge_plf_id_map, mac_id) == 0x0) {
                continue;
            }

            mac_pause_pfc_ctrl_addr = XXVGE_MAC_PAUSE_PFC_CTRL_REG(mac_id);
            hclge_write_dev(&hdev->hw, mac_pause_pfc_ctrl_addr, 0x0);

            mag_disable_single_igu_egu_txrx(hdev, mac_id);
            mac_set_single_mac_disable(hdev, mac_id);
        }
    }

    hclge_write_dev(&hdev->hw, AM_CTRL_GLOBAL_REG, 0x1);

    time_out = wait_hardware_done(hdev->hw.io_base + AM_CURR_PORT_STS_REG, 0xFFFFFFFF, 0x0,
                                  STATE_WAIT_INTERVAL, STATE_WAIT_LOOP_NUM);
    if (time_out) {
        dev_err(&hdev->pdev->dev, "failed to disable AXI_MSTER_OOO!\n");
        return time_out;
    }

    global_reset_cfg(hdev);

    hclge_write_dev(&hdev->hw, AM_CFG_PORT_RD_EN_REG, 0x0);
    hclge_write_dev(&hdev->hw, AM_CFG_PORT_WR_EN_REG, 0x0);

    time_out = wait_hardware_done(hdev->hw.io_base + AM_CURR_PORT_STS_REG, 0xFFFFFFFF, 0x0,
                                  STATE_WAIT_INTERVAL, STATE_WAIT_LOOP_NUM);
    if (time_out) {
        dev_err(&hdev->pdev->dev, "failed to disable AXI_MSTER_OOO!\n");
        return time_out;
    }

    ret = hclge_plf_init_hw(hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "failed to init hw!\n");
        return ret;
    }

    hclge_write_dev(&hdev->hw, AM_CFG_PORT_RD_EN_REG, 0xFFFF);
    hclge_write_dev(&hdev->hw, AM_CFG_PORT_WR_EN_REG, 0xFFFF);

    if (hdev->init_flag == 1) {
        for (ring_id = 0; ring_id < PLATFORM_TQP_NUM; ring_id++) {
            hclge_write_dev(&hdev->hw, TQP_INT_CTRL_REG(ring_id), 0x1);
        }
    }
    if (!test_bit(HCLGE_STATE_INITING, &hdev->state)) {
        hclge_plf_bbox_excep_report(0, HCLGE_PLF_EXCEPID_NIC_GLOBAL_RESET);
    }

    return 0;
}

STATIC int hclge_plf_reset_operation(struct hclge_plf_dev *hdev)
{
    int ret;

    if (hdev->reset_type == HNAE3_FUNC_RESET) {
        plf_nic_func_reset(hdev);
        hdev->rst_stats.pf_rst_cnt++;
    } else if (hdev->reset_type == HNAE3_GLOBAL_RESET) {
        ret = hclge_plf_global_reset(hdev);
        if (ret != 0) {
            dev_err(&hdev->pdev->dev, "asserting global reset fail %d!\n", ret);
            return ret;
        }

        hdev->rst_stats.global_rst_cnt++;
    } else {
        /* do nothing */
    }

    /* inform hardware that preparatory work is done */
    msleep(HCLGE_PLF_RESET_SYNC_TIME);
    dev_info(&hdev->pdev->dev, "prepare wait ok\n");

    return 0;
}

STATIC int hclge_plf_reset_ae_dev(struct hnae3_ae_dev *ae_dev)
{
    struct hclge_plf_dev *hdev = ae_dev->priv;
    struct platform_device *pdev = hdev->pdev;
    int ret;

    set_bit(HCLGE_STATE_DOWN, &hdev->state);

    if (hdev->hw.mac.mac_mode == XXVGE_MAC_MODE_RGMII) {
        hclge_plf_rgmii_clk_cfg(hdev);
    }

    hclge_plf_rcb_init(hdev);
    hclge_plf_ssu_init(hdev);
    hclge_plf_pa_init(hdev);
    hclge_plf_tpu_init(hdev);
    hclge_plf_phy_gpio_init(hdev);

#ifndef CONFIG_PLATFORM_ESL
    ret = xxvge_mag_init(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "Mag init error, ret = %d\n", ret);
        return ret;
    }
    hclge_plf_mac_int_enable(hdev);
#endif

    ret = hclge_plf_tm_schd_init(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "tm schd init error, ret = %d\n", ret);
        return ret;
    }

    hclge_plf_rss_cap_init(hdev);
    hclge_plf_rss_init_cfg(hdev);
    ret = hclge_plf_rss_init_hw(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "Rss init fail, ret = %d\n", ret);
        return ret;
    }

    ret = hclge_plf_init_fd_config(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "fd table init fail, ret = %d\n", ret);
        return ret;
    }

    hclge_plf_init_vlan_config(hdev);
    hclge_plf_ras_int_config(hdev);

    dev_info(&pdev->dev, "Reset done, %s driver initialization finished.\n",
        HCLGE_PLF_DRIVER_NAME);

    return 0;
}

STATIC int hclge_plf_reset_stack(struct hclge_plf_dev *hdev)
{
    int ret;

    ret = hclge_plf_notify_client(hdev, HNAE3_UNINIT_CLIENT);
    if (ret != 0)
        return ret;

    ret = hclge_plf_reset_ae_dev(hdev->ae_dev);
    if (ret != 0)
        return ret;

    return hclge_plf_notify_client(hdev, HNAE3_INIT_CLIENT);
}

enum hnae3_reset_type hclge_plf_get_reset_level(struct hnae3_ae_dev *ae_dev,
                                                unsigned long *addr)
{
    enum hnae3_reset_type rst_level = HNAE3_NONE_RESET;
    struct hclge_plf_dev *hdev = ae_dev->priv;

    /* return the highest priority reset level amongst all */
    if (test_bit(HNAE3_GLOBAL_RESET, addr)) {
        rst_level = HNAE3_GLOBAL_RESET;
        clear_bit(HNAE3_GLOBAL_RESET, addr);
        clear_bit(HNAE3_FUNC_RESET, addr);
    } else if (test_bit(HNAE3_FUNC_RESET, addr)) {
        rst_level = HNAE3_FUNC_RESET;
        clear_bit(HNAE3_FUNC_RESET, addr);
    } else {
        /* do nothing */
    }

    if (hdev->reset_type != HNAE3_NONE_RESET &&
        rst_level < hdev->reset_type)
        return HNAE3_NONE_RESET;

    return rst_level;
}

STATIC void hclge_plf_update_reset_level(struct hclge_plf_dev *hdev)
{
    struct hnae3_ae_dev *ae_dev = platform_get_drvdata(hdev->pdev);
    enum hnae3_reset_type reset_level;

    /* reset request will not be set during reset, so we clear
     * existing reset request to avoid unnecessary reset
     * caused by the same reason
     */
    hclge_plf_get_reset_level(ae_dev, &hdev->reset_request);

    /* if default_reset_request has a higher level reset request,
     * it should be handled as soon as possible. since some errors
     * need this kind of reset to fix.
     */
    reset_level = hclge_plf_get_reset_level(ae_dev, &hdev->default_reset_request);
    if (reset_level != HNAE3_NONE_RESET)
        set_bit(reset_level, &hdev->reset_request);
}

STATIC int hclge_plf_reset_prepare_notify_down(struct hclge_plf_dev *hdev)
{
    struct list_head *ae_dev_list = NULL;
    struct hnae3_ae_dev *ae_dev2 = NULL;
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hclge_plf_dev *tmp = NULL;
    int ret = 0;

    if (hdev->reset_type == HNAE3_FUNC_RESET) {
        hdev->rst_stats.reset_cnt++;
        /* perform reset of the stack & ae device for a client */
        rtnl_lock();
        ret = hclge_plf_notify_client(hdev, HNAE3_DOWN_CLIENT);
        rtnl_unlock();
        if (ret != 0)
            return ret;
    } else if (hdev->reset_type == HNAE3_GLOBAL_RESET) {
        ae_dev_list = hnae3_get_ae_dev_list();
        list_for_each_entry_safe(ae_dev, ae_dev2, ae_dev_list, node) {
            if (ae_dev->priv == NULL) {
                pr_err("hns3:ae_dev->priv is NULL.\n");
                ret = -1;
                break;
            }

            /* multi ports to be optimized */
            tmp = ae_dev->priv;
            tmp->rst_stats.reset_cnt++;
            set_bit(HCLGE_STATE_RST_HANDLING, &hdev->state);
            rtnl_lock();
            ret = hclge_plf_notify_client(tmp, HNAE3_DOWN_CLIENT);
            rtnl_unlock();
            if (ret != 0) {
                dev_err(&hdev->pdev->dev, "notify client fail %d!\n", ret);
                clear_bit(HCLGE_STATE_RST_HANDLING, &hdev->state);
                break;
            }
            clear_bit(HCLGE_STATE_RST_HANDLING, &hdev->state);
        }

        hnae3_put_ae_dev_list();
    }

    return ret;
}

STATIC int hclge_plf_reset_prepare(struct hclge_plf_dev *hdev)
{
    int ret;

    ret = hclge_plf_reset_prepare_notify_down(hdev);
    if (ret != 0)
        return ret;

    return hclge_plf_reset_operation(hdev);
}

STATIC int hclge_plf_reset_func_rebuild(struct hclge_plf_dev *hdev)
{
    struct hnae3_handle *handle = &hdev->vport[0].nic;
    int ret;

    hdev->rst_stats.hw_reset_done_cnt++;

    rtnl_lock();
    ret = hclge_plf_reset_stack(hdev);
    rtnl_unlock();
    if (ret != 0)
        return ret;

    rtnl_lock();
    ret = hclge_plf_notify_client(hdev, HNAE3_UP_CLIENT);
    rtnl_unlock();
    if (ret != 0)
        return ret;

    hdev->last_reset_time = jiffies;
    hdev->rst_stats.reset_fail_cnt = 0;
    hdev->rst_stats.reset_done_cnt++;
    clear_bit(HCLGE_STATE_RST_FAIL, &hdev->state);

    hclge_plf_update_reset_level(hdev);

    if (handle && handle->ae_algo->ops->reset_end)
        handle->ae_algo->ops->reset_end(handle, true);

    return 0;
}

/* reset error post-processing function */
STATIC bool hclge_plf_reset_err_handle(struct hclge_plf_dev *hdev)
{
    struct hnae3_handle *handle = &hdev->vport[0].nic;

    if (hdev->reset_pending) {
        dev_info(&hdev->pdev->dev, "Reset pending %lu\n", hdev->reset_pending);
        return true;
    } else if (hdev->rst_stats.reset_fail_cnt < HCLGE_PLF_RESET_MAX_FAIL_CNT) {
        hdev->rst_stats.reset_fail_cnt++;
        set_bit(hdev->reset_type, &hdev->reset_pending);
        dev_info(&hdev->pdev->dev, "re-schedule reset task(%u)\n", hdev->rst_stats.reset_fail_cnt);
        return true;
    } else {
        if (handle && handle->ae_algo->ops->reset_end)
            handle->ae_algo->ops->reset_end(handle, false);
        set_bit(HCLGE_STATE_RST_FAIL, &hdev->state);
        return false;
    }
}

STATIC void hclge_plf_do_reset(struct hclge_plf_dev *hdev)
{
    struct platform_device *pdev = hdev->pdev;

    switch (hdev->reset_type) {
        case HNAE3_GLOBAL_RESET:
            dev_info(&pdev->dev, "global reset requested\n");
            set_bit(HNAE3_GLOBAL_RESET, &hdev->reset_pending);
#ifndef DEFINE_HNS_LLT
            hclge_plf_reset_task_schedule(hdev);
#endif
            break;
        case HNAE3_FUNC_RESET:
            dev_info(&pdev->dev, "PF reset requested\n");
            /* schedule again to check later */
            set_bit(HNAE3_FUNC_RESET, &hdev->reset_pending);
#ifndef DEFINE_HNS_LLT
            hclge_plf_reset_task_schedule(hdev);
#endif
            break;
        default:
            dev_warn(&pdev->dev, "unsupported reset type: %d\n",
                hdev->reset_type);
            break;
    }
}

STATIC int hclge_plf_reset_global_rebuild(void)
{
    struct list_head *ae_dev_list = NULL;
    struct hnae3_ae_dev *ae_dev2 = NULL;
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hclge_plf_dev *hdev = NULL;
    int ret = 0;

    ae_dev_list = hnae3_get_ae_dev_list();
    list_for_each_entry_safe(ae_dev, ae_dev2, ae_dev_list, node) {
        if (ae_dev->priv == NULL) {
            pr_err("ae_dev->priv is NULL.\n");
            ret = -1;
            break;
        }

        /* multi ports to be optimized */
        hdev = ae_dev->priv;

        /* reset qos settings */
#ifndef CONFIG_PLATFORM_MDC
        if (hdev->id == hclge_plf_get_first_probe_id()) {
            set_bit(HCLGE_STATE_QOS_CFG_CHANGED, &hdev->state);
        }
#endif

        set_bit(HNAE3_FUNC_RESET, &hdev->reset_pending);
#ifndef DEFINE_HNS_LLT
        hclge_plf_reset_task_schedule(hdev);
#endif
    }

    hnae3_put_ae_dev_list();
    return ret;
}

STATIC int hclge_plf_reset_rebuild(struct hclge_plf_dev *hdev)
{
    int ret;

    if (hdev->reset_type == HNAE3_FUNC_RESET) {
        ret = hclge_plf_reset_func_rebuild(hdev);
        if (ret != 0) {
            dev_err(&hdev->pdev->dev, "asserting function reset rebuild fail %d!\n", ret);
            return ret;
        }
    } else if (hdev->reset_type == HNAE3_GLOBAL_RESET) {
        ret = hclge_plf_reset_global_rebuild();
        if (ret != 0) {
            dev_err(&hdev->pdev->dev, "asserting global reset rebuild fail %d!\n", ret);
            return ret;
        }
    } else {
        /* do nothing */
    }

    return 0;
}

STATIC void hclge_plf_reset(struct hclge_plf_dev *hdev)
{
    if (hclge_plf_reset_prepare(hdev))
        goto err_reset;

    if (hclge_plf_reset_rebuild(hdev))
        goto err_reset;

    return;

err_reset:
    if (hclge_plf_reset_err_handle(hdev))
#ifndef DEFINE_HNS_LLT
        hclge_plf_reset_task_schedule(hdev);
#else
        return;
#endif
}

STATIC void hclge_plf_do_reset_event(struct hnae3_ae_dev *ae_dev,
    struct hnae3_handle *handle)
{
    struct hclge_plf_dev *hdev = ae_dev->priv;

    /* We might end up getting called broadly because of 2 below cases:
     * 1. Recoverable error was conveyed through APEI and only way to bring
     *    normalcy is to reset.
     * 2. A new reset request from the stack due to timeout
     *
     * For the first case,error event might not have ae handle available.
     * check if this is a new reset request and we are not here just because
     * last reset attempt did not succeed and watchdog hit us again. We will
     * know this if last reset request did not occur very recently (watchdog
     * timer = 5*HZ, let us check after sufficiently large time, say 4*5*Hz)
     * In case of new request we reset the "reset level" to PF reset.
     * And if it is a repeat reset request of the most recent one then we
     * want to make sure we throttle the reset request. Therefore, we will
     * not allow it again before 3*HZ times.
     */
    if (!handle)
        handle = &hdev->vport[0].nic;

    if (time_before(jiffies, (hdev->last_reset_time + HCLGE_RESET_INTERVAL))) {
        mod_timer(&hdev->reset_timer, jiffies + HCLGE_RESET_INTERVAL);
        return;
    } else if (hdev->default_reset_request) {
        hdev->reset_level = hclge_plf_get_reset_level(ae_dev, &hdev->default_reset_request);
    } else if (time_after(jiffies, (hdev->last_reset_time + 4 * 5 * HZ))) {
        hdev->reset_level = HNAE3_FUNC_RESET;
    } else {
        /* do nothing */
    }

    dev_info(&hdev->pdev->dev, "received reset event, reset type is %d\n",
        hdev->reset_level);

    /* request reset & schedule reset task */
    set_bit(hdev->reset_level, &hdev->reset_request);
#ifndef DEFINE_HNS_LLT
    hclge_plf_reset_task_schedule(hdev);
#endif

    if (hdev->reset_level < HNAE3_GLOBAL_RESET)
        hdev->reset_level++;
}

STATIC void hclge_plf_reset_timer(struct timer_list *t)
{
    struct hclge_plf_dev *hdev = container_of(t, struct hclge_plf_dev,
        reset_timer);
    struct hnae3_ae_dev *ae_dev = hdev->ae_dev;

    /* if default_reset_request has no value, it means that this reset
     * request has already be handled, so just return here
     */
    if (!hdev->default_reset_request)
        return;

    dev_info(&hdev->pdev->dev, "triggering reset in reset timer\n");

    hclge_plf_do_reset_event(ae_dev, NULL);
}

void hclge_plf_set_def_reset_request(struct hnae3_ae_dev *ae_dev,
                                     enum hnae3_reset_type rst_type)
{
    struct hclge_plf_dev *hdev = ae_dev->priv;

    set_bit(rst_type, &hdev->default_reset_request);
}

STATIC bool hclge_plf_ae_dev_resetting(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    return test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state);
}

STATIC bool hclge_plf_reset_end(struct hnae3_handle *handle, bool done)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    if (hdev->rst_stats.reset_fail_cnt >= HCLGE_PLF_RESET_MAX_FAIL_CNT)
        dev_err(&hdev->pdev->dev, "reset fail!\n");
    return done;
}

STATIC void hclge_plf_reset_event(struct pci_dev *pdev,
    struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = NULL;
    struct hnae3_ae_dev *ae_dev = NULL;

    if (handle) {
        vport = hclge_plf_get_vport(handle);
        ae_dev = vport->back->ae_dev;
    } else if (pdev) {
        ae_dev = pci_get_drvdata(pdev);
    }

    if (!ae_dev)
        return;

    hclge_plf_do_reset_event(ae_dev, handle);
}

STATIC void hclge_plf_reset_subtask(struct hclge_plf_dev *hdev)
{
    struct hnae3_ae_dev *ae_dev = platform_get_drvdata(hdev->pdev);

    /* check if there is any ongoing reset in the hardware. This status can
     * be checked from reset_pending. If there is then, we need to wait for
     * hardware to complete reset.
     *    a. If we are able to figure out in reasonable time that hardware
     *       has fully resetted then, we can proceed with driver, client
     *       reset.
     *    b. else, we can come back later to check this status so re-sched
     *       now.
     */
    hdev->last_reset_time = jiffies;
    hdev->reset_type = hclge_plf_get_reset_level(ae_dev, &hdev->reset_pending);
    if (hdev->reset_type != HNAE3_NONE_RESET) {
        hclge_plf_reset(hdev);
    }

    /* check if we got any *new* reset requests to be honored */
    hdev->reset_type = hclge_plf_get_reset_level(ae_dev, &hdev->reset_request);
    if (hdev->reset_type != HNAE3_NONE_RESET)
        hclge_plf_do_reset(hdev);

    hdev->reset_type = HNAE3_NONE_RESET;
}

void hclge_plf_rgmii_clk_cfg(struct hclge_plf_dev *hdev)
{
    void __iomem *reg_addr;
    u32 tx_mode = 0;
    u32 data;

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_ICG_DIS;
    data = ((1 << hdev->id) << 16) | ((1 << hdev->id) << 20); // rx_sds 23:20, tx_sds 19:16
    writel(data, reg_addr);
    hclge_plf_isb();
    hclge_plf_dsb();

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_CLK_SEL;
    data = readl(reg_addr);
    hnae3_set_bit(data, hdev->id, 1); // rx_clk 3:0
    hnae3_set_bit(data, (hdev->id + 4), 1); // tx_clk 7:4
    writel(data, reg_addr);

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_TX_MODE;
    switch (hdev->hw.mac.speed) {
        case XXVGE_MAC_SPEED_1G:
            tx_mode = 0; // 0 : freq 250M
            break;
        case XXVGE_MAC_SPEED_100M:
            tx_mode = 2; // 2 : freq 50M
            break;
        case XXVGE_MAC_SPEED_10M:
            tx_mode = 1; // 1 : freq 5M
            break;
        default:
            break;
    }
    data = readl(reg_addr);
    hnae3_set_field(data, (0x3 << (hdev->id * 2)), (hdev->id * 2), tx_mode); // mac0 1:0, mac1 3:2
    writel(data, reg_addr);
    hclge_plf_isb();
    hclge_plf_dsb();

    udelay(1000); /* delay 1000us */

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_ICG_EN;
    data = ((1 << hdev->id) << 16) | ((1 << hdev->id) << 20); // rx_sds 23:20, tx_sds 19:16
    writel(data, reg_addr);

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_RGMII_ICG_EN;
    // rx 7:6, rx_inv 5:4 tx 3:2 tx_inv 1:0
    data = ((1 << hdev->id) << 6) | ((1 << hdev->id) << 4) | ((1 << hdev->id) << 2) | (1 << hdev->id);
    writel(data, reg_addr);

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_RGMII_RESET_DREQ;
    // rx 7:6, rx_inv 5:4 tx 3:2 tx_inv 1:0
    data = ((1 << hdev->id) << 6) | ((1 << hdev->id) << 4) | ((1 << hdev->id) << 2) | (1 << hdev->id);
    writel(data, reg_addr);
}

STATIC int hclge_plf_nic_module_init(struct hclge_plf_dev *hdev)
{
    int ret;

    if (hdev->init_flag == 0 && hdev->id == hclge_plf_get_first_probe_id()) {
        hclge_plf_nic_rst_cfg(hdev, false);
        hclge_plf_nic_clock_cfg(hdev, true);
        ret = hclge_plf_global_reset(hdev);
        if (ret != 0) {
            dev_err(&hdev->pdev->dev, "init global reset fail %d!\n", ret);
            return ret;
        }
    }

    hdev->init_flag = 1;

    return 0;
}

STATIC int hclge_plf_prepare_resource(struct hclge_plf_dev *hdev)
{
    struct platform_device *pdev = hdev->ae_dev->plfdev;
    int ret;

    ret = hclge_plf_get_cap(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "get cap err, ret = %d.\n", ret);
        return ret;
    }

    ret = hclge_plf_nic_module_init(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "nic module init err, ret = %d.\n", ret);
        return ret;
    }

    if (hdev->hw.mac.mac_mode == XXVGE_MAC_MODE_RGMII) {
        hclge_plf_rgmii_clk_cfg(hdev);
    }

    hclge_plf_rcb_init(hdev);
    hclge_plf_ssu_init(hdev);
    hclge_plf_pa_init(hdev);
    hclge_plf_tpu_init(hdev);
    hclge_plf_phy_gpio_init(hdev);
    hclge_plf_query_dev_specs(hdev);

    return 0;
}

STATIC int hclge_plf_init_hw_resource(struct hclge_plf_dev *hdev)
{
    struct platform_device *pdev = hdev->ae_dev->plfdev;
    int ret;

    ret = hclge_plf_prepare_resource(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "prepare resource error, ret = %d.\n", ret);
        goto out;
    }

    hclge_plf_configure(hdev);

    ret = hclge_plf_init_msi(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "Init MSI/MSI-X error, ret = %d.\n", ret);
        goto out;
    }

    ret = hclge_plf_misc_irq_init(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "misc irq init error, ret = %d.\n", ret);
        goto out;
    }

    ret = hclge_plf_alloc_tqps(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "Allocate TQPs error, ret = %d.\n", ret);
        goto err_msi_irq_uninit;
    }

    ret = hclge_plf_alloc_vport(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "Allocate vport error, ret = %d.\n", ret);
        goto err_msi_irq_uninit;
    }

#if (!defined(CONFIG_PLATFORM_ESL) && !defined(CONFIG_PLATFORM_EMU))
    ret = hns_mac_mdio_init(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "mdio init error, ret = %d.\n", ret);
        goto err_msi_irq_uninit;
    }

    ret = phy_hw_init(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "phy init error, ret = %d.\n", ret);
        goto err_mdiobus_unreg;
    }
#endif

    ret = xxvge_mag_init(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "Mag init error, ret = %d\n", ret);
        goto err_mdiobus_unreg;
    }

    ret = hclge_plf_tm_schd_init(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "tm schd init error, ret = %d\n", ret);
        goto err_mdiobus_unreg;
    }

    hclge_plf_rss_cap_init(hdev);
    hclge_plf_rss_init_cfg(hdev);
    ret = hclge_plf_rss_init_hw(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "Rss init fail, ret = %d\n", ret);
        goto err_mdiobus_unreg;
    }

    ret = hclge_plf_init_fd_config(hdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "Fd init error, ret = %d\n", ret);
        goto err_mdiobus_unreg;
    }

    hclge_plf_init_vlan_config(hdev);
    hclge_plf_ras_int_config(hdev);

    if ((hdev->hw.mac.gpio_index != SFP_GPIO_DEFAULT_INDEX) &&
        (hdev->hw.mac.media_type == HNAE3_MEDIA_TYPE_FIBER)) {
        gpio_present_init(hdev->hw.mac.gpio_base, hdev->hw.mac.gpio_index);
        hdev->hw.mac.sfp_present = gpio_get_present(hdev->hw.mac.gpio_base, hdev->hw.mac.gpio_index);
    }

    return 0;

err_mdiobus_unreg:
    if (hdev->hw.mac.phydev)
        mdiobus_unregister(hdev->hw.mac.mdio_bus);
err_msi_irq_uninit:
    hclge_plf_misc_irq_uninit(hdev);
out:
    return ret;
}

STATIC int hclge_plf_init_ae_dev(struct hnae3_ae_dev *ae_dev)
{
    struct platform_device *pdev = ae_dev->plfdev;
    struct hclge_plf_dev *hdev;
    int ret;

    hdev = devm_kzalloc(&pdev->dev, sizeof(*hdev), GFP_KERNEL);
    if (!hdev) {
        ret = -ENOMEM;
        goto out;
    }

    hdev->pdev = pdev;
    hdev->ae_dev = ae_dev;
    hdev->reset_type = HNAE3_NONE_RESET;
    hdev->reset_level = HNAE3_FUNC_RESET;
    ae_dev->priv = hdev;

    /* HW supprt 2 layer vlan */
    hdev->mps = ETH_FRAME_LEN + ETH_FCS_LEN + 2 * VLAN_HLEN;

    mutex_init(&hdev->vport_lock);
    spin_lock_init(&hdev->fd_rule_lock);
    spin_lock_init(&hdev->vlan_table_lock);
    sema_init(&hdev->reset_sem, 1);

    ret = hclge_platform_init(hdev);
    if (ret != 0) {
        pr_err("hclge platform init failed, ret = %d\n", ret);
        goto out;
    }

    set_bit(HCLGE_STATE_INITING, &hdev->state);
    ret = hclge_plf_init_hw_resource(hdev);
    if (ret != 0) {
        pr_err("hclge platform init hw resource failed, ret = %d\n", ret);
        goto err_platform_uninit;
    }

    timer_setup(&hdev->reset_timer, hclge_plf_reset_timer, 0);

#ifndef DEFINE_HNS_LLT
    INIT_DELAYED_WORK(&hdev->service_task, hclge_plf_service_task);
#endif
    INIT_KFIFO(hdev->mac_tnl_log);

    hclge_dcb_ops_set(hdev);

    hclge_plf_mac_int_enable(hdev);

    hclge_plf_state_init(hdev);
    hdev->last_reset_time = jiffies;

#ifndef DEFINE_HNS_LLT
    hclge_plf_task_schedule(hdev, round_jiffies_relative(HZ));
#endif

    clear_bit(HCLGE_STATE_INITING, &hdev->state);
    dev_info(&hdev->pdev->dev, "%s driver initialization finished.\n", HCLGE_PLF_DRIVER_NAME);

#ifndef DEFINE_HNS_LLT
    atomic_add(hdev->num_tqps, &g_hclge_plf_tqp_offset);
    atomic_add(hdev->tc_max, &g_hclge_plf_tc_offset);
#endif
    return 0;

err_platform_uninit:
    clear_bit(HCLGE_STATE_INITING, &hdev->state);
    hclge_platform_uninit(hdev);
out:
    return ret;
}

STATIC void hclge_plf_uninit_ae_dev(struct hnae3_ae_dev *ae_dev)
{
    struct hclge_plf_dev *hdev = ae_dev->priv;
    struct hclge_plf_mac *mac = &hdev->hw.mac;

    hclge_plf_state_uninit(hdev);
    hclge_plf_del_all_fd_entries(hdev);

    if (mac->phydev)
        mdiobus_unregister(mac->mdio_bus);

    /* Disable MISC vector(vector0) */

    hclge_plf_misc_irq_uninit(hdev);
    hclge_platform_uninit(hdev);
    mutex_destroy(&hdev->vport_lock);
    ae_dev->priv = NULL;
}

STATIC void hclge_plf_prepare_for_reset(struct hnae3_ae_dev *ae_dev, enum hnae3_reset_type rst_type)
{
#define HCLGE_FUNC_RETRY_WAIT_MS 500
#define HCLGE_FUNC_RETRY_CNT 5

    struct hclge_plf_dev *hdev = ae_dev->priv;
    int retry_cnt = 0;
    int ret;

    while (retry_cnt++ < HCLGE_FUNC_RETRY_CNT) {
        down(&hdev->reset_sem);
        set_bit(HCLGE_STATE_RST_HANDLING, &hdev->state);
        hdev->reset_type = rst_type;
        ret = hclge_plf_reset_prepare(hdev);
        if (ret == 0 && hdev->reset_pending == 0) {
            break;
        }
        dev_err(&hdev->pdev->dev,
                "fail to prepare reset, ret = %d, reset_pending:0x%lx, retry_cnt:%d\n",
                ret, hdev->reset_pending, retry_cnt);
        clear_bit(HCLGE_STATE_RST_HANDLING, &hdev->state);
        up(&hdev->reset_sem);
        msleep(HCLGE_FUNC_RETRY_WAIT_MS);
    }

    hdev->rst_stats.pf_rst_cnt++;
}

STATIC void hclge_plf_rebuild_for_reset(struct hnae3_ae_dev *ae_dev)
{
    struct hclge_plf_dev *hdev = ae_dev->priv;
    int ret;

#ifndef DEFINE_HNS_LLT
    INIT_DELAYED_WORK(&hdev->service_task, hclge_plf_service_task);
#endif

    /* reset qos settings */
#ifndef CONFIG_PLATFORM_MDC
    if (hdev->id == hclge_plf_get_first_probe_id()) {
        set_bit(HCLGE_STATE_QOS_CFG_CHANGED, &hdev->state);
    }
#endif

    ret = hclge_plf_reset_rebuild(hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "fail to rebuild, ret = %d\n", ret);
    }

    hdev->reset_type = HNAE3_NONE_RESET;
    clear_bit(HCLGE_STATE_RST_HANDLING, &hdev->state);
    up(&hdev->reset_sem);
}

STATIC int hclge_plf_init_nic_client_instance(struct hnae3_ae_dev *ae_dev, struct hclge_plf_vport *vport)
{
    struct hnae3_client *client = vport->nic.client;
    struct hclge_plf_dev *hdev = ae_dev->priv;
    int rst_cnt = hdev->rst_stats.reset_cnt;
    int ret;

    ret = client->ops->init_instance(&vport->nic);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "fail to init instance, ret = %d\n", ret);
        return ret;
    }

    set_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state);
    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state) || rst_cnt != hdev->rst_stats.reset_cnt) {
        ret = -EBUSY;
        goto init_nic_err;
    }

    hnae3_set_client_init_flag(client, ae_dev, 1);

    return 0;

init_nic_err:
    clear_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state);
    while (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state))
        msleep(HCLGE_WAIT_RESET_DONE);

    client->ops->uninit_instance(&vport->nic, 0);

    return ret;
}

STATIC int hclge_plf_init_client_instance(struct hnae3_client *client, struct hnae3_ae_dev *ae_dev)
{
    struct hclge_plf_dev *hdev = ae_dev->priv;
    struct hclge_plf_vport *vport = &hdev->vport[0];
    int ret;

    switch (client->type) {
        case HNAE3_CLIENT_KNIC:
            hdev->nic_client = client;
            vport->nic.client = client;
            ret = hclge_plf_init_nic_client_instance(ae_dev, vport);
            if (ret != 0)
                goto clear_nic;
            break;
        default:
            return -EINVAL;
    }

    return 0;

clear_nic:
    hdev->nic_client = NULL;
    vport->nic.client = NULL;
    return ret;
}

STATIC void hclge_plf_uninit_client_instance(struct hnae3_client *client, struct hnae3_ae_dev *ae_dev)
{
    struct hclge_plf_dev *hdev = ae_dev->priv;
    struct hclge_plf_vport *vport = &hdev->vport[0];

    if (hdev->nic_client && client->ops->uninit_instance) {
        clear_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state);
        while (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state))
            msleep(HCLGE_WAIT_RESET_DONE);

        client->ops->uninit_instance(&vport->nic, 0);
        hdev->nic_client = NULL;
        vport->nic.client = NULL;
    }
}

STATIC int hclge_plf_get_vector_index(struct hclge_plf_dev *hdev, int vector)
{
    int i;

    for (i = 0; i < hdev->num_msi; i++)
        if (vector == hdev->vector_irq[i])
            return i;

    return -EINVAL;
}

struct hclge_plf_vport *hclge_plf_get_vport(struct hnae3_handle *handle)
{
    return container_of(handle, struct hclge_plf_vport, nic);
}

STATIC int hclge_plf_map_ring_to_vector(
    struct hnae3_handle *handle, int vector, struct hnae3_ring_chain_node *ring_chain)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    struct hnae3_ring_chain_node *node;
    u_rcb_tx_q_intctl q_intctl;
    int vector_id;
    u32 tqp_index;

    vector_id = hclge_plf_get_vector_index(hdev, vector);
    if (vector_id < 0) {
        dev_err(&hdev->pdev->dev, "get vector index fail. vector_id = %d\n", vector_id);
        return vector_id;
    }

    for (node = ring_chain; node; node = node->next) {
        tqp_index = node->tqp_index + hdev->tqp_offset;
        q_intctl.bits.tx_int_idx = vector_id + hdev->tqp_offset;
        q_intctl.bits.tx_int_gl_idx = node->int_gl_idx;
        if (hnae3_get_bit(node->flag, HNAE3_RING_TYPE_B) == HNAE3_RING_TYPE_TX) {
            hclge_write_dev(&hdev->hw, RCB_TX_Q_INTCTL_REG(tqp_index), q_intctl.value);
        } else {
            hclge_write_dev(&hdev->hw, RCB_RX_Q_INTCTL_REG(tqp_index), q_intctl.value);
        }
    }

    return 0;
}

STATIC int hclge_plf_unmap_ring_frm_vector(
    struct hnae3_handle *handle, int vector, struct hnae3_ring_chain_node *ring_chain)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    int vector_id;

    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state))
        return 0;

    vector_id = hclge_plf_get_vector_index(hdev, vector);
    if (vector_id < 0) {
        dev_err(&handle->pdev->dev, "get vector index fail. ret = %d\n", vector_id);
        return vector_id;
    }

    return 0;
}

STATIC void hclge_plf_get_vector_info(struct hclge_plf_dev *hdev, u16 idx, struct hnae3_vector_info *vector_info)
{
    vector_info->vector = hdev->queue_irq[idx];
    vector_info->io_addr = hdev->hw.io_base + HCLGE_PLF_VECTOR_REG_BASE +
                            (idx + hdev->tqp_offset) * HCLGE_PLF_VECTOR_REG_OFFSET;
    hdev->vector_status[idx] = hdev->vport[0].vport_id;
    hdev->vector_irq[idx] = vector_info->vector;
}

STATIC int hclge_plf_get_vector(struct hnae3_handle *handle, u16 vector_num, struct hnae3_vector_info *vector_info)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hnae3_vector_info *vector = vector_info;
    struct hclge_plf_dev *hdev = vport->back;
    int alloc = 0;
    u16 i = 0;
    u16 j;

    vector_num = min_t(u16, hdev->num_nic_msi - 1, vector_num);
    vector_num = min(hdev->num_msi_left, vector_num);

    for (j = 0; j < vector_num; j++) {
        do {
            if (hdev->vector_status[i] == HCLGE_PLF_INVALID_VPORT) {
                hclge_plf_get_vector_info(hdev, i, vector);
                i++;
                vector++;
                alloc++;

                break;
            }
        } while (++i < hdev->num_nic_msi);
    }
    hdev->num_msi_left -= alloc;
    hdev->num_msi_used += alloc;

    return alloc;
}

STATIC int hclge_plf_put_vector(struct hnae3_handle *handle, int vector)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    int vector_id;

    vector_id = hclge_plf_get_vector_index(hdev, vector);
    if (vector_id < 0) {
        dev_err(&hdev->pdev->dev, "Get vector index fail. vector = %d\n", vector);
        return vector_id;
    }

    hclge_plf_free_vector(hdev, vector_id);

    return 0;
}

STATIC void hclge_plf_reset_tqp_stats(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hnae3_knic_private_info *kinfo;
    struct hclge_plf_tqp *tqp;
    u16 i;

    kinfo = &vport->nic.kinfo;
    for (i = 0; i < kinfo->num_tqps; i++) {
        struct hnae3_queue *queue;
        queue = handle->kinfo.tqp[i];
        tqp = container_of(queue, struct hclge_plf_tqp, q);
        (void)memset_s(&tqp->tqp_stats, sizeof(tqp->tqp_stats), 0, sizeof(tqp->tqp_stats));
    }
}

static void hclge_plf_flush_link_update(struct hclge_plf_dev *hdev)
{
#define HCLGE_FLUSH_LINK_TIMEOUT 100000
    int i = 0;

    while (test_bit(HCLGE_STATE_LINK_UPDATING, &hdev->state) && i < HCLGE_FLUSH_LINK_TIMEOUT) {
        usleep_range(1, 1);
        i++;
    }
}

static void hclge_plf_set_timer_task(struct hnae3_handle *handle, bool enable)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    if (enable) {
#ifndef DEFINE_HNS_LLT
        hclge_plf_task_schedule(hdev, 0);
#endif
    } else {
        /* Set the DOWN flag here to disable link updating */
        set_bit(HCLGE_STATE_DOWN, &hdev->state);

        /* flush memory to make sure DOWN is seen by service task */
        smp_mb__before_atomic();
        hclge_plf_flush_link_update(hdev);
    }
}

STATIC int hclge_plf_ae_start(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    clear_bit(HCLGE_STATE_DOWN, &hdev->state);
    hdev->hw.mac.link = 0;

    /* reset tqp stats */
    hclge_plf_reset_tqp_stats(handle);

    mag_set_port_enable(hdev);

#if defined(CONFIG_PLATFORM_ASIC) && defined(CONFIG_PLATFORM_SUPPORT_AUTONEG)
    hclge_plf_mac_start_phy(hdev);
#endif

    return 0;
}

STATIC void hclge_plf_ae_stop(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    set_bit(HCLGE_STATE_DOWN, &hdev->state);

    /* If it is not port reset, the firmware will disable the MAC,
     * so it only need to stop phy here.
     */
    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state) &&
        hdev->reset_type != HNAE3_FUNC_RESET) {
#if defined(CONFIG_PLATFORM_ASIC) && defined(CONFIG_PLATFORM_SUPPORT_AUTONEG)
        hclge_plf_mac_stop_phy(hdev);
#endif
        hclge_plf_update_link_status(hdev);
        return;
    }

    mac_set_mac_disable_rx(hdev);
    hclge_plf_reset_tqp(handle);
    mag_set_port_disable(hdev);
#if defined(CONFIG_PLATFORM_ASIC) && defined(CONFIG_PLATFORM_SUPPORT_AUTONEG)
    hclge_plf_mac_stop_phy(hdev);
#endif

    /* reset tqp stats */
    hclge_plf_reset_tqp_stats(handle);
    hclge_plf_update_link_status(hdev);
}

#ifdef PLATFORM_SUPPORT_EEPROM_MAC
STATIC u16 hclgeplf_calc_crc16(const u8 *pdata, u16 data_len)
{
    u8 crc_low, crc_high, cl, ch, save_high, save_low;
    u16 i, flag;

    crc_high = HCLGE_PLF_CRC16;
    crc_low = HCLGE_PLF_CRC16;
    ch = HCLGE_PLF_CRC_CH;
    cl = HCLGE_PLF_CRC_CL;

    for (i = 0; i < data_len; i++) {
        crc_low ^= *(pdata + i);
        for (flag = 0; flag < HCLGE_PLF_CRC_FLAG; flag++) {
            save_high = crc_high;
            save_low = crc_low;
            crc_high >>= 1;
            crc_low >>= 1;

            if ((save_high & 0x01) == 0x01) {
                crc_low |= HCLGE_PLF_CRC_CODE;
            }

            if ((save_low & 0x01) == 0x01) {
                crc_high ^= ch;
                crc_low ^= cl;
            }
        }
    }

    return (crc_high << HCLGE_PLF_CRC_FLAG) | crc_low;
}

#ifndef DEFINE_HNS_LLT
typedef long (*eeprom_ioctl_kernel_fun)(struct file *file, unsigned int cmd, unsigned long arg);
#endif
STATIC void hclgeplf_get_unique_mac(struct hclge_plf_dev *hdev, u8 *mac_addr)
{
#ifndef DEFINE_HNS_LLT
    eeprom_ioctl_kernel_fun eeprom_ioctl_kernel =
        (eeprom_ioctl_kernel_fun)(uintptr_t)symbol_get(eeprom_ioctl_kernel);
#endif
    struct hclge_plf_mac_info mac_info = { 0 };
    struct eeprom_info einfo;
    struct file *file = NULL;
    u16 crc_ret = 0;
    u16 inverse_byte_order_crc = 0;
    int readlen;
    int ret;

#ifndef DEFINE_HNS_LLT
    if (eeprom_ioctl_kernel == NULL) {
        dev_warn(&hdev->pdev->dev, "eeprom_ioctl_kernel syms not found\n");
        goto out;
    }

    file = filp_open(PLATFORM_EEPROM_DEV_NAME, O_RDWR | O_NDELAY, 0);
    if (IS_ERR(file)) {
        dev_err(&hdev->pdev->dev, "hclge eeprom file (%s) not existed.\n", PLATFORM_EEPROM_DEV_NAME);
        goto out;
    }
#endif

    einfo.buf = (char *)&mac_info;
    einfo.count = sizeof(struct hclge_plf_mac_info);
    einfo.page_address = PLATFORM_EEPROM_OFFSET + (PLATFORM_EEPROM_MAC_SIZE * hdev->id);

    readlen = eeprom_ioctl_kernel(file, EEPROM_READ_CMD, (unsigned long)(uintptr_t)&einfo);
    if (readlen != sizeof(struct hclge_plf_mac_info)) {
        dev_err(&hdev->pdev->dev, "hclge read eeprom by ioctl failed. readlen = %d\n", readlen);
        filp_close(file, NULL);
        file = NULL;
        goto out;
    }

    inverse_byte_order_crc = (mac_info.crc_value >> HCLGE_PLF_CRC_FLAG) + (mac_info.crc_value << HCLGE_PLF_CRC_FLAG);
    crc_ret = hclgeplf_calc_crc16((u8 *)&mac_info.data_length, HCLGE_PLF_CRC_DATA_LEN);
    if (crc_ret != mac_info.crc_value && crc_ret != inverse_byte_order_crc) {
        dev_err(&hdev->pdev->dev, "hclge read mac crc failed. crc_value = %hu, crc_ret = %hu.\n",
            mac_info.crc_value, crc_ret);
        filp_close(file, NULL);
        file = NULL;
        goto out;
    }

    ret = memcpy_s(mac_addr, ETH_ALEN, mac_info.mac_addr, ETH_ALEN);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "hclge mac memcpy_s failed(%d).\n", ret);
    }

    filp_close(file, NULL);
    file = NULL;

out:
#ifndef DEFINE_HNS_LLT
    if (eeprom_ioctl_kernel != NULL) {
        symbol_put(eeprom_ioctl_kernel);
    }
#endif

    return;
}
#endif

STATIC void hclge_plf_get_mac_addr(struct hnae3_handle *handle, u8 *p)
{
#ifdef PLATFORM_SUPPORT_EEPROM_MAC
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    hclgeplf_get_unique_mac(hdev, p);
#endif
}

static int hclge_plf_update_vlan_filter(struct hnae3_handle *handle);
STATIC int hclge_plf_set_mac_addr(struct hnae3_handle *handle, const void *p,
    bool is_first)
{
    const unsigned char *new_addr = (const unsigned char *)p;
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    char format_mac_addr[HNAE3_FORMAT_MAC_ADDR_LEN];
    struct hclge_plf_dev *hdev = vport->back;

    /* mac addr check */
    if (is_zero_ether_addr(new_addr) || is_broadcast_ether_addr(new_addr) || is_multicast_ether_addr(new_addr)) {
        hnae3_format_mac_addr(format_mac_addr, new_addr);
        dev_err(&hdev->pdev->dev, "Change uc mac err! invalid mac:%s.\n", format_mac_addr);
        return -EINVAL;
    }

    spin_lock_bh(&vport->mac_list_lock);
    /* we must update dev addr with spin lock protect, preventing dev addr
     * being removed by set_rx_mode path.
     */
    ether_addr_copy(hdev->hw.mac.mac_addr, new_addr);
    hclge_plf_request_update_promisc_mode(handle);
    spin_unlock_bh(&vport->mac_list_lock);

    return hclge_plf_update_vlan_filter(handle);
}

STATIC int hclge_plf_tqp_enable(struct hnae3_handle *handle, bool enable)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    int time_out = 0;
    u32 tqp_id = 0;
    u16 i;

    for (i = 0; i < handle->kinfo.num_tqps; i++) {
        tqp_id = i + hdev->tqp_offset;
        hclge_write_dev(&hdev->hw, ETS_QUEUE_EN_CFG_REG(tqp_id), enable);
        time_out = wait_hardware_done(hdev->hw.io_base + ETS_QUEUE_EN_CFG_REG(tqp_id), RCB_CFG_EN, enable,
                                      CFG_WAIT_INTERVAL, CFG_WAIT_LOOP_NUM);
        if (time_out) {
            dev_err(&hdev->pdev->dev, "failed to disable ets_queue_en_cfg_reg, ring_id = %u\n", tqp_id);
            return time_out;
        }
    }
    return 0;
}

STATIC int reset_rcb_ring(struct hnae3_handle *handle, u32 ring_id)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    u32 rcb_ring_could_be_rst;
    u32 rcb_ring_to_be_rst;
    int time_out;

    rcb_ring_could_be_rst = RCB_RING_COULD_BE_RST_REG(ring_id);
    rcb_ring_to_be_rst = RCB_RING_TO_BE_RST_REG(ring_id);
    hclge_write_dev(&hdev->hw, rcb_ring_to_be_rst, 0x1);

    time_out = wait_hardware_done(hdev->hw.io_base + rcb_ring_could_be_rst, 0x1, 0x1,
                                  STATE_WAIT_INTERVAL, RCB_RESET_WAIT_LOOP_NUM);
    if (time_out) {
        dev_err(&hdev->pdev->dev, "failed to reset rcb ring, ring_id = %u\n", ring_id);
        return time_out;
    }

    hclge_write_dev(&hdev->hw, rcb_ring_to_be_rst, 0x0);
    return 0;
}

STATIC int hclge_plf_reset_rcb(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    int rcb_reset = 0;
    int time_out = 0;
    u32 tqp_id = 0;
    u16 i;

    for (i = 0; i < handle->kinfo.num_tqps; i++) {
        tqp_id = i + hdev->tqp_offset;
        hclge_write_dev(&hdev->hw, ETS_QUEUE_FLUSH_REQ_REG(tqp_id), 0x1);
        time_out = wait_hardware_done(hdev->hw.io_base + ETS_QUEUE_FLUSH_DONE_REG(tqp_id), ETS_QUEUE_FLUSH_DONE, 0x1,
                                      STATE_WAIT_INTERVAL, STATE_WAIT_LOOP_NUM);
        if (time_out) {
            dev_err(&hdev->pdev->dev, "wait queue_flush done timeout, ring_id = %u\n", tqp_id);
            return time_out;
        }

        rcb_reset = reset_rcb_ring(handle, tqp_id);
        if (rcb_reset) {
            dev_err(&hdev->pdev->dev, "reset_rcb_ring timeout, ring_id = %u\n", tqp_id);
            return rcb_reset;
        }
        hclge_write_dev(&hdev->hw, ETS_QUEUE_FLUSH_REQ_REG(tqp_id), 0x0);
        hclge_write_dev(&hdev->hw, RCB_CFG_RX_RING_HEAD_REG(tqp_id), 0); // Avoiding First Packet Errors
    }
    return 0;
}

int hclge_plf_reset_tqp(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    int ret;

    ret = hclge_plf_tqp_enable(handle, false);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "failed to disable tqp, ret = %d\n", ret);
        return ret;
    }

    ret = hclge_plf_reset_rcb(handle);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "failed to reset rcb, ret = %d\n", ret);
        return ret;
    }

    ret = hclge_plf_tqp_enable(handle, true);
    return ret;
}

STATIC void hclge_plf_update_stats(struct hnae3_handle *handle)
{
    return;
}

STATIC u64 *hclge_plf_tqps_get_stats(struct hnae3_handle *handle, u64 *data)
{
    struct hnae3_knic_private_info *kinfo = &handle->kinfo;
    struct hclge_plf_tqp *tqp;
    u64 *buff = data;
    u16 i;

    for (i = 0; i < kinfo->num_tqps; i++) {
        tqp = container_of(kinfo->tqp[i], struct hclge_plf_tqp, q);
        *buff++ = tqp->tqp_stats.rcb_tx_ring_pktnum_rcd;
    }

    for (i = 0; i < kinfo->num_tqps; i++) {
        tqp = container_of(kinfo->tqp[i], struct hclge_plf_tqp, q);
        *buff++ = tqp->tqp_stats.rcb_rx_ring_pktnum_rcd;
    }

    return buff;
}

STATIC u64 *hclge_plf_get_mac_stats(struct hclge_plf_dev *hdev,
    const struct hclge_comm_stats_str strs[],
    u32 size, u64 *data)
{
    u64 *buf = data;
    u32 i;

    for (i = 0; i < size; i++) {
        *buf = readl(hdev->hw.io_base + XXVGE_MIB_BASE_REG + hdev->id * XXVGE_MAC_REG_PERIOD + i * 0x8); // size 0x8
        buf++;
    }

    return buf;
}

STATIC void hclge_plf_get_stats(struct hnae3_handle *handle, u64 *data)
{
    struct hclge_plf_vport *vport = hclge_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    u64 *p;

    p = hclge_plf_get_mac_stats(hdev, g_mac_stats_string, ARRAY_SIZE(g_mac_stats_string), data);
    (void)hclge_plf_tqps_get_stats(handle, p);
}

STATIC u32 hclge_plf_get_fw_version(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    return hdev->fw_version;
}

STATIC void hclge_plf_get_tqps_and_rss_info(struct hnae3_handle *handle, u16 *alloc_tqps, u16 *max_rss_size)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    *alloc_tqps = vport->alloc_tqps;
    *max_rss_size = hdev->pf_rss_size_max;
}

STATIC int hclge_plf_set_channels(struct hnae3_handle *handle, u32 new_tqps_num, bool rxfh_configured)
{
    struct hnae3_ae_dev *ae_dev = platform_get_drvdata(handle->plfdev);
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hnae3_knic_private_info *kinfo = &vport->nic.kinfo;
    struct hclge_plf_dev *hdev = vport->back;
    u32 tc_offset[HCLGE_MAX_TC_NUM] = {0};
    u32 tc_size[HCLGE_MAX_TC_NUM] = {0};
    u16 cur_rss_size = kinfo->rss_size;
    u16 cur_tqps = kinfo->num_tqps;
    u32 tc_valid[HCLGE_MAX_TC_NUM];
    u32 roundup_size;
    u32 *rss_indir;
    int ret;
    u32 i;

    if (!ae_dev) {
        dev_err(&hdev->pdev->dev, "ae_dev is NULL\n");
        return -EINVAL;
    }

    kinfo->req_rss_size = new_tqps_num;

    ret = hclge_plf_tm_vport_map_update(hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "tm vport map fail, ret = %d\n", ret);
        return ret;
    }

    roundup_size = (u32)roundup_pow_of_two(kinfo->rss_size);
    if (ilog2(roundup_size) < 0) {
        dev_err(&hdev->pdev->dev, "set channels roundup_size invalid, roundup_size = %u\n", roundup_size);
        return -EINVAL;
    }
    roundup_size = (u32)ilog2(roundup_size);
    /* Set the RSS TC mode according to the new RSS size */
    for (i = 0; i < HCLGE_MAX_TC_NUM; i++) {
        tc_valid[i] = 0;

        if (!(hdev->hw_tc_map & BIT(i)))
            continue;

        tc_valid[i] = 1;
        tc_size[i] = roundup_size;
        tc_offset[i] = (u32)(kinfo->rss_size * i + hdev->tqp_offset);
    }
    ret = hclge_plf_set_rss_tc_mode(hdev, tc_valid, tc_size, tc_offset);
    if (ret != 0)
        return ret;

    /* RSS indirection table has been configuared by user */
    if (rxfh_configured)
        goto out;

    /* Reinitializes the rss indirect table according to the new RSS size */
    rss_indir = kcalloc(ae_dev->dev_specs.rss_ind_tbl_size, sizeof(u32),
        GFP_KERNEL);
    if (!rss_indir) {
        dev_err(&hdev->pdev->dev, "failed to alloc rss indirect table\n");
        return -ENOMEM;
    }

    for (i = 0; i < ae_dev->dev_specs.rss_ind_tbl_size; i++)
        rss_indir[i] = i % kinfo->rss_size;

    ret = hclge_plf_set_rss(handle, rss_indir, NULL, ETH_RSS_HASH_NO_CHANGE);
    if (ret != 0)
        dev_err(&hdev->pdev->dev, "set rss indir table fail, ret = %d\n",
            ret);

    kfree(rss_indir);
    rss_indir = NULL;

out:
    if (ret == 0)
        dev_info(&hdev->pdev->dev,
            "Channels changed, rss_size from %d to %d, tqps from %d to %d",
            cur_rss_size, kinfo->rss_size, cur_tqps,
            (kinfo->rss_size * kinfo->tc_info.num_tc));

    return ret;
}

STATIC int hclge_plf_tqps_get_sset_count(struct hnae3_handle *handle, int stringset)
{
    struct hnae3_knic_private_info *kinfo = &handle->kinfo;

    /* each tqp has TX & RX two queues */
    return kinfo->num_tqps * 2;  // sset count is double(2) of num_tqps
}

STATIC int hclge_plf_get_sset_count(struct hnae3_handle *handle, int stringset)
{
#define HCLGE_LOOPBACK_TEST_FLAGS (HNAE3_SUPPORT_APP_LOOPBACK | \
        HNAE3_SUPPORT_PHY_LOOPBACK | \
        HNAE3_SUPPORT_SERDES_SERIAL_LOOPBACK | \
        HNAE3_SUPPORT_SERDES_PARALLEL_LOOPBACK)

#if defined(CONFIG_PLATFORM_ASIC) && defined(CONFIG_PLATFORM_SUPPORT_SERDES)
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
#endif
    int count = 0;

    if (stringset == ETH_SS_TEST) {
        /* clear loopback bit flags at first */
        handle->flags = (handle->flags & (~HCLGE_LOOPBACK_TEST_FLAGS));

        count += 1;
        handle->flags |= HNAE3_SUPPORT_APP_LOOPBACK;
#if defined(CONFIG_PLATFORM_ASIC) && defined(CONFIG_PLATFORM_SUPPORT_SERDES)
        handle->flags |= HNAE3_SUPPORT_SERDES_SERIAL_LOOPBACK;
        handle->flags |= HNAE3_SUPPORT_SERDES_PARALLEL_LOOPBACK;
        count += 2; // add 2 mode : serial&parallel

        if (((hdev->hw.mac.phydev != NULL) && (hdev->hw.mac.phydev->drv != NULL) &&
                (hdev->hw.mac.phydev->drv->set_loopback != NULL))) {
            count += 1;
            handle->flags |= HNAE3_SUPPORT_PHY_LOOPBACK;
        }
#endif
    } else if (stringset == ETH_SS_STATS) {
        count = ARRAY_SIZE(g_mac_stats_string) +
                hclge_plf_tqps_get_sset_count(handle, stringset);
    }

    return count;
}

STATIC u8 *hclge_plf_tqps_get_strings(struct hnae3_handle *handle, u8 *data)
{
    struct hnae3_knic_private_info *kinfo = &handle->kinfo;
    u8 *buff = data;
    u16 i;

    for (i = 0; i < kinfo->num_tqps; i++) {
        struct hclge_plf_tqp *tqp = container_of(handle->kinfo.tqp[i], struct hclge_plf_tqp, q);
        (void)snprintf_s(buff, ETH_GSTRING_LEN, ETH_GSTRING_LEN - 1, "txq%u_pktnum_rcd", tqp->index);
        buff = buff + ETH_GSTRING_LEN;
    }

    for (i = 0; i < kinfo->num_tqps; i++) {
        struct hclge_plf_tqp *tqp = container_of(kinfo->tqp[i], struct hclge_plf_tqp, q);
        (void)snprintf_s(buff, ETH_GSTRING_LEN, ETH_GSTRING_LEN - 1, "rxq%u_pktnum_rcd", tqp->index);
        buff = buff + ETH_GSTRING_LEN;
    }

    return buff;
}

STATIC u8 *hclge_plf_get_mac_strings(struct hclge_plf_dev *hdev, u32 stringset,
    const struct hclge_comm_stats_str strs[],
    u32 size, u8 *data)
{
    char *buff = (char *)data;
    u32 i;

    if (stringset != ETH_SS_STATS)
        return buff;

    for (i = 0; i < size; i++) {
        (void)snprintf_s(buff, ETH_GSTRING_LEN, ETH_GSTRING_LEN - 1, "%s", strs[i].desc);
        buff = buff + ETH_GSTRING_LEN;
    }

    return (u8 *)buff;
}

STATIC void hclge_plf_get_strings(struct hnae3_handle *handle, u32 stringset,
    u8 **data)
{
    struct hclge_plf_vport *vport = hclge_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    u8 *p = *data;
    u32 size;

    if (stringset == ETH_SS_STATS) {
        size = (u32)ARRAY_SIZE(g_mac_stats_string);
        p = hclge_plf_get_mac_strings(hdev, stringset, g_mac_stats_string,
            size, p);
        (void)hclge_plf_tqps_get_strings(handle, p);
    }

    *data = p;
}

STATIC int hclge_plf_set_vport_mtu(struct hclge_plf_vport *vport, int new_mtu)
{
    struct hclge_plf_dev *hdev = vport->back;
    int max_frm_size, ret;

    /* HW supprt 2 layer vlan */
    max_frm_size = new_mtu + ETH_HLEN + ETH_FCS_LEN + 2 * VLAN_HLEN;
    if (max_frm_size < HCLGE_PLF_MAC_MIN_FRAME || max_frm_size > hdev->ae_dev->dev_specs.max_frm_size) {
        dev_err(&hdev->pdev->dev, "max_frm_size is invalid, max_frm_size = %d\n", max_frm_size);
        return -EINVAL;
    }

    max_frm_size = max(max_frm_size, HCLGE_MAC_DEFAULT_FRAME);
    mutex_lock(&hdev->vport_lock);

    ret = hclge_plf_notify_client(hdev, HNAE3_DOWN_CLIENT);
    if (ret != 0) {
        mutex_unlock(&hdev->vport_lock);
        return ret;
    }

    xge_mac_set_mtu(hdev, max_frm_size);

    hdev->mps = max_frm_size;
    vport->mps = max_frm_size;

    hclge_notify_client(hdev, HNAE3_UP_CLIENT);
    mutex_unlock(&hdev->vport_lock);
    return 0;
}

STATIC int hclge_plf_set_mtu(struct hnae3_handle *handle, int new_mtu)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);

    return hclge_plf_set_vport_mtu(vport, new_mtu);
}

STATIC u32 hclge_plf_get_max_channels(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    return min_t(u32, hdev->pf_rss_size_max, vport->alloc_tqps);
}

STATIC void hclge_plf_get_channels(struct hnae3_handle *handle,
    struct ethtool_channels *ch)
{
    ch->max_combined = hclge_plf_get_max_channels(handle);
    ch->other_count = 1;
    ch->max_other = 1;
    ch->combined_count = handle->kinfo.rss_size;
}

STATIC int hclge_plf_set_vlan_table(struct hnae3_handle *handle, u16 vlan_id, u32 loc, u16 mask, u64 action)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    struct ethtool_rxnfc rxnfc_cmd;
    int ret;
    u8 i;

    (void)memset_s(&rxnfc_cmd, sizeof(rxnfc_cmd), 0, sizeof(rxnfc_cmd));
    rxnfc_cmd.fs.flow_type = FLOW_EXT | ETHER_FLOW;
    rxnfc_cmd.fs.location = loc;
    rxnfc_cmd.fs.h_ext.vlan_tci = cpu_to_be16(vlan_id);
    rxnfc_cmd.fs.m_ext.vlan_tci = cpu_to_be16(mask);
    rxnfc_cmd.fs.ring_cookie = action;
    for (i = 0; i < ETH_ALEN; i++) {
        rxnfc_cmd.fs.h_u.ether_spec.h_dest[i] = hdev->hw.mac.mac_addr[i];
        rxnfc_cmd.fs.m_u.ether_spec.h_dest[i] = 0xff;
    }

    set_bit(HCLGE_STATE_FD_USE_RSS_UPDATING, &hdev->state);
    ret = hclge_plf_add_fd_entry(handle, &rxnfc_cmd);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "Failed to set vlan table, vlan_id = %d, loc = %u\n", vlan_id, loc);
        return ret;
    }

    return 0;
}

static int hclge_plf_set_vlan_filter_hw(struct hnae3_handle *handle, u16 vlan_id, bool add)
{
#define VLAN_START_LOC 12
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    u32 vlan_start_loc = VLAN_START_LOC;
    struct ethtool_rxnfc tmp;
    bool exist_flag = false;
    u32 unused_loc = 0;
    int ret;
    u8 i;

    spin_lock_bh(&hdev->vlan_table_lock);
    for (i = 0; i < VLAN_TABLE_NUM - 1; i++) {
        if (vlan_start_loc > i && hdev->vlan_cfg_tbl[i].loc == 0 && unused_loc == 0) {
            unused_loc = vlan_start_loc - i;
        }
        if (hdev->vlan_cfg_tbl[i].vlan_id == vlan_id) {
            exist_flag = true;
            break;
        }
    }

    if (add && !exist_flag) {
        if (unused_loc == 0) {
            dev_err(&hdev->pdev->dev, "Failed to set vlan cfg because vlan table is full, vlan_id = %d\n",
                    vlan_id);
            spin_unlock_bh(&hdev->vlan_table_lock);
            return -EOPNOTSUPP;
        }

        ret = hclge_plf_set_vlan_table(handle, vlan_id, unused_loc, VLAN_ID_MASK, 0);
        if (ret != 0) {
            dev_err(&hdev->pdev->dev, "Failed to set vlan cfg, vlan id = %d, ret = %d\n", vlan_id, ret);
            spin_unlock_bh(&hdev->vlan_table_lock);
            return ret;
        }
        hdev->vlan_cfg_tbl[vlan_start_loc - unused_loc].vlan_id = vlan_id;
        hdev->vlan_cfg_tbl[vlan_start_loc - unused_loc].loc = unused_loc;
    } else if (!add && exist_flag) {
        tmp.fs.location = hdev->vlan_cfg_tbl[i].loc;
        ret = hclge_plf_del_fd_entry(handle, &tmp);
        if (ret != 0) {
            dev_err(&hdev->pdev->dev, "Failed to delete vlan cfg, vlan id = %d, ret = %d\n", vlan_id, ret);
            spin_unlock_bh(&hdev->vlan_table_lock);
            return ret;
        }
        hdev->vlan_cfg_tbl[i].vlan_id = 0;
        hdev->vlan_cfg_tbl[i].loc = 0;
    } else {
        /* nothing */
    }

    spin_unlock_bh(&hdev->vlan_table_lock);
    return 0;
}

STATIC int hclge_plf_set_vlan_filter(struct hnae3_handle *handle, __be16 proto, u16 vlan_id, bool is_kill)
{
    int ret;

    if (is_kill && vlan_id != 0)
        ret = hclge_plf_set_vlan_filter_hw(handle, vlan_id, false); /* RX vlan not receive */
    else
        ret = hclge_plf_set_vlan_filter_hw(handle, vlan_id, true); /* RX vlan receive */

    return ret;
}

STATIC int hclge_plf_enable_vlan_filter(struct hnae3_handle *handle, bool enable)
{
#define CFG_VLAN_FILTER_LOC 14
#define VLAN_FILTER_BASE_NUM 2
#ifndef CONFIG_PLATFORM_MDC
    u32 location[VLAN_FILTER_BASE_NUM] = { CFG_VLAN_FILTER_LOC, CFG_VLAN_FILTER_LOC - 1 };
#endif
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    int ret;
    u8 i;

    u16 mask[VLAN_FILTER_BASE_NUM] = { 0, VLAN_ID_MASK };
    u16 vlan_id[VLAN_FILTER_BASE_NUM] = {0};
    u64 action[VLAN_FILTER_BASE_NUM];

    action[0] = enable ? RX_CLS_FLOW_DISC : 0;
    action[1] = 0;

    for (i = 0; i < VLAN_FILTER_BASE_NUM; i++) {
        ret = hclge_plf_set_vlan_table(handle, vlan_id[i], location[i], mask[i], action[i]);
        if (ret != 0) {
            dev_err(&hdev->pdev->dev, "Failed to %s vlan filter\n", enable ? "enable" : "disable");
            return ret;
        }
    }

    return 0;
}

void hclge_plf_init_vlan_config(struct hclge_plf_dev *hdev)
{
    struct hnae3_handle *handle = &hdev->vport[0].nic;

    hclge_plf_enable_vlan_filter(handle, true);
}

static int hclge_plf_update_vlan_filter(struct hnae3_handle *handle)
{
#define VLAN_START_LOC 12
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    u32 vlan_loc = 0;
    int ret;
    u8 i;

    spin_lock_bh(&hdev->vlan_table_lock);
    for (i = 0; i < VLAN_TABLE_NUM - 1; i++) {
        if (hdev->vlan_cfg_tbl[i].loc == 0) {
            continue;
        }

        vlan_loc = VLAN_START_LOC - i;

        ret = hclge_plf_set_vlan_table(handle, hdev->vlan_cfg_tbl[i].vlan_id, vlan_loc, VLAN_ID_MASK, 0);
        if (ret != 0) {
            dev_err(&hdev->pdev->dev,
                "Failed to update vlan cfg, vlan id = %d, ret = %d\n",
                hdev->vlan_cfg_tbl[i].vlan_id,
                ret);
            spin_unlock_bh(&hdev->vlan_table_lock);
            return ret;
        }
    }
    spin_unlock_bh(&hdev->vlan_table_lock);

    return 0;
}

STATIC void hclge_plf_get_pauseparam(struct hnae3_handle *handle, u32 *auto_neg,
    u32 *rx_en, u32 *tx_en)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    *auto_neg = 0;

    if (hdev->tm_info.fc_mode == HCLGE_FC_RX_PAUSE) {
        *rx_en = 1;
        *tx_en = 0;
    } else if (hdev->tm_info.fc_mode == HCLGE_FC_TX_PAUSE) {
        *tx_en = 1;
        *rx_en = 0;
    } else if (hdev->tm_info.fc_mode == HCLGE_FC_FULL) {
        *rx_en = 1;
        *tx_en = 1;
    } else {
        *rx_en = 0;
        *tx_en = 0;
    }
}

STATIC void hclge_plf_record_user_pauseparam(struct hclge_plf_dev *hdev,
    u32 rx_en, u32 tx_en)
{
    if (rx_en && tx_en)
        hdev->fc_mode_last_time = HCLGE_FC_FULL;
    else if (rx_en && !tx_en)
        hdev->fc_mode_last_time = HCLGE_FC_RX_PAUSE;
    else if (!rx_en && tx_en)
        hdev->fc_mode_last_time = HCLGE_FC_TX_PAUSE;
    else
        hdev->fc_mode_last_time = HCLGE_FC_NONE;

    hdev->tm_info.fc_mode = hdev->fc_mode_last_time;
}

static void hclge_plf_set_flowctrl_adv(struct hclge_plf_dev *hdev, u32 rx_en, u32 tx_en)
{
    struct phy_device *phydev = hdev->hw.mac.phydev;

    if (!phydev)
        return;

#ifdef HAS_LINK_MODE_OPS
    phy_set_asym_pause(phydev, rx_en, tx_en);
#else
    phydev->advertising &= ~(ADVERTISED_Pause | ADVERTISED_Asym_Pause);

    if (rx_en)
        phydev->advertising |= ADVERTISED_Pause | ADVERTISED_Asym_Pause;

    if (tx_en)
        phydev->advertising ^= ADVERTISED_Asym_Pause;
#endif
}

static int hclge_plf_set_pauseparam(struct hnae3_handle *handle, u32 auto_neg,
    u32 rx_en, u32 tx_en)
{
    struct hclge_plf_vport *vport = hclge_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    struct phy_device *phydev = hdev->hw.mac.phydev;

    hclge_plf_set_flowctrl_adv(hdev, rx_en, tx_en);

    hclge_plf_record_user_pauseparam(hdev, rx_en, tx_en);

    if (!auto_neg) {
        mac_pause_en_cfg(hdev, tx_en, rx_en);
        return 0;
    }

    if (phydev)
        return phy_start_aneg(phydev);

    return -EOPNOTSUPP;
}

#ifdef CONFIG_PLATFORM_ASIC
int hclge_plf_cfg_flowctrl(struct hclge_plf_dev *hdev)
{
    struct phy_device *phydev = hdev->hw.mac.phydev;
    u16 remote_advertising = 0;
    u16 local_advertising = 0;
    u32 rx_pause, tx_pause;
    u8 flowctl;

    if (!phydev->link || !phydev->autoneg) {
        return 0;
    }
#ifdef HAS_LINK_MODE_OPS
    local_advertising = linkmode_adv_to_lcl_adv_t(phydev->advertising);
#else
    if ((phydev->advertising & ADVERTISED_Pause) != 0) {
        local_advertising = ADVERTISE_PAUSE_CAP;
    }

    if ((phydev->advertising & ADVERTISED_Asym_Pause) != 0) {
        local_advertising |= ADVERTISE_PAUSE_ASYM;
    }
#endif

    if (phydev->pause != 0) {
        remote_advertising = LPA_PAUSE_CAP;
    }

    if (phydev->asym_pause != 0) {
        remote_advertising |= LPA_PAUSE_ASYM;
    }

    flowctl = mii_resolve_flowctrl_fdx(local_advertising, remote_advertising);
    tx_pause = flowctl & FLOW_CTRL_TX;
    rx_pause = flowctl & FLOW_CTRL_RX;

    if (phydev->duplex == HCLGE_MAC_HALF) {
        tx_pause = 0;
        rx_pause = 0;
    }

    mac_pause_en_cfg(hdev, tx_pause, rx_pause);

    return 0;
}
#endif

STATIC void hclge_plf_set_vlan_rx_offload_cfg(struct hclge_plf_vport *vport)
{
    struct hclge_plf_dev *hdev = vport->back;
    u_ppp_egr_port_attr_cfg port_attr_cfg;
    u_ppp_egr_port_attr port_attr;

    hclge_write_dev(&hdev->hw, PA_VLAN_SELECT_RX_REG(hdev->id), 1);

    port_attr.bits.cfg_egr_vlan_strip_en = (vport->rxvlan_cfg.strip_tag1_en << 1) |
        vport->rxvlan_cfg.strip_tag2_en;
    port_attr.bits.cfg_egr_vlan_strip_discard_en = (vport->rxvlan_cfg.strip_tag1_discard_en << 1) |
        vport->rxvlan_cfg.strip_tag2_discard_en;
    port_attr.bits.cfg_egr_vlan_prionly = (vport->rxvlan_cfg.vlan1_vlan_prionly << 1) |
        vport->rxvlan_cfg.vlan2_vlan_prionly;
    hclge_write_dev(&hdev->hw, PPP_EGR_PORT_ATTR_REG, port_attr.value);

    port_attr_cfg.bits.cfg_egr_port_attr_addr = hdev->id;
    port_attr_cfg.bits.cfg_egr_port_attr_cmd_en = 1;
    port_attr_cfg.bits.cfg_egr_port_attr_cmd = 0; /* 0:write 1:read */
    hclge_write_dev(&hdev->hw, PPP_PPP_EGR_PORT_ATTR_CFG, port_attr_cfg.value);
    /* status bit3 -> 8, wait 100us */
    if (wait_hardware_done(hdev->hw.io_base + PPP_PPP_EGR_PORT_ATTR_CFG, 0x8, 0x0, 1, 100)) {
        dev_err(&hdev->pdev->dev, "failed to set rx vlan offload(0x%x)!\n", PPP_PPP_EGR_PORT_ATTR_CFG);
    }
    hclge_write_dev(&hdev->hw, PPP_PPP_EGR_PORT_ATTR_CFG, 0); /* After completing configuration, set en to 0 */
}

STATIC int hclge_plf_en_hw_strip_rxvtag(struct hnae3_handle *handle, bool enable)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);

    vport->rxvlan_cfg.strip_tag1_en = true;
    vport->rxvlan_cfg.strip_tag2_en = enable;
    vport->rxvlan_cfg.strip_tag2_discard_en = false;

    vport->rxvlan_cfg.strip_tag1_discard_en = false;
    vport->rxvlan_cfg.vlan1_vlan_prionly = false;
    vport->rxvlan_cfg.vlan2_vlan_prionly = false;
    vport->rxvlan_cfg.rx_vlan_offload_en = enable;

    hclge_plf_set_vlan_rx_offload_cfg(vport);

    return 0;
}

STATIC int hclge_plf_do_ioctl(struct hnae3_handle *handle, struct ifreq *ifr, int cmd)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    if (hdev->hw.mac.phydev != NULL) {
        return phy_mii_ioctl(hdev->hw.mac.phydev, ifr, cmd);
    }

    return -EOPNOTSUPP;
}

STATIC void hclge_plf_get_ksettings_an_result(struct hnae3_handle *handle,
    u8 *auto_neg, u32 *speed, u8 *duplex, u32 *lane_num)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    if (speed)
        *speed = hdev->hw.mac.speed;
    if (duplex)
        *duplex = hdev->hw.mac.duplex;
    if (auto_neg)
        *auto_neg = hdev->hw.mac.autoneg;
    if (lane_num)
        *lane_num = 0;
}

STATIC void hclge_plf_get_media_type(struct hnae3_handle *handle, u8 *media_type,
    u8 *module_type)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    if (media_type)
        *media_type = hdev->hw.mac.media_type;

    if (module_type)
        *module_type = hdev->hw.mac.module_type;
}

int hclge_plf_cfg_mac_speed_dup(struct hclge_plf_dev *hdev, int speed, u8 duplex)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u32 bak_speed = mac->speed;
    int ret;

    mac->speed = speed;

    if (hdev->hw.mac.mac_mode == XXVGE_MAC_MODE_RGMII) {
        hclge_plf_rgmii_clk_cfg(hdev);
    }

    ret = xxvge_mac_cfg_speed(hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "failed to config mac speed, ret = %d\n", ret);
        hdev->hw.mac.speed = bak_speed;
        if (hdev->hw.mac.mac_mode == XXVGE_MAC_MODE_RGMII) {
            hclge_plf_rgmii_clk_cfg(hdev);
        }
        (void)xxvge_mac_cfg_speed(hdev);

        return ret;
    }

    ret = hclge_plf_tm_schd_setup_hw(hdev); /* config shaper when speed changes. */
    if (ret != 0) {
        hdev->hw.mac.speed = bak_speed;
        if (hdev->hw.mac.mac_mode == XXVGE_MAC_MODE_RGMII) {
            hclge_plf_rgmii_clk_cfg(hdev);
        }
        (void)xxvge_mac_cfg_speed(hdev);

        (void)hclge_plf_tm_schd_setup_hw(hdev);
        dev_err(&hdev->pdev->dev, "tm scheduler setup failed, ret = %d\n", ret);
        return ret;
    }

    return 0;
}

STATIC int hclge_plf_cfg_mac_speed_dup_h(struct hnae3_handle *handle, int speed,
    u8 duplex, u8 lane_num)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    (void)lane_num;
    return hclge_plf_cfg_mac_speed_dup(hdev, speed, duplex);
}

STATIC int hclge_plf_get_rss_tuple_compat(struct hnae3_handle *handle,
    struct ethtool_rxfh_fields *cmd)
{
    (void)handle;
    (void)cmd;

    return -EOPNOTSUPP;
}

STATIC int hclge_plf_set_rss_tuple_compat(struct hnae3_handle *handle,
    const struct ethtool_rxfh_fields *cmd)
{
    (void)handle;
    (void)cmd;

    return -EOPNOTSUPP;
}

STATIC void hclge_plf_get_link_mode(struct hnae3_handle *handle,
    unsigned long *supported, unsigned long *advertising)
{
    unsigned int size = BITS_TO_LONGS(__ETHTOOL_LINK_MODE_MASK_NBITS);
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    unsigned int idx = 0;

    for (; idx < size; idx++) {
        supported[idx] = hdev->hw.mac.supported[idx];
        advertising[idx] = hdev->hw.mac.advertising[idx];
    }
}

static int hclge_plf_set_autoneg(struct hnae3_handle *handle, bool enable)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    if (!hdev->hw.mac.support_autoneg) {
        if (enable) {
            dev_err(&hdev->pdev->dev,
                "Autoneg is not supported by current port\n");
            return -EOPNOTSUPP;
        }
        return 0;
    }
    mac_sgmii_cfg_autoneg(hdev, enable);
    hdev->hw.mac.autoneg = enable;

    return 0;
}

static int hclge_plf_get_autoneg(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    return hdev->hw.mac.autoneg;
}

static int hclge_plf_restart_autoneg(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    int ret;

    dev_info(&hdev->pdev->dev, "Restart autoneg\n");

    ret = hclge_plf_notify_client(hdev, HNAE3_DOWN_CLIENT);
    if (ret != 0) {
        return ret;
    }

    return hclge_plf_notify_client(hdev, HNAE3_UP_CLIENT);
}

static int hclge_plf_halt_autoneg(struct hnae3_handle *handle, bool halt)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    if (hdev->hw.mac.support_autoneg && hdev->hw.mac.autoneg) {
        mac_sgmii_cfg_autoneg(hdev, !halt);
    }

    return 0;
}

STATIC void config_igu_egu_loopback(struct hclge_plf_dev *hdev, bool en)
{
    u32 igu_egu_loop_addr = IGU_EGU_CFG_IGU_EGU_LOOP_0_REG + hdev->id * 0x200;
    u_igu_egu_loop loop;

    /* disable IGU EGU txrx */
    loop.value = hclge_read_dev(&hdev->hw, igu_egu_loop_addr);
    loop.bits.cfg_tx2rx_loop = en;
    hclge_write_dev(&hdev->hw, igu_egu_loop_addr, loop.value);
}

static int hclge_plf_set_app_loopback(struct hclge_plf_dev *hdev, bool en)
{
    config_igu_egu_loopback(hdev, en);
    return 0;
}

#ifdef CONFIG_PLATFORM_ASIC
static int hclge_plf_mac_link_status_wait(struct hclge_plf_dev *hdev, bool en)
{
#define HCLGE_MAC_LINK_STATUS_NUM  100

    int link_ret;
    int i = 0;

    link_ret = en ? HCLGE_LINK_STATUS_UP : HCLGE_LINK_STATUS_DOWN;

    do {
        hclge_plf_update_link_status(hdev);
        if (hdev->hw.mac.link == link_ret)
            return 0;

        msleep(HCLGE_LINK_STATUS_MS);
    } while (++i < HCLGE_MAC_LINK_STATUS_NUM);

    return -EBUSY;
}

static int hclge_plf_set_phy_loopback(struct hclge_plf_dev *hdev, bool en)
{
    struct phy_device *phydev = hdev->hw.mac.phydev;
    int ret;

    if (!phydev) {
        return -ENOTSUPP;
    }

    if (en) {
        ret = hclge_plf_enable_phy_loopback(hdev, phydev);
    } else {
        ret = hclge_plf_disable_phy_loopback(hdev, phydev);
    }
    if (ret != 0) {
        dev_err(&hdev->pdev->dev,
            "set phy loopback fail, ret = %d\n", ret);
        return ret;
    }

    ret = hclge_plf_mac_link_status_wait(hdev, en);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev,
            "phy loopback config timeout\n");
    }

    return ret;
}

static int hclge_plf_set_serdes_loopback(struct hclge_plf_dev *hdev, bool en, enum hnae3_loop loop_mode)
{
    int ret;

    switch (loop_mode) {
        case HNAE3_LOOP_SERIAL_SERDES:
            /* serdes dfx api */
#if defined(CONFIG_PLATFORM_ASIC) && defined(CONFIG_PLATFORM_SUPPORT_SERDES)
            if (en) {
                ret = EnableTxToRxSerLpbk(1, hdev->hw.mac.ds_index, mac_speed_to_serdes_rate(hdev));
                if (ret != 0) {
                    dev_err(&hdev->pdev->dev, "enable serdes txrx serial loop fail, ret = %d.\n", ret);
                    return ret;
                }
            } else {
                DisableTxToRxSerLpbk(1, hdev->hw.mac.ds_index);
            }
#endif
            break;
        case HNAE3_LOOP_PARALLEL_SERDES:
            /* serdes dfx api */
#if defined(CONFIG_PLATFORM_ASIC) && defined(CONFIG_PLATFORM_SUPPORT_SERDES)
            if (en) {
                EnableTxToRxParLpbk(1, hdev->hw.mac.ds_index);
            } else {
                DisableTxToRxParLpbk(1, hdev->hw.mac.ds_index);
            }
#endif
            break;
        default:
            ret = -ENOTSUPP;
            dev_err(&hdev->pdev->dev, "Loop_mode %d is not supported\n", loop_mode);
            return ret;
    }

    ret = hclge_plf_mac_link_status_wait(hdev, en);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev,
            "serdes loopback config timeout\n");
    }

    return ret;
}
#endif

static int hclge_plf_set_loopback(struct hnae3_handle *handle, enum hnae3_loop loop_mode, bool en)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    int ret;
    u16 i;

    /* Loopback can be enabled in three places: SSU, MAC, and serdes. */
    switch (loop_mode) {
        case HNAE3_LOOP_APP:
            ret = hclge_plf_set_app_loopback(hdev, en);
            break;
#ifdef CONFIG_PLATFORM_ASIC
        case HNAE3_LOOP_SERIAL_SERDES:
        case HNAE3_LOOP_PARALLEL_SERDES:
            ret = hclge_plf_set_serdes_loopback(hdev, en, loop_mode);
            break;
        case HNAE3_LOOP_PHY:
            ret = hclge_plf_set_phy_loopback(hdev, en);
            break;
#endif
        default:
            ret = -ENOTSUPP;
            dev_err(&hdev->pdev->dev, "Loop_mode %d is not supported\n", loop_mode);
            break;
    }

    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "Failed to %s loop_mode: %d, ret = %d\n",
                en ? "enable" : "disable", loop_mode, ret);
        return ret;
    }

    if (en != 0) {
        mag_set_port_enable(hdev);
    } else {
        mag_set_port_disable(hdev);
    }

    ret = hclge_plf_tqp_enable(handle, en);
    if (ret != 0)
        dev_err(&hdev->pdev->dev, "Failed to %s tqp in loopback, ret = %d\n",
                en ? "enable" : "disable", ret);

    for (i = 0; i < handle->kinfo.num_tqps; i++) {
        hclge_write_dev(&hdev->hw, RCB_CFG_EN_REG((i + hdev->tqp_offset)), en);
    }

    return ret;
}

static int hclge_plf_set_promisc_hw(struct hnae3_handle *handle, struct ethtool_rxnfc *cmd, u8 loc,
                                    u64 action, u8 mask)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    int ret;
    u8 i;

    set_bit(HCLGE_STATE_FD_USE_RSS_UPDATING, &hdev->state);

    (void)memset_s(cmd, sizeof(*cmd), 0, sizeof(*cmd));
    cmd->fs.location = loc;
    cmd->fs.ring_cookie = action;
    cmd->fs.flow_type = ETHER_FLOW;
    for (i = 0; i < ETH_ALEN; i++) {
        cmd->fs.h_u.ether_spec.h_dest[i] = hdev->hw.mac.mac_addr[i];
        cmd->fs.m_u.ether_spec.h_dest[i] = mask;
    }
    ret = hclge_plf_add_fd_entry(handle, cmd);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "failed to cfg promisc hw, ret = %d\n", ret);
    }

    return ret;
}

STATIC int hclge_plf_set_bc_mc_promisc(struct hnae3_handle *handle)
{
#define CFG_BC_MC_LOC 8
    u8 bc_mc_mac_addr[ETH_ALEN] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    u8 bc_mc_mac_mask[ETH_ALEN] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    struct ethtool_rxnfc rxnfc_cmd;
    int ret;
    u8 i;

    set_bit(HCLGE_STATE_FD_USE_RSS_UPDATING, &hdev->state);
    rxnfc_cmd.fs.location = CFG_BC_MC_LOC;
    rxnfc_cmd.fs.ring_cookie = 0;
    rxnfc_cmd.fs.flow_type = ETHER_FLOW;
    for (i = 0; i < ETH_ALEN; i++) {
        rxnfc_cmd.fs.h_u.ether_spec.h_dest[i] = bc_mc_mac_addr[i];
        rxnfc_cmd.fs.m_u.ether_spec.h_dest[i] = bc_mc_mac_mask[i];
    }
    ret = hclge_plf_add_fd_entry(handle, &rxnfc_cmd);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "failed to cfg mulitcast and broadcast promisc, ret = %d\n", ret);
    }

    return ret;
}

static int hclge_plf_set_promisc_mode(struct hnae3_handle *handle, bool en_uc_pmc, bool en_mc_pmc)
{
#define CFG_PROMISC_LOC 15
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    struct ethtool_rxnfc rxnfc_cmd;
    int ret;

    if (en_uc_pmc && en_mc_pmc) {
        ret = hclge_plf_set_promisc_hw(handle, &rxnfc_cmd, CFG_PROMISC_LOC, 0, 0x0);
        if (ret != 0) {
            dev_err(&hdev->pdev->dev, "Failed to set promisc mode, ret = %d\n", ret);
            return ret;
        }

        /* if broadcast promisc enabled, vlan filter is always bypassed. */
        hclge_plf_enable_vlan_filter(handle, false);
    } else {
        ret = hclge_plf_set_promisc_hw(handle, &rxnfc_cmd, CFG_PROMISC_LOC, RX_CLS_FLOW_DISC, 0x0);
        if (ret != 0) {
            dev_err(&hdev->pdev->dev, "Failed to cancel promisc mode, ret = %d\n", ret);
            return ret;
        }

        /* if broadcast promisc disabled, vlan filter will open. */
        hclge_plf_enable_vlan_filter(handle, true);
    }

    /* multicast and broadcast packets can be received by default. */
    ret = hclge_plf_set_bc_mc_promisc(handle);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "Failed to cfg bc_mc_promisc, ret = %d\n", ret);
        return ret;
    }

    return 0;
}

static void hclge_plf_request_update_promisc_mode(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    u8 tmp_flags;
    int ret;

    tmp_flags = handle->netdev_flags;

    ret = hclge_plf_set_promisc_mode(handle, tmp_flags & HNAE3_UPE, tmp_flags & HNAE3_MPE);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "Failed to change promisc mode, ret = %d\n", ret);
    }
}

static int hclge_plf_get_sfp_eeprom_info(struct hclge_plf_dev *hdev, u32 offset, u32 len, u8 *data)
{
    return 0;
}

static int hns3_plf_get_module_eeprom(struct hnae3_handle *handle, u32 offset, u32 len, u8 *data)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    u32 read_len = 0;
    u16 data_len;

    if (hdev->hw.mac.media_type != HNAE3_MEDIA_TYPE_FIBER)
        return -EOPNOTSUPP;

    if (!hdev->hw.mac.sfp_present)
        return -ENXIO;

    while (read_len < len) {
        data_len = hclge_plf_get_sfp_eeprom_info(hdev, offset + read_len, len - read_len, data + read_len);
        if (!data_len)
            return -EIO;

        read_len += data_len;
    }

    return 0;
}

static void hclge_plf_restore_hw_table(struct hclge_plf_dev *hdev)
{
    struct hnae3_handle *handle = &hdev->vport[0].nic;

    set_bit(HCLGE_STATE_FD_USER_DEF_CHANGED, &hdev->state);
    hclge_plf_restore_fd_entries(handle);
}

STATIC int hclge_plf_vport_start(struct hclge_plf_vport *vport)
{
    struct hclge_plf_dev *hdev = vport->back;

    set_bit(HCLGE_VPORT_STATE_ALIVE, &vport->state);
    hclge_plf_restore_hw_table(hdev);

    return 0;
}

STATIC void hclge_plf_vport_stop(struct hclge_plf_vport *vport)
{
    clear_bit(HCLGE_VPORT_STATE_ALIVE, &vport->state);
}

static int hclge_plf_client_start(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);

    return hclge_plf_vport_start(vport);
}

static void hclge_plf_client_stop(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);

    hclge_plf_vport_stop(vport);
}

STATIC struct hnae3_ae_ops hclge_plf_ops = {
    .init_ae_dev = hclge_plf_init_ae_dev,
    .uninit_ae_dev = hclge_plf_uninit_ae_dev,
    .reset_prepare = hclge_plf_prepare_for_reset,
    .reset_done = hclge_plf_rebuild_for_reset,
    .init_client_instance = hclge_plf_init_client_instance,
    .uninit_client_instance = hclge_plf_uninit_client_instance,
    .map_ring_to_vector = hclge_plf_map_ring_to_vector,
    .unmap_ring_from_vector = hclge_plf_unmap_ring_frm_vector,
    .get_vector = hclge_plf_get_vector,
    .put_vector = hclge_plf_put_vector,
    .start = hclge_plf_ae_start,
    .stop = hclge_plf_ae_stop,
    .client_start = hclge_plf_client_start,
    .client_stop = hclge_plf_client_stop,
    .get_mac_addr = hclge_plf_get_mac_addr,
    .set_mac_addr = hclge_plf_set_mac_addr,
    .reset_queue = hclge_plf_reset_tqp,
    .get_stats = hclge_plf_get_stats,
    .update_stats = hclge_plf_update_stats,
    .get_strings = hclge_plf_get_strings,
    .get_sset_count = hclge_plf_get_sset_count,
    .get_fw_version = hclge_plf_get_fw_version,

    .reset_event = hclge_plf_reset_event,
    .get_reset_level = hclge_plf_get_reset_level,
    .set_default_reset_request = hclge_plf_set_def_reset_request,
    .ae_dev_resetting = hclge_plf_ae_dev_resetting,
    .reset_end = hclge_plf_reset_end,
    /* flow director */
    .add_fd_entry = hclge_plf_add_fd_user_entry,
    .del_fd_entry = hclge_plf_del_fd_user_entry,
    .get_fd_rule_cnt = hclge_plf_get_fd_rule_cnt,
    .get_fd_rule_info = hclge_plf_get_fd_rule_info,
    .get_fd_all_rules = hclge_plf_get_all_rules,
    .enable_fd = hclge_plf_enable_fd,

    .get_tqps_and_rss_info = hclge_plf_get_tqps_and_rss_info,

    .set_mtu = hclge_plf_set_mtu,
    .get_status = hclge_plf_get_status,

    .get_channels = hclge_plf_get_channels,
    .set_channels = hclge_plf_set_channels,
    .enable_vlan_filter = hclge_plf_enable_vlan_filter,
    .set_vlan_filter = hclge_plf_set_vlan_filter,

    .get_pauseparam = hclge_plf_get_pauseparam,
    .set_pauseparam = hclge_plf_set_pauseparam,

    .get_rss = hclge_plf_get_rss,
    .set_rss = hclge_plf_set_rss,
    .get_rss_tuple = hclge_plf_get_rss_tuple_compat,
    .set_rss_tuple = hclge_plf_set_rss_tuple_compat,

    .enable_hw_strip_rxvtag = hclge_plf_en_hw_strip_rxvtag,
    .do_ioctl = hclge_plf_do_ioctl,
    .get_ksettings_an_result = hclge_plf_get_ksettings_an_result,
    .cfg_mac_speed_dup_h = hclge_plf_cfg_mac_speed_dup_h,
    .get_media_type = hclge_plf_get_media_type,
    .get_link_mode = hclge_plf_get_link_mode,

    .dbg_read_cmd = hclge_plf_dbg_read_cmd,
    .set_autoneg = hclge_plf_set_autoneg,
    .get_autoneg = hclge_plf_get_autoneg,
    .restart_autoneg = hclge_plf_restart_autoneg,
    .halt_autoneg = hclge_plf_halt_autoneg,
    .set_loopback = hclge_plf_set_loopback,
    .set_promisc_mode = hclge_plf_set_promisc_mode,
    .request_update_promisc_mode = hclge_plf_request_update_promisc_mode,
    .get_module_eeprom = hns3_plf_get_module_eeprom,
    .set_timer_task = hclge_plf_set_timer_task,
#if defined(CONFIG_PLATFORM_ASIC) && defined(CONFIG_PLATFORM_SUPPORT_AUTONEG)
    .mac_connect_phy = hclge_plf_mac_connect_phy,
    .mac_disconnect_phy = hclge_plf_mac_disconnect_phy,
#endif
};

STATIC struct hnae3_ae_algo ae_algo_plf = {
    .ops = &hclge_plf_ops,
};

STATIC int hclge_plf_init(void)
{
    pr_info("%s is initializing\n", HCLGE_PLF_NAME);

    hclge_plf_wq = alloc_workqueue("%s", 0, 0, HCLGE_PLF_NAME);
    if (!hclge_plf_wq) {
        pr_err("%s: failed to create workqueue\n", HCLGE_PLF_NAME);
        return -ENOMEM;
    }

    hnae3_register_ae_algo(&ae_algo_plf);

    return 0;
}

STATIC void hclge_plf_exit(void)
{
    hnae3_unregister_ae_algo(&ae_algo_plf);
    destroy_workqueue(hclge_plf_wq);
}

STATIC int hclge_plf_after_probe(struct hnae3_ae_dev *ae_dev)
{
    struct hclge_plf_dev *hdev = ae_dev->priv;

    if (hdev == NULL || hdev->nic_client == NULL) {
        pr_err("probe failed, hdev[%pK] is null or hdev->nic_client is null\n", hdev);
        return -EIO;
    }

    hnae3_set_bit(g_hclge_plf_id_map, hdev->id, 1U);
    if (atomic_read(&g_hclge_plf_first_probe_flag) == false) {
        atomic_set(&g_hclge_plf_first_probe_flag, true);
    }

    return 0;
}

STATIC int hns3_platform_probe(struct platform_device *pdev)
{
    struct hnae3_ae_dev *ae_dev;
    int ret;

    dev_info(&pdev->dev, "Start to platform probe\n");

    ae_dev = devm_kzalloc(&pdev->dev, sizeof(*ae_dev), GFP_KERNEL);
    if (!ae_dev) {
        pr_err("failed to devm_kzalloc ae_dev.\n");
        return -ENOMEM;
    }

    ae_dev->plfdev = pdev;
    ae_dev->pdev = NULL;
    if (pdev->id_entry) {
        ae_dev->flag = pdev->id_entry->driver_data;
    }
    platform_set_drvdata(pdev, ae_dev);

    ret = hnae3_register_ae_dev(ae_dev);
    if (ret != 0) {
        pr_err("failed to register ae dev, ret(%d)\n", ret);
        platform_set_drvdata(pdev, NULL);
        return ret;
    }

    ret = hclge_plf_after_probe(ae_dev);
    if (ret != 0) {
        hnae3_unregister_ae_dev(ae_dev);
        platform_set_drvdata(pdev, NULL);
        return ret;
    }

    device_enable_async_suspend(&pdev->dev);
    dev_info(&pdev->dev, "hns3_platform_probe success.\n");

    return 0;
}

STATIC void hns3_platform_remove(struct platform_device *pdev)
{
    struct hnae3_ae_dev *ae_dev = platform_get_drvdata(pdev);

    dev_info(&pdev->dev, "Start to platform remove\n");

    hnae3_unregister_ae_dev(ae_dev);
    platform_set_drvdata(pdev, NULL);
}

STATIC const struct of_device_id hns3_platform_of_match[] = {
    {.compatible = "hisilicon,hns3-platform-device"},
    {},
};
MODULE_DEVICE_TABLE(of, hns3_platform_of_match);

STATIC int hns3_platform_suspend(struct device *dev)
{
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hclge_plf_dev *hdev = NULL;

    if (dev == NULL) {
        pr_err("[hclgeplf] hns3 suspend dev param is NULL\n");
        return -EINVAL;
    }

    ae_dev = dev_get_drvdata(dev);
    if (ae_dev == NULL || ae_dev->priv == NULL) {
        pr_err("[hclgeplf] hns3 suspend param ae_dev or ae_dev->priv is NULL\n");
        return -EINVAL;
    }

    hdev = ae_dev->priv;
    set_bit(HCLGE_STATE_INITING, &hdev->state);

    dev_info(dev, "[hclgeplf] Start to platform suspend\n");
    hclge_plf_blockdot_begin(hdev->id, g_hclge_plf_id_map, HCLGE_PLF_EXCEPID_DOT_SUSPEND, HCLGE_PLF_DOT_SUSPEND_END);
    hclge_plf_blockdot_record(hdev->id, HCLGE_PLF_DOT_SUSPEND_BEGIN);

    if (ae_dev && ae_dev->ops && ae_dev->ops->reset_prepare) {
        ae_dev->ops->reset_prepare(ae_dev, HNAE3_FUNC_RESET);
        hclge_plf_blockdot_record(hdev->id, HCLGE_PLF_DOT_SUSPEND_FUNC_RESET);
    }

    mac_cfg_common_link_down_int(hdev, 0);
    hclge_plf_blockdot_record(hdev->id, HCLGE_PLF_DOT_SUSPEND_DIS_MAC_DOWN);

    mac_cfg_common_link_up_int(hdev, 0);
    hclge_plf_blockdot_record(hdev->id, HCLGE_PLF_DOT_SUSPEND_DIS_MAC_UP);

    devm_free_irq(&hdev->pdev->dev, hdev->mac_irq, hdev);
    hclge_plf_blockdot_record(hdev->id, HCLGE_PLF_DOT_SUSPEND_RELEASE_MAC_IRQ);

    if (hdev->service_task.work.func)
        cancel_delayed_work_sync(&hdev->service_task);

    hclge_plf_blockdot_end(hdev->id, HCLGE_PLF_DOT_SUSPEND_END);
    return 0;
}

STATIC int hns3_platform_resume(struct device *dev)
{
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hclge_plf_dev *hdev = NULL;
    int ret;

    if (dev == NULL) {
        pr_err("[hclgeplf] hns3 resume param dev is NULL\n");
        return -EINVAL;
    }

    ae_dev = dev_get_drvdata(dev);
    if (ae_dev == NULL || ae_dev->priv == NULL) {
        pr_err("[hclgeplf] hns3 resume ae_dev or ae_dev->priv is NULL\n");
        return -EINVAL;
    }

    hdev = ae_dev->priv;
    dev_info(dev, "[hclgeplf] Start to platform resume\n");
    hclge_plf_blockdot_begin(hdev->id, g_hclge_plf_id_map, HCLGE_PLF_EXCEPID_DOT_RESUME, HCLGE_PLF_DOT_RESUME_END);
    hclge_plf_blockdot_record(hdev->id, HCLGE_PLF_DOT_RESUME_BEGIN);

    if (ae_dev && ae_dev->ops && ae_dev->ops->reset_done) {
        ae_dev->ops->reset_done(ae_dev);
        hclge_plf_blockdot_record(hdev->id, HCLGE_PLF_DOT_RESUME_RESET_DONE);
    }

    ret = devm_request_irq(&hdev->pdev->dev,
        hdev->mac_irq,
        hclge_plf_mac_irq_handle,
        IRQF_SHARED,
        hdev->misc_vector[1].name,
        hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "[hclgeplf] failed to request mac irq, ret = %d\n", ret);
        clear_bit(HCLGE_STATE_INITING, &hdev->state);
        return ret;
    }
    hclge_plf_blockdot_record(hdev->id, HCLGE_PLF_DOT_RESUME_REQUEST_MAC_IRQ);

    clear_bit(HCLGE_STATE_INITING, &hdev->state);
    hclge_plf_blockdot_end(hdev->id, HCLGE_PLF_DOT_RESUME_END);
    return 0;
}

STATIC int hns3_platform_resume_early(struct device *dev)
{
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hclge_plf_dev *hdev = NULL;
    u8 first_probe_id;
    int ret = 0;

    if (dev == NULL) {
        pr_err("[hclgeplf] hns3 resume param dev is NULL\n");
        return -EINVAL;
    }

    ae_dev = dev_get_drvdata(dev);
    if (ae_dev == NULL || ae_dev->priv == NULL) {
        pr_err("[hclgeplf] hns3 resume ae_dev or ae_dev->priv is NULL\n");
        return -EINVAL;
    }

    hdev = ae_dev->priv;
    first_probe_id = hclge_plf_get_first_probe_id();
    if (first_probe_id != hdev->id) {
        return 0;
    }

    dev_info(dev, "[hclgeplf] Start to platform resume early\n");

    hdev->init_flag = 0;
    ret = hclge_plf_nic_module_init(hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "[hclgeplf] failed to init nic module for resume early, ret = %d.\n", ret);
    }

    return ret;
}

STATIC const struct dev_pm_ops hns3_platform_pm_ops = {
    .suspend = hns3_platform_suspend,
    .resume = hns3_platform_resume,
    .resume_early = hns3_platform_resume_early,
};

STATIC struct platform_driver hns3_platform_driver = {
    .driver =
        {
            .name = hns3_platform_drv_name,
            .pm = &hns3_platform_pm_ops,
            .of_match_table = hns3_platform_of_match,
        },
    .probe = hns3_platform_probe,
    .remove = hns3_platform_remove,
};

STATIC int __init hns3_platform_init(void)
{
    int ret;

    pr_info("[hclge_plf]: %s: %s - version\n", hns3_platform_drv_name, hns3_platform_drv_string);
    pr_info("[hclge_plf]: %s: %s\n", hns3_platform_drv_name, hns3_platform_copyright);
    ret = hclge_plf_bbox_init();
    if (ret != 0) {
        pr_err("bbox init fail! ret:%d\n", ret);
        return ret;
    }

    platform_client.type = HNAE3_CLIENT_KNIC;
    (void)snprintf_s(
        platform_client.name, HNAE3_CLIENT_NAME_LENGTH, HNAE3_CLIENT_NAME_LENGTH - 1, "%s", hns3_platform_drv_name);

    platform_client.ops = &platform_client_ops;

    INIT_LIST_HEAD(&platform_client.node);

    ret = hnae3_register_client(&platform_client);
    if (ret != 0) {
        pr_err("failed to register platform client, ret(%d)\n", ret);
        hclge_plf_bbox_uninit();
        return ret;
    }

    ret = hclge_plf_init();
    if (ret != 0) {
        pr_err("failed to register ae algo, ret(%d)\n", ret);
        goto err_reg_platform_drv;
    }

    ret = platform_driver_register(&hns3_platform_driver);
    if (ret != 0) {
        pr_err("failed to register platform driver, ret(%d)\n", ret);
        goto err_init_plf;
    }

#ifndef DEFINE_HNS_LLT
    ret = hclge_plf_register_dms_node();
    if (ret != 0) {
        pr_err("failed to register dms node, ret(%d)\n", ret);
        goto err_plf_driver_reg;
    }
#endif

#ifndef CONFIG_PLATFORM_MDC
    ret = hclge_plf_qos_node_register();
    if (ret != 0) {
        pr_err("failed to register qos node, ret(%d)\n", ret);
        goto err_plf_qos_dms_node;
    }
#endif

    pr_info("register platform driver success!\n");

    return 0;

#ifndef CONFIG_PLATFORM_MDC
err_plf_qos_dms_node:
    hclge_plf_unregister_dms_node();
#endif
err_plf_driver_reg:
    platform_driver_unregister(&hns3_platform_driver);
err_init_plf:
    hclge_plf_exit();
err_reg_platform_drv:
    hnae3_unregister_client(&platform_client);
    hclge_plf_bbox_uninit();
    return ret;
}
module_init(hns3_platform_init);

STATIC void __exit hns3_platform_exit(void)
{
#ifndef CONFIG_PLATFORM_MDC
    hclge_plf_qos_node_unregister();
#endif
#ifndef DEFINE_HNS_LLT
    hclge_plf_unregister_dms_node();
#endif
    platform_driver_unregister(&hns3_platform_driver);
    hclge_plf_exit();
    hnae3_unregister_client(&platform_client);
    hclge_plf_bbox_excep_report(0, HCLGE_PLF_EXCEPID_NIC_UNINIT);
    hclge_plf_bbox_uninit();
}
module_exit(hns3_platform_exit);

MODULE_DESCRIPTION("HNS3: Hisilicon Ethernet Platform Driver");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform: hns3-platform-nic");
MODULE_SOFTDEP("pre: drv_e2prom");
