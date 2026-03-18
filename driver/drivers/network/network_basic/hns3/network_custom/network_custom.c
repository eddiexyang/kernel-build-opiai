// SPDX-License-Identifier: GPL-2.0+
// Copyright (c) 2016-2017 Hisilicon Limited.

#include <linux/dma-mapping.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/cpu_rmap.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/aer.h>
#include <linux/skbuff.h>
#include <linux/sctp.h>
#include <linux/kthread.h>
#include <net/gre.h>
#include <net/ip6_checksum.h>
#include <net/pkt_cls.h>
#include <net/tcp.h>
#include <net/vxlan.h>

#include "hnae3.h"
#include "hns3_enet.h"
#include "hns3_bbox.h"
#include "hclge_cmd.h"
#include "hclge_main.h"
#include "hclge_bbox.h"
#include "hclge_fault_manager.h"
#include "hclge_serdes_adapt.h"
#include "hclge_serdes_rate_switch.h"
#include "hclge_serdes_loopback.h"
#include "network_custom.h"
#include "ascend_kernel_hal.h"

static struct workqueue_struct *network_custom_wq;
static atomic_t ras_init_count = ATOMIC_INIT(0);
static atomic_t bbox_init_count = ATOMIC_INIT(0);
static unsigned long last_print_time = 0;

int hclge_get_mac_phy_link(struct hclge_dev *hdev, int *link_status, int *pcs_link_status);
static void hclge_wait_pcs_link(struct hclge_dev *hdev, int *pcs_link_status)
{
    int mac_link_status;
    int ret = 0;
    u8 count = 0;

    do {
        ret = hclge_get_mac_phy_link(hdev, &mac_link_status, pcs_link_status);
        if (ret) {
            dev_err(&hdev->pdev->dev, "hclge_get_mac_phy_link failed! ret=%d\n", ret);
            return;
        }
        if (*pcs_link_status == HCLGE_LINK_STATUS_UP) {
            break;
        }
        count++;
        msleep(1);
    } while (count < LINK_STATUS_DETECT_NUM);
}

static bool hclge_module_existed(struct hclge_dev *hdev)
{
    struct hclge_desc desc;
    u32 existed;
    int ret;

    hclge_cmd_setup_basic_desc(&desc, HCLGE_OPC_GET_SFP_EXIST, true);
    ret = hclge_cmd_send(&hdev->hw, &desc, 1);
    if (ret) {
        dev_err(&hdev->pdev->dev, "failed to get connector exist state, ret = %d\n", ret);
        return false;
    }

    existed = le32_to_cpu(desc.data[0]);

    return existed != 0;
}

static void hclge_no_lms_los_do_adapt(struct hclge_dev *hdev, u32 *lms_los_status)
{
    u8 count = 0;
    int ret = 0;

    do {
        ret = hclge_h60_check_los(hdev, lms_los_status, LMS_LOS);
        if (ret) {
            dev_err(&hdev->pdev->dev, "hclge check lms los failed! ret=%d\n", ret);
            return;
        }

        if (*lms_los_status == NO_LOSS) {
            break;
        }
        count++;
        hdev->hw.mac.old_los_status = LMS_LOSS;
    } while (count < LMS_LOS_DETECT_NUM);

    if (*lms_los_status == NO_LOSS && hdev->hw.mac.old_los_status == LMS_LOSS) {
        hdev->hw.mac.old_los_status = NO_LOSS;
        ret = hclge_do_h60_adapt(hdev);
        if (ret) {
            dev_err(&hdev->pdev->dev, "hclge detected lms los, do h60 adapt failed! ret=%d\n", ret);
            return;
        }
    }
}

static void hclge_xsfp_existed_do_adapt(struct hclge_dev *hdev, u32 *lms_los_status)
{
    u8 count = 0;
    int ret = 0;

    do {
        if (hclge_module_existed(hdev)) {
            break;
        }
        count++;
        hdev->hw.mac.old_xsfp_exist_flag = false;
    } while (count < XSFP_MODULE_DETECT_NUM);

    if (*lms_los_status == NO_LOSS && hclge_module_existed(hdev) && !hdev->hw.mac.old_xsfp_exist_flag) {
        hdev->hw.mac.old_xsfp_exist_flag = true;
        ret = hclge_do_h60_adapt(hdev);
        if (ret) {
            dev_err(&hdev->pdev->dev, "module exist status change, hclge do h60 adapt failed! ret=%d\n", ret);
            return;
        }
    }
}

static void hclge_detect_cdr_los_do_adapt(struct hclge_dev *hdev, u32 *lms_los_status)
{
    u32 cdr_los_status = 0;
    int ret = 0;

    ret = hclge_h60_check_los(hdev, &cdr_los_status, CDR_LOS);
    if (ret) {
        dev_err(&hdev->pdev->dev, "hclge check lms los failed! ret=%d\n", ret);
        return;
    }

    if (*lms_los_status == NO_LOSS && cdr_los_status != NO_LOSS) {
        ret = hclge_do_h60_adapt(hdev);
        if (ret) {
            dev_err(&hdev->pdev->dev, "hclge detected cdr los, do h60 adapt failed! ret=%d\n", ret);
            return;
        }
    }
}

static int hclge_query_mac_serdes_rate(struct hclge_dev *hdev, u8 *eth_speed)
{
    struct hclge_desc desc;
    int ret;

    if (eth_speed == NULL) {
        dev_err(&hdev->pdev->dev, "hclge query mac serdes rate failed, eth_speed is NULL\n");
        return -EINVAL;
    }

    hclge_cmd_setup_basic_desc(&desc, NETWORK_OPC_QUERY_MAC_SDS_RATE, true);
    ret = hclge_cmd_send(&hdev->hw, &desc, 1);
    if (ret) {
        dev_err(&hdev->pdev->dev, "Failed to query mac serdes rate, ret = %d\n", ret);
        return ret;
    }

    *eth_speed = (u8)le32_to_cpu(desc.data[0x0]);

    return 0;
}

static int hclge_update_serdes_datarate(struct hclge_dev *hdev)
{
    u8 eth_speed;
    int ret;

    ret = hclge_query_mac_serdes_rate(hdev, &eth_speed);
    if (ret) {
        dev_err(&hdev->pdev->dev, "hclge query mac serdes rate failed! ret=%d\n", ret);
        return ret;
    }

    ret = hclge_do_h60_datarate_switch(hdev, eth_speed);
    if (ret) {
        dev_err(&hdev->pdev->dev, "hclge do datarate switch failed! ret=%d\n", ret);
    }
    return ret;
}

int get_serdes_adapt_status(struct hclge_dev *hdev, u32 *adapt_status)
{
    struct hclge_desc desc = {0};
    int ret;

    hclge_cmd_setup_basic_desc(&desc, HCLGE_OPC_CONFIG_SERDES_ADAPT_STATUS, true);
    ret = hclge_cmd_send(&hdev->hw, &desc, 1);
    if (ret) {
        dev_err(&hdev->pdev->dev, "failed to get serdes adapt status, ret = %d\n", ret);
        return ret;
    }

    *adapt_status = le32_to_cpu(desc.data[0]);

    return 0;
}

void set_serdes_adapt_status(struct hclge_dev *hdev, u32 adapt_status)
{
    struct hclge_desc desc;
    int ret;

    hclge_cmd_setup_basic_desc(&desc, HCLGE_OPC_CONFIG_SERDES_ADAPT_STATUS, false);
    desc.data[0] = cpu_to_le32(adapt_status);
    ret = hclge_cmd_send(&hdev->hw, &desc, 1);
    if (ret) {
        dev_err(&hdev->pdev->dev, "failed to set serdes adapt status, ret = %d\n", ret);
        return;
    }
}

static void hclge_update_serdes_adapt(struct hclge_dev *hdev)
{
    int pcs_link_status = HCLGE_LINK_STATUS_DOWN;
    int mac_link_status = HCLGE_LINK_STATUS_DOWN;
    u32 lms_los_status = 0;
    u32 adapt_status;
    int ret = 0;

    if (!hclge_module_existed(hdev)) {
        return;
    }

    if (hdev->hw.mac.mac_type == HNAE3_MAC_ROH) {
        ret = get_serdes_adapt_status(hdev, &adapt_status);
        if (ret != 0 || adapt_status != ROH_SDS_LINK_DO_ADAPT) {
            return;
        }
    }

    ret = hclge_get_mac_phy_link(hdev, &mac_link_status, &pcs_link_status);
    if (ret) {
        dev_err(&hdev->pdev->dev, "hclge_get_mac_phy_link failed! ret=%d\n", ret);
        return;
    }

    if (mac_link_status == HCLGE_LINK_STATUS_DOWN || pcs_link_status == HCLGE_LINK_STATUS_DOWN) {
        ret = hclge_update_serdes_datarate(hdev);
        if (ret) {
            dev_err(&hdev->pdev->dev, "hclge update serdes datarate failed! ret=%d\n", ret);
            return;
        }
    }

    if (pcs_link_status == HCLGE_LINK_STATUS_DOWN) {
        ret = hclge_do_h60_adapt(hdev);
        if ((ret != 0) && time_after(jiffies, last_print_time + SERDES_PRINT_TIMEOUT * HZ)) {
            last_print_time = jiffies;
            dev_err(&hdev->pdev->dev, "hclge_update_serdes_adapt do h60 adapt failed! ret=%d\n", ret);
            return;
        }
        hclge_wait_pcs_link(hdev, &pcs_link_status);
        hclge_no_lms_los_do_adapt(hdev, &lms_los_status);
        hclge_xsfp_existed_do_adapt(hdev, &lms_los_status);
        hclge_detect_cdr_los_do_adapt(hdev, &lms_los_status);
    }

    if (hdev->hw.mac.mac_type == HNAE3_MAC_ROH) {
        set_serdes_adapt_status(hdev, ROH_SDS_LINK_ADAPT_DONE);
    }
}

