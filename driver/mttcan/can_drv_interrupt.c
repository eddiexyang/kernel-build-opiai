/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifdef RUN_IN_AOS
#include <linux/can/error.h>
#endif
#include "can_drv_interrupt.h"
#include "can_drv_core.h"
#include "can_drv_fault_mgr.h"
#include "can_drv_log.h"

#ifdef STATIC_SKIP
    #define STATIC
#else
    #define STATIC static
#endif

#define PROT_ERR_TYPE        2
#define PROT_ERR_LOCATION    3
#define NO_ACK_DELAY_SECOND 10

#define MAX_WEIGHT          64
#define CAN_ERR_STAT        1
#define CAN_TX_ERR          6
#define CAN_RX_ERR          7

#define DELAY2_MS  (2 * 1000 * 1000)
#define DELAY5_MS  (5 * 1000 * 1000)

#define RESTART_ADVANCE_MS 11

void mttcan_clear_all_interrupt_flags(const struct net_device *ndev)
{
    struct mttcan_priv *priv = netdev_priv(ndev);

    /* flags of register ir and ttir are cleared by writing '1' */
    mttcan_write_reg(&priv->addr, REG_IR, REG_IR_MASK);
    mttcan_write_reg(&priv->addr, REG_TTIR, REG_TTIR_MASK);
}

STATIC void mttcan_ack_interrputs(const struct net_device *ndev, u32 ack)
{
    const struct mttcan_priv *priv = netdev_priv(ndev);

    mttcan_write_reg(&priv->addr, REG_IR, ack);
    mb();
}

int mttcan_enable_interrupts(const struct net_device *ndev, u32 intr_sel)
{
    const struct mttcan_priv *priv = netdev_priv(ndev);
    u32 ie = mttcan_read_reg(&priv->addr, REG_IE);
    u32 ile = mttcan_read_reg(&priv->addr, REG_ILE);
    u32 txie;

    if (intr_sel & INT_LINE_0) {
        ie |= INT_LINE0_SEL;
        ile |= INT_LINE_0;
    }

    if (intr_sel & INT_LINE_1) {
        ie |= INT_LINE1_SEL;
        ile |= INT_LINE_1;
    }

    /* enable tx interrupt */
    txie = GENMASK(priv->elmt_param[ELMT_TXB].num - 1, 0);
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_TXBTIE, txie) != 0) {
        mttcan_err("write REG_TXBTIE failed.\n");
        return -1;
    }
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_TXBCIE, txie) != 0) {
        mttcan_err("write REG_TXBCIE failed.\n");
        return -1;
    }

    /* enable interrupt */
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_IE, ie) != 0) {
        mttcan_err("enable interrupt failed.\n");
        return -1;
    }

    /* enable interrupt line */
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_ILE, ile) != 0) {
        mttcan_err("enable interrupt line failed.\n");
        return -1;
    }
    return 0;
}

void mttcan_disable_interrupts(const struct net_device *ndev, u32 intr_sel)
{
    const struct mttcan_priv *priv = netdev_priv(ndev);

    u32 ie = mttcan_read_reg(&priv->addr, REG_IE);
    u32 ile = mttcan_read_reg(&priv->addr, REG_ILE);

    if (intr_sel & INT_LINE_0) {
        ie &= ~INT_LINE0_SEL;
        ile &= ~INT_LINE_0;
    }

    if (intr_sel & INT_LINE_1) {
        ie &= ~INT_LINE1_SEL;
        ile &= ~INT_LINE_1;
    }

    /* disable interrupt line */
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_ILE, ile) != 0) {
        mttcan_err("disable interrupt line failed.\n");
    }

    /* disable interrupt */
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_IE, ie) != 0) {
        mttcan_err("disable interrupt line failed.\n");
    }
}

