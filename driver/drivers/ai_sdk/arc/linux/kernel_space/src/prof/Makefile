ifeq ($(PRODUCT),ascend910)
EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
endif

ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
endif

ifeq ($(fpga),true)
EXTRA_CFLAGS += -DTEMP_FPGA_B100
endif

ifeq ($(PRODUCT),ascend310p)
EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
endif

ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
endif

ifneq ($(filter $(PRODUCT), ascend310Brc ascend310Brcesl ascend310Brcemu),)
    EXTRA_CFLAGS += -DCFG_FEATURE_RC_MODE
    EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
    EXTRA_CFLAGS += -DCFG_FEATURE_NO_SUPPORT_NODE_DIVIDE
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/prof/prof_virt
endif

ifeq ($(PRODUCT), ascend610)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
endif

ifneq ($(filter $(PRODUCT), as31xm1),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
endif

ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
endif

ifneq ($(filter $(PRODUCT), ascend610Liteesl ascend610Lite),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
    EXTRA_CFLAGS += -DCFG_SOC_MDC_V51_LITE
endif

ifeq ($(PRODUCT), bs9sx1a)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
endif

ifeq ($(PRODUCT), helper310p)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_HELPER
    EXTRA_CFLAGS += -DCFG_FEATURE_KTHREAD_BIND_CORE
endif

EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/prof/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/prof/prof_inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/prof/prof_mem_sample
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc/
ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_common/tsdrv_dev
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_common
else
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common/tsdrv_dev
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common
endif

ifeq ($(PRODUCT),ascend910)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/prof/ascd910
    EXTRA_CFLAGS += -DCFG_FEATURE_KTHREAD_BIND_CORE
    EXTRA_CFLAGS += -DCFG_FEATURE_HOST_COLLECTION
else ifneq ($(filter $(PRODUCT), ascend310p ascend610 ascend610Liteesl ascend610Lite bs9sx1a helper310p ),)
    ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_device
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_platform/ts_platform_device/ascend610
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_device/ascend610
    else
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend610
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/ascend610
    endif
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/prof/ascd610
    ifneq ($(filter $(PRODUCT), ascend310p),)
        EXTRA_CFLAGS += -DCFG_FEATURE_KTHREAD_BIND_CORE
        EXTRA_CFLAGS += -DCFG_FEATURE_HOST_COLLECTION
        EXTRA_CFLAGS += -DCFG_FEATURE_PROF_HWTS_THROUGH
    endif
else ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/ascend310
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend310
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/prof/ascd310
    ifneq ($(filter $(PRODUCT), ascend310),)
        EXTRA_CFLAGS += -DCFG_FEATURE_HOST_COLLECTION
    endif
else ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Brc ascend310Brcesl ascend310Besl),)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend310
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/prof/ascd310B
    EXTRA_CFLAGS += -DCFG_FEATURE_KTHREAD_BIND_CORE
    EXTRA_CFLAGS += -DCFG_FEATURE_STARS
    ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl),)
        EXTRA_CFLAGS += -DCFG_FEATURE_HOST_COLLECTION
    endif
else ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/prof/ascd910B
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/prof/prof_virt
    EXTRA_CFLAGS += -DCFG_FEATURE_KTHREAD_BIND_CORE
    EXTRA_CFLAGS += -DCFG_FEATURE_HOST_COLLECTION
    EXTRA_CFLAGS += -DCFG_FEATURE_STARS
    EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
endif

ccflags-y += -Wall -Werror
ccflags-y += $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char  -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-empty-body

ifneq ($(filter $(PRODUCT), ascend910 ascend310p ascend610 ascend610Liteesl ascend610Lite bs9sx1a ascend310 ascend310rc ascend310B as31xm1 ascend310Brc ascend310Brcesl ascend310Besl as31xm1 ascend910B ascend920esl ascend920emu helper310p ),)
    obj-m += drv_prof.o
else
    obj-$(CONFIG_NPU_PROF_DEVDRV)   += drv_prof.o
endif

drv_prof-objs := prof_drv_dev.o prof_ts.o prof_peripheral.o
drv_prof-objs += prof_mem_sample/prof_mem_sample_by_dev.o prof_mem_sample/prof_mem_sample_by_proc.o

ifeq ($(PRODUCT),ascend910)
    drv_prof-objs += ascd910/prof.o prof_drv_hdc_dev.o
else ifneq ($(filter $(PRODUCT), ascend310p ascend610 ascend610Liteesl ascend610Lite bs9sx1a helper310p),)
    drv_prof-objs += ascd610/prof.o
    ifneq ($(filter $(PRODUCT), ascend310p),)
        drv_prof-objs += ascd610/prof_hwts_init.o
        drv_prof-objs += prof_drv_hdc_dev.o
    endif
    ifneq ($(filter $(PRODUCT), ascend610 ascend610Liteesl ascend610Lite bs9sx1a),)
        drv_prof-objs += ascd610/drv_cpu_type.o
    endif
else ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
    drv_prof-objs += ascd310/prof.o
    ifneq ($(filter $(PRODUCT), ascend310),)
        drv_prof-objs += prof_drv_hdc_dev.o
    endif
else ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Brc ascend310Brcesl ascend310Besl),)
    drv_prof-objs += ascd310B/prof.o
    ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl),)
        drv_prof-objs += prof_drv_hdc_dev.o
    endif

    ifneq ($(filter $(PRODUCT), ascend310Brc ascend310Brcesl),)
        drv_prof-objs += prof_virt/dvprof_init.o
    endif

    #for CFG_TRS_REFACTOR_FEATURE
    EXTRA_CFLAGS += -DCFG_TRS_REFACTOR_FEATURE
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/inc
    drv_prof-objs += prof_chan.o
else ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
    drv_prof-objs += ascd910B/prof.o prof_drv_hdc_dev.o prof_virt/dvprof_init.o

    #for CFG_TRS_REFACTOR_FEATURE
    EXTRA_CFLAGS += -DCFG_TRS_REFACTOR_FEATURE
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/inc
    drv_prof-objs += prof_chan.o
endif
