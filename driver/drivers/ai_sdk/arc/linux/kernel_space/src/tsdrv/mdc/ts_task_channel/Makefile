ccflags-y += -Wall -Werror

ifeq ($(TOP_DIR),)
	ifeq ($(TARGET_PRODUCT), mini)
		ifeq ($(TARGET_CHIP_ID),)
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
			EXTRA_CFLAGS += -DCFG_ASCEND_FLORENCE_ASCEND610
		else
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
			EXTRA_CFLAGS += -DCFG_ASCEND_FLORENCE_BS9SX1A
		endif
	endif

	ifeq ($(TARGET_PRODUCT), mdc)
		ifeq ($(TARGET_CHIP_ID),)
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
			EXTRA_CFLAGS += -DCFG_ASCEND_FLORENCE_ASCEND610
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
			EXTRA_CFLAGS += -DCFG_ASCEND_FLORENCE_BS9SX1A
		else
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
		endif
	endif

	ifeq ($(TARGET_PRODUCT), cloud)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
	endif


	KERNEL_DIR :=$(srctree)
	ifeq ($(TARGET_PRODUCT),cloud)
	EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_task_channel/cloud/
	EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/common/
	EXTRA_CFLAGS += -I$(PWD)/inc/soc/cloud_v1/
	EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_drv/ts_drv_device/
	EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_drv/ts_drv_common/
	EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_task_channel/
	endif
	ifeq ($(TARGET_PRODUCT), mini)
		ifeq ($(TARGET_CHIP_ID),)
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_task_channel/mini/
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_drv/ts_drv_device/ascend310/
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_drv/ts_drv_common/
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_task_channel/miniv2/
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_task_channel/
		else
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_task_channel/mini/
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_drv/ts_drv_device/ascend310/
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_drv/ts_drv_common/
		endif
	endif
	ifeq ($(TARGET_PRODUCT), mdc)
		ifeq ($(TARGET_CHIP_ID),)
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_task_channel/mini/
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_task_channel/miniv2/
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_task_channel/
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_task_channel/miniv2/
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_task_channel/
		else
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_task_channel/mini/
		endif
	endif
	EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/event_sched/
	EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc/
	EXTRA_CFLAGS += -I$(PWD)/drivers/npu_inc/
	EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/common/
	EXTRA_CFLAGS += -I$(PWD)/inc/driver/
	EXTRA_CFLAGS += -I$(PWD)/inc/driver/errcode/
	EXTRA_CFLAGS += -I$(PWD)/inc/common/
	EXTRA_CFLAGS += -I$(PWD)/libc_sec/include
	EXTRA_CFLAGS += -I$(PWD)/inc/aicpu/common/

	obj-m  += drv_ts.o
	ifeq ($(TARGET_PRODUCT),cloud)
		drv_ts-objs := ts_drv_init.o
		drv_ts-objs += cloud/ts_drv_sysfs.o
		drv_ts-objs += cloud/hwts_drv_cloud.o
		drv_ts-objs += cloud/ts_aisle_queue.o
	endif
	ifeq ($(TARGET_PRODUCT), mini)
		ifeq ($(TARGET_CHIP_ID),)
		drv_ts-objs := mini/ts_drv_init.o
		drv_ts-objs += mini/hwts_drv_ascend310.o
		drv_ts-objs += mini/ts_aisle_queue.o
		drv_ts-objs += mini/ts_drv_sysfs.o
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
		drv_ts-objs := ts_drv_init.o
		drv_ts-objs += miniv2/ts_drv_sysfs.o
			drv_ts-objs += miniv2/hwts_drv_miniv2.o
			drv_ts-objs += miniv2/ts_aisle_ipc.o
		else
		drv_ts-objs := mini/ts_drv_init.o
		drv_ts-objs += mini/hwts_drv_ascend310.o
		drv_ts-objs += mini/ts_aisle_queue.o
		drv_ts-objs += mini/ts_drv_sysfs.o
		endif
	endif
	ifeq ($(TARGET_PRODUCT), mdc)
		ifeq ($(TARGET_CHIP_ID),)
			drv_ts-objs := mini/ts_drv_init.o
			drv_ts-objs += mini/hwts_drv_ascend310.o
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
			drv_ts-objs := ts_drv_init.o
			drv_ts-objs += miniv2/ts_drv_sysfs.o
			drv_ts-objs += miniv2/bind_core.o
			drv_ts-objs += miniv2/hwts_drv_miniv2.o
			drv_ts-objs += miniv2/ts_aisle_ipc.o
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
			drv_ts-objs := ts_drv_init.o
			drv_ts-objs += miniv2/ts_drv_sysfs.o
			drv_ts-objs += miniv2/bind_core.o
			drv_ts-objs += miniv2/hwts_drv_miniv2.o
			drv_ts-objs += miniv2/ts_aisle_ipc.o
		else
			drv_ts-objs := mini/ts_drv_init.o
			drv_ts-objs += mini/hwts_drv_ascend310.o
		endif
	endif


	ifeq ($(TARGET_PRODUCT),cloud)
		ifeq ($(strip $(TARGET_BUILD_TYPE)),debug)
			EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
		endif
	endif


	ifeq ($(TARGET_PRODUCT), mini)
		ifeq ($(TARGET_CHIP_ID),)
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
			ifeq ($(strip $(TARGET_BUILD_TYPE)),debug)
				EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
			endif
		endif
	endif

	ifeq ($(TARGET_PRODUCT), mdc)
		ifeq ($(TARGET_CHIP_ID),)
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
			ifeq ($(strip $(TARGET_BUILD_TYPE)),debug)
				EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
			endif
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
			ifeq ($(strip $(TARGET_BUILD_TYPE)),debug)
				EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
			endif
		endif
	endif

