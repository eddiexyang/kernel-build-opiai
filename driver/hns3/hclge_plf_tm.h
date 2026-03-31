/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: hclge_plf_tm
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __HCLGE_PLF_TM_H
#define __HCLGE_PLF_TM_H

#include <linux/types.h>
#include "hnae3.h"

/* PA REG */
#define PA_PRI_MAP_TC 0x4300

/* SSU REG */
#define SSU_ETS_UP_LENTH_OFFSET 0x814
#define SSU_MAC_ETS_TC_WEIGHT 0xA000
#define SSU_MAC_ETS_TCG_SHAPING 0xA208
#define SSU_MAC_ETS_TC_SHAPING 0xA218
#define SSU_MAC_ETS_PORT_MAPING 0xA31c
#define SSU_MAC_ETS_TCG_MAPING 0xA32c
#define SSU_MAC_ETS_TC_MAPING 0xA33c
#define SSU_MAC_ETS_PORT_SHAPING 0xA43C
#define SSU_MAC_ETS_QUEUE_LINK_TC_CFG 0xA650
#define SSU_MAC_ETS_TC_SENDSLOPE 0xA850
#define SSU_MAC_ETS_TC_IDLESLOPE 0xA950
#define SSU_MAC_ETS_TC_HICREDIT 0xAA50
#define SSU_MAC_ETS_TC_LOCREDIT 0xAB50
#define SSU_MAC_ETS_TC_CBS_EN 0xAC50
#define SSU_MAC_ETS_TC_SPEED_SEL 0xAD50


#define SSU_HOST_ETS_TC_WEIGHT 0xB000
#define SSU_HOST_ETS_TCG_SHAPING 0xB208
#define SSU_HOST_ETS_TC_SHAPING 0xB218
#define SSU_HOST_ETS_PORT_MAPING 0xB31c
#define SSU_HOST_ETS_TCG_MAPING 0xB32c
#define SSU_HOST_ETS_TC_MAPING 0xB33c
#define SSU_HOST_ETS_PORT_SHAPING 0xB43C
#define SSU_HOST_ETS_QUEUE_LINK_TC_CFG 0xB650

/* RCB REG */
#define RCB_COM_ETS_TC_WEIGHT 0x1500
#define RCB_COM_ETS_TCG_SHAPING 0x1580
#define RCB_COM_ETS_TC_SHAPING 0x1600
#define RCB_COM_ETS_PORT_MAPING 0x1680
#define RCB_COM_ETS_TCG_MAPING 0x1700
#define RCB_COM_ETS_TC_MAPING 0x1780
#define RCB_COM_ETS_PORT_SHAPING 0x1800
#define RCB_COM_ETS_QUEUE_LINK_TC 0x1A00
#define RCB_COM_ETS_TC_LINK_TCG 0x1EA0
#define RCB_COM_ETS_TCG_LINK_PORT 0x1EE0
#define RCB_COMM_ETS_TC_SENDSLOPE 0x1B00
#define RCB_COMM_ETS_TC_IDLESLOPE 0x1B80
#define RCB_COMM_ETS_TC_HICREDIT 0x1C00
#define RCB_COMM_ETS_TC_LOCREDIT 0x1C80
#define RCB_COMM_ETS_TC_CBS_EN 0x1D00
#define RCB_COMM_ETS_TC_SPEED_SEL 0x1D80
#define RCB_COMM_ETS_QUEUE_OFFSET_LEN 0x1E60

/* MAC Pause */
#define HCLGE_TX_MAC_PAUSE_EN_MSK BIT(0)
#define HCLGE_RX_MAC_PAUSE_EN_MSK BIT(1)

#define HCLGE_TM_PORT_BASE_MODE_MSK BIT(0)

#define HCLGE_DEFAULT_PAUSE_TRANS_GAP 0x7F
#define HCLGE_DEFAULT_PAUSE_TRANS_TIME 0xFFFF