STATIC void mttcan_busoff_report_fault(struct mttcan_priv *priv, enum event_direct event)
{
    switch (event) {
        case EVENT_DIRECT_ASSERT:
            if (!priv->busoff_warning && (priv->busoff_counter >= priv->busoff_cfg_param.busoff_report_threshold)) {
                mttcan_print_runlog(priv, CAN_LOG_BUSOFF_WARNING,
                    "CAN state changes to bus off for %d consecutive times.\n",
                    priv->busoff_cfg_param.busoff_report_threshold);
                (void)mttcan_report_fault_event(priv, DMS_EVENT_BUS_OFF_BASE,
                    EVENT_ERR_LEVEL_ERROR, EVENT_DIRECT_ASSERT);
                priv->busoff_warning = true;
            }
            break;
        case EVENT_DIRECT_DEASSERT:
            if (priv->busoff_warning) {
                mttcan_print_runlog(priv, CAN_LOG_BUSOFF_WARNING,
                    "CAN transmits message succ, clear bus off alarm.\n");
                (void)mttcan_report_fault_event(priv, DMS_EVENT_BUS_OFF_BASE,
                    EVENT_ERR_LEVEL_ERROR, EVENT_DIRECT_DEASSERT);
                priv->busoff_warning = false;
            }
            break;
        default:
            mttcan_err("mttcan busoff report fault faild, event_direct is %d.\n", event);
            break;
    }
}

STATIC void mttcan_set_restart_delay(struct net_device *ndev)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    int restart_ts;

    if (priv->busoff_counter < priv->busoff_cfg_param.busoff_quick_times) {
        priv->busoff_restart_ms = (int)(priv->busoff_cfg_param.busoff_quick);
        restart_ts = (int)(priv->busoff_cfg_param.busoff_quick) - RESTART_ADVANCE_MS;
    } else {
        priv->busoff_restart_ms = (int)(priv->busoff_cfg_param.busoff_slow);
        restart_ts = (int)(priv->busoff_cfg_param.busoff_slow) - RESTART_ADVANCE_MS;
    }

    if (restart_ts <= 0) {
        restart_ts = 1;
    }
    priv->can.restart_ms = restart_ts;
    priv->busoff_counter++;
}

STATIC void mttcan_change_state_to_bus_off(struct net_device *ndev, struct can_frame *cf)
{
    struct mttcan_priv *priv = netdev_priv(ndev);

    if (netif_carrier_ok(ndev)) {
        netif_stop_queue(ndev);
        priv->can.can_stats.bus_off++;
        mttcan_disable_interrupts(ndev, INT_LINE_ALL);
        cf->can_id |= CAN_ERR_BUSOFF;
        mttcan_set_restart_delay(ndev);
        can_bus_off(ndev);
        priv->busoff_ts = ktime_to_ns(ktime_get_real());
    } else {
        mttcan_info("%s is in busoff, no need to repeat processing\n", ndev->name);
    }
}

STATIC void mttcan_change_state_to_other(struct net_device *ndev, struct can_frame *cf)
{
    u32 ecr;
    struct can_berr_counter bec;
    struct mttcan_priv *priv = netdev_priv(ndev);

    ecr = mttcan_read_reg(&priv->addr, REG_ECR); /* Error Counter Register */
    bec.rxerr = (ecr & ECR_REC_MASK) >> ECR_REC_SHIFT;
    bec.txerr = (ecr & ECR_TEC_MASK) >> ECR_TEC_SHIFT;
    if (priv->can.state == CAN_STATE_ERROR_PASSIVE) {
        priv->can.can_stats.error_passive++;
        /* Receive Error Passive */
        if (ecr & ECR_RP_BIT) {
            cf->data[CAN_ERR_STAT] |= CAN_ERR_CRTL_RX_PASSIVE;
        }

        if (bec.txerr > CAN_ERR_PASSIVE_CNT) {
            cf->data[CAN_ERR_STAT] |= CAN_ERR_CRTL_TX_PASSIVE;
        }
    } else {
        priv->can.can_stats.error_warning++;
        cf->data[CAN_ERR_STAT] = (bec.txerr > bec.rxerr) ? CAN_ERR_CRTL_TX_WARNING : CAN_ERR_CRTL_RX_WARNING;
    }

    cf->can_id |= CAN_ERR_CRTL;
    cf->data[CAN_TX_ERR] = bec.txerr;
    cf->data[CAN_RX_ERR] = bec.rxerr;
}

