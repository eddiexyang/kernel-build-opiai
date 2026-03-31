/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: hclge_plf_rss
 * Author: huawei
 * Create: 2022-04-03
 */

#include <linux/etherdevice.h>
#include <linux/of.h>

#include "hclge_plf_main.h"
#include "reg_ppp.h"

#define hclge_rss_write_reg(base, reg, value)                                   \
    do {                                                                        \
        hclge_write_reg(base, reg, value);                                      \
    } while (0)

static const u8 hclge_plf_hash_key[] = {
    0x6D, 0x5A, 0x56, 0xDA, 0x25, 0x5B, 0x0E, 0xC2,
    0x41, 0x67, 0x25, 0x3D, 0x43, 0xA3, 0x8F, 0xB0,
    0xD0, 0xCA, 0x2B, 0xCB, 0xAE, 0x7B, 0x30, 0xB4,
    0x77, 0xCB, 0x2D, 0xA3, 0x80, 0x30, 0xF2, 0x0C,
    0x6A, 0x42, 0xB7, 0x3B, 0xBE, 0xAC, 0x01, 0xFA
};

static void hclge_plf_get_rss_type(struct hclge_plf_vport *vport)
{
    if (vport->rss_tuple_sets.ipv4_tcp_en ||
        vport->rss_tuple_sets.ipv4_udp_en ||
        vport->rss_tuple_sets.ipv4_sctp_en ||
        vport->rss_tuple_sets.ipv6_tcp_en ||
        vport->rss_tuple_sets.ipv6_udp_en ||
        vport->rss_tuple_sets.ipv6_sctp_en) {
        vport->nic.kinfo.rss_type = PKT_HASH_TYPE_L4;
    } else if (vport->rss_tuple_sets.ipv4_fragment_en ||
        vport->rss_tuple_sets.ipv6_fragment_en) {
        vport->nic.kinfo.rss_type = PKT_HASH_TYPE_L3;
    } else {
        vport->nic.kinfo.rss_type = PKT_HASH_TYPE_NONE;
    }
}