int hclge_ioremap_hilink_reg(struct hclge_dev *hdev, unsigned long hilink_base_reg, unsigned long reg_size)
{
    struct pci_dev *pdev = hdev->pdev;

    if (!request_mem_region(hilink_base_reg, reg_size, "hilink0reg")) {
        dev_err(&pdev->dev, "request_mem_region failed!\n");
        return -EINVAL;
    }

    hdev->hw.hilink_base = ioremap(hilink_base_reg, reg_size);
    if (hdev->hw.hilink_base == NULL) {
        dev_err(&pdev->dev, "hclge_hilink_init ioremap h60 reg failed! hilink_base is NULL!\n");
        release_mem_region(hilink_base_reg, reg_size);
        return -ENOMEM;
    }

    return 0;
}

int hclge_hilink_init(struct hclge_dev *hdev)
{
    bool macro0_and_macro1 = (hdev->hw.mac.mac_id == HIMAC_MAC_0) && (hdev->mac_mode == MAC_MODE_400G);
    bool macro0_only =
        (hdev->hw.mac.mac_id == HIMAC_MAC_0) && (hdev->mac_mode == MAC_MODE_100G || hdev->mac_mode == MAC_MODE_200G);
    bool macro1_only =
        (hdev->hw.mac.mac_id == HIMAC_MAC_2) && (hdev->mac_mode == MAC_MODE_100G || hdev->mac_mode == MAC_MODE_200G);
    u8 connect_type = ((struct custom_dev *)hdev->custom_priv)->connect_type;
    u8 chip_id = (u8)hdev->chip_id;
    u8 die_id = (u8)hdev->die_id;
    int ret = 0;

    if (!hdev->h60_adapt_flag) {
        return 0;
    }

    if (macro0_only) {
        ret = hclge_ioremap_hilink_reg(hdev, H60_HILINK0_BASE_IOREMAP_ADDR(chip_id, die_id, connect_type),
                                       H60_SINGLE_MACRO_SIZE);
    }

    if (macro1_only) {
        ret = hclge_ioremap_hilink_reg(hdev, H60_HILINK1_BASE_IOREMAP_ADDR(chip_id, die_id, connect_type),
                                       H60_SINGLE_MACRO_SIZE);
    }

    if (macro0_and_macro1) {
        ret = hclge_ioremap_hilink_reg(hdev, H60_HILINK0_BASE_IOREMAP_ADDR(chip_id, die_id, connect_type),
                                       H60_REG_M0_M1_SIZE);
    }

    if (ret) {
        dev_err(&hdev->pdev->dev, "hclge_ioremap_hilink_reg failed! ret=%d\n", ret);
        return ret;
    }

    return 0;
}

void hclge_hilink_uninit(struct hclge_dev *hdev)
{
    bool macro0_and_macro1 = (hdev->hw.mac.mac_id == HIMAC_MAC_0) && (hdev->mac_mode == MAC_MODE_400G);
    bool macro0_only =
        (hdev->hw.mac.mac_id == HIMAC_MAC_0) && (hdev->mac_mode == MAC_MODE_100G || hdev->mac_mode == MAC_MODE_200G);
    bool macro1_only =
        (hdev->hw.mac.mac_id == HIMAC_MAC_2) && (hdev->mac_mode == MAC_MODE_100G || hdev->mac_mode == MAC_MODE_200G);
    u8 connect_type = ((struct custom_dev *)hdev->custom_priv)->connect_type;
    u8 chip_id = (u8)hdev->chip_id;
    u8 die_id = (u8)hdev->die_id;

    if (!hdev->h60_adapt_flag) {
        return;
    }

    dev_info(&hdev->pdev->dev, "begin to uninit hilink!\n");
    if (hdev->hw.hilink_base != NULL) {
        iounmap(hdev->hw.hilink_base);
    }
    if (macro0_only) {
        release_mem_region(H60_HILINK0_BASE_IOREMAP_ADDR(chip_id, die_id, connect_type), H60_SINGLE_MACRO_SIZE);
    }

    if (macro1_only) {
        release_mem_region(H60_HILINK1_BASE_IOREMAP_ADDR(chip_id, die_id, connect_type), H60_SINGLE_MACRO_SIZE);
    }

    if (macro0_and_macro1) {
        release_mem_region(H60_HILINK0_BASE_IOREMAP_ADDR(chip_id, die_id, connect_type), H60_REG_M0_M1_SIZE);
    }
}

void hclge_h60_adapt_task_schedule(struct hclge_dev *hdev, unsigned long delay_time)
{
    if (!test_bit(HCLGE_STATE_REMOVING, &hdev->state) && !test_bit(HCLGE_STATE_RST_FAIL, &hdev->state))
        mod_delayed_work_on(cpumask_first(&hdev->affinity_mask), network_custom_wq, &hdev->h60_adapt_task, delay_time);
}

void hclge_h60_adapt_task(struct work_struct *work)
{
    struct hclge_dev *hdev = container_of(work, struct hclge_dev, h60_adapt_task.work);

    if (test_bit(HCLGE_STATE_H60_ADAPT_HANDLING, &hdev->state)) {
        return;
    }

    set_bit(HCLGE_STATE_H60_ADAPT_HANDLING, &hdev->state);

    hclge_update_serdes_adapt(hdev);

    clear_bit(HCLGE_STATE_H60_ADAPT_HANDLING, &hdev->state);
    hclge_h60_adapt_task_schedule(hdev, round_jiffies_relative(HZ));
}

static int hclge_query_h60_adapt_flag(struct hclge_dev *hdev)
{
    struct hclge_desc desc;
    int ret = 0;

    hclge_cmd_setup_basic_desc(&desc, HCLGE_OPC_QUERY_H60_ADAPT, true);
    ret = hclge_cmd_send(&hdev->hw, &desc, 1);
    if (ret) {
        dev_err(&hdev->pdev->dev, "Query h60 adapt flag failed, ret = %d.\n", ret);
        return ret;
    }

    hdev->h60_adapt_flag = le32_to_cpu(desc.data[0]);
    return ret;
}

static int hclge_bbox_notify_imp(struct hclge_dev *hdev, u64 addr, u32 len)
{
    struct hclge_bbox_config_cmd *req = NULL;
    struct hclge_desc desc;
    int ret;

    hclge_cmd_setup_basic_desc(&desc, HCLGE_OPC_DFX_CONFIG_BBOX, false);

    req = (struct hclge_bbox_config_cmd *)desc.data;
    req->log_addr = cpu_to_le64(addr);
    req->log_len = cpu_to_le32(len);

    dev_info(&hdev->pdev->dev, "BBOX_NETWORK: imp bbox static memory len:%u\n", len);

    ret = hclge_cmd_send(&hdev->hw, &desc, 1);
    if (ret) {
        dev_err(&hdev->pdev->dev, "bbox notify imp fail, ret = %d\n", ret);
    }

    return ret;
}

static int hns3_bbox_notify_imp(struct hclge_dev *hdev)
{
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();
    int ret;
    u64 addr;
    u32 len;

    if (!hns_bbox_log_mem_is_valid()) {
        dev_err(&hdev->pdev->dev, "check bbox mem info failed!\n");
        return -EINVAL;
    }

    ret = 0;
    addr = hns_bbox->log_mem.log_addr + HNS_BBOX_IMP_ADDR_OFFSET;
    len = hns_bbox->log_mem.log_len - HNS_BBOX_IMP_ADDR_OFFSET;
    ret = hclge_bbox_notify_imp(hdev, addr, len);

    return ret;
}

static int hclge_query_imp_share_mem_info(struct hclge_dev *hdev)
{
    u8 connect_type = ((struct custom_dev *)hdev->custom_priv)->connect_type;
    struct hns_bbox_cb *hns_bbox = hns_bbox_get_cb();
    struct hclge_desc desc;
    int ret = 0;

    hclge_cmd_setup_basic_desc(&desc, NETWORK_OPC_QUERY_IMP_SHARE_MEM_INFO, true);
    ret = hclge_cmd_send(&hdev->hw, &desc, 1);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "query imp share mem info failed, ret = %d.\n", ret);
        return ret;
    }

    if (!hns_imp_share_mem_is_valid((u64)(le32_to_cpu(desc.data[0x0])), le32_to_cpu(desc.data[0x1]))) {
        dev_err(&hdev->pdev->dev, "check imp share mem info failed!\n");
        return -EINVAL;
    }

    hns_bbox->hns_imp_share_mem_addr =
        (((u64)(BBOX_ADDR_HIGH_OFFSET(hdev->chip_id, hdev->die_id, connect_type)) << 32UL) |
         (u64)(le32_to_cpu(desc.data[0x0])));

    hns_bbox->hns_imp_share_mem_len = le32_to_cpu(desc.data[0x1]);

    return ret;
}

