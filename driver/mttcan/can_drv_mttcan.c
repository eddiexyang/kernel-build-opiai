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

#include "can_drv_mttcan.h"

#include <linux/securec.h>
#ifdef RUN_IN_AOS
#include <linux/can/error.h>
#endif
#include "can_drv_fault_mgr.h"
#include "can_drv_core.h"
#include "can_drv_log.h"

#ifdef STATIC_SKIP
    #define STATIC
#else
    #define STATIC static
#endif

#define NDAT_BITS 32
#define WRITE_REGISTER_COUNT 3
#define CAN_FRAME_MAX_EQUAL_BITS (5)
#define BITS_OF_2BYTES          (16)
#define TX_TS_LOST_REPORT_THRESHOLD 3

enum data_size {
    DS8 = 0,
    DS12,
    DS16,
    DS20,
    DS24,
    DS32,
    DS48,
    DS64
};

struct reg_attr {
    const char *name;
    u32 offset;
};

static const struct reg_attr mttcan_ctrl_reg[] = {
    {"REG_DBTP  ", REG_DBTP},
    {"REG_CCCR  ", REG_CCCR},
    {"REG_NBTP  ", REG_NBTP},
    {"REG_TDCR  ", REG_TDCR},
    {"REG_IR    ", REG_IR},
    {"REG_IE    ", REG_IE},
    {"REG_ILS   ", REG_ILS},
    {"REG_ILE   ", REG_ILE},
    {"REG_GFC   ", REG_GFC},
    {"REG_SIDFC ", REG_SIDFC},
    {"REG_XIDFC ", REG_XIDFC},
    {"REG_XIDAM ", REG_XIDAM},
    {"REG_HPMS  ", REG_HPMS},
    {"REG_NDAT1 ", REG_NDAT1},
    {"REG_NDAT2 ", REG_NDAT2},
    {"REG_RXF0C ", REG_RXF0C},
    {"REG_RXF0S ", REG_RXF0S},
    {"REG_RXF0A ", REG_RXF0A},
    {"REG_RXBC  ", REG_RXBC},
    {"REG_RXF1C ", REG_RXF1C},
    {"REG_RXF1S ", REG_RXF1S},
    {"REG_RXF1A ", REG_RXF1A},
    {"REG_RXESC ", REG_RXESC},
    {"REG_TXBC  ", REG_TXBC},
    {"REG_TXFQS ", REG_TXFQS},
    {"REG_TXESC ", REG_TXESC},
    {"REG_TXBRP ", REG_TXBRP},
    {"REG_TXBAR ", REG_TXBAR},
    {"REG_TXBCR ", REG_TXBCR},
    {"REG_TXBTO ", REG_TXBTO},
    {"REG_TXBCF ", REG_TXBCF},
    {"REG_TXBTIE", REG_TXBTIE},
    {"REG_TXBCIE", REG_TXBCIE},
    {"REG_TXEFC ", REG_TXEFC},
    {"REG_TXEFS ", REG_TXEFS},
    {"REG_TXEFA ", REG_TXEFA}
};

void mttcan_dump_reg(const struct net_device *ndev)
{
    u32 i;
    const struct mttcan_priv *priv = netdev_priv(ndev);

    for (i = 0; i < ARRAY_SIZE(mttcan_ctrl_reg); i++) {
        mttcan_info("%s:%-12s: [0x%08x]\n",
            ndev->name,
            mttcan_ctrl_reg[i].name,
            mttcan_read_reg(&priv->addr, (int)(mttcan_ctrl_reg[i].offset)));
    }

    mttcan_info("%s:tx_obj = 0x%08llx,\n", ndev->name, priv->tx_obj);
}
int write_reg_and_report_fault(const struct net_device *ndev, void __iomem *addr_base, int addr_offset, u32 val)
{
    int try_times = WRITE_REGISTER_COUNT;
    while (try_times--) {
        writel(val, addr_base + addr_offset);
        if ((u32)readl(addr_base + addr_offset) == val) {
            break;
        }
    }
    if (try_times < 0) {
        mttcan_err("%s: write register failed for %d times.\n", ndev->name, WRITE_REGISTER_COUNT);
        mttcan_dump_reg(ndev);
        return -EFAULT;
    }
    return 0;
}

void mttcan_print_revision(const struct net_device *ndev)
{
    u32 release, step, substep, year, mon, day;
    u32 crel; /* Core Release Register */
    const struct mttcan_priv *priv = netdev_priv(ndev);

    crel = mttcan_read_reg(&priv->addr, REG_CREL);
    release = (crel & CREL_REL_MASK) >> CREL_REL_SHIFT;
    step = (crel & CREL_STEP_MASK) >> CREL_STEP_SHIFT;
    substep = (crel & CREL_SUBSTEP_MASK) >> CREL_SUBSTEP_SHIFT;
    year = (crel & CREL_YEAR_MASK) >> CREL_YEAR_SHIFT;
    mon = (crel & CREL_MON_MASK) >> CREL_MON_SHIFT;
    day = (crel & CREL_DAY_MASK) >> CREL_DAY_SHIFT;

    mttcan_info("%s: Revision %u.%u.%u, Date 201%1.1x/%2.2x/%2.2x\n", ndev->name, release, step, substep, year, mon,
        day);
}

int mttcan_set_loopback_mode(const struct net_device *ndev, bool enable)
{
    u32 cccr; /* CAN Core Control Register */
    u32 test; /* Test Register */
    const struct mttcan_priv *priv = NULL;

    if (unlikely(ndev == NULL)) {
        mttcan_err("Invalid input.\n");
        return -1;
    }

    priv = netdev_priv(ndev);
    cccr = mttcan_read_reg(&priv->addr, REG_CCCR);
    cccr |= CCCR_TEST_BIT;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_CCCR, cccr) != 0) {
        mttcan_err("write CCCR_TEST_BIT failed.\n");
        return -1;
    }

    test = mttcan_read_reg(&priv->addr, REG_TEST);
    if (enable) {
        test |= TEST_LBCK_BIT;
        mttcan_write_reg(&priv->addr, REG_TEST, test);
        mttcan_info("%s: enable loopback mode.\n", ndev->name);
    } else {
        /* TSET.LBCK is auto reset when CCCR.TEST is reset. */
        cccr &= ~CCCR_TEST_BIT;
        if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_CCCR, cccr) != 0) {
            mttcan_err("write CCCR_TEST_BIT failed.\n");
            return -1;
        }
    }
    return 0;
}

int mttcan_set_bus_monitor_mode(const struct net_device *ndev, bool enable)
{
    u32 cccr; /* CAN Core Control Register */
    const struct mttcan_priv *priv = NULL;

    if (unlikely(ndev == NULL)) {
        mttcan_err("mttcan_set_bus_monitor_mode Invalid input.\n");
        return -1;
    }

    priv = netdev_priv(ndev);
    cccr = mttcan_read_reg(&priv->addr, REG_CCCR);
    if (enable) {
        cccr |= CCCR_MON_BIT;
        if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_CCCR, cccr) != 0) {
            mttcan_err("enable bus monitor mode failed.\n");
            return -1;
        }
        mttcan_info("%s: enable bus monitor mode.\n", ndev->name);
    } else {
        cccr &= ~CCCR_MON_BIT;
        if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_CCCR, cccr) != 0) {
            mttcan_err("disable bus monitor mode failed.\n");
            return -1;
        }
    }
    return 0;
}

int mttcan_set_auto_retrans(const struct net_device *ndev, bool enable)
{
    u32 cccr; /* CAN Core Control Register */
    const struct mttcan_priv *priv = NULL;

    if (unlikely(ndev == NULL)) {
        mttcan_err("mttcan_set_auto_retrans Invalid input.\n");
        return -1;
    }

    priv = netdev_priv(ndev);
    cccr = mttcan_read_reg(&priv->addr, REG_CCCR);
    if (enable) {
        cccr &= ~CCCR_DAR_BIT;
        mttcan_info("%s:enable auto-retransfer mode.\n", ndev->name);
    } else {
        cccr |= CCCR_DAR_BIT;
        mttcan_info("%s:enable one-shot mode.\n", ndev->name);
    }

    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_CCCR, cccr) != 0) {
        mttcan_err("write REG_CCCR failed.\n");
        return -1;
    }
    return 0;
}

int mttcan_enable_canfd(const struct net_device *ndev, bool enable)
{
    u32 cccr;
    const struct mttcan_priv *priv = NULL;

    if (unlikely(ndev == NULL)) {
        mttcan_err("Invalid input.\n");
        return -1;
    }

    priv = netdev_priv(ndev);
    cccr = mttcan_read_reg(&priv->addr, REG_CCCR);
    if (enable) {
        cccr |= CCCR_FDOE_BIT | CCCR_BRSE_BIT;
        mttcan_info("%s: can fd is enabled.\n", ndev->name);
    } else {
        cccr &= ~(CCCR_FDOE_BIT | CCCR_BRSE_BIT);
        mttcan_info("%s: can fd is disabled.\n", ndev->name);
    }

    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_CCCR, cccr) != 0) {
        mttcan_err("write REG_CCCR failed.\n");
        return -1;
    }
    return 0;
}

STATIC int mttcan_set_bit_cccr_init(const struct net_device *ndev)
{
    u32 cccr; /* CAN Core Control Register */
    int timeout = SET_INIT_TIMEOUT_US;
    const struct mttcan_priv *priv = netdev_priv(ndev);

    cccr = mttcan_read_reg(&priv->addr, REG_CCCR);
    if (cccr & CCCR_INIT_BIT) {
        return 0;
    }

    cccr |= CCCR_INIT_BIT;
    mttcan_write_reg(&priv->addr, REG_CCCR, cccr);
    do {
        udelay(1);
        cccr = mttcan_read_reg(&priv->addr, REG_CCCR);
        timeout--;
        if (timeout <= 0) {
            mttcan_err("%s: set cccr_init timeout.\n", ndev->name);
            return -ETIMEDOUT;
        }
    } while (!(cccr & CCCR_INIT_BIT));

    return 0;
}

