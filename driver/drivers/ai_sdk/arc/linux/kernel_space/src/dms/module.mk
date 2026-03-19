LOCAL_PATH := $(call my-dir)

ifeq ($(build_device), true)

PRODUCT_SIDE=device

include $(CLEAR_VARS)

LOCAL_MODULE := drv_devmng

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
ifneq ($(TARGET_PRODUCT),mini)
	LOCAL_DEPEND_KO:=drv_icm drv_pcie drv_dfm mntn_bbox drv_davinci_intf ascend_uda ascend_trs_tsmng drv_fpdc drv_user_cfg dbl_chip_config ascend_virtmng_dev
else
	ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
		LOCAL_DEPEND_KO:=drv_icm drv_pcie drv_dfm mntn_bbox drv_davinci_intf ascend_uda ascend_trs_tsmng drv_fpdc drv_user_cfg dbl_chip_config ascend_virtmng_dev
	else
		LOCAL_DEPEND_KO:=drv_icm drv_pcie drv_dfm mntn_bbox drv_davinci_intf ascend_uda ascend_trs_tsmng drv_fpdc drv_user_cfg dbl_chip_config ascend_virtmng_dev
	endif
endif

LOCAL_INSTALLED_KO_FILES := drv_devmng.ko

include $(BUILD_DEVICE_KO)

else

PRODUCT_SIDE=host

include $(CLEAR_VARS)

LOCAL_MODULE := drv_devmng_host

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := drv_devmng_host.ko

include $(BUILD_HOST_KO)

ifeq ($(PRODUCT), ascend910)
	LOCAL_MODULE := drv_vdevmng

	LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

	LOCAL_INSTALLED_KO_FILES := drv_vdevmng.ko

	include $(BUILD_HOST_KO)
endif

endif