static int hclge_plf_set_rss_algo_key_tuple(struct hclge_plf_dev *hdev, const u8 hfunc, const u8 *key)
{
    u_ppp_rss_hash_algo hash_algo;
    u_ppp_rss_type_ipv4 type_ipv4;
    u_ppp_rss_type_ipv6 type_ipv6;
    u_ppp_rss_hash_cfg hash_cfg;
    u32 key_counts;
    u32 reg_addr;
    u32 data = 0;
    u32 i;

    key_counts = hdev->ae_dev->dev_specs.rss_key_size;
    reg_addr = PPP_REG_BASE + PPP_RSS_HASH_KEY_9;
    for (i = 0; i < (key_counts / 4); i++) { // each reg has 4 bytes(u32)
        data = htonl(*(((u32 *)key) + i));
        hclge_rss_write_reg(hdev->hw.io_base, reg_addr, data);
        reg_addr -= 0x4;
    }

    reg_addr = PPP_REG_BASE + PPP_RSS_HASH_ALGO;
    hash_algo.value = 0;
    if (hfunc == HCLGE_PLF_RSS_HASH_ALGO_TOEPLITZ) {
        hash_algo.bits.cfg_rss_hash_algo = 0x2; // 0x2 : bit1(Symmetric Hash)
        hash_algo.bits.cfg_rss_sym_sel = 0x1; // XOR对称
    } else { // HCLGE_PLF_RSS_HASH_ALGO_SIMPLE
        hash_algo.bits.cfg_rss_hash_algo = 0x1;  // simple hash & index[7:0] = tmp1[7:0], 此配置与友商兼容；
    }
    hclge_rss_write_reg(hdev->hw.io_base, reg_addr, hash_algo.value);

    reg_addr = PPP_REG_BASE + PPP_RSS_TYPE_IPV4;
    type_ipv4.value = 0;
    type_ipv4.bits.cfg_rss_tcpipv4_en = hdev->vport->rss_tuple_sets.ipv4_tcp_en;
    type_ipv4.bits.cfg_rss_udpipv4_en = hdev->vport->rss_tuple_sets.ipv4_udp_en;
    type_ipv4.bits.cfg_rss_sctpipv4_en = hdev->vport->rss_tuple_sets.ipv4_sctp_en;
    type_ipv4.bits.cfg_rss_ipv4_en = hdev->vport->rss_tuple_sets.ipv4_fragment_en;
    hclge_rss_write_reg(hdev->hw.io_base, reg_addr, type_ipv4.value);

    reg_addr = PPP_REG_BASE + PPP_RSS_TYPE_IPV6;
    type_ipv6.value = 0;
    type_ipv6.bits.cfg_rss_tcpipv6_en = hdev->vport->rss_tuple_sets.ipv6_tcp_en;
    type_ipv6.bits.cfg_rss_udpipv6_en = hdev->vport->rss_tuple_sets.ipv6_udp_en;
    type_ipv6.bits.cfg_rss_sctpipv6_en = hdev->vport->rss_tuple_sets.ipv6_sctp_en;
    type_ipv6.bits.cfg_rss_ipv6_en = hdev->vport->rss_tuple_sets.ipv6_fragment_en;
    hclge_rss_write_reg(hdev->hw.io_base, reg_addr, type_ipv6.value);
    hclge_plf_get_rss_type(hdev->vport);

    reg_addr = PPP_REG_BASE + PPP_RSS_HASH_CFG;
    hash_cfg.value = 0;
    hash_cfg.bits.cfg_rss_hash_cmd = 0; /* write */
    hash_cfg.bits.cfg_rss_hash_cmd_en = 1; /* enable cmd */
    hash_cfg.bits.cfg_rss_hash_fun_id = hdev->id; /* port id */
    hclge_rss_write_reg(hdev->hw.io_base, reg_addr, hash_cfg.value);
    if (wait_hardware_done(hdev->hw.io_base + reg_addr, 0x8, 0x0, 1, 100)) { /* status bit3 -> 8, wait 100us */
        dev_err(&hdev->pdev->dev, "failed to set hash_cfg(0x%x)!\n", reg_addr);
    }
    hclge_rss_write_reg(hdev->hw.io_base, reg_addr, 0); /* After completing configuration, set en to 0 */

    return 0;
}

static int hclge_plf_set_rss_indir_table(struct hclge_plf_dev *hdev, const u16 *indir)
{
    u_ppp_rss_idt_data idt_data;
    u_ppp_rss_idt_cfg idt_cfg;
    int rss_cfg_tbl_num;
    u8 idt_offset_start;
    u32 idt_cfg_reg_addr;
    u32 idt_data_reg_addr;
    u16 qid;
    int i;
    u32 j;

    rss_cfg_tbl_num = hdev->ae_dev->dev_specs.rss_ind_tbl_size / HCLGE_PLF_RSS_CFG_TBL_SIZE;
    idt_offset_start = hdev->rss_indir_tbl_offset / HCLGE_PLF_RSS_CFG_TBL_SIZE;
    idt_cfg_reg_addr = PPP_REG_BASE + PPP_RSS_IDT_CFG;
    idt_data_reg_addr = PPP_REG_BASE + PPP_RSS_IDT_DATA;

    for (i = 0; i < rss_cfg_tbl_num; i++) {
        for (j = 0; j < HCLGE_PLF_RSS_CFG_TBL_SIZE; j++) {
            qid = indir[i * HCLGE_PLF_RSS_CFG_TBL_SIZE + j] & 0xf;
            idt_data.value = 0;
            idt_data.bits.cfg_rss_idt_qid = qid;
            idt_data.bits.cfg_rss_idt_qid_vld = 1;
            hclge_rss_write_reg(hdev->hw.io_base, (idt_data_reg_addr + j * 0x4), idt_data.value);
        }

        idt_cfg.value = 0;
        idt_cfg.bits.cfg_rss_idt_cmd = 0; /* 0:write 1:read */
        idt_cfg.bits.cfg_rss_idt_cmd_en = 1;
        idt_cfg.bits.cfg_rss_idt_offset = (i + idt_offset_start);
        hclge_rss_write_reg(hdev->hw.io_base, idt_cfg_reg_addr, idt_cfg.value);
        /* status bit3 -> 8, wait 100us */
        if (wait_hardware_done(hdev->hw.io_base + idt_cfg_reg_addr, 0x8, 0x0, 1, 100)) {
            dev_err(&hdev->pdev->dev, "failed to set rss_indir_table(0x%x)!\n", idt_cfg_reg_addr);
        }
        hclge_rss_write_reg(hdev->hw.io_base, idt_cfg_reg_addr, 0); /* After completing configuration, set en to 0 */
    }
    return 0;
}