STATIC int mttcan_set_bit_cccr_cce(const struct net_device *ndev)
{
    u32 cccr; /* CAN Core Control Register */
    const struct mttcan_priv *priv = netdev_priv(ndev);

    cccr = mttcan_read_reg(&priv->addr, REG_CCCR);
    if (cccr & CCCR_CCE_BIT) {
        return 0;
    }

    cccr |= CCCR_CCE_BIT;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_CCCR, cccr) != 0) {
        mttcan_err("write REG_CCCR failed.\n");
        return -1;
    }
    return 0;
}

STATIC int mttcan_clear_bit_cccr_init(const struct net_device *ndev)
{
    u32 cccr; /* CAN Core Control Register */
    int timeout = SET_INIT_TIMEOUT_US;
    const struct mttcan_priv *priv = netdev_priv(ndev);

    cccr = mttcan_read_reg(&priv->addr, REG_CCCR);
    if (!(cccr & CCCR_INIT_BIT)) {
        return 0;
    }

    cccr &= ~CCCR_INIT_BIT;
    mttcan_write_reg(&priv->addr, REG_CCCR, cccr);

    do {
        udelay(1);
        cccr = mttcan_read_reg(&priv->addr, REG_CCCR);
        timeout--;
        if (timeout <= 0) {
            mttcan_err("%s: set cccr_init timeout.\n", ndev->name);
            return -ETIMEDOUT;
        }
    } while (cccr & CCCR_INIT_BIT);

    return 0;
}


int mttcan_set_init_mode(const struct net_device *ndev)
{
    int err = -1;

    if (unlikely(ndev == NULL)) {
        mttcan_err("Invalid input.\n");
        return err;
    }

    /* set cccr.init, start initialization */
    err = mttcan_set_bit_cccr_init(ndev);
    if (err < 0) {
        mttcan_err("%s: mttcan_set_bit_cccr_init failed.\n", ndev->name);
        return err;
    }

    /* set cccr.cce, configuration chang enable */
    if (mttcan_set_bit_cccr_cce(ndev) != 0) {
        mttcan_err("%s: mttcan_set_bit_cccr_cce failed.\n", ndev->name);
        return -1;
    }
    mttcan_info("%s: set init mode succeed.\n", ndev->name);

    return 0;
}

int mttcan_set_normal_mode(const struct net_device *ndev)
{
    int err;

    if (unlikely(ndev == NULL)) {
        mttcan_err("Invalid input.\n");
        return -1;
    }

    /* clear cccr.init */
    err = mttcan_clear_bit_cccr_init(ndev);
    if (err < 0) {
        mttcan_err("%s: failed to clear bit cccr.init\n", ndev->name);
        return err;
    }

    /* cccr.cce is automatically clear when cccr.init is clear */
    mttcan_info("%s: set normal mode succeed.\n", ndev->name);

    return 0;
}

int mttcan_set_bittiming(struct net_device *ndev)
{
    u32 nbtp = 0; /* Nominal Bit Timing & Prescaler Register */
    const struct mttcan_priv *priv = NULL;
    const struct can_bittiming *bittiming = NULL;

    if (unlikely(ndev == NULL)) {
        mttcan_err("Invalid input.\n");
        return -1;
    }

    priv = netdev_priv(ndev);
    bittiming = &priv->can.bittiming;
    if (bittiming->bitrate > MAX_BITRATE) {
        mttcan_err("Invalid bitrate, the value must be smaller than %d.\n", MAX_BITRATE);
        return -EINVAL;
    }
    nbtp |= ((u64)(bittiming->sjw - 1) << NBTP_NSJW_SHIFT) & NBTP_NSJW_MASK;
    nbtp |= ((u64)(bittiming->brp - 1) << NBTP_NBRP_SHIFT) & NBTP_NBRP_MASK;
    nbtp |= ((u64)(bittiming->prop_seg + bittiming->phase_seg1 - 1) << NBTP_NTSEG1_SHIFT) & NBTP_NTSEG1_MASK;
    nbtp |= ((u64)(bittiming->phase_seg2 - 1) << NBTP_NTSEG2_SHIFT) & NBTP_NTSEG2_MASK;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_NBTP, nbtp) != 0) {
        mttcan_err("%s:set bitrate failed.\n", ndev->name);
        return -1;
    }

    mttcan_info("%s:set bitrate as %u succeed.\n", ndev->name, bittiming->bitrate);
    return 0;
}

STATIC int mttcan_set_tx_delay_compensation(const struct mttcan_priv *priv, struct net_device *ndev)
{
    u32 ssp;
    u32 tdco;
    u32 tdcf;
    u32 tdcr_val;

    /* Manual configuration TDC */
    if (priv->tdc_flag == TDC_FLAG_ENABLE) {
        tdco = priv->tdco;
        tdcf = priv->tdcf;
        tdcr_val = (tdco << TDCR_TDCO_SHIFT) | tdcf;
        if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_TDCR, tdcr_val) != 0) {
            mttcan_err("write REG_TDCR failed.\n");
            return -1;
        }
        return 0;
    }

    /* Adaptive configuration TDC */
    /* Use the same value of secondary sampling point
     * as the data sampling point
     */
    ssp = priv->can.fd.data_bittiming.sample_point;

    /* Equation based on Bosch's M_CAN User Manual's
     * Transmitter Delay Compensation Section
     */
    tdco = (priv->can.clock.freq / CAN_CLOCK_FREQ_DGREE) *
        ssp / priv->can.fd.data_bittiming.bitrate;

    /* Max valid TDCO value is 127 */
    if (tdco > MAX_TDCR_TDCO) {
        mttcan_warn("%s:TDCO value of %u is beyond maximum. Using maximum possible value\n",
            ndev->name, tdco);
        tdco = MAX_TDCR_TDCO;
    }

    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_TDCR, tdco << TDCR_TDCO_SHIFT) != 0) {
        mttcan_err("write REG_TDCR failed.\n");
        return -1;
    }

    return 0;
}

STATIC bool need_config_tdc(unsigned char tdc_flag, const struct can_bittiming *data_bittiming)
{
    if ((tdc_flag == TDC_FLAG_AUTOADAPTER) && (data_bittiming->bitrate > MIN_TDCO_DBITRATE)) {
        return true;
    } else if (tdc_flag == TDC_FLAG_ENABLE) {
        return true;
    } else {
        return false;
    }
}

int mttcan_set_data_bittiming(struct net_device *ndev)
{
    u32 dbtp = 0; /* Data Bit Timing & Prescaler Register */
    const struct mttcan_priv *priv = NULL;
    const struct can_bittiming *data_bittiming = NULL;

    if (unlikely(ndev == NULL)) {
        mttcan_err("Invalid input.\n");
        return -1;
    }

    priv = netdev_priv(ndev);
    if (!(priv->can.ctrlmode & CAN_CTRLMODE_FD)) {
        mttcan_err("%s: No support for CAN_CTRLMODE_FD.\n", ndev->name);
        return -1;
    }

    data_bittiming = &priv->can.fd.data_bittiming;
    if (data_bittiming->bitrate > MAX_DATA_BITRATE) {
        mttcan_err("Invalid data bitrate, the value must be smaller than %d.\n", MAX_DATA_BITRATE);
        return -EINVAL;
    }

    /* TDC is only needed for bitrates beyond 2.5 MBit/s.
     * This is mentioned in the "Bit Time Requirements for CAN FD"
     * paper presented at the International CAN Conference 2013
     */
    if (need_config_tdc(priv->tdc_flag, data_bittiming)) {
        if (mttcan_set_tx_delay_compensation(priv, ndev) != 0) {
            return -1;
        }
        dbtp |= DBTP_TDC_EN;
    }

    dbtp |= ((u64)(data_bittiming->brp - 1) << DBTP_DBRP_SHIFT) & DBTP_DBRP_MASK;
    dbtp |= ((u64)(data_bittiming->prop_seg + data_bittiming->phase_seg1 - 1) << DBTP_DTSEG1_SHIFT) & DBTP_DTSEG1_MASK;
    dbtp |= ((u64)(data_bittiming->phase_seg2 - 1) << DBTP_DTSEG2_SHIFT) & DBTP_DTSEG2_MASK;
    dbtp |= ((u64)(data_bittiming->sjw - 1) << DBTP_DSJW_SHIFT) & DBTP_DSJW_MASK;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_DBTP, dbtp) != 0) {
        mttcan_err("%s:set data bitrate failed.\n", ndev->name);
        return -1;
    }

    mttcan_info("%s:set data bitrate as %u succeed.\n", ndev->name, data_bittiming->bitrate);
    return 0;
}

STATIC int mttcan_use_tsu(const struct net_device *ndev)
{
    const struct mttcan_priv *priv = netdev_priv(ndev);
    u32 cccr;

    cccr = mttcan_read_reg(&priv->addr, REG_CCCR);
    cccr |= CCCR_UTSU_BIT;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_CCCR, cccr) != 0) {
        mttcan_err("enable cccr.utsu failed.\n");
        return -1;
    }
    mttcan_info("%s: cccr.utsu is enabled.\n", ndev->name);
    return 0;
}

