############################################################################
####                 official    release modules                  ########
############################################################################

#define driver released modules in DEVICE
#
DRIVER_DEVICE_MODULES += drv_prof.ko drv_platform.ko ascend_soc_resmng.ko ascend_trs_nvme_chan.ko ascend_trs_core.ko ascend_trs_id_pool.ko ascend_trs_id_allocator.ko ascend_trs_pm_adapt.ko ascend_trs_adapt_agent.ko ascend_trs_mia_agent.ko ascend_trs_mia_adapt.ko drv_ts.ko ascend_trs_tsmng.ko ascend_event_sched.ko ascend_trs_stars.ko ascend_uda.ko
DRIVER_DEVICE_MODULES += ipc_drv.ko drv_icm.ko ascend_virtmng_dev.ko drv_mttcan.ko drv_serdes.ko
DRIVER_DEVICE_MODULES += drv_i2c_uniform.ko drv_sys_ctrl_uniform.ko drv_hiusbc.ko drv_spi_uniform.ko drv_gpio_uniform.ko
DRIVER_DEVICE_MODULES += drv_log.ko ascend_xsmem.ko drv_dp_proc_mng.ko mntn_bbox.ko dbl_board_config.ko dbl_cfg_persistent.ko dbl_chip_config.ko dbl_algorithm.ko dbl_dev_identity.ko dbl_runenv_config.ko
DRIVER_DEVICE_MODULES += drv_ascend_ctl.ko drv_user_cfg.ko drv_dfm.ko drv_pkicms.ko drv_fpdc.ko drv_devmng.ko drv_soft_fault.ko drv_upgrade.ko drv_soc_misc.ko drv_davinci_intf.ko drv_memory.ko hclgeplf.ko
DRIVER_DEVICE_MODULES += drv_pcierc_sr.ko pwm.ko
DRIVER_DEVICE_MODULES += ascend_vdp_hifb.ko
DRIVER_DEVICE_MODULES += drv_tde.ko drv_hdmi.ko
DRIVER_DEVICE_MODULES += ascend_dms_dtm.ko ascend_dms_mng.ko ascend_dms_smf.ko ascend_trs_cdqm.ko ascend_urd.ko ascend_virtmng_resmgr.ko
DRIVER_DEVICE_MODULES += drv_centre_notify.ko drv_devdrv.ko drv_e2prom.ko drv_lpm_devmng.ko drv_lpm_fault.ko drv_media_compat.ko drv_osal.ko
DRIVER_DEVICE_MODULES += drv_pcie.ko drv_pcie_hdc.ko drv_pm.ko drv_usb_uniform.ko hnae3.ko hns3.ko

#add ALL module variables
CUSTOM_DEVICE_MODULES_VARIABLES := \
  DRIVER_DEVICE_MODULES

#define driver released modules in HOST
#
DRIVER_HOST_MODULES += ascend_event_sched_host.ko ascend_soc_platform.ko ascend_trs_cdqm.ko ascend_trs_core.ko ascend_trs_id_allocator.ko
DRIVER_HOST_MODULES += ascend_trs_nvme_chan.ko ascend_trs_pm_adapt.ko ascend_trs_sec_eh_agent.ko ascend_trs_shrid.ko ascend_trs_sub_stars.ko
DRIVER_HOST_MODULES += ascend_uda.ko ascend_xsmem.ko dbl_algorithm.ko dbl_dev_identity.ko dbl_runenv_config.ko
DRIVER_HOST_MODULES += drv_davinci_intf_host.ko drv_devdrv_host.ko drv_devmng_host.ko drv_dp_proc_mng_host.ko drv_pcie_hdc_host.ko
DRIVER_HOST_MODULES += drv_tsdrv_platform_host.ko drv_vhdc.ko drv_virtmng_host.ko drv_vpc_host.ko drv_vpcie.ko drv_vtsdrv.ko

CUSTOM_HOST_MODULES_VARIABLES += \
  DRIVER_HOST_MODULES