int hclge_plf_set_rss_tc_mode(struct hclge_plf_dev *hdev, u32 *tc_valid, u32 *tc_size, u32 *tc_offset)
{
    u_ppp_rss_tc_mode_data tc_mode_data;
    u_ppp_rss_tc_mode_cfg tc_mode_cfg;
    u32 tc_mode_data_reg_addr;
    u32 tc_mode_cfg_reg_addr;
    int i;

    tc_mode_data_reg_addr = PPP_REG_BASE + PPP_RSS_TC_MODE_DATA;
    tc_mode_cfg_reg_addr = PPP_REG_BASE + PPP_RSS_TC_MODE_CFG;
    for (i = 0; i < HCLGE_MAX_TC_NUM; i++) {
        tc_mode_data.value = 0;
        tc_mode_data.bits.cfg_rss_tc_size = tc_size[i];
        tc_mode_data.bits.cfg_rss_tc_offset = tc_offset[i];
        tc_mode_data.bits.cfg_rss_tc_valid = tc_valid[i];
        hclge_rss_write_reg(hdev->hw.io_base, tc_mode_data_reg_addr, tc_mode_data.value);

        tc_mode_cfg.value = 0;
        tc_mode_cfg.bits.cfg_rss_tc_mode_cmd = 0; /* 0:write 1:read */
        tc_mode_cfg.bits.cfg_rss_tc_mode_cmd_en = 1;
        tc_mode_cfg.bits.cfg_rss_tc_mode_tc_id = i;
        tc_mode_cfg.bits.cfg_rss_tc_mode_fun_id = hdev->id;
        hclge_rss_write_reg(hdev->hw.io_base, tc_mode_cfg_reg_addr, tc_mode_cfg.value);
        /* status bit3 -> 8, wait 100us */
        if (wait_hardware_done(hdev->hw.io_base + tc_mode_cfg_reg_addr, 0x8, 0x0, 1, 100)) {
            dev_err(&hdev->pdev->dev, "failed to set tc_mode_cfg(0x%x)!\n", tc_mode_cfg_reg_addr);
        }
        /* After completing configuration, set en to 0 */
        hclge_rss_write_reg(hdev->hw.io_base, tc_mode_cfg_reg_addr, 0);
    }

    return 0;
}