STATIC int mttcan_reg_gfc_init(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    const struct mttcan_priv *priv = netdev_priv(ndev);

    /* Global Filter Configuration */
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_GFC, cfg->global_filter_cfg) != 0) {
        mttcan_err("configure gfc = 0x%08x failed.\n", cfg->global_filter_cfg);
        return -1;
    }
    mttcan_info("%s: configure gfc = 0x%08x\n", ndev->name, cfg->global_filter_cfg);
    return 0;
}

STATIC int mttcan_reg_xidam_init(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    const struct mttcan_priv *priv = netdev_priv(ndev);

    /* Extended ID AND Mask */
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_XIDAM, cfg->extend_id_and_mask) != 0) {
        mttcan_err("configure xidam = 0x%08x failed.\n", cfg->extend_id_and_mask);
        return -1;
    }
    mttcan_info("%s: configure xidam = 0x%08x\n", ndev->name, cfg->extend_id_and_mask);
    return 0;
}

STATIC int mttcan_reg_sidfc_init(const struct net_device *ndev)
{
    u32 sidfc = 0; /* Standard ID Filter Configuration */
    const struct mttcan_priv *priv = netdev_priv(ndev);
    u32 elmt_num = priv->elmt_param[ELMT_SIDF].num;
    u32 start_addr = priv->elmt_param[ELMT_SIDF].off / BYTES_OF_WORD;

    sidfc |= ((u64)elmt_num << SIDFC_LSS_SHIFT) & SIDFC_LSS_MASK;
    sidfc |= ((u64)start_addr << SIDFC_FLSSA_SHIFT) & SIDFC_FLSSA_MASK;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_SIDFC, sidfc) != 0) {
        mttcan_err("configure sidfc = 0x%08x failed.\n", sidfc);
        return -1;
    }
    mttcan_info("%s: configure sidfc = 0x%08x\n", ndev->name, sidfc);
    return 0;
}

STATIC int mttcan_reg_xidfc_init(const struct net_device *ndev)
{
    u32 xidfc = 0; /* Extended ID Filter Configuration */
    const struct mttcan_priv *priv = netdev_priv(ndev);
    u32 elmt_num = priv->elmt_param[ELMT_XIDF].num;
    u32 start_addr = priv->elmt_param[ELMT_XIDF].off / BYTES_OF_WORD;

    xidfc |= ((u64)elmt_num << XIDFC_LES_SHIFT) & XIDFC_LES_MASK;
    xidfc |= ((u64)start_addr << XIDFC_FLESA_SHIFT) & XIDFC_FLESA_MASK;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_XIDFC, xidfc) != 0) {
        mttcan_err("configure xidfc = 0x%08x failed.\n", xidfc);
        return -1;
    }
    mttcan_info("%s: configure xidfc = 0x%08x\n", ndev->name, xidfc);
    return 0;
}

STATIC int init_reg_rxf0c(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    u32 rxf0c = 0; /* Rx FIFO 0 Configuration */
    u32 f0wm = cfg->watermark[WATERMARK_RXF0];
    const struct mttcan_priv *priv = netdev_priv(ndev);
    u32 elmt_num = priv->elmt_param[ELMT_RXF0].num;
    u32 start_addr = priv->elmt_param[ELMT_RXF0].off / BYTES_OF_WORD;

    if (cfg->mode[MODE_RXF0]) {
        rxf0c |= RXF0C_F0OM_BIT;
    }

    rxf0c |= ((u64)f0wm << RXF0C_F0WM_SHIFT) & RXF0C_F0WM_MASK;
    rxf0c |= ((u64)elmt_num << RXF0C_F0S_SHIFT) & RXF0C_F0S_MASK;
    rxf0c |= ((u64)start_addr << RXF0C_F0SA_SHIFT) & RXF0C_F0SA_MASK;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_RXF0C, rxf0c) != 0) {
        mttcan_err("configure rxf0c = 0x%08x failed.\n", rxf0c);
        return -1;
    }
    mttcan_info("%s: configure rxf0c = 0x%08x\n", ndev->name, rxf0c);
    return 0;
}

STATIC int init_reg_rxf1c(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    u32 rxf1c = 0; /* Rx FIFO 1 Configuration */
    u32 f1wm = cfg->watermark[WATERMARK_RXF1];
    const struct mttcan_priv *priv = netdev_priv(ndev);
    u32 elmt_num = priv->elmt_param[ELMT_RXF1].num;
    u32 start_addr = priv->elmt_param[ELMT_RXF1].off / BYTES_OF_WORD;

    if (cfg->mode[MODE_RXF1]) {
        rxf1c |= RXF0C_F0OM_BIT;
    }

    rxf1c |= ((u64)f1wm << RXF1C_F1WM_SHIFT) & RXF1C_F1WM_MASK;
    rxf1c |= ((u64)elmt_num << RXF1C_F1S_SHIFT) & RXF1C_F1S_MASK;
    rxf1c |= ((u64)start_addr << RXF1C_F1SA_SHIFT) & RXF1C_F1SA_MASK;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_RXF1C, rxf1c) != 0) {
        mttcan_err("configure rxf1c = 0x%08x failed.\n", rxf1c);
        return -1;
    }
    mttcan_info("%s: configure rxf1c = 0x%08x\n", ndev->name, rxf1c);
    return 0;
}

STATIC int init_reg_rxbc(const struct net_device *ndev)
{
    u32 rxbc = 0; /* Rx Buffer Configuration */
    const struct mttcan_priv *priv = netdev_priv(ndev);
    u32 start_addr = priv->elmt_param[ELMT_RXB].off / BYTES_OF_WORD;

    rxbc |= ((u64)start_addr << RXBC_RBSA_SHIFT) & RXBC_RBSA_MASK;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_RXBC, rxbc) != 0) {
        mttcan_err("configure rxbc = 0x%08x failed.\n", rxbc);
        return -1;
    }
    mttcan_info("%s: configure rxbc = 0x%08x\n", ndev->name, rxbc);
    return 0;
}

STATIC enum data_size encode_data_size(u32 dsize)
{
    switch (dsize) {
    case DATA_SZIE_8:
        return DS8;
    case DATA_SZIE_12:
        return DS12;
    case DATA_SZIE_16:
        return DS16;
    case DATA_SZIE_20:
        return DS20;
    case DATA_SZIE_24:
        return DS24;
    case DATA_SZIE_32:
        return DS32;
    case DATA_SZIE_48:
        return DS48;
    case DATA_SZIE_64:
        return DS64;
    default:
        return DS8;
    }
}

STATIC int init_reg_rxesc(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    u32 rxesc = 0; /* Rx Buffer / FIFO Element Size Configuration */
    const struct mttcan_priv *priv = netdev_priv(ndev);

    u32 rxb_ds = encode_data_size(cfg->dsize[DSIZE_RXB]);   /* Rx Buffer Data Field Size */
    u32 txf1_ds = encode_data_size(cfg->dsize[DSIZE_RXF1]); /* Rx FIFO 1 Data Field Size */
    u32 txf0_ds = encode_data_size(cfg->dsize[DSIZE_RXF0]); /* Rx FIFO 0 Data Field Size */

    rxesc |= ((u64)rxb_ds << RXESC_RBDS_SHIFT) & RXESC_RBDS_MASK;
    rxesc |= ((u64)txf1_ds << RXESC_F1DS_SHIFT) & RXESC_F1DS_MASK;
    rxesc |= ((u64)txf0_ds << RXESC_F0DS_SHIFT) & RXESC_F0DS_MASK;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_RXESC, rxesc) != 0) {
        mttcan_err("configure rxesc = 0x%08x failed.\n", rxesc);
        return -1;
    }
    mttcan_info("%s: configure rxesc = 0x%08x\n", ndev->name, rxesc);
    return 0;
}

STATIC int init_reg_txbc(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    u32 txbc = 0;
    const struct mttcan_priv *priv = netdev_priv(ndev);
    u32 start_addr = priv->elmt_param[ELMT_TXB].off / BYTES_OF_WORD;

    if (cfg->mode[MODE_TXFQ]) {
        txbc |= TXBC_TFQM_BIT;
    }

    txbc |= ((u64)cfg->tx_element_num[TX_ELMT_FQ] << TXBC_TFQS_SHIFT) & TXBC_TFQS_MASK;
    txbc |= ((u64)cfg->tx_element_num[TX_ELMT_BUF] << TXBC_NDTB_SHIFT) & TXBC_NDTB_MASK;
    txbc |= ((u64)start_addr << TXBC_TBSA_SHIFT) & TXBC_TBSA_MASK;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_TXBC, txbc) != 0) {
        mttcan_err("configure txbc = 0x%08x failed.\n", txbc);
        return -1;
    }
    mttcan_info("%s: configure txbc = 0x%08x\n", ndev->name, txbc);
    return 0;
}

STATIC int init_reg_txesc(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    u32 txesc = 0;
    const struct mttcan_priv *priv = netdev_priv(ndev);
    u32 tbds = encode_data_size(cfg->dsize[DSIZE_TXB]); /* Tx Buffer Data Field Size */

    txesc |= ((u64)tbds << TXESC_TBDS_SHIFT) & TXESC_TBDS_MASK;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_TXESC, txesc) != 0) {
        mttcan_err("configure txesc = 0x%08x failed.\n", txesc);
        return -1;
    }
    mttcan_info("%s: configure txesc = 0x%08x\n", ndev->name, txesc);
    return 0;
}

STATIC int init_reg_txbtie(const struct net_device *ndev)
{
    u32 txbtie = 0; /* Tx Buffer Transmission Interrupt Enable */
    const struct mttcan_priv *priv = netdev_priv(ndev);
    u32 elmt_num = priv->elmt_param[ELMT_TXB].num;

    txbtie |= (1 << elmt_num) - 1;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_TXBTIE, txbtie) != 0) {
        mttcan_err("configure txbtie = 0x%08x failed.\n", txbtie);
        return -1;
    }
    mttcan_info("%s: configure txbtie = 0x%08x\n", ndev->name, txbtie);
    return 0;
}

STATIC int init_reg_txbcie(const struct net_device *ndev)
{
    u32 txbcie = 0; /* Tx Buffer Cancellation Finished Interrupt Enable */
    const struct mttcan_priv *priv = netdev_priv(ndev);
    u32 elmt_num = priv->elmt_param[ELMT_TXB].num;

    txbcie |= (1 << elmt_num) - 1;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_TXBCIE, txbcie) != 0) {
        mttcan_err("configure txbcie = 0x%08x failed.\n", txbcie);
        return -1;
    }
    mttcan_info("%s: configure txbcie = 0x%08x\n", ndev->name, txbcie);
    return 0;
}

STATIC int init_reg_txefc(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    u32 txefc = 0;
    const struct mttcan_priv *priv = netdev_priv(ndev);
    u32 efwm = cfg->watermark[WATERMARK_TXEF];
    u32 elmt_num = priv->elmt_param[ELMT_TXEF].num;
    u32 start_addr = priv->elmt_param[ELMT_TXEF].off / BYTES_OF_WORD;

    txefc |= ((u64)efwm << TXEFC_EFWM_SHIFT) & TXEFC_EFWM_MASK;
    txefc |= ((u64)elmt_num << TXEFC_EFS_SHIFT) & TXEFC_EFS_MASK;
    txefc |= ((u64)start_addr << TXEFC_EFSA_SHIFT) & TXEFC_EFSA_MASK;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_TXEFC, txefc) != 0) {
        mttcan_err("configure txefc = 0x%08x failed.\n", txefc);
        return -1;
    }
    mttcan_info("%s: configure txefc = 0x%08x\n", ndev->name, txefc);
    return 0;
}

STATIC int mttcan_sid_filter_config(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    const struct mttcan_priv *priv = netdev_priv(ndev);
    void __iomem *element_addr = priv->addr.mram_base + priv->elmt_param[ELMT_SIDF].off;
    u32 i;
    u32 data_word;

    for (i = 0; i < priv->elmt_param[ELMT_SIDF].num; i++) {
        data_word = cfg->sid_filter[i];
        if (write_reg_and_report_fault(ndev, element_addr, i * ELMT_SIZE_SIDF, data_word) != 0) {
            mttcan_err("configure sid filter %03u = 0x%08x failed.\n", i, data_word);
            return -1;
        }
        mttcan_info("%s: configure sid filter %03u = 0x%08x\n", ndev->name, i, data_word);
    }
    return 0;
}

STATIC int mttcan_xid_filter_config(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    const struct mttcan_priv *priv = netdev_priv(ndev);
    void __iomem *element_addr = priv->addr.mram_base + priv->elmt_param[ELMT_XIDF].off;
    u32 i;
    u32 data_word;

    for (i = 0; i < priv->elmt_param[ELMT_XIDF].num; i++) {
        /* Configure upper 32 bits of the xid filter */
        data_word = cfg->xid_filter[(u64)i << 1];
        if (write_reg_and_report_fault(ndev, element_addr, i * ELMT_SIZE_XIDF, data_word) != 0) {
            mttcan_err("configure xid filter %03u f0 = 0x%08x failed.\n", i << 1, data_word);
            return -1;
        }
        mttcan_info("%s: configure xid filter %03u f0 = 0x%08x\n", ndev->name, i << 1, data_word);

        /* Configure lower 32 bits of the xid filter */
        data_word = cfg->xid_filter[((u64)i << 1) + 1];
        if (write_reg_and_report_fault(ndev, element_addr, i * ELMT_SIZE_XIDF + BYTES_OF_WORD, data_word) != 0) {
            mttcan_err("configure xid filter %03u f1 = 0x%08x failed.\n", (i << 1) + 1, data_word);
            return -1;
        }
        mttcan_info("%s: configure xid filter %03u f1 = 0x%08x\n", ndev->name, (i << 1) + 1, data_word);
    }
    return 0;
}

STATIC int mttcan_config_filter(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    /* Global Filter Configuration */
    if (mttcan_reg_gfc_init(ndev, cfg) != 0) {
        mttcan_err("%s: mttcan_reg_gfc_init failed.\n", ndev->name);
        return -1;
    }

    /* Extended ID AND Mask  */
    if (mttcan_reg_xidam_init(ndev, cfg) != 0) {
        mttcan_err("%s: mttcan_reg_xidam_init failed.\n", ndev->name);
        return -1;
    }

    /* Standard ID Filter Configuration */
    if (mttcan_reg_sidfc_init(ndev) != 0) {
        mttcan_err("%s: mttcan_reg_sidfc_init failed.\n", ndev->name);
        return -1;
    }

    /* Extended ID Filter Configuration */
    if (mttcan_reg_xidfc_init(ndev) != 0) {
        mttcan_err("%s: mttcan_reg_xidfc_init failed.\n", ndev->name);
        return -1;
    }

    /* Standard Message ID Filter Element */
    if (mttcan_sid_filter_config(ndev, cfg) != 0) {
        mttcan_err("%s: mttcan_sid_filter_config failed.\n", ndev->name);
        return -1;
    }

    /* Extended Message ID Filter Element */
    if (mttcan_xid_filter_config(ndev, cfg) != 0) {
        mttcan_err("%s: mttcan_xid_filter_config failed.\n", ndev->name);
        return -1;
    }
    return 0;
}

STATIC int mttcan_config_rx(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    /* Rx FIFO 0 Configuration */
    if (init_reg_rxf0c(ndev, cfg) != 0) {
        mttcan_err("%s: init_reg_rxf0c failed.\n", ndev->name);
        return -1;
    }

    /* Rx FIFO 1 Configuration */
    if (init_reg_rxf1c(ndev, cfg) != 0) {
        mttcan_err("%s: init_reg_rxf1c failed.\n", ndev->name);
        return -1;
    }

    /* Rx Buffer Configuration */
    if (init_reg_rxbc(ndev) != 0) {
        mttcan_err("%s: init_reg_rxbc failed.\n", ndev->name);
        return -1;
    }

    /* Rx Buffer / FIFO Element Size Configuration */
    if (init_reg_rxesc(ndev, cfg) != 0) {
        mttcan_err("%s: init_reg_rxesc failed.\n", ndev->name);
        return -1;
    }
    return 0;
}

STATIC int mttcan_config_tx(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    /* Tx Buffer Configuration */
    if (init_reg_txbc(ndev, cfg) != 0) {
        mttcan_err("%s: init_reg_txbc failed.\n", ndev->name);
        return -1;
    }

    /* Tx Buffer Element Size Configuration */
    if (init_reg_txesc(ndev, cfg) != 0) {
        mttcan_err("%s: init_reg_txesc failed.\n", ndev->name);
        return -1;
    }

    /* Tx Buffer Transmission Interrupt Enable */
    if (init_reg_txbtie(ndev) != 0) {
        mttcan_err("%s: init_reg_txbtie failed.\n", ndev->name);
        return -1;
    }

    /* Tx Buffer Cancellation Finished Interrupt Enable */
    if (init_reg_txbcie(ndev) != 0) {
        mttcan_err("%s: init_reg_txbcie failed.\n", ndev->name);
        return -1;
    }

    /* Tx Event FIFO Configuration */
    if (init_reg_txefc(ndev, cfg) != 0) {
        mttcan_err("%s: init_reg_txefc failed.\n", ndev->name);
        return -1;
    }
    return 0;
}

STATIC int mttcan_config_tsu(const struct net_device *ndev, const struct mttcan_config *cfg)
{
    u32 tscfg; /* Timestamp Configuration */
    u32 tbpre = cfg->ts_cnt_prescaler - 1;
    const struct mttcan_priv *priv = netdev_priv(ndev);

    tscfg = mttcan_read_reg(&priv->addr, REG_TSU_TSCFG);
    tscfg |= ((u64)tbpre << TSCFG_TBPRE_SHIFT) & TSCFG_TBPRE_MASK;
    tscfg |= TSCFG_TSUE_BIT; /* enable tsu */
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_TSU_TSCFG, tscfg) != 0) {
        mttcan_err("configure tscfg = 0x%08x failed.\n", tscfg);
        return -1;
    }
    return 0;
}

STATIC int mttcan_select_int_line(const struct net_device *ndev)
{
    const struct mttcan_priv *priv = netdev_priv(ndev);

    /* select interrupt line */
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_ILS, INT_LINE1_SEL) != 0) {
        mttcan_err("configure ils = 0x%08x failed.\n", (u32)INT_LINE1_SEL);
        return -1;
    }
    mttcan_info("%s: configure ils = 0x%08x\n", ndev->name, (u32)INT_LINE1_SEL);
    return 0;
}

int mttcan_wait_clock_stop_ack(const struct net_device *ndev)
{
    u32 cccr;
    int timeout = WAIT_CSA_TIMEOUT_MS;
    struct mttcan_priv *priv = netdev_priv(ndev);

    /* Clock Stop Request */
    cccr = mttcan_read_reg(&priv->addr, REG_CCCR);
    cccr |= CCCR_CSR_BIT;
    if (write_reg_and_report_fault(ndev, priv->addr.reg_base, REG_CCCR, cccr) != 0) {
        mttcan_err("write REG_CCCR failed.\n");
    }

    /* wait for Clock Stop Acknowledge */
    do {
        usleep_range(1000, 1000 + 100); /* wait time at most 1000 + 100 us */
        cccr = mttcan_read_reg(&priv->addr, REG_CCCR);
        timeout--;
        if (timeout <= 0) {
            mttcan_warn("%s: wait Clock Stop Acknowledge timeout.\n", ndev->name);
            return -ETIMEDOUT;
        }
    } while (!(cccr & CCCR_CSA_BIT));

    return 0;
}

