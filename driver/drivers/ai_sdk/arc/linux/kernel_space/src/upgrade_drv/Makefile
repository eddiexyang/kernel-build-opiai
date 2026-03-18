ifeq ($(TOP_DIR),) # Not used

else # for cmake and control cpu open
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common
    EXTRA_CFLAGS += -I$(DRIVER_CHIP_KERNEL_DIR)/src/nor_flash
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
    EXTRA_CFLAGS += -I$(TOP_DIR)/tee/inc
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/firmware/hiss/inc
    EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox

ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/upgrade_drv/ascd310
else
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/upgrade_drv/common
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/upgrade_drv/ascd610
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/usr_cfg_drv

    ifeq ($(PRODUCT),ascend310p)
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/upgrade_drv/ascd310p
    endif

    ifeq ($(PRODUCT),helper310p)
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/upgrade_drv/helper310p
    endif

    ifneq ($(filter $(PRODUCT), ascend310B ascend310Brc),)
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/upgrade_drv/ascd310b
    endif

    ifeq ($(PRODUCT),ascend910)
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/upgrade_drv/ascd910
    endif

    ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/upgrade_drv/ascd910b
    endif

    ifneq ($(filter $(PRODUCT), ascend310p helper310p),)
        EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv2
        EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/miniv2
    else ifneq ($(filter $(PRODUCT), ascend310B ascend310Brc),)
        EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv3
    else
        EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/cloud
        EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/cloud
    endif

    ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl as31xm1),)
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/faultmng/inc
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/faultmng/msg
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/pkicms/pkicms
        EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/include
        EXTRA_CFLAGS += -I$(TOP_DIR)/common/cmscbb/include
    endif
endif

ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
else ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl as31xm1),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51 -DSUPPORT_TEE_SECURE_UPGRADE -DLINUX_KERNEL_BUILD
    EXTRA_CFLAGS += -DCFG_FEATURE_UPGRADE_CRL
    ifeq ($(PRODUCT),as31xm1)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
        EXTRA_CFLAGS += -DCFG_FEATURE_SIGN_VERSION_1
    endif
else ifeq ($(PRODUCT),ascend310p)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DSUPPORT_TEE_SECURE_UPGRADE
    EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
else ifeq ($(PRODUCT),helper310p)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_HELPER
    EXTRA_CFLAGS += -DSUPPORT_TEE_SECURE_UPGRADE
    EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
    EXTRA_CFLAGS += -DCFG_FEATURE_UPGRADE_CRL
else ifneq ($(filter $(PRODUCT), ascend310B ascend310Brc),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
    EXTRA_CFLAGS += -DSUPPORT_TEE_SECURE_UPGRADE
    EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
    EXTRA_CFLAGS += -DCFG_FEATURE_ROLL_BACK
    ifeq ($(PRODUCT),ascend310Brc)
        EXTRA_CFLAGS += -DCFG_FEATURE_RC_MODE
    endif
else ifeq ($(PRODUCT),ascend910)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
else ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
    EXTRA_CFLAGS += -DSUPPORT_TEE_SECURE_UPGRADE
endif


    obj-m += drv_upgrade.o

ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
    drv_upgrade-objs := ascd310/dev_upgrade_core.o ascd310/dev_upgrade_adapt.o
else
    ccflags-y += -Wall -Werror
    drv_upgrade-objs := common/dev_upgrade_core.o common/dev_upgrade_adapt.o
    ifneq ($(filter $(PRODUCT), ascend310B ascend310Brc),)
        drv_upgrade-objs += common/dev_upgrade_crc.o
    endif
endif

ifneq ($(filter $(PRODUCT), ascend310p helper310p ascend310B ascend310Brc ascend910B ascend920esl ascend920emu),)
    drv_upgrade-objs += ascd610/dev_upgrade_sec.o
endif

ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl as31xm1),)
    drv_upgrade-objs += ascd610/dev_upgrade_sec.o ascd610/dev_upgrade_mdccore.o ascd610/dev_upgrade_crl.o
    drv_upgrade-objs += ascd610/dev_upgrade_ufs.o ascd610/dev_upgrade_ssd.o
endif

ifneq ($(filter $(PRODUCT), ascend610Lite ascend610Liteesl),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_LITE_ESL
endif

endif