/* SP or DWRR */
#define HCLGE_TM_TX_SCHD_DWRR_MSK BIT(0)
#define HCLGE_TM_TX_SCHD_SP_MSK 0xFE

#define HCLGE_ETHER_MAX_RATE 400000

#define HCLGE_TM_PF_MAX_PRI_NUM 8
#define HCLGE_TM_PF_MAX_QSET_NUM 8

struct hclge_pg_to_pri_link_cmd {
    u8 pg_id;
    u8 rsvd[3];
    u8 pri_bit_map;
};

struct hclge_qs_to_pri_link_cmd {
    __le16 qs_id;
    __le16 rsvd;
    u8 priority;
#define HCLGE_TM_QS_PRI_LINK_VLD_MSK BIT(0)
    u8 link_vld;
};

struct hclge_nq_to_qs_link_cmd {
    __le16 nq_id;
    __le16 rsvd;
#define HCLGE_TM_Q_QS_LINK_VLD_MSK BIT(10)
#define HCLGE_TM_QS_ID_L_MSK GENMASK(9, 0)
#define HCLGE_TM_QS_ID_L_S 0
#define HCLGE_TM_QS_ID_H_MSK GENMASK(14, 10)
#define HCLGE_TM_QS_ID_H_S 10
#define HCLGE_TM_QS_ID_H_EXT_S 11
#define HCLGE_TM_QS_ID_H_EXT_MSK GENMASK(15, 11)
    __le16 qset_id;
};

struct hclge_tqp_tx_queue_tc_cmd {
    __le16 queue_id;
    __le16 rsvd;
    u8 tc_id;
    u8 rsvd1[3];
};

struct hclge_pg_weight_cmd {
    u8 pg_id;
    u8 dwrr;
};

struct hclge_priority_weight_cmd {
    u8 pri_id;
    u8 dwrr;
};

struct hclge_pri_sch_mode_cfg_cmd {
    u8 pri_id;
    u8 rsvd[3];
    u8 sch_mode;
};

struct hclge_qs_sch_mode_cfg_cmd {
    __le16 qs_id;
    u8 rsvd[2];
    u8 sch_mode;
};

struct hclge_qs_weight_cmd {
    __le16 qs_id;
    u8 dwrr;
};

struct hclge_ets_tc_weight_cmd {
    u8 tc_weight[HNAE3_MAX_TC];
    u8 weight_offset;
    u8 rsvd[15];
};

#define HCLGE_TM_SHAP_IR_B_MSK GENMASK(7, 0)
#define HCLGE_TM_SHAP_IR_B_LSH 0
#define HCLGE_TM_SHAP_IR_U_MSK GENMASK(11, 8)
#define HCLGE_TM_SHAP_IR_U_LSH 8
#define HCLGE_TM_SHAP_IR_S_MSK GENMASK(15, 12)
#define HCLGE_TM_SHAP_IR_S_LSH 12
#define HCLGE_TM_SHAP_BS_B_MSK GENMASK(20, 16)
#define HCLGE_TM_SHAP_BS_B_LSH 16
#define HCLGE_TM_SHAP_BS_S_MSK GENMASK(25, 21)
#define HCLGE_TM_SHAP_BS_S_LSH 21

enum hclge_shap_bucket {
    HCLGE_TM_SHAP_C_BUCKET = 0,
    HCLGE_TM_SHAP_P_BUCKET
};

/* set bit HCLGE_TM_RATE_VLD to 1 means use 'rate' to config shaping */
#define HCLGE_TM_RATE_VLD 0

struct hclge_pri_shapping_cmd {
    u8 pri_id;
    u8 rsvd[3];
    __le32 pri_shapping_para;
    u8 flag;
    u8 rsvd1[3];
    __le32 pri_rate;
};

struct hclge_pg_shapping_cmd {
    u8 pg_id;
    u8 rsvd[3];
    __le32 pg_shapping_para;
    u8 flag;
    u8 rsvd1[3];
    __le32 pg_rate;
};