int mttcan_config_controller(const struct net_device *ndev)
{
    int err;
    struct mttcan_priv *priv = netdev_priv(ndev);

    if (ndev == NULL) {
        mttcan_err("invalid input.\n");
        return -1;
    }

    /* enable init mode */
    err = mttcan_set_init_mode(ndev);
    if (err != 0) {
        mttcan_err("%s: mttcan_set_init_mode failed.\n", ndev->name);
        return err;
    }

    if (mttcan_use_tsu(ndev) != 0) {
        mttcan_err("%s: mttcan_use_tsu failed.\n", ndev->name);
        return -1;
    }
    if (mttcan_config_filter(ndev, &priv->cfg) != 0) {
        mttcan_err("%s: mttcan_config_filter failed.\n", ndev->name);
        return -1;
    }
    if (mttcan_config_rx(ndev, &priv->cfg) != 0) {
        mttcan_err("%s: mttcan_config_rx failed.\n", ndev->name);
        return -1;
    }
    if (mttcan_config_tx(ndev, &priv->cfg) != 0) {
        mttcan_err("%s: mttcan_config_tx failed.\n", ndev->name);
        return -1;
    }
    if (mttcan_config_tsu(ndev, &priv->cfg) != 0) {
        mttcan_err("%s: mttcan_config_tsu failed.\n", ndev->name);
        return -1;
    }
    if (mttcan_select_int_line(ndev) != 0) {
        mttcan_err("%s: mttcan_select_int_line failed.\n", ndev->name);
        return -1;
    }

    return 0;
}

STATIC void mttcan_handle_rx_timestamp(const struct net_device *ndev, u8 rxtsp, struct sk_buff *skb)
{
    unsigned long irq_flags;
    struct mttcan_priv *priv = netdev_priv(ndev);
    struct skb_shared_hwtstamps *hwtstamps = skb_hwtstamps(skb);
    u32 timestamp;
    u64 ns;
    int ret;

    timestamp = mttcan_read_reg(&priv->addr, REG_TSU_TS0 + ((u32)rxtsp * BYTES_OF_WORD));
    raw_spin_lock_irqsave(&priv->tc_lock, irq_flags);
    ns = timecounter_cyc2time(&priv->tc, timestamp);
    raw_spin_unlock_irqrestore(&priv->tc_lock, irq_flags);
    ret = memset_s(hwtstamps, sizeof(struct skb_shared_hwtstamps), 0, sizeof(struct skb_shared_hwtstamps));
    if (ret != EOK) {
        mttcan_warn("%s:rx timestamp not cleared.(ret=%d)\n", ndev->name, ret);
    }
    hwtstamps->hwtstamp = ns_to_ktime(ns);

    return;
}

STATIC void mttcan_read_rxb_r0(const void __iomem *element_addr, struct canfd_frame *cfd)
{
    u32 rxb_r0;

    rxb_r0 = readl(element_addr);
    if (rxb_r0 & RXB_R0_ESI) {
        cfd->flags |= CANFD_ESI;
    }

    if (rxb_r0 & RXB_R0_XTD) {
        cfd->can_id = rxb_r0 & RXB_R0_ID_MASK;
        cfd->can_id |= CAN_EFF_FLAG;
    } else {
        cfd->can_id = (rxb_r0 >> (CAN_EFF_ID_BITS - CAN_SFF_ID_BITS)) & CAN_SFF_MASK;
    }

    if (rxb_r0 & RXB_R0_RTR) {
        cfd->can_id |= CAN_RTR_FLAG;
    }
}

STATIC struct canfd_frame *mttcan_read_rxb_r1(struct net_device *ndev, const void __iomem *element_addr,
    struct sk_buff **skb)
{
    u32 rxb_r1;
    struct canfd_frame *cfd = NULL;

    /* read R1 */
    rxb_r1 = readl(element_addr + BYTES_OF_WORD);
    if (rxb_r1 & RXB_R1_FDF) {
        *skb = alloc_canfd_skb(ndev, &cfd);
        if (unlikely(*skb == NULL)) {
            mttcan_err("%s: alloc_canfd_skb failed.\n", ndev->name);
            goto exit;
        }

        cfd->len = can_fd_dlc2len((rxb_r1 & RXB_R1_DLC_MASK) >> RXB_R1_DLC_SHIFT);
        if (rxb_r1 & RXB_R1_BRS) {
            cfd->flags |= CANFD_BRS;
        }
    } else {
        *skb = alloc_can_skb(ndev, (struct can_frame **)&cfd);
        if (unlikely(*skb == NULL)) {
            mttcan_err("%s: alloc_can_skb failed.\n", ndev->name);
            goto exit;
        }

        cfd->len = can_fd_dlc2len((rxb_r1 & RXB_R1_DLC_MASK) >> RXB_R1_DLC_SHIFT);
        if (cfd->len > CAN_MAX_DLC) {
            cfd->len = CAN_MAX_DLC;
        }
    }

    if (rxb_r1 & RXB_R1_TSC_BIT) {
        mttcan_handle_rx_timestamp(ndev, rxb_r1 & RXB_R1_RXTSP_MASK, *skb);
    }

    (*skb)->tstamp = ktime_get_real();
    return cfd;

exit:
    ndev->stats.rx_dropped++;
    return NULL;
}

#ifdef SUPPORT_MTTCAN_DFX
STATIC inline u64 bit_count(u64 val)
{
#define SHIFT_3_BIT  3
    return ((val) << SHIFT_3_BIT);
}

