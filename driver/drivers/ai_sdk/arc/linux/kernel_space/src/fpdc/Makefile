ifeq ($(TOP_DIR),)

# for cmake
else
    KERNEL_DIR :=$(srctree)

    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/config
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc/ras/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc/include
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common

    ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef $(WDATE_TIME)
    ccflags-y += -Wstack-usage=2048 -Wno-unused-parameter -Wno-sign-compare
    ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
    	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC
    	EXTRA_CFLAGS += -DDRV_SUPPORT_SYSCOUNT
    	ifeq ($(MAKE_PRODUCT),bs9sx1a)
    		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_BS9SX1A
    	endif
    endif

    ifneq ($(filter $(PRODUCT), ascend310B ascend310Brc as31xm1),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
    endif

    obj-m += drv_fpdc.o
    drv_fpdc-objs += fpdc_main.o
    drv_fpdc-objs += distribute/fpdc_distribute_notify.o
    drv_fpdc-objs += receiver/fpdc_ras_receiver.o
    drv_fpdc-objs += log_restrain/log_restrain.o
    drv_fpdc-objs += ras/ras_int_unmask.o
endif

ifeq ($(strip $(TARGET_BUILD_TYPE)),debug)
        EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif
