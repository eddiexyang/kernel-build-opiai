/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2022. All rights reserved.
 * Description: hdmi product define.
 * Author: Hisilicon multimedia software group
 * Create: 2020/05/15
 */

#ifndef HDMI_PRODUCT_DEFINE_H
#define HDMI_PRODUCT_DEFINE_H

#include <linux/fs.h>
#include "dev_ext.h"
#include "mod_ext.h"
#include "ot_osal.h"
#include "ot_type.h"
#include "drv_hdmi_common.h"
#include "vdp_comm_debug.h"

/*
 * ------------- reg base addr ------------
 *             ctrl     |       dphy      |
 * hdmi0 : 0x4_001A0000 |  0x4_00180000   |
 * hdmi1 : 0x4_001C0000 |  0x4_00190000   |
 * ---------------------|------------------
 */
#define MEDIA_SUBCTRL_ADDR         0x0400130000
#define MEDIA_HDMI_CRG_EN_OFFSET   0x620
#define MEDIA_HDMI_CRG_DIS_OFFSET  0x624
#define MEDIA_HDMI_CRG_ST_OFFSET   0x5620
#define HDMI_CRG_EN_ADDR           (MEDIA_SUBCTRL_ADDR + MEDIA_HDMI_CRG_EN_OFFSET)
#define HDMI_CRG_DIS_ADDR          (MEDIA_SUBCTRL_ADDR + MEDIA_HDMI_CRG_DIS_OFFSET)
#define HDMI_CRG_ST_ADDR           (MEDIA_SUBCTRL_ADDR + MEDIA_HDMI_CRG_ST_OFFSET)

#define MEDIA_HDMI_RESET_REQ       0xC00
#define MEDIA_HDMI_RESET_DREQ      0xC04
#define MEDIA_HDMI_RESET_ST        0x5C00
#define HDMI_RESET_REQ_ADDR        (MEDIA_SUBCTRL_ADDR + MEDIA_HDMI_RESET_REQ)
#define HDMI_RESET_DREQ_ADDR       (MEDIA_SUBCTRL_ADDR + MEDIA_HDMI_RESET_DREQ)
#define HDMI_RESET_ST_ADDR         (MEDIA_SUBCTRL_ADDR + MEDIA_HDMI_RESET_ST)

/* pin mux */
#define HDMI_ADDR_BASE_IO_CFG      0x17C70000
#define HDMI_ADDR_IO_CFG_HOTPLUG   (HDMI_ADDR_BASE_IO_CFG + 0x0)
#define HDMI_ADDR_IO_CFG_SDA       (HDMI_ADDR_BASE_IO_CFG + 0x4)
#define HDMI_ADDR_IO_CFG_SCL       (HDMI_ADDR_BASE_IO_CFG + 0x8)
#define HDMI1_ADDR_IO_CFG_HOTPLUG  (HDMI_ADDR_BASE_IO_CFG + 0x0)
#define HDMI1_ADDR_IO_CFG_SDA      (HDMI_ADDR_BASE_IO_CFG + 0x4)
#define HDMI1_ADDR_IO_CFG_SCL      (HDMI_ADDR_BASE_IO_CFG + 0x8)

/* color bar */
#define HDMI_COLOR_BAR_MASK        0x60000000
#define HDMI_COLOR_BAR_UPDATE_MASK 0x00000001
#define HDMI_COLOR_BAR_BASE        0x17A0D000
/* sub-module offset */
#define HDMI_TX_BASE_ADDR_CTRL     0x0000
#define HDMI_TX_BASE_ADDR_VIDEO    0x0800
#define HDMI_TX_BASE_ADDR_AUDIO    0x1000
#define HDMI_TX_BASE_ADDR_HDMITX   0x1800
#define HDMI_TX_BASE_ADDR_AON      0x4000
/* other macro */
#define HDMI_FILE_MODE             0777
#ifdef OT_ADVCA_FUNCTION_RELEASE
#define CONFIG_HDMI_PROC_DISABLE
#define CONFIG_HDMI_DEBUG_DISABLE
#endif
#define hdmi_mutex_unlock(a) osal_up((osal_semaphore_t *)&(a))
#define VERSION_STRING       ("[HDMI] Version: [" OT_MPP_VERSION "], Build Time["__DATE__", "__TIME__"]")
#define hdmi_get_current_id() (get_current()->tgid)