STATIC void mttcan_change_state(struct net_device *ndev, enum can_state new_error_type)
{
    int ret;
    struct sk_buff *skb = NULL;
    struct can_frame *cf = NULL;
    struct mttcan_priv *priv = netdev_priv(ndev);
    struct net_device_stats *ndev_stats = &ndev->stats;

    if (unlikely(new_error_type == priv->can.state)) {
        mttcan_warn("%s: can state did not change.\n", ndev->name);
        return;
    }

    skb = alloc_can_err_skb(ndev, &cf);
    if (unlikely(skb == NULL)) {
        mttcan_err("%s: failed to alloc can skb of state change.\n", ndev->name);
        return;
    }

    priv->can.state = new_error_type;
    if (new_error_type == CAN_STATE_BUS_OFF) {
        mttcan_change_state_to_bus_off(ndev, cf);
    } else {
        mttcan_change_state_to_other(ndev, cf);
    }

    ndev_stats->rx_bytes += CAN_ERR_DLC;
    ndev_stats->rx_packets++;
#ifdef RUN_IN_AOS
    ret = kfifo_in(&priv->tx_skb_kfifo, &skb, sizeof(struct sk_buff*)); //lint !e1058
#else
    ret = kfifo_put(&priv->tx_skb_kfifo, (const struct sk_buff *)skb); //lint !e1058
#endif
    if (ret == 0) {
        priv->status_cnt.tx_kfifo_full_cnt++;
    }
}

STATIC void mttcan_handle_state_error(struct net_device *ndev, u32 irq, u32 protocol_status)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    u32 psr = protocol_status;

    if (irq & IR_EW_BIT) {
        if (psr & PSR_EW_BIT) {
            mttcan_change_state(ndev, CAN_STATE_ERROR_WARNING);
            mttcan_print_runlog(priv, CAN_LOG_ENTER_WARNING_STATUS, "enter error warning.\n");
        } else {
            priv->can.state = CAN_STATE_ERROR_ACTIVE;
            mttcan_print_runlog(priv, CAN_LOG_ENTER_ACTIVE_STATUS, "exit error warning, enter error active.\n");
        }
    }

    if (irq & IR_EP_BIT) {
        if (psr & PSR_EP_BIT) {
            mttcan_change_state(ndev, CAN_STATE_ERROR_PASSIVE);
            mttcan_print_runlog(priv, CAN_LOG_ENTER_PASSIVE_STATUS, "enter error passive.\n");
        } else if (psr & PSR_EW_BIT) {
            mttcan_change_state(ndev, CAN_STATE_ERROR_WARNING);
            mttcan_print_runlog(priv, CAN_LOG_ENTER_WARNING_STATUS, "exit error passive, enter error warning.\n");
        } else {
            priv->can.state = CAN_STATE_ERROR_ACTIVE;
            mttcan_print_runlog(priv, CAN_LOG_ENTER_ACTIVE_STATUS, "exit error passive, enter error active.\n");
        }
    }

    if ((irq & IR_BO_BIT) && (psr & PSR_BO_BIT)) {
        mttcan_change_state(ndev, CAN_STATE_BUS_OFF);
        mttcan_busoff_report_fault(priv, EVENT_DIRECT_ASSERT);
        mttcan_print_runlog(priv, CAN_LOG_ENTER_BUSOFF_STATUS, "enter bus off state.\n");
        mttcan_warn("%s will restart %d ms later.\n", ndev->name, priv->busoff_restart_ms);
    }
}

STATIC struct can_frame *mttcan_alloc_err_can_frame(struct net_device *ndev, u32 irq, u32 protocol_status,
    struct sk_buff **skb, u32 *err_type)
{
    struct can_frame *cf = NULL;

    if (irq & IR_PEA_BIT) {
        *err_type = (protocol_status & PSR_LEC_MASK) >> PSR_LEC_SHIFT;
    } else if (irq & IR_PED_BIT) {
        *err_type = (protocol_status & PSR_DLEC_MASK) >> PSR_DLEC_SHIFT;
    }

