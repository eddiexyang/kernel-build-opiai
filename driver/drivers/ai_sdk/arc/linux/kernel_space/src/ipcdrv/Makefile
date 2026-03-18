ifeq ($(TOP_DIR),)

CUR_MAKEFILE_PATH := $(strip \
            $(eval LOCAL_MODULE_MAKEFILE := $$(lastword $$(MAKEFILE_LIST))) \
            $(patsubst %/,%, $(dir $(LOCAL_MODULE_MAKEFILE))) \
            )

ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers

ifeq ($(TARGET_PRODUCT),cloud)
EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
endif

ifeq ($(TARGET_PRODUCT), mini)
	ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
	    EXTRA_CFLAGS += -DIPC_ACPI_SUPPORT
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_ESL_FPGA
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
	else
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
	endif
endif

ifeq ($(TARGET_PRODUCT), mdc)
	EXTRA_CFLAGS += -DIPC_ACPI_SUPPORT
	ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2_MDC
	else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_V2))
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2_MDC
	else
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
	endif
endif

EXTRA_CFLAGS += -I$(CUR_MAKEFILE_PATH)/../../inc

ifeq ($(TARGET_PRODUCT),cloud)
	EXTRA_CFLAGS += -I$(CUR_MAKEFILE_PATH)/cloud
	obj-m += ipc_drv.o
	ipc_drv-objs := hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o
	ipc_drv-objs += cloud/hisi_rproc_adapt.o
endif

ifeq ($(TARGET_PRODUCT), mini)
	ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
		EXTRA_CFLAGS += -I$(CUR_MAKEFILE_PATH)/mdc
		obj-m += ipc_drv.o
		ipc_drv-objs := hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o
		ipc_drv-objs += mdc/hisi_rproc_adapt.o
	else
		EXTRA_CFLAGS += -I$(CUR_MAKEFILE_PATH)/mini
		obj-m += ipc_drv.o
		ipc_drv-objs := hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o
		ipc_drv-objs += mini/hisi_rproc_adapt.o
	endif
endif

ifeq ($(TARGET_PRODUCT), mdc)
	ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
		EXTRA_CFLAGS += -I$(CUR_MAKEFILE_PATH)/mdc
		ifeq ($(linux_version), linux-4.14-media)
			ccflags-y += -Idrivers/hisi/ipcdrv
			ccflags-y += -Idrivers/hisi/ipcdrv/mdc

			obj-y += hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o mdc/hisi_rproc_adapt.o drv_cpu_type.o
			obj-m += ipc_drv.o
			ipc_drv-objs += hisi_mailbox_asm.o
		else
			obj-m += ipc_drv.o
			ipc_drv-objs += hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o drv_cpu_type.o
			ipc_drv-objs += mdc/hisi_rproc_adapt.o
		endif
	else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_V2))
                EXTRA_CFLAGS += -I$(CUR_MAKEFILE_PATH)/mdc
                ifeq ($(linux_version), linux-4.14-media)
                        ccflags-y += -Idrivers/hisi/ipcdrv
                        ccflags-y += -Idrivers/hisi/ipcdrv/mdc

                        obj-y += hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o mdc/hisi_rproc_adapt.o drv_cpu_type.o
                        obj-m += ipc_drv.o
                        ipc_drv-objs += hisi_mailbox_asm.o
                else
                        obj-m += ipc_drv.o
                        ipc_drv-objs += hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o drv_cpu_type.o
                        ipc_drv-objs += mdc/hisi_rproc_adapt.o
                endif

	else
		EXTRA_CFLAGS += -I$(CUR_MAKEFILE_PATH)/mini
		obj-m += ipc_drv.o
		ipc_drv-objs := hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o
		ipc_drv-objs += mini/hisi_rproc_adapt.o
	endif
endif
else

ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers

EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/inc
ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
	EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/ipcdrv/cloud_v2
	obj-m += ipc_drv.o
	ipc_drv-objs := hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o drv_snapshot.o
	ipc_drv-objs += cloud_v2/hisi_rproc_adapt.o
else ifneq ($(filter $(PRODUCT),  ascend910),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
	EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/ipcdrv/cloud
	obj-m += ipc_drv.o
	ipc_drv-objs := hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o drv_snapshot.o
	ipc_drv-objs += cloud/hisi_rproc_adapt.o
else ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2_MDC
	EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/ipcdrv/mdc
	ifeq ($(linux_version), linux-4.14-media)
			ccflags-y += -Idrivers/hisi/ipcdrv
			ccflags-y += -Idrivers/hisi/ipcdrv/mdc

			obj-y += hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o mdc/hisi_rproc_adapt.o drv_snapshot.o drv_cpu_type.o
			obj-m += ipc_drv.o
			ipc_drv-objs += hisi_mailbox_asm.o
	else
			EXTRA_CFLAGS += -DSNAPSHOT_ENABLE
			obj-m += ipc_drv.o
			ipc_drv-objs += hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o drv_snapshot.o drv_cpu_type.o
			ipc_drv-objs += mdc/hisi_rproc_adapt.o
	endif
else ifneq ($(filter $(PRODUCT),  ascend310p helper310p),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
	EXTRA_CFLAGS += -DIPC_ACPI_SUPPORT
	EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/ipcdrv/mdc
	obj-m += ipc_drv.o
	ipc_drv-objs := hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o drv_snapshot.o
	ipc_drv-objs += mdc/hisi_rproc_adapt.o
else ifneq ($(filter $(PRODUCT),  ascend310B ascend310Brc as31xm1),)
        ifneq ($(filter $(PRODUCT),  ascend310Brc as31xm1),)
            EXTRA_CFLAGS += -DSNAPSHOT_ENABLE
            EXTRA_CFLAGS += -DNOT_SUPPORT_OSNAME
        endif
        ifneq ($(filter $(PRODUCT),  as31xm1),)
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
        endif
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
	EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/ipcdrv/miniv3
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
	obj-m += ipc_drv.o
	ipc_drv-objs := hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o drv_snapshot.o
	ipc_drv-objs += miniv3/hisi_rproc_adapt.o
else
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
	EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/ipcdrv/mini
	obj-m += ipc_drv.o
	ipc_drv-objs := hisi_mailbox.o hisi_rproc.o hisi_mailbox_dev.o hisi_mailbox_asm.o drv_snapshot.o
	ipc_drv-objs += mini/hisi_rproc_adapt.o
endif

ifneq ($(filter $(PRODUCT), ascend310p),)
	EXTRA_CFLAGS += -DCFG_EDP_MSG
endif
ifneq ($(filter $(PRODUCT), helper310p),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_HELPER
endif

endif
