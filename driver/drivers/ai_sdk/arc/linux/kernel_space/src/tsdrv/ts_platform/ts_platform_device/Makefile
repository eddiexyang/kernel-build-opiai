ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

ccflags-y += -Wall -Werror

ifeq ($(TOP_DIR),)
    KERNEL_DIR := $(srctree)
else
    CFG_FEATURE_TS_IPC := y
    CFG_FEATURE_TS_ALLOC_MEM := y
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/firmware/hiss/inc
    EXTRA_CFLAGS += -I$(TOP_DIR)/tee/inc/tee
    EXTRA_CFLAGS += -I$(TOP_DIR)/tee/inc
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/tee
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
    EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
    EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox/device
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox/device
    EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/aicpu/common

    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/ascend_platform
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_vm
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor/config
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/dfm_template
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/config
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/event
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/time/device
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc/receiver
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_d2h_chan
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_d2h_chan/pci
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_intr
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_ipc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_mem
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_pdata
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_fault
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_pm
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_soc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_firmware
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_parse
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_heartbeat
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/faultmng/fault_fpdc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/tsdrv_virt
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common/tsdrv_osal
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common/tsdrv_dev
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common/tsdrv_sync
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common/tsdrv_id
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_firmware/tsdrv_file_check

    include $(FEATURE_MK_PATH)
    EXTRA_CFLAGS += $(CONFIG_DEFINES)

    obj-m += drv_platform.o
    drv_platform-objs := devdrv_platform.o
    drv_platform-objs += tsdrv_intr/tsdrv_intr.o tsdrv_pdata/tsdrv_pdata.o
    drv_platform-objs += tsdrv_fault/tsdrv_fault_init.o tsdrv_fault/tsdrv_fault_comm.o tsdrv_fault/tsdrv_sys_panic.o
    drv_platform-objs += tsdrv_fault/tsdrv_ras.o tsdrv_fault/tsdrv_non_ras.o tsdrv_fault/tsdrv_tscpu_fault.o
    drv_platform-objs += tsdrv_soc/tsdrv_soc_smc.o tsdrv_soc/config_secure_reg.o
    drv_platform-objs += tsdrv_firmware/tsdrv_firmware_load.o tsdrv_firmware/tsdrv_firmware_reader.o
    drv_platform-objs += tsdrv_pm/tsdrv_pm.o
    drv_platform-objs += tsdrv_parse/tsdrv_parse.o
    drv_platform-objs += tsdrv_heartbeat/tsdrv_heartbeat.o tsdrv_heartbeat/tsdrv_heartbeat_dfx.o

    EXTRA_CFLAGS += -DCFG_FEATURE_TS_IPC

    ccflags-y += $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
    ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-empty-body
ifeq ($(PRODUCT), ascend910)
    ## define
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    EXTRA_CFLAGS += -DCFG_FEATURE_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_SYS_PANIC_DFX
    EXTRA_CFLAGS += -DCFG_FEATURE_TS_ALLOC_MEM
    EXTRA_CFLAGS += -DCFG_FEATURE_FAULT_FPDC
    EXTRA_CFLAGS += -DCFG_FEATURE_TS_HB_DFX

    ## include
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/cloud_v1
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/cloud
    EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/cloud
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_soc/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_firmware/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_fault/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/ascend910

    drv_platform-objs += ascend910/devdrv_parse_pdata.o
    drv_platform-objs += tsdrv_intr/ascend910/tsdrv_intr_platform.o
    drv_platform-objs += tsdrv_pdata/ascend910/tsdrv_pdata_platform.o
    drv_platform-objs += tsdrv_soc/ascend910/tsdrv_soc_pm.o
    drv_platform-objs += tsdrv_soc/tsdrv_soc_sec.o
    drv_platform-objs += tsdrv_pm/ascend910/tsdrv_pm.o
    drv_platform-objs += tsdrv_firmware/ascend910/tsdrv_adapt_firmware_load.o tsdrv_firmware/ascend910/tsdrv_aicpu_config.o tsdrv_firmware/tsdrv_file_check/tsdrv_tee_check/tsdrv_file_tee_check.o
    drv_platform-$(CFG_FEATURE_TS_IPC) += tsdrv_ipc/ascend910/tsdrv_ipc_platform.o
    drv_platform-objs += tsdrv_fault/ascend910/tsdrv_fault_platform.o
    drv_platform-objs += tsdrv_d2h_chan/tsdrv_d2h_chan.o tsdrv_d2h_chan/pci/tsdrv_pci_chan.o
else ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
    EXTRA_CFLAGS += -DCFG_FEATURE_TS_ALLOC_MEM
    EXTRA_CFLAGS += -DCFG_FEATURE_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_SYS_PANIC_DFX

    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/mini_v1
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/mini
    EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/mini
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend310
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend310
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_soc/ascend310
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_firmware/ascend310
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_fault/ascend310
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/ascend310/

    drv_platform-objs += tsdrv_d2h_chan/tsdrv_d2h_chan.o tsdrv_d2h_chan/pci/tsdrv_pci_chan.o
    drv_platform-objs += ascend310/devdrv_parse_pdata.o
    drv_platform-objs += tsdrv_intr/ascend310/tsdrv_intr_platform.o
    drv_platform-objs += tsdrv_pdata/ascend310/tsdrv_pdata_platform.o
    drv_platform-objs += tsdrv_soc/ascend310/tsdrv_soc_pm.o
    drv_platform-objs += tsdrv_pm/ascend310/tsdrv_pm.o
    drv_platform-objs += tsdrv_firmware/ascend310/tsdrv_adapt_firmware_load.o tsdrv_firmware/ascend310/tsdrv_aicpu_config.o tsdrv_firmware/tsdrv_file_check/tsdrv_tee_check/tsdrv_file_tee_check.o
    drv_platform-$(CFG_FEATURE_TS_IPC) += tsdrv_ipc/ascend310/tsdrv_ipc_platform.o
    drv_platform-objs += tsdrv_fault/ascend310/tsdrv_fault_platform.o
else ifneq ($(filter $(PRODUCT), ascend310p helper310p ascend610 bs9sx1a),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_FEATURE_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_TS_HB_DFX
    EXTRA_CFLAGS += -DCFG_FEATURE_AIV_INFO_FROM_PG_INFO_V1

    ifneq ($(filter $(PRODUCT), helper310p), )
        EXTRA_CFLAGS += -DCFG_FEATURE_AICPU_NUM_CFG_UNLIMIT
    endif

    ifneq ($(filter $(PRODUCT), ascend310p helper310p),)
        EXTRA_CFLAGS += -DCFG_FEATURE_TS_ALLOC_MEM
        EXTRA_CFLAGS += -DCFG_FEATURE_FAULT_FPDC
        ifeq ($(PRODUCT), ascend310p)
            EXTRA_CFLAGS += -DCFG_FEATURE_SYS_PANIC_DFX
            EXTRA_CFLAGS += -DCFG_FEATURE_PROF_HWTS_THROUGH
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/lba/local/sia/adapt
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/inc
            drv_platform-objs += tsdrv_parse/mini_v2/set_soc_resmng.o
        endif
        ifeq ($(PRODUCT), helper310p)
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_HELPER
            EXTRA_CFLAGS += -DCFG_FEATURE_CPU_NUMS_FIXED
        endif
    endif

    ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a),)
        CFG_FEATURE_TS_ALLOC_MEM := n
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
        EXTRA_CFLAGS += -DCFG_FEATURE_FAULT_FPDC
        EXTRA_CFLAGS += -DCFG_FEATURE_IPC_CRC
        EXTRA_CFLAGS += -DCFG_FEATURE_BOARD_ID_EXPAND
        # Security failure only on BS9SX1A. for test only in ASCEND610
        EXTRA_CFLAGS += -DCFG_FEATURE_SAFETY_FAULT
        ifeq ($(PRODUCT), bs9sx1a)
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_BS9SX1A
            EXTRA_CFLAGS += -DCFG_SOC_SAMPLE_BACK_DEBUG
        endif
        drv_platform-objs += drv_snapshot.o
        EXTRA_CFLAGS += -DSNAPSHOT_ENABLE
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910/mdc
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_notify
    endif

    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/hi1910p
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv2
    EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/miniv2
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend610
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_soc/ascend610
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_firmware/ascend610
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_fault/ascend610
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/ascend610

    drv_platform-objs += tsdrv_d2h_chan/tsdrv_d2h_chan.o tsdrv_d2h_chan/pci/tsdrv_pci_chan.o
    drv_platform-objs += ascend610/devdrv_parse_pdata.o
    drv_platform-objs += tsdrv_intr/ascend610/tsdrv_intr_platform.o
    drv_platform-objs += tsdrv_pdata/ascend610/tsdrv_pdata_platform.o
    drv_platform-objs += tsdrv_soc/ascend610/tsdrv_soc_pm.o
    drv_platform-objs += tsdrv_soc/tsdrv_soc_sec.o
    drv_platform-objs += tsdrv_pm/ascend610/tsdrv_pm.o
    drv_platform-objs += tsdrv_firmware/ascend610/tsdrv_adapt_firmware_load.o tsdrv_firmware/ascend610/tsdrv_aicpu_config.o tsdrv_firmware/tsdrv_file_check/tsdrv_soc_check/tsdrv_file_soc_check.o
    drv_platform-$(CFG_FEATURE_TS_IPC) += tsdrv_ipc/ascend610/tsdrv_ipc_platform.o
    drv_platform-objs += tsdrv_fault/ascend610/tsdrv_fault_platform.o
else ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
    CFG_FEATURE_TS_ALLOC_MEM := n
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
    EXTRA_CFLAGS += -DCFG_FEATURE_CPU_NUMS_FIXED
    EXTRA_CFLAGS += -DCFG_FEATURE_USE_ICM_CHAN
    EXTRA_CFLAGS += -DCFG_FEATURE_SUPPORT_VSQ_MNG
    EXTRA_CFLAGS += -DCFG_FEATURE_QOS
    EXTRA_CFLAGS += -DCFG_FEATURE_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_FAULT_FPDC
    EXTRA_CFLAGS += -DCFG_FEATURE_HARDWARE_SCHED
    EXTRA_CFLAGS += -DCFG_FEATURE_PG_V2

    #for CFG_TRS_REFACTOR_FEATURE
    EXTRA_CFLAGS += -DCFG_TRS_REFACTOR_FEATURE
    drv_platform-objs += tsdrv_parse/mini_v3/set_soc_resmng.o
    drv_platform-objs += tsdrv_heartbeat/hb_sqcq.o
    drv_platform-objs += adapt_devdrv.o

    ifneq ($(filter $(PRODUCT),  ascend310Brc ascend310Brcesl ascend310Brcemu), )
        EXTRA_CFLAGS += -DCFG_FEATURE_RC_MODE
        EXTRA_CFLAGS += -DCFG_FEATURE_VFIO_DEVICE
        EXTRA_CFLAGS += -DSNAPSHOT_ENABLE
        EXTRA_CFLAGS += -DNOT_SUPPORT_OSNAME
        drv_platform-objs += drv_snapshot.o
    endif

    ifneq ($(filter $(PRODUCT),  ascend310B as31xm1 ascend310Besl ascend310Brc ascend310Brcesl), )
        EXTRA_CFLAGS += -DCFG_FEATURE_PG
    endif

    ifneq ($(filter $(PRODUCT), as31xm1), )
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
        EXTRA_CFLAGS += -DCFG_NOT_SURPPORT_PCIE_HOST_DEVICE_COMM
        EXTRA_CFLAGS += -DSNAPSHOT_ENABLE
        EXTRA_CFLAGS += -DCFG_FEATURE_USE_FAULT_NEW_FRAME_WORK
        drv_platform-objs += drv_snapshot.o
        EXTRA_CFLAGS += -DCFG_MEMORY_OPTIMIZE
    endif

    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/mini_v3
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv3
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend610
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_soc/ascend310B
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_firmware/ascend310B
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_fault/ascend310B
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/ascend610
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_qos
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/tsdrv_cdqm
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/lba/local/sia/adapt
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl

    drv_platform-objs += ascend610/devdrv_parse_pdata.o
    drv_platform-objs += tsdrv_qos/tsdrv_qos.o
    drv_platform-objs += tsdrv_intr/ascend610/tsdrv_intr_platform.o
    drv_platform-objs += tsdrv_pdata/ascend610/tsdrv_pdata_platform.o
    drv_platform-objs += tsdrv_soc/tsdrv_soc_sec.o
    drv_platform-objs += tsdrv_soc/ascend310B/tsdrv_soc_pm.o
    drv_platform-objs += tsdrv_pm/ascend310B/tsdrv_pm.o
    drv_platform-objs += tsdrv_firmware/ascend610/tsdrv_adapt_firmware_load.o tsdrv_firmware/ascend610/tsdrv_aicpu_config.o tsdrv_firmware/tsdrv_file_check/tsdrv_soc_check/tsdrv_file_soc_check.o
    drv_platform-objs += tsdrv_fault/ascend310B/tsdrv_fault_platform.o
    ifneq ($(filter $(PRODUCT), as31xm1), )
        drv_platform-objs += tsdrv_fault/ascend310B/tsdrv_ipc_fault_init.o tsdrv_fault/ascend310B/tsdrv_ipc_fault_rcv.o
    endif
    drv_platform-$(CFG_FEATURE_TS_IPC) += tsdrv_ipc/ascend310B/tsdrv_ipc_platform.o
else ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
    CFG_FEATURE_TS_ALLOC_MEM := n
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
    EXTRA_CFLAGS += -DCFG_FEATURE_SUPPORT_VSQ_MNG
    EXTRA_CFLAGS += -DCFG_FEATURE_TS_HB_DFX
    EXTRA_CFLAGS += -DCFG_FEATURE_QOS
    EXTRA_CFLAGS += -DCFG_FEATURE_HARDWARE_SCHED
    ifeq ($(PRODUCT), ascend920esl)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2_ESL
    endif

    ifneq ($(filter $(PRODUCT), ascend910B ascend920esl), )
        EXTRA_CFLAGS += -DCFG_FEATURE_PG
        EXTRA_CFLAGS += -DCFG_FEATURE_VFIO_DEVICE
        EXTRA_CFLAGS += -DCFG_FEATURE_FAULT
        EXTRA_CFLAGS += -DCFG_FEATURE_FAULT_FPDC
        EXTRA_CFLAGS += -DCFG_FEATURE_USE_ICM_CHAN
        EXTRA_CFLAGS += -DCFG_FEATURE_HW_INFO_FROM_BIOS
        EXTRA_CFLAGS += -DCFG_FEATURE_AIV_INFO_FROM_AIC
    endif

    #for CFG_TRS_REFACTOR_FEATURE
    EXTRA_CFLAGS += -DCFG_TRS_REFACTOR_FEATURE
    drv_platform-objs += tsdrv_parse/cloud_v2/set_soc_resmng.o
    drv_platform-objs += tsdrv_heartbeat/hb_sqcq.o
    drv_platform-objs += adapt_devdrv.o
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/lba/local/sia/adapt
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/inc

    ## include
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/cloud_v2
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/cloud
    EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/cloud
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_soc/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_firmware/ascend910B
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_fault/ascend910B
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/tsdrv_cdqm
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/tsdrv_qos

    drv_platform-objs += ascend910/devdrv_parse_pdata.o
    drv_platform-objs += tsdrv_soc/tsdrv_soc_sec.o
    drv_platform-objs += tsdrv_qos/tsdrv_qos.o
    drv_platform-objs += tsdrv_intr/ascend910/tsdrv_intr_platform.o
    drv_platform-objs += tsdrv_pdata/ascend910/tsdrv_pdata_platform.o
    drv_platform-objs += tsdrv_soc/ascend910/tsdrv_soc_pm.o
    drv_platform-objs += tsdrv_pm/ascend910/tsdrv_pm.o
    drv_platform-objs += tsdrv_firmware/ascend910/tsdrv_adapt_firmware_load.o tsdrv_firmware/ascend910/tsdrv_aicpu_config.o tsdrv_firmware/tsdrv_file_check/tsdrv_soc_check/tsdrv_file_soc_check.o
    drv_platform-$(CFG_FEATURE_TS_IPC) += tsdrv_ipc/ascend920/tsdrv_ipc_platform.o
    drv_platform-objs += tsdrv_fault/ascend910B/tsdrv_fault_platform.o
endif
    drv_platform-$(CFG_FEATURE_TS_IPC) += tsdrv_ipc/tsdrv_ipc.o  tsdrv_ipc/tsdrv_ipc_dfx.o
    drv_platform-$(CFG_FEATURE_TS_ALLOC_MEM) += tsdrv_mem/tsdrv_ts_alloc_mem.o tsdrv_mem/tsdrv_alloc_mem_dfx.o
endif
