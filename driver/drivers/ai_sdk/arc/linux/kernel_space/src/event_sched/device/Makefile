##########################################################################
# 1. CFG_FEATURE_SCHEDULE_REALTIME, the real-time scheduling macro means
# that thread event processing time must be vary fast, and any time beyond
# that is considered timeout, only ascend610 mdc use this macro.
#
# 2. CFG_FEATURE_ASSIST_OTHER_CPU, helps other cpus process event features
# macro, giving priority to higher-priority queues for other Cpus, and then
# processing lower-priority events for their own Cpus, only ascend610 mdc
# use this macro.
#
# 3. CFG_FEATURE_DEEPER_EVENT_QUE, the deeper event queue feature macro,
# it is used to support Ascend910 training scenarios to publish more events.
#
# 4. CFG_FEATURE_TIMEOUT_PROCESS, the timeout-process macro means that if find
# the sched_thread running timeout, it must be process, including record logs,
# callback finish interface, release event rescource, or it managed by the
# upper business, only ascend610 mdc use this macro
###########################################################################
ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

ccflags-y += -Wall -Werror

ifeq ($(TOP_DIR),)
    ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51 -DCFG_FEATURE_MORE_PID_PRIORITY -DCFG_FEATURE_TIMEOUT_PROCESS -DCFG_FEATURE_SCHEDULE_REALTIME
    endif

    ifneq ($(filter $(PRODUCT), as31xm1),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11 -DCFG_FEATURE_THREAD_SWAPOUT
    endif

    ifneq ($(filter $(PRODUCT), ascend610Lite ascend610Liteesl),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51_LITE
    endif

    ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
    endif

    EXTRA_CFLAGS += -DCFG_FEATURE_IDENTIFY_CP

    KERNEL_DIR :=$(srctree)
    EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/event_sched/device/comm/
    EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc/
    EXTRA_CFLAGS += -I$(PWD)/inc/driver/
    EXTRA_CFLAGS += -I$(PWD)/inc/driver/errcode/
    EXTRA_CFLAGS += -I$(PWD)/inc/common/
    EXTRA_CFLAGS += -I$(PWD)/libc_sec/include

    obj-m  += ascend_event_sched.o
    ascend_event_sched-objs := comm/esched_fops.o comm/esched.o comm/esched_sysfs.o

    ifeq ($(TARGET_CHIP_ID), $(A9XXGEN02M01))
        EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/event_sched/device/comm/hard_sched/
        EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/event_sched/device/
        EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/event_sched/device/soc/
        EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/event_sched/device/soc/topic_sched_v1/
        EXTRA_CFLAGS += -DCFG_FEATURE_HARDWARE_SCHED
        ascend_event_sched-objs += comm/hard_sched/esched_drv.o esched_drv_adapt.o soc/topic_sched_v1/topic_sched.o
    endif

    ifneq ($(filter $(PRODUCT),  ascend310p ascend910),)
        EXTRA_CFLAGS += -DCFG_FEATURE_DEEPER_EVENT_QUE -DCFG_FEATURE_VFIO
        EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/event_sched/device/comm/soft_sched/vf_sched/
        ascend_event_sched-objs += comm/soft_sched/vf_sched/esched_vf.o
    endif

else

    ccflags-y += $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char  -Wformat=2 -Wstack-usage=2048 -Wcast-align
    ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-empty-body

    ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51 -DCFG_FEATURE_MORE_PID_PRIORITY -DCFG_FEATURE_TIMEOUT_PROCESS -DCFG_FEATURE_SCHEDULE_REALTIME
    endif

    EXTRA_CFLAGS += -DCFG_FEATURE_IDENTIFY_CP

    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/comm/
    EXTRA_CFLAGS += -I${DRIVER_USER_DIR}/src/event_sched/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver/
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver/errcode/
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/common/
    EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include

    obj-m  += ascend_event_sched.o
    ascend_event_sched-objs := comm/esched_fops.o comm/esched.o comm/esched_sysfs.o

    ifneq ($(filter $(PRODUCT),  ascend610 as31xm1 bs9sx1a ascend610Lite ascend610Liteesl),)
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/bind_cpu
        ascend_event_sched-objs += bind_cpu/drv_cpu_type.o
    endif

    ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl ascend920emu),)
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/comm/hard_sched/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/soc/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/soc/cloud_v2/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/soc/topic_sched_v1/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/inc/
        ifneq ($(filter $(PRODUCT),  ascend920esl),)
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_ESL
        endif
        EXTRA_CFLAGS += -DCFG_FEATURE_HARDWARE_SCHED -DCFG_SOC_PLATFORM_ESL_FPGA -DCFG_FEATURE_REMOTE_SUBMIT -DCFG_FEATURE_SOFT_NON_SCHED_ONLY
        EXTRA_CFLAGS += -DCFG_FEATURE_HARDWARE_MIA
        EXTRA_CFLAGS += -DCFG_FEATURE_HARD_SOFT_SCHED
        ascend_event_sched-objs += comm/hard_sched/esched_drv.o esched_drv_adapt.o soc/topic_sched_v1/topic_sched.o esched_device_msg.o esched_sia_msg.o
        ascend_event_sched-objs += esched_drv_mia.o esched_mia_msg.o
    endif

    ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/comm/hard_sched/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/soc/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/inc
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/soc/mini_v3/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/soc/topic_sched_v1/
        EXTRA_CFLAGS += -DCFG_FEATURE_HARDWARE_SCHED -DCFG_SOC_PLATFORM_ESL_FPGA -DCFG_FEATURE_SOFT_NON_SCHED_ONLY
        ifneq ($(filter $(PRODUCT),  ascend310Besl ascend310Brcesl),)
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_ESL
        endif
        EXTRA_CFLAGS += -DCFG_FEATURE_SIA_MAP_TOPIC_TABLE
        EXTRA_CFLAGS += -DCFG_FEATURE_PM
        ascend_event_sched-objs += comm/hard_sched/esched_drv.o esched_drv_adapt.o soc/topic_sched_v1/topic_sched.o pm/mini_v3/esched_pm.o
        ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu),)
            EXTRA_CFLAGS += -DCFG_FEATURE_REMOTE_SUBMIT
            ascend_event_sched-objs +=  esched_device_msg.o esched_sia_msg.o
        else ifneq ($(PRODUCT), as31xm1)
            EXTRA_CFLAGS += -DCFG_FEATURE_HARDWARE_MIA
            ascend_event_sched-objs +=  esched_drv_mia.o
        endif
    endif

    ifneq ($(filter $(PRODUCT),  ascend310p ascend910),)
        EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/comm/soft_sched/vf_sched/
        ifneq ($(filter $(PRODUCT),  ascend910),)
            EXTRA_CFLAGS += -DCFG_FEATURE_DEEPER_EVENT_QUE
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/soc/cloud
        endif
        ifneq ($(filter $(PRODUCT),  ascend310p),)
            EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/event_sched/device/soc/mini_v2
        endif
        ascend_event_sched-objs += comm/esched_vf.o comm/soft_sched/vf_sched/esched_vf_adapt.o
    endif

    ifneq ($(filter $(PRODUCT), ascend310 ascend310p ascend910),)
        EXTRA_CFLAGS += -DCFG_FEATURE_REMOTE_SUBMIT
        ascend_event_sched-objs += esched_device_msg.o esched_sia_msg.o
    endif

    ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a ascend610Lite ascend610Liteesl as31xm1),)
        EXTRA_CFLAGS += -DCFG_FEATURE_TRACE_RECOED
    else
        EXTRA_CFLAGS += -DCFG_FEATURE_EXTERNAL_CDEV
    endif

    ifneq ($(filter $(PRODUCT),  helper310p ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
        EXTRA_CFLAGS += -DCFG_FEATURE_CPU_NUMS_FIXED
    endif

    ifneq ($(filter $(PRODUCT), as31xm1),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11 -DCFG_FEATURE_NO_BIND_SCHED -DCFG_FEATURE_THREAD_SWAPOUT
        EXTRA_CFLAGS += -DCFG_NOT_SURPPORT_PCIE_HOST_DEVICE_COMM
    endif

    ifneq ($(filter $(PRODUCT), ascend610Lite ascend610Liteesl),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51_LITE
    endif

    ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
    endif

endif
