ifeq ($(TOP_DIR),)
KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/user_cfg_drv
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/nor_flash
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/common
EXTRA_CFLAGS += -I$(PWD)/inc/driver
EXTRA_CFLAGS += -I$(PWD)/inc
EXTRA_CFLAGS += -I$(PWD)/inc/ascend_platform
EXTRA_CFLAGS += -I$(PWD)/drivers/npu_inc
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/drv_devmng/drv_devmng_device/ascend910
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_core/device_manager/device_manager_inc
EXTRA_CFLAGS += -I$(PWD)/drivers/firmware/hiss/inc

obj-m   += drv_user_cfg.o
drv_user_cfg-objs := dev_user_cfg.o user_cfg_management.o drv_whitelist.o
ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wundef $(WDATE_TIME)
ccflags-y += -Wno-unused-parameter -Wno-sign-compare

ifeq ($(TARGET_PRODUCT),cloud)
    EXTRA_CFLAGS += -I$(PWD)/config/user_config/cloud
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
else
    ifeq ($(TARGET_PRODUCT),mini))
        ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
            EXTRA_CFLAGS += -I$(PWD)/config/user_config/miniv2
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
            EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
            EXTRA_CFLAGS += -DCFG_FEATURE_CPU_NUM_CFG
            EXTRA_CFLAGS += -DCFG_FEATURE_GET_DEV_NUM_FROM_ASCEND_CTL
        else
            EXTRA_CFLAGS += -I$(PWD)/config/user_config/mini
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
        endif
    endif
    ifeq ($(TARGET_PRODUCT),mdc))
        EXTRA_CFLAGS += -DCFG_FEATURE_CPU_NUM_CFG
        ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
            EXTRA_CFLAGS += -I$(PWD)/config/user_config/miniv2
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
            EXTRA_CFLAGS += -DCFG_FEATURE_BOARD_ID_EXPAND
            ifneq ($(findstring $(DRIVER_CORE_TYPE)$(COMPILER_TYPE), llvm),)
                EXTRA_CFLAGS += -DCFG_LLVM_BUILD
            endif
        else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
            EXTRA_CFLAGS += -I$(PWD)/config/user_config/miniv2
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
            EXTRA_CFLAGS += -DCFG_FEATURE_BOARD_ID_EXPAND
            ifneq ($(findstring $(DRIVER_CORE_TYPE)$(COMPILER_TYPE), llvm),)
                EXTRA_CFLAGS += -DCFG_LLVM_BUILD
            endif
        endif
    endif
endif
else

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/ascend_platform
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/usr_cfg_drv
EXTRA_CFLAGS += -I$(DRIVER_CHIP_KERNEL_DIR)/src/nor_flash
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
EXTRA_CFLAGS += -I$(TOP_DIR)/inc
EXTRA_CFLAGS += -I$(TOP_DIR)/tee/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/firmware/hiss/inc

include $(FEATURE_MK_PATH)
EXTRA_CFLAGS += $(CONFIG_DEFINES)

obj-m   += drv_user_cfg.o
drv_user_cfg-objs := dev_user_cfg.o user_cfg_management.o drv_whitelist.o
ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wundef $(WDATE_TIME)
ccflags-y += -Wno-unused-parameter -Wno-sign-compare

ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

ifeq ($(PRODUCT),ascend910)
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/cloud
    EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/cloud
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
    EXTRA_CFLAGS += -DCFG_FEATURE_PKCS_SIGN
    EXTRA_CFLAGS += -DCFG_FEATURE_PROCESS_WHITELIST
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_MAC_INFO
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_HCCP
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_DYNAMIC_ITEM
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_MULTI_BLOCK
else ifeq ($(PRODUCT),ascend310p)
    EXTRA_CFLAGS += -DCFG_FEATURE_CPU_NUM_CFG
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv2
    EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/miniv2
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_FEATURE_GET_DEV_NUM_FROM_ASCEND_CTL
    EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
    EXTRA_CFLAGS += -DCFG_FEATURE_PKCS_SIGN
    EXTRA_CFLAGS += -DCFG_FEATURE_PROCESS_WHITELIST
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_DYNAMIC_ITEM
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_SEC_FLASH
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_MULTI_BLOCK
else ifeq ($(PRODUCT),helper310p)
    EXTRA_CFLAGS += -DCFG_FEATURE_CPU_NUM_CFG
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv2
    EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/miniv2
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_FEATURE_GET_DEV_NUM_FROM_ASCEND_CTL
    EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
    EXTRA_CFLAGS += -DCFG_FEATURE_PROCESS_WHITELIST
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_DYNAMIC_ITEM
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_SEC_FLASH
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_MULTI_BLOCK
    EXTRA_CFLAGS += -DCFG_FEATURE_CUSTOM_OS
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_HELPER
else ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv3
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_FEATURE_GET_DEV_NUM_FROM_ASCEND_CTL
    EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
    EXTRA_CFLAGS += -DCFG_FEATURE_PROCESS_WHITELIST
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_DYNAMIC_ITEM
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_SEC_FLASH
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_MULTI_BLOCK
    ifneq ($(filter $(PRODUCT), ascend310Brc),)
        EXTRA_CFLAGS += -DCFG_FEATURE_CONTAINER
    endif
else ifeq ($(PRODUCT), as31xm1)
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv3
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_DYNAMIC_ITEM
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_SEC_FLASH
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_MULTI_BLOCK
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_COMPACT_SPACE
else ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
    EXTRA_CFLAGS += -DCFG_FEATURE_CPU_NUM_CFG
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv2
    EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/miniv2
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_DYNAMIC_ITEM
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_SEC_FLASH
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_MULTI_BLOCK
    EXTRA_CFLAGS += -DCFG_FEATURE_BOARD_ID_EXPAND
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_COMPACT_SPACE
    ifneq ($(filter $(PRODUCT),  ascend610Liteesl),)
        EXTRA_CFLAGS += -DCFG_SOC_MDC_V51_LITE
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_LITE_ESL
    endif
    ifneq ($(filter $(PRODUCT),  ascend610Lite),)
        EXTRA_CFLAGS += -DCFG_SOC_MDC_V51_LITE
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_LITE_ESL
    endif
    ifneq ($(findstring $(DRIVER_CORE_TYPE)$(COMPILER_TYPE), llvm),)
        EXTRA_CFLAGS += -DCFG_LLVM_BUILD
    endif
else ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl ascend920emu),)
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/cloud
    EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/cloud
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
    EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_MAC_INFO
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_HCCP
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_DYNAMIC_ITEM
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_SEC_FLASH
    EXTRA_CFLAGS += -DCFG_USER_CFG_SUPPORT_MULTI_BLOCK
    EXTRA_CFLAGS += -DCFG_FEATURE_PROCESS_WHITELIST
else
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/mini
    EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/mini
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
    EXTRA_CFLAGS += -DCFG_FEATURE_PROCESS_WHITELIST
    ifneq ($(findstring $(PRODUCT), ascend310rc),)
        EXTRA_CFLAGS += -DCFG_FEATURE_CUSTOM_OS
    endif
endif
endif
