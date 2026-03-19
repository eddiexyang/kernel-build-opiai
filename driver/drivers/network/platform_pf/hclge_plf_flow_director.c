/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: platform flow director
 * Author: huawei
 * Create: 2022-08-29
 */

#include <net/ipv6.h>

#include "hnae3.h"
#include "hclge_plf_main.h"
#include "reg_pa.h"
#include "reg_ppp.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

static struct hclge_plf_fd_alloc_info g_port_fd_alloc_info[MAX_PORT_NUMBER];

#define MAX_FD_ENTRY 256
#define MAX_BLK_CNT 16

STATIC u32 g_fd_mode = HCLGE_PLF_FD_MODE_DEPTH_64_WIDTH_400B;
STATIC u8 g_fd_tab_vld[MAX_FD_ENTRY];
STATIC atomic_t g_fd_total_cnt;

static const struct key_info meta_data_key_info[] = {
    { PACKET_TYPE_ID, 6 },
    { IP_FRAGEMENT, 1 },
    { VLAN_NUMBER, 2 },
    { SRC_PORT, 12 },
    { DST_PORT, 12 },
    { MPLS_PACKET, 1 },
    { PACKET_TYPE_L3_ID, 4 },
    { PACKET_TYPE_L4_ID, 4 },
    { PACKET_TYPE_L5_ID, 4 },
    { TEMPLATE_ID, 3 },
};

static const struct key_info g_tuple_key_info[] = {
    { INNER_DST_MAC, 48, KEY_OPT_MAC,
      offsetof(struct hclge_fd_rule, tuples.dst_mac),
      offsetof(struct hclge_fd_rule, tuples_mask.dst_mac) },
    { INNER_SRC_MAC, 48, KEY_OPT_MAC,
      offsetof(struct hclge_fd_rule, tuples.src_mac),
      offsetof(struct hclge_fd_rule, tuples_mask.src_mac) },
    { INNER_VLAN_TAG_OUTER, 16, KEY_OPT_LE16,
      offsetof(struct hclge_fd_rule, tuples.vlan_tag1),
      offsetof(struct hclge_fd_rule, tuples_mask.vlan_tag1) },
    { INNER_VLAN_TAG_INNER, 16, KEY_OPT_LE16, -1, -1 },
    { INNER_ETH_TYPE, 16, KEY_OPT_LE16,
      offsetof(struct hclge_fd_rule, tuples.ether_proto),
      offsetof(struct hclge_fd_rule, tuples_mask.ether_proto) },
    { INNER_L2_UDF, 16, KEY_OPT_LE16,
      offsetof(struct hclge_fd_rule, tuples.l2_user_def),
      offsetof(struct hclge_fd_rule, tuples_mask.l2_user_def) },
    { INNER_IP_TOS, 8, KEY_OPT_U8,
      offsetof(struct hclge_fd_rule, tuples.ip_tos),
      offsetof(struct hclge_fd_rule, tuples_mask.ip_tos) },
    { INNER_IP_PROTO, 8, KEY_OPT_U8,
      offsetof(struct hclge_fd_rule, tuples.ip_proto),
      offsetof(struct hclge_fd_rule, tuples_mask.ip_proto) },
    { INNER_SRC_IP, 32, KEY_OPT_IP,
      offsetof(struct hclge_fd_rule, tuples.src_ip),
      offsetof(struct hclge_fd_rule, tuples_mask.src_ip) },
    { INNER_DST_IP, 32, KEY_OPT_IP,
      offsetof(struct hclge_fd_rule, tuples.dst_ip),
      offsetof(struct hclge_fd_rule, tuples_mask.dst_ip) },
    { INNER_L3_UDF, 16, KEY_OPT_LE16,
      offsetof(struct hclge_fd_rule, tuples.l3_user_def),
      offsetof(struct hclge_fd_rule, tuples_mask.l3_user_def) },
    { INNER_SRC_PORT, 16, KEY_OPT_LE16,
      offsetof(struct hclge_fd_rule, tuples.src_port),
      offsetof(struct hclge_fd_rule, tuples_mask.src_port) },
    { INNER_DST_PORT, 16, KEY_OPT_LE16,
      offsetof(struct hclge_fd_rule, tuples.dst_port),
      offsetof(struct hclge_fd_rule, tuples_mask.dst_port) },
    { INNER_L4_UDF, 32, KEY_OPT_LE32,
      offsetof(struct hclge_fd_rule, tuples.l4_user_def),
      offsetof(struct hclge_fd_rule, tuples_mask.l4_user_def) },
};

static void hclge_plf_set_generic_fd_mode(struct hclge_plf_dev *hdev)
{
    hclge_write_dev(&hdev->hw, PPP_PPP_TCAM_MODE_REG, g_fd_mode);
}

STATIC void hclge_plf_get_fd_mode(struct hclge_plf_dev *hdev, u8 *fd_mode)
{
    *fd_mode = (u8)hclge_read_dev(&hdev->hw, PPP_PPP_TCAM_MODE_REG);
}

static int hclge_plf_fd_key_sel_cfg_set(struct hclge_plf_dev *hdev, u8 template_id)
{
    u_ppp_fd_key_sel_cfg fd_key_sel_cfg;
    int ret;

    fd_key_sel_cfg.value = 0;
    fd_key_sel_cfg.bits.cfg_fd_key_sel_cmd = 0; /* 0 : write */
    fd_key_sel_cfg.bits.cfg_fd_key_sel_addr = template_id;
    fd_key_sel_cfg.bits.cfg_fd_key_sel_cmd_en = 1;
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_SEL_CFG_REG, fd_key_sel_cfg.value);

    /* wait 10 us */
    ret = wait_hardware_done(hdev->hw.io_base + PPP_PPP_FD_KEY_SEL_CFG_REG, FD_KEY_CFG_STATUS_MSK, 0x0, 1, 10);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "Failed to cfg fd key template(timeout)\n");
    }

    fd_key_sel_cfg.bits.cfg_fd_key_sel_cmd_en = 0;
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_SEL_CFG_REG, fd_key_sel_cfg.value);

    return ret;
}

static void hclge_fd_key_sel_tdata_sigle_cfg(struct hclge_fd_key_cfg *key_cfg,
                                             struct hclge_plf_tuple_cfg *tuple_cfg, int tuple_index)
{
#define FD_KEY_SEL_TDATA_REG_BITS 32
    const struct key_info *tuple_single_key_info;
    u32 tuple_offest;
    u8 tuple_size;
    u8 i;

    tuple_size = tuple_cfg->tuple_size;
    tuple_single_key_info = tuple_cfg->tuple_single_key_info;

    if ((key_cfg->tuple_active & BIT(tuple_index)) == 0) {
        tuple_cfg->tuple_offest_total += tuple_size;
        return;
    } else {
        for (i = 0; i < tuple_size; i++) {
            /* when support ipv4, Only the lower 4 bits need to be configured. */
            if ((tuple_single_key_info->key_opt == KEY_OPT_IP) && (i >= sizeof(u32))) {
                tuple_cfg->tuple_offest_total++;
                continue;
            }
            if (tuple_cfg->tuple_offest_total < FD_KEY_SEL_TDATA_REG_BITS) {
                /* config PPP_FD_KEY_SEL_TDATA_0 */
                tuple_offest = tuple_cfg->tuple_offest_total;
                tuple_cfg->fd_key_sel_tdata_0 |= BIT(tuple_offest);
            } else {
                /* config PPP_FD_KEY_SEL_TDATA_1 */
                tuple_offest = tuple_cfg->tuple_offest_total - FD_KEY_SEL_TDATA_REG_BITS;
                tuple_cfg->fd_key_sel_tdata_1 |= BIT(tuple_offest);
            }
            tuple_cfg->tuple_offest_total++;
        }
    }
}

static void hclge_fd_key_sel_tdata_cfg(struct hclge_plf_dev *hdev, const struct key_info *tuple_key_info,
                                       struct hclge_fd_key_cfg *key_cfg)
{
    const struct key_info *tuple_single_key_info;
    struct hclge_plf_tuple_cfg tuple_cfg;
    u32 ip_offset;
    u8 tuple_size;
    u8 i;

    tuple_cfg.fd_key_sel_tdata_0 = 0;
    tuple_cfg.fd_key_sel_tdata_1 = 0;
    tuple_cfg.tuple_offest_total = 0;

    for (i = 0; i < MAX_TUPLE; i++) {
        tuple_single_key_info = &tuple_key_info[i];
        tuple_size = tuple_single_key_info->key_length / BITS_OF_ONE_BYTE;

        if (tuple_single_key_info->key_opt == KEY_OPT_IP) {
            ip_offset = IPV4_INDEX * sizeof(u32);
            tuple_size += ip_offset;
        }
        tuple_cfg.tuple_size = tuple_size;
        tuple_cfg.tuple_single_key_info = tuple_single_key_info;
        hclge_fd_key_sel_tdata_sigle_cfg(key_cfg, &tuple_cfg, i);
    }

    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_SEL_TDATA_0_REG, tuple_cfg.fd_key_sel_tdata_0);
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_SEL_TDATA_1_REG, tuple_cfg.fd_key_sel_tdata_1);
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_SEL_MDATA_REG, key_cfg->meta_data_active);
}

static int hclge_plf_set_fd_key_config(struct hclge_plf_dev *hdev, u8 template_id)
{
    const struct key_info *tuple_key_info_cfg;
    struct hclge_fd_key_cfg *key_cfg;

    key_cfg = &hdev->fd_cfg.key_cfg;
    key_cfg->temp_id = template_id;
    tuple_key_info_cfg = &g_tuple_key_info[0];

    hclge_fd_key_sel_tdata_cfg(hdev, tuple_key_info_cfg, key_cfg);

    return hclge_plf_fd_key_sel_cfg_set(hdev, template_id);
}

static int hclge_plf_fd_key_templs_match_table_cfg(struct hclge_plf_dev *hdev, u8 template_id)
{
    u_ppp_fd_key_templ_cfg templ_cfg;
    u8 port_id = hdev->id;
    int ret;

    /* PPP write: write data_reg, then write cfg_reg */
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_TEMPL_DATA_0_REG, BIT(port_id));
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_TEMPL_DATA_2_REG, 0xf0); /* L3_id mask: invalid status */
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_TEMPL_DATA_3_REG, template_id);

    templ_cfg.value = 0;
    templ_cfg.bits.cfg_fd_key_templ_cmd = 0; /* 0 : write */
    templ_cfg.bits.cfg_fd_key_templ_addr = template_id;
    templ_cfg.bits.cfg_fd_key_templ_cmd_en = 1;
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_TEMPL_CFG_REG, templ_cfg.value);

    /* wait 10 us */
    ret = wait_hardware_done(hdev->hw.io_base + PPP_PPP_FD_KEY_TEMPL_CFG_REG, FD_KEY_CFG_STATUS_MSK, 0x0, 1, 10);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "failed to cfg fd key template(timeout), ret = %d.\n", ret);
    }

    templ_cfg.bits.cfg_fd_key_templ_cmd_en = 0;
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_TEMPL_CFG_REG, templ_cfg.value);

    return ret;
}

static int cfg_tcam_block_data(struct hclge_plf_dev *hdev, fd_key_cfg *cfg, u32 *tcam_data,
                               enum fd_tcam_data_type data_type, enum plf_rw_op op)
{
    u32 tcam_data_num;
    int ret;
    u32 i;

    tcam_data_num = (data_type == FD_TCAM_KEY_DATA) ? FD_KEY_DATA_REG_CNT : FD_AD_DATA_REG_CNT;