static void hclge_imp_bbox_report_task_schedule(struct hclge_dev *hdev, unsigned long delay_time)
{
    if (!test_bit(HCLGE_STATE_REMOVING, &hdev->state))
        mod_delayed_work_on(cpumask_first(&hdev->affinity_mask),
                            network_custom_wq, &hdev->imp_bbox_report_task, delay_time);
}

static void hclge_imp_bbox_report_task(struct work_struct *work)
{
    struct hclge_dev *hdev = container_of(work, struct hclge_dev, imp_bbox_report_task.work);

    if (test_bit(HCLGE_STATE_IMP_BBOX_REPORT_HANDLING, &hdev->state)) {
        return;
    }

    set_bit(HCLGE_STATE_IMP_BBOX_REPORT_HANDLING, &hdev->state);

    hns_imp_bbox_report(hdev->die_id);

    clear_bit(HCLGE_STATE_IMP_BBOX_REPORT_HANDLING, &hdev->state);
    hclge_imp_bbox_report_task_schedule(hdev, round_jiffies_relative(HZ));
}


u8 g_mac_addr_temp[6];
static DECLARE_COMPLETION(get_mac_addr_wait);

static u16 hclge_calc_crc16(const u8 *pdata, u16 data_len)
{
    u8 crc_low, crc_high, cl, ch, save_high, save_low;
    u16 i, flag;

    crc_high = HCLGE_CRC16;
    crc_low = HCLGE_CRC16;
    ch = HCLGE_CRC_CH;
    cl = HCLGE_CRC_CL;

    for (i = 0; i < data_len; i++) {
        crc_low ^= *(pdata + i);
        for (flag = 0; flag < HCLGE_CRC_FLAG; flag++) {
            save_high = crc_high;
            save_low = crc_low;
            crc_high >>= 1;
            crc_low >>= 1;

            if ((save_high & 0x01) == 0x01)
                crc_low |= HCLGE_CRC_CODE;

            if ((save_low & 0x01) == 0x01) {
                crc_high ^= ch;
                crc_low ^= cl;
            }
        }
    }

    return (crc_high << HCLGE_CRC_FLAG) | crc_low;
}

static int hclge_read_mac(void *arg)
{
    u8 crc_data[HCLGE_CRC_DATA_LEN];
    struct hclge_mac_info mac_info;
    unsigned int len = 0;
    u16 crc_ret;
    int ret = 0;

    ret = devdrv_config_get_mac_info(0, (char *)(&mac_info),
                     sizeof(struct hclge_mac_info), &len);
    if (len != sizeof(struct hclge_mac_info) || ret < 0) {
        pr_err("hns3: drv get mac err, ret [%d], ret_len [%u].\n",
               ret, len);
        goto out;
    }

    ret = memcpy_s(crc_data, HCLGE_CRC_DATA_LEN,
               &mac_info.data_length, HCLGE_CRC_DATA_LEN);
    if (ret) {
        pr_err("hns3: memcpys_s err ret [%d].\n", ret);
        goto out;
    }

    crc_ret = hclge_calc_crc16(crc_data, HCLGE_CRC_DATA_LEN);
    if (crc_ret != mac_info.crc_value) {
        pr_err("hns3: crc_value [%hu], crc_ret [%hu].\n",
               mac_info.crc_value, crc_ret);
        goto out;
    }

    ether_addr_copy(g_mac_addr_temp, mac_info.mac_addr);
out:
    complete(&get_mac_addr_wait);
    return 0;
}

static void hclge_get_mac_addr(struct hclge_dev *hdev, u8 *p)
{
    struct task_struct *get_mac_task = NULL;

    /* Only the first PF obtains the MAC address from the flash. */
    if (hdev->pf_id != 0)
        return;

    /* Restricted by flash access permissions in tee hardware security,
     * a kernel thread is required to gain permission to read the flash.
     */
    get_mac_task = kthread_run(hclge_read_mac, NULL, "hns3_get_mac_addr");
    if (!get_mac_task) {
        dev_err(&hdev->pdev->dev,
            "create thread hns3_get_mac_addr failed.\n");
        return;
    }

    if (!wait_for_completion_timeout(&get_mac_addr_wait,
        msecs_to_jiffies(THREAD_GET_MAC_ADDR_TIMEOUT))) {
        dev_err(&hdev->pdev->dev,
            "thread hns3_get_mac_addr timeout.\n");
        goto out;
    }

    ether_addr_copy(p, g_mac_addr_temp);
out:
    (void)kthread_stop(get_mac_task);
}