#define hdmi_err_trace(fmt, ...)          \
        VDP_ERR_TRACE("[drv_hdmi][ERR][%s:%d]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define hdmi_warn_trace(fmt, ...)         \
        VDP_WARN_TRACE("[drv_hdmi][WARN][%s:%d]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define hdmi_info_trace(fmt, ...)         \
        VDP_INFO_TRACE("[drv_hdmi][INFO][%s:%d]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define hdmi_debug_trace(fmt, ...)         \
        VDP_DEBUG_TRACE("[drv_hdmi][DEBUG][%s:%d]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define hdmi_fatal_trace(fmt, ...)         \
        VDP_EMERG_TRACE("[drv_hdmi][EMERG][%s:%d]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define hdmi_mutex_lock(a)                                    \
    do {                                                      \
        td_s32 retsult = osal_down((osal_semaphore_t *)&(a)); \
        if (retsult != 0) {                                   \
            hdmi_err("osal_down failed! ret=%d\n", retsult);  \
        }                                                     \
    } while (0)

#ifdef HDMI_LOG_SUPPORT
#ifndef OT_ADVCA_FUNCTION_RELEASE
#define hdmi_printk(fmt, args...) osal_printk(fmt, ##args)
#else
#define hdmi_printk(fmt, args...)
#endif
#else
#define hdmi_printk(fmt, args...)
#endif

#ifdef CONFIG_HDMI_DEBUG_DISABLE
#define edid_info(fmt...)
#define edid_warn(fmt...)
#define edid_err(fmt...)
#define edid_faital(fmt...)
#define hdmi_info(fmt...)
#define hdmi_warn(fmt...)
#define hdmi_err(fmt...)
#define hdmi_fatal(fmt...)
#elif defined(HDMI_LOG_SUPPORT)
#define edid_info(fmt...)
#define edid_warn(fmt...)
#define edid_err(fmt...)
#define edid_faital(fmt...)
#define hdmi_debug(fmt...)  hdmi_debug_trace(fmt)
#define hdmi_info(fmt...)   hdmi_info_trace(fmt)
#define hdmi_warn(fmt...)   hdmi_warn_trace(fmt)
#define hdmi_err(fmt...)    hdmi_err_trace(fmt)
#define hdmi_fatal(fmt...)  hdmi_fatal_trace(fmt)
#else
#define edid_info(fmt...)
#define edid_warn(fmt...)   hdmi_warn_trace(fmt)
#define edid_err(fmt...)    hdmi_err_trace(fmt)
#define edid_faital(fmt...) hdmi_fatal_trace(fmt)
#define hdmi_debug(fmt...)  hdmi_debug_trace(fmt)
#define hdmi_info(fmt...)   hdmi_info_trace(fmt)
#define hdmi_warn(fmt...)   hdmi_warn_trace(fmt)
#define hdmi_err(fmt...)    hdmi_err_trace(fmt)
#define hdmi_fatal(fmt...)  hdmi_fatal_trace(fmt)
#endif

typedef osal_proc_entry_t hdmi_proc_entry;
typedef struct {
    td_void *private;
    td_s32 (*read)(hdmi_proc_entry *file);
    td_s32 (*write)(hdmi_proc_entry *file, const char *buf, td_u32 count, const td_s64 * const data);
} hdmi_proc_item;

typedef struct {
    td_u32 ssc_bypass_div;
    td_u32 tmds_clk_div;
} hdmi_crg_cfg;

td_void hdmi_tx_reg_write(td_u32 *reg_addr, td_u32 value);
td_u32 hdmi_tx_reg_read(const td_u32 *reg_addr);
td_void hdmi_reg_write_u32(td_u32 reg_addr, td_u32 value);
td_u32 hdmi_reg_read_u32(td_u32 reg_addr);
td_void drv_hdmi_prod_io_cfg_set(td_u32 id);
td_void drv_hdmi_prod_crg_all_reset_set(td_u32 id, td_bool enable);
td_void drv_hdmi_prod_crg_gate_set(td_u32 id, td_bool enable);
td_void drv_hdmi_prod_crg_phy_reset_set(td_bool enable);
td_void drv_hdmi_prod_crg_phy_reset_get(td_bool *enable);
td_void drv_hdmi_prod_crg_init(td_u32 id);
td_void drv_hdmi_proc_crg_deinit(td_void);
td_void drv_hdmi_hardware_reset(td_u32 id);
td_void drv_hdmi_low_power_set(td_u32 id, td_bool enable);

#endif  /* HDMI_PRODUCT_DEFINE_H */