struct hclge_qs_shapping_cmd {
    __le16 qs_id;
    u8 rsvd[2];
    __le32 qs_shapping_para;
    u8 flag;
    u8 rsvd1[3];
    __le32 qs_rate;
};

#define HCLGE_BP_GRP_NUM 32
#define HCLGE_BP_SUB_GRP_ID_S 0
#define HCLGE_BP_SUB_GRP_ID_M GENMASK(4, 0)
#define HCLGE_BP_GRP_ID_S 5
#define HCLGE_BP_GRP_ID_M GENMASK(9, 5)

#define HCLGE_BP_EXT_GRP_NUM 40
#define HCLGE_BP_EXT_GRP_ID_S 5
#define HCLGE_BP_EXT_GRP_ID_M GENMASK(10, 5)

struct hclge_bp_to_qs_map_cmd {
    u8 tc_id;
    u8 rsvd[2];
    u8 qs_group_id;
    __le32 qs_bit_map;
    u32 rsvd1;
};

struct hclge_pfc_en_cmd {
    u8 tx_rx_en_bitmap;
    u8 pri_en_bitmap;
};

struct hclge_cfg_pause_param_cmd {
    u8 mac_addr[ETH_ALEN];
    u8 pause_trans_gap;
    u8 rsvd;
    __le16 pause_trans_time;
    u8 rsvd1[6];
    /* extra mac address to do double check for pause frame */
    u8 mac_addr_extra[ETH_ALEN];
    u16 rsvd2;
};

struct hclge_pfc_stats_cmd {
    __le64 pkt_num[3];
};

struct hclge_port_shapping_cmd {
    __le32 port_shapping_para;
    u8 flag;
    u8 rsvd[3];
    __le32 port_rate;
};

struct hclge_shaper_ir_para {
    u8 ir_b;
    u8 ir_u;
    u8 ir_s;
};

struct hclge_tm_nodes_cmd {
    u8 pg_base_id;
    u8 pri_base_id;
    __le16 qset_base_id;
    __le16 queue_base_id;
    u8 pg_num;
    u8 pri_num;
    __le16 qset_num;
    __le16 queue_num;
};

struct hclge_tm_shaper_para {
    u32 rate;
    u8 ir_b;
    u8 ir_u;
    u8 ir_s;
    u8 bs_b;
    u8 bs_s;
    u8 flag;
};

#define hclge_tm_set_field(dest, string, val) \
    hnae3_set_field((dest), (HCLGE_TM_SHAP_##string##_MSK), (HCLGE_TM_SHAP_##string##_LSH), val)
#define hclge_tm_get_field(src, string) \
    hnae3_get_field((src), HCLGE_TM_SHAP_##string##_MSK, HCLGE_TM_SHAP_##string##_LSH)

typedef struct hclge_plf_dev compat_hdev;
void hclge_plf_tm_prio_tc_info_update(struct hclge_plf_dev *hdev, u8 *prio_tc);
int hclge_plf_tm_schd_setup_hw(struct hclge_plf_dev *hdev);
int hclge_plf_pause_setup_hw(struct hclge_plf_dev *hdev, bool init);
void hclge_plf_tm_schd_info_update(struct hclge_plf_dev *hdev, u8 num_tc);
int hclge_plf_tm_dwrr_cfg(struct hclge_plf_dev *hdev);
int hclge_plf_pfc_tx_stats_get(struct hclge_plf_dev *hdev, u64 *stats);
int hclge_plf_pfc_rx_stats_get(struct hclge_plf_dev *hdev, u64 *stats);
void hclge_plf_tm_pfc_info_update(struct hclge_plf_dev *hdev);

int hclge_plf_tm_schd_init(struct hclge_plf_dev *hdev);
int hclge_plf_tm_vport_map_update(struct hclge_plf_dev *hdev);
int hclge_plf_tm_init_hw(struct hclge_plf_dev *hdev, bool init);
#endif // __HCLGE_PLF_TM_H