else #for cmake

ccflags-y += $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-empty-body

ifeq ($(PRODUCT), ascend310p)
	EXTRA_CFLAGS += -DCFG_FEATURE_ID_MAPING
endif
ifneq ($(filter $(PRODUCT),  ascend310p helper310p),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
	EXTRA_CFLAGS += -DCFG_ASCEND_FLORENCE_ASCEND610
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_task_channel/miniv2/
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_task_channel/cloud/
	ifneq ($(filter $(PRODUCT),  helper310p),)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_HELPER
	endif
else ifneq ($(filter $(PRODUCT),  ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_task_channel/miniv3/
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/mini_v3
else ifneq ($(filter $(PRODUCT),  ascend610 ascend610Liteesl ascend610Lite bs9sx1a),)
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_device/ascend610/
    ifneq ($(filter $(PRODUCT),  ascend610 ascend610Liteesl ascend610Lite),)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
		EXTRA_CFLAGS += -DSUPPORT_FLEXIBLE_BIND_CORE
		EXTRA_CFLAGS += -DCFG_ASCEND_FLORENCE_ASCEND610
		ifneq ($(filter $(PRODUCT),  ascend610Liteesl ascend610Lite),)
			EXTRA_CFLAGS += -DCFG_SOC_MDC_V51_LITE
		endif
    else ifeq ($(PRODUCT),bs9sx1a)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
		EXTRA_CFLAGS += -DSUPPORT_FLEXIBLE_BIND_CORE
		EXTRA_CFLAGS += -DCFG_ASCEND_FLORENCE_BS9SX1A
    else
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
    endif
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_task_channel/miniv2/
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_task_channel/cloud/
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common/
else ifneq ($(filter $(PRODUCT),  ascend910 ascend910B ascend920esl),)
	ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl),)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/cloud_v2/
	endif
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_task_channel/cloud/
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/cloud_v1/
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common/
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_device/
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_common/
else
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_task_channel/mini/
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_device/ascend310/
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_common/
endif

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_task_channel/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/
EXTRA_CFLAGS += -I${DRIVER_USER_DIR}/src/event_sched/
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver/
EXTRA_CFLAGS += -I$(TOP_DIR)/ace/csruntime/inc/tsch
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver/errcode/
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/common/
EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/aicpu/common/

obj-m  += drv_ts.o
ifeq ($(PRODUCT),ascend910)
	drv_ts-objs := ts_drv_init.o
	drv_ts-objs += cloud/ts_drv_sysfs.o
	drv_ts-objs += cloud/hwts_drv_cloud.o
	drv_ts-objs += cloud/ts_aisle_queue.o
else ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
    drv_ts-objs := ts_drv_init.o
    drv_ts-objs += miniv3/hwts_drv_miniv3.o
else ifneq ($(filter $(PRODUCT),ascend310p helper310p),)
	drv_ts-objs := ts_drv_init.o
	drv_ts-objs += miniv2/ts_drv_sysfs.o
	drv_ts-objs += miniv2/hwts_drv_miniv2.o
	drv_ts-objs += miniv2/ts_aisle_ipc.o
else ifneq ($(filter $(PRODUCT),ascend610 ascend610Liteesl ascend610Lite bs9sx1a),)
	drv_ts-objs := ts_drv_init.o
	drv_ts-objs += miniv2/ts_drv_sysfs.o
	drv_ts-objs += miniv2/bind_core.o
	drv_ts-objs += miniv2/hwts_drv_miniv2.o
	drv_ts-objs += miniv2/ts_aisle_ipc.o
else ifneq ($(filter $(PRODUCT),ascend310 ascend310rc),)
	drv_ts-objs := mini/ts_drv_init.o
	drv_ts-objs += mini/hwts_drv_ascend310.o
	drv_ts-objs += mini/ts_aisle_queue.o
	drv_ts-objs += mini/ts_drv_sysfs.o
else ifneq ($(filter $(PRODUCT),ascend910B ascend920esl),)
	drv_ts-objs := ts_drv_init.o
	drv_ts-objs += cloudv2/hwts_drv_cloudv2.o
endif

ifeq ($(filter $(PRODUCT),  ascend310 ascend310rc ascend610 ascend610Liteesl ascend610Lite bs9sx1a helper310p ascend310p ascend910),)
    ifeq ($(strip $(TARGET_BUILD_TYPE)),debug)
		EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
    endif
endif

endif