    if ((*err_type == LEC_TYPE_NO_ERROR) || (*err_type == LEC_TYPE_NO_CHANGE)) {
        mttcan_err("%s: No bus error occurred.\n", ndev->name);
        return NULL;
    }

    *skb = alloc_can_err_skb(ndev, &cf);
    if (unlikely(*skb == NULL)) {
        mttcan_err("%s: alloc_can_err_skb failed.\n", ndev->name);
        return NULL;
    }

    /* error class (mask) in can_id */
    cf->can_id |= CAN_ERR_PROT | CAN_ERR_BUSERROR;

    /* error in CAN protocol (type) / data[2] */
    cf->data[PROT_ERR_TYPE] = CAN_ERR_PROT_UNSPEC;

    /* error in CAN protocol (location) / data[3] */
    cf->data[PROT_ERR_LOCATION] = CAN_ERR_PROT_LOC_UNSPEC;

    return cf;
}

STATIC void mttcan_package_err_frame(u32 err_type, struct can_frame *cf,
    struct mttcan_priv *priv, struct net_device_stats *ndev_stats)
{
    switch (err_type) {
    case LEC_TYPE_STUFF_ERROR:
        cf->data[PROT_ERR_TYPE] |= CAN_ERR_PROT_STUFF;
        priv->status_cnt.lec_type_stuff_err_cnt++;
        ndev_stats->rx_errors++;
        break;
    case LEC_TYPE_FORM_ERROR:
        cf->data[PROT_ERR_TYPE] |= CAN_ERR_PROT_FORM;
        priv->status_cnt.lec_type_form_err_cnt++;
        ndev_stats->rx_errors++;
        break;
    case LEC_TYPE_ACK_ERROR:
        cf->data[PROT_ERR_LOCATION] |= (CAN_ERR_PROT_LOC_ACK | CAN_ERR_PROT_LOC_ACK_DEL);
        priv->status_cnt.lec_type_ack_err_cnt++;
        ndev_stats->tx_errors++;
        break;
    case LEC_TYPE_BIT1_ERROR:
        cf->data[PROT_ERR_TYPE] |= CAN_ERR_PROT_BIT1;
        priv->status_cnt.lec_type_bit1_err_cnt++;
        ndev_stats->tx_errors++;
        break;
    case LEC_TYPE_BIT0_ERROR:
        cf->data[PROT_ERR_TYPE] |= CAN_ERR_PROT_BIT0;
        priv->status_cnt.lec_type_bit0_err_cnt++;
        ndev_stats->tx_errors++;
        break;
    case LEC_TYPE_CRC_ERROR:
        cf->data[PROT_ERR_LOCATION] |= (CAN_ERR_PROT_LOC_CRC_SEQ | CAN_ERR_PROT_LOC_CRC_DEL);
        priv->status_cnt.lec_type_crc_err_cnt++;
        ndev_stats->rx_errors++;
        break;
    default:
        priv->status_cnt.unknow_err_cnt++;
        break;
    }
}

STATIC void mttcan_report_bus_error(struct net_device *ndev, u32 irq, u32 protocol_status)
{
    int ret;
    u32 err_type = LEC_TYPE_NO_ERROR; /* Last Error Code */
    struct mttcan_priv *priv = netdev_priv(ndev);
    struct net_device_stats *ndev_stats = &ndev->stats;
    struct sk_buff *skb = NULL;

    struct can_frame *cf = mttcan_alloc_err_can_frame(ndev, irq, protocol_status, &skb, &err_type);
    if (unlikely(cf == NULL)) {
        mttcan_err("%s:mttcan_alloc_err_can_frame failed.\n", ndev->name);
        return;
    }
    mttcan_package_err_frame(err_type, cf, priv, ndev_stats);
    ndev_stats->rx_packets++;
    ndev_stats->rx_bytes += CAN_ERR_DLC;
    priv->can.can_stats.bus_error++;
#ifdef RUN_IN_AOS
    ret = kfifo_in(&priv->tx_skb_kfifo, &skb, sizeof(struct sk_buff*)); //lint !e1058
#else
    ret = kfifo_put(&priv->tx_skb_kfifo, (const struct sk_buff *)skb); //lint !e1058
#endif
    if (ret == 0) {
        priv->status_cnt.tx_kfifo_full_cnt++;
    }
}

