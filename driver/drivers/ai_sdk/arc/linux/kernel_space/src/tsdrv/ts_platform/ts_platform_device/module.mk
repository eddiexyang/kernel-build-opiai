

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_platform

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

ifeq ($(TARGET_PRODUCT), cloud)
    LOCAL_DEPEND_KO :=
else ifeq ($(TARGET_PRODUCT),mini)
    ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
        LOCAL_DEPEND_KO:=drv_pkicms
    else
        LOCAL_DEPEND_KO :=
    endif
else ifeq ($(TARGET_PRODUCT),mdc)
    ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
        LOCAL_DEPEND_KO:=drv_pkicms
    else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
        LOCAL_DEPEND_KO:=drv_pkicms
    endif
endif

LOCAL_DEPEND_KO+=dbl_chip_config
ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
    LOCAL_DEPEND_KO+=drv_devmng ascend_dms_dtm ascend_dms_smf drv_devdrv drv_pcie drv_icm ipc_drv mntn_bbox drv_dfm ascend_uda drv_fpdc ascend_soc_resmng ascend_trs_nvme_chan ascend_trs_id_pool ascend_trs_pm_adapt ascend_trs_tsmng drv_user_cfg ascend_virtmng_dev
endif

LOCAL_INSTALLED_KO_FILES := drv_platform.ko

include $(BUILD_DEVICE_KO)