    if (op == PLF_WRITE) {
        for (i = 0; i < tcam_data_num; i++) {
            hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_DATA_ADDR(i), tcam_data[i]);
        }
    }

    cfg->bits.cfg_fd_key_cmd_en = 1;
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_CFG_REG, cfg->val);
    /* wait hardware done need 1us*10=10us */
    ret = wait_hardware_done(hdev->hw.io_base + PPP_PPP_FD_KEY_CFG_REG, FD_KEY_SATAUS_BIT, 0x0, 1, 10);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "Failed to cfg tcam, timeout ret = %d\n", ret);
    }

    cfg->bits.cfg_fd_key_cmd_en = 0;
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_CFG_REG, cfg->val);

    if ((op == PLF_READ) && (ret == 0)) {
        for (i = 0; i < tcam_data_num; i++) {
            tcam_data[i] = hclge_read_dev(&hdev->hw, PPP_PPP_FD_KEY_DATA_ADDR(i));
        }
    }

    return ret;
}

STATIC u32 get_global_entry_index(u32 port_id, u32 location)
{
    u32 absolute_index_global;

    absolute_index_global = g_port_fd_alloc_info[port_id].stage_start_entry_index + location;

    return absolute_index_global;
}

STATIC int cfg_tcam_x_data_invalid(struct hclge_plf_dev *hdev, u32 key_addr)
{
    fd_key_cfg key_cfg;
    int ret;

    /* set invalid flag */
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_DATA_ADDR(FD_KEY_DATA_REG_CNT -1), 0);

    key_cfg.val = 0;
    key_cfg.bits.cfg_fd_key_addr = key_addr;
    key_cfg.bits.cfg_fd_key_x = CFG_DF_KEY_X;
    /* set cmd enable */
    key_cfg.bits.cfg_fd_key_cmd_en = 1;
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_CFG_REG, key_cfg.val);
    /* wait hardware done need 1us*10=10us */
    ret = wait_hardware_done(hdev->hw.io_base + PPP_PPP_FD_KEY_CFG_REG, FD_KEY_SATAUS_BIT, 0x0, 1, 10);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "cfg tcam valid cmd timeout, ret = %d.\n", ret);
    }

    key_cfg.bits.cfg_fd_key_cmd_en = 0;
    hclge_write_dev(&hdev->hw, PPP_PPP_FD_KEY_CFG_REG, key_cfg.val);

    return ret;
}

static void flow_director_enable(struct hclge_plf_dev *hdev, bool en)
{
#define FD_EN_VALUE 0x1
    u8 port_id = hdev->id;
    u32 reg_value;

    reg_value = hclge_read_dev(&hdev->hw, PPP_PPP_TNL_MISC_CFG_REG(port_id));
    if (en == true) {
        reg_value |= (FD_EN_VALUE << 4U);
    } else {
        reg_value &= ~(FD_EN_VALUE << 4U);
    }
    hclge_write_dev(&hdev->hw, PPP_PPP_TNL_MISC_CFG_REG(port_id), reg_value);
}

static void hclge_plf_flow_director_update_entry(struct hclge_plf_dev *hdev,
    fd_key_cfg *cfg, bool entry_vld)
{
    u32 tcam_entry_index;

    /* x data */
    if (cfg->bits.cfg_fd_key_x != 0x1) {
        return;
    }

    tcam_entry_index = cfg->bits.cfg_fd_key_addr;
    if (entry_vld == true) {
        if (g_fd_tab_vld[tcam_entry_index] == 0) {
            /* add a new tcam entry */
            g_fd_tab_vld[tcam_entry_index] = 1;
            if (atomic_read(&g_fd_total_cnt) == TCAM_ENRTY_NULL) {
                flow_director_enable(hdev, true);
            }
            atomic_inc(&g_fd_total_cnt);
        }
    } else {
        if (g_fd_tab_vld[tcam_entry_index] == 1) {
            /* delete a tcam entry */
            g_fd_tab_vld[tcam_entry_index] = 0;
            atomic_dec(&g_fd_total_cnt);
            if (atomic_read(&g_fd_total_cnt) == TCAM_ENRTY_NULL) {
                pr_info("FD disabled when no entry exists.\n");
                flow_director_enable(hdev, false);
            }
        }
    }
}

static int flow_director_tcam_write(struct hclge_plf_dev *hdev, bool sel_x, int loction, u8 *key, bool is_add)
{
#define BYTES_NUM_OF_ONE_CFG 4
    u32 fd_key_data[FD_KEY_DATA_REG_CNT] = {0};
    fd_key_cfg key_cfg = {0};
    u32 fd_meta_data_last;
    u8 port_id = hdev->id;
    u32 key_index_cfg;
    u32 fd_key_index;
    bool entry_vld;
    u32 i, j;
    int ret;

    fd_key_index = get_global_entry_index(port_id, loction);
    key_cfg.bits.cfg_fd_key_x = sel_x;
    key_cfg.bits.cfg_fd_key_addr = fd_key_index;
    key_cfg.bits.cfg_fd_key_ad_sel = TCAM_DATA_KEY;
    key_cfg.bits.cfg_fd_key_cmd = FD_KEY_WRITE;
    /* set x data invalid, when set y data. avoid conflict of ecc scan by hardware */
    if (key_cfg.bits.cfg_fd_key_x == 0) {
        ret = cfg_tcam_x_data_invalid(hdev, fd_key_index);
        if (ret) {
            dev_err(&hdev->pdev->dev, "Failed to cfg fd key_x invalid, ret = %d\n", ret);
            return ret;
        }
    }

    if (key != NULL) {
        for (i = 0; i < FD_KEY_DATA_REG_CNT; i++) {
            for (j = 0; j < BYTES_NUM_OF_ONE_CFG; j++) {
                key_index_cfg = i * BYTES_NUM_OF_ONE_CFG + j;
                fd_key_data[i] |= (key[key_index_cfg] << (j * BITS_OF_ONE_BYTE));
            }
        }
    }

    fd_key_data[FD_KEY_DATA_REG_CNT - 1] &= 0xFFFF; /* only the lower 16 bits valid */

    /* The configuration is required only when x is configured and
     * entries are modified, it indicates entries are valid.
     */
    entry_vld = sel_x && is_add;
    if (entry_vld == true) {
        fd_key_data[FD_KEY_DATA_REG_CNT - 1] |= KEY_VLD_BIT;
    }

    if (g_fd_mode == HCLGE_PLF_FD_MODE_DEPTH_64_WIDTH_400B) {
        fd_meta_data_last = (fd_key_data[12U] << 16U) | (fd_key_data[11U] >> 16U);
        fd_key_data[11U] = (fd_key_data[11U] & 0xFFFF) | (fd_meta_data_last << 16U);
        fd_key_data[12U] = (fd_key_data[12U] & (~0xFFFF)) | (fd_meta_data_last >> 16U);
    } else {
        dev_err(&hdev->pdev->dev, "FD current not support other mode\n");
        return -EOPNOTSUPP;
    }

    ret = cfg_tcam_block_data(hdev, &key_cfg, fd_key_data, FD_TCAM_KEY_DATA, PLF_WRITE);
    if (ret) {
        dev_err(&hdev->pdev->dev, "Failed to cfg tcam, ret = %d\n", ret);
        return ret;
    }

    hclge_plf_flow_director_update_entry(hdev, &key_cfg, entry_vld);

    return 0;
}

static bool is_valid_entry_index(u8 port_id, u32 index)
{
    if (port_id >= MAX_PORT_NUMBER) {
        return false;
    }

    if ((g_port_fd_alloc_info[port_id].stage_alloc_entry_num == 0) ||
        (index >= g_port_fd_alloc_info[port_id].stage_alloc_entry_num)) {
        return false;
    }

    return true;
}

static void init_fd_alloc_entry(u32 fd_mode)
{
#define ENTRY_NUM_OF_BLK 16 /* 16 * 16 = 256 */
    u32 one_entry_blk_num[MAX_FD_MODE] = { 1, 2, 4 }; /* 100:200:400 */
    u32 used_blk_num = 0;
    u32 entry_index = 0;
    u32 stage_blk_num;
    u32 rem_blk_num;
    u32 per_port_blk;
    u32 port_num;
    u32 port_id;

    port_num = MAX_PLATFORM_QSET_NUM;
    stage_blk_num = MAX_BLK_CNT / one_entry_blk_num[fd_mode];
    per_port_blk = stage_blk_num / port_num;
    rem_blk_num = stage_blk_num % port_num;
    for (port_id = 0; port_id < port_num; port_id++) {
        if ((used_blk_num * one_entry_blk_num[fd_mode]) >= MAX_BLK_CNT) {
            break;
        }
        if (rem_blk_num != 0) {
            g_port_fd_alloc_info[port_id].stage_alloc_entry_num = (per_port_blk + rem_blk_num) * ENTRY_NUM_OF_BLK;
            g_port_fd_alloc_info[port_id].stage_start_entry_index = entry_index;
            used_blk_num += per_port_blk + rem_blk_num;
            rem_blk_num = 0;
        } else {
            g_port_fd_alloc_info[port_id].stage_alloc_entry_num = per_port_blk * ENTRY_NUM_OF_BLK;
            g_port_fd_alloc_info[port_id].stage_start_entry_index = entry_index;
            used_blk_num += per_port_blk;
        }
        entry_index += g_port_fd_alloc_info[port_id].stage_alloc_entry_num;
    }
}

STATIC void init_fd_entry_num(void)
{
    atomic_set(&g_fd_total_cnt, TCAM_ENRTY_NULL);
    (void)memset_s(g_fd_tab_vld, sizeof(g_fd_tab_vld), 0, sizeof(g_fd_tab_vld));
    init_fd_alloc_entry(g_fd_mode);
}

STATIC int hclge_plf_fd_cap_config(struct hclge_plf_dev *hdev)
{
#define MAX_KEY_LENGTH
    struct hclge_fd_key_cfg *key_cfg;

    /* fd mode: 64*400bit, 128*200bit, 256*100bit */
    hclge_plf_get_fd_mode(hdev, &hdev->fd_cfg.fd_mode);

    switch (hdev->fd_cfg.fd_mode) {
        case HCLGE_PLF_FD_MODE_DEPTH_64_WIDTH_400B:
            hdev->fd_cfg.max_key_length = MAX_KEY_LENGTH_MODE2;
            break;
        case HCLGE_PLF_FD_MODE_DEPTH_128_WIDTH_200B:
            hdev->fd_cfg.max_key_length = MAX_KEY_LENGTH_MODE1;
            break;
        case HCLGE_PLF_FD_MODE_DEPTH_256_WIDTH_100B:
            hdev->fd_cfg.max_key_length = MAX_KEY_LENGTH_MODE0;
            break;
        default:
            dev_err(&hdev->pdev->dev,
                "unsupported flow director mode %d\n",
                hdev->fd_cfg.fd_mode);
            return -EOPNOTSUPP;
    }

    key_cfg = &hdev->fd_cfg.key_cfg;
    key_cfg->key_sel = HCLGE_PLF_FD_KEY_BASE_ON_TUPLE;

    /* select tuples to generate key */
    key_cfg->tuple_active = BIT(INNER_VLAN_TAG_OUTER) | BIT(INNER_ETH_TYPE) |
                BIT(INNER_IP_TOS) | BIT(INNER_IP_PROTO) | BIT(INNER_SRC_IP) |
                BIT(INNER_DST_IP) | BIT(INNER_SRC_PORT) | BIT(INNER_DST_PORT);

    /* If use max 400bit key, we can support tuples for ether type */
    if (hdev->fd_cfg.fd_mode == HCLGE_PLF_FD_MODE_DEPTH_64_WIDTH_400B) {
        key_cfg->tuple_active |=
                BIT(INNER_DST_MAC) | BIT(INNER_SRC_MAC) | HCLGE_PLF_FD_TUPLE_USER_DEF_TUPLES;
    }

    /* temple_id is used to specify the rule based on port numbers */
    key_cfg->meta_data_active = BIT(TEMPLATE_ID);

    hdev->fd_cfg.rule_num = g_port_fd_alloc_info[hdev->id].stage_alloc_entry_num;

    return 0;
}

static int hclge_plf_fd_templs_table_cfg(struct hclge_plf_dev *hdev, u8 template_id)
{
    int ret;

    ret = hclge_plf_fd_key_templs_match_table_cfg(hdev, template_id);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "failed to set fd key template, ret = %d\n", ret);
        return ret;
    }

    ret = hclge_plf_fd_cap_config(hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "Failed to config fd cap, ret = %d\n", ret);
        return ret;
    }

    return hclge_plf_set_fd_key_config(hdev, template_id);
}

int hclge_plf_init_fd_config(struct hclge_plf_dev *hdev)
{
#define FD_FW_LKUP_EN_S 4
#define KEY_SEL_S 5
    u32 cport_misc_cfg = 0;
    u8 port_id = hdev->id;
    u8 template_id = 0;
    int ret;

    if ((port_id == hclge_plf_get_first_probe_id()) && !test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state)) {
        init_fd_entry_num();
    }

    if (hnae3_ae_dev_fd_supported(hdev->ae_dev)) {
        template_id = port_id;
        hclge_plf_set_generic_fd_mode(hdev);
        ret = hclge_plf_fd_templs_table_cfg(hdev, template_id);
        if (ret != 0) {
            dev_err(&hdev->pdev->dev, "Failed to config fd key templs match table, ret = %d\n", ret);
            return ret;
        }

        /* Rx fd lookup enable, tuple mode */
        cport_misc_cfg |= BIT(FD_FW_LKUP_EN_S) | BIT(KEY_SEL_S);
        hclge_write_dev(&hdev->hw, PPP_PPP_TNL_MISC_CFG_REG(port_id), cport_misc_cfg);
    }

    return 0;
}

static int hclge_plf_fd_tcam_config(struct hclge_plf_dev *hdev, bool sel_x,
                                    int loc, u8 *key, bool is_add)
{
    u8 port_id = hdev->id;
    int ret;

    ret = is_valid_entry_index(port_id, loc);
    if (ret != true) {
        dev_err(&hdev->pdev->dev, "parameter is invalid! port_id = %d, index = %d\n",
                port_id, loc);
        return -EINVAL;
    }

    return flow_director_tcam_write(hdev, sel_x, loc, key, is_add);
}

static int hclge_fd_check_tcpip4_tuple(struct ethtool_tcpip4_spec *spec,
                                       u32 *unused_tuple)
{
    if (!spec || !unused_tuple)
        return -EINVAL;

    *unused_tuple |= BIT(INNER_SRC_MAC) | BIT(INNER_DST_MAC);

    if (!spec->ip4src)
        *unused_tuple |= BIT(INNER_SRC_IP);

    if (!spec->ip4dst)
        *unused_tuple |= BIT(INNER_DST_IP);

    if (!spec->psrc)
        *unused_tuple |= BIT(INNER_SRC_PORT);

    if (!spec->pdst)
        *unused_tuple |= BIT(INNER_DST_PORT);

    if (!spec->tos)
        *unused_tuple |= BIT(INNER_IP_TOS);

    return 0;
}

static int hclge_fd_check_ip4_tuple(struct ethtool_usrip4_spec *spec,
                                    u32 *unused_tuple)
{
    if (!spec || !unused_tuple)
        return -EINVAL;

    *unused_tuple |= BIT(INNER_SRC_MAC) | BIT(INNER_DST_MAC) |
        BIT(INNER_SRC_PORT) | BIT(INNER_DST_PORT);

    if (!spec->ip4src)
        *unused_tuple |= BIT(INNER_SRC_IP);

    if (!spec->ip4dst)
        *unused_tuple |= BIT(INNER_DST_IP);

    if (!spec->tos)
        *unused_tuple |= BIT(INNER_IP_TOS);

    if (!spec->proto)
        *unused_tuple |= BIT(INNER_IP_PROTO);

    if (spec->l4_4_bytes)
        return -EOPNOTSUPP;

    if (spec->ip_ver != ETH_RX_NFC_IP4)
        return -EOPNOTSUPP;

    return 0;
}

static int hclge_fd_check_tcpip6_tuple(struct ethtool_tcpip6_spec *spec,
                                       u32 *unused_tuple)
{
    if (!spec || !unused_tuple)
        return -EINVAL;

    *unused_tuple |= BIT(INNER_SRC_MAC) | BIT(INNER_DST_MAC);

    /* check whether src/dst ip address used */
    if (ipv6_addr_any((struct in6_addr *)spec->ip6src))
        *unused_tuple |= BIT(INNER_SRC_IP);

    if (ipv6_addr_any((struct in6_addr *)spec->ip6dst))
        *unused_tuple |= BIT(INNER_DST_IP);

    if (!spec->psrc)
        *unused_tuple |= BIT(INNER_SRC_PORT);

    if (!spec->pdst)
        *unused_tuple |= BIT(INNER_DST_PORT);

    if (!spec->tclass)
        *unused_tuple |= BIT(INNER_IP_TOS);

    return 0;
}

static int hclge_fd_check_ip6_tuple(struct ethtool_usrip6_spec *spec,
                                    u32 *unused_tuple)
{
    if (!spec || !unused_tuple)
        return -EINVAL;

    *unused_tuple |= BIT(INNER_SRC_MAC) | BIT(INNER_DST_MAC) |
            BIT(INNER_SRC_PORT) | BIT(INNER_DST_PORT);

    /* check whether src/dst ip address used */
    if (ipv6_addr_any((struct in6_addr *)spec->ip6src))
        *unused_tuple |= BIT(INNER_SRC_IP);

    if (ipv6_addr_any((struct in6_addr *)spec->ip6dst))
        *unused_tuple |= BIT(INNER_DST_IP);

    if (!spec->l4_proto)
        *unused_tuple |= BIT(INNER_IP_PROTO);

    if (!spec->tclass)
        *unused_tuple |= BIT(INNER_IP_TOS);

    if (spec->l4_4_bytes)
        return -EOPNOTSUPP;

    return 0;
}

static int hclge_fd_check_ether_tuple(struct ethhdr *spec, u32 *unused_tuple)
{
    if (!spec || !unused_tuple)
        return -EINVAL;

    *unused_tuple |= BIT(INNER_SRC_IP) | BIT(INNER_DST_IP) |
        BIT(INNER_SRC_PORT) | BIT(INNER_DST_PORT) |
        BIT(INNER_IP_TOS) | BIT(INNER_IP_PROTO);

    if (is_zero_ether_addr(spec->h_source))
        *unused_tuple |= BIT(INNER_SRC_MAC);

    if (is_zero_ether_addr(spec->h_dest))
        *unused_tuple |= BIT(INNER_DST_MAC);

    if (!spec->h_proto)
        *unused_tuple |= BIT(INNER_ETH_TYPE);

    return 0;
}

static int hclge_fd_check_ext_tuple(struct hclge_plf_dev *hdev,
                                    struct ethtool_rx_flow_spec *fs,
                                    u32 *unused_tuple)
{
    if (fs->flow_type & FLOW_EXT) {
        if (fs->h_ext.vlan_etype) {
            dev_err(&hdev->pdev->dev, "Vlan-etype is not supported!\n");
            return -EOPNOTSUPP;
        }

        /* This modification is to ensure that packets of other VLANs can be dropped,
         * but the packets which VLAN ID is not configured can be received(vlan ID 0).
         */
        if (!fs->h_ext.vlan_tci && !fs->m_ext.vlan_tci) {
            *unused_tuple |= BIT(INNER_VLAN_TAG_OUTER);
        }

        if (fs->m_ext.vlan_tci &&
            be16_to_cpu(fs->h_ext.vlan_tci) >= VLAN_N_VID) {
            dev_err(&hdev->pdev->dev,
                "failed to config vlan_tci, invalid vlan_tci: %d, max is %d.\n",
                ntohs(fs->h_ext.vlan_tci), VLAN_N_VID - 1);
            return -EINVAL;
        }
    } else {
        *unused_tuple |= BIT(INNER_VLAN_TAG_OUTER);
    }

    if (fs->flow_type & FLOW_MAC_EXT) {
        if (hdev->fd_cfg.fd_mode !=
            HCLGE_PLF_FD_MODE_DEPTH_64_WIDTH_400B) {
            dev_err(&hdev->pdev->dev,
                "FLOW_MAC_EXT is not supported in current fd mode!\n");
            return -EOPNOTSUPP;
        }

        if (is_zero_ether_addr(fs->h_ext.h_dest))
            *unused_tuple |= BIT(INNER_DST_MAC);
        else
            *unused_tuple &= ~BIT(INNER_DST_MAC);
    }

    return 0;
}

static int hclge_fd_get_user_def_layer(u32 flow_type, u32 *unused_tuple,
                                       struct hclge_fd_user_def_info *info)
{
    switch (flow_type) {
        case ETHER_FLOW:
            info->layer = HCLGE_FD_USER_DEF_L2;
            *unused_tuple &= ~BIT(INNER_L2_UDF);
            break;
        case IP_USER_FLOW:
        case IPV6_USER_FLOW:
            info->layer = HCLGE_FD_USER_DEF_L3;
            *unused_tuple &= ~BIT(INNER_L3_UDF);
            break;
        case TCP_V4_FLOW:
        case UDP_V4_FLOW:
        case TCP_V6_FLOW:
        case UDP_V6_FLOW:
            info->layer = HCLGE_FD_USER_DEF_L4;
            *unused_tuple &= ~BIT(INNER_L4_UDF);
            break;
        default:
            return -EOPNOTSUPP;
    }

    return 0;
}

static bool hclge_fd_is_user_def_all_masked(struct ethtool_rx_flow_spec *fs)
{
    return be32_to_cpu(fs->m_ext.data[1] | fs->m_ext.data[0]) == 0;
}

static bool hclge_fd_is_support_udf(struct hclge_plf_dev *hdev)
{
    u32 tuple_active;

    tuple_active = hdev->fd_cfg.key_cfg.tuple_active;
    if (!(tuple_active & HCLGE_PLF_FD_TUPLE_USER_DEF_TUPLES)) {
        dev_err(&hdev->pdev->dev, "User-def bytes are not supported\n");
        return false;
    }

    return true;
}

static int hclge_fd_parse_user_def_field(struct hclge_plf_dev *hdev,
                                         struct ethtool_rx_flow_spec *fs,
                                         u32 *unused_tuple,
                                         struct hclge_fd_user_def_info *info)
{
    u32 flow_type = fs->flow_type & ~(FLOW_EXT | FLOW_MAC_EXT);
    u16 data, offset, data_mask, offset_mask;
    int ret;

    info->layer = HCLGE_FD_USER_DEF_NONE;
    *unused_tuple |= HCLGE_PLF_FD_TUPLE_USER_DEF_TUPLES;

    if (!(fs->flow_type & FLOW_EXT) || hclge_fd_is_user_def_all_masked(fs))
        return 0;

    /* user-def data from ethtool is 64 bit value, the bit0~15 is used
     * for data, and bit32~47 is used for offset.
     */
    data = (u16)(be32_to_cpu(fs->h_ext.data[1]) & HCLGE_FD_USER_DEF_DATA);
    data_mask = (u16)(be32_to_cpu(fs->m_ext.data[1]) & HCLGE_FD_USER_DEF_DATA);
    offset = (u16)(be32_to_cpu(fs->h_ext.data[0]) & HCLGE_FD_USER_DEF_OFFSET);
    offset_mask = (u16)(be32_to_cpu(fs->m_ext.data[0]) & HCLGE_FD_USER_DEF_OFFSET);

    if (hclge_fd_is_support_udf(hdev) != true) {
        dev_err(&hdev->pdev->dev, "User-def bytes are not supported\n");
        return -EOPNOTSUPP;
    }

    if (offset > HCLGE_FD_MAX_USER_DEF_OFFSET) {
        dev_err(&hdev->pdev->dev,
            "user-def offset[%d] should be no more than %d\n",
            offset, HCLGE_FD_MAX_USER_DEF_OFFSET);
        return -EINVAL;
    }