int hclge_plf_get_rss(struct hnae3_handle *handle, u32 *indir, u8 *key, u8 *hfunc)
{
    struct hnae3_ae_dev *ae_dev = platform_get_drvdata(handle->plfdev);
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    int i;

    if (!ae_dev) {
        dev_err(&hdev->pdev->dev, "ae_dev is NULL\n");
        return -EINVAL;
    }

    /* Get hash algorithm */
    if (hfunc) {
        switch (vport->rss_algo) {
            case HCLGE_PLF_RSS_HASH_ALGO_TOEPLITZ:
                *hfunc = ETH_RSS_HASH_TOP;
                break;
            case HCLGE_PLF_RSS_HASH_ALGO_SIMPLE:
                *hfunc = ETH_RSS_HASH_XOR;
                break;
            default:
                *hfunc = ETH_RSS_HASH_UNKNOWN;
                break;
        }
    }

    /* Get the RSS Key required by the user */
    if (key)
        (void)memcpy_s(key, ae_dev->dev_specs.rss_key_size,
                    vport->rss_hash_key, ae_dev->dev_specs.rss_key_size);

    /* Get indirect table */
    if (indir)
        for (i = 0; i < ae_dev->dev_specs.rss_ind_tbl_size; i++)
            indir[i] = vport->rss_indirection_tbl[i];

    return 0;
}

static int hclge_plf_parse_rss_hfunc(struct hclge_plf_vport *vport, const u8 hfunc, u8 *hash_algo)
{
    switch (hfunc) {
        case ETH_RSS_HASH_TOP:
            *hash_algo = HCLGE_PLF_RSS_HASH_ALGO_TOEPLITZ;
            return 0;
        case ETH_RSS_HASH_XOR:
            *hash_algo = HCLGE_PLF_RSS_HASH_ALGO_SIMPLE;
            return 0;
        case ETH_RSS_HASH_NO_CHANGE:
            *hash_algo = vport->rss_algo;
            return 0;
        default:
            return -EINVAL;
    }
}

int hclge_plf_set_rss(struct hnae3_handle *handle, const u32 *indir, const u8 *key, const u8 hfunc)
{
    struct hnae3_ae_dev *ae_dev = platform_get_drvdata(handle->plfdev);
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    u8 hash_algo;
    int ret, i;

    if (!ae_dev) {
        dev_err(&hdev->pdev->dev, "ae_dev is NULL.\n");
        return -EINVAL;
    }

    ret = hclge_plf_parse_rss_hfunc(vport, hfunc, &hash_algo);
    if (ret) {
        dev_err(&hdev->pdev->dev, "invalid hfunc type(%d).\n", hfunc);
        return ret;
    }

    /* Set the RSS Hash Key if specified by the user */
    if (key) {
        ret = hclge_plf_set_rss_algo_key_tuple(hdev, hash_algo, key);
        if (ret)
            return ret;

        /* Update the shadow RSS key with user specified qids */
        (void)memcpy_s(vport->rss_hash_key, sizeof(vport->rss_hash_key), key, sizeof(vport->rss_hash_key));
    } else {
        ret = hclge_plf_set_rss_algo_key_tuple(hdev, hash_algo, vport->rss_hash_key);
        if (ret)
            return ret;
    }
    vport->rss_algo = hash_algo;

    /* Update the shadow RSS table with user specified qids */
    for (i = 0; i < ae_dev->dev_specs.rss_ind_tbl_size; i++)
        vport->rss_indirection_tbl[i] = (u16)indir[i];

    /* Update the hardware */
    return hclge_plf_set_rss_indir_table(hdev, vport->rss_indirection_tbl);
}

static u8 hclge_plf_get_rss_hash_bits(struct ethtool_rxnfc *nfc)
{
    u8 hash_sets = nfc->data & RXH_L4_B_0_1 ? HCLGE_PLF_S_PORT_BIT : 0;

    if (nfc->data & RXH_L4_B_2_3) {
        hash_sets |= HCLGE_PLF_D_PORT_BIT;
    } else {
        hash_sets &= ~HCLGE_PLF_D_PORT_BIT;
    }

    if (nfc->data & RXH_IP_SRC) {
        hash_sets |= HCLGE_PLF_S_IP_BIT;
    } else {
        hash_sets &= ~HCLGE_PLF_S_IP_BIT;
    }

    if (nfc->data & RXH_IP_DST) {
        hash_sets |= HCLGE_PLF_D_IP_BIT;
    } else {
        hash_sets &= ~HCLGE_PLF_D_IP_BIT;
    }

    if (nfc->flow_type == SCTP_V4_FLOW || nfc->flow_type == SCTP_V6_FLOW) {
        hash_sets |= HCLGE_PLF_V_TAG_BIT;
    }

    return hash_sets;
}