STATIC int mttcan_calc_stuff_length(const u8 *bitmap, u32 bit_num)
{
#define CLZ_SIZE 32
#define BYTE_NEXT 1
#define BIT_STUFF 1
    const static u8 clz[CLZ_SIZE] = { /* count of leading zeros in 5 bit numbers */
        5, 4, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    u8 mask = 0x1F;
    u8 lookfor = 0;
    u32 i = 0;
    u32 stuffed = 0;

    while (i < bit_num) {
        unsigned change;
        unsigned bits;

        if (i < (bit_num - BIT_OF_BYTE)) {
            bits = ((bitmap[i / BIT_OF_BYTE] << BIT_OF_BYTE) | bitmap[i / BIT_OF_BYTE + BYTE_NEXT]) >>
                (BITS_OF_2BYTES - CAN_FRAME_MAX_EQUAL_BITS - (i % BIT_OF_BYTE));
        } else {
            bits = (bitmap[i / BIT_OF_BYTE] << BIT_OF_BYTE) >>
                (BITS_OF_2BYTES - CAN_FRAME_MAX_EQUAL_BITS - (i % BIT_OF_BYTE));
        }
        lookfor = lookfor ? 0 : mask;
        change = (bits & mask) ^ lookfor;
        if (change) {
            i += clz[change];
            mask = 0x1f;
        } else {
            /* If the fill character is the same as the next character,
             * 4 consecutive identical characters should be queried. */
            i += (mask == 0x1f) ? CAN_FRAME_MAX_EQUAL_BITS : (CAN_FRAME_MAX_EQUAL_BITS - BIT_STUFF);
            if (i <= bit_num) {
                stuffed++;
                mask = 0x1e;
            }
        }
    }
    return stuffed;
}

/**
 * calculate can data len with stuff
 */
STATIC u32 mttcan_calc_data_len(const struct canfd_frame *cfd)
{
    u32 bit_cnt = bit_count(cfd->len);
    return bit_cnt + mttcan_calc_stuff_length(cfd->data, bit_cnt);
}

STATIC u32 mttcan_calc_canid_stuff_len(const struct canfd_frame *cfd)
{
    if (cfd->can_id & CAN_EFF_FLAG) {
        return mttcan_calc_stuff_length((u8 *)(&cfd->can_id), CAN_EFF_ID_BITS);
    } else {
        return mttcan_calc_stuff_length((u8 *)(&cfd->can_id), CAN_SFF_ID_BITS);
    }
}

STATIC void mttcan_statistics_canbus(const struct sk_buff *skb, const struct canfd_frame *cfd, struct mttcan_priv *priv)
{
    if (!can_is_canfd_skb(skb)) {
        if (cfd->can_id & CAN_EFF_FLAG) {
            ++(priv->busload.stats.can_ext_packets);
        } else {
            ++(priv->busload.stats.can_std_packets);
        }
        priv->busload.stats.can_payload_bits += mttcan_calc_data_len(cfd);
        priv->busload.stats.can_payload_bits += mttcan_calc_canid_stuff_len(cfd);
    } else {
        if (cfd->can_id & CAN_EFF_FLAG) {
            if (cfd->flags & CANFD_BRS) {
                ++(priv->busload.stats.canfd_ext_brs_packets);
                priv->busload.stats.canfd_ext_brs_bits += mttcan_calc_data_len(cfd);
                priv->busload.stats.canfd_ext_normal_bits += mttcan_calc_canid_stuff_len(cfd);
            } else {
                ++(priv->busload.stats.canfd_ext_normal_packets);
                priv->busload.stats.canfd_ext_normal_bits += mttcan_calc_data_len(cfd);
                priv->busload.stats.canfd_ext_normal_bits += mttcan_calc_canid_stuff_len(cfd);
            }
        } else {
            if (cfd->flags & CANFD_BRS) {
                ++(priv->busload.stats.canfd_std_brs_packets);
                priv->busload.stats.canfd_std_brs_bits += mttcan_calc_data_len(cfd);
                priv->busload.stats.canfd_std_normal_bits += mttcan_calc_canid_stuff_len(cfd);
            } else {
                ++(priv->busload.stats.canfd_std_normal_packets);
                priv->busload.stats.canfd_std_normal_bits += mttcan_calc_data_len(cfd);
                priv->busload.stats.canfd_std_normal_bits += mttcan_calc_canid_stuff_len(cfd);
            }
        }
    }
}
#endif

STATIC void mttcan_read_rx_element(struct net_device *ndev, u32 addr_offset)
{
    int ret;
    u32 data_word;
    u8 remain_bytes;
    u8 read_bytes = 0;
    struct mttcan_priv *priv = netdev_priv(ndev);
    void __iomem *element_addr = priv->addr.mram_base + addr_offset;
    struct sk_buff *skb = NULL;
    struct canfd_frame *cfd = NULL;

    /* read R1 */
    cfd = mttcan_read_rxb_r1(ndev, element_addr, &skb);
    if (cfd == NULL) {
        mttcan_err("%s: mttcan_read_rxb_r1 failed.\n", ndev->name);
        return;
    }

    /* read R0 */
    mttcan_read_rxb_r0(element_addr, cfd);

    /* read data */
    remain_bytes = cfd->len;

    while (remain_bytes >= BYTES_OF_WORD) {
        /* read data_word from message ram */
        data_word = readl(element_addr + DATA_OFFSET + read_bytes);
        *(u32 *)(cfd->data + read_bytes) = data_word;
        read_bytes += BYTES_OF_WORD;
        remain_bytes -= BYTES_OF_WORD;
    }

    data_word = readl(element_addr + DATA_OFFSET + read_bytes);
    if (remain_bytes == 0x3) {
        cfd->data[read_bytes + 0x2] = (data_word >> DATA_BYTE2_OFF) & 0xFF;
        remain_bytes--;
    }

    if (remain_bytes == 0x2) {
        cfd->data[read_bytes + 1] = (data_word >> DATA_BYTE1_OFF) & 0xFF;
        remain_bytes--;
    }

    if (remain_bytes == 1) {
        cfd->data[read_bytes + 0] = (data_word >> DATA_BYTE0_OFF) & 0xFF;
    }

    ndev->stats.rx_packets++;
    ndev->stats.rx_bytes += cfd->len;

    // if is canfd frame
#ifdef SUPPORT_MTTCAN_DFX
    mttcan_statistics_canbus(skb, cfd, priv);
#endif
#ifdef RUN_IN_AOS
    ret = kfifo_in(&priv->rx_skb_kfifo, &skb, sizeof(struct sk_buff *)); //lint !e1058
#else
    ret = kfifo_put(&priv->rx_skb_kfifo, (const struct sk_buff *)skb); //lint !e1058
#endif
    if (ret == 0) {
        priv->status_cnt.rx_kfifo_full_cnt++;
    }
}

STATIC void mttcan_check_and_handle_ts_lost(const struct net_device *ndev, u32 txtsp)
{
    u32 tss1;
    struct mttcan_priv *priv = netdev_priv(ndev);
    tss1 = mttcan_read_reg(&priv->addr, REG_TSU_TSS1);
    if (tss1 & BIT(txtsp + TSS1_BIT_SHIFT)) {
        mttcan_err("%s: TSS1=0x%08x\n", ndev->name, tss1);
        mttcan_err("%s: timestamp lost\n", ndev->name);
        priv->tx_ts_lost_count++;
        if (priv->tx_ts_lost_count >= TX_TS_LOST_REPORT_THRESHOLD) {
            priv->tx_ts_lost_count = 0;
        }
    }
}

STATIC void mttcan_set_skb_hwtstamps(const struct net_device *ndev, struct sk_buff * skb, u64 ns)
{
    int ret;
    struct skb_shared_hwtstamps *hwtstamps = skb_hwtstamps(skb);
    ret = memset_s(hwtstamps, sizeof(struct skb_shared_hwtstamps), 0, sizeof(struct skb_shared_hwtstamps));
    if (ret != EOK) {
        mttcan_warn("%s:tx timestamp not cleared.(ret=%d)\n", ndev->name, ret);
    }
    hwtstamps->hwtstamp = ns_to_ktime(ns);
}

STATIC void mttcan_handle_tx_timestamp(const struct net_device *ndev, u32 txe_e1)
{
    int ret;
    u32 txtsp;
    u32 mm_low;
    unsigned long irq_flags;
    struct mttcan_priv *priv = netdev_priv(ndev);
    u32 timestamp;
    u64 ns;
    mm_low = (txe_e1 & TXE_E1_MM_MASK) >> TXE_E1_MM_SHIFT;

    if (priv->can.echo_skb[mm_low]) {
        /* Using "struct canfd_frame::len" for the frame
         * length is supported on both CAN and CANFD frames.
        */
        struct sk_buff *skb = priv->can.echo_skb[mm_low];
        struct sk_buff *tmp_skb = skb_copy(skb, GFP_ATOMIC);
        if (tmp_skb == NULL) {
            return;
        }
        txtsp = txe_e1 & TXE_E1_TXTSP_MASK;
        timestamp = mttcan_read_reg(&priv->addr, REG_TSU_TS0 + ((u32)txtsp * BYTES_OF_WORD));

        raw_spin_lock_irqsave(&priv->tc_lock, irq_flags);
        ns = timecounter_cyc2time(&priv->tc, timestamp);
        raw_spin_unlock_irqrestore(&priv->tc_lock, irq_flags);

        mttcan_set_skb_hwtstamps(ndev, tmp_skb, ns);
        mttcan_set_skb_hwtstamps(ndev, skb, ns);
#ifdef RUN_IN_AOS
        ret = kfifo_in(&priv->tx_skb_kfifo, &tmp_skb, sizeof(struct sk_buff *)); //lint !e1058
#else
        ret = kfifo_put(&priv->tx_skb_kfifo, (const struct sk_buff *)tmp_skb); //lint !e1058
#endif
        if (ret == 0) {
            priv->status_cnt.tx_kfifo_full_cnt++;
        }
        mttcan_check_and_handle_ts_lost(ndev, txtsp);
    }
}

STATIC void mttcan_read_tx_event_element(const struct net_device *ndev, u32 addr_offset)
{
    u32 txe_e0, txe_e1;
    const struct mttcan_priv *priv = netdev_priv(ndev);
    void __iomem *element_addr = priv->addr.mram_base + addr_offset;

    /* read E0 */
    txe_e0 = readl(element_addr);

    /* read E1 */
    txe_e1 = readl(element_addr + BYTES_OF_WORD);
    if (txe_e1 & TXE_E1_TSC_MASK) {
        mttcan_handle_tx_timestamp(ndev, txe_e1);
    }
}

STATIC void mttcan_write_txb_t0(void __iomem *element_addr, struct sk_buff *skb)
{
    u32 txb_t0 = 0;
    const struct canfd_frame *cf = (struct canfd_frame *)skb->data;

    if (cf->flags & CANFD_ESI) {
        txb_t0 |= TXB_T0_ESI;
    }

    if (cf->can_id & CAN_EFF_FLAG) {
        txb_t0 |= TXB_T0_XTD;
        txb_t0 |= cf->can_id & CAN_EFF_MASK;
    } else {
        txb_t0 |= ((cf->can_id & CAN_SFF_MASK) << (CAN_EFF_ID_BITS - CAN_SFF_ID_BITS));
    }

    if (cf->can_id & CAN_RTR_FLAG) {
        txb_t0 |= TXB_T0_RTR;
    }

    writel(txb_t0, element_addr);
}

STATIC bool mttcan_need_tx_timestamp(struct sk_buff *skb)
{
#ifdef RUN_IN_AOS
    if (skb_get_txflags(skb) & SKBTX_HW_TSTAMP) {
#else
    struct skb_shared_info *ssinfo = skb_shinfo(skb);
    if (ssinfo == NULL) {
        return false;
    }
    if (ssinfo->tx_flags & SKBTX_HW_TSTAMP) {
#endif
        return true;
    } else {
        return false;
    }
}

STATIC void mttcan_write_txb_t1(void __iomem *element_addr, struct sk_buff *skb, struct mttcan_priv *priv,
    u32 txb_mm)
{
    u32 txb_t1, txb_dlc;
    struct canfd_frame *cf = (struct canfd_frame *)skb->data;

    txb_t1 = ((u64)txb_mm << TXB_T1_MML_SHIFT) & TXB_T1_MML_MASK;
    if (priv->elmt_param[ELMT_TXEF].num) {
        txb_t1 |= TXB_T1_EFC;
    }

    if (mttcan_need_tx_timestamp(skb)) {
        txb_t1 |= TXB_T1_TSCE;
    }

    if (can_is_canfd_skb(skb)) {
        txb_t1 |= TXB_T1_FDF;
        if (cf->flags & CANFD_BRS) {
            txb_t1 |= TXB_T1_BRS;
        }
    }

    txb_dlc = can_fd_len2dlc(cf->len);
    txb_t1 |= ((u64)txb_dlc << TXB_T1_DLC_SHIFT) & TXB_T1_DLC_MASK;

    writel(txb_t1, element_addr + BYTES_OF_WORD);
}

STATIC void mttcan_write_tx_element(const struct net_device *ndev, u32 addr_offset, struct sk_buff *skb, u32 txb_mm)
{
    u32 data_word;
    u8 remain_bytes;
    u8 write_bytes = 0;
    struct mttcan_priv *priv = netdev_priv(ndev);
    void __iomem *element_addr = priv->addr.mram_base + addr_offset;
    struct canfd_frame *cf = (struct canfd_frame *)skb->data;

    /* write T0 */
    mttcan_write_txb_t0(element_addr, skb);

    /* write T1 */
    mttcan_write_txb_t1(element_addr, skb, priv, txb_mm);

    /* write data */
    remain_bytes = cf->len;
    while (remain_bytes >= BYTES_OF_WORD) {
        data_word = *(u32 *)(cf->data + write_bytes);

        /* write data_word to message ram */
        writel(data_word, element_addr + DATA_OFFSET + write_bytes);
        write_bytes += BYTES_OF_WORD;
        remain_bytes -= BYTES_OF_WORD;
    }

    data_word = 0;
    if (remain_bytes == 0x3) {
        data_word |= cf->data[write_bytes + 0x2] << DATA_BYTE2_OFF;
        remain_bytes--;
    }

    if (remain_bytes == 0x2) {
        data_word |= cf->data[write_bytes + 1] << DATA_BYTE1_OFF;
        remain_bytes--;
    }

    if (remain_bytes == 1) {
        data_word |= cf->data[write_bytes + 0] << DATA_BYTE0_OFF;
        writel(data_word, element_addr + DATA_OFFSET + write_bytes);
    }
#ifdef SUPPORT_MTTCAN_DFX
    mttcan_statistics_canbus(skb, cf, priv);
#endif
}

/* read-write control of message ram */
STATIC u32 get_rxb_index(const struct net_device *ndev, u32 *ndat1, u32 *ndat2)
{
    int index1, index2;
    u32 rxb_idx;
    u32 temp;
    const struct mttcan_priv *priv = netdev_priv(ndev);

    index1 = ffs(*ndat1) - 1;
    index2 = ffs(*ndat2) - 1;

    if (index1 >= 0) {
        rxb_idx = (u32)index1;
        temp = 1U << rxb_idx;
        mttcan_write_reg(&priv->addr, REG_NDAT1, temp);
        (*ndat1) &= ~temp;
    } else {
        rxb_idx = (u32)index2 + NDAT_BITS;
        temp = 1U << (u32)index2;
        mttcan_write_reg(&priv->addr, REG_NDAT2, temp);
        (*ndat2) &= ~temp;
    }

    return rxb_idx;
}

STATIC int get_txb_index(const struct net_device *ndev, u32 *txb_idx)
{
    u32 txbrp;        /* Tx Buffer Request Pending */
    u32 txfqs;        /* Tx FIFO/Queue Status */
    u32 ded_txb_free; /* Dedicated Tx Buffers free */
    u32 txf_put_idx;
    struct mttcan_priv *priv = netdev_priv(ndev);

    spin_lock(&priv->tx_lock);
    /* 1. get dedicated tx buffer index which is free */
    txbrp = mttcan_read_reg(&priv->addr, REG_TXBRP);
    ded_txb_free = ~(txbrp | (u32)priv->tx_obj);
    ded_txb_free &= priv->dedicated_txb_mask;
    if (ded_txb_free != 0) {
        *txb_idx = (u32)(ffs(ded_txb_free) - 1);
        spin_unlock(&priv->tx_lock);
        return 0;
    }

    /* 2. get tx fifo/queue put_index if tx buffer full */
    txfqs = mttcan_read_reg(&priv->addr, REG_TXFQS);
    if (txfqs & TXFQS_TFQF_BIT) {
        spin_unlock(&priv->tx_lock);
        return -EBUSY;
    }

    txf_put_idx = (txfqs & TXFQS_TFQPI_MASK) >> TXFQS_TFQPI_SHIFT;
    if (unlikely((u32)priv->tx_obj & (1U << txf_put_idx))) {
#ifdef SUPPORT_MTTCAN_FAULT_RECOVERY
        if (priv->last_busy_tx_idx != txf_put_idx) {
            priv->last_busy_tx_time = jiffies;
        }
        priv->last_busy_tx_idx = txf_put_idx;
#endif
        spin_unlock(&priv->tx_lock);
        return -EBUSY;
    }

#ifdef SUPPORT_MTTCAN_FAULT_RECOVERY
    priv->last_busy_tx_time = 0;
    priv->last_busy_tx_idx = txf_put_idx;
#endif
    spin_unlock(&priv->tx_lock);
    *txb_idx = txf_put_idx;

    return 0;
}

int mttcan_read_rx_fifo0(struct net_device *ndev, int weight)
{
    int packets = 0;
    u32 fifo0_fill_level;
    u32 addr_offset;
    u32 f0gi;  /* Rx FIFO 0 Get Index */
    u32 rxf0s; /* Rx FIFO 0 Status */
    struct mttcan_priv *priv = netdev_priv(ndev);

    if (weight <= 0) {
        return 0;
    }

    rxf0s = mttcan_read_reg(&priv->addr, REG_RXF0S);
    fifo0_fill_level = rxf0s & RXF0S_F0FL_MASK;

    while ((fifo0_fill_level > 0) && (packets < weight)) {
        f0gi = (rxf0s & RXF0S_F0GI_MASK) >> RXF0S_F0GI_SHIFT;
        if (unlikely(priv->elmt_param[ELMT_RXF0].num <= f0gi)) {
            mttcan_err("%s: reg err f0gi >= rxf0 ram size.\n", ndev->name);
        } else {
            addr_offset = priv->elmt_param[ELMT_RXF0].off + f0gi * priv->elmt_size.rxf0;
            mttcan_read_rx_element(ndev, addr_offset);
        }

        mttcan_write_reg(&priv->addr, REG_RXF0A, f0gi);
        packets++;
        rxf0s = mttcan_read_reg(&priv->addr, REG_RXF0S);
        fifo0_fill_level = rxf0s & RXF0S_F0FL_MASK;
    }

    return packets;
}

int mttcan_read_rx_fifo1(struct net_device *ndev, int weight)
{
    int packets = 0;
    u32 addr_offset;
    u32 fifo1_fill_level;
    u32 rxf1s; /* Rx FIFO 1 Status */
    u32 f1gi;  /* Rx FIFO 1 Get Index */
    struct mttcan_priv *priv = netdev_priv(ndev);

    if (weight <= 0) {
        return 0;
    }

    rxf1s = mttcan_read_reg(&priv->addr, REG_RXF1S);
    fifo1_fill_level = rxf1s & RXF1S_F1FL_MASK;

    while ((fifo1_fill_level > 0) && (packets < weight)) {
        f1gi = (rxf1s & RXF1S_F1GI_MASK) >> RXF1S_F1GI_SHIFT;
        if (unlikely(priv->elmt_param[ELMT_RXF1].num <= f1gi)) {
            mttcan_err("%s: reg err f1gi >= rxf1 ram size.\n", ndev->name);
        } else {
            addr_offset = priv->elmt_param[ELMT_RXF1].off + f1gi * priv->elmt_size.rxf1;
            mttcan_read_rx_element(ndev, addr_offset);
        }
        mttcan_write_reg(&priv->addr, REG_RXF1A, f1gi);
        packets++;
        rxf1s = mttcan_read_reg(&priv->addr, REG_RXF1S);
        fifo1_fill_level = rxf1s & RXF1S_F1FL_MASK;
    }

    return packets;
}

int mttcan_read_rx_buffer(struct net_device *ndev, int weight)
{
    u32 addr_offset;
    u32 ndat1, ndat2; /* new data */
    u32 rxb_idx;
    int packets = 0;
    const struct mttcan_priv *priv = netdev_priv(ndev);

    ndat1 = mttcan_read_reg(&priv->addr, REG_NDAT1);
    ndat2 = mttcan_read_reg(&priv->addr, REG_NDAT2);

    while ((ndat1 | ndat2) && (packets < weight)) {
        rxb_idx = get_rxb_index(ndev, &ndat1, &ndat2);
        if (unlikely(priv->elmt_param[ELMT_RXB].num <= rxb_idx)) {
            mttcan_err("%s: reg err rxb_idx >= rxb ram size.\n", ndev->name);
        } else {
            addr_offset = priv->elmt_param[ELMT_RXB].off + rxb_idx * priv->elmt_size.rxb;
            mttcan_read_rx_element(ndev, addr_offset);
        }
        packets++;
    }

    return packets;
}

STATIC int mttcan_tx_free_skb(struct net_device *ndev, u32 tc_idx,
    u32 skb_max, struct sk_buff *skb)
{
    int bytes = 0;

    if (likely(tc_idx < skb_max)) {
        if (skb != NULL) {
            /* struct canfd is compatible with can frame */
            struct canfd_frame *cf = (struct canfd_frame *)skb->data;
            bytes = cf->len;
            can_free_echo_skb(ndev, tc_idx, NULL);
        }
    } else {
        bytes = CAN_ERR_DLC;
    }

    return bytes;
}

STATIC void mttcan_get_tx_latency(struct mttcan_priv *priv, u32 tx_idx)
{
    u32 index;
    struct sk_buff *skb = priv->can.echo_skb[tx_idx];
    s64 ns_delta;
    if (unlikely(skb == NULL)) {
        return;
    }
    ns_delta = ktime_to_ns(ktime_get_real()) - ktime_to_ns(skb->tstamp);

    index = priv->can_perf_record[CAN_XMIT].index;
    if (ns_delta >= priv->can_perf_record[CAN_XMIT].latency_threshold) {
        if (index >= PERF_RECORD_SIZE) {
            index = 0;
        }
        priv->can_perf_record[CAN_XMIT].data[index].start_time = ktime_to_ns(skb->tstamp);
        priv->can_perf_record[CAN_XMIT].data[index].latency = ns_delta;
        index++;
    }
    priv->can_perf_record[CAN_XMIT].index = index;
    skb->tstamp = (ktime_t)0;
}

STATIC struct sk_buff* get_echo_skb(struct mttcan_priv *priv, u32 tc_idx)
{
    if (tc_idx >= priv->can.echo_skb_max) {
        return NULL;
    }

    if (priv->can.echo_skb[tc_idx] != NULL) {
        struct sk_buff *skb = priv->can.echo_skb[tc_idx];
        priv->can.echo_skb[tc_idx] = NULL;
        return skb;
    }

    return NULL;
}

STATIC void mttcan_tx_complete_done(struct net_device *ndev, u32 tc_idx, u32 *tx_complete)
{
    struct net_device_stats *ndev_stats = &ndev->stats;
    struct mttcan_priv *priv = netdev_priv(ndev);
    u32 bytes = 0;
    struct sk_buff *skb = NULL;

    if (!netif_running(ndev)) {
        skb = priv->can.echo_skb[tc_idx];
        bytes = mttcan_tx_free_skb(ndev, tc_idx, priv->can.echo_skb_max, skb);
    } else {
        mttcan_get_tx_latency(priv, tc_idx);
        skb = get_echo_skb(priv, tc_idx);
        if (skb != NULL) {
            int ret;
            struct canfd_frame *cf = (struct canfd_frame *)skb->data;
            bytes = (u32)cf->len;
#ifdef RUN_IN_AOS
            ret = kfifo_in(&priv->tx_skb_kfifo, &skb, sizeof(struct sk_buff *));
#else
            ret = kfifo_put(&priv->tx_skb_kfifo, (const struct sk_buff *)skb);
#endif
            if (ret == 0) {
                priv->status_cnt.tx_kfifo_full_cnt++;
            }
        }
    }
    pr_debug("%s: tx_complete ok.\n", ndev->name);

    ndev_stats->tx_bytes += bytes;
    ndev_stats->tx_packets++;
    clear_bit(tc_idx, (void *)&priv->tx_obj);
    *tx_complete &= ~(1U << tc_idx);
    return;
}
STATIC void mttcan_tx_complete_from_buf(struct net_device *ndev, u32 *tx_complete)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    u32 buff_mask = 0; /* buff mask */
    u32 tc_idx;

    if (priv->cfg.tx_element_num[TX_ELMT_BUF] != 0) {
        buff_mask = GENMASK(priv->cfg.tx_element_num[TX_ELMT_BUF], 0);
    } else {
        return;
    }

    while (*tx_complete & buff_mask) {
        tc_idx = ffs(*tx_complete) - 1;
        if (tc_idx >= priv->cfg.tx_element_num[TX_ELMT_BUF]) {
            /* buff fetch finished. */
            break;
        }
        mttcan_tx_complete_done(ndev, tc_idx, tx_complete);
    }
    return;
}

STATIC void mttcan_tx_complete_from_queue(struct net_device *ndev, u32 *tx_complete)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    u32 tc_idx;

    while (*tx_complete) {
        tc_idx = ffs(*tx_complete) - 1;
        if (tc_idx >= priv->cfg.tx_element_num[TX_ELMT_FQ] + priv->cfg.tx_element_num[TX_ELMT_BUF]) {
            /* buff fetch finished. */
            break;
        }
        mttcan_tx_complete_done(ndev, tc_idx, tx_complete);
    }
    return;
}

