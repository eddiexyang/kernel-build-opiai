LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := hns3

LOCAL_KO_SRC_FOLDER := kernel/linux-source/drivers/net/ethernet/hisilicon/hns3
LOCAL_DEPEND_KO := hnae3

LOCAL_INSTALLED_KO_FILES := hns3.ko

include $(BUILD_DEVICE_KO)