static int hclge_init_flash_mac_addr(struct hclge_dev *hdev)
{
    struct net_device *netdev = hdev->vport[0].nic.netdev;
    struct hns3_nic_priv *priv = netdev_priv(netdev);
    struct hnae3_handle *h = priv->ae_handle;
    u8 mac_addr_temp[ETH_ALEN] = {0};
    int ret = 0;

    hclge_get_mac_addr(hdev, mac_addr_temp);
    /* Check if the MAC address is valid, if not get a random one */
    if (!is_valid_ether_addr(mac_addr_temp)) {
        return 0;
    } else if (!ether_addr_equal(netdev->dev_addr, mac_addr_temp)) {
        ether_addr_copy(netdev->dev_addr, mac_addr_temp);
        ether_addr_copy(netdev->perm_addr, mac_addr_temp);
    } else {
        return 0;
    }

    if (h->ae_algo->ops->set_mac_addr) {
        ret = h->ae_algo->ops->set_mac_addr(h, mac_addr_temp, false);
    }

    return ret;
}

int network_custom_client_init(struct hnae3_handle *handle)
{
    struct hnae3_ae_dev *ae_dev = hns3_get_ae_dev(handle);
    struct hclge_dev *hdev = ae_dev->priv;
    devdrv_hardware_info_t hardware_info;
    struct custom_dev *cstm_dev = NULL;
    int ret;

    dev_info(&hdev->pdev->dev, "custom client init start.\n");

    cstm_dev = kzalloc(sizeof(struct custom_dev), GFP_KERNEL);
    if (cstm_dev == NULL) {
        dev_err(&hdev->pdev->dev, "alloc custom_priv fail!\n");
        return -ENOMEM;
    }

    cstm_dev->hdev = hdev;
    hdev->custom_priv = (void *)cstm_dev;
    spin_lock_init(&(cstm_dev->link_record.lock));

    ret = hal_kernel_get_hardware_info(hdev->die_id, &hardware_info);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "hal kernel get hardware info failed, ret=%d\n", ret);
        return ret;
    }
    cstm_dev->connect_type = (u8)hardware_info.base_hw_info.connect_type;

    if (atomic_read(&ras_init_count) != 0) {
        atomic_inc(&ras_init_count);
    } else {
        atomic_inc(&ras_init_count);
        ret = hclge_fault_mgr_init(hdev);
        if (ret != 0) {
            dev_err(&hdev->pdev->dev, "hclge fault mgr init fail! ret=%d\n", ret);
            kfree(cstm_dev);
            hdev->custom_priv = NULL;
            cstm_dev = NULL;
            return ret;
        }
    }

    if (atomic_read(&bbox_init_count) != 0) {
        atomic_inc(&bbox_init_count);
    } else {
        atomic_inc(&bbox_init_count);
        ret = hns_bbox_init(hdev->chip_id);
        if (ret != 0) {
            dev_err(&hdev->pdev->dev, "bbox init fail! ret:%d\n", ret);
            goto err_fault_mgr;
        }
    }

    ret = hns3_bbox_notify_imp(hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "bbox notify imp fail! ret=%d\n", ret);
        goto err_bbox;
    }

    ret = hclge_query_imp_share_mem_info(hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "query imp share mem info fail! ret=%d\n", ret);
        goto err_bbox;
    }

    ret = hclge_query_h60_adapt_flag(hdev);
    if (ret) {
        dev_err(&hdev->pdev->dev, "query h60 adapt flag error %d.\n", ret);
        goto err_bbox;
    }

    ret = hclge_hilink_init(hdev);
    if (ret) {
        dev_err(&hdev->pdev->dev, "hclge_hilink_init failed! ret=%d\n", ret);
        goto err_bbox;
    }

    ret = hclge_init_flash_mac_addr(hdev);
    if (ret) {
        dev_warn(&hdev->pdev->dev, "hclge init mac addr failed! ret=%d\n", ret);
    }

    if (hdev->h60_adapt_flag) {
        hdev->hw.mac.old_xsfp_exist_flag = false;
        INIT_DELAYED_WORK(&hdev->h60_adapt_task, hclge_h60_adapt_task);
        hclge_h60_adapt_task_schedule(hdev, round_jiffies_relative(HZ));
    }

    INIT_DELAYED_WORK(&hdev->imp_bbox_report_task, hclge_imp_bbox_report_task);
    hclge_imp_bbox_report_task_schedule(hdev, round_jiffies_relative(HZ));

    dev_info(&hdev->pdev->dev, "custom client init success.\n");

    return ret;

