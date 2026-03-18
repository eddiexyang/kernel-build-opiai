ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align -Wextra
ccflags-y += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-sign-compare

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

ifeq ($(TOP_DIR),)
    ifeq ($(TARGET_PRODUCT),cloud)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    endif
    ifeq ($(TARGET_PRODUCT),mini)
        ifeq ($(TARGET_CHIP_ID), )
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
        else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
        else
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
        endif
    endif
    ifeq ($(TARGET_PRODUCT),mdc)
        ifeq ($(TARGET_CHIP_ID), )
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
        else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
        else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
        else
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
        endif
    endif

    ifeq ($(TARGET_BOARD_PLATFORM),kirin990)
        ifeq ($(chip_type),es)
            CHIP_VERSION=hi3690_es
        else
            CHIP_VERSION=hi3690
        endif
    endif
    $(warning chip version $(CHIP_VERSION))

    KERNEL_DIR :=$(srctree)

    ifeq ($(TARGET_PRODUCT),cloud)
        EXTRA_CFLAGS += -I$(PWD)/inc/driver
        EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc
        EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_drv/ts_drv_device/
        EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_drv/ts_drv_common/
        EXTRA_CFLAGS += -I$(PWD)/drivers/dev_core/device_manager/device_manager_inc/
        EXTRA_CFLAGS += -I$(PWD)/drivers/npu_inc/
        EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_platform/ts_platform_device/
        EXTRA_CFLAGS += -I$(PWD)/src/ipcdrv
        EXTRA_CFLAGS += -I$(PWD)/src/drv_devmng/drv_devmng_device/ascend910
        EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_drv_res/ascd910/
        EXTRA_CFLAGS += -I$(PWD)/src/hdcdrv/hdc_host/
        EXTRA_CFLAGS += -I$(PWD)/libc_sec/include/
    else ifeq ($(TARGET_PRODUCT),mini)
        EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc
        EXTRA_CFLAGS += -I$(PWD)/drivers/dev_core/device_manager/device_manager_inc/
        EXTRA_CFLAGS += -I$(PWD)/drivers/npu_inc/
        EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_platform/ts_platform_device/
        EXTRA_CFLAGS += -I$(PWD)/src/ipcdrv
        EXTRA_CFLAGS += -I$(PWD)/src/hdcdrv/hdc_host/
        EXTRA_CFLAGS += -I$(PWD)/libc_sec/include/
        ifeq ($(TARGET_CHIP_ID), )
            EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_drv_res/ascd310/
            EXTRA_CFLAGS += -I$(PWD)/src/drv_devmng/drv_devmng_device/ascend310
            EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_drv/ts_drv_device/ascend310
            EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_drv/ts_drv_common
        else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
            EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_drv_res/ascd310p/
            EXTRA_CFLAGS += -I$(PWD)/src/drv_devmng/drv_devmng_device/ascend910
            EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_drv/ts_drv_device/
            EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_drv/ts_drv_common/
        else
            EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_drv_res/ascd310/
            EXTRA_CFLAGS += -I$(PWD)/src/drv_devmng/drv_devmng_device/ascend310
            EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_drv/ts_drv_device/ascend310
            EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_drv/ts_drv_common
        endif
    else ifeq ($(TARGET_PRODUCT),mdc)
        EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc
        EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/mdc/ts_drv/ts_drv_device/
        EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/mdc/ts_drv/ts_drv_common/
        EXTRA_CFLAGS += -I$(PWD)/drivers/dev_core/device_manager/device_manager_inc/
        EXTRA_CFLAGS += -I$(PWD)/drivers/npu_inc/
        EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/mdc/ts_platform/ts_platform_device/
        EXTRA_CFLAGS += -I$(PWD)/src/ipcdrv
        EXTRA_CFLAGS += -I$(PWD)/src/drv_devmng/drv_devmng_device/ascend910
        EXTRA_CFLAGS += -I$(PWD)/src/hdcdrv/hdc_host/
        EXTRA_CFLAGS += -I$(PWD)/libc_sec/include/
        ifeq ($(TARGET_CHIP_ID), )
            EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_drv_res/ascd310/
        else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
            EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_drv_res/ascd610/
            EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_xpc/
            EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_bind_core/
        else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
            EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_drv_res/ascd610/
            EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_xpc/
            EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_bind_core/
        else
            EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_drv_res/ascd310/
        endif
    else
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/inc
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/inc/comm
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/src/tsdrv/ts_drv/ts_drv_device/
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/src/tsdrv/ts_drv/ts_drv_common/
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_core/device_manager/device_manager_inc/
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/src/tsdrv/ts_platform/ts_platform_device/
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/src/ipcdrv
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/src/drv_devmng/drv_devmng_device
        EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_drv_res/ascd310/
        EXTRA_CFLAGS += -I$(PWD)/src/hdcdrv/hdc_host/
        EXTRA_CFLAGS += -I$(PWD)/libc_sec/include/
    endif

    EXTRA_CFLAGS += -I$(PWD)/inc/toolchain/bbox/
    EXTRA_CFLAGS += -I$(PWD)/inc/toolchain/bbox/device/
    EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_drv_conn/
    EXTRA_CFLAGS += -I$(PWD)/src/logdrv/log_drv_res/
    EXTRA_CFLAGS += -I$(PWD)/src/logdrv/
    EXTRA_CFLAGS += -I$(PWD)/../../../../inc/driver

    ifeq ($(TARGET_PRODUCT),cloud)
        EXTRA_CFLAGS += -I$(PWD)/inc/soc/cloud_v1
        EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_platform/ts_platform_device/ascend910
        obj-m   += drv_log.o
        drv_log-objs := log_drv_dev.o   log_drv_conn/log_drv_ipc.o log_drv_conn/log_drv_shm.o log_drv_conn/log_drv_sqcq.o log_drv_res/log_drv_res_common.o
    else ifeq ($(TARGET_PRODUCT),mini)
        ifeq ($(TARGET_CHIP_ID), )
            EXTRA_CFLAGS += -I$(PWD)/inc/soc/mini
            EXTRA_CFLAGS += -I$(PWD)/inc/soc/mini_v1

            EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_platform/ts_platform_device/ascend310
            obj-m   += drv_log.o
            drv_log-objs := log_drv_dev.o   log_drv_conn/log_drv_ipc.o log_drv_conn/log_drv_shm.o log_drv_conn/log_drv_sqcq.o log_drv_res/log_drv_res_common.o
        else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
            EXTRA_CFLAGS += -I$(PWD)/inc/soc/hi1910p
            EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_platform/ts_platform_device/ascend610
            obj-m   += drv_log.o
            drv_log-objs := log_drv_dev.o   log_drv_conn/log_drv_ipc.o log_drv_conn/log_drv_shm.o log_drv_conn/log_drv_sqcq.o log_drv_res/log_drv_res_common.o
        else
            EXTRA_CFLAGS += -I$(PWD)/inc/soc/mini
            EXTRA_CFLAGS += -I$(PWD)/inc/soc/mini_v1
            EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_platform/ts_platform_device/ascend310
            obj-m   += drv_log.o
            drv_log-objs := log_drv_dev.o   log_drv_conn/log_drv_ipc.o log_drv_conn/log_drv_shm.o log_drv_conn/log_drv_sqcq.o log_drv_res/log_drv_res_common.o
        endif
    else ifeq ($(TARGET_PRODUCT),mdc)
        ifeq ($(TARGET_CHIP_ID), )
            EXTRA_CFLAGS += -I$(PWD)/inc/soc/mini
            EXTRA_CFLAGS += -I$(PWD)/inc/soc/mini_v1
            EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_platform/ts_platform_device/ascend310
            obj-m   += drv_log.o
            drv_log-objs := log_drv_dev.o   log_drv_conn/log_drv_ipc.o log_drv_conn/log_drv_shm.o log_drv_conn/log_drv_sqcq.o log_drv_res/log_drv_res_common.o
        else ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
	    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
            EXTRA_CFLAGS += -I$(PWD)/inc/soc/hi1910p
            EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/mdc/ts_platform/ts_platform_device/ascend610
	    EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/mdc/ts_drv/ts_drv_device/ascend610
            obj-m   += drv_log.o
            drv_log-objs := log_drv_dev.o   log_drv_conn/log_drv_ipc.o log_drv_conn/log_drv_shm.o log_drv_conn/log_drv_sqcq.o log_drv_res/log_drv_res_common.o log_xpc/log_xpc.o log_xpc/log_xpc_server.o log_bind_core/drv_cpu_type.o
        else
            EXTRA_CFLAGS += -I$(PWD)/inc/soc/mini
            EXTRA_CFLAGS += -I$(PWD)/inc/soc/mini_v1
            EXTRA_CFLAGS += -I$(PWD)/src/tsdrv/ts_platform/ts_platform_device/ascend310
            obj-m   += drv_log.o
            drv_log-objs := log_drv_dev.o   log_drv_conn/log_drv_ipc.o log_drv_conn/log_drv_shm.o log_drv_conn/log_drv_sqcq.o log_drv_res/log_drv_res_common.o
        endif
    else
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/soc/hi1910
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/src/tsdrv/ts_platform/ts_platform_device/ascend310
        obj-$(CONFIG_NPU_LOG_DEVDRV)   += drv_log.o
        drv_log-objs := log_drv_dev.o   log_drv_conn/log_drv_ipc.o log_drv_conn/log_drv_shm.o log_drv_conn/log_drv_sqcq.o log_drv_res/log_drv_res_common.o
    endif

