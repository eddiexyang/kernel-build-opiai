LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_id_allocator
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_id_allocator.ko

ifeq ($(PRODUCT_SIDE), device)
include $(BUILD_DEVICE_KO)
LOCAL_DEPEND_KO:=
else
include $(BUILD_HOST_KO)
LOCAL_DEPEND_KO:=drv_seclib_host
endif