STATIC void mttcan_handle_fifo_lost(struct net_device *ndev, u32 irq)
{
    int ret;
    struct net_device_stats *ndev_stats = &ndev->stats;
    struct sk_buff *skb = NULL;
    struct can_frame *cf = NULL;
    struct mttcan_priv *priv = netdev_priv(ndev);

    skb = alloc_can_err_skb(ndev, &cf);
    if (unlikely(skb == NULL)) {
        mttcan_err("%s:mttcan_handle_fifo_lost failed.\n", ndev->name);
        return;
    }

    cf->can_id |= CAN_ERR_CRTL;
    cf->data[1] = CAN_ERR_CRTL_RX_OVERFLOW;
    ndev_stats->rx_errors++;
    ndev_stats->rx_over_errors++;
#ifdef RUN_IN_AOS
    ret = kfifo_in(&priv->rx_skb_kfifo, &skb, sizeof(struct sk_buff*)); //lint !e1058
#else
    ret = kfifo_put(&priv->rx_skb_kfifo, (const struct sk_buff *)skb); //lint !e1058
#endif
    if (ret == 0) {
        priv->status_cnt.rx_kfifo_full_cnt++;
    }
}

STATIC void mttcan_handle_tx_event_fifo_lost(struct net_device *ndev)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    mttcan_err("%s: tx event fifo lost\n", ndev->name);
    priv->status_cnt.tx_fifo_lost_err_cnt++;
}

STATIC int read_rx_msg(struct net_device *ndev, int weight, u32 irq)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    int packets = 0;

    if (irq & INT_FIFO_LOST) {
        mttcan_handle_fifo_lost(ndev, irq);
        irq |= (IR_RF0N_BIT | IR_RF1N_BIT | IR_DRX_BIT);
        priv->status_cnt.rx_fifo_lost_err_cnt++;
    }

    if (irq & (IR_RF0F_BIT | IR_RF0W_BIT | IR_RF0N_BIT)) {
        packets += mttcan_read_rx_fifo0(ndev, weight);
    }

    if (irq & (IR_RF1F_BIT | IR_RF1W_BIT | IR_RF1N_BIT)) {
        packets += mttcan_read_rx_fifo1(ndev, weight - packets);
    }

    if (irq & IR_DRX_BIT) {
        packets += mttcan_read_rx_buffer(ndev, weight - packets);
    }

    return packets;
}

STATIC void check_skb_delay(struct sk_buff* skb, struct mttcan_priv *priv)
{
    u32 index;
    s64 ns_delta = ktime_to_ns(ktime_get_real()) - ktime_to_ns(skb->tstamp);

    if (DELAY2_MS <= ns_delta && ns_delta < DELAY5_MS) {
        priv->status_cnt.rx_tasklet_delay2ms_cnt++;
    } else if (ns_delta >= DELAY5_MS) {
        priv->status_cnt.rx_tasklet_delay5ms_cnt++;
    }

    index = priv->can_perf_record[CAN_RECV].index;
    if (ns_delta >= priv->can_perf_record[CAN_RECV].latency_threshold) {
        if (index >= PERF_RECORD_SIZE) {
            index = 0;
        }
        priv->can_perf_record[CAN_RECV].data[index].start_time = ktime_to_ns(skb->tstamp);
        priv->can_perf_record[CAN_RECV].data[index].latency = ns_delta;
        index++;
    }
    priv->can_perf_record[CAN_RECV].index = index;

    skb->tstamp = (ktime_t)0;
    return;
}

