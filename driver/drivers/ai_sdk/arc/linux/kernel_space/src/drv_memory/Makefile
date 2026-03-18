KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
EXTRA_CFLAGS += -I$(TOP_DIR)/abl/ascend_hal/user_space/src/dms
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/core
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/config
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor/config
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/event
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc/receiver
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common
EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/tsdrv/ts_platform/ts_platform_device/ascend910
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc/receiver
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/icm
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/timer
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dbl/dev_urd
EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/firmware/hiss/inc/hiss

ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef $(WDATE_TIME)
ccflags-y += -Wstack-usage=2048 -Wno-unused-parameter -Wno-sign-compare
ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

obj-m += drv_memory.o
drv_memory-objs := memory_init.o
drv_memory-objs += memory_ecc.o
drv_memory-objs += memory_scan.o

ifeq ($(PRODUCT),ascend910)
    drv_memory-objs += safety_ras/safety_ras_node.o
    drv_memory-objs += safety_ras/safety_ras_910/safety_ras_init_910.o
    drv_memory-objs += memory_ddr.o
    drv_memory-objs += memory_hbm.o
    drv_memory-objs += hbm_isolation/hbm_isolation_common.o
    drv_memory-objs += hbm_isolation/hbm_multi.o
    drv_memory-objs += hbm_isolation/hbm_single.o
    drv_memory-objs += hbm_isolation/flash_ops_wrap.o
    drv_memory-objs += device/ascend910/drvmem_base_info.o
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/inc/
    EXTRA_CFLAGS += -I$(DRIVER_CHIP_KERNEL_DIR)/src/nor_flash
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/ipc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory/safety_ras
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory/hbm_isolation
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory/device/ascend910/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    EXTRA_CFLAGS += -DSUPPORT_HBM_ISOLATION
    EXTRA_CFLAGS += -DSUPPORT_IPC_GET_ECC
    EXTRA_CFLAGS += -DCFG_MEMORY_SAFETY_RAS
    EXTRA_CFLAGS += -DCFG_MEMORY_DDR
    EXTRA_CFLAGS += -DCFG_MEMORY_HBM
    EXTRA_CFLAGS += -DCFG_DDR_REG_ADDR_910
    EXTRA_CFLAGS += -DCFG_HBM_REG_ADDR_910
    EXTRA_CFLAGS += -DCFG_DDR_BANDWIDTH_BY_CHAN_NUM
    EXTRA_CFLAGS += -DCFG_INIT_DDR_STATISTICS
    EXTRA_CFLAGS += -DCFG_MEMORY_ECC_INFO_FROM_LP
    EXTRA_CFLAGS += -DCFG_NOR_FLASH_OPS_VER1
    EXTRA_CFLAGS += -DNEED_RECOVER_HBM_INTERRUPT
    EXTRA_CFLAGS += -DCFG_LPM_INFO_FROM_IPC
else ifneq ($(filter $(PRODUCT), ascend910B ascend920emu ascend920esl),)
    drv_memory-objs += safety_ras/safety_ras_node.o
    drv_memory-objs += safety_ras/safety_ras_910/safety_ras_init_910.o
    drv_memory-objs += memory_hbm.o
    drv_memory-objs += fault_inject/drvmem_fault_inject.o
    drv_memory-objs += fault_inject/drvmem_fault_inject_node.o
    drv_memory-objs += fault_inject/drvmem_fault_inject_common.o
    drv_memory-objs += fault_inject/device/ascend910b/drvmem_fault_inject_define.o
    drv_memory-objs += hbm_isolation/hbm_isolation_common.o
    drv_memory-objs += hbm_isolation/hbm_multi.o
    drv_memory-objs += hbm_isolation/hbm_single.o
    drv_memory-objs += hbm_isolation/flash_ops_wrap.o
    drv_memory-objs += device/ascend910B/drvmem_base_info.o
    drv_memory-objs += fault_inject/device/ascend910b/drvmem_hbm_fault_inject_define.o
    drv_memory-objs += fault_inject/device/ascend910b/drvmem_hbmc_fault_inject_define.o
    drv_memory-objs += fault_inject/device/ascend910b/drvmem_mata_fault_inject_define.o
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/firmware/hiss/inc
    EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/inc/
    EXTRA_CFLAGS += -I$(DRIVER_CHIP_KERNEL_DIR)/src/nor_flash
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory/safety_ras
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory/hbm_isolation
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory/device/ascend910B/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory/fault_inject
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory/fault_inject/device/ascend910b
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
    EXTRA_CFLAGS += -DSUPPORT_HBM_ISOLATION
    EXTRA_CFLAGS += -DCFG_MEMORY_HBM
    EXTRA_CFLAGS += -DCFG_DDR_REG_ADDR_910B
    EXTRA_CFLAGS += -DCFG_MEMORY_ECC_INFO_FROM_LP
    EXTRA_CFLAGS += -DCFG_INIT_DDR_STATISTICS
    ifeq ($(TARGET_BUILD_TYPE), debug)
        EXTRA_CFLAGS += -DCFG_MEMORY_FAULT_INJECT
    endif
    EXTRA_CFLAGS += -DCFG_MEMORY_SAFETY_RAS
    EXTRA_CFLAGS += -DCFG_HBM_FLASH
    EXTRA_CFLAGS += -DCFG_HBM_FAULT_EVENT
    EXTRA_CFLAGS += -DCFG_LPM_INFO_FROM_SHAREMEM
    EXTRA_CFLAGS += -DCFG_NOR_FLASH_OPS_VER2
else ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
    drv_memory-objs += memory_ddr.o
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend310
    EXTRA_CFLAGS += -DCFG_MEMORY_DDR
    EXTRA_CFLAGS += -DCFG_DDR_REG_ADDR_310
    EXTRA_CFLAGS += -DCFG_DDR_BANDWIDTH_BY_CHAN_NUM
    EXTRA_CFLAGS += -DCFG_MEMORY_DDR_INFO_FROM_REG
else ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl as31xm1),)
    drv_memory-objs += memory_ddr.o
    drv_memory-objs += safety_ras/safety_ras_node.o
    drv_memory-objs += safety_ras/safety_ras_report.o
    drv_memory-objs += safety_ras/safety_ras_ipc.o
    drv_memory-objs += safety_ras/safety_ras_310/safety_ras_init_310.o
    drv_memory-objs += safety_ras/safety_ras_310/safety_ras_ipc_310.o
    drv_memory-objs += safety_ras/safety_ras_310/safety_ras_mata_310.o
    drv_memory-objs += safety_ras/safety_ras_310/safety_ras_mata_scan_310.o
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory/safety_ras
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
    EXTRA_CFLAGS += -I$(TOP_DIR)/abl/ascend_hal/user_space/src/dms
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc/receiver
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/config
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor/config
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/event
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/dfm_template
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
    EXTRA_CFLAGS += -DCFG_MEMORY_SAFETY_RAS
    EXTRA_CFLAGS += -DCFG_MEMORY_DDR
    EXTRA_CFLAGS += -DCFG_DDR_REG_ADDR_310B
    EXTRA_CFLAGS += -DCFG_MEMORY_DDR_INFO_FROM_SHAREMEM
    EXTRA_CFLAGS += -DCFG_INIT_DDR_STATISTICS
    EXTRA_CFLAGS += -DCFG_FEATURE_FORCE_NO_SCAN
    ifneq ($(filter $(PRODUCT), as31xm1),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
        drv_memory-objs += safety_ras/safety_ras_310/safety_ras_reg_check_310.o
    endif
    ifneq ($(filter $(PRODUCT), ascend310Brc ascend310Brcesl as31xm1),)

        EXTRA_CFLAGS += -DCFG_FEATURE_DEVICE_DEVID_CONVERT
    endif
    ifneq ($(filter $(PRODUCT), ascend310B ascend310Brc as31xm1),)
        EXTRA_CFLAGS += -DCFG_DDR_BANDWIDTH_BY_FREQ
    else
        EXTRA_CFLAGS += -DCFG_DDR_BANDWIDTH_STUB
    endif
else ifneq ($(filter $(PRODUCT), ascend310p helper310p),)
    drv_memory-objs += memory_ddr.o
    drv_memory-objs += memory_common.o
    drv_memory-objs += memory_dev_manage.o
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
    EXTRA_CFLAGS += -DCFG_MEMORY_DDR
    EXTRA_CFLAGS += -DCFG_DDR_REG_ADDR_710
    EXTRA_CFLAGS += -DCFG_DDR_BANDWIDTH_BY_FREQ
    EXTRA_CFLAGS += -DCFG_MEMORY_ECC_INFO_FROM_LP
    EXTRA_CFLAGS += -DCFG_INIT_DDR_STATISTICS
    EXTRA_CFLAGS += -DCFG_FEATURE_LP_ENABLE
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_MEMORY_DDR_INFO_FROM_LP
    ifneq ($(filter $(PRODUCT), ascend310p),)
        drv_memory-objs += safety_ras/safety_ras_node.o
        drv_memory-objs += safety_ras/safety_ras_ipc.o
        drv_memory-objs += safety_ras/safety_ras_710/safety_ras_init_710.o
        drv_memory-objs += safety_ras/safety_ras_710/safety_ras_ipc_710.o
        drv_memory-objs += safety_ras/safety_ras_710/safety_ras_isolation_710.o
        drv_memory-objs += hbm_isolation/hbm_isolation_common.o
        drv_memory-objs += hbm_isolation/hbm_multi.o
        drv_memory-objs += hbm_isolation/hbm_single.o
        drv_memory-objs += hbm_isolation/flash_ops_wrap.o
        drv_memory-objs += device/ascend310P/drvmem_base_info.o
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory/safety_ras
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory/hbm_isolation
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory/device/ascend310P/
        EXTRA_CFLAGS += -DCFG_DDR_ISOLATION_710
        EXTRA_CFLAGS += -DCFG_FEATURE_SUPPORT_2P
        EXTRA_CFLAGS += -DCFG_FEATURE_NO_CECC_ADDR_REC
        EXTRA_CFLAGS += -DCFG_HBM_FLASH
        EXTRA_CFLAGS += -DCFG_NOR_FLASH_OPS_VER2
        EXTRA_CFLAGS += -DCFG_MEMORY_SAFETY_RAS
        EXTRA_CFLAGS += -I$(PWD)/inc/hiss
        EXTRA_CFLAGS += -I$(PWD)/inc/toolchain/bbox
        ifeq ($(TARGET_BUILD_TYPE), debug)
            drv_memory-objs += safety_ras/safety_ras_710/safety_ras_debug_710.o
            EXTRA_CFLAGS += -DCFG_MEMORY_DEBUGFS
        endif
    endif
else ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
    drv_memory-objs += memory_ddr.o
    drv_memory-objs += memory_common.o
    drv_memory-objs += memory_dev_manage.o
    drv_memory-objs += memory_log.o
    drv_memory-objs += safety_ras/safety_ras_node.o
    drv_memory-objs += safety_ras/safety_ras_report.o
    drv_memory-objs += safety_ras/safety_ras_ipc.o
    drv_memory-objs += safety_ras/safety_ras_610/safety_ras_init_610.o
    drv_memory-objs += safety_ras/safety_ras_610/safety_ras_ipc_610.o
    drv_memory-objs += safety_ras/safety_ras_610/safety_ras_subctrl_610.o
    drv_memory-objs += safety_ras/safety_ras_610/safety_ras_dha_610.o
    drv_memory-objs += safety_ras/safety_ras_610/safety_ras_dha_dfx_610.o
    drv_memory-objs += safety_ras/safety_ras_610/safety_ras_dha_report_610.o
    drv_memory-objs += safety_ras/safety_ras_610/safety_ras_common_610.o
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_memory/safety_ras
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
    EXTRA_CFLAGS += -I$(TOP_DIR)/abl/ascend_hal/user_space/src/dms
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc/receiver
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/config
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor/config
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/event
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/dfm_template
    EXTRA_CFLAGS += -DCFG_MEMORY_DDR
    EXTRA_CFLAGS += -DCFG_FEATURE_LP_ENABLE
    ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a),)
        EXTRA_CFLAGS += -DDDR_VPRINTK
        EXTRA_CFLAGS += -DCFG_DDR_REG_ADDR_710
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
    endif
    EXTRA_CFLAGS += -DCFG_DDR_BANDWIDTH_BY_FREQ
    EXTRA_CFLAGS += -DCFG_MEMORY_ECC_INFO_FROM_LP
    EXTRA_CFLAGS += -DCFG_INIT_DDR_STATISTICS
    EXTRA_CFLAGS += -DCFG_FEATURE_FORCE_NO_SCAN
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_MEMORY_DDR_INFO_FROM_LP
    EXTRA_CFLAGS += -DDRV_SUPPORT_SYSCOUNT
    EXTRA_CFLAGS += -DCFG_MEMORY_SAFETY_RAS
    ifneq ($(filter $(PRODUCT), ascend610Lite ascend610Liteesl),)
        EXTRA_CFLAGS += -DCFG_DDR_REG_ADDR_610VER3
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51LITE
    endif
endif