STATIC void mttcan_tx_complete_from_fifo(struct net_device *ndev, u32 *tx_complete, u32 fifo_len)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    bool find_first_bit = false;
    u32 tc_idx;
    int i;
    u32 tfqs;
    u32 tfgi;
    u32 tfqpi;

    tfqs = mttcan_read_reg(&priv->addr, REG_TXFQS);
    tfqpi = (tfqs & TXFQS_TFQPI_MASK) >> TXFQS_TFQPI_SHIFT;
    tfgi = (tfqs & TXFQS_TFGI_MASK) >> TXFQS_TFGI_SHIFT;
    pr_debug("%s: tfqs = %u.\n", ndev->name, tfqs);

    tc_idx = tfqpi + 1;
    for (i = 0; i < fifo_len; i++, tc_idx++) {
        if (tc_idx >= priv->cfg.tx_element_num[TX_ELMT_FQ] + priv->cfg.tx_element_num[TX_ELMT_BUF]) {
            tc_idx = priv->cfg.tx_element_num[TX_ELMT_BUF];
        }

        /* find the first set position */
        if (!find_first_bit) {
            if ((*tx_complete & (1 << tc_idx)) == 0) {
                continue;
            } else {
                find_first_bit = true;
            }
        }

        if (find_first_bit) {
            if ((*tx_complete & (1 << tc_idx)) == 0) {
                break;
            }
        }
        pr_debug("%s: tx_complete = %u tc_idx = %u.\n", ndev->name, *tx_complete, tc_idx);
        mttcan_tx_complete_done(ndev, tc_idx, tx_complete);
    }
    return;
}