    if (offset_mask != HCLGE_FD_USER_DEF_OFFSET_UNMASK) {
        dev_err(&hdev->pdev->dev, "User-def offset can't be masked\n");
        return -EINVAL;
    }

    ret = hclge_fd_get_user_def_layer(flow_type, unused_tuple, info);
    if (ret) {
        dev_err(&hdev->pdev->dev,
            "Unsupported flow type for user-def bytes, ret = %d\n",
            ret);
        return ret;
    }

    info->data = data;
    info->data_mask = data_mask;
    info->offset = offset;

    return 0;
}

static int hclge_fd_check_spec(struct hclge_plf_dev *hdev,
                               struct ethtool_rx_flow_spec *fs,
                               u32 *unused_tuple,
                               struct hclge_fd_user_def_info *info)
{
    u32 flow_type;
    int ret;

    if (fs->location >= hdev->fd_cfg.rule_num) {
        dev_err(&hdev->pdev->dev,
            "Failed to config fd rules, invalid rule location: %u, max is %u\n",
            fs->location,
            hdev->fd_cfg.rule_num - 1);
        return -EINVAL;
    }

    ret = hclge_fd_parse_user_def_field(hdev, fs, unused_tuple, info);
    if (ret)
        return ret;

    flow_type = fs->flow_type & ~(FLOW_EXT | FLOW_MAC_EXT);
    switch (flow_type) {
        case SCTP_V4_FLOW:
        case TCP_V4_FLOW:
        case UDP_V4_FLOW:
            ret = hclge_fd_check_tcpip4_tuple(&fs->h_u.tcp_ip4_spec, unused_tuple);
            break;
        case IP_USER_FLOW:
            ret = hclge_fd_check_ip4_tuple(&fs->h_u.usr_ip4_spec, unused_tuple);
            break;
        case SCTP_V6_FLOW:
        case TCP_V6_FLOW:
        case UDP_V6_FLOW:
            ret = hclge_fd_check_tcpip6_tuple(&fs->h_u.tcp_ip6_spec, unused_tuple);
            break;
        case IPV6_USER_FLOW:
            ret = hclge_fd_check_ip6_tuple(&fs->h_u.usr_ip6_spec, unused_tuple);
            break;
        case ETHER_FLOW:
            if (hdev->fd_cfg.fd_mode !=
                HCLGE_PLF_FD_MODE_DEPTH_64_WIDTH_400B) {
                dev_err(&hdev->pdev->dev,
                    "ETHER_FLOW is not supported in current fd mode!\n");
                return -EOPNOTSUPP;
            }

            ret = hclge_fd_check_ether_tuple(&fs->h_u.ether_spec, unused_tuple);
            break;
        default:
            dev_err(&hdev->pdev->dev,
                "Unsupported protocol type, protocol type = %#x\n",
                flow_type);
            return -EOPNOTSUPP;
    }

    if (ret) {
        dev_err(&hdev->pdev->dev,
            "Failed to check flow union tuple, ret = %d\n",
            ret);
        return ret;
    }

    return hclge_fd_check_ext_tuple(hdev, fs, unused_tuple);
}

static int hclge_fd_parse_ring_cookie(struct hclge_plf_dev *hdev, u64 ring_cookie,
                                      u8 *action, u16 *queue_id)
{
    if (ring_cookie == RX_CLS_FLOW_DISC) {
        *action = HCLGE_FD_ACTION_DROP_PACKET;
    } else {
        u32 ring = ethtool_get_flow_spec_ring(ring_cookie);

        /* when the MAC address is updated, the MAC and VLAN entries
        in the flow table need to be updated accordingly. */
        if (test_and_clear_bit(HCLGE_STATE_FD_USE_RSS_UPDATING, &hdev->state)) {
            *action = HCLGE_FD_ACTION_ACCEPT_USE_RSS_GEN_QUEID;
        } else {
            u16 tqps = hdev->vport->nic.kinfo.num_tqps;

            if (ring >= (u32)tqps) {
                dev_err(&hdev->pdev->dev,
                    "Error: queue id (%u) > max tqp num (%d)\n",
                    ring, tqps - 1);
                return -EINVAL;
            }

            *action = HCLGE_FD_ACTION_ACCEPT_DIRECT_QUEID;
            *queue_id = ring;
        }
    }

    return 0;
}

static void hclge_fd_get_tcpip4_tuple(struct hclge_plf_dev *hdev,
                                      struct ethtool_rx_flow_spec *fs,
                                      struct hclge_fd_rule *rule, u8 ip_proto)
{
    rule->tuples.src_ip[IPV4_INDEX] =
            be32_to_cpu(fs->h_u.tcp_ip4_spec.ip4src);
    rule->tuples_mask.src_ip[IPV4_INDEX] =
            be32_to_cpu(fs->m_u.tcp_ip4_spec.ip4src);

    rule->tuples.dst_ip[IPV4_INDEX] =
            be32_to_cpu(fs->h_u.tcp_ip4_spec.ip4dst);
    rule->tuples_mask.dst_ip[IPV4_INDEX] =
            be32_to_cpu(fs->m_u.tcp_ip4_spec.ip4dst);

    rule->tuples.src_port = be16_to_cpu(fs->h_u.tcp_ip4_spec.psrc);
    rule->tuples_mask.src_port = be16_to_cpu(fs->m_u.tcp_ip4_spec.psrc);

    rule->tuples.dst_port = be16_to_cpu(fs->h_u.tcp_ip4_spec.pdst);
    rule->tuples_mask.dst_port = be16_to_cpu(fs->m_u.tcp_ip4_spec.pdst);

    rule->tuples.ip_tos = fs->h_u.tcp_ip4_spec.tos;
    rule->tuples_mask.ip_tos = fs->m_u.tcp_ip4_spec.tos;

    rule->tuples.ether_proto = ETH_P_IP;
    rule->tuples_mask.ether_proto = 0xFFFF;

    rule->tuples.ip_proto = ip_proto;
    rule->tuples_mask.ip_proto = 0xFF;
}

static void hclge_fd_get_ip4_tuple(struct hclge_plf_dev *hdev,
                                   struct ethtool_rx_flow_spec *fs,
                                   struct hclge_fd_rule *rule)
{
    rule->tuples.src_ip[IPV4_INDEX] =
            be32_to_cpu(fs->h_u.usr_ip4_spec.ip4src);
    rule->tuples_mask.src_ip[IPV4_INDEX] =
            be32_to_cpu(fs->m_u.usr_ip4_spec.ip4src);

    rule->tuples.dst_ip[IPV4_INDEX] =
            be32_to_cpu(fs->h_u.usr_ip4_spec.ip4dst);
    rule->tuples_mask.dst_ip[IPV4_INDEX] =
            be32_to_cpu(fs->m_u.usr_ip4_spec.ip4dst);

    rule->tuples.ip_tos = fs->h_u.usr_ip4_spec.tos;
    rule->tuples_mask.ip_tos = fs->m_u.usr_ip4_spec.tos;

    rule->tuples.ip_proto = fs->h_u.usr_ip4_spec.proto;
    rule->tuples_mask.ip_proto = fs->m_u.usr_ip4_spec.proto;

    rule->tuples.ether_proto = ETH_P_IP;
    rule->tuples_mask.ether_proto = 0xFFFF;
}

static void hclge_fd_get_tcpip6_tuple(struct hclge_plf_dev *hdev,
                                      struct ethtool_rx_flow_spec *fs,
                                      struct hclge_fd_rule *rule, u8 ip_proto)
{
    be32_to_cpu_array(rule->tuples.src_ip, fs->h_u.tcp_ip6_spec.ip6src,
                      IPV6_SIZE);
    be32_to_cpu_array(rule->tuples_mask.src_ip, fs->m_u.tcp_ip6_spec.ip6src,
                      IPV6_SIZE);

    be32_to_cpu_array(rule->tuples.dst_ip, fs->h_u.tcp_ip6_spec.ip6dst,
                      IPV6_SIZE);
    be32_to_cpu_array(rule->tuples_mask.dst_ip, fs->m_u.tcp_ip6_spec.ip6dst,
                      IPV6_SIZE);

    rule->tuples.src_port = be16_to_cpu(fs->h_u.tcp_ip6_spec.psrc);
    rule->tuples_mask.src_port = be16_to_cpu(fs->m_u.tcp_ip6_spec.psrc);

    rule->tuples.dst_port = be16_to_cpu(fs->h_u.tcp_ip6_spec.pdst);
    rule->tuples_mask.dst_port = be16_to_cpu(fs->m_u.tcp_ip6_spec.pdst);

    rule->tuples.ether_proto = ETH_P_IPV6;
    rule->tuples_mask.ether_proto = 0xFFFF;

    rule->tuples.ip_tos = fs->h_u.tcp_ip6_spec.tclass;
    rule->tuples_mask.ip_tos = fs->m_u.tcp_ip6_spec.tclass;

    rule->tuples.ip_proto = ip_proto;
    rule->tuples_mask.ip_proto = 0xFF;
}

static void hclge_fd_get_ip6_tuple(struct hclge_plf_dev *hdev,
                                   struct ethtool_rx_flow_spec *fs,
                                   struct hclge_fd_rule *rule)
{
    be32_to_cpu_array(rule->tuples.src_ip, fs->h_u.usr_ip6_spec.ip6src,
                      IPV6_SIZE);
    be32_to_cpu_array(rule->tuples_mask.src_ip, fs->m_u.usr_ip6_spec.ip6src,
                      IPV6_SIZE);

    be32_to_cpu_array(rule->tuples.dst_ip, fs->h_u.usr_ip6_spec.ip6dst,
                      IPV6_SIZE);
    be32_to_cpu_array(rule->tuples_mask.dst_ip, fs->m_u.usr_ip6_spec.ip6dst,
                      IPV6_SIZE);

    rule->tuples.ip_proto = fs->h_u.usr_ip6_spec.l4_proto;
    rule->tuples_mask.ip_proto = fs->m_u.usr_ip6_spec.l4_proto;

    rule->tuples.ip_tos = fs->h_u.tcp_ip6_spec.tclass;
    rule->tuples_mask.ip_tos = fs->m_u.tcp_ip6_spec.tclass;

    rule->tuples.ether_proto = ETH_P_IPV6;
    rule->tuples_mask.ether_proto = 0xFFFF;
}

static void hclge_fd_get_ether_tuple(struct hclge_plf_dev *hdev,
                                     struct ethtool_rx_flow_spec *fs,
                                     struct hclge_fd_rule *rule)
{
    ether_addr_copy(rule->tuples.src_mac, fs->h_u.ether_spec.h_source);
    ether_addr_copy(rule->tuples_mask.src_mac, fs->m_u.ether_spec.h_source);

    ether_addr_copy(rule->tuples.dst_mac, fs->h_u.ether_spec.h_dest);
    ether_addr_copy(rule->tuples_mask.dst_mac, fs->m_u.ether_spec.h_dest);

    rule->tuples.ether_proto = be16_to_cpu(fs->h_u.ether_spec.h_proto);
    rule->tuples_mask.ether_proto = be16_to_cpu(fs->m_u.ether_spec.h_proto);
}

static void hclge_fd_get_user_def_tuple(struct hclge_fd_user_def_info *info,
                                        struct hclge_fd_rule *rule)
{
#define L4_UDF_OFFEST 16
    switch (info->layer) {
        case HCLGE_FD_USER_DEF_L2:
            rule->tuples.l2_user_def = info->data;
            rule->tuples_mask.l2_user_def = info->data_mask;
            break;
        case HCLGE_FD_USER_DEF_L3:
            rule->tuples.l3_user_def = info->data;
            rule->tuples_mask.l3_user_def = info->data_mask;
            break;
        case HCLGE_FD_USER_DEF_L4:
            rule->tuples.l4_user_def = (u32)info->data << L4_UDF_OFFEST;
            rule->tuples_mask.l4_user_def = (u32)info->data_mask << L4_UDF_OFFEST;
            break;
        default:
            break;
    }

    rule->user_def = *info;
}