static int hclge_plf_set_rss_tuple_data(struct hclge_plf_vport *vport, struct ethtool_rxnfc *nfc)
{
    u8 tuple_sets;

    tuple_sets = hclge_plf_get_rss_hash_bits(nfc);
    switch (nfc->flow_type) {
        case TCP_V4_FLOW:
            vport->rss_tuple_sets.ipv4_tcp_en = tuple_sets;
            break;
        case TCP_V6_FLOW:
            vport->rss_tuple_sets.ipv6_tcp_en = tuple_sets;
            break;
        case UDP_V4_FLOW:
            vport->rss_tuple_sets.ipv4_udp_en = tuple_sets;
            break;
        case UDP_V6_FLOW:
            vport->rss_tuple_sets.ipv6_udp_en = tuple_sets;
            break;
        case SCTP_V4_FLOW:
            vport->rss_tuple_sets.ipv4_sctp_en = tuple_sets;
            break;
        case SCTP_V6_FLOW:
            vport->rss_tuple_sets.ipv6_sctp_en = tuple_sets;
            break;
        case IPV4_FLOW:
            vport->rss_tuple_sets.ipv4_fragment_en = HCLGE_PLF_RSS_INPUT_TUPLE_OTHER;
            break;
        case IPV6_FLOW:
            vport->rss_tuple_sets.ipv6_fragment_en = HCLGE_PLF_RSS_INPUT_TUPLE_OTHER;
            break;
        default:
            return -EINVAL;
    }

    return 0;
}

int hclge_plf_set_rss_tuple(struct hnae3_handle *handle, struct ethtool_rxnfc *nfc)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    int ret;

    if (nfc->data & ~(RXH_IP_SRC | RXH_IP_DST | RXH_L4_B_0_1 | RXH_L4_B_2_3))
        return -EINVAL;

    ret = hclge_plf_set_rss_tuple_data(vport, nfc);
    if (ret) {
        dev_err(&hdev->pdev->dev, "failed to init rss tuple data, ret = %d\n", ret);
        return ret;
    }

    ret = hclge_plf_set_rss_algo_key_tuple(hdev, 0, vport->rss_hash_key);
    if (ret) {
        dev_err(&hdev->pdev->dev, "Set rss tuple fail, status = %d\n", ret);
        return ret;
    }

    hclge_plf_get_rss_type(vport);
    return 0;
}

static int hclge_plf_get_vport_rss_tuple(struct hclge_plf_vport *vport, int flow_type, u8 *tuple_sets)
{
    switch (flow_type) {
        case TCP_V4_FLOW:
            *tuple_sets = vport->rss_tuple_sets.ipv4_tcp_en;
            break;
        case UDP_V4_FLOW:
            *tuple_sets = vport->rss_tuple_sets.ipv4_udp_en;
            break;
        case TCP_V6_FLOW:
            *tuple_sets = vport->rss_tuple_sets.ipv6_tcp_en;
            break;
        case UDP_V6_FLOW:
            *tuple_sets = vport->rss_tuple_sets.ipv6_udp_en;
            break;
        case SCTP_V4_FLOW:
            *tuple_sets = vport->rss_tuple_sets.ipv4_sctp_en;
            break;
        case SCTP_V6_FLOW:
            *tuple_sets = vport->rss_tuple_sets.ipv6_sctp_en;
            break;
        case IPV4_FLOW:
        case IPV6_FLOW:
            *tuple_sets = HCLGE_PLF_S_IP_BIT | HCLGE_PLF_D_IP_BIT;
            break;
        default:
            return -EINVAL;
    }

    return 0;
}

