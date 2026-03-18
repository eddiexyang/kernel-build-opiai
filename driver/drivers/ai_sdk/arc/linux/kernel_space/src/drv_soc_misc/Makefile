# for cmake & ctrl cpu open
KERNEL_DIR :=$(srctree)
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
EXTRA_CFLAGS += -I$(TOP_DIR)/abl/ascend_hal/user_space/src/dms
EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/ai_sdk/driver/linux/kernel_space/src/PCA6416
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/ascend_platform
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dbl/dev_urd
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/config
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor/config
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/event
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/core
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/dfm_template
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/faultmng/fault_fpdc
ifneq ($(filter $(PRODUCT),  ascend610 ascend610Liteesl ascend610Lite bs9sx1a),)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_common
else
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common
endif
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/features
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/chipinfo

ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef $(WDATE_TIME)
ccflags-y += -Wstack-usage=2048 -Wno-unused-parameter -Wno-sign-compare
ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

obj-m += drv_soc_misc.o
drv_soc_misc-objs := soc_misc_init.o
drv_soc_misc-objs += chipinfo/soc_misc_feature.o chipinfo/soc_misc_info.o
drv_soc_misc-objs += soc_misc_common.o chipinfo/soc_misc_chip_info.o soc_misc_cpu_utilization.o

include $(FEATURE_MK_PATH)
EXTRA_CFLAGS += $(CONFIG_DEFINES)

include ${DRIVER_KERNEL_DIR}/src/drv_soc_misc/features/features.mk

ifeq ($(PRODUCT),ascend910)
    EXTRA_CFLAGS += -DCFG_FEATURE_FAULT_MANAGER
    EXTRA_CFLAGS += -DCFG_FEATURE_DIE_ID_N_V
    EXTRA_CFLAGS += -DCFG_FEATURE_SLOT_ID_FROM_DEVINFO
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    EXTRA_CFLAGS += -DCFG_FEATURE_BASE_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_RING_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_PCIE_LOCAL_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_CHIP_EXPAND_VERSION
    EXTRA_CFLAGS += -DCFG_FEATURE_DFM_RAS_FAULT

    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3d
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3t
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend910

    drv_soc_misc-objs += features/slot_id_from_devinfo.o
    drv_soc_misc-objs += faultmgr/soc_misc_dms_init.o faultmgr/soc_misc_err_info.o faultmgr/soc_misc_fault.o faultmgr/l3d/l3d_dms_init.o faultmgr/l3t/l3t_dms_init.o
    drv_soc_misc-objs += faultmgr/l3d/l3d_arm_ras.o faultmgr/l3d/l3d_arm_ras_unmask.o faultmgr/l3t/l3t_arm_ras.o faultmgr/l3t/l3t_arm_ras_unmask.o
    drv_soc_misc-objs += faultmgr/soc_misc_cpucore.o faultmgr/soc_misc_ring.o faultmgr/soc_misc_pcie.o
    drv_soc_misc-objs += faultmgr/soc_misc_ras.o faultmgr/soc_misc_cpucore.o faultmgr/soc_misc_ring.o faultmgr/soc_misc_pcie.o
else ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
    EXTRA_CFLAGS += -DCFG_FEATURE_DIE_ID_PMU
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/ascend310
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend310
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend310
else ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a),)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3d
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3t
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/ascend610
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_notify
    EXTRA_CFLAGS += -DCFG_FEATURE_FAULT_MANAGER
    EXTRA_CFLAGS += -DCFG_FEATURE_SAFETY_MANAGER
    EXTRA_CFLAGS += -DCFG_FEATURE_BASE_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_RING_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_MN_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_RBRG_FAULT
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
    EXTRA_CFLAGS += -DCFG_FEATURE_DFM_RAS_FAULT
    ifneq ($(filter $(PRODUCT), ascend610Lite ascend610Liteesl),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_LITE_ESL
    endif
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_platform/ts_platform_device/ascend610
    drv_soc_misc-objs += faultmgr/soc_misc_dms_init.o faultmgr/soc_misc_err_info.o
    drv_soc_misc-objs += faultmgr/soc_misc_fault.o faultmgr/soc_misc_safety.o faultmgr/soc_misc_safety_conf.o faultmgr/soc_misc_ras.o faultmgr/l3d/l3d_dms_init.o faultmgr/l3t/l3t_dms_init.o
    drv_soc_misc-objs += faultmgr/soc_misc_cpucore.o faultmgr/l3d/l3d_safety.o faultmgr/l3d/l3d_arm_ras.o faultmgr/l3t/l3t_safety.o faultmgr/l3t/l3t_arm_ras.o
    drv_soc_misc-objs += faultmgr/soc_misc_ring.o faultmgr/soc_misc_pcie.o
    drv_soc_misc-objs += faultmgr/soc_misc_mn.o faultmgr/soc_misc_cpucluster.o faultmgr/soc_misc_io_subsys.o faultmgr/soc_misc_hac_subsys.o
    drv_soc_misc-objs += faultmgr/soc_misc_peri_subsys.o faultmgr/soc_misc_mbigen_peri.o faultmgr/soc_misc_mbigen_io.o faultmgr/soc_misc_rbrg.o
    drv_soc_misc-objs += faultmgr/l3t/l3t_irq_ras.o faultmgr/l3d/l3d_irq_ras.o
else ifneq ($(filter $(PRODUCT), ascend610Lite ascend610Liteesl),)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3d
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3t
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/ascend610L
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_notify
    EXTRA_CFLAGS += -DCFG_FEATURE_FAULT_MANAGER
    EXTRA_CFLAGS += -DCFG_FEATURE_SAFETY_MANAGER
    EXTRA_CFLAGS += -DCFG_FEATURE_BASE_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_MN_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_CPU_INFO_FROM_DEVINFO_AND_PCIE
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_LITE_ESL
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_LITE
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_platform/ts_platform_device/ascend610
    drv_soc_misc-objs += faultmgr/soc_misc_dms_init.o faultmgr/soc_misc_err_info.o faultmgr/soc_misc_safety_conf.o
    drv_soc_misc-objs += faultmgr/soc_misc_fault.o faultmgr/soc_misc_safety.o  faultmgr/l3d/l3d_dms_init.o faultmgr/l3t/l3t_dms_init.o
    drv_soc_misc-objs += faultmgr/l3d/l3d_safety.o faultmgr/l3t/l3t_safety.o
    drv_soc_misc-objs += faultmgr/soc_misc_mn.o
else ifneq ($(filter $(PRODUCT), helper310p),)
    EXTRA_CFLAGS += -DCFG_FEATURE_CPU_NUMS_FIXED
    EXTRA_CFLAGS += -DCFG_FEATURE_BASE_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_FAULT_MANAGER
    EXTRA_CFLAGS += -DCFG_FEATURE_PCIE_LOCAL_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_EMMC_HISI_COMM_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_DFM_RAS_FAULT
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3d
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3t
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/helper310p
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_notify
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend610
    drv_soc_misc-objs += faultmgr/soc_misc_dms_init.o faultmgr/soc_misc_err_info.o faultmgr/soc_misc_fault.o faultmgr/l3d/l3d_dms_init.o faultmgr/l3t/l3t_dms_init.o
    drv_soc_misc-objs += faultmgr/l3d/l3d_arm_ras.o faultmgr/l3d/l3d_arm_ras_unmask.o faultmgr/l3t/l3t_arm_ras.o faultmgr/l3t/l3t_arm_ras_unmask.o
    drv_soc_misc-objs += faultmgr/soc_misc_ras.o faultmgr/soc_misc_cpucore.o faultmgr/soc_misc_ring.o faultmgr/soc_misc_pcie.o faultmgr/soc_misc_emmc.o
else ifeq ($(PRODUCT), ascend310p)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3d
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3t
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/ascend310p
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_notify
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend610
    EXTRA_CFLAGS += -DCFG_FEATURE_FAULT_MANAGER
    EXTRA_CFLAGS += -DCFG_FEATURE_PCIE_LOCAL_FAULT
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_FEATURE_BASE_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_DFM_RAS_FAULT
    drv_soc_misc-objs += faultmgr/soc_misc_dms_init.o faultmgr/soc_misc_err_info.o faultmgr/soc_misc_fault.o faultmgr/l3d/l3d_dms_init.o faultmgr/l3t/l3t_dms_init.o
    drv_soc_misc-objs += faultmgr/l3d/l3d_arm_ras.o faultmgr/l3t/l3t_arm_ras.o
    drv_soc_misc-objs += faultmgr/soc_misc_ras.o faultmgr/soc_misc_cpucore.o faultmgr/soc_misc_ring.o faultmgr/soc_misc_pcie.o

else ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu as31xm1),)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3d
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3t
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/ascend310B
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend610
    EXTRA_CFLAGS += -DCFG_FEATURE_ENTIRETY_UNMASK
    EXTRA_CFLAGS += -DCFG_FEATURE_FAULT_MANAGER
    EXTRA_CFLAGS += -DCFG_FEATURE_BASE_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_PCIE_HISI_COMM_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_CPU_NUMS_FIXED
    EXTRA_CFLAGS += -DCFG_FEATURE_DFM_RAS_FAULT
    ifneq ($(filter $(PRODUCT), ascend310Brc ascend310Brcesl as31xm1),)
        EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
        EXTRA_CFLAGS += -DCFG_FEATURE_RC_MODE
        EXTRA_CFLAGS += -DCFG_FEATURE_DEVICE_DEVID_CONVERT
    endif
    ifneq ($(filter $(PRODUCT), as31xm1),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    else

        EXTRA_CFLAGS += -DCFG_FEATURE_CPUCORE_UNMASK
        drv_soc_misc-objs += faultmgr/soc_misc_cpucore.o faultmgr/l3d/l3d_arm_ras_unmask.o faultmgr/l3t/l3t_arm_ras_unmask.o
    endif
    drv_soc_misc-objs += faultmgr/soc_misc_dms_init.o faultmgr/soc_misc_err_info.o faultmgr/soc_misc_fault.o faultmgr/l3d/l3d_dms_init.o faultmgr/l3t/l3t_dms_init.o
    drv_soc_misc-objs += faultmgr/l3d/l3d_arm_ras.o faultmgr/l3t/l3t_arm_ras.o faultmgr/soc_misc_pcie.o
    drv_soc_misc-objs += faultmgr/soc_misc_ras.o faultmgr/soc_misc_cpucore.o
else ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3d
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/l3t
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr/ascend910B
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/faultmgr
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend910
    EXTRA_CFLAGS += -DCFG_FEATURE_ENTIRETY_UNMASK
    EXTRA_CFLAGS += -DCFG_FEATURE_FAULT_MANAGER
    EXTRA_CFLAGS += -DCFG_FEATURE_BASE_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_PCIE_HISI_COMM_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_PCIE_DISP_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_DIE_ID_N_V
    EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
    EXTRA_CFLAGS += -DCFG_FEATURE_SLOT_ID_FROM_MODULE_ID
    EXTRA_CFLAGS += -DCFG_FEATURE_DRIVER_QUERY_RAS_ERROR
    EXTRA_CFLAGS += -DCFG_FEATURE_HCCS_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_DFM_RAS_FAULT
    EXTRA_CFLAGS += -DCFG_FEATURE_SLOT_ID_FROM_DEVINFO
    drv_soc_misc-objs += faultmgr/soc_misc_dms_init.o faultmgr/soc_misc_err_info.o faultmgr/soc_misc_fault.o faultmgr/soc_misc_ras.o faultmgr/l3d/l3d_dms_init.o faultmgr/l3t/l3t_dms_init.o
    drv_soc_misc-objs += faultmgr/l3d/l3d_arm_ras.o faultmgr/l3d/l3d_arm_ras_unmask.o faultmgr/l3t/l3t_arm_ras.o faultmgr/l3t/l3t_arm_ras_unmask.o
    drv_soc_misc-objs += faultmgr/soc_misc_pcie.o faultmgr/soc_misc_hccs.o
    drv_soc_misc-objs += faultmgr/soc_misc_cpucore.o
    ifneq ($(filter $(PRODUCT), ascend920esl),)
        EXTRA_CFLAGS += -DCFG_FEATURE_ESL_NOT_SUPPORT
    endif
else
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend310
endif

ifneq ($(filter $(PRODUCT), ascend610Lite ascend610Liteesl),)
EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_LITE_ESL
endif

ifeq ($(strip $(TARGET_BUILD_TYPE)),debug)
        EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif
