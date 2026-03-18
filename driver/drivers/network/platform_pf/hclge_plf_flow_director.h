/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: platform flow director
 * Author: huawei
 * Create: 2022-08-29
 */

#ifndef __HCLGE_PLF_FLOW_DIRECTOR_H
#define __HCLGE_PLF_FLOW_DIRECTOR_H

#include "hnae3.h"


#define MAX_FD_MODE 3
#define TCAM_DATA_KEY 0
#define TCAM_DATA_AD 1

#define TEMPLATE_ID_IPV6_OFFSET 4
#define BITS_OF_ONE_BYTE 8

#define CFG_DF_KEY_X 1
#define CFG_DF_KEY_Y 0

#define TCAM_ENRTY_NULL 0

#define FD_KEY_DATA_REG_CNT 13
#define FD_AD_DATA_REG_CNT 1

#define KEY_VLD_BIT BIT(16)
#define FD_KEY_SATAUS_BIT BIT(3)
#define FD_CNT_SATAUS_BIT BIT(0)

#define HCLGE_PORT_ID_S 0
#define HCLGE_PORT_ID_M GENMASK(1, 0)
#define HCLGE_PORT_TYPE_B 11

#define MAX_KEY_LENGTH_MODE0 100
#define MAX_KEY_LENGTH_MODE1 200
#define MAX_KEY_LENGTH_MODE2 400

#define MAX_KEY_DWORDS DIV_ROUND_UP(MAX_KEY_LENGTH_MODE2 / 8, 4)
#define MAX_KEY_BYTES (MAX_KEY_DWORDS * 4)
#define MAX_META_DATA_LENGTH 32

#define HCLGE_PLF_FD_KEY_CFG_ADDR_M GENMASK(18, 16)
#define HCLGE_PLF_FD_KEY_CFG_ADDR_S 16
#define HCLGE_PLF_FD_KEY_CFG_EN_ADDR_S 4
#define HCLGE_PLF_FD_KEY_CFG_SEL_ADDR_S 0
#define FD_KEY_CFG_STATUS_MSK 0x8

#define HCLGE_FD_MAX_USER_DEF_OFFSET 9000
#define HCLGE_FD_USER_DEF_DATA GENMASK(15, 0)
#define HCLGE_FD_USER_DEF_OFFSET GENMASK(15, 0)
#define HCLGE_FD_USER_DEF_OFFSET_UNMASK GENMASK(15, 0)

/* assigned by firmware, the real filter number for each pf may be less */
#define MAX_FD_FILTER_NUM 256

typedef union {
    struct {
        u32 cfg_fd_key_cmd : 2;       /* bit[1:0] */
        u32 reserve0 : 1;             /* bit[2] */
        u32 cfg_fd_key_status : 1;    /* bit[3] */
        u32 cfg_fd_key_cmd_en : 1;    /* bit[4] */
        u32 reserve1 : 1;             /* bit[5] */
        u32 cfg_fd_key_x : 1;         /* bit[6] */
        u32 reserve2 : 1;             /* bit[7] */
        u32 cfg_fd_key_ad_sel : 1;    /* bit[8] */
        u32 reserve3 : 7;             /* bit[15:9] */
        u32 cfg_fd_key_addr : 8;      /* bit[23:16] */
        u32 reserve4 : 8;             /* bit[31:24] */
    } bits;
    u32 val;
} fd_key_cfg;

typedef union {
    struct {
        unsigned int port_id : 2;     /* bit[1:0] */
        unsigned int reserve : 9;     /* bit[10:2] */
        unsigned int port_type : 1;   /* bit[11:11] */
        unsigned int reserve0 : 20;   /* bit[31:12] */
    } port_bits;
    unsigned int val;
} vport_number;

enum fd_tcam_data_type {
    FD_TCAM_KEY_DATA = 0,
    FD_TCAM_AD_DATA
};

enum hclge_plf_fd_key_op {
    FD_KEY_WRITE = 0,
    FD_KEY_READ,
    FD_KEY_LKUP
};

enum hclge_port_type {
    HOST_PORT,
    NETWORK_PORT
};

enum plf_rw_op {
    PLF_WRITE = 0,
    PLF_READ
};

enum hclge_plf_fd_key_type {
    HCLGE_PLF_FD_KEY_BASE_ON_PTYPE,
    HCLGE_PLF_FD_KEY_BASE_ON_TUPLE,
};

enum hclge_plf_fd_mode {
    HCLGE_PLF_FD_MODE_DEPTH_256_WIDTH_100B = 0, /* 256*100bit mode */
    HCLGE_PLF_FD_MODE_DEPTH_128_WIDTH_200B, /* 128*200bit mode */
    HCLGE_PLF_FD_MODE_DEPTH_64_WIDTH_400B, /* 64*400bit mode */
};

enum hclge_fd_key_type {
    HCLGE_FD_KEY_BASE_ON_PTYPE,
    HCLGE_FD_KEY_BASE_ON_TUPLE,
};

enum hclge_plf_fd_tuple {
    INNER_DST_MAC,
    INNER_SRC_MAC,
    INNER_VLAN_TAG_OUTER,
    INNER_VLAN_TAG_INNER,
    INNER_ETH_TYPE,
    INNER_L2_UDF,
    INNER_IP_TOS,
    INNER_IP_PROTO,
    INNER_SRC_IP,
    INNER_DST_IP,
    INNER_L3_UDF,
    INNER_SRC_PORT,
    INNER_DST_PORT,
    INNER_L4_UDF,
    MAX_TUPLE,
};

#define HCLGE_PLF_FD_TUPLE_USER_DEF_TUPLES \
    (BIT(INNER_L2_UDF) | BIT(INNER_L3_UDF) | BIT(INNER_L4_UDF))

enum hclge_plf_fd_meta_data {
    PACKET_TYPE_ID,
    IP_FRAGEMENT,
    VLAN_NUMBER,
    SRC_PORT,
    DST_PORT,
    MPLS_PACKET,
    PACKET_TYPE_L3_ID,
    PACKET_TYPE_L4_ID,
    PACKET_TYPE_L5_ID,
    TEMPLATE_ID,
    MAX_META_DATA,
};

enum hclge_plf_fd_key_opt {
    KEY_OPT_U8,
    KEY_OPT_LE16,
    KEY_OPT_LE32,
    KEY_OPT_MAC,
    KEY_OPT_IP,
    KEY_OPT_VNI,
};

struct key_info {
    u8 key_type;
    u8 key_length; /* use bit as unit */
    enum hclge_plf_fd_key_opt key_opt;
    int offset;
    int moffset;
};

enum hclge_fd_active_rule_type {
    HCLGE_FD_RULE_NONE,
    HCLGE_FD_EP_ACTIVE,
};

enum hclge_fd_action {
    HCLGE_FD_ACTION_DROP_PACKET,
    HCLGE_FD_ACTION_ACCEPT_DIRECT_QUEID,
    HCLGE_FD_ACTION_ACCEPT_USE_RSS_GEN_QUEID,
};

enum hclge_fd_node_state {
    HCLGE_FD_TO_ADD,
    HCLGE_FD_TO_DEL,
    HCLGE_FD_ACTIVE,
    HCLGE_FD_DELETED,
};

enum hclge_fd_user_def_layer {
    HCLGE_FD_USER_DEF_NONE,
    HCLGE_FD_USER_DEF_L2,
    HCLGE_FD_USER_DEF_L3,
    HCLGE_FD_USER_DEF_L4,
};

#define HCLGE_FD_USER_DEF_LAYER_NUM 3
struct hclge_fd_user_def_cfg {
    u16 ref_cnt;
    u16 offset;
};

struct hclge_fd_user_def_info {
    enum hclge_fd_user_def_layer layer;
    u16 data;
    u16 data_mask;
    u16 offset;
};

struct hclge_fd_key_cfg {
    u8 temp_id;
    u8 key_sel;
    u32 tuple_active;
    u32 meta_data_active;
};

struct hclge_plf_tuple_cfg {
    u32 tuple_offest_total;
    u32 fd_key_sel_tdata_0;
    u32 fd_key_sel_tdata_1;
    u8 tuple_size;
    const struct key_info *tuple_single_key_info;
};

struct hclge_plf_fd_cfg {
    u8 fd_mode;
    u16 max_key_length; /* use bit as unit */
    u32 rule_num; /* rule entry number */
    u16 cnt_num;  /* rule hit counter number */
    struct hclge_fd_key_cfg key_cfg;
    struct hclge_fd_user_def_cfg user_def_cfg[HCLGE_FD_USER_DEF_LAYER_NUM];
};

#define IPV4_INDEX 3
#define IPV6_SIZE 4
struct hclge_fd_rule_tuples {
    u8 src_mac[ETH_ALEN];
    u8 dst_mac[ETH_ALEN];
    /* Be compatible for ip address of both ipv4 and ipv6.
     * For ipv4 address, we store it in src/dst_ip[3].
     */
    u32 src_ip[IPV6_SIZE];
    u32 dst_ip[IPV6_SIZE];
    u16 src_port;
    u16 dst_port;
    u16 vlan_tag1;
    u16 ether_proto;
    u16 l2_user_def;
    u16 l3_user_def;
    u32 l4_user_def;
    u8 ip_tos;
    u8 ip_proto;
};