void mttcan_tx_complete(struct net_device *ndev)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    unsigned long irq_flags;
    u32 tx_complete;
    u32 txbc;
    u32 txbc_tfqs; /* fifo or queue size */
    u32 txbto; /* Tx Buffer Transmission Occurred */

    raw_spin_lock_irqsave(&priv->tx_complete, irq_flags);
    txbto = mttcan_read_reg(&priv->addr, REG_TXBTO);
    tx_complete = txbto & (u32)priv->tx_obj;

    if (unlikely(!tx_complete)) {
        raw_spin_unlock_irqrestore(&priv->tx_complete, irq_flags);
        return;
    }

    txbc = mttcan_read_reg(&priv->addr, REG_TXBC);
    txbc_tfqs = (txbc & TXBC_TFQS_MASK) >> TXBC_TFQS_SHIFT;

    pr_debug("%s, txbc = %u,tx_complete = %u.\n", ndev->name, txbc, tx_complete);
    /* first buff */
    mttcan_tx_complete_from_buf(ndev, &tx_complete);

    /* check fifo/queue size */
    if (txbc_tfqs == 0) {
        raw_spin_unlock_irqrestore(&priv->tx_complete, irq_flags);
        return;
    }

    if (txbc & TXBC_TFQM_BIT) { /* queue */
        mttcan_tx_complete_from_queue(ndev, &tx_complete);
    } else { /* fifo */
        mttcan_tx_complete_from_fifo(ndev, &tx_complete, txbc_tfqs);
    }
    raw_spin_unlock_irqrestore(&priv->tx_complete, irq_flags);
}

void mttcan_tx_event(struct net_device *ndev)
{
    u32 addr_offset, txefs, fill_level;
    u32 efgi = 0;
    struct mttcan_priv *priv = netdev_priv(ndev);

    txefs = mttcan_read_reg(&priv->addr, REG_TXEFS);
    fill_level = (txefs & TXEFS_EFFL_MASK) >> TXEFS_EFFL_SHIFT;
    if (unlikely(fill_level == 0)) {
        return;
    }

    do {
        efgi = (txefs & TXEFS_EFGI_MASK) >> TXEFS_EFGI_SHIFT;
        if (unlikely(priv->elmt_param[ELMT_TXEF].num <= efgi)) {
            mttcan_err("%s: reg err, efgi >= txef ram size.\n", ndev->name);
        } else {
            addr_offset = priv->elmt_param[ELMT_TXEF].off + efgi * ELMT_SIZE_TXEF;
            /* read tx event fifo */
            mttcan_read_tx_event_element(ndev, addr_offset);
        }

        mttcan_write_reg(&priv->addr, REG_TXEFA, efgi);
        txefs = mttcan_read_reg(&priv->addr, REG_TXEFS);
        fill_level = (txefs & TXEFS_EFFL_MASK) >> TXEFS_EFFL_SHIFT;
    } while (fill_level > 0);
}

void mttcan_tx_cancel_finish(struct net_device *ndev)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    struct net_device_stats *ndev_stats = &ndev->stats;
    u32 txbcf;
    u32 tx_cancel;
    u32 tx_cancel_idx;
    unsigned long irq_flags;

    raw_spin_lock_irqsave(&priv->tx_complete, irq_flags);
    txbcf = mttcan_read_reg(&priv->addr, REG_TXBCF);
    tx_cancel = txbcf & priv->tx_obj;

    while (tx_cancel) {
        tx_cancel_idx = ffs(tx_cancel) - 1;
        can_free_echo_skb(ndev, tx_cancel_idx, NULL);
        clear_bit(tx_cancel_idx, (void *)&priv->tx_obj);
        tx_cancel &= ~(1U << tx_cancel_idx);
        ndev_stats->tx_aborted_errors++;
    }
    raw_spin_unlock_irqrestore(&priv->tx_complete, irq_flags);
}

int mttcan_write_tx_message(const struct net_device *ndev, struct sk_buff *skb, u32 *txb_idx)
{
    int err;
    u32 addr_offset;
    struct mttcan_priv *priv = netdev_priv(ndev);

    /* get dedicated tx buffer or tx fifo/queue index */
    err = get_txb_index(ndev, txb_idx);
    if (unlikely(err < 0)) {
        return err;
    }

    if (unlikely(*txb_idx >= priv->cfg.element_num[ELMT_TXB])) {
        mttcan_err("%s: get_txb_index error, index is invalid.\n", ndev->name);
        return -EFAULT;
    }

    addr_offset = priv->elmt_param[ELMT_TXB].off + (*txb_idx) * priv->elmt_size.txb;
    mttcan_write_tx_element(ndev, addr_offset, skb, *txb_idx);

    return 0;
}