err_bbox:
    if (atomic_dec_and_test(&bbox_init_count)) {
        hns_bbox_uninit();
    }
err_fault_mgr:
    if (atomic_dec_and_test(&ras_init_count)) {
        hclge_fault_mgr_unint();
    }
    kfree(cstm_dev);
    hdev->custom_priv = NULL;
    cstm_dev = NULL;
    return ret;
}

void network_custom_client_uninit(struct hnae3_handle *handle, bool reset)
{
    struct hnae3_ae_dev *ae_dev = hns3_get_ae_dev(handle);
    struct hclge_dev *hdev = ae_dev->priv;

    if ((hdev->h60_adapt_flag) && (hdev->h60_adapt_task.work.func != NULL)) {
        cancel_delayed_work_sync(&hdev->h60_adapt_task);
    }

    if (hdev->imp_bbox_report_task.work.func != NULL) {
        cancel_delayed_work_sync(&hdev->imp_bbox_report_task);
    }

    hns_bbox_excep_report(hdev->die_id, HNS_EXCEPID_CUSTOM_UNINIT);

    hclge_hilink_uninit(hdev);

    if (atomic_dec_and_test(&bbox_init_count)) {
        hns_bbox_uninit();
    }

    if (atomic_dec_and_test(&ras_init_count)) {
        hclge_fault_mgr_unint();
    }

    kfree(hdev->custom_priv);
    hdev->custom_priv = NULL;
}

static int hns3_reset_notify_init_custom(struct hnae3_handle *handle)
{
    struct hnae3_ae_dev *ae_dev = hns3_get_ae_dev(handle);
    struct hclge_dev *hdev = ae_dev->priv;
    int ret;

    ret = hns3_bbox_notify_imp(hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "hns3: bbox notify imp failed!\n");
        return ret;
    }

    ret = hclge_init_flash_mac_addr(hdev);
    if (ret) {
        dev_warn(&hdev->pdev->dev, "hclge reinit mac addr failed! ret=%d\n", ret);
    }

    return 0;
}

int network_custom_reset_notify(struct hnae3_handle *handle, enum hnae3_reset_notify_type type)
{
    int ret = 0;

    switch (type) {
        case HNAE3_UP_CLIENT:
            break;
        case HNAE3_DOWN_CLIENT:
            break;
        case HNAE3_INIT_CLIENT:
            ret = hns3_reset_notify_init_custom(handle);
            break;
        case HNAE3_UNINIT_CLIENT:
            break;
        default:
            break;
    }

    return ret;
}

int hclge_clear_link_his_stat(int dev_id)
{
    struct hclge_dev *hdev = hclge_dev_get(dev_id);
    struct hclge_link_record *link_record = NULL;
    struct custom_dev *cstm_dev = NULL;

    if (hdev == NULL) {
        pr_err("hns3: clear link history statistic err: hdev is NULL, dev_id[%d].\n", dev_id);
        return -EINVAL;
    }

    cstm_dev = (struct custom_dev *)hdev->custom_priv;
    if (cstm_dev == NULL) {
        pr_err("hns3: clear link history statistic err: cstm_dev is NULL, dev_id[%d].\n", dev_id);
        return -EINVAL;
    }
    link_record = &cstm_dev->link_record;

    spin_lock(&link_record->lock);

    link_record->link_up_cnt = 0;
    link_record->link_down_cnt = 0;
    link_record->link_stat_idx = 0;
    (void)memset_s((void *)&link_record->his_stat[0], sizeof(link_record->his_stat), 0, sizeof(link_record->his_stat));

    spin_unlock(&link_record->lock);

    return 0;
}
EXPORT_SYMBOL(hclge_clear_link_his_stat);

