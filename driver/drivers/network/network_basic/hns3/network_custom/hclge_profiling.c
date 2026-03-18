/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* Description:
* Author: huawei
* Create: 2019-10-15
*/

#include <linux/acpi.h>
#include <linux/etherdevice.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/crash_dump.h>
#include <linux/platform_device.h>

#include "securec.h"
#include "hclge_main.h"
#include "hns3_enet.h"
#include "hclge_profiling.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

#define HCLGE_MAC_STATS_FIELD_OFF(f) (offsetof(struct hclge_mac_stats, f))
#define NANOSECOND_TO_SECOND 1000000000
#define ONE_SECOND 1
#define HCLGE_SAMPLE_TIME_INTERVAL 9000000
#define MIN_TIME_INTERVAL 0

struct timespec64 g_profiling_nic_sample_time = {0};
struct timespec64 g_profiling_roce_sample_time = {0};

STATIC u64 hclge_sample_mac_stats(const struct hclge_dev *hdev, const char *str)
{
    struct hclge_comm_stats_str *mac_stats_string;
    size_t array_size = 0;
    size_t i;

    mac_stats_string = hclge_get_mac_stats_array(&array_size);
    for (i = 0; i < array_size; i++) {
        if (strncmp(mac_stats_string[i].desc, str, strlen(str)) == 0) {
            return HCLGE_STATS_READ(&hdev->mac_stats, mac_stats_string[i].offset);
        }
    }
    dev_warn(&hdev->pdev->dev, "can not find %s in the mac_stats_string\n", str);
    return 0;
}

STATIC void hclge_sample_net_info(struct hclge_dev *hdev, struct hclge_sample_stats_info *stats_info)
{
    stats_info->bandwidth = (u64)hdev->hw.mac.speed;
    stats_info->rxPackets = hclge_sample_mac_stats(hdev, "mac_rx_total_pkt_num");
    stats_info->rxBytes = hclge_sample_mac_stats(hdev, "mac_rx_total_oct_num");
    stats_info->txPackets = hclge_sample_mac_stats(hdev, "mac_tx_total_pkt_num");
    stats_info->txBytes = hclge_sample_mac_stats(hdev, "mac_tx_total_oct_num");
    stats_info->rxErrors = hclge_sample_mac_stats(hdev, "mac_rx_fcs_err_pkt_num");
    stats_info->txErrors = hclge_sample_mac_stats(hdev, "mac_tx_err_all_pkt_num");
    stats_info->txDropped = 0;
    stats_info->rxDropped = 0;
}

STATIC int hclge_sample_format_info(const struct hclge_dev *hdev, struct hclge_sample_stats_info sample_stats_first,
    struct hclge_sample_stats_info sample_stats_second, char *buf, int len)
{
    int ret;
    u64 period_nanosecond, rxPacketSpeed, rxByteSpeed, txPacketSpeed, txByteSpeed;

    period_nanosecond =
        ((sample_stats_second.sample_time.tv_sec - sample_stats_first.sample_time.tv_sec) * NANOSECOND_TO_SECOND +
        (sample_stats_second.sample_time.tv_nsec - sample_stats_first.sample_time.tv_nsec));
    if (period_nanosecond == 0) {
        dev_err(&hdev->pdev->dev, "sample period time is zero\n");
        return 0;
    }

    rxPacketSpeed =
        (sample_stats_second.rxPackets - sample_stats_first.rxPackets) * NANOSECOND_TO_SECOND / period_nanosecond;
    rxByteSpeed = (sample_stats_second.rxBytes - sample_stats_first.rxBytes) * NANOSECOND_TO_SECOND / period_nanosecond;
    txPacketSpeed =
        (sample_stats_second.txPackets - sample_stats_first.txPackets) * NANOSECOND_TO_SECOND / period_nanosecond;
    txByteSpeed = (sample_stats_second.txBytes - sample_stats_first.txBytes) * NANOSECOND_TO_SECOND / period_nanosecond;
    ret = snprintf_s(buf, len, len - 1,
        "%llu:%09lu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %u\n",
        sample_stats_second.sample_time.tv_sec, sample_stats_second.sample_time.tv_nsec, sample_stats_second.bandwidth,
        rxPacketSpeed, rxByteSpeed, sample_stats_second.rxPackets, sample_stats_second.rxBytes,
        sample_stats_second.rxErrors, sample_stats_second.rxDropped, txPacketSpeed, txByteSpeed,
        sample_stats_second.txPackets, sample_stats_second.txBytes, sample_stats_second.txErrors,
        sample_stats_second.txDropped, 0);
    if (ret <= 0) {
        dev_err(&hdev->pdev->dev, "sprintf_err (%s) line(%d)\n", __func__, __LINE__);
        return ret;
    }

