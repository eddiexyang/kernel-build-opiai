#include <linux/etherdevice.h>

#include "securec.h"
#include "hclge_cmd.h"
#include "hclge_main.h"
#include "hclge_tm.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

int hclge_tm_port_shaper_cfg(struct hclge_dev *hdev);
int hclge_shaper_para_calc(u32 ir, u8 shaper_level, struct hclge_shaper_ir_para *ir_para, u32 max_tm_rate);
extern int memcpy_s(void *dest, size_t destMax, const void *src, size_t count);

STATIC void hclge_drain_tos_array(u8 *tos, u8 tos_id, u8 *tc_ptr)
{
    *tc_ptr = (tos[tos_id >> 1U] >> ((tos_id & 1U) * TC_SHIFT)) & TC_MASK;
}

STATIC int hclge_qos_to_tc_map_get(struct hclge_dev *hdev)
{
    struct hclge_desc desc[DESC_CNT] = {0};
    u8 *tos = NULL;
    u8 tc_val = 0;
    u8 tos_id;
    u8 i;
    u8 index;
    int ret;

    for (i = 0; i < DESC_CNT; i++) {
        hclge_cmd_setup_basic_desc(&desc[i], HCLGE_OPC_QOS_MAP, true);
        /* The first descriptor set the NEXT bit to 1 */
        if (i == 0) {
            desc[i].flag |= cpu_to_le16(HCLGE_CMD_FLAG_NEXT);
        } else {
            desc[i].flag &= ~cpu_to_le16(HCLGE_CMD_FLAG_NEXT);
        }
    }
    ret = hclge_cmd_send(&hdev->hw, desc, DESC_CNT);
    if (ret) {
        dev_err(&hdev->pdev->dev, "hclge_qos_to_tc_map_get fail, ret[%d]\n", ret);
        return ret;
    }

    for (i = 0; i < DESC_CNT; i++) {
        tos = (u8 *)(desc[i].data);
        for (tos_id = 0; tos_id < HNAE3_MAX_USER_TOS / DESC_CNT; tos_id++) {
            index = tos_id + i * HNAE3_MAX_USER_TOS / DESC_CNT;
            hclge_drain_tos_array(tos, tos_id, &tc_val);
            hdev->tm_info.tos_tc[index] = tc_val;
        }
    }

    return 0;
}

STATIC void hclge_fill_tos_array(u8 *tos, u8 tos_id, u8 tc)
{
    tos[tos_id >> 1U] |= tc << ((tos_id & 1U) * TC_SHIFT);
}

STATIC int hclge_qos_to_tc_map_set(struct hclge_dev *hdev)
{
    struct hclge_desc desc[DESC_CNT] = {0};
    u8 *tos = NULL;
    u8 tos_id;
    u8 i;
    u8 index;

    for (i = 0; i < DESC_CNT; i++) {
        hclge_cmd_setup_basic_desc(&desc[i], HCLGE_OPC_QOS_MAP, false);
        /* The first descriptor set the NEXT bit to 1 */
        if (i == 0) {
            desc[i].flag |= cpu_to_le16(HCLGE_CMD_FLAG_NEXT);
        } else {
            desc[i].flag &= ~cpu_to_le16(HCLGE_CMD_FLAG_NEXT);
        }

        tos = (u8 *)(desc[i].data);
        for (tos_id = 0; tos_id < HNAE3_MAX_USER_TOS / DESC_CNT; tos_id++) {
            index = tos_id + i * HNAE3_MAX_USER_TOS / DESC_CNT;
            hclge_fill_tos_array(tos, tos_id, hdev->tm_info.tos_tc[index]);
        }
    }

    return hclge_cmd_send(&hdev->hw, desc, DESC_CNT);
}

struct hclge_dev* hclge_dev_get(int dev_id)
{
    struct pci_dev *pdev = hclge_get_pci_dev(dev_id);
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hclge_dev *hdev = NULL;

    if (pdev == NULL) {
        pr_err("hns3: no pdev, dev_id[%d].\n", dev_id);
        return NULL;
    }

    ae_dev = pci_get_drvdata(pdev);
    if (ae_dev == NULL) {
        pr_err("hns3: no drvdata, dev_id[%d].\n", dev_id);
        return NULL;
    }

    hdev = ae_dev->priv;
    if (hdev == NULL) {
        pr_err("hns3: no hdev, dev_id[%d]\n", dev_id);
        return NULL;
    }

    return hdev;
}

int hclge_dscp_map_set(int dev_id, u8 dscp_val, u8 tc_val)
{
    struct hclge_dev *hdev = hclge_dev_get(dev_id);
    int ret;

    if (hdev == NULL) {
        pr_err("hns3: dscp map set param err hdev is NULL, dev_id[%d].\n", dev_id);
        return -EINVAL;
    }

    if (tc_val >= (hdev->tc_max) || (dscp_val >= HNAE3_MAX_USER_TOS)) {
        dev_err(&hdev->pdev->dev, "tc_val[%u] or dscp_val[%u] is out of range\n",
            tc_val, dscp_val);
        return -EINVAL;
    }

    ret = hclge_qos_to_tc_map_get(hdev);
    if (ret) {
        dev_err(&hdev->pdev->dev, "get dscp to tc map fail, ret[%d]\n", ret);
        return ret;
    }

    hdev->tm_info.tos_tc[dscp_val] = tc_val;

    return hclge_qos_to_tc_map_set(hdev);
}
EXPORT_SYMBOL(hclge_dscp_map_set);

