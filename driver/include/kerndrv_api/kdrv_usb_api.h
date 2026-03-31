/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Description: USB external interface
 * Author: huawei
 * Create: 2022-10-25
 */

#ifndef KDRV_USB_API_H
#define KDRV_USB_API_H
#include <linux/init.h>
#ifdef __cplusplus
extern "C" {
#endif

struct kdrv_usb_err_sts {
    u32 hc_error;    /* The controller happened internal error */
    u32 ss_error;     /* An error occurred during the save or restore operation */
    u32 host_sys_err;    /* The controller has generated a critical error */
};

struct kdrv_usb_port_status {
    u32 plc;    /* Port Link State Change */
    u32 csc;    /* Connect Status Change */
    u32 speed;    /* port speed 1:FS 2:LS 3:HS */
    u32 pp;    /* port power 0:power off 1:power on */
    u32 pls;    /* port link state 0:U0 2:U2 3:U3 4:disable */
    u32 oca;    /* overcurrent */
    u32 ped;    /* port enable */
    u32 connect;    /* connect status */
};

struct kdrv_usb_event_int_status {
    u32 int_enable;     /* Interrupt enable status */
    u32 int_pending;    /* Whether there are pending interrupts */
};

struct kdrv_usb_cmd_ring_info {
    u32 crr;    /*  1:the controller is running and the doorbell register is configured. */
    u32 rcs;    /* Indicates the value of consumer cycle state in the cmd ring. */
};

struct kdrv_usb_config_info {
    u32 cie;  /* 1:The extended Input Control Context field is valid. */
    u32 u3e;  /* 1:When the roothub port enters the U3 state, the xhc should set plc flag to 1. */
    u32 dev_slot_ena;  /* Maximum number of device slots that can be enabled */
};

struct kdrv_usb_pmsc {
    struct usb_u2pm {
        u32 test_mode;   /* Test mode 0: not enable 1:J 2:K 3:SE0 4:packet 15:err */
        u32 hle;         /* Hardware LPM enable */
        u32 remote_wake_ena;  /* Remote wakeup enable */
        u32 l1_status;  /* Handshake result of LPM packets entering L1 */
    } u2pm;
    struct usb_u3pm {
        u32 u2_timeout;  /* Timeout interval for entering the u2 state */
        u32 u1_timeout;  /* Timeout interval for entering the u1 state */
    } u3pm;
};

struct kdrv_usb_u3_link_info {
    u32 tlc;   /* Number of TX channels negotiated by the port */
    u32 rlc;   /* Number of RX channels negotiated by the port */
    u32 link_err_cnt;   /* Number of link errors detected by the port */
};

struct kdrv_usb_global_status {
    u32 ssic_ip; /* Indicates that there is an SSIC-related interrupt waiting for processing in the SEVT register. */
    u32 otg_ip; /* Indicates that there is an OTG-related interrupt waiting for processing in the OEVT register. */
    u32 bc_ip; /* Indicates that there is a BC-related interrupt waiting for processing in the BCEVT register. */
    u32 adp_ip; /* Indicates that there is an ADP-related interrupt waiting for processing in the ADPEVT register. */
    u32 host_ip; /* Indicates that there is an xHCI-related interrupt waiting for processing in the host event queue. */
    u32 device_ip; /* Indicates that there is an xHCI-related interrupt waiting for processing in device event queue. */
    u32 csrtimeout; /* Indicates that the software access register times out. */
};

struct kdrv_usb_global_bus_err_addr {
    u32 buserraddrvld;   /* Indicates whether the GBUSERRADDR register is valid. */
    u32 busaddrlo;  /* Lower 32 bits of the global bus error address */
    u32 busaddrhi;  /* Upper 32 bits of the global bus error address */
};

struct kdrv_usb_trans_packet {
    u32 send_pkt_num;        /* Number of sent packets,Read clear */
    u32 rec_good_pkt_num;    /* Number of received correct packets,Read clear */
    u32 rec_bad_pkt_num;     /* Number of received error packets,Read clear */
};

struct kdrv_usb_axi_cfg {
    u32 qos;        /* Corresponding csr_lmi_axi_qos_0 */
};

/**
 * @brief Obtains the error status of the USB host controller.
 * @param[in]   host       Controller ID
 * @param[out]  sts        USB controller error status structure
 * @return int  Return value. 0: success Others: failure
 */
int kdrv_usb_get_host_error_status(u32 host, struct kdrv_usb_err_sts *sts);

/**
 * @brief   Function description: Obtains the status information of the USB port.
 * @param[in]  host         Controller ID
 * @param[in]  port         Port number, starting from 1.
 * @param[out] sts          Indicates the port status. For details, see. struct kdrv_usb_port_status
 * @return int              Return value. 0: success Others: failure, unreliable write or output parameters on failure
 */
int kdrv_usb_get_port_status(u32 host, u32 port, struct kdrv_usb_port_status *sts);

/**
 * @brief Get port low power consumption info, different definitions of USB 2.0 and 3.0 needs to distinguish
 * @param[in]  host    Controller ID
 * @param[in]  port    Port number, starting from 1.
 * @param[out] pmsc    Low-power state structure
 * @return int         Return value. 0: success Others: failure
 */
int kdrv_usb_get_port_pmsc(u32 host, u32 port, struct kdrv_usb_pmsc *pmsc);

/**
 * @brief Obtains the port link information. This parameter is valid only in USB 3.0 mode.
 * @param[in]   host       Controller ID
 * @param[in]   port       Port number, starting from 1.
 * @param[out]  info       U3 link information structure
 * @return int  Return value. 0: success Others: failure
 */
int kdrv_usb_get_port_link_info(u32 host, u32 port, struct kdrv_usb_u3_link_info *info);

/**
 * @brief Obtains the USB interrupt status information.
 * @param[in]   host       Controller ID
 * @param[out]  sts        USB interrupt status structure
 * @return int  Return value. 0: success Others: failure
 */
int kdrv_usb_get_event_int_status(u32 host, struct kdrv_usb_event_int_status *sts);

/**
 * @brief Get usb cmd ring info
 * @param[in]   host       Controller ID
 * @param[out]  info       cmdring info struct
 * @return int  Return value. 0: success Others: failure
 */
int kdrv_usb_get_cmdring_info(u32 host, struct kdrv_usb_cmd_ring_info *info);

/**
 * @brief Obtains the configuration information during USB running.
 * @param[in]   host    Controller ID
 * @param[out]  info    config info struct
 * @return int  Return value. 0: success Others: failure
 */
int kdrv_usb_get_config_info(u32 host, struct kdrv_usb_config_info *info);

/**
 * @brief To query the global status register information
 * @param[in]    host       Controller ID
 * @param[out]   gsts       Global status information structure
 * @return int   Return value. 0: success Others: failure
 */
int kdrv_usb_get_global_status(u32 host, struct kdrv_usb_global_status *sts);

/**
 * @brief Obtaining the global bus error address
 * @param[in]   host     Controller ID
 * @param[out]  addr     Global bus information structure
 * @return int  Return value. 0: success Others: failure
 */
int kdrv_usb_get_global_bus_err_addr(u32 host, struct kdrv_usb_global_bus_err_addr *addr);

/**
 * @brief Obtains the current mode information.
 * @param[in]   host        Controller ID
 * @param[out]  curmod      Crrent working mode. Purchased IP(0 : device, 1:host.) Huawei-developed IP are opposite.
 * @return int  Return value. 0: success Others: failure
 */
int kdrv_usb_get_current_mode(u32 host, u32 *curmod);

/**
 * @brief 
 * Enable collecting statistics on transmitted func, received packets only after the function is enabled. 
 * Only for Huawei-developed IP addresses.
 * @param[in]   host        Controller ID
 * @param[in]   enable      0:off, not 0:on
 * @return int  Return value. 0: success Others: failure
 */
int kdrv_usb_enable_transfer_packet_dfx(u32 host, u32 enable);

/**
 * @brief 
 * Get trans packets num, inc total trans packets num, total received correct packets num,
 * total received err packets num. Only for Huawei-developed IP addr.
 * @param[in]   host        Controller ID
 * @param[out]  pkt         Transmission packet structure
 * @return int  Return value. 0: success Others: failure
 */
int kdrv_usb_get_transfer_packet(u32 host, struct kdrv_usb_trans_packet *pkt);

/**
 * @brief Obtains the vendor information of the USB host device.
 * @param[in]   host         Controller ID
 * @param[out]  vender_id    Vendor ID
 * @param[out]  device_id    Device ID
 * @return int               Return value. 0: success Others: failure, Unreliable write or output parameters on failure
 */
int kdrv_usb_get_vendor_id(u32 host, u16 *vender_id, u16 *device_id);

/**
 * @brief Reset usb Controller
 * @param[in]  host              Controller ID
 * @return int                   Return value. 0: success Others: failure
 */
int kdrv_usb_reset(u32 host);
/**
 * @brief usb fmea Inspection Entry
 * @param[in] devid                   usb Controller ID
 * @param[out] err_info               Error information returned by the fmea
 * @param[out] alm_flg                Indicates the alarm type returned by the FMEA.
 * @param[out] buf                    Start address of the description string returned by the FMEA.
 * @param[in] size                    buf size
 * @return int                        Return value. 0: success Others: failure
 */

/**
 * @brief set USB axi attribute
 * @param[in]   host             Controller ID
 * @param[in]   cfg              kdrv_usb_axi_cfg Structure pointer
 * @return int                   Return value. 0: success Others: failure
 */
int kdrv_usb_set_axi_cfg(u32 host, struct kdrv_usb_axi_cfg *cfg);

/**
 * @brief get USB axi attribute
 * @param[in]   host              Controller ID
 * @param[out]  cfg               kdrv_usb_axi_cfg Structure pointer
 * @return int                    Return value. 0: success Others: failure
 */
int kdrv_usb_get_axi_cfg(u32 host, struct kdrv_usb_axi_cfg *cfg);

#ifdef CONFIG_FMEA
int kdrv_usb_fmea_entry(u32 devid, u64 *err_info, u32 *alm_flg, char *buf, u32 size);
#endif
#ifdef __cplusplus
}
#endif

#endif