static int hclge_fd_get_tuple(struct hclge_plf_dev *hdev,
                              struct ethtool_rx_flow_spec *fs,
                              struct hclge_fd_rule *rule,
                              struct hclge_fd_user_def_info *info)
{
    u32 flow_type = fs->flow_type & ~(FLOW_EXT | FLOW_MAC_EXT);

    switch (flow_type) {
        case SCTP_V4_FLOW:
            hclge_fd_get_tcpip4_tuple(hdev, fs, rule, IPPROTO_SCTP);
            break;
        case TCP_V4_FLOW:
            hclge_fd_get_tcpip4_tuple(hdev, fs, rule, IPPROTO_TCP);
            break;
        case UDP_V4_FLOW:
            hclge_fd_get_tcpip4_tuple(hdev, fs, rule, IPPROTO_UDP);
            break;
        case IP_USER_FLOW:
            hclge_fd_get_ip4_tuple(hdev, fs, rule);
            break;
        case SCTP_V6_FLOW:
            hclge_fd_get_tcpip6_tuple(hdev, fs, rule, IPPROTO_SCTP);
            break;
        case TCP_V6_FLOW:
            hclge_fd_get_tcpip6_tuple(hdev, fs, rule, IPPROTO_TCP);
            break;
        case UDP_V6_FLOW:
            hclge_fd_get_tcpip6_tuple(hdev, fs, rule, IPPROTO_UDP);
            break;
        case IPV6_USER_FLOW:
            hclge_fd_get_ip6_tuple(hdev, fs, rule);
            break;
        case ETHER_FLOW:
            hclge_fd_get_ether_tuple(hdev, fs, rule);
            break;
        default:
            return -EOPNOTSUPP;
    }

    if (fs->flow_type & FLOW_EXT) {
        rule->tuples.vlan_tag1 = be16_to_cpu(fs->h_ext.vlan_tci);
        rule->tuples_mask.vlan_tag1 = be16_to_cpu(fs->m_ext.vlan_tci);
        hclge_fd_get_user_def_tuple(info, rule);
    }

    if (fs->flow_type & FLOW_MAC_EXT) {
        ether_addr_copy(rule->tuples.dst_mac, fs->h_ext.h_dest);
        ether_addr_copy(rule->tuples_mask.dst_mac, fs->m_ext.h_dest);
    }

    return 0;
}

static struct hclge_fd_rule *hclge_find_fd_rule(struct hlist_head *hlist,
                                                u16 location,
                                                struct hclge_fd_rule **parent)
{
    struct hclge_fd_rule *rule;
    struct hlist_node *node;

    hlist_for_each_entry_safe(rule, node, hlist, rule_node) {
        if (rule->location == location) {
            return rule;
        } else if (rule->location > location) {
            return NULL;
        } else {
            /* nothing */
        }
        /* record the parent node, use to keep the nodes in fd_rule_list
         * in ascend order.
         */
        *parent = rule;
    }

    return NULL;
}

static int hclge_fd_check_user_def_refcnt(struct hclge_plf_dev *hdev,
                                          struct hclge_fd_rule *rule)
{
    struct hlist_head *hlist = &hdev->fd_rule_list;
    struct hclge_fd_user_def_info *info, *old_info;
    struct hclge_fd_rule *parent = NULL;
    struct hclge_fd_user_def_cfg *cfg;
    struct hclge_fd_rule *fd_rule;

    if (!rule || rule->rule_type != HCLGE_FD_EP_ACTIVE ||
        rule->user_def.layer == HCLGE_FD_USER_DEF_NONE)
        return 0;

    /* for valid layer is start from 1, so need minus 1 to get the cfg */
    cfg = &hdev->fd_cfg.user_def_cfg[rule->user_def.layer - 1];
    info = &rule->user_def;

    if (!cfg->ref_cnt || cfg->offset == info->offset) {
        return 0;
    }

    if (cfg->ref_cnt > 1) {
        goto error;
    }

    fd_rule = hclge_find_fd_rule(hlist, rule->location, &parent);
    if (fd_rule) {
        old_info = &fd_rule->user_def;
        if (info->layer == old_info->layer)
            return 0;
    }

error:
    dev_err(&hdev->pdev->dev,
        "No available offset for layer%d fd rule, each layer only support one user def offset.\n",
        info->layer + 1);
    return -ENOSPC;
}

static void hclge_fd_dec_rule_cnt(struct hclge_plf_dev *hdev, u16 location)
{
    if (test_bit(location, hdev->fd_bmap)) {
        clear_bit(location, hdev->fd_bmap);
        hdev->hclge_fd_rule_num--;
        if (location < USER_BC_MC_ENTRY_NUM - 1) {
            hdev->hclge_fd_user_rule_num--;
        }
    }
}

static void hclge_sync_fd_state(struct hclge_plf_dev *hdev)
{
    if (hlist_empty(&hdev->fd_rule_list)) {
        hdev->fd_active_type = HCLGE_FD_RULE_NONE;
    }
}

static int hclge_plf_fd_ad_config(struct hclge_plf_dev *hdev, int loc,
                                  struct hclge_plf_fd_ad_data *action)
{
    u32 fd_ad_data[FD_AD_DATA_REG_CNT] = {0};
    plf_fd_ad_reg ad_reg_data;
    fd_key_cfg key_cfg;
    u32 fd_ad_index;
    u8 port_id;
    int ret;
    int i;

    port_id = hdev->id;
    ret = is_valid_entry_index(port_id, loc);
    if (ret != true) {
        dev_err(&hdev->pdev->dev, "Invalid fd entry index %d, ret = %d\n", loc, ret);
        return ret;
    }

    ad_reg_data.val = 0;
    key_cfg.val = 0;
    fd_ad_index = get_global_entry_index(port_id, loc);
    key_cfg.bits.cfg_fd_key_addr = fd_ad_index;
    key_cfg.bits.cfg_fd_key_ad_sel = TCAM_DATA_AD;

    key_cfg.bits.cfg_fd_key_cmd = FD_KEY_WRITE;
    ad_reg_data.bits.statistic_cnt_vld = action->statistic_cnt_vld;
    ad_reg_data.bits.rule_id = action->rule_id;
    ad_reg_data.bits.fd_tc_size = action->fd_tc_size;
    ad_reg_data.bits.queue_id_tc_offset = action->queue_id_tc_offset;
    ad_reg_data.bits.rss_tc_override_en = action->rss_tc_override_en;
    ad_reg_data.bits.queue_id_mode = action->queue_id_mode;
    ad_reg_data.bits.next_step = action->next_step;

    for (i = 0; i < FD_AD_DATA_REG_CNT; i++) {
        fd_ad_data[i] = ad_reg_data.val;
    }
    ret = cfg_tcam_block_data(hdev, &key_cfg, fd_ad_data, FD_TCAM_AD_DATA, PLF_WRITE);
    if (ret) {
        dev_err(&hdev->pdev->dev, "When write action, config tcam block data failed %d(%d)\n", loc, ret);
        return ret;
    }

    return 0;
}

static bool hclge_plf_fd_convert_tuple(u32 tuple_bit, u8 *key_x, u8 *key_y,
                                       struct hclge_fd_rule *rule)
{
    const struct key_info *tuple_key_info_cfg;
    enum hclge_plf_fd_key_opt key_opt;
    int offset, moffset, ip_offset;
    u16 tmp_x_s, tmp_y_s;
    u32 tmp_x_l, tmp_y_l;
    u8 *p = (u8 *)rule;
    int i;

    if (rule->unused_tuple & BIT(tuple_bit))
        return true;

    tuple_key_info_cfg = &g_tuple_key_info[0];
    key_opt = tuple_key_info_cfg[tuple_bit].key_opt;
    offset = tuple_key_info_cfg[tuple_bit].offset;
    moffset = tuple_key_info_cfg[tuple_bit].moffset;

    switch (key_opt) {
        case KEY_OPT_U8:
            calc_x(*key_x, p[offset], p[moffset]);
            calc_y(*key_y, p[offset], p[moffset]);
            return true;
        case KEY_OPT_LE16:
            calc_x(tmp_x_s, *(u16 *)(&p[offset]), *(u16 *)(&p[moffset]));
            calc_y(tmp_y_s, *(u16 *)(&p[offset]), *(u16 *)(&p[moffset]));
            *(__le16 *)key_x = cpu_to_le16(tmp_x_s);
            *(__le16 *)key_y = cpu_to_le16(tmp_y_s);
            return true;
        case KEY_OPT_LE32:
            calc_x(tmp_x_l, *(u32 *)(&p[offset]), *(u32 *)(&p[moffset]));
            calc_y(tmp_y_l, *(u32 *)(&p[offset]), *(u32 *)(&p[moffset]));
            *(__le32 *)key_x = cpu_to_le32(tmp_x_l);
            *(__le32 *)key_y = cpu_to_le32(tmp_y_l);
            return true;
        case KEY_OPT_MAC:
            for (i = 0; i < ETH_ALEN; i++) {
                calc_x(key_x[ETH_ALEN - 1 - i], p[offset + i], p[moffset + i]);
                calc_y(key_y[ETH_ALEN - 1 - i], p[offset + i], p[moffset + i]);
            }
            return true;
        case KEY_OPT_IP:
            ip_offset = IPV4_INDEX * sizeof(u32); /* ipv4 and ipv6 only check 32 bits */
            calc_x(tmp_x_l, *(u32 *)(&p[offset + ip_offset]),
                   *(u32 *)(&p[moffset + ip_offset]));
            calc_y(tmp_y_l, *(u32 *)(&p[offset + ip_offset]),
                   *(u32 *)(&p[moffset + ip_offset]));
            *(__le32 *)key_x = cpu_to_le32(tmp_x_l);
            *(__le32 *)key_y = cpu_to_le32(tmp_y_l);
            return true;
        default:
            return false;
    }
}

static void hclge_plf_fd_convert_meta_data(u8 port_id, struct hclge_fd_key_cfg *key_cfg,
                                           u32 *key_x, u32 *key_y,
                                           struct hclge_fd_rule *rule)
{
    u32 tuple_bit, tmp_x, tmp_y;
    u8 tuple_size, shift_bits;
    u32 meta_data = 0;
    u8 cur_pos = 0;
    u32 i;

    for (i = 0; i < MAX_META_DATA; i++) {
        tuple_size = meta_data_key_info[i].key_length;
        tuple_bit = key_cfg->meta_data_active & BIT(i);
        if (tuple_bit == BIT(TEMPLATE_ID)) {
            hnae3_set_field(meta_data, GENMASK(cur_pos + tuple_size, cur_pos),
                            cur_pos, key_cfg->temp_id);
            cur_pos += tuple_size;
        }
    }

    calc_x(tmp_x, meta_data, 0xFFFFFFFF);
    calc_y(tmp_y, meta_data, 0xFFFFFFFF);
    shift_bits = sizeof(meta_data) * 8 - cur_pos; /* 8 bits of one byte  */

    *key_x = cpu_to_le32(tmp_x << shift_bits);
    *key_y = cpu_to_le32(tmp_y << shift_bits);
}

/* A complete key is combined with meta data key and tuple key.
 * Meta data key is stored at the MSB region, and tuple key is stored at
 * the LSB region, unused bits will be filled 0.
 */
