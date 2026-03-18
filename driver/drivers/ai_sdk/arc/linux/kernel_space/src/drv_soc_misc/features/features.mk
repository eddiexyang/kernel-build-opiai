ifeq ($(CONFIG_BOARD_ID_FROM_DEVINFO),y)
    drv_soc_misc-objs += features/board_id_from_devinfo.o
else ifeq ($(CONFIG_BOARD_ID_FROM_DTS),y)
    drv_soc_misc-objs += features/board_id_from_dts.o
else ifeq ($(CONFIG_BOARD_ID_FROM_GPIO),y)
    drv_soc_misc-objs += features/board_id_from_gpio.o
else ifeq ($(CONFIG_BOARD_ID_STUB),y)
    drv_soc_misc-objs += features/board_id_stub.o
else
    $(error error failed no feature defined)
endif

ifeq ($(CONFIG_BOARD_INFO_FROM_PCA6416),y)
    drv_soc_misc-objs += features/boardinfo_from_pca6416.o
else ifeq ($(CONFIG_BOARD_INFO_NOT_SUPPORT_DEFAULT_5A),y)
    drv_soc_misc-objs += features/boardinfo_not_support_default_5a.o
else ifeq ($(CONFIG_BOARD_INFO_NOT_SUPPORT_DEFAULT_01),y)
    drv_soc_misc-objs += features/boardinfo_not_support_default_01.o
else ifeq ($(CONFIG_BOARD_INFO_NOT_SUPPORT_DEFAULT_FF),y)
    drv_soc_misc-objs += features/boardinfo_not_support_default_ff.o
else
    $(error error failed no feature defined)
endif

ifeq ($(CONFIG_CPU_INFO_FROM_DEVINFO),y)
    drv_soc_misc-objs += features/cpu_info_from_devinfo.o
else ifeq ($(CONFIG_CPU_INFO_FROM_PCIE),y)
    drv_soc_misc-objs += features/cpu_info_from_pcie.o
else
    $(error error failed no feature defined)
endif

ifeq ($(CONFIG_SLOT_ID_FROM_CMDLINE),y)
    drv_soc_misc-objs += features/slot_id_from_cmdline.o
else ifeq ($(CONFIG_SLOT_ID_FROM_DEVINFO),y)
    drv_soc_misc-objs += features/slot_id_from_devinfo.o
else ifeq ($(CONFIG_SLOT_ID_FROM_GPIO),y)
    drv_soc_misc-objs += features/slot_id_from_gpio.o
else ifeq ($(CONFIG_SLOT_ID_STUB),y)
    drv_soc_misc-objs += features/slot_id_stub.o
else
    $(error error failed no feature defined)
endif