else
    ifneq ($(filter $(PRODUCT),  ascend910),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    endif

    ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
    endif

    ifneq ($(filter $(PRODUCT), ascend610 ascend310p helper310p ),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    endif

    ifneq ($(filter $(PRODUCT),  ascend610Liteesl ascend610Lite),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
        EXTRA_CFLAGS += -DCFG_SOC_MDC_V51_LITE
    endif
    ifneq ($(filter $(PRODUCT), ascend610Lite),)
        EXTRA_CFLAGS += -DCFG_SOC_MDC_V51_LITE_FPGA
    endif

    ifneq ($(filter $(PRODUCT),  as31xm1),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
    endif

    ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a as31xm1),)
        EXTRA_CFLAGS += -DCFG_FEATURE_HISTORY_CONFIG
    endif

    ifeq ($(PRODUCT),bs9sx1a)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_BS9SX1A
    endif

    ifneq ($(filter $(PRODUCT),  ascend310 ascend310rc),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
    endif

    ifeq ($(TARGET_BOARD_PLATFORM),kirin990)
        ifeq ($(chip_type),es)
            CHIP_VERSION=hi3690_es
        else
            CHIP_VERSION=hi3690
        endif
    endif
    $(warning chip version $(CHIP_VERSION))

    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
    ifneq ($(filter $(PRODUCT),  ascend910 ascend910B ascend920esl ascend920emu),)
        EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/ipcdrv
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
        ifeq ($(PRODUCT), ascend910)
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_drv_res/ascd910/
        else
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_drv_res/ascd910b/
        endif
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/hdcdrv/hdc_host/
    else ifneq ($(filter $(PRODUCT),  ascend310 ascend310rc ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend310p helper310p),)
        EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/ipcdrv
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/hdcdrv/hdc_host/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
        ifneq ($(filter $(PRODUCT), ascend310p helper310p),)
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_drv_res/ascd310p/
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common/
        else ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_drv_res/ascd310b/
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common/
        else ifneq ($(filter $(PRODUCT), as31xm1),)
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_drv_res/as31xm1/
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common/
        else
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_drv_res/ascd310/
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend310
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/ascend310
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common
        endif
    else ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
        EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_device/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_common/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_platform/ts_platform_device/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/ipcdrv
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/hdcdrv/hdc_host/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_drv_res/ascd610/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_xpc/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_bind_core/
    else ifneq ($(filter $(PRODUCT), ascend610Liteesl ascend610Lite),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
        EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_device/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_common/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_platform/ts_platform_device/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/ipcdrv
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/hdcdrv/hdc_host/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_drv_res/ascd610lite/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_xpc/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_bind_core/
    else
        EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/inc
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/inc/comm
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/src/tsdrv/ts_drv/ts_drv_device/ascend310
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/src/tsdrv/ts_drv/ts_drv_device/ascend310/common
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_core/device_manager/device_manager_inc
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/ai_sdk/arc/linux/kernel_space/src/drv_devmng/drv_devmng_inc
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/src/tsdrv/ts_platform/ts_platform_device/
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/src/ipcdrv
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_drv_res/ascd310/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/hdcdrv/hdc_host/
    endif

    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox/device
    EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
    EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox/device
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_drv_conn/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/log_drv_res/
    ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_common/tsdrv_dev
    else
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common/tsdrv_dev
    endif
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/logdrv/

    ifneq ($(filter $(PRODUCT),  ascend910 ascend910B ascend920esl ascend920emu),)
        EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/cloud_v1
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend910
        obj-m   += drv_log.o
        drv_log-objs := log_drv_dev.o log_drv_conn/log_drv_ipc.o log_drv_conn/log_drv_shm.o log_drv_conn/log_drv_sqcq.o log_drv_res/log_drv_res_common.o

        ifneq ($(filter $(PRODUCT),  ascend910B),)
            #for CFG_TRS_REFACTOR_FEATURE
            EXTRA_CFLAGS += -DCFG_TRS_REFACTOR_FEATURE
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/inc
            drv_log-objs += log_drv_conn/log_drv_chan.o
        endif
    else ifneq ($(filter $(PRODUCT),  ascend310 ascend310rc ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend310p helper310p),)
        ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend310p helper310p),)
            EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/hi1910p
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend610
            obj-m   += drv_log.o
            drv_log-objs := log_drv_dev.o   log_drv_conn/log_drv_ipc.o log_drv_conn/log_drv_shm.o log_drv_conn/log_drv_sqcq.o log_drv_res/log_drv_res_common.o
        else
            EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/mini
            EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/mini_v1
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend310
            obj-m   += drv_log.o
            drv_log-objs := log_drv_dev.o   log_drv_conn/log_drv_ipc.o log_drv_conn/log_drv_shm.o log_drv_conn/log_drv_sqcq.o log_drv_res/log_drv_res_common.o
        endif

        ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3

            #for CFG_TRS_REFACTOR_FEATURE
            EXTRA_CFLAGS += -DCFG_TRS_REFACTOR_FEATURE
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/inc
            drv_log-objs += log_drv_conn/log_drv_chan.o
        endif
    else ifneq ($(filter $(PRODUCT),  ascend610 ascend610Liteesl ascend610Lite bs9sx1a),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
        EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/hi1910p
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_platform/ts_platform_device/ascend610
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_device/ascend610
        obj-m   += drv_log.o
        drv_log-objs := log_drv_dev.o   log_drv_conn/log_drv_ipc.o log_drv_conn/log_drv_shm.o log_drv_conn/log_drv_sqcq.o log_drv_res/log_drv_res_common.o log_xpc/log_xpc.o log_xpc/log_xpc_server.o log_bind_core/drv_cpu_type.o
    else
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/soc/hi1910
        EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/src/tsdrv/ts_platform/ts_platform_device/ascend310
        obj-$(CONFIG_NPU_LOG_DEVDRV)   += drv_log.o
        drv_log-objs := log_drv_dev.o   log_drv_conn/log_drv_ipc.o log_drv_conn/log_drv_shm.o log_drv_conn/log_drv_sqcq.o log_drv_res/log_drv_res_common.o
    endif

endif