int hclge_dscp_map_get(int dev_id, u8 tos_tc[], const u8 tos_tc_len)
{
    struct hclge_dev *hdev = hclge_dev_get(dev_id);
    int ret;

    if (hdev == NULL || tos_tc == NULL || tos_tc_len != HNAE3_MAX_USER_TOS) {
        pr_err("hns3: dscp map get param err dev_id[%d], tos_tc_len[%u], hdev is %pK, tos_tc is %pK\n",
               dev_id, tos_tc_len, hdev, tos_tc);
        return -EINVAL;
    }

    ret = hclge_qos_to_tc_map_get(hdev);
    if (ret) {
        dev_err(&hdev->pdev->dev, "get dscp to tc map fail, ret[%d]\n", ret);
        return ret;
    }

    ret = memcpy_s(tos_tc, tos_tc_len, hdev->tm_info.tos_tc, tos_tc_len);
    if (ret) {
        dev_err(&hdev->pdev->dev, "memcpy_s tos_tc fail, ret[%d]\n", ret);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(hclge_dscp_map_get);

int hclge_tm_port_shaping_set(int dev_id, int bw_limit)
{
	struct hclge_dev *hdev = hclge_dev_get(dev_id);
	int bw_limit_tmp;
	int ret;

	if (hdev == NULL) {
		pr_err("hns3: TM port shaping set err: hdev is NULL, dev_id[%d].\n", dev_id);
		return -EINVAL;
	}

	if (bw_limit < HCLGE_ETHER_MIN_RATE || (bw_limit > hdev->hw.mac.speed)) {
		pr_err("hns3: TM port shaper bw_limit[%d] out of range.", bw_limit);
		return -EINVAL;
	}

	bw_limit_tmp = hdev->tm_info.bw_limit;
	hdev->tm_info.bw_limit = bw_limit;

	/* Cfg port shaper */
	ret = hclge_tm_port_shaper_cfg(hdev);
	if (ret) {
		dev_err(&hdev->pdev->dev, "hclge_tm_port_shaper_cfg err %d\n", ret);
		hdev->tm_info.bw_limit = bw_limit_tmp;
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL(hclge_tm_port_shaping_set);

int hclge_tm_port_shaping_get(int dev_id, struct hclge_limit_port_info *get_port_info)
{
	struct hclge_shaper_ir_para ir_para;
	struct hclge_dev *hdev = NULL;
	u32 max_tm_rate;
	int ret;

	if (dev_id > MAX_DEV_ID || dev_id < 0 || get_port_info == NULL) {
		pr_err("hns3: tm_port_shaping_get(%d): param is invalid, devid(%u), get_port_info(%pK)\n", __LINE__, dev_id,
			get_port_info);
		return -EINVAL;
	}

	hdev = hclge_dev_get(dev_id);
	if (hdev == NULL) {
		pr_err("hns3: TM port shaping get param err: hdev is NULL, dev_id[%d].\n", dev_id);
		return -EINVAL;
	}

	max_tm_rate = hdev->ae_dev->dev_specs.max_tm_rate;
	ret = hclge_shaper_para_calc(hdev->tm_info.bw_limit,
								 HCLGE_SHAPER_LVL_PORT,
								 &ir_para, max_tm_rate);
	if (ret) {
		dev_err(&hdev->pdev->dev,
			"TM port shaping: hclge calc shaper para fail, dev id [%d] ret[%d]\n", ret, dev_id);
		return ret;
	}

	get_port_info->ir_b = ir_para.ir_b;
	get_port_info->ir_u = ir_para.ir_u;
	get_port_info->ir_s = ir_para.ir_s;
	get_port_info->bs_b = HCLGE_SHAPER_BS_U_DEF;
	get_port_info->bs_s = HCLGE_SHAPER_BS_S_DEF;
	get_port_info->bw_limit = hdev->tm_info.bw_limit;
	get_port_info->bw_max_cap = hdev->hw.mac.speed;

	return 0;
}
EXPORT_SYMBOL(hclge_tm_port_shaping_get);

int hns_get_firmware_version_info(int chip_id, u8 *version, u32 *length)
{
	u8 product_ver, major_ver, subversion, phase_ver;
	struct hclge_dev *hdev = hclge_dev_get(chip_id);

	if (hdev == NULL) {
		pr_err("hns3: hns_get_firmware_version_info err hdev is NULL, chip_id[%d].\n", chip_id);
		return -EINVAL;
	}

	if (version == NULL || length == NULL) {
		dev_err(&hdev->pdev->dev, "version or length is NULL\n");
		return -EINVAL;
	}

	if (*length == 0) {
		dev_err(&hdev->pdev->dev, "length is 0\n");
		return -EINVAL;
	}

	product_ver = (hdev->fw_version >> 24U) & 0xFF;
	major_ver = (hdev->fw_version >> 16U) & 0xFF;
	subversion = (hdev->fw_version >> 8U) & 0xFF;
	phase_ver = hdev->fw_version & 0xFF;

	snprintf_s(version, *length, *length - 1, "%u.%u.%u.%u", product_ver, major_ver, subversion, phase_ver);

	*length = strlen(version) + 1;
	pr_info("hns3: hns_get_firmware_version_info: chipid=%d, version=%s, length=%u\n", chip_id, version, *length);
	return 0;
}
EXPORT_SYMBOL(hns_get_firmware_version_info);