    return strlen(buf);
}

STATIC bool hclge_is_avai_time_interval(const struct timespec64 *last_time, const struct timespec64 *this_time)
{
    if ((last_time == NULL) ||
        (this_time == NULL)) {
        pr_err("pointer is NULL! func %s line %d\n", __func__, __LINE__);
        return false;
    }

    if (this_time->tv_sec - last_time->tv_sec > ONE_SECOND) {
        return true;
    } else if (((this_time->tv_sec - last_time->tv_sec) * NANOSECOND_TO_SECOND +
        (this_time->tv_nsec - last_time->tv_nsec)) > HCLGE_SAMPLE_TIME_INTERVAL) {
        return true;
    }

    return false;
}

STATIC int hclge_profling_para_init(unsigned int dev_id, const char *buf, struct hclge_dev **hdev,
    struct hnae3_handle **hnae, struct net_device **netdev)
{
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hclge_vport *vport = NULL;
    struct pci_dev *pdev = NULL;
    struct hns3_nic_priv *priv = NULL;

    if ((buf == NULL) ||
        (hdev == NULL) ||
        (hnae == NULL) ||
        (netdev == NULL)) {
        pr_err("pointer is NULL! func %s line %d\n", __func__, __LINE__);
        return -EINVAL;
    }

    if (dev_id > MAX_DEV_ID) {
        pr_err("hns3: dev_id[%d] is not effective, max valid dev_id[%d]\n", dev_id, MAX_DEV_ID);
        return -EINVAL;
    }

    pdev = hclge_get_pci_dev(dev_id);
    if (pdev == NULL) {
        pr_err("hns3: get pci_dev from dev_id[%d] failed\n", dev_id);
        return -EINVAL;
    }

    ae_dev = pci_get_drvdata(pdev);
    *hdev = ae_dev->priv;
    vport = (*hdev)->vport;
    *hnae = &vport->nic;
    priv = (*hnae)->priv;
    dev_hold(priv->netdev);
    *netdev = priv->netdev;
    dev_put(priv->netdev);

    return 0;
}

STATIC void hclge_nic_sample_info(const struct hclge_dev *hdev, struct hclge_sample_stats_info *stats_info,
    const struct rtnl_link_stats64 *rtnl_stats)
{
    if ((hdev == NULL) ||
        (stats_info == NULL) ||
        (rtnl_stats == NULL)) {
        pr_err("pointer is NULL! func %s line %d\n", __func__, __LINE__);
        return;
    }

    stats_info->bandwidth = (u64)hdev->hw.mac.speed;
    stats_info->rxPackets = rtnl_stats->rx_packets;
    stats_info->rxBytes = rtnl_stats->rx_bytes;
    stats_info->txPackets = rtnl_stats->tx_packets;
    stats_info->txBytes = rtnl_stats->tx_bytes;
    stats_info->rxErrors = rtnl_stats->rx_errors;
    stats_info->txErrors = rtnl_stats->tx_errors;
    stats_info->txDropped = rtnl_stats->tx_dropped;
    stats_info->rxDropped = rtnl_stats->rx_dropped;
}

