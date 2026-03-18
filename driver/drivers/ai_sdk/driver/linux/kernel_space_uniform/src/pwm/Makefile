ifeq ($(TOP_DIR),)
KERNEL_DIR :=$(srctree)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/../../drivers/dev_inc
else
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
endif
EXTRA_CFLAGS += -fstack-protector-all
obj-m   += pwm.o
pwm-objs := pwm_drv.o