void mttcan_rx_push_skb(unsigned long priv_ptr)
{
    struct sk_buff* skb = NULL;
    struct mttcan_priv *priv;
    priv = (struct mttcan_priv *)(uintptr_t)priv_ptr;
#ifdef RUN_IN_AOS
    while (kfifo_out(&priv->rx_skb_kfifo, &skb, sizeof(struct sk_buff*))) {
#else
    while (kfifo_get(&priv->rx_skb_kfifo, &skb)) {
#endif
        check_skb_delay(skb, priv);
        netif_receive_skb(skb);
    }
}

void mttcan_tx_push_skb(unsigned long priv_ptr)
{
    struct sk_buff* skb = NULL;
    struct mttcan_priv *priv;
    priv = (struct mttcan_priv *)(uintptr_t)priv_ptr;
#ifdef RUN_IN_AOS
    while (kfifo_out(&priv->tx_skb_kfifo, &skb, sizeof(struct sk_buff*))) {
#else
    while (kfifo_get(&priv->tx_skb_kfifo, &skb)) {
#endif
        netif_receive_skb(skb);
    }
}

/* interrupt service routine 0 for rx and tx interrupts */
irqreturn_t mttcan_isr0(int irq0, void *dev)
{
    struct net_device *ndev = (struct net_device *)dev;
    struct mttcan_priv *priv = netdev_priv(ndev);
    u32 ir;
    u32 irq;

    ir = mttcan_read_reg(&priv->addr, REG_IR);
    irq = ir & INT_LINE0_SEL;
    mttcan_ack_interrputs(ndev, irq);
    read_rx_msg(ndev, MAX_WEIGHT, irq);

    /* hi schedule or normal schedule */
    if (priv->cfg.tasklet_schedule_type == TASKLET_HI_SCHEDULE) {
        tasklet_hi_schedule(&priv->rx_tasklet);
    } else {
        tasklet_schedule(&priv->rx_tasklet);
    }
    return IRQ_HANDLED;
}

/* interrupt service routine 1 for others interrupts */
irqreturn_t mttcan_isr1(int irq1, void *dev)
{
    u32 ir; /* Interrupt Register */
    u32 psr; /* Protocol Status Register */
    u32 status_ir1;
    struct net_device *ndev = (struct net_device *)dev;
    struct mttcan_priv *priv = netdev_priv(ndev);

    ir = mttcan_read_reg(&priv->addr, REG_IR);
    psr = mttcan_read_reg(&priv->addr, REG_PSR);
    status_ir1 = ir & INT_LINE1_SEL;
    mttcan_ack_interrputs(ndev, status_ir1);

    if (status_ir1 & INT_ERR_STATUS) {
        mttcan_handle_state_error(ndev, status_ir1, psr);
    }

    if ((status_ir1 & INT_ERR_PROTO) && (priv->can.ctrlmode & CAN_CTRLMODE_BERR_REPORTING)) {
        mttcan_report_bus_error(ndev, status_ir1, psr);
    }

    if (status_ir1 & INT_TX) {
        if (status_ir1 & (IR_TEFW_BIT | IR_TEFN_BIT | IR_TEFL_BIT | IR_TEFF_BIT)) {
            mttcan_tx_event(ndev);
        }

        if (status_ir1 & IR_TC_BIT) {
            priv->busoff_counter = 0;
            mttcan_busoff_report_fault(priv, EVENT_DIRECT_DEASSERT);
            mttcan_tx_complete(ndev);
        }

        if (status_ir1 & IR_TEFL_BIT) {
            mttcan_handle_tx_event_fifo_lost(ndev);
        }
    }

    mttcan_tx_cancel_finish(ndev);
    /* hi schedule or normal schedule */
    if (priv->cfg.tasklet_schedule_type == TASKLET_HI_SCHEDULE) {
        tasklet_hi_schedule(&priv->tx_tasklet);
    } else {
        tasklet_schedule(&priv->tx_tasklet);
    }
    if (netif_queue_stopped(ndev)) {
        if (priv->can.state != CAN_STATE_BUS_OFF) {
            netif_wake_queue(ndev);
        }
#ifdef SUPPORT_MTTCAN_FAULT_RECOVERY
        priv->last_netif_stop_time = 0;
#endif
    }
    return IRQ_HANDLED;
}