static u64 hclge_plf_convert_rss_tuple(u8 tuple_sets)
{
    u64 tuple_data = 0;

    if (tuple_sets & HCLGE_PLF_D_PORT_BIT)
        tuple_data |= RXH_L4_B_2_3;
    if (tuple_sets & HCLGE_PLF_S_PORT_BIT)
        tuple_data |= RXH_L4_B_0_1;
    if (tuple_sets & HCLGE_PLF_D_IP_BIT)
        tuple_data |= RXH_IP_DST;
    if (tuple_sets & HCLGE_PLF_S_IP_BIT)
        tuple_data |= RXH_IP_SRC;

    return tuple_data;
}

int hclge_plf_get_rss_tuple(struct hnae3_handle *handle, struct ethtool_rxnfc *nfc)
{
    struct hclge_plf_vport *vport = hclge_get_vport(handle);
    u8 tuple_sets;
    int ret;

    nfc->data = 0;

    ret = hclge_plf_get_vport_rss_tuple(vport, nfc->flow_type, &tuple_sets);
    if (ret || !tuple_sets)
        return ret;

    nfc->data = hclge_plf_convert_rss_tuple(tuple_sets);

    return 0;
}

static int hclge_plf_init_rss_tc_mode(struct hclge_plf_dev *hdev)
{
    struct hnae3_ae_dev *ae_dev = hdev->ae_dev;
    struct hclge_plf_vport *vport = hdev->vport;
    struct hnae3_knic_private_info *kinfo = &vport->nic.kinfo;
    u32 tc_offset[HCLGE_MAX_TC_NUM] = {0};
    u32 tc_size[HCLGE_MAX_TC_NUM] = {0};
    u32 tc_valid[HCLGE_MAX_TC_NUM];
    u32 roundup_size;
    u16 rss_size;
    int i;

    for (i = 0; i < HCLGE_MAX_TC_NUM; i++) {
        rss_size = kinfo->tc_info.tqp_count[i];
        tc_valid[i] = 0;

        if (!(hdev->hw_tc_map & BIT(i)))
            continue;

        /* Each TC have the same queue size, and tc_size set to
         * hardware is the log2 of roundup power of two of rss_size,
         * the acutal queue size is limited by indirection table.
         */
        if (rss_size > ae_dev->dev_specs.rss_ind_tbl_size || rss_size == 0) {
            dev_err(&hdev->pdev->dev, "Configure rss tc size failed, invalid TC_SIZE = %hu\n", rss_size);
            return -EINVAL;
        }

        roundup_size = (u32)roundup_pow_of_two(rss_size);
        if (ilog2(roundup_size) < 0) {
            dev_err(&hdev->pdev->dev, " rss roundup_size invalid, roundup_size = %u\n", roundup_size);
            return -EINVAL;
        }
        roundup_size = (u32)ilog2(roundup_size);

        tc_valid[i] = 1;
        tc_size[i] = roundup_size;
        tc_offset[i] = kinfo->tc_info.tqp_offset[i] + hdev->tqp_offset;
    }

    return hclge_plf_set_rss_tc_mode(hdev, tc_valid, tc_size, tc_offset);
}

int hclge_plf_rss_init_hw(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_vport *vport = hdev->vport;
    u16 *rss_indir = vport->rss_indirection_tbl;
    u8 *key = vport->rss_hash_key;
    u8 hfunc = vport->rss_algo;
    int ret;

    ret = hclge_plf_set_rss_indir_table(hdev, rss_indir);
    if (ret)
        return ret;

    ret = hclge_plf_set_rss_algo_key_tuple(hdev, hfunc, key);
    if (ret)
        return ret;

    return hclge_plf_init_rss_tc_mode(hdev);
}

void hclge_plf_rss_indir_init_cfg(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_vport *vport = hdev->vport;
    u16 i;

    for (i = 0; i < hdev->ae_dev->dev_specs.rss_ind_tbl_size; i++)
        vport->rss_indirection_tbl[i] = i % vport->alloc_rss_size;
}

