# for cmake
KERNEL_DIR :=$(srctree)

EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
EXTRA_CFLAGS += -I$(TOP_DIR)/abl/ascend_hal/user_space/src/dms
EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/firmware/hiss/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/config
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor/config
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/event
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/status
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/dfm_template
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc

EXTRA_CFLAGS += -fstack-protector-all
ccflags-y += -Wall -Werror -fno-common -Wfloat-equal -Wformat=2 $(WDATE_TIME) -Wcast-align  -Wundef -Wstrict-prototypes
ccflags-y += -Wunused -O2 -Wextra -Wno-unused-parameter -Wno-sign-compare

ccflags-y += -I$(M)/ \
             -I$(M)/common/ \
             -I$(M)/init/ \
             -I$(M)/report/ \
             -I$(M)/detect/ \
             -I$(M)/detect/heartbeat/ \
             -I$(M)/detect/startup/ \
             -I$(M)/detect/safety/ \
             -I$(M)/detect/ras/ \
             -I$(M)/external/ \
             -I$(M)/dfx/ \
             -I$(M)/dfx/debugfs/

ifeq ($(use_hitest),kernel_true)
    CC:=hitestwrapper $(CC)
    LD:=hitestwrapper $(LD)
endif

#for dc
ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DLPM_BUILD_DEBUG
endif

EXTRA_CFLAGS += -DDRV_LPM_FAULT
EXTRA_CFLAGS += -DLPM_FAULT_HEARTBEAT

obj-m += drv_lpm_fault.o
drv_lpm_fault-objs := init/lpm_init.o common/lpm_fault_common.o detect/heartbeat/lpm_fault_heartbeat.o
drv_lpm_fault-objs += dfx/debugfs/lpm_debugfs_base.o dfx/debugfs/lpm_fault_debugfs.o
drv_lpm_fault-objs += common/lpm_fault_ipc.o common/lpm_fault_channel.o

ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
    EXTRA_CFLAGS += -DLPM_START_DIRECT
    EXTRA_CFLAGS += -DLPM_IPC_V1
endif

ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
    EXTRA_CFLAGS += -DLPM_IPC_V3
    EXTRA_CFLAGS += -DLPM_REPORT
    EXTRA_CFLAGS += -DLPM_SHAREMEM_V2
    drv_lpm_fault-objs += common/lpm_fault_sharemem.o common/lpm_fault_sharemem_v2.o
    drv_lpm_fault-objs += report/lpm_fault_report.o report/lpm_fault_config.o
endif

ifneq ($(filter $(PRODUCT), ascend310B ascend310Brc as31xm1),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
    EXTRA_CFLAGS += -DLPM_IPC_V3
    EXTRA_CFLAGS += -DLPM_REPORT
    EXTRA_CFLAGS += -DLPM_FAULT_AO_SAFETY
    EXTRA_CFLAGS += -DLPM_FAULT_RAS
    EXTRA_CFLAGS += -DLPM_REPORT_FROM_IPC
    EXTRA_CFLAGS += -DLPM_SHAREMEM_V2
    drv_lpm_fault-objs += common/lpm_fault_sharemem.o common/lpm_fault_sharemem_v2.o
    drv_lpm_fault-objs += external/lpm_fault_from_ipc.o
    drv_lpm_fault-objs += report/lpm_fault_report.o report/lpm_fault_config.o
    drv_lpm_fault-objs += detect/ras/lpm_fault_aosub_ras.o detect/ras/lpm_fault_ras.o
endif

ifneq ($(filter $(PRODUCT), ascend910),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    EXTRA_CFLAGS += -DLPM_START_DIRECT
    EXTRA_CFLAGS += -DLPM_IPC_V2
    EXTRA_CFLAGS += -DLPM_REPORT_FROM_IPC
    EXTRA_CFLAGS += -DLPM_REPORT
    drv_lpm_fault-objs += external/lpm_fault_from_ipc.o
    drv_lpm_fault-objs += report/lpm_fault_report.o report/lpm_fault_config.o
endif

ifneq ($(filter $(PRODUCT), ascend610 BS9SX1A),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
    EXTRA_CFLAGS += -DLPM_IPC_V2
    EXTRA_CFLAGS += -DLPM_REPORT_FROM_IPC
    EXTRA_CFLAGS += -DLPM_FAULT_STARTUP
    EXTRA_CFLAGS += -DLPM_FAULT_SAFETY
    EXTRA_CFLAGS += -DLPM_FAULT_AO_SAFETY
    EXTRA_CFLAGS += -DLPM_REPORT
    EXTRA_CFLAGS += -DLPM_BUILD_DEBUG
    EXTRA_CFLAGS += -DCFG_FEATURE_PARTIAL_GOOD
    EXTRA_CFLAGS += -DLPM_VPRINTK
    drv_lpm_fault-objs += detect/startup/lpm_fault_startup.o
    drv_lpm_fault-objs += detect/safety/lpm_fault_safety.o detect/safety/lpm_fault_crg.o detect/safety/lpm_fault_aosub_safety.o
    drv_lpm_fault-objs += external/lpm_fault_from_ipc.o
    drv_lpm_fault-objs += report/lpm_fault_report.o report/lpm_fault_config.o
    drv_lpm_fault-objs += common/lpm_log_base.o
endif

ifneq ($(filter $(PRODUCT), helper310p),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_HELPER
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DLPM_START_DIRECT
    EXTRA_CFLAGS += -DLPM_IPC_V2
    EXTRA_CFLAGS += -DLPM_REPORT
    EXTRA_CFLAGS += -DLPM_SHAREMEM_V1
    drv_lpm_fault-objs += common/lpm_fault_sharemem.o
    drv_lpm_fault-objs += report/lpm_fault_report.o report/lpm_fault_config.o
endif

ifneq ($(filter $(PRODUCT), ascend310p),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DLPM_IPC_V2
    EXTRA_CFLAGS += -DLPM_REPORT
    EXTRA_CFLAGS += -DLPM_SHAREMEM_V1
    drv_lpm_fault-objs += common/lpm_fault_sharemem.o
    drv_lpm_fault-objs += report/lpm_fault_report.o report/lpm_fault_config.o
endif

ifneq ($(filter $(PRODUCT), ascend910B),)
    EXTRA_CFLAGS += -DLPM_REPORT_FROM_IPC
    drv_lpm_fault-objs += external/lpm_fault_from_ipc.o
endif

ifneq ($(filter $(PRODUCT), as31xm1),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
    EXTRA_CFLAGS += -DLPM_FAULT_RAS_PLL
    EXTRA_CFLAGS += -DLPM_VPRINTK
    drv_lpm_fault-objs += detect/ras/lpm_fault_lpm_ras.o
    drv_lpm_fault-objs += common/lpm_log_base.o
endif