int hns_roce_prof_sample(struct prof_peri_para para)
{
    struct hclge_sample_stats_info sample_stats_first, sample_stats_second;
    struct hclge_dev *hdev = NULL;
    struct hnae3_handle *hnae = NULL;
    struct hns3_nic_priv *priv = NULL;
    struct net_device *netdev = NULL;
    struct timespec64 sample_time;
    unsigned int dev_id = para.device_id;
    void *buf = para.buff;
    int len = para.buff_len;
    int ret;
    if (dev_id > MAX_DEV_ID || buf == NULL) {
        pr_err("hns3: invalid param, func %s line %d, dev_id[%u], buf[%pK] \n", __func__, __LINE__,
            dev_id, buf);
        return -EINVAL;
    }
    ktime_get_raw_ts64(&sample_time);
    if (hclge_is_avai_time_interval(&g_profiling_roce_sample_time, &sample_time) == FALSE) {
        pr_err("hns3: profiling sample time interval is too short\n");
        return -EINVAL;
    }
    ktime_get_raw_ts64(&g_profiling_roce_sample_time);
    ret = hclge_profling_para_init(dev_id, buf, &hdev, &hnae, &netdev);
    if (ret || len <= 0) {
        pr_err("hns3: get pci_dev from dev_id:%d failed, len %d\n", dev_id, len);
        return -EINVAL;
    }
    priv = hnae->priv;
    if (!test_bit(HNS3_NIC_STATE_INITED, &priv->state) || hns3_nic_resetting(netdev)) {
        dev_err(&hdev->pdev->dev, "dev resetting!priv->state:%lu\n", priv->state);
        return -EINVAL;
    }

    if (hnae->ae_algo->ops->update_stats == NULL) {
        dev_err(&hdev->pdev->dev, "update_stats is NULL,could not get any statistics\n");
        return -EINVAL;
    }

    ktime_get_raw_ts64(&sample_stats_first.sample_time);
    hnae->ae_algo->ops->update_stats(hnae, &netdev->stats);
    hclge_sample_net_info(hdev, &sample_stats_first);
    msleep(MIN_TIME_INTERVAL);

    ktime_get_raw_ts64(&sample_stats_second.sample_time);
    hnae->ae_algo->ops->update_stats(hnae, &netdev->stats);
    hclge_sample_net_info(hdev, &sample_stats_second);
    return hclge_sample_format_info(hdev, sample_stats_first, sample_stats_second, (char *)buf, len);
}
EXPORT_SYMBOL(hns_roce_prof_sample);

int higmac_sample_fun(struct prof_peri_para para)
{
    struct hclge_sample_stats_info sample_stats_first, sample_stats_second;
    struct rtnl_link_stats64 stats;
    struct hclge_dev *hdev = NULL;
    struct hnae3_handle *hnae = NULL;
    struct hns3_nic_priv *priv = NULL;
    struct net_device *netdev = NULL;
    struct timespec64 sample_time;
    unsigned int dev_id = para.device_id;
    void *buf = para.buff;
    int len = para.buff_len;
    int ret;
    if (dev_id > MAX_DEV_ID || buf == NULL) {
        pr_err("hns3: invalid param, func %s line %d, dev_id[%u], buf[%pK] \n", __func__, __LINE__,
            dev_id, buf);
        return -EINVAL;
    }
    ktime_get_raw_ts64(&sample_time);
    if (hclge_is_avai_time_interval(&g_profiling_nic_sample_time, &sample_time) == FALSE) {
        pr_err("hns3: profiling sample time interval is too short\n");
        return -EINVAL;
    }
    ktime_get_raw_ts64(&g_profiling_nic_sample_time);

    ret = hclge_profling_para_init(dev_id, buf, &hdev, &hnae, &netdev);
    if (ret || len <= 0) {
        pr_err("hns3: get pci_dev from dev_id:%d failed, len %d\n", dev_id, len);
        return -EINVAL;
    }

    priv = hnae->priv;
    if (test_bit(HNS3_NIC_STATE_DOWN, &priv->state)) {
        dev_err(&hdev->pdev->dev, "priv->state:%lu\n", priv->state);
        return -EINVAL;
    }

    if (netdev->netdev_ops->ndo_get_stats64 == NULL) {
        dev_err(&hdev->pdev->dev, "ndo_get_stats64 is NULL,could not get any statistics\n");
        return -EINVAL;
    }

    ktime_get_raw_ts64(&sample_stats_first.sample_time);
    netdev->netdev_ops->ndo_get_stats64(netdev, &stats);
    hclge_nic_sample_info(hdev, &sample_stats_first, &stats);
    msleep(MIN_TIME_INTERVAL);

    ktime_get_raw_ts64(&sample_stats_second.sample_time);
    netdev->netdev_ops->ndo_get_stats64(netdev, &stats);
    hclge_nic_sample_info(hdev, &sample_stats_second, &stats);

    return hclge_sample_format_info(hdev, sample_stats_first, sample_stats_second, (char *)buf, len);
}
EXPORT_SYMBOL(higmac_sample_fun);