static int hclge_plf_config_key(struct hclge_plf_dev *hdev, struct hclge_fd_rule *rule)
{
    u8 key_x[MAX_KEY_BYTES], key_y[MAX_KEY_BYTES];
    const struct key_info *tuple_key_info_cfg;
    struct hclge_fd_key_cfg *key_cfg;
    u8 *cur_key_x, *cur_key_y;
    u8 meta_data_region;
    u8 tuple_size;
    u8 port_id;
    int ret;
    u32 i;

    port_id = hdev->id;
    (void)memset_s(key_x, sizeof(key_x), 0, sizeof(key_x));
    (void)memset_s(key_y, sizeof(key_y), 0, sizeof(key_y));
    cur_key_x = key_x;
    cur_key_y = key_y;

    tuple_key_info_cfg = &g_tuple_key_info[0];
    key_cfg = &hdev->fd_cfg.key_cfg;

    for (i = 0; i < MAX_TUPLE; i++) {
        bool tuple_valid;

        tuple_size = tuple_key_info_cfg[i].key_length / BITS_OF_ONE_BYTE;
        if (!(key_cfg->tuple_active & BIT(i)))
            continue;

        tuple_valid = hclge_plf_fd_convert_tuple(i, cur_key_x,
                             cur_key_y, rule);
        if (tuple_valid) {
            cur_key_x += tuple_size;
            cur_key_y += tuple_size;
        }
    }

    meta_data_region = (hdev->fd_cfg.max_key_length / BITS_OF_ONE_BYTE) -(MAX_META_DATA_LENGTH / BITS_OF_ONE_BYTE) +
                       (hdev->fd_cfg.max_key_length % BITS_OF_ONE_BYTE);

    hclge_plf_fd_convert_meta_data(port_id, key_cfg,
                                   (u32 *)(key_x + meta_data_region),
                                   (u32 *)(key_y + meta_data_region),
                                   rule);

    ret = hclge_plf_fd_tcam_config(hdev, false, rule->location, key_y, true);
    if (ret) {
        dev_err(&hdev->pdev->dev,
                "FD key_y config fail, loc = %d, ret = %d\n", rule->location, ret);
        return ret;
    }

    ret = hclge_plf_fd_tcam_config(hdev, true, rule->location, key_x, true);
    if (ret) {
        dev_err(&hdev->pdev->dev,
                "FD key_x config fail, loc = %d, ret = %d\n", rule->location, ret);
    }

    return ret;
}

static int hclge_plf_config_action(struct hclge_plf_dev *hdev, struct hclge_fd_rule *rule)
{
    struct hclge_plf_fd_ad_data ad_data = {0};

    ad_data.ad_id = rule->location;

    switch (rule->action) {
        case HCLGE_FD_ACTION_DROP_PACKET:
            ad_data.next_step = true;
            break;
        case HCLGE_FD_ACTION_ACCEPT_DIRECT_QUEID:
            ad_data.next_step = false;
            ad_data.queue_id_mode = true;
            ad_data.queue_id_tc_offset = rule->queue_id;
            break;
        case HCLGE_FD_ACTION_ACCEPT_USE_RSS_GEN_QUEID:
            ad_data.next_step = false;
            ad_data.queue_id_mode = false;
            ad_data.rss_tc_override_en = false;
            break;
        default:
            ad_data.next_step = false;
            break;
    }
    ad_data.rule_id = rule->location;
    ad_data.statistic_cnt_vld = false;

    return hclge_plf_fd_ad_config(hdev, ad_data.ad_id, &ad_data);
}

static int hclge_plf_fd_config_rule(struct hclge_plf_dev *hdev,
                                    struct hclge_fd_rule *rule)
{
    int ret;

    ret = hclge_plf_config_action(hdev, rule);
    if (ret) {
        dev_err(&hdev->pdev->dev, "Failed to config fd action, ret = %d\n", ret);
        return ret;
    }

    ret = hclge_plf_config_key(hdev, rule);
    if (ret) {
        dev_err(&hdev->pdev->dev, "Failed to config fd key, ret = %d\n", ret);
        return ret;
    }

    return 0;
}

static void hclge_fd_dec_user_def_refcnt(struct hclge_plf_dev *hdev,
                                         struct hclge_fd_rule *rule)
{
    struct hclge_fd_user_def_cfg *cfg;

    if (!rule || rule->rule_type != HCLGE_FD_EP_ACTIVE ||
        rule->user_def.layer == HCLGE_FD_USER_DEF_NONE)
        return;

    /* for valid layer is start from 1, so need minus 1 to get the cfg */
    cfg = &hdev->fd_cfg.user_def_cfg[rule->user_def.layer - 1];
    if (!cfg->ref_cnt)
        return;

    cfg->ref_cnt--;
    if (!cfg->ref_cnt) {
        cfg->offset = 0;
        set_bit(HCLGE_STATE_FD_USER_DEF_CHANGED, &hdev->state);
    }
}

static u32 hclge_plf_fd_set_user_def(struct hclge_plf_dev *hdev, struct hclge_fd_user_def_cfg user_def_cfg[], int size)
{
#define PA_UDF_DEF_VALUE 0x0
    u32 mac_id = hdev->hw.mac.mac_id;
    u_pa_l2_flex_offset l2_offset;
    u_pa_l3_flex_offset l3_offset;
    u_pa_l4_flex_offset l4_offset;

    if (size != HCLGE_FD_USER_DEF_LAYER_NUM) {
        dev_err(&hdev->pdev->dev, "Size(%d) mismatch %d\n", size, HCLGE_FD_USER_DEF_LAYER_NUM);
        return 1;
    }

    l2_offset.value = 0;
    l2_offset.bits.cfg_l2_flex_oft_en = (user_def_cfg[0].ref_cnt > 0) ? 1 : 0;
    l2_offset.bits.cfg_l2_flex_oft = user_def_cfg[0].offset;
    hclge_write_dev(&hdev->hw, PA_L2_FLEX_OFFSET_CFG_ADDR(mac_id), l2_offset.value);

    l3_offset.value = 0;
    l3_offset.bits.cfg_l3_flex_oft_en = (user_def_cfg[1].ref_cnt > 0) ? 1 : 0;
    l3_offset.bits.cfg_l3_flex_oft = user_def_cfg[1].offset;
    hclge_write_dev(&hdev->hw, PA_L3_FLEX_OFFSET_CFG_ADDR(mac_id), l3_offset.value);

    l4_offset.value = 0;
    l4_offset.bits.cfg_l4_flex_oft_en = (user_def_cfg[2].ref_cnt > 0) ? 1 : 0; /* L4 : index 2 */
    l4_offset.bits.cfg_l4_flex_oft = user_def_cfg[2].offset; /* L4 : index 2 */
    hclge_write_dev(&hdev->hw, PA_L4_FLEX_OFFSET_CFG_ADDR(mac_id), l4_offset.value);

    return 0;
}

static void hclge_plf_sync_fd_user_def_cfg(struct hclge_plf_dev *hdev, bool locked)
{
    int ret;

    if (!test_and_clear_bit(HCLGE_STATE_FD_USER_DEF_CHANGED, &hdev->state)) {
        return;
    }

    if (!locked)
        spin_lock_bh(&hdev->fd_rule_lock);

    ret = hclge_plf_fd_set_user_def(hdev, hdev->fd_cfg.user_def_cfg, HCLGE_FD_USER_DEF_LAYER_NUM);
    if (ret)
        set_bit(HCLGE_STATE_FD_USER_DEF_CHANGED, &hdev->state);

    if (!locked)
        spin_unlock_bh(&hdev->fd_rule_lock);
}

static void hclge_fd_inc_user_def_refcnt(struct hclge_plf_dev *hdev,
                                         struct hclge_fd_rule *rule)
{
    struct hclge_fd_user_def_cfg *cfg;

    if (!rule || rule->rule_type != HCLGE_FD_EP_ACTIVE ||
        rule->user_def.layer == HCLGE_FD_USER_DEF_NONE)
        return;

    /* for valid layer is start from 1, so need minus 1 to get the cfg */
    cfg = &hdev->fd_cfg.user_def_cfg[rule->user_def.layer - 1];
    if (!cfg->ref_cnt) {
        cfg->offset = rule->user_def.offset;
        set_bit(HCLGE_STATE_FD_USER_DEF_CHANGED, &hdev->state);
    }
    cfg->ref_cnt++;
}

static void hclge_fd_free_node(struct hclge_plf_dev *hdev,
                               struct hclge_fd_rule *rule)
{
    hlist_del(&rule->rule_node);
    kfree(rule);
    hclge_sync_fd_state(hdev);
}

static void hclge_update_fd_rule_node(struct hclge_plf_dev *hdev,
                                      struct hclge_fd_rule *old_rule,
                                      struct hclge_fd_rule *new_rule,
                                      enum hclge_fd_node_state state)
{
    switch (state) {
        case HCLGE_FD_TO_ADD:
        case HCLGE_FD_ACTIVE:
            /* 1) if the new state is TO_ADD, just replace the old rule
            * with the same location, no matter its state, because the
            * new rule will be configured to the hardware.
            * 2) if the new state is ACTIVE, it means the new rule
            * has been configured to the hardware, so just replace
            * the old rule node with the same location.
            * 3) for it doesn't add a new node to the list, so it's
            * unnecessary to update the rule number and fd_bmap.
            */
            new_rule->rule_node.next = old_rule->rule_node.next;
            new_rule->rule_node.pprev = old_rule->rule_node.pprev;
            (void)memcpy_s(old_rule, sizeof(*old_rule), new_rule, sizeof(*old_rule));
            kfree(new_rule);
            break;
        case HCLGE_FD_DELETED:
            hclge_fd_dec_rule_cnt(hdev, old_rule->location);
            hclge_fd_free_node(hdev, old_rule);
            break;
        case HCLGE_FD_TO_DEL:
            /* if new request is TO_DEL, and old rule is existent
            * 1) the state of old rule is TO_DEL, we need do nothing,
            * because we delete rule by location, other rule content
            * is unncessary.
            * 2) the state of old rule is ACTIVE, we need to change its
            * state to TO_DEL, so the rule will be deleted when periodic
            * task being scheduled.
            * 3) the state of old rule is TO_ADD, it means the rule hasn't
            * been added to hardware, so we just delete the rule node from
            * fd_rule_list directly.
            */
            if (old_rule->state == HCLGE_FD_TO_ADD) {
                hclge_fd_dec_rule_cnt(hdev, old_rule->location);
                hclge_fd_free_node(hdev, old_rule);
                return;
            }
            old_rule->state = HCLGE_FD_TO_DEL;
            break;
        default:
            dev_err(&hdev->pdev->dev, "unsupported fd request state.\n");
            break;
    }
}

static void hclge_fd_insert_rule_node(struct hlist_head *hlist,
                                      struct hclge_fd_rule *rule,
                                      struct hclge_fd_rule *parent)
{
    INIT_HLIST_NODE(&rule->rule_node);

    if (parent)
        hlist_add_behind(&rule->rule_node, &parent->rule_node);
    else
        hlist_add_head(&rule->rule_node, hlist);
}

static void hclge_fd_inc_rule_cnt(struct hclge_plf_dev *hdev, u16 location)
{
    if (!test_bit(location, hdev->fd_bmap)) {
        set_bit(location, hdev->fd_bmap);
        hdev->hclge_fd_rule_num++;
        if (location < USER_BC_MC_ENTRY_NUM - 1) {
            hdev->hclge_fd_user_rule_num++;
        }
    }
}

static void hclge_update_fd_list(struct hclge_plf_dev *hdev,
                                 enum hclge_fd_node_state state, u16 location,
                                 struct hclge_fd_rule *new_rule)
{
    struct hlist_head *hlist = &hdev->fd_rule_list;
    struct hclge_fd_rule *parent = NULL;
    struct hclge_fd_rule *fd_rule;

    fd_rule = hclge_find_fd_rule(hlist, location, &parent);
    if (fd_rule) {
        hclge_fd_dec_user_def_refcnt(hdev, fd_rule);
        if (state == HCLGE_FD_ACTIVE)
            hclge_fd_inc_user_def_refcnt(hdev, new_rule);
        hclge_plf_sync_fd_user_def_cfg(hdev, true);

        hclge_update_fd_rule_node(hdev, fd_rule, new_rule, state);
        return;
    }

    /* it's unlikely to fail here, because we have checked the rule
     * exist before.
     */
    if (unlikely(state == HCLGE_FD_TO_DEL || state == HCLGE_FD_DELETED)) {
        dev_warn(&hdev->pdev->dev,
                 "Failed to delete fd rule %d, it's inexistent\n",
                 location);
        return;
    }

