LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_core
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_core.ko

ifeq ($(PRODUCT_SIDE), device)
include $(BUILD_DEVICE_KO)
LOCAL_DEPEND_KO:=ascend_trs_nvme_chan ascend_trs_id_allocator drv_davinci_intf drv_devmng
else
include $(BUILD_HOST_KO)
LOCAL_DEPEND_KO:=ascend_trs_nvme_chan ascend_trs_id_allocator drv_davinci_intf_host drv_devmng_host drv_seclib_host
endif
