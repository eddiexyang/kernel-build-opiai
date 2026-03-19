LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_nvme_chan
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_nvme_chan.ko

ifeq ($(PRODUCT_SIDE), device)
LOCAL_DEPEND_KO := ascend_trs_id_allocator
include $(BUILD_DEVICE_KO)
else
LOCAL_DEPEND_KO := ascend_trs_id_allocator drv_seclib_host
include $(BUILD_HOST_KO)
endif