    hclge_fd_inc_user_def_refcnt(hdev, new_rule);
    hclge_plf_sync_fd_user_def_cfg(hdev, true);

    hclge_fd_insert_rule_node(hlist, new_rule, parent);
    hclge_fd_inc_rule_cnt(hdev, new_rule->location);

    if (state == HCLGE_FD_TO_ADD) {
        set_bit(HCLGE_STATE_FD_TBL_CHANGED, &hdev->state);
#ifndef DEFINE_HNS_LLT
        hclge_plf_task_schedule(hdev, 0);
#endif
    }
}

static int hclge_add_fd_entry_common(struct hclge_plf_dev *hdev,
                                     struct hclge_fd_rule *rule)
{
    int ret;

    spin_lock_bh(&hdev->fd_rule_lock);

    ret = hclge_fd_check_user_def_refcnt(hdev, rule);
    if (ret) {
        goto out;
    }

    ret = hclge_plf_fd_config_rule(hdev, rule);
    if (ret) {
        goto out;
    }

    rule->state = HCLGE_FD_ACTIVE;
    hdev->fd_active_type = rule->rule_type;
    hclge_update_fd_list(hdev, rule->state, rule->location, rule);

out:
    spin_unlock_bh(&hdev->fd_rule_lock);
    return ret;
}

int hclge_plf_add_fd_entry(struct hnae3_handle *handle,
                           struct ethtool_rxnfc *cmd)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    struct hclge_fd_user_def_info info;
    struct ethtool_rx_flow_spec *fs;
    struct hclge_fd_rule *rule;
    u16 q_index = 0;
    u32 unused = 0;
    u8 action;
    int ret;

    if (!hnae3_ae_dev_fd_supported(hdev->ae_dev)) {
        dev_err(&hdev->pdev->dev, "Flow table director is not supported\n");
        return -EOPNOTSUPP;
    }

    if (!hdev->fd_en) {
        dev_err(&hdev->pdev->dev, "Please enable flow director first\n");
        return -EOPNOTSUPP;
    }

    fs = (struct ethtool_rx_flow_spec *)&cmd->fs;

    ret = hclge_fd_check_spec(hdev, fs, &unused, &info);
    if (ret)
        return ret;

    ret = hclge_fd_parse_ring_cookie(hdev, fs->ring_cookie,
                                     &action, &q_index);
    if (ret)
        return ret;

    rule = kzalloc(sizeof(*rule), GFP_KERNEL);
    if (!rule)
        return -ENOMEM;

    ret = hclge_fd_get_tuple(hdev, fs, rule, &info);
    if (ret) {
        kfree(rule);
        return ret;
    }

    rule->flow_type = fs->flow_type;
    rule->location = fs->location;
    rule->unused_tuple = unused;
    rule->queue_id = q_index + hdev->tqp_offset;
    rule->action = action;
    rule->rule_type = HCLGE_FD_EP_ACTIVE;

    ret = hclge_add_fd_entry_common(hdev, rule);
    if (ret)
        kfree(rule);

    return ret;
}

int hclge_plf_del_fd_entry(struct hnae3_handle *handle,
                           struct ethtool_rxnfc *cmd)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    struct ethtool_rx_flow_spec *fs;
    int ret;

    if (!hnae3_ae_dev_fd_supported(hdev->ae_dev))
        return -EOPNOTSUPP;

    fs = (struct ethtool_rx_flow_spec *)&cmd->fs;
    if (fs->location >= hdev->fd_cfg.rule_num)
        return -EINVAL;

    spin_lock_bh(&hdev->fd_rule_lock);
    if (!test_bit(fs->location, hdev->fd_bmap)) {
        dev_err(&hdev->pdev->dev,
            "Delete fail, rule %u is inexistent\n", fs->location);
        spin_unlock_bh(&hdev->fd_rule_lock);
        return -ENOENT;
    }

    ret = hclge_plf_fd_tcam_config(hdev, true, fs->location, NULL, false);
    if (ret)
        goto out;

    hclge_update_fd_list(hdev, HCLGE_FD_DELETED, fs->location, NULL);

out:
    spin_unlock_bh(&hdev->fd_rule_lock);
    return ret;
}

int hclge_plf_add_fd_user_entry(struct hnae3_handle *handle, struct ethtool_rxnfc *cmd)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct ethtool_rx_flow_spec *fs = &cmd->fs;
    struct hclge_plf_dev *hdev = vport->back;

    if (fs->location >= USER_BC_MC_ENTRY_NUM - 1) {
        dev_err(&hdev->pdev->dev,
            "Failed to add fd rules, invalid rule location: %u, max is %u\n",
            fs->location,
            USER_BC_MC_ENTRY_NUM - 1);
        return -EINVAL;
    }

    return hclge_plf_add_fd_entry(handle, cmd);
}

int hclge_plf_del_fd_user_entry(struct hnae3_handle *handle, struct ethtool_rxnfc *cmd)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct ethtool_rx_flow_spec *fs = &cmd->fs;
    struct hclge_plf_dev *hdev = vport->back;

    if (fs->location >= USER_BC_MC_ENTRY_NUM - 1) {
        dev_err(&hdev->pdev->dev,
            "Failed to delete fd rules, invalid rule location: %u, max is %u\n",
            fs->location,
            USER_BC_MC_ENTRY_NUM - 1);
        return -EINVAL;
    }

    return hclge_plf_del_fd_entry(handle, cmd);
}

int hclge_plf_get_fd_rule_cnt(struct hnae3_handle *handle,
                              struct ethtool_rxnfc *cmd)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    if (!hnae3_ae_dev_fd_supported(hdev->ae_dev))
        return -EOPNOTSUPP;

    cmd->rule_cnt = hdev->hclge_fd_user_rule_num;
    cmd->data = USER_BC_MC_ENTRY_NUM - 1;

    return 0;
}

static void hclge_fd_get_tcpip4_info(struct hclge_fd_rule *rule,
                                     struct ethtool_tcpip4_spec *spec,
                                     struct ethtool_tcpip4_spec *spec_mask)
{
    spec->ip4src = cpu_to_be32(rule->tuples.src_ip[IPV4_INDEX]);
    spec_mask->ip4src = rule->unused_tuple & BIT(INNER_SRC_IP) ?
            0 : cpu_to_be32(rule->tuples_mask.src_ip[IPV4_INDEX]);

    spec->ip4dst = cpu_to_be32(rule->tuples.dst_ip[IPV4_INDEX]);
    spec_mask->ip4dst = rule->unused_tuple & BIT(INNER_DST_IP) ?
            0 : cpu_to_be32(rule->tuples_mask.dst_ip[IPV4_INDEX]);

    spec->psrc = cpu_to_be16(rule->tuples.src_port);
    spec_mask->psrc = rule->unused_tuple & BIT(INNER_SRC_PORT) ?
            0 : cpu_to_be16(rule->tuples_mask.src_port);

    spec->pdst = cpu_to_be16(rule->tuples.dst_port);
    spec_mask->pdst = rule->unused_tuple & BIT(INNER_DST_PORT) ?
            0 : cpu_to_be16(rule->tuples_mask.dst_port);

    spec->tos = rule->tuples.ip_tos;
    spec_mask->tos = rule->unused_tuple & BIT(INNER_IP_TOS) ?
            0 : rule->tuples_mask.ip_tos;
}

static void hclge_fd_get_ip4_info(struct hclge_fd_rule *rule,
                                  struct ethtool_usrip4_spec *spec,
                                  struct ethtool_usrip4_spec *spec_mask)
{
    spec->ip4src = cpu_to_be32(rule->tuples.src_ip[IPV4_INDEX]);
    spec_mask->ip4src = rule->unused_tuple & BIT(INNER_SRC_IP) ?
                        0 : cpu_to_be32(rule->tuples_mask.src_ip[IPV4_INDEX]);

    spec->ip4dst = cpu_to_be32(rule->tuples.dst_ip[IPV4_INDEX]);
    spec_mask->ip4dst = rule->unused_tuple & BIT(INNER_DST_IP) ?
                        0 : cpu_to_be32(rule->tuples_mask.dst_ip[IPV4_INDEX]);

    spec->tos = rule->tuples.ip_tos;
    spec_mask->tos = rule->unused_tuple & BIT(INNER_IP_TOS) ?
                     0 : rule->tuples_mask.ip_tos;

    spec->proto = rule->tuples.ip_proto;
    spec_mask->proto = rule->unused_tuple & BIT(INNER_IP_PROTO) ?
                       0 : rule->tuples_mask.ip_proto;

    spec->ip_ver = ETH_RX_NFC_IP4;
}

static void hclge_fd_get_tcpip6_info(struct hclge_fd_rule *rule,
                                     struct ethtool_tcpip6_spec *spec,
                                     struct ethtool_tcpip6_spec *spec_mask)
{
    cpu_to_be32_array(spec->ip6src,
                      rule->tuples.src_ip, IPV6_SIZE);
    cpu_to_be32_array(spec->ip6dst,
                      rule->tuples.dst_ip, IPV6_SIZE);
    if (rule->unused_tuple & BIT(INNER_SRC_IP))
        (void)memset_s(spec_mask->ip6src, sizeof(spec_mask->ip6src), 0, sizeof(spec_mask->ip6src));
    else
        cpu_to_be32_array(spec_mask->ip6src, rule->tuples_mask.src_ip,
                          IPV6_SIZE);

    if (rule->unused_tuple & BIT(INNER_DST_IP))
        (void)memset_s(spec_mask->ip6dst, sizeof(spec_mask->ip6dst), 0, sizeof(spec_mask->ip6dst));
    else
        cpu_to_be32_array(spec_mask->ip6dst, rule->tuples_mask.dst_ip,
                          IPV6_SIZE);

    spec->tclass = rule->tuples.ip_tos;
    spec_mask->tclass = rule->unused_tuple & BIT(INNER_IP_TOS) ?
            0 : rule->tuples_mask.ip_tos;

    spec->psrc = cpu_to_be16(rule->tuples.src_port);
    spec_mask->psrc = rule->unused_tuple & BIT(INNER_SRC_PORT) ?
            0 : cpu_to_be16(rule->tuples_mask.src_port);

    spec->pdst = cpu_to_be16(rule->tuples.dst_port);
    spec_mask->pdst = rule->unused_tuple & BIT(INNER_DST_PORT) ?
                      0 : cpu_to_be16(rule->tuples_mask.dst_port);
}

static void hclge_fd_get_ip6_info(struct hclge_fd_rule *rule,
                                  struct ethtool_usrip6_spec *spec,
                                  struct ethtool_usrip6_spec *spec_mask)
{
    cpu_to_be32_array(spec->ip6src, rule->tuples.src_ip, IPV6_SIZE);
    cpu_to_be32_array(spec->ip6dst, rule->tuples.dst_ip, IPV6_SIZE);
    if (rule->unused_tuple & BIT(INNER_SRC_IP))
        (void)memset_s(spec_mask->ip6src, sizeof(spec_mask->ip6src), 0, sizeof(spec_mask->ip6src));
    else
        cpu_to_be32_array(spec_mask->ip6src,
                          rule->tuples_mask.src_ip, IPV6_SIZE);

    if (rule->unused_tuple & BIT(INNER_DST_IP))
        (void)memset_s(spec_mask->ip6dst, sizeof(spec_mask->ip6dst), 0, sizeof(spec_mask->ip6dst));
    else
        cpu_to_be32_array(spec_mask->ip6dst,
                          rule->tuples_mask.dst_ip, IPV6_SIZE);

    spec->tclass = rule->tuples.ip_tos;
    spec_mask->tclass = rule->unused_tuple & BIT(INNER_IP_TOS) ?
                        0 : rule->tuples_mask.ip_tos;

    spec->l4_proto = rule->tuples.ip_proto;
    spec_mask->l4_proto = rule->unused_tuple & BIT(INNER_IP_PROTO) ?
                          0 : rule->tuples_mask.ip_proto;
}

