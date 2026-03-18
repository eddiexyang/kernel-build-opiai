ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef $(WDATE_TIME)
ccflags-y += -Wstack-usage=2048 -Wno-unused-parameter -Wno-sign-compare
ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif
EXTRA_CFLAGS += -DCONFIG_USB_PCS -DCONFIG_UDRV
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_UNIFORM_DIR)/src/include/kerndrv_api

obj-m += drv_usb_uniform.o
drv_usb_uniform-objs := usb_hisi.o usb_hiusbc3xv100.o usb_hiusbc3xv100_pcs.o usb_hiusbc3xv100_subctrl.o
