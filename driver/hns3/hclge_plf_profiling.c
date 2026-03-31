/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
* Description: profiling
* Author: huawei
* Create: 2022-12-7
*/

#include <linux/netdevice.h>
#include "hclge_plf_main.h"
#include "hclge_plf_profiling.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

#define HCLGE_STATS_READ(p, offset) (*((u64 *)((u8 *)(p) + (offset))))
#define HCLGE_MAC_STATS_FIELD_OFF(f) (offsetof(struct hclge_mac_stats, f))
#define NANOSECOND_TO_SECOND 1000000000
#define ONE_SECOND 1
#define HCLGE_SAMPLE_TIME_INTERVAL 9000000
#define MIN_TIME_INTERVAL 0
#define TEMP_BUF_SIZE 300

STATIC struct timespec64 g_profiling_nic_sample_time;

STATIC int hclge_sample_format_info(const struct hclge_plf_dev *hdev, struct hclge_sample_stats_info sample_stats_first,
    struct hclge_sample_stats_info sample_stats_second, char *buf, int len)
{
    u64 tx_packet_speed, tx_byte_speed;
    u64 rx_packet_speed, rx_byte_speed;
    u64 period_nanosecond;
    int ret;

    if (hdev == NULL) {
        pr_err("hdev is NULL\n");
        return 0;
    }

    period_nanosecond =
        ((sample_stats_second.sample_time.tv_sec - sample_stats_first.sample_time.tv_sec) * NANOSECOND_TO_SECOND +
        (sample_stats_second.sample_time.tv_nsec - sample_stats_first.sample_time.tv_nsec));
    if (period_nanosecond == 0) {
        dev_err(&hdev->pdev->dev, "sample period time is zero\n");
        return 0;
    }

    rx_packet_speed =
        (sample_stats_second.rx_packets - sample_stats_first.rx_packets) * NANOSECOND_TO_SECOND / period_nanosecond;
    rx_byte_speed = (sample_stats_second.rx_bytes - sample_stats_first.rx_bytes) *
                    NANOSECOND_TO_SECOND / period_nanosecond;
    tx_packet_speed =
        (sample_stats_second.tx_packets - sample_stats_first.tx_packets) * NANOSECOND_TO_SECOND / period_nanosecond;
    tx_byte_speed = (sample_stats_second.tx_bytes - sample_stats_first.tx_bytes) *
                    NANOSECOND_TO_SECOND / period_nanosecond;
    ret = snprintf_s(buf, len, len - 1,
        "%llu:%09lu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %u\n",
        sample_stats_second.sample_time.tv_sec, sample_stats_second.sample_time.tv_nsec, sample_stats_second.bandwidth,
        rx_packet_speed, rx_byte_speed, sample_stats_second.rx_packets, sample_stats_second.rx_bytes,
        sample_stats_second.rx_errors, sample_stats_second.rx_dropped, tx_packet_speed, tx_byte_speed,
        sample_stats_second.tx_packets, sample_stats_second.tx_bytes, sample_stats_second.tx_errors,
        sample_stats_second.tx_dropped, hdev->id);
    if (ret <= 0) {
        dev_err(&hdev->pdev->dev, "sprintf_err: ret = %d \n", ret);
        return ret;
    }

    return (int)strlen(buf);
}

STATIC bool hclge_is_avai_time_interval(const struct timespec64 *last_time, const struct timespec64 *this_time)
{
    if ((last_time == NULL) ||
        (this_time == NULL)) {
        pr_err("[hclge_plf]: pointer is NULL! last_time[%pK], this_time[%pK]\n", last_time, this_time);
        return false;
    }

    if (this_time->tv_sec - last_time->tv_sec > ONE_SECOND) {
        return true;
    }
    if (((this_time->tv_sec - last_time->tv_sec) * NANOSECOND_TO_SECOND +
        (this_time->tv_nsec - last_time->tv_nsec)) > HCLGE_SAMPLE_TIME_INTERVAL) {
        return true;
    }

    return false;
}

STATIC int hclge_plf_profling_para_init(u8 port_id, const char *buf, struct hclge_plf_dev **hdev,
    struct hnae3_handle **hnae, struct net_device **netdev)
{
    struct hclge_plf_vport *vport = NULL;
    struct platform_device *pdev = NULL;
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hns3_nic_priv *priv = NULL;

    if ((buf == NULL) ||
        (hdev == NULL) ||
        (hnae == NULL) ||
        (netdev == NULL)) {
        pr_err("[hclge_plf]: pointer is NULL! buf[%pK], hdev[%pK], hnae[%pK], netdev[%pK]\n",
               buf, hdev, hnae, netdev);
        return -EINVAL;
    }

    pdev = hclge_get_platform_device(port_id);
    if (pdev == NULL) {
        pr_err("hns3: get platform_device from port_id[%d] failed\n", port_id);
        return -EINVAL;
    }

    ae_dev = platform_get_drvdata(pdev);
    *hdev = ae_dev->priv;
    vport = (*hdev)->vport;
    *hnae = &vport->nic;
    priv = (*hnae)->priv;
#ifndef DEFINE_HNS_LLT
    dev_hold(priv->netdev);
    *netdev = priv->netdev;
    dev_put(priv->netdev);
#endif
    return 0;
}

STATIC void hclge_nic_sample_info(const struct hclge_plf_dev *hdev, struct hclge_sample_stats_info *stats_info,
    const struct rtnl_link_stats64 *rtnl_stats)
{
    if ((hdev == NULL) ||
        (stats_info == NULL) ||
        (rtnl_stats == NULL)) {
        pr_err("[hclge_plf]: pointer is NULL! hdev[%pK], stats_info[%pK], rtnl_stats[%pK]\n",
               hdev, stats_info, rtnl_stats);
        return;
    }

    stats_info->bandwidth = hdev->hw.mac.speed;
    stats_info->rx_packets = rtnl_stats->rx_packets;
    stats_info->rx_bytes = rtnl_stats->rx_bytes;
    stats_info->tx_packets = rtnl_stats->tx_packets;
    stats_info->tx_bytes = rtnl_stats->tx_bytes;
    stats_info->rx_errors = rtnl_stats->rx_errors;
    stats_info->tx_errors = rtnl_stats->tx_errors;
    stats_info->tx_dropped = rtnl_stats->tx_dropped;
    stats_info->rx_dropped = rtnl_stats->rx_dropped;
}

int higmac_sample_fun(struct prof_peri_para para)
{
    struct hclge_sample_stats_info sample_stats_first, sample_stats_second;
    unsigned int dev_id = para.device_id;
    struct hclge_plf_dev *hdev = NULL;
    struct hns3_nic_priv *priv = NULL;
    struct hnae3_handle *hnae = NULL;
    struct net_device *netdev = NULL;
    u32 id_map = hclge_get_id_map();
    struct rtnl_link_stats64 stats;
    struct timespec64 sample_time;
    char temp_buf[TEMP_BUF_SIZE];
    int len = para.buff_len;
    void *buf = para.buff;
    u8 port_id;
    int ret;

    if (dev_id > MAX_DEV_ID || buf == NULL || len <= 0) {
        pr_err("[hclge_plf]: invalid params, dev_id[%u], buf[%pK], buf_len[%d] \n", dev_id, buf, len);
        return -EINVAL;
    }

    ktime_get_raw_ts64(&sample_time);
    if (hclge_is_avai_time_interval(&g_profiling_nic_sample_time, &sample_time) == FALSE) {
        pr_err("hns3: profiling sample time interval is too short\n");
        return -EINVAL;
    }
    ktime_get_raw_ts64(&g_profiling_nic_sample_time);

    for (port_id = 0; id_map != 0; port_id++, id_map >>= 1) {
        if ((id_map & 0x1) == 0x0) {
            continue;
        }

        ret = hclge_plf_profling_para_init(port_id, buf, &hdev, &hnae, &netdev);
        if (ret < 0) {
            pr_err("hns3: get platform_device from port_id:%d failed, len %d\n", port_id, len);
            continue;
        }

        priv = hnae->priv;
        if (test_bit(HNS3_NIC_STATE_DOWN, &priv->state)) {
            dev_err(&hdev->pdev->dev, "priv->state:%lu\n", priv->state);
            continue;
        }
#ifndef DEFINE_HNS_LLT
        if (netdev->netdev_ops->ndo_get_stats64 == NULL) {
            dev_err(&hdev->pdev->dev, "ndo_get_stats64 is NULL,could not get any statistics\n");
            continue;
        }
#endif
        ktime_get_raw_ts64(&sample_stats_first.sample_time);
#ifndef DEFINE_HNS_LLT
        netdev->netdev_ops->ndo_get_stats64(netdev, &stats);
#endif
        hclge_nic_sample_info(hdev, &sample_stats_first, &stats);

        msleep(MIN_TIME_INTERVAL);

        ktime_get_raw_ts64(&sample_stats_second.sample_time);
#ifndef DEFINE_HNS_LLT
        netdev->netdev_ops->ndo_get_stats64(netdev, &stats);
#endif
        hclge_nic_sample_info(hdev, &sample_stats_second, &stats);

        hclge_sample_format_info(hdev, sample_stats_first, sample_stats_second, (char *)temp_buf, TEMP_BUF_SIZE);

        ret = strncat_s(buf, len, temp_buf, len - 1);
        if (ret != 0) {
            dev_err(&hdev->pdev->dev, "strcat_err: ret = %d \n", ret);
            return ret;
        }
    }
    return (int)strlen(buf);
}
EXPORT_SYMBOL(higmac_sample_fun);