static void hclge_fd_get_ether_info(struct hclge_fd_rule *rule,
                                    struct ethhdr *spec,
                                    struct ethhdr *spec_mask)
{
    ether_addr_copy(spec->h_source, rule->tuples.src_mac);
    ether_addr_copy(spec->h_dest, rule->tuples.dst_mac);

    if (rule->unused_tuple & BIT(INNER_SRC_MAC))
        eth_zero_addr(spec_mask->h_source);
    else
        ether_addr_copy(spec_mask->h_source, rule->tuples_mask.src_mac);

    if (rule->unused_tuple & BIT(INNER_DST_MAC))
        eth_zero_addr(spec_mask->h_dest);
    else
        ether_addr_copy(spec_mask->h_dest, rule->tuples_mask.dst_mac);

    spec->h_proto = cpu_to_be16(rule->tuples.ether_proto);
    spec_mask->h_proto = rule->unused_tuple & BIT(INNER_ETH_TYPE) ?
                         0 : cpu_to_be16(rule->tuples_mask.ether_proto);
}

static void hclge_fd_get_user_def_info(struct ethtool_rx_flow_spec *fs,
                                       struct hclge_fd_rule *rule)
{
    if ((rule->unused_tuple & HCLGE_PLF_FD_TUPLE_USER_DEF_TUPLES) ==
        HCLGE_PLF_FD_TUPLE_USER_DEF_TUPLES) {
        fs->h_ext.data[0] = 0;
        fs->h_ext.data[1] = 0;
        fs->m_ext.data[0] = 0;
        fs->m_ext.data[1] = 0;
    } else {
        fs->h_ext.data[0] = cpu_to_be16(rule->user_def.offset);
        fs->h_ext.data[1] = cpu_to_be16(rule->user_def.data);
        fs->m_ext.data[0] = cpu_to_be16(HCLGE_FD_USER_DEF_OFFSET_UNMASK);
        fs->m_ext.data[1] = cpu_to_be16(rule->user_def.data_mask);
    }
}

static void hclge_fd_get_ext_info(struct ethtool_rx_flow_spec *fs,
                                  struct hclge_fd_rule *rule)
{
    if (fs->flow_type & FLOW_EXT) {
        fs->h_ext.vlan_tci = cpu_to_be16(rule->tuples.vlan_tag1);
        fs->m_ext.vlan_tci = rule->unused_tuple & BIT(INNER_VLAN_TAG_INNER) ?
                             0 : cpu_to_be16(rule->tuples_mask.vlan_tag1);

        hclge_fd_get_user_def_info(fs, rule);
    }

    if (fs->flow_type & FLOW_MAC_EXT) {
        ether_addr_copy(fs->h_ext.h_dest, rule->tuples.dst_mac);
        if (rule->unused_tuple & BIT(INNER_DST_MAC))
            eth_zero_addr(fs->m_u.ether_spec.h_dest);
        else
            ether_addr_copy(fs->m_u.ether_spec.h_dest,
                            rule->tuples_mask.dst_mac);
    }
}

int hclge_plf_get_fd_rule_info(struct hnae3_handle *handle,
                               struct ethtool_rxnfc *cmd)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    struct hclge_fd_rule *rule = NULL;
    struct ethtool_rx_flow_spec *fs;
    struct hlist_node *node;

    if (!hnae3_ae_dev_fd_supported(hdev->ae_dev))
        return -EOPNOTSUPP;

    fs = (struct ethtool_rx_flow_spec *)&cmd->fs;
    if (fs->location >= USER_BC_MC_ENTRY_NUM - 1) {
        dev_err(&hdev->pdev->dev,
            "Failed to get fd rule info, invalid rule location: %u, max is %u\n",
            fs->location,
            USER_BC_MC_ENTRY_NUM - 1);
        return -EINVAL;
    }

    spin_lock_bh(&hdev->fd_rule_lock);

    hlist_for_each_entry_safe(rule, node, &hdev->fd_rule_list, rule_node) {
        if (rule->location >= fs->location)
            break;
    }

    if (!rule || fs->location != rule->location) {
        spin_unlock_bh(&hdev->fd_rule_lock);
        return -ENOENT;
    }

    fs->flow_type = rule->flow_type;
    switch (fs->flow_type & ~(FLOW_EXT | FLOW_MAC_EXT)) {
        case SCTP_V4_FLOW:
        case TCP_V4_FLOW:
        case UDP_V4_FLOW:
            hclge_fd_get_tcpip4_info(rule, &fs->h_u.tcp_ip4_spec,
                                     &fs->m_u.tcp_ip4_spec);
            break;
        case IP_USER_FLOW:
            hclge_fd_get_ip4_info(rule, &fs->h_u.usr_ip4_spec,
                                  &fs->m_u.usr_ip4_spec);
            break;
        case SCTP_V6_FLOW:
        case TCP_V6_FLOW:
        case UDP_V6_FLOW:
            hclge_fd_get_tcpip6_info(rule, &fs->h_u.tcp_ip6_spec,
                                     &fs->m_u.tcp_ip6_spec);
            break;
        case IPV6_USER_FLOW:
            hclge_fd_get_ip6_info(rule, &fs->h_u.usr_ip6_spec,
                                  &fs->m_u.usr_ip6_spec);
            break;
        /* The flow type of fd rule has been checked before adding in to rule
        * list. As other flow types have been handled, it must be ETHER_FLOW
        * for the default case
        */
        default:
            hclge_fd_get_ether_info(rule, &fs->h_u.ether_spec, &fs->m_u.ether_spec);
            break;
    }

    hclge_fd_get_ext_info(fs, rule);

    if (rule->action == HCLGE_FD_ACTION_DROP_PACKET) {
        fs->ring_cookie = RX_CLS_FLOW_DISC;
    } else {
        fs->ring_cookie = rule->queue_id;
    }

    spin_unlock_bh(&hdev->fd_rule_lock);

    return 0;
}

int hclge_plf_get_all_rules(struct hnae3_handle *handle,
                            struct ethtool_rxnfc *cmd, u32 *rule_locs)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    struct hclge_fd_rule *rule;
    struct hlist_node *node;
    int cnt = 0;

    if (!hnae3_ae_dev_fd_supported(hdev->ae_dev))
        return -EOPNOTSUPP;

    cmd->data = USER_BC_MC_ENTRY_NUM - 1;

    spin_lock_bh(&hdev->fd_rule_lock);
    hlist_for_each_entry_safe(rule, node, &hdev->fd_rule_list, rule_node) {
        if (rule->location >= USER_BC_MC_ENTRY_NUM - 1) {
            continue;
        }
        if (cnt == cmd->rule_cnt) {
            spin_unlock_bh(&hdev->fd_rule_lock);
            return -EMSGSIZE;
        }

        if (rule->state == HCLGE_FD_TO_DEL)
            continue;

        rule_locs[cnt] = rule->location;
        cnt++;
    }

    spin_unlock_bh(&hdev->fd_rule_lock);
    cmd->rule_cnt = cnt;

    return 0;
}

static void hclge_plf_clear_fd_rules_in_list(struct hclge_plf_dev *hdev,
                                             bool clear_list)
{
    struct hclge_fd_rule *rule;
    struct hlist_node *node;
    u16 location;

    if (!hnae3_ae_dev_fd_supported(hdev->ae_dev))
        return;

    spin_lock_bh(&hdev->fd_rule_lock);

    for_each_set_bit(location, hdev->fd_bmap, hdev->fd_cfg.rule_num)
        hclge_plf_fd_tcam_config(hdev, true, location, NULL, false);

    if (clear_list) {
        hlist_for_each_entry_safe(rule, node, &hdev->fd_rule_list,
                        rule_node) {
            hlist_del(&rule->rule_node);
            kfree(rule);
        }
        hdev->fd_active_type = HCLGE_FD_RULE_NONE;
        hdev->hclge_fd_rule_num = 0;
        hdev->hclge_fd_user_rule_num = 0;
        bitmap_zero(hdev->fd_bmap, hdev->fd_cfg.rule_num);
    }

    spin_unlock_bh(&hdev->fd_rule_lock);
}

static void hclge_plf_fd_disable_user_def(struct hclge_plf_dev *hdev)
{
    struct hclge_fd_user_def_cfg *cfg = hdev->fd_cfg.user_def_cfg;

    spin_lock_bh(&hdev->fd_rule_lock);
    (void)memset_s(cfg, sizeof(hdev->fd_cfg.user_def_cfg), 0, sizeof(hdev->fd_cfg.user_def_cfg));
    spin_unlock_bh(&hdev->fd_rule_lock);

    hclge_plf_fd_set_user_def(hdev, hdev->fd_cfg.user_def_cfg, HCLGE_FD_USER_DEF_LAYER_NUM);
}

void hclge_plf_del_all_fd_entries(struct hclge_plf_dev *hdev)
{
    hclge_plf_clear_fd_rules_in_list(hdev, true);
    hclge_plf_fd_disable_user_def(hdev);
}

static void hclge_plf_sync_fd_list(struct hclge_plf_dev *hdev, struct hlist_head *hlist)
{
    struct hclge_fd_rule *rule;
    struct hlist_node *node;
    int ret = 0;

    if (!test_and_clear_bit(HCLGE_STATE_FD_TBL_CHANGED, &hdev->state))
        return;

    spin_lock_bh(&hdev->fd_rule_lock);

    hlist_for_each_entry_safe(rule, node, hlist, rule_node) {
        switch (rule->state) {
            case HCLGE_FD_TO_ADD:
                ret = hclge_plf_fd_config_rule(hdev, rule);
                if (ret)
                    goto out;
                rule->state = HCLGE_FD_ACTIVE;
                break;
            case HCLGE_FD_TO_DEL:
                ret = hclge_plf_fd_tcam_config(hdev, true, rule->location, NULL, false);
                if (ret)
                    goto out;
                hclge_fd_dec_rule_cnt(hdev, rule->location);
                hclge_fd_free_node(hdev, rule);
                break;
            default:
                break;
        }
    }

out:
    if (ret)
        set_bit(HCLGE_STATE_FD_TBL_CHANGED, &hdev->state);

    spin_unlock_bh(&hdev->fd_rule_lock);
}

void hclge_plf_sync_fd_table(struct hclge_plf_dev *hdev)
{
    if (test_and_clear_bit(HCLGE_STATE_FD_CLEAR_ALL, &hdev->state)) {
        hclge_plf_clear_fd_rules_in_list(hdev, false);
    }

    hclge_plf_sync_fd_user_def_cfg(hdev, false);

    hclge_plf_sync_fd_list(hdev, &hdev->fd_rule_list);
}

int hclge_plf_restore_fd_entries(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    struct hclge_fd_rule *rule;
    struct hlist_node *node;

    /* Return ok here, because reset error handling will check this
     * return value. If error is returned here, the reset process will
     * fail.
     */
    if (!hnae3_ae_dev_fd_supported(hdev->ae_dev))
        return 0;

    /* if fd is disabled, should not restore it when reset */
    if (!hdev->fd_en)
        return 0;

    spin_lock_bh(&hdev->fd_rule_lock);
    hlist_for_each_entry_safe(rule, node, &hdev->fd_rule_list, rule_node) {
        if (rule->state == HCLGE_FD_ACTIVE)
            rule->state = HCLGE_FD_TO_ADD;
    }
    spin_unlock_bh(&hdev->fd_rule_lock);
    set_bit(HCLGE_STATE_FD_TBL_CHANGED, &hdev->state);

    return 0;
}

void hclge_plf_enable_fd(struct hnae3_handle *handle, bool enable)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;

    hdev->fd_en = enable;

    if (!enable)
        set_bit(HCLGE_STATE_FD_CLEAR_ALL, &hdev->state);
    else
        hclge_plf_restore_fd_entries(handle);
#ifndef DEFINE_HNS_LLT
    hclge_plf_task_schedule(hdev, 0);
#endif

    return;
}
