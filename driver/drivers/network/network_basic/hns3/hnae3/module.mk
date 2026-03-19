LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := hnae3

LOCAL_KO_SRC_FOLDER := kernel/linux-source/drivers/net/ethernet/hisilicon/hns3

LOCAL_INSTALLED_KO_FILES := hnae3.ko

include $(BUILD_DEVICE_KO)