void hclge_plf_rss_init_cfg(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_vport *vport = hdev->vport;

    vport->rss_tuple_sets.ipv4_tcp_en = HCLGE_PLF_RSS_INPUT_TUPLE_OTHER;
    vport->rss_tuple_sets.ipv4_udp_en = HCLGE_PLF_RSS_INPUT_TUPLE_OTHER;
    vport->rss_tuple_sets.ipv4_sctp_en = HCLGE_PLF_RSS_INPUT_TUPLE_SCTP;
    vport->rss_tuple_sets.ipv4_fragment_en = HCLGE_PLF_RSS_INPUT_TUPLE_OTHER;
    vport->rss_tuple_sets.ipv6_tcp_en = HCLGE_PLF_RSS_INPUT_TUPLE_OTHER;
    vport->rss_tuple_sets.ipv6_udp_en = HCLGE_PLF_RSS_INPUT_TUPLE_OTHER;
    vport->rss_tuple_sets.ipv6_sctp_en = HCLGE_PLF_RSS_INPUT_TUPLE_SCTP;
    vport->rss_tuple_sets.ipv6_fragment_en = HCLGE_PLF_RSS_INPUT_TUPLE_OTHER;

    vport->rss_algo = HCLGE_PLF_RSS_HASH_ALGO_TOEPLITZ;
    (void)memcpy_s(vport->rss_hash_key, sizeof(vport->rss_hash_key), hclge_plf_hash_key, sizeof(vport->rss_hash_key));

    hclge_plf_rss_indir_init_cfg(hdev);
}

void hclge_plf_rss_cap_init(struct hclge_plf_dev *hdev)
{
    u_ppp_rss_idt_alloc_data alloc_data;
    u_ppp_pkt_tc_ovrd_cfg ovrd_cfg;
    u32 idt_alloc_data_addr;
    u32 tc_ovrd_cfg_reg_addr;

    hdev->rss_indir_tbl_offset = hdev->id * hdev->ae_dev->dev_specs.rss_ind_tbl_size;

    idt_alloc_data_addr = PPP_REG_BASE + PPP_RSS_IDT_ALLOC_DATA;
    tc_ovrd_cfg_reg_addr = PPP_REG_BASE + PPP_PKT_TC_OVRD_CFG;

    alloc_data.value = 0;
    /* each block has 4 entrys, [8:3]:block offset, [2:0]:blocks num */
    alloc_data.bits.cfg_idt_block_offset_data = ((hdev->rss_indir_tbl_offset / 4) << 3) |
        ilog2(hdev->ae_dev->dev_specs.rss_ind_tbl_size / 4); /* each block has 4 entrys */
    hclge_rss_write_reg(hdev->hw.io_base, idt_alloc_data_addr, alloc_data.value);

    ovrd_cfg.value = 0;
    ovrd_cfg.bits.cfg_pkt_tc_ovrd_cmd = 0; /* 0:write 1:read */
    ovrd_cfg.bits.cfg_pkt_tc_ovrd_cmd_en = 1;
    ovrd_cfg.bits.cfg_pkt_tc_ovrd_fun_id = hdev->id;
    hclge_rss_write_reg(hdev->hw.io_base, tc_ovrd_cfg_reg_addr, ovrd_cfg.value);
    /* status bit3 -> 8, wait 100us */
    if (wait_hardware_done(hdev->hw.io_base + tc_ovrd_cfg_reg_addr, 0x8, 0x0, 1, 100)) {
        dev_err(&hdev->pdev->dev, "failed to set tc_ovrd(0x%x)!\n", tc_ovrd_cfg_reg_addr);
    }
    /* After completing configuration, set en to 0 */
    hclge_rss_write_reg(hdev->hw.io_base, tc_ovrd_cfg_reg_addr, 0);
}
