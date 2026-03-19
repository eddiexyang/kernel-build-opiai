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

#add ALL module variables
CUSTOM_DEVICE_MODULES_VARIABLES := \
  DRIVER_DEVICE_MODULES