int hclge_get_link_his_stat(int dev_id, char *buf, int buf_len)
{
    struct hclge_link_his_stat *link_his_stat = (struct hclge_link_his_stat *)buf;
    struct hclge_dev *hdev = hclge_dev_get(dev_id);
    struct hclge_link_record *link_record = NULL;
    const long long time_base = UNIX_TIME_BASE;
    struct custom_dev *cstm_dev = NULL;
    unsigned int start_id = 0;
    struct timespec64 kts;
    unsigned int i;

    if (hdev == NULL) {
        pr_err("hns3: get link history statistic err: hdev is NULL, dev_id[%d].\n", dev_id);
        return -EINVAL;
    }

    if (buf == NULL || buf_len != sizeof(struct hclge_link_his_stat)) {
        dev_err(&hdev->pdev->dev, "get link history statistic input para error.\n");
        return -EINVAL;
    }

    cstm_dev = (struct custom_dev *)hdev->custom_priv;
    if (cstm_dev == NULL) {
        pr_err("hns3: get link history statistic err: cstm_dev is NULL, dev_id[%d].\n", dev_id);
        return -EINVAL;
    }
    link_record = &cstm_dev->link_record;

    ktime_get_real_ts64(&kts);
    if (kts.tv_sec <= time_base) {
        dev_err(&hdev->pdev->dev, "get link history statistic get time err kts.tv_sec:0x%llx.", kts.tv_sec);
        return -EIO;
    }

    spin_lock(&link_record->lock);
    link_his_stat->cur_tv_sec = (unsigned long long)kts.tv_sec;
    link_his_stat->link_up_cnt = link_record->link_up_cnt;
    link_his_stat->link_down_cnt = link_record->link_down_cnt;
    /* 满了之后 */
    if (link_record->his_stat[link_record->link_stat_idx % LINK_STAT_MAX_IDX].link_tv_sec != 0) {
        start_id = link_record->link_stat_idx % LINK_STAT_MAX_IDX;
        link_his_stat->stat_cnt = LINK_STAT_MAX_IDX;
    } else {
        link_his_stat->stat_cnt = link_record->link_stat_idx;
    }

    for (i = 0; i < LINK_STAT_MAX_IDX; i++) {
        link_his_stat->stat[i].link_status = link_record->his_stat[(start_id + i) % LINK_STAT_MAX_IDX].link_status;
        link_his_stat->stat[i].link_tv_sec = link_record->his_stat[(start_id + i) % LINK_STAT_MAX_IDX].link_tv_sec;
    }

    spin_unlock(&link_record->lock);

    return 0;
}
EXPORT_SYMBOL(hclge_get_link_his_stat);

void hclge_save_link_status(struct hclge_dev *hdev, int state)
{
    struct custom_dev *cstm_dev = NULL;
    struct hclge_link_record *link_record = NULL;
    const long long time_base = UNIX_TIME_BASE;
    struct timespec64 kts;

    if (hdev == NULL) {
        pr_err("hns3: save link status err: hdev is NULL.\n");
        return;
    }

    cstm_dev = (struct custom_dev *)hdev->custom_priv;
    if (cstm_dev == NULL) {
        pr_err("hns3: save link status err: cstm_dev is NULL.\n");
        return;
    }

    link_record = &cstm_dev->link_record;
    ktime_get_real_ts64(&kts);
    if (kts.tv_sec <= time_base) {
        dev_err(&hdev->pdev->dev, "save link history statistic get time err kts.tv_sec:0x%llx.", kts.tv_sec);
        return;
    }

    spin_lock(&link_record->lock);

    link_record->his_stat[link_record->link_stat_idx].link_tv_sec = (unsigned long long)kts.tv_sec;
    link_record->his_stat[link_record->link_stat_idx].link_status = (unsigned int)state;

    hclge_link_down_fault_report(hdev, state == 0);

    if (state) {
        link_record->link_up_cnt++;
    } else {
        link_record->link_down_cnt++;
    }
    link_record->link_stat_idx = (link_record->link_stat_idx + 1) % LINK_STAT_MAX_IDX;

    spin_unlock(&link_record->lock);
}

void network_custom_link_status_change(struct hnae3_handle *handle, bool linkup)
{
    struct hnae3_ae_dev *ae_dev = hns3_get_ae_dev(handle);
    if (ae_dev == NULL) {
        pr_err("hns3: save link status err: ae_dev is NULL.\n");
        return;
    }

    struct hclge_dev *hdev = ae_dev->priv;

    hclge_save_link_status(hdev, linkup);
}

const struct hnae3_client_ops client_ops = {
    .init_instance = network_custom_client_init,
    .uninit_instance = network_custom_client_uninit,
    .reset_notify = network_custom_reset_notify,
    .link_status_change = network_custom_link_status_change,
    .serdes_loopback = network_custom_set_loopback,
};

static struct hnae3_client network_custom_client = {
    .name = "network_custom",
    .type = HNAE3_CLIENT_CUSTOM,
    .ops = &client_ops,
};

static int __init network_custom_init_module(void)
{
    network_custom_wq = alloc_workqueue("%s", 0, 0, "NETWORK_CUSTOM");
    if (!network_custom_wq) {
        pr_err("%s: failed to create workqueue\n", "NETWORK_CUSTOM");
        return -ENOMEM;
    }
    return hnae3_register_client(&network_custom_client);
}
module_init(network_custom_init_module);

static void __exit network_custom_exit_module(void)
{
    hnae3_unregister_client(&network_custom_client);
    destroy_workqueue(network_custom_wq);
}
module_exit(network_custom_exit_module);

MODULE_DESCRIPTION("HNS3: Network Custom Driver");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_LICENSE("GPL");