struct hclge_fd_rule {
    struct hlist_node rule_node;
    struct hclge_fd_rule_tuples tuples;
    struct hclge_fd_rule_tuples tuples_mask;
    u32 unused_tuple;
    u32 flow_type;
    u8 action;
    u16 vf_id;
    u16 queue_id;
    u8 tc_offect;
    u8 fd_tc_size;
    u16 location;
    u16 flow_id; /* only used for arfs */
    enum hclge_fd_active_rule_type rule_type;
    enum hclge_fd_node_state state;
    struct hclge_fd_user_def_info user_def;
};

struct hclge_plf_fd_ad_data {
    u16 ad_id; /* action id */
    u8 next_step; /* 1:drop   0:accept */
    u8 queue_id_mode; /* 1:direct QueID   0:use RSS gen QueID */
    u8 rss_tc_override_en;
    /* FD queue_id or FD_TC_offset
     * When Direct QueID/or use RSS gen QueID is 1, this field indicates FD_queue_id.
     * When Direct QueID/or use RSS gen QueID is 0 and RSS_TC_override_en is 0, this field is invalid.
     * When Direct QueID/or use RSS gen QueID is 0 and RSS_TC_override_en is 1, this field indicates FD_TC_offset.
     */
    u8 queue_id_tc_offset;
    u8 fd_tc_size;
    u8 rule_id;
    u8 statistic_cnt_vld; /* 0: invaild   1:vaild */
    u8 statistics_cnt_index;
};

typedef union {
    struct {
        u32 statistics_cnt_index : 5;        /* bit[4:0] */
        u32 statistic_cnt_vld : 1;           /* bit[5] */
        u32 rule_id : 8;                     /* bit[13:6] */
        u32 fd_tc_size : 3;                  /* bit[16:14] */
        u32 queue_id_tc_offset : 4;          /* bit[20:17] */
        u32 rss_tc_override_en : 1;          /* bit[21] */
        u32 queue_id_mode : 1;               /* bit[22] */
        u32 next_step : 1;                   /* bit[23] */
        u32 reserve : 8;                     /* bit[31:24] */
    } bits;
    u32 val;
} plf_fd_ad_reg;

/* For each bit of TCAM entry, it uses a pair of 'x' and
 * 'y' to indicate which value to match, like below:
 * ----------------------------------
 * | bit x | bit y |  search value  |
 * ----------------------------------
 * |   0   |   0   |   always hit   |
 * ----------------------------------
 * |   1   |   0   |   match '0'    |
 * ----------------------------------
 * |   0   |   1   |   match '1'    |
 * ----------------------------------
 * |   1   |   1   |   invalid      |
 * ----------------------------------
 * Then for input key(k) and mask(v), we can calculate the value by
 * the formulae:
 *	x = (~k) & v
 *	y = (k ^ ~v) & k
 */
#define calc_x(x, k, v) (x = ~(k) & (v))
#define calc_y(y, k, v)             \
    do {                            \
        const typeof(k) _k_ = (k);  \
        const typeof(v) _v_ = (v);  \
        (y) = (_k_ ^ ~_v_) & (_k_); \
    } while (0)

#define MAX_PORT_NUMBER 4
struct hclge_plf_fd_alloc_info {
    u16 stage_alloc_entry_num;
    u16 stage_start_entry_index;
};

int hclge_plf_add_fd_entry(struct hnae3_handle *handle, struct ethtool_rxnfc *cmd);
int hclge_plf_del_fd_entry(struct hnae3_handle *handle, struct ethtool_rxnfc *cmd);
int hclge_plf_add_fd_user_entry(struct hnae3_handle *handle, struct ethtool_rxnfc *cmd);
int hclge_plf_del_fd_user_entry(struct hnae3_handle *handle, struct ethtool_rxnfc *cmd);
int hclge_plf_get_fd_rule_cnt(struct hnae3_handle *handle, struct ethtool_rxnfc *cmd);
int hclge_plf_get_fd_rule_info(struct hnae3_handle *handle, struct ethtool_rxnfc *cmd);
int hclge_plf_get_all_rules(struct hnae3_handle *handle, struct ethtool_rxnfc *cmd, u32 *rule_locs);
void hclge_plf_enable_fd(struct hnae3_handle *handle, bool enable);
void hclge_plf_del_all_fd_entries(struct hclge_plf_dev *hdev);
void hclge_plf_sync_fd_table(struct hclge_plf_dev *hdev);
int hclge_plf_init_fd_config(struct hclge_plf_dev *hdev);
int hclge_plf_restore_fd_entries(struct hnae3_handle *handle);

#endif // __HCLGE_PLF_FLOW_DIRECTOR_H
